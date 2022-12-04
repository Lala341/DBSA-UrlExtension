/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"

static URL* build_url_with_port(char *protocol, char *host, unsigned port, char *path){
	
    // Sizes = (0:Protocol, 1:Host, 2:Path)
    int sizes[3];
    sizes[0] = strlen(protocol);
    sizes[1] = strlen(host);
    sizes[2] = strlen(path);

    size_t size = VARHDRSZ + 5*4 + sizes[0] + sizes[1] + sizes[2] + 3;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    offset = copyString(u, &u->host, host, sizes[1], offset);
    offset = copyString(u, &u->path, path, sizes[2], offset);
    // Reset other fields, otherwise it would reference other memory parts thus leading to crash
    u->query = 0;
    u->fragment = 0;
    u->port = port;

    return u;
}

static URL* build_url_with_all_parts(char *protocol, char *host, unsigned port, char *path, char *query, char *fragment){
	
    // Sizes = (0:Protocol, 1:Host, 2:Path, 3:Query, 4:Fragment)
    int sizes[5];
    sizes[0] = strlen(protocol);
    sizes[1] = strlen(host);
    sizes[2] = strlen(path);
    sizes[3] = strlen(query);
    sizes[4] = strlen(fragment);

    size_t size = VARHDRSZ + 5*4 + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + 3;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    offset = copyString(u, &u->host, host, sizes[1], offset);
    offset = copyString(u, &u->path, path, sizes[2], offset);
    offset = copyString(u, &u->query, query, sizes[3], offset);
    copyString(u, &u->fragment, fragment, sizes[4], offset);
    
    u->port = port;

    return u;
}

URL * url_constructor_spec(char* spec){


  
    regex_t rx_p;
    int rc_p;
    regmatch_t pmatch[8];
    char msg_p[100];
    const char *p_regex ="^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*";

    if (0 != (rc_p = regcomp(&rx_p, p_regex, REG_EXTENDED))) {

        regerror(rc_p, &rx_p, msg_p, 100); ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc_p, msg_p)
            )
        );
    }

    
    if (0 != (rc_p = regexec(&rx_p, spec, 8, pmatch, 0))) {

        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Error Code: %d): \"%s\"", rc_p, spec)
            )
        );
    }
    //check each part
    int pos_protocol=1;
    int pos_host=2;
    int current=2;
    
    int port_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):([0-9]{1,4})(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    int pos_port=current+port_c;
    current=pos_port;

    int path_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)+\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    int pos_path=current+path_c;
    current=pos_path;

    int query_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)+(\\#[A-Za-z0-9]+)*");
    int pos_query=current+query_c;
    current=pos_query;

    int fragment_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)+");
    int pos_fragment=current+fragment_c;

   
    char *protocol = removeChar(extractStr(pmatch[pos_protocol], spec), ':');
    char *host = extractStr(pmatch[pos_host], spec);
    unsigned int port = 0;
    if(port_c>0){port = atoi(removeChar(extractStr(pmatch[pos_port], spec), ':'));}
    char *path = "";
    if(path_c>0){path = extractStr(pmatch[pos_path], spec);}
    char *query = "";
    if(query_c>0){query =removeChar(extractStr(pmatch[pos_query], spec), '?');}
    char *fragment = "";
    if(fragment_c>0){fragment =removeChar(extractStr(pmatch[pos_fragment], spec),'#');}


   // ereport(NOTICE,errmsg("protocol: %s", protocol));
  //  ereport(NOTICE,errmsg("host: %s", host));
  //  ereport(NOTICE,errmsg("port: %d", port));
  //  ereport(NOTICE,errmsg("path: %s", path));
  //  ereport(NOTICE,errmsg("query: %s", query));
  //  ereport(NOTICE,errmsg("fragment: %s", fragment));

    
    URL *url = build_url_with_all_parts(protocol, host, port, path, query, fragment);
    
    regfree(&rx_p);

    return url;
}   



static inline char* url_to_str(const URL * url)
{
    size_t size = url->protocol + url->host + 3; // Extra ://
    // Construct char * from the len fields in url
    char *protocol = url->data;
    char *host = url->data + url->protocol;
    char *path = url->data + url->protocol + url->host;
    unsigned port_len = num_digits(url->port);

    char *result;

    if(url->port > 0)       size += port_len + 1;  // For : prefix
    if(url->path > 0)       size += url->path + 1; // For an extra / prefix
    if(url->query > 0)      size += url->query + 1; // For an extra ? prefix
    if(url->fragment > 0)   size += url->fragment + 1;  // For an extra # prefix

    // The 5 extra char represents the :// after protocol and : and /
    result = palloc(size);

    if(url->port > 0) {
        result = psprintf("%s://%s:%d", protocol, host, url->port);
    } else {
        result = psprintf("%s://%s", protocol, host);
    }

    if(url->path > 0) {
        result = psprintf("%s/%s", result, path);
    }

    if(url->query > 0){
        char *query = path + url->query;
        result = psprintf("%s?%s", result, query);
    }

    if(url->fragment > 0){
        char *fragment = path + url->query + url->fragment;
        result = psprintf("%s#%s", result, fragment);
    }
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
    char *spec = PG_GETARG_CSTRING(0);
    PG_RETURN_POINTER( url_constructor_spec(spec) );
}

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    // pg_detoast_datum retrieves unpacks the detoasted input with alignment order intact
    url = (URL *) pg_detoast_datum(input_arr);
    PG_RETURN_CSTRING( url_to_str(url) );
}

PG_FUNCTION_INFO_V1(construct_url_with_port);
Datum construct_url_with_port(PG_FUNCTION_ARGS){

    char *protocol = NULL, *host = NULL, *path = NULL;
    protocol = PG_GETARG_CSTRING(0);
    host = PG_GETARG_CSTRING(1);
    unsigned port = PG_GETARG_INT32(2);
    path = PG_GETARG_CSTRING(3);
    
    PG_RETURN_POINTER( build_url_with_port(protocol, host, port, path) );
}

PG_FUNCTION_INFO_V1(construct_url_without_port);
Datum construct_url_without_port(PG_FUNCTION_ARGS){

    char *protocol = NULL, *host = NULL, *path = NULL;
    protocol = PG_GETARG_CSTRING(0);
    host = PG_GETARG_CSTRING(1);
    path = PG_GETARG_CSTRING(2);
    unsigned port = extract_port_from_protocol(protocol);
    
    PG_RETURN_POINTER( build_url_with_port(protocol, host, port, path) );
}


PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    char *result = palloc(url->protocol + 1);
    result = psprintf("%s", url->data);
    PG_RETURN_CSTRING( result );
}


// /**
//  * Used for Casting Text -> URL
//  */
// PG_FUNCTION_INFO_V1(text_to_url);
// Datum text_to_url(PG_FUNCTION_ARGS)
// {
//     text *txt = PG_GETARG_TEXT_P(0);
//     char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
//     URL * r = url_constructor_spec( str );
//     pfree(str);
//     pfree(txt);
//     pfree(r);
//     PG_RETURN_POINTER( r );
// }

// /**
//  * Used for Casting URL -> Text
//  */
// PG_FUNCTION_INFO_V1(url_to_text);
// Datum url_to_text(PG_FUNCTION_ARGS)
// {
//     // URL *s = (URL *) ; 
    
//     URL *url = (URL *) PG_GETARG_POINTER(0);   
//     pfree(url);
//     // char * ss = url_to_str(s);
//     // text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum(url) );
//     // pfree(url);
//     // pfree(ss);
//     // pfree(out);
//     PG_RETURN_CSTRING(url->protocol);
// }


// To be continue --start
PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    char *result;
    char *host = url->data + url->protocol;
    
    if(url->port > 0) {
        result = palloc(url->host + num_digits(url->port) + 2); // 2 for : and \0
        result = psprintf("%s:%d", host, url->port);
    } else {
        result = palloc(url->host + 1);
        result = psprintf("%s", host);
    }
    PG_RETURN_CSTRING( result );
}


/*

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    if(url->path != "" && url->query != "")
        PG_RETURN_CSTRING( psprintf("%s?%s", url->path, url->query) );
    if(url->path != "")
        PG_RETURN_CSTRING( psprintf("%s", url->path) );
    if(url->query != "")
        PG_RETURN_CSTRING( psprintf("?%s", url->query) );
    PG_RETURN_CSTRING("");
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url->path );
}


// not working
PG_FUNCTION_INFO_V1(same_host);
Datum same_host(PG_FUNCTION_ARGS)
{
        PG_RETURN_BOOL( true );

    // const URL *url_1 = (URL *) PG_GETARG_POINTER(0);
    // const URL *url_2 = (URL *) PG_GETARG_POINTER(1);
    // if (strcmp(namet2, nameIt2) != 0)
    // if (namet2 != nameIt2)
    // PG_RETURN_BOOL( true );
}

// not working
PG_FUNCTION_INFO_V1(get_default_port);
Datum get_default_port(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    // int j = 0;
    // char str[6] = {0,0,0,0,0,0};
    // strncpy(str, url->protocol, 6);
    
    // char ch;
    // while (str[j]) {
        
    //     ch = str[j];
    //     putchar(toupper(ch));
    //     j++;
    // }
    if (url->protocol == "HTTP") 
        PG_RETURN_INT32( 80 );
    if (url->protocol == "HTTPS") 
        PG_RETURN_INT32( 443 );
    if (url->protocol == "FTP") 
        PG_RETURN_INT32( 21 );
    PG_RETURN_INT32( 0 );
}

PG_FUNCTION_INFO_V1(get_host);
Datum get_host(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url->host );
}

PG_FUNCTION_INFO_V1(get_port);
Datum get_port(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32( url->port );
}

PG_FUNCTION_INFO_V1(get_query);
Datum get_query(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url->query );
}

PG_FUNCTION_INFO_V1(get_ref);
Datum get_ref(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url->fragment );
}


PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS)
{
    text *txt1 = PG_GETARG_TEXT_P(0);
    text *txt2 = PG_GETARG_TEXT_P(1);
    char *str1 = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt1) ) );
    char *str2 = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt2) ) );
    int len = strlen(str1);
    if(len != strlen(str2))
        PG_RETURN_BOOL( false );
    PG_RETURN_BOOL( compairChars(str1, str2, len) );
}

// To be continue --finish

*/

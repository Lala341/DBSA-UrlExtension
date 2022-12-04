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

    size_t size = VARHDRSZ + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + 5*6;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    offset = copyString(u, &u->host, host, sizes[1], offset);
    offset = copyString(u, &u->path, path, sizes[2], offset);
    offset = copyString(u, &u->query, query, sizes[3], offset);
    offset = copyString(u, &u->fragment, fragment, sizes[4], offset);
    
    u->port = port;
    // elog(INFO,"P:%d H:%d, Path:%d, Q:%d, F:%d OFF:%d", u->protocol, u->host, u->path, u->query, u->fragment, offset);
    return u;
}

URL * url_constructor_spec(char* spec){

    char *protocol = "";
    char *host = "";
    unsigned port  = 0;
    char *path = "";
    char *query = "";
    char *fragment = "";

       
    bool general = check_regex_part(true,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    //check each part
    bool port_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):([0-9]{1,4})(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    
    bool has_path_division_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?\\/([A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    bool path_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)+\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    bool query_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)+(\\#[A-Za-z0-9]+)*");
    bool fragment_c = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)+");
    

    // Protocol
    char *p;
    p = strtokm(spec, "://");
    if(p) protocol = p;
    
    p = strtokm(NULL, "://");
    if(p){
        bool previoussplit = false;

        const char *tempport = stripString(p);
        char *charactersplit = "/";

        if(has_path_division_c == false){
            charactersplit = "?";
            if(query_c == false){
                charactersplit = "#";
            }
            previoussplit = true;
        }
        p = strtok(p, charactersplit);
        if(p){

            if(p && port_c == false)
                host = p;

            tempport = stripString(p);
            p = strtok(NULL, "");

        }
            
        const char *data = stripString(p);
        if(p)
            data = stripString(p);
            
        if(p && port_c == true){
            tempport = strtok(tempport, ":");  
            host = tempport;
            tempport = strtok(NULL, "");  
            port = atoi(tempport);
        }
        
        if(p && previoussplit == true){
            psprintf(p, "%s%s", charactersplit, data);
        }
       
        if(p){
            char *tempquery = p;
            char *charactersplit="?";
            if(query_c == false){
                charactersplit = "#";
            }
            p = stripString(strtok(tempquery, charactersplit));
        }
            if(p && path_c == true){
                path = p; 
                p = strtok(NULL, "");
            }
               
            if(p && query_c == true){
                if(fragment_c == false) query = p;
            }

            if(p && fragment_c == true){
                char *tempquery = stripString(p);
                p = strtok(tempquery, "#");

                if(query_c == true){
                    query = p;
                    p = strtok(NULL, "");
                }
                fragment = p;
            }
       
    }

    elog(INFO,"protocol: %s", protocol);
    elog(INFO,"host: %s", host);
    elog(INFO,"port: %d", port);
    elog(INFO,"path: %s", path);
    elog(INFO,"query: %s", query);
    elog(INFO,"fragment: %s", fragment);

    URL *url = build_url_with_all_parts(protocol, host, port, path, query, fragment);
    return url;
}   



static inline char* url_to_str(const URL * url)
{
    size_t size = url->protocol + url->host + 1; // Extra ://
    // Construct char * from the len fields in url
    char *protocol = url->data;
    char *host = protocol + url->protocol;
    char *path = host + url->host;
    unsigned port_len = num_digits(url->port);
    char *query = path + url->path;
    char *fragment = query + url->query;

    char *result;

    // Since all url segment contains size + 1, that's why we are not adding an extra + 1 in size
    // for example path contains an extra size for '\0'
    if(url->port > 0)       size += port_len;
    if(url->path > 0)       size += url->path;
    if(url->query > 0)      size += url->query;
    if(url->fragment > 0)   size += url->fragment;

    // The 5 extra char represents the :// after protocol and : and /
    result = palloc(size);

    if(url->port > 0)
        result = psprintf("%s://%s:%d", protocol, host, url->port);
    else
        result = psprintf("%s://%s", protocol, host);

    if(url->path > 0)
        result = psprintf("%s/%s", result, path);

    if(url->query > 0)
        result = psprintf("%s?%s", result, query);

    if(url->fragment > 0)
        result = psprintf("%s#%s", result, fragment);

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

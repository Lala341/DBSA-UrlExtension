/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"

URL * url_constructor_spec(char* str){

    // HeapTupleHeader  t = PG_GETARG_HEAPTUPLEHEADER(0);
    // bool is_protocol_null, is_domain_null;
    // Datum protocol, domain;

    // protocol = GetAttributeByName(t, "protocol", &is_protocol_null);
    // domain = GetAttributeByName(t, "domain", &is_domain_null);

    // char *protocol = NULL, *domain = NULL, *port = NULL;
    // if (!PG_ARGISNULL(1)) protocol = TextDatumGetCString(PG_GETARG_DATUM(1));
    // if (!PG_ARGISNULL(2)) domain = TextDatumGetCString(PG_GETARG_DATUM(2));
    // if (!PG_ARGISNULL(3)) port = TextDatumGetCString(PG_GETARG_DATUM(3));


    // ereport(
    //     ERROR,
    //     (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //     errmsg("No %s, %s, %s", protocol, domain, port))
    // );

    // size_t arg_len = VARSIZE(str) + VARHDRSZ;
    // char *raw = palloc0((strlen(str) + 1) * sizeof(char));

    // size_t size = VARSIZE(str);
    // text *destination = (text *) palloc(VARHDRSZ + size);
    // destination->length = VARHDRSZ + size;
    // memcpy(destination->data, buffer, 40);

    // if (sscanf(str, "( %s )", raw) != 1)
    // {
    //     ereport(
    //         ERROR,
    //         (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //         errmsg("Invalid input syntax for type 'URL': \"%s\"", str))
    //     );
    // }
    // Remove quotes and special char
    // char *spec = stripString(str);
    // pfree(raw);

    URL * url = (URL *) palloc( sizeof(URL) );
    url->protocol = "";
    url->host = "";
    url->port = 0;
    url->query = "";
    url->fragment = "";

    regex_t rx;
    int rc;
    char msg[100];
    const char *string_regex ="^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{2,4})?((\\/[A-Za-z0-9]+)*)*\\/?(\\?[A-Za-z0-9]+\\=[A-Za-z0-9]+(\\&[A-Za-z0-9]+\\=[A-Za-z0-9]+)*)*(\\#[A-Za-z0-9]+)*";

    if (0 != (rc = regcomp(&rx, string_regex, REG_EXTENDED))) {

        regerror(rc, &rx, msg, 100);
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc, msg)
            )
        );
    }

    // size_t match_count = rx.re_nsub + 1;
    size_t match_count = 9;
    regmatch_t pmatch[9];

    if (0 != (rc = regexec(&rx, str, match_count, pmatch, 0))) {
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Error Code: %d): \"%s\"", rc, str)
            )
        );
    }

    url->protocol = removeChar(extractStr(pmatch[1], str), ':');
    url->host = extractStr(pmatch[2], str);
    url->port = atoi(removeChar(extractStr(pmatch[3], str), ':'));
    url->path = extractStr(pmatch[4], str);
    url->query = removeChar(extractStr(pmatch[6], str), '?');
    url->fragment = removeChar(extractStr(pmatch[8], str),'#');

    // PG_RETURN_TEXT_P( &url );
    // pfree(url);
    regfree(&rx);
    return url;
}   

PG_FUNCTION_INFO_V1(url_constructor_port);
Datum url_constructor_port(PG_FUNCTION_ARGS){

    char *spec = NULL, *protocol = NULL, *host = NULL, *file = NULL;
    int port = 0;
    protocol = PG_GETARG_CSTRING(0);
    host = PG_GETARG_CSTRING(1);
    port = PG_GETARG_INT32(2);
    file = PG_GETARG_CSTRING(3);

    size_t size = VARHDRSZ + sizeof(URL);
    URL * url = (URL *) palloc0( size );
    SET_VARSIZE(url, size);
    
    url->protocol = copyStr(protocol);
    url->host = copyStr(host);
    url->port = 0;
    // url->port = port;
    url->query = copyStr(file);
    url->fragment = "";

    pfree(protocol);
    pfree(host);
    pfree(file);
    
    PG_RETURN_POINTER( url );
}

static inline char* url_to_str(const URL * url)
{
    char *authority;
    char *result;
    if(url->port > 0)
        authority = psprintf("%s:%d", url->host, url->port);
    else
        authority = psprintf("%s", url->host);
    result = psprintf("%s://%s", url->protocol, authority);
    if(url->path)
        strcat(result, url->path);
    if(url->query) 
        result = psprintf("%s?%s", result, url->query);
    if(url->fragment)
        result = psprintf("%s#%s", result, url->fragment);
    pfree(url);
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
    char *spec = NULL, *protocol = NULL, *file = NULL;
    // switch(PG_NARGS()) {
    //     int port = 0;
    //     case 4:
    //         protocol = PG_GETARG_CSTRING(0);
    //         domain = PG_GETARG_CSTRING(1);
    //         port = PG_GETARG_INT32(2);
    //         file = PG_GETARG_CSTRING(3);
    //         PG_RETURN_POINTER( url_constructor_port(protocol, domain, port, file) );
    //         break;
    //     default:
    // }
    
    spec = PG_GETARG_CSTRING(0);
    URL *r = url_constructor_spec(spec);
    // pfree(spec);
    pfree(r);
    PG_RETURN_POINTER( r );
}

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url_to_str(url) );
}


PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);

    PG_RETURN_CSTRING( url->protocol );
}


/**
 * Used for Casting Text -> URL
 */
PG_FUNCTION_INFO_V1(text_to_url);
Datum text_to_url(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
    URL * r = url_constructor_spec( str );
    pfree(str);
    pfree(txt);
    pfree(r);
    PG_RETURN_POINTER( r );
}

/**
 * Used for Casting URL -> Text
 */
PG_FUNCTION_INFO_V1(url_to_text);
Datum url_to_text(PG_FUNCTION_ARGS)
{
    // URL *s = (URL *) ; 
    
    URL *url = (URL *) PG_GETARG_POINTER(0);   
    pfree(url);
    // char * ss = url_to_str(s);
    // text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum(url) );
    // pfree(url);
    // pfree(ss);
    // pfree(out);
    PG_RETURN_CSTRING(url->protocol);
}

// To be continue --start
PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    if(url->port > 0) 
        PG_RETURN_CSTRING(psprintf("%s:%d", url->host, url->port));
    else 
        PG_RETURN_CSTRING( url->host );
    //it returns a random char in the first time only
}

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

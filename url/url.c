/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"

static inline URL* str_to_url(const char* str)
{
    char spec[200];

    if (sscanf(str, "(%s)", spec) != 1)
    {
        ereport(
            ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("Invalid input syntax for type 'URL': \"%s\"", str))
        );
    }

    URL url = {
        .protocol = "http", 
        .host = "",
        .port = 0,
        .query = "",
        .fragment = ""
    };

    regex_t rx;
    int rc;
    regmatch_t pmatch[2];
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

    size_t match_count = rx.re_nsub + 1;

    if (0 != (rc = regexec(&rx, spec, match_count, pmatch, 0))) {
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Error Code: %d): \"%s\"", rc, spec)
            )
        );
    }

    url.protocol = extractStr(pmatch[1], str);
    removeChar(url.protocol, ':');
    url.host = extractStr(pmatch[2], str);
    // Protocol
    char *protocol_str = extractStr(pmatch[3], str);
    removeChar(protocol_str, ':');
    // Cast to int
    url.port = atoi(protocol_str);
    // Path
    url.path = extractStr(pmatch[4], str);
    char *query_str = extractStr(pmatch[6], str);
    removeChar(query_str, '?');
    url.query = query_str;
    

    // URL * s = (URL *) palloc( sizeof(URL) );

    return url;
}

static inline const char* url_to_str(const URL* url)
{
    char *authority = calloc(50, sizeof(char));
    char *result = calloc(100, sizeof(char));
    
    // if(url->protocol) 
    //     fprintf(stdout, "Protocol: %s\n", url->protocol);
    // if(url->host) 
    //     fprintf(stdout, "Host: %s\n", url->host);
    // fprintf(stdout, "Port: %d\n", url->port);
    if(url->port > 0) {
        sprintf(authority, "%s:%d", url->host,url->port);
        // fprintf(stdout, "Authority: %s:%d\n", url->host, url->port);
    } else {
        sprintf(authority, "%s", url->host);
        // fprintf(stdout, "Authority: %s\n", url->host);
    }
    sprintf(result, "%s://%s", url->protocol, authority);
    // strcat(result, url->protocol);
        // strcat(result, "://");
    // strcat(result, authority);
    // fprintf(stdout, "result with protocol: %s\n", result);
    if(url->path) {
        strcat(result, url->path);
        // fprintf(stdout, "Path: %s\n", url->path);
    }
    if(url->query != "") {
        char * temp = result;
        sprintf(result, "%s?%s", temp, url->query);
        // fprintf(stdout, "Query: %s\n", url->query);
    }
    if(url->fragment != ""){ 
        char * temp = result;
        sprintf(result, "#%s", url->fragment);
        // fprintf(stdout, "Fragment: %s\n", url->fragment);
    }
    // fprintf(stdout, "result with fragment: %s\n", result);
    // char * result = psprintf("%s", result);
    return result;

}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
    char *str = PG_GETARG_CSTRING(0);
    PG_RETURN_TEXT_P( str_to_url(str));
    // PG_RETURN_POINTER( str_to_url( str ) );
}   

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    // Datum arg = PG_GETARG_DATUM(0);

	// PG_RETURN_CSTRING( url_to_str() );
    const URL *s = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url_to_str(s) );
}

/**
 * Used for Casting Text -> URL
//  */
// PG_FUNCTION_INFO_V1(text_to_url);
// Datum text_to_url(PG_FUNCTION_ARGS)
// {
//     text *txt = PG_GETARG_TEXT_P(0);
//     char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
//     PG_RETURN_POINTER( str_to_url( str ) );
// }

/**
 * Used for Casting URL -> Text
 */
// PG_FUNCTION_INFO_V1(url_to_text);
// Datum url_to_text(PG_FUNCTION_ARGS)
// {
//     const URL *s = (URL *) PG_GETARG_POINTER(0);    
//     text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum( url_to_str(s) ) );
//     PG_RETURN_TEXT_P(out);
// }

/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h";
#include <regex.h>

static inline url* str_to_url(const char* str)
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
        .authority = "",
        .host = "",
        .port = -1,
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
        printf("regcomp() failed, returning nonzero (%d), msg = %s\n", rc, msg);
        return -1;
    }

    size_t match_count = rx.re_nsub + 1;

    if (0 != (rc = regexec(&rx, spec, match_count, pmatch, 0))) {
        printf("Failed to match '%s' with pattern ,returning %d.\n", spec, rc);
    }

    int len = 0;
    if( pmatch[1].rm_so >= 0) {
        len = pmatch[1].rm_eo - pmatch[1].rm_so;
        url.protocol = calloc( len + 1, sizeof(char));
        // Copy the url from regex find_start till find_end
        strncpy(url.protocol, spec + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
        // Remove the colon if any
        removeChar(url.protocol, ':');
    }

    if( pmatch[2].rm_so >= 0) {
        len = pmatch[2].rm_eo - pmatch[2].rm_so;
        url.host = calloc( len + 1, sizeof(char));
        strncpy(url.host, spec + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
    }

    if( pmatch[3].rm_so >= 0) {
        len = pmatch[3].rm_eo - pmatch[3].rm_so;
        char *protocol_str = calloc( len + 1, sizeof(char));
        strncpy(protocol_str, spec + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
        removeChar(protocol_str, ':');
        // Cast to int
        url.port = atoi(protocol_str);
    }

    // URL * s = (URL *) palloc( sizeof(URL) );

    return url;
}

static inline const char* url_to_str(const url* s)
{
    // TODO: Add check if the port is similar as protocol then don't print port, only print when it's not default

    // TODO: Add check if query, fragment are not available then don't add
    if(url.protocol) 
        fprintf(stdout, "Protocol: %s\n", url.protocol);
    if(url.host) 
        fprintf(stdout, "Host: %s\n", url.host);
    fprintf(stdout, "Port: %d\n", url.port);
    if(url.port > 0) {
        sprintf(url.authority, "%s:%d", url.host,url.port);
        fprintf(stdout, "Authority: %s:%d\n", url.host, url.port);
    } else {
        url.authority = url.host
        fprintf(stdout, "Authority: %s\n", url.host);
    }
    if(url.path) 
        fprintf(stdout, "Path: %s\n", url.path);
    if(url.query) 
        fprintf(stdout, "Query: %s\n", url.query);
    if(url.fragment) 
        fprintf(stdout, "Fragment: %s\n", url.fragment);

    char * result = psprintf("%s://%s/%s?%s#%s", u->protocol, u->authority, u->path, u->query, "#" + u->fragment);
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
    char *str = PG_GETARG_CSTRING(0);
    PG_RETURN_TEXT_P(str);
    // PG_RETURN_POINTER( str_to_url( str ) );
}   

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    Datum arg = PG_GETARG_DATUM(0);

	PG_RETURN_CSTRING();
    // const url *s = (url *) PG_GETARG_POINTER(0);
    // PG_RETURN_CSTRING( url_to_str(s) );
}

/**
 * Used for Casting Text -> URL
 */
PG_FUNCTION_INFO_V1(text_to_url);
Datum text_to_url(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
    PG_RETURN_POINTER( str_to_url( str ) );
}

/**
 * Used for Casting URL -> Text
 */
PG_FUNCTION_INFO_V1(url_to_text);
Datum url_to_text(PG_FUNCTION_ARGS)
{
    const URL *s = (URL *) PG_GETARG_POINTER(0);    
    text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum( url_to_str(s) ) );
    PG_RETURN_TEXT_P(out);
}

/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "testurl.h"

TESTURL * url_constructor_spec(char* str){

    TESTURL * url = (TESTURL *) palloc( sizeof(TESTURL) );
    url->protocol = "https";
    url->host = "stackoverflow.com";
    url->port = 8080;
    url->query = "/test";
    url->fragment = "#new-start";
    
    return url;
}   


static inline char* url_to_str(const TESTURL * url)
{
    char * result = psprintf("%s://%s:%d/%s#%s", url->protocol, url->host,url->port, url->query, url->fragment);
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(test_url_in);
Datum test_url_in(PG_FUNCTION_ARGS){
    char *spec = PG_GETARG_CSTRING(0);
    PG_RETURN_POINTER( url_constructor_spec(spec) );
}

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(test_url_out);
Datum test_url_out(PG_FUNCTION_ARGS)
{
    const TESTURL *url = (TESTURL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url_to_str(url) );
}
/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "testurl.h"

 TESTURL * url_constructor_spec(char* str){

char *prot="hy";
     TESTURL * url = ( TESTURL *) palloc( sizeof(TESTURL) );
    url->protocol =prot;
    url->host = "stackoverflow.com";
    url->port = 8080;
    url->query = "/test";
    url->fragment = "#new-start";
    return url;
}   

char * concat_text(char *arg1, char *arg2 )
{
    int new_text_size = VARSIZE(arg1) + VARSIZE(arg2) - VARHDRSZ;
    char *new_text = (char *) palloc(new_text_size);

    memcpy(VARDATA(new_text), VARDATA(arg1), VARSIZE(arg1) - VARHDRSZ);
    memcpy(VARDATA(new_text) + (VARSIZE(arg1) - VARHDRSZ),
           VARDATA(arg2), VARSIZE(arg2) - VARHDRSZ);
    return (new_text);
}

 char * copytext(char *t)
{
   char     *new_t = (char *) palloc(VARSIZE(t));
   memcpy((void *) VARDATA(new_t), /* destination */
           (void *) VARDATA(t),     /* source */
           VARSIZE(t) - VARHDRSZ);  /* how many bytes */
    return new_t;
}

static inline char* url_to_str( TESTURL * url)
{
  

//char *temp;
//temp=copytext(url->protocol);
  char *result;
  asprintf(&result,"Test 1 %s",url->protocol);
 asprintf(&result,"Test 1 %s",url->host);

 // pfree(ptr);
   
    //pfree(result);
    //pfree(url);
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(test_url_in);
Datum test_url_in(PG_FUNCTION_ARGS){
    char *spec = PG_GETARG_CSTRING(0);
    PG_RETURN_TEXT_P( url_constructor_spec(spec));
}

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(test_url_out);
Datum test_url_out(PG_FUNCTION_ARGS)
{
     TESTURL *url = (  TESTURL*) PG_GETARG_TEXT_P(0);
	PG_RETURN_CSTRING(url_to_str(url));

}
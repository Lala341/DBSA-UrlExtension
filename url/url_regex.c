/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"



int check_regex_part(bool showerror, char* str, char* data){


    char *spec = stripString(str);
    regex_t rx_p;
    int rc_p;
    regmatch_t pmatch_p[8];
    char msg_p[100];
    const char *p_regex =data;

    if (0 != (rc_p = regcomp(&rx_p, p_regex, REG_EXTENDED))) {

        regerror(rc_p, &rx_p, msg_p, 100); ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc_p, msg_p)
            )
        );
    }

    
    if (0 != (rc_p = regexec(&rx_p, spec, 8, pmatch_p, 0))) {

        if(showerror==true){
             ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Error Code: %d): \"%s\"", rc_p, spec)
            )
        );
        }
     //  pfree(pmatch_p);
       return 0;
    }
  //  pfree(pmatch_p);
    return 1;

}


char *strtokm(char *str, const char *delim)
{
    static char *tok;
    static char *next;
    char *m;

    if (delim == NULL) return NULL;

    tok = (str) ? str : next;
    if (tok == NULL) return NULL;

    m = strstr(tok, delim);

    if (m) {
        next = m + strlen(delim);
        *m = '\0';
    } else {
        next = NULL;
    }

    return tok;
}
URL * url_constructor_spec(char* str){

   
    char *spec = stripString(str);
    URL * url = (URL *) palloc0( sizeof(URL) );
    

    int general = check_regex_part(true,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    //check each part
    int current=0;
    int pos_protocol=1;
    int pos_host=2;
    
    int port = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):([0-9]{1,4})(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    int pos_port=current+port;
    current=current+1;

    int path = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)+\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    
    int pos_path=current+path;
    current=current+1;

    int query = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)+(\\#[A-Za-z0-9]+)*");
    
    int pos_query=current+query;
    current=current+1;

    int fragment = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)+");
    
    int pos_fragment=current+fragment;

    url->full="";


  
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
    }
     //  pfree(pmatch_p);
   ereport(
            NOTICE,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
 errmsg("Invalid URL pattern provided (Error Code: %d):  %s", pmatch[pos_protocol].rm_eo,extractStrF(pmatch[pos_protocol], spec))
            )
        );
        ereport(
            NOTICE,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
 errmsg("Invalid URL pattern provided (Error Code: %d):  %s", pmatch[pos_fragment].rm_eo,extractStrF(pmatch[pos_fragment],spec))
            )
        );
    asprintf(&url->protocol,extractStrF(pmatch[pos_protocol], spec));
    asprintf(&url->host,extractStrF(pmatch[pos_host], spec));
    asprintf(&url->port,extractStrF(pmatch[pos_port], spec));
    asprintf(&url->path,extractStrF(pmatch[pos_path], spec));
    asprintf(&url->query,extractStrF(pmatch[pos_query], spec));
    asprintf(&url->fragment,extractStrF(pmatch[pos_fragment], spec));

  //  pfree(pmatch_p);

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
    url->port = port;
    url->query = copyStr(file);
    url->fragment = "";
    
    PG_RETURN_TEXT_P( url );
}


static inline char* url_to_str( URL * url)
{       
    char *result;
    asprintf(&result,"URL: %s",url->protocol);
    asprintf(&result,"%s %s %s %s %s %s",result, url->host, url->port,url->path, url->query, url->fragment);


    char *authority;
    asprintf(&authority,"%s",url->host);

    if(url->port != NULL){
    asprintf(&authority,"%s:%s",authority,url->port);
    }

    char *final_part;
    asprintf(&final_part,"%s","");

    if(url->path!=NULL){
    asprintf(&final_part,"%s/%s",final_part,url->path);
    }
    if(url->query!=NULL){
    asprintf(&final_part,"%s?%s",final_part,url->query);
    }
    if(url->fragment!=NULL){
    asprintf(&final_part,"%s#%s",final_part,url->fragment);
    }

    
  //  asprintf(&result,"%s://%s%s",url->protocol, authority, final_part);

    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){

    char *spec= (char *) PG_GETARG_CSTRING(0);
    PG_RETURN_TEXT_P( url_constructor_spec(spec) );
}

/**
 * Construct string from url -> toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    
    URL *url = (URL *) PG_GETARG_TEXT_P(0);
    PG_RETURN_CSTRING( url_to_str(url) );
}


PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_TEXT_P(0);
    PG_RETURN_CSTRING( url->protocol );
}


/**
 * Used for Casting Text -> URL
 */
PG_FUNCTION_INFO_V1(text_to_url);
Datum text_to_url(PG_FUNCTION_ARGS)
{
    
    char *spec= (char *) PG_GETARG_CSTRING(0);
    PG_RETURN_TEXT_P( url_constructor_spec(spec) );
}

/**
 * Used for Casting URL -> Text
 */
PG_FUNCTION_INFO_V1(url_to_text);
Datum url_to_text(PG_FUNCTION_ARGS)
{
    URL *s = (URL *) PG_GETARG_TEXT_P(0);  
    PG_RETURN_TEXT_P(url_to_str(s));
}

// To be continue --start
PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_TEXT_P(0);
    if(url->port > 0) 
        PG_RETURN_CSTRING(psprintf("%s:%s", url->host, url->port));
    else 
        PG_RETURN_CSTRING( url->host );
}

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_TEXT_P(0);
    PG_RETURN_CSTRING( psprintf("%s?%s", url->path, url->query) );
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_TEXT_P(0);
    PG_RETURN_CSTRING( url->path );
}

PG_FUNCTION_INFO_V1(same_host);
Datum same_host(PG_FUNCTION_ARGS)
{
    const URL *url_1 = (URL *) PG_GETARG_TEXT_P(0);
    const URL *url_2 = (URL *) PG_GETARG_TEXT_P(1);
    // if (strcmp(namet2, nameIt2) != 0)
    // if (namet2 != nameIt2)
    PG_RETURN_CSTRING( url_1 );
}

/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"



bool check_regex_part(bool showerror, char* str, char* data){


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
       return false;
    }
    return true;

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
    url->protocol = "";
    url->host = "";
    url->port = 0;
    url->path = "";
    url->query = "";
    url->fragment = "";

    bool general = check_regex_part(true,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    //check each part
    bool port = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):([0-9]{1,4})(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    bool path = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)+\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    bool query = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)+(\\#[A-Za-z0-9]+)*");
    bool fragment = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)+");
    
    // Protocol
    char *p;
    p = strtokm(spec, "://");

    if(p)
    {
        ereport(NOTICE,( errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), errmsg("Test 1 %s\n", p)));
        url->protocol =p;
    }
    p = strtokm(NULL, "://");
    
    if(p){
        ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 2 %s\n", p)));
        bool previoussplit=false;
        char *charactersplit="/";
        if(path==false){
            charactersplit="?";
            if(query==false){
                charactersplit="#";
            }
            previoussplit=true;
        }
       p = strtok(p, charactersplit);
       if(p){
        ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 51 %s\n", p)));

        if(p&&port==false){
            url->host=p;
        }
    
        ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 21 %s\n", p)));
        const char *tempport=stripString(p);
        p=strtok(NULL, "");
        if(p){
                const char *data=stripString(p);
                p=stripString(data);

        
        
        if(p&&port==true){


            tempport = strtok(tempport, ":");  
            url->host=tempport;
            tempport = strtok(NULL, "");  
            url->port=  atoi(tempport);
        }
        
        ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 21 %s\n", p)));

        if(p&&previoussplit==true){
            psprintf(p, "%s%s",charactersplit,data);
            ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 21 %s\n", p)));

        }
        }
        
       }
        if(p){
            ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 4 %s\n", p)));

            char *tempquery=p;
            char *charactersplit="?";
            if(query==false){
                charactersplit="#";
            }
            p = stripString(strtok(tempquery, charactersplit));
            ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 51 %s\n", p)));

        }
            if(p&&path==true){
                ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 5 %s\n", p)));
                url->path=  p; 
                p = strtok(NULL, "");
            }
               
            if(p&&query==true){
                ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 7 %s\n", p)));
                
                if(fragment==false){
                    url->query=  p;
                }
            }
             if(p&&fragment==true){
                
                char *tempquery=stripString(p);
                p = strtok(tempquery, "#");
                ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 8 %s\n", p)));

                if(query==true){
                    url->query=  p;
                    p = strtok(NULL, "");

                }
                ereport( NOTICE,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),errmsg("Test 8 %s\n", p)));
                url->fragment=  p;
        }
       
    }
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
    
    PG_RETURN_POINTER( NULL );
}

static inline char* url_to_str(const URL * url)
{

    // // PG_RETURN_CSTRING( url_to_str() );
    char * authority = psprintf("%s", url->host);
    if(url->port > 0){
      authority = psprintf("%s:%d", url->host,url->port);
    }

    char * tempdata = "";
    
    
     if(strcmp(url->path, "")!=0) {
          tempdata = psprintf("%s%s", tempdata,url->path);
     }
     if(strcmp(url->query, "")!=0) {
         tempdata = psprintf("%s?%s", tempdata,url->query);
     }
     if(strcmp(url->fragment, "")!=0){ 
         tempdata = psprintf("%s#%s", tempdata,url->fragment);
     }
  
	
    char * result = psprintf("%s://%s%s", url->protocol, authority,tempdata);
    return result;
}

/**
 * Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
    switch(PG_NARGS()) {
        char *spec = NULL, *protocol = NULL, *domain = NULL, *file = NULL;
        int port = 0;
        // case 4:
        //     protocol = PG_GETARG_CSTRING(0);
        //     domain = PG_GETARG_CSTRING(1);
        //     port = PG_GETARG_INT32(2);
        //     file = PG_GETARG_CSTRING(3);
        //     PG_RETURN_POINTER( url_constructor_port(protocol, domain, port, file) );
        //     break;
        default:
            spec = PG_GETARG_CSTRING(0);
            PG_RETURN_POINTER( url_constructor_spec(spec) );
    }
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
    PG_RETURN_POINTER( url_constructor_spec( str ) );
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

// To be continue --start
PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    if(url->port > 0) 
        PG_RETURN_CSTRING(psprintf("%s:%d", url->host, url->port));
    else 
        PG_RETURN_CSTRING( url->host );
}

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( psprintf("%s?%s", url->path, url->query) );
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING( url->path );
}

PG_FUNCTION_INFO_V1(same_host);
Datum same_host(PG_FUNCTION_ARGS)
{
    const URL *url_1 = (URL *) PG_GETARG_POINTER(0);
    const URL *url_2 = (URL *) PG_GETARG_POINTER(1);
    // if (strcmp(namet2, nameIt2) != 0)
    // if (namet2 != nameIt2)
    PG_RETURN_CSTRING( url_1 );
}
// To be continue --finish
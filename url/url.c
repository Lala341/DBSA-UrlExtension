/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"


URL * url_constructor_spec(char* str){

   
    char *spec = stripString(str);
    URL * url = (URL *) palloc0( sizeof(URL) );
    

    bool general = check_regex_part(true,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    //check each part
    bool port = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):([0-9]{1,4})(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    
    bool has_path_division = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?\\/([A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");

    bool path = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)+\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)*");
    bool query = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)+(\\#[A-Za-z0-9]+)*");
    bool fragment = check_regex_part(false,spec, "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{1,4})?(\\/[A-Za-z0-9]+)*\\/?(\\?[A-Za-z0-9&=]+)*(\\#[A-Za-z0-9]+)+");
    
    url->full="";

    // Protocol
    char *p;
    p = strtokm(spec, "://");
    if(p)
    {
        asprintf(&url->protocol,p);
    }
    p = strtokm(NULL, "://");
    if(p){
        bool previoussplit=false;

        const char *tempport=stripString(p);
        char *charactersplit="/";

        if(has_path_division==false){
            charactersplit="?";
            if(query==false){
                charactersplit="#";
            }
            previoussplit=true;
        }
       p = strtok(p, charactersplit);
       if(p){

        if(p&&port==false){
            asprintf(&url->host,p);
        }
        tempport=stripString(p);
        p=strtok(NULL, "");

       }
            
        const char *data=stripString(p);
        if(p){
            data=stripString(p);
        }
        if(p&&port==true){
            tempport = strtok(tempport, ":");  
            asprintf(&url->host,tempport);
            tempport = strtok(NULL, "");  
            asprintf(&url->port,tempport);
        }
        
        if(p&&previoussplit==true){
            psprintf(p, "%s%s",charactersplit,data);
        }
       
        if(p){
            char *tempquery=p;
            char *charactersplit="?";
            if(query==false){
                charactersplit="#";
            }
            p = stripString(strtok(tempquery, charactersplit));
        }
            if(p&&path==true){
                asprintf(&url->path, p); 
                p = strtok(NULL, "");
            }
               
            if(p&&query==true){
                
                if(fragment==false){
                    asprintf(&url->query,p);
                }
            }
             if(p&&fragment==true){
                
                char *tempquery=stripString(p);
                p = strtok(tempquery, "#");

                if(query==true){
                    asprintf(&url->query,p);
                    p = strtok(NULL, "");

                }
                asprintf(&url->fragment, p);
        }
       
    }
                
    return url;
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

    
    asprintf(&result,"%s://%s%s",url->protocol, authority, final_part);

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


PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS)
{
    const URL *url = (URL *) PG_GETARG_POINTER(0);

    PG_RETURN_CSTRING( url->protocol );
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

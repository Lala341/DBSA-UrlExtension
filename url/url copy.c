/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"

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
    u->userinfo=0;
    u->query = 0;
    u->fragment = 0;
    u->port = port;

    return u;
}

static URL* build_url_with_all_parts(char *protocol, char *userinfo, char *host, unsigned port, char *path, char *query, char *fragment){
	
    // Sizes = (0:Protocol, 1:Host, 2:Path, 3:Query, 4:Fragment)
    int sizes[6];
    sizes[0] = strlen(protocol);
    sizes[1] = strlen(userinfo);
    sizes[2] = strlen(host);
    sizes[3] = strlen(path);
    sizes[4] = strlen(query);
    sizes[5] = strlen(fragment);

    size_t size = VARHDRSZ + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + sizes[5] + 5*6;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    offset = copyString(u, &u->userinfo, userinfo, sizes[1], offset);
    offset = copyString(u, &u->host, host, sizes[2], offset);
    offset = copyString(u, &u->path, path, sizes[3], offset);
    offset = copyString(u, &u->query, query, sizes[4], offset);
    offset = copyString(u, &u->fragment, fragment, sizes[5], offset);
    
    u->port = port;
    // elog(INFO,"P:%d H:%d, Path:%d, Q:%d, F:%d OFF:%d", u->protocol, u->host, u->path, u->query, u->fragment, offset);
    return u;
}
//4,7,9,11
//URG:12 space-tobase64, 13,14,16
//https://url.spec.whatwg.org/#percent-encoded-bytes
URL * url_constructor_spec(char* spec){

    char *protocol = "";
    char *userinfo = "";
    char *host = "";
    unsigned port  = 0;
    char *path = "";
    char *query = "";
    char *fragment = "";

    
if(strchr(spec, ' ') != NULL)
  {
    ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Not whitespaces allowed).")
            ));
  }
    
    bool general = check_regex_part(true,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");

    bool userinfo_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool port_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool has_path_division_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?\\/((\\/)?[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool path_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)+(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool query_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)(\\#[^\\/\\?\\#]+)?)$");
    bool fragment_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+))$");
    

    // Protocol
    char *p;
    p = strtokm(spec, "://");
    if(p) protocol = p;
    
    p = strtokm(NULL, "://");
    if(p){
        bool previoussplit = false;

        const char *tempport = stripString(p);
        if(userinfo_c == true){
            char *charactersplit = "@";

            p = strtok(p, charactersplit);
            if(p){
                userinfo = p;
                tempport = stripString(p);
                p = strtok(NULL, "");
            }
        }
        
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
            
        const char *data = p;
        if(p){
            stripString(p);
            data = stripString(p);
        }  
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
     elog(INFO,"userinfo: %s", userinfo);
     elog(INFO,"host: %s", host);
     elog(INFO,"port: %d", port);
     elog(INFO,"path: %s", path);
     elog(INFO,"query: %s", query);
     elog(INFO,"fragment: %s", fragment);

    URL *url = build_url_with_all_parts(protocol, userinfo, host, port, path, query, fragment);
    return url;
}   


URL * url_constructor_spec_for_context(char* spec, URL * url){


    char *protocol_ind = url->data;
    char *userinfo_ind = protocol_ind + url->protocol;
    char *host_ind = userinfo_ind + url->userinfo;
    char *path_ind = host_ind + url->host;
    char *query_ind = path_ind + url->path;
    char *fragment_ind = query_ind + url->query;

    char *protocol = psprintf("%s",protocol_ind);
    char *userinfo =psprintf("%s",userinfo_ind);
    char *host = psprintf("%s",host_ind);
    char *path = psprintf("%s",path_ind);
    unsigned port_final =url->port;
    char *query = psprintf("%s",query_ind);
    char *fragment = psprintf("%s",fragment_ind);

    char *port = "";


    // Protocol
    char *p= stripString(spec);
    char *last_split_character;
    char *prev_split_character;
    char *rest;
    bool first_part=false;
    char *first_part_s="";
    char *first_split_character;
    char *temp_clean_data;



    p = strtokm(p, "@");
    last_split_character="@";
    prev_split_character="";

     
    if(p){
        
        rest=strtokm(NULL, "@"); 
        if(!rest){
            last_split_character=prev_split_character;

        }else{
             if(first_part==false){
                first_part=true;
                first_part_s=psprintf("%s", p);
                first_split_character=last_split_character;
            }
            p=psprintf("%s", rest);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "?#");
            path=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }
        

    }
    if(p){
         p = strtokm(p, ":");
         last_split_character=":";
    }
    if(p){
        
        rest=strtokm(NULL, ":"); 
        if(!rest){
            last_split_character=prev_split_character;

        }else{
             if(first_part==false){
                first_part=true;
                first_part_s=psprintf("%s", p);
                first_split_character=last_split_character;
            }
            p=psprintf("%s", rest);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "?#");
            path=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }

    }
    if(p){
         p = strtokm(p, "/");
         last_split_character="/";
    }
    if(p){
        
        rest=strtokm(NULL, "/"); 
        if(!rest){
            last_split_character=prev_split_character;

        }else{
             if(first_part==false){
                first_part=true;
                first_part_s=psprintf("%s", p);
                first_split_character=last_split_character;
            }
            p=psprintf("%s", rest);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "?#");
            path=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }

    }
    if(p){
         p = strtokm(p, "?");
         last_split_character="?";
    }
    if(p){
         
        rest=strtokm(NULL, "?"); 
         if(!rest){
            last_split_character=prev_split_character;

        }else{
             if(first_part==false){
                first_part=true;
                first_part_s=psprintf("%s", p);
                first_split_character=last_split_character;
            }
            p=psprintf("%s", rest);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "?#");
            path=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }

    }
    if(p){
         p = strtokm(p, "#");
         last_split_character="#";
    }
    if(p){
         
        rest=strtokm(NULL, "#"); 
        if(!rest){
            last_split_character=prev_split_character;

        }else{
             if(first_part==false){
                first_part=true;
                first_part_s=psprintf("%s", p);
                first_split_character=last_split_character;
            }
            p=psprintf("%s", rest);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "?#");
            path=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }

    }
    if(p){

        if(strcmp(last_split_character,"@")==0){
            host=psprintf("%s", p);
        }else if(strcmp(last_split_character,":")==0){
            port=psprintf("%s", p);
        }else if(strcmp(last_split_character,"/")==0){
            path=psprintf("%s", p);
        }else if(strcmp(last_split_character,"?")==0){
            query=psprintf("%s", p);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }
    }
    //Recognize first part
    
    if(strcmp(first_part_s,"")!=0){
    
    bool path_c = check_regex_part(false,first_part_s, "^\\/");
    bool query_c = check_regex_part(false,first_part_s, "^\\?");
    bool fragment_c = check_regex_part(false,first_part_s, "^\\#");
    

        if(strcmp(first_split_character,"@")==0){
            userinfo=psprintf("%s",first_part_s );
        }else if(strcmp(first_split_character,":")==0){
            host=psprintf("%s", first_part_s);
        }else if(strcmp(first_split_character,"/")==0){
            if(path_c==true){
                path=psprintf("%s", first_part_s);
            }else{
                host=psprintf("%s", first_part_s);
            }
            
        }else if(strcmp(first_split_character,"?")==0){
             if(path_c==true){
                path=psprintf("%s", first_part_s);
            }else if(query_c==true){
                query=psprintf("%s", first_part_s);
            } else{
                host=psprintf("%s", first_part_s);
            }
        }else if(strcmp(first_split_character,"#")==0){
            if(path_c==true){
                path=psprintf("%s", first_part_s);
            }else if(query_c==true){
                query=psprintf("%s", first_part_s);
            }else if(fragment_c==true){
                fragment=psprintf("%s", first_part_s);
            } else{
                host=psprintf("%s", first_part_s);
            }
        }

    }
    if(strcmp(port,"")!=0){
        port_final=atoi(port);
    }

   
    elog(INFO,"protocol: %s", protocol);
    elog(INFO,"userinfo: %s", userinfo);
    elog(INFO,"host: %s", host);
    elog(INFO,"port: %s", port);
    elog(INFO,"path: %s", path);
    elog(INFO,"query: %s", query);
    elog(INFO,"fragment: %s", fragment);

    URL * url_parts = build_url_with_all_parts(protocol, userinfo, host, port_final, path, query, fragment);
    return url_parts;
} 

static inline char* url_to_str(const URL * url)
{
    size_t size = url->protocol + url->host + 1; // Extra ://
    // Construct char * from the len fields in url
    char *protocol = url->data;
    char *userinfo = protocol + url->protocol;
    char *host = userinfo + url->userinfo;
    char *path = host + url->host;
    unsigned port_len = num_digits(url->port);
    char *query = path + url->path;
    char *fragment = query + url->query;

    char *result;

    // Since all url segment contains size + 1, that's why we are not adding an extra + 1 in size
    // for example path contains an extra size for '\0'
    if(url->userinfo > 0)   size += url->userinfo;
    if(url->port > 0)       size += port_len;
    if(url->path > 0)       size += url->path;
    if(url->query > 0)      size += url->query;
    if(url->fragment > 0)   size += url->fragment;

    // The 5 extra char represents the :// after protocol and : and /
    result = palloc(size);

    if(url->userinfo > 0)
        result = psprintf("%s://%s@%s", protocol, userinfo, host);
    else
        result = psprintf("%s://%s", protocol, host);

    if(url->port > 0)
        result = psprintf("%s:%d", result, url->port);

    if(url->path > 0)
        result = psprintf("%s/%s", result, path);

    if(url->query > 0)
        result = psprintf("%s?%s", result, query);

    if(url->fragment > 0)
        result = psprintf("%s#%s", result, fragment);

    return result;
}

static inline char* url_spec_context(const URL * url, const char * spec)
{
    bool protocol_c = check_regex_part(false,spec, "^((.*):\\/\\/([A-Za-z0-9\\.\\:\\_]+@)?([A-Za-z0-9\\.\\_]+)(:[0-9]{1,4})?(\\/[A-Za-z0-9\\_]+)*(\\/)?(\\?[^\\/\\?\\#]+)?(\\#[^\\/\\?\\#]+)?)$");

    if(protocol_c==true){
        URL *urlresult=url_constructor_spec(spec);  
        return urlresult;
    }

    URL *urlspec=url_constructor_spec_for_context(spec, url);
    return urlspec;

}

static inline char* getFile(const URL * url)
{
    size_t size = 0;

    if(url->path > 1)       size += url->path;
    if(url->query > 1)      size += url->query;

    char *result;
    char *path;
    
    result = palloc(size);
    
    if(url->path > 1){
        path = url->data + url->protocol + url->userinfo + url->host;
        result = psprintf("/%s", path);
    }
    if(url->query > 1){
        path = url->data + url->protocol + url->userinfo + url->host + url->path;
        result = psprintf("%s?%s", result, path);
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


PG_FUNCTION_INFO_V1(construct_url_with_context);
Datum construct_url_with_context(PG_FUNCTION_ARGS){

    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    char *spec = NULL;
    spec = PG_GETARG_CSTRING(1);
    
    PG_RETURN_POINTER( url_spec_context(url, spec) );
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


PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    char *result;
    char *host = url->data + url->protocol + url->userinfo;
    
    if(url->port > 0) {
        result = palloc(url->host + num_digits(url->port) + 2); // 2 for : and \0
        result = psprintf("%s:%d", host, url->port);
    } else {
        result = palloc(url->host + 1);
        result = psprintf("%s", host);
    }
    PG_RETURN_CSTRING( result );
}

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    PG_RETURN_CSTRING( getFile(url) );
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    if(url->path > 1){
        char * path = url->data + url->protocol + url->userinfo + url->host;
        PG_RETURN_CSTRING(psprintf("/%s", path));
    }
    PG_RETURN_CSTRING( "" );
}

PG_FUNCTION_INFO_V1(same_host);
Datum same_host(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr1 = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url1 = (URL *)(&(input_arr1->vl_dat));
    url1 = (URL *) pg_detoast_datum(input_arr1);

    VAR_ARR* input_arr2 = (VAR_ARR*) PG_GETARG_VARLENA_P(1);
    URL *url2 = (URL *)(&(input_arr2->vl_dat));
    url2 = (URL *) pg_detoast_datum(input_arr2);

    if(url1->host != url2->host)
        PG_RETURN_BOOL( false );

    char *host1 = url1->data + url1->protocol + url1->userinfo;
    char *host2 = url2->data + url2->protocol + url2->userinfo;

    PG_RETURN_BOOL( compairChars(host1, host2, url1->host) );
}

PG_FUNCTION_INFO_V1(get_default_port);
Datum get_default_port(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);
    int defaultPort = extract_port_from_protocol(url->data);
    PG_RETURN_INT32( defaultPort == 0 ? -1 : defaultPort );
}

PG_FUNCTION_INFO_V1(get_host);
Datum get_host(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);
    
    PG_RETURN_CSTRING( psprintf("%s", url->data + url->protocol + url->userinfo));
}

PG_FUNCTION_INFO_V1(get_port);
Datum get_port(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    PG_RETURN_INT32( url->port > 0 ? url->port : -1 );
}

PG_FUNCTION_INFO_V1(get_query);
Datum get_query(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    if(url->query > 1){
        char *query = url->data + url->protocol + url->userinfo + url->host + url->path;
        PG_RETURN_CSTRING( psprintf("%s", query) );
    }
    PG_RETURN_CSTRING( "" );
}

PG_FUNCTION_INFO_V1(get_ref);
Datum get_ref(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    if(url->fragment > 1){
        char *ref = url->data + url->protocol + url->userinfo + url->host + url->path + url->query;
        PG_RETURN_CSTRING( psprintf("%s", ref) );
    }
    PG_RETURN_CSTRING( "" );
}

PG_FUNCTION_INFO_V1(same_file);
Datum same_file(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr1 = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url1 = (URL *)(&(input_arr1->vl_dat));
    url1 = (URL *) pg_detoast_datum(input_arr1);

    VAR_ARR* input_arr2 = (VAR_ARR*) PG_GETARG_VARLENA_P(1);
    URL *url2 = (URL *)(&(input_arr2->vl_dat));
    url2 = (URL *) pg_detoast_datum(input_arr2);

    char *file1 = getFile(url1);
    char *file2 = getFile(url2);

    int len1 = strlen(file1);
    int len2 = strlen(file2);
    if(len1 != len2)
        PG_RETURN_BOOL( false );

    PG_RETURN_BOOL( compairChars(file1, file2, len1) );
}

PG_FUNCTION_INFO_V1(get_user_info);
Datum get_user_info(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    if(url->userinfo > 1){
        char *userinfo = url->data + url->protocol;
        PG_RETURN_CSTRING( psprintf("%s", userinfo) );
    }
    PG_RETURN_CSTRING( "" );
}

bool equals(URL *l, URL *r) {
    if(!primitive_compare(l, r))
        return false;

    char *s_left = url_to_str( l );
    char *s_right = url_to_str( r );
    
    return strcmp(s_left, s_right) == 0;
}

PG_FUNCTION_INFO_V1(url_equals);
Datum url_equals(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));
    PG_RETURN_BOOL( equals(u_left, u_right));
}

PG_FUNCTION_INFO_V1(url_not_equals);
Datum url_not_equals(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));
    PG_RETURN_BOOL( !equals(u_left, u_right));
}

PG_FUNCTION_INFO_V1(url_compare);
Datum url_compare(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));

    char *s_left = url_to_str( u_left );
    char *s_right = url_to_str( u_right );
    
    PG_RETURN_INT32( strcmp(s_left, s_right) );
}

PG_FUNCTION_INFO_V1(url_less_than);
Datum url_less_than(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));

    char *s_left = url_to_str( u_left );
    char *s_right = url_to_str( u_right );
    
    PG_RETURN_BOOL( strcmp(s_left, s_right) < 0);
}

PG_FUNCTION_INFO_V1(url_greater_than);
Datum url_greater_than(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));

    char *s_left = url_to_str( u_left );
    char *s_right = url_to_str( u_right );

    PG_RETURN_BOOL( strcmp(s_left, s_right) > 0);
}

PG_FUNCTION_INFO_V1(url_less_than_equal);
Datum url_less_than_equal(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));

    char *s_left = url_to_str( u_left );
    char *s_right = url_to_str( u_right );
    
    PG_RETURN_BOOL( strcmp(s_left, s_right) <= 0);
}

PG_FUNCTION_INFO_V1(url_greater_than_equal);
Datum url_greater_than_equal(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));

    char *s_left = url_to_str( u_left );
    char *s_right = url_to_str( u_right );

    PG_RETURN_BOOL( strcmp(s_left, s_right) >= 0);
}
/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "utils/builtins.h"  /* for text_to_cstring */

void validateProtocol(char *protocol){
    if (strlen(protocol) == 5 && compairChars("HTTPS", protocol, 5)) return;
    if (strlen(protocol) == 4 && (compairChars("HTTP", protocol, 4) || compairChars("FILE", protocol, 4))) return;
    if (strlen(protocol) == 3 && compairChars("FTP", protocol, 3)) return;
    ereport(ERROR,(
        errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("Unsupported/invalid `protocol`: \"%s\"", protocol)
    ));
}

void validateHost(char *host){
    if(check_regex_acceptable(host, "(:)|(\\/{1,})|([+?=_:,;'\\^\"!~`])")) {
        ereport(ERROR,(
            errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("URL `host` must not contain protocol/port/path or invalid special characters: \"%s\"", host)
        ));
    };
}

void validatePort(unsigned port){
    if(port < 1 || port > 65535) {
        ereport(ERROR,(
            errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("invalid `port`: \"%d\"", port)
        ));
    };
}

void validatePath(char *path){
    if(check_regex_acceptable(path, "(:)|(/{2,})|([@'\\^\"!`\\\\*])")) {
        ereport(ERROR,(
            errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("URL `path` must not contain protocol/domain/port or invalid special characters: \"%s\"", path)
        ));
    };
}

void sendErrorInput(char *msg){

   ereport(ERROR,(
            errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("INPUT must not be NULL, %s.", msg)
        ));
    
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

    // elog(INFO, "P: %s (%d), U: %s (%d), H: %s (%d), P: %s (%d), Q: %s (%d), F: %s (%d)", protocol, sizes[0], userinfo, sizes[1], host, sizes[2],  path, sizes[3], query, sizes[4], fragment, sizes[5]);

    size_t size = VARHDRSZ + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + sizes[5] + (SEGMENTS * 5) + SEGMENTS;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    int off[6] = {0,0,0,0,0,0};
    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    off[0] = offset;
    offset = copyString(u, &u->userinfo, userinfo, sizes[1], offset);
    off[1] = offset;
    offset = copyString(u, &u->host, host, sizes[2], offset);
    off[2] = offset;
    offset = copyString(u, &u->path, path, sizes[3], offset);
    off[3] = offset;
    offset = copyString(u, &u->query, query, sizes[4], offset);
    off[4] = offset;
    offset = copyString(u, &u->fragment, fragment, sizes[5], offset);
    off[5] = offset;
    // elog(INFO, "P: 0, U: %d, H: %d, P: %d, Q: %d, F: %d, after: %d", off[0], off[1], off[2], off[3], off[4], off[5]);

    u->port = port;
    // elog(INFO,"P:%d H:%d, Path:%d, Q:%d, F:%d OFF:%d", u->protocol, u->host, u->path, u->query, u->fragment, offset);
    return u;
}

URL * url_constructor_spec_regex(char* spec, char* regexnormal, char* regexfile){

    char *protocol = "";
    char *userinfo = "";
    char *host = "";
    char *path = "";
    char *query = "";
    char *fragment = "";
    unsigned port_final =0;

      
    if(strchr(spec, ' ') != NULL)
    {
        ereport(
                ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Not whitespaces allowed).")));
    }

    bool file_c = check_regex_part(false,spec, "^file");
    char *p= stripString(spec);
    
    if(file_c==true){
    bool general_file = check_regex_part(true,spec, regexfile);
    protocol="file";

    bool host_file = check_regex_part(false,spec, "^(file:\\/\\/([^\\/\\?\\#\\:]+)(\\/[^\\/\\?\\#]*)*(\\/)?)$");
    bool path_file = check_regex_part(false,spec, "^((file:[\\/]+)([^\\/\\?\\#\\:]+)?(\\/[^\\/\\?\\#]*)+(\\/)?)$");

    char *rest_file;
    char *rest_too;
    char *temp_clean_data;

    if(host_file==true){


        p = strtokm(p, "://");
        if(p){
        p=strtokm(NULL, "://"); 
        }
        if(p){

        temp_clean_data=psprintf("%s", p);
        temp_clean_data=strtok(temp_clean_data, ":/?#");
        host=psprintf("%s", temp_clean_data);
        p=strtokm(p, "/"); 
        p=strtokm(NULL, "/"); 

        }
        if(p){
        char *path_add=psprintf("%s",p);
        rest_too=strtokm(NULL, "/"); 
        while (rest_too != NULL)
        {
            path_add=psprintf("%s/%s",path_add,rest_too);
            rest_too = strtokm(NULL, "/");
        }
        path=psprintf("%s", path_add);    
        }

        }else{

        host="localhost";
        p = strtokm(p, "file:///");
        p = strtokm(NULL, "file:///");

        if(p){
        char *path_add=psprintf("%s",p);
        rest_too=strtokm(NULL, "/"); 
        while (rest_too != NULL)
        {
            path_add=psprintf("%s/%s",path_add,rest_too);
            rest_too = strtokm(NULL, "/");
        }
        path=psprintf("%s", path_add);   
        }

        }

    }else{
    bool general = check_regex_part(true,spec, regexnormal);

    bool protocol_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool host_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool userinfo_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool port_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool path_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)+(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool query_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)(\\#[^\\/\\?\\#]+)?)$");
    bool fragment_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+))$");
   

    char *port = "";
    

    // Protocol
    char *last_split_character;
    char *prev_split_character;
    char *rest;
    bool first_part=false;
    char *first_part_s="";
    char *first_split_character;
    char *temp_clean_data;
    char *rest_too="";

    
    p = strtokm(p, "://");
    last_split_character="://";
    prev_split_character="";



    if(p){
        
        rest=strtokm(NULL, "://"); 
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
        if(strcmp(last_split_character,"://")==0){

            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
                        temp_clean_data=strtok(temp_clean_data, "@:/?#");
                        if(userinfo_c==true){
                            userinfo=psprintf("%s", temp_clean_data);
                        }else{
                                host=psprintf("%s", temp_clean_data);
                            
                    }
                           

            }
               
            
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }
        

    }
    if(p){
        p = strtokm(p, "@");
        last_split_character="@";
    }
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
        if(strcmp(last_split_character,"://")==0){
            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
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
        if(strcmp(last_split_character,"://")==0){
            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
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
            char *path_add=psprintf("%s",rest);
            rest_too=strtokm(NULL, "/"); 
            while (rest_too != NULL)
            {
                path_add=psprintf("%s/%s",path_add,rest_too);
                rest_too = strtokm(NULL, "/");

            }
            p=psprintf("%s",path_add);
            prev_split_character=last_split_character;

        }
        if(strcmp(last_split_character,"://")==0){
            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
            
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
        if(strcmp(last_split_character,"://")==0){
            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
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
        if(strcmp(last_split_character,"://")==0){
            if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
        }else if(strcmp(last_split_character,"@")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, ":/?#");
            host=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,":")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "/?#");
            port=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"/")==0){
            temp_clean_data=psprintf("%s", p);
            bool start_something = check_regex_part(false,temp_clean_data, "^[\\/\\?\\#\\:]");
            if(start_something==false){
                temp_clean_data=strtok(temp_clean_data, "?#");
                path=psprintf("%s", temp_clean_data);
            }
        }else if(strcmp(last_split_character,"?")==0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "#");
            query=psprintf("%s", temp_clean_data);
        }else if(strcmp(last_split_character,"#")==0){
            fragment=psprintf("%s", p);
        }

    }
    if(p){

        if(strcmp(last_split_character,"://")==0){
           
           
          if(p&&strcmp(p,"")!=0){
            temp_clean_data=psprintf("%s", p);
            temp_clean_data=strtok(temp_clean_data, "@:/?#");
            if(userinfo_c==true){
                userinfo=psprintf("%s", temp_clean_data);
            }else{
                    host=psprintf("%s", temp_clean_data);
                
        }
                           

            }
            
        }
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
    

        if(strcmp(first_split_character,"://")==0){
            protocol=psprintf("%s",first_part_s );
        }else if(strcmp(first_split_character,"@")==0){
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
   
    }
    elog(INFO, "protocol:%s",protocol );
    elog(INFO, "host:%s",host );
     URL *url = build_url_with_all_parts(protocol, userinfo, host, port_final, path, query, fragment);

    return url;
}   

URL * url_constructor_spec(char* spec){

    char* regexnormal="^(([a-zA-Z]+:[\\/]+)([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$";
    char* regexfile="^((file:[\\/]+)?([^\\/\\?\\#\\:]+)?(\\/[^\\/\\?\\#]*)*(\\/)?)$";
    URL *urlspec=url_constructor_spec_regex(spec,regexnormal , regexfile);
    return urlspec;
}

static URL* build_url_with_port(char *protocol, char *host, unsigned port, char *path){

    validateProtocol(protocol);
    char * spec = psprintf("%s://", protocol);

    if(host != NULL){
        validateHost(host);
        spec = psprintf("%s%s", spec, host);
        if(port != NULL){
            validatePort(port);
            spec = psprintf("%s:%d", spec, port);
        }
    }
    validatePath(path);
    if(path[0] == '/') 
        memmove(path, path+1, strlen(path));
    if(host != NULL)
        spec = psprintf("%s/%s", spec, path);
    else 
        spec = psprintf("%s%s", spec, path);
    return url_constructor_spec(spec);
}

static URL* build_url_without_user_info(char *protocol, char *authority, char *path, char *query, char *fragment){
	
    // Sizes = (0:Protocol, 1:Host, 2:Path, 3:Query, 4:Fragment)
    int sizes[5];
    sizes[0] = strlen(protocol);
    sizes[1] = strlen(authority);
    sizes[2] = strlen(path);
    sizes[3] = strlen(query);
    sizes[4] = strlen(fragment);

    elog(INFO, "P: %s (%d), A: %s (%d), P: %s (%d), Q: %s (%d), F: %s (%d)", protocol, sizes[0], authority, sizes[1],  path, sizes[2], query, sizes[3], fragment, sizes[4]);

    size_t size = VARHDRSZ + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + (SEGMENTS * 5) + SEGMENTS;
    URL *u = (URL *) palloc(size);
    SET_VARSIZE(u, size);

    int offset = 0;

    int off[5] = {0,0,0,0,0};
    offset = copyString(u, &u->protocol, protocol, sizes[0], offset);
    off[0] = offset;
    offset = copyString(u, &u->host, authority, sizes[1], offset);
    off[1] = offset;
    offset = copyString(u, &u->path, path, sizes[2], offset);
    off[2] = offset;
    offset = copyString(u, &u->query, query, sizes[3], offset);
    off[3] = offset;
    offset = copyString(u, &u->fragment, fragment, sizes[4], offset);
    off[4] = offset;
    // elog(INFO, "P: 0, H: %d, P: %d, Q: %d, F: %d, after: %d", off[0], off[1], off[2], off[3], off[4]);
    // elog(INFO,"P:%d H:%d, Path:%d, Q:%d, F:%d OFF:%d", u->protocol, u->host, u->path, u->query, u->fragment, offset);
    return u;
}

URL * url_constructor_spec_for_context(char* spec, URL * url){

    char* regexnormal= "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$";
    char* regexfile="^((file:[\\/]+)?([^\\/\\?\\#\\:]+)?(\\/[^\\/\\?\\#]*)*(\\/)?)$";

    URL *urlspec=url_constructor_spec_regex(spec,regexnormal , regexfile);
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

    protocol_ind = urlspec->data;
    userinfo_ind = protocol_ind + urlspec->protocol;
    host_ind = userinfo_ind + urlspec->userinfo;
    path_ind = host_ind + urlspec->host;
    query_ind = path_ind + urlspec->path;
    fragment_ind = query_ind + urlspec->query;

    char *protocol_spec = psprintf("%s",protocol_ind);
    char *userinfo_spec =psprintf("%s",userinfo_ind);
    char *host_spec = psprintf("%s",host_ind);
    char *path_spec = psprintf("%s",path_ind);
    unsigned port_final_spec =urlspec->port;
    char *query_spec = psprintf("%s",query_ind);
    char *fragment_spec = psprintf("%s",fragment_ind);



    if(strcmp(protocol_spec,"")!=0)
        protocol = psprintf("%s", protocol_ind);

    if(strcmp(userinfo_spec,"")!=0)
        userinfo = psprintf("%s", userinfo_ind);

    if(strcmp(host_spec,"")!=0)
        host = psprintf("%s", host_ind);

    if(port_final_spec!=0)
        port_final = atoi(psprintf("%d", urlspec->port));

    if(strcmp(path_spec,"")!=0)
        path = psprintf("%s", path_ind);

    if(strcmp(query_spec,"")!=0)
        query = psprintf("%s", query_ind);

    if(strcmp(fragment_spec,"")!=0)
        fragment = psprintf("%s", fragment_ind);


    URL * url_parts = build_url_with_all_parts(protocol, userinfo, host, port_final, path, query, fragment);
    return url_parts;
} 

static inline char* url_spec_context(const URL * url, const char * spec){

    bool check_general = check_regex_part(true,spec, "^(([a-zA-Z]+:[\\/]+)?([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");
    bool protocol_c = check_regex_part(false,spec, "^(([a-zA-Z]+:[\\/]+)([^\\/\\?\\#\\:]+@)?([^\\/\\?\\#\\:]+)?(:[0-9]{1,5})?(\\/[^\\/\\?\\#\\:]*)*(\\/)?(\\?[^\\/\\?\\#\\:]+)?(\\#[^\\/\\?\\#]+)?)$");


    if(strcmp(spec,"")==0){
        return url;
    }

    if(protocol_c==true){
        URL *urlresult=url_constructor_spec(spec);  
        return urlresult;
    }

    URL *urlspec=url_constructor_spec_for_context(spec, url);
    return urlspec;

}

static inline char* url_to_str(const URL * url)
{
    size_t size = url->protocol + url->host + 1; // Extra ://
    // Construct char * from the len fields in url
    char *protocol = url->data;
    char *userinfo = protocol + url->protocol;
    char *host = userinfo + url->userinfo;
    char *path = host + url->host;
    char *query = path + url->path;
    char *fragment = query + url->query;
    unsigned port_len = num_digits(url->port);

    char *result;

    // Since all url segment contains size + 1, that's why we are not adding an extra + 1 in size
    // for example path contains an extra size for '\0'
    if(url->userinfo > DEFAULT_URL_SEGMENT_LEN)   size += url->userinfo;
    if(url->port > DEFAULT_URL_SEGMENT_LEN)       size += port_len;
    if(url->path > DEFAULT_URL_SEGMENT_LEN)       size += url->path;
    if(url->query > DEFAULT_URL_SEGMENT_LEN)      size += url->query;
    if(url->fragment > DEFAULT_URL_SEGMENT_LEN)   size += url->fragment;

    // The 5 extra char represents the :// after protocol and : and /
    result = malloc(size);
    memset(result, 0, size);

    if(url->userinfo > DEFAULT_URL_SEGMENT_LEN)
        result = psprintf("%s://%s@%s", protocol, userinfo, host);
    else
        result = psprintf("%s://%s", protocol, host);

    if(url->port > DEFAULT_URL_SEGMENT_LEN)
        result = psprintf("%s:%d", result, url->port);

    if(url->path > DEFAULT_URL_SEGMENT_LEN)
        result = psprintf("%s/%s", result, path);

    if(url->query > DEFAULT_URL_SEGMENT_LEN)
        result = psprintf("%s?%s", result, query);

    if(url->fragment > DEFAULT_URL_SEGMENT_LEN)
        result = psprintf("%s#%s", result, fragment);

    return result;
}


static inline char* getFile(const URL * url)
{
    int size = 0;

    if(url->path > DEFAULT_URL_SEGMENT_LEN)       size += url->path-1;
    if(url->query > DEFAULT_URL_SEGMENT_LEN)      size += url->query-1;

    char *result;
    char *path;
    
    result = malloc(size + 1);
    memset(result, 0, size + 1);
    
    if(url->path > DEFAULT_URL_SEGMENT_LEN){
        path = url->data + url->protocol + url->userinfo + url->host;
        result = psprintf("/%s", path);
    }
    if(url->query > DEFAULT_URL_SEGMENT_LEN){
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
    if(spec==NULL){
        sendErrorInput("spec");
    }

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

PG_FUNCTION_INFO_V1(url_to_string);
Datum url_to_string(PG_FUNCTION_ARGS)
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

    if(protocol==NULL||port==NULL||path==NULL)
        sendErrorInput("protocol or port or path");
    
    PG_RETURN_POINTER( build_url_with_port(protocol, host, port, path) );
}

PG_FUNCTION_INFO_V1(construct_url_without_port);
Datum construct_url_without_port(PG_FUNCTION_ARGS){

    char *protocol = NULL, *host = NULL, *path = NULL;
    protocol = PG_GETARG_CSTRING(0);
    host = PG_GETARG_CSTRING(1);
    path = PG_GETARG_CSTRING(2);

    if(protocol==NULL||path==NULL)
        sendErrorInput("protocol or path");
    
    PG_RETURN_POINTER( build_url_with_port(protocol, host, NULL, path) );
}

PG_FUNCTION_INFO_V1(construct_url_with_context);
Datum construct_url_with_context(PG_FUNCTION_ARGS){

    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    char *spec = NULL;
    spec = PG_GETARG_CSTRING(1);

    if(url==NULL||spec==NULL){
        sendErrorInput("url or spec");
    }
    
    PG_RETURN_POINTER( url_spec_context(url, spec) );
}


char* getProtocol(URL *url)
{
    return psprintf("%s", url->data);
}

PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    PG_RETURN_CSTRING( getProtocol(url) );
}


/**
 * Used for Casting Text -> URL
 */
PG_FUNCTION_INFO_V1(text_to_url);
Datum text_to_url(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    // char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
    char *str = text_to_cstring(txt);
    URL * r = url_constructor_spec( str );

    if(txt==NULL){
        sendErrorInput("text");
    }
    
    PG_RETURN_POINTER( r );
}

/**
 * Used for Casting URL -> Text
 */
PG_FUNCTION_INFO_V1(url_to_text);
Datum url_to_text(PG_FUNCTION_ARGS)
{
    URL *url = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));

   
    
    // URL *url = (URL *) PG_GETARG_POINTER(0);
    // char * url_str = url_to_str(url);
    text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum(url) );
    // pfree(url);
    // pfree(ss);
    // pfree(out);
    // PG_RETURN_CSTRING(url->protocol);
    PG_RETURN_TEXT_P(out);
}

char* getAuthority(URL *url)
{
    char *result;
    char *host = url->data + url->protocol + url->userinfo;
    
    if(url->port > 0) {
        result = palloc(url->host + num_digits(url->port) + 2); // 2 for : and \0
        result = psprintf("%s:%d", host, url->port);
    } else {
        result = palloc(url->host + 1);
        result = psprintf("%s", host);
    }
    return result;
}

PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    
    PG_RETURN_CSTRING( getAuthority(url) );
}

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

   

    PG_RETURN_CSTRING( getFile(url) );
}

char* getPath(URL *url)
{
    if(url->path > 1){
        char * path = url->data + url->protocol + url->userinfo + url->host;
        return psprintf("/%s", path);
    }
    return "";
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

   

    PG_RETURN_CSTRING( getPath(url) );
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

char* getQuery(URL *url)
{
    if(url->query > 1){
        char *query = url->data + url->protocol + url->userinfo + url->host + url->path;
        return psprintf("%s", query);
    }
    return "";
}

PG_FUNCTION_INFO_V1(get_query);
Datum get_query(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);


    PG_RETURN_CSTRING( getQuery(url) );
}

char* getRef(URL *url)
{
    if(url->fragment > 1){
        char *ref = url->data + url->protocol + url->userinfo + url->host + url->path + url->query;
        return psprintf("%s", ref);
    }
    return "";
}

PG_FUNCTION_INFO_V1(get_ref);
Datum get_ref(PG_FUNCTION_ARGS)
{
    VAR_ARR* input_arr = (VAR_ARR*) PG_GETARG_VARLENA_P(0);
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);

    PG_RETURN_CSTRING( getRef(url) );
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

bool equalURLs(URL *l, URL *r) {
    if(!primitive_compare(l, r))
        return false;

    char *s_left = url_to_str( build_url_without_user_info(getProtocol(l), getAuthority(l), getPath(l), getQuery(l), getRef(l)) );
    char *s_right = url_to_str( build_url_without_user_info(getProtocol(r), getAuthority(r), getPath(r), getQuery(r), getRef(r)) );
    
    return compairChars(s_left, s_right, strlen(s_left));
}

PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));
    PG_RETURN_BOOL( equalURLs(u_left, u_right));
}

PG_FUNCTION_INFO_V1(url_equals);
Datum url_equals(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));
    PG_RETURN_BOOL( equalURLs(u_left, u_right));
}

PG_FUNCTION_INFO_V1(url_not_equals);
Datum url_not_equals(PG_FUNCTION_ARGS)
{
    URL *u_left = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(0));
    URL *u_right = get_input_url((VAR_ARR*) PG_GETARG_VARLENA_P(1));
    PG_RETURN_BOOL( !equalURLs(u_left, u_right));
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
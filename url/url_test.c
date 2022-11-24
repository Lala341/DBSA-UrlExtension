#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>



const unsigned int HTTPS = 443;
const unsigned int HTTP = 8080;

typedef struct
{
    char protocol[5];
    char host[50];
    unsigned int port;
    char path[255];
    char query[255];
    char fragment[255];
} url;

void print_url_struct(url *s){

    printf("The value of s->protocol is: %s\n", s->protocol);
    printf("The value of s->host is: %s\n", s->host);
    printf("The value of s->port is: %d\n", s->port);
    printf("The value of s->path is: %s\n", s->path);
    printf("The value of s->query is: %s\n", s->query);
    printf("The value of s->fragment is: %s\n", s->fragment);

}
int get_num_matches_verify(char *string_compare, char *string_regex){
    regex_t regex;
    /*Create regex*/
    if ( regcomp(&regex, string_regex, REG_EXTENDED) != 0 )
    {
        printf( "Creation regex failed");
        return 0;
        
    }
    /* Compare and print subexpressions */
    fprintf( stdout, "Subexpressions number: %zu\n", regex.re_nsub );
    size_t match_count = regex.re_nsub + 1;
    regmatch_t pointer_match[match_count];

    int ret;
     if ( ( ret = regexec( &regex, string_compare, match_count, pointer_match, 0)) != 0 )
    {
        printf("Does not match\n" );
        return 0;
    }
    else
    {
        fprintf( stdout, "Sucessful match\n" );
        return match_count ;
      
    }
}
url * get_pointer_url(char *string_compare, char *string_regex){
    regex_t regex;
    /*Create regex*/
    url *s = (url*) malloc( sizeof(url) );
            
    if ( regcomp(&regex, string_regex, REG_EXTENDED) != 0 )
    {
        printf( "Creation regex failed");
        
    }
    /* Compare and print subexpressions */
    fprintf( stdout, "Subexpressions number: %zu\n", regex.re_nsub );
    size_t match_count = regex.re_nsub + 1;
    regmatch_t pointer_match[match_count];

    int ret;
     if ( ( ret = regexec( &regex, string_compare, match_count, pointer_match, 0)) != 0 )
    {
        printf("Does not match\n" );
        
    }
    else
    {
        fprintf( stdout, "Sucessful match\n" );

        for ( size_t i = 0; i < match_count; i++ )
        {
            if ( pointer_match[i].rm_so >= 0 )
            {
                fprintf( stdout, "Match %zu (start: %3lu; end: %3lu): %*.*s\n", i,
                    (unsigned long) pointer_match[i].rm_so,
                    (unsigned long) pointer_match[i].rm_eo,
                    (int) ( pointer_match[i].rm_eo - pointer_match[i].rm_so ),
                    (int) ( pointer_match[i].rm_eo - pointer_match[i].rm_so ),
                    string_compare + pointer_match[i].rm_so );


                   
            }

        }
            int star_p=(int) pointer_match[1].rm_so;
            int term_p=(int) pointer_match[1].rm_eo;
            strncpy(s->protocol, string_compare+star_p, term_p);

            

            print_url_struct(s);

        
    }
    return s;
}

int main(  ) 
{

    const char *string_compare="https://example.com:3000/pathname/data/?search=test#hash";
    const char *string_regex ="^(.*):\/\/([A-Za-z0-9\\-\\.]+):?([0-9]{2,4})?((\/[A-Za-z0-9]+)*)*\/?(\\?[A-Za-z0-9]+\=[A-Za-z0-9]+(\&[A-Za-z0-9]+\=[A-Za-z0-9]+)*)*(\#[A-Za-z0-9]+)*";
    //(^http[s])?:\/\/(www\.)?(.*)??\/?((.)*)

    int num_matches= get_num_matches_verify(string_compare, string_regex);
    
    if(num_matches>0){
        url *s= get_pointer_url(string_compare, string_regex);
    }

    return 0;
     // PG_RETURN_POINTER( get_pointer_url( str ) );

}

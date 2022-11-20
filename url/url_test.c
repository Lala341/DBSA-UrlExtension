#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>



const unsigned int HTTPS = 443;
const unsigned int HTTP = 8080;

typedef struct
{
    char protocol[5];
    char authority[50];
    unsigned int port;
    char path[255];
    char query[255];
    char fragment[50];
} url;


int get_substring_matches(char *string_compare, char *string_regex){
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
    }
    return 0;
}
int main(  ) 
{

    const char *string_compare="http://example.com:3000/pathname/?search=test#hash";
    const char *string_regex ="(http[s]?:\/\/)?([^\/\s]+\/)(.*)";
    get_substring_matches(string_compare, string_regex);
    return 0;
}

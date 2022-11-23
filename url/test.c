#include "utils.h"
#include <regex.h>

// typedef struct
// {
//     char protocol[5];
//     char authority[50];
//     unsigned int port;
//     char path[255];
//     char query[255];
//     char fragment[50];
// } url;

typedef struct
{
    char *protocol;
    char *authority;
    unsigned int port;
    char *path;
    char *query;
    char *fragment;
} URL;


URL url = {
    .protocol = "http", 
    .authority = "",
    .port = -1,
    .query = "",
    .fragment = ""
};

/**
 * Print utility function
 */
void print(URL url) {
    if(url.protocol) 
        fprintf(stdout, "Protocol: %s\n", url.protocol);
    if(url.authority) 
        fprintf(stdout, "Authority: %s\n", url.authority);
    fprintf(stdout, "Port: %d\n", url.port);
    if(url.path) 
        fprintf(stdout, "Path: %s\n", url.path);
    if(url.query) 
        fprintf(stdout, "Query: %s\n", url.query);
    if(url.fragment) 
        fprintf(stdout, "Fragment: %s\n", url.fragment);
}

int main() {

    const char * str = "https://www.google.com:8080/search?q=32&34#subfragment";

    regex_t rx;
    int rc;
    regmatch_t pmatch[2];
    char msg[100];

    const char *pattern = "^(.*:)//([A-Za-z0-9\\-\\.]+)(:[0-9]+)?(.*)$";
    // const char *pattern = "(http|ftp|https):\\/\\/([\\w_-]+(?:(?:\\.[\\w_-]+)+))([\\w.,@?^=%%&:\\/~+#-]*[\\w@?^=%%&\\/~+#-])";

    if (0 != (rc = regcomp(&rx, pattern, REG_EXTENDED))) {

        regerror(rc, &rx, msg, 100);
        printf("regcomp() failed, returning nonzero (%d), msg = %s\n", rc, msg);
        return -1;
    }

    size_t match_count = rx.re_nsub + 1;

    if (0 != (rc = regexec(&rx, str, match_count, pmatch, 0))) {
        printf("Failed to match '%s' with pattern ,returning %d.\n", str, rc);
    }

    int len = 0;
    if( pmatch[1].rm_so >= 0) {
        len = pmatch[1].rm_eo - pmatch[1].rm_so;
        url.protocol = calloc( len + 1, sizeof(char));
        // Copy the url from regex find_start till find_end
        strncpy(url.protocol, str + pmatch[1].rm_so, pmatch[1].rm_eo);
        // Remove the colon if any
        removeChar(url.protocol, ':');
    }

    if( pmatch[2].rm_so >= 0) {
        len = pmatch[2].rm_eo - pmatch[2].rm_so;
        url.authority = calloc( len + 1, sizeof(char));
        strncpy(url.authority, str + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
    }

    if( pmatch[3].rm_so >= 0) {
        len = pmatch[3].rm_eo - pmatch[3].rm_so;
        char *protocol_str = calloc( len + 1, sizeof(char));
        strncpy(protocol_str, str + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
        removeChar(protocol_str, ':');
        // Cast to int
        url.port = atoi(protocol_str);
    }
    
    print(url);

    regfree(&rx);
}

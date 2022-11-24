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
    char *host;
    unsigned int port;
    char *path;
    char *query;
    char *fragment;
} URL;


URL url = {
    .protocol = "http", 
    .host = "",
    .port = 0,
    .query = "",
    .fragment = ""
};

/**
 * Print utility function
 */
void print(URL url) {
    if(url.protocol) 
        fprintf(stdout, "Protocol: %s\n", url.protocol);
    if(url.host) 
        fprintf(stdout, "Host: %s\n", url.host);
    fprintf(stdout, "Port: %d\n", url.port);
    if(url.port > 0) {
        fprintf(stdout, "Authority: %s:%d\n", url.host, url.port);
    } else {
        fprintf(stdout, "Authority: %s\n", url.host);
    }
    if(url.path) 
        fprintf(stdout, "Path: %s\n", url.path);
    if(url.query) 
        fprintf(stdout, "Query: %s\n", url.query);
    if(url.fragment) 
        fprintf(stdout, "Fragment: %s\n", url.fragment);
}

const char* url_to_str(URL * url)
{
    
    char *authority = calloc(50, sizeof(char));
    char *result = calloc(100, sizeof(char));
    
    if(url->protocol) 
        fprintf(stdout, "Protocol: %s\n", url->protocol);
    if(url->host) 
        fprintf(stdout, "Host: %s\n", url->host);
    fprintf(stdout, "Port: %d\n", url->port);
    if(url->port > 0) {
        sprintf(authority, "%s:%d", url->host,url->port);
        fprintf(stdout, "Authority: %s:%d\n", url->host, url->port);
    } else {
        sprintf(authority, "%s", url->host);
        fprintf(stdout, "Authority: %s\n", url->host);
    }
    sprintf(result, "%s://%s", url->protocol, authority);
    // strcat(result, url->protocol);
        // strcat(result, "://");
    // strcat(result, authority);
    fprintf(stdout, "result with protocol: %s\n", result);
    if(url->path) {
        strcat(result, url->path);
        fprintf(stdout, "Path: %s\n", url->path);
    }
    fprintf(stdout, "result with path: %s\n", result);
    if(url->query != "") {
        char * temp = result;
        sprintf(result, "%s?%s", temp, url->query);
        fprintf(stdout, "Query: %s\n", url->query);
    }
    fprintf(stdout, "result with query: %s\n", result);
    if(url->fragment != ""){ 
        char * temp = result;
        sprintf(result, "#%s", url->fragment);
        fprintf(stdout, "Fragment: %s\n", url->fragment);
    }
    fprintf(stdout, "result with fragment: %s\n", result);
    // char * result = psprintf("%s", result);
    return result;
}

char * extractStr(regmatch_t pmatch, const char *str) {
    int len = pmatch.rm_eo - pmatch.rm_so;
    char *dest = calloc( len + 1, sizeof(char));
    // Copy the url from regex find_start till find_end
    strncpy(dest, str + pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);
    return dest;
}

int main() {

    const char * str = "https://www.google.com:8080/search/thing/that?q=32&34%%20#subfragment";

    regex_t rx;
    int rc;
    regmatch_t pmatch[2];
    char msg[100];

    const char *pattern = "^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{2,4})?((\\/[A-Za-z0-9]+)*)*\\/?(\\?[A-Za-z0-9]+\\=[A-Za-z0-9]+(\\&[A-Za-z0-9]+\\=[A-Za-z0-9]+)*)*(\\#[A-Za-z0-9]+)*";
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

    url.protocol = extractStr(pmatch[1], str);
    removeChar(url.protocol, ':');
    url.host = extractStr(pmatch[2], str);
    // Protocol
    char *protocol_str = extractStr(pmatch[3], str);
    removeChar(protocol_str, ':');
    // Cast to int
    url.port = atoi(protocol_str);
    // Path
    url.path = extractStr(pmatch[4], str);
    url.query = extractStr(pmatch[6], str);


    // if( pmatch[1].rm_so >= 0) {
    //     len = pmatch[1].rm_eo - pmatch[1].rm_so;
    //     url.protocol = calloc( len + 1, sizeof(char));
    //     // Copy the url from regex find_start till find_end
    //     strncpy(url.protocol, str + pmatch[1].rm_so, pmatch[1].rm_eo);
    //     // Remove the colon if any
    //     removeChar(url.protocol, ':');
    // }

    // if( pmatch[2].rm_so >= 0) {
    //     len = pmatch[2].rm_eo - pmatch[2].rm_so;
    //     url.host = calloc( len + 1, sizeof(char));
    //     strncpy(url.host, str + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
    // }

    // if( pmatch[3].rm_so >= 0) {
    //     len = pmatch[3].rm_eo - pmatch[3].rm_so;
    //     char *protocol_str = calloc( len + 1, sizeof(char));
    //     strncpy(protocol_str, str + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
    //     removeChar(protocol_str, ':');
    //     // Cast to int
    //     url.port = atoi(protocol_str);
    // }

    // if( pmatch[4].rm_so >= 0) {
    //     len = pmatch[4].rm_eo - pmatch[4].rm_so;
    //     url.path = calloc( len + 1, sizeof(char));
    //     strncpy(url.path, str + pmatch[4].rm_so, pmatch[4].rm_eo - pmatch[4].rm_so);
    //     // removeChar(protocol_str, '/');
    // }

    // if( pmatch[6].rm_so >= 0) {
    //     len = pmatch[6].rm_eo - pmatch[6].rm_so;
    //     url.query = calloc( len + 1, sizeof(char));
    //     strncpy(url.query, str + pmatch[6].rm_so, pmatch[6].rm_eo - pmatch[6].rm_so);
    //     removeChar(url.query, '?');
    // }
    
    print(url);
    printf("\n");
    url_to_str(&url);

    regfree(&rx);
}

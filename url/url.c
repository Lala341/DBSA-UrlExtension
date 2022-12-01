/**
 * url PostgreSQL input/output function for bigint
 *
 */
#include "url.h"
#include "executor/executor.h"
#include "utils/builtins.h"
#include "utils/datum.h"

URL * url_constructor_spec(char* str){

    // HeapTupleHeader  t = PG_GETARG_HEAPTUPLEHEADER(0);
    // bool is_protocol_null, is_domain_null;
    // Datum protocol, domain;

    // protocol = GetAttributeByName(t, "protocol", &is_protocol_null);
    // domain = GetAttributeByName(t, "domain", &is_domain_null);

    // char *protocol = NULL, *domain = NULL, *port = NULL;
    // if (!PG_ARGISNULL(1)) protocol = TextDatumGetCString(PG_GETARG_DATUM(1));
    // if (!PG_ARGISNULL(2)) domain = TextDatumGetCString(PG_GETARG_DATUM(2));
    // if (!PG_ARGISNULL(3)) port = TextDatumGetCString(PG_GETARG_DATUM(3));


    // ereport(
    //     ERROR,
    //     (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //     errmsg("No %s, %s, %s", protocol, domain, port))
    // );

    // size_t arg_len = VARSIZE(str) + VARHDRSZ;
    // char *raw = palloc0((strlen(str) + 1) * sizeof(char));

    // size_t size = VARSIZE(str);
    // text *destination = (text *) palloc(VARHDRSZ + size);
    // destination->length = VARHDRSZ + size;
    // memcpy(destination->data, buffer, 40);

    // if (sscanf(str, "( %s )", raw) != 1)
    // {
    //     ereport(
    //         ERROR,
    //         (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //         errmsg("Invalid input syntax for type 'URL': \"%s\"", str))
    //     );
    // }
    // Remove quotes and special char
    char *spec = stripString(str);
    // pfree(raw);

    URL * url = (URL *) palloc0( sizeof(URL) );
    url->protocol = "http";
    url->host = "";
    url->port = 0;
    url->query = "";
    url->fragment = "";

    regex_t rx;
    int rc;
    regmatch_t pmatch[8];
    char msg[100];
    const char *string_regex ="^(.*):\\/\\/([A-Za-z0-9\\-\\.]+):?([0-9]{2,4})?((\\/[A-Za-z0-9]+)*)*\\/?(\\?[A-Za-z0-9]+\\=[A-Za-z0-9]+(\\&[A-Za-z0-9]+\\=[A-Za-z0-9]+)*)*(\\#[A-Za-z0-9]+)*";

    if (0 != (rc = regcomp(&rx, string_regex, REG_EXTENDED))) {

        regerror(rc, &rx, msg, 100);
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc, msg)
            )
        );
    }

    size_t match_count = rx.re_nsub + 1;

    if (0 != (rc = regexec(&rx, spec, match_count, pmatch, 0))) {
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Invalid URL pattern provided (Error Code: %d): \"%s\"", rc, spec)
            )
        );
    }

    // Protocol
    url->protocol = extractStr(pmatch[1], spec);
    url->protocol = removeChar(url->protocol, ':');
    url->host = extractStr(pmatch[2], spec);
    // Port
    char *port_str = extractStr(pmatch[3], spec);
    port_str = removeChar(port_str, ':');
    // Cast to int
    url->port = atoi(port_str);
    // Path
    url->path = extractStr(pmatch[4], spec);
    char *query_str = extractStr(pmatch[6], spec);
    query_str = removeChar(query_str, '?');
    url->query = query_str;

    // ereport(
    //         ERROR,
    //         (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //         errmsg("Reg: [1]%3lu,%3lu = [2]%3lu,%3lu = [3]%3lu,%3lu", (unsigned long) pmatch[1].rm_so, (unsigned long) pmatch[1].rm_eo, (unsigned long) pmatch[2].rm_so, (unsigned long) pmatch[2].rm_eo, (unsigned long) pmatch[3].rm_so, (unsigned long) pmatch[3].rm_eo))
    //     );
    
    // ereport(
    //         ERROR,
    //         (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    //         errmsg("size (%d) Output: %s || %s || %d || %s || %s", PG_NARGS(), spec, url->protocol, url->port, url->path, url->query))
    //     );

    // PG_RETURN_TEXT_P( &url );
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
    
    PG_RETURN_POINTER( url );
}

static inline char* url_to_str(const URL * url)
{

    // char *authority = palloc0(50 * sizeof(char));
    // char *result = palloc0(100 * sizeof(char));
    // 
    // // if(url->protocol) 
    // //     fprintf(stdout, "Protocol: %s\n", url->protocol);
    // // if(url->host) 
    // //     fprintf(stdout, "Host: %s\n", url->host);
    // // fprintf(stdout, "Port: %d\n", url->port);
    // if(url->port > 0) {
    //     psprintf(authority, "%s:%d", url->host,url->port);
    //     // fprintf(stdout, "Authority: %s:%d\n", url->host, url->port);
    // } else {
    //     psprintf(authority, "%s", url->host);
    //     // fprintf(stdout, "Authority: %s\n", url->host);
    // }
    // psprintf(result, "%s://%s", url->protocol, authority);
    // // strcat(result, url->protocol);
    //     // strcat(result, "://");
    // // strcat(result, authority);
    // // fprintf(stdout, "result with protocol: %s\n", result);
    // if(url->path) {
    //     strcat(result, url->path);
    //     // fprintf(stdout, "Path: %s\n", url->path);
    // }
    // if(url->query != "") {
    //     char * temp = result;
    //     psprintf(result, "%s?%s", temp, url->query);
    //     // fprintf(stdout, "Query: %s\n", url->query);
    // }
    // if(url->fragment != ""){ 
    //     char * temp = result;
    //     psprintf(result, "#%s", url->fragment);
    //     // fprintf(stdout, "Fragment: %s\n", url->fragment);
    // }
    // // fprintf(stdout, "result with fragment: %s\n", result);
    // // char * result = psprintf("%s", result);

    // // Datum arg = PG_GETARG_DATUM(0);

	// // PG_RETURN_CSTRING( url_to_str() );
    char * result = psprintf("%s://%s:%d/%s#%s", url->protocol, url->host,url->port, url->query, url->fragment);
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
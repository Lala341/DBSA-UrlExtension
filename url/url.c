/**
 * url PostgreSQL input/output function for bigint
 *
 */

#include <stdio.h>
#include "postgres.h"

// #include "access/gist.h"
// #include "access/skey.h"
// #include "utils/elog.h"
// #include "utils/palloc.h"
// #include "utils/builtins.h"
// #include "libpq/pqformat.h"
// #include "utils/date.h"
// #include "utils/datetime.h"
// #include "utils/guc.h"
// #include <sys/time.h>
// #include <time.h>
// #include <stdlib.h>
// #include <ctype.h>


PG_MODULE_MAGIC;

const unsigned int HTTPS = 443;
const unsigned int HTTP = 8080;
// default HTTP is 80

typedef struct
{
    char protocol[5];
    char host[50];
    unsigned int port;
    char path[255];
    char query[255];
    char fragment[50];
    // Authority is host:port
} url;

static inline url* str_to_url(const char* str)
{
    char name[24];
    int age;
    float grade;

    if (sscanf(str, "( %s , %d , %f )", name, &age, &grade) != 3)
    {
        ereport(
            ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("Invalid input syntax for type 'url': \"%s\"", str))
        );
    }

    url * s = (url *) palloc( sizeof(url) );

    strcpy(s->name, name);
    s->age = age;
    s->grade = grade;
    
    return s;
}

static inline const char* url_to_str(const url* s)
{
    char * result = psprintf("( %s , %d , %f )", s->name, s->age, s->grade);
    return result;
}

/**
 * Macro mapping Constructor of URL
 */
PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
   char       *str = PG_GETARG_CSTRING(0);
    
    PG_RETURN_TEXT_P(str);
    // PG_RETURN_POINTER( str_to_url( str ) );
}   

/**
 * Macro toString
 */
PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS)
{
    Datum arg = PG_GETARG_DATUM(0);

	PG_RETURN_CSTRING();
    // const url *s = (url *) PG_GETARG_POINTER(0);
    // PG_RETURN_CSTRING( url_to_str(s) );
}

/**
 * Macro mapping function Text -> URL
 */
PG_FUNCTION_INFO_V1(text_to_url);
Datum text_to_url(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    char *str = DatumGetCString( DirectFunctionCall1(textout, PointerGetDatum(txt) ) );
    PG_RETURN_POINTER( str_to_url( str ) );
}

/**
 * Macro mapping function URL -> Text
 */
PG_FUNCTION_INFO_V1(url_to_text);
Datum url_to_text(PG_FUNCTION_ARGS)
{
    const url *s = (url *) PG_GETARG_POINTER(0);    
    text *out = (text *) DirectFunctionCall1(textin, PointerGetDatum( url_to_str(s) ) );
    PG_RETURN_TEXT_P(out);
}

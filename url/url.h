/**
 * url PostgreSQL input/output function for bigint
 *
 */

#include <stdio.h>
#include "utils.h"
#include "fmgr.h"

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

// typedef struct
// {
//     char protocol[FLEXIBLE_ARRAY_MEMBER];
//     char host[FLEXIBLE_ARRAY_MEMBER];
//     unsigned int port;
//     char path[FLEXIBLE_ARRAY_MEMBER];
//     char query[FLEXIBLE_ARRAY_MEMBER];
//     char fragment[FLEXIBLE_ARRAY_MEMBER];
// } URL;

typedef struct
{
    char *protocol;
    char *host;
    unsigned int port;
    char *path;
    char *query;
    char *fragment;
} URL;
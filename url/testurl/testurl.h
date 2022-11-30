/**
 * url PostgreSQL input/output function for bigint
 *
 */

#include <stdio.h>
#include "utils.h"
#include "fmgr.h"

PG_MODULE_MAGIC;

const unsigned int HTTPS = 443;
const unsigned int HTTP = 8080;

typedef struct
{
    char *protocol;
    char *host;
    unsigned int port;
    char *path;
    char *query;
    char *fragment;
} URL;
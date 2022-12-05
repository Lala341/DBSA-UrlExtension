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

// typedef struct
// {
//     char protocol[FLEXIBLE_ARRAY_MEMBER];
//     char host[FLEXIBLE_ARRAY_MEMBER];
//     unsigned int port;
//     char path[FLEXIBLE_ARRAY_MEMBER];
//     char query[FLEXIBLE_ARRAY_MEMBER];
//     char fragment[FLEXIBLE_ARRAY_MEMBER];
// } URL;

// typedef struct
// {
//     char *protocol;
//     char *host;
//     unsigned int port;
//     char *path;
//     char *query;
//     char *fragment;
// } URL;

typedef struct {
  char vl_len_[4];
  int protocol;
  int host;
  unsigned port;
  int path;
  int query;
  int fragment;
  char data[1];
} URL;

typedef struct varlena VAR_ARR;

URL * get_input_url(VAR_ARR* input_arr){
    URL *url = (URL *)(&(input_arr->vl_dat));
    url = (URL *) pg_detoast_datum(input_arr);
    return url;
}

/*
 * - Copies the fragment of url to the url destination
 * - Returns an incremented offset to next fragment
 */
int copyString(URL *url, int *dest, char* src, int size, int offset) {
    int s = size + 1;
    memset(url->data + offset, 0, s);
    *dest = s;
    memcpy(url->data + offset, src, s);
    return offset + s;
}

static int extract_port_from_protocol(char *protocol) {
  if (strcasecmp(protocol, "http") == 0) return 8080;
  if (strcasecmp(protocol, "https") == 0) return 443;
  if (strcasecmp(protocol, "ftp") == 0) return 21;
  if (strcasecmp(protocol, "ssh") == 0) return 22;
  return 0;
}

bool primitive_compare(URL *left, URL *right) {
    return  left->port == right->port &&
            left->protocol == right->protocol &&
            left->host == right->host &&
            left->path == right->path &&
            left->query == right->query &&
            left->fragment == right->fragment;
}
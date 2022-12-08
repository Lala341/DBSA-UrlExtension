/**
 * url PostgreSQL input/output function for bigint
 *
 */

#include <stdio.h>
#include "utils.h"
#include "fmgr.h"

PG_MODULE_MAGIC;

#define DEFAULT_URL_SEGMENT_LEN 1

typedef struct {
  char vl_len_[4];
  int protocol;
  int userinfo;
  int host;
  unsigned port;
  int path;
  int query;
  int fragment;
  char data[1];
} URL;

// If struct url is changed this value should be reflected
const unsigned SEGMENTS = 6;  

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
    size_t s = size + 1;
    memset(url->data + offset, 0, s);
    *dest = s;
    memcpy(url->data + offset, src, size);
    return offset + s;
}

// Copy a 0 to memory
int copyNullString(URL *url, int *dest, int size, int offset) {
    size_t s = size + 1;
    memset(url->data + offset, 0, s);
    *dest = s;
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
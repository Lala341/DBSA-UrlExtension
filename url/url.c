/*
 * url PostgreSQL input/output function for bigint
 *
 */

#include <stdio.h>
#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/guc.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>


PG_MODULE_MAGIC;


Datum url_in(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS){
   char       *str = PG_GETARG_CSTRING(0);
    
    PG_RETURN_TEXT_P(str);
}   


PG_FUNCTION_INFO_V1(url_out);
Datum
url_out(PG_FUNCTION_ARGS)
{
    Datum arg = PG_GETARG_DATUM(0);

	PG_RETURN_CSTRING();
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "postgres.h"

// char* replace(char* str, char find, char replace){
//     char *current_pos = strchr(str,find);
//     while (current_pos) {
//         *current_pos = replace;
//         current_pos = strchr(current_pos,find);
//     }
//     return str;
// }

void removeChar(char * str, char c){
    int i, j;
    int len = strlen(str);
    for(i=0; i<len; i++)
    {
        if(str[i] == c)
        {
            for(j=i; j<len; j++)
            {
                str[j] = str[j+1];
            }
            len--;
            i--;
        }
    }
}

char * extractStr(regmatch_t pmatch, const char *str) {
    int len = pmatch.rm_eo - pmatch.rm_so;

    // size_t size = VARSIZE(str);
    // text *destination = (text *) palloc(VARHDRSZ + size);
    char *dest = (char *) palloc0( (VARHDRSZ + len + 1) * sizeof(char));
    SET_VARSIZE(dest, VARHDRSZ + len + 1);

    // Copy the url from regex find_start till find_end
    strncpy(dest, str + pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);
    // dest[len + 1] = 0;
    return dest;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
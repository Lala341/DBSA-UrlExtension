#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "postgres.h"

int countChar(char * str, char c);
char * stripString(char * str);
char * extractStr(regmatch_t pmatch, const char *str);
bool isSpecialChar(char c);
int countSpecialChars(char * str);
char* removeChar(char * str, char c);

int countChar(char * str, char c){
    int i, count = 0;
    int len = strlen(str);
    for(i=0; i<len; i++)
        if(str[i] == c) count++;
    return count;
}

bool isSpecialChar(char c) {
    return (c == '\'' || c == '"' || c == '(' || c == ')' || c == ' ');
}

int countSpecialChars(char * str){
    int i, count = 0;
    int len = strlen(str);
    for(i=0; i<len; i++)
        if(isSpecialChar(str[i]))
            count++;
    return count;
}

// void removeChar(char * str, char c){
//     int i, j;
//     int len = strlen(str);
//     for(i=0; i<len; i++)
//     {
//         if(str[i] == c)
//         {
//             for(j=i; j<len; j++)
//             {
//                 str[j] = str[j+1];
//             }
//             len--;
//             i--;
//         }
//     }
// }

char * removeChar(char * str, char c){
    int i, j = 0;
    int len = strlen(str);
    int spChar = countChar(str, c);
    if(spChar == 0)
        return str;
    char* result = palloc0(sizeof(char) * (len + 1 - spChar));
    for(i=0; i<len; i++)
    {
        if(str[i] != c) {
            result[j++] = str[i];
        }
    }
    if(j < len) {
        result[j] = '\0';
    }
    return result;
}

char * stripString(char * str){
    int i, j = 0;
    int len = strlen(str);
    int spChar = countSpecialChars(str);
    if(spChar == 0)
        return str;
    char* result = palloc0(sizeof(char) * (len - spChar));
    for(i=0; i<len; i++)
    {
        if(!isSpecialChar(str[i])) {
            result[j++] = str[i];
        }
    }
    if(j < len) {
        result[j] = '\0';
    }
    return result;
}

char * extractStr(regmatch_t pmatch, const char *str) {
    int len = pmatch.rm_eo - pmatch.rm_so;

    // size_t size = VARSIZE(str);
    // text *destination = (text *) palloc(VARHDRSZ + size);
    char *dest = (char *) palloc0( (len) * sizeof(char));
    // SET_VARSIZE(dest, len);

    // Copy the url from regex find_start till find_end
    strncpy(dest, str + pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);
    // dest[len + 1] = 0;
    return dest;
}

char * copyStr(const char *source) {
    size_t len = strlen(source) + 1;

    char *dest = (char *) palloc0( len );
    SET_VARSIZE(dest, len);
    memcpy(dest, source, len);
    // strncpy(dest, source, len);
    return dest;
}

bool compairChars(char * str1, char * str2, int len){
    for(int i=0; i<len; i++)
        if(str1[i] != str2[i]) 
            return false;
    return true;
}

// TODO delete before submitting
void test() {
    ereport(
            ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
            errmsg("TEST"))
        );
}
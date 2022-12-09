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

    // char *dest = (char *) palloc0( (len) * sizeof(char));
    // // Copy the url from regex find_start till find_end
    // strncpy(dest, str + pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);

    char *ptr_start = str + pmatch.rm_so;
    char *dest = palloc0(len + 1);
    memset(dest, 0, len + 1);
    memcpy(dest, ptr_start, len);
    
    return dest;
}

// char * copyStr(const char *source) {
//     size_t len = strlen(source) + 1;

//     char *dest = (char *) palloc0( len );
//     SET_VARSIZE(dest, len);
//     memcpy(dest, source, len);
//     // strncpy(dest, source, len);
//     return dest;
// }

bool compairChars(char * str1, char * str2, int len){
    for(int i=0; i<len; i++)
        if(toupper(str1[i]) != toupper(str2[i])) 
            return false;
    return true;
}

unsigned num_digits(const unsigned n) {
    if (n < 10) return 1;
    return 1 + num_digits(n / 10);
}

bool check_regex_part(bool showerror, char* str, char* data){

    char *spec = stripString(str);
    regex_t rx_p;
    int rc_p;
    regmatch_t pmatch_p[2];
    char msg_p[100];
    const char *p_regex =data;

    if (0 != (rc_p = regcomp(&rx_p, p_regex, REG_EXTENDED))) {

        regerror(rc_p, &rx_p, msg_p, 100); ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc_p, msg_p)
            )
        );
    }

    
    if (0 != (rc_p = regexec(&rx_p, spec, 2, pmatch_p, 0))) {

        if(showerror==true){
             ereport(
                ERROR,
                (
                    errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                    errmsg("Unsupported/invalid URL spec provided (Error Code: %d): \"%s\"", rc_p, spec)
                )
            );
        }
        regfree(&rx_p);
       return false;
    }
    regfree(&rx_p);
    return true;

}

bool check_regex_acceptable(char* spec, char* data){

    regex_t rx_p;
    int rc_p;
    regmatch_t pmatch_p[2];
    char msg_p[100];
    const char *p_regex =data;

    if (0 != (rc_p = regcomp(&rx_p, p_regex, REG_EXTENDED))) {

        regerror(rc_p, &rx_p, msg_p, 100); 
        ereport(
            ERROR,
            (
                errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("Internal REGEX error in URL (Error Code: %d): \"%s\"", rc_p, msg_p)
            )
        );
    }

    
    if (0 != (rc_p = regexec(&rx_p, spec, 2, pmatch_p, 0))) {
        regfree(&rx_p);
        return false;
    }
    regfree(&rx_p);
    return true;

}

char *strtokm(char *str, const char *delim)
{
    static char *tok;
    static char *next;
    char *m;

    if (delim == NULL) return NULL;

    tok = (str) ? str : next;
    if (tok == NULL) return NULL;

    m = strstr(tok, delim);

    if (m) {
        next = m + strlen(delim);
        *m = '\0';
    } else {
        next = NULL;
    }

    return tok;
}
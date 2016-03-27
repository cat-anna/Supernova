
#ifndef _SUPERNOVA_INCLUDE_LIB_H
#define _SUPERNOVA_INCLUDE_LIB_H

#include <sysdef.h>

void Sleep(uint32 ms);
void putf(const char* c);

int strcmpdelim(const char *s1, const char *s2, const char delim);

void itoa(int n, char s[], int base);
void reverse(char s[]);

#endif


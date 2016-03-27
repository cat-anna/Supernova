/*
	string.c is part of PawlOS libc
	this file can be included
*/

#ifndef LIBC_STRING_C
#define LIBC_STRING_C

#define uint8 char
#define uint32 int

void memcpy(void *dest, const void *src, uint32 len){
    const uint8 *sp = (const uint8*)src;
    uint8 *dp = (uint8*)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

void memset(void *dest, uint8 val, uint32 len){
    uint8 *temp = (uint8*)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

char *strrchr(const char *s, int c){
	char *rtnval = 0;	
	do {
		if (*s == c)
		rtnval = (char*) s;
	} while (*s++);
	return (rtnval);
}

char* strchr(const char *s, int c){
	/* Scan s for the character.  When this loop is finished,
	s will either point to the end of the string or the
	character we were looking for.  */
	while (*s != '\0' && *s != (char)c)s++;
	return ( (*s == c) ? (char *) s : 0 );
} 

int strcmp(const char *s1, const char *s2){
	unsigned char uc1, uc2;
	/* Move s1 and s2 to the first differing characters 
	in each string, or the ends of the strings if they
	are identical.  */
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}
	/* Compare the characters as unsigned char and
	return the difference.  */
	uc1 = (*(unsigned char *) s1);
	uc2 = (*(unsigned char *) s2);
	return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}

char *strncpy(char *dest, const char *src, uint32 n){
    char *ret = dest;
    do {
        if (!n--) return ret;
    } while (*dest++ == *src++);
    while (n--) *dest++ = 0;
    return ret;
}

uint32 strlen(const char *s){
	if(!s)return 0;
	const char *p = s;
//Loop over the data in s. 
	while (*p != '\0') p++;
	return (uint32)(p - s);
}

char *strcpy(char *s1, const char *s2){
	if(!s1 || !s2) return 0;
	char *dst = s1;
	const char *src = s2;
	// Do the copying in a loop.  
	while ((*dst++ = *src++) != '\0'){}; // The body of this loop is left empty. 
	// Return the destination string.  
	return s1;
}

char *strcat(char *s1, const char *s2){
	if(!s1)return 0;
	char *s = s1;
	//Move s so that it points to the end of s1.
	while (*s != '\0') s++;
	//Copy the contents of s2 into the space at the end of s1.

	while ((*s++ = *s2++) != '\0'){};

	return s1;
}

int strcmpdelim(const char *s1, const char *s2, const char delim){
	unsigned char uc1, uc2;
	/* Move s1 and s2 to the first differing characters 
	in each string, or the ends of the strings if they
	are identical.  */
	while (*s1 && *s1 == *s2 && *s1 != delim && *s2 != delim) {
		s1++;
		s2++;
	}
	/* Compare the characters as unsigned char and
	return the difference.  */
	if((*s1 == 0 || *s1 == delim)&&(*s2 == 0 || *s2 == delim))return 0;
	uc1 = (*(unsigned char *) s1);
	uc2 = (*(unsigned char *) s2);
	return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}

int strcmppat(const char *s, const char *pat){	
	if(!s)return 1;
	if(!pat)return 0;
	uint8 star = 0;
	while(*s/* && *pat*/){
		if(*pat == '*'){
			star = 1;
			pat++;
			if(!*pat)return 0;
		}		
		if(star == 0){
			if(*pat != *s)return 1;
			pat++;
		}else
			if(*pat == *s){
				star = 0;
				pat++;
			}
		s++;
	}	
//	if(*s || *pat) return 1;
	return 0;
}

#endif

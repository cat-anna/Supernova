#ifndef _HEADER_CHS_H
#define _HEADER_CHS_H

typedef struct{
	unsigned char c, h, s;
} CHS_t, *CHS_p;

static inline void CHS_calc(CHS_p chs, unsigned LBA){
	chs->c =  LBA /(18  * 2);
	chs->h = (LBA / 18) % 2;
	chs->s = (LBA % 18) + 1;
}

static inline void CHS_set(CHS_p chs, unsigned char c, unsigned char h, unsigned char s){
	chs->c = c;
	chs->h = h;
	chs->s = s;
}

#endif

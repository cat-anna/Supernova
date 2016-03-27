#ifndef LIBC_STDDEF_H
#define LIBC_STDDEF_H

typedef unsigned long long 	uint64;
typedef          long long 	sint64;
#ifndef _STDDEF_H
typedef unsigned long  		ulong;
#endif
typedef 		 long  		slong;
typedef unsigned int   		uint32;
typedef          int   		sint32;
typedef unsigned short 		uint16;
typedef          short 		sint16;
typedef unsigned char  		uint8;
typedef          char  		sint8;

#include <stddef.h>

#ifndef _STDDEF_H
typedef unsigned short 		wchar_t;
#endif

typedef void* HANDLE;
typedef volatile uint32 SPINLOCK;

#ifndef _BUILDING_SSM_

typedef uint32 bool;
enum{
	true	= 1,
	false	= 0,
};

enum{
#ifndef NULL
	NULL 					= 0,
#endif
	INVALID_HANDLE_VALUE	= 0,
};

#endif

#endif

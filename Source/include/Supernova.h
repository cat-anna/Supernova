#ifndef _SUPERNOVA_H
#define _SUPERNOVA_H

#ifndef _INLINES_H_

inline static void outw(int port, unsigned short data){
   __asm__ __volatile__("outw %%ax,%%dx\n\t" :: "a" (data), "d" (port));
}

static inline void Breakpoint(){
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8aE0);
}

#endif

#include <sysdef.h>
#include <Supernova/ErrorCodes.h>
#include <Supernova/files.h>
#include <Supernova/messages.h>
#include <Supernovalib.h>

#define HiWord(A) (A >> 16)
#define LowWord(A) (A & 0xFFFF)
#define MakeDword(A, B) ((A << 16) | B)

#define MinValue(A, B) ((A)<(B)?(A):(B))
#define MaxValue(A, B) ((A)>(B)?(A):(B))

typedef struct VersionInfo_s{
	unsigned major;
	unsigned minor;
	unsigned build;
}VersionInfo_t, *VersionInfo_p;

typedef struct SystemInfo_s{
	unsigned size;
	VersionInfo_t version;
}SystemInfo_t, *SystemInfo_p;

#endif 

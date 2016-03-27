//LCF=0x16F0A96E;name=kheap.h;part=kernel;
#ifndef KLIBC_H
#define KLIBC_H

#include "kernel.h"

enum {
	KERNEL_HEAP_START 			= 0xC8000000,
	KERNEL_HEAP_END 			= 0xD0000000,
	KERNEL_HEAP_TABLE_SIZE		= 0x00010000,
};

void kprintf(const char* format, ...);

void* kmalloc(uint32 sz);
void kfree(void *p);

uint32 Lowkmalloc(uint32 sz, uint32 *Vaddr, uint32 *PhAddr);
void Lowkfree(uint32 Vaddr, uint32 PhAddr);

void ShowKHeapStructure();

int strcmpdelim(const char *s1, const char *s2, const char delim);

#endif

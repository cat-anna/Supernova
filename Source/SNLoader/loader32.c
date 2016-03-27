#include <sysdef.h>
#include "headers/ELF.h"
#include "headers/Loader.h"

extern unsigned char Con_X_Pos;
extern unsigned char Con_Y_Pos;
unsigned char Con_attrib = 0x03;

static inline void outb(unsigned short port, unsigned char val) {
	asm volatile("outb %0,%1"::"a"(val), "Nd" (port));
}

/*
inline static void outw(int port, unsigned short data) {
	__asm__ __volatile__("outw %%ax,%%dx\n\t" :: "a" (data), "d" (port));
}

static inline void Breakpoint() {
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8aE0);
}*/


void putch(unsigned char c) {
	if (c == '\n') {
		Con_X_Pos = 0;
		Con_Y_Pos++;
	} else {
		short *where = (short*) (0xB8000 + ((Con_Y_Pos) * 80 + Con_X_Pos) * 2);
		*where = (c | (Con_attrib << 8));
		Con_X_Pos++;
	}

}

void puts(char *s) {
	while (*s)
		putch(*s++);
	short temp = (Con_Y_Pos * 80 + Con_X_Pos);
	outb(0x03D4 + 0, 14);
	outb(0x03D4 + 1, temp >> 8);
	outb(0x03D4 + 0, 15);
	outb(0x03D4 + 1, temp);
}

/*void put_hex(unsigned i) {
	//print("0x");
	int b = 0;
	int a;
	for (a = 2 * sizeof(int) - 1; a >= 0; a--) {
		char c = "0123456789ABCDEF"[((i >> a * 4) & 0xF)];
		if (!b && c == '0')
			continue;
		b = 1;
		putch(c);
	}
	if (!b)
		putch('0');
}*/

void memcpy(void *dest, const void *src, uint32 len) {
	const uint8 *sp = (const uint8*) src;
	uint8 *dp = (uint8*) dest;
	for (; len != 0; len--)
		*dp++ = *sp++;
}

//relocates elf file at its designed address, returns entry point
unsigned Relocate(void* ELF) {
	Elf32_Ehdr *header = (Elf32_Ehdr*) ELF;
	if (!IS_ELF(header)) {
		Con_attrib = 0x04;
		puts("Error! Wrong file format!\n");
		while (1) {};
	}

	Elf32_Phdr *proghead = (Elf32_Phdr*) ((unsigned) ELF + header->e_phoff);

	unsigned memsz = proghead->p_memsz;
	unsigned filesz = proghead->p_filesz;

	char* k_src = (char*) ((unsigned) ELF + proghead->p_offset);
	char* k_dst = (char*) (proghead->p_paddr);
	while (filesz-- && memsz--)
		*k_dst++ = *k_src++; //copy Kernel Code and Data
	while (memsz--)
		*k_dst++ = 0; //Zero bss section

	return header->e_entry;
}


void LoaderMain(unsigned *KernelEntry) {
#if 0
	int i;
	int count = DataFromLoader->LoadedFilesCount;
	for (i = 0; i < count; i++) {
		LoaderFile_p f = &DataFromLoader->LoadedFiles[i];
		puts(f->Name);
		putch('\n');
	}
	while(1);
#endif
	//save Boot partition id form bootsector
//	DataFromLoader->BootPartitionID = *((unsigned*)(0x7C0F));
	*KernelEntry = Relocate(DataFromLoader->LoadedFiles[0].FilePtr);
}

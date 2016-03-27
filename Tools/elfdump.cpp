#include <cstdlib>
#include <stdio.h>

typedef unsigned long long uint64;
typedef          long long sint64;
typedef unsigned long  ulong;
typedef unsigned int   uint32;
typedef          int   sint32;
typedef unsigned short uint16;
typedef          short sint16;
typedef unsigned char  uint8;
typedef          char  sint8;

#include "d:\!prog\!OS\Supernova\Source\include\headers\elf.h"
#include "windows.h"

using namespace std;

int main(int argc, char *argv[])
{
	
	if(argc < 2){
		printf("Not enough parameters.\n");	
		return 1;
	}
	
	void* f = malloc(1024*1024);
	
	HANDLE inf = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD read;	
	ReadFile(inf, f, 1024*1024, &read, NULL);
	if(read == 1024*1024){
		printf("File is too large!!!");	
		return 1;
	}
	if(read == 0){
		printf("Error reading file!!!");	
		return 1;
	}	
	CloseHandle(inf);
	
	printf("header dump:\n");

	Elf32_Ehdr * header = (Elf32_Ehdr*)f;
	
/*  Elf32_Half	e_machine;			 Architecture 
  Elf32_Word	e_version;			/ Object file version 
  Elf32_Half	e_ehsize;			/ ELF header size in bytes 
  Elf32_Half	e_phentsize;		/ Program header table entry size 
  Elf32_Half	e_phnum;			/ Program header table entry count 
  Elf32_Half	e_shentsize;		/ Section header table entry size 
*/

	printf("  ident[EI_CLASS]: %d\n", header->e_ident[EI_CLASS]);
	printf("  ident[EI_DATA]: %d\n", header->e_ident[EI_DATA]);
	printf("  entry: %x\n", header->e_entry);
	printf("  type: %d\n", header->e_type);
	printf("  flags: %d\n", header->e_flags);
	printf("  phoff: %x\n", header->e_phoff);
	printf("  phnum: %d\n", header->e_phnum);
	printf("  shoff: %x\n", header->e_shoff);
	printf("  shnum: %d\n", header->e_shnum);
	printf("  shstrndx: %d\n\n", header->e_shstrndx);
	
	Elf32_Shdr * sections = (Elf32_Shdr*)((uint32)f + header->e_shoff);
	
	int strsh = 0;

	printf("Sections dump:\n  no offset\tsize\taddr\ttype\tname\n");
	for (int i = 0; i < header->e_shnum; i++)
	{
		printf("  %2d. %x\t%x\t%x\t%d\t%s\n", i, sections[i].sh_offset,
			sections[i].sh_size, sections[i].sh_addr, sections[i].sh_type,
			 ((int)f + sections[header->e_shstrndx].sh_offset + sections[i].sh_name));
		if(sections[i].sh_type == SHT_STRTAB && i != header->e_shstrndx)		
			strsh = i;
	}
	
	printf("\ndumping sections data:\n");
	for (int i = 0; i < header->e_shnum; i++)
	{
		printf("\nsection %d (%s) - ", i, ((int)f + sections[header->e_shstrndx].sh_offset + sections[i].sh_name));
		switch(sections[i].sh_type){
			case SHT_NULL:
				printf("Null section\n");
				break;	
			case SHT_PROGBITS:
				printf("Program data\n");
				break;
			case SHT_SYMTAB:{
				printf("Symbol Table\n");
				uint32 entryc = sections[i].sh_size / sections[i].sh_entsize;
				Elf32_Sym *SymTab = (Elf32_Sym*)((uint32)f + sections[i].sh_offset);
				printf("   no  value\tsize\tbind\ttype\tother\tshndx\tname\n");
				for (int j = 0; j < entryc; j++)
				{
					printf("  %2d.  %-8x %-8x%-8x%-8x%-8x%-8x%s\n", 
						j, SymTab[j].st_value, SymTab[j].st_size, 
						ELF32_ST_BIND(SymTab[j].st_info), ELF32_ST_TYPE(SymTab[j].st_info),
						SymTab[j].st_other, SymTab[j].st_shndx,
						((uint32)f + sections[strsh].sh_offset+SymTab[j].st_name));
				}				
				}break;
			case SHT_STRTAB:
				printf("String Table\n");	
				break;
			case SHT_RELA:{
				Elf32_Shdr *Rel_Sect = &sections[sections[i].sh_info];
				printf("(RELA) Relocating data for %d (%s)\n", sections[i].sh_info, 
					((int)f + sections[header->e_shstrndx].sh_offset + Rel_Sect->sh_name));
				Elf32_Sym *SymTab = (Elf32_Sym*)((uint32)f + sections[sections[i].sh_link].sh_offset);
				Elf32_Rela *Rel = (Elf32_Rela*)((uint32)f + sections[i].sh_offset);
				uint32 Rel_entryc = sections[i].sh_size / sections[i].sh_entsize;	
	
				printf("  no  offset\tsym\ttype\taddend\tname\n");
				uint32 j = 0;
				for(; j < Rel_entryc; j++)
				{
					uint32 R_sym = ELF32_R_SYM(Rel[j].r_info);
					uint32 R_type = ELF32_R_TYPE(Rel[j].r_info);
					printf("  %2d.  %-8x%d\t%d\t%-8x%s\n", 
						i, Rel[j].r_offset, R_sym, R_type, Rel[j].r_addend,
						((int)f + sections[strsh].sh_offset + SymTab[R_sym].st_name));
				}	
				}break;
			case SHT_REL:{
				Elf32_Shdr *Rel_Sect = &sections[sections[i].sh_info];
				printf("(REL) Relocating data for %d (%s)\n", sections[i].sh_info, 
					((int)f + sections[header->e_shstrndx].sh_offset + Rel_Sect->sh_name));
				Elf32_Sym *SymTab = (Elf32_Sym*)((uint32)f + sections[sections[i].sh_link].sh_offset);
				Elf32_Rel *Rel = (Elf32_Rel*)((uint32)f + sections[i].sh_offset);
				uint32 Rel_entryc = sections[i].sh_size / sections[i].sh_entsize;	
	
				printf("   no  offset\tsym\ttype\tname\n");
				uint32 j = 0;
				for(; j < Rel_entryc; j++)
				{
					uint32 R_sym = ELF32_R_SYM(Rel[j].r_info);
					uint32 R_type = ELF32_R_TYPE(Rel[j].r_info);
					printf("  %2d.  %-8x%d\t%d\t%s\n", 
						j, Rel[j].r_offset, R_sym, R_type, 
						((int)f + sections[strsh].sh_offset + SymTab[R_sym].st_name));
				}	
				}break;	
			case SHT_NOBITS:
				printf("no bits section\n");
				break;
			case SHT_DYNAMIC:{
				printf("Dynamic section\n");
				Elf32_Dyn *Dyn = (Elf32_Dyn*)((int)f + sections[i].sh_offset);
				uint32 entryc = sections[i].sh_size / sections[i].sh_entsize;	
	
				printf("   no  d_tag\td_un\n");
				uint32 j = 0;
				for(; j < entryc; j++)
				{
					printf("  %2d.  %-8x %-8x\n", 
						j, Dyn[j].d_tag, Dyn[j].d_un.d_ptr);
				}						
				}break;
			case SHT_HASH:{
				printf("Symbol Hash Table\n");
				/*uint32 entryc = sections[i].sh_size / sections[i].sh_entsize;
				Elf32_Sym *SymTab = (Elf32_Sym*)((uint32)f + sections[i].sh_offset);
				printf("   no  value\tsize\tbind\ttype\tother\tshndx\tname\n");
				for (int j = 0; j < entryc; j++)
				{
					printf("  %2d.  %-8x %-8x%-8x%-8x%-8x%-8x%s\n", 
						j, SymTab[j].st_value, SymTab[j].st_size, 
						ELF32_ST_BIND(SymTab[j].st_info), ELF32_ST_TYPE(SymTab[j].st_info),
						SymTab[j].st_other, SymTab[j].st_shndx,
						((uint32)f + sections[strsh].sh_offset+SymTab[j].st_name));
				}		*/
				}break;	
			case SHT_DYNSYM:{
				printf("Dynamic Symbol Table\n");
				uint32 entryc = sections[i].sh_size / sections[i].sh_entsize;
				Elf32_Sym *SymTab = (Elf32_Sym*)((uint32)f + sections[i].sh_offset);
				printf("   no  value\tsize\tbind\ttype\tother\tshndx\tname\n");
				for (int j = 0; j < entryc; j++)
				{
					printf("  %2d.  %-8x %-8x%-8x%-8x%-8x%-8x%s\n", 
						j, SymTab[j].st_value, SymTab[j].st_size, 
						ELF32_ST_BIND(SymTab[j].st_info), ELF32_ST_TYPE(SymTab[j].st_info),
						SymTab[j].st_other, SymTab[j].st_shndx,
						((uint32)f + sections[strsh].sh_offset+SymTab[j].st_name));
				}				
				}break;				
				
				
/*
#define SHT_HASH	5		 symbol hash table section 
#define SHT_NOTE	7		 note section 
#define SHT_DYNSYM	11		 dynamic symbol table section 
#define SHT_NUM		12		 number of section types */
			default:
				printf("unknown type!!!\n");
		}
	}	
	
/*
typedef struct {
	Elf32_Word	st_name;	 String table index of name. 
	Elf32_Addr	st_value;	 Symbol value. 
	Elf32_Size	st_size;	 Size of associated object. 
	unsigned char	st_info;	Type and binding information. 
	unsigned char	st_other;	Reserved (not used). 
	Elf32_Half	st_shndx;	 Section index of symbol. 
} Elf32_Sym;
*/
/*	
/*	
	printf("\nDumping section 1:\n");
	printf("\tName:\t%s\n", elf_sectionName(image, 1));
	printf("\tType:\t%d\n", sections[1].sh_type);
	printf("\tFlags:\t0x%x\n", sections[1].sh_flags);
	printf("\tAddr:\t0x%x\n", sections[1].sh_addr);
	printf("\tOffset:\t0x%x\n", sections[1].sh_offset);
	printf("\tSize:\t0x%x\n", sections[1].sh_size);
	printf("\tLink:\t0x%x\n", sections[1].sh_link);
	printf("\tInfo:\t0x%x\n", sections[1].sh_info);
	printf("\tAddrAl:\t0x%x\n", sections[1].sh_addralign);
	printf("\tEtSize:\t0x%x\n", sections[1].sh_entsize);
*/
/*
	Elf32_Phdr * p_headers = (Elf32_Phdr*)((uint32)image + header->e_phoff);

	printf("Program header dump:\n");
	for (i = 0; i < header->e_phnum; i++)
	{
		printf("  %s\n", elf_sectionName(image, i));
		printf("  p_offset: %x\n", p_headers[i].p_offset);
	}
	*/

    return EXIT_SUCCESS;
}

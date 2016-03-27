#define _COMPILING_KERNEL_HEAP_
#include "kernel.h"

#include "stdarg.h"

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

static void print_s(char *s, uint8 minlenght){
	if(!s || !*s)return;
	uint8 l = strlen(s);
	for(;l < minlenght; l++)putch(' ');
	for(;*s;)putch(*s++);
}

static void print_dec(uint32 n, uint8 minlenght, uint8 zerosc){
    uint32 acc = n;
    char c[12];
	memset(c, '0', 11);
    uint8 i = 0;
    while (acc > 0){
        c[i] = '0' + acc % 10;
        acc /= 10;
        i++;
    }
	if(n == 0)i++;
    if(i<zerosc)i=zerosc;
	uint8 j;
    for(j = (i<zerosc)?zerosc:i; j < minlenght; j++){
		putch(0x20);
	}
  //  if(n>0)i--;
	for(;i;)putch(c[--i]);
//	putch(c[0]);
}

static void print_bin(int i){
	//print("0b");
	int b = 0;
	int a;
	for (a=31; a>=0; a--) {
		char c = "01"[(i >> a) & 1];
		if(!b && c == '0')continue;
		b = 1;
		putch(c);
	}
	if(!b)putch('0');
}

static void print_hex(uint32 i, char *Params, uint32 ParamsDef){
	int b = 0;
	int a;
	char buffer[9];
	buffer[8] = 0;
	for (a = 2 * sizeof(int) - 1; a >= 0; --a) {
		char c = "0123456789ABCDEF"[((i >> a*4) & 0xF)];
		buffer[7-a] = c;
		if(c && !b)b = a+1;
	}
	if(ParamsDef & 1){
		b = Params[0];
	}
	if(b > 8)b = 8;
	buffer[b] = 0;
	puts(buffer + 8 - b);
}

void ConSetAttrib(char *p, uint32 def)
{
	static char text_col, back_col;
	if(def & 1){
		text_col = (Console->attrib & 0xF);
		Console_SetTextColor(p[0]);
	}
	if(def & 1){
		back_col = Console->attrib >> 4;
		Console_SetBackColor(p[1]);
	}

	if(!(def & 3)){
		Console_SetTextColor(text_col);
		Console_SetBackColor(back_col);
	}
}

void kprintf(const char* fmt, ...){
	va_list vl;
	va_start(vl,fmt);

	for(;*fmt;fmt++)
	{
		if(*fmt == '%'){
			char params[4] = {0, 0, 0, 0};
			unsigned paramsdef = 0;
			int parno = 0;
			while(parno < 4){
				fmt++;
				if(*fmt >= '0' && *fmt <= '9'){
					params[parno] *= 10;
					params[parno] += (*fmt - '0');
					paramsdef |= 1 << parno;
				} else {
					if(*fmt == ';') parno++;
					else {
						switch(*fmt){
							case 'd': print_dec(va_arg(vl, int), params[0], params[1]); break;
							case 's': print_s(va_arg(vl, char*), params[0]); break;
							case 'c': putch(va_arg(vl, int)); break;
							case 'x': print_hex(va_arg(vl, int), params, paramsdef); break;
							case '%': putch('%'); break;
	//						case 'g': Console_Goto((paramsdef[0])?(params[0]):(300), (paramsdef & 1)?(params[1]):(300)); break;
							case 'w': print_bin(va_arg(vl, int)); break;
							case 'b': Console_SetBackColor(params[0]); break;
							case 'f': Console_SetTextColor(params[0]); break;
							case 't': ConSetAttrib(params, paramsdef); break;
						}//switch(*fmt)
						break;
					}//if(*fmt == ';')
				}//esle isdigit
			}//while(parno < 3)
		}//if(*fmt == '%')
		else putch(*fmt);
	}
	va_end(vl);
}


enum {
	HEAP_HEADER_MAGIC	= 0x123890AB,
	HEAP_FOOTER_MAGIC	= 0x2684BDEA,
	HEAP_DELTA_SIZE		= 0x4000,
};

typedef struct{
    uint32 magic;			//Magic number, used for error checking and identification.
    uint32 inuse;			//0 means block is used otherwise next free block
    uint32 size;			//Size of the block, including size of header and footer
} heap_header_t, *heap_header_p;

typedef struct{
    uint32 magic;     		// Magic number, used for error checking and identification.
    heap_header_t *header;	// Pointer to the block header.
} heap_footer_t, *heap_footer_p;

typedef struct{
    uint32 begin;			// begin of allocating area
    uint32 end;				// end of allocating area
	uint32 size;			// current end of heap allocated area

	uint32 table_begin;		// table begin
	uint32 table_end;		// table end
	uint32 table_size;		// current table size
	uint32 table_count;		// count of valid entry's is table
} __attribute__((packed)) heap_t, *heap_p;

heap_t k_heap;

void ShowKHeapStructure(){
#if 0
	kprintf("Kernel Heap:\n\tStart:%x\tEnd:%x\nStructure:\n", k_heap.start, k_heap.end);
	heap_header_p h = (heap_header_p)k_heap.start;
	while((uint32)h < k_heap.end && (uint32)h >= k_heap.start){
		kprintf("\ta:%x s:%8x iu:%8x\n", h, h->size, h->inuse);
		if(h->size == 0)break;
		h = (heap_header_p)((uint32)h + h->size);
	}
#endif
}

static void RemoveFreeBlock(heap_p heap, heap_header_p h_old){
	uint32* T = (uint32*)heap->table_begin;
	while((uint32)T < heap->table_size && *T){
		if(*T == (uint32)h_old)break;
		++T;
	}
	while(*T){
		*T = *(T + 1);
		++T;
	}
}

//TODO: case when heap->table gets full
static void InsertFreeBlock(heap_p heap, heap_header_p h_new){
	uint32* T = (uint32*)heap->table_begin;
	h_new->inuse = 0;
	if(heap->table_count == 0){
		*T = (uint32)h_new;
		++T;
		*T = 0;
		return;
	}

	uint32 i = 0;
	for(; i < heap->table_count; ++i){
		if(((heap_header_p)*T)->size >= h_new->size)break;
		++T;
	}
	uint32 tmp = *T;
	*T = (uint32)h_new;
	for(; i < heap->table_count; ++i){
		++T;
		uint32 s = *T;
		*T = tmp;
		tmp = s;
	}
	++T;
	*T = tmp;
}

static heap_header_p GetFreeBlock(heap_p heap, uint32 size){
	uint32* T = (uint32*)heap->table_begin;

	while((uint32)T < heap->table_size && *T){
		if(((heap_header_p)*T)->size >= size)break;
		++T;
	}
	heap_header_p h = (heap_header_p)*T;

	if(!h){//big enough header was not found
		uint32 old_end = heap->size;
		uint32 delta_size = (size / HEAP_DELTA_SIZE + 1) * HEAP_DELTA_SIZE;
		uint32 new_end = old_end + delta_size;

		if(KernelHeapAlloc(old_end, new_end, 0)) //get some memory
			return 0;
		 heap->size = new_end;
		 ++KernelAreaVersion;

		//if last block of old heap is unused increase its size
		heap_footer_p f_test = (heap_footer_p)(old_end - sizeof(heap_footer_t));
		if(f_test->magic == HEAP_FOOTER_MAGIC &&
				f_test->header->magic == HEAP_HEADER_MAGIC &&
				f_test->header->inuse == 0)
		{
			h = f_test->header;
			RemoveFreeBlock(heap, h);
			h->size += delta_size;
		} else { // otherwise add new block
			h = (heap_header_p)old_end;
			h->magic = HEAP_HEADER_MAGIC;
			h->size = delta_size;
		}
		f_test = (heap_footer_p)(new_end - sizeof(heap_footer_t));
		f_test->header = h;
		f_test->magic = HEAP_FOOTER_MAGIC;
	} else { //big enough header was found
		while(*T){//remove it
			*T = *(T + 1);
			++T;
		}
	}
	h->inuse = 1;	//mark header as used
	return h;
}

static void* do_alloc(heap_p heap, uint32 sz){
	if(sz == 0)return 0;

	//check and set 4 bytes boundary
	if (sz % sizeof(uint32))sz += (sizeof(uint32) - (sz % sizeof(uint32)));
	sz += (sizeof(heap_header_t) + sizeof(heap_footer_t));

	heap_header_p h = GetFreeBlock(heap, sz);//find the smallest hole which will fit
	if(!h)return 0;
	//if remain space in found hole is too small to create new block
	if(h->size - sz < sizeof(heap_header_t) + sizeof(heap_footer_t) + sizeof(uint32)){
		sz = h->size;//increase requested hole size
	} else {//otherwise create new hole after requested one
		heap_header_p h_rest = (heap_header_p)((uint32)h + sz);
		h_rest->magic = HEAP_HEADER_MAGIC;
		h_rest->size = h->size - sz;
		heap_footer_p f_rest = (heap_footer_p)((uint32)h + h->size - sizeof(heap_footer_t));
		f_rest->header = h_rest;
		f_rest->magic = HEAP_FOOTER_MAGIC;
		InsertFreeBlock(heap, h_rest);
	}
	//fill requested hole header and footer
	h->size = sz;
	h->magic = HEAP_HEADER_MAGIC;
	h->inuse = 1;
	heap_footer_p f = (heap_footer_p)((uint32)h + sz - sizeof(heap_footer_t));
	f->header = h;
	f->magic = HEAP_FOOTER_MAGIC;
	return (void*)((uint32)h + sizeof(heap_header_t));
}

static void do_free(heap_p heap, void* p){
	if(!p)return;
//	kprintf("kfree: p:%x", p);
	heap_header_p h = (heap_header_p)((uint32)p - sizeof(heap_header_t));
	heap_footer_p f = (heap_footer_p)((uint32)h + h->size - sizeof(heap_footer_t));
//	kprintf("\th:%x f:%x", h, f);
	if(h->magic != HEAP_HEADER_MAGIC || f->magic != HEAP_FOOTER_MAGIC){
		return;
	}
	bool doAdd = true;
//check on the left
	heap_footer_p f_test = (heap_footer_p)((uint32)h - sizeof(heap_footer_t));
	if((uint32)f_test > heap->begin &&
		f_test->magic == HEAP_FOOTER_MAGIC &&
		f_test->header->magic == HEAP_HEADER_MAGIC &&
		f_test->header->inuse == 0)
	{
//		kprintf("\tleft");
		f_test->header->size += h->size;
		h = f_test->header;
		doAdd = false;
	}

	//check on the right
	heap_header_p h_test = (heap_header_p)((uint32)f + sizeof(heap_footer_t));
	if((uint32)h_test < heap->size &&
		h_test->magic == HEAP_HEADER_MAGIC &&
		h_test->inuse == 0)
	{
	//	kprintf("\tright");
		RemoveFreeBlock(heap, h_test);
		h->size += h_test->size;
		f = (heap_footer_p)((uint32)h + h->size - sizeof(heap_footer_t));
	}

	f->header = h;
	h->magic = HEAP_HEADER_MAGIC;
	f->magic = HEAP_FOOTER_MAGIC;
//	kprintf("\tfree:%d\n", doAdd);
	if(doAdd)InsertFreeBlock(heap, h);
}

void* kmalloc(uint32 sz){
	return do_alloc(&k_heap, sz);
}

void kfree(void *p){
	do_free(&k_heap, p);
}

enum{
	LOW_HEAP_BLOCK_SIZE		= 0x1000,
	LOW_ALLOC_TABLE_SIZE	= LOW_HEAP_SIZE / LOW_HEAP_BLOCK_SIZE,
};

uint8 LowAllocTable[LOW_ALLOC_TABLE_SIZE];

uint32 Lowkmalloc(uint32 sz, uint32 *Vaddr, uint32 *PhAddr){
	if(!sz || !Vaddr || !PhAddr)return ERRORCODE_WRONG_INPUT;
	sz >>= 12;
	uint32 i, free;
	uint8 *tab = LowAllocTable;
	uint32 begin = 0xFF;
	for(i = 0, free = 0; i < LOW_ALLOC_TABLE_SIZE; ++i, ++tab){
		if(*tab == 0){
			if(free == 0)
				begin = i;
			++free;
		}
		else free = 0;
		if(free == sz) break;
	}
	if(free != sz) return ERRORCODE_NO_MORE_MEMORY;
	uint32 offset = LOW_HEAP_BLOCK_SIZE * begin;

	*Vaddr = (LOW_HEAP_LOCATION_VIRTUAL + offset);
	*PhAddr = (LOW_HEAP_LOCATION_PHYSICAL + offset);

	LowAllocTable[begin] = sz;
	--sz;
	++begin;
	while(sz--){
		LowAllocTable[begin] = 0xFF;
		++begin;
	}

/*	kprintf("LOW: %d %x\nDUMP:", i, offset);
	for(i = 0; i < LOW_ALLOC_TABLE_SIZE; ++i)
		kprintf("%2x|", LowAllocTable[i]);
	putch('\n');
	kprintf("Vaddr:%x\tPhAddr:%x\n", *Vaddr, *PhAddr);*/

	return SUCCES;
}

void Lowkfree(uint32 Vaddr, uint32 PhAddr){
	Vaddr = PhAddr = 0;

}

uint32 Heap_init(){
//Kernel heap
	memset(&k_heap, 0, sizeof(heap_t));
	k_heap.table_begin = KERNEL_HEAP_START;
	k_heap.table_end = KERNEL_HEAP_START + KERNEL_HEAP_TABLE_SIZE;
	k_heap.begin = k_heap.table_end;
	k_heap.end = KERNEL_HEAP_END;
	k_heap.size = k_heap.begin;
	k_heap.table_size = k_heap.table_begin;
	if(KernelHeapAlloc(k_heap.size, k_heap.size + HEAP_DELTA_SIZE, 0))
		return ERRORCODE_NO_MORE_MEMORY;
	k_heap.size += HEAP_DELTA_SIZE;
	if(KernelHeapAlloc(k_heap.table_size, k_heap.table_size + 0x1000, 0))
		return ERRORCODE_NO_MORE_MEMORY;
	k_heap.table_size += 0x1000;
	heap_header_p h = (heap_header_p)k_heap.begin;
	h->magic = HEAP_HEADER_MAGIC;
	h->inuse = 0;
	h->size = k_heap.size - k_heap.begin;
	heap_footer_p f = (heap_footer_p)(k_heap.size - sizeof(heap_footer_t));
	f->header = h;
	f->magic = HEAP_FOOTER_MAGIC;
	InsertFreeBlock(&k_heap, h);
//Low kernel heap
	memset(LowAllocTable, 0, sizeof(uint8) * LOW_ALLOC_TABLE_SIZE);
	return SUCCES;
}

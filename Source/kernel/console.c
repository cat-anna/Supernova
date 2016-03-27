#include "kernel.h"

#define __wmemcpy(dst,src,count) \
{ \
int d0,d1,d2; \
__asm__ __volatile__("cld;rep;movsw":"=&S"(d0),"=&D"(d1),"=&c"(d2):"0"(src),"1"(dst),"2"(count):"memory"); \
}

#define __wmemset(dst,val,count) \
{ \
int d0,d1; \
__asm__ __volatile__("cld;rep;stosw":"=&D"(d0),"=&c"(d1):"0"(dst),"a"(val),"1"(count):"memory"); \
}


#define BasicConsoleAddr 0xC00B8000

void BIOSConMoveCsr() {
	uint16 temp;
	temp = (Console->y_pos * Console->width + Console->x_pos);
	uint16 *where = Console->buffer + temp;
	*where = (0x20 | Console->attrib << 8);
	outb(Console->io_adr + 0, 14);
	outb(Console->io_adr + 1, temp >> 8);
	outb(Console->io_adr + 0, 15);
	outb(Console->io_adr + 1, temp);
}

Console_t BIOSConsole = { (uint16*) BasicConsoleAddr, 0x9A0, 80, 25, 0, 0,
		2, 0x03D4, &BIOSConMoveCsr, };

Console_p Console = &BIOSConsole;

void ScrollConsole(Console_p con) {
	uint32 blank, width, pos;
	width = con->width;
	uint16 *fb_adr = con->buffer;
	blank = 0x20 | ((unsigned) con->attrib << 8);
	pos = (con->height - 1) * width;

	__wmemcpy(fb_adr, (fb_adr + width), pos);
	__wmemset(fb_adr + pos, blank, width);

	con->y_pos--;
}

void Console_SetTextColor(uint8 col) {
	Console->attrib = (Console->attrib & 0xF0) | col;
}

void Console_SetBackColor(uint8 col) {
	Console->attrib = (Console->attrib & 0xF) | (col << 4);
}

/*void SelectConsole(Console_p con)
 {
 Console = con;
 uint16 i = (uint32)Console->buffer - BasicConsoleAddr;
 outb(Console->io_adr + 0, 12);
 outb(Console->io_adr + 1, i >> 8);
 outb(Console->io_adr + 0, 13);
 outb(Console->io_adr + 1, i);
 move_csr();
 }*/

void Console_Goto(uint32 x, uint32 y) {
	if (!(x & (1 << 31)))Console->x_pos = x;
	if (!(y & (1 << 31)))Console->y_pos = y;
	if (Console->MoveCsr)Console->MoveCsr();
}

void Console_Clear() {
	unsigned att;
	att = (((unsigned) Console->attrib << 8) | ' ') & 0xffff;
	Console->x_pos = Console->y_pos = 0;
	__wmemset(Console->buffer, att,
			Console->height * Console->width);
	if (Console->MoveCsr)
		Console->MoveCsr();
}

static void printchar(uint8 c) {
	switch (c) {
	case 0x08: { //backspace
		if (Console->x_pos > 0)	Console->x_pos--;
		uint16 *where = Console->buffer	+ (Console->y_pos * Console->width + Console->x_pos);
		uint16 att = Console->attrib << 8;
		*where = (' ' | att);
		break;
	}
	case '\t':
		Console->x_pos = (Console->x_pos + 8) & ~(8 - 1);
		break;
	case '\r':
		Console->x_pos = 0;
		break;
	case '\n':
		Console->x_pos = 0;
		Console->y_pos++;
		if(Console->y_pos == Console->height - 1){
//			Breakpoint();
			ScrollConsole(Console);
		}
		break;
	default: {
		uint16 *where = Console->buffer + (Console->y_pos * Console->width + Console->x_pos);
		uint16 att = Console->attrib << 8;
		*where = (c | att);
		Console->x_pos++;
	}
		// no break
	}
	if (Console->x_pos > Console->width) {
		Console->x_pos = 0;
		Console->y_pos++;
	}
}

void putch(char c) {
	printchar(c);
	if (Console->MoveCsr)Console->MoveCsr();
}

void puts(char *s)
{
	while(*s){
		printchar(*s++);
	}
	if (Console->MoveCsr)Console->MoveCsr();
}

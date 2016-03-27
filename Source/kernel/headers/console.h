#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel.h"

typedef void(*ConsoleMoveCursor_p)();

typedef struct {
	uint16 *buffer;
	uint32 buf_size;
	uint16 width;
	uint16 height;
	uint16 x_pos;
	uint16 y_pos;
	uint16 attrib;
	uint16 io_adr;
	ConsoleMoveCursor_p MoveCsr;
} Console_t, *Console_p;

extern Console_p CurrentConsole;
extern Console_p Console;

void ScrollConsole(Console_p con);

void Console_Goto(uint32 x, uint32 y);
void Console_Clear();
void Console_SetTextColor(uint8 col);
void Console_SetBackColor(uint8 col);

void putch(char c);
void puts(char *s);
//void puts

#endif

;Supernova loader
;32bit part

bits 32
section .text

%macro b_debug 0
	pusha
	mov dx, 0x8A00 
	mov ax, dx
	out dx, ax	
	mov ax, 0x8aE0
	mov dx, 0x8a00
	out dx, ax	
	popa
%endmacro

global Loader32
Loader32:
	mov ax, 0x10      ; entry #2 in GDT
	mov ds, ax        ; ds = 10h
	mov es, ax        ; es = 10h
	mov fs, ax        ; fs = 10h
	mov gs, ax        ; gs = 10h
	mov ss, ax        ; ss = 10h  
	
extern Drive_Number
	mov dl, [Drive_Number]	; Select which motor to stop
	mov edx, 0x3f2; Select Stop Floppy Motor function:
	mov al, 0x0c	
	out dx, al   ; Stop floppy motor:  
	
extern LoaderMain
	push jmp_addr	
	call LoaderMain
	add esp, 0x04
	
	push msg_LoaderDone
extern puts
	call puts	
	add esp, 0x04

	xor ecx, ecx
extern Con_X_Pos
	mov cl, [Con_X_Pos]
extern Con_Y_Pos
	mov ch, [Con_Y_Pos]	
	
	lgdt [trickgdt]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

;far jump opcodes
			db 0xEA
jmp_addr:	dd 0xC0010000
			dw 0x0008
;above 3 lines are equivalent to following line
;	jmp long 0x08:0xC0010000
	
trickgdt:
	dw gdt_end - gdt - 1 ; size of the GDT
	dd gdt ; linear address of GDT
gdt:
	dd 0, 0							; null gate
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40	; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40	; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
gdt_end:	

section .data

msg_LoaderDone: db 'Executing Supernova...',0x0A,0

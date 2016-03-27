bits 32
align 4

%macro debug 0
	pusha
	mov dx, 0x8A00 
	mov ax, dx
	out dx, ax	
	mov ax, 0x8aE0
	mov dx, 0x8a00
	out dx, ax	
	popa
%endmacro

global Start
Start:
    cli
	mov esp, 0xC0007000
	push ecx
extern Kernel_Init	
    call Kernel_Init		;init kernel
	add esp, 4
	sti						;run multitasking
	jmp $					;wait for int0
	
CPU_Reset:		;function came from http://wiki.osdev.org/PS2_Keyboard
	;Wait for a empty Input Buffer
	wait1:
	in   al, 0x64
	test al, 00000010b
	jne  wait1

	;Send 0xFE to the keyboard controller.
	mov  al, 0xFE
	out  0x64, al

global SafeLaunch
SafeLaunch:
	;debug
	pop eax
	call eax
	mov ebx, eax
	xor eax, eax
	int 0x80
	jmp $
	
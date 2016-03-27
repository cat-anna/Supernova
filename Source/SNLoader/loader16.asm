;Supernova loader
;16bit part

%define EC_CPUID 5

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

STRUC   LoaderFile
	.Name:			resb 20
	.Attributes:	resb 4		;there is 3 bytes of free space (because of 4bytes align)
	.Date:			resd 1
	.Size:			resd 1
	.Ptr			resd 1
	.This_Size		resb 1
ENDSTRUC

%define BS_Data			0x7c00
%define BUFFER_SEG		0x0800

bits 16
section .bits16

global Loader16
Loader16:
;	b_debug
	mov ax, 0x0920
	mov bx, 3
	xor cx, cx
	not cx
	int 0x10		;Clear the screen
	mov si, msg_welecome
	call puts16	
	mov ax, 0x0100			;prepare LoaderFromData segment
	mov gs, ax
;A20 unlock
	mov ax, 0x2401
	int 0x15
	jnc .A20Ready
	
	call Enable_A20
	jmp .A20Ready
	
	mov ax, 3
	mov bx, 1
	call boot_fail
.A20Ready:
;Test CPUID
	call Check_CPUID
	test eax, eax
	jnz .CPUIDReady
	mov ax, 3
	mov bx, EC_CPUID
.CPUIDReady:
;get the memory map
	mov ax,gs
	mov es,ax
	mov di, 0x170
	call GetMemoryMap	
	xor eax, eax
	mov ax, bp
	mov [gs:0x16C], eax
	
;	b_debug
	xor ax, ax
	mov di, 0x170
.FindFreeMem:
	mov eax, [gs:di] 
	test eax, eax
	jnz .FindFreeMemNext
	mov ebx, [gs:di + 4]
	jz .FreeMemFound
.FindFreeMemNext:
	add di, 24
	dec bp
	jnz .FindFreeMem
	mov ax, 3
	call boot_fail
.FreeMemFound:
	mov ebx, [gs:di + 8]
	shr ebx, 4
	test ebx, 0xFFFF0000
	jz .NoMemError
	mov ax, 3
	mov bx, 2
	call boot_fail
.NoMemError:
	push bx 				;push the free mem end

;	b_debug
;	extern FAT_init
;	call FAT_init
	
	call Is_SFS_Check
	jc Load_Supernova
	call Is_FAT_Check
	jc Load_Supernova
	
	mov ax, 1
	jmp boot_fail
	 
Load_Supernova:
	mov [gs:0x260], eax
	mov si, msg_fs_detected
	call puts16
	mov si, [FS_Name]
	call puts16
	mov si, msg_EOL
	call puts16
	mov si, msg_loading
	call puts16	

	;b_debug
	
	pop bx					;get the free mem end
	mov es, bx
	xor bx, bx	
	mov fs, bx
	xor ecx, ecx			;reset file counter
	mov bp, 0x0004
	mov si, FilesTable 
	
.LoadFile:	
	push cx
	push gs
	push bp
	push si
	mov cx, 4
	mov al, 0x20
	call puts16_space		;put small tabulator
	call puts16				;put file name
;	b_debug
	
	call [FS_FindFile]
	jnc .File_Found
	mov ax, 0x8002
	call boot_fail
	mov dx, 0x0001
	jmp .SelectNextFile
.File_Found:
	mov [gs:bp + LoaderFile.Size], eax
	mov [gs:bp + LoaderFile.Date], ebx
	mov [gs:bp + LoaderFile.Attributes], ecx
;	b_debug
;calculate address to load
	shr eax, 4
	xor ebx, ebx
	mov bx, es
	sub bx, ax
	and bx, 0xFF00
;check whether is still space for kernel ~100kb 	
	cmp bx, 0x2000		
	ja .StillHasMem
	mov ax, 4
	call boot_fail
.StillHasMem:
	mov es, bx
;calculate 32bit address
	shl ebx, 4
	mov [gs:bp + LoaderFile.Ptr], ebx
	xor ebx, ebx
	push es
	call [FS_LoadFile]
	pop es
	jnc .Load_File_OK
	mov ax, 0x8002
	call boot_fail
	jmp .SelectNextFile
	
.Load_File_OK:
	mov si, msg_loadfile_ok
	call puts16
	xor dx, dx
	
.SelectNextFile:
	pop si
	pop bp
	pop gs
	;b_debug
	mov di, bp
	call strcpy 
	call strlen
	add si, ax
	inc si
	pop cx
	test dx, dx
	jnz .FileNotLoaded
	inc cx	
	add bp, LoaderFile.This_Size
.FileNotLoaded:
	mov al, [si]	
	cmp al, 0xFF	
	jb .LoadFile
	
	mov [gs:0], ecx
	mov si, msg_setenv
	call puts16
;	b_debug
;Get cursor pos
	mov ah, 0x03
	xor bx, bx
	int 0x10
	mov [Con_X_Pos], dl
	mov [Con_Y_Pos], dh
;enter protected mode
	cli
	lgdt [gdt_desc]
	mov   eax,cr0
	or    al,1
	mov   cr0,eax
extern Loader32
	jmp dword 0x08:Loader32	
	  
%include "SFS.asm"
%include "FAT.asm"
%include "ReadSector.asm"
%include "IA-32.asm"
%include "String16.asm"

;current FS functions
FS_Reset: 			dw 0
FS_FindFile			dw 0
FS_LoadFile:		dw 0
FS_Name:			dw 0 

sectors_per_track:	dw 0
number_of_heads:	dw 0
bytes_per_sector:	dw 0
  
boot_fail:
	;b_debug
	mov cx, ax
	cmp ax, 3			;requirements check
	jne .NoErrorCode
	add bl, '0'
	mov [Error_Code], bl
.NoErrorCode:
	mov bl, 0x04
	and ax, 0x00FF
	dec ax
	shl ax, 1
	mov di, msg_FailTable
	add di, ax
	mov si, [ds:di]
	test si, si
	jz .WrongMsgCode
	call puts16_color
	test cx, 0x8000
	jnz .DoNotHang
.WrongMsgCode:
	mov si, msg_fail
	call puts16_color
	cmp cx, 3
	jne .Hang
	mov si, msg_Code
	call puts16_color
.Hang:
	jmp $
.DoNotHang:
	ret
  
msg_welecome: 		db 'Welecome to Supernova Loader'
msg_EOL:			db 0x0D,0x0A,0

msg_fs_detected:	db 'Detected file system: ',0

msg_loading:		db 'Loading files...',0x0D,0x0A,0
msg_setenv:			db 'Setting up 32bit enviroment...',0x0D,0x0A,0

msg_loadfile_ok:	db '  [OK]',0x0D,0x0A,0

msg_fs_check_error: db 'Could not recognize file system',0x0D,0x0A,0
msg_loadfile_error:	db '  [ERROR!]',0x0D,0x0A,0
msg_Requirements:	db 'This computer does not meet the requirements',0x0D,0x0A,0
msg_Unknown_Error:	db 'Unknown Error!',0x0D,0x0A
msg_NotEnoughMemory:db 'NotEnoughMemory!',0x0D,0x0A
msg_fail:			db 'System halted',0x0D,0x0A,0

msg_Code:			db 'Error code: '
Error_Code:			db '0',0x0D,0x0A,0

msg_FailTable:
	dw msg_fs_check_error
	dw msg_loadfile_error
	dw msg_Requirements
	dw msg_NotEnoughMemory
	dw 0
	dw 0
	dw 0 

global FilesTable
FilesTable:       	db 'Supernova.elf',0
					db 'Supernova.lib',0
					db 'SYS.drv',0
					db 'FAT.drv',0
					db 'FDD.drv',0
					db 0xFF;0xFF - means end of table

gdt:                    ; Address for the GDT
	dd 0, 0       		; Null Segment
	dw 0xFFFF, 0		; Code segment, read/execute, nonconforming	
	db 0, 0x9A, 11001111b, 0		     
	dw 0xFFFF, 0		; Data segment, read/write, expand down
	db 0, 0x92, 11001111b, 0
gdt_end:                ; Used to calculate the size of the GDT
gdt_desc:                       ; The GDT descriptor
    dw gdt_end - gdt - 1    ; Limit (size)
    dd gdt                  ; Address of the GDT   
					
section .data
global Drive_Number
Drive_Number		db 0

global Con_X_Pos
Con_X_Pos			db 0
global Con_Y_Pos
Con_Y_Pos			db 0

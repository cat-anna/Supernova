	org 0x7c00
	bits 16
	
%define Buffer_Seg			0x0700
	   
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

	jmp short main
	nop
;BPB - Bios Parameter Block
BPB_OEM:				Times 8 db 0x20
BPB_BytesPerSector:		dw 512
BPB_SectorsPerCluster:	db 1
BPB_ReservedSectors:	dw 1
BPB_NumberOfFATs:		db 2
BPB_RootEntires:		dw 224
BPB_TotalSectors:		dw 2880
BPB_Descriptor:			db 0xF0
BPB_SectorsPerFAT:		dw 9
BPB_SectorsPerTrack:	dw 18
BPB_NumberOfHeads:		dw 2
BPB_HiddenSectors:		dd 0
BPB_TotalSecotrs32:		dd 0
;Extended
BPB_DriveNumber:		db 0
BPB_NTFlags:			db 0
BPB_Signature:			db 0x29
BPB_VolumeID:			dd 0xAABBCCDD
BPB_LabelString:		times 11 db 0x20
BPB_SystemString:		times 8 db 0x20

%define Buffer_Seg			0x1000

%include "FAT.inc"

main:
	xor eax, eax 
	mov ds,	ax
	mov es,	ax
	mov ss, ax
;	mov sp, 0x1000
	mov [BPB_DriveNumber], dl
	int 13h						; reset drive
	 
	mov ax,	3					; Set text-mode 3.
	int 10h						; text-mode 3 set. the screen is cleared	

;Lookup Root Direcotry
	xor eax, eax
	mov al, [BPB_NumberOfFATs]
	mov dx, [BPB_SectorsPerFAT]
	mul dx
	add ax, [BPB_ReservedSectors]
	push ax
	xor di, di
	mov si, LoaderFile
	mov bx, Buffer_Seg
	mov es, bx
	xor dx, dx
	
.NextDirectorySector:
	mov cx, 1
	xor bx, bx
	push ax
	push di
	add ax, di
	call ReadSector
	jc boot_fail
	mov cx, 0x10
.NextFilePtr:
	mov al, [es:bx]
	test al, al
	jz .NoFile
	cmp al, 0xE5
	je .NoFile
	mov al, [es:bx + 0xB]
	cmp al, 0x0F
	je .NoFile
	 
	mov di, bx
	call strcmp
	jc FileFound
	 
.NoFile:
	add bx, 0x20
	dec cx
	jnz .NextFilePtr

	pop di
	pop ax
	inc di
	mov bx, di
	cmp di, [BPB_RootEntires]
	jb .NextDirectorySector
	jmp boot_fail

boot_fail:
	mov si,msg_boot_fail
	call print
	mov ax,0
	int 0x16
	int 0x19	 
	jmp $
  
FileFound:
	add sp, 4 
	pop di
	mov ax, [BPB_RootEntires]
	shr ax, 4
	add di, ax
	mov si, [es:bx + FAT_FileEntry.LowCluster]
	
	mov bx, Buffer_Seg
	mov es, bx
	mov gs, bx
	xor bx, bx
	
	xor dx, dx
	mov ax, 1
	mov cx, [BPB_SectorsPerFAT]
	call ReadSector
	jc boot_fail
	
	mov bx, 0x0200
	mov es, bx
	xor bx, bx

.NextCluster:
	cmp si, 0x0FF8
	je .NoMoreClusters
	call ReadCluster;
	jmp .NextCluster
.NoMoreClusters:
	;debug	
	mov sp, 0x1000
	jmp 0x0000:0x2000
	 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a FAT12 cluster      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Inout:  ES:BX -> buffer    ;;
;;         SI = cluster no    ;;
;;		   GS:BP -> Loaded fat;;
;; Output: SI = next cluster  ;;
;;         ES:BX -> next addr ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ReadCluster:
        mov     bp, sp
 
		mov ax, si
        add ax, di
		sub ax, 2
        xor     ch, ch
        mov     cl, [BPB_SectorsPerCluster]
                ; cx = sector count
        mul     cx
	xor dx,dx
    ;    add     ax, [ss:bp+1*2]
     ;   adc     dx, [ss:bp+2*2]
                ; dx:ax = LBA

        call    ReadSector

        mov     ax, [BPB_BytesPerSector]
        shr     ax, 4                   ; ax = paragraphs per sector
        mul     cx                      ; ax = paragraphs read

        mov     cx, es
        add     cx, ax
        mov     es, cx                  ; es:bx updated
	
        mov ax, 3
        mul si
		mov cx, ax
        shr ax, 1

        mov si, ax                  ; si = cluster * 3 / 2
        mov ax, [gs:si]             ; si = next cluster
		
		test cx, 1
		jz .NoClusterShift
		shr ax, 4
.NoClusterShift:
		and ax, 0x0FFF
		mov si, ax
ReadClusterDone:
        ret

%define sectors_per_track 		BPB_SectorsPerTrack
%define number_of_heads			BPB_NumberOfHeads
%define Drive_Number			BPB_DriveNumber
%define bytes_per_sector		BPB_BytesPerSector

%include "ReadSector.asm"

print:   			; ds:si - text
	push ax
	push bx
	push cx
	push si
	mov ah,0x0e
	mov cx,1
	xor bx,bx
print_1:
	mov al,[si]
	test al, al
	jz print_end
	inc si
	int 0x10
	jmp print_1
print_end:
	pop si
	pop cx
	pop bx
	pop ax
	ret
	
;INPUT:		ds:si - text1
;			es:di - text2 
;OUTPUT:	if texts are equal carry flag is set 
;this function changes only carry flag
;this function are not case sensitive
strcmp:
	push si
	push di
	push ax
.strcmp_loop:
	mov al, [ds:si]
	mov ah, [es:di]
	test ax, ax
	jz .equal
	cmp ah, al
	jne .not_equal
	inc di
	inc si
	jmp .strcmp_loop
.equal:
	stc
	jmp .end
.not_equal:
	clc
.end:
	pop ax
	pop di
	pop si
	ret
	
LoaderFile       	db 'SNLOADERBIN',0
msg_boot_fail   	db 'ERROR!',0x0A,0x0D,0
;load_kernel 		db 'Booting Supernova...',10,13,0

TIMES 510-($-$$) DB 0
DW 0AA55H

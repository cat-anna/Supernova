; nasm -o boot.bin -f bin boot.asm

%define FilesPerSector 8 		;ilosc struktur plikow w sektorze
%define FileNameLength 18 		;maksymalna dlugosc nazwy pliku
%define FileStructureSize 64 	;rozmiar struktury pliku
%define FileSegmentsOffset 29	;offset pozycji segmentu 1
%define FileSegments 5

%define Buffer_Seg			0x0700

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

	org 0x7c00
	bits 16
	
	jmp main
	nop	
		
;Basic Info Structure (bis)
bis_name                    db 'SFS',0,0
bis_bytes_per_sector        dw 512
bis_sectors_per_cluster     db 1
bis_sectors_per_track       dw 18
bis_number_of_heads         dw 2
bis_partition_id            dd 0xbaabdccd
bis_total_sectors           dd 2880
bis_file_struct_pos			dd 2
bis_file_struct_size		dd 1
bis_drive_number            db 0
bis_version					dw 0xaaaa
bis_media_type              db 0x0f
;kilka z nich nie wiem po co umiescilem, moze kiedys sie przydadza
;=======
	
main: ; poczatek bootloadera
;	b_debug
	xor ax,	ax 
	mov ds,	ax
	mov es,	ax
	mov ss, ax
;	mov sp, 0x1000
	mov [bis_drive_number],dl
	int 13h						; reset drive	
	
	mov ax,	3					; Set text-mode 3.
	int 10h						; text-mode 3 set. the screen is cleared	

;	b_debug
	mov si, LoaderFile
	jmp find_file
;	jmp $
	
boot_fail:
	mov si,msg_boot_fail
	call print
	mov ax,0
	int 0x16
	int 0x19	 
	jmp $
		
find_file:
	mov bp, sp
	mov ax,[bis_file_struct_pos];do ax idze pozycja struktur plikow
find_file_1:
	mov cx,[bis_file_struct_size];do cx ilosc sektorów z strukturami
	push cx						;ilosc struktur na stos
	push ax						;pozycje struktur na dysku odkladamy na stos
	xor dx, dx
	mov cx, 1
	;b_debug
	mov bx, Buffer_Seg
	mov es, bx
	xor bx, bx
	call ReadSector				;ES:BX - segment i offset struktur do porownania
	mov cx, FilesPerSector 		;cx - ilosc plikow w strukturze	
.find_file_2:
	mov di,bx
	call strcmpIC				;ds:si - text1    es:di - text2 
	jc file_found 				;loader zaleziony
	add bx, FileStructureSize 	;"idziemy" do kolejnego pliku
	dec cx
	jnz .find_file_2 			;sprawdzamy kolejny plik	
	pop ax 						;podnosimy pozycje struktur na dysku
	inc ax 						;wskazyjemy na nastepna
	pop cx 						;podnosimy ilosc sektorów ze strkturami
	dec cx
	test cx, cx
	jnz find_file_1				; wracamy do pkt wyjœcia
	jmp boot_fail				; ops! plik nie znaleziony	
	
file_found:
	mov dx, es
	mov gs, dx
	mov si, bx
	add si, FileSegmentsOffset
	mov dx, FileSegments
	push dx
	push si	
	
;	mov si,load_kernel
;	call print		
read_next_segment:
	pop si
	pop dx
	test dx, dx
	jz read_done			;kernel zosta³ przeczytany
	mov eax, [gs:si]			;pobieramy pozycjê segmentu 
	test eax, eax
	jz read_done
	add si, 4		
	mov ecx, [gs:si]			;pobieramy rozmiar segmentu
	and ecx, 0x00FFFFFF		;odrzucamy najstarszy bajt (size == 24bit)
	add si, 3
	dec dx	
	push dx	
	push si
	
	mov dx, 0x0200
	mov es, dx
	xor dx, dx
	xor bx, bx
	call ReadSector
	jc boot_fail
	jmp read_next_segment
	
read_done:
	mov sp, 0x1000
	jmp 0x0000:0x2000

%define sectors_per_track 		bis_sectors_per_track
%define number_of_heads			bis_number_of_heads
%define Drive_Number			bis_drive_number
%define bytes_per_sector		bis_bytes_per_sector

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
strcmpIC:
	push si
	push di
	push ax
.strcmpIC_loop:
	mov al, [ds:si]
	mov ah, [es:di]
	
	test ax, ax
	jz .equal
	and ax, ~0x2020
	cmp ah, al
	jne .not_equal
	inc di
	inc si
	jmp .strcmpIC_loop
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
	
LoaderFile       	db 'loader.bin',0
msg_boot_fail   	db 'ERROR!',0x0A,0x0D,0
;load_kernel 		db 'Booting Supernova...',10,13,0

TIMES 510-($-$$) DB 0
DW 0AA55H

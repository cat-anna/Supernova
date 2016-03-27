;Supernova Loader
;SFS 16bit support
;this file was designed to be included

%include "SFS.inc"

;is boot drive SFS?
;if boot drive is SFS carry flag is set 
Is_SFS_Check:
	mov cx, 5	 				;cx - ilosc bajtow do porownania
	xor dx, dx
	mov es, dx
	mov ds, dx
	mov di, BS_Data + SFS_BIS.FSName
	mov si, SFS_sign
	repz cmpsb 					;repz cmpsb - porównuje DS:SI z ES:DI	
	jz IC_yes
	clc
	ret
IC_yes:
;fill global varibles needed by loader to work on SFS
	mov word [FS_Reset], SFS_Reset
	mov word [FS_FindFile], SFS_FindFile
	mov word [FS_LoadFile], SFS_LoadFile
	mov word [FS_Name], SFS_sign
	
	mov al, [BS_Data + SFS_BIS.DriveNumber]
	mov [Drive_Number], al
	mov ax, [BS_Data + SFS_BIS.SectorsPerTrack]
	mov [sectors_per_track], ax
	mov ax, [BS_Data + SFS_BIS.NumberOfHeads]
	mov [number_of_heads], ax
	mov ax, [BS_Data + SFS_BIS.BytesPerSector]
	mov [bytes_per_sector], ax
	
	call SFS_Reset
	stc
	ret

;Load file from SFS
;input:		di - pointer to file
;			es:bx - file buffer
;output:
;if function fails carry flag is set
;trashes all registers
SFS_LoadFile:
	push bp
	mov bp, sp

	xor esi, esi
	add di, SFS_file.Segment1
	mov edx, FileSegments	
		
.LF_NextSegment:
	test dx, dx
	jz .LF_success			;plik zosta³ przeczytany
	mov eax, [di]			;pobieramy pozycjê segmentu 
	test eax, eax
	jz .LF_success
	add di, 4		
	mov ecx, [di]			;pobieramy rozmiar segmentu
	and ecx, 0x00FFFFFF		;odrzucamy najstarszy bajt (size == 24bit)
	add di, 3
	dec dx

	call ReadSector
	jc .LF_fail
	shl cx, 5
	mov ax, es
	add ax, cx
	mov es, ax
	jmp .LF_NextSegment
.LF_fail:
	stc
	jmp .LF_return
.LF_success:
	clc
.LF_return:
	mov sp, bp
	pop bp
	ret
	
SFS_Reset:
	mov dword [SFS_loaded_file_struct], 0xFFFFFFFF
	ret
	
;find the file in POFS
;input: 	ds:si - file name
;output:	di - pointer to file strucure
;			eax - file size
;			ebx - file date
;			ecx - attributes && misc info
;if function fails carry flag is set
SFS_FindFile:
	push bp
	push es
	push ds
	mov bp, sp
	xor eax, eax
	mov ds, ax
	mov ax, BUFFER_SEG
	mov es, ax
	
	mov eax, [SFS_loaded_file_struct]
	test eax, eax
	jz .no_reset_file_struct
	xor eax, eax
.Next_struct:
	mov [SFS_loaded_file_struct], eax
	call SFS_LoadFileStruct
.no_reset_file_struct:
	xor di, di
	mov cx, FilesPerSector
.Compare_Next:
	call strcmpIC
	jc .Found
	dec cx
	jz .No_More
	add di, FileStructureSize
	jmp .Compare_Next
.No_More:
	inc eax
	mov [SFS_loaded_file_struct], eax
	cmp eax, [BS_Data + SFS_BIS.FileStructSize]
	jnb .Error
	jmp .Next_struct
.Found:
	mov eax, [di + SFS_file.Size] 
	mov ebx, [di + SFS_file.Date] 
	mov ecx, [di + SFS_file.Attributes] 
	and ecx, 0xFF
	clc
	jmp .end
.Error:	
	xor eax, eax
	stc
.end:
	mov sp, bp
	pop ds
	pop es
	pop bp
	ret

;Load file structure
;input:		eax - number of structure
;output:	none
;if function fails carry flag is set ?
SFS_LoadFileStruct:
	push eax
	push ecx
	push ebx
	push es
	add eax, [BS_Data + SFS_BIS.FileStructPos]
	mov cx, 1
	mov bx, BUFFER_SEG
	mov es, bx
	xor bx, bx
	call ReadSector
	pop es
	pop ebx
	pop ecx
	pop eax
	ret
	
SFS_loaded_file_struct			dd 0	
SFS_sign						db 'SFS',0,0

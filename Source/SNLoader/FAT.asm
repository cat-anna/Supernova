;Supernova Loader
;FAT 16bit support
;this file was designed to be included

%include "FAT.inc"

;find the file in FAT
;input: 	ds:si - file name
;output:	fs:di - pointer to file strucure
;			eax - file size
;			ebx - file date
;			ecx - attributes && misc info
;if function fails carry flag is set
FAT_FindFile:
	push es
	push dx
	mov di, FAT_NameBuffer
	call FAT_ConvertNameForFat
	mov si, di
	xor dx, dx

;Lookup loaded data
.FAT_FF_DoSearch:
	mov bx, BUFFER_SEG
	mov es, bx
	mov bx, [FAT_RootLoadOffset]
	mov cx, [BS_Data + FAT_BPB.RootEntryCount]
.FAT_FF_NextFilePtr:	
	mov al, [es:bx]
	test al, al
	jz .NoFile
	cmp al, 0xE5
	je .NoFile
	mov al, [es:bx + FAT_FileEntry.Attributes]
	cmp al, 0x0F
	je .NoFile
	mov di, bx
	call strcmpIC
	jc .FileFound
.NoFile:
	add bx, FAT_FileEntry.ThisSize
	dec cx
	jnz .FAT_FF_NextFilePtr
	jmp .NotFound
.FileFound:
	mov di, bx
	mov bx, es
	mov fs, bx
	mov eax, [fs:di + FAT_FileEntry.FileSize]
	xor ebx, ebx
	xor ecx, ecx
	mov cl, [fs:di + FAT_FileEntry.Attributes]
	clc
	jmp .Return 
.NotFound:
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor di, di
	stc
.Return:
	pop dx
	pop es
	ret
	
;Load file from FAT
;input:		fs:di - pointer to file
;			es:bx - file buffer
;output:	es:bx - file data
;if function fails carry flag is set
FAT12_LoadFile:
;	b_debug
	push ax
	push si
	push di
	push gs
	push bx
	push es
	
	mov si, [fs:di + FAT_FileEntry.LowCluster]
	
	mov ax, [FAT_FATLoadOffset]
	shr ax, 4
	add ax, BUFFER_SEG
	mov gs, ax	
	
	mov di, [FAT_DataArea]
.NextCluster:
	cmp si, 0x0FF8
	je .Succes
	test si, si
	jz .Failed
	call FAT12_ReadCluster
	jmp .NextCluster
.Failed:
	stc
	jmp .Return
.Succes:
	clc
.Return:	
	pop es
	pop bx
	pop gs
	pop di
	pop si
	pop ax
	ret

FAT12_Reset:
	push ax
	push cx
	push es
	push bx
	
	mov ax, [BS_Data + FAT_BPB.BytesPerSector]
	mul word [BS_Data + FAT_BPB.SectorsPerCluster]
	mov [FAT_ClusterSize], ax
	
	mov ax, [BS_Data + FAT_BPB.SectorsPerFAT]
	mul byte [BS_Data + FAT_BPB.NumberOfFATs]
	inc ax
	mov [FAT_RootDirStart], ax
	
	xor cx, cx
	mov cx, [BS_Data + FAT_BPB.RootEntryCount]
	shr cx, 4
	add ax, cx
	mov [FAT_DataArea], ax
	
	mov cx, [BS_Data + FAT_BPB.SectorsPerFAT]
	mov ax, 1
	mov bx, BUFFER_SEG
	mov es, bx
	xor bx, bx
	mov [FAT_FATLoadOffset], bx
	call ReadSector

	mov ax, [BS_Data + FAT_BPB.BytesPerSector]
	mul cx
	mov [FAT_RootLoadOffset], ax
	mov bx, ax
	mov ax, [FAT_RootDirStart]
	mov cx, [BS_Data + FAT_BPB.RootEntryCount]
	shr cx, 4
	call ReadSector
	
	pop bx
	pop es
	pop cx
	pop ax
	ret

FAT_ClusterSize:			dw 0
FAT_DataArea:				dw 0
FAT_RootDirStart:			dw 0

FAT_FATLoadOffset:			dw 0
FAT_RootLoadOffset:			dw 0

;Converts filename for FAT short format
;input: 	ds:si - long file name
;			ds:di - buffor address
;output:	ds:di - FAT short file name
FAT_ConvertNameForFat:			;funkcja przet³umaczona z c
	push si						
	push ax						
	push cx
	push di
	mov cx, 8					;cx - iloœæ znaków w nazwie pliku
.NameLoop:					;pêtla dla nazwy pliku, pierwsza pêtla for
	mov al, [ds:si]				;pobierz znak
	test al, al					;jeœli jest on zerem
	jz .NoSrcInc					;brak znaku wejœciowego
	cmp al, '.'					;i kropk¹ nie jest kropk¹
	je .NoSrcInc					;brak znaku wejœciowego
	inc si						;nastêpny znak
.NoSrcInc:					;sprawdzanie poprawnoœci znaku
	test al, al 				;jeœli znak jest zerem
	jz .Set0x20						;zmieniamy na spacjê
	cmp al, '.'					;lub je7st kropk¹
	je .Set0x20						;zmieniamy na spacjê
	jmp .NoSet0x20
.Set0x20:					;pobrany znak jest niepoprawny
	mov al, 0x20				;zmieniamy go na spacjê
	jmp .SetName				;znak jest poprawny
.NoSet0x20:					;pobrany znak jest poprawny
	cmp al, 0x20				;jeœli znak jest spacj¹
	je .NameLoop					;nie wpisujmy go do bufora
.SetName:					;wstawiamy znak do bufora
	cmp al, 0x20				;jeœli znak jest spacj¹
	je .SkipNameAnd					;nie zmieniamy na du¿a literkê
	and al, 0xDF				;zmiana na du¿¹ literkê
.SkipNameAnd:				;Spacji nie trzeba and'owaæ (0x20 & 0xDF = 0)
	mov [ds:di], al				;wpisujemy znak do bufora
	inc di						;nstêpny znak w buforze
	dec cx						;kolejny znak gotowy
	jnz .NameLoop					;jeœli pozosta³y nie przerobione znaki
;pierwszej koniec pêtli for
	
	mov al, [ds:si]				;pobierz znak dla którego pierwsza pêtla for skoñczy³a
	cmp al, '.'					;porównaj z kropk¹
	je .NoNameShift					;jeœli by³ kropk¹ nie ma zmiany na krótki format nazwy
	mov byte [FAT_NameBuffer + 6], '~'	;wpisz tyldê do przed ostatniego znaku
	mov byte [FAT_NameBuffer + 7], '1'	;wpisz jedynkê do ostatniego znaku
.SkipLongName:				;bêdziemy pomijaæ wszystkie znaki d³ugiej nazy
	inc si						;nastêpny znak
	mov al, [ds:si]				;pobierz obecnu znak
	cmp al, '.'					;jeœli nie jest kropk¹
	jne .SkipLongName				;powtórz pêtlê
.NoNameShift:				;doszliœmy do rozszerzenia
	inc si						;pomiñ kropkê
	mov cx, 3					;cx - iloœæ znaków w rozszerzeniu
	
.ExtLoop:					;pêtla dla rozszerzenia, druga pêtla for
	mov al, [ds:si]				;pobierz znak
	test al, al					;testuj czy zero
	jnz .MoreText					;jeœli nie, znak jest poprawny
	mov al, 0x20				;wpisz spacjê, gdy znak nie jest poprawny
.MoreText:					
	inc si						;nastêpny znak
	;jmp .SetExt					;skocz do 
;.SetExt:
	cmp al, 0x20				;prównaj znak ze spacj¹
	je .SkipExtAnd					;jeœli równe pomiñ and'owanie
	and al, 0xDF				;zmiana na du¿¹ literkê
.SkipExtAnd:					
	mov [ds:di], al				;wpisz znak do bufora
	inc di						;nastêpny znak
	dec cx						;kolejny znak gotowy
	jnz .ExtLoop					;jeœli s¹ nie gotowe powtórz pêtlê
	pop di
	pop cx
	pop ax
	pop si
	ret

;is boot drive SFS?
;if boot drive is SFS carry flag is set 
Is_FAT_Check:
;check fat jump
	cmp byte [BS_Data], 0xEB
	jne .FAT_IC_No
	cmp byte [BS_Data + 2], 0x90
	jne .FAT_IC_No

;check total sectors
	cmp byte [BS_Data + FAT12_EXT_BPB.BootSignature], 0x29
	jne .FAT_IC_No
	mov ax, [BS_Data + FAT_BPB.TotalSecotrs16]
	test ax, ax
	jz .FAT_IC_No
	cmp ax, 4085
	ja .FAT_IC_No
	jmp .FAT_IC_yes
.FAT_IC_No:
	clc
	ret
	
.FAT_IC_yes:
;fill global varibles needed work on FAT
	mov word [FS_Reset], FAT12_Reset
	mov word [FS_FindFile], FAT_FindFile
	mov word [FS_LoadFile], FAT12_LoadFile
	mov word [FS_Name], FAT12_sign
	
	mov al, [BS_Data + FAT12_EXT_BPB.DriveNumber]
	mov [Drive_Number], al
	mov ax, [BS_Data + FAT_BPB.SectorsPerTrack]
	mov [sectors_per_track], ax
	mov ax, [BS_Data + FAT_BPB.NumberOfHeads]
	mov [number_of_heads], ax
	mov ax, [BS_Data + FAT_BPB.BytesPerSector]
	mov [bytes_per_sector], ax
	
	call FAT12_Reset
	mov eax, [BS_Data + FAT12_EXT_BPB.VolumeId]
	stc
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a FAT12 cluster      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Inout:  ES:BX -> buffer    ;;
;;         SI = cluster no    ;;
;;		   GS:BP -> Loaded fat;;
;; Output: SI = next cluster  ;;
;;         ES:BX -> next addr ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FAT12_ReadCluster:
	mov ax, si
	add ax, di
	sub ax, 2
	xor ch, ch
	mov cl, [BS_Data + FAT_BPB.SectorsPerCluster]
		; cx = sector count
	mul cx
	xor dx,dx
	call ReadSector
	mov ax, [bytes_per_sector]
	shr ax, 4                   ; ax = paragraphs per sector
	mul cx                      ; ax = paragraphs read

	mov cx, es
	add cx, ax
	mov es, cx                  ; es:bx updated

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
		
FAT_NameBuffer:		times 12 db 0
FAT12_sign: 		db 'FAT12',0

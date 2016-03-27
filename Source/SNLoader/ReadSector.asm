;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a sector using BIOS Int 13h fn 2 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Input:  DX:AX = LBA                    ;;
;;         CX    = sector count           ;;
;;         ES:BX -> buffer address        ;;
;; Output: CF = 1 if error                ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;/	
ReadSector:
	pusha
	xor dx, dx	
ReadSectorNext:
	mov     di, 5                   ; attempts to read
ReadSectorRetry:
	pusha
	div     word [sectors_per_track]
			; ax = LBA / SPT
			; dx = LBA % SPT         = sector - 1
	mov     cx, dx
	inc     cx
			; cx = sector no.
	xor     dx, dx
	div     word [number_of_heads]
			; ax = (LBA / SPT) / HPC = cylinder
			; dx = (LBA / SPT) % HPC = head
	mov     ch, al
			; ch = LSB 0...7 of cylinder no.
	shl     ah, 6
	or      cl, ah
			; cl = MSB 8...9 of cylinder no. + sector no.
	mov     dh, dl
			; dh = head no.
	mov     dl, [Drive_Number]
			; dl = drive no.
	mov     ax, 201h
									; al = sector count
									; ah = 2 = read function no.
	int     13h                     ; read sectors
	jnc     ReadSectorDone          ; CF = 0 if no error
	xor     ah, ah                  ; ah = 0 = reset function
	int     13h                     ; reset drive
	popa
	dec     di
	jnz     ReadSectorRetry         ; extra attempt
	jmp     boot_fail
ReadSectorDone:
	popa
	dec     cx
	jz      ReadSectorDone2         ; last sector
	add     bx, [bytes_per_sector] ; adjust offset for next sector
	jnc RS_SkipAddSeg
	mov bx, es
	add bx, 0x1000
	mov es, bx
	xor bx, bx
RS_SkipAddSeg:	
	add     ax, 1
	adc     dx, 0                   ; adjust LBA for next sector
	jmp     short ReadSectorNext
ReadSectorDone2:
	popa
	ret

;Supernova Loader
;16bit strings support

;INPUT:
;	ds:si - src
;	gs:di - dest
;function does not care about overflow
strcpy:
	push si
	push di
	push ax
.strcpy_loop:
	mov al, [ds:si]
	mov [gs:di], al
	inc si
	inc di
	test al, al
	jnz .strcpy_loop 
	pop ax
	pop di
	pop si
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

;INPUT:		ds:si - text 
;OUTPUT:	ax - length
;this function changes only ax
strlen:				
	push si
	push bx
	xor ax, ax
.strlen_loop:
	mov bl, [ds:si]
	test bl, bl
	jz .strlen_end
	inc ax
	inc si
	jmp .strlen_loop
.strlen_end:	
	pop bx
	pop si
	ret

;Prints chars
;INPUT:	al - char   cx - amount
puts16_space:
	push cx
	push ax
	push bx
	mov ah,0x0e
.loop:
	int 0x10
	dec cx;
	jnz .loop
	pop bx
	pop ax
	pop cx
	ret

puts16:   			; ds:si - text
	push ax
	push bx
	push cx
puts16_pushed:
	push si
	mov ah,0x0e
	mov cx,1
	xor bx,bx
.loop:
	mov al,[si]
	test al, al
	jz .end
	inc si
	int 0x10
	jmp .loop
.end:
	pop si
	pop cx
	pop bx
	pop ax
	ret
	
puts16_color:   			; ds:si - text, bl = color
	push ax
	push bx
	push cx
	call strlen
	mov cx, ax
	mov ax, 0x0920
	xor bh, bh
	int 0x10
	jmp puts16_pushed
	
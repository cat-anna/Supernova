;Supernova loader 16bit part

;Folowig codes came from osdev.org

; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
GetMemoryMap:
	xor ebx, ebx		; ebx must be 0 to start
	xor bp, bp		; keep an entry count in bp
	mov edx, 0x0534D4150	; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc short .failed	; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150	; Some BIOSes apparently trash this register?
	cmp eax, edx		; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc short .e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
.jmpin:
	jcxz .skipent		; skip any 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]	; get lower dword of memory region length
	or ecx, [es:di + 12]	; "or" it with upper dword to test for zero
	jz .skipent		; if length qword is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
;	mov [0x1000], bp	; store the entry count
	clc			; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.failed:
	stc			; "function unsupported" error exit
	ret
	
Check_A20:
    pushf
    push ds
    push es
    push di
    push si
    cli
    xor ax, ax ; ax = 0
    mov es, ax
    not ax ; ax = 0xFFFF
    mov ds, ax
    mov di, 0x0500
    mov si, 0x0510
    mov al, byte [es:di]
    push ax
    mov al, byte [ds:si]
    push ax
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
    cmp byte [es:di], 0xFF
    pop ax
    mov byte [ds:si], al
    pop ax
    mov byte [es:di], al
    mov ax, 0
    je check_a20__exit
    mov ax, 1
check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
    ret
	
Enable_A20:
        cli
        call    a20wait
        mov     al,0xAD
        out     0x64,al
        call    a20wait
        mov     al,0xD0
        out     0x64,al
        call    a20wait2
        in      al,0x60
        push    eax
        call    a20wait
        mov     al,0xD1
        out     0x64,al
        call    a20wait
        pop     eax
        or      al,2
        out     0x60,al
        call    a20wait
        mov     al,0xAE
        out     0x64,al
        call    a20wait
        sti
        ret
a20wait:
        in      al,0x64
        test    al,2
        jnz     a20wait
        ret
a20wait2:
        in      al,0x64
        test    al,1
        jz      a20wait2
        ret
		
		
; returns 1 if CPUID is supported, 0 otherwise (ZF is also set accordingly)
Check_CPUID:
	pushfd ; get
	pop eax
	mov ecx, eax ; save 
	xor eax, 0x200000 ; flip
	push eax ; set
	popfd
	pushfd ; and test
	pop eax
	xor eax, ecx ; mask changed bits
	shr eax, 21 ; move bit 21 to bit 0
	and eax, 1 ; and mask others
	push ecx
	popfd ; restore original flags
	ret

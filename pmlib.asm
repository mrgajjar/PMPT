GLOBAL  _READ_MSW, _READ_CR0, _WRITE_CR0, _LGDT, _LIDT, _LLDT, _UPDATE_CS
GLOBAL  _LTR, _STR, _CLTS, _LOAD_FS, _LOAD_GS, _JUMP_TO_TSS

SEGMENT _TEXT PUBLIC CLASS=CODE USE16

_READ_MSW:
	smsw    ax
	retn

_READ_CR0:
	mov     eax, cr0        ; read CR0 to eax
	mov     edx, eax
	shr     edx, 16         ; dx:ax = CR0 (return value)
	retn

_WRITE_CR0:
	push    bp
	mov     bp, sp
	mov     eax, [ss:bp+4]  ; eax = 32-bit parameter
	mov     cr0, eax
	pop     bp
	retn

_LGDT:
	push    bp
	mov     bp, sp
	push    bx
	mov     bx, [ss:bp+4]   ; ds:bx = pointer to GDTR structure
	lgdt    [ds:bx]         ; load GDTR
	pop     bx
	pop     bp
	retn

_LIDT:
	push    bp
	mov     bp, sp
	push    bx
	mov     bx, [ss:bp+4]   ; ds:bx = pointer to IDTR structure
	lidt    [ds:bx]         ; load IDTR
	pop     bx
	pop     bp
	retn

_LLDT:
	push    bp
	mov     bp, sp
	mov     ax, [ss:bp+4]
	lldt    ax              ; load LDTR
	pop     bp
	retn

_UPDATE_CS:
	push    bp
	mov     bp, sp
	mov     ax, [ss:bp+4]   ; ax = new cs
	push    ax              ; push segment
	push    word .1         ; push offset
	retf                    ; we have a new cs now
.1:
	pop     bp
	retn

_LTR:
	push    bp
	mov     bp, sp
	ltr     word [ss:bp+4]
	pop     bp
	retn

_STR:
	str     ax
	retn

_CLTS:
	clts
	retn

_LOAD_FS:
	push    bp
	mov     bp, sp
	mov     fs, [ss:bp+4]
	pop     bp
	retn

_LOAD_GS:
	push    bp
	mov     bp, sp
	mov     gs, [ss:bp+4]
	pop     bp
	retn

_JUMP_TO_TSS:
	push    bp
	mov     bp, sp
	jmp far [ss:bp+2]
	pop     bp
	retn

SEGMENT _DATA PUBLIC CLASS=DATA

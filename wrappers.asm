GLOBAL  _ISR_00_WRAPPER, _ISR_01_WRAPPER, _ISR_02_WRAPPER, _ISR_03_WRAPPER
GLOBAL  _ISR_04_WRAPPER, _ISR_05_WRAPPER, _ISR_06_WRAPPER, _ISR_07_WRAPPER
GLOBAL  _ISR_08_WRAPPER, _ISR_09_WRAPPER, _ISR_0A_WRAPPER, _ISR_0B_WRAPPER
GLOBAL  _ISR_0C_WRAPPER, _ISR_0D_WRAPPER, _ISR_0E_WRAPPER, _ISR_0F_WRAPPER
GLOBAL  _ISR_10_WRAPPER, _ISR_11_WRAPPER, _ISR_12_WRAPPER, _ISR_13_WRAPPER
GLOBAL  _ISR_14_WRAPPER, _ISR_15_WRAPPER, _ISR_16_WRAPPER, _ISR_17_WRAPPER
GLOBAL  _ISR_18_WRAPPER, _ISR_19_WRAPPER, _ISR_1A_WRAPPER, _ISR_1B_WRAPPER
GLOBAL  _ISR_1C_WRAPPER, _ISR_1D_WRAPPER, _ISR_1E_WRAPPER, _ISR_1F_WRAPPER
GLOBAL  _ISR_20_WRAPPER, _ISR_21_WRAPPER,_ISR_22_WRAPPER,_ISR_23_WRAPPER
GLOBAL  _ISR_24_WRAPPER, _ISR_25_WRAPPER

GLOBAL  _EXC_HAS_ERROR,_int10,_int23,_ROW,_COL,_left,_right,_ROW1,_ROW2,_COL1,_COL2,_cur_task_in_display
EXTERN _EXC_HANDLER, _TIMER_HANDLER,_KBD_HANDLER,_UPDATE_CS,_int10, _old_CS,_old_DS,_old_SS,_idtr,_int23,_SHUT_DOWN,_SETUP_PMODE,_SCANCODE,_FLAG,_left,_right,_cur_task,_int25

SEGMENT _TEXT PUBLIC CLASS=CODE USE16
GLOBAL _EXC_HANDLER, _TIMER_HANDLER,_KBD_HANDLER


%macro  WRAPPER 2
%1:
	push    ax
	mov     ax, %2                  ; save exception number
	jmp     _EXC_COMMON             ; jump to the common code
%endmacro

_EXC_COMMON:
	push    bp
	mov     bp, sp
	push    ds
	push    es                      ; saving segment registers and
	pushad                          ; other regs because it's an ISR
	mov     bx, 10h
	mov     ds, bx
	mov     es, bx                  ; load ds and es with valid selector
	mov     bx, ax
	cmp     byte [_EXC_HAS_ERROR+bx], 0
	je      .1
	push    word [ss:bp+4]          ; error code
	push    dword [ss:bp+8]         ; ip
	push    word [ss:bp+12]         ; cs
	jmp     .2
.1:
	push    word 0                  ; error code
	push    dword [ss:bp+4]         ; ip
	push    word [ss:bp+8]          ; cs
.2:
	push    ax                      ; exception no
; void exc_handler (no, cs, ip, error)
EXTERN _EXC_HANDLER        
	call   _EXC_HANDLER            ; call actual ISR code
	add     sp, 10
	popad                           ; restoring the regs
	pop     es
	pop     ds
	pop     bp
	pop     ax
	iretd

WRAPPER _ISR_00_WRAPPER, 00h
WRAPPER _ISR_01_WRAPPER, 01h
WRAPPER _ISR_02_WRAPPER, 02h
WRAPPER _ISR_03_WRAPPER, 03h
WRAPPER _ISR_04_WRAPPER, 04h
WRAPPER _ISR_05_WRAPPER, 05h
WRAPPER _ISR_06_WRAPPER, 06h
WRAPPER _ISR_07_WRAPPER, 07h
WRAPPER _ISR_08_WRAPPER, 08h
WRAPPER _ISR_09_WRAPPER, 09h
WRAPPER _ISR_0A_WRAPPER, 0Ah
WRAPPER _ISR_0B_WRAPPER, 0Bh
WRAPPER _ISR_0C_WRAPPER, 0Ch
WRAPPER _ISR_0D_WRAPPER, 0Dh
WRAPPER _ISR_0E_WRAPPER, 0Eh
WRAPPER _ISR_0F_WRAPPER, 0Fh
WRAPPER _ISR_10_WRAPPER, 10h
WRAPPER _ISR_11_WRAPPER, 11h
WRAPPER _ISR_12_WRAPPER, 12h
WRAPPER _ISR_13_WRAPPER, 13h
WRAPPER _ISR_14_WRAPPER, 14h
WRAPPER _ISR_15_WRAPPER, 15h
WRAPPER _ISR_16_WRAPPER, 16h
WRAPPER _ISR_17_WRAPPER, 17h
WRAPPER _ISR_18_WRAPPER, 18h
WRAPPER _ISR_19_WRAPPER, 19h
WRAPPER _ISR_1A_WRAPPER, 1Ah
WRAPPER _ISR_1B_WRAPPER, 1Bh
WRAPPER _ISR_1C_WRAPPER, 1Ch
WRAPPER _ISR_1D_WRAPPER, 1Dh
WRAPPER _ISR_1E_WRAPPER, 1Eh
WRAPPER _ISR_1F_WRAPPER, 1Fh

_ISR_20_WRAPPER:
	push    ds
	push    es                      ; saving segment registers and
	pushad                          ; other regs because it's an ISR
	mov     bx, 10h
	mov     ds, bx
	mov     es, bx                  ; load ds and es with valid selector
EXTERN _TIMER_HANDLER
	call    _TIMER_HANDLER          ; call actual ISR code
	popad                           ; restoring the regs
	pop     es
	pop     ds
	iretd

_ISR_21_WRAPPER:
	push    ds
	push    es                      ; saving segment registers and
	pushad                          ; other regs because it's an ISR
	mov     bx, 10h
	mov     ds, bx
	mov     es, bx                  ; load ds and es with valid selector
EXTERN _KBD_HANDLER
	call    _KBD_HANDLER            ; call actual ISR code
	popad                           ; restoring the regs
	pop     es
	pop     ds
	iretd

_ISR_22_WRAPPER:
	push ds
	push es
	pushad
	mov cx,10h
	mov ds,cx
	mov dx,[_cur_task]
	mov [_cur_task_in_display],dx
	call _int10
	popad
	pop es
	pop ds
	iretd
_ISR_23_WRAPPER:
	sti
	push ds
	push es
	mov ax,10h
	mov ds,ax
	call _int23
	pop es
	pop ds
	iretd

_ISR_24_WRAPPER:
back:   mov al,[ds:edx]
	inc edx
	cmp al,0
	je exit
	push edx
	int 22h
	pop edx
	jmp back
exit: 
	iretd
_ISR_25_WRAPPER:
	pushad
	push ds
	push es
	call _int25
	pop es
	pop ds
	popad
	iretd



SEGMENT _DATA PUBLIC CLASS=DATA
	_EXC_HAS_ERROR  DB      0,0,0,0,0,0,0,0, 1,0,1,1,1,1,1,0
			DB      0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
	_COL DB 0
	_ROW DB 0
	_COL1 DB 0
	_COL2 DB 41
	_ROW1 DB 0
	_ROW2 DB 0
	_cur_task_in_display DW 0

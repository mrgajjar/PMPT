.386
DATA SEGMENT use16
	a dw 0
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:  
	mov ax,0ffffh
	mov bx,a
	div bx
	RET
CODE ENDS
END START

.386
DATA SEGMENT use16
a db  'mrugesh',0
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:  
	mov edx,0
	mov edx,offset a
	mov bl,0
	int 24h
	mov al,13
	mov bl,0
	int 22h

	jmp START
	RET
CODE ENDS
END START

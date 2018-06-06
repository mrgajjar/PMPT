; generate overflow exception
.386
DATA SEGMENT use16
a db  'mrugesh',0
b db 'dharmesh',0
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:  
	mov al,108
	mov bl,30
	add al,bl
	into
	int 25h
	ret
CODE ENDS
END START

; generate general protection fault
; tries to access byte beyond your data segment
.386
DATA SEGMENT use16
a db  'mrugesh',0
b db 'dharmesh',0
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:  
	mov al,ds:33
	ret
CODE ENDS
END START

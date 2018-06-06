.386
DATA SEGMENT use16
prompt db  'Enter characters [right] : ',13,0
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:
	mov edx,0
	mov edx,offset prompt
	mov bl,1
	int 24h
START1: int 23h                    
	mov bl,1
	int 22h
	jmp START1
	int 25h
	ret
CODE ENDS
END START

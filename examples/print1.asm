.386
DATA SEGMENT use16
DATA ENDS

CODE SEGMENT use16
	ASSUME CS:CODE,DS:DATA
START:  
        mov bl,2
        mov al,'1'
	int 22h
        mov bl,2
        mov al,13
        int 22h

	jmp START
	RET
CODE ENDS
END START

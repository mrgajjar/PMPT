@echo off
cls
path=%path%;c:\pmpt\tools
c:\pmpt\tools\nasm pmlib.asm -f obj
c:\pmpt\tools\nasm wrappers.asm -f obj
c:\tc\bin\tcc -1 -K -Ic:\tc\include\ -Lc:\tc\lib\ -c  -ms pmpt.c
c:\tc\bin\tcc -1 -K -Ic:\tc\include\ -Lc:\tc\lib\ -c  -ms keyboard.c
c:\tc\bin\tlink c:\tc\lib\c0s pmpt pmlib wrappers keyboard, pmpt,, c:\tc\lib\cs.lib
copy pmpt.exe examples
del *.obj
del *.map

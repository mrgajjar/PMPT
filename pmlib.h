#ifndef _pmlib_h_
#define _pmlib_h_

#include "pmlibdef.h"

unsigned int    read_msw();
unsigned long   read_cr0();
void            write_cr0 (unsigned long value);

void            lgdt (GDTR *gdtr);
void            lidt (IDTR *idtr);
void            lldt (unsigned int selector);

void            ltr (unsigned int selector);
unsigned int    str();
void            clts();
void            jump_to_tss (unsigned int selector);

void            update_cs (unsigned int bew_cs);
void            load_fs (unsigned int fs);
void            load_gs (unsigned int gs);

#endif

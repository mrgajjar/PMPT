#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define BUFSIZE 10000

#include "pmlib.h"
#include "pmlibdefs.h"
#include "wrappers.h"

#define max_tasks       6
#define stack_size      1024
extern byte cur_task_in_display;
extern void init_keyb();
word fg_task=1;
byte prev;
int die;

void scursor(int);
void int25();
byte task_stack[max_tasks-1][stack_size]; /* stacks for tasks (except main()) */
word task_code_offsets[max_tasks];

int flag=0;
word free_ptr=0;

DESCR_SEG gdt[7+2*(max_tasks-1)];       /* GDT */
GDTR gdtr;                              /* GDTR */

DESCR_INT idt[0x26];                    /* IDT */
IDTR idtr;                              /* IDTR */

word old_CS, old_DS, old_SS;
byte old_IRQ_mask[2];
volatile byte scancode=0;

TSS tss[max_tasks];
word total_tasks, cur_task=0;
word task_sels[max_tasks];              /* TSS selectors for tasks */

byte buff[BUFSIZE]={0};
word task_cs_offsets[max_tasks]={0};
word task_ip_offsets[max_tasks]={0};

extern byte row,col,row1,row2,col1,col2;

void setup_GDT_entry (DESCR_SEG *item,dword base, dword limit, byte access, byte attribs) 
{
  item->base_l = base & 0xFFFF;
  item->base_m = (base >> 16) & 0xFF;
  item->base_h = base >> 24;
  item->limit = limit & 0xFFFF;
  item->attribs = attribs | ((limit >> 16) & 0x0F);
  item->access = access;
}

void setup_IDT_entry (DESCR_INT *item,word selector, dword offset, byte access, byte param_cnt) 
{
  item->selector = selector;
  item->offset_l = offset & 0xFFFF;
  item->offset_h = offset >> 16;
  item->access = access;
  item->param_cnt = param_cnt;
}

void setup_GDT() 
{
  dword tmp;
  word i=0;

  /* 0x00 -- null descriptor */
  setup_GDT_entry (&gdt[0], 0, 0, 0, 0);

  /* 0x08 -- code segment descriptor */
  setup_GDT_entry (&gdt[1], ((dword)_CS)<<4, 0xFFFF, ACS_CODE, 0x8f);

  /* 0x10 -- data segment descriptor */
  setup_GDT_entry (&gdt[2], ((dword)_DS)<<4, 0xFFFF, ACS_DATA, 0x8f);

  /* 0x18 -- stack segment descriptor */
  setup_GDT_entry (&gdt[3], ((dword)_SS)<<4, 0xFFFF, ACS_STACK, 0x8f);

  /* 0x20 -- text video mode segment descriptor */
  setup_GDT_entry (&gdt[4], 0xB8000L, 0xFFFF, ACS_DATA, 0);

  /* 0x28 -- TSS for main() */
  tmp = ((dword)_DS)<<4;
  tmp += (word)&tss[0];
  setup_GDT_entry (&gdt[5], tmp, sizeof(TSS), ACS_TSS, 0);
  task_sels[0] = 0x28;

	/* 0x30 = code segment for all tasks except main() */
  setup_GDT_entry (&gdt[6], ((dword)_DS)<<4, 0xFFFF, ACS_CODE, 0x8f);

	/*DO NOT ADD NEW DESCRIPTORS BEFORE THIS LINE !!!! */

  /* 0x38 -- TSS for tasks */
  for(i=0;i<max_tasks-1;i++)
  {
	tmp = ((dword)_DS)<<4;
	tmp += (word)&tss[i+1];
	setup_GDT_entry (&gdt[7+i], tmp, sizeof(TSS), ACS_TSS, 0);
	task_sels[i+1] = (7+i)*8;
  }
  for(i=0;i<max_tasks-1;i++)
  {
	setup_GDT_entry(&gdt[7+max_tasks-1+i],
		(((dword)_DS)<<4)+(word)(&buff[task_code_offsets[i]]),
		task_cs_offsets[i]*16+task_ip_offsets[i]/*0xffff*/,ACS_DATA,0x0);
	/* add one data segment descriptor for each task */
  }


  /* setting up the GDTR register */
  gdtr.base = ((dword)_DS)<<4;
  gdtr.base += (word)&gdt;
  gdtr.limit = sizeof(gdt)-1;
  lgdt (&gdtr);
}

void setup_IDT() 
{
  /* setting up the exception handlers and timer, keyboard ISRs */
  setup_IDT_entry (&idt[0x00], 0x08, (word)&isr_00_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x01], 0x08, (word)&isr_01_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x02], 0x08, (word)&isr_02_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x03], 0x08, (word)&isr_03_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x04], 0x08, (word)&isr_04_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x05], 0x08, (word)&isr_05_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x06], 0x08, (word)&isr_06_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x07], 0x08, (word)&isr_07_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x08], 0x08, (word)&isr_08_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x09], 0x08, (word)&isr_09_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0A], 0x08, (word)&isr_0A_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0B], 0x08, (word)&isr_0B_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0C], 0x08, (word)&isr_0C_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0D], 0x08, (word)&isr_0D_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0E], 0x08, (word)&isr_0E_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x0F], 0x08, (word)&isr_0F_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x10], 0x08, (word)&isr_10_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x11], 0x08, (word)&isr_11_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x12], 0x08, (word)&isr_12_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x13], 0x08, (word)&isr_13_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x14], 0x08, (word)&isr_14_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x15], 0x08, (word)&isr_15_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x16], 0x08, (word)&isr_16_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x17], 0x08, (word)&isr_17_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x18], 0x08, (word)&isr_18_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x19], 0x08, (word)&isr_19_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1A], 0x08, (word)&isr_1A_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1B], 0x08, (word)&isr_1B_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1C], 0x08, (word)&isr_1C_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1D], 0x08, (word)&isr_1D_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1E], 0x08, (word)&isr_1E_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x1F], 0x08, (word)&isr_1F_wrapper, ACS_INT, 0);

  setup_IDT_entry (&idt[0x20], 0x08, (word)&isr_20_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x21], 0x08, (word)&isr_21_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x22], 0x08, (word)&isr_22_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x23], 0x08, (word)&isr_23_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x24], 0x08, (word)&isr_24_wrapper, ACS_INT, 0);
  setup_IDT_entry (&idt[0x25], 0x08, (word)&isr_25_wrapper, ACS_INT, 0);

  /* setting up the IDTR register */
  idtr.base = ((dword)_DS)<<4;
  idtr.base += (word)&idt;
  idtr.limit = sizeof(idt)-1;
  lidt (&idtr);
}

void setup_PIC (byte master_vector, byte slave_vector) 
{
  outportb (PORT_8259M, 0x11);                  /* start 8259 initialization */
  outportb (PORT_8259S, 0x11);
  outportb (PORT_8259M+1, master_vector);       /* master base interrupt vector */
  outportb (PORT_8259S+1, slave_vector);        /* slave base interrupt vector */
  outportb (PORT_8259M+1, 1<<2);                /* bitmask for cascade on IRQ2 */
  outportb (PORT_8259S+1, 2);                   /* cascade on IRQ2 */
  outportb (PORT_8259M+1, 1);                   /* finish 8259 initialization */
  outportb (PORT_8259S+1, 1);
}
struct
{   
     unsigned char m;
     unsigned char z;
     unsigned int  remnant;
     unsigned int  paragraphs;
     unsigned int  no_of_relocatables;
     unsigned int  header_size;
     unsigned int  min_count;
     unsigned int  hma_lma;
     unsigned int  ss_offset;
     unsigned int  sp_offset;
     unsigned int  check_sum;
     unsigned int  ip_offset;
     unsigned int  cs_offset;
     unsigned int  rt_offset;
     unsigned int  overlay_number;
}header;


void setup_PMode() 
{
  int i;

  /* disable interrupts so that IRQs don't cause exceptions */
  disable();

  /* setup GDT */
  setup_GDT();

  /* setup IDT */
  setup_IDT();

  /* save IRQ masks */
  old_IRQ_mask[0] = inportb (PORT_8259M+1);
  old_IRQ_mask[1] = inportb (PORT_8259S+1);

  /* setup PIC */
  setup_PIC (0x20, 0x28);

  /* set new IRQ masks */
  outportb (PORT_8259M+1, 0xFC);       /* enable timer ans keyboard (master) */
  outportb (PORT_8259S+1, 0xFF);       /* disable all (slave) */

  /* saving real mode segment addresses */
  old_CS = _CS;
  old_DS = _DS;
  old_SS = _SS;

  /* This switches us to PMode just setting up CR0.PM bit to 1 */
  write_cr0 (read_cr0() | 1);

  /* loading segment registers with PMode selectors */
  update_cs (0x08);
  _ES = _DS = 0x10;
  _SS = 0x18;

  /* if we don't load fs and gs with valid selectors, task switching may fail. */
  load_fs (0x10);
  load_gs (0x10);

  lldt (0);

  for (i=0;i<max_tasks;i++) 
  {
    tss[i].trace = 0;
    tss[i].io_map_addr = sizeof(TSS);           /* I/O map just after the TSS */

    if (i) 
    {
      tss[i].ldtr = 0;                          /* ldtr=0 */

      tss[i].fs = tss[i].gs = 0;                /* fs=gs=0 */

      tss[i].ds= tss[i].es = tss[i].ss = _DS;  /* ds=es=ss = data segment */

      tss[i].ds = (word)((7+max_tasks-1+i-1)*8);

      tss[i].cs = 0x30;

      tss[i].eflags = 0x202L;                   /* interrupts are enabled */
      tss[i].esp = (word)&task_stack[i];        /* sp points to task stack top */

      tss[i].eip = (word)((word)&buff[task_code_offsets[i-1]]+task_cs_offsets[i-1]*16+task_ip_offsets[i-1]);                    /* cs:eip point to task1() */

    }
  }

  /* load the TR register */
  ltr (task_sels[0]);

  enable();
}

void shut_down() 
{
  int p;
  /* clear CR0.TS flag so that DPMI programs can normally startup 
     after this program terminates. */
  clts();

  /* load fs and gs with selectors of 64KB segments */
  load_fs (0x10);
  load_gs (0x10);

  /* get out of PMode clearing CR0.PM bit to 0 */
  write_cr0 (read_cr0() & 0xFFFFFFFEL);

  /* restoring real mode segment values */
  update_cs (old_CS);
  _ES = _DS = old_DS;
  _SS = old_SS;

  idtr.base = 0;
  idtr.limit = 0x3FF;
  lidt (&idtr);

  /* setup PIC */
  setup_PIC (8, 0x70);

  /* restore IRQ masks */
  outportb (PORT_8259M+1, old_IRQ_mask[0]);     /* master */
  outportb (PORT_8259S+1, old_IRQ_mask[1]);     /* slave */

  /* enabling interrupts */
  enable();
space:        p=row*80+col;
  scursor(p);
     
}
char *exefiles[max_tasks];

void exc_handler (word exc_no, word cs, dword ip, word error) 
{
  word tr;

  tr=str();
  shut_down();

  textbackground (RED); textcolor (WHITE);
  cprintf ("exception no: %02XH\n\r", exc_no);
  cprintf ("at address  : %04XH:%08XH\n\r", cs, ip);
  if (exc_has_error[exc_no]) 
  {
    cprintf ("error code  : %04XH [ Index:%04XH, Type:%d ]\n\r",
	    error, error >> 3, error & 7);
  }
  cprintf ("Task Register: %04XH\n\r", tr);

  textbackground (GREEN); textcolor (WHITE);

  if(tr==0x28)
	cprintf("Task: %s\n\r",exefiles[0]);
  else
	cprintf("Task: %s\n\r",exefiles[tr/8-6]);
  switch(exc_no)
  {
	case 0: cprintf("Vector No: 0\n\rMnenonic: #DE\n\rDescription: Divide Error\n\rType: Fault\n\rSource: DIV and IDIV instructions");
		break;
	case 1: cprintf("Vector No: 1\n\rMnenonic: #DB\n\rDescription: Debug\n\rType: Fault/Trap\n\rSource: Any code or data reference or the INT 1 instruction");
		break;
	case 2: cprintf("Vector No: 2\n\rMnenonic: #__\n\rDescription: NMI interrupt\n\rType: Interrupt\n\rSource: Nonmaskable external interrupt");
		break;
	case 3: cprintf("Vector No: 3\n\rMnenonic: #BP\n\rDescription: Breakpoint\n\rType: Trap\n\rSource: INT 3 instruction");
		break;
	case 4: cprintf("Vector No: 4\n\rMnenonic: #OF\n\rDescription: Overflow\n\rType: Trap\n\rSource: INTO instruction");
		break;
	case 5: cprintf("Vector No: 5\n\rMnenonic: #BR\n\rDescription: BOUND Range Exceeded\n\rType: Fault\n\rSource: BOUND instruction");
		break;
	case 6: cprintf("Vector No: 6\n\rMnenonic: #UD\n\rDescription: Invalid Opcode(Undefined Opcode)\n\rType: Fault\n\rSource: UD2 instruction or reserved opcode");
		break;
	case 7: cprintf("Vector No: 7\n\rMnenonic: #NM\n\rDescription: Device not available (Not math Coprocessor)\n\rType: Fault\n\rSource: Floating point or WAIT/FWAIT instruction ");
		break;
	case 9: cprintf("Vector No: 8\n\rMnenonic:    \n\rDescription: Coprocessor Segment Overrun(reserved)\n\rType: Fault\n\rSource: Floating point instruction");
		break;
	case 8: cprintf("Vector No: 8\n\rMnenonic: #DF\n\rDescription: Double Fault\n\rType: Abort\n\rSource: Any instruction that can generate an exception, an NMI or an INTR");
		break;
	case 10: cprintf("Vector No: 10\n\rMnenonic: #TS\n\rDescription: Invalid TSS\n\rType: Fault\n\rSource: Task switch or TSS access");
		break;
	case 11: cprintf("Vector No: 11\n\rMnenonic: #NP\n\rDescription: Segment Not Present\n\rType: Fault\n\rSource: Loading segment registers or accessing system segments");
		break;
	case 12: cprintf("Vector No: 12\n\rMnenonic: #SS\n\rDescription: Stack-Segment Fault\n\rType: Fault\n\rSource: Stack operations and SS register loads");
		break;
	case 13: cprintf("Vector No: 13\n\rMnenonic: #GP\n\rDescription: General Protection\n\rType: Fault\n\rSource: Any memory reference and other protection checks");
		break;
	case 14: cprintf("Vector No: 14\n\rMnenonic: #PF\n\rDescription: Page Fault\n\rType: Fault\n\rSource: Any memory reference");
		break;
	case 15: cprintf("Vector No: 15\n\rMnenonic: \n\rDescription: Intel reserved(do not use)\n\rType: \n\rSource: ");
		break;
	case 16: cprintf("Vector No: 16\n\rMnenonic: #MF\n\rDescription: Floating point Error (Math fault)\n\rType: Fault\n\rSource: Floating point or WAIT/FWAIT instruction");
		break;
	case 17: cprintf("Vector No: 17\n\rMnenonic: #AC\n\rDescription: Alignment Check\n\rType: Fault\n\rSource: Any data reference in memory ");
		break;
	case 18: cprintf("Vector No: 18\n\rMnenonic: #MC\n\rDescription: Machine Check\n\rType: Abort\n\rSource: Error codes(if any) and source are model dependent");
		break;
	case 19: cprintf("Vector No: 19\n\rMnenonic: #XF\n\rDescription: Streaming SIMD Extensions\n\rType: Fault\n\rSource: SIMD floating point instrucitons");break;
	default: cprintf("Intel reserved (do not use)\n\r");
  }
  exit (1);
}
void scheduler() 
{
   if (++cur_task >= total_tasks)
    cur_task = 0;

  jump_to_tss (task_sels[cur_task]);
}

void timer_handler()
{
  outportb (PORT_8259M, EOI);
  scheduler();
}
void int25()
{
  asm mov ax,10h
  asm mov ds,ax
  asm sti
  while (1); 
}

void left(),right();
void int10()
{
   int pos,i,p,temp;
   if(_BL==0)
   {
        left();
        return;
   }
   else if(_BL==1)
   {
        right();
        return;
   }
   pos=0;
   asm pusha

   asm mov bx,10h
   asm mov ds,bx
   switch(_AL)
   {
	case 13: 
		row++;
		col=0;
		break;
	case 8: 
		if(col==0)
		{
		 col=79;
		 row--;
		}
		else
		 col--;
		 break;
	default: break;
   }
skip:
   if(col>79)
   {     row++;
	col=0;
   }
   if(row>24)
   {
	asm push es
	asm mov ax,20h
	asm mov es,ax
	for(i=0;i<160*24;i+=2)
	{
				 
	      asm    mov bx,i
	      asm    mov ax,es:[bx+160]
	      asm    mov es:[bx],ax
		   
	 }
	 for(i=160*24;i<160*24+160;i+=2)
	 {
		
	       asm   mov al,' '
	       asm   mov ah,7
	       asm   mov bx,i
	       asm   mov es:[bx],ax
		  
	  }
	  asm pop es
	  row=24;
    }
     for(i=0;i<row;i++) pos+=160;
     pos+=(col)*2;
     asm popa
   
     asm pusha
     asm  mov bx,20h
     asm mov es,bx
     asm mov bx,pos
     asm cmp al,13
     asm je space
     if(_AL==8)
     {
	 asm  mov BYTE PTR es:[bx],' '
     }
     else
     {
	col++;
	asm mov es:[bx],AL
     }
 /*-------------------------*/
space:  p=row*80+col;
	scursor(p); 
	asm popa
}
void scursor(int p)
{
	asm        MOV CX,p
	asm        MOV AH,14
	asm        MOV DX,003D4H
	asm        MOV AL,AH
	asm        OUT DX,AL
	asm        INC DX
	asm        MOV AL,CH
	asm        OUT DX,AL
	asm        DEC DX
	asm        MOV AL,AH
	asm        INC AL
	asm        OUT DX,AL
	asm        INC DX
	asm        MOV AL,CL
	asm        OUT DX,AL
}                        
void left()
{
   int pos,i,p,temp,c;
   pos=0;
   asm pusha

   asm mov bx,10h
   asm mov ds,bx

   switch(_AL)
   {
	case 13: 
		row1++;
		col1=0;
		break;
	case 8: 
		if(col1==0)
		{
		 col1=39;
		 row1--;
		}
		else
		 col1--;
		 break;
	default: break;
   }
skip:
   if(col1>39)
   {
	row1++;
	col1=0;
   }
   if(row1>24)
   {
	asm push es
	asm mov ax,20h
	asm mov es,ax
	for(i=0,c=0;i<160*24;i+=2,c++)
	{
	   if(c<=39)
	   {
	      asm    mov bx,i
	      asm    mov ax,es:[bx+160]
	      asm    mov es:[bx],ax
	   }
	   if(c==79)
	      c=-1;
		   
	 }
	 for(i=160*24;i<160*24+80;i+=2)
	 {
	       asm   mov al,' '
	       asm   mov ah,7
	       asm   mov bx,i
	       asm   mov es:[bx],ax
		  
	  }
	  asm pop es
	  row1=24;
    }
   for(i=0;i<row1;i++) pos+=160;
   pos+=(col1)*2;
   asm popa
   
     asm pusha
     asm  mov bx,20h
     asm mov es,bx
     asm mov bx,pos
     asm cmp al,13
     asm je space
     if(_AL==8)
     {
	 asm  mov BYTE PTR es:[bx],' '
     }
     else
     {
	col1++;
	asm mov es:[bx],AL
     }
space:
     p=row1*80+col1;
     if((word)cur_task_in_display==fg_task)
     scursor(p);
     asm popa
}
void right()
{
   int pos,i,p,temp,c;
   pos=0;
   asm pusha

   asm mov bx,10h
   asm mov ds,bx
  
   switch(_AL)
   {
	case 13: 
		row2++;
		col2=41;
		break;
	case 8: 
		if(col2==41)
		{
		 col2=79;
		 row2--;
		}
		else
		 col2--;
		 break;
	default: break;
   }
skip:
   if(col2>79)
   {
	row2++;
	col2=41;
   }
   if(row2>24)
   {
	asm push es
	asm mov ax,20h
	asm mov es,ax
	for(i=0,c=0;i<160*24;i+=2,c++)
	{
	   if(c>=41)
	   {
	      asm    mov bx,i
	      asm    mov ax,es:[bx+160]
	      asm    mov es:[bx],ax
	   }
	   if(c==79)
	      c=-1;
	 }
	 for(i=160*24+80;i<160*24+160;i+=2)
	 {
	       asm   mov al,' '
	       asm   mov ah,7
	       asm   mov bx,i
	       asm   mov es:[bx],ax
		  
	  }
	  asm pop es
	  row2=24;
    }
   for(i=0;i<row2;i++) pos+=160;
   pos+=(col2)*2;
   asm popa
   
     asm pusha
     asm mov bx,20h
     asm mov es,bx
     asm mov bx,pos
     asm cmp al,13
     asm je space
     if(_AL==8)
     {
	 asm  mov BYTE PTR es:[bx],' '
     }
     else
     {
	col2++;
	asm mov es:[bx],AL
     }
space:  p=row2*80+col2;
	if((word)cur_task_in_display==fg_task)
	scursor(p);
	asm popa
}

/* ----------------loader -------------------*/

/*struct  ntype
  {
     unsigned int offset;
     unsigned int segment;
	 struct ntype *next;
  } *relocater,*head;*/

FILE *fpt;
unsigned long code;
unsigned long fs;

void loadexe(char *file)
{
        unsigned long i;
	int j;
	unsigned int  ch,ch1,bl;

	if ((fpt=fopen(file,"rb"))==NULL)
	{
	   printf("\nERROR ENCOUNTERED OPENING FILE... %s",file);
	   exit(1);
	}
	fread(&header,sizeof(header),1,fpt);
	clrscr();
	if(header.m !='M' || header.z != 'Z')
	{
	   printf("THIS IS NOT A EXECUTABLE FILE.... %s",file);
	   exit(1);
	}
	i=0;
	fs=((unsigned long)header.paragraphs-1)*512;
	fs+=header.remnant; /* THIS GIVES FILE SIZE */
	code=(unsigned long)header.header_size*16; 
	/* fpt posn from where i should load in memory.SEE HEADER INFO ABOVE */ 

	fseek(fpt,code,SEEK_SET); /* POSN FILE PTR AT CODE AREA */
	i=code;
	if((free_ptr+fs-header.header_size*16)<BUFSIZE)
	{
	     j=free_ptr;
	     while ( i < fs)
	     {
		  ch=fgetc(fpt);
		  buff[j++]=ch;
		  i++;
	     }
	     free_ptr=j+(16-(j%16));
	}
	else
	{
	     printf("Out of memory for %d bytes\n",free_ptr+fs-header.header_size*16-BUFSIZE);
	     exit(0);
	}
	fclose(fpt);
}
/*------------------------------loader ends---------------------*/
int main(int argc,char *argv[])
{
  int i=0;
  if (read_msw() & 1)
  {
    printf ("The CPU is already in PMode.\nAborting...");
    return 0;
  }
  if(argc==1)
  {
	printf("Usage: progname exe files......");
	exit(1);
  }
  if(argc>max_tasks)
  {
        printf("Maximum %d tasks can be executed\n",max_tasks);
        exit(1);
  }
  for(i=0;i<argc;i++)
	exefiles[i]=argv[i];
  total_tasks=argc;
  argc--;
  for(i=0;i<argc;i++)
  {
       task_code_offsets[i]=free_ptr;
       loadexe(argv[i+1]);
       task_cs_offsets[i]=header.cs_offset;
       task_ip_offsets[i]=header.ip_offset;
  }

  /* setting up pmode */
  flushall();
  init_keyb();
  setup_PMode();
  while (!die);
  /* going back to real mode */
  shut_down();
  gotoxy(1,25);
  cprintf("Press any key.....");
  getch();
  clrscr();
  printf("The Protected Mode Programming Tool\n");
  printf("\t\tPMPT 1.0\n");
  printf("Developers : \n");
  printf("%-20s\t%-30s\n","Mrugesh Gajjar","mrugesh386@gmail.com");
  printf("%-20s\t%-30s\n","Parag Panchal","paragpanchal@yahoo.com");
  printf("%-20s\t%-30s\n","Nirav Patel","patel_nirav@yahoo.com");
  printf("%-20s\t%-30s\n","Dharmesh Thakkar","t_dharmesh@yahoo.com");
  return 0;
}
  

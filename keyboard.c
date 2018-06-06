#include "pmlibdef.h"

extern int fg_task,die,cur_task;
static void _kbdFlush(void);
extern void scheduler();
extern row1,row2,col1,col2;

typedef struct  /* circular queue structure:buffer for keyboard*/
{       bool NonEmpty;
	u8 *Data;
	u16 Size, Inptr, Outptr;
} queue;

#define BUFSIZE                 512     /*size of the circular queue*/

static u8 keybuf[BUFSIZE];

static queue kb_queue;

int kb_wait=0;

static int shift = 0;
static int ctrl = 0;
static int alt = 0;
static int caps = 0;
static int num = 0;


static char normal[] = {
  0x00,0x1B,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
'q','w','e','r','t','y','u','i','o','p','[',']',0x0D,0x80,
'a','s','d','f','g','h','j','k','l',';',047,0140,0x80,
0134,'z','x','c','v','b','n','m',',','.','/',0x80,
'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,'0',0177
};


static char shifted[] = {
  0,033,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
'Q','W','E','R','T','Y','U','I','O','P','{','}',015,0x80,
'A','S','D','F','G','H','J','K','L',':',042,'~',0x80,
'|','Z','X','C','V','B','N','M','<','>','?',0x80,
'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,'7','8','9',0x80,'4','5','6',0x80,
'1','2','3','0',177
};

/*****************************************************************************
	name:   inq
	action: tries to add byte Data to queue Queue
	returns:-1 queue full
		0  success
*****************************************************************************/
static int 
inq(queue *Queue, u8 Data)
{       u16 Temp;

	Temp=Queue->Inptr + 1;
	if(Temp >= Queue->Size)
		Temp=0;
	if(Temp == Queue->Outptr)
		return(-1);     /* full */
	Queue->Data[Queue->Inptr]=Data;
	Queue->Inptr=Temp;
	Queue->NonEmpty=true;
	return(0);
}

/*****************************************************************************
	name:   deq
	action: tries to get byte from Queue
	returns:-1  queue empty
		>=0 success (byte read)
*****************************************************************************/
static int 
deq(queue *Queue)
{       u8 RetVal;

	if(Queue->NonEmpty == false)
		return(-1);     /* empty */
	RetVal=Queue->Data[Queue->Outptr++];
	if(Queue->Outptr >= Queue->Size)
		Queue->Outptr=0;
	if(Queue->Outptr == Queue->Inptr)
		Queue->NonEmpty=false;
	return(RetVal);
}

/*****************************************************************************
	name:   init_keyb
	action: initialises the keyboard buffer
	returns:void
*****************************************************************************/
void
init_keyb()
{
	kb_queue.NonEmpty = false;
	kb_queue.Data = keybuf;
	kb_queue.Size = BUFSIZE;
	kb_queue.Inptr=0;
	kb_queue.Outptr=0;
	_kbdFlush();
}


/*****************************************************************************
	name:   init_keyb
	action: initialises the keyboard buffer
	returns:void
*****************************************************************************/
static int
kb_special(unsigned char key)
{
   switch(key) {
      case 0x36:
      case 0x2A:
	 shift = 1;
	 break;
      case 0xB6:
      case 0xAA:
	 shift = 0;
	 break;
      case 0x1D:
	 ctrl = 1;
	 break;
      case 0x9D:
	 ctrl = 0;
	 break;
      case 0x38:
	 alt = 1;
	 break;
      case 0xB8:
	 alt = 0;
	 break;
      case 0x3A:
      case 0x45:
	 break;
      case 0xBA:
	 caps = !caps;
	 break;
      case 0xC5:
	 num = !num;
	 break;
      case 0xE0:
	 break;
      default:
	 return(0);
   }
   return (1);
}


void int23()
{       int i,p;
	while(fg_task!=cur_task);

	kb_queue.Outptr=kb_queue.Inptr;
	kb_queue.NonEmpty=false;
	while(kb_queue.NonEmpty!=true);
	i=deq(&kb_queue);
	asm mov ax,i
}

/*****************************************************************************
	name:   _kbdFlush
	action: initialises the keyboard buffer
	returns:void
*****************************************************************************/
static void 
_kbdFlush(void)
{       unsigned short Timeout;
	u8 Stat;

	for(Timeout=20;Timeout>0;Timeout--)
	{       Stat=inportb(_KBD_STAT_REG);
		/* loop until 8042 output buffer full */
		if((Stat & _KBD_STAT_OBF) != 0) inportb(_KBD_DATA_REG);
		else return;
	}
}

void kbd_handler() {
	u8 c;
	c=inportb(PORT_KBD_A);

	if((!kb_special(c))&&(c < 0x80))
		{
		if(ctrl && (c == 0x2E))
			die=1;
		else if(ctrl && (c == 0x1f))
		     {
			if(fg_task==1)  fg_task=2; else fg_task=1;
		      }
		else
		      {
			c=shift ? shifted[c] : normal[c];
			inq(&kb_queue,c);
		      }
		}
	 outportb (PORT_8259M, EOI);
}



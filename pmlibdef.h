#ifndef _pm_defs_
#define _pm_defs_

#define byte unsigned char
#define word unsigned int
#define dword unsigned long
typedef enum
{ false=0, true=1 } bool;

typedef unsigned char   u8;
typedef unsigned short  u16;

/* Ports */
#define PORT_8259M      0x20
#define PORT_8259S      0xA0
#define PORT_KBD_A      0x60
#define _KBD_STAT_REG           0x64   
#define _KBD_DATA_REG           0x60
#define _KBD_STAT_OBF           0x01    /* output bffr full (data for host) */

#define EOI             0x20

/* Access byte's flags */
#define ACS_PRESENT     0x80            /* present segment */
#define ACS_CSEG        0x18            /* code segment */
#define ACS_DSEG        0x10            /* data segment */
#define ACS_CONFORM     0x04            /* conforming segment */
#define ACS_READ        0x02            /* readable segment */
#define ACS_WRITE       0x02            /* writable segment */
#define ACS_IDT         ACS_DSEG        /* segment type is the same type */
#define ACS_INT_GATE    0x0E            /* int gate for 386 */
#define ACS_INT         (ACS_PRESENT | ACS_INT_GATE) /* present int gate */
#define ACS_TSS_GATE    0x09
#define ACS_TSS         (ACS_PRESENT | ACS_TSS_GATE) /* present tss gate */

/* Ready-made values */
#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)


/* Segment desciptor definition */
typedef struct 
{
  word limit,
       base_l;
  byte base_m,
       access,
       attribs,
       base_h;
} DESCR_SEG;

/* GDTR register definition */
typedef struct 
{
  word limit;
  dword base;
} GDTR;

/* Interrupt desciptor definition */
typedef struct 
{
  word offset_l,
       selector;
  byte param_cnt,
       access;
  word offset_h;
} DESCR_INT;

/* IDTR register definition */
typedef struct 
{
  word limit;
  dword base;
} IDTR;

/* TSS definition */
typedef struct 
{        /* TSS for 386+ */
        dword link,
        esp0,
        ss0,
        esp1,
        ss1,
        esp2,
        ss2,
        cr3,
        eip,
        eflags,
        eax,
        ecx,
        edx,
        ebx,
        esp,
        ebp,
        esi,
        edi,
        es,
        cs,
        ss,
        ds,
        fs,
        gs,
        ldtr;
  word  trace,
        io_map_addr;
} TSS;


#endif

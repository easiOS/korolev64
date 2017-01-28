#ifndef H_INT
#define H_INT

#include <base.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

struct idtd {
	WORD limit;
	LONG base;
} PACK;

typedef struct idtd idtd_t;
typedef struct idtd gdtd_t;

struct idt_entry {
	WORD off1;
	WORD sel;
	BYTE zero;
	BYTE attr;
	WORD off2;
} PACK;

struct gdt_entry
{
   WORD limit;
   WORD base_low;
   BYTE  base_middle;
   BYTE  access;
   BYTE  granularity;
   BYTE  base_high;
} PACK;

typedef struct
{
   LONG ds;                  // Data segment selector
   LONG edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   LONG int_no, err_code;    // Interrupt number and error code (if applicable)
   LONG eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} PACK regs_t;

typedef void (*isr_t)(regs_t);

typedef struct idt_entry idt_t;
typedef struct gdt_entry gdt_t;

void int_regh(BYTE id, isr_t handler);

void int_setup(void);

void idt_setup(void);
void idt_set(void* idtd);
void idt_set_gate(BYTE i, LONG base, WORD sel, BYTE flags);

void gdt_setup(void);
void gdt_set(void* gdtd);
void gdt_set_gate(BYTE i, LONG base, LONG lim, BYTE acc, BYTE gran);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr127();
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif /* H_INT */
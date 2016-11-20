#include <base.h>
#include <text.h>
#include <int.h>
#include <port.h>
#include <string.h>

gdt_t gdt[5];
gdtd_t gdtd;
idt_t idt[256];
idtd_t idtd;

isr_t int_handlers[256];

void int_setup(void)
{
	gdt_setup();
	idt_setup();
}

void gdt_setup(void)
{
	puts("GDT setup...");
	gdtd.limit = (sizeof(gdt_t) * 5) - 1;
	gdtd.base = (LONG)&gdt;

	gdt_set_gate(0, 0, 0, 0, 0); // null
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data

	gdt_set(&gdtd);

	puts("OK!\n");
}

void gdt_set_gate(BYTE i, LONG base, LONG lim, BYTE acc, BYTE gran)
{
	gdt[i].base_low = (base & 0xFFFF);
	gdt[i].base_middle = (base >> 16) & 0xFF;
	gdt[i].base_high = (base >> 24) & 0xFF;

	gdt[i].limit = (lim & 0xFFFF);
	gdt[i].granularity = (lim >> 16) & 0x0F;

	gdt[i].granularity |= gran & 0xF0;
	gdt[i].access = acc;
}

void idt_setup(void)
{
	puts("IDT setup...");
	idtd.limit = (sizeof(idt_t) * 256) - 1;
	idtd.base = (LONG)&idt;

	kmemset(idt, 0, sizeof(idt_t) * 256);

	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	idt_set_gate( 0, (LONG)isr0 , 0x08, 0x8E);
	idt_set_gate( 1, (LONG)isr1 , 0x08, 0x8E);
	idt_set_gate( 2, (LONG)isr2 , 0x08, 0x8E);
	idt_set_gate( 3, (LONG)isr3 , 0x08, 0x8E);
	idt_set_gate( 4, (LONG)isr4 , 0x08, 0x8E);
	idt_set_gate( 5, (LONG)isr5 , 0x08, 0x8E);
	idt_set_gate( 6, (LONG)isr6 , 0x08, 0x8E);
	idt_set_gate( 7, (LONG)isr7 , 0x08, 0x8E);
	idt_set_gate( 8, (LONG)isr8 , 0x08, 0x8E);
	idt_set_gate( 9, (LONG)isr9 , 0x08, 0x8E);
	idt_set_gate( 10, (LONG)isr10 , 0x08, 0x8E);
	idt_set_gate( 11, (LONG)isr11 , 0x08, 0x8E);
	idt_set_gate( 12, (LONG)isr12 , 0x08, 0x8E);
	idt_set_gate( 13, (LONG)isr13 , 0x08, 0x8E);
	idt_set_gate( 14, (LONG)isr14 , 0x08, 0x8E);
	idt_set_gate( 15, (LONG)isr15 , 0x08, 0x8E);
	idt_set_gate( 16, (LONG)isr16 , 0x08, 0x8E);
	idt_set_gate( 17, (LONG)isr17 , 0x08, 0x8E);
	idt_set_gate( 18, (LONG)isr18 , 0x08, 0x8E);
	idt_set_gate( 19, (LONG)isr19 , 0x08, 0x8E);
	idt_set_gate( 20, (LONG)isr20 , 0x08, 0x8E);
	idt_set_gate( 21, (LONG)isr21 , 0x08, 0x8E);
	idt_set_gate( 22, (LONG)isr22 , 0x08, 0x8E);
	idt_set_gate( 23, (LONG)isr23 , 0x08, 0x8E);
	idt_set_gate( 24, (LONG)isr24 , 0x08, 0x8E);
	idt_set_gate( 25, (LONG)isr25 , 0x08, 0x8E);
	idt_set_gate( 26, (LONG)isr26 , 0x08, 0x8E);
	idt_set_gate( 27, (LONG)isr27 , 0x08, 0x8E);
	idt_set_gate( 28, (LONG)isr28 , 0x08, 0x8E);
	idt_set_gate( 29, (LONG)isr29 , 0x08, 0x8E);
	idt_set_gate( 30, (LONG)isr30 , 0x08, 0x8E);
	idt_set_gate(31, (LONG)isr31, 0x08, 0x8E);
	idt_set_gate(32, (LONG)irq0, 0x08, 0x8E);
	idt_set_gate(33, (LONG)irq1, 0x08, 0x8E);
	idt_set_gate(34, (LONG)irq2, 0x08, 0x8E);
	idt_set_gate(35, (LONG)irq3, 0x08, 0x8E);
	idt_set_gate(36, (LONG)irq4, 0x08, 0x8E);
	idt_set_gate(37, (LONG)irq5, 0x08, 0x8E);
	idt_set_gate(38, (LONG)irq6, 0x08, 0x8E);
	idt_set_gate(39, (LONG)irq7, 0x08, 0x8E);
	idt_set_gate(40, (LONG)irq8, 0x08, 0x8E);
	idt_set_gate(41, (LONG)irq9, 0x08, 0x8E);
	idt_set_gate(42, (LONG)irq10, 0x08, 0x8E);
	idt_set_gate(43, (LONG)irq11, 0x08, 0x8E);
	idt_set_gate(44, (LONG)irq12, 0x08, 0x8E);
	idt_set_gate(45, (LONG)irq13, 0x08, 0x8E);
	idt_set_gate(46, (LONG)irq14, 0x08, 0x8E);
	idt_set_gate(47, (LONG)irq15, 0x08, 0x8E);
	idt_set_gate(128, (LONG)isr_sc, 0x08, 0x8E);

	idt_set(&idtd);

	puts("OK!\n");
}

void idt_set_gate(BYTE i, LONG base, WORD sel, BYTE flags)
{
	idt[i].off1 = base & 0xFFFF;
	idt[i].off2 = (base >> 16) & 0xFFFF;
	idt[i].sel = sel;
	idt[i].zero = 0;
	idt[i].attr = flags;
}

void int_regh(BYTE id, isr_t handler)
{
	int_handlers[id] = handler;
}

// interrupt handlers

void isr_handler(regs_t regs)
{
	if(int_handlers[regs.int_no] && regs.int_no < 256)
	{
		int_handlers[regs.int_no](regs);
	}
}

void irq_handler(regs_t regs)
{
	if(int_handlers[regs.int_no])
	{
		int_handlers[regs.int_no](regs);	
	}

	if(regs.int_no >= 40) // send EOI to slave too
	{
		outb(0xA0, 0x20);
	}

	outb(0x20, 0x20); // send EOI
}

void syscall_handler(regs_t regs)
{
	switch(regs.eax)
	{
		case 0:
			put('a'); put('y'); put('y'); put('\n');
			break;
	}
}
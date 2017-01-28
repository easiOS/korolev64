#include <base.h>
#include <port.h>
#include <int.h>

LONG timer;
LONG sleepc;

static void timer_handler(regs_t regs)
{
	timer++;
	if(sleepc)
		sleepc--;
}

void timer_setup(void)
{
	puts("[timer] Timer setup...");
	timer = 0;
	sleepc = 0;
	outb(0x43, 0x36);
	WORD divisor = 11931;
	BYTE l = divisor & 0xFF;
	BYTE h = (divisor >> 8) & 0xFF;
	outb(0x40, l);
	outb(0x40, h);
	int_regh(IRQ0, &timer_handler);
	puts("OK!\n");
}

void sleep(LONG t)
{
	for(int i = 0; i < t; i++)
		cpu_relax();
}

LONG timer_get(void)
{
	return timer;
}
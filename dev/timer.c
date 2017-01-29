#include <base.h>
#include <port.h>
#include <int.h>

volatile LONG seconds;
volatile LONG ticks;
volatile LONG ticks2;

static void timer_handler(regs_t regs)
{
	ticks++;
	ticks2++;
	if(ticks == 100)
	{
		seconds++;
		ticks = 0;
	}
}

void timer_setup(void)
{
	puts("[timer] Timer setup...");
	ticks = 0;
	ticks2 = 0;
	seconds = 0;
	outb(0x43, 0x36);
	WORD divisor = 11931;
	BYTE l = divisor & 0xFF;
	BYTE h = (divisor >> 8) & 0xFF;
	outb(0x40, l);
	outb(0x40, h);
	int_regh(IRQ0, &timer_handler);
	puts("OK!\n");
}

void __sleep(LONG ticks)
{
	if(!ticks2) // won't sleep without PIT
		return;
    volatile LONG end = ticks2 + ticks;
    while(end > ticks2)
    {
        cpu_relax();
    }
}

void sleep(LONG ticks)
{
  __sleep(ticks);
}

LONG __ticks(void)
{
	return ticks2;
}

LONG time(void*p)
{
	return seconds;
}

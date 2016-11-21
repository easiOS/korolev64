#include <base.h>
#include <ports.h>
#include <text.h>
#include <int.h>
#include <dev/kbd.h>

kbd_event_t kbd_stack[512];

static void kbd_interrupt(regs_t regs)
{

}

void kbd_setup(void)
{
	puts("Keyboard setup...");
	int_regh(IRQ1, )
}
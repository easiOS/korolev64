#include <base.h>
#include <port.h>
#include <text.h>
#include <int.h>
#include <dev/kbd.h>

kbd_event_t kbd_stack[512];
kbd_event_t* kbd_stack_sp = &kbd_stack[511];

BYTE ctrl, alt, shift;

char kbd_keycode_default_map[] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u',
	'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f',
	'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '|', 'z', 'x',
	'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
	[74] = '-', [78] = '+', [127] = 0
};

char kbd_keycode_shift_default_map[] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
	'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S', 'D', 'F',
	'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '\'', 'Z', 'X',
	'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
	[74] = '-', [78] = '+', [127] = 0
};

char* kbd_keycode_map;
char* kbd_keycode_shift_map;

static void kbd_interrupt(regs_t regs)
{
	if(!(inb(0x64) & 1)) return;
	BYTE scancode;
	LONG a, spec, kc;
	scancode = inb(0x60);
	a = inb(0x61);
	spec = scancode == 0xE0;
	if(spec)
	{
		scancode = inb(0x60);
		a = inb(0x61);
		outb(0x61, a | 0x80);
		outb(0x61, a);
	}
	kc = scancode & ~0x80;
	if(kc == 0x2a || kc == 0x36)
	{
		shift = !(scancode & 0x80);
		goto end;
	}
	if(kc == 0x1d)
	{
		ctrl = !(scancode & 0x80);
		goto end;
	}
	if(kc == 0x38)
	{
		alt = !(scancode & 0x80);
		goto end;
	}
	kbd_stack_sp->keycode = kc;
	kbd_stack_sp->release = scancode & 0x80;
	kbd_stack_sp->ctrl = ctrl;
	kbd_stack_sp->alt = alt;
	kbd_stack_sp->shift = shift;
	kbd_stack_sp->special = spec;
	if(kbd_stack_sp != kbd_stack)
	{
		kbd_stack_sp--;
	}
	end:
	return;
}

void kbd_setup(void)
{
	puts("Keyboard setup...");
	kmemset(kbd_stack, 0, 512 * sizeof(kbd_event_t));
	ctrl = 0;
	alt = 0;
	shift = 0;
	int_regh(IRQ1, &kbd_interrupt);
	puts("OK!\n");
}

LONG kbd_avail(void)
{
	return kbd_stack_sp != &kbd_stack[511];
}

kbd_event_t kbd_pop(void)
{
	return *--kbd_stack_sp;
}

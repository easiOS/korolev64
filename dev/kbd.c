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
	BYTE scancode;
	LONG a, spec, kc;
	if(!(inb(0x64) & 1)) return;
	puts("Keyboard interrupt\n");
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
	// disable ports
	kbd_ps2_cmd(0xAD);
	kbd_ps2_cmd(0xA7);
	// flush output buffer
	kbd_ps2_flush_buf();
	if(!kbd_ps2_selftest())
	{
		puts("SELF TEST FAIL\n");
		return;
	}
	if(!kbd_ps2_testch(1))
	{
		puts("CH1 TEST FAIL\n");
		return;
	}
	// enable port 1
	kbd_ps2_cmd(0xAE);
	kbd_ps2_enable_irq();
	kbd_ps2_write_data(0xFF); // reset KBD
	ctrl = 0;
	alt = 0;
	shift = 0;
	int_regh(IRQ1, &kbd_interrupt);
	kbd_keycode_map = kbd_keycode_default_map;
	kbd_keycode_shift_map = kbd_keycode_shift_default_map;
	puts("OK!\n");
}

void kbd_reset(void)
{
	kbd_stack_sp = &kbd_stack[511];
}

LONG kbd_avail(void)
{
	return kbd_stack_sp != &kbd_stack[511];
}

kbd_event_t kbd_pop(void)
{
	return *(++kbd_stack_sp);
}

void kbd_ps2_cmd(BYTE cmd)
{
	while(inb(0x64) & 2);
	outb(0x64, cmd);
}

void kbd_ps2_flush_buf(void)
{
	inb(0x60);
}

BYTE kbd_ps2_read_data(void)
{
	while(~inb(0x64) & 1);
	return inb(0x60);
}

void kbd_ps2_write_data(BYTE val)
{
	while(inb(0x64) & 2);
	outb(0x60, val);
}

BYTE kbd_ps2_selftest(void)
{
	kbd_ps2_cmd(0xAA);
	return kbd_ps2_read_data() == 0x55;
}

BYTE kbd_ps2_testch(BYTE ch)
{
	switch(ch)
	{
		case 1:
			kbd_ps2_cmd(0xAB);
			return kbd_ps2_read_data() == 0;
		case 2:
			kbd_ps2_cmd(0xA9);
			return kbd_ps2_read_data() == 0;
	}
}

void kbd_ps2_enable_irq(void)
{
	kbd_ps2_cmd(0x60);
	kbd_ps2_write_data(0b01100111);
}

BYTE kbd_kc2ch(kbd_event_t kc)
{
	return 0;
}

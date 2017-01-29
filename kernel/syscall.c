#include <base.h>
#include <int.h>
#include <dev/kbd.h>
#include <text.h>

static void syscall_handler(regs_t regs)
{
	//puts_int("syscall!\n");
	//LONG a, b, c, d;
	char* e;
	kbd_event_t k;
	switch(regs.eax)
	{
		case 0: // print string
			e = (char*)regs.ebx;
			//while(*e)
			//	put(*e++);
			puts_int(e);
			break;
		case 1: // clear screen
			clear();
			break;
		case 2: // read keyboard (non-blocking, returns 0xffff if there's no available key)
			if(kbd_avail())
			{
				k = kbd_pop();
				regs.ebx = *((WORD*)(&k));
			}
			else
			{
				regs.ebx = 0xffff;
			}
			break;
		case 3: // read keyboard (blocking)
			while(!kbd_avail());
			put('K');
			k = kbd_pop();
			regs.ebx = k.keycode;
			regs.ecx = k.release | k.ctrl << 1 | k.alt << 2 | k.shift << 3 | k.special << 4;
			break;
		case 4: // read character from keyboard (blocking)
			break;
		//case 256: // sys_restart_syscall
		//case 257: // sys_exit
		//case 258: // sys_fork
		case 259: // sys_read
			break;
		case 260: // sys_write
			while(0);
			LONG c = regs.edx;
			char* s = (char*)regs.ecx;
			while(c--)
				put(*(s++));
			regs.eax = regs.edx;
			break;
	}
}

void syscall_setup(void)
{
	int_regh(127, syscall_handler);
}

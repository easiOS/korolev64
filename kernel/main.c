#include <base.h>
#include <runtime.h>
#include <text.h>
#include <int.h>
#include <multiboot.h>
#include <port.h>
#include <dev/pci.h>
#include <dev/timer.h>
#include <dev/kbd.h>

static void gpf(regs_t regs)
{
	//puts("========================\n");
	//puts("GENERAL PROTECTION FAULT\n");
	//puts("========================\n");
}

static void syscall_handler(regs_t regs)
{
	//puts("syscall!\n");
	LONG a, b, c, d;
	char* e;
	kbd_event_t k;
	switch(regs.eax)
	{
		case 0: // print string
			e = (char*)regs.ebx;
			while(*e)
				put(*e++);
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

	}
	put('S'); put('T'); put('O'); put('P'); 
}

void kmain(LONG magic, LONG address)
{
	if(magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		puts("Please reboot from a Multiboot-compliant bootloader.");
		return;
	}
	puts("Korolev64\n");
	int_setup();
	multiboot_process(address);
	timer_setup();
	pci_setup();
	kbd_setup();
	int_regh(127, syscall_handler);
	asm volatile("sti");
	if(!kssfs_avail())
	{
		puts("no disk available, halt\n");
		goto halt;
	}

	BYTE* init = (BYTE*)0x3D09000;

	if(!kssfs_read_file(init, "init"))
	{
		puts("no init found, halt\n");
		goto halt;
	}

	puts("found init\n");

	kbd_reset();

	kom_hdr_t* kom_hdr = (kom_hdr_t*)init;
	if(kom_hdr->signature[0] == 'K' && kom_hdr->signature[1] == 'O' && kom_hdr->signature[2] == 'M' && kom_hdr->signature[3] == '0')
	{
		puts("KOM executable\n");
		puts("Entry point: "); putn(kom_hdr->entry_point, 16); puts("\n");
		puts("Begin...\n");
		jmp_to_init(kom_hdr->entry_point);
	}
	else
	{
		puts("not a KOM executable: ");
		for(int i = 0; i < 4; i++)
			put(kom_hdr->signature[i]);
		puts("\n");
	}

	puts("attempted to kill init\n");

	halt:
	while(1);
}
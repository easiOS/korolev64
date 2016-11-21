#include <base.h>
#include <runtime.h>
#include <text.h>
#include <int.h>
#include <multiboot.h>
#include <port.h>
#include <dev/pci.h>
#include <dev/timer.h>

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
	switch(regs.eax)
	{
		case 0:
			e = (char*)regs.ebx;
			while(*e)
				put(*e++);
			break;
		case 1:
			clear();
			break;
	}
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
#include <base.h>
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

	puts("found init\n some bytes from the file:\n");

	for(int i = 0; i < 29; i++)
	{
		putn(init[i], 16);
		puts(" ");
	}

	jmp_to_init(init);

	puts("attempted to kill init\n");

	halt:
	while(1);
}
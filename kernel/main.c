#include <base.h>
#include <runtime.h>
#include <text.h>
#include <int.h>
#include <multiboot.h>
#include <port.h>
#include <dev/pci.h>
#include <dev/timer.h>
#include <dev/kbd.h>
#include <syscall.h>
#include <net/ethernet.h>
#include <net/network.h>

void kshell_main(void);

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
	syscall_setup();
	timer_setup();
	kbd_setup();
	ethernet_setup();
	pci_setup();
	asm volatile("sti");
	network_setup();
	/*if(!kssfs_avail())
	{
		puts("no disk available, halt\n");
		goto halt;
	}

	BYTE* init = (BYTE*)0x3D09000;

	if(!kssfs_read_file(init, "init"))
	{
		puts("no init found, kshell\n");
		kshell_main();
	}

	puts("found init\n");

	kbd_reset();

	kom_hdr_t* kom_hdr = (kom_hdr_t*)init;
	if(kom_hdr->signature[0] == 'K' && kom_hdr->signature[1] == 'O' && kom_hdr->signature[2] == 'M' && kom_hdr->signature[3] == '0')
	{
		puts("KOM executable\n");
		puts("Entry point: "); putn(kom_hdr->entry_point, 16); puts("\n");
		puts("Begin...\n\n");
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

	halt:*/
	while(1);
}
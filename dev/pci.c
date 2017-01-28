#include <base.h>
#include <dev/pci.h>
#include <port.h>
#include <text.h>
#include <int.h>

#include <dev/disk.h>
#include <dev/e1000.h>

pci_dev_t* pci_devices[] = {
	&pci_dev_disk,
	&pci_dev_e1000,
	NULL,
};

pci_int_entry pci_int_entries[256];
static void pci_irq_handler(regs_t regs);

void pci_setup(void)
{
	puts("Resetting PCI interrupt entries\n");
	kmemset(pci_int_entries, 0, 256 * sizeof(pci_int_entry));
	int_regh(10, &pci_irq_handler);
	puts("PCI enumeration...\n");
	for(BYTE bus = 0; bus < 255; bus++)
	{
		for(BYTE device = 0; device < 32; device++)
		{
			for(BYTE function = 0; function < 8; function++)
			{
				WORD vendor = pci_cfg_readw(bus, device, function, PCI_CFG_VEN);
				if(vendor == 0xffff)
					continue;

				WORD devid = pci_cfg_readw(bus, device, function, PCI_CFG_DEV);
				BYTE class = pci_cfg_readb(bus, device, function, PCI_CFG_CLA);
				BYTE sclass = pci_cfg_readb(bus, device, function, PCI_CFG_SCL);

				for(int j = 0; j < 256; j++)
				{
					pci_dev_t* dev = pci_devices[j];
					if(!dev)
						break;
					if(dev->ids_len == 33)
					{
						if(dev->class_code == class && dev->subclass == sclass)
						{
							puts("PCI device detected: \"");
							puts(dev->name);
							puts("\"...");
							if(dev->initf)
							{
								if(dev->initf(bus, device, function))
								{
									puts("failed.\n");
								}
								else
								{
									puts("OK.\n");
								}
							}
							else
							{
								puts("skipped.\n");
							}
						}
					}
					else
					{
						for(int i = 0; i < dev->ids_len; i++)
						{
							pci_devid_t* id = &dev->ids[i];
							if(id->vendor == vendor && id->device == devid)
							{
								puts("PCI device detected: ");
								puts(dev->name);
								puts("\n");
								if(dev->initf)
								{
									puts("PCI device detected: \"");
									puts(dev->name);
									puts("\"...");
									if(dev->initf)
									{
										if(dev->initf(bus, device, function))
										{
											puts("failed.\n");
										}
										else
										{
											puts("OK.\n");
										}
									}
									else
									{
										puts("skipped.\n");
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	puts("PCI enumeration OK!\n");
}

BYTE pci_cfg_readb(BYTE bus, BYTE dev, BYTE func, BYTE off)
{
	LONG addr;
	BYTE res;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	res = (BYTE)((inl(0xCFC) >> ((off & 2) * 8)) & 0xff);
	return res;
}

WORD pci_cfg_readw(BYTE bus, BYTE dev, BYTE func, BYTE off)
{
	LONG addr;
	WORD res;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	res = (WORD)((inl(0xCFC) >> ((off & 2) * 8)) & 0xffff);
	return res;
}

LONG pci_cfg_readl(BYTE bus, BYTE dev, BYTE func, BYTE off)
{
	LONG addr;
	LONG res;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	res = inl(0xCFC) >> ((off & 2) * 8);
	return res;
}

void pci_cfg_writeb(BYTE bus, BYTE dev, BYTE func, BYTE off, BYTE val)
{
	LONG addr;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	outb(PCI_PORT_CONF_DATA, val);
	io_wait();
}

void pci_cfg_writew(BYTE bus, BYTE dev, BYTE func, BYTE off, WORD val)
{
	LONG addr;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	outw(PCI_PORT_CONF_DATA, val);
	io_wait();
}

void pci_cfg_writel(BYTE bus, BYTE dev, BYTE func, BYTE off, LONG val)
{
	LONG addr;

	addr = (LONG)((bus << 16) | (dev << 11) | (func << 8) |
			(off & 0xFC) | ((LONG)0x80000000));
	outl(PCI_PORT_CONF_ADDR, addr);
	io_wait();
	outl(PCI_PORT_CONF_DATA, val);
	io_wait();
}

static void pci_irq_handler(regs_t regs)
{
	for(int i = 0; i < 256; i++)
	{
		pci_int_entry* ie = pci_int_entries + i;
		if(ie->priv == NULL)
		{
			continue;
		}
		
		LONG status, command, intl;
		status = pci_cfg_read_status(ie->bus, ie->device, ie->function);
		command = pci_cfg_read_command(ie->bus, ie->device, ie->function);
		intl = pci_cfg_read_intl(ie->bus, ie->device, ie->function);
		
		if(status & 8 && !(command & 1024) && intl == regs.int_no)
		// is interrupt status 1, are interrupts enabled, is the int line matching
		{
			ie->handler(regs, ie->priv);
		}
		else
			continue;
	}
}

int pci_int_request(LONG bus, LONG device, LONG func, void* dev_id, void (*handler)(regs_t regs, void* dev_id))
{
	for(int i = 0; i < 256; i++)
	{
		if(pci_int_entries[i].priv == NULL)
		{
			pci_int_entries[i].bus = bus;
			pci_int_entries[i].device = device;
			pci_int_entries[i].function = func;
			pci_int_entries[i].priv = dev_id;
			pci_int_entries[i].handler = handler;
			return 0;
		}
	}
	puts("[pci] out of interrupt handler entries\n");
	return 1;
}

void pci_int_release(void* dev_id)
{
	for(int i = 0; i < 256; i++)
	{
		if(pci_int_entries[i].priv == dev_id)
		{
			pci_int_entries[i].priv = NULL;
			pci_int_entries[i].bus = 0;
			pci_int_entries[i].device = 0;
			pci_int_entries[i].function = 0;
			pci_int_entries[i].handler = NULL;
		}
	}
}

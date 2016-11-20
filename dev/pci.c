#include <base.h>
#include <dev/pci.h>
#include <port.h>
#include <text.h>

#include <dev/disk.h>

pci_dev_t* pci_devices[] = {
	&pci_dev_disk,
	NULL,
};

void pci_setup(void)
{
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
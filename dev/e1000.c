// Intel Ethernet i217

#include <base.h>
#include <port.h>
#include <dev/pci.h>
#include <dev/e1000.h>
#include <net/ethernet.h>

#include <dev/timer.h>

#include <text.h>
#include <stdlib.h>

int e1000_init(LONG bus, LONG device, LONG function);

pci_dev_t pci_dev_e1000 = {
	"Intel Ethernet i217",
	&e1000_init,
	7,
	{{0x8086, 0x0438}, {0x8086, 0x043a}, {0x8086, 0x043c}, {0x8086, 0x1000}, {0x8086, 0x100e}, {0x8086, 0x100f},
	 {0x8086, 0x1004}},
	0x00, 0x00
};

BYTE e1000_mmio_read8(LONG addr)
{
	return *((volatile BYTE*)(addr));
}

WORD e1000_mmio_read16(LONG addr)
{
	return *((volatile WORD*)(addr));
}

LONG e1000_mmio_read32(LONG addr)
{
	return *((volatile LONG*)(addr));
}

uint64_t e1000_mmio_read64(LONG addr)
{
	return *((volatile uint64_t*)(addr));
}

void e1000_mmio_write8(LONG addr, BYTE val)
{
	(*((volatile BYTE*)(addr)))=(val);
}

void e1000_mmio_write16(LONG addr, WORD val)
{
	(*((volatile WORD*)(addr)))=(val);
}

void e1000_mmio_write32(LONG addr, LONG val)
{
	(*((volatile LONG*)(addr)))=(val);
}

void e1000_mmio_write64(LONG addr, uint64_t val)
{
	(*((volatile uint64_t*)(addr)))=(val);
}

void e1000_writecmd(ethdev* dev, uint16_t addr, uint32_t val)
{
	struct e1000_priv* p = dev->priv;
	if(p->bar_type == 0)
	{
		e1000_mmio_write32(p->memory + addr, val);
	}
	else
	{
		outl(p->iobase, addr);
		outl(p->iobase + 4, val);
	}
}

LONG e1000_readcmd(ethdev* dev, uint16_t addr)
{
	struct e1000_priv* p = dev->priv;
	if(p->bar_type == 0)
		return e1000_mmio_read32(p->memory + addr);
	else
	{
		outl(p->iobase, addr);
		return inl(p->iobase + 4);
	}
}

LONG e1000_detect_eeprom(ethdev* dev)
{
	struct e1000_priv* p = dev->priv;
	LONG val = 0;
	e1000_writecmd(dev, REG_EEPROM, 0x1);

	for(int i = 0; i < 1000 && !p->eeprom_exists; i++)
	{
		val = e1000_readcmd(dev, REG_EEPROM);
		if(val & 0x10)
			p->eeprom_exists = 1;
		else
			p->eeprom_exists = 0;
	}
	return p->eeprom_exists;
}

LONG e1000_eeprom_read(ethdev* dev, BYTE addr)
{
	struct e1000_priv* p = dev->priv;
	WORD data = 0;
	LONG tmp = 0;
	if(p->eeprom_exists)
	{
		e1000_writecmd(dev, REG_EEPROM, (1) | ((LONG)(addr) << 8));
		while(!((tmp = e1000_readcmd(dev, REG_EEPROM)) & (1 << 4)));
	}
	else
	{
		e1000_writecmd(dev, REG_EEPROM, (1) | ((LONG)(addr) << 2));
		while(!((tmp = e1000_readcmd(dev, REG_EEPROM)) & (1 << 1)));
	}
	data = (WORD)((tmp >> 16) & 0xFFFF);
	return data;
}

LONG e1000_read_mac(ethdev* dev)
{
	struct e1000_priv* p = dev->priv;
	if(p->eeprom_exists)
	{
		LONG tmp;
		tmp = e1000_eeprom_read(dev, 0);
		dev->addr[0] = tmp & 0xff;
		dev->addr[1] = tmp >> 8;
		tmp = e1000_eeprom_read(dev, 1);
		dev->addr[2] = tmp & 0xff;
		dev->addr[3] = tmp >> 8;
		tmp = e1000_eeprom_read(dev, 2);
		dev->addr[4] = tmp & 0xff;
		dev->addr[5] = tmp >> 8;
	}
	else
	{
		BYTE* membase_mac_8 = (BYTE*)(p->memory + 0x5400);
		LONG* membase_mac_32 = (LONG*)(p->memory + 0x5400);
		if(membase_mac_32[0] != 0)
		{
			for(int i = 0; i < 6; i++)
			{
				dev->addr[i] = membase_mac_8[i];
			}
		}
		else
			return 0;
	}
	return 1;
}

void e1000_enable_int(ethdev* dev)
{
	e1000_writecmd(dev, REG_IMASK, 0x1f6dc);
	e1000_writecmd(dev, REG_IMASK, 0xff & ~4);
	e1000_readcmd(dev, 0xc0);
}

void e1000_rxinit(ethdev* dev)
{
	struct e1000_priv* p = dev->priv;
	void* ptr;
	struct e1000_rx_desc* descs;
	puts("[e1000] setting up rx buffer\n");
	ptr = malloc(sizeof(struct e1000_rx_desc)*E1000_NUM_RX_DESC + 16);

	descs = (struct e1000_rx_desc*)ptr;
	for(int i = 0; i < E1000_NUM_RX_DESC; i++)
	{
		p->rx_descs[i] = (struct e1000_rx_desc*)((BYTE*)descs + i*16);
		p->rx_descs[i]->addr = (QWORD)(LONG)(BYTE*)malloc(8192 + 16);
		p->rx_descs[i]->status = 0;
	}

	e1000_writecmd(dev, REG_TXDESCLO, (LONG)ptr);
	e1000_writecmd(dev, REG_TXDESCHI, 0);

	e1000_writecmd(dev, REG_RXDESCLO, (QWORD)(LONG)ptr);
	e1000_writecmd(dev, REG_RXDESCHI, 0);

	e1000_writecmd(dev, REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

	e1000_writecmd(dev, REG_RXDESCHEAD, 0);
	e1000_writecmd(dev, REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
	p->rx_cur = 0;
	e1000_writecmd(dev, REG_RCTRL, RCTL_EN|RCTL_SBP|RCTL_UPE|RCTL_MPE|RCTL_LBM_NONE|RTCL_RDMTS_HALF|RCTL_SECRC|RCTL_BSIZE_2048);
}

void e1000_txinit(ethdev* dev)
{
	void* ptr;
	struct e1000_priv* p = dev->priv;
	struct e1000_tx_desc* descs;
	puts("[e1000] setting up tx buffer\n");
	ptr = malloc(sizeof(struct e1000_tx_desc)*E1000_NUM_TX_DESC + 16);

	descs = (struct e1000_tx_desc*)ptr;
	for(int i = 0; i < E1000_NUM_TX_DESC; i++)
	{
		p->tx_descs[i] = (struct e1000_tx_desc*)((BYTE*)descs + i*16);
		p->tx_descs[i]->addr = 0;
		p->tx_descs[i]->cmd = 0;
		p->tx_descs[i]->status = TSTA_DD;
	}

	e1000_writecmd(dev, REG_TXDESCHI, 0);
	e1000_writecmd(dev, REG_TXDESCLO, (LONG)ptr);

	e1000_writecmd(dev, REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

	e1000_writecmd(dev, REG_TXDESCHEAD, 0);
	e1000_writecmd(dev, REG_TXDESCTAIL, 0);
	p->tx_cur = 0;
	e1000_writecmd(dev, REG_TCTRL, TCTL_EN
		| TCTL_PSP
		| (15 << TCTL_CT_SHIFT)
		| (64 << TCTL_COLD_SHIFT)
		| TCTL_RTLC);

	e1000_writecmd(dev, REG_TCTRL, 0b0110000000000111111000011111010);
	e1000_writecmd(dev, REG_TIPG, 0x60200a);
}


int e1000_write(void* buf, size_t len, ethdev* dev)
{
	
	struct e1000_priv* p = dev->priv;
	//printf("e1000: sending frame from buf at 0x%x with length %x...", buf, len);

	p->tx_descs[p->tx_cur]->addr = (LONG)buf;
	p->tx_descs[p->tx_cur]->length = sizeof(ethhdr) + len + 4;
	p->tx_descs[p->tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS | 0x02; //0x02 = insert CRC
	p->tx_descs[p->tx_cur]->status = 0;
	BYTE old_cur = p->tx_cur;
	p->tx_cur = (p->tx_cur + 1) % E1000_NUM_TX_DESC;
	e1000_writecmd(dev, REG_TXDESCTAIL, p->tx_cur);
	int timeout = 100;
	while(!(p->tx_descs[old_cur]->status & 0xff))
	{
		sleep(1);
		timeout--;
		if(!timeout)
		{
			//puts("timed out, packet dropped\n");
			goto timeout;
		}
	}
	//puts("sent!\n");
	goto ok;

	timeout:
	len = 0;
	ok:
	return len;
}

LONG e1000_check_link(ethdev* dev)
{
	return dev->link_status = e1000_readcmd(dev, REG_STATUS) & 2;
}

void e1000_reset(ethdev* dev)
{
	/*LONG pbs, pba, ctrl, status;
	printf("e1000: device reset\n");

	//note: reset delay is 20ms

	ctrl = e1000_readcmd(dev, REG_CTRL);
	e1000_writecmd(dev, REG_CTRL, ctrl | 0x04000000); //send reset command
	sleep(20);

	//default config
//	ctrl |= ()*/
}

void e1000_receive(ethdev* dev)
{
	struct e1000_priv* p = dev->priv;
	WORD old_cur;

	BYTE* buf;
	WORD len;

	while((p->rx_descs[p->rx_cur]->status & 1))
	{
		// TODO: send packet to layer 2
		puts("[e1000] packet received\n");

		p->rx_descs[p->rx_cur]->status = 0;
		old_cur = p->rx_cur;
		p->rx_cur = (p->rx_cur + 1) % E1000_NUM_RX_DESC;
		e1000_writecmd(dev, REG_RXDESCTAIL, old_cur);
	}
}

static void e1000_handler(regs_t regs, void* dev_id)
{
	ethdev* dev = dev_id;
	struct e1000_priv* p = dev->priv;
	LONG status = e1000_readcmd(dev, CMD_ICR);

	if(status & 0x80)
	{
		e1000_receive(dev);
	}
}

int e1000_init(LONG bus, LONG device, LONG function)
{
	ethdev* dev;
	struct e1000_priv* priv;
	LONG bar0;
	WORD cmd;

	puts("[e1000] Initialization\n");

	dev = ethernet_allocate();
	if(!dev)
	{
		puts("[e1000] cannot allocate ethdev\n");
		return 1;
	}
	priv = dev->priv = malloc(sizeof(struct e1000_priv));

	priv->bus = bus;
	priv->device = device;
	priv->function = function;

	bar0 = pci_cfg_read_bar(bus, device, function, 0);
	priv->bar_type = bar0 & 1;
	priv->memory = bar0 & 0xFFFFFFF0;
	priv->iobase = bar0 & 0xFFFFFFFC;
	
	priv->eeprom_exists = 0;

	puts("[e1000] enabling busmastering\n");
	cmd = pci_cfg_read_command(bus, device, function);
	cmd |= (1 << 2);
	pci_cfg_write_command(bus, device, function, cmd);

	puts("[e1000] detecting eeprom: ");
	puts(e1000_detect_eeprom(dev) ? "yes\n" : "no\n");

	puts("[e1000] fetching MAC address\n");
	if(!e1000_read_mac(dev))
	{
		puts("[e1000] cannot read MAC addr\n");
		free(priv);
		ethernet_free(dev);
		return 1;
	}

	for(int i = 0; i<0x80; i++)
		e1000_writecmd(dev, 0x5200 + i * 4, 0);

	pci_cfg_writeb(bus, device, function, PCI_CFG_INT, 11);
	pci_int_request(bus, device, function, dev, &e1000_handler);

	e1000_enable_int(dev);
	e1000_rxinit(dev);
	e1000_txinit(dev);

	LONG tctl;
	tctl = e1000_readcmd(dev, REG_TCTRL);
	tctl &= ~((0xff << 4) | (0x3ff << 12));
	tctl |= (TCTL_EN | TCTL_PSP | (0x0f << 4) | (0x40 << 12));
	e1000_writecmd(dev, REG_TCTRL, tctl);

	LONG rctl;
	rctl = e1000_readcmd(dev, REG_RCTRL);
	//rctl &= ~(RCTL_BSIZE_4096);
	rctl |= (RCTL_EN | /*RCTL_UPE | RCTL_MPE |*/ RCTL_BAM | RCTL_BSIZE_8192 | RCTL_SECRC);
	e1000_writecmd(dev, REG_RCTRL, rctl);

	e1000_writecmd(dev, REG_CTRL, ECTRL_SLU | ECTRL_ASDE); //set link up, activate auto-speed detection
	puts("[e1000] waiting for link\n");
	while(!e1000_check_link(dev));

	dev->write = &e1000_write;

	puts("[e1000] device ready\n");
	puts("[e1000] MAC address: ");
	for(int i = 0; i < 6; i++)
	{
		putn(dev->addr[i], 16);
		puts(":");
	}
	puts("\n");

	return 0;
}

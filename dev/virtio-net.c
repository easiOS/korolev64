#include <base.h>
#include <text.h>
#include <port.h>
#include <dev/pci.h>
#include <stdlib.h>
#include <string.h>
#include <net/ethernet.h>
#include <dev/virtio-net.h>

pci_dev_t pci_dev_virtio_net = {
	.name = "VirtIO-net",
	.initf = &virtionet_init,
	.ids_len = 1,
	.ids = {{0x1af4, 0x1000},},
	.class_code = 0, .subclass = 0
};

void virtionet_initialize_queue(ethdev* dev)
{
	struct virtionet_priv* p = dev->priv;

	puts("[vnet] vqueue\n");

	p->qsize = VNET_QSIZE;

	void* ptr = malloc(
		VNET_DTABLE_SIZE(VNET_QSIZE) + VNET_ALIGN +
		VNET_ARING_SIZE(VNET_QSIZE) + VNET_ALIGN +
		VNET_URING_SIZE(VNET_QSIZE) + VNET_ALIGN);

	puts("[vnet] vring: 0x"); putn16(ptr);

	LONG pmod = (LONG)ptr % 4096;
	if(pmod != 0)
		ptr += (4096 - pmod);

	 puts(" aligned: 0x"); putn16(ptr); puts("\n");

	 p->rx_desc = ptr;
	 p->rx_avail = ptr + VNET_DTABLE_SIZE(VNET_QSIZE);
	 p->rx_used = (void*)(((LONG)&p->rx_avail->ring[32] + sizeof(WORD)
	 	+ VNET_ALIGN-1)
	 	& ~(VNET_ALIGN - 1));

	 struct virtionet_desc* desc = p->rx_desc;
	 for(int i = 0; i < VNET_QSIZE / 2; i++)
	 {
	 	desc->flags = ~VNET_DESC_WRITE;
	 	desc->addr = (QWORD)malloc(4096);
	 	desc->len = 4096;
	 }
}

void virtionet_write_dstatus(ethdev* dev, BYTE val)
{
	struct virtionet_priv* p = dev->priv;
	outb(p->iobase + VNET_R_DSTAT, val);
}

LONG virtionet_read_features(ethdev* dev)
{
	struct virtionet_priv* p = dev->priv;
	return inl(p->iobase);
}

void virtionet_write_features(ethdev* dev, LONG val)
{
	struct virtionet_priv* p = dev->priv;
	outl(p->iobase + VNET_R_GFEAT, val);
}

LONG virtionet_check_link(ethdev* dev)
{
	return 0; // TODO
}

LONG virtionet_is_broadcast(ethdev* dev)
{
	return 0; // TODO
}

LONG virtionet_is_multicast(ethdev* dev)
{
	return 0; // TODO
}

LONG virtionet_is_promisc(ethdev* dev)
{
	return 0; // TODO
}

void virtionet_drvfail(LONG iobase)
{
	outb(iobase + VNET_R_DSTAT, VNET_S_DRVFAIL);
}

LONG virtionet_read_reg(ethdev* dev, LONG reg)
{
	struct virtionet_priv* p = dev->priv;
	return inl(p->iobase + reg);
}

void virtionet_load_macaddr(ethdev* dev)
{
	struct virtionet_priv* p = dev->priv;
	for(int i = 0; i < 6; i++)
		dev->addr[i] = inb(p->iobase + VNET_R_MACAD + i);
}

int virtionet_init(LONG bus, LONG device, LONG function)
{
	ethdev* dev;
	struct virtionet_priv* priv;
	LONG iobase;

	puts("[vnet] Initialization\n");

	iobase = pci_cfg_read_bar(bus, device, function, 0);
	if(iobase & 1)
	{
		iobase &= 0xfffffffe;
	}
	else
	{
		puts("[vnet] BAR0 is not IO port\n");
		return 1;
	}

	WORD subsys = pci_cfg_readw(bus, device, function, PCI_CFG_SSS);
	if(subsys != 1)
	{
		puts("[vnet] virtio device is not network interface\n");
		virtionet_drvfail(iobase);
		return 1;
	}

	dev = ethernet_allocate();
	if(!dev)
	{
		puts("[vnet] cannot allocate ethdev\n");
		virtionet_drvfail(iobase);
		return 1;
	}
	dev->allocated = 1;
	priv = dev->priv = malloc(sizeof(struct virtionet_priv));
	kmemset(priv, 0, sizeof(struct virtionet_priv));

	priv->iobase = iobase;
	puts("[vnet] IO base: "); putn16(iobase); put('\n');

	virtionet_write_dstatus(dev, VNET_S_DEVACK);
	virtionet_write_dstatus(dev, VNET_S_DEVACK | VNET_S_DRVLOAD);

	LONG feat = virtionet_read_features(dev);
	feat &= ~(VNET_F_CTRL_VQ | VNET_F_EVENT_IDX | VNET_F_GUEST_TSO4 |
		VNET_F_GUEST_TSO6 | VNET_F_GUEST_UFO | VNET_F_MRG_RXBUF);
	feat |= VNET_F_CSUM;
	virtionet_write_features(dev, feat);

	virtionet_write_dstatus(dev, VNET_S_DEVACK | VNET_S_DRVLOAD | VNET_S_FEATOK);

	if((virtionet_read_reg(dev, VNET_R_DSTAT) & VNET_S_FEATOK) == 0)
	{
		puts("[vnet] device has disagreed with the requested features\n");
		return;
	}
	else
	{
		puts("[vnet] device has accepted the requested features\n");
	}

	virtionet_load_macaddr(dev);
	virtionet_initialize_queue(dev);

	dev->is_up = &virtionet_check_link;
	dev->is_broadcast = &virtionet_is_broadcast;
	dev->is_multicast = &virtionet_is_multicast;
	dev->is_promisc = &virtionet_is_promisc;



	return 0;
}
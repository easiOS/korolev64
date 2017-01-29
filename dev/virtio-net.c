#include <base.h>
#include <text.h>
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

	 p->desc = ptr;
	 p->avail = ptr + VNET_DTABLE_SIZE(VNET_QSIZE);
	 p->used = (void*)(((LONG)&p->avail->ring[32] + sizeof(WORD)
	 	+ VNET_ALIGN-1)
	 	& ~(VNET_ALIGN - 1));
}

void virtionet_write_dstatus(ethdev* dev, LONG val)
{

}

LONG virtionet_read_features(ethdev* dev)
{

}

void virtionet_write_features(ethdev* dev, LONG val)
{
	
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

int virtionet_init(LONG bus, LONG device, LONG function)
{
	ethdev* dev;
	struct e1000_priv* priv;

	puts("[vnet] Initialization\n");

	dev = ethernet_allocate();
	if(!dev)
	{
		puts("[vnet] cannot allocate ethdev\n");
		return 1;
	}
	dev->allocated = 1;
	priv = dev->priv = malloc(sizeof(struct virtionet_priv));
	kmemset(priv, 0, sizeof(struct virtionet_priv));

	LONG bar0 = pci_cfg_read_bar(bus, device, function, 0);

	puts("[vnet] bar0: "); putn16(bar0); put('\n');

	virtionet_initialize_queue(dev);

	dev->is_up = &virtionet_check_link;
	dev->is_broadcast = &virtionet_is_broadcast;
	dev->is_multicast = &virtionet_is_multicast;
	dev->is_promisc = &virtionet_is_promisc;

	return 0;
}
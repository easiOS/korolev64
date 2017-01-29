#include <base.h>
#include <text.h>
#include <net/ethernet.h>
#include <net/network.h>

netdev* net_ifs;

LONG net_eth_counter = 0;
const char* net_ethernet_ifname = "eth";

void network_setup(void)
{
	puts("[network] setup\n");
	net_ifs = malloc(NET_MAX_IFS * sizeof(netdev));
	kmemset(net_ifs, 0, NET_MAX_IFS * sizeof(netdev));

	for(int i = 0; i < ETH_MAX_IFS; i++)
	{
		ethdev* pdev = ethernet_getdev(i);
		if(pdev == NULL)
			continue;
		char buf[32];
		const char* map = "0123456789abcdef";
		char buf2[2];
		buf2[1] = 0;

		netdev* dev = netdev_allocate();
		strncpy(buf, net_ethernet_ifname, 32);
		strncpy(dev->name, buf, 32);
		dev->iftype = NET_IF_ETHERNET;
		dev->physical = pdev;
		netdev_set(dev, 1);
		puts("[");
		puts(dev->name);
		puts("] waiting for link\n");
		LONG s = __ticks();
		while(!pdev->is_up(pdev))
		{
			if(__ticks() - s > 100)
			{
				netdev_set(dev, 0);
				break;
			}
		}

		netdev_print(dev);
	}
}

netdev* netdev_allocate(void)
{
	for(int i = 0; i < ETH_MAX_IFS; i++)
	{
		if(net_ifs[i].allocated)
			continue;
		net_ifs[i].allocated = 1;
		return net_ifs + i;
	}
	return NULL;
}

void netdev_free(netdev* dev)
{
	if(dev->addresses)
		netdev_addr_free(dev->addresses);
	kmemset(dev, 0, sizeof(netdev));
}

void netdev_addr_free(netdev_addr* addr)
{
	if(addr->next)
	{
		netdev_addr_free(addr->next);
	}
	free(addr);
}

LONG netdev_get_mtu(netdev* dev)
{
	switch(dev->iftype)
	{
		case NET_IF_ETHERNET:
		case NET_IF_VSWITCH:
			return ((ethdev*)dev->physical)->mtu;
	}
	return 0;
}

void put_hwaddr(BYTE* b)
{
	for(int i = 0; i < 6; i++)
	{
		if(!b[i])
			put('0');
		putn16(b[i]);
		if(i != 5)
			put(':');
	}
}

LONG netdev_get_status(netdev* dev)
{
	LONG status = 0;
	void* phys = dev->physical;
	switch(dev->iftype)
	{
		case NET_IF_ETHERNET:
		case NET_IF_VSWITCH:
			if(((ethdev*)phys)->is_up)
				if(((ethdev*)phys)->is_up(phys))
					status |= NET_IF_STA_LOWER_UP;
			if(((ethdev*)phys)->is_broadcast)
				if(((ethdev*)phys)->is_broadcast(phys))
					status |= NET_IF_STA_BROADCAST;
			if(((ethdev*)phys)->is_multicast)
				if(((ethdev*)phys)->is_multicast(phys))
					status |= NET_IF_STA_MULTICAST;
			if(((ethdev*)phys)->is_promisc)
				if(((ethdev*)phys)->is_promisc(phys))
					status |= NET_IF_STA_PROMISC;
	}

	if(dev->up)
		status |= NET_IF_STA_UP;
	if(dev->master)
		status |= NET_IF_STA_SLAVE;

	return status;
}

void netdev_print(netdev* dev)
{
	char buffer[64];
	LONG status;

	status = netdev_get_status(dev);

	puts(dev->name); puts(": <");
	if(status & NET_IF_STA_UP)
		puts("UP");
	if(status & NET_IF_STA_BROADCAST)
		puts(",BROADCAST");
	if(status & NET_IF_STA_MULTICAST)
		puts(",MULTICAST");
	if(status & NET_IF_STA_PROMISC)
		puts(",PROMISC");
	if(status & NET_IF_STA_SLAVE)
		puts(",SLAVE");
	if(status & NET_IF_STA_LOWER_UP)
		puts(",LOWER_UP");
	puts("> mtu "); putn10(netdev_get_mtu(dev));
	puts(" state "); puts(status & NET_IF_STA_UP ? "UP" : "DOWN");
	puts("\n    link/");
	switch(dev->iftype)
	{
		case NET_IF_ETHERNET:
		case NET_IF_VSWITCH:
			puts("ether ");
			put_hwaddr(((ethdev*)dev->physical)->addr);
			break;
		default:
			puts("unknown");
	}
	puts("\n");
}

LONG netdev_set(netdev* dev, LONG state)
{
	dev->up = state;
	return dev->up;
}

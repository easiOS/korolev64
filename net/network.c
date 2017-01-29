#include <base.h>
#include <text.h>
#include <stdlib.h>
#include <string.h>
#include <dev/timer.h>
#include <net/ethernet.h>
#include <net/network.h>

netdev* net_ifs;

LONG net_eth_counter = 0;
LONG net_vsw_counter = 0;
const char* net_ethernet_ifname = "eth";
const char* net_vswitch_ifname = "sw";

int network__disable = 0;

void network_disable(void)
{
	network__disable = 1;
}

void network_setup(void)
{
	if(network__disable)
		return;
	puts("[network] setup\n");
	net_ifs = malloc(NET_MAX_IFS * sizeof(netdev));
	kmemset(net_ifs, 0, NET_MAX_IFS * sizeof(netdev));

	network_create_loopback();

	for(int i = 0; i < ETH_MAX_IFS; i++)
	{
		ethdev* pdev = ethernet_getdev(i);
		if(pdev == NULL)
			continue;

		netdev* dev = netdev_allocate();

		strncpy(dev->name, net_ethernet_ifname, 32);
		dev->iftype = NET_IF_ETHERNET;
		dev->physical = pdev;
		netdev_set(dev, 1);
		puts("[");
		puts(dev->name);
		puts("] waiting for link...");
		LONG s = 3;
		while(!pdev->is_up(pdev) && s)
		{
			sleep(10);
			s--;
		}

		if(s == 0xffffffff)
		{
			puts("timed out\n");
			continue;
		}
		else
		{
			puts("online\n");
		}

		netdev_add_linklocal(dev);
		
	}

	for(int i = 0; i < NET_MAX_IFS; i++)
	{
		netdev* dev = net_ifs + i;
		if(dev->allocated)
			netdev_print(dev);
	}
}

netdev_addr* netdev_add_ip6_address(netdev* dev, BYTE* address, LONG prefix, LONG attr)
{
	netdev_addr* addr = malloc(sizeof(netdev_addr));
	kmemset(addr, 0, sizeof(netdev_addr));

	memcpy(addr->addr, address, 16);

	LONG fb = prefix / 8;
	LONG rb = prefix - fb * 8;
	for(int i = 0; i < fb; i++)
	{
		addr->mask[i] = 0xff;
	}
	if(rb)
		addr->mask[fb] = (1 << (8 - rb)) - 1;
	addr->addrtype = NET_ADDR_IPV6;
	addr->attr = attr;

	netdev_addr* last_addr = NULL;

	if(dev->addresses)
	{
		last_addr = dev->addresses;
		while(last_addr->next)
		{
			last_addr = last_addr->next;
		}
		last_addr->next = addr;
	}
	else
	{
		dev->addresses = addr;
	}
	return addr;
}

void netdev_add_linklocal(netdev* dev)
{
	ethdev* ethdev = dev->physical;
	BYTE addr[16];
	kmemset(addr, 0, 16);
	addr[0] = 0x80;
	addr[1] = 0xfe;
	addr[10] = ethdev->addr[1];
	addr[11] = ethdev->addr[0];
	addr[12] = ethdev->addr[3];
	addr[13] = ethdev->addr[2];
	addr[14] = ethdev->addr[5];
	addr[15] = ethdev->addr[4];

	netdev_add_ip6_address(dev, addr, 64, NET_ADDR_IPV6_LOCAL);
}

void network_create_loopback(void)
{
	puts("[network] creating loopback\n");
	netdev* dev = netdev_allocate();
	strncpy(dev->name, "lo", 32);

	netdev_set(dev, 1);
	dev->iftype = NET_IF_LOOPBACK;

	BYTE addr[16];
	kmemset(addr, 0, 16);
	addr[14] = 1;

	netdev_add_ip6_address(dev, addr, 64, NET_ADDR_IPV6_LOOP);
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
	netdev_addr* addr = dev->addresses;
	while(addr)
	{
		switch(addr->addrtype)
		{
			case NET_ADDR_IPV6:
			{
				puts("\n    inet6 ");
				network_print_ipv6(addr);
				puts(" scope ");
				switch(addr->attr)
				{
					case NET_ADDR_IPV6_UNI:
						puts("global\n");
						break;
					case NET_ADDR_IPV6_LOCAL:
						puts("local\n");
						break;
					case NET_ADDR_IPV6_LOOP:
						puts("host\n");
						break;
				}
				break;
			}
			default:
				break;
		}		

		addr = addr->next;
	}
	puts("\n");
}

LONG netdev_set(netdev* dev, LONG state)
{
	dev->up = state;
	return dev->up;
}

void network_print_ipv6(netdev_addr* addr)
{
	WORD* waddr = (WORD*)addr->addr;
	BYTE* mask = addr->mask;
	int omit = 0;
	for(int i = 0; i < 8; i++)
	{
		if(waddr[i] == 0 && omit == 0)
		{
			omit = 1;
			continue;
		}
		if(waddr[i] == 0 && omit == 1 && i == 7)
		{
			put(':');
			break;
		}
		if(waddr[i] == 0 && omit == 1)
			continue;
		if(waddr[i] != 0 && omit == 1)
		{
			put(':');
			omit = 2;
		}
		putn16(waddr[i]);
		if(i != 7)
			put(':');
	}
	LONG prefix = 0;
	for(int i = 0; i < 16; i++)
	{
		if(mask[i] == 0xff)
			prefix += 8;
		else if(mask[i] == 0x00)
			break;
		else
		{
			BYTE b = mask[i];
			while(b)
			{
				prefix++;
				b >>= 1;
			}
			break;
		}
	}
	put('/'); putn10(prefix);
}

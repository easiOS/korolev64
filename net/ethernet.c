#include <base.h>
#include <net/ethernet.h>
#include <text.h>
#include <string.h>
#include <stdlib.h>

ethdev* eth_ifs;

void ethernet_setup(void)
{
	puts("[ethernet] setup\n");
	eth_ifs = malloc(ETH_MAX_IFS * sizeof(ethdev));
	kmemset(eth_ifs, 0, ETH_MAX_IFS * sizeof(ethdev));
}

ethdev* ethernet_allocate(void)
{
	for(int i = 0; i < ETH_MAX_IFS; i++)
	{
		if(eth_ifs[i].allocated)
		{
			continue;
		}
		eth_ifs[i].allocated = 1;
		eth_ifs[i].mtu = 1500;
		return eth_ifs + i;
	}
	return NULL;
}

void ethernet_free(ethdev* dev)
{
	kmemset(dev, 0, sizeof(ethdev));
}

ethdev* ethernet_getdev(int index)
{
	if(index >= ETH_MAX_IFS)
		return NULL;
	if(!eth_ifs[index].allocated)
		return NULL;
	return eth_ifs + index;
}

#include <base.h>
#include <net/ethernet.h>
#include <text.h>
#include <string.h>

ethdev eth_ifs[ETH_MAX_IFS];

void ethernet_setup(void)
{
	puts("[ethernet] setup\n");
	kmemset(eth_ifs, 0, ETH_MAX_IFS * sizeof(ethdev));
}

ethdev* ethernet_allocate(void)
{
	for(int i = 0; i < ETH_MAX_IFS; i++)
	{
		if(eth_ifs[i].allocated)
			continue;
		eth_ifs[i].allocated = 1;
		return eth_ifs + i;
	}
	return NULL;
}

void ethernet_free(ethdev* dev)
{
	kmemset(dev, 0, sizeof(ethdev));
}

#ifndef H_NET_ETHERNET
#define H_NET_ETHERNET

#include <base.h>

#define ETH_MAX_IFS	16

typedef struct ethdev ethdev;

struct ethdev {
	LONG allocated;
	void* priv;
	BYTE addr[6];
	LONG link_status;
	LONG mtu;
	int (*write)(void* src, size_t len, ethdev* dev);
	LONG (*is_up)(ethdev* dev);
	LONG (*is_broadcast)(ethdev* dev);
	LONG (*is_multicast)(ethdev* dev);
	LONG (*is_promisc)(ethdev* dev);
};

void ethernet_setup(void);
ethdev* ethernet_allocate(void);
void ethernet_free(ethdev* dev);
ethdev* ethernet_getdev(int index);

#endif /* H_NET_ETHERNET */

#ifndef H_NET_ETHERNET
#define H_NET_ETHERNET

#include <base.h>

#define ETH_MAX_IFS	16

#define ETH_P_IPV6	0x86DD

typedef struct {
	BYTE dest[6];
	BYTE src[6];
	WORD protocol;
} __attribute__((packed)) ethhdr;

typedef struct ethdev ethdev;

struct ethdev {
	LONG allocated;
	void* priv;
	BYTE addr[6];
	LONG link_status;
	LONG mtu;
	int (*write)(void* src, size_t len, ethdev* dev);
	LONG (*is_link_up)(ethdev* dev);
};

void ethernet_setup(void);
ethdev* ethernet_allocate(void);
void ethernet_free(ethdev* dev);

#endif /* H_NET_ETHERNET */

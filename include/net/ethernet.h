#ifndef H_NET_ETHERNET
#define H_NET_ETHERNET

#define ETH_P_IPV6	0x86DD

typedef struct {
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t protocol;
} __attribute__((packed)) ethhdr;

#endif /* H_NET_ETHERNET */

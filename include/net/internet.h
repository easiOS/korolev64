#ifndef H_NET_INTERNET
#define H_NET_INTERNET

#define INET_P_TCP	0x06
#define INET_P_UDP	0x11
#define INET_P_ICMP	0x3A

typedef struct {
	uint32_t version : 4;
	uint32_t tc : 8;
	uint32_t flow : 20;
	uint16_t len;
	uint8_t nxthdr;
	uint8_t ttl;
} __attribute__((packed)) ip6hdr;

#endif /* H_NET_INTERNET */

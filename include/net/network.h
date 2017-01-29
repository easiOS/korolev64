#ifndef H_NET_INTERNET
#define H_NET_INTERNET

#define NET_MAX_IFS 16

#define ETH_P_IPV6	0x86DD

typedef struct {
	BYTE dest[6];
	BYTE src[6];
	WORD protocol;
} __attribute__((packed)) ethhdr;

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

typedef struct {
	ethhdr* l2;
	ip6hdr* l3;
	void* l4;
} network_buf;

#define NET_ADDR_UNDEFINED		0
#define NET_ADDR_IPV6			1

#define NET_ADDR_IPV6_UNI		0
#define NET_ADDR_IPV6_LOCAL		1
#define NET_ADDR_IPV6_LOOP		2

#define NET_IF_UNDEFINED		0
#define NET_IF_ETHERNET			1
#define NET_IF_VSWITCH			2
#define NET_IF_LOOPBACK			3

#define NET_IF_STA_UP			(1 << 0)
#define NET_IF_STA_BROADCAST	(1 << 1)
#define NET_IF_STA_MULTICAST	(1 << 2)
#define NET_IF_STA_PROMISC		(1 << 3)
#define NET_IF_STA_SLAVE		(1 << 4)
#define NET_IF_STA_LOWER_UP		(1 << 5)

typedef struct {
	LONG addrtype; // NET_ADDR_*
	BYTE addr[16];
	BYTE mask[16];
	LONG attr; // NET_ADDR_[type]_*
	void* next;
} netdev_addr;

typedef struct {
	LONG allocated;

	char name[32]; // name of the interface
	LONG up;
	LONG iftype; // NET_IF_*
	netdev_addr* addresses;
	void* physical; // ethdev, etc.
	void* master;
} netdev;

void network_setup(void);
netdev* netdev_allocate(void);
void netdev_free(netdev* dev);
LONG netdev_get_status(netdev* dev);
LONG netdev_set(netdev* dev, LONG state);
void netdev_add_linklocal(netdev* dev);

void netdev_addr_free(netdev_addr* addr);

void network_create_loopback(void);

void network_disable(void);
void netdev_print(netdev* dev);
void network_print_ipv6(netdev_addr* addr);

#endif /* H_NET_INTERNET */

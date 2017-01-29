#ifndef H_DEV_VNET_NET
#define H_DEV_VNET_NET

int virtionet_init(LONG bus, LONG device, LONG function);

extern pci_dev_t pci_dev_virtio_net;

#define VNET_QSIZE				2
#define VNET_ALIGN				4096

#define VNET_DTABLE_SIZE(n)		(16 * n)
#define VNET_ARING_SIZE(n)		(6 * 2 * n)
#define VNET_URING_SIZE(n)		(6 * 4 * n)

struct virtionet_desc {
	QWORD addr;
	LONG len;

	#define VNET_DESC_NEXT	1
	#define VNET_DESC_WRITE	2
	#define VNET_DESC_INDIR	4
	WORD flags;
	WORD next;
} __attribute__((packed));

struct virtionet_avail {
	#define VNET_AVAIL_NO_INT	1
	WORD flags;
	WORD idx;
	WORD ring[VNET_QSIZE];
	WORD used_event;
} __attribute__((packed));

struct virtionet_used_elem {
	LONG id;
	LONG len;
} __attribute__((packed));

struct virtionet_used {
	#define VNET_USED_NO_NOTIFY	1
	WORD flags;
	WORD idx;
	struct virtionet_used_elem ring[VNET_QSIZE];
	WORD avail_event;
} __attribute__((packed));

struct virtionet_priv {
	LONG bus, device, function;
	LONG qsize;
	void* buf_rx; // where the receive queue resides
	struct virtionet_desc* rx_desc;
	struct virtionet_avail* rx_avail;
	struct virtionet_used* rx_used;
	void* buf_tx; // where the receive queue resides
	struct virtionet_desc* tx_desc;
	struct virtionet_avail* tx_avail;
	struct virtionet_used* tx_used;
	LONG iobase;
};


/* The standard layout for the ring is a continuous chunk of memory which
 * looks like this.  We assume num is a power of 2.
 *
 * struct vring {
 *      // The actual descriptors (16 bytes each)
 *      struct vring_desc desc[num];
 *
 *      // A ring of available descriptor heads with free-running index.
 *      le16 avail_flags;
 *      le16 avail_idx;
 *      le16 available[num];
 *      le16 used_event_idx; // Only if VNET_RING_F_EVENT_IDX
 *
 *      // Padding to the next align boundary.
 *      char pad[];
 *
 *      // A ring of used descriptor heads with free-running index.
 *      le16 used_flags;
 *      le16 used_idx;
 *      struct vring_used_elem used[num];
 *      le16 avail_event_idx; // Only if VNET_RING_F_EVENT_IDX
 * };
 * Note: for virtio PCI, align is 4096.
 */

#define VNET_F_CSUM			(1 << 0)
#define VNET_F_GCSUM		(1 << 1)
#define VNET_F_CTRL_G_OFF	(1 << 2)
#define VNET_F_MAC			(1 << 5)
#define VNET_F_GUEST_TSO4	(1 << 7)
#define VNET_F_GUEST_TSO6	(1 << 8)
#define VNET_F_GUEST_UFO	(1 << 10)
#define VNET_F_MRG_RXBUF	(1 << 15)
#define VNET_F_STATUS		(1 << 16)
#define VNET_F_CTRL_VQ		(1 << 17)
#define VNET_F_CTRL_RX		(1 << 18)
#define VNET_F_CTRL_VL		(1 << 19)
#define VNET_F_G_ANNO		(1 << 21)
#define VNET_F_EVENT_IDX	(1 << 29)

#define VNET_S_DEVACK		0x01
#define VNET_S_DRVLOAD		0x02
#define VNET_S_DRVOK		0x04
#define VNET_S_FEATOK		0x08
#define VNET_S_DRVFAIL		0x80

#define VNET_R_DFEAT 0x00
#define VNET_R_GFEAT 0x04
#define VNET_R_QADDR 0x08
#define VNET_R_QSIZE 0x0C
#define VNET_R_QSELE 0x0E
#define VNET_R_QNOTI 0x10
#define VNET_R_DSTAT 0x12
#define VNET_R_ISRST 0x13
#define VNET_R_MACAD 0x14

#endif /* H_DEV_VIRTIO_NET */

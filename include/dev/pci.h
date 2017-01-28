#ifndef H_DEV_PCI
#define H_DEV_PCI

#include <base.h>
#include <int.h>

#define PCI_PORT_CONF_ADDR 0xCF8
#define PCI_PORT_CONF_DATA 0xCFC

#define PCI_CFG_VEN 0x00
#define PCI_CFG_DEV 0x02
#define PCI_CFG_CMD 0x04
#define PCI_CFG_STA 0x06
#define PCI_CFG_REV 0x08
#define PCI_CFG_PIF 0x09
#define PCI_CFG_SCL 0x0A
#define PCI_CFG_CLA 0x0B
#define PCI_CFG_HDR 0x0E
#define PCI_CFG_BR0 0x10
#define PCI_CFG_BR1 0x14
#define PCI_CFG_BR2 0x18
#define PCI_CFG_BR3 0x1C
#define PCI_CFG_BR4 0x20
#define PCI_CFG_BR5 0x24
#define PCI_CFG_INT 0x3C

// Pointer to a function with arguments: bus, device, function
typedef int (*pci_initf_t)(LONG, LONG, LONG);

typedef struct {
	WORD vendor;
	WORD device;
} pci_devid_t;

// PCI device descriptor
// name is the device's human-readable name
// ids_len is the length of the ids array (count, not bytes)
// if this is 33, the class-subclass values will be used
// ids is the array containing all the IDs a device is identifiable by
// initf is the function that initializes the device

typedef struct {
	char name[64];
	pci_initf_t initf;
	LONG ids_len;
	pci_devid_t ids[32];
	BYTE class_code;
	BYTE subclass;
} pci_dev_t;

typedef struct {
	void* priv;
	void (*handler)(regs_t regs, void* dev_id);
	LONG bus, device, function;
} pci_int_entry;

void pci_setup(void);

BYTE pci_cfg_readb(BYTE bus, BYTE dev, BYTE func, BYTE off);
WORD pci_cfg_readw(BYTE bus, BYTE dev, BYTE func, BYTE off);
LONG pci_cfg_readl(BYTE bus, BYTE dev, BYTE func, BYTE off);
void pci_cfg_writeb(BYTE bus, BYTE dev, BYTE func, BYTE off, BYTE val);
void pci_cfg_writew(BYTE bus, BYTE dev, BYTE func, BYTE off, WORD val);
void pci_cfg_writel(BYTE bus, BYTE dev, BYTE func, BYTE off, LONG val);

#define pci_cfg_read_status(bus, dev, func) pci_cfg_readw(bus, dev, func, PCI_CFG_STA)
#define pci_cfg_read_command(bus, dev, func) pci_cfg_readw(bus, dev, func, PCI_CFG_CMD)
#define pci_cfg_read_intl(bus, dev, func) pci_cfg_readb(bus, dev, func, PCI_CFG_INT)

#endif /* H_DEV_PCI */
#ifndef H_DEV_DISK
#define H_DEV_DISK

#include <stdint.h>
#include <dev/pci.h>

struct chs_addr {
	BYTE head, sector, cylinder;
} __attribute__((packed));

struct mbr_pe {
	BYTE status;
	struct chs_addr begin;
	BYTE type;
	struct chs_addr end;
	LONG lba;
	LONG sectors;
} __attribute__((packed));

struct mbr {
	BYTE code[446];
	struct mbr_pe partitions[4];
	WORD signature; // 0xAA55 in memory, 0x55AA on disk
}__attribute__((packed));

struct guid {
	LONG data1;
	WORD data2, data3;
	LONG data4, data5;
} __attribute__((packed));

struct gpt_hdr {
	QUAD signature; //gpt signature (0x5452415020494645 little-endian)
	LONG revision; //gpt revision
	LONG hdrsiz; //header size
	LONG crc32; //crc32 of header
	LONG zero; //must be zero
	QUAD curlba; //current lba
	QUAD bcklba; //header backup lba
	QUAD ftulba; //first usable lba
	QUAD ltulba; //last usable lba
	struct guid disk_guid; //disk GUID
	QUAD pealba; //partition entry array lba
	LONG nope; //number of partition entries
	LONG pesiz; //partition entry size
	LONG peacrc; //partition entry array crc32
	BYTE reserved[420]; //must be all zero
} __attribute__((packed));

struct gpt_pe {
	struct guid ptype; 	//EasiOS 0.3 Partition GUID:
	                   	//6054BBB2-E732-4645-85CA-B58CC586C7D7
	struct guid pguid; //partition guid
	QUAD startlba; //start of partition
	QUAD lastlba; //end of partition (inclusive)
	QUAD flags; //flags
	BYTE pname[72]; //partition name UTF-16LE
} __attribute__((packed));

struct eos_drives { //physical or virtual partition using the EOS initrd filesystem 
	char letter;
	BYTE type; //0 = physical, 1 = virtual, 2 = FAT32 Physical
	union {
		struct {
			LONG lba;
			LONG size; //sectors
		} phys;
		LONG virt;
	} address;
};

struct ide_device {
   BYTE  Reserved;    // 0 (Empty) or 1 (This Drive really exists).
   BYTE  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
   BYTE  Drive;       // 0 (Master Drive) or 1 (Slave Drive).
   WORD Type;        // 0: ATA, 1:ATAPI.
   WORD Signature;   // Drive Signature
   WORD Capabilities;// Features.
   LONG   CommandSets; // Command Sets Supported.
   LONG   Size;        // Size in Sectors.
   BYTE  Model[41];   // Model in string.
};

struct IDEChannelRegisters {
   WORD base;  // I/O Base.
   WORD ctrl;  // Control Base
   WORD bmide; // Bus Master IDE
   BYTE  nIEN;  // nIEN (No Interrupt);
};

//ATA STATUS

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Inlex
#define ATA_SR_ERR     0x01    // Error

//ATA ERROR CODES

#define ATA_ER_BBK      0x80    // Bad sector
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // No media
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // No media
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

//ATA COMMANDS

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01
 
// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

void disk_init();
int ide_init(LONG bus, LONG device, LONG function);
int ide_read_sector(int LBA, void* ide_buf, int count, int slavebit);
int ide_write_sector(int LBA, void* ide_buf, int count, int slavebit);

extern pci_dev_t pci_dev_disk;

#endif /* H_DEV_DISK */
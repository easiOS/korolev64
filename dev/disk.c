#include <base.h>
#include <port.h>
#include <dev/pci.h>
#include <dev/disk.h>
#include <dev/timer.h>
#include <int.h>
#include <text.h>
#include <stdlib.h>
#include <string.h>

int ide_init(LONG bus, LONG device, LONG function);

BYTE ide_buffer[8192];

pci_dev_t pci_dev_disk = {
	"PCI IDE Controller",
	&ide_init,
	33,
	{{0, 0}},
	0x01, 0x01
};

struct IDEChannelRegisters channels[2];

struct ide_device ide_devices[4];

struct eos_drives drives[4] = {0};

BYTE ide_buf[2048] = {0};
unsigned static char ide_irq_invoked = 0;
uint32_t package[2];

BYTE ide_read(BYTE channel, BYTE reg);
void ide_write(BYTE channel, BYTE reg, BYTE data);
void ide_read_buffer(BYTE channel, BYTE reg, LONG buffer,
                     LONG quads);
BYTE ide_polling(BYTE channel, LONG advanced_check);
BYTE ide_print_error(LONG drive, BYTE err);
void ide_read_sectors(BYTE drive, BYTE numsects, LONG lba,
                      //WORD es, LONG edi);
						LONG out);
int ide_get_first_hdd();
void ide_detect_partitions();

int ide_init(LONG bus, LONG device, LONG function)
{
	kmemset(drives, 0, sizeof(struct eos_drives) * 4);
	for(int i = 0; i < 4; i++)
	{
		if(ide_devices[i].Reserved)
		{
			return 0;
		}
	}
	int i, j, k, count = 0;
	//Get BAR values
	LONG bar0, bar1, bar2, bar3, bar4;
	bar0 = pci_cfg_readl(bus, device, 0, PCI_CFG_BR0);
  	bar1 = pci_cfg_readl(bus, device, 0, PCI_CFG_BR1);
  	bar2 = pci_cfg_readl(bus, device, 0, PCI_CFG_BR2);
  	bar3 = pci_cfg_readl(bus, device, 0, PCI_CFG_BR3);
  	bar4 = pci_cfg_readl(bus, device, 0, PCI_CFG_BR4);
  	//Set I/O ports
	channels[ATA_PRIMARY  ].base  = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
   	channels[ATA_PRIMARY  ].ctrl  = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
   	channels[ATA_SECONDARY].base  = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
   	channels[ATA_SECONDARY].ctrl  = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
   	channels[ATA_PRIMARY  ].bmide = (bar4 & 0xFFFFFFFC) + 0; // Bus Master IDE
   	channels[ATA_SECONDARY].bmide = (bar4 & 0xFFFFFFFC) + 8; // Bus Master IDE
   	puts("\nI/O ports set\n");
   	//Disable IRQs
   	//ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
   	//ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
   	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, channels[ATA_PRIMARY].nIEN = (ide_irq_invoked = 0x0) + 0x02);
   	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, channels[ATA_SECONDARY].nIEN = (ide_irq_invoked = 0x0) + 0x02);
   	puts("IRQs disabled\n");
   	//Detect ATA/ATAPI devices
   	for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++) {
         BYTE err = 0, type = IDE_ATA, status;
         ide_devices[count].Reserved = 0; // Assuming that no drive here.
 
         // (I) Select Drive:
         ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
         sleep(1); // Wait 1ms for drive select to work.
 
         // (II) Send ATA Identify Command:
         ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
         sleep(1); // This function should be implemented in your OS. which waits for 1 ms.
                   // it is based on System Timer Device Driver.
         // (III) Polling:
         if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.
         while(1) {
            status = ide_read(i, ATA_REG_STATUS);
            if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
         }
 
         // (IV) Probe for ATAPI Devices:
 
         if (err != 0) {
            BYTE cl = ide_read(i, ATA_REG_LBA1);
            BYTE ch = ide_read(i, ATA_REG_LBA2);
 
            if (cl == 0x14 && ch ==0xEB)
               type = IDE_ATAPI;
            else if (cl == 0x69 && ch == 0x96)
               type = IDE_ATAPI;
            else
               continue; // Unknown Type (may not be a device).
 
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            sleep(1);
         }
         // (V) Read Identification Space of the Device:
         ide_read_buffer(i, ATA_REG_DATA, (LONG) ide_buf, 128);
         // (VI) Read Device Parameters:
         WORD* signature = (WORD*)(ide_buf + ATA_IDENT_DEVICETYPE), *capabilities = (WORD*)(ide_buf + ATA_IDENT_CAPABILITIES);
         LONG* cmdsets = (LONG*)(ide_buf + ATA_IDENT_COMMANDSETS);
         ide_devices[count].Reserved     = 1;
         ide_devices[count].Type         = type;
         ide_devices[count].Channel      = i;
         ide_devices[count].Drive        = j;
         ide_devices[count].Signature    = *signature;
         ide_devices[count].Capabilities = *capabilities;
         ide_devices[count].CommandSets  = *cmdsets;
 
         // (VII) Get Size:
         if (ide_devices[count].CommandSets & (1 << 26))
            // Device uses 48-Bit Addressing:
            ide_devices[count].Size   = *((LONG *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
         else
            // Device uses CHS or 28-bit Addressing:
            ide_devices[count].Size   = *((LONG *)(ide_buf + ATA_IDENT_MAX_LBA));
 
         // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
         for(k = 0; k < 40; k += 2) {
            ide_devices[count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
            ide_devices[count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];}
         ide_devices[count].Model[40] = 0; // Terminate String.
         count++;
      }
 	puts("Devices detected\n");
    ide_detect_partitions();
    return 0;
}

BYTE ide_read(BYTE channel, BYTE reg) {
   BYTE result = 0;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inb(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channels[channel].ctrl  + reg - 0x0A);
   else if (reg < 0x16)
      result = inb(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
   return result;
}

void ide_write(BYTE channel, BYTE reg, BYTE data) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      outb(channels[channel].base  + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channels[channel].base  + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channels[channel].ctrl  + reg - 0x0A, data);
   else if (reg < 0x16)
      outb(channels[channel].bmide + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

void ide_read_buffer(BYTE channel, BYTE reg, LONG buffer,
                     LONG quads) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   asm("pushw %es; pushw %ax; movw %ds, %ax; movw %ax, %es; popw %ax;"); 
   if (reg < 0x08)
      insl(channels[channel].base  + reg - 0x00, (void*)buffer, quads);
   else if (reg < 0x0C)
      insl(channels[channel].base  + reg - 0x06, (void*)buffer, quads);
   else if (reg < 0x0E)
      insl(channels[channel].ctrl  + reg - 0x0A, (void*)buffer, quads);
   else if (reg < 0x16)
      insl(channels[channel].bmide + reg - 0x0E, (void*)buffer, quads);
   asm("popw %es;");
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

BYTE ide_polling(BYTE channel, LONG advanced_check) {
 
   // (I) Delay 400 nanosecond for BSY to be set:
   // -------------------------------------------------
   for(int i = 0; i < 4; i++)
      ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.
 
   // (II) Wait for BSY to be cleared:
   // -------------------------------------------------
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
      ; // Wait for BSY to be zero.
 
   if (advanced_check) {
      BYTE state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.
 
      // (III) Check For Errors:
      // -------------------------------------------------
      if (state & ATA_SR_ERR)
         return 2; // Error.
 
      // (IV) Check If Device fault:
      // -------------------------------------------------
      if (state & ATA_SR_DF)
         return 1; // Device Fault.
 
      // (V) Check DRQ:
      // -------------------------------------------------
      // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
      if ((state & ATA_SR_DRQ) == 0)
         return 3; // DRQ should be set
 
   }
 
   return 0; // No Error.
 
}

BYTE ide_print_error(LONG drive, BYTE err) {
   if (err == 0)
      return err;
 
   puts("IDE:");
   if (err == 1) {puts("- Device Fault\n     "); err = 19;}
   else if (err == 2) {
      BYTE st = ide_read(ide_devices[drive].Channel, ATA_REG_ERROR);
      if (st & ATA_ER_AMNF)   {puts("- No Address Mark Found\n     ");   err = 7;}
      if (st & ATA_ER_TK0NF)   {puts("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_ABRT)   {puts("- Command Aborted\n     ");      err = 20;}
      if (st & ATA_ER_MCR)   {puts("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_IDNF)   {puts("- ID mark not Found\n     ");      err = 21;}
      if (st & ATA_ER_MC)   {puts("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_UNC)   {puts("- Uncorrectable Data Error\n     ");   err = 22;}
      if (st & ATA_ER_BBK)   {puts("- Bad Sectors\n     ");       err = 13;}
   } else  if (err == 3)           {puts("- Reads Nothing\n     "); err = 23;}
     else  if (err == 4)  {puts("- Write Protected\n     "); err = 8;}
 
   return err;
}

BYTE ide_ata_access(BYTE direction, BYTE drive, LONG lba, 
                             BYTE numsects, LONG out) {
	BYTE lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
 	BYTE lba_io[6];
   	LONG  channel      = ide_devices[drive].Channel; // Read the Channel.
   	LONG  slavebit      = ide_devices[drive].Drive; // Read the Drive [Master/Slave]
   	LONG  bus = channels[channel].base; // Bus Base, like 0x1F0 which is also data port.
   	LONG  words      = 256; // Almost every ATA drive has a sector-size of 512-byte.
   	WORD cyl, i;
   	BYTE head, sect, err;

   	ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);
   	   // (I) Select one from LBA28, LBA48 or CHS;
   if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are
                            // giving a wrong LBA.
      // LBA48:
      lba_mode  = 2;
      lba_io[0] = (lba & 0x000000FF) >> 0;
      lba_io[1] = (lba & 0x0000FF00) >> 8;
      lba_io[2] = (lba & 0x00FF0000) >> 16;
      lba_io[3] = (lba & 0xFF000000) >> 24;
      lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
   } else if (ide_devices[drive].Capabilities & 0x200)  { // Drive supports LBA?
      // LBA28:
      lba_mode  = 1;
      lba_io[0] = (lba & 0x00000FF) >> 0;
      lba_io[1] = (lba & 0x000FF00) >> 8;
      lba_io[2] = (lba & 0x0FF0000) >> 16;
      lba_io[3] = 0; // These Registers are not used here.
      lba_io[4] = 0; // These Registers are not used here.
      lba_io[5] = 0; // These Registers are not used here.
      head      = (lba & 0xF000000) >> 24;
   } else {
      // CHS:
      lba_mode  = 0;
      sect      = (lba % 63) + 1;
      cyl       = (lba + 1  - sect) / (16 * 63);
      lba_io[0] = sect;
      lba_io[1] = (cyl >> 0) & 0xFF;
      lba_io[2] = (cyl >> 8) & 0xFF;
      lba_io[3] = 0;
      lba_io[4] = 0;
      lba_io[5] = 0;
      head      = (lba + 1  - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
   }
   dma = 0;
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if busy.
   if (lba_mode == 0)
      ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head); // Drive & CHS.
   else
      ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA
  if (lba_mode == 2) {
      ide_write(channel, ATA_REG_SECCOUNT1,   0);
      ide_write(channel, ATA_REG_LBA3,   lba_io[3]);
      ide_write(channel, ATA_REG_LBA4,   lba_io[4]);
      ide_write(channel, ATA_REG_LBA5,   lba_io[5]);
   }
   ide_write(channel, ATA_REG_SECCOUNT0,   numsects);
   ide_write(channel, ATA_REG_LBA0,   lba_io[0]);
   ide_write(channel, ATA_REG_LBA1,   lba_io[1]);
   ide_write(channel, ATA_REG_LBA2,   lba_io[2]);
   if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;   
   if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;   
   if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
   if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
   if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
   ide_write(channel, ATA_REG_COMMAND, cmd);               // Send the Command.
   if (dma)
      if (direction == 0);
         // DMA Read.
      else;
         // DMA Write.
   else
      if (direction == 0)
         // PIO Read.
      for (i = 0; i < numsects; i++) {
         if ((err = ide_polling(channel, 1)))
            return err; // Polling, set error and exit if there is.
        insw(bus, (void*)out, words);
        out += (words * 2);
        /*
         asm("pushw %es");
         asm("mov %%ax, %%es" : : "a"(selector));
         asm("rep insw" : : "c"(words), "d"(bus), "D"(edi)); // Receive Data.
         asm("popw %es");
         edi += (words*2);*/
      } else {
      // PIO Write.
         for (i = 0; i < numsects; i++) {
            ide_polling(channel, 0); // Polling.
            //asm("pushw %ds");
            //asm("mov %%ax, %%ds"::"a"(selector));
            //asm("rep outsw"::"c"(words), "d"(bus), "S"(edi)); // Send Data
            //asm("popw %ds");
            //edi += (words*2);
         }
         ide_write(channel, ATA_REG_COMMAND, (char []) {   ATA_CMD_CACHE_FLUSH,
                        ATA_CMD_CACHE_FLUSH,
                        ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
         ide_polling(channel, 0); // Polling.
      }
 
   return 0; // Easy, isn't it?
}

void ide_read_sectors(BYTE drive, BYTE numsects, LONG lba,
                      LONG out) {
 
   // 1: Check if the drive presents:
   // ==================================
   if (drive > 3 || ide_devices[drive].Reserved == 0) package[0] = 0x1;      // Drive Not Found!
 
   // 2: Check if inputs are valid:
   // ==================================
   else if (((lba + numsects) > ide_devices[drive].Size) && (ide_devices[drive].Type == IDE_ATA))
      package[0] = 0x2;                     // Seeking to invalid position.
 
   // 3: Read in PIO Mode through Polling & IRQs:
   // ============================================
   else {
      BYTE err = 0;
      if (ide_devices[drive].Type == IDE_ATA)
         //err = ide_ata_access(ATA_READ, drive, lba, numsects, es, edi);
      	err = ide_ata_access(ATA_READ, drive, lba, numsects, out);
      /*else if (ide_devices[drive].Type == IDE_ATAPI)
         for (i = 0; i < numsects; i++)
            err = ide_atapi_read(drive, lba + i, 1, es, edi + (i*2048));*/
      package[0] = ide_print_error(drive, err);
   }
}

int ide_get_first_hdd()
{
	for(int i = 0; i < 4; i++)
	{
		if(ide_devices[i].Reserved)
		{
			if(ide_devices[i].Type == IDE_ATA)
			{
				return i;
			}
		}
	}
	return -1;
}

int ide_read_sector(int LBA, void* ide_buf, int count, int slavebit) {
   int stat;

   outb(0x3f6, 0x02);   //disable interrupts

   outb(0x1F6, (0xE0 | (slavebit <<  4) | (LBA >> 24 & 0x0F)));

   //waste some time
   outb(0x1f1, 0x00);

   //set the sector count
   outb(0x1f2, (BYTE)count);
   sleep(1);
   stat = ide_polling(0, 0);
      if(stat != 0)
         return stat;
   //send the low 8 bits of lbs to 1f3
   outb(0x1f3, (BYTE)LBA);
   sleep(1);
   stat = ide_polling(0, 0);
      if(stat != 0)
         return stat;
   //send the middle 8 bits to 1f4
   outb(0x1f4, (BYTE)(LBA >> 8));
   sleep(1);
   stat = ide_polling(0, 0);
      if(stat != 0)
         return stat;
   //send the high 8 to 1f5
   outb(0x1f5, (BYTE)(LBA >> 16));
   sleep(1);
   stat = ide_polling(0, 0);
   if(stat != 0)
      return stat;
   //issue a read sectors command
   outb(0x1f7, 0x20);
   sleep(1);
   stat = ide_polling(0, 0);
      if(stat != 0)
         return stat;

   //eat 256 words form the buffer
   insw(0x1f0, ide_buf, 256);
   
   ide_polling(0,0);
   return 0;
}

int ide_write_sector(int LBA, void* outarray, int count, int slavebit)  {
   int stat=0;

   outb(0x3f6, 0x02);   //disable interrupts

   outb(0x1F6, (0xE0 | (slavebit <<  4) | (LBA >> 24 & 0x0F)));

   //waste some time
   outb(0x1f1, 0x00);

   //set the sector count
   outb(0x1f2, (BYTE)count);
   sleep(1);
   stat = ide_polling(0, 1);
      if(stat == 1 || stat ==2)
         puts("error after 1f2  ");
   //send the low 8 bits of lbs to 1f3
   outb(0x1f3, (BYTE)LBA);
   sleep(1);
   stat = ide_polling(0,1);
      if(stat == 1 || stat ==2)
         puts("error after 1f3  ");
   //send the middle 8 bits to 1f4
   outb(0x1f4, (BYTE)(LBA >> 8));
   sleep(1);
   stat = ide_polling(0, 1);
      if(stat == 1 || stat ==2)
         puts("error after 1f4  ");
   //send the high 8 to 1f5
   outb(0x1f5, (BYTE)(LBA >> 16));
   sleep(1);
   stat = ide_polling(0, 1);
   if(stat == 1 || stat ==2)
         puts("error after 1f5  ");
   //issue a WRITE sectors command
   outb(0x1f7, 0x30);
   sleep(1);
   stat = ide_polling(0, 1);
      if(stat == 1 || stat ==2)
         puts("error after write command  ");

   //output 256 words form the buffer
   
   outsw(0x1f0,outarray, 256);

   stat = ide_polling(0, 1);
      if(stat == 1 || stat ==2)
         puts("error after write data  ");

   outb(0x1f7, 0xE7); //flush the cache after each write command

   ide_polling(0,1);
   return 0;
}

void ide_detect_partitions()
{
   puts("Searching partitions\n");
	uint8_t mbrbuffer[512];
	//uint8_t partbuffer[512];
   //Check for MBR
	struct mbr* mbr = (struct mbr*)mbrbuffer;
    ide_read_sector(0, mbr, 1, 0);
    if(mbr->signature != 0xAA55)
    {
    	puts("No MBR found\n");
    	return;
    }
   	for(int i = 0; i < 4; i++)
   	{
        //printf("Partition type: 0x%x, lba: 0x%x, size: 0x%x\n", mbr->partitions[i].type, mbr->partitions[i].lba, mbr->partitions[i].sectors);
        if(mbr->partitions[i].type == 0x7e)
   		{
   			puts("kssfs partition found\n");
   			//printf("Partition found %d at LBA %d (size: %d) FAT32\n", i, mbr->partitions[i].lba, mbr->partitions[i].sectors);
   			//tf_info.type = 1;
   			int di = -1;
   			for(int j = 0;j < 4; j++)
   			{
   				if(drives[j].letter == 0)
   				{
   					di = j;
   					break;
   				}
   			}
   			if(di == -1)
   			{
   				puts("  Cannot add more drives\n");
   				return;
   			}
   			drives[di].letter = 'a' + di;
   			drives[di].type = 2;
   			drives[di].address.phys.lba = mbr->partitions[i].lba;
   			drives[di].address.phys.size = mbr->partitions[i].sectors;
            kssfs_init(mbr->partitions[i].lba);
   		}
   	}
}
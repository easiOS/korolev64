// IDE driver
// Only native-PCI mode is supported
#include <base.h>
#include <text.h>
#include <dev/ide.h>
#include <dev/pci.h>
#include <dev/timer.h>
#include <int.h>

pci_dev_t pci_dev_disk = {
   "PCI IDE Controller",
   &ide_init,
   33,
   {{0, 0}},
   0x01, 0x01
};

LONG ide_pch_base, ide_pch_ctrl, ide_pch_bm;

int ide_channel_setnative(LONG bus, LONG device, LONG function, LONG channel)
{
   LONG pif, chmask;
   
   puts("[ide] setting channel op mode\n");
   chmask = (channel == 2) ? 4 : 1;
   pif = pci_cfg_read_pif(bus, device, function);

   if(!(pif & chmask))
   {
      puts("[ide] channel is in compat mode\n");
      pci_cfg_writeb(bus, device, function, PCI_CFG_PIF, pif | 1);
      sleep(1);
      pif = pci_cfg_read_pif(bus, device, function);
      if(!(pif & chmask))
      {
         puts("[ide] channel does not support native mode\n");
         return 1;
      }
   }
   puts("[ide] channel is in native mode\n");
   return 0;
}

void ide_reg_write(LONG reg, LONG data)
{
   reg &= 0xff;
   if(reg > 0x07 && reg < 0x0C)
   {
      //ide_reg_write(ATA_PRIMARY, ATA_REG_CONTROL, 0x80 | )
   }
}

static void ide_handle_int(regs_t regs, void* dev_id)
{
   if(dev_id != 0xf0f0f0f0)
      return;
   puts("[ide] interrupt!\n");
}

static void ide_handle_irq(regs_t regs)
{
   ide_handle_int(regs, 0xf0f0f0f0);
}

void ide_enable_int(LONG bus, LONG device, LONG function)
{
   LONG intl;
   pci_cfg_write_intl(bus, device, function, 0xFE);
   intl = pci_cfg_read_intl(bus, device, function);
   if((intl & 0xff) == 0xfe)
   {
      pci_cfg_write_intl(bus, device, function, IRQ10);
      if(pci_int_request(bus, device, function, 0xf0f0f0f0, &ide_handle_int))
         return;
      puts("[ide] interrupt line set to IRQ10\n");
   }
   else
   {
      int_regh(IRQ14, &ide_handle_irq);
      int_regh(IRQ15, &ide_handle_irq);
      puts("[ide] ye olde parallel\n");
   }
}

int ide_init(LONG bus, LONG device, LONG function)
{
   LONG bar0, bar1, bar4;
   puts("[ide] init\n");
   
   if(ide_channel_setnative(bus, device, function, 1))
      return 1;
   ide_channel_setnative(bus, device, function, 2);
   puts("[ide] channel 2 won't be used\n");

   bar0 = pci_cfg_read_bar(bus, device, function, 0);
   bar1 = pci_cfg_read_bar(bus, device, function, 1);
   bar4 = pci_cfg_read_bar(bus, device, function, 4);

   ide_pch_base = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
   ide_pch_ctrl = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
   ide_pch_bm = (bar4 & 0xFFFFFFFC);

   ide_enable_int(bus, device, function);
   return 0;   
}

int ide_read_sector(int LBA, void* ide_buf, int count, int slavebit)
{
   return 0;
}

int ide_write_sector(int LBA, void* outarray, int count, int slavebit)
{
   return 0;
}

#include <base.h>
#include <text.h>
#include <dev/disk.h>
#include <fs/kssfs.h>
#include <string.h>

LONG kssfs_lba = 0;
BYTE kssfs_buf[16384];
LONG kssfs_clstr_index = 0;
LONG kssfs_cluster_n = 0;
LONG kssfs_err = 0;
kssfs_fn_cache_t kssfs_fn_cache[32];

kssfs_header* kssfs_hdr = (kssfs_header*)kssfs_buf;
kssfs_clstr_hdr* kssfs_clstr = (kssfs_clstr_hdr*)kssfs_buf;

int kssfs_init(LONG lba)
{
	if(kssfs_lba)
		return 0;
	kssfs_lba = lba;
	puts("[kssfs] init: reading fs header\n");
	if(ide_read_sector(kssfs_lba, kssfs_buf, 1, 0))
	{
		puts("[kssfs] init: read error while reading kssfs_header\n");
		kssfs_lba = 0;
		return 0;
	}
	if(kssfs_hdr->signature != KSSFS_SIGNATURE)
	{
		puts("[kssfs] init: signature is incorrect\n");
		kssfs_lba = 0;
		return 0;
	}
	kssfs_cluster_n = kssfs_hdr->cluster_n;
	puts("[kssfs] init: header is correct\n");
	kmemset(kssfs_fn_cache, 0, 32 * sizeof(kssfs_fn_cache_t));
	kmemset(kssfs_buf, 0, 8192);
	kssfs_clstr_index = 0xffffffff;
	puts("[kssfs] init: OK\n");
	return 1;
}

void kssfs_read_cluster(LONG n)
{
	if(!kssfs_lba)
		return;
	if(n > kssfs_cluster_n)
		return;
	if(n == kssfs_clstr_index)
		return;
	LONG startc = KSSFS_CLSTR_LBA(n);
	for(int i = 0; i < 16; i++)
	{
		ide_read_sector(startc + i, kssfs_buf + 512 * i, 1, 0);
	}
	kssfs_clstr_index = n;
}

void kssfs_write_cluster(LONG n, BYTE* buf)
{
	if(!kssfs_lba)
		return;
	if(n > kssfs_cluster_n)
		return;
	LONG startc = KSSFS_CLSTR_LBA(n);
	for(int i = 0; i < 16; i++)
	{
		ide_write_sector(startc + i, buf + i * 512, 1, 0);
	}
}

int kssfs_read_file(void* dest, char* filename)
{
	if(!kssfs_lba)
		return 0;
	/*for(int i = 0; i < 32; i++)
	{
		if(strncmp(kssfs_fn_cache[i].filename, filename, 256) == 0)
		{
			kssfs_read_cluster(kssfs_fn_cache[i].cluster);
			memcpy(dest, kssfs_buf + 512, 7680);
			return 1;
		}
	}*/
	for(int i = 0; i < kssfs_cluster_n; i++)
	{
		kssfs_read_cluster(i);
		if(strncmp((char*)kssfs_clstr->filename, filename, 256) == 0)
		{
			memcpy(dest, kssfs_clstr->data, kssfs_clstr->len);
			return 1;
		}
	}
	return 0;
}

LONG kssfs_avail(void)
{
	return (LONG)kssfs_buf;
}

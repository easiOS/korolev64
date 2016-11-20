#include <base.h>
#include <text.h>
#include <dev/disk.h>
#include <fs/kssfs.h>

LONG kssfs_lba = 0;
BYTE kssfs_buf[16384];
LONG kssfs_clstr_index = 0;
LONG kssfs_cluster_n = 0;
LONG kssfs_err = 0;
kssfs_fn_cache_t kssfs_fn_cache[32];

kssfs_header* kssfs_hdr = (kssfs_header*)kssfs_buf;
kssfs_clstr_hdr* kssfs_clstr = (kssfs_clstr_hdr*)kssfs_buf;

void kssfs_init(LONG lba)
{
	if(kssfs_lba)
		return;
	kssfs_lba = lba;
	puts("[kssfs] init: reading fs header\n");
	if(ide_read_sector(kssfs_lba, kssfs_buf, 1, 0))
	{
		puts("[kssfs] init: read error while reading kssfs_header\n");
		kssfs_lba = 0;
		return;
	}
	if(kssfs_hdr->signature != KSSFS_SIGNATURE)
	{
		puts("[kssfs] init: signature is incorrect\n");
		kssfs_lba = 0;
		return;
	}
	kssfs_cluster_n = kssfs_hdr->cluster_n;
	putn(kssfs_hdr->cluster_n, 10);
	puts("\n");
	puts("[kssfs] init: header is correct\n");
	kmemset(kssfs_fn_cache, 0, 32 * sizeof(kssfs_fn_cache_t));
	kmemset(kssfs_buf, 0, 8192);
	kssfs_clstr_index = 0xffffffff;
	puts("[kssfs] init: OK\n");
}

void kssfs_read_cluster(LONG n)
{
	if(!kssfs_lba)
		return;
	if(n > kssfs_cluster_n)
		return;
	if(n == kssfs_clstr_index)
		return;
	ide_read_sector(KSSFS_CLSTR_LBA(n), kssfs_buf, 16, 0);
	kssfs_clstr_index = n;
}

void kssfs_write_cluster(LONG n, BYTE* buf)
{
	if(!kssfs_lba)
		return;
	if(n > kssfs_cluster_n)
		return;
	ide_write_sector(KSSFS_CLSTR_LBA(n), buf, 16, 0);
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
		puts("read_file: \"");
		puts(kssfs_clstr->filename);
		puts("\"\n");
		if(strncmp(kssfs_clstr->filename, filename, 256) == 0)
		{
			memcpy(dest, kssfs_clstr->data, kssfs_clstr->len);
			for(int i = 0; i < 128; i++)
			{
				putn(kssfs_clstr->data[i], 16);
				puts(" ");
			}
			return 1;
		}
	}
	return 0;
}

LONG kssfs_avail(void)
{
	return (kssfs_buf);
}

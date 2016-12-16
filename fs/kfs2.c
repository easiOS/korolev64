#include <base.h>
#include <text.h>
#include <dev/disk.h>
#include <fs/kfs2.h>

LONG kfs2_lba = 0;
BYTE kfs2_buf[16384];
LONG kfs2_cluster_index = 0;
LONG kfs2_cluster_n = 0;
LONG kfs2_err = 0;

kfs2_file kfs2_handles[KFS2_MAX_FILES];

kfs2_header* kfs2_hdr = (kfs2_header*)kfs2_buf;
kfs2_clstr* kfs2_cluster = (kfs2_clstr*)kfs2_buf;

int kfs2_init(LONG lba)
{
	if(kfs2_lba)
		return 0;
	kfs2_lba = lba;
	puts("[kfs2] init: reading fs header\n");
	if(ide_read_sector(kfs2_lba, kfs2_buf, 1, 0))
	{
		puts("[kfs2] init: read error while reading kssfs_header\n");
		kfs2_lba = 0;
		return 0;
	}
	if(kfs2_hdr->signature != KFS2_SIGNATURE)
	{
		puts("[kssfs] init: signature is incorrect\n");
		kfs2_lba = 0;
		return 0;
	}
	kfs2_cluster_n = kfs2_hdr->cluster_n;
	puts("[kfs2] init: header is correct\n");
	kmemset(kfs2_buf, 0, 8192);
	kmemset(kfs2_handles, 0, sizeof(kfs2_file) * KFS2_MAX_FILES);
	kfs2_cluster_index = 0xffffffff;
	puts("[kfs2] init: OK\n");
	return 1;
}

void kfs2_read_cluster(LONG n)
{
	if(!kfs2_lba)
		return;
	if(n > kfs2_cluster_n)
		return;
	if(n == kfs2_cluster_index)
		return;
	LONG startc = KFS2_CLSTR_LBA(n);
	for(int i = 0; i < 16; i++)
	{
		ide_read_sector(startc + i, kfs2_buf + 512 * i, 1, 0);
	}
	kfs2_cluster_index = n;
}

LONG kfs2_read_byte(LONG file, BYTE* dest, LONG n, LONG off)
{

}

LONG kfs2_open_file(char* filename)
{

}

#include <base.h>
#include <text.h>
#include <dev/disk.h>
#include <fs/kfs2.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
		puts("[kfs2] init: signature is incorrect\n");
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
	LONG dest_p;
	kfs2_file* f;
	LONG clstr, clstr_n, t;

	dest_p = 0;
	f = &kfs2_handles[file];
	if(!(f->flags & 1))
	{
		kfs2_err = ENOENT;
		return 0;
	}
	for(int i = off; i < off + n; i++)
	{
		t = KFS2_BYTE_CLSTR(i);
		if(t != clstr_n)
		{
			clstr = f->startclstr;
			clstr_n = t;
		}
		while(1)
		{
			kfs2_read_cluster(clstr);
			if(clstr_n != 0)
			{
				clstr = kfs2_cluster->next;
				clstr_n--;
				continue;
			}
			break;
		}
		dest[dest_p++] = kfs2_buf[512 + KFS2_BYTE_CLSTR_OFF(i)];
	}
	return dest_p;
}

LONG kfs2_open_file(char* filename)
{
	for(int i = 0; i < kfs2_cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(kfs2_cluster->type != KFS2_TYPE_FILE)
			continue;
		if(strncmp(filename, (char*)kfs2_cluster->name, 256) == 0)
		{
			for(int j = 0; j < KFS2_MAX_FILES; j++)
			{
				if((kfs2_handles[j].flags & 1) == 0)
				{
					kfs2_handles[j].flags = 1;
					kfs2_handles[j].startclstr = i;
					return j;
				}
			}
			kfs2_err = EMFILE;
			return -1;
		}
	}
	kfs2_err = ENOENT;
	return -1;
}

void kfs2_close_file(LONG handle)
{
	if(handle >= KFS2_MAX_FILES)
	{
		kfs2_err = ENOENT;
		return;
	}
	if(kfs2_handles[handle].flags & 1)
	{
		kfs2_handles[handle].flags = 0;
		kfs2_handles[handle].startclstr = 0;
	}
}

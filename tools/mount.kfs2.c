#define _FILE_OFFSET_BITS  64
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fuse/fuse_opt.h>

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include <base.h>
#include <fs/kfs2.h>

FILE* kfs2_file = NULL;
BYTE buf[8192];
kfs2_header* hdr = (kfs2_header*)buf;
kfs2_clstr* chdr = (kfs2_clstr*)buf;
LONG cluster_n;

void kfs2_read_cluster(LONG i);
void kfs2_write_cluster(LONG i, BYTE* buf);
int kfs2_checkdisk(void);

static int kfs2_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_nlink = 2;
		return res;
	}
	else
	{
		time_t current = time(NULL);
		for(int i = 0; i < cluster_n; i++)
		{
			kfs2_read_cluster(i);
			if(strncmp(path + 1, chdr->name, 256) == 0)
			{
				stbuf->st_mode = S_IFREG | 0755;
				stbuf->st_uid = chdr->uid;
				stbuf->st_gid = chdr->gid;
				stbuf->st_size = chdr->lenght;
				stbuf->st_mtime = current;
				stbuf->st_atime = current;
				stbuf->st_ctime = current;
				return res;
			}
		}
	}
	return -ENOENT;
}

static int kfs2_release(const char* path, struct fuse_file_info *fi)
{
	fflush(kfs2_file);
	return 0;
}

static int kfs2_fgetattr(const char *path, struct stat *stbuf,
			struct fuse_file_info* fi)
{
	printf("fgetattr\n");
	return kfs2_getattr(path, stbuf);
}

static int kfs2_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;
	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for(int i = 0; i < cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(chdr->type != 0)
		{
			filler(buf, chdr->name, NULL, 0);
		}
	}

	return 0;
}

LONG kfs2_read_byte(LONG file, BYTE* dest, LONG n, LONG off)
{
	LONG dest_p;
	LONG clstr, clstr_n, t;

	dest_p = 0;
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
				clstr = chdr->next;
				clstr_n--;
				continue;
			}
			break;
		}
		dest[dest_p++] = buf[512 + KFS2_BYTE_CLSTR_OFF(i)];
	}
	return dest_p;
}

static int kfs2_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	return kfs2_read_byte(fi->fh, buf, size, offset);
}

static int kfs2_write(const char* path, const char *fbuf, size_t size, off_t offset, 
			  struct fuse_file_info* fi)
{
	kfs2_read_cluster(fi->fh);
	unsigned writesiz = 0;
	if(offset + size > 7680)
	{
		writesiz = size - ((offset + size) - 7680);
	}
	else
	{
		writesiz = size;
	}
	memcpy(chdr->data + offset, fbuf, size);
	chdr->length = offset + writesiz;
	kfs2_write_cluster(fi->fh, buf);
	printf("offset: %d size: %d writesiz: %d\n", offset, size, writesiz);
	return writesiz;
}

static int kfs2_unlink(const char* path)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(chdr->type != KFS2_TYPE_FREE && strncmp(path, chdr->name, 256) == 0)
		{
			memset(buf, 0, 8192);
			kfs2_write_cluster(i, buf);
			return 0;
		}
	}
	return -ENOENT;
}

static int kfs2_create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(chdr->type == KFS2_TYPE_FREE)
		{
			chdr->type = KFS2_TYPE_FILE;
			strncpy(chdr->name, path + 1, 256);
			kfs2_write_cluster(i, buf);
			fi->fh = i;
			return 0;
		}
	}
	return -ENOSPC;
}

static int kfs2_open(const char *path, struct fuse_file_info *fi)
{
	printf("open\n");
	for(int i = 0; i < cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(chdr->type != 0 && strncmp(path + 1, chdr->name, 256) == 0)
		{
			fi->fh = i;
			return 0;
		}
	}
	return kfs2_create(path, fi->flags, fi);
}

static int kfs2_rename(const char* from, const char* to)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kfs2_read_cluster(i);
		if(chdr->type != 0 && strncmp(from + 1, chdr->name, 256) == 0)
		{
			strncpy(chdr->name, to + 1, 256);
			return 0;
		}
	}
	return -ENOENT;
}

void kfs2_read_cluster(LONG i)
{
	fseek(kfs2_file, 512 + i * 8192, SEEK_SET);
	fread(buf, 512, 16, kfs2_file);
}

void kfs2_write_cluster(LONG i, BYTE* buf)
{
	fseek(kfs2_file, 512 + i * 8192, SEEK_SET);
	fwrite(buf, 512, 16, kfs2_file);
}

int kfs2_checkdisk(void)
{
	fseek(kfs2_file, 0, SEEK_SET);
	fread(buf, 512, 1, kfs2_file);
	if(hdr->signature != KFS2_SIGNATURE || hdr->version != KFS2_VERSION)
	{
		return 1;
	}
	cluster_n = hdr->cluster_n;
	return 0;
}

int kfs2_truncate(const char * path, off_t offset)
{
	return 0;
}

static struct fuse_operations kfs2_oper = {
	.getattr	= kfs2_getattr,
	.fgetattr	= kfs2_fgetattr,
	.readdir	= kfs2_readdir,
	.open		= kfs2_open,
	.read		= kfs2_read,
	.write		= kfs2_write,
	.create		= kfs2_create,
	.rename		= kfs2_rename,
	.truncate	= kfs2_truncate,
	.unlink		= kfs2_unlink,
	.release	= kfs2_release,
};

struct kfs2_config {
     char* disk;
};

static struct fuse_opt kfs2_opts[] = {
	{ "-d %s", offsetof(struct kfs2_config, disk), 0}
};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct kfs2_config cfg;
	cfg.disk = NULL;
	memset(&cfg, 0, sizeof(cfg));
	fuse_opt_parse(&args, &cfg, kfs2_opts, NULL);
	if(!cfg.disk)
	{
		printf("Usage: %s -d disk.img mountpoint\n", argv[0]);
		return -1;
	}
	kfs2_file = fopen(cfg.disk, "r+");
	if(!kfs2_file)
	{
		perror("Cannot open disk");
		return -1;
	}
	if(kfs2_checkdisk())
	{
		printf("No.\n");
		return -1;
	}
	return fuse_main(args.argc, args.argv, &kfs2_oper, NULL);
}

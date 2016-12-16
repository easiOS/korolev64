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
#include <fs/kssfs.h>

FILE* kssfs_file = NULL;
BYTE buf[8192];
kssfs_header* hdr = (kssfs_header*)buf;
kssfs_clstr_hdr* chdr = (kssfs_clstr_hdr*)buf;
LONG cluster_n;

void kssfs_read_cluster(LONG i);
void kssfs_write_cluster(LONG i, BYTE* buf);
int kssfs_checkdisk(void);

static int kssfs_getattr(const char *path, struct stat *stbuf)
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
			kssfs_read_cluster(i);
			if(strncmp(path + 1, chdr->filename, 256) == 0)
			{
				stbuf->st_mode = S_IFREG | 0755;
				stbuf->st_uid = chdr->uid;
				stbuf->st_gid = chdr->gid;
				stbuf->st_size = chdr->len;
				stbuf->st_mtime = current;
				stbuf->st_atime = current;
				stbuf->st_ctime = current;
				return res;
			}
		}
	}
	return -ENOENT;
}

static int kssfs_release(const char* path, struct fuse_file_info *fi)
{
	fflush(kssfs_file);
	return 0;
}

static int kssfs_fgetattr(const char *path, struct stat *stbuf,
			struct fuse_file_info* fi)
{
	printf("fgetattr\n");
	return kssfs_getattr(path, stbuf);
}

static int kssfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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
		kssfs_read_cluster(i);
		if(chdr->type != 0)
		{
			filler(buf, chdr->filename, NULL, 0);
		}
	}

	return 0;
}

static int kssfs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	kssfs_read_cluster(fi->fh);
	unsigned readsiz = 0;
	if(offset + size > 7680)
	{
		readsiz = size - ((offset + size) - 7680);
	}
	else
	{
		readsiz = size;
	}
	memcpy(buf, chdr->data + offset, size);
	return readsiz;
}

static int kssfs_write(const char* path, const char *fbuf, size_t size, off_t offset, 
			  struct fuse_file_info* fi)
{
	kssfs_read_cluster(fi->fh);
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
	chdr->len = offset + writesiz;
	kssfs_write_cluster(fi->fh, buf);
	printf("offset: %d size: %d writesiz: %d\n", offset, size, writesiz);
	return writesiz;
}

static int kssfs_unlink(const char* path)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kssfs_read_cluster(i);
		if(chdr->type != KSSFS_CLSTR_FREE && strncmp(path, chdr->filename, 256) == 0)
		{
			memset(buf, 0, 8192);
			kssfs_write_cluster(i, buf);
			return 0;
		}
	}
	return -ENOENT;
}

static int kssfs_create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kssfs_read_cluster(i);
		if(chdr->type == KSSFS_CLSTR_FREE)
		{
			chdr->type = KSSFS_CLSTR_COM;
			strncpy(chdr->filename, path + 1, 256);
			kssfs_write_cluster(i, buf);
			fi->fh = i;
			return 0;
		}
	}
	return -ENOSPC;
}

static int kssfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("open\n");
	for(int i = 0; i < cluster_n; i++)
	{
		kssfs_read_cluster(i);
		if(chdr->type != 0 && strncmp(path + 1, chdr->filename, 256) == 0)
		{
			fi->fh = i;
			return 0;
		}
	}
	return kssfs_create(path, fi->flags, fi);
}

static int kssfs_rename(const char* from, const char* to)
{
	for(int i = 0; i < cluster_n; i++)
	{
		kssfs_read_cluster(i);
		if(chdr->type != 0 && strncmp(from + 1, chdr->filename, 256) == 0)
		{
			strncpy(chdr->filename, to + 1, 256);
			return 0;
		}
	}
	return -ENOENT;
}

void kssfs_read_cluster(LONG i)
{
	fseek(kssfs_file, 512 + i * 8192, SEEK_SET);
	fread(buf, 512, 16, kssfs_file);
}

void kssfs_write_cluster(LONG i, BYTE* buf)
{
	fseek(kssfs_file, 512 + i * 8192, SEEK_SET);
	fwrite(buf, 512, 16, kssfs_file);
}

int kssfs_checkdisk(void)
{
	fseek(kssfs_file, 0, SEEK_SET);
	fread(buf, 512, 1, kssfs_file);
	if(hdr->signature != KSSFS_SIGNATURE)
	{
		return 1;
	}
	cluster_n = hdr->cluster_n;
	return 0;
}

int kssfs_truncate(const char * path, off_t offset)
{
	return 0;
}

static struct fuse_operations kssfs_oper = {
	.getattr	= kssfs_getattr,
	.fgetattr	= kssfs_fgetattr,
	.readdir	= kssfs_readdir,
	.open		= kssfs_open,
	.read		= kssfs_read,
	.write		= kssfs_write,
	.create		= kssfs_create,
	.rename		= kssfs_rename,
	.truncate	= kssfs_truncate,
	.unlink		= kssfs_unlink,
	.release	= kssfs_release,
};

struct kssfs_config {
     char* disk;
};

#define KSSFS_OPT(t, p, v) { t, offsetof(struct kssfs_config, p), v }

static struct fuse_opt kssfs_opts[] = {
	{ "-d %s", offsetof(struct kssfs_config, disk), 0}
};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct kssfs_config cfg;
	cfg.disk = NULL;
	memset(&cfg, 0, sizeof(cfg));
	fuse_opt_parse(&args, &cfg, kssfs_opts, NULL);
	if(!cfg.disk)
	{
		printf("Usage: %s -d disk.img mountpoint\n", argv[0]);
		return -1;
	}
	kssfs_file = fopen(cfg.disk, "r+");
	if(!kssfs_file)
	{
		perror("Cannot open disk");
		return -1;
	}
	if(kssfs_checkdisk())
	{
		printf("No.\n");
		return -1;
	}
	return fuse_main(args.argc, args.argv, &kssfs_oper, NULL);
}

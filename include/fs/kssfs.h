// Korolev Semi-Static File System (Korolev File System Mark I)
// Deprecated
// For specifications, see fs/kssfs.txt

#ifndef H_FS_KSSFS
#define H_FS_KSSFS

#ifdef __EASIOS__
#include <base.h>
#endif

#define KSSFS_MBR_TYPE 0x7E
#define KSSFS_CLSTR_SZ 8192
#define KSSFS_SIGNATURE 0x37019626

#define KSSFS_CLSTR_FREE 0
#define KSSFS_CLSTR_COM 1
#define KSSFS_CLSTR_DATA 2

#define KSSFS_CLSTR_LBA(n) (kssfs_lba + 1 + n * 16)

typedef struct {
	LONG signature;
	LONG cluster_n;
	BYTE reserved[504];
} PACK kssfs_header;

typedef struct {
	LONG type;
	WORD len;
	BYTE filename[256];
	LONG uid, gid, perm;
	BYTE reserved[238];
	BYTE data[0];
} PACK kssfs_clstr_hdr;

typedef struct {
	char filename[256];
	LONG cluster;
} PACK kssfs_fn_cache_t;

int kssfs_init(LONG lba);

#endif /* H_FS_KSSFS */

// Korolev File System Mark II
// For specifications, see fs/kfs2.txt

#ifndef H_FS_KFS2
#define H_FS_KFS2

#include <base.h>

#define KFS2_MBR_TYPE 0x7E
#define KFS2_CLSTR_SZ 8192
#define KFS2_DATA_SZ 7680
#define KFS2_SIGNATURE 0x37019627

#define KFS2_TYPE_FREE	0
#define KFS2_TYPE_FILE	1
#define KFS2_TYPE_CONT	2

#define KFS2_CLSTR_EOF	0

#define KFS2_VERSION	1

#define KFS2_MAX_FILES	16

#define KFS2_CLSTR_LBA(n) (kfs2_lba + 1 + n * 16)
#define KFS2_BYTE_CLSTR(n) (n / KFS2_DATA_SZ)
#define KFS2_BYTE_CLSTR_OFF(n) (KFS2_BYTE_CLSTR(n) * KFS2_DATA_SZ - n)

typedef struct {
	LONG signature;
	LONG cluster_n;
	LONG version;
	BYTE reserved[500];
} PACK kfs2_header;

typedef struct {
	LONG type;
	LONG length;
	BYTE name[256];
	LONG owner;
	LONG group;
	LONG perm;
	LONG next;
	BYTE data[0];
} PACK kfs2_clstr;

typedef struct {
	LONG flags; // 1 == open
	LONG startclstr;
} kfs2_file;

#endif /* H_FS_KFS2 */

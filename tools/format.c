#include <base.h>
#include <fs/kssfs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv)
{
	printf("mkfs.kssfs\n");
	FILE* f = fopen(argv[1], "w");
	if(!f)
	{
		printf("cannot open target\n");
		return 0;
	}
	LONG file_n = atoi(argv[2]);
	kssfs_header hdr;
	memset(&hdr, 0, 512);
	hdr.signature = KSSFS_SIGNATURE;
	hdr.cluster_n = 256;
	fwrite(&hdr, 512, 1, f);
	printf("FS header written\n");
	kssfs_clstr_hdr chdr;
	for(int i = 0; i < file_n; i++)
	{
		FILE* f2;
		char buf[7680];
		struct stat sb;
		f2 = fopen(argv[3 + i], "r");
		printf("Processing file \"%s\"\n", argv[i+3]);
		if(!f2)
			continue;
		memset(&buf, 0, 512);
		memset(&chdr, 0, 512);
		fread(buf, 512, 15, f2);
		stat(argv[3 + i], &sb);
		if(sb.st_size > 7680)
		{
			printf("File too large\n");
			fclose(f2);
			continue;
		}
		if(sb.st_mode & S_IXUSR)
			chdr.type = KSSFS_CLSTR_COM;
		else
			chdr.type = KSSFS_CLSTR_DATA;
		chdr.len = sb.st_size;
		strncpy(chdr.filename, argv[3 + i], 256);
		printf("filename: %s\n", chdr.filename);
		chdr.uid = sb.st_uid;
		chdr.gid = sb.st_gid;
		chdr.perm = 0;
		printf("Writing header...");
		fwrite(&chdr, sizeof(kssfs_clstr_hdr), 1, f);
		printf("writing data...");
		fwrite(buf, 512, 15, f);
		fclose(f2);
		fflush(f);
		printf("OK\n");
	}
	char buf[8192];
	memset(buf, 0, 8192);
	for(int i = 0; i < 256 - file_n; i++)
	{
		fwrite(buf, 512, 16, f);
	}
	fclose(f);
	return 0;
}

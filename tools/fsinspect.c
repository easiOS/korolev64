#include <base.h>
#include <fs/kssfs.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	BYTE buf[8192];
	FILE* f;
	kssfs_header* hdr = (kssfs_header*)buf;
	kssfs_clstr_hdr* chdr = (kssfs_clstr_hdr*)buf;

	f = fopen(argv[1], "r");
	if(!f)
	{
		printf("cannot open target\n");
		return 1;
	}

	fread(buf, 512, 1, f);
	printf("KSSFS header\n=========\n");
	printf("Signature: 0x%x (%s)\n", hdr->signature, (hdr->signature == KSSFS_SIGNATURE) ? "Correct" : "Incorrect");
	printf("Cluster count: %d\n", hdr->cluster_n);
	printf("=========\n");
	LONG cluster_n = hdr->cluster_n;
	for(int i = 0; i < cluster_n; i++)
	{
		fread(buf, 512, 16, f);
		printf("Filename: %s\n", chdr->filename);
		printf("Size: %d\n", chdr->len);
	}

	fclose(f);
	return 0;
}
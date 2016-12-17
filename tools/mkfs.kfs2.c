#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <base.h>
#include <fs/kfs2.h>

LONG prepare(FILE* f, LONG size)
{
	kfs2_header h;
	
	h.signature = KFS2_SIGNATURE;
	h.cluster_n = (size - 512) / KFS2_CLSTR_SZ;
	h.version = KFS2_VERSION;
	memset(h.reserved, 0, 500);
	fwrite(&h, sizeof(kfs2_header), 1, f);
	printf("Header written\n");
	return h.cluster_n;
}

void fill(FILE* f, LONG cluster_n)
{
	kfs2_clstr c;
	
	memset(&c, 0, sizeof(kfs2_clstr));
	fwrite(&c, sizeof(kfs2_clstr), cluster_n, f);
	printf("Clusters written (%u)\n", cluster_n);
}

int main(int argc, char** argv)
{
	FILE* f;
	struct stat st;
	LONG cluster_n;
	
	printf("mkfs.kfs2\n");
	if(argc < 2)
	{
		printf("Usage: %s target\n", argv[0]);
		return 1;
	}
	f = fopen(argv[1], "wb");
	if(!f)
	{
		perror("Cannot open target file");
		return 1;
	}

	fstat(fileno(f), &st);
	cluster_n = prepare(f, st.st_size);
	fill(f, cluster_n);

	fclose(f);
	return 0;
}

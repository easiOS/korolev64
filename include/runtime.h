#ifndef H_RUNTIME
#define H_RUNTIME

#define KOM_DEST 0x3C00000

/*
KOM executables are like COM files from DOS, but this has a minimal header. KOM
files are loaded at 0x3C00000.
It doesn't support relocation or any fancy things like that, but the programmer
can define a entry point. The header is defined below (kom_hdr_t).
*/

typedef struct {
	BYTE signature[4]; // KOM0
	LONG entry_point;
	BYTE reserved[16];
} kom_hdr_t;

#endif /* H_RUNTIME */

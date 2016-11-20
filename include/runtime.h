#ifndef H_RUNTIME
#define H_RUNTIME

typedef struct {
	BYTE signature[4]; // KOM0
	LONG entry_point;
	BYTE reserved[16];
} kom_hdr_t;

#endif /* H_RUNTIME */

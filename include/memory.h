#ifndef H_MEMORY
#define H_MEMORY

#include <base.h>
#include <multiboot.h>

void mmgmt_init(struct multiboot_mmap_entry* mmap, int mmap_size);
void* mmgmt_alloc(size_t size);
void mmgmt_free(void* ptr);

#endif /* H_MEMORY */

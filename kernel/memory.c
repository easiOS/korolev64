#include <base.h>
#include <text.h>
#include <stdlib.h>
#include <multiboot.h>

LONG mmgmt_counter = 0;
LONG mmgmt_start;
LONG mmgmt_len;

void mmgmt_greatest_chunk(struct multiboot_mmap_entry* mmap, int n, LONG* index, LONG* len)
{
    LONG max = 0, maxi = 0;
    for(int i = 0; i < n; i++)
    {
        LONG start = mmap[i].addr;
        LONG end = mmap[i].addr + mmap[i].len;
        LONG type = mmap[i].type;
        puts("[memory]     "); putn10(i + 1); puts(". Type: "); putn10(type); puts(" Range: "); putn16(start); puts("-0x"); putn16(end); puts("\n");
        if(mmap[i].len > max && type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            max = mmap[i].len;
            maxi = i;
        }
    }
    *index = maxi;
    *len = max;
}

void mmgmt_init(struct multiboot_mmap_entry* mmap, int n)
{
    LONG maxi, max;
    mmgmt_greatest_chunk(mmap, n, &maxi, &max);

    mmgmt_counter = 0;
    mmgmt_start = mmap[maxi].addr;
    mmgmt_len = max >> 12;

    puts("[memory] Memory available: "); putn10(max); puts(" bytes\n");
}

void* mmgmt_alloc(size_t size)
{
    if(mmgmt_counter >= mmgmt_len)
        return NULL;
    if(!size)
        return NULL;

    size = size + (4096 - size % 4096);
    LONG n = size >> 12;
    LONG addr = mmgmt_start + mmgmt_counter * 4096;
    mmgmt_counter += n;

    return (void*)addr;
}

void mmgmt_free(void* ptr)
{
    return;
}

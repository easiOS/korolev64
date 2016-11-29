#include <base.h>
#include <paging.h>

LONG paging_directory[1024] ALIGN4K;

LONG paging_fmb_table[1024] ALIGN4K;
LONG paging_kernel_table[1024] ALIGN4K;

void paging_setup(void)
{
	// Identity map 1st megabyte

}
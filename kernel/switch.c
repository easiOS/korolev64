#include <base.h>
#include <switch.h>
#include <runtime.h>

proc_ctx_t switch_contexts[MAX_PROC];
LONG switch_proc_flags[MAX_PROC];
LONG switch_ctx = 0;
LONG switch_current_ctx = 0;

void switch_setup(void)
{
	kmemset(switch_contexts, 0, MAX_PROC * sizeof(proc_ctx_t));
	kmemset(switch_proc_flags, 0, MAX_PROC * sizeof(LONG));
}

proc_ctx_t switch_switchnext(void)
{
	if(switch_current_ctx == (MAX_PROC - 1))
	{
		switch_current_ctx = 0;
	}
	while((switch_proc_flags[switch_current_ctx] & 1) == 0)
	{
		
	}
}

proc_ctx_t* switch_get_current(void)
{
	return &switch_contexts[switch_current_ctx];
}



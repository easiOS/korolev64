#include <base.h>
#include <switch.h>
#include <runtime.h>

proc_ctx_t switch_contexts[8];
LONG switch_ctx = 0;
LONG switch_current_ctx = 0;

void switch_setup(void)
{
	kmemset(switch_contexts, 0, 8 * sizeof(proc_ctx_t));
}

proc_ctx_t switch_switchnext(void)
{
	if(switch_current_ctx == 0)
	{
		if(switch_ctx & 1 == 0)
		{
			puts("Cannot switch to first process\n");
		}
		else
		{
			switch_current_ctx = 1;
			return switch_contexts[1];
		}
	}
}
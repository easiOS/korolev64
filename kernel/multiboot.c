#include <base.h>
#include <multiboot.h>
#include <text.h>
#include <string.h>

void multiboot_process(LONG address)
{
	struct multiboot_tag* tag;
	for (tag = (struct multiboot_tag *) (address + 8);
  	 tag->type != MULTIBOOT_TAG_TYPE_END;
     tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
        + ((tag->size + 7) & ~7)))
    {
    	char buf[32];
    	kmemset(buf, 0, 32);
    	switch(tag->type)
    	{
    		case MULTIBOOT_TAG_TYPE_CMDLINE:
    			puts("Kernel command line: ");
    			puts(((struct multiboot_tag_string*)tag)->string);
    			puts("\n");
    			break;
    	}
    }
}
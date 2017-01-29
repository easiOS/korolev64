#include <base.h>
#include <multiboot.h>
#include <text.h>
#include <string.h>
#include <memory.h>

#include <cmdline.h>

void cmdline_process(char* cmd)
{
    char* cc;
    char* arg = NULL;

    cc = cmd;
    do
    {
        if(*cc == ':')
        {
            *cc = '\0';
            arg = cc+1;
        }
        cc++;
    } while(*cc);
    cmdline_cmd* c = cmdline_cmds;
    while(c->cmd[0] != '\0')
    {
        if(strncmp(cmd, c->cmd, 128) == 0)
        {
            c->func(arg);
            break;
        }
        c++;
    }
}

void multiboot_process(LONG address)
{
	struct multiboot_tag* tag;
	for (tag = (struct multiboot_tag *) (address + 8);
  	 tag->type != MULTIBOOT_TAG_TYPE_END;
     tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
        + ((tag->size + 7) & ~7)))
    {
        char buf[256];
        char* cmdline;
        char* sc, *cc;
    	kmemset(buf, 0, 32);
    	switch(tag->type)
    	{
    		case MULTIBOOT_TAG_TYPE_CMDLINE:
            {
                cmdline = ((struct multiboot_tag_string*)tag)->string;
    			puts("Kernel command line: ");
    			puts(cmdline);
    			puts("\n");

                cc = cmdline;
                sc = cmdline;
                do
                {
                    if(*cc == ' ')
                    {
                        memcpy(buf, sc, cc - sc);
                        buf[cc - sc] = '\0';
                        cmdline_process(buf);
                        sc = cc+1;
                    }
                    cc++;
                } while(*cc);
                memcpy(buf, sc, cc - sc);
                buf[cc - sc] = '\0';
                cmdline_process(buf);

    			break;
            }
            case MULTIBOOT_TAG_TYPE_MMAP:
            {
                struct multiboot_tag_mmap *tagmmap = (struct multiboot_tag_mmap *)tag;
                int mmap_n = (tagmmap->size - 16) / sizeof(struct multiboot_mmap_entry);
                memmgmt_init(tagmmap->entries, mmap_n);
            }
    	}
    }
}
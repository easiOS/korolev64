#include <base.h>
#include <cmdline.h>

void _cmdline_serial(char* arg)
{
	puts("Serial enable\n");
}

cmdline_cmd cmdline_cmds[] = {
    {
    	"serial", _cmdline_serial
    },
    {
    	"", NULL
    }
};
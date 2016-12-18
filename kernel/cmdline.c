#include <base.h>
#include <cmdline.h>

#include <dev/serial.h>

void _cmdline_serial(char* arg)
{
	if(strncmp(arg, "true", 128) != 0)
		return;
	serial_setup();
	serial_setup_port(0);
}

cmdline_cmd cmdline_cmds[] = {
    {
    	"serial", _cmdline_serial
    },
    {
    	"", NULL
    }
};
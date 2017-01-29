#include <base.h>
#include <cmdline.h>
#include <string.h>

#include <dev/serial.h>
#include <net/network.h>

void _cmdline_serial(char* arg)
{
	if(strncmp(arg, "true", 128) != 0)
		return;
	serial_setup();
	serial_setup_port(0);
}

void _cmdline_network(char* arg)
{
    if(strncmp(arg, "false", 128) != 0)
        return;
    network_disable();
}

cmdline_cmd cmdline_cmds[] = {
    {
    	"serial", _cmdline_serial
    },
    {
        "network", _cmdline_network
    },
    {
    	"", NULL
    }
};
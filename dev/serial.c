#include <base.h>
#include <port.h>
#include <dev/serial.h>

BYTE serial_status[4];

WORD serial_hwport[4] = {
	COM1, COM2, COM3, COM4
};

void serial_setup(void)
{
	kmemset(serial_status, 0, 4);
}

void serial_setup_port(WORD port)
{
	WORD hwport = serial_hwport[port];
	
	outb(hwport + 1, 0x00); // disable interrupts
	
	outb(hwport + 3, 0x80); // set DLAB high
	outb(hwport + 0, 0x01); // speed divisor high byte
	outb(hwport + 1, 0x00); // speed divisor low byte
	outb(hwport + 3, 0x03); // 8 bits, no parity, stop
	outb(hwport + 2, 0xC7); // enable fifo, clear them with 14-byte threshold
	outb(hwport + 4, 0x0B); // enable interrupts, RTS/DSR set

	serial_status[port] = 1;
}

LONG serial_enabled(WORD port)
{
	return serial_status[port];
}

LONG serial_available(WORD port)
{
	return inb(serial_hwport[port] + 5) & 1;
}

LONG serial_empty(WORD port)
{
	return inb(serial_hwport[port] + 5) & 0x20;
}

BYTE serial_read(WORD port)
{
	while(!serial_available(port));
	return inb(serial_hwport[port]);
}

void serial_write(WORD port, BYTE data)
{
	WORD hwport = serial_hwport[port];

	while(!serial_empty(port));
	if(data == '\n')
	{
		outb(hwport, '\r');
		outb(hwport, '\n');
	}
	else
		outb(hwport, data);
}

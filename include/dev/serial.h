#ifndef H_DEV_SERIAL
#define H_DEV_SERIAL

#include <base.h>

#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

void serial_setup(void);
void serial_setup_port(WORD port);
LONG serial_enabled(WORD port);
LONG serial_available(WORD port);
LONG serial_empty(WORD port);
BYTE serial_read(WORD port);
void serial_write(WORD port, BYTE data);

#endif /* H_DEV_SERIAL */

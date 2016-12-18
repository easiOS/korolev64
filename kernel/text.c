#include <base.h>
#include <port.h>
#include <text.h>
#include <dev/serial.h>

#define TEXTMEM (WORD*)(0xB8000)
#define MAX_X 80
#define MAX_Y 25

// indices of the next character
BYTE text_x = 0;
BYTE text_y = 0;
// string mode, do not update cursor position until the string is printed
BYTE strm = 0;

BYTE fg = C_LGRAY, bg = C_BLACK;

void put(char c)
{
	if(serial_enabled(0))
	{
		serial_write(0, (BYTE)c);
	}
	switch(c)
	{
		case '\n':
			text_y++;
			text_x = 255; // hack: move_cur increases text_x and we need x to be 0
			move_cur();
			return;
		case '\b':
			if(text_x)
				text_x--;
			else
			{
				if(text_y)
				{
					text_y--;
					text_x = 79;	
				}
				else
				{
					text_x = 0;
				}
			}
			return;
	}
	WORD* a = (WORD*)((TEXTMEM + (text_y * 80)) + text_x);
	*a = ((bg << 4 | fg) << 8) | c;
	move_cur();
}

void move_cur(void)
{
	text_x++;
	if(text_x >= 80)
	{
		text_x = 0;
		text_y++;
	}
	if(text_y >= 25)
	{
		text_y = 24;
		scroll();
	}
	if(!strm)
		upd_cur();
}

void upd_cur(void)
{
	WORD pos = text_y * 80 + text_x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, pos & 0xff);
	outb(0x3D4, 0x0E);
	outb(0x3D5, pos >> 8);
}

void scroll(void)
{
	for(WORD* a = TEXTMEM; a < TEXTMEM + 2000; a++)
	{
		*a = *(a + 80);
	}
	for(WORD* a = TEXTMEM + 1920; a < TEXTMEM + 2000; a++)
	{
		*a = 0;
	}
}

void clear(void)
{
	for(WORD* a = TEXTMEM; a < TEXTMEM + 0x2000; a++)
	{
		*a = 0;
	}
	text_x = 0;
	text_y = 0;
	upd_cur();
}

void puts(char* s)
{
	char* s2 = s;
	strm = 1;
	while(*s2)
	{
		put(*s2++);
	}
	strm = 0;
	upd_cur();
}

void putn(int n, int base)
{
	char buf[32];
	itoa(n, buf, base);
	puts(buf);
}
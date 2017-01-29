#ifndef H_TEXT
#define H_TEXT

#include <base.h>

#define C_BLACK 0x0
#define C_BLUE 0x1
#define C_GREEN 0x2
#define C_CYAN 0x3
#define C_RED 0x4
#define C_MAGENTA 0x5
#define C_BROWN 0x6
#define C_LGRAY 0x7
#define C_WHITE 0xf

// put character at cursor and move cursor
void put(char c);
// put character at indices
void putat(char c, unsigned char x, unsigned char y);
// scroll screen
void scroll(void);
// set foreground color
void set_fg(unsigned char col);
// set background color
void set_bg(unsigned char col);
// set cursor
void set_cur(unsigned char x, unsigned char y);
// move cursor
void move_cur(void);
// clear screen
void clear(void);
// put string
void puts(char* s);
// update VGA cursor
void upd_cur(void);
// print number
void putn(int n, int base);

void putn2(LONG n);
void putn10(LONG n);
void putn16(LONG n);

// puts for interrupt handlers
void puts_int(char* s);

#endif /* H_TEXT */
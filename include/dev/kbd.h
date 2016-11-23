#ifndef H_DEV_KBD
#define H_DEV_KBD

typedef struct {
	BYTE keycode;
	BYTE release : 1;
	BYTE ctrl : 1;
	BYTE alt : 1;
	BYTE shift : 1;
	BYTE special : 1; // doublescan in easiOS
} PACK kbd_event_t;

void kbd_setup(void);
LONG kbd_avail(void);
kbd_event_t kbd_pop(void); 

void kbd_ps2_cmd(BYTE cmd);
void kbd_ps2_flush_buf(void);
BYTE kbd_ps2_read_data(void);
void kbd_ps2_write_data(BYTE val);
BYTE kbd_ps2_selftest(void);
BYTE kbd_ps2_testch(BYTE ch);
void kbd_ps2_enable_irq(void);
void kbd_reset_stack(void);

#endif /* H_DEV_KBD */
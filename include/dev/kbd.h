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

#endif /* H_DEV_KBD */
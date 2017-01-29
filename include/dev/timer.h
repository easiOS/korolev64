#ifndef H_DEV_TIMER
#define H_DEV_TIMER

void timer_setup(void);
void sleep(LONG t);
LONG __ticks(void);
LONG time(void*);

#endif /* H_DEV_TIMER */
#ifndef H_CMDLINE
#define H_CMDLINE

#include <base.h>

typedef struct {
    char cmd[128];
    void (*func)(char* arg);
} cmdline_cmd;

extern cmdline_cmd cmdline_cmds[];

#endif /* H_CMDLINE */

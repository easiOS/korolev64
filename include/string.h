#ifndef H_STRING
#define H_STRING

#ifndef __linux__

#include <base.h>

void* kmemset(void* dest, BYTE val, LONG count);
unsigned strlen(char* s);
void reverse(char* s);
unsigned strlen(char* s);
int strncmp( const char *ptr0, const char *ptr1, int len );

#endif /* __linux__ */

#endif
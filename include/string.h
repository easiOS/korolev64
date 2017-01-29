#ifndef H_STRING
#define H_STRING

#ifndef __linux__

#include <base.h>

void *kmemset(void *s, int c, size_t n);
unsigned strlen(char* s);
void reverse(char* s);
unsigned strlen(char* s);
int strncmp( const char *ptr0, const char *ptr1, int len );
char *strncpy(char *dest, const char *src, size_t n);
char *strncat(char *dest, const char *src, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#endif /* __linux__ */

#endif
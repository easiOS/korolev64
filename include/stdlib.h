#ifndef H_STDLIB
#define H_STDLIB

#ifndef __linux__

int atoi(const char* s);
char* itoa(int n, char* s, int base);
void free(void* ptr);
void* malloc(size_t size);

#endif /* __linux__ */

#endif /* H_STDLIB */
#include <base.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

int atoi(const char* s)
{
	int n = 0;
	
	if(!*s)
		return 0;

	while(*s)
	{
		n *= 10;
		n += (int)(*s - 48);
	}
	return n;
}

char* itoa(int n, char* s, int base)
{
  char* orig = s;
  char tmp[33];
  kmemset(tmp, 0, 33);
  char *tp = tmp;
  int i;
  unsigned v;

  int sign = (base == 10 && n < 0);
  if (sign)
      v = -n;
  else
      v = (unsigned)n;

  while (v || tp == tmp)
  {
      i = v % base;
      v /= base;
      if (i < 10)
        *tp++ = i+'0';
      else
        *tp++ = i + 'a' - 10;
  }

  int len = tp - tmp;

  if (sign)
  {
      *s++ = '-';
      len++;
  }

  while (tp > tmp)
      *s++ = *--tp;
  *s = '\0';
  return orig;
}

void reverse(char* s)
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

int strncmp( const char *ptr0, const char *ptr1, int len ){
  int fast = len/sizeof(size_t) + 1;
  int offset = (fast-1)*sizeof(size_t);
  int current_block = 0;

  if( len <= sizeof(size_t)){ fast = 0; }


  size_t *lptr0 = (size_t*)ptr0;
  size_t *lptr1 = (size_t*)ptr1;

  while( current_block < fast ){
    if( (lptr0[current_block] ^ lptr1[current_block] )){
      int pos;
      for(pos = current_block*sizeof(size_t); pos < len ; ++pos ){
        if( (ptr0[pos] ^ ptr1[pos]) || (ptr0[pos] == 0) || (ptr1[pos] == 0) ){
          return  (int)((unsigned char)ptr0[pos] - (unsigned char)ptr1[pos]);
          }
        }
      }

    ++current_block;
    }

  while( len > offset ){
    if( (ptr0[offset] ^ ptr1[offset] )){ 
      return (int)((unsigned char)ptr0[offset] - (unsigned char)ptr1[offset]); 
      }
    ++offset;
    }
  
  
  return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
  int d0,d1,d2,d3,d4,d5;
  char *ret = dest;

  __asm__ __volatile__(
    /* Handle more 16 bytes in loop */
    "cmp $0x10, %0\n\t"
    "jb 1f\n\t"

    /* Decide forward/backward copy mode */
    "cmp %2, %1\n\t"
    "jb 2f\n\t"

    /*
     * movs instruction have many startup latency
     * so we handle small size by general register.
     */
    "cmp  $680, %0\n\t"
    "jb 3f\n\t"
    /*
     * movs instruction is only good for aligned case.
     */
    "mov %1, %3\n\t"
    "xor %2, %3\n\t"
    "and $0xff, %3\n\t"
    "jz 4f\n\t"
    "3:\n\t"
    "sub $0x10, %0\n\t"

    /*
     * We gobble 16 bytes forward in each loop.
     */
    "3:\n\t"
    "sub $0x10, %0\n\t"
    "mov 0*4(%1), %3\n\t"
    "mov 1*4(%1), %4\n\t"
    "mov  %3, 0*4(%2)\n\t"
    "mov  %4, 1*4(%2)\n\t"
    "mov 2*4(%1), %3\n\t"
    "mov 3*4(%1), %4\n\t"
    "mov  %3, 2*4(%2)\n\t"
    "mov  %4, 3*4(%2)\n\t"
    "lea  0x10(%1), %1\n\t"
    "lea  0x10(%2), %2\n\t"
    "jae 3b\n\t"
    "add $0x10, %0\n\t"
    "jmp 1f\n\t"

    /*
     * Handle data forward by movs.
     */
    ".p2align 4\n\t"
    "4:\n\t"
    "mov -4(%1, %0), %3\n\t"
    "lea -4(%2, %0), %4\n\t"
    "shr $2, %0\n\t"
    "rep movsl\n\t"
    "mov %3, (%4)\n\t"
    "jmp 11f\n\t"
    /*
     * Handle data backward by movs.
     */
    ".p2align 4\n\t"
    "6:\n\t"
    "mov (%1), %3\n\t"
    "mov %2, %4\n\t"
    "lea -4(%1, %0), %1\n\t"
    "lea -4(%2, %0), %2\n\t"
    "shr $2, %0\n\t"
    "std\n\t"
    "rep movsl\n\t"
    "mov %3,(%4)\n\t"
    "cld\n\t"
    "jmp 11f\n\t"

    /*
     * Start to prepare for backward copy.
     */
    ".p2align 4\n\t"
    "2:\n\t"
    "cmp  $680, %0\n\t"
    "jb 5f\n\t"
    "mov %1, %3\n\t"
    "xor %2, %3\n\t"
    "and $0xff, %3\n\t"
    "jz 6b\n\t"

    /*
     * Calculate copy position to tail.
     */
    "5:\n\t"
    "add %0, %1\n\t"
    "add %0, %2\n\t"
    "sub $0x10, %0\n\t"

    /*
     * We gobble 16 bytes backward in each loop.
     */
    "7:\n\t"
    "sub $0x10, %0\n\t"

    "mov -1*4(%1), %3\n\t"
    "mov -2*4(%1), %4\n\t"
    "mov  %3, -1*4(%2)\n\t"
    "mov  %4, -2*4(%2)\n\t"
    "mov -3*4(%1), %3\n\t"
    "mov -4*4(%1), %4\n\t"
    "mov  %3, -3*4(%2)\n\t"
    "mov  %4, -4*4(%2)\n\t"
    "lea  -0x10(%1), %1\n\t"
    "lea  -0x10(%2), %2\n\t"
    "jae 7b\n\t"
    /*
     * Calculate copy position to head.
     */
    "add $0x10, %0\n\t"
    "sub %0, %1\n\t"
    "sub %0, %2\n\t"

    /*
     * Move data from 8 bytes to 15 bytes.
     */
    ".p2align 4\n\t"
    "1:\n\t"
    "cmp $8, %0\n\t"
    "jb 8f\n\t"
    "mov 0*4(%1), %3\n\t"
    "mov 1*4(%1), %4\n\t"
    "mov -2*4(%1, %0), %5\n\t"
    "mov -1*4(%1, %0), %1\n\t"

    "mov  %3, 0*4(%2)\n\t"
    "mov  %4, 1*4(%2)\n\t"
    "mov  %5, -2*4(%2, %0)\n\t"
    "mov  %1, -1*4(%2, %0)\n\t"
    "jmp 11f\n\t"

    /*
     * Move data from 4 bytes to 7 bytes.
     */
    ".p2align 4\n\t"
    "8:\n\t"
    "cmp $4, %0\n\t"
    "jb 9f\n\t"
    "mov 0*4(%1), %3\n\t"
    "mov -1*4(%1, %0), %4\n\t"
    "mov  %3, 0*4(%2)\n\t"
    "mov  %4, -1*4(%2, %0)\n\t"
    "jmp 11f\n\t"

    /*
     * Move data from 2 bytes to 3 bytes.
     */
    ".p2align 4\n\t"
    "9:\n\t"
    "cmp $2, %0\n\t"
    "jb 10f\n\t"
    "movw 0*2(%1), %%dx\n\t"
    "movw -1*2(%1, %0), %%bx\n\t"
    "movw %%dx, 0*2(%2)\n\t"
    "movw %%bx, -1*2(%2, %0)\n\t"
    "jmp 11f\n\t"

    /*
     * Move data for 1 byte.
     */
    ".p2align 4\n\t"
    "10:\n\t"
    "cmp $1, %0\n\t"
    "jb 11f\n\t"
    "movb (%1), %%cl\n\t"
    "movb %%cl, (%2)\n\t"
    ".p2align 4\n\t"
    "11:"
    : "=&c" (d0), "=&S" (d1), "=&D" (d2),
      "=r" (d3),"=r" (d4), "=r"(d5)
    :"0" (n),
     "1" (src),
     "2" (dest)
    :"memory");

  return ret;

}

void* malloc(size_t size)
{
  if(size <= 0){return 0;}
  return mmgmt_alloc(size);
}

void free(void* ptr)
{
  if(!ptr) return;
  mmgmt_free(ptr);
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    do {
        if (!n--)
            return ret;
    } while (*dest++ = *src++);
    while (n--)
        *dest++ = 0;
    return ret;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while (n--)
        if (!(*dest++ = *src++))
            return ret;
    *dest = 0;
    return ret;
}

void *kmemset(void *s, int c, size_t n)
{
    volatile unsigned char* p=s;
    while(n--)
        *p++ = (unsigned char)c;
    return s;
}

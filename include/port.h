#include <base.h>

inline BYTE inb(LONG port)
{
	BYTE ret;
	asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
	return ret;
}

inline void outb(LONG port, BYTE value)
{
	asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

inline WORD inw(WORD port)
{
   WORD ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

inline void outl(WORD port, LONG data)
{
	asm volatile("outl %%eax, %%dx" :: "d" (port), "a" (data));
}

inline LONG inl(WORD port)
{
	LONG ret;
	asm volatile("inl %%dx, %%eax" : "=a" (ret): "dN" (port));
	return ret;
}

static inline void io_wait(void)
{
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}

static inline void
insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}


static inline void
outw(WORD port, WORD data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

static inline void
stosl(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

static inline void outsw(WORD port, const void *addr, int cnt){
   asm volatile("rep; outsw" : "+S" (addr), "+c" (cnt) : "d" (port));
}

static inline void insw(WORD port, void *addr, int cnt)
{
   asm volatile("rep; insw"
       : "+D" (addr), "+c" (cnt)
       : "d" (port)
       : "memory");
}
#ifndef H_BASE
#define H_BASE

#include <stdint.h>
#include <stddef.h>

typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint32_t LONG;
typedef uint64_t QUAD;

#define PACK __attribute__((packed))

static inline void cpu_relax(void)
{
	asm volatile("rep; nop" ::: "memory");
}

#endif /* H_BASE */
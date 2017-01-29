#ifndef H_BASE
#define H_BASE

#include <stdint.h>
#include <stddef.h>

#undef itoa

typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint32_t LONG;
typedef uint64_t QUAD;
typedef uint64_t QWORD;

#define PACK __attribute__((packed))
#define ALIGN4K __attribute__((aligned(4096)))

static inline void cpu_relax(void)
{
	asm volatile("rep; nop" ::: "memory");
}

#endif /* H_BASE */
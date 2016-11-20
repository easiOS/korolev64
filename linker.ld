ENTRY(_start)

SECTIONS
{
        . = 1M;

        .text BLOCK(16M) : ALIGN(4K)
        {
                *(.multiboot)
                *(.text)
        }

        .rodata BLOCK(16M) : ALIGN(4K)
        {
                *(.rodata)
        }


        .data BLOCK(16M) : ALIGN(4K)
        {
                *(.data)
        }

        .bss BLOCK(16M) : ALIGN(4K)
        {
                *(COMMON)
                *(.bss)
                *(.bootstrap_stack)
        }
}

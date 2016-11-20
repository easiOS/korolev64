; Koshmar64 entry point

section .text

global _start
extern kmain

multiboot_entry:
	jmp _start

; Multiboot 2 header
section .multiboot
multiboot:
; Signature
align 8

dd 0xE85250D6
dd 0
dd multiboot_end - multiboot
dd 0xFFFFFFFF & -(0xE85250D6 + (multiboot_end - multiboot))

entry_addr:
align 8

dw 3, 1 ; entry address
dd entry_addr_end - entry_addr
dd multiboot_entry
entry_addr_end:

end_tag:
align 8

dw 0,0
dd end_tag_end - end_tag
end_tag_end:

multiboot_end:

_start:
	; disable paging
	mov ecx, cr0
	and ecx, 0x7fffffff
	mov cr0, ecx
	mov esp, stack_top
	mov ebp, stack_top
	push 0
	popf
	push ebx
	push eax
	call kmain
	cli
	.exitloop:
	hlt
	jmp .exitloop

section .stack, nobits
align 4
resb 0x4000
stack_top:
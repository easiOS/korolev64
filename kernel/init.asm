global jmp_to_init

jmp_to_init:
	push ebp
	mov ebp, esp
	xchg bx, bx
	mov eax, [esp + 8]
	pusha
	call eax
	popa
	pop ebp
	ret
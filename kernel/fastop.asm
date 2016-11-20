global kmemset

kmemset:
	push edi

	mov ecx, [esp + 16]
	mov al, [esp + 12]
	mov edi, [esp + 8]
	rep stosb

	mov eax, [esp + 8]
	pop edi
	ret

global strlen

strlen:
	push esi
	push ecx

	xor ecx, ecx
	mov esi, [esp + 8]
	.aloop:
	cmp BYTE [esi], 0
	jz .end
	inc esi
	jnz .aloop
	.end:
	mov ecx, eax

	pop ecx
	pop esi
	ret
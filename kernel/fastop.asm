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
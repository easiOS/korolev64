org 0x3D09000
[bits 32]

kom_header:
.signature: db "KOM0"
.entry_point: dd _start
.reserved: times 16 db 0

_start:
	.start:
	mov eax, 0
	mov ebx, msg
	int 0x7f

	mov eax, 3
	int 0x7f
	cmp ebx, 0x39
	je .end
	mov eax, 0
	mov ebx, msg_not_good
	int 0x7f

	jmp .start

	.end:
	ret

msg: db 10, 10, "Nyomj space-t a kilepeshez!", 10, 0
msg_not_good: db 10, "Nem jo!", 10, 0

times 512 - ($-$$) db 0xff

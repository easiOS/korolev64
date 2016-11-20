org 0x3D09000
[bits 32]

kom_header:
.signature: db "KOM0"
.entry_point: dd _start
.reserved: times 16 db 0

_start:
	pusha
	mov eax, 0
	mov ebx, msg
	int 0x7f
	popa
	ret

msg: db "Hello World!", 10, 0

times 512 - ($-$$) db 0xff
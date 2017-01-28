org 0x3D09000
[bits 32]

kom_header:
.signature: db "KOM0"
.entry_point: dd _start
.reserved: times 16 db 0

_start:
	.start:
	jmp .start

times 512 - ($-$$) db 0xff

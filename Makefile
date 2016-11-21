all: kernel iso format fsinspect kssfs_fuse

kernel:
	mkdir -p build
	cd build && cmake .. && make -j $(nproc)

iso: kernel
	python3 biso.py

run: disk.iso
	qemu-system-x86_64 -boot d -hda disk.img -cdrom disk.iso -m 512

runb: disk.iso
	bochs -q

clean:
	rm -rf build
	rm -rf disk.iso

format: format.c include/base.h include/fs/kssfs.h
	gcc -m32 -I include -o format format.c

fsinspect: fsinspect.c include/base.h include/fs/kssfs.h
	gcc -m32 -I include -o fsinspect fsinspect.c

kssfs_fuse: kssfs_fuse.c
	gcc -lfuse -o kssfs_fuse kssfs_fuse.c

.PHONY: kernel iso run clean

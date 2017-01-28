all: kernel iso

kernel:
	mkdir -p build
	cd build && cmake .. && make -j $(nproc)

iso: kernel
	python3 biso.py

run: disk.iso
	qemu-system-x86_64 -boot d -hda disk.img -cdrom disk.iso -m 512 -serial stdio -netdev user,id=net -device e1000,netdev=net

runb: disk.iso
	bochs -q

clean:
	rm -rf build
	rm -rf disk.iso

.PHONY: kernel iso run clean

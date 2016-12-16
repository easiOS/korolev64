all: tools

tools: build build/mkfs.kssfs build/mount.kssfs build/fsinspect	

build:
	mkdir -p build

build/mkfs.kssfs: format.c ../include/base.h ../include/fs/kssfs.h
	gcc -I ../include/ -g -o build/mkfs.kssfs format.c	

build/mount.kssfs: kssfs_fuse.c ../include/base.h ../include/fs/kssfs.h
	gcc -I ../include/ -lfuse -g -o build/mount.kssfs kssfs_fuse.c

build/fsinspect: fsinspect.c ../include/base.h ../include/fs/kssfs.h
	gcc -I ../include/ -g -o build/fsinspect fsinspect.c

clean:
	rm -rf build

.PHONY: tools clean

CC = gcc
CFLAGS = -I ../include/ -g

all: tools tools2

tools: build build/mkfs.kssfs build/mount.kssfs build/fsinspect

tools2: build build/mkfs.kfs2

build:
	mkdir -p build

build/mkfs.kssfs: format.c ../include/base.h ../include/fs/kssfs.h
	gcc $(CFLAGS) -o build/mkfs.kssfs format.c	

build/mount.kssfs: kssfs_fuse.c ../include/base.h ../include/fs/kssfs.h
	gcc $(CFLAGS) -lfuse -o build/mount.kssfs kssfs_fuse.c

build/fsinspect: fsinspect.c ../include/base.h ../include/fs/kssfs.h
	gcc $(CFLAGS) -o build/fsinspect fsinspect.c

build/mkfs.kfs2: mkfs.kfs2.c ../include/base.h ../include/fs/kfs2.h
	gcc $(CFLAGS) -o build/mkfs.kfs2 mkfs.kfs2.c

clean:
	rm -rf build

.PHONY: tools tools2 clean

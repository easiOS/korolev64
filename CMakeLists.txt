cmake_minimum_required(VERSION 2.8.7)
project (korolev64)

include_directories(include)

ENABLE_LANGUAGE(ASM)

set(CMAKE_C_COMPILER i686-elf-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D __EASIOS__ -O1 -g -std=gnu99 -ffreestanding -Wall -fdiagnostics-color=always -Wno-unused-function")
set(CMAKE_C_LINK_EXECUTABLE "i686-elf-gcc -T ${CMAKE_SOURCE_DIR}/linker.ld -O1 -o <TARGET> -ffreestanding -nostdlib <OBJECTS>")
set(CMAKE_ASM_COMPILE_OBJECT "nasm -felf32 -o <OBJECT> <SOURCE>")

file(GLOB SRC_BOOT	"boot/*.asm")
file(GLOB SRC_DEV	"dev/*.c")
file(GLOB SRC_FS	"fs/*.c")
file(GLOB SRC_KERNC	"kernel/*.c")
file(GLOB SRC_KERNA	"kernel/*.asm")
file(GLOB SRC_LIBC	"libc/*.c")
file(GLOB SRC_NET	"net/*.c")

add_executable(kernel.elf ${SRC_BOOT} ${SRC_DEV} ${SRC_FS} ${SRC_KERNC} ${SRC_KERNA} ${SRC_LIBC} ${SRC_NET})

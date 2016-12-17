# Korolev64

Korolev64 is an in-development microkernel for x86.

## Features implemented
* Text terminal
* Partial stdlib
* Basic filesystem (KSSFS)
* Load and run executable
* System calls
* Linux userspace tools for the built-in filesystem

## Planned features
* Task switching
* Paging
* Improved KSSFS (KFS2)
* Move filesystems and disk handling into userspace once task switching and system calls are complete

## How to build
### Requirements:
#### To build the kernel only
* GNU/Linux host
* GCC i686-elf cross-compiler
* Netwide Assembler
* Automake
* CMake
#### To build the live ISO too
* Python 3
* GRUB2
* xorriso, mkisofs
#### To build the userspace tools only
* GNU/Linux host
* GCC
* Automake


### Building the kernel
1. Clone the repository: `git clone https://github.com/easios/korolev64`
2. Run the makefile: `make kernel`
3. (optional) Create bootable ISO: `make iso`, or just `make`
You will find the iso file in the root directory with the name `disk.iso`.

### Building the userspace tools
1. Clone the repository: `git clone https://github.com/easios/korolev64`
2. Enter the `/tools` directory
3. Run the makefile: `make`
The built executables should be in `/tools/build`.

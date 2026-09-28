
# Larpix

Larpix is the distribution of the uCnix kernel i am developing for cortex-m and other mmu-less embedded platforms.

## Features

- Multitasking kernel that fits in 32KiB sram
- Basic userspace on a rom filesystem
- Suports the MCXA153VLH

## Build

1. `git clone https://github.com/imbluedabr/uCnix-image`
2. `cd uCnix-image`
3. `git submodule init`
4. `git submodule update`
5. `make kernel`
6. `make libc`
7. `make utils`
8. `make image`

## Install
### MCXA153VLH

`./scripts/jlink_flash.sh`



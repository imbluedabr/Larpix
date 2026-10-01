
# Larpix

Larpix is the distribution of the uCnix kernel i am developing for cortex-m and other mmu-less embedded platforms.

## Features

- Multitasking kernel that fits in 32KiB sram
- Basic userspace on a rom filesystem
- Suports the MCXA153VLH

## Build

1. `git clone https://github.com/imbluedabr/Larpix`
2. `cd Larpix`
3. `git submodule init`
4. `git submodule update`
5. `make kernel`
6. `make image`

> [!NOTE]
> the build system is still in its early stages so this is subject to change.

## Flashing

To actually install the operating system on a microcontroller you have to flash `image.bin`. Below are flashing instructions per suported microcontroller.

### FRDM-MCXA153

There are 2 ways of flashing the frdm-mcxa153. You can either use `pyocd` or you can use Jlink. `pyocd` is recommended for people new to the board.

#### pyocd
1. install the `pyocd` debugger/programmer.
2. install the mcxa153 pack: `pyocd pack install MCXA153VFM`
3. connect the microcontroller.
4. check the connection, the mcxa153 should show up: `pyocd list`
5. flash the image: `./scripts/pyocd_flash.sh`
6. connect a serial terminal to the usb vcom port.
7. done.

#### Jlink

You need to have already flashed and updated the jlink debugger firmware onto the nxp board, then all you have to do is run `./scripts/jlink_flash.sh`.


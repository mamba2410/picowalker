# picowalker

## About

**Pico-specific driver files and build system**

See the sister project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

This repo intends to add driver code for the picowalker-core to make a fully functioning pico-based
pokewalker.
Some other drivers will be added as well to support different hardware.

## Project state

Currently on a breadboard, but PCB schematics are in the works.

- SSD1327 OLED display with on-the-fly image transcoding
- IrDA 3-click IR interface
- Generic interface-based buttons
- 25LC512 64k EEPROM
- Serial debug outputs over pico stdout
- Debugging on swd
- BMA400 Accelerometer

The OLED can draw original Pokewalker-encoded images and convert them on-the-fly.
The new images are 4-bpp so are twice as large in file size unfortunately.

Still to do:

- RTC
- Battery
- Get a first (portable) hardware prototype!
- Sound

## Help Wanted

This is a very large project and I can't do it alone, so extra hands would be extremely welcome and appreciated.

Help is needed to:

- Find hardware that can be used as the peripherals which are able to be controlled by the Pico.
- Write drivers/interface code for the hardware chosen.
- Design the physical layout and connections of the hardware.
- Design shells/casing for the end product.
- Find/create a good license that won't get us in trouble. (see License section)

If you would like to try out the current implementation or contribute to the project, please read
the [design doc](./docs/DESIGN.md).

For things that need doing, see the [todo doc](./docs/TODO.md).

## Resources

### Pico

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Getting Started with Pico C](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c)

### Pokewalker

- [Original pokewalker hack by Dmitry.GR](http://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker)
- [H8/300h Series software manual (for looking at the original disassembly)](https://www.renesas.com/us/en/document/mah/h8300h-series-software-manual)

### Hardware

(datasheets for the hardware go here when we have them)
See [design doc for now](docs/DESIGN.md)

## Building and Testing

Make sure you have installed and built the [Raspberry Pi Pico SDK](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf) and can run the simple `blink` program before continuing.

You also need to compile [picowalker-core]() for ARM cortex-m0+ and move the output to
`lib/libpicowalker-core.a`.

### Linux

It should be as easy as

```sh
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd ..
make
```

Then copy over the `picowalker.uf2` to the pico and it should work.
Alternatively, debugging with SWD.

### Mac

Should be the same as Linux?

### Windows

You're on your own.
The top level Makefile is for linux but doesn't actually do a lot.
The whole buidl system is managed by the original Pico CMake so getting it to build on Windows shouldn't be too hard if you try.

See instructions on the [Pico SDK datasheet.](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf)

### Note on my own testing

Programming over swd with FT2232h.
See [getting started with pico sec. 6](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)

See [the design doc](docs/DESIGN.md#witing-diagram) for components.

Debug wiring:

```raw
FT2232h ---- pico
GND -------- GND
3v3 -------- VSYS
GND -------- SGND (mid)
AD0 -------- SCLK (left)
AD1 -/220R\- SWIO (right)
AD2 -------- SWIO (right)
BD0 -------- TX (2)
BD1 -------- RX (1)
```

Custom interface cfg in `ft2232h.cfg`.

Run with

```sh
# openocd -f ./ft2232h.cfg -f target/rp2040.cfg &
$ arm-none-eabi-gdb build/picowalker.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) continue
```

(You might want to create a `.gdbinit` file in the project directory to auto-target OpenOCD)

## License

The code in this repo should be ok, since it's all driver code.

Either MIT or GPL-3, whichever you want.


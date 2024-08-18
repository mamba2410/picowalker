# picowalker

## About

**Pico-specific driver files and build system**

See the sister project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

This repo is for people who want to make their own Raspberry pi pico based pokewalker
using the `picowalker-core` code.

**Update regarding the pico 2**
The pico (rp2040) wasn't really up to the task of this being a serious replacement
to the Pokewalker due to the lack of a (useable) QSPI bus for the screen, as well
as the high sleep power usage.
The rp2350 and the pico 2 solves both of these issues by lowering the sleep
current draw and adding the HSTX peripheral which would allow a pseudo-QSPI 
interface. It's only one-way but the screen doesn't need to send data back to
the host, so its perfect for this use-case.

## Project state

I have a working prototype on a breadboard using the following hardware:

- DO180PFST05 screen with rp2350 HSTX (SH8601Z driver)
- IrDA 3-click IR interface
- Generic push buttons
- 25LC512 64k EEPROM
- BMA400 Accelerometer
- Serial debug outputs over pico stdout on uart0 (ping 0 and 1)
- Debugging on swd

Still to do:

- IrDA over PIO (using dmitry gr's code)
- RTC
- Secondary flash (for colour images and possibly cries)
- Sound
- Battery
- Get a first (portable) hardware prototype!

## Building for yourself

I have written a [tutorial](docs/TUTORIAL.md) on how to build this project for yourself.
First you'll want to check the [design doc](docs/DESIGN.md) to assemble the hardware
like how I have for the prototype.

If you feel like writing some code so that you can use different parts, then go for it!
Hopefully the current driver code is a good enough reference to help you understand what
it is you need to do.

### Debugging with `picoprobe` and openocd

Make sure you get raspberry pi's [openocd build](https://github.com/raspberrypi/openocd) and follow
the build instructions in [Appendix A of the getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

From openocd directory:

```bash
src/openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000"
```

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

You can follow the detailed [tutorial](docs/TUTORIAL.md) if you'd like, or below is a quick-start guide.

Make sure you have installed and built the [Raspberry Pi Pico SDK](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf) and can run the simple `blink` program before continuing.

You also need to compile [picowalker-core](https://github.com/mamba2410/picowalker-core) for ARM cortex-m0+ and move the output to `lib/libpicowalker-core.a`.

### Linux

It should be as easy as

```sh
cmake -B build/ -DCMAKE_BUILD_TYPE=Debug .
cmake --build build/
```

Then copy over the `picowalker.uf2` to the pico and it should work.
Alternatively, debugging with gdb and SWD.

### Mac

Should be the same as Linux?

### Windows

You're on your own.
The top level Makefile is for linux but doesn't actually do a lot.
The whole buidl system is managed by the original Pico CMake so getting it to build on Windows shouldn't be too hard if you try.

See instructions on the [Pico SDK datasheet.](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf)

## Note on my own testing

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


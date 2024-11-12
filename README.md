# picowalker

## About

**Pico-specific driver files and build system**

See the sister project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

This project is the board-specific firmware for running the "picowalker" software.

This branch is for the `picowalker` hardware v0.1, a custom PCB which is a stepping stone for an eventual custom PCB replacement for the Pokewalker.

## Project state

This is working with the [picowalker-hardware v0.1](https://github.com/mamba2410/picowalker-hardware)
which is a Raspberry Pi Pico 2 based custom PCB, including:

- DO180PFST05 OLED screen controlled over the rp2350 HSTX (SH8601Z driver)
- IrDA over PIO
- M95512 64kB EEPROM
- BMA400 Accelerometer
- BQ25628E PMIC for lithium battery charging and power management
- W25Q128J 16MB flash memory chip
- Serial debug outputs over pico stdout on uart0 (pins 0 and 1)
- Debugging on swd
- Generic push buttons

Hardware to get working:

- Battery
    - Charging and comms work, ADC readings for battery level monitoring needs implementing
- RTC
    - Using RP2350's AON timer
- Secondary flash (for colour images and possibly cries)
- USB
- Sound

## Building for yourself

I have written a [tutorial](docs/TUTORIAL.md) on how to build this project for yourself.
You'll need the hardware, but a lot of this can be done with just a pico 2 and some breakout boards.


## Help Wanted

This is a very large project and I can't do it alone, so extra hands would be extremely welcome and appreciated.

Help is needed to:

- Write drivers/interface code for the hardware.
- Design shells/casing for the end product.
- Find/create a good license. (see License section)

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

### Debugging with The Raspberry Pi Debug Probe and openocd

Make sure you get Raspberry Pi's [openocd build](https://github.com/raspberrypi/openocd) and follow the build instructions in [Appendix A of the getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

I'm also using the Raspberry Pi debug probe which is a really easy and cheap (~$15) USB debug adapter for ARM devices.

From openocd directory:

```bash
src/openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000"
```

See debug output with

```bash
screen /dev/ttyACM0 115200
```

Run GDB with

```bash
gdb build/picowalker.elf
(gdb) target remote :333
(gdb) load
(gdb) continue
```

### Linux

It should be as easy as

```sh
cmake -B build/ -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Debug .
cmake --build build/
```

Then copy over the `picowalker.uf2` to the pico and it should work.
Alternatively, debugging with gdb and SWD.

### Mac

Should be the same as Linux?

### Windows

You're on your own.
The top level Makefile is for linux but doesn't actually do a lot.
The whole build system is managed by the original Pico CMake so getting it to build on Windows shouldn't be too hard if you try.

See instructions on the [Pico SDK datasheet.](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf)

## License

Either MIT or GPL-3, whichever you want.

If you fork or make changes, I'd love to know what cool things you're doing with it!


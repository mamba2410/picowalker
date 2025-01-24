<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/mamba2410/picowalker/blob/hardware-v0.1/assets/picowalker_logo_white.png?raw=true">
    <img width=256 src="https://github.com/mamba2410/picowalker/blob/hardware-v0.1/assets/picowalker_logo_black.png?raw=true">
  </picture>
</p>

# picowalker

## About

The overall Picowalker project aims to recreate the Pokewalker from Pokemon HeartGold/SoulSilver using custom hardware based around the Raspberry Pi Pico/rp2040 series of chips.
This includes custom hardware, software and drivers, with some modern convenience features added in as well.
Users should also be able to build their own based on a Raspberry Pi Pico, given that drivers are available for it.

This repo is the board-specific firmware which supports the picowalker-core application code.

See the core code for this project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

See the custom hardware these drivers are for: [picowalker-hardware](https://github.com/mamba2410/picowalker-hardware)

Releases here will contain a full UF2 image which can be uploaded to the v1.0 picowalker hardware to run a full picowalker device.

There are multiple branches in this repo, all with similar functionality

This is the branch for the [Waveshare RP2040 Touch LCD 1.28"](https://www.waveshare.com/wiki/RP2040-Touch-LCD-1.28) board.
Under development, this is just a starting point to help out.

## Hardware

- Screen - [GC0A01A by Waveshare](https://files.waveshare.com/wiki/common/GC9A01A.pdf) (SPI 4-wire)
- Touch - [CST816S](https://files.waveshare.com/upload/5/51/CST816S_Datasheet_EN.pdf) (I2C, unused)
- Accel - [QMI8658](https://files.waveshare.com/upload/5/5f/QMI8658A_Datasheet_Rev_A.pdf) (I2C)
- EEPROM - None (linked in at compile time, read only)
- Flash - Internal (linked in at compile time, read only)
- PMIC - ETA6096, not digitally controlled
- Buttons - None, touch to be used eventually
- No IR

## Project state

Working (as intended):

Probably eeprom, flash and PMIC because they don't do a lot.

Not working:

- Screen
- Accel
- Buttons/touch

## Resources

### Pico

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Getting Started with Pico C](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c)
- [Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2040-datasheet.pdf)

## Building and Testing

You can follow the detailed [tutorial](docs/TUTORIAL.md).

### Debugging with The Raspberry Pi Debug Probe and openocd

Make sure you get Raspberry Pi's [openocd build](https://github.com/raspberrypi/openocd) and follow the build instructions in [Appendix A of the getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

I'm also using the Raspberry Pi debug probe which is a really easy and cheap (~$15) USB debug adapter for ARM devices.

From openocd directory:

```bash
src/openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg
```

See debug output with

```bash
picocom -b 115200 /dev/ttyACM0
```
Exit with `C-a` then `C-x`

Run GDB with

```bash
arm-none-eabi-gdb build/picowalker.elf
(gdb) load
(gdb) continue
```

## Resources

### Pico

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Getting Started with Pico C](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c)
- [Pico 2 Datasheet](https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf)
- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)


### Pokewalker

- [Original pokewalker hack by Dmitry.GR](http://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker)
- [H8/300h Series software manual (for reverse-engineering)](https://www.renesas.com/us/en/document/mah/h8300h-series-software-manual)

### Hardware

(datasheets for the hardware go here when we have them)
See [design doc for now](docs/DESIGN.md)

## License

Either MIT or GPL-3, whichever you want.

If you fork or make changes, I'd love to know what cool things you're doing with it!


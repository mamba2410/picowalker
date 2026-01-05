<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/mamba2410/picowalker/blob/main/assets/picowalker_logo_white.png?raw=true">
    <img width=256 src="https://github.com/mamba2410/picowalker/blob/main/assets/picowalker_logo_black.png?raw=true">
  </picture>
</p>

# picowalker

## About

The Picowalker project aims to recreate the Pokewalker from Pokemon HeartGold/SoulSilver using modern hardware based around the Raspberry Pi Pico and RP2040 series of chips.
This means custom hardware and software, with some modern convenience features added in as well.
The picowalker will (eventually) be a fully compatible replacement for the Pokewalker, being able to interface with the games and other Pokewalkers.

- Drivers: This repo
- Application core: [picowalker-core](https://github.com/mamba2410/picowalker-core)
- Hardware: [picowalker-hardware](https://github.com/mamba2410/picowalker-hardware)

Releases here will contain a full UF2 image which can be uploaded to the v1.0 picowalker hardware to run a full picowalker device.

There are multiple branches in this repo, all with similar functionality
(providing the "driver API" for the picowalker-core).

Multiple boards are contained in this repo, but the main supported boards are for the picowalker hardware v1.0. Other boards are:

- `hardware-v0.x` - Development boards while making the V1.0 hardware
- `waveshare` - An in-progress branch for the commercially available Waveshare RP2350-LCD-1.28 board

## Project state

This is working with the [picowalker-hardware v0.4](https://github.com/mamba2410/picowalker-hardware)
which is a Raspberry Pi RP2350-based custom board, including:

- DO180PFST05 OLED screen controlled over PIO QSPI (SH8601Z driver)
    - On-the-fly decode of picowalker images to RGB565
    - Currently only greyscale images
- IrDA over PIO via Dmitry Gr.
- M95512 64kB EEPROM
- BMA400 Accelerometer over SPI
- BQ25628E PMIC for lithium battery charging and power management
- USB (TinyUSB)
    - Mass Storage Controller (MSC) for backing up and restoring the eeprom save data
- RTC using RP2350's AON peripherals and an external 32.768 kHz clock

Things for the future:

- Optimise sleep current (theoretically ~200uA is possible) [example](https://github.com/mamba2410/rp2350-powman-sleep).
- Colour sprites [issue](https://github.com/mamba2410/picowalker-core/issues/9).

## Issues and Contributing

Please leave all issues in this repo, irrespective of what parts of the code they technically belong to.

If you'd like to contribute, please see the [design doc](./docs/DESIGN.md) and the build guide below.

## Building and Testing

You can follow the detailed [tutorial](docs/TUTORIAL.md).

### Debugging with The Raspberry Pi Debug Probe and openocd

Make sure you get Raspberry Pi's [openocd build](https://github.com/raspberrypi/openocd) and follow the build instructions in [Appendix A of the getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

I'm also using the Raspberry Pi debug probe which is a really easy and cheap (~$15) USB debug adapter for ARM devices.

From openocd directory:

```bash
src/openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000"
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


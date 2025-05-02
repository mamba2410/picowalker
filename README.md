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
(providing the "driver API" for the picowalker-core).

- `hardware-v0.1` - The current active branch, drivers specific to the 
    [custom PCB](https://github.com/mamba2410/picowalker-hardware) used as a
    stepping stone to creating a modern rebuild.
- `pico2` - Branch used to test out hardware using a pico 2 on a breadboard.
    It is mostly compatible with `hardware-v0.1` except the pinouts will have
    changed and some of the hardware is difficult to connect on a breadboard.
- `waveshare` - An in-progress branch for the Waveshare RP2040 dev board

## Project state

This is working with the [picowalker-hardware v0.1](https://github.com/mamba2410/picowalker-hardware)
which is a Raspberry Pi Pico 2 based custom PCB, including:

- DO180PFST05 OLED screen controlled over PIO QSPI or the rp2350 HSTX (SH8601Z driver)
    - On-the-fly decode of picowalker images to RGB565.
    - Currently only greyscale images.
- IrDA over PIO via Dmitry Gr.
    - Currently CPU-fed but needs to be DMA-fed
- M95512 64kB EEPROM
- BMA400 Accelerometer over SPI
    - Automatic step detection, with optional interrupts
- BQ25628E PMIC for lithium battery charging and power management, with I2C interface
    - Charging
    - Voltage and current measurements
- Serial debug outputs over pico stdout on a uart
- Debugging on swd
- Generic push buttons
- USB (TinyUSB)
    - Mass Storage Controller (MSC) for backing up and restoring the eeprom save data.
- RTC
    - Using RP2350's AON timer (internal LPOSC)

Hardware to get working:

- Battery
    - Turn VBAT voltage reading into a battery level estimate
    - Safe shutdown if battery voltage goes too low
- Sound (not on hardware-v0.1)
- External RTC (on hardware-v0.2)
- Optimise sleep current (theoretically ~200uA is possible) [example](https://github.com/mamba2410/rp2350-powman-sleep).
- Colour sprites.

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

### Hardware

(datasheets for the hardware go here when we have them)
See [design doc for now](docs/DESIGN.md)

## License

Either MIT or GPL-3, whichever you want.

If you fork or make changes, I'd love to know what cool things you're doing with it!


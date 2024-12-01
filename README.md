# picowalker

## About

**Pico-specific driver files and build system**

See the sister project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

This project is the board-specific firmware for running the "picowalker" software.

There are multiple branches in this repo, all with similar functionality
(providing the "hardware API" for the picowalker-core).

- `hardware-v0.1` - The current active branch, drivers specific to the 
    [custom PCB](https://github.com/mamba2410/picowalker-hardware) used as a
    stepping stone to creating a modern rebuild.
- `pico2` - Branch used to test out hardware using a pico 2 on a breadboard.
    It is mostly compatible with `hardware-v0.1` except the pinouts will have
    changed and some of the hardware is difficult to connect on a breadboard.
- `master` - Branch used to test out hardware using an original pico 1 on a 
    breadboard. This will hopefully, eventually contain lots of "drivers" to 
    be able to run a homebrew Pokewalker using DIY hardware, for now, based
    around the pico and pico 2.

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
    - Charging, comms and ADC readings work
    - Need to turn VBAT voltage reading into a battery level estimate
- RTC
    - Using RP2350's AON timer
- Secondary flash (for colour images and possibly cries)
- Sound (not on hardware-v0.1)
- USB
- Optimise sleep current (theoretically ~200uA is possible)

## Help Wanted

This is a very large project and I can't do it alone, so extra hands would be extremely welcome and appreciated.

Help is needed to:

- Write drivers/interface code for the hardware.
- Design shells/casing for the end product.
- Find/create a good license. (see License section)

If you would like to try out the current implementation or contribute to the project, please read
the [design doc](./docs/DESIGN.md) and [build tutorial](./docs/TUTORIAL.md).

## Resources

### Pico

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Getting Started with Pico C](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c)
- [Pico 2 Datasheet](https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf)
- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)


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
picocom -b 115200 /dev/ttyACM0
```
Exit with `C-a` then `C-x`

Run GDB with

```bash
arm-none-eabi-gdb build/picowalker.elf
(gdb) load
(gdb) continue
```

## License

Either MIT or GPL-3, whichever you want.

If you fork or make changes, I'd love to know what cool things you're doing with it!


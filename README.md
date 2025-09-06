# picowalker

## About

**Pico-specific driver files and build system**

See the sister project: [picowalker-core](https://github.com/mamba2410/picowalker-core).

This project is the board-specific firmware for running the "picowalker" software.

There are multiple branches in this repo, all with similar functionality

This is the branch for the [Waveshare RP2350 Touch LCD 1.28"](https://www.waveshare.com/wiki/RP2350-Touch-LCD-1.28) board. The RP2040 version currently has bugs at this time. 
> [!Note]
> Corrupt eeprom when transferring eeprom file

## Hardware

| Hardware | Implementation | Information
| ---      | ---            | ---
| Screen   | Screen          | [GC0A01A](https://files.waveshare.com/wiki/common/GC9A01A.pdf) (SPI 4-wire)
| Touch    | Buttons / Menu | [CST816S](https://files.waveshare.com/upload/5/51/CST816S_Datasheet_EN.pdf) (I2C)
| Accel    | Steps          | [QMI8658](https://files.waveshare.com/upload/5/5f/QMI8658A_Datasheet_Rev_A.pdf) (I2C)
| EEPROM   | Emulated       | None 
| Flash    | Internal       | Internal
| PMIC     | None           | [ETA6096](https://www.eta-semi.com/wp-content/uploads/2022/03/ETA6096_V1.4.pdf), not digitally controlled
| Buttons  | Touchscreen    | None
| IR       | External       | TBD
| Audio    | External       | [0.5W 8 Ohm Speaker](https://www.amazon.com/dp/B0826YLV6C)
| Battery  | External       | TBD

## Project state

Working (as intended):

- Screen
- Touch
- EEPROM
- Flash

Not working:

- Accel - Needs review of how to count steps.
- IR
- Audio


## Resources

### Pico

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Getting Started with Pico C](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c)
- [Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)

## Building and Testing

You can follow the detailed [tutorial](docs/TUTORIAL.md) if you'd like, or below is a quick-start guide.

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

## License

Either MIT or GPL-3, whichever you want.

If you fork or make changes, I'd love to know what cool things you're doing with it!


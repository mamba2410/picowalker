# picowalker

## About

This project aims to recreate a Pokewalker from Pokemon HeartGold/SoulSilver using custom hardware based around the Raspberry Pi Pico.
People should be able to build their own fully functioning device which can interact with the original HG/SS games as the pokewalker did.
This project does *NOT* aim to create a device used to hack/manipulate the HG/SS games and it is *NOT* aimed at creating a product to sell. This is purely for hobby and educational purposes.
We will try to stay faithful to the original use and intent of the pokewalker, but on a new, relatively easily buildable device, since working, original Pokewalkers are becoming more and more rare.

There are other projects based around emulating the code that is on the pokewalker, however this project aims to create a new device which is capable of emulating all of the features of the original pokewalker, with room for improvement.

The project is written in C, aimed at the [Raspberry Pi Pico](https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c) and will try to remain faithful to the original pokewalker code, with some more modern and high level approaches.


## Help Wanted

This is a very large project and I can't do it alone, so extra hands would be extremely welcome and appreciated.

Help is needed to:
- Translate and modernise the code on the original Pokewalker to the Pico.
- Find hardware that can be used as the peripherals which are able to be controlled by the Pico.
- Write drivers/interface code for the hardware chosen.
- Design the physical layout and connections of the hardware.
- Design shells/casing for the end product.
- Find/create a good license that won't get us in trouble. (see License section)


If you would like to contribute, please make sure you have read the [design doc](./docs/DESIGN.md).

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


## Building and Testing

Make sure you have installed and built the [Raspberry Pi Pico SDK](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf) and can run the simple `blink` program before continuing.

### Linux
It should be as easy as 
```
$ cd build
$ cmake ..
$ cd ..
$ make
```

Then copy over the `picowalker.uf2` to the pico and it should work

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

Wiring:
```
FT2232h	---- pico
GND -------- GND
3v3 -------- VSYS
AD0 -------- SCLK (left)
AD1 -------- SGND (mid)
AD2 -------- SWIO (right)
AD3 -/220R\- SWIO (right)
BD0 -------- TX (1)
BD1 -------- RX (2)
```

Custom interface cfg in `ft2232h.cfg`.

Run with
```
$ openocd -f ./ft2232h.cfg -f target/rp2040.cfg &
$ arm-none-eabi-gdb build/picowalker.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) continue
```



## License

As this is technically not an original project, I am unsure about the license.
I would like as much of this project to be as free and open source as possible, with the exception of being able to sell this as a product, since that will probably get everyone in trouble with Nintendo licensing and nobody wants that.

Licensing suggestions would be welcome. In the meantime, I guess this is fully copyrighted to the contributors.




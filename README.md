# picowalker

## About

This branch for `picowalker` is for emulating the original code run on the Pokewalker.
Users supply their own rom files (`pweep.rom` and `pwflash.rom`) and are then able to emulate the walker.

The emulator core is [written in zig by PoroCYon](https://git.titandemo.org/PoroCYon/pwemu/)

This is being developed first as we would like a finished product to be available faster.
The original recreation will take a long time.

## Help Wanted

This is a very large project and I can't do it alone, so extra hands would be extremely welcome and appreciated.

Help is needed to:
- Update and maintain the [emulator core](https://git.titandemo.org/PoroCYon/pwemu/).
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

There are a few prerequisites for building this:

- Make sure you have installed and built the [Raspberry Pi Pico SDK](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf) and can run the simple `blink` program before continuing.
- Cross-compile [PoroCYon's emulator core](https://git.titandemo.org/PoroCYon/pwemu/) for pico and put it in `lib/libpwlib.a`. Instructions are on their git page.
- You need two ROMs for the walker to function, the internal flash rom `rom/pwflash.rom` and the external eeprom `rom/pweep.rom`. These will not be provided.


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




## License

As this is technically not an original project, I am unsure about the license.
I would like as much of this project to be as free and open source as possible, with the exception of being able to sell this as a product, since that will probably get everyone in trouble with Nintendo licensing and nobody wants that.

Emulator core license is given at [the original repo](https://git.titandemo.org/PoroCYon/pwemu/-/blob/master/build.zig)

Licensing suggestions would be welcome. In the meantime, I guess this is fully copyrighted to the contributors.




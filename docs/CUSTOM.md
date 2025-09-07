# Building your custom board

This repo supports, and encourages, building custom boards for the picowalker.
I'm going to assume you know what you're doing if you're making your own board.
I'll be giving instructions on how to modify the source code to use on your board.

To set up the repo for your custom board you will need to:

- Make a new directory in the `boards/` directory.
- Create a `CMakeLists.txt` file to build the right drivers for your board.
- Create a `main.c` for your board which runs the `picowalker-core` code and initialises the buses.
- Create a `board_resources.h` to keep track of pins, peripherals and channels used.

The easiest way to get started is to copy an existing boards configuration and modify that.
`picowalker-v0.3` is based on the `rp2350` and is a good start.

## Substituting hardware parts

If your board contains hardware that isn't currently in the repo, you will need to write a driver for it which provides all of the functions that `picowalker-core` expects.

For example, if you have the `BMA150` accelerometer that runs in SPI mode and runs on the rp2040/rp2350, you would need to:

- Create a new pair of files called `src/drivers/accel/bma150_rp2xxx_spi.c`/`h`
- Write functions to fill the eeprom driver role. Mainly `pw_eeprom_init()`, `pw_eeprom_read()` and `pw_eeprom_write()`
- Change your `boards/my_board/CMakeLists.txt` to use the new driver by removing `drivers/accel/bma400_rp2xxx_spi.c` and adding `drivers/accel/bma150_rp2xxx_spi.c`. Same for `.h` files.

The naming structure is `drivers/<function>/<part_number>_<microcontroller>_<interface>.c`.

Of course, you could name it whatever you want, as long as it compiles, but we might be picky with accepting pull requests if you don't follow it :p

## Changing pinouts

If your board has different pins for the same hardware, you can pretty easily rearrange it.

Edit `board/my_board/board_resources.h` and update the pin numbers.
You might also need to edit the peripheral instances, like swapping `spi0` to `spi1`.



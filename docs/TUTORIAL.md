# Tutorial on setting up the picowalker DIY edition

## Prerequisites

This tutorial aims to help you:

- Set up a cross-compiling toolchain.
- Compile the [picowalker-core](https://github.com/mamba2410/picowalker-core) code for the cortex-m33 processor
    (the processor in the pico 2/rp2350 chip).
- Compile the [picowalker](https://github.com/mamba2410/picowalker) code for the default hardware configuration
    (see [design.md](./design.md)).
- Upload the code to the pico

I assume that you:

- Are running linux
- Are semi-familiar with the C language
- Have a build DIY version of the picowalker on a breadboard.
    (See the [wiring diagram]() for more info.)

With that out of the way, lets get started.

## Installing the toolchain

To get set up, there are a few tools we'll need in order to compile the
software for the pico:

- git
- cmake
- A C cross-compiler for the arm architecture
- The raspberry pi pico SDK

First, lets install `git` and `cmake`.
I'm running void linux so I'll use the xbps package manager, so make sure to
modify the command for your own distro's one.

Void Linux
```sh
sudo xbps-install -S git cmake
```
Ubuntu / WSL
```sh
sudo apt install git cmake
```

Great, now next lets install a cross-compiler.
We want one that targets bare-metal 32-bit arm processors, so we want something
that looks like `arm-none-eabi`.

On void linux, the package is called `cross-arm-none-eabi` but this will likely
be different on other distros (on debian, it is called `gcc-arm-none-eabi`).

Void Linux
```sh
sudo xbps-install -S cross-arm-none-eabi
```
Ubuntu / WSL
```sh
sudo apt install gcc-arm-none-eabi
sudo apt install build-essential
```

Next, we'll need to install the raspberry pi pico C/C++ SDK.
Raspberry pi already has [a tutorial](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
on how to get started with the pico SDK, so go read that for more details,
but I will write some rough instructions here:

First, create a directory and clone the [pico-sdk](https://github.com/raspberrypi/pico-sdk)
git repository into it.

```sh
mkdir ~/repos
cd ~/repos
git clone https://github.com/raspberrypi/pico-sdk
git clone https://github.com/raspberrypi/pico-extras
git clone https://github.com/raspberrypi/pico-examples
```

Next you'll want to set an environment variable to let `cmake` know where to
find the pico SDK.
We'll add it to our `.profile` file which automatically sets environment
variables when we log in.

```sh
echo "export PICO_SDK_PATH=$HOME/repos/pico-sdk" >> ~/.profile
source ~/.profile
```

Again, make sure you read the full documentation on how to get set up with it.
I won't repeat it here since it might change in the future.

It might be a good idea to see if you can build the "blink" project in
`pico-examples` to make sure you've done it right.

With that done, it's time to get some compiling done.

## Compiling `picowalker-core`

Next we need to compile the [picowalker-core](https://github.com/mamba2410/picowalker-core) library.
This is the platform-independant code to make the picowalker run.

First, we need to clone it.
Inside of our project directory:

```sh
git clone https://github.com/mamba2410/picowalker-core
cd picowalker-core
```

Now we need to compile it into a static library using `cmake`.
Since we're cross-compiling it, we need to specify which toolchain we want to
use.
This is done with the `-DCMAKE_TOOLCHAIN_FILE` flag.

So now, lets run

```sh
cmake -B build/waveshare -DCMAKE_TOOLCHAIN_FILE=./toolchain-pico2.cmake .
cmake --build build/waveshare
```

if you encounter errors here, it likely means that `cmake` can't find your
cross-compiler.
Check the error messages for more details, and try googling the error messages.
I'm not a CMake expert so that's all I do when something goes wrong ¯\\_(ツ)_/¯

If all went well, you should now have a file located at
`build/pico2/libpicowalker-core.a`.
This is our static library!

Now we can move on to compiling the picowalker drivers for our hardware.

## Compiling `picowalker`

First, we'll need to clone the repo.
Inside of our project directory:

Main Branch
```sh
cd ~/repos
git clone https://github.com/mamba2410/picowalker
cd picowalker
```

Waveshare Branch
```sh
cd ~/repos
git clone --single-branch --branch waveshare https://github.com/DaveuRrr/picowalker
cd picowalker
```

Now we'll need to copy over our library to be included in the `picowalker`
project.

#### Pokewalker-core
```sh
mkdir lib
cp ../picowalker-core/build/waveshare/libpicowalker-core.a lib/
```
#### LVGL
Include LVGL
```sh
git clone https://github.com/lvgl/lvgl  lib/lvgl
```
#### RP2350TouchLCD128
```sh
git clone https://github.com/DaveuRrr/RP2350TouchLCD128 lib/RP2350TouchLCD128
```

And now we get to build it with `cmake`:

```sh
cmake -B build/waveshare -DPICO_BOARD=pico2 .
cmake --build build/waveshare
```

If that all went well, you should have a file called `build/waveshare/picowalker.uf2`.
This is the file you'll want to copy to your pico.

If that didn't work out for you, make sure that you have installed the
pico SDK correctly.
Check what errors cmake is giving you and google them if necessary.

Now we have a build version of the software, lets get it running!

## Copying the software to the pico

### Via USB

Hold down the `bootsel` button on the pico and connect it to your computer
via a USB cable.
It should show up as a "USB mass storage device".
Copy the `build/pico2/picowalker.uf2` file into this "mass storage device".
The pico should reboot and start running the software.

Hopefully, you should see an image on the screen!
Assuming your eeprom is empty, the board will act as an uninitialised pokewalker
waiting to receive IR commands from a game.

## Conclusion

There you have it, you should now be able to run the picowalker software on
your raspberry pi pico 2!
For now, there are only drivers for the specific hardware that I have been
prototyping with, but with some extra help, we can make drivers for lots
of different hardware so that people can mix-and-match with what they have.

Any questions you have, shoot me a message on the Pokewalker Discord and I'll
be happy to help!
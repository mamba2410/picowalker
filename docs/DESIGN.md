# picowalker Design Document


## Goals

Faithful recreation of the original Pokewalker with some minor quality of life improvements, such as rechargeable battery etc.

People should be able to build the hardware themselves, then load the code onto the pico and have a working device.


## Software

Pico C code to recreate all of the features of the original Pokewalker.
The code itself does not need to be as similar to the original as possible, however the output of said code should be.

In the end, it would be nice if multiple peripherals or even boards could be supported so try to keep all specific device/driver code separate, but this isn't a must right now.



## Hardware

This is completely up for debate.

The key components (as far as I'm aware) are:

- Main controller (Pico)
- LCD screen (96x64 2-bit greyscale) (or equivalent backlit OLED)
- IrDA interface
- Step counter
- Control buttons
- Speaker
- Rechargeable battery
- 3D printed case

I would like for multiple types of component to be supported in the future, since some may be easier to get a hold of than others, but for now getting a prototype is the goal.

# picowalker Design Document


## Goals

Faithful recreation of the original Pokewalker with some minor quality of life improvements, such as rechargeable battery etc.

People should be able to build the hardware themselves, then load the code onto the pico and have a working device.


## Software

Pico C code to recreate all of the features of the original Pokewalker.
The code itself does not need to be as similar to the original as possible, however the output of said code should be.

In the end, it would be nice if multiple peripherals or even boards could be supported so try to keep all specific device/driver code separate, but this isn't a must right now.



## Hardware

Completely open to suggestions.

Proposed hardware (for prototyping at least):

- Main controller (Pico)
- LCD screen 96x64 2-bit greyscale
    - SSD1327 128x128 4-bit greyscale is best I can find for now
- IrDA interface
    - TFDU4100/TFDS4500 TxR
    - MCP2122 UART to IrDA encoder
- Step counter
- Control buttons
- Speaker
- Rechargeable battery
- 3D printed case

I would like for multiple types of component to be supported in the future, since some may be easier to get a hold of than others, but for now getting a prototype is the goal.



## What does the code need to emulate?

- Mainly the IR comms. Want a seamless communication with the games and the walker.
- Pokemon/items/routes consistency with the games.
- Minigames and micro-apps.
- Uh some other stuff that I can't think of right now.



## WIP: How the program flow should be structured

- Entry/reset
	- Device initialisation, back to a known state.
- Main event loop
	- Wait for steps/screen timeout/etc
- Main menu
	-


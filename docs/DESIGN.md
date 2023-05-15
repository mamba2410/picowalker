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
  - SSD1327 128x128 4-bit greyscale oled is best I can find for now.
  - [AliExpress SPI](https://www.aliexpress.com/item/32920538010.html)
- IrDA interface
  - Mikroe IrDA 3-click (breadboard development) [Mouser](https://www.mouser.co.uk/ProductDetail/Mikroe/MIKROE-2871?qs=f9yNj16SXrKCBoWeEAc2vQ%3D%3D)
  - TFDU4101 TxR [Mouser](https://www.mouser.co.uk/ProductDetail/Vishay-Semiconductors/TFDU4101-TR3?qs=4vzNq8kLoSZaXF7Bmpq4%252Bw%3D%3D)
  - MCP2122 UART to IrDA encoder [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/MCP2122T-E-SN?qs=php1hWwkCL7dJgtHA%2FdIXg%3D%3D)
  - DSC1001CL1-001.8432T clock oscillator [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/DSC1001CL1-001.8432T?qs=yDQnu9hAfI%2FtBK%252Bb6vr8lA%3D%3D)
- Accelerometer
  - Bosch BMA400 SPI [Mouser](https://www.mouser.co.uk/ProductDetail/Bosch-Sensortec/BMA400?qs=f9yNj16SXrKBoguHUc32eQ%3D%3D)
- External EEPROM
  - ST M95512 (128-byte page size, same model as pokewalker) [Mouser](https://www.mouser.co.uk/ProductDetail/STMicroelectronics/M95512-DWDW4TP-K?qs=gNDSiZmRJS8kMWQ2whknqA%3D%3D)
  - 64K 25LC512 (128-byte page size)
  - 128K 25LC1024 (256-byte page size, if you want extra storage) [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/25LC1024-E-P?qs=0R2K%2FH4DmMNuJO3S%2F11Ktg%3D%3D)
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

## Wiring diagram

```raw
Pico           Peripheral
----------------------------
17 (GP13) ---- Button left
19 (GP14) ---- Button middle
20 (GP15) ---- Button right
 6 (GP4) ----- OLED SDA
 7 (GP5) ----- OLED SCL
```

Buttons have a 1k pull-up resistor.

SDA/SCL have a 4k pull-up resistor.

Everything runs on 5V DC.

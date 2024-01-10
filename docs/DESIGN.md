# picowalker Design Document

## Goals

People should be able to build the hardware themselves, then load the code onto the pico and have a working device.

Alternatively, a second version which can fit into the original Pokewalker form-factor.
This would require SMD components so not so easy to build at home.

## Hardware

The current hardware I'm using to prototype this:

- Main controller (Pico)
- LCD screen 96x64 2-bit greyscale
  - SSD1327 128x128 4-bit greyscale oled (via i2c) is best I can find for now.
  - [AliExpress SPI](https://www.aliexpress.com/item/32920538010.html)
- IrDA interface
  - Mikroe IrDA 3-click (breadboard development) [Mouser](https://www.mouser.co.uk/ProductDetail/Mikroe/MIKROE-2871?qs=f9yNj16SXrKCBoWeEAc2vQ%3D%3D)
- Alternative IrDA interface that will need some manual building (not recommended)
  - TFDU4101 TxR [Mouser](https://www.mouser.co.uk/ProductDetail/Vishay-Semiconductors/TFDU4101-TR3?qs=4vzNq8kLoSZaXF7Bmpq4%252Bw%3D%3D)
  - MCP2122 UART to IrDA encoder [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/MCP2122T-E-SN?qs=php1hWwkCL7dJgtHA%2FdIXg%3D%3D)
  - DSC1001CL1-001.8432T clock oscillator [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/DSC1001CL1-001.8432T?qs=yDQnu9hAfI%2FtBK%252Bb6vr8lA%3D%3D)
- Accelerometer
  - Bosch BMA400 SPI [Mouser](https://www.mouser.co.uk/ProductDetail/Bosch-Sensortec/BMA400?qs=f9yNj16SXrKBoguHUc32eQ%3D%3D)
- External EEPROM
  - 64K 25LC512 (128-byte page size) (currently use this one since it fits in a breadbaord)
  - ST M95512 (128-byte page size, same model as pokewalker) [Mouser](https://www.mouser.co.uk/ProductDetail/STMicroelectronics/M95512-DWDW4TP-K?qs=gNDSiZmRJS8kMWQ2whknqA%3D%3D)
  - 128K 25LC1024 (256-byte page size, if you want extra storage, but needs code modifications) [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/25LC1024-E-P?qs=0R2K%2FH4DmMNuJO3S%2F11Ktg%3D%3D)
- Control buttons

What else is needed:

- Speaker
- Rechargeable battery
- 3D printed case

## Wiring diagram

Chart for the [pico pinout](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html#pinout-and-design-files).

```raw
Pico           Peripheral
----------------------------
17 (GP13) ---- Button left
19 (GP14) ---- Button middle
20 (GP15) ---- Button right
21 (GP16) ---- OLED SDA
22 (GP17) ---- OLED SCL
 4 (GP2) ----- Accel SCK, eeprom SCK
 5 (GP3) ----- Accel MOSI, eeprom MOSI
 6 (GP4) ----- Accel MISO, eeprom MISO
 7 (GP5) ----- eeprom CSB
 9 (GP6) ----- Accel CSB
11 (GP8) ----- IR TX
12 (GP9) ----- IR RX
```

Buttons have a 1k pull-up resistor (connected to 5V).

SDA/SCL have a 10k pull-up resistor (connected to 5V).

Everything runs on 5V DC.


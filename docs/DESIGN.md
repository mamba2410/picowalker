# picowalker Design Document

## Goals

There are currently two goals of this project:

The first is to provide a set of drivers that can be mixed together to create
a custom Pokewalker by connecting a bunch of "module" boards together.
This is supposed to be so that anybody can design and assemble the hardware,
and then compile the code for their board, with some pinout tweaking.

The second is to provide a smaller set of drivers specifically designed for
a [custom PCB](https://github.com/mamba2410/picowalker-hardware) as a
Pokewalker replacement in a much closer form-factor to the original.

The second goal is the one that's active in development, but the two are not
mutually exclusive.
Once there is one working example, extending it to different hardware is a
slightly easier task.

## Hardware

For the `hardware-v0.1` branch, the hardware schematics can be found in the
[picowalker-hardware repo](https://github.com/mamba2410/picowalker-hardware).

For the breadboarding approach, the hardware is:

- Main controller (Pico 2)
- OLED screen SH8601Z driver (DO0180PFST05 module)
    - [Module (Chinese website)](https://www.dwo.net.cn/pd.jsp?id=11924#_jcp=3_38)
    - [Panel (Chinese website)](https://www.dwo.net.cn/pd.jsp?fromColId=2&id=11921#_pp=2_322)
- Optional SSD1327 drivers for different screen
- IrDA interface
  - Mikroe IrDA 3-click [Mouser](https://www.mouser.co.uk/ProductDetail/Mikroe/MIKROE-2871?qs=f9yNj16SXrKCBoWeEAc2vQ%3D%3D)
- Alternative IrDA interface that will need manually breaking out (not recommended)
  - TFDU4101 TxR [Mouser](https://www.mouser.co.uk/ProductDetail/Vishay-Semiconductors/TFDU4101-TR3?qs=4vzNq8kLoSZaXF7Bmpq4%252Bw%3D%3D)
- Accelerometer
  - Bosch BMA400 SPI [Mouser](https://www.mouser.co.uk/ProductDetail/Bosch-Sensortec/BMA400?qs=f9yNj16SXrKBoguHUc32eQ%3D%3D)
- External EEPROM
  - 64K 25LC512 (128-byte page size) (currently use this one since it fits in a breadbaord)
  - ST M95512 (128-byte page size, same model as pokewalker) [Mouser](https://www.mouser.co.uk/ProductDetail/STMicroelectronics/M95512-DWDW4TP-K?qs=gNDSiZmRJS8kMWQ2whknqA%3D%3D)
  - 128K 25LC1024 (256-byte page size, if you want extra storage, but needs code modifications) [Mouser](https://www.mouser.co.uk/ProductDetail/Microchip-Technology-Atmel/25LC1024-E-P?qs=0R2K%2FH4DmMNuJO3S%2F11Ktg%3D%3D)
- Control buttons

What is not included:

- Speaker
- Rechargeable battery
- 3D printed case

## Wiring diagram

Chart for the [pico pinout](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html#pinout-and-design-files).

```raw
Pico           Peripheral
----------------------------
 1 GP00 -- DBG_TX
 2 GP01 -- DBG_RX
 4 GP02 -- SPI_SCK
 5 GP03 -- SPI_MOSI
 6 GP04 -- SPI_MISO
 7 GP05 -- FLASH_CSB
10 GP07 -- ACCEL_CSB
11 GP08 -- ACCEL_INT1
12 GP09 -- IR_SD
14 GP10 -- IR_PIO_RX
15 GP11 -- IR_PIO_TX
16 GP12 -- SCREEN_RST
17 GP13 -- SCREEN_HSTX_CLK
19 GP14 -- SCREEN_HSTX_SD0
20 GP15 -- SCREEN_HSTX_SD1
21 GP16 -- SCREEN_HSTX_SD2
22 GP17 -- SCREEN_HSTX_SD3
24 GP18 -- SCREEN_CSB
25 GP19 -- BUTTON_R
26 GP20 -- I2C_SDA
27 GP21 -- I2C_SCL
29 GP22 -- BUTTON_M
31 GP26 -- BUTTON_L
32 GP27 -- BAT_INT
34 GP28 -- BAT_CE
3v3 ------ 3.3V rail
GND ------ Ground rail
```

Everything runs on 3.3V DC.


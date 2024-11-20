# picowalker Design Document

## Goals

People should be able to build the hardware themselves, then load the code onto the pico and have a working device.

Alternatively, a second version which can fit into the original Pokewalker form-factor.
This would require SMD components so not so easy to build at home.

## Hardware

The current hardware I'm using to prototype this:

- Main controller (Pico 2)
- OLED screen SH8601Z driver (DO0180PFST05 module)
    - [Module (Chinese website)](https://www.dwo.net.cn/pd.jsp?id=11924#_jcp=3_38)
    - [Panel (Chinese website)](https://www.dwo.net.cn/pd.jsp?fromColId=2&id=11921#_pp=2_322)
- Optional SSD1327 drivers for different screen
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


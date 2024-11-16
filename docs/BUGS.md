# Bugs that need fixing

May or may not be in `picowalker-core` instead.

## Screen

## EEPROM

- Hardware needs 10k pull-up on CSB pin, so software shouldn't drive it high
    - Switch between output-low and highz

## SPI

- SPI is initialised by all peripherals. Add a lock and common clock speed.

## Power

- Only wake on 1 second of button press

## Timers

- Set up AON timer from eeprom timestamp
- Set up AON timer from IR packets


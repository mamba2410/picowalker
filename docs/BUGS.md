# Bugs that need fixing

May or may not be in `picowalker-core` instead.

## Screen

- Clocks
    - Screen runs well enough on 24MHz at 3x resolution, but not ideal
    - Might need to have shorter wires, long leads are definitely introducing
        clock interference
    - Last-ditch use GPIO relaxation oscillator to feed into HSTX (sacrfices 
        2 gpio pins which is really not ideal)
- Rotation
    - Rotate the other way for hardware v0.1

## EEPROM

- Hardware needs 10k pull-up on CSB pin, so software shouldn't drive it high
    - Switch between output-low and highz

## SPI

- SPI is initialised by all peripherals. Add a lock and common clock speed.



# TODO

## Frontburner

- Render everything on the splash screen correctly
- Add IR code from `pw-ir-linux`
- Add IR error messages
- Speed up screen I2C speed?
- Speed up draw update interval
- Move button code to driver
- Change button code to polling instead of interrupts?
- `anim_frame` updated in main loop and passed to draw funcs

## Backburner

- General refactor/clean up
- Add pico eeprom driver
- Pokewalker data
- Minigames
- Proper IR functionality
- Accelerometer
- Sound
- Watchdog reset
- Screensaver after time limit
- Change buttons to general input?


## Apps

- PokeRadar
- Dowsing
    - Read route items
    - Write to inventory
    - Choose item override if full
    - Choose bush
    - RNG
    - Read min item steps?
    - Read current steps
- Connect
    - IR handshake
    - Master peer play
    - Slave actions
- Pokemon and Items
    - 2 screens + cursor
    - Read caught pokemon
    - Read obtained items
    - Read peer-play items
        - eeprom read
- Settings
    - Sound
    - Shade
- Splash
- Menu
- Sleep


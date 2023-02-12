# TODO

## Frontburner

- Sort and add a key to the eeeprom map.
    - Make names consistent.
    - Identify what things are route-specific and what are inventory
- Render everything on the splash screen correctly
- Add IR code from `pw-ir-linux`
- Add IR error messages
- Speed up screen I2C speed?
- Add `substate_request_redraw` which calls `state_draw_init()` again

## Backburner

- General refactor/clean up
- Fix pico eeprom driver
- Pokewalker data
- Minigames
- Proper IR functionality
- Accelerometer
- Sound
- Watchdog reset
- Screensaver after time limit
- Change buttons to general input?
- Change button code to polling instead of interrupts?


## Apps

- PokeRadar
    - Countdown timer/alarm
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
- Settings
    - Sound
    - Shade
- Splash
- Menu
- Sleep


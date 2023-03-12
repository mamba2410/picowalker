# TODO

See focalboard.


## Frontburner

- Sort and add a key to the eeeprom map.
    - Make names consistent.
    - Identify what things are route-specific and what are inventory
- Add `substate_request_redraw` which calls `state_draw_init()` again
- Add ram cache for health data.


## Backburner

- Implement more IR slave command responses
- IR messages when comms end
- Walk start/end animations
- General refactor/clean up
- Speed up screen I2C speed?
- Poke Radar
- Accelerometer
- Sound
- Watchdog reset
- Screensaver after time limit
- Change buttons to general input?
- Change button code to polling instead of interrupts?
- add copying eeprom from existing walker


## Bugs

- Inventory
    - Do not open if no inventory
    - Cursor bugs on going backwards
    - do not open page 2 if no gifted items


## Apps

- PokeRadar
    - Countdown timer/alarm
- Dowsing
    - Read min item steps?
    - Read current steps
    - Determine item based on current steps and percentages
- Settings
    - Sound
    - Shade
- Splash
- Menu
- Sleep


# Notes on development

## Comparison to Dmitry's docs

- `RELIABLE_DATA` has padding in it, 1 byte between each major struct.
- `struct CopyMarker` is listed as taking up 3-bytes in the map, but is of size 1.
- 16 bytes are unused, `0x0170-0x017f`.
- Interestingly, some of the "padding" isn't just zero. I don't know if this is significant.

- Splash menu steps reset on power loss, but current watts and total lifetime steps (in trainer card) remain

- is `HealthData.todaySteps` in the eeprom really today's steps? I have 13707 on a fresh route.


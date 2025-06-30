#ifndef PW_RP2350TouchLCD128_H
#define PW_RP2350TouchLCD128_H

#include "stdio.h"
#include "pico/stdlib.h"
#include "lib/RP2350TouchLCD128/Config/DEV_Config.h"
#include "lib/RP2350TouchLCD128/LCD/LCD_1in28.h"
#include "lib/RP2350TouchLCD128/Touch/CST816S.h"
#include "lib/RP2350TouchLCD128/QMI8658/QMI8658.h"
#include "lvgl.h"
#include "pokeball.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240

int RP2350TouchLCD128PW(void);

#endif /* PW_RP2350TouchLCD128_H */
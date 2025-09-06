#ifndef PW_DRIVER_SCREEN_RP2XXX_GC9A01A_LVGL_H
#define PW_DRIVER_SCREEN_RP2XXX_GC9A01A_LVGL_H

// Pico
#include "stdio.h"
#include "pico/stdlib.h"

// RP2350TouchLCD128 Library
#include "GC9A01A.h"
#include "CST816S.h"

// LVGL
#include "lvgl.h"

// Background Image
#include "picowalker_background_240x240_lvgl.h"

// Picowalker
#include "picowalker-defs.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240   

// For 1x scaling (96x64):
// #define PW_SCALE_NUMERATOR   1
// #define PW_SCALE_DENOMINATOR 1

// For 1.5x scaling (144x96):  
#define PW_SCALE_NUMERATOR   3
#define PW_SCALE_DENOMINATOR 2

// For 2x scaling (192x128):
// #define PW_SCALE_NUMERATOR   2  
// #define PW_SCALE_DENOMINATOR 1

#define CANVAS_WIDTH  ((SCREEN_WIDTH * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR)   // 96 * 3 / 2 = 144
#define CANVAS_HEIGHT ((SCREEN_HEIGHT * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR) // 64 * 3 / 2 = 96

//#define CANVAS_WIDTH  144  // Original 96, 1.5 x 96 = 144, 2 x 96 = 192
//#define CANVAS_HEIGHT 96   // Original 64, 1.5 x 64 = 96,  2 x 64 = 128

// Global variables
extern bool is_sleeping;

// Button callback is implemented in main_ws.c
//extern void pw_button_callback(uint8_t button);

static void display_flush_callback(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color);
static void direct_memory_access_handler(void);
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer);

// Battery functions
void pw_screen_update_battery();

#endif /* PW_DRIVER_SCREEN_RP2XXX_GC9A01A_LVGL_H */
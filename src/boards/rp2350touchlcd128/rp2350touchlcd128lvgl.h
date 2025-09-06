#ifndef PW_RP2350TOUCHLCD128LVGL_H
#define PW_RP2350TOUCHLCD128LVGL_H

// Pico
#include "stdio.h"
#include "pico/stdlib.h"

// RP2350TouchLCD128 Library
#include "WS_Config.h"
#include "GC9A01A.h"
#include "CST816S.h"
#include "QMI8658.h"

// LVGL
#include "lvgl.h"

// Background Image
#include "picowalker_background_240x240_lvgl.h"

// Picowalker
//#include "picowalker-defs.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240  

//#define CANVAS_WIDTH  ((SCREEN_WIDTH * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR)   // 96 * 3 / 2 = 144
//#define CANVAS_HEIGHT ((SCREEN_HEIGHT * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR) // 64 * 3 / 2 = 96

// Global variables
extern bool is_sleeping;

// Button callback is implemented in main_ws.c
//extern void pw_button_callback(uint8_t button);

static void display_flush_callback(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color);
static void direct_memory_access_handler(void);
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer);

#endif /* PW_RP2350TOUCHLCD128LVGL_H */
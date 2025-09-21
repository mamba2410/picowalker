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

#ifndef TOUCH
#define TOUCH true
#endif

#define DISP_HOR_RES 240
#define DISP_VER_RES 240   

// 1x = 96x64, 1.5x = 144x96, 2x = 192x128
#ifndef CANVAS_SCALE
#define CANVAS_SCALE 2
#endif

#if CANVAS_SCALE < 2
#define LR_BUTTON_Y_OFFSET  70
#define MD_BUTTON_Y_OFFSET  80
#define CANVAS_Y_OFFSET    -10
#else
#define LR_BUTTON_Y_OFFSET  85
#define MD_BUTTON_Y_OFFSET 100
#define CANVAS_Y_OFFSET      0
#endif

#define CANVAS_WIDTH  (int)(SCREEN_WIDTH * CANVAS_SCALE)
#define CANVAS_HEIGHT (int)(SCREEN_HEIGHT * CANVAS_SCALE)

// Global variables
extern bool is_sleeping;

// Button callback is implemented in main_ws.c
//extern void pw_button_callback(uint8_t button);

static void display_flush_callback(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color);
static void direct_memory_access_handler(void);
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer);

// Battery functions
void pw_screen_update_battery();

// Image scaling functions
lv_color_t get_color(screen_colour_t color);

#endif /* PW_DRIVER_SCREEN_RP2XXX_GC9A01A_LVGL_H */
#ifndef PW_DRIVER_SCREEN_GC9A01A_LVGL_H
#define PW_DRIVER_SCREEN_GC9A01A_LVGL_H

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
#include "picowalker_background_ultra_240x240_lvgl.h"

// Picowalker
#include "picowalker_structures.h"
#include "picowalker_core.h"

#define GC9A01A_MIN_BRIGHTNESS 2
#define GC9A01A_MAX_BRIGHTNESS 90

// Use dynamic resolution from GC9A01A attributes (set by orientation)
extern GC9A01A_ATTRIBUTES GC9A01A;
#define DISP_HOR_RES GC9A01A.WIDTH   // Dynamic width based on orientation
#define DISP_VER_RES GC9A01A.HEIGHT  // Dynamic height based on orientation

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

#define CANVAS_WIDTH  (int)(PW_SCREEN_WIDTH * CANVAS_SCALE)
#define CANVAS_HEIGHT (int)(PW_SCREEN_HEIGHT * CANVAS_SCALE)

// Global variables
extern bool is_sleeping;

// Button callback is implemented in main_ws.c
//extern void pw_button_callback(uint8_t button);

static void display_flush_callback(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color);
static void direct_memory_access_handler(void);
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer);

// Battery functions
void pw_screen_update_battery();

// Image color functions
lv_color_t get_color(pw_screen_color_t color);

#endif /* PW_DRIVER_SCREEN_GC9A01A_LVGL_H */
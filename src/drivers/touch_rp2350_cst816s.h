#ifndef PW_TOUCH_RP2350_CST816S_H
#define PW_TOUCH_RP2350_CST816S_H

#include <stdlib.h>
#include <stdio.h>

#include "i2c_rp2350.h"

/********************************************************************************
 * Development Board Configuration 
 * from RP2040-LCD-1.28 & RP2350-LCD-1.28
 * Found in /RP*-LCD-1.28/c/lib/DEV_CONFIG.h
********************************************************************************/
#define TOUCH_PIN_INT 21
#define TOUCH_PIN_RST 22

#define TOUCH_ADDRESS 0x15

enum touch_register
{
    TOUCH_GESTURE_ID = 0x01,
	TOUCH_FINGER_NUM,
	TOUCH_X_POS_HIGH,
	TOUCH_X_POS_LOW,
	TOUCH_Y_POS_HIGH,
	TOUCH_Y_POS_LOW,

	TOUCH_CHIP_ID = 0xA7,
	TOUCH_PROJECT_ID,
	TOUCH_FW_VERSION,
	TOUCH_MOTION_MASK,

    // Baseline Period Count
	TOUCH_BPC0H = 0xB0, 
	TOUCH_BPC0L,
	TOUCH_BPC1H,
	TOUCH_BPC1L,

	TOUCH_IRQ_PULSE_WIDTH = 0xED,
	TOUCH_NORMAL_SCAN_PERIOD,
	TOUCH_MOTION_SLIDE_ANGLE,

    // Low Power Scan
	TOUCH_LPS_RAW1H =0XF0,
	TOUCH_LPS_RAW1L,
	TOUCH_LPS_RAW2H,
	TOUCH_LPS_RAW2L,
	TOUCH_LP_AUTO_WAKE_TIME,
	TOUCH_LPS_THRESHOLD,
	TOUCH_LPS_WINDOW,
	TOUCH_LPS_FREQUENCY,
	TOUCH_LPS_IDAC,
	TOUCH_AUTO_SLEEP_TIME,
	TOUCH_IRQ_CONTROL,
	TOUCH_AUTO_RESET,
	TOUCH_LONG_PRESS_TIME,
	TOUCH_IO_CONTROL,
	TOUCH_DISABLE_AUTO_SLEEP
};

typedef enum
{
    TOUCH_POINT_MODE = 1,
    TOUCH_GESTURE_MODE,
    TOUCH_ALL_MODE,
} touch_mode_t;

typedef enum
{
    TOUCH_GESTURE_NONE = 0,
    TOUCH_GESTURE_UP,
    TOUCH_GESTURE_DOWN,
    TOUCH_GESTURE_LEFT,
    TOUCH_GESTURE_RIGHT,
    TOUCH_GESTURE_CLICK,
    TOUCH_GESTURE_DOUBLE_CLICK = 0x0B,
    TOUCH_GESTURE_LONG_PRESS = 0x0C,
} touch_gesture_t;

typedef struct 
{
    uint16_t x_point;
    uint16_t y_point;
    touch_mode_t mode;
} touch_screen;

#endif /* PW_TOUCH_RP2350_CST816S_H */
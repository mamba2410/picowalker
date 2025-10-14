#ifndef PW_BOARD_RESOURCES_H
#define PW_BOARD_RESOURCES_H

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"

/********************************************************************************
 * Board Specific pin overrides for RP2040 Touch LCD 1.28"
********************************************************************************/
#define PLL_SYS_KHZ (270 * 1000)

#define SCREEN_SPI_PORT     (spi1)
#define SENSOR_I2C_PORT     (i2c1)

#define SENSOR_SDA_PIN      (6)
#define SENSOR_SCL_PIN      (7)

#define SCREEN_DC_PIN       (8)
#define SCREEN_CS_PIN       (9)
#define SCREEN_CLK_PIN      (10)
#define SCREEN_MOSI_PIN     (11)
#define SCREEN_MISO_PIN     (12)
#define SCREEN_RST_PIN      (13)
#define SCREEN_BL_PIN       (25)

#define TOUCH_INT_PIN       (21)
#define TOUCH_RST_PIN       (22)

#define DOF_INT1            (23)
#define DOF_INT2            (24)

#define BAT_ADC_PIN         (29)
#define BAT_CHANNEL         (3)

/********************************************************************************
 * Other Available Pins
********************************************************************************/

/* IR UART Configuration */
#define IR_TX_PIN 28    // TODO Figure what PINs later
#define IR_RX_PIN 27    // TODO Figure what PINs later
#define IR_SD_PIN 26    // TODO Figure what PINs later
#define IR_PIO_HW pio1
#define IR_PIO_SM 0
#define IR_DMA_CHAN 0
#define IR_DMA_IRQ_NUM 0

/* Audio Configuration */
// Configure for piezo buzzer optimal frequency (4kHz base)
// System clock = 125MHz, target 4kHz: 125MHz / 4kHz = 31250
// Use wrap=1000, clkdiv=31.25 to get 4kHz base frequency
#define AUDIO_SPEAKER_PIN  16
#define AUDIO_ALARM_NUM 1

/********************************************************************************
 * Board Specific Functions
********************************************************************************/
#define TOUCH           true    // Touch variant
#define SCREEN_ROTATION 0       // 0 = 0d, 1 = 90d, 2 = 180d, 3 = 270d
#define CANVAS_SCALE    2       // 1x = 96x64, 1.5x = 144x96, 2x = 192x128

void board_spi_init();
void board_i2c_init();

#endif /* BOARD_RESOURCES_H */
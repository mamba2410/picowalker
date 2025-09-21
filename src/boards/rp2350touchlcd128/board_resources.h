#ifndef BOARD_RESOURCES_H
#define BOARD_RESOURCES_H

// Board-specific pin overrides for RP2350 Touch LCD 1.28"
// Override IR pins to avoid conflicts with WS_Config.h

#define TOUCH           true    // Touch variant
#define CANVAS_SCALE    2       // 1x = 96x64, 1.5x = 144x96, 2x = 192x128
// Device Configuration
//#define LCD_RST_PIN     13

/* IR UART Configuration */
#define IR_SD_PIN       26    // TODO Figure what PINs later
#define IR_PIO_RX       27    // TODO Figure what PINs later
#define IR_PIO_TX       28    // TODO Figure what PINs later

/* Audio Configuration */
// Configure for piezo buzzer optimal frequency (4kHz base)
// System clock = 125MHz, target 4kHz: 125MHz / 4kHz = 31250
// Use wrap=1000, clkdiv=31.25 to get 4kHz base frequency
#define PW_SPEAKER_PIN  16
#define AUDIO_CLKDIV 31.25f

// Add other board-specific overrides here as needed

#endif /* BOARD_RESOURCES_H */
#ifndef BOARD_RESOURCES_H
#define BOARD_RESOURCES_H

// Board-specific pin overrides for RP2350 Touch LCD 1.28"
// Override IR pins to avoid conflicts with WS_Config.h

#define IR_SD_PIN 26    // TODO Figure what PINs later
#define IR_PIO_RX 27    // TODO Figure what PINs later
#define IR_PIO_TX 28    // TODO Figure what PINs later

#define PW_SPEAKER_PIN 16

// Add other board-specific overrides here as needed

#endif /* BOARD_RESOURCES_H */
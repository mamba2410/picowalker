#ifndef PW_DRIVER_IR_USB_CDC_H
#define PW_DRIVER_IR_USB_CDC_H

#include <stdint.h>
#include <stddef.h>

int pw_ir_read(uint8_t *buf, size_t max_len);
int pw_ir_write(uint8_t *buf, size_t len);

void pw_ir_init(void);
void pw_ir_deinit(void);
void pw_ir_clear_rx(void);

#endif /* DRIVER_IR_USB_CDC_H */
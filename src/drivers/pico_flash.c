#include <stdint.h>
#include <string.h>

#include "../flash.h"
#include "pico_flash.h"

static const size_t offsets[] = {0, 0x100, 0x120, 0x140, 0x160, 0x170, 0x180};
static const size_t sizes[]   = {0x100, 0x20, 0x20, 0x20, 0x10, 0x10};

void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf) {
    uint8_t *addr = pw_flash_images + offsets[img_index];
    size_t sz = sizes[img_index];
    memcpy(buf, addr, sz);
}

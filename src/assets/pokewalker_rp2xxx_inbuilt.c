#include <stdint.h>

#include <string.h>

#include "../picowalker-defs.h"
#include "pokewalker_rp2xxx_inbuilt.h"

static const size_t offsets[] = {0x0000, 0x0100, 0x0120, 0x0140, 0x0160, 0x0170, 0x180};
static const size_t sizes[]   = {0x0100, 0x0020, 0x0020, 0x0020, 0x0010, 0x0010, 0x010};

void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf) {
    uint8_t *addr = pw_flash_images + offsets[img_index];
    size_t sz = sizes[img_index];
    memcpy(buf, addr, sz);
}

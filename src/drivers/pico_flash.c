#include <stdint.h>
#include <string.h>

#include "../flash.h"

void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf) {
    switch(img_index) {
        case FLASH_IMG_POKEWALKER: {
            memcpy(buf, FLASH_DATA_POKEWALKER, FLASH_DATA_POKEWALKER_SIZE);
            break;
        }
    };
}

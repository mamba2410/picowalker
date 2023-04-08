#include "pico_flash.h"

.section .data.flash_images, "ax", %progbits
.type pw_flash_images, %object
.size pw_flash_images, DRIVER_FLASH_IMAGES_SIZE

.global pw_flash_images
pw_flash_images: .incbin "../rom/flash_images.bin"

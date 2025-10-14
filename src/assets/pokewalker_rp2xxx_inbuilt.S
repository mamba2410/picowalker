#include "pokewalker_rp2xxx_inbuilt.h"

// Flags: a = allocatable, r = readable, x = executable (invalid flag for data?)
.section .data.flash_images, "a", %progbits
.type pw_flash_images, %object
.size pw_flash_images, DRIVER_FLASH_IMAGES_SIZE

.global pw_flash_images
pw_flash_images: .incbin "../../rom/flash_images.bin"

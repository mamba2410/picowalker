#include "flash_rp2xxx_internal.h"

.section .data.flash_images, "a", %progbits
.type pw_flash_images, %object
.size pw_flash_images, 492

.global pw_flash_images
pw_flash_images: .incbin "../../../rom/flash_images.bin"

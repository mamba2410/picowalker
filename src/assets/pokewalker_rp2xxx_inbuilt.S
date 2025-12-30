#include "pokewalker_rp2xxx_inbuilt.h"

.section .rodata.flash_images
.global pw_flash_images
pw_flash_images: .incbin "../../rom/flash_images.bin"


#include "picowalker_rp2xxx_inbuilt.h"

.section .rodata.picowalker
.global sad_pokewalker
sad_pokewalker: .incbin "../../rom/test_image.bin"

.global picowalker_fancy_text
picowalker_fancy_text: .incbin "../../rom/picowalker-fancy.bin"

.global picowalker_border_text
picowalker_border_text: .incbin "../../rom/picowalker-border.bin"

.global battery_fancy_text
battery_fancy_text: .incbin "../../rom/battery-fancy.bin"

.global percent_char
percent_char: .incbin "../../rom/percent-char.bin"


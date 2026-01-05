#include "picowalker_rp2xxx_inbuilt.h"

.section .rodata.picowalker

.global picowalker_fancy_text
picowalker_fancy_text: .incbin "assets/picowalker/picowalker_fancy.bin"

.global picowalker_border_text
picowalker_border_text: .incbin "assets/picowalker/picowalker_border.bin"

.global battery_fancy_text
battery_fancy_text: .incbin "assets/picowalker/battery_fancy.bin"

.global percent_char
percent_char: .incbin "assets/picowalker/percent_char.bin"


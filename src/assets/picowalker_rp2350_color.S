#include "picowalker_rp2350_color_icons.h"
#include "picowalker_rp2350_color_routes.h"

.section .rom.color_icons, "a", %progbits
.global color_icons
color_icons: .incbin "picowalker/color_icons.bin"

.section .rom.color_routes, "a", %progbits
.global color_routes
color_routes: .incbin "picowalker/color_routes.bin"

.section .rom.color_pokemon_small, "a", %progbits
.global color_pokemon_small
color_pokemon_small: .incbin "picowalker/color_pokemon_small.bin"

.section .rom.color_pokemon_large, "a", %progbits
.global color_pokemon_large
color_pokemon_large: .incbin "picowalker/color_pokemon_large.bin"

.section .rom.color_pokemon_large_shiny, "a", %progbits
.global color_pokemon_large_shiny
color_pokemon_large_shiny: .incbin "picowalker/color_pokemon_large_shiny.bin"
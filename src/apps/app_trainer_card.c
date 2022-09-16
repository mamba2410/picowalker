#include <stdint.h>

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../pw_images.h"

void pw_trainer_card_init_display() {
    pw_screen_draw_img(&text_mm_trainer, 8, 0);
    pw_screen_draw_img(&icon_mm_return, 0, 0);
    pw_screen_draw_img(&icon_mm_rarrow, SCREEN_WIDTH-icon_mm_rarrow.width, 0);

    pw_screen_draw_img(&icon_generic_person, 0, 16);
    pw_screen_draw_img(&text_trainer_name, 16, 16);
    pw_screen_draw_img(&icon_small_route, 0, 32);
    pw_screen_draw_img(&text_route_name, 16, 32);
    pw_screen_draw_img(&text_time, 0, 48);
    pw_screen_draw_time(23, 59, 59, 32, 48);


}

void pw_trainer_card_handle_input(uint8_t b) {
    switch(b) {
        case BUTTON_L:
            pw_set_state(STATE_MAIN_MENU);
            break;
        case BUTTON_M:
            pw_set_state(STATE_SPLASH);
            break;
        case BUTTON_R: // TODO
        default:
            pw_set_state(STATE_ERROR);
            break;
    }
}



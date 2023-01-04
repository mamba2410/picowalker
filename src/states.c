#include <stdio.h>

#include "ir_comms.h"
#include "states.h"
#include "menu.h"
#include "buttons.h"
#include "screen.h"
#include "utils.h"
#include "trainer_info.h"
#include "eeprom_map.h"
#include "eeprom.h"

#include "apps/app_trainer_card.h"

#include "pwroms.h"

const char* const state_strings[N_STATES] = {
    [STATE_SCREENSAVER]     = "Screensaver",
	[STATE_SPLASH]          = "Splash screen",
	[STATE_MAIN_MENU]       = "Main menu",
	[STATE_POKE_RADAR]      = "Poke radar",
	[STATE_DOWSING]         = "Dowsing",
	[STATE_CONNECT]         = "Connect",
	[STATE_TRAINER_CARD]    = "Trainer card",
	[STATE_INVENTORY]       = "Pokemon and items",
	[STATE_SETTINGS]        = "Settings",
    [STATE_ERROR]           = "Error",
};

state_event_func_t* const state_init_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_empty_event,
	[STATE_MAIN_MENU]       = pw_empty_event,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_empty_event,
	[STATE_CONNECT]         = pw_empty_event,
	[STATE_TRAINER_CARD]    = pw_trainer_card_init,
	[STATE_INVENTORY]       = pw_empty_event,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

state_event_func_t* const state_event_loop_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_empty_event,
	[STATE_MAIN_MENU]       = pw_empty_event,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_empty_event,
	[STATE_CONNECT]         = pw_empty_event,
	[STATE_TRAINER_CARD]    = pw_empty_event,
	[STATE_INVENTORY]       = pw_empty_event,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

state_input_func_t* const state_input_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_input,
	[STATE_SPLASH]          = pw_splash_handle_input,
	[STATE_MAIN_MENU]       = pw_menu_handle_input,
	[STATE_POKE_RADAR]      = pw_send_to_error,
	[STATE_DOWSING]         = pw_send_to_error,
	[STATE_CONNECT]         = pw_send_to_error,
	[STATE_TRAINER_CARD]    = pw_trainer_card_handle_input,
	[STATE_INVENTORY]       = pw_send_to_error,
	[STATE_SETTINGS]        = pw_send_to_error,
    [STATE_ERROR]           = pw_send_to_splash,
};

state_draw_func_t* const state_draw_init_funcs[] = {
    [STATE_SCREENSAVER]     = pw_screen_clear,
	[STATE_SPLASH]          = pw_splash_init_display,
	[STATE_MAIN_MENU]       = pw_menu_init_display,
	[STATE_POKE_RADAR]      = pw_screen_clear,
	[STATE_DOWSING]         = pw_screen_clear,
	[STATE_CONNECT]         = pw_screen_clear,
	[STATE_TRAINER_CARD]    = pw_trainer_card_init_display,
	[STATE_INVENTORY]       = pw_screen_clear,
	[STATE_SETTINGS]        = pw_screen_clear,
    [STATE_ERROR]           = pw_error_init_display,
};

state_draw_func_t* const state_draw_update_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_splash_update_display,
	[STATE_MAIN_MENU]       = pw_menu_update_display,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_empty_event,
	[STATE_CONNECT]         = pw_empty_event,
	[STATE_TRAINER_CARD]    = pw_trainer_card_draw_update,
	[STATE_INVENTORY]       = pw_empty_event,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

static pw_state_t pw_current_state = STATE_SCREENSAVER;
static pw_state_t pw_requested_state = 0;
static uint32_t pw_requests = 0;

static state_vars_t global_statevars = {0,};


void pw_request_state(pw_state_t s_to) {
    PW_SET_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE);
    pw_requested_state = s_to;
}

void pw_request_redraw() {
    PW_SET_REQUEST(pw_requests, PW_REQUEST_REDRAW);
}

bool pw_set_state(pw_state_t s) {
	if(s > N_STATES || s < 0) {
		// reset
		return false;
	}

	pw_state_t prev_state = pw_current_state;

	if(s != pw_current_state) {
		pw_current_state = s;
        pw_screen_clear();
        state_init_funcs[s](&global_statevars);
        state_draw_init_funcs[s](&global_statevars);
		// TODO: Notify when changed to and from state
		return true;
	} else {
		printf("Warn: tried to change state to same state\n");
		return false;
	}
}


pw_state_t pw_get_state() {
	return pw_current_state;
}


//may not be needed/used
void pw_state_init() {
    state_init_funcs[pw_current_state](&global_statevars);
}

void pw_state_run_event_loop() {
    if(PW_GET_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE)) {
        pw_set_state(pw_requested_state);
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE);
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_REDRAW);
    }

    state_event_loop_funcs[pw_current_state](&global_statevars);

    if(PW_GET_REQUEST(pw_requests, PW_REQUEST_REDRAW)) {
        state_draw_update_funcs[pw_current_state](&global_statevars);
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_REDRAW);
    }
}

/*
 *	Triggers when button is pressed.
 *	Want to spend as little time as possible in here
 */
void pw_state_handle_input(uint8_t b) {
    state_input_funcs[pw_current_state](&global_statevars, b);
}

void pw_state_draw_init() {
    state_draw_init_funcs[pw_current_state](&global_statevars);
}

void pw_state_draw_update() {
    state_draw_update_funcs[pw_current_state](&global_statevars);
}

void pw_send_to_error(state_vars_t *sv, uint8_t b) {
    pw_request_state(STATE_ERROR);
}

void pw_send_to_splash(state_vars_t *sv, uint8_t b) {
    pw_request_state(STATE_SPLASH);
}


/*
 *  ========================================
 *  State functions
 *  ========================================
 */
void pw_empty_event(state_vars_t *sv) {}
void pw_empty_input(state_vars_t *sv, uint8_t b) {}

void pw_splash_handle_input(state_vars_t *sv, uint8_t b) {
	switch(b) {
		case BUTTON_M: { pw_menu_set_cursor(sv, (MENU_SIZE-1)/2); break; }
		case BUTTON_L: { pw_menu_set_cursor(sv, MENU_SIZE-1); break; }
		case BUTTON_R:
		default: { pw_menu_set_cursor(sv, 0); break; }
	}
	pw_request_state(STATE_MAIN_MENU);
}

void pw_splash_init_display(state_vars_t *sv) {

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-64, 0,
        64, 48,
        PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2,
        PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
    );

    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-24-16,
        32, 24,
        PW_EEPROM_ADDR_IMG_ROUTE_LARGE,
        PW_EEPROM_SIZE_IMG_ROUTE_LARGE
    );

    pokemon_summary_t caught_pokemon;
    for(uint8_t i = 0; i < 3; i++) {
        pw_eeprom_read(
            PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+i*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE,
            (uint8_t*)(&caught_pokemon),
            PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE
        );
        if(caught_pokemon.le_species != 0) {
            pw_screen_draw_from_eeprom(
                i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_BALL,
                PW_EEPROM_SIZE_IMG_BALL
            );
        }
    }

    //uint16_t *items = (uint16_t*)(&caught_pokemon); // re-use the memory
    uint16_t items[6];
    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        //(uint8_t*)(&caught_pokemon),
        (uint8_t*)(items),
        PW_EEPROM_SIZE_OBTAINED_ITEMS
    );
    for(uint8_t i = 0; i < 3; i++) {
        if(items[2*i+1] != 0) {
            pw_screen_draw_from_eeprom(
                16+i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_ITEM,
                PW_EEPROM_SIZE_IMG_ITEM
            );
        }
    }

    uint8_t special_inventory;
    pw_eeprom_read(
        PW_EEPROM_ADDR_RECEIVED_BITFIELD,
        &special_inventory,
        1
    );

    for(uint8_t i = 0; i < 4; i++) {
        if( (special_inventory&(1<<i)) ) {
            pw_screen_draw_from_eeprom(
                16+i*8, SCREEN_HEIGHT-16,
                8, 8,
                PW_EEPROM_ADDR_IMG_CARD_SUITS+i*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL,
                PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL
            );
        }

    }

    if( special_inventory&(1<<5) ) {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_ITEM,
        (uint8_t*)items,
        PW_EEPROM_SIZE_EVENT_ITEM
    );
    if( items[3] != 0) {     // Check third word because first 6 bytes are useless
        pw_screen_draw_from_eeprom(
            8, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }

    health_data_t health_data;
    int err = pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );
    uint32_t today_steps = swap_bytes_u32(health_data.be_today_steps);
    pw_screen_draw_integer(today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);

    pw_screen_draw_horiz_line(0, SCREEN_HEIGHT-16, SCREEN_WIDTH, SCREEN_BLACK);
}

void pw_splash_update_display(state_vars_t *sv) {

    uint16_t frame_addr;
    if(sv->anim_frame) {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME1;
    } else {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2;
    }
    sv->anim_frame = !sv->anim_frame;

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-64, 0,
        64, 48,
        frame_addr,
        PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
    );

    health_data_t health_data;
    int err = pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );
    uint32_t today_steps = swap_bytes_u32(health_data.be_today_steps);
    pw_screen_draw_integer(today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);
    pw_screen_draw_horiz_line(48, SCREEN_HEIGHT-16, SCREEN_WIDTH-48, SCREEN_BLACK);

}

void pw_error_init_display(state_vars_t *sv) {
    pw_img_t sad_pokewalker_img   = {.height=48, .width=48, .data=sad_pokewalker, .size=576};
    pw_screen_draw_img(&sad_pokewalker_img, 0, 0);

}



#include <stdint.h>

#include "app_inventory.h"

#include "../route_info.h"
#include "../trainer_info.h"
#include "../eeprom.h"
#include "../eeprom_map.h"
#include "../screen.h"
#include "../buttons.h"

uint16_t inv_state = 0;;
uint8_t n_presents = 0;

struct owned_things_s {
    uint16_t le_species[3];
    uint16_t le_item[3];
} owned_things;

static void pw_inventory_draw_screen1();
static void pw_inventory_draw_screen2();
static void pw_inventory_update_screen1();
static void pw_inventory_update_screen2();
static void pw_inventory_move_cursor(state_vars_t *sv, int8_t m);

void pw_inventory_init(state_vars_t *sv) {
    inv_state = 0;
    sv->cursor = 0;
    sv->subscreen = 0;

    // get num pokemon
    pokemon_summary_t test_pokemon[3];

    pw_eeprom_read(
        PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY,
        (uint8_t*)(test_pokemon),
        PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE*3
    );

    for(uint8_t i = 0; i < 3; i++) {
        owned_things.le_species[i] = test_pokemon[i].le_species;
        if(test_pokemon[i].le_species != 0) inv_state |= 1<<(1+i);
    }

    struct {
        uint16_t le_item;
        uint16_t pad;
    } test_item[10];

    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        (uint8_t*)(test_item),
        PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE*3
    );

    // get num items
    for(uint8_t i = 0; i < 3; i++) {
        owned_things.le_item[i] = test_item[i].le_item;
        if(test_item[i].le_item != 0) inv_state |= 1<<(6+i);
    }

    pw_eeprom_read(
        PW_EEPROM_ADDR_PEER_PLAY_ITEMS,
        (uint8_t*)(test_item),
        PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE*10
    );

    // get num peer play items
    for(uint8_t i = 0; i < 10; i++) {
        if(test_item[i].le_item == 0) {
            n_presents = i;
            break;
        }
    }

    inv_state = 0x03df; // all items unlocked

}


void pw_inventory_init_display(state_vars_t *sv) {
    if(sv->subscreen == 0) {
        pw_inventory_draw_screen1(sv);
    } else {
        pw_inventory_draw_screen2(sv);
    }
}


void pw_inventory_update_display(state_vars_t *sv) {
    if(sv->subscreen == 0) {
        pw_inventory_update_screen1(sv);
    } else {
        pw_inventory_update_screen2(sv);
    }

}


void pw_inventory_handle_input(state_vars_t *sv, uint8_t b) {
    switch(b) {
        case BUTTON_L: { pw_inventory_move_cursor(sv, -1); break; };
        case BUTTON_M: { pw_inventory_move_cursor(sv,  0); break; };
        case BUTTON_R: { pw_inventory_move_cursor(sv, +1); break; };
    };

}

/*
 *  cursor:
 *
 *  0  1 2 3 4
 *  5  6 7 8 9
 *
 *
 */
static void pw_inventory_move_cursor(state_vars_t *sv, int8_t m) {
    int8_t c = sv->cursor;

    uint16_t is_filled = 0;
    if(sv->subscreen == 0) {
        do {
            c += m;
            is_filled = inv_state & (1<<c);
        } while( (!is_filled) && (c>=0) && (c<=9) );

        if(c < 0) {
            pw_request_state(STATE_MAIN_MENU); // back to main menu
        }
        if(c > 9) {
            sv->subscreen = 1;
            pw_request_redraw();
        }

    } else {

    }

    sv->cursor = c;
}

static void pw_inventory_draw_screen1(state_vars_t *sv) {

    pw_screen_draw_from_eeprom(
        0, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RETURN,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RETURN
    );

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-8, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RIGHT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RIGHT
    );

    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        PW_EEPROM_ADDR_IMG_MENU_TITLE_INVENTORY,
        PW_EEPROM_SIZE_IMG_MENU_TITLE_INVENTORY
    );

    uint8_t buf_pokeball[PW_EEPROM_SIZE_IMG_BALL];
    uint8_t buf_item[PW_EEPROM_SIZE_IMG_ITEM];

    pw_img_t pokeball = {.height=8, .width=8, .data=buf_pokeball, .size=PW_EEPROM_SIZE_IMG_BALL};
    pw_eeprom_read(PW_EEPROM_ADDR_IMG_BALL, buf_pokeball, PW_EEPROM_SIZE_IMG_BALL);

    pw_img_t item = {.height=8, .width=8, .data=buf_item, .size=PW_EEPROM_SIZE_IMG_ITEM};
    pw_eeprom_read(PW_EEPROM_ADDR_IMG_ITEM, buf_item, PW_EEPROM_SIZE_IMG_ITEM);

    uint8_t xs[] = {8, 24, 32, 40, 48};
    const uint8_t yp = 24, yi = 40;

    // draw normal pokeballs
    for(uint8_t i = 0; i < 4; i++) {
        if( inv_state & (1<<i) ) {
            pw_screen_draw_img(&pokeball, xs[i], yp);
        }
    }

    // draw normal items
    for(uint8_t i = 5; i < 9; i++) {
        if( inv_state & (1<<i) ) {
            pw_screen_draw_img(&item, xs[i%5], yi);
        }
    }


    if(inv_state & (1<<4)) {
        pw_screen_draw_from_eeprom(
            xs[4], yp,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    if(inv_state & 1<<9) {
        pw_screen_draw_from_eeprom(
            xs[4], yi,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }

    uint8_t cx = xs[ (sv->cursor)%5 ];
    uint8_t cy = (sv->cursor>5)?yi:yp;
    cy -= 8;

    uint16_t addr = (sv->anim_frame)?PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET;

    pw_screen_draw_from_eeprom(
        cx, cy,
        8, 8,
        addr,
        PW_EEPROM_SIZE_IMG_ARROW
    );

    /*
     *  Special pokemon is route template
     *  event pokemon is dowsed or gifted
     */

    /*
     *  For pokemon index (A, B, C) need to compare CAUGHT_POKEMON_SUMMARY.le_species
     *  to ROUTE_INFO.route_pokemon[i].le_species
     *  Probably same index order in all the stuff like small_anim and text_name
     *  Do similar stuff for item index with ROUTE_INFO.le_route_items and OBTAINED_ITEMS.le_item
     *  Can hardcode cursor for walk mon and special pokemon/items since there's only one option
     */
    route_info_t route_info;
    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)(&route_info), PW_EEPROM_SIZE_ROUTE_INFO);


    // IMG_EVENT_POKEMON_SMALL_ANIMATED
    // IMG_POKEMON_SMALL_ANIMATED
    // TREASURE_LARGE
    addr = PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED + sv->anim_frame*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME;
    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-8-32, SCREEN_HEIGHT-16-24,
        32, 24,
        addr,
        PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME
    );

    // TEXT_EVENT_ITEM_NAME
    // TEXT_EVENT_POKEMON_NAME
    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-16,
        80, 16,
        PW_EEPROM_ADDR_TEXT_POKEMON_NAME,
        PW_EEPROM_SIZE_TEXT_POKEMON_NAME
    );

    // draw box


    // TREASURE_LARGE for item view
    // IMG_EVENT_PoKEMON_SMALL_ANIMATED

}


static void pw_inventory_draw_screen2(state_vars_t *sv) {
    // PEER_PLAY_ITEMS {u16 item, u16 pad}[10]
    // PRESENT_LARGE

}


static void pw_inventory_update_screen1(state_vars_t *sv) {

}


static void pw_inventory_update_screen2(state_vars_t *sv) {

}

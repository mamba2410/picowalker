#include <stdint.h>

#include "app_inventory.h"

#include "../route_info.h"
#include "../trainer_info.h"
#include "../eeprom.h"
#include "../eeprom_map.h"
#include "../screen.h"
#include "../buttons.h"
#include "../globals.h"
#include "../types.h"

struct owned_things_s {
    uint16_t le_found[10];
    uint16_t le_presents[10];
    uint8_t n_presents;
} inventory;

enum search_type {
    SEARCH_POKEMON_NAME,
    SEARCH_POKEMON_SPRITE,
    SEARCH_ITEM_NAME,
    SEARCH_ITEM_SPRITE,
};

enum subscreen_type {
    SUBSCREEN_FOUND,
    SUBSCREEN_PRESENTS,
    N_SUBSCREENS,
};

static void pw_inventory_draw_screen1(state_vars_t *sv);
static void pw_inventory_draw_screen2(state_vars_t *sv);
static void pw_inventory_update_screen1(state_vars_t *sv);
static void pw_inventory_update_screen2(state_vars_t *sv);

static void pw_inventory_move_cursor(state_vars_t *sv, int8_t m);
static uint8_t pw_inventory_find_index(state_vars_t *sv, uint16_t data, enum search_type type);
static void pw_inventory_index_to_data(state_vars_t *sv, uint8_t *buf, uint8_t idx, enum search_type type);

draw_func_t* const draw_funcs[N_SUBSCREENS] = {
    [SUBSCREEN_FOUND]       = pw_inventory_draw_screen1,
    [SUBSCREEN_PRESENTS]    = pw_inventory_draw_screen2,
};

draw_func_t* const update_funcs[N_SUBSCREENS] = {
    [SUBSCREEN_FOUND]       = pw_inventory_update_screen1,
    [SUBSCREEN_PRESENTS]    = pw_inventory_update_screen2,
};


void pw_inventory_init(state_vars_t *sv) {
    inventory.n_presents = 0;
    for(uint8_t i = 0; i < 10; i++) {
        inventory.le_found[i] = 0;
        inventory.le_presents[i] = 0;
    }
    sv->current_cursor = 0;
    sv->cursor_2 = 0;
    sv->current_substate = SUBSCREEN_FOUND;
    sv->substate_2 = SUBSCREEN_FOUND;


    /*
     *  Check for pokemon
     *  1x walking mon      [0]
     *  3x caught inventory [1..3]
     *  1x special/event    [4]
     */
    pokemon_summary_t test_pokemon[3];

    pw_eeprom_read(
        PW_EEPROM_ADDR_ROUTE_INFO+0x00,
        (uint8_t*)(test_pokemon),
        sizeof(pokemon_summary_t)
    );
    inventory.le_found[0] = test_pokemon[0].le_species;

    pw_eeprom_read(
        PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY,
        (uint8_t*)(test_pokemon),
        PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE*3
    );

    for(uint8_t i = 0; i < 3; i++) {
        inventory.le_found[i+1] = test_pokemon[i].le_species;
    }

    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_POKEMON_BASIC_DATA,
        (uint8_t*)(test_pokemon),
        PW_EEPROM_SIZE_EVENT_POKEMON_BASIC_DATA
    );
    inventory.le_found[4] = test_pokemon[0].le_species;



    /*
     *  Check for items
     *  1x empty            [5]
     *  3x dowsed items     [6..8]
     *  1x special/event    [9]
     */
    struct {
        uint16_t le_item;
        uint16_t pad;
    } test_item[10];

    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        (uint8_t*)(test_item),
        PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE*3
    );

    for(uint8_t i = 0; i < 3; i++) {
        inventory.le_found[i+6] = test_item[i].le_item;
    }

    uint16_t *buf = (uint16_t*)test_item;
    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_ITEM,
        (uint8_t*)buf,
        PW_EEPROM_SIZE_EVENT_ITEM
    );
    inventory.le_found[9] = buf[3]; // 6 bytes zero then u16 le_item



    /*
     *  Check for presents
     *  10x peer-play presents [0..9]
     */
    pw_eeprom_read(
        PW_EEPROM_ADDR_PEER_PLAY_ITEMS,
        (uint8_t*)(test_item),
        PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE*10
    );
    for(uint8_t i = 0; i < 10; i++) {
        inventory.le_presents[i] = test_item[i].le_item;
    }

    // count presents
    for(
            inventory.n_presents = 0;
            inventory.le_presents[inventory.n_presents] != 0 && inventory.n_presents < 10;
            inventory.n_presents++
        ) ;

    //inv_state = 0x03df; // all pokemon/items unlocked
    //n_presents = 10;    // we have 10 presents :D
}


void pw_inventory_init_display(state_vars_t *sv) {
    draw_funcs[sv->current_substate](sv);
}


void pw_inventory_update_display(state_vars_t *sv) {
    if(sv->substate_2 != sv->current_substate) {
        pw_screen_clear();
        draw_funcs[sv->current_substate](sv);
    } else {
        update_funcs[sv->current_substate](sv);
    }

    sv->substate_2 = sv->current_substate;
}


void pw_inventory_handle_input(state_vars_t *sv, uint8_t b) {
    switch(b) {
        case BUTTON_L: { pw_inventory_move_cursor(sv, -1); break; };
        case BUTTON_M: {
                           if(sv->current_substate == SUBSCREEN_FOUND) {
                               sv->current_substate = SUBSCREEN_PRESENTS;
                           } else {
                               pw_request_state(STATE_SPLASH);
                           }
        };
        case BUTTON_R: { pw_inventory_move_cursor(sv, +1); break; };
    };

}

/*
 *  cursor found screen:
 *
 *  0  1 2 3 4  // pokemon
 *  5  6 7 8 9  // items
 *
 *  0 = walking mon
 *  1-3 = caught mon
 *  4 = special/gifted mon
 *  5 = always empty
 *  6-8 = dowsed items
 *  9 = special/gifted item
 */
static void pw_inventory_move_cursor(state_vars_t *sv, int8_t m) {

    switch(sv->current_substate) {
        case SUBSCREEN_FOUND: {
            uint16_t is_filled = 0;

            // move cursor by `m` until it hits a nonzero bit, cursor<0 or cursor>=10
            do {
                sv->current_cursor += m;
                is_filled = inventory.le_found[sv->current_cursor] != 0;
            } while( (!is_filled) && (sv->current_cursor>=0) && (sv->current_cursor<=9) );

            if(sv->current_cursor < 0) {
                sv->current_cursor = 4;
                pw_request_state(STATE_MAIN_MENU); // back to main menu
            }
            if(sv->current_cursor > 9) {
                sv->current_substate = SUBSCREEN_PRESENTS;  // change to presents screen
                sv->current_cursor = 0;
            }
            break;
        }
        case SUBSCREEN_PRESENTS: {
            sv->current_cursor += m;
            if(sv->current_cursor < 0) {
                sv->current_substate = SUBSCREEN_FOUND;
                sv->current_cursor = 10;
                pw_inventory_move_cursor(sv, -1);   // laziest way of setting cursor to last non-empty slot
            }

            if(sv->current_cursor >= inventory.n_presents)
                sv->current_cursor = inventory.n_presents-1;

            break;
        }
        default: break;
    }

    pw_request_redraw();
}

static void draw_cursor(state_vars_t *sv) {
    uint8_t cx=0, cy=0;
    uint8_t xs[] = {8, 24, 32, 40, 48};
    const uint8_t yp = 24, yi = 40;
    uint8_t x0 = 16, y0 = 24;

    switch(sv->current_substate) {
        case SUBSCREEN_FOUND: {
            cx = xs[ (sv->current_cursor)%5 ];
            cy = (sv->current_cursor>5)?yi:yp;
            cy -= 8;

            break;
        }
        case SUBSCREEN_PRESENTS: {
            cx = x0 + 8*(sv->current_cursor%5);
            cy = y0 + 16*(sv->current_cursor/5) - 8;

            break;
        }
        default: break;
    }

    uint16_t addr = (sv->anim_frame)?PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET;

    pw_screen_draw_from_eeprom(
        cx, cy,
        8, 8,
        addr,
        PW_EEPROM_SIZE_IMG_ARROW
    );
}


static void draw_animated_sprite(state_vars_t *sv) {

    uint8_t *buf = eeprom_buf;
    uint8_t idx, w;
    size_t size;
    pw_img_t sprite;
    enum search_type type;


    /*
     *  Draw icon
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     */
    if(sv->current_cursor < 5) {
        type = SEARCH_POKEMON_SPRITE;
        idx = pw_inventory_find_index(sv, inventory.le_found[sv->current_cursor], type);
        size = PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED;
    } else {
        type = SEARCH_ITEM_SPRITE;
        idx = 0;
        size = PW_EEPROM_SIZE_IMG_TREASURE_LARGE;;
    }

    pw_inventory_index_to_data(sv, buf, idx, type);
    sprite = (pw_img_t){.data=buf, .width=32, .height=24, .size=size};
    pw_screen_draw_img(&sprite, SCREEN_WIDTH-32-4, SCREEN_HEIGHT-16-24);

}


static void draw_name(state_vars_t *sv) {

    uint8_t *buf = eeprom_buf;
    uint8_t idx, w;
    size_t size;
    enum search_type type;


    /*
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     */
    if(sv->current_cursor < 5 && sv->current_substate == SUBSCREEN_FOUND) {
        type = SEARCH_POKEMON_NAME;
        w = 80;
        size = PW_EEPROM_SIZE_TEXT_POKEMON_NAME;
    } else {
        type = SEARCH_ITEM_NAME;
        w = 96;
        size = PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE;
    }

    uint16_t data = (sv->current_substate==SUBSCREEN_FOUND)?
        inventory.le_found[sv->current_cursor]:inventory.le_presents[sv->current_cursor];

    idx = pw_inventory_find_index(sv, data, type);
    pw_inventory_index_to_data(sv, buf, idx, type);
    pw_img_t sprite = {.data=buf, .width=w, .height=16, .size=size};
    pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);

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

    // Draw icons
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
        if( inventory.le_found[i] != 0 ) {
            pw_screen_draw_img(&pokeball, xs[i], yp);
        }
    }

    // draw normal items
    for(uint8_t i = 5; i < 9; i++) {
        if( inventory.le_found[i] != 0 ) {
            pw_screen_draw_img(&item, xs[i%5], yi);
        }
    }


    // draw special pokeball
    if( inventory.le_found[4] != 0 ) {
        pw_screen_draw_from_eeprom(
            xs[4], yp,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    // draw special item
    if(inventory.le_found[9] != 0) {
        pw_screen_draw_from_eeprom(
            xs[4], yi,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }


    draw_cursor(sv);
    draw_name(sv);
    draw_animated_sprite(sv);

    // Draw text box

}


static void pw_inventory_draw_screen2(state_vars_t *sv) {
    // PEER_PLAY_ITEMS {u16 item, u16 pad}[10]
    // PRESENT_LARGE

    pw_screen_draw_from_eeprom(
        0, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_LEFT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_LEFT
    );

    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        PW_EEPROM_ADDR_IMG_MENU_TITLE_INVENTORY,
        PW_EEPROM_SIZE_IMG_MENU_TITLE_INVENTORY
    );


    uint8_t buf_item[PW_EEPROM_SIZE_IMG_ITEM];
    pw_img_t item = {.height=8, .width=8, .data=buf_item, .size=PW_EEPROM_SIZE_IMG_ITEM};
    pw_eeprom_read(PW_EEPROM_ADDR_IMG_ITEM, buf_item, PW_EEPROM_SIZE_IMG_ITEM);

    uint8_t x0 = 16, y0 = 24;
    for(uint8_t i = 0; i < inventory.n_presents; i++) {
        pw_screen_draw_img(&item,
            x0 + 8*(i%5),
            y0 + 16*(i/5)
        );
    }

    // don't draw this if we don't have presents
    if(inventory.n_presents > 0) {

        draw_cursor(sv);

        pw_screen_draw_from_eeprom(
            SCREEN_WIDTH-32-4, SCREEN_HEIGHT-16-24,
            32, 24,
            PW_EEPROM_ADDR_IMG_PRESENT_LARGE,
            PW_EEPROM_SIZE_IMG_PRESENT_LARGE
        );

        /*
        uint8_t buf[PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE];
        pw_img_t sprite = {.width=96, .height=16, .data=buf, .size=PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE};

        uint8_t idx = pw_inventory_find_index(sv, owned_things.le_presents[sv->current_cursor], SEARCH_ITEM_NAME);
        pw_inventory_index_to_data(sv, buf, idx, SEARCH_ITEM_NAME);

        pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);
        */
        draw_name(sv);

    }


}


static void pw_inventory_update_screen1(state_vars_t *sv) {
    pw_screen_clear_area(0, 16, 56, 8);
    pw_screen_clear_area(0, 32, 56, 8);

    draw_cursor(sv);
    draw_name(sv);
    draw_animated_sprite(sv);

}


static void pw_inventory_update_screen2(state_vars_t *sv) {

    uint8_t x0 = 16, y0 = 24;
    pw_screen_clear_area(x0, y0-8, 40, 8);
    pw_screen_clear_area(x0, y0-8+16, 40, 8);


    // don't draw this if we don't have presents
    if(inventory.n_presents > 0) {

        draw_cursor(sv);

        /*
        uint8_t buf[PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE];
        pw_img_t sprite = {.width=96, .height=16, .data=buf, .size=PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE};

        uint8_t idx = pw_inventory_find_index(sv, owned_things.le_presents[sv->current_cursor], SEARCH_ITEM_NAME);
        pw_inventory_index_to_data(sv, buf, idx, SEARCH_ITEM_NAME);

        pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);
        */
        draw_name(sv);


    }
}


static uint8_t pw_inventory_find_index(state_vars_t *sv, uint16_t data, enum search_type type) {
    uint8_t idx;

    switch(type) {
        case SEARCH_POKEMON_SPRITE:
        case SEARCH_POKEMON_NAME: {
            // pokemon name
            uint16_t le_species = data;

            pokemon_summary_t route_pokemon[3];
            pw_eeprom_read(
                PW_EEPROM_ADDR_ROUTE_INFO+0x52, // get route available pokemon
                (uint8_t*)(route_pokemon),
                3*sizeof(pokemon_summary_t)
            );

            for(idx = 0; idx < 3; idx++)
                if(route_pokemon[idx].le_species == le_species) break;

            if(idx == 3) {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_ROUTE_INFO+0x00,
                    (uint8_t*)route_pokemon,
                    sizeof(pokemon_summary_t)
                );
                if(route_pokemon[0].le_species == le_species)
                    idx = 4;
            }

            // idx == 3 means special pokemon
            break;
        }
        case SEARCH_ITEM_NAME: {
            // item name
            uint16_t le_item = data;

            struct {
                uint16_t le_item;
            } items[10];

            pw_eeprom_read(
                PW_EEPROM_ADDR_ROUTE_INFO+0x8c,
                (uint8_t*)(items),
                sizeof(items)
            );

            for(idx = 0; idx < 10; idx++) {
                if(items[idx].le_item == le_item) break;
            }

            break;
            // idx == 11 means special item
        }
        case SEARCH_ITEM_SPRITE:
        default: {
            idx = 0;
            break;
        }
    }
    return idx;
}

static void pw_inventory_index_to_data(state_vars_t *sv, uint8_t *buf, uint8_t idx, enum search_type type) {
    switch(type) {
        case SEARCH_POKEMON_SPRITE: {
            if(idx < 3) {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+(2*idx+sv->anim_frame)*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME,
                    buf,
                    PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME
                );
            } else if (idx == 3){
                pw_eeprom_read(
                    PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED,
                    buf,
                    PW_EEPROM_SIZE_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME
                );
            } else {
                // walk pokemon
                pw_eeprom_read(
                    PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED+sv->anim_frame*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME,
                    buf,
                    PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME
                );
            }
            break;
        }
        case SEARCH_ITEM_NAME: {
            if(idx < 10) {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_TEXT_ITEM_NAMES+idx*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE,
                    buf,
                    PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE
                );
            } else {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_TEXT_EVENT_ITEM_NAME,
                    buf,
                    PW_EEPROM_SIZE_TEXT_EVENT_ITEM_NAME
                );
            }

            break;
        }
        case SEARCH_POKEMON_NAME: {
            if(idx < 3) {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_TEXT_POKEMON_NAMES+idx*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
                    buf,
                    PW_EEPROM_SIZE_TEXT_POKEMON_NAME
                );
            } else if(idx == 3) {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_TEXT_EVENT_POKEMON_NAME,
                    buf,
                    PW_EEPROM_SIZE_TEXT_EVENT_POKEMON_NAME
                );
            } else {
                pw_eeprom_read(
                    PW_EEPROM_ADDR_TEXT_POKEMON_NAME,
                    buf,
                    PW_EEPROM_SIZE_TEXT_POKEMON_NAME
                );
            }
            break;
        }
        case SEARCH_ITEM_SPRITE: {
            pw_eeprom_read(
                PW_EEPROM_ADDR_IMG_TREASURE_LARGE,
                buf,
                PW_EEPROM_SIZE_IMG_TREASURE_LARGE
            );
            break;
        }
    }   // switch
}


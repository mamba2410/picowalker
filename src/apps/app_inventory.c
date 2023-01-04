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
    uint16_t le_current;
    uint16_t le_species[4];
    uint16_t le_item[4];
    uint16_t le_presents[10];
} owned_things;

enum search_type {
    SEARCH_POKEMON_NAME,
    SEARCH_POKEMON_SPRITE,
    SEARCH_ITEM_NAME,
    SEARCH_ITEM_SPRITE,
};

static void pw_inventory_draw_screen1();
static void pw_inventory_draw_screen2();
static void pw_inventory_update_screen1();
static void pw_inventory_update_screen2();
static void pw_inventory_move_cursor(state_vars_t *sv, int8_t m);
static uint8_t pw_inventory_find_index(state_vars_t *sv, uint16_t data, enum search_type type);
static void pw_inventory_index_to_data(state_vars_t *sv, uint8_t *buf, uint8_t idx, enum search_type type);


/*
 *  TODO: optimisation
 *  chance owned_items to match cursor position
 *  so that we don't need too much logic to separate drawing items and pokemon
 *  will need padding for the empty item slot
 *  Do all the sorting in `init` and the rest follows
 */
void pw_inventory_init(state_vars_t *sv) {
    inv_state = 1;
    n_presents = 0;
    sv->cursor = 0;
    sv->prev_cursor = 0;
    sv->subscreen = 0;
    sv->prev_subscreen = 0;

    // get num pokemon
    pokemon_summary_t test_pokemon[3];

    pw_eeprom_read(
        PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY,
        (uint8_t*)(test_pokemon),
        PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE*3
    );

    /*
     *  Check for pokemon
     *  3x caught inventory [1..3]
     *  1x special/event    [4]
     *  1x walking mon      [0]
     */
    for(uint8_t i = 0; i < 3; i++) {
        owned_things.le_species[i] = test_pokemon[i].le_species;
        if(test_pokemon[i].le_species != 0) inv_state |= 1<<(1+i);
    }

    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_POKEMON_BASIC_DATA,
        (uint8_t*)(test_pokemon),
        PW_EEPROM_SIZE_EVENT_POKEMON_BASIC_DATA
    );
    owned_things.le_species[3] = test_pokemon[0].le_species;

    pw_eeprom_read(
        PW_EEPROM_ADDR_ROUTE_INFO+0x00,
        (uint8_t*)(test_pokemon),
        sizeof(pokemon_summary_t)
    );
    owned_things.le_current = test_pokemon[0].le_species;


    /*
     *  Check for items
     *  1x empty            (5)
     *  3x dowsed items     [0..3] (6..8)
     *  1x special/event    [4] (9)
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
        owned_things.le_item[i] = test_item[i].le_item;
        if(test_item[i].le_item != 0) inv_state |= 1<<(6+i);
    }

    uint16_t *buf = (uint16_t*)test_item;
    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_ITEM,
        (uint8_t*)buf,
        PW_EEPROM_SIZE_EVENT_ITEM
    );
    owned_things.le_item[4] = buf[3]; // 6 bytes zero then u16 le_item

    pw_eeprom_read(
        PW_EEPROM_ADDR_PEER_PLAY_ITEMS,
        (uint8_t*)(test_item),
        PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE*10
    );


    /*
     *  Check for presents
     *  10x peer-play presents [0..9]
     */
    for(uint8_t i = 0; i < 10; i++) {
        owned_things.le_presents[i] = test_item[i].le_item;
    }

    // count presents
    for(n_presents = 0; owned_things.le_presents[n_presents] != 0 && n_presents < 10; n_presents++) ;

    //inv_state = 0x03df; // all pokemon/items unlocked
    //n_presents = 10;    // we have 10 presents :D
}




// TODO: make this a jump table
// TODO: make subscreen static enum
void pw_inventory_init_display(state_vars_t *sv) {
    if(sv->subscreen == 0) {
        pw_inventory_draw_screen1(sv);
    } else {
        pw_inventory_draw_screen2(sv);
    }
}


void pw_inventory_update_display(state_vars_t *sv) {
    if(sv->prev_subscreen != sv->subscreen) {
        pw_screen_clear();
        // TODO: make this a jump table
        if(sv->subscreen == 0)
            pw_inventory_draw_screen1(sv);
        else
            pw_inventory_draw_screen2(sv);
    } else {
        if(sv->subscreen == 0)
            pw_inventory_update_screen1(sv);
        else
            pw_inventory_update_screen2(sv);
    }

    sv->prev_subscreen = sv->subscreen;
}


void pw_inventory_handle_input(state_vars_t *sv, uint8_t b) {
    switch(b) {
        case BUTTON_L: { pw_inventory_move_cursor(sv, -1); break; };
        case BUTTON_M: {
                           if(sv->subscreen == 0) {
                               sv->subscreen = 1;
                           } else {
                               pw_request_state(STATE_SPLASH);
                           }
        };
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

    if(sv->subscreen == 0) {
        // pokemon and items screen
        uint16_t is_filled = 0;
        // move cursor by `m` until it hits a nonzero bit, cursor<0 or cursor>=10
        do {
            sv->cursor += m;
            is_filled = inv_state & (1<<sv->cursor);
        } while( (!is_filled) && (sv->cursor>=0) && (sv->cursor<=9) );

        if(sv->cursor < 0) {
            sv->cursor = 4;
            pw_request_state(STATE_MAIN_MENU); // back to main menu
        }
        if(sv->cursor > 9) {
            sv->subscreen = 1;  // change to presents screen
            sv->cursor = 0;
        }

    } else {
        // presents screen
        sv->cursor += m;
        if(sv->cursor < 0) {
            sv->subscreen = 0;
            sv->cursor = 10;
            pw_inventory_move_cursor(sv, -1);   // laziest way of setting cursor to last non-empty slot
        }
        if(sv->cursor>=n_presents) sv->cursor = n_presents-1;
    }

    pw_request_redraw();
}

// TODO: separate into smaller functions to share with update
static void pw_inventory_draw_screen1(state_vars_t *sv) {

    // Header
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

    // Draw cursor
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


    // Draw selected name and sprite

    // 96x16 = 32x24x2 = 384 bytes
    uint8_t buf[PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED];
    uint8_t is_pokemon = (sv->cursor < 5);
    uint8_t idx, w;
    size_t size;
    enum search_type type;


    /*
     *  Draw name
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     *  TODO: simplify index with `owned_things` restructure
     *  TODO: make this a function
     */
    if(is_pokemon) {
        type = SEARCH_POKEMON_NAME;
        uint16_t species;
        // relies on le_cpecies[-1] = le_current
        idx = pw_inventory_find_index(sv, owned_things.le_species[sv->cursor-1], type);
        w = 80;
        size = PW_EEPROM_SIZE_TEXT_POKEMON_NAME;
    } else {
        type = SEARCH_ITEM_NAME;
        idx = pw_inventory_find_index(sv, owned_things.le_item[sv->cursor - 6], type);
        w = 96;
        size = PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE;
    }

    pw_inventory_index_to_data(sv, buf, idx, type);
    pw_img_t sprite = {.data=buf, .width=w, .height=16, .size=size};
    pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);


    /*
     *  Draw icon
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     *  TODO: simplify index with `owned_things` restructure
     *  TODO: make this a function
     */
    if(is_pokemon) {
        type = SEARCH_POKEMON_SPRITE;
        idx = pw_inventory_find_index(sv, owned_things.le_species[sv->cursor-1], type);
        size = PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED;
    } else {
        type = SEARCH_ITEM_SPRITE;
        idx = 0;
        size = PW_EEPROM_SIZE_IMG_TREASURE_LARGE;;
    }

    pw_inventory_index_to_data(sv, buf, idx, type);
    sprite = (pw_img_t){.data=buf, .width=32, .height=24, .size=size};
    pw_screen_draw_img(&sprite, SCREEN_WIDTH-32, SCREEN_HEIGHT-16-24);

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
    for(uint8_t i = 0; i < n_presents; i++) {
        pw_screen_draw_img(&item,
            x0 + 8*(i%5),
            y0 + 16*(i/5)
        );
    }

    // don't draw this if we don't have presents
    if(n_presents > 0) {
        uint8_t cx = x0 + 8*(sv->cursor%5);
        uint8_t cy = y0 + 16*(sv->cursor/5) - 8;

        uint16_t addr = (sv->anim_frame)?PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET;

        pw_screen_draw_from_eeprom(
            cx, cy,
            8, 8,
            addr,
            PW_EEPROM_SIZE_IMG_ARROW
        );

        pw_screen_draw_from_eeprom(
            SCREEN_WIDTH-8-32, SCREEN_HEIGHT-16-24,
            32, 24,
            PW_EEPROM_ADDR_IMG_PRESENT_LARGE,
            PW_EEPROM_SIZE_IMG_PRESENT_LARGE
        );

        uint8_t buf[PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE];
        pw_img_t sprite = {.width=96, .height=16, .data=buf, .size=PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE};

        uint8_t idx = pw_inventory_find_index(sv, owned_things.le_presents[sv->cursor], SEARCH_ITEM_NAME);
        pw_inventory_index_to_data(sv, buf, idx, SEARCH_ITEM_NAME);

        pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);

    }


}


static void pw_inventory_update_screen1(state_vars_t *sv) {
    pw_screen_clear_area(0, 16, 56, 8);
    pw_screen_clear_area(0, 32, 56, 8);

    uint8_t xs[] = {8, 24, 32, 40, 48};
    const uint8_t yp = 24, yi = 40;

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

    // 96x16 = 32x24x2 = 384 bytes
    uint8_t buf[PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED];
    uint8_t is_pokemon = (sv->cursor < 5);
    uint8_t idx, w;
    size_t size;
    enum search_type type;

    /*
     *  Draw name
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     */
    if(is_pokemon) {
        type = SEARCH_POKEMON_NAME;
        idx = pw_inventory_find_index(sv, owned_things.le_species[sv->cursor-1], type);
        w = 80;
        size = PW_EEPROM_SIZE_TEXT_POKEMON_NAME;
    } else {
        type = SEARCH_ITEM_NAME;
        idx = pw_inventory_find_index(sv, owned_things.le_item[sv->cursor - 6], type);
        w = 96;
        size = PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE;
    }

    pw_inventory_index_to_data(sv, buf, idx, type);
    pw_img_t sprite = {.data=buf, .width=w, .height=16, .size=size};
    pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);


    /*
     *  Draw icon
     *  cursor < 5 -> pokemon
     *  cursor > 4 -> item
     */
    if(is_pokemon) {
        type = SEARCH_POKEMON_SPRITE;
        idx = pw_inventory_find_index(sv, owned_things.le_species[sv->cursor-1], type);
        size = PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED;
    } else {
        type = SEARCH_ITEM_SPRITE;
        idx = 0;
        size = PW_EEPROM_SIZE_IMG_TREASURE_LARGE;;
    }

    pw_inventory_index_to_data(sv, buf, idx, type);
    sprite = (pw_img_t){.data=buf, .width=32, .height=24, .size=size};
    pw_screen_draw_img(&sprite, SCREEN_WIDTH-32, SCREEN_HEIGHT-16-24);

}


static void pw_inventory_update_screen2(state_vars_t *sv) {

    uint8_t x0 = 16, y0 = 24;
    pw_screen_clear_area(x0, y0-8, 40, 8);
    pw_screen_clear_area(x0, y0-8+16, 40, 8);


    // don't draw this if we don't have presents
    if(n_presents > 0) {
        uint8_t cx = x0 + 8*(sv->cursor%5);
        uint8_t cy = y0 + 16*(sv->cursor/5) - 8;

        uint16_t addr = (sv->anim_frame)?PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET;

        pw_screen_draw_from_eeprom(
            cx, cy,
            8, 8,
            addr,
            PW_EEPROM_SIZE_IMG_ARROW
        );

        uint8_t buf[PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE];
        pw_img_t sprite = {.width=96, .height=16, .data=buf, .size=PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE};

        uint8_t idx = pw_inventory_find_index(sv, owned_things.le_presents[sv->cursor], SEARCH_ITEM_NAME);
        pw_inventory_index_to_data(sv, buf, idx, SEARCH_ITEM_NAME);

        pw_screen_draw_img(&sprite, 0, SCREEN_HEIGHT-16);


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


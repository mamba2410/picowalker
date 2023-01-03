#include <stdint.h>

#include "app_inventory.h"

#include "../eeprom.h"
#include "../screen.h"

static int8_t cursor = 0;
static int8_t screen_state = 0;
static uint8_t anim_frame = 0;

struct inventory_state_s {
    uint8_t n_pokemon: 2;       // range 0-3
    uint8_t n_items: 2;         // range 0-3
    uint8_t n_presents: 4;      // range 0-10
    uint8_t special_pokemon: 1; // range 0-1
    uint8_t special_item: 1;    // range 0-1
} inv_state;

struct owned_things_s {
    uint16_t le_species[3];
    uint16_t le_item[3];
} owned_things;

static void pw_inventory_draw_screen1();
static void pw_inventory_draw_screen2();
static void pw_inventory_update_screen1();
static void pw_inventory_update_screen2();

void pw_inventory_init() {
    cursor = 0;
    screen_state = 0;

    // get num pokemon
    pokemon_summary_t test_pokemon;
    for(uint8_t i = 0; i < 3; i++) {
        pw_eeprom_read(
            PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+i*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE,
            (uint8_t*)(&test_pokemon),
            PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE
        );

        owned_things.le_species[i] = test_pokemon.le_species;
        if(test_pokemon.le_species == 0) {
            inv_state.n_pokemon = i;
            break;
        }

    }

    struct {
        uint16_t le_item;
        uint16_t pad;
    } test_item;

    // get num items
    for(uint8_t i = 0; i < 3; i++) {
        pw_eeprom_read(
            PW_EEPROM_ADDR_OBTAINED_ITEMS+i*PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE,
            (uint8_t*)(&test_item),
            PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE
        );

        owned_things.le_item[i] = test_item.le_item;
        if(test_item.le_item == 0) {
            inv_state.n_items = i;
            break;
        }
    }


    // get num peer play items
    for(uint8_t i = 0; i < 10; i++) {
        pw_eeprom_read(
            PW_EEPROM_ADDR_PEER_PLAY_ITEMS+i*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE,
            (uint8_t*)(&test_item),
            PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE
        );

        if(test_item.le_item == 0) {
            inv_state.n_presents = i;
            break;
        }
    }

}


void pw_inventory_init_display() {

}


void pw_inventory_update_display() {

}


void pw_inventory_handle_input(uint8_t b) {

}

void pw_inventory_move_cursor(int8_t m) {
}


static void pw_inventory_draw_screen1() {
    uint8_t buf_pokeball[PW_EEPROM_SIZE_IMG_BALL];
    uint8_t buf_item[PW_EEPROM_SIZE_IMG_ITEM];

    pw_img_t pokeball = {.height=8, .width=8, .data=buf_pokeball, .len=PW_EEPROM_SIZE_IMG_BALL.};
    pw_eeprom_read(PW_EEPROM_ADDR_IMG_BALL, buf_pokeball, PW_EEPROM_SIZE_IMG_BALL);

    pw_img_t item = {.height=8, .width=8, .data=buf_item, .len=PW_EEPROM_SIZE_IMG_ITEM.};
    pw_eeprom_read(PW_EEPROM_ADDR_IMG_ITEM, buf_item, PW_EEPROM_SIZE_IMG_ITEM);

    uint8_t xs[] = {8, 24, 32, 40, 48};
    const uint8_t yp = 24, yi = 40;

    for(uint8_t i = 0; i < inv_status.n_pokemon+1; i++) {
        pw_screen_draw_img(&pokeball, xs[i], yp);
    }

    for(uint8_t i = 0; i < inv_status.n_items; i++) {
        pw_screen_draw_img(&item, xs[i+1], yi);
    }

    if(inv_status.special_pokemon) {
        pw_screen_draw_from_eeprom(
            xs[4], yp,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    if(inv_status.special_item) {
        pw_screen_draw_from_eeprom(
            xs[4], yi,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }

    cx = xs[ (cursor)%4 ];
    cy = (cursor>5)?yi:yp;

    const uint16_t addr = (anim_frame)?PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET;

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
    addr = PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED + anim_frame*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME
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

    );


    // TREASURE_LARGE for item view
    // IMG_EVENT_PoKEMON_SMALL_ANIMATED

}


static void pw_inventory_draw_screen2() {
    // PEER_PLAY_ITEMS {u16 item, u16 pad}[10]
    // PRESENT_LARGE

}


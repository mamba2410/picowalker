#include <stdint.h>
#include <stddef.h>

#include "../states.h"
#include "../types.h"
#include "../globals.h"
#include "../screen.h"
#include "../utils.h"


/*
 *  reg_x = pokemon_id
 *  reg_a = pokemon_number (0=none, 1..3=route, 4=event)
 *  reg_b = exclamation_level (1..3)
 */
void pw_poke_radar_init(state_vars_t *sv) {

}

void pw_poke_radar_init_display(state_vars_t *sv) {

}

void pw_poke_radar_update_display(state_vars_t *sv) {

}

void pw_poke_radar_handle_input(state_vars_t *sv, uint8_t b) {

}


/*
 *  TODO: Give index 1-4 instead of species?
 */
uint16_t pw_poke_radar_choose_pokemon(state_vars_t *sv, route_info_t *ri, health_data_t *hd) {
    uint32_t today_steps = swap_bytes_u32(hd->be_today_steps);

    special_inventory_t inv;
    pw_eeprom_read(PW_EEPROM_ADDR_RECEIVED_BITFIELD, (uint8_t*)&inv, 1);

    uint8_t event_index;
    pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_POKEMON_EVENT_INDEX, &event_index, 1);

    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_POKEMON, (uint8_t*)(ri->route_pokemon), PW_EEPROM_SIZE_ROUTE_POKEMON);

    // event
    if( event_index > 0 && !inv.event_pokemon) {

        pokemon_summary_t event_pokemon;
        pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_POKEMON_BASIC_DATA, (uint8_t*)&event_pokemon, sizeof(pok));
        struct {
            uint16_t le_steps;
            uint8_t chance;
        } special;
        pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_POKEMON_STEPS_REQUIRED, (uint8_t*)&special, sizeof(special));

        if(today_steps > special.le_steps) {
            if( pw_rand()%100 < special.chance ) {
                return pw_rand()%2 ? ri->route_pokemon[2].le_species:event_pokemon.le_species;
            }
        }
    }

    // not event

    uint8_t rnd = pw_rand()%100;

    for(uint8_t i = 0; i < 3; i++) {
        if(today_steps > ri->le_route_pokemon_steps[i]) {
            if(rnd < ri->route_pokemon_percent[i]) {
                uint8_t offset = pw_rand()%2;
                return ri->route_pokemon[3-i+offset].le_species;
            }
        }
    }

    return ri->route_pokemon[2].le_species;

}


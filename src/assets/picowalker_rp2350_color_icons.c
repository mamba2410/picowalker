#include <stddef.h>
#include <stdio.h>
#include "picowalker_rp2350_color_icons.h"

const color_icons_t icons_map[ICONS_COUNT] = {
    { 0x0000, 0x000000,  698, 2048, 32, 32 },  // 0x0000_pokewalker_big.png
    { 0x0460, 0x0002BA,   94,  128,  8,  8 },  // 0x0460_pokeball.png
    { 0x0470, 0x000318,   94,  128,  8,  8 },  // 0x0470_pokeball_event.png
    { 0x0488, 0x000376,   93,  128,  8,  8 },  // 0x0488_item.png
    { 0x0498, 0x0003D3,   93,  128,  8,  8 },  // 0x0498_item_event.png
    { 0x04A8, 0x000430,   96,  128,  8,  8 },  // 0x04A8_map_icon.png
    { 0x04B8, 0x000490,   60,  128,  8,  8 },  // 0x04B8_card_heart.png
    { 0x04C8, 0x0004CC,   51,  128,  8,  8 },  // 0x04C8_card_spade.png
    { 0x04D8, 0x0004FF,   48,  128,  8,  8 },  // 0x04D8_card_diamond.png
    { 0x04E8, 0x00052F,   48,  128,  8,  8 },  // 0x04E8_card_club.png
    { 0x0660, 0x00055F,   89,  128,  8,  8 },  // 0x0660_low_battery.png
    { 0x06D0, 0x0005B8,  285,  768, 24, 16 },  // 0x06D0_talk_face_heart.png
    { 0x0730, 0x0006D5,  274,  768, 24, 16 },  // 0x0730_talk_face_music.png
    { 0x0790, 0x0007E7,  255,  768, 24, 16 },  // 0x0790_talk_face_smile.png
    { 0x07F0, 0x0008E6,  241,  768, 24, 16 },  // 0x07F0_talk_face_neutral.png
    { 0x0850, 0x0009D7,  235,  768, 24, 16 },  // 0x0850_talk_face_ellipsis.png
    { 0x08B0, 0x000AC2,  240,  768, 24, 16 },  // 0x08B0_talk_exclamation.png
    { 0x1090, 0x000BB2,  278,  512, 16, 16 },  // 0x1090_menu_icon_pokeradar.png
    { 0x10D0, 0x000CC8,  277,  512, 16, 16 },  // 0x10D0_menu_icon_dowsing.png
    { 0x1110, 0x000DDD,  195,  512, 16, 16 },  // 0x1110_menu_icon_connect.png
    { 0x1150, 0x000EA0,  183,  512, 16, 16 },  // 0x1150_menu_icon_trainer_card.png
    { 0x1190, 0x000F57,  250,  512, 16, 16 },  // 0x1190_menu_icon_inventory.png
    { 0x11D0, 0x001051,  234,  512, 16, 16 },  // 0x11D0_menu_icon_settings.png
    { 0x1210, 0x00113B,  262,  512, 16, 16 },  // 0x1210_person_icon.png
    { 0x1390, 0x001241,  200,  512, 16, 16 },  // 0x1390_route_small.png
    { 0x17D0, 0x001309,  255,  768, 24, 16 },  // 0x17D0_speaker_off.png
    { 0x1830, 0x001408,  305,  768, 24, 16 },  // 0x1830_speaker_low.png
    { 0x1890, 0x001539,  379,  768, 24, 16 },  // 0x1890_speaker_high.png
    { 0x18F0, 0x0016B4,  228,  256,  8, 16 },  // 0x18F0_contrast_demo.png
    { 0x1910, 0x001798,  563, 1536, 32, 24 },  // 0x1910_treasure_large.png
    { 0x19D0, 0x0019CB,  681, 1536, 32, 24 },  // 0x19D0_map_large.png
    { 0x1A90, 0x001C74,  726, 1536, 32, 24 },  // 0x1A90_present_large.png
    { 0x1B50, 0x001F4A,  147,  512, 16, 16 },  // 0x1B50_dowsing_bush_dark.png
    { 0x1B90, 0x001FDD,  139,  512, 16, 16 },  // 0x1B90_dowsing_bush_light.png
    { 0x1CB0, 0x002068,  577, 1536, 32, 24 },  // 0x1CB0_radar_bush.png
    { 0x1D70, 0x0022A9,  225,  512, 16, 16 },  // 0x1D70_radar_bubble_one.png
    { 0x1DB0, 0x00238A,  273,  512, 16, 16 },  // 0x1DB0_radar_bubble_two.png
    { 0x1DF0, 0x00249B,  285,  512, 16, 16 },  // 0x1DF0_radar_bubble_three.png
    { 0x1E30, 0x0025B8,  153,  512, 16, 16 },  // 0x1E30_radar_click.png
    { 0x1E70, 0x002651,  241, 1024, 16, 32 },  // 0x1E70_radar_attack_hit.png
    { 0x1EF0, 0x002742,  421, 1024, 16, 32 },  // 0x1EF0_radar_critical_hit.png
    { 0x1F70, 0x0028E7,  641, 1536, 32, 24 },  // 0x1F70_radar_appear_cloud.png
    { 0x2030, 0x002B68,   35,  128,  8,  8 },  // 0x2030_radar_hp_blip.png
    { 0x2040, 0x002B8B,   87,  128,  8,  8 },  // 0x2040_radar_catch_effect.png
    { 0x2350, 0x002BE2,  698, 2048, 32, 32 },  // 0x2350_pokewalker_big.png
    { 0x2450, 0x002E9C,   54,  256,  8, 16 },  // 0x2450_ir_arcs.png
    { 0x2470, 0x002ED2,   41,  128,  8,  8 },  // 0x2470_music_note.png
};


//uint8_t* find_icon_by_eeprom_address(uint16_t eeprom_address) 
const color_icons_t* find_icon_by_eeprom_address(uint16_t eeprom_address)
{
    int left = 0;
    int right = ICONS_COUNT - 1;

    while (left <= right) 
    {
        int mid = left + (right - left) / 2;
        uint16_t mid_addr = icons_map[mid].eeprom_address;

        if (mid_addr == eeprom_address) 
        {
            uint32_t offset = icons_map[mid].bin_offset;
            uint32_t size = icons_map[mid].size;

            if (offset + size > ICONS_BIN_SIZE) 
            {
                // printf("[COLOR_ICON_ERROR] Address 0x%04X: bounds check failed (offset=0x%06X + size=%u > BIN_SIZE=%u)\n", eeprom_address, offset, size, ICONS_BIN_SIZE);
                return NULL; // Out of bounds
            }
            // printf("[COLOR_ICON_FOUND] Address 0x%04X: offset=0x%06X, size=%u bytes, %ux%u pixels\n", eeprom_address, offset, size, icons_map[mid].width, icons_map[mid].height);
            // return color_icons + offset;
            return &icons_map[mid];
        } 
        else if (mid_addr < eeprom_address) left = mid + 1;
        else right = mid - 1;
    }

    // printf("[COLOR_ICON_MISS] Address 0x%04X: not found in lookup table\n", eeprom_address);
    return NULL;
}
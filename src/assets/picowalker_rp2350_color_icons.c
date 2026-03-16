#include <stddef.h>
#include <stdio.h>
#include "picowalker_rp2350_color_icons.h"

const color_icons_t icons_map[ICONS_COUNT] = {
    { 0x0000, 0x000000,  698, 2048, 32, 32 },  // 0x0000_large_pokewalker.png
    { 0x0460, 0x0002BA,   94,  128,  8,  8 },  // 0x0460_inventory_pokeball.png
    { 0x0470, 0x000318,   94,  128,  8,  8 },  // 0x0470_inventory_pokeball_event.png
    { 0x0488, 0x000376,   93,  128,  8,  8 },  // 0x0488_inventory_item.png
    { 0x0498, 0x0003D3,   93,  128,  8,  8 },  // 0x0498_inventory_item_event.png
    { 0x04A8, 0x000430,   96,  128,  8,  8 },  // 0x04A8_inventory_map.png
    { 0x04B8, 0x000490,   60,  128,  8,  8 },  // 0x04B8_card_heart.png
    { 0x04C8, 0x0004CC,   51,  128,  8,  8 },  // 0x04C8_card_spade.png
    { 0x04D8, 0x0004FF,   48,  128,  8,  8 },  // 0x04D8_card_diamond.png
    { 0x04E8, 0x00052F,   48,  128,  8,  8 },  // 0x04E8_card_club.png
    { 0x04F8, 0x00055F,   49,  128,  8,  8 },  // 0x04F8_system_arrow_up_normal.png
    { 0x0508, 0x000590,   49,  128,  8,  8 },  // 0x0508_system_arrow_up_offset.png
    { 0x0518, 0x0005C1,   49,  128,  8,  8 },  // 0x0518_system_arrow_up_invert.png
    { 0x0528, 0x0005F2,   49,  128,  8,  8 },  // 0x0528_system_arrow_down_normal.png
    { 0x0538, 0x000623,   49,  128,  8,  8 },  // 0x0538_system_arrow_down_invert.png
    { 0x0538, 0x000654,   49,  128,  8,  8 },  // 0x0538_system_arrow_down_offset.png
    { 0x0548, 0x000685,   49,  128,  8,  8 },  // 0x0548_system_arrow_down_invert.png
    { 0x0558, 0x0006B6,   98,  128,  8,  8 },  // 0x0558_system_arrow_left_normal.png
    { 0x0568, 0x000718,   96,  128,  8,  8 },  // 0x0568_system_arrow_left_offset.png
    { 0x0578, 0x000778,   98,  128,  8,  8 },  // 0x0578_system_arrow_left_invert.png
    { 0x0588, 0x0007DA,   98,  128,  8,  8 },  // 0x0588_system_arrow_right_normal.png
    { 0x0598, 0x00083C,   96,  128,  8,  8 },  // 0x0598_system_arrow_right_offset.png
    { 0x05A8, 0x00089C,   98,  128,  8,  8 },  // 0x05A8_system_arrow_right_invert.png
    { 0x05B8, 0x0008FE,   97,  256,  8, 16 },  // 0x05B8_system_arrow_left.png
    { 0x05D8, 0x00095F,   97,  256,  8, 16 },  // 0x05D8_system_arrow_right.png
    { 0x05F8, 0x0009C0,   92,  256,  8, 16 },  // 0x05F8_system_arrow_return.png
    { 0x0650, 0x000A1C,   96,  128,  8,  8 },  // 0x0650_inventory_gift.png
    { 0x0660, 0x000A7C,   89,  128,  8,  8 },  // 0x0660_system_low_battery.png
    { 0x0670, 0x000AD5,  240,  768, 24, 16 },  // 0x0670_talk_face_exclam.png
    { 0x06D0, 0x000BC5,  285,  768, 24, 16 },  // 0x06D0_talk_face_heart.png
    { 0x0730, 0x000CE2,  274,  768, 24, 16 },  // 0x0730_talk_face_music.png
    { 0x0790, 0x000DF4,  255,  768, 24, 16 },  // 0x0790_talk_face_smile.png
    { 0x07F0, 0x000EF3,  241,  768, 24, 16 },  // 0x07F0_talk_face_neutral.png
    { 0x0850, 0x000FE4,  235,  768, 24, 16 },  // 0x0850_talk_face_ellipsis.png
    { 0x08B0, 0x0010CF,  240,  768, 24, 16 },  // 0x08B0_talk_exclamation.png
    { 0x1090, 0x0011BF,  278,  512, 16, 16 },  // 0x1090_menu_icon_pokeradar.png
    { 0x10D0, 0x0012D5,  277,  512, 16, 16 },  // 0x10D0_menu_icon_dowsing.png
    { 0x1110, 0x0013EA,  195,  512, 16, 16 },  // 0x1110_menu_icon_connect.png
    { 0x1150, 0x0014AD,  183,  512, 16, 16 },  // 0x1150_menu_icon_trainer_card.png
    { 0x1190, 0x001564,  250,  512, 16, 16 },  // 0x1190_menu_icon_inventory.png
    { 0x11D0, 0x00165E,  234,  512, 16, 16 },  // 0x11D0_menu_icon_settings.png
    { 0x1210, 0x001748,  262,  512, 16, 16 },  // 0x1210_trainer_card_person.png
    { 0x1390, 0x00184E,  200,  512, 16, 16 },  // 0x1390_trainer_card_route.png
    { 0x17D0, 0x001916,  255,  768, 24, 16 },  // 0x17D0_system_speaker_off.png
    { 0x1830, 0x001A15,  305,  768, 24, 16 },  // 0x1830_system_speaker_low.png
    { 0x1890, 0x001B46,  379,  768, 24, 16 },  // 0x1890_system_speaker_high.png
    { 0x18F0, 0x001CC1,  228,  256,  8, 16 },  // 0x18F0_system_contrast_demo.png
    { 0x1910, 0x001DA5,  563, 1536, 32, 24 },  // 0x1910_large_treasure.png
    { 0x19D0, 0x001FD8,  681, 1536, 32, 24 },  // 0x19D0_large_map.png
    { 0x1A90, 0x002281,  726, 1536, 32, 24 },  // 0x1A90_large_present.png
    { 0x1B50, 0x002557,  147,  512, 16, 16 },  // 0x1B50_dowsing_bush_dark.png
    { 0x1B90, 0x0025EA,  139,  512, 16, 16 },  // 0x1B90_dowsing_bush_light.png
    { 0x1CB0, 0x002675,  577, 1536, 32, 24 },  // 0x1CB0_radar_bush.png
    { 0x1D70, 0x0028B6,  225,  512, 16, 16 },  // 0x1D70_radar_bubble_one.png
    { 0x1DB0, 0x002997,  273,  512, 16, 16 },  // 0x1DB0_radar_bubble_two.png
    { 0x1DF0, 0x002AA8,  285,  512, 16, 16 },  // 0x1DF0_radar_bubble_three.png
    { 0x1E30, 0x002BC5,  153,  512, 16, 16 },  // 0x1E30_radar_click.png
    { 0x1E70, 0x002C5E,  241, 1024, 16, 32 },  // 0x1E70_battle_attack_hit.png
    { 0x1EF0, 0x002D4F,  421, 1024, 16, 32 },  // 0x1EF0_battle_critical_hit.png
    { 0x1F70, 0x002EF4,  641, 1536, 32, 24 },  // 0x1F70_large_appear_cloud.png
    { 0x2030, 0x003175,   35,  128,  8,  8 },  // 0x2030_battle_hp_blip.png
    { 0x2040, 0x003198,   87,  128,  8,  8 },  // 0x2040_battle_catch_effect.png
    { 0x2350, 0x0031EF,  698, 2048, 32, 32 },  // 0x2350_large_pokewalker.png
    { 0x2450, 0x0034A9,   54,  256,  8, 16 },  // 0x2450_system_ir_arcs.png
    { 0x2470, 0x0034DF,   41,  128,  8,  8 },  // 0x2470_music_note.png
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

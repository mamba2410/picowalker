#include "pw_images.h"

pw_img_t text_mm_pokeradar  = {.height=16, .width=80, .data=&(eeprom[0x0910]), .size=320};
pw_img_t text_mm_dowsing    = {.height=16, .width=80, .data=&(eeprom[0x0a50]), .size=320};
pw_img_t text_mm_connect    = {.height=16, .width=80, .data=&(eeprom[0x0b90]), .size=320};
pw_img_t text_mm_trainer    = {.height=16, .width=80, .data=&(eeprom[0x0cd0]), .size=320};
pw_img_t text_mm_inventory  = {.height=16, .width=80, .data=&(eeprom[0x0e10]), .size=320};
pw_img_t text_mm_settings   = {.height=16, .width=80, .data=&(eeprom[0x0f50]), .size=320};

pw_img_t icon_mm_pokeradar  = {.height=16, .width=16, .data=&(eeprom[0x1090]), .size=0x40};
pw_img_t icon_mm_dowsing    = {.height=16, .width=16, .data=&(eeprom[0x10d0]), .size=0x40};
pw_img_t icon_mm_connect    = {.height=16, .width=16, .data=&(eeprom[0x1110]), .size=0x40};
pw_img_t icon_mm_trainer    = {.height=16, .width=16, .data=&(eeprom[0x1150]), .size=0x40};
pw_img_t icon_mm_inventory  = {.height=16, .width=16, .data=&(eeprom[0x1190]), .size=0x40};
pw_img_t icon_mm_settings   = {.height=16, .width=16, .data=&(eeprom[0x11d0]), .size=0x40};

pw_img_t text_characters[10] = {
    {.height=16, .width=8, .data=&(eeprom[0x0280]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x02a0]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x02c0]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x02e0]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x0300]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x0320]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x0340]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x0360]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x0380]), .size=0x20},
    {.height=16, .width=8, .data=&(eeprom[0x03a0]), .size=0x20},
};

pw_img_t text_character_colon = {.height=16, .width=8, .data=&(eeprom[0x03c0]), .size=0x20};
pw_img_t text_character_dash  = {.height=16, .width=8, .data=&(eeprom[0x03e0]), .size=0x20};
pw_img_t text_character_slash = {.height=16, .width=8, .data=&(eeprom[0x0400]), .size=0x20};

pw_img_t img_route = {.width=32, .height=24, .data=&(eeprom[0x8fbe]), .size=0xc0};

pw_img_t img_pokemon_small_frame1 = {.width=32, .height=24, .data=&(eeprom[0x91be]), .size=48};
pw_img_t img_pokemon_small_frame2 = {.width=32, .height=24, .data=&(eeprom[0x91ee]), .size=48};
pw_img_t img_pokemon_large_frame1 = {.width=64, .height=48, .data=&(eeprom[0x933e]), .size=0x300};
pw_img_t img_pokemon_large_frame2 = {.width=64, .height=48, .data=&(eeprom[0x973e]), .size=0x300};
pw_img_t text_pokemon_name = {.width=80, .height=16, .data=&(eeprom[0x993e]), .size=320};

pw_img_t icon_mm_larrow = {.width=8, .height=16, .data=&(eeprom[0x05b8]), .size=0x20};;
pw_img_t icon_mm_rarrow = {.width=8, .height=16, .data=&(eeprom[0x05d8]), .size=0x20};;
pw_img_t icon_mm_return = {.width=8, .height=16, .data=&(eeprom[0x05f8]), .size=0x20};;

pw_img_t icon_arrow_up_normal      = {.width=8, .height=8, .data=&(eeprom[0x04f8]), .size=16};
pw_img_t icon_arrow_up_offset      = {.width=8, .height=8, .data=&(eeprom[0x0508]), .size=16};
pw_img_t icon_arrow_up_inverted    = {.width=8, .height=8, .data=&(eeprom[0x0518]), .size=16};
pw_img_t icon_arrow_down_normal    = {.width=8, .height=8, .data=&(eeprom[0x0528]), .size=16};
pw_img_t icon_arrow_down_offset    = {.width=8, .height=8, .data=&(eeprom[0x0538]), .size=16};
pw_img_t icon_arrow_down_inverted  = {.width=8, .height=8, .data=&(eeprom[0x0548]), .size=16};
pw_img_t icon_arrow_left_normal    = {.width=8, .height=8, .data=&(eeprom[0x0558]), .size=16};
pw_img_t icon_arrow_left_offset    = {.width=8, .height=8, .data=&(eeprom[0x0568]), .size=16};
pw_img_t icon_arrow_left_inverted  = {.width=8, .height=8, .data=&(eeprom[0x0578]), .size=16};
pw_img_t icon_arrow_right_normal   = {.width=8, .height=8, .data=&(eeprom[0x0588]), .size=16};
pw_img_t icon_arrow_right_offset   = {.width=8, .height=8, .data=&(eeprom[0x0598]), .size=16};
pw_img_t icon_arrow_right_inverted = {.width=8, .height=8, .data=&(eeprom[0x05a8]), .size=16};

pw_img_t icon_watt_symbol = {.width=16, .height=16, .data=&(eeprom[0x0420]), .size=0x40};

pw_img_t icon_generic_person = {.width=16, .height=16, .data=&(eeprom[0x1210]), .size=0x40};
pw_img_t icon_small_route    = {.width=16, .height=16, .data=&(eeprom[0x1390]), .size=0x40};

pw_img_t text_trainer_name = {.width=80, .height=16, .data=&(eeprom[0x1250]), .size=0x320};
pw_img_t text_route_name   = {.width=80, .height=16, .data=&(eeprom[0x907e]), .size=0x320};

pw_img_t text_time  = {.width=32, .height=16, .data=&(eeprom[0x1470]), .size=0x80};
pw_img_t text_steps = {.width=40, .height=16, .data=&(eeprom[0x13d0]), .size=0xc0};
pw_img_t text_days  = {.width=40, .height=16, .data=&(eeprom[0x14f0]), .size=0xc0};
pw_img_t text_total_days  = {.width=64, .height=16, .data=&(eeprom[0x1590]), .size=0x100};



#include "pw_images.h"

pw_img_t text_mm_pokeradar = {.height=16, .width=80, .data=&(eeprom[0x0910]), .size=320};
pw_img_t text_mm_dowsing   = {.height=16, .width=80, .data=&(eeprom[0x0a50]), .size=320};
pw_img_t text_mm_connect   = {.height=16, .width=80, .data=&(eeprom[0x0b90]), .size=320};
pw_img_t text_mm_trainer   = {.height=16, .width=80, .data=&(eeprom[0x0cd0]), .size=320};
pw_img_t text_mm_inventory = {.height=16, .width=80, .data=&(eeprom[0x0e10]), .size=320};
pw_img_t text_mm_settings  = {.height=16, .width=80, .data=&(eeprom[0x0f50]), .size=320};



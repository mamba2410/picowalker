#include <stdint.h>
#include <stddef.h>

#include "pwroms.h"
#include "route_info.h"

route_info_t const *g_route_info = (route_info_t*)&(eeprom[0x8f00]);
special_route_info_t const *g_special_route_info = (special_route_info_t*)&(eeprom[0xbf00]);


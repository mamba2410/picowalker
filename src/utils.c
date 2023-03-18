#include <stdint.h>
#include <stddef.h>

#include "utils.h"
#include "states.h"
#include "types.h"
#include "eeprom_map.h"
#include "eeprom.h"


/*
 * Sets sv->reg{a,b,c}
 */
void pw_read_inventory(state_vars_t *sv) {
    sv->reg_a = 0;
    sv->reg_b = 0;
    sv->reg_c = 0;

    pokemon_summary_t caught_pokemon;
    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)(&caught_pokemon), sizeof(caught_pokemon));

    if(caught_pokemon.le_species != 0) {
        sv->reg_a |= HAVE_POKEMON;
    }

    struct {
        uint16_t le_item;
        uint16_t pad;
    } items[3];
    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        (uint8_t*)(items),
        PW_EEPROM_SIZE_OBTAINED_ITEMS
    );

    for(uint8_t i = 0; i < 3; i++) {
        if(items[i].le_item != 0) {
            sv->reg_b |= (1<<i);
        }
    }

    uint8_t special_inventory;
    pw_eeprom_read(
        PW_EEPROM_ADDR_RECEIVED_BITFIELD,
        &(sv->reg_c),
        1
    );
}

int nintendo_to_ascii(uint8_t *str, char* buf, size_t len) {
    uint16_t c;
    uint16_t nintendo;
    int ret = 0;
    for(size_t i = 0; i < len; i++) {
        nintendo = str[2*i] | ((uint16_t)str[2*i+1])<<8;
        switch(nintendo) {
            case 0x0000: c=0x0000; break;
            case 0x0112: c=0x00A5; break;
            case 0x0121: c=0x0030; break;
            case 0x0122: c=0x0031; break;
            case 0x0123: c=0x0032; break;
            case 0x0124: c=0x0033; break;
            case 0x0125: c=0x0034; break;
            case 0x0126: c=0x0035; break;
            case 0x0127: c=0x0036; break;
            case 0x0128: c=0x0037; break;
            case 0x0129: c=0x0038; break;
            case 0x012A: c=0x0039; break;
            case 0x012B: c=0x0041; break;
            case 0x012C: c=0x0042; break;
            case 0x012D: c=0x0043; break;
            case 0x012E: c=0x0044; break;
            case 0x012F: c=0x0045; break;
            case 0x0130: c=0x0046; break;
            case 0x0131: c=0x0047; break;
            case 0x0132: c=0x0048; break;
            case 0x0133: c=0x0049; break;
            case 0x0134: c=0x004A; break;
            case 0x0135: c=0x004B; break;
            case 0x0136: c=0x004C; break;
            case 0x0137: c=0x004D; break;
            case 0x0138: c=0x004E; break;
            case 0x0139: c=0x004F; break;
            case 0x013A: c=0x0050; break;
            case 0x013B: c=0x0051; break;
            case 0x013C: c=0x0052; break;
            case 0x013D: c=0x0053; break;
            case 0x013E: c=0x0054; break;
            case 0x013F: c=0x0055; break;
            case 0x0140: c=0x0056; break;
            case 0x0141: c=0x0057; break;
            case 0x0142: c=0x0058; break;
            case 0x0143: c=0x0059; break;
            case 0x0144: c=0x005A; break;
            case 0x0145: c=0x0061; break;
            case 0x0146: c=0x0062; break;
            case 0x0147: c=0x0063; break;
            case 0x0148: c=0x0064; break;
            case 0x0149: c=0x0065; break;
            case 0x014A: c=0x0066; break;
            case 0x014B: c=0x0067; break;
            case 0x014C: c=0x0068; break;
            case 0x014D: c=0x0069; break;
            case 0x014E: c=0x006A; break;
            case 0x014F: c=0x006B; break;
            case 0x0150: c=0x006C; break;
            case 0x0151: c=0x006D; break;
            case 0x0152: c=0x006E; break;
            case 0x0153: c=0x006F; break;
            case 0x0154: c=0x0070; break;
            case 0x0155: c=0x0071; break;
            case 0x0156: c=0x0072; break;
            case 0x0157: c=0x0073; break;
            case 0x0158: c=0x0074; break;
            case 0x0159: c=0x0075; break;
            case 0x015A: c=0x0076; break;
            case 0x015B: c=0x0077; break;
            case 0x015C: c=0x0078; break;
            case 0x015D: c=0x0079; break;
            case 0x015E: c=0x007A; break;
            case 0x015F: c=0x00C0; break;
            case 0x0160: c=0x00C1; break;
            case 0x0161: c=0x00C2; break;
            case 0x0162: c=0x00C3; break;
            case 0x0163: c=0x00C4; break;
            case 0x0164: c=0x00C5; break;
            case 0x0165: c=0x00C6; break;
            case 0x0166: c=0x00C7; break;
            case 0x0167: c=0x00C8; break;
            case 0x0168: c=0x00C9; break;
            case 0x0169: c=0x00CA; break;
            case 0x016A: c=0x00CB; break;
            case 0x016B: c=0x00CC; break;
            case 0x016C: c=0x00CD; break;
            case 0x016D: c=0x00CE; break;
            case 0x016E: c=0x00CF; break;
            case 0x016F: c=0x00D0; break;
            case 0x0170: c=0x00D1; break;
            case 0x0171: c=0x00D2; break;
            case 0x0172: c=0x00D3; break;
            case 0x0173: c=0x00D4; break;
            case 0x0174: c=0x00D5; break;
            case 0x0175: c=0x00D6; break;
            case 0x0176: c=0x00D7; break;
            case 0x0177: c=0x00D8; break;
            case 0x0178: c=0x00D9; break;
            case 0x0179: c=0x00DA; break;
            case 0x017A: c=0x00DB; break;
            case 0x017B: c=0x00DC; break;
            case 0x017C: c=0x00DD; break;
            case 0x017D: c=0x00DE; break;
            case 0x017E: c=0x00DF; break;
            case 0x017F: c=0x00E0; break;
            case 0x0180: c=0x00E1; break;
            case 0x0181: c=0x00E2; break;
            case 0x0182: c=0x00E3; break;
            case 0x0183: c=0x00E4; break;
            case 0x0184: c=0x00E5; break;
            case 0x0185: c=0x00E6; break;
            case 0x0186: c=0x00E7; break;
            case 0x0187: c=0x00E8; break;
            case 0x0188: c=0x00E9; break;
            case 0x0189: c=0x00EA; break;
            case 0x018A: c=0x00EB; break;
            case 0x018B: c=0x00EC; break;
            case 0x018C: c=0x00ED; break;
            case 0x018D: c=0x00EE; break;
            case 0x018E: c=0x00EF; break;
            case 0x018F: c=0x00F0; break;
            case 0x0190: c=0x00F1; break;
            case 0x0191: c=0x00F2; break;
            case 0x0192: c=0x00F3; break;
            case 0x0193: c=0x00F4; break;
            case 0x0194: c=0x00F5; break;
            case 0x0195: c=0x00F6; break;
            case 0x0196: c=0x00F7; break;
            case 0x0197: c=0x00F8; break;
            case 0x0198: c=0x00F9; break;
            case 0x0199: c=0x00FA; break;
            case 0x019A: c=0x00FB; break;
            case 0x019B: c=0x00FC; break;
            case 0x019C: c=0x00FD; break;
            case 0x019D: c=0x00FE; break;
            case 0x019E: c=0x00FF; break;
            case 0x01A3: c=0x00AA; break;
            case 0x01A4: c=0x00BA; break;
            case 0x01A5: c=0x00B9; break;
            case 0x01A6: c=0x00B2; break;
            case 0x01A7: c=0x00B3; break;
            case 0x01A8: c=0x0024; break;
            case 0x01A9: c=0x00A1; break;
            case 0x01AA: c=0x00BF; break;
            case 0x01AB: c=0x0021; break;
            case 0x01AC: c=0x003F; break;
            case 0x01AD: c=0x002C; break;
            case 0x01AE: c=0x002E; break;
            case 0x01B0: c=0x00B7; break;
            case 0x01B1: c=0x002F; break;
            case 0x01B9: c=0x0028; break;
            case 0x01BA: c=0x0029; break;
            case 0x01BD: c=0x002B; break;
            case 0x01BE: c=0x002D; break;
            case 0x01BF: c=0x002A; break;
            case 0x01C0: c=0x0023; break;
            case 0x01C1: c=0x003D; break;
            case 0x01C2: c=0x0026; break;
            case 0x01C3: c=0x007E; break;
            case 0x01C4: c=0x003A; break;
            case 0x01C5: c=0x003B; break;
            case 0x01D0: c=0x0040; break;
            case 0x01D2: c=0x0025; break;
            case 0x01DE: c=0x0020; break;
            case 0x01E8: c=0x00B0; break;
            case 0x01E9: c=0x005F; break;
            case 0xE000: c=0x000A; break;
            case 0x25BC: c=0x000D; break;
            case 0x25BD: c=0x000C; break;
            case 0xffff: c = 0; break;
            default: ret = -1;
        } // switch

        buf[i] = (char)c;
    } // for

    return ret;
}


#include <stddef.h>
#include <stdint.h>

#include "trainer_info.h"
#include "route_info.h"
#include "pwroms.h"

reliable_data_t const *g_reliable_data_1 = 0;
reliable_data_t const *g_reliable_data_2 = 0;



void pw_check_struct_sizes() {
    size_t reliable_data_size = sizeof(reliable_data_t), reliable_data_expected = 0x0171-0x0080+1;
    size_t walker_info_size = sizeof(walker_info_t), walker_info_expected = 0x0155-0x00ed+1;
    size_t health_data_size = sizeof(health_data_t), health_data_expected = 0x016e -0x0156+1;
    size_t route_info_size = sizeof(route_info_t), route_info_expected = 0x8fbd-0x8f00+1;
    size_t pokemon_summary_size = sizeof(pokemon_summary_t), pokemon_summary_expected = 0xba53-0xba44+1;
    size_t special_pokemon_info_size = sizeof(special_pokemon_info_t), special_pokemon_info_expected = 0xba7f-0xba54+1;
    size_t special_route_info_size = sizeof(special_route_info_t), special_route_info_expected = 0xcbbb-0xbf00+1;

    printf("rd: size 0x%x, expected 0x%x\n", reliable_data_size,reliable_data_expected);
    printf("wi: size 0x%x, expected 0x%x\n", walker_info_size,walker_info_expected);
    printf("hd: size 0x%x, expected 0x%x\n", health_data_size,health_data_expected);
    printf("ri: size 0x%x, expected 0x%x\n", route_info_size,route_info_expected);
    printf("ps: size 0x%x, expected 0x%x\n", pokemon_summary_size,pokemon_summary_expected);
    printf("si: size 0x%x, expected 0x%x\n", special_pokemon_info_size,special_pokemon_info_expected);
    printf("sr: size 0x%x, expected 0x%x\n", special_route_info_size,special_route_info_expected);

    printf("%p\n", 0x80 + (void*)&(g_reliable_data_1->unique_data) - (void*)g_reliable_data_1);
    printf("%p\n", 0x80 + (void*)&(g_reliable_data_1->lcd_config) - (void*)g_reliable_data_1);
    printf("%p\n", 0x80 + (void*)&(g_reliable_data_1->walker_info) - (void*)g_reliable_data_1);
    printf("tid: %d\n", g_reliable_data_1->walker_info.le_tid);

    char chbuf[11];
    nintendo_to_ascii((uint8_t*)&((g_reliable_data_1->walker_info).le_trainer_name), chbuf, 8);
    printf("Trainer name: %s\n", chbuf);



}



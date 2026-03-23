#include <stddef.h>
#include <stdio.h>
#include "picowalker_rp2350_color_routes.h"

const color_routes_t routes_map[ROUTES_COUNT] = {
    { 0, 0x000000, 281, 1536, 32, 24 },  // 0x8FBE_route_0_refreshing_field.png
    { 1, 0x000119, 434, 1536, 32, 24 },  // 0x8FBE_route_1_noisy_forest.png
    { 2, 0x0002CB, 347, 1536, 32, 24 },  // 0x8FBE_route_2_surburban_area.png
    { 3, 0x000426, 407, 1536, 32, 24 },  // 0x8FBE_route_3_town_outskirts.png
    { 4, 0x0005BD, 202, 1536, 32, 24 },  // 0x8FBE_route_4_rugged_road.png
    { 5, 0x000687, 145, 1536, 32, 24 },  // 0x8FBE_route_5_dim_cave.png
    { 6, 0x000718, 412, 1536, 32, 24 },  // 0x8FBE_route_6_blue_lake.png
    { 7, 0x0008B4, 371, 1536, 32, 24 },  // 0x8FBE_route_7_beautiful_beach.png
};


//uint8_t* find_route_by_index(uint8_t route_index) 
const color_routes_t* find_route_by_index(uint8_t route_index)
{
    if (route_index >= ROUTES_COUNT) 
    {
        // printf("[COLOR_ROUTE_MISS] Invalid Index %u\n", route_index);
        return NULL;
    }

    uint32_t offset = routes_map[route_index].bin_offset;
    uint32_t size = routes_map[route_index].size;

    if (offset + size > ROUTES_BIN_SIZE) 
    {
        // printf("[COLOR_ROUTE_ERROR] bounds check failed (offset=0x%06X + size=%u > BIN_SIZE=%u)\n", offset, size, ROUTES_BIN_SIZE);
        return NULL; // Out of bounds
    }

    // printf("[COLOR_ROUTE_FOUND] index=%u\n", route_index);
    // return color_routes + offset;
    return &routes_map[route_index];
}

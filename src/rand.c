#include <stdint.h>

#include "rand.h"

static uint32_t g_seed = 0;

void pw_srand(uint32_t seed) {
    g_seed = seed;
}

/*
 *  Simple xorshift
 *  https://en.wikipedia.org/wiki/Xorshift
 *  Doesn't need to be anything fancy
 */
uint32_t pw_rand_r(uint32_t *seed) {

    *seed ^= (*seed) << 13;
    *seed ^= (*seed) >> 17;
    *seed ^= (*seed) << 5;

    return *seed;
}

uint32_t pw_rand() {
    return pw_rand_r(&g_seed);
}


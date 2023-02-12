#ifndef PW_RAND_H
#define PW_RAND_H

#include <stdint.h>

void pw_srand(uint32_t seed);
uint32_t pw_rand_r(uint32_t *seed);
uint32_t pw_rand();

#endif /* PW_RAND_H */

#include "random.h"

void random_seed(random_state_t *r, u32 seed) {
    r->z = ~seed;
    r->w = seed;
}	

/*
 * MWC generator concatenates two 16-bit multiply-
 * with-carry generators, x(n)=36969x(n-1)+carry,
 * y(n)=18000y(n-1)+carry mod 2^16, has period about 2^60
 */
u32 random_next(random_state_t *r) {
    r->z = 36969 * (r->z & 65535) + (r->z >> 16);
    r->w = 18000 * (r->w & 65535) + (r->w >> 16);
    return ((r->z << 16) + r->w) & 0x7FFFFFFF;
}


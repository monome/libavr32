#include "random.h"

void random_init(random_state_t *r, u32 seed, s16 min, s16 max) {
	r->min = min;
	r->max = max;

	random_seed(r, seed);
}

void random_seed(random_state_t *r, u32 seed) {
    r->z = 12345;
    r->w = seed;
}	

/*
 * MWC generator concatenates two 16-bit multiply-
 * with-carry generators, x(n)=36969x(n-1)+carry,
 * y(n)=18000y(n-1)+carry mod 2^16, has period about 2^60
 */
s16 random_next(random_state_t *r) {
    r->z = 36969 * (r->z & 65535) + (r->z >> 16);
    r->w = 18000 * (r->w & 65535) + (r->w >> 16);
    u32 val = ((r->z << 16) + r->w) & 0x7FFFFFFF;

	val = (val % ((r->max - r->min) + 1)) + r->min;
	return val;
}


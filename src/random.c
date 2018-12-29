#include "random.h"

// cribbed from aleph/bees op_random

void random_init(random_state_t *r, u32 seed, s16 min, s16 max) {
	r->min = min;
	r->max = max;

	random_seed(r, seed);
}

void random_seed(random_state_t *r, u32 seed) {
	r->val = 0;
	
	r->a = 0x19660d;
	r->c = 0x3c6ef35f;
	r->x = seed;
}	
	
s16 random_next(random_state_t *r) {
	r->x = r->x * r->a + r->c;
  r->val = r->x;
  if (r->val < 0) {
    r->val *= -1;
	}
  r->val = (r->val % ((r->max - r->min) + 1)) + r->min;
	return r->val;
}


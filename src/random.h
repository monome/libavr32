#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "types.h"

//------------------------------
//----- types

typedef struct {
	s16 min, max;
	u32 z, w;
} random_state_t;

//------------------------------
//----- functions

// initialize random generator
extern void random_init(random_state_t *r, u32 seed, s16 min, s16 max);
// (re)seed generator, restarting sequence
extern void random_seed(random_state_t *r, u32 seed);
extern s16 random_next(random_state_t *r);

#endif // header guard

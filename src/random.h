#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "types.h"

//------------------------------
//----- types

typedef struct {
	u32 z, w;
} random_state_t;

//------------------------------
//----- functions

// (re)seed generator, restarting sequence
extern void random_seed(random_state_t *r, u32 seed);
extern u32 random_next(random_state_t *r);

#endif // header guard

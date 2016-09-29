#ifndef __NOTES__
#define __NOTES__

#include <stdio.h>

#include "types.h"

// maintains a fixed size pool of held notes
//
// the pool is structured as a linked list ordered from most recent to least
// recent notes.

#define NOTE_POOL_SIZE 12

typedef enum {
  kNotePriorityHigh,
  kNotePriorityLow,
  kNotePriorityLast  
} note_priority;

typedef struct {
  u8   num;
  u8   vel;
} held_note_t;

struct pool_element {
  held_note_t note;
  u8 is_free;
  struct pool_element* next;
};
typedef struct pool_element pool_element_t;

typedef struct {
	pool_element_t  elements[NOTE_POOL_SIZE];
	u8              count;
	pool_element_t* head;
	pool_element_t* last_free;
} note_pool_t;


void notes_init(note_pool_t *pool);
void notes_hold(note_pool_t *pool, u8 num, u8 vel);
void notes_release(note_pool_t *pool, u8 num);
const held_note_t *notes_get(note_pool_t *pool, note_priority p);

#endif // __NOTES__

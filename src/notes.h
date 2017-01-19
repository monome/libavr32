#ifndef __NOTES__
#define __NOTES__

#include <stdio.h>

#include "types.h"
#include "compiler.h"

// maintains a fixed size pool of held notes
//
// the pool is structured as a linked list ordered from most recent to least
// recent notes.

#ifndef NOTE_POOL_SIZE
#define NOTE_POOL_SIZE 16
#endif

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
u8 notes_count(note_pool_t *pool);

typedef struct {
	note_pool_t *pool;
	pool_element_t *e;
} note_pool_iter_t;

void notes_iter_init(note_pool_iter_t *i, note_pool_t *p);
const held_note_t *notes_iter_next(note_pool_iter_t *i);

#endif // __NOTES__

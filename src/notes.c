#include "notes.h"

#ifndef __TDD__
#include "print_funcs.h"
#endif

// maintains a fixed size pool of held notes
//
// the pool is structured as a linked list ordered from most recent to least
// recent notes.

#define POOL_SIZE 24

struct pool_element {
  held_note_t note;
  u8 is_free;
  struct pool_element* next;
};
typedef struct pool_element pool_element_t;


struct pool {
	pool_element_t  elements[POOL_SIZE];
	u8              count;
	pool_element_t* head;
	pool_element_t* last_free;
};
typedef struct pool pool_t;

//
// private
//

static pool_t _pool;
static pool_t* notes = &_pool;


static pool_element_t *pool_tail(pool_t *p) {
  pool_element_t *element = p->head;
  while (element) element = element->next;
  return element;
}

static inline pool_element_t *pool_head(pool_t *p) {
  return p->head;
}

static inline void pool_push(pool_t *p, pool_element_t *head) {
  head->next = p->head;
  p->head = head;
}

inline static u8 pool_remaining(pool_t *p) {
  return POOL_SIZE - p->count;
}

inline static u8 pool_count(pool_t *p) {
  return p->count;
}

static pool_element_t *pool_take(pool_t *p) {
  pool_element_t *allocated = NULL;
  
  if (pool_remaining(p) == 0) {
    // fail fast if full
    return NULL;
  }
  if (p->last_free) {
    // quick
    allocated = p->last_free;
    p->last_free = NULL;
  }
  else {
    // search
    for (u8 i = 0; i < POOL_SIZE; i++) {
      if (p->elements[i].is_free) {
        allocated = &(p->elements[i]);
        break;
      }
    }
  }
  if (allocated) {
    // record
    allocated->is_free = 0;
    allocated->next = NULL;
    p->count++;
    pool_push(p, allocated); // chain and set as head
  }
  
  return allocated;
}

static void pool_return(pool_t *p, pool_element_t *element) {
  pool_element_t *before;

  if (element) {
    if (element->next) {
      // middle of chain, pop out of chain
      before = pool_head(p);
      while (before && before->next != element)
        before = before->next;
      before->next = element->next;
    }

    element->is_free = 1;
    element->next = NULL;
    element->note.num = 0;
    element->note.vel = 0;

    p->last_free = element;
    p->count--;
  }
}

static void pool_init(pool_t *p) {
	pool_element_t *e = &(p->elements[0]);

  for (u8 i = 0; i < POOL_SIZE; i++) {
    e->note.num = 0;
    e->note.vel = 0;
    e->is_free = 1;
    e->next = NULL;
		e++;
  }

	p->count = 0;
  p->head = NULL;
  p->last_free = NULL;
}

//
// public
//
void notes_init(void) {
  pool_init(&_pool);
}

void notes_hold(u8 num, u8 vel) {
  pool_element_t *head;
  
  // release note if already held
  notes_release(num);

  // if at capacity, free the least recently held note
  if (pool_remaining(notes) == 0) {
    pool_return(notes, pool_tail(notes));
  }

  head = pool_take(notes); // shouldn't be NULL given the above
  head->note.num = num;
  head->note.vel = vel;
}

void notes_release(u8 num) {
  // FIXME: uses pool internals :|
  pool_element_t *element = pool_head(notes);
  pool_element_t *before = NULL;
  
  // find matching note
  while (element && element->note.num != num) {
    before = element;
    element = element->next;
  }
  
  if (element) {
    // found a match
    if (before) {
      // in the middle
      before->next = element->next;
    }
    else {
      // element is head
      notes->head = element->next; // FIXME: we are breaking the pool abstraction here
      element->next = NULL; // FIXME: to prevent pool_return from messing up head?? nope
    }
    pool_return(notes, element);
  }
}

const held_note_t *notes_get(note_priority p) {
  pool_element_t *element = pool_head(notes);
  if (element) {
    // at least one held note...
    switch (p) {
    case kNotePriorityLast:
      return &(element->note);
    default:
      // FIME: others not implemented
      return NULL;
    }
  }
  // nothing held
  return NULL;
}

#include "notes.h"

#ifndef TEST
#include "print_funcs.h"
#endif

//
// private
//

static pool_element_t *pool_tail(note_pool_t *p) {
  pool_element_t *element = p->head;
  while (element) element = element->next;
  return element;
}

static inline pool_element_t *pool_head(note_pool_t *p) {
  return p->head;
}

static inline void pool_push(note_pool_t *p, pool_element_t *head) {
  head->next = p->head;
  p->head = head;
}

inline static u8 pool_remaining(note_pool_t *p) {
  return NOTE_POOL_SIZE - p->count;
}

inline static u8 pool_count(note_pool_t *p) {
  return p->count;
}

static pool_element_t *pool_take(note_pool_t *p) {
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
    for (u8 i = 0; i < NOTE_POOL_SIZE; i++) {
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

static void pool_return(note_pool_t *p, pool_element_t *element) {
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

static void pool_init(note_pool_t *p) {
	pool_element_t *e = &(p->elements[0]);

  for (u8 i = 0; i < NOTE_POOL_SIZE; i++) {
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
void notes_init(note_pool_t *pool) {
  pool_init(pool);
}

void notes_hold(note_pool_t *pool, u8 num, u8 vel) {
  pool_element_t *head;
  
  // release note if already held
  notes_release(pool, num);

  // if at capacity, free the least recently held note
  if (pool_remaining(pool) == 0) {
    pool_return(pool, pool_tail(pool));
  }

  head = pool_take(pool); // shouldn't be NULL given the above
  head->note.num = num;
  head->note.vel = vel;
}

void notes_release(note_pool_t *pool, u8 num) {
  // FIXME: uses pool internals :|
  pool_element_t *element = pool_head(pool);
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
      pool->head = element->next; // FIXME: we are breaking the pool abstraction here
      element->next = NULL; // FIXME: to prevent pool_return from messing up head?? nope
    }
    pool_return(pool, element);
  }
}

const held_note_t *notes_get(note_pool_t *pool, note_priority p) {
  pool_element_t *element = pool_head(pool);
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

u8 notes_count(note_pool_t *pool) {
	return pool->count;
}

void notes_iter_init(note_pool_iter_t *i, note_pool_t *p) {
	i->pool = p;
	i->e = p ? p->head : NULL;
}

const held_note_t *notes_iter_next(note_pool_iter_t *i) {
	held_note_t *n = NULL;

	if (i->e) {
		n = &(i->e->note);
		i->e = i->e->next;
	}

	return n;
}

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

//
// private
//

static pool_element_t  _pool[POOL_SIZE];
static u8              _pool_count; 
static pool_element_t* _pool_head;
static pool_element_t* _pool_last_free;

static pool_element_t *pool_tail(void) {
  pool_element_t *element = _pool_head;
  while (element) element = element->next;
  //print_dbg("\r\npool_tail() = ");
  //print_dbg_hex(element);
  return element;
}

static inline pool_element_t *pool_head(void) {
  //print_dbg("\r\npool_head() = ");
  //print_dbg_hex(_pool_head);
  return _pool_head;
}

static inline void pool_push(pool_element_t *head) {
  head->next = _pool_head;
  _pool_head = head;
}

inline static u8 pool_remaining(void) {
  return POOL_SIZE - _pool_count;
}

inline static u8 pool_count(void) {
  return _pool_count;
}

static pool_element_t *pool_take(void) {
  pool_element_t *allocated = NULL;
  
  // print_dbg("\r\npool_take()... ");
  
  if (pool_remaining() == 0) {
    // fail fast if full
    return NULL;
  }
  if (_pool_last_free) {
    // quick
    allocated = _pool_last_free;
    _pool_last_free = NULL;
  }
  else {
    // search
    for (u8 i = 0; i < POOL_SIZE; i++) {
      if (_pool[i].is_free) {
        allocated = &(_pool[i]);
        break;
      }
    }
  }
  if (allocated) {
    // record
    allocated->is_free = 0;
    allocated->next = NULL;
    _pool_count++;
    pool_push(allocated); // chain and set as head
  }
  
  // print_dbg_hex(allocated);
  
  return allocated;
}

static void pool_return(pool_element_t *element) {
  pool_element_t *before;
  // print_dbg("\r\n pool_return()... ");
  if (element) {
    if (element->next) {
      // middle of chain, pop out of chain
      before = pool_head();
      while (before && before->next != element)
        before = before->next;
      before->next = element->next;
    }
    // print_dbg(" num: ");
    // print_dbg_ulong(element->note.num);
    element->is_free = 1;
    element->next = NULL;
    element->note.num = 0;
    element->note.vel = 0;
    _pool_last_free = element;
    _pool_count--;
    // print_dbg(" new count: ");
    // print_dbg_ulong(_pool_count);
  }
}

static void pool_init(void) {
  // print_dbg("\r\npool_init...");
  for (u8 i = 0; i < POOL_SIZE; i++) {
    _pool[i].note.num = 0;
    _pool[i].note.vel = 0;
    _pool[i].is_free = 1;
    _pool[i].next = NULL;
  }
  _pool_count = 0;
  _pool_head = NULL;
  _pool_last_free = NULL;
  // print_dbg(" done.");  
}

//
// public
//
void notes_init(void) {
  pool_init();
}

void notes_hold(u8 num, u8 vel) {
  pool_element_t *head;
  
  // release note if already held
  notes_release(num);

  // if at capacity, free the least recently held note
  if (pool_remaining() == 0) {
    pool_return(pool_tail());
  }

  head = pool_take(); // shouldn't be NULL given the above
  head->note.num = num;
  head->note.vel = vel;
}

void notes_release(u8 num) {
  // FIXME: uses pool internals :|
  pool_element_t *element = pool_head();
  pool_element_t *before = NULL;
  
  // print_dbg("\r\nnotes_release()...");
  
  // find matching note
  while (element && element->note.num != num) {
    before = element;
    element = element->next;
  }
  
  if (element) {
    // print_dbg(" match");
    // found a match
    if (before) {
      // in the middle
      before->next = element->next;
      // print_dbg(" middle.");
    }
    else {
      // element is head
      _pool_head = element->next;
      element->next = NULL; // FIXME: to prevent pool_return from messing up head?? nope
      // print_dbg(" head.");
    }
    pool_return(element);
  }
}

const held_note_t *notes_get(note_priority p) {
  pool_element_t *element = pool_head();
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

#ifndef __NOTES__
#define __NOTES__

#include <stdio.h>

#include "types.h"

typedef enum {
  kNotePriorityHigh,
  kNotePriorityLow,
  kNotePriorityLast  
} note_priority;

// private
typedef struct {
  u8   num;
  u8   vel;
} held_note_t;

void notes_init(void);
void notes_hold(u8 num, u8 vel);
void notes_release(u8 num);
const held_note_t *notes_get(note_priority p);

#endif // __NOTES__

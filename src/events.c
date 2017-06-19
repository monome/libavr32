/* events.c
 * aleph
 *
 * simple event queue
 * disables interrupts around queue manipulation
 */

// ASF
#include "compiler.h"
#include "print_funcs.h"

#include "interrupts.h"
#include "events.h"

 static void handler_Ignore(s32 data) { }

// global array of pointers to handlers
// initialise all it's entries to handler_Ignore
 void (*app_event_handlers[kNumEventTypes])(s32 data) = { handler_Ignore };


/// NOTE: if we are ever over-filling the event queue, we have problems.
/// making the event queue bigger not likely to solve the problems.
#define MAX_EVENTS   40

// macro for incrementing an index into a circular buffer.
#define INCR_EVENT_INDEX( x )  { if ( ++x == MAX_EVENTS ) x = 0; }

// et/Put indexes inxto sysEvents[] array
volatile static int putIdx = 0;
volatile  static int getIdx = 0;

// The system event queue is a circular array of event records.
volatile  static event_t sysEvents[ MAX_EVENTS ];

// initializes (or re-initializes)  the system event queue.
 void init_events( void ) {
  int k;
  
  // set queue (circular list) to empty
  putIdx = 0;
  getIdx = 0;

  // zero out the event records
  for ( k = 0; k < MAX_EVENTS; k++ ) {
    sysEvents[ k ].type = 0;
    sysEvents[ k ].data = 0;
  }
}

// get next event
// Returns non-zero if an event was available
u8 event_next( event_t *e ) {
  u8 status;

  u8 flags = irqs_pause();

  // if pointers are equal, the queue is empty... don't allow idx's to wrap!
  if ( getIdx != putIdx ) {
    INCR_EVENT_INDEX( getIdx );
    e->type = sysEvents[ getIdx ].type;
    e->data = sysEvents[ getIdx ].data;
    status = true;
  } else {
    e->type  = 0xff;
    e->data = 0;
    status = false;
  }

  irqs_resume(flags);

  return status;
}


// add event to queue, return success status
u8 event_post( event_t *e ) {
   // print_dbg("\r\n posting event, type: ");
   // print_dbg_ulong(e->type);

  u8 flags = irqs_pause();
  u8 status = false;

  // increment write idx, posbily wrapping
  int saveIndex = putIdx;
  INCR_EVENT_INDEX( putIdx );
  if ( putIdx != getIdx  ) {
    sysEvents[ putIdx ].type = e->type;
    sysEvents[ putIdx ].data = e->data;
    status = true;
  } else {
    // idx wrapped, so queue is full, restore idx
    putIdx = saveIndex;
  } 

  irqs_resume(flags);

  //if (!status)
  //  print_dbg("\r\n event queue full!");

  return status;
}

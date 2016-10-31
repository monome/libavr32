#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "types.h"

// enumerate event types
typedef enum {
  kEventNone,
  kEventFront,
  kEventFrontShort,
  kEventFrontLong,
  kEventTimer,
  kEventPollADC,
  kEventKeyTimer,
  kEventSaveFlash,
  kEventClockNormal,
  kEventClockExt,
  /// connect/disconnect FTDI-based devices
  kEventFtdiConnect,
  kEventFtdiDisconnect,
  /// monome protocol
  kEventMonomeConnect,
  kEventMonomeDisconnect,
  kEventMonomePoll,	
  kEventMonomeRefresh, 	
  kEventMonomeGridKey, 
  kEventMonomeRingEnc,
  // HID
  kEventHidConnect,
  kEventHidDisconnect,
  kEventHidPacket,
  kEventHidTimer,
  
  kEventScreenRefresh,
  // Trigger EVENT (8 digital inputs)
  kEventTrigger,
  kEventII,
  kEventIItx,
  kEventIIrx,

  kEventMidiConnect,
  kEventMidiDisconnect,
  kEventMidiPacket,
  kEventMidiRefresh,

  kEventTr,
  kEventTrNormal,
  kEventKey,
  
  // aleph-specific
  kEventAdc0 , 	// receive values from polled ADC channels
  kEventAdc1 ,
  kEventAdc2 ,
  kEventAdc3 ,
  /// encoders
  kEventEncoder0,
  kEventEncoder1,
  kEventEncoder2 ,
  kEventEncoder3 ,
  //// switches
  kEventSwitch0, // fn1
  kEventSwitch1, // fn2
  kEventSwitch2, // fn3
  kEventSwitch3, // fn4
  kEventSwitch4, // mode
  kEventSwitch5, // power
  kEventSwitch6, // foot1
  kEventSwitch7, // foot2
  // serial rx
  kEventSerial,
  // arbitrary loopback from within application
  kEventAppCustom,
  /// dummy/count
  kNumEventTypes,
} etype;

typedef struct {
  etype type;
  s32 data;
} event_t;

// global array of pointers to handlers
extern void (*app_event_handlers[])(s32 data);

// init event queue
void init_events( void );

// check the queue for pending events
// return 1 if found
u8 event_next( event_t *e );

// add event to tail of queue
// return 1 if success
u8 event_post( event_t *e );

#endif // header guard

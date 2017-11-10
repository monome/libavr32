/*
  midi.c
  libavr32

  usb MIDI functions.
*/

// asf
#include "print_funcs.h"

// libavr32
#include "conf_usb_host.h"
#include "events.h"
#include "events.h"
#include "midi.h"
#include "uhi_midi.h"


//------------------------------------
//------ defines

// RX buffer size.
// the buffer will start filling up if events come in faster than the polling rate
// the more full the buffer, the longer we'll spend in the usb read ISR parsing it...
// so it is a tradeoff.
#define MIDI_RX_EVENT_BUF_SIZE 16
#define MIDI_TX_EVENT_BUF_SIZE 16


//------------------------------
//----- types

// usb midi event type
// See http://www.usb.org/developers/docs/devclass_docs/midi10.pdf
// Section 4 (page 16)
typedef union {
  struct { u8 header; u8 msg[3]; };
  u32 raw;
} usb_midi_event_t;

//------------------------------------
//------ static variables
// 

// buffers must be word aligned per docs on uhd_ep_run()
COMPILER_WORD_ALIGNED static usb_midi_event_t rxBuf[MIDI_RX_EVENT_BUF_SIZE];
static volatile bool rxBusy = false;
static u32 rxBytes = 0;

// try using an output buffer and adding the extra nib we saw on input ... 
COMPILER_WORD_ALIGNED static usb_midi_event_t txBuf[MIDI_TX_EVENT_BUF_SIZE];
static volatile bool txBusy = false;
static volatile u8 txGetIdx = 0;
static volatile u8 txPutIdx = 0;

// current packet data
static event_t ev = { .type = kEventMidiPacket, .data = 0x00000000 };

//------------------------------------
//----- static functions

// parse the buffer and spawn appropriate events
static void midi_parse_event(void) {
  int i;
  int eventCount = rxBytes >> 2; // assume we receive full events, partials are dropped

  union { u32 data; s32 sdata; } buf;

  usb_midi_event_t* rxEvent = &(rxBuf[0]);

  for (i = 0; i < eventCount; i++) {
    buf.data = (rxEvent->raw) << 8;
    ev.data = buf.sdata;
    event_post(&ev);

    ++rxEvent;
  }
}

// callback for the non-blocking asynchronous read.
static void midi_rx_done(usb_add_t add,
                         usb_ep_t ep,
                         uhd_trans_status_t stat,
                         iram_size_t nb) {
  if (nb > 0) {
    if (stat == UHD_TRANS_NOERROR) {
      rxBytes = nb;
      midi_parse_event();
    }
  }

  rxBusy = false;
}

// callback for the non-blocking asynchronous write.
static void midi_tx_done(usb_add_t add,
                         usb_ep_t ep,
                         uhd_trans_status_t stat,
                         iram_size_t nb) {
  if (stat != UHD_TRANS_NOERROR) {
    print_dbg("\r\n midi tx error (in callback). status: 0x");
    print_dbg_hex((u32)stat);
  }

  txBusy = false;
}


//-----------------------------------------
//----- extern functions

// read and spawn events (non-blocking)
extern void midi_read(void) {
  if (rxBusy == false) {
    rxBytes = 0;
    rxBusy = true;
    if (!uhi_midi_in_run((u8*)rxBuf, sizeof rxBuf, &midi_rx_done)) {
      // hm, every uhd enpoint run always returns error...
      // ...because most of the time a rx job is already running, by only
      // running the endpoint read after midi_rx_done has set rxBusy to false
      // the errors here stop.
      print_dbg("\r\n midi rx endpoint error");
    }
  }
  return;
}

// write to MIDI device
extern bool midi_write(const u8* data, u32 bytes) {
  // NB: this function is not currently used across the module code
  // base therefore the precise nature of the incoming buffer layout
  // is not well defined.
  //
  // here it is assumed that the midi data is packed (not padded to 3
  // bytes) but that running status is not used...
  //
  // TODO: testing...
	//
	// FIXME: if txBuf is not large enough to hold all of data the extra
	// msgs in data are dropped

  u8 events = 1;
  usb_midi_event_t* tx = &(txBuf[0]);
  const usb_midi_event_t* txEnd = &(txBuf[MIDI_RX_EVENT_BUF_SIZE]);

  u8* d = (u8*)data;
  const u8* dEnd = data + bytes;

  u8 status, com, ch;

  if (txBusy == false) {
    print_dbg("\r\n midi_write: no buffers available");
    return false;
  }

  while (tx < txEnd && d < dEnd) {
    // clean the tx buffer
    tx->raw = 0x00000000;

    // grab the status byte
    status = *d; d++;
    com = status >> 4;
    ch = status & 0x0f;

    if (com < 0x8) {
      // bad status byte, just bail
      print_dbg("\r\n midi_write: bad status in data, skipping write");
      return false;
    }

    // format usb midi header, high nib of 1st byte = virtual cable
    // low nib = 4b com code, duplicated from status byte
    tx->header = 0x10 | com;
    tx->msg[0] = status;

    // based on message type copy the correct number of bytes into the
    // events
    if (com < 0xf) {
      // channel mode message
      tx->msg[1] = *d; d++;
      if (com == 0xc || com == 0xd) {
        // program change, aftertouch => 1 byte, nothing more
      }
      else {
        tx->msg[2] = *d; d++;
      }
    }
    else if (ch < 0x8) {
      // system common message
      if (ch == 0x0 || ch == 0x7) {
        // sysex start, end
        print_dbg("\r\n midi_write: sysex not supported, skipping write");
        return false;
      }
      else if (ch == 0x2) {
        // song position pointer
        tx->msg[1] = *d; d++;
        tx->msg[2] = *d; d++;
      }
      else if (ch == 0x1 || ch == 0x3) {
        // midi time code quarter frame, song select
        tx->msg[1] = *d; d++;
      }
      else {
        // undefined or tune request
        // ...nothing to do
      }
    }
    else {
      // system realtime message
      // ...status byte only, nothing to do
    }

    tx++; events++;
  }

  txBusy = true;

  if (!uhi_midi_out_run((uint8_t*)txBuf, events * sizeof(usb_midi_event_t), &midi_tx_done)) {
    // hm, every uhd enpoint run always returns unspecified error...
    //  print_dbg("\r\n midi tx endpoint error");
  }

  return true;
}
extern void midi_write_packet(u8 cable_number, u8 *pack) {
  uint8_t txBuf[4] = {
    pack[0] >> 4,
    pack[0],
    pack[1],
    pack[2]
  };
  txBuf[0] |= (cable_number << 4) & 0xf0;
  txBusy = true;
  if (!uhi_midi_out_run((uint8_t*)txBuf, 4, &midi_tx_done)) {
    print_dbg("\r\n midi tx endpoint error");
  }
}

// MIDI device was plugged or unplugged
extern void midi_change(uhc_device_t* dev, u8 plug) {
  event_t e;

  if (plug) { 
    e.type = kEventMidiConnect; 
  } else {
    e.type = kEventMidiDisconnect;
  }

  // posting an event so the main loop can respond
  event_post(&e); 
}


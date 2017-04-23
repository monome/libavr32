#include "msc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"

// libavr32
#include "events.h"

void msc_change(uhc_device_t* dev, uint8_t plug) {
    if (plug) {
        event_t e = {.type = kEventMscConnect, .data = 0};
        event_post(&e);
    }
    else {
        event_t e = {.type = kEventMscDisconnect, .data = 0};
        event_post(&e);
    }
}

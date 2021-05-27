#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

// libavr32
//#include "events.h"

void cdc_change(uhc_device_t* dev, uint8_t plug) {
    print_dbg("\r\ncdc plug");
    if (plug) {
    }
    else {
    }
}

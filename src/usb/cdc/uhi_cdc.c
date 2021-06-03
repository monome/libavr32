// asf
#include <string.h>
#include "delay.h"
#include "print_funcs.h"
#include "usb_protocol.h"
#include "uhd.h"
#include "uhc.h"
// aleph
#include "conf_usb_host.h"
#include "cdc.h"
#include "uhi_cdc.h"

//------ DEFINES
/*
#define UHI_FTDI_TIMEOUT 20
#define FTDI_STRING_DESC_REQ_TYPE ( (USB_REQ_DIR_IN) | (USB_REQ_TYPE_STANDARD) | (USB_REQ_RECIP_DEVICE) )
#define FTDI_STRING_DESC_LANGID USB_LANGID_EN_US
// offset into the string descriptor to get an actual (unicode) string
#define FTDI_STRING_DESC_OFFSET 2

// control request types
#define FTDI_DEVICE_OUT_REQTYPE 0b01000000
#define FTDI_DEVICE_IN_REQTYPE  0b11000000
// control requests
#define FTDI_REQ_BAUDRATE         0x03
#define FTDI_REQ_LINE_PROPERTIES  0x04
#define FTDI_REQ_BITMODE          0x0b
#define FTDI_REQ_READ_EEPROM      0x90
*/

#define  CDC_CLASS_DEVICE     0x02	//!< USB Communication Device Class
#define  CDC_CLASS_DATA       0x0A	//!< CDC Data Class Interface
#define UHI_MCDC_TIMEOUT 20

//----- data types
// device data structure
typedef struct {
  uhc_device_t *dev;
  usb_ep_t ep_in;
  usb_ep_t ep_out;
} uhi_cdc_dev_t;

//----- static variables

// device data
static uhi_cdc_dev_t uhi_cdc_dev = {
  .dev = NULL,
};

// control read-busy flag
static volatile uint8_t ctlReadBusy = 0;

//------- static funcs

//----- external (UHC) functions
uhc_enum_status_t uhi_cdc_install(uhc_device_t* dev) {
  bool b_iface_supported;
  uint16_t conf_desc_lgt, vid, pid;
  usb_iface_desc_t *ptr_iface;

  // print_dbg("\r\n run uhi_ftdi_install");

  if (uhi_cdc_dev.dev != NULL) {
    return UHC_ENUM_SOFTWARE_LIMIT; // Device already allocated
  }

  // check vid/pid
  vid = le16_to_cpu(dev->dev_desc.idVendor);
  pid = le16_to_cpu(dev->dev_desc.idProduct);

  conf_desc_lgt = le16_to_cpu(dev->conf_desc->wTotalLength);
  ptr_iface = (usb_iface_desc_t*)dev->conf_desc;
  b_iface_supported = false;

  while(conf_desc_lgt) {
    switch (ptr_iface->bDescriptorType) {

      case USB_DT_INTERFACE:
        //print_dbg("\r\nINTERFACE: ");
        //print_dbg_ulong(ptr_iface->bInterfaceClass);
        if ((ptr_iface->bInterfaceClass == CDC_CLASS_DATA) ) {
          //print_dbg("\r\n class/protocol matches CDC. ");
          b_iface_supported = true;
          uhi_cdc_dev.ep_in = 0;
          uhi_cdc_dev.ep_out = 0;
        } else {
          b_iface_supported = false;
        }
        break;

      case USB_DT_ENDPOINT:
        //print_dbg("\r\nENDPOINT:");
        if (!b_iface_supported) {
          break;
        }
        if (!uhd_ep_alloc(dev->address, (usb_ep_desc_t*)ptr_iface)) {
          print_dbg("\r\n endpoint allocation failed");
          return UHC_ENUM_HARDWARE_LIMIT;
        }

        //print_dbg_ulong(((usb_ep_desc_t*)ptr_iface)->bmAttributes & USB_EP_TYPE_MASK);

        switch(((usb_ep_desc_t*)ptr_iface)->bmAttributes & USB_EP_TYPE_MASK) {
          case USB_EP_TYPE_BULK:
            //print_dbg("\r\n allocating bulk endpoint: ");
            if (((usb_ep_desc_t*)ptr_iface)->bEndpointAddress & USB_EP_DIR_IN) {
              uhi_cdc_dev.ep_in = ((usb_ep_desc_t*)ptr_iface)->bEndpointAddress;
            } else {
              uhi_cdc_dev.ep_out = ((usb_ep_desc_t*)ptr_iface)->bEndpointAddress;
            }
            break;
          default:
            print_dbg("\r\n unhandled endpoint in device enumeration");
            break;
        }
        break;
      default:
        //print_dbg("\r\n...");
        break;
    }
    Assert(conf_desc_lgt>=ptr_iface->bLength);
    conf_desc_lgt -= ptr_iface->bLength;
    ptr_iface = (usb_iface_desc_t*)((uint8_t*)ptr_iface + ptr_iface->bLength);
  }

  if (b_iface_supported) {
    uhi_cdc_dev.dev = dev;
    //print_dbg("\r\n completed device install");
    return UHC_ENUM_SUCCESS;
  }
  //print_dbg("\r\n enumeration failed");
  return UHC_ENUM_UNSUPPORTED; // No interface supported
}

void uhi_cdc_enable(uhc_device_t* dev) {
  if (uhi_cdc_dev.dev != dev) {
    return;  // No interface to enable
  }

  cdc_change(dev, true);
}

void uhi_cdc_uninstall(uhc_device_t* dev) {
  if (uhi_cdc_dev.dev != dev) {
    return; // Device not enabled in this interface
  }
  uhi_cdc_dev.dev = NULL;
  Assert(uhi_cdc_dev.report!=NULL);
  cdc_change(dev, false);
}

// run the input endpoint (bulk)
bool uhi_cdc_in_run(uint8_t * buf, iram_size_t buf_size,
    uhd_callback_trans_t callback) {
  return uhd_ep_run(uhi_cdc_dev.dev->address,
      uhi_cdc_dev.ep_in, false, buf, buf_size,
      UHI_MCDC_TIMEOUT, callback);
}

// run the output endpoint (bulk)
bool uhi_cdc_out_run(uint8_t * buf, iram_size_t buf_size,
    uhd_callback_trans_t callback) {
  return uhd_ep_run(uhi_cdc_dev.dev->address,
      uhi_cdc_dev.ep_out, true, buf, buf_size,
      UHI_MCDC_TIMEOUT, callback);
}


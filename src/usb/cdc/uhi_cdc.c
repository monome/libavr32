// asf
#include <string.h>
#include "delay.h"
#include "print_funcs.h"
#include "usb_protocol.h"
#include "uhd.h"
#include "uhc.h"
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
  uint8_t iface_comm;
} uhi_cdc_dev_t;

//----- static variables

// device data
static uhi_cdc_dev_t uhi_cdc_dev = {
  .dev = NULL,
  .ep_in = 0,
  .ep_out = 0,
  .iface_comm = 0,
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

  vid = le16_to_cpu(dev->dev_desc.idVendor);
  pid = le16_to_cpu(dev->dev_desc.idProduct);
  print_dbg("\r\nuhi_cdc_install ");
  print_dbg_hex(vid);
  print_dbg(":");
  print_dbg_hex(pid);

  conf_desc_lgt = le16_to_cpu(dev->conf_desc->wTotalLength);
  ptr_iface = (usb_iface_desc_t*)dev->conf_desc;
  b_iface_supported = false;

  while(conf_desc_lgt) {
    switch (ptr_iface->bDescriptorType) {

      case USB_DT_INTERFACE:
        //print_dbg("\r\nINTERFACE: ");
        //print_dbg_ulong(ptr_iface->bInterfaceClass);
        if ((ptr_iface->bInterfaceClass == CDC_CLASS_DEVICE)) {
          uhi_cdc_dev.iface_comm = ptr_iface->bInterfaceNumber;
        }
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
    print_dbg("\r\nuhi_cdc ok in=0x");
    print_dbg_hex(uhi_cdc_dev.ep_in);
    print_dbg(" out=0x");
    print_dbg_hex(uhi_cdc_dev.ep_out);
    print_dbg(" comm_if=");
    print_dbg_ulong(uhi_cdc_dev.iface_comm);
    return UHC_ENUM_SUCCESS;
  }
  print_dbg("\r\nuhi_cdc unsupported");
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
  cdc_change(dev, false);
}

void uhi_cdc_abort(void) {
  if (uhi_cdc_dev.dev == NULL) {
    return;
  }
  if (uhi_cdc_dev.ep_in != 0) {
    uhd_ep_abort(uhi_cdc_dev.dev->address, uhi_cdc_dev.ep_in);
  }
  if (uhi_cdc_dev.ep_out != 0) {
    uhd_ep_abort(uhi_cdc_dev.dev->address, uhi_cdc_dev.ep_out);
  }
}

// run the input endpoint (bulk)
bool uhi_cdc_in_run(uint8_t * buf, iram_size_t buf_size,
    uhd_callback_trans_t callback) {
  if (uhi_cdc_dev.dev == NULL || uhi_cdc_dev.ep_in == 0) {
    return false;
  }
  return uhd_ep_run(uhi_cdc_dev.dev->address,
      uhi_cdc_dev.ep_in, false, buf, buf_size,
      UHI_MCDC_TIMEOUT, callback);
}

// run the output endpoint (bulk)
bool uhi_cdc_out_run(uint8_t * buf, iram_size_t buf_size,
    uhd_callback_trans_t callback) {
  if (uhi_cdc_dev.dev == NULL || uhi_cdc_dev.ep_out == 0) {
    return false;
  }
  return uhd_ep_run(uhi_cdc_dev.dev->address,
      uhi_cdc_dev.ep_out, true, buf, buf_size,
      UHI_MCDC_TIMEOUT, callback);
}

#define CDC_CTL_SLICES 200
#define CDC_CTL_SLICE_US 100
#define USB_REQ_CDC_SET_LINE_CODING 0x20
#define USB_REQ_CDC_SET_CONTROL_LINE_STATE 0x22
#define CDC_CTRL_SIGNAL_DTE_PRESENT (1 << 0)
#define CDC_CTRL_SIGNAL_ACTIVATE_CARRIER (1 << 1)

static void cdc_ctl_end(usb_add_t add, uhd_trans_status_t status,
                        uint16_t payload_trans) {
  (void)add;
  (void)status;
  (void)payload_trans;
  ctlReadBusy = 0;
}

static uint8_t cdc_ctl_wait(void) {
  uint16_t n = 0;

  while (ctlReadBusy) {
    if (uhi_cdc_dev.dev == NULL) {
      return 0;
    }
    if (++n > CDC_CTL_SLICES) {
      ctlReadBusy = 0;
      return 0;
    }
    delay_us(CDC_CTL_SLICE_US);
  }
  return 1;
}

bool uhi_cdc_acm_open(void) {
  usb_setup_req_t req;
  /* dwDTERate LE 115200, 1 stop, no parity, 8 data */
  static uint8_t coding[7] = {0x00, 0xC2, 0x01, 0x00, 0, 0, 8};

  if (uhi_cdc_dev.dev == NULL) {
    return false;
  }

  req.bmRequestType =
      USB_REQ_DIR_OUT | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_INTERFACE;
  req.bRequest = USB_REQ_CDC_SET_LINE_CODING;
  req.wValue = 0;
  req.wIndex = uhi_cdc_dev.iface_comm;
  req.wLength = sizeof(coding);
  ctlReadBusy = 1;
  if (!uhd_setup_request(uhi_cdc_dev.dev->address, &req, coding,
                         sizeof(coding), NULL, &cdc_ctl_end)) {
    ctlReadBusy = 0;
    print_dbg("\r\ncdc acm line coding req fail");
    return false;
  }
  if (!cdc_ctl_wait()) {
    print_dbg("\r\ncdc acm line coding timeout");
    return false;
  }

  req.bRequest = USB_REQ_CDC_SET_CONTROL_LINE_STATE;
  req.wValue = CDC_CTRL_SIGNAL_DTE_PRESENT | CDC_CTRL_SIGNAL_ACTIVATE_CARRIER;
  req.wIndex = uhi_cdc_dev.iface_comm;
  req.wLength = 0;
  ctlReadBusy = 1;
  if (!uhd_setup_request(uhi_cdc_dev.dev->address, &req, NULL, 0, NULL,
                         &cdc_ctl_end)) {
    ctlReadBusy = 0;
    print_dbg("\r\ncdc acm dtr req fail");
    return false;
  }
  if (!cdc_ctl_wait()) {
    print_dbg("\r\ncdc acm dtr timeout");
    return false;
  }
  print_dbg("\r\ncdc acm dtr on");
  return true;
}


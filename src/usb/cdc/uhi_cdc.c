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
#define UHI_CDCM_TIMEOUT 20

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

// manufacturer string
char manufacturer_string[64];
// product string
char product_string[64];
// serial number string
char serial_string[64];

// control read-busy flag
static volatile uint8_t ctlReadBusy = 0;

//------- static funcs

// send control request
static uint8_t send_ctl_request(uint8_t reqtype, uint8_t reqnum,
    uint8_t* data, uint16_t size,
    uint16_t index, uint16_t val,
    uhd_callback_setup_end_t callbackEnd);
// control request end
static void ctl_req_end(
    usb_add_t add,
    uhd_trans_status_t status,
    uint16_t payload_trans);

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

  /*if( (vid == FTDI_VID) && (pid == FTDI_PID) ) {
    ;; // this is an FTDI device, so continue
    } else {
    return UHC_ENUM_UNSUPPORTED;
    }
   */

  conf_desc_lgt = le16_to_cpu(dev->conf_desc->wTotalLength);
  ptr_iface = (usb_iface_desc_t*)dev->conf_desc;
  b_iface_supported = false;

  while(conf_desc_lgt) {
    switch (ptr_iface->bDescriptorType) {

      case USB_DT_INTERFACE:
        print_dbg("\r\nINTERFACE: ");
        print_dbg_ulong(ptr_iface->bInterfaceClass);
        //if ((ptr_iface->bInterfaceClass == CDC_CLASS_DEVICE) ) {
        if ((ptr_iface->bInterfaceClass == CDC_CLASS_DATA) ) {
            //&& (ptr_iface->bInterfaceProtocol == FTDI_PROTOCOL) ) {
          print_dbg("\r\n class/protocol matches CDC. ");
          b_iface_supported = true;
          uhi_cdc_dev.ep_in = 0;
          uhi_cdc_dev.ep_out = 0;
        } else {
          b_iface_supported = false;
        }
        break;

      case USB_DT_ENDPOINT:
        print_dbg("\r\nENDPOINT:");
        if (!b_iface_supported) {
          break;
        }
        if (!uhd_ep_alloc(dev->address, (usb_ep_desc_t*)ptr_iface)) {
          print_dbg("\r\n endpoint allocation failed");
          return UHC_ENUM_HARDWARE_LIMIT;
        }

        print_dbg_ulong(((usb_ep_desc_t*)ptr_iface)->bmAttributes & USB_EP_TYPE_MASK);

        switch(((usb_ep_desc_t*)ptr_iface)->bmAttributes & USB_EP_TYPE_MASK) {
          case USB_EP_TYPE_BULK:
            print_dbg("\r\n allocating bulk endpoint: ");
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
        print_dbg("\r\n...");
        break;
    }
    Assert(conf_desc_lgt>=ptr_iface->bLength);
    conf_desc_lgt -= ptr_iface->bLength;
    ptr_iface = (usb_iface_desc_t*)((uint8_t*)ptr_iface + ptr_iface->bLength);
  }

  if (b_iface_supported) {
    uhi_cdc_dev.dev = dev;
    print_dbg("\r\n completed device install");
    return UHC_ENUM_SUCCESS;
  }
  print_dbg("\r\n enumeration failed");
  return UHC_ENUM_UNSUPPORTED; // No interface supported
}

void uhi_cdc_enable(uhc_device_t* dev) {

  /*
  if (uhi_cdc_dev.dev != dev) {
    return;  // No interface to enable
  }
  /// bit mode (not bitbang? )
  /// todo: what do these mean???
  // val : ff
  // indx : 1
  send_ctl_request(FTDI_DEVICE_OUT_REQTYPE,
      FTDI_REQ_BITMODE,
      NULL, 0,
      1, 0xff,
      NULL);
  /// line property
  /// todo: what do these mean???
  // index 1
  // val : 8
  send_ctl_request(FTDI_DEVICE_OUT_REQTYPE,
      FTDI_REQ_LINE_PROPERTIES,
      NULL, 0,
      1, 8,
      NULL);
  /// baud rate
  // rq : 3
  // value: 26 (baudrate: 115200)
  // value: 49206 (baudrate : 57600)
  // index: 0
  send_ctl_request(FTDI_DEVICE_OUT_REQTYPE,
      FTDI_REQ_BAUDRATE,
      NULL, 0,
      0, 49206,
      NULL);

  cdc_change(dev, true);
  */
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
      UHI_CDCM_TIMEOUT, callback);
}

// run the output endpoint (bulk)
bool uhi_cdc_out_run(uint8_t * buf, iram_size_t buf_size,
    uhd_callback_trans_t callback) {
  return uhd_ep_run(uhi_cdc_dev.dev->address,
      uhi_cdc_dev.ep_out, true, buf, buf_size,
      UHI_CDCM_TIMEOUT, callback);
}

//----------------
//---- static functions definitions

// send control request
static uint8_t send_ctl_request(uint8_t reqtype, uint8_t reqnum,
    uint8_t* data, uint16_t size,
    uint16_t index, uint16_t val,
    uhd_callback_setup_end_t callbackEnd) {
  usb_setup_req_t req;

  /* if (uhi_ftdi_dev.dev != dev) { */
  /*   return;  // No interface to enable */
  /* } */

  req.bmRequestType = reqtype;
  req.bRequest = reqnum;
  req.wValue = (val);
  req.wIndex = (index);
  req.wLength = (size);
  return uhd_setup_request(
      uhi_cdc_dev.dev->address,
      &req,
      data,
      size,
      NULL,
      callbackEnd
      );
}

// to be called when control read is complete
static void ctl_req_end(
    usb_add_t add,
    uhd_trans_status_t status,
    uint16_t payload_trans) {
  // last transfer ok?
  //  print_dbg("\r\n ctl request end, status: ");
  //  print_dbg_hex((u32)status);
  ctlReadBusy = 0;
}

// read eeprom
uint8_t cdc_get_strings(char** pManufacturer, char** pProduct, char** pSerial) {
  return 0;
}
#if 0
  // get manufacturer string
  ctlReadBusy = 1;
  //  print_dbg("\r\n sending ctl request for manufacturer string, index : ");
  //  print_dbg_hex(uhi_ftdi_dev.dev->dev_desc.iManufacturer);
  if(!(send_ctl_request(
          /* req type*/
          FTDI_STRING_DESC_REQ_TYPE,
          /* req num */
          USB_REQ_GET_DESCRIPTOR,
          /* data */
          (u8*)manufacturer_string,
          /* size */
          FTDI_STRING_MAX_LEN,
          //			0x2000,
          /*idx*/
          FTDI_STRING_DESC_LANGID,
          /*val*/
          // high byte is 3 for string descriptor (yay, magic!)
          (USB_DT_STRING << 8) | uhi_cdc_dev.dev->dev_desc.iManufacturer,
          // end-transfer callback
          &ctl_req_end )

      )) {
    // print_dbg("\r\n control request for string descriptor failed");
    return 0;
  }
  // wait for transfer end
  while(ctlReadBusy) { ;; }

  // get product string
  ctlReadBusy = 1;
  //  print_dbg("\r\n sending ctl request for product string, index : ");
  //  print_dbg_ulong( uhi_ftdi_dev.dev->dev_desc.iProduct);
  if(!(send_ctl_request(
          /* req type*/
          FTDI_STRING_DESC_REQ_TYPE,
          /* req num */
          USB_REQ_GET_DESCRIPTOR,
          /* data */
          (u8*)product_string,
          /* size */
          FTDI_STRING_MAX_LEN,
          /*idx*/
          FTDI_STRING_DESC_LANGID,
          /*val*/
          // ??
          (USB_DT_STRING << 8) | uhi_cdc_dev.dev->dev_desc.iProduct,
          // end-transfer callback
          &ctl_req_end )

      )) {
    // print_dbg("\r\n control request for string descriptor failed");
    return 0;
  }
  // wait for transfer end
  while(ctlReadBusy) { ;; }

  // get serial string
  ctlReadBusy = 1;
  // print_dbg("\r\n sending ctl request for serial string : ");
  if(!(send_ctl_request(
          /* req type*/
          FTDI_STRING_DESC_REQ_TYPE,
          /* req num */
          USB_REQ_GET_DESCRIPTOR,
          /* data */
          (u8*)serial_string,
          /* size */
          FTDI_STRING_MAX_LEN,
          /*idx*/
          FTDI_STRING_DESC_LANGID,
          /*val*/
          (USB_DT_STRING << 8) | uhi_cdc_dev.dev->dev_desc.iSerialNumber,
          // end-transfer callback
          &ctl_req_end )

      )) {
    // print_dbg("\r\n control request for string descriptor failed");
    return 0;
  }
  // wait for transfer end
  while(ctlReadBusy) { ;; }

  //  print_dbg("\r\n requested all string descriptors.");
  *pManufacturer = manufacturer_string + FTDI_STRING_DESC_OFFSET;
  *pProduct = product_string + FTDI_STRING_DESC_OFFSET;
  *pSerial = serial_string + FTDI_STRING_DESC_OFFSET;

  return 1;
}
#endif

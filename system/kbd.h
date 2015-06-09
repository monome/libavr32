#ifndef _KBD_H_
#define _KBD_H_

#include "types.h"

#define BANG        (0x1E)
#define AT          (0x1F)
#define POUND       (0x20)
#define DOLLAR      (0x21)
#define PERCENT     (0x22)
#define CAP         (0x23)
#define AND         (0x24)
#define STAR        (0x25)
#define OPENBKT     (0x26)
#define CLOSEBKT    (0x27)

#define RETURN      (0x28)
#define ESCAPE      (0x29)
#define BACKSPACE   (0x2A)
#define TAB         (0x2B)
#define SPACE       (0x2C)
#define HYPHEN      (0x2D)
#define EQUAL       (0x2E)
#define SQBKTOPEN   (0x2F)
#define SQBKTCLOSE  (0x30)
#define BACKSLASH   (0x31)
#define SEMICOLON   (0x33)
#define INVCOMMA    (0x34)
#define TILDE       (0x35)
#define COMMA       (0x36)
#define PERIOD      (0x37)
#define FRONTSLASH  (0x38)
#define DELETE      (0x4c)
/**/
/* Modifier masks. One for both modifiers */
#define SHIFT       0x22
#define CTRL        0x11
#define ALT         0x44
#define META         0x88
/**/
/* "Sticky keys */
#define CAPSLOCK    (0x39)
#define NUMLOCK     (0x53)
#define SCROLLLOCK  (0x47)
/* Sticky keys output report bitmasks */
#define bmNUMLOCK       0x01
#define bmCAPSLOCK      0x02
#define bmSCROLLLOCK    0x04
/**/


s8 old_frame[8];

extern u8 hid_to_ascii_raw(u8 data);
extern u8 hid_to_ascii(u8 data, u8 mod);
extern bool frame_compare(u8 data);
extern void set_old_frame(const s8 *data);

#endif
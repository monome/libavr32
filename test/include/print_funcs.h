#ifndef __PRINT_FUNCS_H__
#define __PRINT_FUNCS_H__

#include <stdio.h>

#define print_dbg(s) printf("%s", (s))
#define print_dbg_char(c) printf("%c", (c))
#define print_dbg_hex(x) printf("%lx", ((unsigned long)x))
#define print_dbg_ulong(n) printf("%d", (n))

#endif

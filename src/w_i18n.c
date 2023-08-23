/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2023 by Thomas Loimer
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and documentation
 * files (the "Software"), including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense and/or sell copies of
 * the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

/*
 * Copyright (c) 1995-2002 by T. Sato
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "w_i18n.h"

#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>     /* includes X11/Xlib.h, which includes X11/X.h */

#include "resources.h"
#include "u_fonts.h"
#include "xfig_math.h"


/* replacement of Times_Roman_bits etc, for Japanese */
unsigned char Japanese_Times_Roman_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0xfe,0x20,0xf8,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0xfe,0x67,0x00,0x00,0x00,0x00,0xc0,0x3f,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x7e,0x82,0xfe,0x8b,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x80,0x71,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x42,0x82,0x20,0x88,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x62,0x04,0x00,0x00,0x00,0x00,0x80,0x61,0x00,0x00,
   0x00,0x00,0x00,0x00,0x20,0x00,0x42,0x82,0xfc,0x89,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x80,0x61,0x00,
   0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x42,0xfe,0x04,0xf9,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x60,0x70,0xce,0x18,0x3c,0x16,0x80,0x61,
   0x78,0x9c,0x31,0x3c,0xce,0x00,0x20,0x00,0x7e,0x82,0x04,0x89,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x60,0x60,0xec,0x3d,0x66,0x19,0x80,
   0x31,0xcc,0xd8,0x7b,0x26,0xec,0x01,0x20,0x00,0x42,0x82,0xfc,
   0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x9c,0x33,0x43,0x13,
   0x80,0x1f,0x86,0x39,0x67,0x66,0x9c,0x01,0xfe,0x03,0x42,0x82,
   0x04,0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x8c,0x31,0x7f,
   0x07,0x80,0x1d,0x86,0x19,0x63,0x70,0x8c,0x01,0x20,0x00,0x42,
   0xfe,0x04,0xf9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x8c,0x31,
   0x03,0x8e,0x8f,0x19,0x86,0x19,0x63,0x6c,0x8c,0x01,0x20,0x00,
   0x42,0x82,0xfc,0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x8c,
   0x31,0x03,0x1c,0x80,0x31,0x86,0x19,0x63,0x66,0x8c,0x01,0x20,
   0x00,0x7e,0x82,0x20,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,
   0x8c,0x31,0x03,0x19,0x80,0x61,0x86,0x19,0x63,0x66,0x8c,0x01,
   0x20,0x00,0x00,0x83,0xfe,0x8b,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,
   0x60,0x8c,0x31,0x66,0x13,0x80,0xc1,0xcc,0x18,0x63,0x7e,0x8c,
   0x01,0x00,0x00,0x00,0x81,0x20,0x8c,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0xf0,0xf0,0xde,0x7b,0x3c,0x0d,0xc0,0xc3,0x79,0xbc,0xf7,0xcc,
   0x9e,0x03,0x00,0x00,0x80,0x81,0x20,0x84,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0xe0,0xe0,0x20,0xc6,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char Japanese_Roman_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x01,0x00,0x80,0x7f,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0xf0,0x07,0xc1,0x07,0x00,0xe3,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0xf0,0x13,0xf4,0x5f,0x04,0x00,0xc3,
   0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x10,0x12,0x04,0x41,0x04,
   0x00,0xc3,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x10,0x12,0xe4,
   0x4f,0x04,0x00,0xc3,0xf0,0x38,0x63,0xf0,0x38,0x03,0x08,0x10,
   0xf2,0x27,0xc8,0x07,0x00,0x63,0x98,0xb1,0xf7,0x98,0xb1,0x07,
   0x08,0xf0,0x13,0x24,0x48,0x04,0x00,0x3f,0x0c,0x73,0xce,0x98,
   0x71,0x86,0xff,0x10,0x12,0xe4,0x4f,0x04,0x00,0x33,0x0c,0x33,
   0xc6,0xc0,0x31,0x06,0x08,0x10,0x12,0x24,0x48,0x04,0x00,0x63,
   0x0c,0x33,0xc6,0xb0,0x31,0x06,0x08,0x10,0xf2,0x27,0xc8,0x07,
   0x00,0xc3,0x0c,0x33,0xc6,0x98,0x31,0x06,0x08,0x10,0x12,0xe4,
   0x4f,0x04,0x00,0xc3,0x0c,0x33,0xc6,0x98,0x31,0x06,0x08,0xf0,
   0x13,0x04,0x41,0x04,0x00,0x83,0x99,0x31,0xc6,0xf8,0x31,0x06,
   0x00,0x00,0x18,0xf4,0x5f,0x04,0x80,0x87,0xf3,0x78,0xef,0x31,
   0x7b,0x0e,0x00,0x00,0x08,0x04,0x61,0x04,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x04,0x21,0x04,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x31,0x06,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00};

unsigned char Japanese_Times_Bold_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x0a,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,
   0x7f,0x0e,0x00,0x00,0x00,0x00,0xe0,0x0f,0x00,0x1e,0x78,0x00,
   0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x30,0x67,0x0e,0x00,0x00,0x00,0x00,0xc0,0x39,0x00,0x1c,0x70,
   0x00,0x08,0xc0,0xff,0x03,0x06,0x00,0x00,0x00,0xfc,0x01,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x10,0x47,0x0e,0x00,0x00,0x00,0x00,0xc0,0x71,0x00,0x1c,
   0x70,0x00,0x08,0x00,0x00,0x02,0x04,0x00,0x00,0x00,0x04,0x01,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0xc0,0x71,0x00,
   0x1c,0x70,0x00,0x08,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x06,
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x07,0xef,0x9d,0x83,0xc7,0x07,0xc0,0x71,
   0x3c,0x1c,0x76,0x00,0x08,0x00,0x00,0xc2,0x00,0x04,0x04,0x00,
   0x83,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x07,0xce,0x7b,0xc7,0x6d,0x06,0xc0,
   0x39,0x66,0x1c,0x7f,0x80,0xff,0x00,0x00,0x82,0x01,0x86,0x0c,
   0x81,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xce,0x39,0xe7,0xec,0x04,
   0xc0,0x0f,0xe7,0x9c,0x73,0x00,0x08,0x00,0x00,0x02,0x01,0x83,
   0x09,0x01,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xce,0x39,0xe7,0xef,
   0xe1,0xc7,0x39,0xe7,0x9c,0x73,0x00,0x08,0x00,0x00,0x02,0x80,
   0x01,0x81,0x01,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xce,0x39,0xe7,
   0xc0,0xe3,0xc7,0x71,0xe7,0x9c,0x73,0x00,0x08,0x00,0x00,0x02,
   0xc0,0x00,0x80,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xce,0x39,
   0xe7,0x80,0x07,0xc0,0x71,0xe7,0x9c,0x73,0x00,0x08,0x00,0x00,
   0x02,0x60,0x00,0xc0,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xce,
   0x39,0xe7,0x20,0x07,0xc0,0x71,0xe7,0x9c,0x73,0x00,0x00,0x00,
   0x00,0x02,0x38,0x00,0x60,0x00,0x18,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,
   0xce,0x39,0xc7,0x6d,0x06,0xc0,0x39,0x66,0x1c,0x77,0x00,0x00,
   0xc0,0xff,0x03,0x0e,0x00,0x30,0x00,0x0e,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
   0x0f,0xff,0x7b,0x8f,0xe7,0x03,0xe0,0x1f,0x3c,0x3e,0xee,0x00,
   0x00,0x00,0x00,0xc0,0x03,0x00,0x18,0x80,0x03,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char Japanese_Bold_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,
   0x01,0x00,0x00,0x00,0x00,0x01,0xf8,0x07,0x80,0x07,0x1e,0x00,
   0x00,0x40,0x01,0x00,0x00,0x00,0x00,0x01,0x70,0x0e,0x00,0x07,
   0x1c,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x01,0x70,0x1c,
   0x00,0x07,0x1c,0x10,0xf8,0x7f,0xc0,0x00,0x00,0x00,0x80,0x3f,
   0x70,0x1c,0x00,0x07,0x1c,0x10,0x00,0x40,0x80,0x00,0x00,0x00,
   0x80,0x20,0x70,0x1c,0x0f,0x87,0x1d,0x10,0x00,0x40,0x00,0x00,
   0x00,0x00,0xc0,0x20,0x70,0x8e,0x19,0xc7,0x1f,0x10,0x00,0x40,
   0x18,0x80,0x80,0x00,0x60,0x30,0xf0,0xc7,0x39,0xe7,0x1c,0xff,
   0x01,0x40,0x30,0xc0,0x90,0x21,0x30,0x10,0x70,0xdc,0x39,0xe7,
   0x1c,0x10,0x00,0x40,0x20,0x60,0x30,0x21,0x00,0x18,0x70,0xf8,
   0x39,0xe7,0x1c,0x10,0x00,0x40,0x00,0x30,0x20,0x30,0x00,0x08,
   0x70,0xf8,0x39,0xe7,0x1c,0x10,0x00,0x40,0x00,0x18,0x00,0x10,
   0x00,0x0c,0x70,0xf8,0x39,0xe7,0x1c,0x10,0x00,0x40,0x00,0x0c,
   0x00,0x18,0x00,0x06,0x70,0x9c,0x19,0xc7,0x1d,0x00,0x00,0x40,
   0x00,0x07,0x00,0x0c,0x00,0x03,0xf8,0x07,0x8f,0x8f,0x3b,0x00,
   0xf8,0x7f,0xc0,0x01,0x00,0x06,0xc0,0x01,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x03,0x70,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x01,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00};

/* replacement of Times_Roman_bits etc, for Korean (from Jeon Hyoung-Jo) */
unsigned char Korean_Times_Roman_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0xfc,0xcf,0x00,0x00,0x00,0x00,0x80,0x7f,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0xcc,0xcc,0x00,0x00,0x00,0x00,0x00,0xe3,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0xfc,0x61,0xe0,0x0f,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0xc4,0x08,0x00,0x00,0x00,0x00,0x00,0xc3,0x00,0x00,
   0x00,0x00,0x00,0x00,0x40,0x00,0x98,0x7f,0x00,0x07,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0xc3,0x00,
   0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x98,0x61,0x80,0x01,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0xc0,0xe0,0x9c,0x31,0x78,0x2c,0x00,0xc3,
   0xf0,0x38,0x63,0x78,0x9c,0x01,0x40,0x00,0x98,0x7f,0xc0,0x1e,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0xd8,0x7b,0xcc,0x32,0x00,
   0x63,0x98,0xb1,0xf7,0x4c,0xd8,0x03,0x40,0x00,0xf8,0x61,0x60,
   0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x38,0x67,0x86,0x26,
   0x00,0x3f,0x0c,0x73,0xce,0xcc,0x38,0x03,0xfc,0x07,0x08,0x20,
   0x30,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x18,0x63,0xfe,
   0x0e,0x00,0x3b,0x0c,0x33,0xc6,0xe0,0x18,0x03,0x40,0x00,0x00,
   0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x18,0x63,
   0x06,0x1c,0x1f,0x33,0x0c,0x33,0xc6,0xd8,0x18,0x03,0x40,0x00,
   0x00,0x3e,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x18,
   0x63,0x06,0x38,0x00,0x63,0x0c,0x33,0xc6,0xcc,0x18,0x03,0x40,
   0x00,0x00,0x63,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,
   0x18,0x63,0x06,0x32,0x00,0xc3,0x0c,0x33,0xc6,0xcc,0x18,0x03,
   0x40,0x00,0x00,0x63,0xfc,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,
   0xc0,0x18,0x63,0xcc,0x26,0x00,0x83,0x99,0x31,0xc6,0xfc,0x18,
   0x03,0x00,0x00,0x00,0x63,0x08,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0xe0,0xe1,0xbd,0xf7,0x78,0x1a,0x80,0x87,0xf3,0x78,0xef,0x99,
   0x3d,0x07,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char Korean_Roman_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0xc6,0x01,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x00,0x86,
   0x01,0x00,0x00,0x00,0x00,0x00,0x10,0xf0,0x87,0x81,0x3f,0x00,
   0x00,0x86,0x01,0x00,0x00,0x00,0x00,0x00,0x10,0x60,0xfe,0x01,
   0x1c,0x00,0x00,0x86,0xe1,0x71,0xc6,0xe0,0x71,0x06,0x10,0x60,
   0x86,0x01,0x06,0x00,0x00,0xc6,0x30,0x63,0xef,0x31,0x63,0x0f,
   0x10,0x60,0xfe,0x01,0x7b,0x00,0x00,0x7e,0x18,0xe6,0x9c,0x31,
   0xe3,0x0c,0xff,0xe1,0x87,0x81,0xe1,0x00,0x00,0x66,0x18,0x66,
   0x8c,0x81,0x63,0x0c,0x10,0x20,0x80,0x40,0x04,0x00,0x00,0xc6,
   0x18,0x66,0x8c,0x61,0x63,0x0c,0x10,0x00,0x00,0x00,0x04,0x00,
   0x00,0x86,0x19,0x66,0x8c,0x31,0x63,0x0c,0x10,0x00,0xf8,0x00,
   0x04,0x00,0x00,0x86,0x19,0x66,0x8c,0x31,0x63,0x0c,0x10,0x00,
   0x8c,0x01,0x8c,0x01,0x00,0x06,0x33,0x63,0x8c,0xf1,0x63,0x0c,
   0x00,0x00,0x8c,0xf1,0xff,0x03,0x00,0x0f,0xe7,0xf1,0xde,0x63,
   0xf6,0x1c,0x00,0x00,0x8c,0x21,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00};

unsigned char Korean_Times_Bold_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0xfe,0xcf,0x01,0x00,0x00,0x00,0x00,0xfc,0x01,0xc0,0x03,
   0x0f,0x00,0x00,0x00,0x00,0xe0,0x0f,0x01,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0xe6,0xcc,0x01,0x00,0x00,0x00,0x00,0x38,0x07,0x80,
   0x03,0x0e,0x00,0x01,0xe0,0xff,0x20,0x00,0x01,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0xe2,0xc8,0x01,0x00,0x00,0x00,0x00,0x38,0x0e,
   0x80,0x03,0x0e,0x00,0x01,0x00,0x80,0x20,0x00,0x01,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x38,
   0x0e,0x80,0x03,0x0e,0x00,0x01,0x00,0x80,0x20,0x00,0x01,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0xe0,0xe0,0xbd,0x73,0xf0,0xf8,0x00,
   0x38,0x8e,0x87,0xc3,0x0e,0x00,0x01,0x00,0x80,0x20,0x00,0x01,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xc0,0x79,0xef,0xb8,0xcd,
   0x00,0x38,0xc7,0x8c,0xe3,0x0f,0xf0,0x1f,0x00,0x80,0xe0,0x0f,
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xc0,0x39,0xe7,0x9c,
   0x9d,0x00,0xf8,0xe1,0x9c,0x73,0x0e,0x00,0x01,0x00,0x80,0x00,
   0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xc0,0x39,0xe7,
   0xfc,0x3d,0xfc,0x38,0xe7,0x9c,0x73,0x0e,0x00,0x01,0x00,0x04,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xc0,0x39,
   0xe7,0x1c,0x78,0xfc,0x38,0xee,0x9c,0x73,0x0e,0x00,0x01,0x00,
   0x04,0x80,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xc0,
   0x39,0xe7,0x1c,0xf0,0x00,0x38,0xee,0x9c,0x73,0x0e,0x00,0x01,
   0x00,0x04,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,
   0xc0,0x39,0xe7,0x1c,0xe4,0x00,0x38,0xee,0x9c,0x73,0x0e,0x00,
   0x00,0xf0,0xff,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0xe0,0xc0,0x39,0xe7,0xb8,0xcd,0x00,0x38,0xc7,0x8c,0xe3,0x0e,
   0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0xf0,0xe1,0x7f,0xef,0xf1,0x7c,0x00,0xfc,0x83,0xc7,0xc7,
   0x1d,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char Korean_Bold_bits[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xf0,0xc0,
   0x03,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0xce,0x01,
   0xe0,0x80,0x03,0x00,0x00,0x00,0xf0,0x87,0x00,0x00,0x00,0x00,
   0x8e,0x03,0xe0,0x80,0x03,0x08,0xf0,0x7f,0x10,0x80,0x00,0x00,
   0x00,0x00,0x8e,0x03,0xe0,0x80,0x03,0x08,0x00,0x40,0x10,0x80,
   0x00,0x00,0x00,0x00,0x8e,0xe3,0xe1,0xb0,0x03,0x08,0x00,0x40,
   0x10,0x80,0x00,0x00,0x00,0x00,0xce,0x31,0xe3,0xf8,0x03,0x08,
   0x00,0x40,0x10,0x80,0x00,0x00,0x00,0x00,0xfe,0x38,0xe7,0x9c,
   0x83,0xff,0x00,0x40,0xf0,0x87,0x00,0x00,0x00,0x00,0x8e,0x3b,
   0xe7,0x9c,0x03,0x08,0x00,0x40,0x00,0x80,0x00,0x00,0x00,0x00,
   0x0e,0x3f,0xe7,0x9c,0x03,0x08,0x00,0x40,0x00,0x00,0x00,0x00,
   0x00,0x00,0x0e,0x3f,0xe7,0x9c,0x03,0x08,0x00,0x02,0xc0,0xff,
   0x00,0x00,0x00,0x00,0x0e,0x3f,0xe7,0x9c,0x03,0x08,0x00,0x02,
   0x00,0x80,0x00,0x00,0x00,0x00,0x8e,0x33,0xe3,0xb8,0x03,0x00,
   0x00,0x02,0x00,0x80,0x00,0x00,0x00,0x00,0xff,0xe0,0xf1,0x71,
   0x07,0x00,0xf8,0xff,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00};


/* check if str include i18n character */
static Boolean is_i18n_text(str)
     char *str;
{
  int i;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] & 0x80) return TRUE;
  }
  return FALSE;
}

/* get fontset and size corresponding to font specified by fid */
/* return FALSE if the font is not for i18n */
static Boolean seek_fontset(fid, fontset, size_ret)
     Font fid;
     XFontSet *fontset;
     int *size_ret;
{
  extern struct _xfstruct x_fontinfo[]; /* X11 fontnames */
  struct xfont *nf;
  int i;
  if (!appres.international) return FALSE;
  for (i = 0; i < NUM_FONTS; i++) {
    nf = x_fontinfo[i].xfontlist;
    while (nf != NULL && (nf->fset == NULL || nf->fstruct->fid != fid))
      nf = nf->next;
    if (nf != NULL && nf->fset != NULL) {
      *fontset=nf->fset;
      *size_ret = nf->size;
      return TRUE;
    }
  }
  return FALSE;
}

int
i18n_fontset_height(fontset)
     XFontSet fontset;
{
  XFontSetExtents *extents;
  extents = XExtentsOfFontSet(fontset);
  return extents->max_logical_extent.height;
}


/* get extents of the text */
void
i18n_text_extents(font, str, len, dir, asc, des, overall)
     XFontStruct *font;
     char *str;
     int len;
     int *dir, *asc, *des;  /* assume these return values are not used */
     XCharStruct *overall;
{
  XFontSet fontset;
  int font_size;
  XRectangle ink, logical;
  double scale;
  if (!appres.international || !is_i18n_text(str)
      || !seek_fontset(font->fid, &fontset, &font_size)) {
    XTextExtents(font, str, len, dir, asc, des, overall);
  } else {
    XmbTextExtents(fontset, str, len, &ink, &logical);
    if (appres.fontset_size <= 0)
      scale = (double)font_size / (double)i18n_fontset_height(fontset);
    else
      scale = (double)font_size / (double)appres.fontset_size;
    overall->width = max2(logical.width, ink.width + ink.x) * scale + 0.5;
    overall->lbearing = -ink.x * scale;
    overall->rbearing = overall->width;
    overall->ascent = -logical.y * scale;
    overall->descent = logical.height * scale - overall->ascent;
  }
}


#ifndef HAVE_SETLOCALE

char *setlocale(category, locale)
     int category;
     const char *locale;
{
  static char old_locale[100] = "C";
  static char cur_locale[100] = "C";
  const char *s;
  if (locale == NULL) {
    return cur_locale;
  } else if (category == LC_ALL) {
    strcpy(old_locale, cur_locale);
    if (locale[0] == '\0') {
      s = getenv("LANG");
      if (s == NULL) s = "C";  /* LANG not defined */
    } else {
      s = locale;
    }
    strncpy(cur_locale, s, sizeof(cur_locale) - 1);
    return old_locale;
  } else {
    return cur_locale;
  }
}
#endif  /* HAVE_SETLOCALE */

/* ************************************************************************ */

/* Header file for the `xvertext 5.0' routines.
 *
 *  Copyright (c) 1993 Alan Richardson (mppa3@uk.ac.sussex.syma)
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
 *  Modified 2018-09-18 by Thomas Loimer <thomas.loimer@tuwien.ac.at>
 *
 */

/* ************************************************************************ */

#ifndef W_ROTTEXT_H
#define W_ROTTEXT_H

#define XV_VERSION      5.0
#define XV_COPYRIGHT \
      "xvertext routines Copyright (c) 1993 Alan Richardson"


/* ---------------------------------------------------------------------- */


/* text alignment */

#define NONE             0
#define TLEFT            1
#define TCENTRE          2
#define TRIGHT           3
#define MLEFT            4
#define MCENTRE          5
#define MRIGHT           6
#define BLEFT            7
#define BCENTRE          8
#define BRIGHT           9


/* ---------------------------------------------------------------------- */

extern float   XRotVersion(char *str, int n);
extern void    XRotSetMagnification(float m);
extern void    XRotSetBoundingBoxPad(int p);
extern int     XRotDrawString(Display *dpy, XFontStruct *font, float angle, Drawable drawable, GC gc, int x, int y, char *str);
extern int     XRotDrawImageString(Display *dpy, XFontStruct *font, float angle, Drawable drawable, GC gc, int x, int y, char *str);

/* ---------------------------------------------------------------------- */


#endif /* W_ROTTEXT_H */

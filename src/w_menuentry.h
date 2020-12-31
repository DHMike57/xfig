/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2020 by Thomas Loimer
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
    w_menuentry.h - Public Header file for subclassed BSB Menu Entry object

    This adds the underline resource to underline one character of the label
*/

#ifndef W_MENUENTRY_H
#define W_MENUENTRY_H

#if defined HAVE_CONFIG_H && !defined VERSION
#include "config.h"
#endif

#ifdef XAW3D
#include <X11/Xaw3d/Sme.h>
#else
#include <X11/Xaw/Sme.h>
#endif

#ifdef XAW3D1_5E
#include <X11/Xaw3d/SmeBSB.h>
#else
#include "SmeBSB.h"
#endif

/****************************************************************
 *
 * FigSmeBSB object
 *
 ****************************************************************/

/* FigBSB Menu Entry Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 underline	     Index		int		-1

*/

typedef struct _FigSmeBSBClassRec    *FigSmeBSBObjectClass;
typedef struct _FigSmeBSBRec         *FigSmeBSBObject;

extern WidgetClass figSmeBSBObjectClass;

#define XtNunderline   "underline"

#endif

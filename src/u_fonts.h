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


#ifndef U_FONTS_H
#define U_FONTS_H

#include <X11/Xlib.h>		/* includes X11/X.h */
#include <X11/Xft/Xft.h>

#include "object.h"		/* F_pos */

#define DEF_FONTSIZE		12		/* default font size in pts */
#define DEF_LATEX_FONT		0
#define PS_FONTPANE_WD		290
#define LATEX_FONTPANE_WD	112
#define PS_FONTPANE_HT		20
#define LATEX_FONTPANE_HT	20
#define NUM_FONTS		35
#define NUM_LATEX_FONTS		6
/* font number for the "nil" font (when user wants tiny text) */
#define NILL_FONT NUM_FONTS

/* element of linked list for each font
   The head of list is for the different font NAMES,
   and the elements of this list are for each different
   point size of that font */

struct xfont {
    int		    size;	/* size in points */
    Font	    fid;	/* X font id */
    char	   *fname;	/* actual name of X font found */
    char	   *bname;	/* name of backup X font to try if first doesn't exist */
    XFontStruct	   *fstruct;	/* X font structure */
    XFontSet       fset;	/* X font set - used in international mode*/
    struct xfont   *next;	/* next in the list */
};

struct _fstruct {
    char	   *name;	/* Postscript font name */
    int		    xfontnum;	/* template for locating X fonts */
};


extern int	psfontnum(char *font);
extern int	latexfontnum(char *font);
extern int	x_fontnum(int psflag, int fnum);
extern void	closefont(XftFont *font);
extern XftFont	*getfont(int psflag, int fnum, double size, double angle);
extern void	textextents(F_text *t);
extern int	textlength(XftFont *horfont, XftChar8 *string, int len);
extern void	textmaxheight(int psflag, int font, int size, int *ascent,
				int *descent);
extern void	text_origin(int *draw_x, int *draw_y, int base_x, int base_y,
				int align, F_pos offset);
extern struct _fstruct	ps_fontinfo[];
extern struct _fstruct	latex_fontinfo[];

/*
 * For (ITC) Zapf Dingbats, URW Dingbats, or URW D050000L as well
 * as Symbol, Standard Symbols PS, or URW Standard Symbols L we
 * need to map the byte characters into UTF-8 multi byte characters.
 * This to make XftTextExtentsUtf8() and XftDrawStringUtf8() working.
 */
typedef XftChar8 (*map_f)(XftChar8);
extern XftChar32 map_dingbats(XftChar8);
extern XftChar32 map_symbols(XftChar8);
extern map_f adobe_charset(XftFont *font);

#endif /* U_FONTS_H */

/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty."
 *
 */

/*
 * This file provides some drawing primitives which make use of the
 * underlying low-level windowing system operations.
 *
 * The file is divided into routines for:
 *
 * GRAPHICS CONTEXTS (which are used by all the following)
 * FONTS
 * LINES
 * SHADING
 */

/* IMPORTS */

#include "fig.h"
#include "resources.h"
#include "paintop.h"
#include "mode.h"
#include "object.h"
#include "u_fonts.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_icons.h"		/* for none_ic in init_fill_pm */
#include "w_indpanel.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"

#define set_x_color(gc,col) XSetForeground(tool_d,gc,\
	(!all_colors_available? (col==WHITE?x_bg_color.pixel:x_fg_color.pixel): \
	(col<0||col>=NUMCOLORS)? x_fg_color.pixel:appres.color[col]))

#define x_color(col)\
	(!all_colors_available? (col==WHITE?x_bg_color.pixel:x_fg_color.pixel): \
	(col<0||col>=NUMCOLORS)? x_fg_color.pixel:appres.color[col])

extern struct _xfstruct x_fontinfo[NUM_X_FONTS];
extern struct _fstruct ps_fontinfo[];	/* font names */
extern choice_info fillstyle_choices[];

/* EXPORTS */

PIX_FONT	bold_font;
PIX_FONT	roman_font;
PIX_FONT	canvas_font;
PIX_FONT	canv_zoomed_font;
PIX_FONT	button_font;

/* LOCAL */

static int	gc_font[NUMOPS], gc_fontsize[NUMOPS];
static Pixel	gc_color[NUMOPS];
static XRectangle clip[1];
static pr_size	pfx_textwidth();
static int	parsesize();

#define MAXNAMES 35

static struct {
    char	   *fn;
    int		    s;
}		flist[MAXNAMES];

init_font()
{
    struct xfont   *newfont, *nf;
    int		    f, count, i, p, ss;
    char	  **fontlist, **fname;

    if (appres.boldFont == NULL || *appres.boldFont == NULL)
	appres.boldFont = BOLD_FONT;
    if (appres.normalFont == NULL || *appres.normalFont == NULL)
	appres.normalFont = NORMAL_FONT;
    if (appres.buttonFont == NULL || *appres.buttonFont == NULL)
	appres.buttonFont = BUTTON_FONT;

    roman_font = XLoadQueryFont(tool_d, appres.normalFont);
    hidden_text_length = 4 * char_width(roman_font);
    if ((bold_font = XLoadQueryFont(tool_d, appres.boldFont)) == 0) {
	fprintf(stderr, "Can't load font: %s, using %s\n",
		appres.boldFont, appres.normalFont);
	bold_font = XLoadQueryFont(tool_d, appres.normalFont);
    }
    if ((button_font = XLoadQueryFont(tool_d, appres.buttonFont)) == 0) {
	fprintf(stderr, "Can't load font: %s, using %s\n",
		appres.buttonFont, appres.normalFont);
	button_font = XLoadQueryFont(tool_d, appres.normalFont);
    }
    /*
     * Now initialize the font structure for the X fonts corresponding to the
     * Postscript fonts for the canvas.	 OpenWindows can use any LaserWriter
     * fonts at any size, so we don't need to load anything if we are using
     * it.
     */

#ifndef OPENWIN
    if (!appres.SCALABLEFONTS) {/* X11R5 has scalable fonts */
	for (f = 0; f < NUM_X_FONTS; f++) {
	    nf = NULL;
	    if (fontlist = XListFonts(tool_d, x_fontinfo[f].template, MAXNAMES, &count)) {
		fname = fontlist;	/* go through the list finding point
					 * sizes */
		p = 0;
		while (count--) {
		    ss = parsesize(*fname);	/* get the point size from
						 * the name */
		    flist[p].fn = *fname++;	/* save name of this size
						 * font */
		    flist[p++].s = ss;	/* and save size */
		}
		for (ss = 4; ss <= 50; ss++) {
		    for (i = 0; i < p; i++)
			if (flist[i].s == ss)
			    break;
		    if (i < p && flist[i].s == ss) {
			newfont = (struct xfont *) malloc(sizeof(struct xfont));
			if (nf == NULL)
			    x_fontinfo[f].xfontlist = newfont;
			else
			    nf->next = newfont;
			nf = newfont;	/* keep current ptr */
			nf->size = ss;	/* store the size here */
			nf->fid = NULL; /* haven't loaded the font yet */
			nf->fstruct = NULL;	/* ditto */
			nf->fname = flist[i].fn;	/* keep actual name */
			nf->next = NULL;
		    }
		}
	    }
	}
    }				/* next font, f */
#endif				/* OPENWIN */
}

/* parse the point size of font 'name' */
/* e.g. -adobe-courier-bold-o-normal--10-100-75-75-m-60-iso8859-1 */

static int
parsesize(name)
    char	   *name;
{
    int		    s;
    char	   *np;

    for (np = name; *(np + 1); np++)
	if (*np == '-' && *(np + 1) == '-')	/* look for the -- */
	    break;
    s = 0;
    if (*(np + 1)) {
	np += 2;		/* point past the -- */
	s = atoi(np);		/* get the point size */
    } else
	fprintf(stderr, "Can't parse '%s'\n", name);
    return s;
}

/*
 * Lookup an X font corresponding to a Postscript font style that is close in
 * size
 */

PIX_FONT
lookfont(f, s)
    int		    f, s;
{
    struct xfont   *xf;
    XFontStruct	   *fontst;

#ifdef OPENWIN
    /* to search for OpenWindows font - see below */
    char	    fn[128];
    int		    i;

#endif

    if (f == DEFAULT)
	return roman_font;	/* pass back the -normal font font */
    if (s < 0)
	s = DEF_FONTSIZE;	/* default font size */

#ifdef OPENWIN

    for (i = 1; i < NUM_PS_FONTS + 1; i++)
	if (ps_fontinfo[i].xfontnum == f)
	    {
	    sprintf(fn, "%s-%d", ps_fontinfo[i].name, s);
	    break;
	    }

    for (i = strlen(fn) - 1; i >= 0; i--)
	if (isupper(fn[i]))
	    fn[i] = tolower(fn[i]);
    if (appres.DEBUG)
	fprintf(stderr, "Loading font %s\n", fn);
    fontst = XLoadQueryFont(tool_d, fn);
    if (fontst == NULL) {
	fprintf(stderr, "xfig: Can't load font %s ?!, using %s\n",
		fn, appres.normalFont);
	fontst = XLoadQueryFont(tool_d, appres.normalFont);
    }
    return (fontst);

#else
    if (appres.SCALABLEFONTS) { /* scalable fonts! */
	char		fn[128];
	char		template[128];

	strcpy(template, x_fontinfo[f].template);
	template[strlen(template) - 1] = 0;
	strcat(template, "%d-*-*-*-*-*-*-*");
	sprintf(fn, template, s);
	if (appres.DEBUG)
	    fprintf(stderr, "Loading font %s\n", fn);
	fontst = XLoadQueryFont(tool_d, fn);
	if (fontst == NULL) {
	    fprintf(stderr, "xfig: Can't load font %s ?!, using %s\n",
		    fn, appres.normalFont);
	    fontst = XLoadQueryFont(tool_d, appres.normalFont);
	}
	return (fontst);
    } else {				/* prior to X11R5: get best matching font */

	xf = x_fontinfo[f].xfontlist;	/* go through the linked list looking
					 * for match */
	if (xf == NULL)
	    return roman_font;		/* use a default font */
	while (1) {
	    if (s < xf->size)		/* larger point size */
		{
		put_msg("Font size %d not found, using larger %d point",s,xf->size);
		break;
		}
	    if (s == xf->size)		/* exact point size */
		break;
	    if (xf->next != NULL)	/* keep ptr to last if not found */
		xf = xf->next;
	    else
		{
		put_msg("Font size %d not found, using smaller %d point",s,xf->size);
		break;		/* not found, use largest point size in the list */
		}
	}
	if (xf->fid == NULL) {		/* if the font is not yet loaded, load it */
	    if (appres.DEBUG)
		fprintf(stderr, "Loading font %s\n", xf->fname);
	    if (fontst = XLoadQueryFont(tool_d, xf->fname)) {	/* load it */
		xf->fid = fontst->fid;	/* save the id */
		xf->fstruct = fontst;	/* and the XFontStruct ptr */
	    } else {
		fprintf(stderr, "xfig: Can't load font %s ?!, using %s\n",
			xf->fname, appres.normalFont);
		fontst = XLoadQueryFont(tool_d, appres.normalFont);
		xf->fid = fontst->fid;	/* save the id */
		xf->fstruct = fontst;	/* and the XFontStruct ptr */
	    }
	}
	return (xf->fstruct);
    }

#endif				/* !OPENWIN */

}

/* print "string" in window "w" using font number "font" and size "size" */

pw_text(w, x, y, op, font, psflag, size, string, color)
    Window	    w;
    int		    x, y, op, font, psflag, size;
    char	   *string;
    Color	    color;
{
    pwx_text(w, x, y, op, x_fontnum(psflag, font), size * zoomscale, string,
	     color);
}

pwx_text(w, x, y, op, font, size, string, color)
    Window	    w;
    int		    x, y, op, font, size;
    char	   *string;
    Color	    color;
{
    if (gc_font[op] != font || gc_fontsize[op] != size ||
	gc_color[op] != x_color(color)) {
	if (op == PAINT) {
	    set_x_color(gccache[op], color);
	    gc_color[op] = x_color(color);
	}
	canv_zoomed_font = lookfont(font, size);
	XSetFont(tool_d, gccache[op], canv_zoomed_font->fid);
	gc_font[op] = font;
	gc_fontsize[op] = size;
    }
    zXDrawString(tool_d, w, gccache[op], x, y, string, strlen(string));
}

pr_size
pf_textwidth(font, psflag, size, n, s)
    int		    font, psflag, size, n;
    char	   *s;
{
    return pfx_textwidth(x_fontnum(psflag, font), size, n, s);
}

static		pr_size
pfx_textwidth(font, size, n, s)
    int		    font, size, n;
    char	   *s;
{
    int		    dummy;
    XCharStruct	    ch;
    pr_size	    ret;

    canvas_font = lookfont(font, size); /* make sure it is the right font */
    XTextExtents(canvas_font, s, n, &dummy, &dummy, &dummy, &ch);
    ret.x = ch.width;
    ret.y = (ch.ascent + ch.descent);
    return (ret);
}

/* LINES */

static int	gc_thickness[NUMOPS], gc_line_style[NUMOPS];

static		GC
makegc(op, fg, bg)
    int		    op;
    Pixel	    fg;
    Pixel	    bg;
{
    register GC	    ngc;
    XGCValues	    gcv;
    unsigned long   gcmask;

    gcv.font = roman_font->fid;
    gcv.join_style = JoinMiter;
    gcmask = GCJoinStyle | GCFunction | GCForeground | GCBackground | GCFont;
    switch (op) {
    case PAINT:
	gcv.foreground = fg;
	gcv.background = bg;
	gcv.function = GXcopy;
	break;
    case ERASE:
	gcv.foreground = bg;
	gcv.background = bg;
	gcv.function = GXcopy;
	break;
    case INV_PAINT:
	gcv.foreground = fg ^ bg;
	gcv.background = bg;
	gcv.function = GXxor;
	break;
    case MERGE:
	gcv.foreground = fg;
	gcv.background = bg;
	gcv.function = GXor;
	break;
    }

    ngc = XCreateGC(tool_d, XtWindow(canvas_sw), gcmask, &gcv);
    XCopyGC(tool_d, gc, ~(gcmask), ngc);	/* add main gc's values to
						 * the new one */
    return (ngc);
}

init_gc()
{
    int		    i;

    gccache[PAINT] = makegc(PAINT, x_fg_color.pixel, x_bg_color.pixel);
    gccache[ERASE] = makegc(ERASE, x_fg_color.pixel, x_bg_color.pixel);
    gccache[INV_PAINT] = makegc(INV_PAINT, x_fg_color.pixel, x_bg_color.pixel);
    gccache[MERGE] = makegc(MERGE, x_fg_color.pixel, x_bg_color.pixel);

    for (i = 0; i < NUMOPS; i++) {
	gc_font[i] = -1;
	gc_color[i] = -1;
	gc_fontsize[i] = 0;
	gc_thickness[i] = -1;
	gc_line_style[i] = -1;
    }
}

/* create the gc's for fill style (PAINT and ERASE) */
/* the fill_pm[] and unfill_pm[] must already be created */

init_fill_gc()
{
    XGCValues	    gcv;
    int		    i;

    gcv.fill_style = FillOpaqueStippled;
    gcv.arc_mode = ArcPieSlice; /* fill mode for arcs */
    gcv.fill_rule = EvenOddRule /* WindingRule */ ;
    for (i = 0; i < NUMFILLPATS; i++) {
	/* make color fill pattern with black bg (fg is set later in set_x_color() */
	fill_gc[i] = makegc(PAINT, x_fg_color.pixel, x_color(BLACK));
	un_fill_gc[i] = makegc(ERASE, x_fg_color.pixel, x_color(BLACK));
	/* make black fill pattern with default background */
	black_fill_gc[i] = makegc(PAINT, x_fg_color.pixel, x_bg_color.pixel);
	black_un_fill_gc[i] = makegc(ERASE, x_fg_color.pixel, x_bg_color.pixel);
	gcv.stipple = fill_pm[i];
	XChangeGC(tool_d, fill_gc[i],
		  GCStipple | GCFillStyle | GCFillRule | GCArcMode, &gcv);
	XChangeGC(tool_d, black_fill_gc[i],
		  GCStipple | GCFillStyle | GCFillRule | GCArcMode, &gcv);
	XChangeGC(tool_d, un_fill_gc[i],
		  GCStipple | GCFillStyle | GCArcMode, &gcv);
	XChangeGC(tool_d, black_un_fill_gc[i],
		  GCStipple | GCFillStyle | GCArcMode, &gcv);
    }
}

/* SHADING */

/* grey images for fill patterns */

static char	fill_images[NUMFILLPATS][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x40, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00},
    {0x00, 0x40, 0x00, 0x10, 0x00, 0x04, 0x00, 0x00},
    {0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00},
    {0x00, 0x44, 0x00, 0x10, 0x00, 0x44, 0x00, 0x00},
    {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00},
    {0x88, 0x00, 0x32, 0x00, 0x88, 0x00, 0x23, 0x00},
    {0xc4, 0x00, 0x31, 0x00, 0x8c, 0x00, 0x23, 0x00},
    {0x99, 0x00, 0xcc, 0x00, 0x66, 0x00, 0x33, 0x00},
    {0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00},
    {0x55, 0x22, 0x55, 0x08, 0x55, 0x22, 0x55, 0x00},
    {0x55, 0xa2, 0x55, 0x88, 0x55, 0xa2, 0x55, 0x2a},
    {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa},
    {0x55, 0xaa, 0x77, 0xaa, 0x55, 0xaa, 0xdd, 0xaa},
    {0x55, 0xae, 0x5f, 0xae, 0x55, 0xea, 0xf5, 0xea},
    {0x5d, 0xb6, 0x7f, 0xd5, 0x7f, 0xb6, 0x5d, 0xaa},
    {0x77, 0xff, 0x77, 0xaa, 0x77, 0xff, 0x77, 0xaa},
    {0x77, 0xdd, 0x7f, 0xbe, 0x7f, 0xdd, 0x77, 0xaa},
    {0x77, 0xff, 0x7f, 0xbe, 0x7f, 0xff, 0x77, 0xaa},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

/* generate the fill pixmaps */

init_fill_pm()
{
    int		    i;
    register	    j, k;
    Pixmap	    tmpPM,tmpblackPM;

    for (i = 0; i < NUMFILLPATS + 1; i++) {
	fillstyle_choices[i].value = i;
	fillstyle_choices[i].icon = &none_ic;
    }
    /* use same colors for "NONE" indicator for black and color */
    fillstyle_choices[0].normalPM = 
	fillstyle_choices[0].blackPM = XCreatePixmapFromBitmapData(tool_d,
			XtWindow(ind_panel), none_ic.data, none_ic.width,
			none_ic.height, x_fg_color.pixel, x_bg_color.pixel,
			DefaultDepthOfScreen(tool_s));

    for (i = 0; i < NUMFILLPATS; i++) {
	fill_pm[i] = XCreateBitmapFromData(tool_d, XtWindow(canvas_sw),
					   fill_images[i], 8, 8);
	/* create fill style pixmaps for indicator button */
	fillstyle_choices[i + 1].normalPM = XCreatePixmap(tool_d,
		 XtWindow(canvas_sw), 32, 32, DefaultDepthOfScreen(tool_s));
	tmpPM = XCreatePixmapFromBitmapData(tool_d, XtWindow(canvas_sw),
				     fill_images[i], 8, 8, x_bg_color.pixel,
			    x_fg_color.pixel, DefaultDepthOfScreen(tool_s));
	fillstyle_choices[i + 1].blackPM = XCreatePixmap(tool_d,
		 XtWindow(canvas_sw), 32, 32, DefaultDepthOfScreen(tool_s));
	tmpblackPM = XCreatePixmapFromBitmapData(tool_d, XtWindow(canvas_sw),
				     fill_images[i], 8, 8, x_fg_color.pixel,
			    x_bg_color.pixel, DefaultDepthOfScreen(tool_s));
	for (j = 0; j <= 24; j += 8)
	    for (k = 0; k <= 24; k += 8)
		{
		XCopyArea(tool_d, tmpPM, fillstyle_choices[i + 1].normalPM,
			  ind_button_gc, 0, 0, 8, 8, j, k);
		XCopyArea(tool_d, tmpblackPM, fillstyle_choices[i + 1].blackPM,
			  ind_button_gc, 0, 0, 8, 8, j, k);
		}
	XFreePixmap(tool_d, tmpPM);
	XFreePixmap(tool_d, tmpblackPM);
    }
}

pw_vector(w, x1, y1, x2, y2, op, line_width, line_style, style_val, color)
    Window	    w;
    int		    x1, y1, x2, y2, op, line_width, line_style;
    float	    style_val;
    Color	    color;
{
    if (line_width == 0)
	return;
    set_line_stuff(line_width, line_style, style_val, op, color);
    if (line_style == PANEL_LINE)
	XDrawLine(tool_d, w, gccache[op], x1, y1, x2, y2);
    else
	zXDrawLine(tool_d, w, gccache[op], x1, y1, x2, y2);
}

pw_curve(w, xstart, ystart, xend, yend,
	 op, linewidth, style, style_val, fill_style, color)
    Window	    w;
    int		    xstart, ystart, xend, yend;
    int		    op, linewidth, style, fill_style;
    float	    style_val;
    Color	    color;
{
    short	    xmin, ymin;
    unsigned short  wd, ht;

    xmin = (short) min2(xstart, xend);
    ymin = (short) min2(ystart, yend);
    wd = (unsigned short) abs(xstart - xend);
    ht = (unsigned short) abs(ystart - yend);

    /* if it's a fill pat we know about */
    if (fill_style >= 1 && fill_style <= NUMFILLPATS) {
	set_fillgc(fill_style, op, color);
	zXFillArc(tool_d, w, fillgc, xmin, ymin, wd, ht, 0, 360 * 64);
    }
    if (linewidth == 0)
	return;
    if (op == ERASE) {
	/* kludge - to speed things up we erase with thick solid lines */
	set_line_stuff(linewidth + 3, SOLID_LINE, 0.0, op, color);	/* +2 or +3 ok */
	zXDrawArc(tool_d, w, gccache[op], xmin, ymin, wd, ht, 0, 360 * 64);
    } else {
	set_line_stuff(linewidth, style, style_val, op, color);
	zXDrawArc(tool_d, w, gccache[op], xmin, ymin, wd, ht, 0, 360 * 64);
    }
}

pw_point(w, x, y, line_width, op, color)
    Window	    w;
    int		    x, y;
    int		    op, line_width;
    Color	    color;
{
    /* pw_point doesn't use line_style or fill_style - maybe not needed */
    /* (needs color though - hns) */
    set_line_stuff(line_width, SOLID_LINE, 0.0, op, color);
    zXDrawPoint(tool_d, w, gccache[op], x, y);
}

pw_arcbox(w, xmin, ymin, xmax, ymax, radius, op,
	  line_width, line_style, style_val, fill_style, color)
    Window	    w;
    int		    xmin, ymin, xmax, ymax, radius;
    int		    op, line_width, line_style, fill_style;
    float	    style_val;
    Color	    color;
{
    GC		    gc;
    int		    diam = 2 * radius;

    /* if it's a fill pat we know about */
    if (fill_style >= 1 && fill_style <= NUMFILLPATS) {
	set_fillgc(fill_style, op, color);
	/* upper left */
	zXFillArc(tool_d, w, fillgc, xmin, ymin, diam, diam, 90 * 64, 90 * 64);
	/* lower left */
	zXFillArc(tool_d, w, fillgc, xmin, ymax - diam, diam, diam,
		  180 * 64, 90 * 64);
	/* lower right */
	zXFillArc(tool_d, w, fillgc, xmax - diam, ymax - diam, diam, diam,
		  270 * 64, 90 * 64);
	/* upper right */
	zXFillArc(tool_d, w, fillgc, xmax - diam, ymin, diam, diam,
		  0 * 64, 90 * 64);
	/* fill strip on left side between upper and lower arcs */
	if (ymax - ymin - diam > 0)
	    zXFillRectangle(tool_d, w, fillgc, xmin, ymin + radius, radius,
			    ymax - ymin - diam);
	/* fill middle section */
	if (xmax - xmin - diam > 0)
	    zXFillRectangle(tool_d, w, fillgc, xmin + radius, ymin,
			    xmax - xmin - diam, ymax - ymin);
	/* fill strip on right side between upper and lower arcs */
	if (ymax - ymin - diam > 0)
	    zXFillRectangle(tool_d, w, fillgc, xmax - radius, ymin + radius,
			    radius, ymax - ymin - diam);
    }
    if (line_width == 0)
	return;

    set_line_stuff(line_width, line_style, style_val, op, color);
    gc = gccache[op];
    zXDrawArc(tool_d, w, gc, xmin, ymin, diam, diam, 90 * 64, 90 * 64);
    zXDrawLine(tool_d, w, gc, xmin, ymin + radius, xmin, ymax - radius + 1);
    zXDrawArc(tool_d, w, gc, xmin, ymax - diam, diam, diam, 180 * 64, 90 * 64);
    zXDrawLine(tool_d, w, gc, xmin + radius, ymax, xmax - radius + 1, ymax);
    zXDrawArc(tool_d, w, gc, xmax - diam, ymax - diam,
	      diam, diam, 270 * 64, 90 * 64);
    zXDrawLine(tool_d, w, gc, xmax, ymax - radius, xmax, ymin + radius - 1);
    zXDrawArc(tool_d, w, gc, xmax - diam, ymin, diam, diam, 0 * 64, 90 * 64);
    zXDrawLine(tool_d, w, gc, xmax - radius, ymin, xmin + radius - 1, ymin);
}

pw_lines(w, points, npoints, op, line_width, line_style, style_val, fill_style, color)
    Window	    w;
    int		    npoints;
    XPoint	   *points;
    int		    op, line_width, line_style, fill_style;
    float	    style_val;
    Color	    color;
{
    /* if it's a fill pat we know about */
    if (fill_style >= 1 && fill_style <= NUMFILLPATS) {
	set_fillgc(fill_style, op, color);
	if (line_style == PANEL_LINE)
	    XFillPolygon(tool_d, w, fillgc, points, npoints,
			 Complex, CoordModeOrigin);
	else
	    zXFillPolygon(tool_d, w, fillgc, points, npoints,
			  Complex, CoordModeOrigin);
    }
    if (line_width == 0)
	return;
    set_line_stuff(line_width, line_style, style_val, op, color);
    if (line_style == PANEL_LINE)
	XDrawLines(tool_d, w, gccache[op], points, npoints, CoordModeOrigin);
    else
	zXDrawLines(tool_d, w, gccache[op], points, npoints, CoordModeOrigin);
}

set_clip_window(xmin, ymin, xmax, ymax)
    int		    xmin, ymin, xmax, ymax;
{
    clip_xmin = clip[0].x = xmin;
    clip_ymin = clip[0].y = ymin;
    clip_xmax = xmax;
    clip_ymax = ymax;
    clip_width = clip[0].width = xmax - xmin;
    clip_height = clip[0].height = ymax - ymin;
    XSetClipRectangles(tool_d, gccache[PAINT], 0, 0, clip, 1, YXBanded);
    XSetClipRectangles(tool_d, gccache[INV_PAINT], 0, 0, clip, 1, YXBanded);
}

set_zoomed_clip_window(xmin, ymin, xmax, ymax)
    int		    xmin, ymin, xmax, ymax;
{
    set_clip_window(ZOOMX(xmin), ZOOMY(ymin), ZOOMX(xmax), ZOOMY(ymax));
}

reset_clip_window()
{
    set_clip_window(0, 0, CANVAS_WD, CANVAS_HT);
}

set_fillgc(fill_style, op, color)
    int		    fill_style;
    int		    op;
    Color	    color;
{
    if (op == PAINT) {
	fillgc = ((color==BLACK || 
	     (color==DEFAULT_COLOR && x_fg_color.pixel==appres.color[BLACK]) ||
	     (!all_colors_available && color!=WHITE))? 
		black_fill_gc[fill_style - 1]: fill_gc[fill_style - 1]);
	set_x_color(fillgc, color);
    } else
	fillgc = ((color==BLACK || 
	     (color==DEFAULT_COLOR && x_fg_color.pixel==appres.color[BLACK]) ||
	     (!all_colors_available && color!=WHITE))? 
		black_un_fill_gc[fill_style - 1]: un_fill_gc[fill_style - 1]);
    XSetClipRectangles(tool_d, fillgc, 0, 0, clip, 1, YXBanded);
}

set_line_stuff(width, style, style_val, op, color)
    int		    width, style, op;
    float	    style_val;
    Color	    color;
{
    XGCValues	    gcv;
    unsigned long   mask;
    static char	    dash_list[2] = {-1, -1};

    switch (style) {
    case RUBBER_LINE:
	width = 0;
	break;
    case PANEL_LINE:
	break;
    default:
	width = zoomscale * width;
	break;
    }

    /* user zero-width lines for speed with SOLID lines */
    /* can't do this for dashed lines because server isn't */
    /* required to draw dashes for zero-width lines */
    if (width == 1 && style == SOLID_LINE)
	width = 0;

    /* see if all gc stuff is already correct */

    if (width == gc_thickness[op] && style == gc_line_style[op] &&
	x_color(color) == gc_color[op] &&
	(style != DASH_LINE && style != DOTTED_LINE ||
	 dash_list[1] == (char) style_val * zoomscale))
	return;			/* no need to change anything */

    gcv.line_width = width;
    mask = GCLineWidth | GCLineStyle | GCCapStyle;
    if (op == PAINT)
	mask |= GCForeground;
    gcv.line_style = (style == DASH_LINE || style == DOTTED_LINE) ?
	LineOnOffDash : LineSolid;
    gcv.cap_style = (style == DOTTED_LINE) ? CapRound : CapButt;
    gcv.foreground = x_color(color);

    XChangeGC(tool_d, gccache[op], mask, &gcv);
    if (style == DASH_LINE || style == DOTTED_LINE) {
	if (style_val > 0.0) {	/* style_val of 0.0 causes problems */
	    /* length of ON/OFF pixels */
	    dash_list[0] = dash_list[1] = (char) style_val *zoomscale;

	    if (style == DOTTED_LINE)
		dash_list[0] = 1;	/* length of ON pixels for dotted */
	    XSetDashes(tool_d, gccache[op], 0, dash_list, 2);
	}
    }
    gc_thickness[op] = width;
    gc_line_style[op] = style;
    gc_color[op] = x_color(color);
}

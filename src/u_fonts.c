/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1989-2007 by Brian V. Smith
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

#include "fig.h"

#include <X11/Xft/Xft.h>

#include "resources.h"
#include "u_fonts.h"
#include "object.h"
#include "w_msgpanel.h"		/* file_msg() */
#include "w_setup.h"		/* DISPLAY_PIX_PER_INCH */

#define DEF_PS_FONT		0

/* X11 font names */

struct _xfstruct x_fontinfo[NUM_FONTS] = {
    {"-*-times-medium-r-normal--", (struct xfont*) NULL},
    {"-*-times-medium-i-normal--", (struct xfont*) NULL},
    {"-*-times-bold-r-normal--", (struct xfont*) NULL},
    {"-*-times-bold-i-normal--", (struct xfont*) NULL},
    {"-*-avantgarde-book-r-normal--", (struct xfont*) NULL},
    {"-*-avantgarde-book-o-normal--", (struct xfont*) NULL},
    {"-*-avantgarde-demi-r-normal--", (struct xfont*) NULL},
    {"-*-avantgarde-demi-o-normal--", (struct xfont*) NULL},
    {"-*-bookman-light-r-normal--", (struct xfont*) NULL},
    {"-*-bookman-light-i-normal--", (struct xfont*) NULL},
    {"-*-bookman-demi-r-normal--", (struct xfont*) NULL},
    {"-*-bookman-demi-i-normal--", (struct xfont*) NULL},
    {"-*-courier-medium-r-normal--", (struct xfont*) NULL},
    {"-*-courier-medium-o-normal--", (struct xfont*) NULL},
    {"-*-courier-bold-r-normal--", (struct xfont*) NULL},
    {"-*-courier-bold-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-r-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-medium-o-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-narrow--", (struct xfont*) NULL},
    {"-*-new century schoolbook-medium-r-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-medium-i-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-r-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-i-normal--", (struct xfont*) NULL},
    {"-*-palatino-medium-r-normal--", (struct xfont*) NULL},
    {"-*-palatino-medium-i-normal--", (struct xfont*) NULL},
    {"-*-palatino-bold-r-normal--", (struct xfont*) NULL},
    {"-*-palatino-bold-i-normal--", (struct xfont*) NULL},
    {"-*-symbol-medium-r-normal--", (struct xfont*) NULL},
    {"-*-itc zapf chancery-medium-i-normal--", (struct xfont*) NULL},
    {"-*-itc zapf dingbats-*-*-*--", (struct xfont*) NULL},
};

/* Use the following font names for any font that doesn't exist in the table above.
 * These come with the Open Group X distribution so they should be a common set.
 *
 * The XFontStruct * slot is also used to store a 12 point (or closest size) font
 * structure when needed by draw_text() to scale text down below MIN_FONT_SIZE points.
*/

struct _xfstruct x_backup_fontinfo[NUM_FONTS] = {
    {"-*-times-medium-r-normal--", (struct xfont*) NULL},
    {"-*-times-medium-i-normal--", (struct xfont*) NULL},
    {"-*-times-bold-r-normal--", (struct xfont*) NULL},
    {"-*-times-bold-i-normal--", (struct xfont*) NULL},
    {"-*-lucida-medium-r-normal-sans-", (struct xfont*) NULL}, /* closest to Avant-Garde */
    {"-*-lucida-medium-i-normal-sans-", (struct xfont*) NULL},
    {"-*-lucida-bold-r-normal-sans-", (struct xfont*) NULL},
    {"-*-lucida-bold-i-normal-sans-", (struct xfont*) NULL},
    {"-*-times-medium-r-normal--", (struct xfont*) NULL},      /* closest to Bookman */
    {"-*-times-medium-i-normal--", (struct xfont*) NULL},
    {"-*-times-bold-r-normal--", (struct xfont*) NULL},
    {"-*-times-bold-i-normal--", (struct xfont*) NULL},
    {"-*-courier-medium-r-normal--", (struct xfont*) NULL},
    {"-*-courier-medium-o-normal--", (struct xfont*) NULL},
    {"-*-courier-bold-r-normal--", (struct xfont*) NULL},
    {"-*-courier-bold-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-r-normal--", (struct xfont*) NULL},  /* closest to Helv-nar. */
    {"-*-helvetica-medium-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-medium-r-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-medium-i-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-r-normal--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-i-normal--", (struct xfont*) NULL},
    {"-*-lucidabright-medium-r-normal--", (struct xfont*) NULL},   /* closest to Palatino */
    {"-*-lucidabright-medium-i-normal--", (struct xfont*) NULL},
    {"-*-lucidabright-demibold-r-normal--", (struct xfont*) NULL},
    {"-*-lucidabright-demibold-i-normal--", (struct xfont*) NULL},
    {"-*-symbol-medium-r-normal--", (struct xfont*) NULL},
    {"-*-zapf chancery-medium-i-normal--", (struct xfont*) NULL},
    {"-*-zapf dingbats-*-*-*--", (struct xfont*) NULL},
};

/* PostScript font names matched with X11 font names in x_fontinfo */

struct _fstruct ps_fontinfo[NUM_FONTS + 1] = {
    {"Default",				-1},
    {"Times-Roman",			0},
    {"Times-Italic",			1},
    {"Times-Bold",			2},
    {"Times-BoldItalic",		3},
    {"AvantGarde-Book",			4},
    {"AvantGarde-BookOblique",		5},
    {"AvantGarde-Demi",			6},
    {"AvantGarde-DemiOblique",		7},
    {"Bookman-Light",			8},
    {"Bookman-LightItalic",		9},
    {"Bookman-Demi",			10},
    {"Bookman-DemiItalic",		11},
    {"Courier",				12},
    {"Courier-Oblique",			13},
    {"Courier-Bold",			14},
    {"Courier-BoldOblique",		15},
    {"Helvetica",			16},
    {"Helvetica-Oblique",		17},
    {"Helvetica-Bold",			18},
    {"Helvetica-BoldOblique",		19},
    {"Helvetica-Narrow",		20},
    {"Helvetica-Narrow-Oblique",	21},
    {"Helvetica-Narrow-Bold",		22},
    {"Helvetica-Narrow-BoldOblique",	23},
    {"NewCenturySchlbk-Roman",		24},
    {"NewCenturySchlbk-Italic",		25},
    {"NewCenturySchlbk-Bold",		26},
    {"NewCenturySchlbk-BoldItalic",	27},
    {"Palatino-Roman",			28},
    {"Palatino-Italic",			29},
    {"Palatino-Bold",			30},
    {"Palatino-BoldItalic",		31},
    {"Symbol",				32},
    {"ZapfChancery-MediumItalic",	33},
    {"ZapfDingbats",			34},
};

/*
 * Xft font names
 * These are really the free-form fontconfig patterns.
 * The "Adobe..."-names usually select an X bitmap font.
 * Fontconfig seems to ignore space or uppercase. Nevertheless, for
 * readability, some uppercase letters are retained.
 * Microsoft names, viz., Times New Roman, Courier New, Arial, are
 * probably selected by fontconfig, thus do not mention them explicitly.
 */
const char *const xft_name[NUM_FONTS] = {
	"times,AdobeTimes,serif:roman",		/* Times-Roman */
	"times,AdobeTimes,serif:italic",	/* Times-Italic */
	"times,AdobeTimes,serif:bold",		/* Times-Bold */
	"times,AdobeTimes,serif:bold:italic",	/* Times-BoldItalic */
	"avantgarde:book",			/* AvantGarde-Book */
	"avantgarde:book:oblique",		/* AvantGarde-BookOblique */
	"avantgarde:demibold",			/* AvantGarde-Demi */
	"avantgarde:demibold:oblique",		/* AvantGarde-DemiOblique */
	"bookman:light",			/* Bookman-Light */
	"bookman:light:italic",			/* Bookman-LightItalic */
	"bookman:demibold",			/* Bookman-Demi */
	"bookman:demibold:italic",		/* Bookman-DemiItalic */
	"courier,AdobeCourier,monospace",	/* Courier */
	"courier,AdobeCourier,monospace:oblique",	/* Courier-Oblique */
	"courier,AdobeCourier,monospace:bold",		/* Courier-Bold */
	"courier,AdobeCourier,monospace:bold:oblique",	/* Courier-BoldOblique*/
	"helvetica,sans",			/* Helvetica */
	"helvetica,sans:oblique",		/* Helvetica-Oblique */
	"helvetica,sans:bold",			/* Helvetica-Bold */
	"helvetica,sans:bold:oblique",		/* Helvetica-BoldOblique */
	"helveticanarrow,sans:semicondensed",	/* Helvetica-Narrow */
	"helveticanarrow,sans:semicondensed:oblique",
					/* Helvetica-Narrow-Oblique */
	"helveticanarrow,sans:semicondensed:bold",
					/* Helvetica-Narrow-Bold */
	"helveticanarrow,sans:semicondensed:bold:oblique",
					/* Helvetica-Narrow-BoldOblique */
	"newcenturyschoolbook,AdobeNewCenturySchoolbook",
					/* NewCenturySchlbk-Roman */
	"newcenturyschoolbook,AdobeNewCenturySchoolbook:italic",
					/* NewCenturySchlbk-Italic */
	"newcenturyschoolbook,AdobeNewCenturySchoolbook:bold",
					/* NewCenturySchlbk-Bold */
	"newcenturyschoolbook,AdobeNewCenturySchoolbook:bold:italic",
					/* NewCenturySchlbk-BoldItalic */
	"palatino",				/* Palatino-Roman */
	"palatino:italic",			/* Palatino-Italic */
	"palatino:bold",			/* Palatino-Bold */
	"palatino.bold:italic",			/* Palatino-BoldItalic */
	"symbol",				/* Symbol */
	"zapfchancery:medium:italic",		/* ZapfChancery-MediumItalic */
	"zapfdingbats"				/* ZapfDingbats */
};


/* LaTeX font names and the corresponding PostScript font index into ps_fontinfo */

struct _fstruct latex_fontinfo[NUM_LATEX_FONTS] = {
    {"Default",		0},
    {"Roman",		0},
    {"Bold",		2},
    {"Italic",		1},
    {"Sans Serif",	16},
    {"Typewriter",	12},
};

int x_fontnum(int psflag, int fnum)
{
    int x_font;

    if ((psflag && fnum >= NUM_FONTS) || (!psflag && fnum >= NUM_LATEX_FONTS)) {
	file_msg("Illegal font number, using font 0");
	fnum = 0;
    }
    x_font = (psflag ?  ps_fontinfo[fnum + 1].xfontnum :
			latex_fontinfo[fnum].xfontnum);
    return x_font;
}

int psfontnum(char *font)
{
    int i;

    if (font == NULL)
	return(DEF_PS_FONT);
    for (i = 0; i < NUM_FONTS; ++i)	/* Do not start with Zapf Dingbats */
	if (strcasecmp(ps_fontinfo[i].name, font) == 0)
		return (i-1);
    return(DEF_PS_FONT);
}

int latexfontnum(char *font)
{
    int i;

    if (font == NULL)
	return(DEF_LATEX_FONT);
    for (i=0; i<NUM_LATEX_FONTS; i++)
	if (strcasecmp(latex_fontinfo[i].name, font) == 0)
		return (i);
    return(DEF_LATEX_FONT);
}

void
closefont(XftFont *font)
{
	XftFontClose(tool_d, font);
}

XftFont *
getfont(int psflag, int fnum, int size3, /* SIZE_FLT times the font size */
		double angle /* must be larger than 0! */)
{
	/*
	 * The base pattern is the maximum common pattern for a given font.
	 * Extend this pattern by the requested size and font angle to
	 * return a specific font face.
	 */
	static XftPattern	*xftbasepattern[NUM_FONTS] = {NULL};
	/* TODO: write a destroybasepatterns() function, put the base patterns
		 into file scope */
	double		pixelsize;
	XftPattern	*want, *have;
	XftResult	res;
	XftFont		*xftfont;

	/* sanitize fnum */
	if (fnum < 0 || psflag && fnum >= NUM_FONTS ||
			!psflag && fnum >= NUM_LATEX_FONTS) {
		file_msg("Illegal font number, using default font.");
		fnum = DEF_PS_FONT;
	}
	if (!psflag)
		fnum = latex_fontinfo[fnum].xfontnum;

	/* assign the base pattern */
	if (xftbasepattern[fnum] == NULL) {
		xftbasepattern[fnum] = XftNameParse(xft_name[fnum]);
		/* XftPatternAddBool returns 1, if succesful */
		/* XftPatternAddBool(xftbasepattern[fnum], "hinting", False); */
	}

	want = XftPatternDuplicate(xftbasepattern[fnum]);

	/* add the actual pixel size and matrix transformation */
	pixelsize = size3 * DISPLAY_PIX_PER_INCH /
		(SIZE_FLT * (appres.correct_font_size ? 72.0 : 80.0));
	XftPatternAddDouble(want, XFT_PIXEL_SIZE, pixelsize);

	/* Rotated text - negative angle not allowed! */
if (angle < 0.) {
fputs("Negative angle passed to getfont().\n", stderr); exit(1);
}
	if (angle > 0.01) {
		const double	cosa = cos(angle);
		const double	sina = sin(angle);
		const XftMatrix	mat = {cosa, -sina, sina, cosa};
		XftPatternAddMatrix(want, XFT_MATRIX, &mat);
	}

	/*
	 * man xft(3) says, "XftFonts are internally allocated,
	 * reference-counted, and freed by Xft;"
	 * No need to create a self-made cache.
	 */
	have = XftFontMatch(tool_d, tool_sn, want, &res);
	if (appres.DEBUG) {
		char	buf[BUFSIZ];
		XftNameUnparse(xftbasepattern[fnum], buf, BUFSIZ);
		fprintf(stderr, "Font request: %s\nresult: %s\n",
				xft_name[fnum], buf);
	}

	if (res == XftResultMatch) {
		xftfont = XftFontOpenPattern(tool_d, have);
		/*
		 * Do not destroy "have". The xft-tutorial says,
		 * "The returned XftFont contains a reference to the passed
		 * pattern, this pattern will be destroyed when the XftFont is
		 * passed to XftFontClose."
		 */
		XftPatternDestroy(want);

	} else if (fnum != DEF_PS_FONT)
		xftfont = getfont(1 /*psflag*/, DEF_PS_FONT, size3, angle);
	else {
		/* why should this find a result, if XftFontMatch() fails? */
		fprintf(stderr, "trying XftFontOpenPattern!\n");
		xftfont = XftFontOpenPattern(tool_d, want);
		XftPatternDestroy(want);
	}

	return xftfont;
}

/*
 * Compute the horizontal bounding box bb[2] and the four corners of the
 * rectangle that bound the text rotbb[4] with respect to the drawing origin
 * (base_x, base_y).
 * In addition, return the vector "offset" from the drawing origin to the
 * drawing origin of the glyph following this string.
 * Also, return the length and the height of the text.
 * Initially, the small distance from the left edge of the bounding box to the
 * origin of the drawing, (base_x, base_y), was computed. For instance, if a "N"
 * is printed with New Century Schoolbook, the serif of N extends a little bit
 * to the left of base_x. Conversely, the letter "u" in Helvetica is drawn a bit
 * to the right of base_x. Hence, the left edge of the bounding box, for
 * horizontal text, does not coincide with base_x. However, the postscript files
 * or pdf-files produced with the pict2e or tikz output drivers of fig2dev also
 * let the text begin in the same fashion. Therefore, the positioning of the
 * text should not be corrected by passing a slightly corrected position (base_x
 * + epsilon_x, base_y + epsilon_y) to XftDrawStringXX()
 */
void
textextents(F_text *t)
{
	XGlyphInfo	extents;
	XftFont		*rotfont;
	int		len = (int)strlen(t->cstring);

	/* shortcut, nothing to do for an empty string */
	if (len == 0) {
		t->length = t->height = 0;
		t->offset.x = t->offset.y = 0;
		t->bb[0].x = t->bb[0].y = 0;
		t->bb[1].x = t->bb[1].y = 0;
		t->rotbb[0].x = t->rotbb[0].y = 0;
		t->rotbb[1].x = t->rotbb[1].y = 0;
		t->rotbb[2].x = t->rotbb[2].y = 0;
		t->rotbb[3].x = t->rotbb[3].y = 0;
		return;
	}

	/* Get the font at native Fig resolution (often, 1200 ppi) */
	rotfont = getfont(psfont_text(t), t->font,
			t->size * SIZE_FLT * ZOOM_FACTOR, (double)t->angle);
	XftTextExtentsUtf8(tool_d, rotfont, (XftChar8 *)t->cstring, len,
			&extents);
	/* libxft keeps the last 16, closed fonts in cache.
	   Hence, no need to keep rotfont open, in our own cache. */
	XftFontClose(tool_d, rotfont);

	t->bb[0].x = 0 - extents.x;
	t->bb[0].y = 0 - extents.y;
	t->bb[1].x = extents.width - extents.x;
	t->bb[1].y = extents.height - extents.y;

	t->offset.x = extents.xOff;
	t->offset.y = extents.yOff;

	/* shortcut for horizontal and vertical texts */
	if (extents.xOff == 0) {
		/* vertical text */

		/* possibly an empty string? */
		if (extents.yOff == 0) {
			t->length = t->height = 0;
			t->offset.x = t->offset.y = 0;
			t->bb[0].x = t->bb[0].y = 0;
			t->bb[1].x = t->bb[1].y = 0;
			t->rotbb[0].x = t->rotbb[0].y = 0;
			t->rotbb[1].x = t->rotbb[1].y = 0;
			t->rotbb[2].x = t->rotbb[2].y = 0;
			t->rotbb[3].x = t->rotbb[3].y = 0;
			return;
		}

		t->length = extents.height;
		t->height = extents.width;
		/*
		 * Initially, the corners of the rotated rectangle were sorted.
		 * This is unnecessary, but the code was kept.
		 */
		if (extents.yOff > 0) {
			/* rotbb[0] is top left, rotbb[1] bottom left,... */
			t->rotbb[0].x = t->bb[1].x;  t->rotbb[0].y = t->bb[0].y;
			t->rotbb[1].x = t->bb[0].x;  t->rotbb[1].y = t->bb[0].y;
			t->rotbb[2].x = t->bb[0].x;  t->rotbb[2].y = t->bb[1].y;
			t->rotbb[3].x = t->bb[1].x;  t->rotbb[3].y = t->bb[1].y;
			//origin->x = 0;		origin->y = extents.y;
		} else { /* extents.yOff < 0 */
			t->rotbb[0].x = t->bb[0].x;  t->rotbb[0].y = t->bb[1].y;
			t->rotbb[1].x = t->bb[1].x;  t->rotbb[1].y = t->bb[1].y;
			t->rotbb[2].x = t->bb[1].x;  t->rotbb[2].y = t->bb[0].y;
			t->rotbb[3].x = t->bb[0].x;  t->rotbb[3].y = t->bb[0].y;
			//origin->x = 0;
			//origin->y = extents.y - extents.height;
		}

		t->top.x = t->rotbb[0].x;
		t->top.y = 0;

	} else if (extents.yOff == 0) {
		/* horizontal text */

		t->length = extents.width;
		t->height = extents.height;

		if (extents.xOff > 0) {
			t->rotbb[0].x = t->bb[0].x;  t->rotbb[0].y = t->bb[0].y;
			t->rotbb[1].x = t->bb[0].x;  t->rotbb[1].y = t->bb[1].y;
			t->rotbb[2].x = t->bb[1].x;  t->rotbb[2].y = t->bb[1].y;
			t->rotbb[3].x = t->bb[1].x;  t->rotbb[3].y = t->bb[0].y;
			//origin->x = extents.x;	origin->y = 0;
		} else { /* extents.xOff < 0 */
			t->rotbb[0].x = t->bb[1].x;  t->rotbb[0].y = t->bb[1].y;
			t->rotbb[1].x = t->bb[1].x;  t->rotbb[1].y = t->bb[0].y;
			t->rotbb[2].x = t->bb[0].x;  t->rotbb[2].y = t->bb[0].y;
			t->rotbb[3].x = t->bb[0].x;  t->rotbb[3].y = t->bb[1].y;
			//origin->x = extents.x - extents.width;
			//origin->y = 0;
		}

		t->top.x = 0;
		t->top.y = t->rotbb[0].y;


	} else {
		/* a font at an angle */
		XftFont 	*horfont;
		double		cosa, sina;
		struct f_pos	tl, bl, tr, br;

		horfont = getfont(psfont_text(t), t->font,
					t->size * SIZE_FLT * ZOOM_FACTOR, 0.0);
		XftTextExtentsUtf8(tool_d, horfont, (XftChar8 *)t->cstring, len,
				&extents);
		/* See above, libxft keeps a cache of 16 closed fonts. */
		XftFontClose(tool_d, horfont);

		t->length = extents.width;
		t->height = extents.height;
		tl.x = 0 - extents.x;		tl.y = 0 - extents.y;
		bl.x = 0 - extents.x;		bl.y = t->height - extents.y;
		br.x = t->length - extents.x;	br.y = t->height - extents.y;
		tr.x = t->length - extents.x;	tr.y = 0 - extents.y;
		//origin->x = extents.x;	origin->y = 0;

#define ROTPOS(rot, orig)	rot.x = orig.x * cosa + orig.y * sina; \
				rot.y = -orig.x * sina + orig.y * cosa
		cosa = cos(t->angle);
		sina = sin(t->angle);

		ROTPOS(t->rotbb[0], tl);
		ROTPOS(t->rotbb[1], bl);
		ROTPOS(t->rotbb[2], br);
		ROTPOS(t->rotbb[3], tr);

		/* position of the top text marker */
		tl.x = 0;	/* tl.y is correct */
		ROTPOS(t->top, tl);

#undef ROTPOS
	}
}


/*
 * Return the pixel length of the text.
 */
int
textlength(XftFont *horfont, XftChar8 *string, int len)
{
	XGlyphInfo	extents;

	if (len == 0)
		return 0;

	XftTextExtentsUtf8(tool_d, horfont, string, len, &extents);
	return (int)extents.xOff;
}


/*
 * Return ascent and descent, testing with a few chars that should provide a
 * good approximation to the maximum ascent and descent.
 * This works mainly for western languages, not sure about eastern languages.
 */
void
textmaxheight(int psflag, int font, int size, int *ascent, int *descent)
{
	XGlyphInfo	extents;
	XftFont		*horfont;
	XftChar8	max_height_str[] = "{(fgjOÜ";

	horfont = getfont(psflag, font, size * SIZE_FLT * ZOOM_FACTOR, 0.0);
	XftTextExtentsUtf8(tool_d, horfont, max_height_str,
			(int)sizeof(max_height_str), &extents);
	XftFontClose(tool_d, horfont);
	*ascent = extents.y;
	*descent = extents.height - extents.y;
}


/*
 * Return the drawing origin, given the marker position (base_x, base_y)
 * and the alignment.
 */
void
text_origin(int *draw_x, int *draw_y, int base_x, int base_y, int align,
		F_pos offset)
{
	switch (align) {
	case T_LEFT_JUSTIFIED:
		*draw_x = base_x;
		*draw_y = base_y;
		break;
	case T_CENTER_JUSTIFIED:
		*draw_x = base_x - offset.x/2;
		*draw_y = base_y - offset.y/2;
		break;
	case T_RIGHT_JUSTIFIED:
		*draw_x = base_x - offset.x;
		*draw_y = base_y - offset.y;
		break;
	default:
		file_msg("Incorrect text alignment %d, error in function text_origin().");
		break;
	}
}

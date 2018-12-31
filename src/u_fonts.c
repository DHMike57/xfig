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
		/* Erasing by painting over with the canvas background color
		   does not work with antialiased text */
		XftPatternAddBool(xftbasepattern[fnum], XFT_ANTIALIAS, False);
		/* XftPatternAddBool returns 1, if succesful */
		/* XftPatternAddBool(xftbasepattern[fnum], "hinting", False); */
		if (appres.DEBUG) {
			char	buf[BUFSIZ];
			XftNameUnparse(xftbasepattern[fnum], buf, BUFSIZ);
			fprintf(stderr, "Font request: %s\nresult: %s\n",
					xft_name[fnum], buf);
		}
	}

	want = XftPatternDuplicate(xftbasepattern[fnum]);

	/* add the actual pixel size and matrix transformation */
	pixelsize = size3 * DISPLAY_PIX_PER_INCH /
		(SIZE_FLT * (appres.correct_font_size ? 72.0 : 80.0));
	XftPatternAddDouble(want, XFT_PIXEL_SIZE, pixelsize);

	/* Rotated text - negative angle not allowed! */
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
	if (res == XftResultMatch) {
		xftfont = XftFontOpenPattern(tool_d, have);
		/*
		 * Do not destroy "have". The xft-tutorial says,
		 * "The returned XftFont contains a reference to the passed
		 * pattern, this pattern will be destroyed when the XftFont is
		 * passed to XftFontClose."
		 */
		XftPatternDestroy(want);
		/*
		XGlyphInfo	extents;
		XftTextExtents8(tool_d, xftfont, "Hallo", 5, &extents);
		fprintf(stderr, "Hallo extents: width = %u, height = %u\n"
				"  x = %d, y = %d, xOff = %d, yOff = %d.\n",
				extents.width, extents.height, extents.x,
				extents.y, extents.xOff, extents.yOff);
		*/
		/*
		   Helvetica pixelsize=13.333
Hallo extents: width = 29, height = 10
  x = -1, y = 10, xOff = 30, yOff = 0.
		   Helvetica pixelsize=30
Hallo extents: width = 66, height = 22
  x = -2, y = 22, xOff = 70, yOff = 0.
Hallo extents: width = 29, height = 10
  x = -1, y = 10, xOff = 30, yOff = 0.
  matrix=0,866025 -0,5 0,5 0,866025 (30°)
Hallo extents: width = 67, height = 47
  x = 9, y = 48, xOff = 61, yOff = -37.
		*/
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
void
textextents(XftFont *font, const XftChar8 *string, int len, XGlyphInfo *extents)
{
	XftTextExtentsUtf8(tool_d, font, string, len, extents);
}
*/

void
textextents(XftFont *font, const XftChar8 *string, int len, int base_x, int
		base_y, struct f_pos bb[2], struct f_pos rotbb[4], struct f_pos
		*origin);
{
	/* corners of the rotated rectangle, with respect to the orientation
	   of the text */
	struct f_pos	rottl, rotbl, rotbr, rottr;
	/* bounding box, top left = (0,0) */
	int		xOff, yOff;	/* abs of extents.xOff, extents.yOff */
	XGlyphInfo	extents;

	XftTextExtentsUtf8(tool_d, font, string, len, &extents);

	xOff = abs(extents.xOff);
	yOff = abs(extents.yOff);

	/*
	 * Internally, origin is first used as the vector from the baseline text
	 * marker (base_x, base_y) to the position that is passed to
	 * XftDrawString().
	 */
	/* shortcut for horizontal and vertical texts */
	/* shortcut for height, width == 0 ? */
	if (xOff == 0) {
		if (extents.yOff > 0) {
			rotbl = {0, 0};
			rotbr = {0, extents.height};
			rottr = {extents.width, extents.height};
			rottl = {extents.width, 0};
			*origin = {0, extents.y};
		} else { /* extents.yOff < 0 */
			rotbl = {extents.width, extents.height};
			rotbr = {extents.width, 0};
			rottr = {0, 0};
			rottl = {0, extents.height};
			*origin = {0, extents.y - extents.height};
		}
	} else if (yOff == 0) {
		if (extents.xOff > 0) {
			rottl = {0, 0};
			rotbl = {0, extents.height};
			rotbr = {extents.width, extents.height};
			rottr = {extents.width, 0};
			*origin = {extents.x, 0};
		} else { /* extents.xOff < 0 */
			rottl = {extents.width, extents.height};
			rotbl = {extents.width, 0};
			rotbr = {0, 0};
			rottr = {0, extents.height};
			*origin = {extents.x - extents.width};
		}

	/* for 45°, the algorithm below does not work */
	} else if (xOff == yOff) {
		if (extents.xOff > 0) {
			if (extents.yOff > 0) {
				rottl = {extents.width - xOff, 0};
				rotbl = {0, rottl.x};
				rotbr = {xOff, extents.height};
				rottr = {extents.width, xOff};
			} else { /* extents.yOff < 0 */
				rottl = {0, xOff};
				rotbl = {extents.height,extents.width - xOff};
				rotbr = {extents.width,extents.height - xOff};
				rottr = {xOff, 0};
			}
		} else { /* extents.xOff < 0 */
			if (extents.yOff > 0) {
				rottl = {extents.width,extents.height - xOff};
				rotbl = {xOff, 0};
				rotbr = {0, xOff};
				rottr = {extents.height,extents.width - xOff};
			} else { /* extents.yOff < 0 */
				rottl = {xOff, extents.height};
				rotbl = {extents.width, xOff};
				rotbr = {extents.width - xOff, 0};
				rottr = {0, extents.height - xOff}
			}
		}
	} else {
		double	f1, f2;
		int	x, y;

		f1 = (double)(extents.width * xOff - extents.height * yOff) /
			(xOff*xOff - yOff*yOff);
		/* f2 = (double)(extents.width * yOff - extents.height * xOff)
			/ (yOff*yOff - xOff*xOff);	*/
		/* round a positive number */
		x = (int)(f1 * xOff + 0.5);
		y = (int)(f1 * yOff + 0.5);

		if (extents.xOff > 0) {
			if (extents.yOff > 0) {
				rottl = {extents.width - x, 0};
				rotbl = {0, extents.height - y};
				rotbr = {x, extents.height};
				rottr = {extents.width, y};
			} else { /* extents.yOff < 0 */
				rottr = {x, 0};
				rottl = {0, y};
				rotbl = {extents.width - x, extents.height};
				rotbr = {extents.width, extents.height - y};
			}
		} else { /* extents.xOff < 0 */
			if (extents.yOff > 0) {
				rotbl = {x, 0};
				rotbr = {0, y};
				rottr = {extents.width - x, extents.height};
				rottl = {extents.width, extents.height - y};
			} else { /* extents.yOff < 0 */
				rottl = {x, extents.height};
				rotbl = {extents.width, y};
				rotbr = {extents.width - x, 0};
				rottr = {0, extents.height - y};
			}
		}
	}

	/* Correct origin for centered and right-adjusted text */

	/* Assign the results */

	/* bb[0] is equal to the necessary shift */
	*bb = {base_x + origin->x - extents.x, base_y + origin->y - extents.y};

	bb[1] = {extents.width + *bb.x, extents.height + *bb.y};
	rotbb[0] = {rottl.x + *bb.x, rottl.y + *bb.y};
	rotbb[1] = {rotbl.x + *bb.x, rotbl.y + *bb.y};
	rotbb[2] = {rotbr.x + *bb.x, rotbr.y + *bb.y};
	rotbb[3] = {rottr.x + *bb.x, rottr.y + *bb.y};

	/* Now assign the absolute position of the drawing origin. */
	*origin = {base_x + origin->x, base_y + origin->y};
}

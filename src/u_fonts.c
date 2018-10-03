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

#include <X11/Xft/Xft.h>

#include "fig.h"
#include "resources.h"
#include "u_fonts.h"
#include "object.h"
#include "w_msgpanel.h"

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
    {"Default", -1},
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

/*
 * On first call, the xftbasepattern[] will be set to the result from
 * calling XftNameParse() on the corresponding xft_name above.
 */
XftPattern *
xftbasepattern[NUM_FONTS];

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
    for (i=0; i<NUM_FONTS; i++)
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
getxftfont(int psflag, int fnum, int size)
{
	XftPattern	*want, *have;
	XftResult	res;
	XftFont		*xftfont;
	char	buf[BUFSIZ];	/* DEBUG */

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
		XftNameUnparse(xftbasepattern[fnum], buf, BUFSIZ); /* DEBUG */
		fprintf(stderr,"request %s, result:\n%s\n", xft_name[fnum], buf);

		/* erasing, rotation would not work with antialiased text */
		fprintf(stderr,"result code (0, I presume): %d\n",
		XftPatternAddBool(xftbasepattern[fnum], XFT_ANTIALIAS, False));
		XftPatternAddBool(xftbasepattern[fnum], "hinting", False);
		XftNameUnparse(xftbasepattern[fnum], buf, BUFSIZ); /* DEBUG */
		fprintf(stderr, "add aa, hinting: %s\n", buf);
	}
	want = XftPatternDuplicate(xftbasepattern[fnum]);
	fprintf(stderr, "hallo here\n");
	XftPatternAddDouble(want, XFT_SIZE, (double)size);

	/*
	 * man xft(3) says, "XftFonts are internally allocated,
	 * reference-counted, and freed by Xft;"
	 * No need to create a self-made cache.
	 */

	have = XftFontMatch(tool_d, tool_sn, want, &res);
	XftNameUnparse(have, buf, BUFSIZ); /* DEBUG */
	fprintf(stderr, "chosen font: %s\n", buf);
	if (res == XftResultMatch)
		xftfont = XftFontOpenPattern(tool_d, have);
	else if (fnum != DEF_PS_FONT)
		xftfont = getxftfont(1, DEF_PS_FONT, size);
	else {
		/* why should this find a result, if XftFontMatch() fails? */
		fprintf(stderr, "trying XftFontOpenPattern!\n");
		xftfont = XftFontOpenPattern(tool_d, want);
	}

	XftPatternDestroy(want);
	XftPatternDestroy(have);
	return xftfont;
}

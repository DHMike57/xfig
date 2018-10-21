/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2018 by Thomas Loimer
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and documentation
 * files (the "Software"), including without limitation the rights to use,
 * copy, modify, merge, publish distribute, sublicense and/or sell copies
 * of the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>			/* XColor, Status */

#include "u_colors.h"

#define OPAQUE		0xffff		/* opaque alpha mask */
#define FIRST_GRAY	DARK_GRAY
#define LAST_GRAY	LT_GRAY

Boolean		all_colors_available;
Boolean		colorUsed[MAX_USR_COLS];
Boolean		colorFree[MAX_USR_COLS];
Boolean		n_colorFree[MAX_USR_COLS];
int		num_usr_cols = 0;
int		n_num_usr_cols;
XftColor	color_storage[SPECIAL_COLS + NUM_STD_COLS + MAX_USR_COLS];
/* Convenience pointers into interesting positions of color_storage. */
XftColor	*colors = color_storage + SPECIAL_COLS;
XftColor	*user_colors = color_storage + SPECIAL_COLS + NUM_STD_COLS;
// TODO: eliminate dark_gray_color, med_gray_color, black_color, white_color 
unsigned long	dark_gray_color, med_gray_color, lt_gray_color;
unsigned long	pageborder_color;
unsigned long	axis_lines_color;
unsigned long	but_fg, but_bg;
unsigned long	ind_but_fg, ind_but_bg;
unsigned long	mouse_but_fg, mouse_but_bg;
Color		grid_color;

/* TODO: change to colors[Black], colors[WHITE] */

XftColor	n_user_colors[MAX_USR_COLS];
XftColor	save_colors[MAX_USR_COLS];
XftColor	user_colors[MAX_USR_COLS];
XftColor	undel_user_color;
XColor		x_fg_color, x_bg_color;

/* Number of colors we want to use for pictures. This will be determined
   when the first picture is used. We will take
   min(number_of_free_colorcells, 100, appres.maximagecolors) */
int		avail_image_cols = -1;
/* colormaps used for the pictures */
XColor		image_cells[MAX_COLORMAP_SIZE];

fig_color colorNames[] = {
	{"Default",	"Default",	0,	0,	0},
	{"Black",	"Blk",		0,	0,	0},
	{"Blue",	"Blu",		0x0000, 0x0000, 0xffff},
	{"Green",	"Grn",		0x0000, 0xffff, 0x0000},
	{"Cyan",	"Cyn",		0x0000, 0xffff, 0xffff},
	{"Red",		"Red",		0xffff, 0x0000, 0x0000},
	{"Magenta",	"Mag",		0xffff, 0x0000, 0xffff},
	{"Yellow",	"Yel",		0xffff, 0xffff, 0x0000},
	{"White",	"Wht",		0xffff, 0xffff, 0xffff},
	{"Blue4",	"Bl4",		0x0000, 0x0000, 0x9000},
	{"Blue3",	"Bl3",		0x0000, 0x0000, 0xb000},
	{"Blue2",	"Bl2",		0x0000, 0x0000, 0xd000},
	{"LtBlue",	"LBl",		0x8700, 0xce00, 0xff00},
	{"Green4",	"Gr4",		0x0000, 0x9000, 0x0000},
	{"Green3",	"Gr3",		0x0000, 0xb000, 0x0000},
	{"Green2",	"Gr2",		0x0000, 0xd000, 0x0000},
	{"Cyan4",	"Cn4",		0x0000, 0x9000, 0x9000},
	{"Cyan3",	"Cn3",		0x0000, 0xb000, 0xb000},
	{"Cyan2",	"Cn2",		0x0000, 0xd000, 0xd000},
	{"Red4",	"Rd4",		0x9000, 0x0000, 0x0000},
	{"Red3",	"Rd3",		0xb000, 0x0000, 0x0000},
	{"Red2",	"Rd2",		0xd000, 0x0000, 0x0000},
	{"Magenta4",	"Mg4",		0x9000, 0x0000, 0x9000},
	{"Magenta3",	"Mg3",		0xb000, 0x0000, 0xb000},
	{"Magenta2",	"Mg2",		0xd000, 0x0000, 0xd000},
	{"Brown4",	"Br4",		0x8000, 0x3000, 0x0000},
	{"Brown3",	"Br3",		0xa000, 0x4000, 0x0000},
	{"Brown2",	"Br2",		0xc000, 0x6000, 0x0000},
	{"Pink4",	"Pk4",		0xff00, 0x8000, 0x8000},
	{"Pink3",	"Pk3",		0xff00, 0xa000, 0xa000},
	{"Pink2",	"Pk2",		0xff00, 0xc000, 0xc000},
	{"Pink",	"Pnk",		0xff00, 0xe000, 0xe000},
	{"Gold",	"Gld",		0xff00, 0xd700, 0x0000}
};

/* Given a figcolor, return the corresponding XftColor. */
static void
getxftcolor(fig_color *in, XftColor *out)
{
	XRenderColor	buf;

	buf.red = in->red;
	buf.green = in->green;
	buf.blue = in->blue;
	buf.alpha = OPAQUE;

	XftColorAllocValue(tool_d, tool_v, tool_cm, &buf, out);
}

/*
 * For non-TrueColor visuals, I do not trust XftColorAllocValue().
 * Therefore, write the XftColor struct with the values we get.
 */
static void
writexftcolor(fig_color *in, XftColor *out)
{
	XColor	buf;
	Status	status;

	buf.red = in->red;
	buf.green = in->green;
	buf.blue = in->blue;
	buf.flags = DoRed | DoGreen | DoBlue;
	if (status = XAllocColor(tool_d, tool_cm, &buf)) {
		out->color.red = buf.red;
		out->color.green = buf.green;
		out->color.blue = buf.blue;
		out->color.alpha = OPAQUE;
		out->pixel = buf.pixel;
	}
	return status;
}


/*
 * Allocate standard colors, three gray colors, the pageborder and
 * the axisline color. The latter two might be set from resources.
 * Provide for three cases, (i) TrueColor, (ii) black and white and
 * (iii) legacy XColors, with fallback values if not all colors can
 * be allocated. 
 */
void
check_colors(void)
{
	int		i;
	XColor		x_color;
	figcolors	grays[LAST_GRAY - FIRST_GRAY + 1] = {
				{"", "", 166, 166, 166},	/* gray65 */ 
				{"", "", 204, 204, 204},	/* gray80 */ 
				{"", "", 229, 229, 229}		/* gray90 */ 
			}

	/* initialize user color cells */
	for (i = 0; i < MAX_USR_COLS; ++i) {
		colorFree[i] = True;
		n_colorFree[i] = True;
		num_usr_cols = 0;
	}

	/* if monochrome resource is set, only allocate black and white */
	if (!all_colors_available || appres.monochrome || tool_cells == 2) {

		/* set all colors to black, except white */
		for (i = 0; i < NUM_STD_COLS; ++i)
			if (i == WHITE)
				getxftcolor(&colorNames[i + 1], &colors[i]);
			else
				getxftcolor(&colorNames[BLACK + 1],
						&colors[i]);

		/* set all grays to white */
		for (i = FIRST_GRAY; i <= LAST_GRAY; ++i)
			colors[i] = colors[WHITE];

		pageborder_color = colors[BLACK].pixel;
		axis_lines_color = colors[BLACK].pixel;

		return;
	}

	/*
	 * XftColorAllocValue() seems to return the requested color, not the
	 * color actually allocated by the server (see issue 7 at
	 * https://gitlab...) Therefore, use
	 * XftColorAllocValue() for TrueColor visuals only, otherwise use
	 * XAllocColor etc.
	 */
	if (tool_v == TrueColor) {

		/* Standard colors */
		for (i = 0; i < NUM_STD_COLS; ++i) {
			getxftcolor(&colorNames[i + 1], &colors[i]);
		}
		/* Gray colors */
		for (i = FIRST_GRAY; i <= LAST_GRAY; ++i)
			getxftcolor(&grays[i], &colors[i + FIRST_GRAY]);

	} else { /* !TrueColor */

		/* Allocate the standard colors, otherwise set to black. */
		for (i = 0; i < NUM_STD_COLS; ++i) {
			if (i == WHITE) {
				writexftcolor(&colorNames[i + 1], &colors[i]);
			} else if (!all_colors_available) {
				colors[i] = colors[BLACK];
			} else if (!writexftcolor(&colorNames[i + 1],
						&colors[i])) {
				if (!switch_colormap() || !writexftcolor(
							&colorNames[i + 1],
							&colors[i])) {
#define CMAPERR	"Not enough colormap entries available for basic colors, using monochrome mode.\n"
					fprintf(stderr, CMAPERR);
					all_colors_available = False;
					colors[i] = colors[BLACK];
				}
			}
		}

		for (i = FIRST_GRAY; i <= LAST_GRAY; ++i) {
			if (!all_colors_available) {
				colors[i + FIRST_GRAY] = colors[WHITE];
			} else if (!writexftcolor(&grays[i],
						&colors[i + FIRST_GRAY])) {
				if (!switch_colormap() || !writexftcolor(
							&grays[i], &colors[i +
							FIRST_GRAY])) {
					all_colors_available = False;
					colors[i + FIRST_GRAY] = colors[WHITE];
				}
			}
		}
	}

	/* Continue for both TrueColor and non-TrueColor visuals. */

	/* page border color */
	XParseColor(tool_d, tool_cm, appres.pageborder, &x_color);
	if (XAllocColor(tool_d, tool_cm, &x_color))
		pageborder_color = x_color.pixel;
	else
		pageborder_color = colors[BLACK].pixel;

	/* axis lines color */
	XParseColor(tool_d, tool_cm, appres.axislines, &x_color);
	if (XAllocColor(tool_d, tool_cm, &x_color))
		axis_lines_color = x_color.pixel;
	else
		axis_lines_color = colors[BLACK].pixel;
}

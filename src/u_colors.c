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

#include "u_colors.h"

Boolean		all_colors_available;
Boolean		colorUsed[MAX_USR_COLS];
Boolean		colorFree[MAX_USR_COLS];
Boolean		n_colorFree[MAX_USR_COLS];
int		num_usr_cols = 0;
int		n_num_usr_cols;
unsigned long	axis_lines_color;
unsigned long	colors[NUM_STD_COLS + MAX_USR_COLS];
unsigned long	dark_gray_color, med_gray_color, lt_gray_color;
unsigned long	pageborder_color;
unsigned long	but_fg, but_bg;
unsigned long	ind_but_fg, ind_but_bg;
unsigned long	mouse_but_fg, mouse_but_bg;
Color		grid_color;

/* These are allocated in main() in case we are not using default
   colormap (se we cannot use BlackPixelOfScreen...) */
XColor		black_color, white_color;

XColor		n_user_colors[MAX_USR_COLS];
XColor		save_colors[MAX_USR_COLS];
XColor		user_colors[MAX_USR_COLS];
XColor		undel_user_color;
XColor		x_fg_color, x_bg_color;

/* Number of colors we want to use for pictures. This will be determined
   when the first picture is used. We will take
   min(number_of_free_colorcells, 100, appres.maximagecolors) */
int		avail_image_cols = -1;
/* colormaps used for the pictures */
XColor		image_cells[MAX_COLORMAP_SIZE];

fig_colors colorNames[] = {
	{"Default",	"NULL"},
	{"Black",	"black"},
	{"Blue",	"blue"},
	{"Green",	"green"},
	{"Cyan",	"cyan"},
	{"Red",		"red"},
	{"Magenta",	"magenta"},
	{"Yellow",	"yellow"},
	{"White",	"white"},
	{"Blue4",	"#000090"},	/* NOTE: hex colors must be 6 digits */
	{"Blue3",	"#0000b0"},
	{"Blue2",	"#0000d0"},
	{"LtBlue",	"#87ceff"},
	{"Green4",	"#009000"},
	{"Green3",	"#00b000"},
	{"Green2",	"#00d000"},
	{"Cyan4",	"#009090"},
	{"Cyan3",	"#00b0b0"},
	{"Cyan2",	"#00d0d0"},
	{"Red4",	"#900000"},
	{"Red3",	"#b00000"},
	{"Red2",	"#d00000"},
	{"Magenta4",	"#900090"},
	{"Magenta3",	"#b000b0"},
	{"Magenta2",	"#d000d0"},
	{"Brown4",	"#803000"},
	{"Brown3",	"#a04000"},
	{"Brown2",	"#c06000"},
	{"Pink4",	"#ff8080"},
	{"Pink3",	"#ffa0a0"},
	{"Pink2",	"#ffc0c0"},
	{"Pink",	"#ffe0e0"},
	{"Gold",	"gold" }
};

char	*short_clrNames[] = {
	"Default", "Blk", "Blu", "Grn", "Cyn", "Red", "Mag", "Yel", "Wht",
	"Bl4", "Bl3", "Bl2", "LBl", "Gr4", "Gr3", "Gr2",
	"Cn4", "Cn3", "Cn2", "Rd4", "Rd3", "Rd2",
	"Mg4", "Mg3", "Mg2", "Br4", "Br3", "Br2",
	"Pk4", "Pk3", "Pk2", "Pnk", "Gld"
};

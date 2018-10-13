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

/**
 * @file	u_colors.h
 * @author	Thomas Loimer, 2018
 *
 * @brief	Provide variables and routines related to color.
 *
 */


#ifndef U_COLORS_H
#define U_COLORS_H

#include <X11/Intrinsic.h>		/* Boolean */
#include <X11/Xft/Xft.h>

/* XFT DEBUG START */
/*
//extern XftColor		 xftcolors[NUM_STD_COLS + 1];
extern XftDraw		*main_xftdraw;
extern XftColor		xftwhite;
extern XftFont	*xftsmall;
extern XftFont	*xftbig;
extern XftFont	*xftrot;
*/
/* XFT DEBUG END */

/* Color definition */
#define	Color		int
#define NUM_STD_COLS	32	/**< Number of standard colors. */
#define MAX_USR_COLS	512	/**< Maximum number of user-defined colors. */
#define	MAX_COLORMAP_SIZE	MAX_USR_COLS	//!< for picture files


/* forward declaration */
struct XColor;

struct Cmap {
	unsigned short red, green, blue;
	unsigned long pixel;
};
typedef struct {
	char	*name;
	char	*rgb;
} fig_colors;

/* Globals */
extern Boolean		all_colors_available;
extern Boolean		colorUsed[MAX_USR_COLS];
extern Boolean		colorFree[MAX_USR_COLS];
extern Boolean		n_colorFree[MAX_USR_COLS];
extern char		*short_clrNames[NUM_STD_COLS + 1];
extern int		num_usr_cols, n_num_usr_cols;
extern unsigned long	axis_lines_color;
extern unsigned long	colors[NUM_STD_COLS+MAX_USR_COLS];
extern unsigned long	dark_gray_color, med_gray_color, lt_gray_color;
extern unsigned long	pageborder_color;
extern unsigned long	but_fg, but_bg;
extern unsigned long	ind_but_fg, ind_but_bg;
extern unsigned long	mouse_but_fg, mouse_but_bg;
extern Color		grid_color;
extern XColor		black_color, white_color;
extern XColor		n_user_colors[MAX_USR_COLS];
extern XColor		save_colors[MAX_USR_COLS];
extern XColor		user_colors[MAX_USR_COLS];
extern XColor		undel_user_color;
extern XColor		x_fg_color, x_bg_color;
extern fig_colors	colorNames[NUM_STD_COLS + 1];

/* For GIF/XPM images */
/* number of colors we want to use for GIF/XPM images */
extern int		avail_image_cols;
/* colormap used for same */
extern XColor		image_cells[MAX_COLORMAP_SIZE];

#endif /* U_COLORS_H */

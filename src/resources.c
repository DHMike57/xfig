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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "resources.h"

#include <stddef.h>
#include <sys/types.h>

#include <u_colors.h>


/* current export/print background color */
int		export_background_color = COLOR_NONE;

/* these are allocated in main() in case we aren't using default colormap
   (so we can't use BlackPixelOfScreen...) */

/* for the xfig icon */
Pixmap		fig_icon;

/* version string - generated in main() */
char		xfig_version[100];

/* original directory where xfig started */
char		orig_dir[PATH_MAX+2];

/* whether user is updating Fig files or loading one to view */
Boolean		update_figs = False;

#ifdef USE_XPM
XpmAttributes	xfig_icon_attr;
#endif /* USE_XPM */
int		current_memory;
int		max_depth=-1;
int		min_depth=-1;
char		tool_name[200];
Boolean		display_fractions=True;	/* whether to display fractions in lengths */
char		*userhome=NULL;		/* user's home directory */
float		 scale_factor=1.0;	/* scale drawing as it is read in */
char		 minor_grid[40], major_grid[40]; /* export/print grid values */
Boolean		 draw_parent_gray;	/* in open compound, draw rest in gray */

appresStruct	appres;
Window		main_canvas;		/* main canvas window */
Window		canvas_win;		/* current canvas */
Window		msg_win, sideruler_win, topruler_win;
XftDraw		*main_draw;
XftDraw		*canvas_draw;

Cursor		cur_cursor;
Cursor		arrow_cursor, bull_cursor, buster_cursor, crosshair_cursor,
		null_cursor, text_cursor, pick15_cursor, pick9_cursor,
		panel_cursor, l_arrow_cursor, lr_arrow_cursor, r_arrow_cursor,
		u_arrow_cursor, ud_arrow_cursor, d_arrow_cursor, wait_cursor,
		magnify_cursor;

Widget		tool;
XtAppContext	tool_app;

Widget		canvas_sw, ps_fontmenu,		/* printer font menu tool */
		latex_fontmenu,			/* printer font menu tool */
		msg_panel, name_panel, cmd_form, mode_panel,
		d_label, e_label, mousefun,
		ind_panel, ind_box, upd_ctrl,	/* indicator panel */
		snap_indicator_panel,		/* snap indicator */
		unitbox_sw, sideruler_sw, topruler_sw;

Display	       *tool_d;
Screen	       *tool_s;
Window		tool_w;
Widget		tool_form;
int		tool_sn;
int		tool_vclass;
Visual	       *tool_v;
int		tool_dpth;
int		tool_cells;
int		image_bpp;		/* # of bytes-per-pixel for images at this visual */
int		screen_wd, screen_ht;	/* width and height of screen */
Colormap	tool_cm, newcmap;
Boolean		swapped_cmap = False;
Atom		wm_delete_window;
int		num_recent_files;	/* number of recent files in list */
int		max_recent_files;	/* user max number of recent files */
int		splash_onscreen = False; /* flag used to clear off splash graphic */
time_t		figure_timestamp;	/* last time file was written externally (for -autorefresh) */

GC		border_gc, button_gc, ind_button_gc, mouse_button_gc, pic_gc,
		fill_color_gc, pen_color_gc, blank_gc, ind_blank_gc,
		mouse_blank_gc, gccache[NUMOPS], grid_gc,
		fillgc, fill_gc[NUMFILLPATS],	/* fill style gc's */
		tr_gc, tr_xor_gc, tr_erase_gc,	/* for the rulers */
		sr_gc, sr_xor_gc, sr_erase_gc;

Pixmap		fill_pm[NUMFILLPATS],fill_but_pm[NUMPATTERNS];
float		fill_pm_zoom[NUMFILLPATS],fill_but_pm_zoom[NUMFILLPATS];

float		ZOOM_FACTOR;	/* assigned in main.c */
float		PIC_FACTOR;	/* assigned in main.c, updated in unit_panel_set() and
					update_settings() when reading figure file */

/* will be filled in with environment variable XFIGTMPDIR */
char	       *TMPDIR;

/* will contain environment variable FIG2DEV_DIR, if any */
char	       *fig2dev_path;
char	        fig2dev_cmd[PATH_MAX];

/***** translations used for asciiTextWidgets in general windows *****/
String  text_translations =
	"<Key>Return: no-op()\n\
	Ctrl<Key>J: no-op()\n\
	Ctrl<Key>M: no-op()\n\
	Ctrl<Key>X: EmptyTextKey()\n\
	Ctrl<Key>U: multiply(4)\n\
	<Key>F18: PastePanelKey()\n";

/* for w_export.c and w_print.c */

char    *orient_items[] = {
    " Portrait ",
    "Landscape "};

char    *just_items[] = {
    "Centered  ",
    "Flush left"};

/* IMPORTANT:  if the number or order of this table is changed be sure
		to change the PAPER_xx definitions in resources.h */

struct	paper_def paper_sizes[NUMPAPERSIZES] = {
    {"Letter"  , "Letter  (8.5\" x 11\" / 216 x 279 mm)", LETTER_WIDTH, LETTER_HEIGHT},
    {"Legal"   , "Legal   (8.5\" x 14\" / 216 x 356 mm)",   10200, 16800},
    {"Tabloid" , "Tabloid ( 11\" x 17\" / 279 x 432 mm)",   13200, 20400},
    {"A"       , "ANSI A  (8.5\" x 11\" / 216 x 279 mm)",   10200, 13200},
    {"B"       , "ANSI B  ( 11\" x 17\" / 279 x 432 mm)",   13200, 20400},
    {"C"       , "ANSI C  ( 17\" x 22\" / 432 x 559 mm)",   20400, 26400},
    {"D"       , "ANSI D  ( 22\" x 34\" / 559 x 864 mm)",   26400, 40800},
    {"E"       , "ANSI E  ( 34\" x 44\" / 864 x 1118mm)",   40800, 52800},
    {"A9"      , "ISO A9  (  37 x   52 mm)           ",  1748,  2467},
    {"A8"      , "ISO A8  (  52 x   74 mm)           ",  2457,  3500},
    {"A7"      , "ISO A7  (  74 x  105 mm)           ",  3496,  4960},
    {"A6"      , "ISO A6  ( 105 x  148 mm)           ",  4960,  6992},
    {"A5"      , "ISO A5  ( 148 x  210 mm)           ",  6992,  9921},
    {"A4"      , "ISO A4  ( 210 x  297 mm)           ",  A4_WIDTH, A4_HEIGHT},
    {"A3"      , "ISO A3  ( 297 x  420 mm)           ", 14031, 19843},
    {"A2"      , "ISO A2  ( 420 x  594 mm)           ", 19843, 28063},
    {"A1"      , "ISO A1  ( 594 x  841 mm)           ", 28063, 39732},
    {"A0"      , "ISO A0  ( 841 x 1189 mm)           ", 39732, 56173},
    {"B9"      , "JIS B9  (  45 x   64 mm)           ",  2117,  3017},
    {"B8"      , "JIS B8  (  64 x   91 mm)           ",  3017,  4300},
    {"B7"      , "JIS B7  (  91 x  128 mm)           ",  4300,  6050},
    {"B6"      , "JIS B6  ( 128 x  182 mm)           ",  6050,  8598},
    {"B5"      , "JIS B5  ( 182 x  257 mm)           ",  8598, 12150},
    {"B4"      , "JIS B4  ( 257 x  364 mm)           ", 12150, 17200},
    {"B3"      , "JIS B3  ( 364 x  515 mm)           ", 17200, 24333},
    {"B2"      , "JIS B2  ( 515 x  728 mm)           ", 24333, 34400},
    {"B1"      , "JIS B1  ( 728 x 1030 mm)           ", 34400, 48666},
    {"B0"      , "JIS B0  (1030 x 1456 mm)           ", 48666, 68783},
    {"B10"     , "JIS B10 (  32 x   45 mm)           ",  1516,  2117},
    };

char    *multiple_pages[] = {
    "  Single  ",
    " Multiple "};

char    *overlap_pages[] = {
    " No Overlap",
    "  Overlap  "};

/* for w_file.c and w_export.c */
char    *offset_unit_items[] = {
        " Inches  ", " Centim. ", "Fig Units" };

int	RULER_WD;

/* flag for when picture object is read in merge_file to see if need to remap
   existing picture colors */
Boolean	pic_obj_read;

/* recent file structure/array */
_recent_files recent_files[MAX_RECENT_FILES];

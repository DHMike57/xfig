/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2022 by Thomas Loimer
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

#ifndef W_CANVAS_H
#define W_CANVAS_H

#include <X11/Intrinsic.h>	/* String, Widget, Cardinal */
#include <X11/Xlib.h>		/* XButtonEvent */

extern void	init_canvas(Widget tool);
extern void	add_canvas_actions(void);
extern void	(*canvas_kbd_proc) ();
extern void	(*canvas_locmove_proc) ();
extern void	(*canvas_ref_proc) ();
extern void	(*canvas_leftbut_proc) ();
extern void	(*canvas_middlebut_proc) ();
extern void	(*canvas_middlebut_save) ();
extern void	(*canvas_rightbut_proc) ();
extern void	(*return_proc) ();
extern void	null_proc(void);
extern void	toggle_show_balloons(void);
extern void	toggle_show_lengths(void);
extern void	toggle_show_vertexnums(void);
extern void	toggle_show_borders(void);
extern void	round_coords(int *restrict x, int *restrict y);
extern void	floor_coords(int *restrict x, int *restrict y);
extern void	ceil_coords(int *restrict x, int *restrict y);
extern int	point_spacing(void);

extern void	canvas_selected(Widget tool, XButtonEvent *event,
				String *params, Cardinal *nparams);
extern void	paste_primary_selection(void);
extern void	setup_canvas(void);
extern void	clear_region(int xmin, int ymin, int xmax, int ymax);
extern void	clear_canvas(void);

extern int	clip_xmin, clip_ymin, clip_xmax, clip_ymax;
extern int	clip_width, clip_height;
extern int	cur_x, cur_y;
extern int	fix_x, fix_y;
extern int	ignore_exp_cnt;
extern int	last_x, last_y;	/* last position of mouse */
extern int	shift;		/* global state of shift key */
extern int	pointer_click;	/* for counting multiple clicks */

/* for Sun keyboard, define COMP_LED 2 */
extern void	setCompLED(int on);

extern String	local_translations;

#define LOC_OBJ	"Locate Object"

#endif /* W_CANVAS_H */

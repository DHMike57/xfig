/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Parts Copyright (c) 1994 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 *
 * The X Consortium, and any party obtaining a copy of these files from
 * the X Consortium, directly or indirectly, is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software subject to the restriction stated
 * below, and to permit persons who receive copies from any such party to
 * do so, with the only requirement being that this copyright notice remain
 * intact.
 * This license includes without limitation a license to do the foregoing
 * actions under any patents of the party supplying this software to the 
 * X Consortium.
 *
 */

/********************** DECLARATIONS ********************/

/* IMPORTS */

#include "fig.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "u_create.h"
#include "u_elastic.h"
#include "u_list.h"
#include "w_canvas.h"
#include "w_mousefun.h"

/* LOCAL */

F_pos		point[3];

static int	create_arcobject();
static int	get_arcpoint();
static int	init_arc_drawing();
static int	cancel_arc();

arc_drawing_selected()
{
    set_mousefun("first point", "", "", "", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_arc_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static
init_arc_drawing(x, y)
    int		    x, y;
{
    set_mousefun("mid point", "", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_rightbut_proc = cancel_arc;
    num_point = 0;
    point[num_point].x = fix_x = cur_x = x;
    point[num_point++].y = fix_y = cur_y = y;
    canvas_locmove_proc = freehand_line;
    canvas_leftbut_proc = get_arcpoint;
    canvas_middlebut_proc = null_proc;
    elastic_line();
    set_temp_cursor(null_cursor);
    set_action_on();
}

static
cancel_arc()
{
    elastic_line();
    if (num_point == 2) {
	/* erase initial part of line */
	cur_x = point[0].x;
	cur_y = point[0].y;
	elastic_line();
    }
    arc_drawing_selected();
    draw_mousefun_canvas();
}

static
get_arcpoint(x, y)
    int		    x, y;
{
    if (x == fix_x && y == fix_y)
	return;

    if (num_point == 1) {
	set_mousefun("final point", "", "cancel", "", "", "");
	draw_mousefun_canvas();
    }
    if (num_point == 2) {
	create_arcobject(x, y);
	return;
    }
    elastic_line();
    cur_x = x;
    cur_y = y;
    elastic_line();
    point[num_point].x = fix_x = x;
    point[num_point++].y = fix_y = y;
    elastic_line();
}

static
create_arcobject(lx, ly)
    int		    lx, ly;
{
    F_arc	   *arc;
    int		    x, y, i;
    float	    xx, yy;

    elastic_line();
    cur_x = lx;
    cur_y = ly;
    elastic_line();
    point[num_point].x = lx;
    point[num_point++].y = ly;
    x = point[0].x;
    y = point[0].y;
    /* erase previous line segment(s) if necessary */
    for (i = 1; i < num_point; i++) {
	pw_vector(canvas_win, x, y, point[i].x, point[i].y, INV_PAINT,
		  1, RUBBER_LINE, 0.0, DEFAULT);
	x = point[i].x;
	y = point[i].y;
    }
    if (!compute_arccenter(point[0], point[1], point[2], &xx, &yy)) {
	put_msg("Invalid ARC geometry");
	arc_drawing_selected();
	draw_mousefun_canvas();
	return;
    }
    if ((arc = create_arc()) == NULL) {
	arc_drawing_selected();
	draw_mousefun_canvas();
	return;
    }
    arc->type = cur_arctype;
    arc->style = cur_linestyle;
    arc->thickness = cur_linewidth;
    /* scale dash length according to linethickness */
    arc->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    arc->pen_style = 0;
    arc->fill_style = cur_fillstyle;
    arc->pen_color = cur_pencolor;
    arc->fill_color = cur_fillcolor;
    arc->cap_style = cur_capstyle;
    arc->depth = cur_depth;
    arc->direction = compute_direction(point[0], point[1], point[2]);
    if (autoforwardarrow_mode)
	arc->for_arrow = forward_arrow();
    else
	arc->for_arrow = NULL;
    if (autobackwardarrow_mode)
	arc->back_arrow = backward_arrow();
    else
	arc->back_arrow = NULL;
    arc->center.x = xx;
    arc->center.y = yy;
    arc->point[0].x = point[0].x;
    arc->point[0].y = point[0].y;
    arc->point[1].x = point[1].x;
    arc->point[1].y = point[1].y;
    arc->point[2].x = point[2].x;
    arc->point[2].y = point[2].y;
    arc->next = NULL;
    add_arc(arc);
    /* draw it and anything on top of it */
    redisplay_arc(arc);
    arc_drawing_selected();
    draw_mousefun_canvas();
}

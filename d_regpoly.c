/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1994 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 *
 * The X Consortium, and any party obtaining a copy of these files from
 * the X Consortium, directly or indirectly, is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.  This license includes without
 * limitation a license to do the foregoing actions under any patents of
 * the party supplying this software to the X Consortium.
 */

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

extern double	compute_angle();

/*************************** local declarations *********************/

static		init_regpoly_drawing(), create_regpoly(), cancel_regpoly();

regpoly_drawing_selected()
{
    set_mousefun("center point", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_regpoly_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static
init_regpoly_drawing(x, y)
    int		    x, y;
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    work_numsides = cur_numsides;
    set_mousefun("final point", "", "cancel");
    draw_mousefun_canvas();
    canvas_locmove_proc = resizing_poly;
    canvas_leftbut_proc = create_regpoly;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = cancel_regpoly;
    elastic_poly(fix_x, fix_y, cur_x, cur_y, work_numsides);
    set_temp_cursor(null_cursor);
    set_action_on();
}

static
cancel_regpoly()
{
    elastic_poly(fix_x, fix_y, cur_x, cur_y, work_numsides);
    regpoly_drawing_selected();
    draw_mousefun_canvas();
}

static
create_regpoly(x, y)
    int		    x, y;
{
    register float  angle;
    register int    nx, ny, i;
    double	    dx, dy;
    double	    init_angle, mag;
    F_line	   *poly;
    F_point	   *point;

    elastic_poly(fix_x, fix_y, cur_x, cur_y, work_numsides);
    if (fix_x == x && fix_y == y)
	return;			/* 0 size */

    if ((point = create_point()) == NULL)
	return;

    point->x = x;
    point->y = y;
    point->next = NULL;

    if ((poly = create_line()) == NULL) {
	free((char *) point);
	return;
    }
    poly->type = T_POLYGON;
    poly->style = cur_linestyle;
    poly->thickness = cur_linewidth;
    poly->pen_color = cur_pencolor;
    poly->fill_color = cur_fillcolor;
    poly->depth = cur_depth;
    poly->pen_style = 0;
    poly->join_style = cur_joinstyle;
    poly->cap_style = cur_capstyle;
    poly->fill_style = cur_fillstyle;
    /* scale dash length by line thickness */
    poly->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    poly->radius = 0;
    poly->points = point;

    dx = x - fix_x;
    dy = y - fix_y;
    mag = sqrt(dx * dx + dy * dy);
    init_angle = compute_angle(dx, dy);

    /* now append cur_numsides points */
    for (i = 1; i < cur_numsides; i++) {
	angle = init_angle - M_2PI * (double) i / (double) cur_numsides;
	if (angle < 0)
	    angle += M_2PI;
	nx = fix_x + round(mag * cos(angle));
	ny = fix_y + round(mag * sin(angle));
	append_point(nx, ny, &point);
    }
    append_point(x, y, &point);

    draw_line(poly, PAINT);
    add_line(poly);
    regpoly_drawing_selected();
    draw_mousefun_canvas();
}

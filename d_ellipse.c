/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2000 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 *
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

/*************************  local procedures  ********************/

static void	init_ellipsebyradius_drawing();
static void	init_ellipsebydiameter_drawing();
static void	init_circlebyradius_drawing();
static void	init_circlebydiameter_drawing();
static void	create_ellipsebydia();
static void	create_ellipsebyrad();
static void	create_circlebyrad();
static void	create_circlebydia();
static void	cancel_ellipsebydia();
static void	cancel_ellipsebyrad();
static void	cancel_circlebyrad();
static void	cancel_circlebydia();

void
ellipsebyradius_drawing_selected()
{
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_ellipsebyradius_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    set_mousefun("center point", "", "", "", "", "");
    reset_action_on();
}

static void
init_ellipsebyradius_drawing(x, y)
    int		    x, y;
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    cur_angle = cur_elltextangle/180.0*M_PI;
    center_marker(fix_x, fix_y);
    set_mousefun("corner point", "", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_locmove_proc = resizing_ebr;
    canvas_rightbut_proc = cancel_ellipsebyrad;
    canvas_leftbut_proc = create_ellipsebyrad;
    set_cursor(null_cursor);
    elastic_ebr();
    set_action_on();
}

static void
cancel_ellipsebyrad()
{
    elastic_ebr();
    center_marker(fix_x, fix_y);
    ellipsebyradius_drawing_selected();
    draw_mousefun_canvas();
}

static void
create_ellipsebyrad(x, y)
    int		    x, y;
{
    F_ellipse	   *ellipse;

    elastic_ebr();
    center_marker(fix_x, fix_y);
    if ((ellipse = create_ellipse()) == NULL)
	return;

    ellipse->type = T_ELLIPSE_BY_RAD;
    ellipse->style = cur_linestyle;
    ellipse->thickness = cur_linewidth;
    ellipse->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    ellipse->angle = cur_elltextangle/180.0*M_PI;	/* convert to radians */
    ellipse->pen_color = cur_pencolor;
    ellipse->fill_color = cur_fillcolor;
    ellipse->depth = cur_depth;
    ellipse->pen_style = 0;
    ellipse->fill_style = cur_fillstyle;
    ellipse->direction = 1;
    ellipse->center.x = fix_x;
    ellipse->center.y = fix_y;
    ellipse->radiuses.x = abs(x - fix_x);
    ellipse->radiuses.y = abs(y - fix_y);
    ellipse->start.x = fix_x;
    ellipse->start.y = fix_y;
    ellipse->end.x = x;
    ellipse->end.y = y;
    ellipse->next = NULL;
    add_ellipse(ellipse);
    reset_action_on(); /* this signals redisplay_curobj() not to refresh */
    /* draw it and anything on top of it */
    redisplay_ellipse(ellipse);
    ellipsebyradius_drawing_selected();
    draw_mousefun_canvas();
}

ellipsebydiameter_drawing_selected()
{
    set_mousefun("first corner", "", "", "", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_ellipsebydiameter_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static void
init_ellipsebydiameter_drawing(x, y)
    int		    x, y;
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    cur_angle = cur_elltextangle/180.0*M_PI;
    center_marker(fix_x, fix_y);
    set_mousefun("final corner", "", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_locmove_proc = resizing_ebd;
    canvas_rightbut_proc = cancel_ellipsebydia;
    canvas_leftbut_proc = create_ellipsebydia;
    set_cursor(null_cursor);
    elastic_ebd();
    set_action_on();
}

static void
cancel_ellipsebydia()
{
    elastic_ebd();
    center_marker(fix_x, fix_y);
    ellipsebydiameter_drawing_selected();
    draw_mousefun_canvas();
}

static void
create_ellipsebydia(x, y)
    int		    x, y;
{
    F_ellipse	   *ellipse;

    elastic_ebd();
    center_marker(fix_x, fix_y);
    if ((ellipse = create_ellipse()) == NULL)
	return;

    ellipse->type = T_ELLIPSE_BY_DIA;
    ellipse->style = cur_linestyle;
    ellipse->thickness = cur_linewidth;
    ellipse->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    ellipse->angle = cur_elltextangle/180.0*M_PI;	/* convert to radians */
    ellipse->pen_color = cur_pencolor;
    ellipse->fill_color = cur_fillcolor;
    ellipse->depth = cur_depth;
    ellipse->pen_style = 0;
    ellipse->fill_style = cur_fillstyle;
    ellipse->direction = 1;
    ellipse->center.x = (fix_x + x) / 2;
    ellipse->center.y = (fix_y + y) / 2;
    ellipse->radiuses.x = abs(ellipse->center.x - fix_x);
    ellipse->radiuses.y = abs(ellipse->center.y - fix_y);
    ellipse->start.x = fix_x;
    ellipse->start.y = fix_y;
    ellipse->end.x = x;
    ellipse->end.y = y;
    ellipse->next = NULL;
    add_ellipse(ellipse);
    reset_action_on(); /* this signals redisplay_curobj() not to refresh */
    /* draw it and anything on top of it */
    redisplay_ellipse(ellipse);
    ellipsebydiameter_drawing_selected();
    draw_mousefun_canvas();
}

/***************************  circle  section  ************************/

circlebyradius_drawing_selected()
{
    set_mousefun("center point", "", "", "", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_circlebyradius_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static void
init_circlebyradius_drawing(x, y)
    int		    x, y;
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    center_marker(fix_x, fix_y);
    set_mousefun("set radius", "", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_locmove_proc = resizing_cbr;
    canvas_rightbut_proc = cancel_circlebyrad;
    canvas_leftbut_proc = create_circlebyrad;
    set_cursor(null_cursor);
    elastic_cbr();
    set_action_on();
}

static void
cancel_circlebyrad()
{
    elastic_cbr();
    center_marker(fix_x, fix_y);
    circlebyradius_drawing_selected();
    draw_mousefun_canvas();
}

static void
create_circlebyrad(x, y)
    int		    x, y;
{
    F_ellipse	   *c;
    double	    rx, ry;

    elastic_cbr();
    center_marker(fix_x, fix_y);
    if ((c = create_ellipse()) == NULL)
	return;

    c->type = T_CIRCLE_BY_RAD;
    c->style = cur_linestyle;
    c->thickness = cur_linewidth;
    c->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    c->angle = 0.0;
    c->pen_color = cur_pencolor;
    c->fill_color = cur_fillcolor;
    c->depth = cur_depth;
    c->pen_style = 0;
    c->fill_style = cur_fillstyle;
    c->direction = 1;
    c->center.x = fix_x;
    c->center.y = fix_y;
    rx = fix_x - x;
    ry = fix_y - y;
    c->radiuses.x = c->radiuses.y = round(sqrt(rx * rx + ry * ry));
    c->start.x = fix_x;
    c->start.y = fix_y;
    c->end.x = x;
    c->end.y = y;
    c->next = NULL;
    add_ellipse(c);
    reset_action_on(); /* this signals redisplay_curobj() not to refresh */
    /* draw it and anything on top of it */
    redisplay_ellipse(c);
    circlebyradius_drawing_selected();
    draw_mousefun_canvas();
}

circlebydiameter_drawing_selected()
{
    set_mousefun("diameter point", "", "", "", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_circlebydiameter_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static void
init_circlebydiameter_drawing(x, y)
    int		    x, y;
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    center_marker(fix_x, fix_y);
    set_mousefun("final point", "", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_locmove_proc = resizing_cbd;
    canvas_leftbut_proc = create_circlebydia;
    canvas_rightbut_proc = cancel_circlebydia;
    set_cursor(null_cursor);
    elastic_cbd();
    set_action_on();
}

static void
cancel_circlebydia()
{
    elastic_cbd();
    center_marker(fix_x, fix_y);
    circlebydiameter_drawing_selected();
    draw_mousefun_canvas();
}

static void
create_circlebydia(x, y)
    int		    x, y;
{
    F_ellipse	   *c;
    double	    rx, ry;

    elastic_cbd();
    center_marker(fix_x, fix_y);
    if ((c = create_ellipse()) == NULL)
	return;

    c->type = T_CIRCLE_BY_DIA;
    c->style = cur_linestyle;
    c->thickness = cur_linewidth;
    c->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    c->angle = 0.0;
    c->pen_color = cur_pencolor;
    c->fill_color = cur_fillcolor;
    c->depth = cur_depth;
    c->pen_style = 0;
    c->fill_style = cur_fillstyle;
    c->direction = 1;
    c->center.x = round((fix_x + x) / 2);
    c->center.y = round((fix_y + y) / 2);
    rx = x - c->center.x;
    ry = y - c->center.y;
    c->radiuses.x = c->radiuses.y = round(sqrt(rx * rx + ry * ry));
    c->start.x = fix_x;
    c->start.y = fix_y;
    c->end.x = x;
    c->end.y = y;
    c->next = NULL;
    add_ellipse(c);
    reset_action_on(); /* this signals redisplay_curobj() not to refresh */
    /* draw it and anything on top of it */
    redisplay_ellipse(c);
    circlebydiameter_drawing_selected();
    draw_mousefun_canvas();
}

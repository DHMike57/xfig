/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1994 by Brian V. Smith
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

#include "fig.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "u_create.h"
#include "u_draw.h"
#include "u_elastic.h"
#include "u_search.h"
#include "u_list.h"
#include "u_undo.h"
#include "w_canvas.h"
#include "w_mousefun.h"
#include "w_setup.h"

static int	init_delete();
static int	init_delete_region(), delete_region(), cancel_delete_region();
static int	init_delete_to_scrap();

delete_selected()
{
    set_mousefun("delete object", "delete region", "del to cut buf", "", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_delete);
    init_searchproc_right(init_delete_to_scrap);
    canvas_leftbut_proc = object_search_left;
    canvas_rightbut_proc = object_search_right;
    canvas_middlebut_proc = init_delete_region;
    set_cursor(buster_cursor);
    reset_action_on();
}

static
init_delete(p, type, x, y, px, py)
    char	   *p;
    int		    type;
    int		    x, y;
    int		    px, py;
{
    switch (type) {
    case O_COMPOUND:
	cur_c = (F_compound *) p;
	delete_compound(cur_c);
	redisplay_compound(cur_c);
	break;
    case O_POLYLINE:
	cur_l = (F_line *) p;
	delete_line(cur_l);
	redisplay_line(cur_l);
	break;
    case O_TEXT:
	cur_t = (F_text *) p;
	delete_text(cur_t);
	redisplay_text(cur_t);
	break;
    case O_ELLIPSE:
	cur_e = (F_ellipse *) p;
	delete_ellipse(cur_e);
	redisplay_ellipse(cur_e);
	break;
    case O_ARC:
	cur_a = (F_arc *) p;
	delete_arc(cur_a);
	redisplay_arc(cur_a);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	delete_spline(cur_s);
	redisplay_spline(cur_s);
	break;
    default:
	return;
    }
}

static
init_delete_region(x, y)
    int		    x, y;
{
    init_box_drawing(x, y);
    set_mousefun("", "final corner", "cancel", "", "", "");
    draw_mousefun_canvas();
    canvas_leftbut_proc = null_proc;
    canvas_middlebut_proc = delete_region;
    canvas_rightbut_proc = cancel_delete_region;
}

static
cancel_delete_region()
{
    elastic_box(fix_x, fix_y, cur_x, cur_y);
    delete_selected();
    draw_mousefun_canvas();
}

static
delete_region(x, y)
    int		    x, y;
{
    F_compound	   *c;

    if ((c = create_compound()) == NULL)
	return;

    elastic_box(fix_x, fix_y, cur_x, cur_y);
    c->nwcorner.x = min2(fix_x, x);
    c->nwcorner.y = min2(fix_y, y);
    c->secorner.x = max2(fix_x, x);
    c->secorner.y = max2(fix_y, y);
    tag_obj_in_region(c->nwcorner.x,c->nwcorner.y,c->secorner.x,c->secorner.y);
    if (compose_compound(c) == 0) {
	free((char *) c);
	delete_selected();
	draw_mousefun_canvas();
	put_msg("Empty region, figure unchanged");
	return;
    }
    clean_up();
    toggle_markers_in_compound(c);
    set_tags(c,0);
    set_latestobjects(c);
    tail(&objects, &object_tails);
    append_objects(&objects, &saved_objects, &object_tails);
    cut_objects(&objects, &object_tails);
    set_action_object(F_DELETE, O_ALL_OBJECT);
    set_modifiedflag();
    redisplay_compound(c);
    delete_selected();
    draw_mousefun_canvas();
}

static
init_delete_to_scrap(p, type, x, y, px, py)
    char	   *p;
    int		    type;
    int		    x, y;
    int		    px, py;
{
    FILE	   *fp;
    FILE	   *open_cut_file();

    if ((fp=open_cut_file())==NULL)
	return;
    write_file_header(fp);

    switch (type) {
    case O_COMPOUND:
	cur_c = (F_compound *) p;
	write_compound(fp, cur_c);
	delete_compound(cur_c);
	redisplay_compound(cur_c);
	break;
    case O_POLYLINE:
	cur_l = (F_line *) p;
	write_line(fp, cur_l);
	delete_line(cur_l);
	redisplay_line(cur_l);
	break;
    case O_TEXT:
	cur_t = (F_text *) p;
	write_text(fp, cur_t);
	delete_text(cur_t);
	redisplay_text(cur_t);
	break;
    case O_ELLIPSE:
	cur_e = (F_ellipse *) p;
	write_ellipse(fp, cur_e);
	delete_ellipse(cur_e);
	redisplay_ellipse(cur_e);
	break;
    case O_ARC:
	cur_a = (F_arc *) p;
	write_arc(fp, cur_a);
	delete_arc(cur_a);
	redisplay_arc(cur_a);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	write_spline(fp, cur_s);
	delete_spline(cur_s);
	redisplay_spline(cur_s);
	break;
    default:
	fclose(fp);
	return;
    }
    put_msg("Object deleted to scrapfile %s",cut_buf_name);
    fclose(fp);
}

FILE *
open_cut_file()
{
    extern char	    cut_buf_name[];

    FILE	   *fp;
    struct stat	    file_status;

    if (stat(cut_buf_name, &file_status) == 0) {	/* file exists */
	if (file_status.st_mode & S_IFDIR) {
	    put_msg("Error: \"%s\" is a directory", cut_buf_name);
	    return NULL;
	}
	if (file_status.st_mode & S_IWRITE) {	/* writing is permitted */
	    if (file_status.st_uid != geteuid()) {
		put_msg("Error: access denied to cut file");
		return NULL;
	    }
	} else {
	    put_msg("Error: cut file is read only");
	    return NULL;
	}
    } else if (errno != ENOENT) {
	put_msg("Error: cut file didn't pass stat check");
	return NULL;			/* file does exist but stat fails */
    }
    if ((fp = fopen(cut_buf_name, "w")) == NULL) {
	put_msg("Error: couldn't open cut file %s", sys_errlist[errno]);
	return NULL;
    }
    return fp;
}

delete_all()
{
    clean_up();
    set_action_object(F_DELETE, O_ALL_OBJECT);

    /* in case the user is inside any compounds */
    close_all_compounds();

    set_latestobjects(&objects);

    objects.arcs = NULL;
    objects.compounds = NULL;
    objects.ellipses = NULL;
    objects.lines = NULL;
    objects.splines = NULL;
    objects.texts = NULL;

    object_tails.arcs = NULL;
    object_tails.compounds = NULL;
    object_tails.ellipses = NULL;
    object_tails.lines = NULL;
    object_tails.splines = NULL;
    object_tails.texts = NULL;
}

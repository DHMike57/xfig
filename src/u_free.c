/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2024 by Thomas Loimer
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
#include "u_free.h"

#include <X11/Xlib.h>		/* includes X11/X.h */

#include <stdio.h>
#include <stdlib.h>

#include "resources.h"
#include "object.h"
#include "paintop.h"
#include "u_fonts.h"
#include "u_undo.h"		/* saved_objects */
#include "w_drawprim.h"


void
free_arc(F_arc **list)
{
	F_arc	   *a, *arc;

	for (a = *list; a != NULL;) {
		arc = a;
		a = a->next;
		if (arc->for_arrow)
			free((char *) arc->for_arrow);
		if (arc->back_arrow)
			free((char *) arc->back_arrow);
		if (arc->comments)
			free(arc->comments);
		free((char *) arc);
	}
	*list = NULL;
}

void
free_compound(F_compound **list)
{
	F_compound	   *c, *compound;

	for (c = *list; c != NULL;) {
		compound = c;
		c = c->next;
		free_arc(&compound->arcs);
		free_compound(&compound->compounds);
		free_ellipse(&compound->ellipses);
		free_line(&compound->lines);
		free_spline(&compound->splines);
		free_text(&compound->texts);
		if (compound->comments) {
			free(compound->comments);
			compound->comments = NULL;
		}
		free((char *) compound);
		compound = NULL;
	}
	*list = NULL;
}

void
free_ellipse(F_ellipse **list)
{
	F_ellipse	   *e, *ellipse;

	for (e = *list; e != NULL;) {
		ellipse = e;
		e = e->next;
		if (ellipse->comments)
			free(ellipse->comments);
		free((char *) ellipse);
	}
	*list = NULL;
}

void
free_line(F_line **list)
{
	F_line	   *l, *line;

	for (l = *list; l != NULL;) {
		line = l;
		l = l->next;
		free_linestorage(line);
	}
	*list = NULL;
}

void
free_text(F_text **list)
{
	F_text	   *t, *text;

	for (t = *list; t != NULL;) {
		text = t;
		t = t->next;
		free(text->cstring);
		if (text->xftfont)
			closefont(text->xftfont);
		if (text->comments)
			free(text->comments);
		free(text);
	}
	*list = NULL;
}

void
free_spline(F_spline **list)
{
	F_spline	   *s, *spline;

	for (s = *list; s != NULL;) {
		spline = s;
		s = s->next;
		free_splinestorage(spline);
	}
	*list = NULL;
}

void
free_splinestorage(F_spline *s)
{
	free_points(s->points);
	free_sfactors(s->sfactors);
	if (s->for_arrow)
		free((char *) s->for_arrow);
	if (s->back_arrow)
		free((char *) s->back_arrow);
	if (s->comments)
		free(s->comments);
	free((char *) s);
}

void
free_linestorage(F_line *l)
{
	free_points(l->points);
	if (l->for_arrow)
		free((char *) l->for_arrow);
	if (l->back_arrow)
		free((char *) l->back_arrow);
	if (l->pic) {
		free_picture_entry(l->pic->pic_cache);
		if (l->pic->pixmap != 0)
			XFreePixmap(tool_d, l->pic->pixmap);
		l->pic->pixmap = (Pixmap) 0;
		if (l->pic->mask != 0)
			XFreePixmap(tool_d, l->pic->mask);
		l->pic->mask = (Pixmap) 0;
		free((char *) l->pic);
	}
	if (l->comments)
		free(l->comments);
	free((char *) l);
}

void
free_picture_entry(struct _pics *picture)
{
	if (!picture)
		return;

	if (picture->refcount == 0) {
		fprintf(stderr, "Error freeing picture %p %s with refcount = 0\n",
				(void *)picture, picture->file);
		return;
	}
	picture->refcount--;
	if (picture->refcount == 0) {
		if (appres.DEBUG)
			fprintf(stderr, "Delete picture %p %s, refcount = %d\n",
					(void *)picture, picture->file,
					picture->refcount);
		if (picture->bitmap)
			free(picture->bitmap);
		free(picture->file);
		/* unlink from list */
		if (picture->next)
			picture->next->prev = picture->prev;
		if (picture->prev)
			picture->prev->next = picture->next;
		/* at the head of the list */
		if (picture->prev == NULL)
			pictures = NULL;
		free(picture);
	} else {
		if (appres.DEBUG)
			fprintf(stderr,
					"Decrease refcount for picture %p %s, refcount = %d\n",
					(void *)picture, picture->file,
					picture->refcount);
	}
}

void
free_points(F_point *first_point)
{
	F_point	   *p, *q;

	for (p = first_point; p != NULL; p = q) {
		q = p->next;
		free((char *) p);
	}
}

void
free_sfactors(F_sfactor *sf)
{
	F_sfactor	   *a, *b;
	for (a = sf; a != NULL; a = b) {
		b = a->next;
		free((char *) a);
	}
}

void
free_linkinfo(F_linkinfo **list)
{
	F_linkinfo	   *l, *link;

	for (l = *list; l != NULL;) {
		link = l;
		l = l->next;
		free((char *) link);
	}
	*list = NULL;
}

/* free up all the GC's before leaving xfig */

void
free_GCs(void)
{
#ifdef USE_XPM
	/* free any colors from the xfig xpm icon (if used) */
	if (xfig_icon_attr.npixels > 0) {
		XFreeColors(tool_d, tool_cm,
				xfig_icon_attr.pixels, xfig_icon_attr.npixels,
				(unsigned long) 0);
	}
#endif /* USE_XPM */
	XFreeGC(tool_d, border_gc);
	XFreeGC(tool_d, pic_gc);
	XFreeGC(tool_d, button_gc);
	XFreeGC(tool_d, fill_color_gc);
	XFreeGC(tool_d, pen_color_gc);
	XFreeGC(tool_d, ind_button_gc);
	XFreeGC(tool_d, ind_blank_gc);
	XFreeGC(tool_d, blank_gc);
	XFreeGC(tool_d, mouse_blank_gc);
	XFreeGC(tool_d, mouse_button_gc);
	XFreeGC(tool_d, tr_gc);
	XFreeGC(tool_d, tr_erase_gc);
	XFreeGC(tool_d, tr_xor_gc);
	XFreeGC(tool_d, sr_gc);
	XFreeGC(tool_d, sr_erase_gc);
}
/* free up all the Fonts before leaving xfig */

void
free_Fonts(void)
{
	int i;

	closefont(mono_font);
	if (roman_font!=NULL) {
		XFreeFont(tool_d, roman_font);
	};
	if (button_font!=NULL) {
		XFreeFont(tool_d, button_font);
	};
	XFreeGC(tool_d, sr_xor_gc);

	for (i=0; i<NUMOPS; i++) {
		XFreeGC(tool_d, gccache[i]);
	}
	for (i=0; i<NUMFILLPATS; i++) {
		XFreeGC(tool_d, fill_gc[i]);
	}
}

#ifdef FREEMEM
/*
 * Free F_compound objects, but not saved_objects.
 * There are many other pointers to various objects,
 * in undo.h: F_compound object_tails;
 * F_arrow saved_for_arrow, saved_back_arrow;
 * F_line latest_line; F_spline latest_spline;
 * In object.h: cur_l, new_l, old_l; and so on for all objects.
 * These may point to objects already stored in objects,
 * therefore, do not try to free them.
 */
void
free_objects(void)
{
	free_arc(&objects.arcs);
	free_compound(&objects.compounds);
	free_ellipse(&objects.ellipses);
	free_line(&objects.lines);
	free_spline(&objects.splines);
	free_text(&objects.texts);
	if (objects.comments) {
		free(objects.comments);
	}
}

#endif /* FREEMEM */

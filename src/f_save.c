/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2023 by Thomas Loimer
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
#include "f_save.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>     /* includes X11/Xlib.h, which includes X11/X.h */

#include "resources.h"
#include "mode.h"
#include "object.h"

#include "e_compound.h"
#include "f_load.h"
#include "f_picobj.h"
#include "f_read.h"
#include "f_util.h"
#include "u_bound.h"
#include "u_colors.h"
#include "u_convert.h"
#include "w_export.h"
#include "w_msgpanel.h"
#include "w_setup.h"

static int	write_tmpfile = 0;
static char	save_cur_dir[PATH_MAX];

static void	write_arrows(FILE *fp, F_arrow *f, F_arrow *b);
static void	write_comments (FILE *fp, char *com);
static void	write_colordefs (FILE *fp);
static int	write_objects(FILE *fp);
static int	write_objects_close(FILE *fp);


void
init_write_tmpfile(void)
{
	write_tmpfile=1;
	/* save current file directory */
	strcpy(save_cur_dir, cur_file_dir);
	/* and set to export dir to have correct paths
	   for any imported pictures */
	strcpy(cur_file_dir, cur_export_dir);
}

void
end_write_tmpfile(void)
{
	write_tmpfile = 0;
	/* restore current file directory */
	strcpy(cur_file_dir, save_cur_dir);
}

int
write_fd(int fd)
{
	FILE	*fp;

	if (!(fp = fdopen(fd, "wb"))) {
		file_msg("Cannot create stream: %s", strerror(errno));
		return -1;
	}
	num_object = 0;
	/* write_objects() inserts picture paths relative to cur_file_dir */
	return write_objects(fp);
}

int write_file(char *file_name, Boolean update_recent)
{
	FILE	   *fp;

	if (!ok_to_write(file_name, "SAVE"))
		return (-1);

	if ((fp = fopen(file_name, "wb")) == NULL) {
		file_msg("Couldn't open file %s, %s",
				file_name, strerror(errno));
		beep();
		return (-1);
	}
	num_object = 0;
	if (write_objects_close(fp)) {
		file_msg("Error writing file %s, %s",
				file_name, strerror(errno));
		beep();
		exit (2);
		return (-1);
	}
	if (!update_figs)
		put_msg("%d object(s) saved in \"%s\"", num_object, file_name);

	/* update the recent list if caller desires */
	if (update_recent)
		update_recent_list(file_name);

	return (0);
}


/* for fig2dev */

static int
write_objects(FILE *fp)
{
	F_arc		*a;
	F_compound	*c;
	F_ellipse	*e;
	F_line		*l;
	F_spline	*s;
	F_text		*t;

	/*
	 * A 2 for the orientation means that the origin (0,0) is at the upper
	 * left corner of the screen (2nd quadrant).
	 */

	if (!update_figs)
		put_msg("Writing . . .");
	/* set the numeric locale to C so we get decimal points for numbers */
	setlocale(LC_NUMERIC, "C");
	write_fig_header(fp);
	for (a = objects.arcs; a != NULL; a = a->next) {
		num_object++;
		write_arc(fp, a);
	}
	for (c = objects.compounds; c != NULL; c = c->next) {
		num_object++;
		write_compound(fp, c);
	}
	for (e = objects.ellipses; e != NULL; e = e->next) {
		num_object++;
		write_ellipse(fp, e);
	}
	for (l = objects.lines; l != NULL; l = l->next) {
		num_object++;
		write_line(fp, l);
	}
	for (s = objects.splines; s != NULL; s = s->next) {
		num_object++;
		write_spline(fp, s);
	}
	for (t = objects.texts; t != NULL; t = t->next) {
		num_object++;
		write_text(fp, t);
	}
	/* reset to original locale */
	setlocale(LC_NUMERIC, "");

	/* The written data is either accessed by the stream or
	   a file descriptor. Closing the file descriptor withoug flushing the
	   stream first might leave unwritten data in the stream buffer. */
	if (fflush(fp) == EOF) {
		file_msg("Error exporting objects: %s", strerror(errno));
		return errno;
	} else {
		return 0;
	}
}

static int
write_objects_close(FILE *fp)
{
	if (write_objects(fp) || ferror(fp)) {
		fclose(fp);
		return (-1);
	}

	if (fclose(fp) == EOF)
		return (-1);
	return (0);
}

void
write_fig_header(FILE *fp)
{
	char	str[40], *com;
	int		i, len;

	if (appres.write_v40) {
		fprintf(fp, "#FIG 4.0  Produced by xfig version %s\n",
				PACKAGE_VERSION);
		compound_bound(&objects, &objects.nwcorner.x,
				&objects.nwcorner.y, &objects.secorner.x,
				&objects.secorner.y);
		fprintf(fp, "Header {\n");
		fprintf(fp, "    Resolution	%d\n",
				appres.INCHES ? PIX_PER_INCH : PIX_PER_CM);
		fprintf(fp, "    Bounds	%d %d %d %d\n",
				objects.nwcorner.x, objects.nwcorner.y,
				objects.secorner.x, objects.secorner.y);
		fprintf(fp, "    Orient	%s\n",
				appres.landscape ? "Landscape" : "Portrait");
		fprintf(fp, "    Units	%s\n",
				appres.INCHES ? "Inches" : "Metric");
		fprintf(fp, "    Uscale	%.3f%s=1%s\n", appres.userscale,
				appres.INCHES ? "in" :"cm", cur_fig_units);
		fprintf(fp, "    Pagejust	%s\n",
				appres.flushleft ? "Flush left" : "Center");
		fprintf(fp, "    Pagesize	%s\n",
				paper_sizes[appres.papersize].sname);
		fprintf(fp, "    Pages	%s\n",
				appres.multiple ? "Multiple" : "Single");
		fprintf(fp, "    Mag		%.2f\n", appres.magnification);
		get_grid_spec(str, export_grid_minor_text,
				export_grid_major_text);
		fprintf(fp, "    PGrid	%s\n", str);
		fprintf(fp, "    SGrid	%d\n", cur_gridmode);
		fprintf(fp, "    Smoothing	%d\n", appres.smooth_factor);
		fprintf(fp, "    ExportBgColor %d\n", export_background_color);
		fprintf(fp, "    Transp	%d\n", appres.transparent);
		fprintf(fp, "    Margin	%d\n", appres.export_margin);
#ifdef DONT_SHOW_DEPTHS
		if (dont_show_depths) {
			fprintf(fp, "    DontShowDepths	{%s}\n", ......);
		}
#endif /* DONT_SHOW_DEPTHS */
		if (objects.comments) {
			fprintf(fp, "    Description {\n");
			/* escape any '{' we may find in the comments */
			com = objects.comments;
			len = strlen(com);
			for (i=0; i<len; i++) {
				if (com[i] == '{')
					fputc('\\', fp);
				fputc(com[i], fp);
			}
			fprintf(fp, "\n    }\n");
		}
		fprintf(fp, "}\n");
	} else {
		/* V3.2 */
		fprintf(fp, "%s  Produced by xfig version %s\n",
				file_header, PACKAGE_VERSION);
		fputs("#encoding: UTF-8\n", fp);
		fputs(appres.landscape? "Landscape\n": "Portrait\n", fp);
		fputs(appres.flushleft? "Flush left\n": "Center\n", fp);
		fputs(appres.INCHES? "Inches\n": "Metric\n", fp);
		fprintf(fp, "%s\n", paper_sizes[appres.papersize].sname);
		fprintf(fp, "%.2f\n", appres.magnification);
		fprintf(fp, "%s\n", appres.multiple? "Multiple": "Single");
		fprintf(fp, "%d\n", appres.transparent);
		/* figure comments before resolution */
		write_comments(fp, objects.comments);
		/* resolution */
		fprintf(fp, "%d %d\n", PIX_PER_INCH, 2);
	} /* if V4.0 */
	/* write the user color definitions (if any) */
	write_colordefs(fp);
}

/* write the user color definitions (if any) */
void
write_colordefs(FILE *fp)
{
	int		i;

	if (num_usr_cols == 0)
		return;

	if (appres.write_v40)
		fprintf(fp, "UserColors {\n");
	for (i=0; i<num_usr_cols; i++) {
		if (colorUsed[i])
			fprintf(fp, "%s %d #%02x%02x%02x\n",
					appres.write_v40? "  Ucol": "0",
					i + NUM_STD_COLS,
					user_color[i].color.red / 256,
					user_color[i].color.green / 256,
					user_color[i].color.blue / 256);
	}
	if (appres.write_v40)
		fprintf(fp, "}\n");
}

void
write_arc(FILE *fp, F_arc *a)
{
	/* any comments first */
	write_comments(fp, a->comments);
	if (appres.write_v40) {
		fprintf(fp, "Arc {\n");
		switch (a->type) {
		case T_OPEN_ARC:
			fprintf(fp, "  Open");
			break;
		case T_PIE_WEDGE_ARC:
			fprintf(fp, "  PieWedge");
			break;
		case T_ELLIPTICAL:
			fprintf(fp, "  Ellip");
			break;
		default:
			/* arc is corrupt, close off */
			fprintf(fp, "\n}\n");
			return;
		}
		fprintf(fp, "  %d %d %d %d %d %d %d %.3f %d %d %.3f %.3f "
					"%d %d %d %d %d %d %.4f\n",
				a->style, a->thickness, a->pen_color,
				a->fill_color, a->depth, a->pen_style,
				a->fill_style, a->style_val, a->cap_style,
				a->direction, a->center.x, a->center.y,
				a->point[0].x, a->point[0].y,
				a->point[1].x, a->point[1].y,
				a->point[2].x, a->point[2].y, a->angle);
		fprintf(fp, "\n");
		/* finish with any arrowheads */
		write_arrows(fp, a->for_arrow, a->back_arrow);
		fprintf(fp, "}\n");
	} else {
		/* V3.2 */
		/* externally, type 1=open arc, 2=pie wedge */
		fprintf(fp, "%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d "
					"%.3f %.3f %d %d %d %d %d %d\n",
				O_ARC, a->type+1, a->style, a->thickness,
				a->pen_color, a->fill_color, a->depth,
				a->pen_style, a->fill_style, a->style_val,
				a->cap_style, a->direction,
				a->for_arrow ? 1 : 0, a->back_arrow ? 1 : 0,
				a->center.x, a->center.y,
				a->point[0].x, a->point[0].y,
				a->point[1].x, a->point[1].y,
				a->point[2].x, a->point[2].y);
		/* write any arrowheads */
		write_arrows(fp, a->for_arrow, a->back_arrow);
	} /* V4.0/3.2 */
}

void
write_compound(FILE *fp, F_compound *com)
{
	F_arc		*a;
	F_compound	*c;
	F_ellipse	*e;
	F_line		*l;
	F_spline	*s;
	F_text		*t;

	/* any comments first */
	write_comments(fp, com->comments);

	if (appres.write_v40) {
		fprintf(fp, "Compound (%d %d %d %d) {\n",
				com->nwcorner.x, com->nwcorner.y,
				com->secorner.x, com->secorner.y);
	} else {
		/* V3.2 */
		fprintf(fp, "%d %d %d %d %d\n", O_COMPOUND,
				com->nwcorner.x, com->nwcorner.y,
				com->secorner.x, com->secorner.y);
	}
	for (a = com->arcs; a != NULL; a = a->next)
		write_arc(fp, a);
	for (c = com->compounds; c != NULL; c = c->next)
		write_compound(fp, c);
	for (e = com->ellipses; e != NULL; e = e->next)
		write_ellipse(fp, e);
	for (l = com->lines; l != NULL; l = l->next)
		write_line(fp, l);
	for (s = com->splines; s != NULL; s = s->next)
		write_spline(fp, s);
	for (t = com->texts; t != NULL; t = t->next)
		write_text(fp, t);

	/* close off the compound */
	if (appres.write_v40) {
		fprintf(fp, "}\n");
	} else {
		/* V3.2 */
		fprintf(fp, "%d\n", O_END_COMPOUND);
	}
}

void
write_ellipse(FILE *fp, F_ellipse *e)
{
	/* get rid of any evil ellipses which have either radius = 0 */
	if (e->radiuses.x == 0 || e->radiuses.y == 0)
		return;

	/* any comments first */
	write_comments(fp, e->comments);
	if (appres.write_v40) {
		fprintf(fp, "Ellipse {\n");
		if (e->type == T_ELLIPSE_BY_RAD)
			fprintf(fp, "  ByRad {\n");
		else
			fprintf(fp, "  ByDia {\n");

		fprintf(fp, "%d %d %d %d %d %d %d %.3f %d %.4f "
						"%d %d %d %d %d %d %d %d\n",
				e->style, e->thickness, e->pen_color,
				e->fill_color, e->depth, e->pen_style,
				e->fill_style, e->style_val, e->direction,
				e->angle, e->center.x, e->center.y,
				e->radiuses.x, e->radiuses.y, e->start.x,
				e->start.y, e->end.x, e->end.y);
		fprintf(fp, "  }\n");
		fprintf(fp, "}\n");
	} else {
		/* V3.2 */
		fprintf(fp, "%d %d %d %d %d %d %d %d %d %.3f %d %.4f "
						"%d %d %d %d %d %d %d %d\n",
				O_ELLIPSE, e->type, e->style, e->thickness,
				e->pen_color, e->fill_color, e->depth,
				e->pen_style, e->fill_style, e->style_val,
				e->direction, e->angle, e->center.x,
				e->center.y, e->radiuses.x, e->radiuses.y,
				e->start.x, e->start.y, e->end.x, e->end.y);
	} /* V4.0/3.2 */
}

void
write_line(FILE *fp, F_line *l)
{
	F_point	*p;
	int	npts;

	if (l->points == NULL)
		return;

	/* any comments first */
	write_comments(fp, l->comments);

	/* count number of points and put it in the object */
	for (npts=0, p = l->points; p != NULL; p = p->next)
		npts++;
	if (appres.write_v40) {
		fprintf(fp, "Polyline {\n");
		switch (l->type) {
		case T_POLYLINE:
			fprintf(fp, " Line ");
			break;
		case T_BOX:
			fprintf(fp, " Box ");
			break;
		case T_POLYGON:
			fprintf(fp, " Polygon ");
			break;
		case T_ARCBOX:
			fprintf(fp, " Arcbox ");
			break;
		case T_PICTURE:
			fprintf(fp, " Picture ");
			break;
		}
		fprintf(fp, "\n");
		/* finish with any arrowheads */
		write_arrows(fp, l->for_arrow, l->back_arrow);
		fprintf(fp, "}\n");
	} else {
		/* V3.2 */
		fprintf(fp, "%d %d %d %d %d %d %d %d %d %.3f "
						"%d %d %d %d %d %d\n",
				O_POLYLINE, l->type, l->style, l->thickness,
				l->pen_color, l->fill_color, l->depth,
				l->pen_style, l->fill_style, l->style_val,
				l->join_style, l->cap_style, l->radius,
				l->for_arrow ? 1 : 0,
				l->back_arrow ? 1 : 0, npts);
		/* write any arrowheads */
		write_arrows(fp, l->for_arrow, l->back_arrow);

		/* handle picture stuff */
		if (l->type == T_PICTURE) {
			char	picfile_buf[128] = "";
			char	*picfile = picfile_buf;
			char	*utf8_name = NULL;
			if (l->pic->pic_cache && l->pic->pic_cache->file) {
				external_path(&picfile, sizeof picfile_buf,
						l->pic->pic_cache->file);
				if (picfile[0])
					utf8_name = conv_utf8strdup(picfile);
			}
			fprintf(fp, "\t%d %s\n", l->pic->flipped,
					picfile[0] ? utf8_name : EMPTY_PIC);

			if (picfile != picfile_buf)
				free(picfile);
			if (utf8_name)
				free(utf8_name);
		}

		fprintf(fp, "\t");
		npts=0;
		for (p = l->points; p != NULL; p = p->next) {
			fprintf(fp, " %d %d", p->x, p->y);
			if (++npts >= 6 && p->next != NULL) {
				fprintf(fp,"\n\t");
				npts=0;
			}
		}
		fprintf(fp, "\n");
	} /* if V4.0 */
}

void
write_spline(FILE *fp, F_spline *s)
{
	F_sfactor	*cp;
	F_point	*p;
	int		npts;

	if (s->points == NULL)
		return;

	/* any comments first */
	write_comments(fp, s->comments);

	/* count number of points and put it in the object */
	for (npts=0, p = s->points; p != NULL; p = p->next)
		npts++;
	fprintf(fp, "%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d\n",
			O_SPLINE, s->type, s->style, s->thickness,
			s->pen_color, s->fill_color, s->depth, s->pen_style,
			s->fill_style, s->style_val, s->cap_style,
			s->for_arrow ? 1 : 0, s->back_arrow ? 1 : 0, npts);
	/* write any arrowheads */
	write_arrows(fp, s->for_arrow, s->back_arrow);
	fprintf(fp, "\t");
	npts=0;
	for (p = s->points; p != NULL; p = p->next) {
		fprintf(fp, " %d %d", p->x, p->y);
		if (++npts >= 6 && p->next != NULL) {
			fprintf(fp,"\n\t");
			npts=0;
		}
	};
	fprintf(fp, "\n");

	if (s->sfactors == NULL)
		return;

	/* save new shape factor */

	fprintf(fp, "\t");
	npts=0;
	for (cp = s->sfactors; cp != NULL; cp = cp->next) {
		fprintf(fp, " %.3f",cp->s);
		if (++npts >= 8 && cp->next != NULL) {
			fprintf(fp,"\n\t");
			npts=0;
		}
	}
	fprintf(fp, "\n");
}


void
write_text(FILE *fp, F_text *t)
{
	size_t	l, len;
	unsigned char	c;

	if (t->length == 0)
		return;

	/* any comments first */
	write_comments(fp, t->comments);

	fprintf(fp, "%d %d %d %d %d %d %d %.4f %d %d %d %d %d ",
			O_TXT, t->type, t->color, t->depth, t->pen_style,
			t->font, t->size, t->angle,
			t->flags, t->height, t->length,
			t->base_x, t->base_y);
	len = strlen(t->cstring);
	for (l = 0; l < len; ++l) {
		c = t->cstring[l];
		if (c == '\\')
			fputs("\\\\", fp);  /* escape a '\' with another one */
		else
			putc(c,fp);
	}
	fputs("\\001\n", fp);		/* finish off with '\001' string */
}

/* write any arrow heads */
static void
write_arrows(FILE *fp, F_arrow *f, F_arrow *b)
{
	if (appres.write_v40) {
		if (f)
			fprintf(fp, "  ForwardArrow { %d %d %.2f %.2f %.2f }\n",
					f->type, f->style, f->thickness,
					f->wd * 15.0, f->ht * 15.0);
		if (b)
			fprintf(fp,"  BackwardArrow { %d %d %.2f %.2f %.2f }\n",
					b->type, b->style, b->thickness,
					b->wd * 15.0, b->ht * 15.0);
	} else {
		/* V3.2 */
		if (f)
			fprintf(fp, "\t%d %d %.2f %.2f %.2f\n",
					f->type, f->style, f->thickness,
					f->wd * 15.0, f->ht * 15.0);
		if (b)
			fprintf(fp, "\t%d %d %.2f %.2f %.2f\n",
					b->type, b->style, b->thickness,
					b->wd * 15.0, b->ht * 15.0);
	} /* V4.0/V3.2 */
}

void
write_comments(FILE *fp, char *com)
{
	char	last;

	if (!com || !*com)
		return;
	fprintf(fp,"# ");
	while (*com) {
		last = *com;
		fputc(*com,fp);
		if (*com == '\n' && *(com+1) != '\0')
			fprintf(fp,"# ");
		com++;
	}
	/* add newline if last line of comment didn't have one */
	if (last != '\n')
		fputc('\n',fp);
}

int
emergency_save(char *file_name)
{
	FILE	*fp;

	if ((fp = fopen(file_name, "wb")) == NULL)
		return (-1);
	/* first close any open compounds */
	close_all_compounds();
	num_object = 0;
	if (write_objects_close(fp))
		return (-1);
	if (file_name[0] != '/') {
		(void)fprintf(stderr, "xfig: %d object(s) saved in \"%s/%s\"\n",
				num_object, cur_file_dir, file_name);
	} else {
		(void)fprintf(stderr, "xfig: %d object(s) saved in \"%s\"\n",
				num_object, file_name);
	}
	return (0);
}

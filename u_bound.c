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
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.  This license includes without
 * limitation a license to do the foregoing actions under any patents of
 * the party supplying this software to the X Consortium.
 */

#include "fig.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "paintop.h"
#include "u_bound.h"
#include "w_setup.h"

#define		Ninety_deg		M_PI_2
#define		One_eighty_deg		M_PI
#define		Two_seventy_deg		(M_PI + M_PI_2)
#define		Three_sixty_deg		(M_PI + M_PI)
#define		half(z1 ,z2)		((z1+z2)/2.0)

static void	points_bound();
static void	int_spline_bound();
static void	normal_spline_bound();

arc_bound(arc, xmin, ymin, xmax, ymax)
    F_arc	   *arc;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    float	    alpha, beta;
    double	    dx, dy, radius;
    int		    bx, by, sx, sy;
    int		    half_wd;

    dx = arc->point[0].x - arc->center.x;
    dy = arc->center.y - arc->point[0].y;
    alpha = atan2(dy, dx);
    if (alpha < 0.0)
	alpha += Three_sixty_deg;

    radius = sqrt(dx * dx + dy * dy);

    dx = arc->point[2].x - arc->center.x;
    dy = arc->center.y - arc->point[2].y;
    beta = atan2(dy, dx);
    if (beta < 0.0)
	beta += Three_sixty_deg;

    bx = max2(arc->point[0].x, arc->point[1].x);
    bx = max2(arc->point[2].x, bx);
    by = max2(arc->point[0].y, arc->point[1].y);
    by = max2(arc->point[2].y, by);
    sx = min2(arc->point[0].x, arc->point[1].x);
    sx = min2(arc->point[2].x, sx);
    sy = min2(arc->point[0].y, arc->point[1].y);
    sy = min2(arc->point[2].y, sy);

    if (arc->direction == 1) {	/* counter clockwise */
	if (alpha > beta) {
	    if (alpha <= 0 || 0 <= beta)
		bx = (int) (arc->center.x + radius + 1.0);
	    if (alpha <= Ninety_deg || Ninety_deg <= beta)
		sy = (int) (arc->center.y - radius - 1.0);
	    if (alpha <= One_eighty_deg || One_eighty_deg <= beta)
		sx = (int) (arc->center.x - radius - 1.0);
	    if (alpha <= Two_seventy_deg || Two_seventy_deg <= beta)
		by = (int) (arc->center.y + radius + 1.0);
	} else {
	    if (0 <= beta && alpha <= 0)
		bx = (int) (arc->center.x + radius + 1.0);
	    if (Ninety_deg <= beta && alpha <= Ninety_deg)
		sy = (int) (arc->center.y - radius - 1.0);
	    if (One_eighty_deg <= beta && alpha <= One_eighty_deg)
		sx = (int) (arc->center.x - radius - 1.0);
	    if (Two_seventy_deg <= beta && alpha <= Two_seventy_deg)
		by = (int) (arc->center.y + radius + 1.0);
	}
    } else {			/* clockwise	 */
	if (alpha > beta) {
	    if (beta <= 0 && 0 <= alpha)
		bx = (int) (arc->center.x + radius + 1.0);
	    if (beta <= Ninety_deg && Ninety_deg <= alpha)
		sy = (int) (arc->center.y - radius - 1.0);
	    if (beta <= One_eighty_deg && One_eighty_deg <= alpha)
		sx = (int) (arc->center.x - radius - 1.0);
	    if (beta <= Two_seventy_deg && Two_seventy_deg <= alpha)
		by = (int) (arc->center.y + radius + 1.0);
	} else {
	    if (0 <= alpha || beta <= 0)
		bx = (int) (arc->center.x + radius + 1.0);
	    if (Ninety_deg <= alpha || beta <= Ninety_deg)
		sy = (int) (arc->center.y - radius - 1.0);
	    if (One_eighty_deg <= alpha || beta <= One_eighty_deg)
		sx = (int) (arc->center.x - radius - 1.0);
	    if (Two_seventy_deg <= alpha || beta <= Two_seventy_deg)
		by = (int) (arc->center.y + radius + 1.0);
	}
    }
    half_wd = arc->thickness / 2;
    *xmax = bx + half_wd;
    *ymax = by + half_wd;
    *xmin = sx - half_wd;
    *ymin = sy - half_wd;

    /* now add in the arrow (if any) boundaries */
    arrow_bound(O_ARC, arc, xmin, ymin, xmax, ymax);
}

compound_bound(compound, xmin, ymin, xmax, ymax)
    F_compound	   *compound;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    F_arc	   *a;
    F_ellipse	   *e;
    F_compound	   *c;
    F_spline	   *s;
    F_line	   *l;
    F_text	   *t;
    int		    bx, by, sx, sy, first = 1;
    int		    llx, lly, urx, ury;

    for (a = compound->arcs; a != NULL; a = a->next) {
	arc_bound(a, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (c = compound->compounds; c != NULL; c = c->next) {
	sx = c->nwcorner.x;
	sy = c->nwcorner.y;
	bx = c->secorner.x;
	by = c->secorner.y;
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (e = compound->ellipses; e != NULL; e = e->next) {
	ellipse_bound(e, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (l = compound->lines; l != NULL; l = l->next) {
	line_bound(l, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (s = compound->splines; s != NULL; s = s->next) {
	spline_bound(s, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (t = compound->texts; t != NULL; t = t->next) {
	int    dum;
	text_bound(t, &sx, &sy, &bx, &by,
		  &dum,&dum,&dum,&dum,&dum,&dum,&dum,&dum);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    /* round the corners to the current positioning grid */
    floor_coords(llx);
    floor_coords(lly);
    ceil_coords(urx);
    ceil_coords(ury);
    *xmin = llx;
    *ymin = lly;
    *xmax = urx;
    *ymax = ury;
}

/* basically, use the code for drawing the ellipse to find its bounds */
/* From James Tough (see u_draw.c: angle_ellipse() */
/* include the bounds for the markers (even though we don't know if they
   are on or off now */

ellipse_bound(e, xmin, ymin, xmax, ymax)
    F_ellipse	   *e;
    int		   *xmin, *ymin, *xmax, *ymax;
{
	int	    half_wd;
	double	    c1, c2, c3, c4, c5, c6, v1, cphi, sphi, cphisqr, sphisqr;
	double	    xleft, xright, d, asqr, bsqr;
	int	    yymax, yy=0;
	float	    xcen, ycen, a, b;

	xcen = e->center.x;
	ycen = e->center.y;
	a = e->radiuses.x;
	b = e->radiuses.y;
	if (a==0 || b==0) {
	    *xmin = *xmax = xcen;
	    *ymin = *ymax = ycen;
	    return;
	}
	/* angle of 0 is easy */
	if (e->angle == 0) {
	    *xmin = xcen - a;
	    *xmax = xcen + a;
	    *ymin = ycen - b;
	    *ymax = ycen + b;
	    return;
	}

	/* divide by ZOOM_FACTOR because we don't need such precision */
	a /= ZOOM_FACTOR;
	b /= ZOOM_FACTOR;
	xcen /= ZOOM_FACTOR;
	ycen /= ZOOM_FACTOR;

	cphi = cos((double)e->angle);
	sphi = sin((double)e->angle);
	cphisqr = cphi*cphi;
	sphisqr = sphi*sphi;
	asqr = a*a;
	bsqr = b*b;
	
	c1 = (cphisqr/asqr)+(sphisqr/bsqr);
	c2 = ((cphi*sphi/asqr)-(cphi*sphi/bsqr))/c1;
	c3 = (bsqr*cphisqr) + (asqr*sphisqr);
	yymax = sqrt(c3);
	c4 = a*b/c3;
	c5 = 0;
	v1 = c4*c4;
	c6 = 2*v1;
	c3 = c3*v1-v1;
	/* odd first points */
	*xmin = *ymin =  100000;
	*xmax = *ymax = -100000;
	if (yymax % 2) {
		d = sqrt(c3);
		*xmin = min2(*xmin,xcen-d);
		*xmax = max2(*xmax,xcen+d);
		*ymin = min2(*ymin,ycen);
		*ymax = max2(*ymax,ycen);
		c5 = c2;
		yy=1;
	}
	while (c3>=0) {
		d = sqrt(c3);
		xleft = c5-d;
		xright = c5+d;
		*xmin = min2(*xmin,xcen+xleft);
		*xmax = max2(*xmax,xcen+xleft);
		*ymax = max2(*ymax,ycen+yy);
		*xmin = min2(*xmin,xcen+xright);
		*xmax = max2(*xmax,xcen+xright);
		*ymax = max2(*ymax,ycen+yy);
		*xmin = min2(*xmin,xcen-xright);
		*xmax = max2(*xmax,xcen-xright);
		*ymin = min2(*ymin,ycen-yy);
		*xmin = min2(*xmin,xcen-xleft);
		*xmax = max2(*xmax,xcen-xleft);
		*ymin = min2(*ymin,ycen-yy);
		c5+=c2;
		v1+=c6;
		c3-=v1;
		yy=yy+1;
	}
	/* for simplicity, just add half the line thickness to xmax and ymax
	   and subtract half from xmin and ymin */
	half_wd = e->thickness/2;
	*xmax += half_wd;
	*ymax += half_wd;
	*xmin -= half_wd;
	*ymin -= half_wd;
	/* now include the markers because they could be outside the bounds of
	   the ellipse (+/-3 is (roughly) half the size of the markers (5)) */
	/* and multiply back to real coordinates */
	*xmax = max2(*xmax*ZOOM_FACTOR, max2(e->start.x, e->end.x)+3);
	*ymax = max2(*ymax*ZOOM_FACTOR, max2(e->start.y, e->end.y)+3);
	*xmin = min2(*xmin*ZOOM_FACTOR, min2(e->start.x, e->end.x)-3);
	*ymin = min2(*ymin*ZOOM_FACTOR, min2(e->start.y, e->end.y)-3);
}

line_bound(l, xmin, ymin, xmax, ymax)
    F_line	   *l;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    points_bound(l->points, (l->thickness / 2), xmin, ymin, xmax, ymax);
    /* now add in the arrow (if any) boundaries */
    arrow_bound(O_POLYLINE, l, xmin, ymin, xmax, ymax);
}

spline_bound(s, xmin, ymin, xmax, ymax)
    F_spline	   *s;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    if (int_spline(s)) {
	int_spline_bound(s, xmin, ymin, xmax, ymax);
    } else {
	normal_spline_bound(s, xmin, ymin, xmax, ymax);
    }
    /* now add in the arrow (if any) boundaries */
    arrow_bound(O_SPLINE, s, xmin, ymin, xmax, ymax);
}

static void
int_spline_bound(s, xmin, ymin, xmax, ymax)
    F_spline	   *s;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    F_point	   *p1, *p2;
    F_control	   *cp1, *cp2;
    float	    x0, y0, x1, y1, x2, y2, x3, y3, sx1, sy1, sx2, sy2;
    float	    tx, ty, tx1, ty1, tx2, ty2;
    float	    sx, sy, bx, by;
    int		    half_wd;

    p1 = s->points;
    sx = bx = p1->x;
    sy = by = p1->y;
    cp1 = s->controls;
    for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
	 p1 = p2, cp1 = cp2, p2 = p2->next, cp2 = cp2->next) {
	x0 = p1->x;
	y0 = p1->y;
	x1 = cp1->rx;
	y1 = cp1->ry;
	x2 = cp2->lx;
	y2 = cp2->ly;
	x3 = p2->x;
	y3 = p2->y;
	tx = half(x1, x2);
	ty = half(y1, y2);
	sx1 = half(x0, x1);
	sy1 = half(y0, y1);
	sx2 = half(sx1, tx);
	sy2 = half(sy1, ty);
	tx2 = half(x2, x3);
	ty2 = half(y2, y3);
	tx1 = half(tx2, tx);
	ty1 = half(ty2, ty);

	sx = min2(x0, sx);
	sy = min2(y0, sy);
	sx = min2(sx1, sx);
	sy = min2(sy1, sy);
	sx = min2(sx2, sx);
	sy = min2(sy2, sy);
	sx = min2(tx1, sx);
	sy = min2(ty1, sy);
	sx = min2(tx2, sx);
	sy = min2(ty2, sy);
	sx = min2(x3, sx);
	sy = min2(y3, sy);

	bx = max2(x0, bx);
	by = max2(y0, by);
	bx = max2(sx1, bx);
	by = max2(sy1, by);
	bx = max2(sx2, bx);
	by = max2(sy2, by);
	bx = max2(tx1, bx);
	by = max2(ty1, by);
	bx = max2(tx2, bx);
	by = max2(ty2, by);
	bx = max2(x3, bx);
	by = max2(y3, by);
    }
    half_wd = s->thickness / 2;
    *xmin = round(sx) - half_wd;
    *ymin = round(sy) - half_wd;
    *xmax = round(bx) + half_wd;
    *ymax = round(by) + half_wd;
}

static void
normal_spline_bound(s, xmin, ymin, xmax, ymax)
    F_spline	   *s;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    F_point	   *p;
    float	    cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
    float	    x1, y1, x2, y2, sx, sy, bx, by;
    float	    px, py, qx, qy;
    int		    half_wd;

    p = s->points;
    x1 = p->x;
    y1 = p->y;
    p = p->next;
    x2 = p->x;
    y2 = p->y;
    cx1 = (x1 + x2) / 2.0;
    cy1 = (y1 + y2) / 2.0;
    cx2 = (cx1 + x2) / 2.0;
    cy2 = (cy1 + y2) / 2.0;
    if (closed_spline(s)) {
	x1 = (cx1 + x1) / 2.0;
	y1 = (cy1 + y1) / 2.0;
    }
    sx = min2(x1, cx2);
    sy = min2(y1, cy2);
    bx = max2(x1, cx2);
    by = max2(y1, cy2);

    for (p = p->next; p != NULL; p = p->next) {
	x1 = x2;
	y1 = y2;
	x2 = p->x;
	y2 = p->y;
	cx4 = (x1 + x2) / 2.0;
	cy4 = (y1 + y2) / 2.0;
	cx3 = (x1 + cx4) / 2.0;
	cy3 = (y1 + cy4) / 2.0;
	cx2 = (cx4 + x2) / 2.0;
	cy2 = (cy4 + y2) / 2.0;

	px = min2(cx2, cx3);
	py = min2(cy2, cy3);
	qx = max2(cx2, cx3);
	qy = max2(cy2, cy3);

	sx = min2(sx, px);
	sy = min2(sy, py);
	bx = max2(bx, qx);
	by = max2(by, qy);
    }
    half_wd = s->thickness / 2;
    if (closed_spline(s)) {
	*xmin = round(sx) - half_wd;
	*ymin = round(sy) - half_wd;
	*xmax = round(bx) + half_wd;
	*ymax = round(by) + half_wd;
    } else {
	*xmin = round(min2(sx, x2)) - half_wd;
	*ymin = round(min2(sy, y2)) - half_wd;
	*xmax = round(max2(bx, x2)) + half_wd;
	*ymax = round(max2(by, y2)) + half_wd;
    }
}

/* This procedure calculates the bounding box for text.  It returns
   the min/max x and y coords of the enclosing HORIZONTAL rectangle.
   The actual corners of the rectangle are returned in (rx1,ry1)...(rx4,ry4)
 */

text_bound(t, xmin, ymin, xmax, ymax,
		  rx1, ry1, rx2, ry2, rx3, ry3, rx4, ry4)
    F_text	   *t;
    int		   *xmin, *ymin, *xmax, *ymax;
    int		   *rx1,*ry1, *rx2,*ry2, *rx3,*ry3, *rx4,*ry4;
{
    int		    h, l;
    int		    x1,y1, x2,y2, x3,y3, x4,y4;
    double	    cost, sint;
    double	    dcost, dsint, lcost, lsint, hcost, hsint;

    cost = cos((double)t->angle);
    sint = sin((double)t->angle);
    l = text_length(t);
    h = t->ascent+t->descent;
    lcost = round(l*cost);
    lsint = round(l*sint);
    hcost = round(h*cost);
    hsint = round(h*sint);
    dcost = round(t->descent*cost);
    dsint = round(t->descent*sint);
    x1 = t->base_x+dsint;
    y1 = t->base_y+dcost;
    if (t->type == T_CENTER_JUSTIFIED) {
	x1 = t->base_x+dsint - round((l/2)*cost);
	y1 = t->base_y+dcost + round((l/2)*sint);
	x2 = x1 + lcost;
	y2 = y1 - lsint;
    }
    else if (t->type == T_RIGHT_JUSTIFIED) {
	x1 = t->base_x+dsint - lcost;
	y1 = t->base_y+dcost + lsint;
	x2 = t->base_x+dsint;
	y2 = t->base_y+dcost;
    }
    else {
	x2 = x1 + lcost;
	y2 = y1 - lsint;
    }
    x4 = x1 - hsint;
    y4 = y1 - hcost;
    x3 = x2 - hsint;
    y3 = y2 - hcost;

    *xmin = min2(x1,min2(x2,min2(x3,x4)));
    *xmax = max2(x1,max2(x2,max2(x3,x4)));
    *ymin = min2(y1,min2(y2,min2(y3,y4)));
    *ymax = max2(y1,max2(y2,max2(y3,y4)));
    *rx1=x1; *ry1=y1;
    *rx2=x2; *ry2=y2;
    *rx3=x3; *ry3=y3;
    *rx4=x4; *ry4=y4;
}

static void
points_bound(points, half_wd, xmin, ymin, xmax, ymax)
    F_point	   *points;
    int		    half_wd;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    int		    bx, by, sx, sy;
    F_point	   *p;

    bx = sx = points->x;
    by = sy = points->y;
    for (p = points->next; p != NULL; p = p->next) {
	sx = min2(sx, p->x);
	sy = min2(sy, p->y);
	bx = max2(bx, p->x);
	by = max2(by, p->y);
    }
    *xmin = sx - half_wd;
    *ymin = sy - half_wd;
    *xmax = bx + half_wd;
    *ymax = by + half_wd;
}

int
overlapping(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2)
    int		    xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2;
{
    if (xmin1 < xmin2)
	if (ymin1 < ymin2)
	    return (xmax1 >= xmin2 && ymax1 >= ymin2);
	else
	    return (xmax1 >= xmin2 && ymin1 <= ymax2);
    else if (ymin1 < ymin2)
	return (xmin1 <= xmax2 && ymax1 >= ymin2);
    else
	return (xmin1 <= xmax2 && ymin1 <= ymax2);
}

/* extend xmin, ymin xmax, ymax by the arrow boundaries of obj (if any) */

arrow_bound(objtype, obj, xmin, ymin, xmax, ymax)
    int		    objtype;
    F_line	   *obj;
    int		   *xmin, *ymin, *xmax, *ymax;
{
    int		    fxmin, fymin, fxmax, fymax;
    int		    bxmin, bymin, bxmax, bymax;
    F_point	   *p, *q;
    F_arc	   *a;
    int		    p1x, p1y, p2x, p2y;

    if (obj->for_arrow) {
	if (objtype == O_ARC) {
	    a = (F_arc *) obj;
	    compute_normal(a->center.x, a->center.y, a->point[2].x,
		       a->point[2].y, a->direction, &p1x, &p1y);
	    p2x = a->point[2].x;	/* forward tip */
	    p2y = a->point[2].y;
	} else {
	    /* this doesn't work very well for a spline with few points 
		and lots of curvature */
	    /* locate last point (forward tip) and next-to-last point */
	    for (p = obj->points; p->next; p = p->next)
		q = p;
	    p1x = q->x;
	    p1y = q->y;
	    p2x = p->x;
	    p2y = p->y;
	}
	calc_arrow_width(obj->for_arrow->type,
		obj->for_arrow->wid + obj->for_arrow->thickness*ZOOM_FACTOR/2.0,
		obj->for_arrow->ht,
		p1x, p1y, p2x, p2y,
		&fxmin, &fymin, &fxmax, &fymax);
	*xmin = min2(*xmin, fxmin);
	*xmax = max2(*xmax, fxmax);
	*ymin = min2(*ymin, fymin);
	*ymax = max2(*ymax, fymax);
    }
    if (obj->back_arrow) {
	if (objtype == O_ARC) {
	    a = (F_arc *) obj;
	    compute_normal(a->center.x, a->center.y, a->point[0].x,
		       a->point[0].y, a->direction ^ 1, &p1x, &p1y);
	    p2x = a->point[0].x;	/* backward tip */
	    p2y = a->point[0].y;
	} else {
	    p1x = obj->points->next->x;	/* second point */
	    p1y = obj->points->next->y;
	    p2x = obj->points->x;	/* first point (forward tip) */
	    p2y = obj->points->y;
	}
	calc_arrow_width(obj->back_arrow->type,
		obj->back_arrow->wid + obj->back_arrow->thickness*ZOOM_FACTOR/2.0,
		obj->back_arrow->ht,
		p1x, p1y, p2x, p2y,
		&bxmin, &bymin, &bxmax, &bymax);
	*xmin = min2(*xmin, bxmin);
	*xmax = max2(*xmax, bxmax);
	*ymin = min2(*ymin, bymin);
	*ymax = max2(*ymax, bymax);
    }
}

/* calculate the width of an arrow in the direction going from (x1,y1) to (x2,y2) */

calc_arrow_width(type, wid, ht, x1, y1, x2, y2, xmin, ymin, xmax, ymax)
    int		    type, x1, y1, x2, y2;
    int		   *xmin, *ymin, *xmax, *ymax;
    float	    wid, ht;
{
    double	    l, sina, cosa, xb, yb, xc, yc, xd, yd;
    double	    x, y, dx, dy;

    dx = x2 - x1;
    dy = y1 - y2;
    l = sqrt(dx * dx + dy * dy);
    if (l == 0)
	return;
    sina = dy / l;
    cosa = dx / l;
    xb = x2 * cosa - y2 * sina;
    yb = x2 * sina + y2 * cosa;
    /* lengthen the "height" if type 2 */
    if (type == 2)
	x = xb - ht * 1.2;
    /* shorten the "height" if type 3*/
    else if (type == 3)
	x = xb - ht * 0.8;
    else
	x = xb - ht;
    y = yb - wid / 2;
    xc = round( x * cosa + y * sina);
    yc = round(-x * sina + y * cosa);
    y = yb + wid / 2;
    xd = round( x * cosa + y * sina);
    yd = round(-x * sina + y * cosa);

    *xmin = min2(xc, xd);
    *xmax = max2(xc, xd);
    *ymin = min2(yc, yd);
    *ymax = max2(yc, yd);
}

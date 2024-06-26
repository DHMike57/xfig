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

#ifndef U_TRANSLATE_H
#define U_TRANSLATE_H

#include "object.h"

extern void	translate_arc(F_arc *arc, int dx, int dy);
extern void	translate_compound(F_compound *compound, int dx, int dy);
extern void	translate_ellipse(F_ellipse *ellipse, int dx, int dy);
extern void	translate_line(F_line *line, int dx, int dy);
extern void	translate_spline(F_spline *spline, int dx, int dy);
extern void	translate_text(F_text *text, int dx, int dy);

#endif

/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1991 by Henning Spruth
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

#ifndef W_ZOOM_H
#define W_ZOOM_H

#include <X11/Intrinsic.h>

extern float	zoomscale;
extern float	display_zoomscale;
extern int	zoomxoff;
extern int	zoomyoff;
extern Boolean	zoom_in_progress;
extern Boolean	integral_zoom;

extern void	unzoom(void);
extern void	cancel_zoom(void);
extern void	zoom_selected(int x, int y, unsigned int button);

#endif /* W_ZOOM_H */

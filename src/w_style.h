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

#ifndef W_STYLE_H
#define W_STYLE_H

#include <X11/Intrinsic.h>

extern void	init_manage_style_panel(void);
extern void	setup_manage_style_panel(void);
extern void	add_style_actions(void);
extern void	popup_manage_style_panel(void);
extern int	confirm_close_style(void);
extern void	add_style_actions(void);
extern void	init_manage_style_panel(void);
extern void	setup_manage_style_panel(void);

extern Boolean	style_dirty_flag;

#define MAX_STYLE_ELEMENT 30
#define MAX_STYLE_FAMILY 16
#define MAX_STYLE_FAMILY_SET 16

typedef enum Element_type {
  Tint, Tfloat, TColor
} Element_type;

typedef struct Element {
  char *name;
  Element_type type;
  void *value,* toset;
  unsigned long flag;
} Element;

typedef struct Style {
  char *name;
  Element element[MAX_STYLE_ELEMENT];
} Style;

typedef struct Style_family {
  char *name;
  Style style[MAX_STYLE_FAMILY];
} Style_family;

#endif

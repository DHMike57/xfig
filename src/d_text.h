/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2007 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
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

#include <X11/Xlib.h>	/* XFT DEBUG */
#include <X11/Xft/Xft.h> /* XFT DEBUG */

extern int		work_font;
//extern XftFont		*canvas_xftfont;
extern XFontStruct	*canvas_font;
extern void	char_handler(XKeyEvent *kpe, unsigned char c, KeySym keysym);
extern void	draw_char_string(void);
extern void	erase_char_string(void);
extern void	finish_text_input(int x, int y, int shift);
extern void	reload_text_fstruct(F_text *t);
extern void	reload_text_fstructs(void);
extern Boolean	text_selection_active;
extern Boolean	ConvertSelection();
extern void	LoseSelection(), TransferSelectionDone();

#ifdef I18N
extern XIC	xim_ic;
extern Boolean	xim_active;
extern Boolean	xim_initialize(Widget w);
extern void	i18n_char_handler(unsigned char *str);
extern void	prefix_append_char(unsigned char ch);
extern void	xim_set_ic_geometry(XIC ic, int width, int height);
#ifdef I18N_USE_PREEDIT
extern void	kill_preedit();
#endif  /* I18N_USE_PREEDIT */
#endif  /* I18N */
extern void text_drawing_selected (void);

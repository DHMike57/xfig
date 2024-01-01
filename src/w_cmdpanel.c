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
#include "w_cmdpanel.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fontconfig/fontconfig.h>	/* FcUcs4ToUtf8() */
#include <X11/Xlib.h>			/* includes X11/X.h */
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>			/* XA_STRING */
#include <X11/Xft/Xft.h>

#include "figx.h"
#include "resources.h"
#include "main.h"
#include "mode.h"
#include "object.h"

#include "d_text.h"			/* char_handler(), xim_ic */
#include "e_delete.h"
#include "f_load.h"
#include "f_read.h"
#include "f_util.h"
#include "u_bound.h"
#include "u_create.h"
#include "u_draw.h"
#include "u_fonts.h"
#include "u_free.h"
#include "u_list.h"
#include "u_pan.h"
#include "u_redraw.h"
#include "u_translate.h"
#include "u_undo.h"
#include "w_canvas.h"
#include "w_cmdpanel.h"
#include "w_cursor.h"
#include "w_drawprim.h"
#include "w_export.h"
#include "w_file.h"
#include "w_help.h"
#include "w_icons.h"
#include "w_indpanel.h"
#include "w_layers.h"
#include "w_modepanel.h"
#include "w_mousefun.h"
#include "w_msgpanel.h"
#include "w_print.h"
#include "w_rulers.h"
#include "w_setup.h"
#include "w_snap.h"
#include "w_srchrepl.h"
#include "w_style.h"
#include "w_util.h"
#include "w_zoom.h"
#include "xfig_math.h"

#ifdef DIGITIZE
#include "w_digitize.h"
#endif
#ifndef XAW3D1_5E
#include "w_menuentry.h"
#endif

/* internal features and definitions */

/* Character map features */

#define LASTCHAR	255
#define STARTGAP	0x8d
#define MISSING		18
#define CMAP_FONTSIZE	16

static int	charmap_psflag;
static int	charmap_font;
static XftDraw	*xftdraw[LASTCHAR + 1];
static Widget	charcell[LASTCHAR + 1];

DeclareStaticArgs(12);

#define menu_item_bitmap_width 9
#define menu_item_bitmap_height 8
static unsigned char menu_item_bitmap_bits[] = {
	0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
	0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};

static Pixmap	menu_item_bitmap = None;

/* Widgets holding the ascii values for the string-based settings */

Widget	bal_delay;
Widget	n_freehand_resolution;
Widget	n_recent_files;
Widget	max_colors;
Widget	image_ed;
Widget	spell_chk;
Widget	browser;
Widget	pdfview;

/* global settings */

Widget	global_popup = (Widget) 0;
Widget	global_panel;

/* prototypes */

static void	enter_cmd_but(Widget widget, XtPointer closure, XEvent *event,
				Boolean *continue_to_dispatch);
void		delete_all_cmd(Widget w, int closure, int call_data);
static void	init_move_paste_object(int x, int y);
static void	move_paste_object(int x, int y);
static void	place_object(int x, int y, unsigned int shift);
static void	cancel_paste(void);
static void	paste_draw(int paint_mode);
static void	place_object_orig_posn(int x, int y, unsigned int shift);
static void	place_menu(Widget w, XEvent *event, String *params,
				Cardinal *nparams);
static void	popup_menu(Widget w, XEvent *event, String *params,
				Cardinal *nparams);
static void	load_recent_file(Widget w, XtPointer client_data,
				XtPointer call_data);

static void	popup_global_panel(Widget w);
static void	global_panel_done(Widget w, XButtonEvent *ev);
static void	global_panel_cancel(Widget w, XButtonEvent *ev);
static void	character_panel_close(void);
static void	paste_character(Widget w, XtPointer client_data, XEvent *ev,
				Boolean *pass);
static void	redraw_character(Widget w, XtPointer client_data, XEvent *ev,
				Boolean *pass);
static void	redraw_all(Widget w, XtPointer client_data, XEvent *ev,
				Boolean *pass);

Widget		CreateLabelledAscii(Widget *text_widg, char *label,
				char *widg_name, Widget parent, Widget below,
				char *str, int width);
static Widget	create_main_menu(int menu_num, Widget beside);

static int	off_paste_x,off_paste_y;
static int	orig_paste_x,orig_paste_y;

static Widget	character_map_popup = (Widget) 0;
static Widget	character_map_panel, close_but;
static Widget	charmap_font_label;

#ifdef XAW3D1_5E
#else
/* popup message over command button when mouse enters it */
static void	cmd_balloon_trigger(Widget widget, XtPointer closure,
				XEvent *event, Boolean *continue_to_dispatch);
static void	cmd_unballoon(Widget widget, XtPointer closure,
				XEvent *event, Boolean *continue_to_dispatch);

/* popup message over filename window when mouse enters it */
static void	filename_balloon_trigger(Widget widget, XtPointer closure,
				XEvent *event, Boolean *continue_to_dispatch);
static void	filename_unballoon(Widget widget, XtPointer closure,
				XEvent *event, Boolean *continue_to_dispatch);
#endif /* XAW3D1_5E */

String  global_translations =
	"<Message>WM_PROTOCOLS: DismissGlobal()\n";

String  charmap_translations =
	"<Message>WM_PROTOCOLS: DismissCharmap()\n";

static XtActionsRec	global_actions[] = {
	{"DismissGlobal", (XtActionProc)global_panel_cancel},
};
static XtActionsRec	charmap_actions[] = {
	{"DismissCharmap", (XtActionProc)character_panel_close},
};

static XtActionsRec	menu_actions[] = {
	{"xMenuPopup", (XtActionProc)popup_menu},
	{"PlaceMenu", (XtActionProc)place_menu},
};

menu_def file_menu_items[] = {
	{"New         (Meta-N)",	0, new, False},
	{"Open...     (Meta-O)",	0, popup_open_panel, False},
	{"Merge...    (Meta-M)",	0, popup_merge_panel, False},
#ifdef DIGITIZE
	{"Digitize... (Meta-Z)",	0, popup_digitize_panel, False},
#endif /* DIGITIZE */
	{"Save        (Meta-S)",	0, do_save, False},
	{"Save As...  (Meta-A)",	5, popup_saveas_panel, False},
	{"Export...   (Meta-X) (Quick = Shift-Meta-X)",	0, popup_export_panel, False},
	{"Print...    (Meta-P) (Quick = Shift-Meta-P)",	0, popup_print_panel, False},
	{"Exit        (Meta-Q)",	1, quit, False},
	/* makes a line separator followed by recently loaded files */
	{(char *) -1,			0, NULL, False},
	{NULL, 0, NULL, False},
};

menu_def edit_menu_items[] = {
	{"Undo               (Meta-U) ", 0, undo, False},
	{"Paste Objects      (Meta-T) ", 0, paste, False},
	{"Paste Text         (F18/F20)", 6, paste_primary_selection, False},
	{"Search/Replace...  (Meta-I) ", -1, popup_search_panel, False},
	{"Spell Check...     (Meta-K) ", 0, spell_check, False},
	{"Delete All         (Meta-D) ", 0, delete_all_cmd, False},
	{"-",				 0, NULL, False},   /* divider line */
	{"Global settings... (Meta-G) ", 0, show_global_settings, False},
	{"Set units...       (Shift-U)", 5, popup_unit_panel, False},
	{NULL, 0, NULL, False},
};

#define PAGE_BRD_MSG	"Show page borders   (Meta-B)"
#define DPTH_MGR_MSG	"Show depth manager          "
#define INFO_BAL_MSG	"Show info balloons  (Meta-Y)"
#define LINE_LEN_MSG	"Show line lengths   (Meta-L)"
#define VRTX_NUM_MSG	"Show vertex numbers         "
#define AUTO_RFS_MSG	"Autorefresh mode            "

menu_def view_menu_items[] = {
	{"Manage Styles...      (Ctrl-Y)",  7, popup_manage_style_panel, False},
	{"Redraw                (Ctrl-L)",  0, redisplay_canvas, False},
	{"Portrait/Landscape    (Meta-C)",  3, change_orient, False},
	{"Zoom In               (Shift-Z)", 5, inc_zoom_centered, False},
	{"Zoom Out              (z)",	    5, dec_zoom_centered, False},
	{"Zoom to Fit canvas    (Ctrl-Z)",  8, fit_zoom, False},
	{"Unzoom",			    0, unzoom, False},
	{"Pan to origin",		    0, pan_origin, False},
	{"Character map",		    0, popup_character_map, False},
	{"-",				    0, NULL, False}, /* divider line */
	/* the following menu labels will be refreshed in refresh_view_menu() */
	{PAGE_BRD_MSG,			    10, toggle_show_borders, True},
	{DPTH_MGR_MSG,			     5, toggle_show_depths, True},
	{INFO_BAL_MSG,			     6, toggle_show_balloons, True},
	{LINE_LEN_MSG,			    10, toggle_show_lengths, True},
	{VRTX_NUM_MSG,			     5, toggle_show_vertexnums, True},
	{AUTO_RFS_MSG,			     0, toggle_refresh_mode, True},
	{NULL, 0, NULL, False},
    };

menu_def help_menu_items[] = {
	{"Xfig Reference (HTML)...",	0, launch_refman, False},
#ifdef FIXED_JAPANESE_PDF
	{"Xfig Reference (PDF, English)...",	0, launch_refpdf_en, False},
	/* Tom Sato said that the Japanese version of the pdf looked ugly so we'll not distribute it now */
	{"Xfig Reference (PDF, Japanese)...",	0, launch_refpdf_jp, False},
#else
	{"Xfig Reference (PDF)...",	0, launch_refpdf_en, False},
#endif /* FIXED_JAPANESE_PDF */
	{"Xfig Man Pages (HTML)...",	5, launch_man, False},
	{"How-To Guide (PDF)...",	0, launch_howto, False},
	{"About Xfig...",		0, launch_about, False},
	{NULL, 0, NULL, False},
};

menu_def snap_menu_items[] = {
	{"Hold",	0, snap_hold, False},		/* hol snap mode until
						   released	*/
  {"Release",	0, snap_release, False},	/* release hold	*/
  {"-",		0, NULL, False},		/* make a dividing line	*/
/* selections that always work*/
  {"Endpoint",	0, snap_endpoint, False},	/* snap to vertices or other
						   endpoints*/
  {"Midpoint",	0, snap_midpoint, False},	/* snap to segment or other
						   midpoints */
  {"Nearest",	0, snap_nearest, False},	/* snap to nearest object */
  {"Focus",	0, snap_focus, False},		/* snap to ellipse focus or
						   circle centerpoint */
  {"-",		0, NULL, False},		/* make a dividing line */
/* selections that only work as a polyline vertex (other stuff?) */
  {"Normal",	0, snap_normal, False},		/* snap to point that results in
						   a seg normal to snapped-to
						   obj */
  {"Tangent",	0, snap_tangent, False},	/* snap to point that results in
						   a seg tangent to obj */
  {"Intersection", 0, snap_intersect, False},	/* snap to intersection of
						   picked objs */
  {"Diameter",	0, snap_diameter, False},	/* snap to ellipse or circle
						   opposite diameter */
  {"-",		0, NULL, False},		/* make a dividing line */
  {NULL, 0, NULL, False},
};

/* command panel of menus */

main_menu_info main_menus[] = {
	{"File", "filemenu", "File menu", file_menu_items, NULL, NULL},
	{"Edit", "editmenu", "Edit menu", edit_menu_items, NULL, NULL},
	{"View", "viewmenu", "View menu", view_menu_items, NULL, NULL},
	{"Snap", "snapmenu", "Snap menu", snap_menu_items, NULL, NULL},
	{"Help", "helpmenu", "Help menu", help_menu_items, NULL, NULL},
};
#define		NUM_CMD_MENUS  (sizeof(main_menus) / sizeof(main_menu_info))

/* needed by setup_sizes() */


static void	create_global_panel (Widget w);
static size_t	locate_menu (String *params, Cardinal *nparams);


int
num_main_menus(void)
{
	return (int)(NUM_CMD_MENUS);
}

/* command panel */
void
init_main_menus(Widget tool, char *filename)
{
	register int	i;
	Widget	beside = NULL;
	DeclareArgs(11);

	FirstArg(XtNborderWidth, 0);
	NextArg(XtNcolormap, tool_cm);
	NextArg(XtNdefaultDistance, 0);
	NextArg(XtNhorizDistance, 0);
	NextArg(XtNvertDistance, 0);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	cmd_form = XtCreateWidget("commands", formWidgetClass,
			tool, Args, ArgCount);

	for (i = 0; i < (int)NUM_CMD_MENUS; ++i) {
		beside = create_main_menu(i, beside);
	}

	/* now setup the filename label widget to the right
	   of the command menu buttons */

	FirstArg(XtNlabel, filename);
	NextArg(XtNinternational, appres.international);
	NextArg(XtNfromHoriz, cmd_form);
	NextArg(XtNhorizDistance, -INTERNAL_BW);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNwidth, NAMEPANEL_WD);
	NextArg(XtNheight, CMD_BUT_HT+INTERNAL_BW);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNborderWidth, INTERNAL_BW);
	name_panel = XtCreateManagedWidget("file_name", labelWidgetClass,
			tool, Args, ArgCount);
#ifndef XAW3D1_5E
	/* popup balloon when mouse passes over filename */
	XtAddEventHandler(name_panel, EnterWindowMask, False,
			filename_balloon_trigger, (XtPointer) name_panel);
	XtAddEventHandler(name_panel, LeaveWindowMask, False,
			filename_unballoon, (XtPointer) name_panel);
#endif
	/* add actions to position the menus if the user uses an accelerator */
	refresh_view_menu();
}

void
add_cmd_actions(void)
{
	XtAppAddActions(tool_app, menu_actions, XtNumber(menu_actions));
}

static Widget
create_main_menu(int menu_num, Widget beside)
{
	register main_menu_info *menu;

	menu = &main_menus[menu_num];
	FirstArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfont, button_font);
	NextArg(XtNwidth, CMD_BUT_WD);
	NextArg(XtNheight, CMD_BUT_HT);
	NextArg(XtNvertDistance, 0);
	NextArg(XtNhorizDistance, -INTERNAL_BW);
	NextArg(XtNlabel, menu->label);
	NextArg(XtNinternational, False);
	NextArg(XtNfromHoriz, beside);
	NextArg(XtNmenuName, menu->menu_name);
	/* make button to popup each menu */
	menu->widget = XtCreateManagedWidget(menu->label, menuButtonWidgetClass,
			cmd_form, Args, ArgCount);
	XtAddEventHandler(menu->widget, EnterWindowMask, False,
			enter_cmd_but, (XtPointer) menu);

	/* now the menu itself */
	menu->menuwidget = create_menu_item(menu);

#ifndef XAW3D1_5E
	/* popup when mouse passes over button */
	XtAddEventHandler(menu->widget, EnterWindowMask, False,
			cmd_balloon_trigger, (XtPointer) menu);
	XtAddEventHandler(menu->widget, LeaveWindowMask, False,
			cmd_unballoon, (XtPointer) menu);
#endif

	return menu->widget;
}

void
rebuild_file_menu(Widget menu)
{
	static Boolean	first = TRUE;
	Widget		entry;
	int		j;
	char		id[10];

	if (menu == None)
		menu = main_menus[0].menuwidget;

	if (first) {
		first = FALSE;
		for (j = 0; j < MAX_RECENT_FILES; j++) {
			sprintf(id, "%1d", j + 1);
			FirstArg(XtNvertSpace, 10);
			NextArg(XtNinternational, appres.international);
#ifndef XAW3D1_5E
			NextArg(XtNunderline, 0); /* underline # digit */
			entry = XtCreateWidget(id, figSmeBSBObjectClass,
					menu, Args, ArgCount);
#else
			entry = XtCreateWidget(id, smeBSBObjectClass,
					menu, Args, ArgCount);
#endif
			XtAddCallback(entry, XtNcallback, load_recent_file,
					(XtPointer)strdup(id));
			if (j < max_recent_files)
				XtManageChild(entry);
		}
	}
	for (j = 0; j < max_recent_files; j++) {
		sprintf(id, "%1d", j + 1);
		entry = XtNameToWidget(menu, id);
		if (entry != None) {
			if (j < num_recent_files) {
				FirstArg(XtNlabel, recent_files[j].name);
				NextArg(XtNsensitive, True);
				SetValues(entry);
			} else {
				FirstArg(XtNlabel, id);
				NextArg(XtNsensitive, False);
				SetValues(entry);
			}
		}
	}
}

Widget
create_menu_item(main_menu_info *menup)
{
	int	i;
	Widget	menu, entry;
	DeclareArgs(5);

	FirstArg(XtNallowShellResize, True);
	menu = XtCreatePopupShell(menup->menu_name, simpleMenuWidgetClass,
			menup->widget, Args, ArgCount);
	/* make the menu items */
	for (i = 0; menup->menu[i].name != NULL; i++) {
		if ((intptr_t) menup->menu[i].name == -1) {
			/* put in a separator line */
			FirstArg(XtNlineWidth, 2);
			(void) XtCreateManagedWidget("line", smeLineObjectClass,
					menu, Args, ArgCount);
			/* and add recently loaded files to the menu */
			rebuild_file_menu(menu);
		} else if (strcmp(menup->menu[i].name, "-") == 0) {
			/* put in a separator line */
			(void)XtCreateManagedWidget("line", smeLineObjectClass,
					menu, NULL, 0);
		} else {
			/* normal menu entry */
			FirstArg(XtNvertSpace, 10);
			NextArg(XtNinternational, False);
			/* leave space for the checkmark bitmap */
			if (menup->menu[i].checkmark) {
				NextArg(XtNleftMargin, 12);
			}
#ifndef XAW3D1_5E
			/* any underline */
			NextArg(XtNunderline, menup->menu[i].u_line);
			entry = XtCreateManagedWidget(menup->menu[i].name,
					figSmeBSBObjectClass, menu, Args,
					ArgCount);
#else
			entry = XtCreateManagedWidget(menup->menu[i].name,
					smeBSBObjectClass, menu, Args,
					ArgCount);
#endif
			XtAddCallback(entry, XtNcallback, menup->menu[i].func,
					(XtPointer)menup->widget);
		}
	}
	return menu;
}

void
setup_main_menus(void)
{
	register int		i;
	register main_menu_info	*menu;
	DeclareArgs(2);

	XDefineCursor(tool_d, XtWindow(cmd_form), arrow_cursor);

	for (i = 0; i < (int)NUM_CMD_MENUS; ++i) {
		menu = &main_menus[i];
		FirstArg(XtNfont, button_font); /* label font */
		if ( menu->menu )
			/* use menu arrow for pull-down */
			NextArg(XtNleftBitmap, menu_arrow);
		SetValues(menu->widget);
	}
}

#ifndef XAW3D1_5E
/* come here when the mouse passes over a button in the command panel */

static Widget		cmd_balloon_popup = (Widget) 0;
static XtIntervalId	balloon_id = (XtIntervalId) 0;
static Widget		balloon_w;
static XtPointer	clos;

static void cmd_balloon(Widget w, XtPointer closure, XtPointer call_data);

static void
cmd_balloon_trigger(Widget widget, XtPointer closure, XEvent *event,
			Boolean *continue_to_dispatch)
{
	if (!appres.showballoons)
		return;
	balloon_w = widget;
	clos = closure;
	/* if an old balloon is still up, destroy it */
	if ((balloon_id != 0) || (cmd_balloon_popup != (Widget) 0)) {
		cmd_unballoon(NULL, NULL, NULL, NULL);
	}
	balloon_id = XtAppAddTimeOut(tool_app, appres.balloon_delay,
			(XtTimerCallbackProc)cmd_balloon, NULL);
}

static void
cmd_balloon(Widget w, XtPointer closure, XtPointer call_data)
{
	Position	x, y;
	Dimension	wid, ht;
	main_menu_info	*menu = (main_menu_info *) clos;
	Widget		box, balloons_label;

	/* get width and height of this button */
	FirstArg(XtNwidth, &wid);
	NextArg(XtNheight, &ht);
	GetValues(balloon_w);
	/* find middle and lower edge */
	XtTranslateCoords(balloon_w, wid/2, ht+2, &x, &y);
	/* put popup there */
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	cmd_balloon_popup = XtCreatePopupShell("cmd_balloon_popup",
			overrideShellWidgetClass, tool, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNhSpace, 0);
	NextArg(XtNvSpace, 0);
	NextArg(XtNorientation, XtorientVertical);
	box = XtCreateManagedWidget("box", boxWidgetClass, cmd_balloon_popup,
			Args, ArgCount);

	/* put left/right mouse button labels as message */
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNlabel, menu->hint);
	NextArg(XtNinternational, False);
	balloons_label = XtCreateManagedWidget("label", labelWidgetClass, box,
			Args, ArgCount);

	XtPopup(cmd_balloon_popup,XtGrabNone);
	XtRemoveTimeOut(balloon_id);
	balloon_id = (XtIntervalId) 0;
}

/* come here when the mouse leaves a button in the command panel */

static void
cmd_unballoon(Widget widget, XtPointer closure, XEvent *event,
		Boolean *continue_to_dispatch)
{
	(void)widget;
	(void)event;
	(void)continue_to_dispatch;
	if (balloon_id) {
		XtRemoveTimeOut(balloon_id);
	}
	balloon_id = (XtIntervalId) 0;
	if (cmd_balloon_popup != (Widget) 0) {
		XtDestroyWidget(cmd_balloon_popup);
		cmd_balloon_popup = (Widget) 0;
	}
}
#endif /* XAW3D1_5E */

static void
enter_cmd_but(Widget widget, XtPointer closure, XEvent *event,
		Boolean *continue_to_dispatch)
{
	(void)widget;
	(void)event;
	(void)continue_to_dispatch;
	main_menu_info *menu = (main_menu_info *) closure;
	draw_mousefun(menu->hint, "", "");
}

static char	quit_msg[] = "The current figure is modified.\n"
				"Do you want to save it before quitting?";

void
quit(Widget w, XtPointer closure, XtPointer call_data)
{
	(void)closure;
	(void)call_data;
	/* turn off Compose key LED */
	setCompLED(0);

	/* don't quit if in the middle of drawing/editing */
	if (check_action_on())
		return;

	XtSetSensitive(w, False);
	/* if modified (and non-empty) ask to save first */
	if (!query_save(quit_msg)) {
		XtSetSensitive(w, True);
		return;		/* cancel, do not quit */
	}
	/* if the user hasn't saved changes to the named styles confirm */
	if (style_dirty_flag)
		if (confirm_close_style() == RESULT_CANCEL) {
			XtSetSensitive(w, True);
			return;	/* cancel, don't quit */
		}

	goodbye(False);	/* finish up and exit */
}

void goodbye(Boolean abortflag)
{
	kill_preedit();
	/* delete the cut buffer only if it is in a temporary directory */
	if (strncmp(cut_buf_name, TMPDIR, strlen(TMPDIR)) == 0)
		unlink(cut_buf_name);

	/* delete any batch print file */
	if (batch_exists)
		unlink(batch_file);

	XSync(tool_d, False);	/* https://sourceforge.net/p/mcj/tickets/54 */

	/* free all the GC's */
	free_GCs();
	/* free all the loaded X-Fonts*/
	free_Fonts();
	/*
	 * Do not free Xft fonts, because
	 * https://lists.cinelerra-cv.org/pipermail/cinelerra/2014q2/001223.html
	 *	If I understand libXft sources correctly, Xft cares
	 *	itself about font closing durig XCloseDisplay.
	 */

	XtDestroyWidget(tool);

	/* Call after free_GCs(); XftDrawDestroy() does not free unavailable GC. */
	XftDrawDestroy(main_draw);

#ifdef FREEMEM
	free_compound(&objects);
	free_compound(&saved_objects);
#endif

	/* generate a fault to cause core dump */
	if (abortflag) {
		/* go to orig_dir, in case core dumps go to the cwd */
		(void) change_directory(orig_dir);
		abort();
	}
	exit(0);
}

void
paste(Widget w, XtPointer closure, XtPointer call_data)
{
	(void)w;
	(void)closure;
	(void)call_data;
	fig_settings	settings;
	int		x,y;
	struct stat	file_status;

	/* turn off Compose key LED */
	setCompLED(0);

	/* don't paste if in the middle of drawing/editing */
	if (check_action_on())
		return;

	/* turn off anypointposn so cur_pointposn is used for pasting (user may
	 * be in a mode that doesn't use the point positioning grid */
	anypointposn = 0;

	set_cursor(wait_cursor);
	turn_off_current();
	set_mousefun("place object","place at orig posn","cancel paste",
			"place object", "place at orig posn", "cancel paste");
	/* set to paste mode */
	set_action_on();
	cur_mode = F_PASTE;

	cur_c = create_compound();
	cur_c->parent = NULL;
	cur_c->GABPtr = NULL;
	cur_c->arcs = NULL;
	cur_c->compounds = NULL;
	cur_c->ellipses = NULL;
	cur_c->lines = NULL;
	cur_c->splines = NULL;
	cur_c->texts = NULL;
	cur_c->comments = NULL;
	cur_c->next = NULL;

	/* read in the cut buf file */
	if (read_figc(cut_buf_name, cur_c, MERGE, DONT_REMAP_IMAGES, 0, 0,
				&settings) == 0) {
		compound_bound(cur_c, &cur_c->nwcorner.x, &cur_c->nwcorner.y,
				&cur_c->secorner.x, &cur_c->secorner.y);

		/* save orig coords of object */
		orig_paste_x = cur_c->nwcorner.x;
		orig_paste_y = cur_c->nwcorner.y;

		/* make it relative for mouse positioning */
		translate_compound(cur_c, -cur_c->nwcorner.x,
				-cur_c->nwcorner.y);
	} else {
		/* an error reading the .xfig file */
		if (stat(cut_buf_name, &file_status) == 0) {  /* file exists */
			file_msg("Error reading %s", cut_buf_name);
		} else if (errno == ENOENT) {
			file_msg("Cut buffer (%s) is empty", cut_buf_name);
		}
		reset_action_on();
		turn_off_current();
		set_cursor(arrow_cursor);
		free_compound(&cur_c);
		return;
	}
	/* redraw all of the pictures already on the canvas */
	canvas_ref_proc = null_proc;
	redraw_images(&objects);

	put_msg("Reading objects from \"%s\" ...Done", cut_buf_name);
	new_c=copy_compound(cur_c);
	/* add it to the depths so it is displayed */
	add_compound_depth(new_c);
	off_paste_x=new_c->secorner.x;
	off_paste_y=new_c->secorner.y;
	canvas_locmove_proc = init_move_paste_object;
	canvas_ref_proc = move_paste_object;
	canvas_leftbut_proc = place_object;
	canvas_middlebut_proc = place_object_orig_posn;
	canvas_rightbut_proc = cancel_paste;

	/* set crosshair cursor */
	set_cursor(null_cursor);

	/* get the pointer position */
	get_pointer_win_xy(&x, &y);
	/* if pasting from the command button,
	   reset coords so object is fully on canvas */
	if (x<0)
		x = 20;
	if (y<0)
		y = 20;
	/* draw the first image */
	init_move_paste_object(BACKX(x), BACKY(y));
}

static void
cancel_paste(void)
{
	reset_action_on();
	canvas_leftbut_proc = null_proc;
	canvas_middlebut_proc = null_proc;
	canvas_rightbut_proc = null_proc;
	canvas_locmove_proc = null_proc;
	canvas_ref_proc = null_proc;
	clear_mousefun();
	set_mousefun("","","", "", "", "");
	turn_off_current();
	set_cursor(arrow_cursor);
	cur_mode = F_NULL;
	paste_draw(ERASE);
	/* remove it from the depths */
	remove_compound_depth(new_c);
}

static void
paste_draw(int paint_mode)
{
	if (paint_mode==ERASE)
		redisplay_compound(new_c);
	else
		redisplay_objects(new_c);
}

static void
move_paste_object(int x, int y)
{
	int	dx,dy;
	void	(*save_canvas_locmove_proc) ();
	void	(*save_canvas_ref_proc) ();

	save_canvas_locmove_proc = canvas_locmove_proc;
	save_canvas_ref_proc = canvas_ref_proc;
	/* so we don't recurse infinitely */
	canvas_locmove_proc = null_proc;
	canvas_ref_proc = null_proc;
	paste_draw(ERASE);
	dx=x-cur_x;
	dy=y-cur_y;
	translate_compound(new_c,dx,dy);
	cur_x=x;
	cur_y=y;
	paste_draw(PAINT);
	canvas_locmove_proc = save_canvas_locmove_proc;
	canvas_ref_proc = save_canvas_ref_proc;
}

static void
init_move_paste_object(int x, int y)
{
	cur_x = x;
	cur_y = y;
	translate_compound(new_c,x,y);

	paste_draw(PAINT);
	canvas_locmove_proc = move_paste_object;
	canvas_ref_proc = move_paste_object;
}

/* button 1: paste object at current position of mouse */

static void
place_object(int x, int y, unsigned int shift)
{
	(void)x;
	(void)y;
	(void)shift;
	clean_up();
	add_compound(new_c);
	set_modifiedflag();
	redisplay_compound(new_c);
	cancel_paste();
}

/* button 2: paste object in original location whence it came */

static void
place_object_orig_posn(int x, int y, unsigned int shift)
{
	(void)shift;
	int	dx,dy;

	canvas_ref_proc = null_proc;
	paste_draw(ERASE);
	clean_up();
	/* move back to original position */
	dx = orig_paste_x-x;
	dy = orig_paste_y-y;
	translate_compound(new_c,dx,dy);
	add_compound(new_c);
	set_modifiedflag();
	redisplay_compound(new_c);
	cancel_paste();
}

void
new(Widget w, XtPointer closure, XtPointer call_data)
{
	(void)w;
	(void)closure;
	(void)call_data;
	/* turn off Compose key LED */
	setCompLED(0);

	/* don't allow if in the middle of drawing/editing */
	if (check_action_on())
		return;
	if (!emptyfigure()) {
		/* check if user wants to save figure first */
		if (query_save("The current figure is modified, "
					"do you want to save it first?")) {
			delete_all();
			strcpy(save_filename,cur_filename);
		} else {
			/* cancel new */
			return;
		}
	}
	set_action(F_LOAD);
	update_cur_filename("");
	put_msg("Immediate Undo will restore the figure");
	redisplay_canvas();
}

void
delete_all_cmd(Widget w, int closure, int call_data)
{
	(void)w;
	(void)closure;
	(void)call_data;
	/* turn off Compose key LED */
	setCompLED(0);

	/* don't allow if in the middle of drawing/editing */
	if (check_action_on())
		return;
	if (emptyfigure()) {
		put_msg("Figure already empty");
		return;
	}
	delete_all();
	put_msg("Immediate Undo will restore the figure");
	redisplay_canvas();
}

/* Toggle canvas orientation from Portrait to Landscape or vice versa */

void
change_orient()
{
	Dimension	formw, formh;
	int		dx, dy;

	/* turn off Compose key LED */
	setCompLED(0);

	/* don't change orientation if in the middle of drawing/editing */
	if (check_action_on())
		return;

	/* don't resize anything if the user specified xfig's geometry */
	if (!geomspec) {
		/* get the current size of the canvas */
		FirstArg(XtNwidth, &formw);
		NextArg(XtNheight, &formh);
		GetValues(canvas_sw);

		if (appres.landscape) {
			/* save current size for switching back */
			CANVAS_WD_LAND = CANVAS_WD;
			CANVAS_HT_LAND = CANVAS_HT;
			dx = CANVAS_WD_PORT - formw;
			dy = CANVAS_HT_PORT - formh;
			TOOL_WD += dx;
			TOOL_HT += dy;
			XtResizeWidget(tool, TOOL_WD, TOOL_HT, 0);
			resize_all((int)(CANVAS_WD_PORT),(int)(CANVAS_HT_PORT));
			appres.landscape = False;
		} else {
			/* save current size for switching back */
			CANVAS_WD_PORT = CANVAS_WD;
			CANVAS_HT_PORT = CANVAS_HT;
			dx = CANVAS_WD_LAND - formw;
			dy = CANVAS_HT_LAND - formh;
			TOOL_WD += dx;
			TOOL_HT += dy;
			XtResizeWidget(tool, TOOL_WD, TOOL_HT, 0);
			resize_all((int)(CANVAS_WD_LAND),(int)(CANVAS_HT_LAND));
			appres.landscape = True;
		}
	} else {
		/* just toggle the flag */
		appres.landscape = !appres.landscape;
	}
	/* change the printer and export orientation labels */
	FirstArg(XtNlabel, orient_items[(int)appres.landscape]);
	if (print_orient_panel)
		SetValues(print_orient_panel);
	if (export_orient_panel)
		SetValues(export_orient_panel);

	/* draw the new orientation of the page border */
	clear_canvas();
	redisplay_canvas();

	/* the figure has been modified */
	set_modifiedflag();
	if (xim_ic != NULL)
		xim_set_ic_geometry(xim_ic, CANVAS_WD, CANVAS_HT);
}

/*
 * Popup a global settings panel with:
 *
 * Widget Type     Description
 * -----------     -----------------------------------------
 * checkbutton     mouse tracking (ruler arrows)
 * checkbutton     show page borders in red
 * checkbutton     show balloons
 *   entry           balloon delay
 * checkbutton     show lengths of lines
 * checkbutton     show point numbers above polyline points
 * int entry       max image colors
 * str entry       image editor
 * str entry       spelling checker
 * str entry       html browser
 * str entry       pdf viewer
 *
 */

typedef struct _global {
	Boolean	tracking;		/* show mouse tracking in rulers */
	Boolean	autorefresh;		/* autorefresh mode */
	Boolean	show_pageborder;	/* show page borders in red on canvas */
	Boolean	showdepthmanager;	/* show depth manager panel */
	Boolean	showballoons;		/* show popup messages */
	Boolean	showlengths;		/* length/width lines */
	Boolean	shownums;		/* print point numbers  */
	Boolean	allownegcoords;	/* allow negative x/y coordinates for panning */
	Boolean	showaxislines;		/* draw lines through 0,0 */
}	globalStruct;

globalStruct	global;

void
show_global_settings(Widget w)
{
	/* turn off Compose key LED */
	setCompLED(0);

	global.tracking = appres.tracking;
	global.autorefresh = appres.autorefresh;
	global.show_pageborder = appres.show_pageborder;
	global.showdepthmanager = appres.showdepthmanager;
	global.showballoons = appres.showballoons;
	global.showlengths = appres.showlengths;
	global.shownums = appres.shownums;
	global.allownegcoords = appres.allownegcoords;
	global.showaxislines = appres.showaxislines;

	popup_global_panel(w);
}

static Widget	show_bal, delay_label;

static void
popup_global_panel(Widget w)
{
	Dimension	ht;

	if (global_popup == 0) {
		create_global_panel(w);
		XtPopup(global_popup, XtGrabNonexclusive);
		(void)XSetWMProtocols(tool_d, XtWindow(global_popup),
				&wm_delete_window, 1);
		XtUnmanageChild(delay_label);
		/* make the balloon delay label as tall as the checkbutton */
		FirstArg(XtNheight, &ht);
		GetValues(show_bal);
		FirstArg(XtNheight, ht);
		SetValues(delay_label);
		/* remanage the label */
		XtManageChild(delay_label);
		return;
	}
	XtPopup(global_popup, XtGrabNonexclusive);
}

static void
create_global_panel(Widget w)
{
	(void)w;
	DeclareArgs(11);
	Widget		beside, below, freehand, recent;
	Widget		delay_form, delay_spinner;
	Position	xposn, yposn;
	char		buf[80];

	XtTranslateCoords(tool, (Position) 0, (Position) 0, &xposn, &yposn);

	FirstArg(XtNtitle, "Xfig: Global Settings");
	NextArg(XtNtitleEncoding, XA_STRING);
	NextArg(XtNx, xposn+50);
	NextArg(XtNy, yposn+50);
	NextArg(XtNcolormap, tool_cm);
	global_popup = XtCreatePopupShell("global_settings",
			transientShellWidgetClass, tool, Args, ArgCount);
	XtOverrideTranslations(global_popup,
			XtParseTranslationTable(global_translations));
	XtAppAddActions(tool_app, global_actions, XtNumber(global_actions));

	global_panel = XtCreateManagedWidget("global_panel", formWidgetClass,
			global_popup, NULL, ZERO);

	below = CreateCheckbutton("Autorefresh figure      ", "auto_refresh",
			global_panel, NULL, NULL, MANAGE, LARGE_CHK,
			&global.autorefresh, 0,0);
	below = CreateCheckbutton("Track mouse in rulers   ", "track_mouse",
			global_panel, NULL, NULL, MANAGE, LARGE_CHK,
			&global.tracking, 0,0);
	below = CreateCheckbutton("Show page borders       ", "page_borders",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.show_pageborder, 0,0);
	below = CreateCheckbutton("Show depth manager      ", "depth_manager",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.showdepthmanager, 0,0);
	show_bal = CreateCheckbutton("Show info balloons      ","show_balloons",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.showballoons,0,0);

	/* put the delay label and spinner in a form to group them */
	FirstArg(XtNdefaultDistance, 1);
	NextArg(XtNfromHoriz, show_bal);
	NextArg(XtNfromVert, below);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	delay_form = XtCreateManagedWidget("bal_del_form", formWidgetClass,
			global_panel, Args, ArgCount);

	FirstArg(XtNlabel,"Delay (ms):");
	NextArg(XtNinternational, False);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	delay_label = beside = XtCreateManagedWidget("balloon_delay",
			labelWidgetClass, delay_form, Args, ArgCount);
	sprintf(buf, "%d", appres.balloon_delay);
	delay_spinner = MakeIntSpinnerEntry(delay_form, &bal_delay,
			"balloon_delay", NULL, beside, (XtCallbackProc)NULL,
			buf, 0, 100000, 1, 40);
	FirstArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	SetValues(delay_spinner);

	below = CreateCheckbutton("Show line lengths       ", "show_lengths",
			global_panel, show_bal, NULL, MANAGE, LARGE_CHK,
			&global.showlengths, 0, 0);
	below = CreateCheckbutton("Show vertex numbers     ", "show_vertexnums",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.shownums, 0, 0);
	below = CreateCheckbutton("Allow negative coords   ", "show_vertexnums",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.allownegcoords, 0, 0);
	below = CreateCheckbutton("Draw axis lines         ", "showaxislines",
			global_panel, below, NULL, MANAGE, LARGE_CHK,
			&global.showaxislines, 0, 0);

	FirstArg(XtNlabel, "Freehand drawing resolution");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, below);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	freehand = XtCreateManagedWidget("freehand_resolution",
			labelWidgetClass, global_panel, Args, ArgCount);
	sprintf(buf,"%d",appres.freehand_resolution);
	(void)MakeIntSpinnerEntry(global_panel, &n_freehand_resolution,
			"freehand_res", below, freehand, (XtCallbackProc)NULL,
			buf, 0, 100000, 10, 26);
	below = freehand;

	FirstArg(XtNlabel, "Recently used files        ");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, below);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	recent = XtCreateManagedWidget("recent_file_entries", labelWidgetClass,
			global_panel, Args, ArgCount);
	sprintf(buf,"%d",max_recent_files);
	(void)MakeIntSpinnerEntry(global_panel, &n_recent_files,
			"max_recent_files", below, recent, (XtCallbackProc)NULL,
			buf, 0, MAX_RECENT_FILES, 1, 26);
	below = recent;

	sprintf(buf,"%d",appres.max_image_colors);
	below = CreateLabelledAscii(&max_colors, "Maximum image colors       ",
			"max_image_colors", global_panel, below, buf, 40);
	below = CreateLabelledAscii(&image_ed, "Image editor ", "image_editor",
			global_panel, below, cur_image_editor, 340);
	below = CreateLabelledAscii(&spell_chk, "Spell checker", "spell_check",
			global_panel, below, cur_spellchk, 340);
	below = CreateLabelledAscii(&browser, "HTML Browser ", "html_browser",
			global_panel, below, cur_browser, 340);
	below = CreateLabelledAscii(&pdfview, "PDF Viewer   ", "pdf_viewer",
			global_panel, below, cur_pdfviewer, 340);

	FirstArg(XtNlabel, "Cancel");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, below);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNtop, XtChainBottom);
	NextArg(XtNbottom, XtChainBottom);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	beside = XtCreateManagedWidget("cancel", commandWidgetClass,
			global_panel, Args, ArgCount);
	XtAddEventHandler(beside, ButtonReleaseMask, False,
			(XtEventHandler)global_panel_cancel, (XtPointer)NULL);

	FirstArg(XtNlabel, "  Ok  ");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, below);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNfromHoriz, beside);
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNtop, XtChainBottom);
	NextArg(XtNbottom, XtChainBottom);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	below = XtCreateManagedWidget("global_ok", commandWidgetClass,
			global_panel, Args, ArgCount);
	XtAddEventHandler(below, ButtonReleaseMask, False,
			(XtEventHandler)global_panel_done, (XtPointer)NULL);

	/* install accelerators for the following functions */
	XtInstallAccelerators(global_panel, below);

}

/* make a label and asciiText widget to its right */

Widget
CreateLabelledAscii(Widget *text_widg, char *label, char *widg_name, Widget
		parent, Widget below, char *str, int width)
{
	DeclareArgs(11);
	Widget	lab_widg;

	FirstArg(XtNlabel, label);
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, below);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	lab_widg = XtCreateManagedWidget("label", labelWidgetClass,
			parent, Args, ArgCount);

	FirstArg(XtNstring, str);
	NextArg(XtNinternational, False);
	NextArg(XtNinsertPosition, strlen(str));
	NextArg(XtNeditType, XawtextEdit);
	NextArg(XtNfromVert, below);
	NextArg(XtNfromHoriz, lab_widg);
	NextArg(XtNwidth, width);
	NextArg(XtNtop, XtChainTop);
	NextArg(XtNbottom, XtChainTop);
	NextArg(XtNleft, XtChainLeft);
	NextArg(XtNright, XtChainLeft);
	*text_widg = XtCreateManagedWidget(widg_name, asciiTextWidgetClass,
			parent, Args, ArgCount);
	/* install "standard" translations */
	XtOverrideTranslations(*text_widg,
			XtParseTranslationTable(text_translations));
	return lab_widg;
}

static void
global_panel_done(Widget w, XButtonEvent *ev)
{
	(void)w;
	(void)ev;
	Boolean	asp, gsp, adz, gdz;
	int	temp;
	char	buf[80];

	/* copy all new values back to masters */
	appres.tracking = global.tracking;
	if (appres.autorefresh && !global.autorefresh)
		cancel_autorefresh();
	else if (!appres.autorefresh && global.autorefresh)
		set_autorefresh();
	appres.autorefresh = global.autorefresh;
	asp = appres.show_pageborder;
	gsp = global.show_pageborder;
	adz = appres.showaxislines;
	gdz = global.showaxislines;
	/* update settings */
	appres.show_pageborder = gsp;
	appres.showaxislines = gdz;

	/* if show_pageborder or showaxislines WAS on and is now off, redraw */
	if ((asp && !gsp) || (adz && !gdz)) {
		/* was on, turn off */
		clear_canvas();
		redisplay_canvas();
	} else if ((!asp && gsp) || (!adz && gdz)) {
		/* if show_pageborder or showaxislines WAS off and is now on,
		   draw them */
		redisplay_pageborder();
	}
	/* see if user toggled the depth manager setting */
	if (appres.showdepthmanager != global.showdepthmanager)
		toggle_show_depths();
	appres.showdepthmanager = global.showdepthmanager;
	appres.showballoons = global.showballoons;
	temp = atoi(panel_get_value(bal_delay));
	if (temp < 0) {
		temp = 0;
		panel_set_int(bal_delay, temp);
	}
	appres.balloon_delay = temp;
	appres.showlengths = global.showlengths;
	appres.shownums = global.shownums;
	appres.allownegcoords = global.allownegcoords;
	appres.showaxislines = global.showaxislines;
	/* go to 0,0 if user turned off neg coords and we're in the negative */
	if (!appres.allownegcoords)
		if (zoomxoff < 0 || zoomyoff < 0)
			pan_origin();

	/* get the freehand resolution spinner value */
	temp = atoi(panel_get_value(n_freehand_resolution));
	if (temp < 0) {
		temp = 0;
		panel_set_int(n_freehand_resolution, temp);
	}
	appres.freehand_resolution = temp;

	/* get the number of recent files spinner value */
	temp = atoi(panel_get_value(n_recent_files));
	if (temp > MAX_RECENT_FILES)
		temp = MAX_RECENT_FILES;
	else if (temp < 0)
		temp = 0;
	panel_set_int(n_recent_files, temp);
	/* if number of recent files has changed,
	   update it in the .xfigrc file */
	if (max_recent_files != temp) {
		Widget	menu, entry;
		int	i;
		char	id[10];

		menu = main_menus[0].menuwidget;
		max_recent_files = temp;
		num_recent_files = min2(num_recent_files, max_recent_files);
		sprintf(buf,"%d",max_recent_files);
		update_xfigrc("max_recent_files", buf);
		for (i=0; i<MAX_RECENT_FILES; i++) {
			sprintf(id, "%1d", i + 1);
			entry = XtNameToWidget(menu, id);
			if (i < max_recent_files) {
				XtManageChild(entry);
				/* if new entries created, clear them */
				if (i >= num_recent_files) {
					FirstArg(XtNlabel, id);
					NextArg(XtNsensitive, False);
					SetValues(entry);
				}
			} else {
				XtUnmanageChild(entry);
			}
		}
		/* remanage menu so it resizes */
		XtUnmanageChild(main_menus[0].widget);
		XtManageChild(main_menus[0].widget);
	}
	temp = atoi(panel_get_value(max_colors));
	if (temp <= 0) {
		temp = 10;
		panel_set_int(max_colors, temp);
	}
	appres.max_image_colors = temp;
	strcpy(cur_image_editor, panel_get_value(image_ed));
	strcpy(cur_spellchk, panel_get_value(spell_chk));
	strcpy(cur_browser, panel_get_value(browser));
	strcpy(cur_pdfviewer, panel_get_value(pdfview));

	XtPopdown(global_popup);

	refresh_view_menu();
}

static void
global_panel_cancel(Widget w, XButtonEvent *ev)
{
	(void)w;
	(void)ev;
	XtDestroyWidget(global_popup);
	global_popup = (Widget) 0;
}

#ifndef XAW3D1_5E
/* come here when the mouse passes over the filename window */

static Widget		filename_balloon_popup = (Widget) 0;
static XtIntervalId	fballoon_id = (XtIntervalId) 0;
static Widget		fballoon_w;

static void		file_balloon(void);

static void
filename_balloon_trigger(Widget widget, XtPointer closure, XEvent *event,
		Boolean *continue_to_dispatch)
{
	if (!appres.showballoons)
		return;
	fballoon_w = widget;
	fballoon_id = XtAppAddTimeOut(tool_app, appres.balloon_delay,
			(XtTimerCallbackProc)file_balloon, (XtPointer)NULL);
}

static void
file_balloon(void)
{
	Position	x, y;
	Dimension	w, h;
	Widget		box, balloons_label;

	/* get width and height of this window */
	FirstArg(XtNwidth, &w);
	NextArg(XtNheight, &h);
	GetValues(fballoon_w);
	/* find center and lower edge */
	XtTranslateCoords(fballoon_w, w/2, h+2, &x, &y);

	/* put popup there */
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	filename_balloon_popup = XtCreatePopupShell("filename_balloon_popup",
				overrideShellWidgetClass, tool, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNhSpace, 0);
	NextArg(XtNvSpace, 0);
	box = XtCreateManagedWidget("box", boxWidgetClass,
				filename_balloon_popup, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNlabel, "Current filename");
	NextArg(XtNinternational, False);
	balloons_label = XtCreateManagedWidget("label", labelWidgetClass,
				box, Args, ArgCount);
	XtPopup(filename_balloon_popup,XtGrabNone);
}

/* come here when the mouse leaves the filename window */

static void
filename_unballoon(Widget widget, XtPointer closure, XEvent *event, Boolean
		*continue_to_dispatch)
{
	if (fballoon_id)
		XtRemoveTimeOut(fballoon_id);
	fballoon_id = (XtIntervalId) 0;
	if (filename_balloon_popup != (Widget) 0) {
		XtDestroyWidget(filename_balloon_popup);
		filename_balloon_popup = (Widget) 0;
	}
}
#endif /* XAW3D1_5E */

/*
 * Update the current filename in the name_panel widget, and the xfig icon.
 * Also update the current filename in the File popup (if it has been created).
 */

void
update_cur_filename(char *newname)
{
	if (cur_filename!=newname) strcpy(cur_filename,newname);
	/* store the new filename in the name_panel widget */
	FirstArg(XtNlabel, newname);
	SetValues(name_panel);
	if (cfile_text)    /* if the name widget in the file popup exists... */
		SetValues(cfile_text);

	/* put the filename being edited in the icon */
	XSetIconName(tool_d, tool_w, xf_basename(cur_filename));

	update_def_filename();    /* update default filename in export panel */
	update_wm_title(cur_filename);	/* and window title bar */
}

static void
popup_menu(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
	(void)w;
	(void)event;
	size_t	which;

	which = locate_menu(params, nparams);
	if (which < 0)
		return;
	XtPopupSpringLoaded(main_menus[which].menuwidget);
}

static void
place_menu(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
	(void)w;
	(void)event;
	Position	x, y;
	Dimension	height;
	size_t		which;

	which = locate_menu(params, nparams);
	if (which < 0)
		return;
	/* get the height of the menu button on the command panel */
	FirstArg(XtNheight, &height);
	GetValues(main_menus[which].widget);
	XtTranslateCoords(main_menus[which].widget, (Position)0, height+4,
			&x, &y);
	/* position the popup menu just under the button */
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	SetValues(main_menus[which].menuwidget);
}

static size_t
locate_menu(String *params, Cardinal *nparams)
{
	size_t		which;

	if (*nparams < 1)
		return -1;

	/* find which menu the user wants */
	for (which = 0; which < NUM_CMD_MENUS; ++which)
		if (strcmp(params[0], main_menus[which].menu_name) == 0)
			break;
	if (which >= NUM_CMD_MENUS)
		return -1;
	return which;
}

/* callback to load a recently used file (from the File menu) */

static void
load_recent_file(Widget w, XtPointer client_data, XtPointer call_data)
{
	(void)w;
	(void)call_data;
	int		 which = atoi((char *) client_data);
	char	*filename;
	char	 dir[PATH_MAX], *c;

	filename = recent_files[which-1].name+2;  /* point past number, space */
	/* if file panel is open, popdown it */
	if (file_up)
		file_panel_dismiss();

	/* check if modified, unsaved figure on canvas */
	if (!query_save(quit_msg))
		return;
	/* go to the directory where the figure is in case
	   it has imported pictures */
	strcpy(dir,filename);
	if ((c=strrchr(dir,'/'))) {
		*c = '\0';			/* terminate dir at last '/' */
		change_directory(dir);
		strcpy(cur_file_dir, dir);	/* update current directory */
		strcpy(cur_export_dir, dir);  /* and export current directory */
	}
	/* load the file */
	(void) load_file(filename, 0, 0);
}

/* this one is called by the accelerator (File) 1/2/3... */

void
acc_load_recent_file(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
	(void)event;
	(void)nparams;
	/* get file number from passed arg */
	int		which = atoi(*params);

	/* turn off Compose key LED */
	setCompLED(0);

	/* see if that file exists in the list */
	if (which > num_recent_files)
		return;

	/* now load by calling the callback */
	load_recent_file(w, (XtPointer)*params, (XtPointer)NULL);
}

/*
 * Get the font dimensions for display in the current charmap.
 * Allocates a new XftFont, free it (closefont() or XftFontClose()) after use.
 */
static void
charmap_font_dimensions(int psflag, int font,
		XftFont **xftfont, int *width, int *height, int *x, int *y)
{
	int	ascent, descent;

	*xftfont = getfont(psflag, font, CMAP_FONTSIZE, 0);
	textmaxheight(psflag, font, CMAP_FONTSIZE, &ascent, &descent);
	ascent /= ZOOM_FACTOR;
	descent /= ZOOM_FACTOR;
	*x = textlength(*xftfont, (XftChar8 *)"W", 1);
	*height = max2((ascent + descent + 1) / 2, 6);
	*height += ascent + descent;
	*width = max2((*x + 1) / 2, 4);
	*width += *x;
	*x = (*width - *x) / 2;
	*y = (*height + ascent + descent + 1) / 2 - descent;
}


/*
 * refresh character map (e.g. when user changes font) by changing
 * the font label and font in the buttons
 */

void
refresh_character_panel(int ps_sel, int font_sel)
{
	Boolean		resize = True;
	int		width, height;
	int		w, h;
	int		x, y;
	unsigned     i;
	char	     fname[80];
	XftFont    *work_xftfont;
	XftColor	xft_bg;
	XColor		x_bg;

	ps_sel = ps_sel ? 1 : 0;
	if (!character_map_popup ||
			(charmap_psflag == ps_sel && charmap_font == font_sel))
		return;
	charmap_psflag = ps_sel;
	charmap_font = font_sel;

	sprintf(fname, "%s font characters:",
			ps_sel ? ps_fontinfo[font_sel + 1].name :
					latex_fontinfo[font_sel].name);
	/* change font name label */
	FirstArg(XtNlabel, fname);
	SetValues(charmap_font_label);

	FirstArg(XtNwidth, &w);
	NextArg(XtNheight, &h);
	NextArg(XtNbackground, &x_bg.pixel);
	GetValues(charcell[33]);

	/* get the background color */
	XQueryColor(tool_d, tool_cm, &x_bg);
	xtoxftcolor(&xft_bg, &x_bg);

	/* and the (possibly) new dimensions */
	charmap_font_dimensions(charmap_psflag, charmap_font,
			&work_xftfont, &width, &height, &x, &y);
	if (w == width && h == height) {
		resize = False;
	} else {
		XtUnmapWidget(character_map_panel);
		/* inhibit resizing of the parent as long as children change */
		XawFormDoLayout(character_map_panel, False);
		/* re-use the arguments further below */
		if (w == width) {
			FirstArg(XtNheight, height);
		 } else {
			FirstArg(XtNwidth, width);
			if (h != height)
				NextArg(XtNheight, height);
		 }
	}

	for (i = 32; i <= LASTCHAR; ++i) {
		if (resize) {
			SetValues(charcell[i]);
			XftDrawDestroy(xftdraw[i]);
		}
		if (i == STARTGAP)
			i += MISSING;
	}

	if (resize) {
		XawFormDoLayout(character_map_panel, True);
		XtMapWidget(character_map_panel);
		app_flush();
	}

	/* first make sure the widgets are available in the correct size,
	 * only then paint on them (client-side) with Xft fonts */
	for (i = 32; i <= LASTCHAR; ++i) {
		if (resize)
			xftdraw[i] = XftDrawCreate(tool_d,
					XtWindow(charcell[i]), tool_v, tool_cm);
		XftDrawRect(xftdraw[i], &xft_bg,
				0, 0, (unsigned)width, (unsigned)height);
		XftDrawString8(xftdraw[i], xftcolor + BLACK, work_xftfont,
				x, y, (FcChar8 *)&i, 1);
		if (i == STARTGAP)
			i += MISSING;
	}

	XftFontClose(tool_d, work_xftfont);
}

static void
character_panel_close(void)
{
	int	i;
	XtDestroyWidget(character_map_popup);
	character_map_popup = (Widget) 0;
	for (i = 32; i <= LASTCHAR; ++i) {
		XftDrawDestroy(xftdraw[i]);
		if (i == STARTGAP)
			i += MISSING;
	}
}

/*
 * popup a window showing the symbol character map, each char in a
 * different button widget so the user paste directly into text
 * Activated from the View/Character Map menu
 */

void
popup_character_map(void)
{
	Widget		beside, below;
	int		i;
	int		vertDist;
	int		width, height;
	int		x, y;
	char		fname[80];
	static Boolean	actions_added=False;
	XftFont		*work_xftfont;
	char		translations[] =
		"<EnterWindow>: highlight()\n"
		"<LeaveWindow>: reset()\n";

	/* only allow one copy */
	if (character_map_popup)
		return;

	FirstArg(XtNtitle, "Xfig: Character Map");
	NextArg(XtNtitleEncoding, XA_STRING);
	NextArg(XtNallowShellResize, True);
	NextArg(XtNcolormap, tool_cm);
	character_map_popup = XtCreatePopupShell("character_map_popup",
			transientShellWidgetClass,
			tool, Args, ArgCount);
	XtOverrideTranslations(character_map_popup,
			XtParseTranslationTable(charmap_translations));
	if (!actions_added) {
		XtAppAddActions(tool_app, charmap_actions,
				XtNumber(charmap_actions));
		actions_added = True;
	}

	character_map_panel = XtCreateManagedWidget("character_map_panel",
			formWidgetClass, character_map_popup, NULL, ZERO);

	charmap_psflag = using_ps ? 1 : 0;
	charmap_font = charmap_psflag ? cur_ps_font : cur_latex_font;
	sprintf(fname, "%s font characters:",
			charmap_psflag ? ps_fontinfo[cur_ps_font+1].name :
			latex_fontinfo[cur_latex_font].name);
	FirstArg(XtNlabel, fname);
	NextArg(XtNinternational, False);
	NextArg(XtNborderWidth, 0);
	charmap_font_label = below = XtCreateManagedWidget("charmap_font_label",
			labelWidgetClass, character_map_panel, Args, ArgCount);

	/* get the font and font dimensions */
	charmap_font_dimensions(charmap_psflag, charmap_font,
			&work_xftfont, &width, &height, &x, &y);
	vertDist = height / 6;

	beside = (Widget) 0;

	/*
	 * The sync is especially necessary for the XftDraw() call further
	 * below, otherwise glyphs do not appear.
	 * If placed here, the first cells (usually the digits) seem to appear
	 * more reliably then if the XSyncOn() call is placed further below
	 * just before the XDraw..() loop.
	 */
	XSyncOn();	/* without sync, the glyphs do not appear */
	for (i = 32; i <= LASTCHAR; ++i) {
		ptr_int		p;
		p.val = i;
		/*
		 * Add an event handler for  each character to paste that char.
		 * Using the callback of the CommandWidget did not work, because
		 * the default translation table contain actions (set(),
		 * unset()) that invert and revert the background of the cell.
		 * This process effectively erases the glyph painted with xft
		 * onto the command widget. The default translation table is
		 * "<EnterWindow>: highlight()\n  <LeaveWindow>: reset()\n\
		 *  <Btn1Down>:set()\n  <Btn1Up>: notify() unset()\n".
		 *  Modifying the translation table to not contain the set() and
		 *  unset() actions also did not work, because then the callback
		 *  was not invoked. One could directly invoke the action, but
		 *  an event handler is simpler.
		 */
		FirstArg(XtNlabel, "");
		NextArg(XtNtranslations, XtParseTranslationTable(translations));
		NextArg(XtNresizable, True);
		NextArg(XtNwidth, width);
		NextArg(XtNheight, height);
		NextArg(XtNfromVert, below);
		NextArg(XtNvertDistance, vertDist);
		NextArg(XtNfromHoriz, beside);
		beside = charcell[i] = XtCreateManagedWidget("char_button",
				commandWidgetClass, character_map_panel,
				Args, ArgCount);
		XtAddEventHandler(beside, ButtonPressMask, False,
				/* ButtonReleaseMask did not work */
				paste_character, p.ptr);
		XtAddEventHandler(beside, EnterWindowMask, False,
				redraw_character, p.ptr);
		/* skip empty entries */
		if (i == STARTGAP) {
			below = beside;
			beside = (Widget) 0;
			i += MISSING;
		} else if ((i+1)%16 == 0 && i != LASTCHAR) {
			below = beside;
			beside = (Widget) 0;
		}
	}

	/* close button */
	FirstArg(XtNlabel, "Close");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, beside);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	close_but = XtCreateManagedWidget("close", commandWidgetClass,
			character_map_panel, Args, ArgCount);
	XtAddEventHandler(close_but, ButtonReleaseMask, False,
			(XtEventHandler)character_panel_close, (XtPointer)NULL);
	/* redraw button */
	FirstArg(XtNlabel, "Redraw");
	NextArg(XtNinternational, False);
	NextArg(XtNfromVert, beside);
	NextArg(XtNfromHoriz, close_but);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	beside = XtCreateManagedWidget("redraw", commandWidgetClass,
			character_map_panel, Args, ArgCount);
	XtAddEventHandler(beside, ButtonReleaseMask, False,
			redraw_all, (XtPointer) NULL);

	XtPopup(character_map_popup, XtGrabNone);

	(void)XSetWMProtocols(tool_d, XtWindow(character_map_popup),
			&wm_delete_window, 1);

	for (i = 32; i <= LASTCHAR; ++i) {
		xftdraw[i] = XftDrawCreate(tool_d, XtWindow(charcell[i]),
				tool_v, tool_cm);
		XftDrawString8(xftdraw[i], xftcolor + BLACK, work_xftfont,
				x, y, (FcChar8 *)&i, 1);
		if (i == STARTGAP)
			i += MISSING;
	}
	XSyncOff();
	XftFontClose(tool_d, work_xftfont);
}

static void
paste_character(Widget w, XtPointer client_data, XEvent *ev, Boolean *pass)
{
	(void)w;
	(void)ev;
	(void)pass;

	int		len;
	ptr_int		i = {client_data};
	FcChar8		str[FC_UTF8_MAX_LEN];

	/* only allow during text input */
	if (canvas_kbd_proc != (void (*)())char_handler)
		return;

	len = FcUcs4ToUtf8((FcChar32)i.val, str);
	char_handler(str, len, (KeySym) 0);
}

static void
redraw_character(Widget w, XtPointer client_data, XEvent *ev, Boolean *pass)
{
	(void)w;
	(void)ev;
	(void)pass;

	int		x, y, width, height;
	ptr_int		i = {client_data};
	XftFont		*work_xftfont;

	charmap_font_dimensions(charmap_psflag, charmap_font,
			&work_xftfont, &width, &height, &x, &y);
	XftDrawString8(xftdraw[i.val], xftcolor + BLACK, work_xftfont, x, y,
			(FcChar8 *)&i, 1);
}

static void
redraw_all(Widget w, XtPointer client_data, XEvent *ev, Boolean *pass)
{
	(void)w;
	(void)client_data;
	(void)ev;
	(void)pass;

	int		i;
	int		x, y, width, height;
	XftFont		*work_xftfont;
	XftColor	xft_bg;
	XColor		x_bg;

	FirstArg(XtNbackground, &x_bg.pixel);
	GetValues(charcell[33]);

	/* get the background color */
	XQueryColor(tool_d, tool_cm, &x_bg);
	xtoxftcolor(&xft_bg, &x_bg);

	charmap_font_dimensions(charmap_psflag, charmap_font,
			&work_xftfont, &width, &height, &x, &y);
	/*
	 * Resizing the window might have broken the layout of the character map
	 * panel. Therefore, tear down and bring up again.
	 */
	character_panel_close();
	popup_character_map();

	for (i = 32; i <= LASTCHAR; ++i) {
		XftDrawRect(xftdraw[i], &xft_bg,
				0, 0, (unsigned)width, (unsigned)height);
		XftDrawString8(xftdraw[i], xftcolor + BLACK, work_xftfont,
				x, y, (FcChar8 *)&i, 1);
		if (i == STARTGAP)
			i += MISSING;
	}
}

/* add or remove a checkmark to a menu entry to show that it
   is selected or unselected respectively */

static void
refresh_view_menu_item(char *name, Boolean state)
{
	Widget	menu, w;
	Pixmap	bitmap;
	DeclareStaticArgs(10);

	menu = XtNameToWidget(tool, "*viewmenu");
	if (menu == NULL) {
		fprintf(stderr, "xfig: refresh_view_menu: can't find *viewmenu\n");
	} else {
		w = XtNameToWidget(menu, name);
		if (w == NULL) {
			fprintf(stderr, "xfig: can't find \"viewmenu%s\"\n",
					name);
		} else {
			if (state) {
				if (menu_item_bitmap == None)
					menu_item_bitmap = XCreateBitmapFromData(XtDisplay(menu),
							RootWindowOfScreen(XtScreen(menu)),
							(char *)menu_item_bitmap_bits,
							menu_item_bitmap_width,
							menu_item_bitmap_height);
				bitmap = menu_item_bitmap;
			} else {
				bitmap = None;
			}
			FirstArg(XtNleftBitmap, bitmap);
			SetValues(w);
		}
	}
}

/* update the menu entries with or without an asterisk */

void
refresh_view_menu(void)
{
#ifdef XAW3D1_5E
	int			i;
	register main_menu_info	*menu;
#endif /* XAW3D1_5E */
	/* turn off Compose key LED */
	setCompLED(0);

	refresh_view_menu_item(PAGE_BRD_MSG, appres.show_pageborder);
	refresh_view_menu_item(DPTH_MGR_MSG, appres.showdepthmanager);
	refresh_view_menu_item(INFO_BAL_MSG, appres.showballoons);
	refresh_view_menu_item(LINE_LEN_MSG, appres.showlengths);
	refresh_view_menu_item(VRTX_NUM_MSG, appres.shownums);
	refresh_view_menu_item(AUTO_RFS_MSG, appres.autorefresh);

#ifdef XAW3D1_5E
	if (appres.showballoons) {
		XawTipEnable(name_panel, "Current filename");
		for (i = 0; i < (int)NUM_CMD_MENUS; ++i) {
			menu = &main_menus[i];
			XawTipEnable(menu->widget, menu->hint);
		}
	} else {
		XawTipDisable(name_panel);
		for (i = 0; i < (int)NUM_CMD_MENUS; ++i) {
			menu = &main_menus[i];
			XawTipDisable(menu->widget);
		}
	}
	update_indpanel(cur_indmask);
	update_modepanel();
	update_layerpanel();
	update_mousepanel();
	update_rulerpanel();
#endif /* XAW3D1_5E */
}

/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1989-2000 by Brian V. Smith
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
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "d_text.h"
#include "e_flip.h"
#include "e_rotate.h"
#include "u_search.h"
#include "w_drawprim.h"
#include "w_library.h"
#include "w_indpanel.h"
#include "w_util.h"
#include "w_mousefun.h"
#include "w_setup.h"
#include "d_spline.h"

/* it is easier to put these extern declarations here than to create 
   all the d_*.h and e_*.h files to do it */

extern void	circlebyradius_drawing_selected();
extern void	circlebydiameter_drawing_selected();
extern void	ellipsebyradius_drawing_selected();
extern void	ellipsebydiameter_drawing_selected();
extern void	box_drawing_selected();
extern void	arcbox_drawing_selected();
extern void	line_drawing_selected();
extern void	regpoly_drawing_selected();
extern void	picobj_drawing_selected();
extern void	text_drawing_selected();
extern void	arc_drawing_selected();
extern void	align_selected();
extern void	compound_selected();
extern void	open_compound();
extern void	break_selected();
extern void	join_split_selected();
extern void	scale_selected();
extern void	point_adding_selected();
extern void	delete_point_selected();
extern void	sel_place_lib_obj();
extern void	move_selected();
extern void	move_point_selected();
extern void	delete_selected();
extern void	copy_selected();
extern void	rotate_cw_selected();
extern void	rotate_ccw_selected();
extern void	flip_ud_selected();
extern void	flip_lr_selected();
extern void	convert_selected();
extern void	arrow_head_selected();
extern void	edit_item_selected();
extern void	update_selected();

/* locals */

static void	stub_circlebyradius_drawing_selected();
static void	stub_circlebydiameter_drawing_selected();
static void	stub_ellipsebyradius_drawing_selected();
static void	stub_ellipsebydiameter_drawing_selected();
static void	stub_box_drawing_selected();
static void	stub_arcbox_drawing_selected();
static void	stub_line_drawing_selected();
static void	stub_poly_drawing_selected();
static void	stub_regpoly_drawing_selected();
static void	stub_picobj_drawing_selected();
static void	stub_text_drawing_selected();
static void	stub_arc_drawing_selected();
static void	stub_spline_drawing_selected();
static void	stub_cl_spline_drawing_selected();
static void	stub_intspline_drawing_selected();
static void	stub_cl_intspline_drawing_selected();
static void	stub_align_selected();
static void	stub_compound_selected();
static void	stub_break_selected();
static void	stub_open_compound_selected();
static void	stub_join_split_selected();
static void	stub_scale_selected();
static void	stub_point_adding_selected();
static void	stub_delete_point_selected();
static void	stub_move_selected();
static void	stub_popup_library();
static void	stub_move_point_selected();
static void	stub_delete_selected();
static void	stub_copy_selected();
static void	stub_rotate_cw_selected();
static void	stub_rotate_ccw_selected();
static void	stub_flip_ud_selected();
static void	stub_flip_lr_selected();
static void	stub_convert_selected();
static void	stub_arrow_head_selected();
static void	stub_edit_item_selected();
static void	stub_update_selected();
static void	stub_enter_mode_btn();

/**************	    local variables and routines   **************/

#define MAX_MODEMSG_LEN 80
typedef struct mode_switch_struct {
    icon_struct	   *icon;
    int		    mode;
    void	    (*setmode_func) ();
    int		    objmask;
    unsigned long   indmask;		/* mask to display indicators for this func */
    char	    modemsg[MAX_MODEMSG_LEN];
    Boolean	    popup;		/* true for commands that popup something */
    Widget	    widget;
    Pixmap	    pixmap, reversePM;
}               mode_sw_info;

DeclareStaticArgs(13);
/* pointer to current mode switch */
static mode_sw_info *current = NULL;

/* button selection event handler */
static void     sel_mode_but();

/* popup message over button when mouse enters it */
static void     mode_balloon_trigger();
static void     mode_unballoon();

/* popdown message */
static void     turn_on();

/* The M_XXX indicate which objects are selectable when the mode is on, and
   the I_XXX say which indicator buttons will appear on the indicator panel */

/* IMPORTANT NOTE:  The "popup" boolean must be True for those commands which 
   may popup a window.  Because the command button is set insensitive in those
   cases, the LeaveWindow event never happens on that button so the balloon popup
   would never be destroyed in that case.  */

static mode_sw_info mode_switches[] = {

    /* DRAWING MODES */

    {&cirrad_ic, F_CIRCLE_BY_RAD, circlebyradius_drawing_selected, M_NONE,
       I_CIRCLE,
       "CIRCLE drawing: specify RADIUS   (c)",
       False},
    {&cirdia_ic, F_CIRCLE_BY_DIA, circlebydiameter_drawing_selected, M_NONE,
       I_CIRCLE,
       "CIRCLE drawing: specify DIAMETER   (Shift-c)",
       False},
    {&ellrad_ic, F_ELLIPSE_BY_RAD, ellipsebyradius_drawing_selected, M_NONE,
       I_ELLIPSE,
       "ELLIPSE drawing: specify RADII   (e)",
       False},
    {&elldia_ic, F_ELLIPSE_BY_DIA, ellipsebydiameter_drawing_selected, M_NONE,
       I_ELLIPSE,
       "ELLIPSE drawing: specify DIAMETERS   (Shift-e)",
       False},
    {&c_spl_ic, F_CLOSED_APPROX_SPLINE, spline_drawing_selected, M_NONE,
       I_CLOSED,
       "CLOSED APPROXIMATED SPLINE drawing: specify control points   (Shift-s)",
       False},
    {&spl_ic, F_APPROX_SPLINE, spline_drawing_selected, M_NONE,
       I_OPEN,
       "APPROXIMATED SPLINE drawing: specify control points   (s)",
       False},
    {&c_intspl_ic, F_CLOSED_INTERP_SPLINE, spline_drawing_selected, M_NONE,
       I_CLOSED,
       "CLOSED INTERPOLATED SPLINE drawing: specify control points   (Shift-i)",
       False},
    {&intspl_ic, F_INTERP_SPLINE, spline_drawing_selected, M_NONE,
       I_OPEN,
       "INTERPOLATED SPLINE drawing: specify control points   (i)",
       False},
    {&polygon_ic, F_POLYGON, line_drawing_selected, M_NONE,
       I_CLOSED,
       "POLYGON drawing   (p)",
       False},
    {&line_ic, F_POLYLINE, line_drawing_selected, M_NONE,
       I_LINE,
       "POLYLINE drawing   (l)",
       False},
    {&box_ic, F_BOX, box_drawing_selected, M_NONE,
       I_BOX,
       "Rectangular BOX drawing   (b)",
       False},
    {&arc_box_ic, F_ARC_BOX, arcbox_drawing_selected, M_NONE,
       I_ARCBOX,
       "Rectangular BOX drawing with ROUNDED CORNERS   (Shift-b)",
       False},
    {&regpoly_ic, F_REGPOLY, regpoly_drawing_selected, M_NONE,
       I_REGPOLY,
       "Regular Polygon   (Shift-p)",
       False},
    {&arc_ic, F_CIRCULAR_ARC, arc_drawing_selected, M_NONE,
       I_ARC,
       "ARC drawing: specify three points on the arc   (r)",
       False},
    {&picobj_ic, F_PICOBJ, picobj_drawing_selected, M_NONE,
       I_PICOBJ,
       "Picture Object   (Ctrl-p)",
       True},		/* popups a panel */
    {&text_ic, F_TEXT, text_drawing_selected, M_TEXT_NORMAL,
       I_TEXT,
       "TEXT input from keyboard   (t)",
       False},
    {&library_ic, F_PLACE_LIB_OBJ, sel_place_lib_obj, M_NONE,
       I_MIN2,
       "PLACE a library element   (Shift-l)",
       True},		/* popups a panel */

      /* EDITING MODES FOLLOW */

    {&glue_ic, F_GLUE, compound_selected, M_ALL,
       I_MIN2,
       "GLUE objects into COMPOUND object   (g)",
       False},
    {&break_ic, F_BREAK, break_selected, M_COMPOUND,
       0,
       "BREAK COMPOUND object   (Shift-g)",
       False},
    {&open_comp_ic, F_ENTER_COMP, open_compound, M_COMPOUND,
       0,
       "OPEN COMPOUND object   (o)",
       False},
    {&join_split_ic, F_JOIN, join_split_selected, M_POLYLINE | M_SPLINE_O |M_SPLINE_C,
       0,
       "Join or Split lines/splines   (j)",
       False},
    {&scale_ic, F_SCALE, scale_selected, M_NO_TEXT,
       I_MIN2,
       "SCALE objects   (Ctrl-s)",
       False},
    {&align_ic, F_ALIGN, align_selected, M_COMPOUND,
       I_ALIGN,
       "ALIGN objects within a COMPOUND or to CANVAS   (a)",
       False},
    {&movept_ic, F_MOVE_POINT, move_point_selected, M_NO_TEXT,
       I_ADDMOVPT,
       "MOVE POINTs   (Shift-m)",
       False},
    {&move_ic, F_MOVE, move_selected, M_ALL,
       I_MIN3,
       "MOVE objects   (m)",
       False},
    {&addpt_ic, F_ADD_POINT, point_adding_selected, M_VARPTS_OBJECT,
       I_ADDMOVPT,
       "ADD POINTs to lines, polygons and splines   (Ctrl-a)",
       False},
    {&copy_ic, F_COPY, copy_selected, M_ALL,
       I_COPY,
       "COPY objects  (Ctrl-c)",
       False},
    {&deletept_ic, F_DELETE_POINT, delete_point_selected, M_VARPTS_OBJECT,
       0,
       "DELETE POINTs from lines, polygons and splines   (Shift-d)",
       False},
    {&delete_ic, F_DELETE, delete_selected, M_ALL,
       0,
       "DELETE objects   (d)",
       False},
    {&update_ic, F_UPDATE, update_selected, M_ALL,
       I_OBJECT,
       "UPDATE object <-> current settings   (u)",
       False},
    {&edit_ic, F_EDIT, edit_item_selected, M_ALL,
       0,
       "CHANGE OBJECT via EDIT panel   (Ctrl-e)",
       False},
    {&flip_y_ic, F_FLIP, flip_ud_selected, M_NO_TEXT,
       I_MIN2,
       "FLIP objects up or down   (Shift-f)",
       False},
    {&flip_x_ic, F_FLIP, flip_lr_selected, M_NO_TEXT,
       I_MIN2,
       "FLIP objects left or right   (f)",
       False},
    {&rotCW_ic, F_ROTATE, rotate_cw_selected, M_ALL,
       I_ROTATE,
       "ROTATE objects clockwise   (Ctrl-r)",
       False},
    {&rotCCW_ic, F_ROTATE, rotate_ccw_selected, M_ALL,
       I_ROTATE,
       "ROTATE objects counter-clockwise   (Shift-r)",
       False},
    {&convert_ic, F_CONVERT, convert_selected, M_VARPTS_OBJECT | M_POLYLINE_BOX, 
       0,
       "CONVERSION between lines, polygons and splines   (v)",
       False},
    {&autoarrow_ic, F_AUTOARROW, arrow_head_selected, M_OPEN_OBJECT,
       I_ADD_DEL_ARROW,
       "ADD/DELETE ARROWHEADS   (Shift-a)",
       False},
};

int	NUM_MODE_SW = (sizeof(mode_switches) / sizeof(mode_sw_info));

static Arg      button_args[] =
{
     /* 0 */ {XtNlabel, (XtArgVal) "    "},
     /* 1 */ {XtNwidth, (XtArgVal) 0},
     /* 2 */ {XtNheight, (XtArgVal) 0},
     /* 3 */ {XtNresizable, (XtArgVal) False},
     /* 4 */ {XtNborderWidth, (XtArgVal) 0},
     /* 5 */ {XtNresize, (XtArgVal) False},	/* keeps buttons from being
						 * resized when there are not
						 * a multiple of three of
						 * them */
     /* 6 */ {XtNbackgroundPixmap, (XtArgVal) NULL},
};

static void
stub_enter_mode_btn(widget, event, params, num_params)
    Widget	 widget;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    draw_mousefun_mode();
}

static XtActionsRec mode_actions[] =
{
    {"EnterModeSw", (XtActionProc) stub_enter_mode_btn},
    {"LeaveModeSw", (XtActionProc) clear_mousefun},
    {"PressMiddle", (XtActionProc) notused_middle},
    {"ReleaseMiddle", (XtActionProc) clear_middle},
    {"PressRight", (XtActionProc) notused_right},
    {"ReleaseRight", (XtActionProc) clear_right},
    {"ModeCircleR", (XtActionProc) stub_circlebyradius_drawing_selected},
    {"ModeCircleD", (XtActionProc) stub_circlebydiameter_drawing_selected},
    {"ModeEllipseR", (XtActionProc) stub_ellipsebyradius_drawing_selected},
    {"ModeEllipseD", (XtActionProc) stub_ellipsebydiameter_drawing_selected},
    {"ModeBox", (XtActionProc) stub_box_drawing_selected},
    {"ModeArcBox", (XtActionProc) stub_arcbox_drawing_selected},
    {"ModeLine", (XtActionProc) stub_line_drawing_selected},
    {"ModePoly", (XtActionProc) stub_poly_drawing_selected},
    {"ModeRegPoly", (XtActionProc) stub_regpoly_drawing_selected},
    {"ModePIC", (XtActionProc) stub_picobj_drawing_selected},
    {"ModeText", (XtActionProc) stub_text_drawing_selected},
    {"ModeArc", (XtActionProc) stub_arc_drawing_selected},
    {"ModeSpline", (XtActionProc) stub_spline_drawing_selected},
    {"ModeClSpline", (XtActionProc) stub_cl_spline_drawing_selected},
    {"ModeIntSpline", (XtActionProc) stub_intspline_drawing_selected},
    {"ModeClIntSpline", (XtActionProc) stub_cl_intspline_drawing_selected},
    {"ModeAlign", (XtActionProc) stub_align_selected},
    {"ModeCompound", (XtActionProc) stub_compound_selected},
    {"ModeBreakCompound", (XtActionProc) stub_break_selected},
    {"ModeOpenCompound", (XtActionProc) stub_open_compound_selected},
    {"ModeJoinSplit", (XtActionProc) stub_join_split_selected},
    {"ModeScale", (XtActionProc) stub_scale_selected},
    {"ModeAddPoint", (XtActionProc) stub_point_adding_selected},
    {"ModeDeletePoint", (XtActionProc) stub_delete_point_selected},
    {"ModeMoveObject", (XtActionProc) stub_move_selected},
    {"ModePopupLibrary", (XtActionProc) stub_popup_library},
    {"ModeMovePoint", (XtActionProc) stub_move_point_selected},
    {"ModeDeleteObject", (XtActionProc) stub_delete_selected},
    {"ModeCopyObject", (XtActionProc) stub_copy_selected},
    {"ModeRotateObjectCW", (XtActionProc) stub_rotate_cw_selected},
    {"ModeRotateObjectCCW", (XtActionProc) stub_rotate_ccw_selected},
    {"ModeFlipObjectUD", (XtActionProc) stub_flip_ud_selected},
    {"ModeFlipObjectLR", (XtActionProc) stub_flip_lr_selected},
    {"ModeConvertObject", (XtActionProc) stub_convert_selected},
    {"ModeArrow", (XtActionProc) stub_arrow_head_selected},
    {"ModeEditObject", (XtActionProc) stub_edit_item_selected},
    {"ModeUpdateObject", (XtActionProc) stub_update_selected},
};

static String   mode_translations =
"<EnterWindow>:EnterModeSw()highlight()\n\
    <Btn1Down>:\n\
    <Btn1Up>:\n\
    <Btn2Down>:PressMiddle()\n\
    <Btn2Up>:ReleaseMiddle()\n\
    <Btn3Down>:PressRight()\n\
    <Btn3Up>:ReleaseRight()\n\
    <LeaveWindow>:LeaveModeSw()unhighlight()\n";

int
init_mode_panel(tool)
    Widget           tool;
{
    register int    i;
    register mode_sw_info *sw;

    FirstArg(XtNwidth, MODEPANEL_WD);
    NextArg(XtNhSpace, INTERNAL_BW);
    NextArg(XtNvSpace, INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNfromVert, msg_panel);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    NextArg(XtNresizable, False);
    NextArg(XtNborderWidth, 0);
    NextArg(XtNmappedWhenManaged, False);

    mode_panel = XtCreateWidget("mode_panel", boxWidgetClass, tool,
				Args, ArgCount);

    XtAppAddActions(tool_app, mode_actions, XtNumber(mode_actions));

    for (i = 0; i < NUM_MODE_SW; ++i) {
	sw = &mode_switches[i];
	if (sw->mode == FIRST_DRAW_MODE) {
	    FirstArg(XtNwidth, MODE_SW_WD * SW_PER_ROW +
		     INTERNAL_BW * (SW_PER_ROW - 1));
	    NextArg(XtNborderWidth, 0);
	    NextArg(XtNresize, False);
	    NextArg(XtNheight, (MODEPANEL_SPACE + 1) / 2);
	    NextArg(XtNlabel, "Drawing\n modes");
	    d_label = XtCreateManagedWidget("label", labelWidgetClass,
					    mode_panel, Args, ArgCount);
	} else if (sw->mode == FIRST_EDIT_MODE) {
	    /* assume Args still set up from d_label */
	    ArgCount -= 2;
	    NextArg(XtNheight, (MODEPANEL_SPACE) / 2);
	    NextArg(XtNlabel, "Editing\n modes");
	    e_label = XtCreateManagedWidget("label", labelWidgetClass,
					    mode_panel, Args, ArgCount);
	}
	button_args[1].value = sw->icon->width;
	button_args[2].value = sw->icon->height;
	sw->widget = XtCreateManagedWidget("button", commandWidgetClass,
			    mode_panel, button_args, XtNumber(button_args));

	/* left button changes mode */
	XtAddEventHandler(sw->widget, ButtonPressMask, (Boolean) 0,
			  sel_mode_but, (XtPointer) sw);
	/* popup when mouse passes over button */
	XtAddEventHandler(sw->widget, EnterWindowMask, (Boolean) 0,
			  mode_balloon_trigger, (XtPointer) sw);
	XtAddEventHandler(sw->widget, LeaveWindowMask, (Boolean) 0,
			  mode_unballoon, (XtPointer) sw);
	XtOverrideTranslations(sw->widget,
			       XtParseTranslationTable(mode_translations));
    }
    return;
}

/*
 * after panel widget is realized (in main) put some bitmaps etc. in it
 */

setup_mode_panel()
{
    register int    i;
    register mode_sw_info *msw;

    blank_gc = XCreateGC(tool_d, XtWindow(mode_panel), (unsigned long) 0, NULL);
    FirstArg(XtNforeground, &but_fg);
    NextArg(XtNbackground, &but_bg);
    GetValues(mode_switches[0].widget);

    XSetBackground(tool_d, blank_gc, but_bg);
    XSetForeground(tool_d, blank_gc, but_bg);

    FirstArg(XtNfont, button_font);
    SetValues(d_label);
    SetValues(e_label);

    for (i = 0; i < NUM_MODE_SW; ++i) {
	msw = &mode_switches[i];
	/* create normal bitmaps */
	msw->pixmap = XCreatePixmapFromBitmapData(tool_d, XtWindow(msw->widget),
		       msw->icon->bits, msw->icon->width, msw->icon->height,
				   but_fg, but_bg, tool_dpth);

	FirstArg(XtNbackgroundPixmap, msw->pixmap);
	SetValues(msw->widget);

	/* create reverse bitmaps */
	msw->reversePM = XCreatePixmapFromBitmapData(tool_d, XtWindow(msw->widget),
		       msw->icon->bits, msw->icon->width, msw->icon->height,
				   but_bg, but_fg, tool_dpth);
    }

    XDefineCursor(tool_d, XtWindow(mode_panel), arrow_cursor);
    FirstArg(XtNmappedWhenManaged, True);
    SetValues(mode_panel);
}

/* come here when the mouse passes over a button in the mode panel */

static	Widget mode_balloon_popup = (Widget) 0;
static	XtIntervalId balloon_id = (XtIntervalId) 0;
static	Widget balloon_w;
static	XtPointer clos;

static void mode_balloon();

static void
mode_balloon_trigger(widget, closure, event, continue_to_dispatch)
    Widget        widget;
    XtPointer	  closure;
    XEvent*	  event;
    Boolean*	  continue_to_dispatch;
{
	if (!appres.showballoons)
		return;
	balloon_w = widget;
	clos = closure;
	/* if an old balloon is still up, destroy it */
	if ((balloon_id != 0) || (mode_balloon_popup != (Widget) 0)) {
		mode_unballoon((Widget) 0, (XtPointer) 0, (XEvent*) 0, (Boolean*) 0);
	}
	balloon_id = XtAppAddTimeOut(tool_app, appres.balloon_delay,
			(XtTimerCallbackProc) mode_balloon, (XtPointer) NULL);
}

static void
mode_balloon()
{
	Position  x, y;
	mode_sw_info *msw = (mode_sw_info *) clos;
	Widget box, balloon_label;

	/* get position of this button */
	XtTranslateCoords(balloon_w, msw->icon->width+2, 0, &x, &y);
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	mode_balloon_popup = XtCreatePopupShell("mode_balloon_popup",
				    overrideShellWidgetClass, tool, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNhSpace, 0);
	NextArg(XtNvSpace, 0);
	box = XtCreateManagedWidget("box", boxWidgetClass, mode_balloon_popup,
				Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNlabel, msw->modemsg);
	balloon_label = XtCreateManagedWidget("label", labelWidgetClass,
				    box, Args, ArgCount);

	/* if the panel is on the right-hand side shift popup to the left */
	if (appres.RHS_PANEL) {
	    XtWidgetGeometry xtgeom,comp;
	    Dimension wpop,wbut;

	    /* get width of popup with label in it */
	    FirstArg(XtNwidth, &wpop);
	    GetValues(balloon_label);
	    /* and width of button */
	    FirstArg(XtNwidth, &wbut);
	    GetValues(balloon_w);
	    /* only change X position of widget */
	    xtgeom.request_mode = CWX;
	    /* shift popup left */
	    xtgeom.x = x-wpop-wbut-10;
	    (void) XtMakeGeometryRequest(mode_balloon_popup, &xtgeom, &comp);
	}
	XtPopup(mode_balloon_popup,XtGrabNone);
	XtRemoveTimeOut(balloon_id);
	balloon_id = (XtIntervalId) 0;
}

/* come here when the mouse leaves a button in the mode panel */

static void
mode_unballoon(widget, closure, event, continue_to_dispatch)
    Widget          widget;
    XtPointer	    closure;
    XEvent*	    event;
    Boolean*	    continue_to_dispatch;
{
    if (balloon_id) 
	XtRemoveTimeOut(balloon_id);
    balloon_id = (XtIntervalId) 0;
    if (mode_balloon_popup != (Widget) 0) {
	XtDestroyWidget(mode_balloon_popup);
	mode_balloon_popup = (Widget) 0;
    }
}

/* come here when a button is pressed in the mode panel */

static void
sel_mode_but(widget, closure, event, continue_to_dispatch)
    Widget          widget;
    XtPointer	    closure;
    XEvent*	    event;
    Boolean*	    continue_to_dispatch;
{
    XButtonEvent    xbutton;
    mode_sw_info    *msw = (mode_sw_info *) closure;

    /* erase any existing anchor for flips */
    if (setanchor)
	center_marker(setanchor_x, setanchor_y);
    /* and any center for rotations */
    if (setcenter)
	center_marker(setcenter_x, setcenter_y);
    setcenter = 0;
    setanchor = 0;

    xbutton = event->xbutton;
    if (check_action_on())
	return;
    else if (highlighting)
	erase_objecthighlight();

    /* if this command popups a window, destroy the balloon popup now. See the
       note above about this above the command panel definition. */
    if (msw->popup) {
	mode_unballoon((Widget) 0, (XtPointer) 0, (XEvent*) 0, (Boolean*) 0);
    }
    app_flush();

    if (xbutton.button == Button1) {	/* left button */
	turn_off_current();
	turn_on(msw);
	if (msw->mode == F_UPDATE) {	/* map the set/clr/toggle button for update */
	    if (cur_mode != F_UPDATE) {
		update_indpanel(0);	/* first remove ind buttons */
		XtUnmanageChild(ind_panel);
		XtManageChild(upd_ctrl);
		/* get the width of the update control panel */
		/* now put the ind_panel to our right */
		FirstArg(XtNfromHoriz, upd_ctrl);
		NextArg(XtNwidth, INDPANEL_WD-UPD_CTRL_WD-2*INTERNAL_BW); /* resize it */
		SetValues(ind_panel);
		XtManageChild(ind_panel);
		update_indpanel(msw->indmask);	/* now manage the relevant buttons */
	    }
	} else { 	/* turn off the update boxes if not in update mode */
	    if (cur_mode == F_UPDATE) {	/* if previous mode is update and current */
		update_indpanel(0);	/* is not, first remove ind buttons */
		unmanage_update_buts();
		XtUnmanageChild(ind_panel);
		XtUnmanageChild(upd_ctrl);
		/* now put the ind_panel to the right of the canvas */
		FirstArg(XtNfromHoriz, NULL);
		NextArg(XtNwidth, INDPANEL_WD);	/* resize it */
		SetValues(ind_panel);
		XtManageChild(ind_panel);
		update_indpanel(msw->indmask);	/* now manage the relevant buttons */
	    } else {
		update_indpanel(msw->indmask);	/* just update indicator buttons */
	    }
	}
	put_msg(msw->modemsg);
	if ((cur_mode == F_GLUE || cur_mode == F_BREAK) &&
	    msw->mode != F_GLUE && msw->mode != F_BREAK) {
		/* reset tagged items when changing modes; perhaps this
		   is not really necessary */
		set_tags(&objects, 0);
	}
	cur_mode = msw->mode;
	anypointposn = !(msw->indmask & I_POINTPOSN);
	new_objmask = msw->objmask;
	if (cur_mode == F_ROTATE && cur_rotnangle != 90)
	    new_objmask = M_ROTATE_ANGLE;
	update_markers(new_objmask);
	current = msw;
	/* call the mode function */
	msw->setmode_func();
    }
}

void
force_positioning()
{
    update_indpanel(current->indmask | I_POINTPOSN);
    anypointposn = 0;
}

void
force_nopositioning()
{
    update_indpanel(current->indmask & ~I_POINTPOSN);
    anypointposn = 1;
}

void
force_anglegeom()
{
    update_indpanel(current->indmask | I_ANGLEGEOM);
}

void
force_noanglegeom()
{
    update_indpanel(current->indmask & ~I_ANGLEGEOM);
}

static void
turn_on(msw)
    mode_sw_info   *msw;
{
    FirstArg(XtNbackgroundPixmap, msw->reversePM);
    SetValues(msw->widget);
}

turn_on_current()
{
    if (current)
	turn_on(current);
}

turn_off_current()
{
    if (current) {
	XtOverrideTranslations(current->widget,
		XtParseTranslationTable(mode_translations));
	FirstArg(XtNbackgroundPixmap, current->pixmap);
	SetValues(current->widget);
    }
}

change_mode(icon)
icon_struct *icon;
{
    int i;
    XButtonEvent ev; /* To fake an event with */

    ev.button = Button1;
    for (i = 0; i < NUM_MODE_SW; ++i)
	if (mode_switches[i].icon == icon) {
	    sel_mode_but(0,&mode_switches[i],&ev,0);
	    break;
	}
    /* force update of mouse function window */
    draw_mousefun_canvas();
}

static void
stub_circlebyradius_drawing_selected()
{
	change_mode(&cirrad_ic);
}

static void
stub_circlebydiameter_drawing_selected()
{
	change_mode(&cirdia_ic);
}

static void
stub_ellipsebyradius_drawing_selected()
{
	change_mode(&ellrad_ic);
}

static void
stub_ellipsebydiameter_drawing_selected()
{
	change_mode(&elldia_ic);
}

static void
stub_box_drawing_selected()
{
	change_mode(&box_ic);
}

static void
stub_arcbox_drawing_selected()
{
	change_mode(&arc_box_ic);
}

static void
stub_poly_drawing_selected()
{
	change_mode(&polygon_ic);
}

static void
stub_line_drawing_selected()
{
	change_mode(&line_ic);
}

static void
stub_regpoly_drawing_selected()
{
	change_mode(&regpoly_ic);
}

static void
stub_picobj_drawing_selected()
{
	change_mode(&picobj_ic);
}

static void
stub_text_drawing_selected()
{
	change_mode(&text_ic);
}

static void
stub_arc_drawing_selected()
{
	change_mode(&arc_ic);
}

static void
stub_cl_spline_drawing_selected()
{
	change_mode(&c_spl_ic);
}

static void
stub_spline_drawing_selected()
{
	change_mode(&spl_ic);
}

static void
stub_cl_intspline_drawing_selected()
{
	change_mode(&c_intspl_ic);
}

static void
stub_intspline_drawing_selected()
{
	change_mode(&intspl_ic);
}

static void
stub_align_selected()
{
	change_mode(&align_ic);
}

static void
stub_compound_selected()
{
	change_mode(&glue_ic);
}

static void
stub_break_selected()
{
	change_mode(&break_ic);
}

static void
stub_join_split_selected()
{
	change_mode(&join_split_ic);
}

static void
stub_open_compound_selected()
{
	change_mode(&open_comp_ic);
}

static void
stub_scale_selected()
{
	change_mode(&scale_ic);
}

static void
stub_point_adding_selected()
{
	change_mode(&addpt_ic);
}

static void
stub_delete_point_selected()
{
	change_mode(&deletept_ic);
}

static void
stub_move_selected()
{
	change_mode(&move_ic);
}
static void
stub_popup_library()
{
	change_mode(&library_ic);
}


static void
stub_move_point_selected()
{
	change_mode(&movept_ic);
}

static void
stub_delete_selected()
{
	change_mode(&delete_ic);
}

static void
stub_copy_selected()
{
	change_mode(&copy_ic);
}

static void
stub_rotate_cw_selected()
{
	change_mode(&rotCW_ic);
}

static void
stub_rotate_ccw_selected()
{
	change_mode(&rotCCW_ic);
}

static void
stub_flip_ud_selected()
{
	change_mode(&flip_y_ic);
}

static void
stub_flip_lr_selected()
{
	change_mode(&flip_x_ic);
}

static void
stub_convert_selected()
{
	change_mode(&convert_ic);
}

static void
stub_arrow_head_selected()
{
	change_mode(&autoarrow_ic);
}

static void
stub_edit_item_selected()
{
	change_mode(&edit_ic);
}

static void
stub_update_selected()
{
	change_mode(&update_ic);
}

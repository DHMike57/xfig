/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Parts Copyright (c) 1994 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
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


/* This is for the message window below the command panel */
/* The popup message window is handled in the second part of this file */

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "paintop.h"
#include "u_elastic.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_util.h"
#include "w_setup.h"
#include <varargs.h>

/********************* IMPORTS *******************/

extern char    *basname();
extern char    *read_file_name;
extern Widget	cfile_text;		/* widget for the current filename */

/********************* EXPORTS *******************/

int		put_msg();
int		init_msgreceiving();
Boolean		popup_up = False;

/* so w_file.c can access */
Boolean	file_msg_is_popped=False;
Widget	file_msg_popup;

/* for f_read.c */
Boolean	first_file_msg;

/************************  LOCAL ******************/

#define		BUF_SIZE		128
static char	prompt[BUF_SIZE];

DeclareStaticArgs(12);

/* for the balloon toggle window */
static Widget	balloon_toggle;

/* turns on and off the balloons setting */
static XtCallbackProc toggle_balloons();

/* popup message over toggle window when mouse enters it */
static void     toggle_balloon();
static void     toggle_unballoon();

/* popup message over filename window when mouse enters it */
static void     filename_balloon();
static void     filename_unballoon();

/* for the popup message (file_msg) window */

static int	file_msg_length=0;
static char	tmpstr[300];
static Widget	file_msg_panel,
		file_msg_win, file_msg_dismiss;

static String	file_msg_translations =
	"<Message>WM_PROTOCOLS: DismissFileMsg()\n";
static XtEventHandler file_msg_panel_dismiss();
static XtActionsRec	file_msg_actions[] =
{
    {"DismissFileMsg", (XtActionProc) file_msg_panel_dismiss},
};

#define balloons_on_width 16
#define balloons_on_height 15
static char balloons_on_bits[] = {
   0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x67, 0xfe, 0x63, 0xfe, 0x71, 0xfe, 0x79,
   0xfe, 0x7c, 0xe2, 0x7c, 0x46, 0x7e, 0x0e, 0x7e, 0x0e, 0x7f, 0x1e, 0x7f,
   0x9e, 0x7f, 0xfe, 0x7f, 0x00, 0x00};

#define balloons_off_width 16
#define balloons_off_height 15
static char balloons_off_bits[] = {
   0xff, 0xff, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
   0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
   0x01, 0x80, 0x01, 0x80, 0xff, 0xff};

/* message window code begins */

int
init_msg(tool, filename)
    Widget	    tool;
    char	   *filename;
{
    /* first make a form to put the four widgets in */
    FirstArg(XtNwidth, MSGFORM_WD);
    NextArg(XtNheight, MSGFORM_HT);
    NextArg(XtNfromVert, cmd_panel);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNdefaultDistance, 0);
    NextArg(XtNborderWidth, 0);
    msg_form = XtCreateManagedWidget("msg_form", formWidgetClass, tool,
				      Args, ArgCount);
    /* setup the file name widget first */
    FirstArg(XtNresizable, True);
    NextArg(XtNfont, bold_font);
    NextArg(XtNlabel, (filename!=NULL? filename: DEF_NAME));
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNborderWidth, INTERNAL_BW);
    name_panel = XtCreateManagedWidget("file_name", labelWidgetClass, msg_form,
				      Args, ArgCount);
    /* popup when mouse passes over filename */
    XtAddEventHandler(name_panel, EnterWindowMask, (Boolean) 0,
		      filename_balloon, (XtPointer) name_panel);
    XtAddEventHandler(name_panel, LeaveWindowMask, (Boolean) 0,
		      filename_unballoon, (XtPointer) name_panel);

    /* now the message panel */
    FirstArg(XtNfont, roman_font);
    NextArg(XtNstring, "\0");
    NextArg(XtNfromHoriz, name_panel);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNdisplayCaret, False);
    msg_panel = XtCreateManagedWidget("message", asciiTextWidgetClass, msg_form,
				      Args, ArgCount);
    /* and finally, the button to turn on/off the popup balloon messages */
    /* the checkmark bitmap is created and set in setup_msg() */
    FirstArg(XtNstate, appres.show_balloons);
    NextArg(XtNinternalWidth, 1);
    NextArg(XtNinternalHeight, 1);
    NextArg(XtNfromHoriz, msg_panel);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNleft, XtChainRight);
    NextArg(XtNright, XtChainRight);
    NextArg(XtNlabel, "Balloons   ");
    NextArg(XtNborderWidth, INTERNAL_BW);
    balloon_toggle = XtCreateManagedWidget("balloon_toggle", toggleWidgetClass,
				   msg_form, Args, ArgCount);
    /* popup when mouse passes over toggle */
    XtAddEventHandler(balloon_toggle, EnterWindowMask, (Boolean) 0,
		      toggle_balloon, (XtPointer) balloon_toggle);
    XtAddEventHandler(balloon_toggle, LeaveWindowMask, (Boolean) 0,
		      toggle_unballoon, (XtPointer) balloon_toggle);
    XtAddCallback(balloon_toggle, XtNcallback, (XtCallbackProc) toggle_balloons,
		  (XtPointer) NULL);
}

static	Pixmap balloons_on_bitmap=(Pixmap) 0, balloons_off_bitmap=(Pixmap) 0;

/* at this point the widget has been realized so we can do more */

setup_msg()
{
    Dimension htn,htf;

    XtUnmanageChild(msg_panel);

    /* set the height of the message panel and filename panel to the 
       greater of the heights for everything in that form */
    FirstArg(XtNheight, &htn);
    GetValues(name_panel);
    FirstArg(XtNheight, &htf);
    GetValues(msg_panel);
    htf = max2(htf,htn);

    /* now the toggle widget */
    FirstArg(XtNheight, &htn);
    GetValues(balloon_toggle);
    htf = max2(htf,htn);

    /* set the MSGFORM_HT variable so the mouse panel can be resized to fit */
    MSGFORM_HT = htf;

    FirstArg(XtNheight, htf);
    SetValues(msg_panel);
    SetValues(name_panel);
    SetValues(balloon_toggle);

    /* create two bitmaps to show on/off state */
    balloons_on_bitmap = XCreateBitmapFromData(tool_d, tool_w, 
			balloons_on_bits, balloons_on_width, balloons_on_height);
    balloons_off_bitmap = XCreateBitmapFromData(tool_d, tool_w, 
			balloons_off_bits, balloons_off_width, balloons_off_height);
    FirstArg(XtNleftBitmap, appres.show_balloons? balloons_on_bitmap: balloons_off_bitmap);
    SetValues(balloon_toggle);

    XtManageChild(msg_panel);
    if (msg_win == 0)
	msg_win = XtWindow(msg_panel);
    XDefineCursor(tool_d, msg_win, null_cursor);
}

/* come here when the mouse passes over the filename window */

static	Widget filename_balloon_popup = (Widget) 0;

static void
filename_balloon(widget, closure, event, continue_to_dispatch)
    Widget        widget;
    XtPointer	  closure;
    XEvent*	  event;
    Boolean*	  continue_to_dispatch;
{
	Widget	  box, balloons_label;
	Position  x, y;
	Dimension w;

	if (!appres.show_balloons)
	    return;

	/* get width of this window */
	FirstArg(XtNwidth, &w);
	GetValues(widget);
	/* find right edge + 5 pixels */
	XtTranslateCoords(widget, w+5, 0, &x, &y);

	/* put popup there */
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	filename_balloon_popup = XtCreatePopupShell("filename_balloon_popup",
				overrideShellWidgetClass, tool, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNhSpace, 0);
	NextArg(XtNvSpace, 0);
	box = XtCreateManagedWidget("box", boxWidgetClass, filename_balloon_popup, 
			Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNlabel, "Current filename");
	balloons_label = XtCreateManagedWidget("label", labelWidgetClass,
				    box, Args, ArgCount);
	XtPopup(filename_balloon_popup,XtGrabNone);
}

/* come here when the mouse leaves the filename window */

static void
filename_unballoon(widget, closure, event, continue_to_dispatch)
    Widget          widget;
    XtPointer	    closure;
    XEvent*	    event;
    Boolean*	    continue_to_dispatch;
{
    if (filename_balloon_popup != (Widget) 0)
	XtDestroyWidget(filename_balloon_popup);
    filename_balloon_popup = 0;
}

/* come here when the mouse passes over the toggle window */

static	Widget toggle_balloon_popup = (Widget) 0;

static void
toggle_balloon(widget, closure, event, continue_to_dispatch)
    Widget        widget;
    XtPointer	  closure;
    XEvent*	  event;
    Boolean*	  continue_to_dispatch;
{
	Widget	  box, balloons_label;
	Position  x, y;
	Dimension w;

	if (!appres.show_balloons)
	    return;

	/* get width of this button */
	FirstArg(XtNwidth, &w);
	GetValues(widget);
	/* find right edge + 5 pixels */
	XtTranslateCoords(widget, w+5, 0, &x, &y);

	/* put popup there */
	FirstArg(XtNx, x);
	NextArg(XtNy, y);
	toggle_balloon_popup = XtCreatePopupShell("toggle_balloon_popup",
				overrideShellWidgetClass, tool, Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNhSpace, 0);
	NextArg(XtNvSpace, 0);
	box = XtCreateManagedWidget("box", boxWidgetClass, toggle_balloon_popup, 
			Args, ArgCount);
	FirstArg(XtNborderWidth, 0);
	NextArg(XtNlabel, "Turn on and off balloon messages");
	balloons_label = XtCreateManagedWidget("label", labelWidgetClass,
				    box, Args, ArgCount);
	XtPopup(toggle_balloon_popup,XtGrabNone);
}

/* come here when the mouse leaves the toggle window */

static void
toggle_unballoon(widget, closure, event, continue_to_dispatch)
    Widget          widget;
    XtPointer	    closure;
    XEvent*	    event;
    Boolean*	    continue_to_dispatch;
{
    if (toggle_balloon_popup != (Widget) 0)
	XtDestroyWidget(toggle_balloon_popup);
    toggle_balloon_popup = 0;
}

static
XtCallbackProc
toggle_balloons(w, dummy, dummy2)
    Widget	   w;
    XtPointer	   dummy;
    XtPointer	   dummy2;
{
    /* toggle flag */
    appres.show_balloons = !appres.show_balloons;
    /* and change bitmap to show state */
    if (appres.show_balloons) {
	FirstArg(XtNleftBitmap, balloons_on_bitmap);
    } else {
	FirstArg(XtNleftBitmap, balloons_off_bitmap);
    }
    SetValues(w);
}

/*
 * Update the current filename in the name_panel widget (it will resize
 * automatically) and resize the msg_panel widget to fit in the remaining 
 * space, by getting the width of the command panel and subtract the new 
 * width of the name_panel to get the new width of the message panel.
 * Also update the current filename in the File popup (if it has been created).
 */

update_cur_filename(newname)
	char	*newname;
{
	Dimension namwid,togwid;

	XtUnmanageChild(msg_form);
	XtUnmanageChild(msg_panel);
	XtUnmanageChild(name_panel);
	XtUnmanageChild(balloon_toggle);

	strcpy(cur_filename,newname);

	/* store the new filename in the name_panel widget */
	FirstArg(XtNlabel, newname);
	SetValues(name_panel);
	if (cfile_text)			/* if the name widget in the file popup exists... */
	    SetValues(cfile_text);

	/* get the new size of the name_panel */
	FirstArg(XtNwidth, &namwid);
	GetValues(name_panel);
	/* and the other two */
	FirstArg(XtNwidth, &togwid);
	GetValues(balloon_toggle);

	/* new size is form width minus all others */
	MSGPANEL_WD = MSGFORM_WD-namwid-togwid;
	if (MSGPANEL_WD <= 0)
		MSGPANEL_WD = 100;
	/* resize the message panel to fit with the new width of the name panel */
	FirstArg(XtNwidth, MSGPANEL_WD);
	SetValues(msg_panel);
	/* keep the height the same */
	FirstArg(XtNheight, MSGFORM_HT);
	SetValues(name_panel);
	SetValues(balloon_toggle);

	XtManageChild(msg_panel);
	XtManageChild(name_panel);
	XtManageChild(balloon_toggle);

	/* now resize the whole form */
	FirstArg(XtNwidth, MSGFORM_WD);
	SetValues(msg_form);
	XtManageChild(msg_form);
	/* put the filename being edited in the icon */
	XSetIconName(tool_d, tool_w, basname(cur_filename));

	update_def_filename();		/* update default filename in export panel */
}

/* VARARGS1 */
int
put_msg(va_alist)
    va_dcl
{
    va_list ap;
    char *format;

    va_start(ap);
    format = va_arg(ap, char *);
    vsprintf(prompt, format, ap );
    va_end(ap);
    FirstArg(XtNstring, prompt);
    SetValues(msg_panel);
}

clear_message()
{
    FirstArg(XtNstring, "\0");
    SetValues(msg_panel);
}

boxsize_msg(fact)
    int fact;
{
    float	dx, dy;

    dx = (float) fact * abs(cur_x - fix_x) /
		(float)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM);
    dy = (float) fact * abs(cur_y - fix_y) /
		(float)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM);
    put_msg("Width = %.3lf %s, Length = %.3lf %s",
		dx*appres.user_scale, cur_fig_units,
		dy*appres.user_scale, cur_fig_units);
}

length_msg(type)
int type;
{
    altlength_msg(type, fix_x, fix_y);
}

/*
** In typical usage, point fx,fy is the fixed point.
** Distance will be measured from it to cur_x,cur_y.
*/

altlength_msg(type, fx, fy)
int type;
{
    float	len;
    double	dx,dy;

    dx = (cur_x - fx)/(double)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM);
    dy = (cur_y - fy)/(double)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM);
    len = (float)sqrt(dx*dx + dy*dy);
    put_msg("%s = %.3f %s, dx = %.3lf %s, dy = %.3lf %s",
		(type==MSG_RADIUS? "Radius":
                  (type==MSG_DIAM? "Diameter":
		  (type==MSG_LENGTH? "Length": "Distance"))),
		len*appres.user_scale, cur_fig_units,
		(float)dx*appres.user_scale, cur_fig_units,
		(float)dy*appres.user_scale, cur_fig_units);
}

/*
** In typical usage, point x3,y3 is the one that is moving,
** the other two are fixed.  Distances will be measured from
** points 1 -> 3 and 2 -> 3.
*/

length_msg2(x1,y1,x2,y2,x3,y3)
int x1,y1,x2,y2,x3,y3;
{
    float	len1,len2;
    double	dx1,dy1,dx2,dy2;

    len1=len2=0.0;
    if (x1 != -999) {
	    dx1 = x3 - x1;
	    dy1 = y3 - y1;
	    len1 = (float)(sqrt(dx1*dx1 + dy1*dy1)/
		(double)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM));
    }
    if (x2 != -999) {
	    dx2 = x3 - x2;
	    dy2 = y3 - y2;
	    len2 = (float)(sqrt(dx2*dx2 + dy2*dy2)/
		(double)(appres.INCHES? PIX_PER_INCH: PIX_PER_CM));
    }
    put_msg("Length 1 = %.3f, Length 2 = %.3f %s",
		len1*appres.user_scale, len2*appres.user_scale, cur_fig_units);
}

/* This is the section for the popup message window (file_msg) */

/* VARARGS1 */
int
file_msg(va_alist) va_dcl
{
    va_list arglist;
    XawTextBlock block;
    va_list ap;
    char *format;

    popup_file_msg();
    if (first_file_msg)
	{
	first_file_msg = False;
	file_msg("---------------------");
	file_msg("File %s:",read_file_name);
	}

    va_start(ap);
    format = va_arg(ap, char *);
    vsprintf(tmpstr, format, ap );
    va_end(ap);

    strcat(tmpstr,"\n");
    /* append this message to the file message widget string */
    block.firstPos = 0;
    block.ptr = tmpstr;
    block.length = strlen(tmpstr);
    block.format = FMT8BIT;
    /* make editable to add new message */
    FirstArg(XtNeditType, XawtextEdit);
    SetValues(file_msg_win);
    /* insert the new message after the end */
    (void) XawTextReplace(file_msg_win,file_msg_length,file_msg_length,&block);
    (void) XawTextSetInsertionPoint(file_msg_win,file_msg_length);

    /* make read-only again */
    FirstArg(XtNeditType, XawtextRead);
    SetValues(file_msg_win);
    file_msg_length += block.length;
}

clear_file_message(w, ev)
    Widget	    w;
    XButtonEvent   *ev;
{
    XawTextBlock	block;
    int			replcode;

    if (!file_msg_popup)
	return;

    tmpstr[0]=' ';
    block.firstPos = 0;
    block.ptr = tmpstr;
    block.length = 1;
    block.format = FMT8BIT;

    /* make editable to clear message */
    FirstArg(XtNeditType, XawtextEdit);
    NextArg(XtNdisplayPosition, 0);
    SetValues(file_msg_win);

    /* replace all messages with one blank */
    replcode = XawTextReplace(file_msg_win,0,file_msg_length,&block);
    if (replcode == XawPositionError)
	fprintf(stderr,"XawTextReplace XawPositionError\n");
    else if (replcode == XawEditError)
	fprintf(stderr,"XawTextReplace XawEditError\n");

    /* make read-only again */
    FirstArg(XtNeditType, XawtextRead);
    SetValues(file_msg_win);
    file_msg_length = 0;
}

static Bool grabbed=False;

static
XtEventHandler
file_msg_panel_dismiss(w, ev)
    Widget	    w;
    XButtonEvent   *ev;
{
	if ((grabbed) && (!popup_up))
		XtAddGrab(file_msg_popup, False, False);
	XtPopdown(file_msg_popup);
	file_msg_is_popped=False;
}

popup_file_msg()
{
	extern Atom wm_delete_window;

	if (file_msg_popup) {
	    if (!file_msg_is_popped) {
		if (popup_up) {
		    XtPopup(file_msg_popup, XtGrabNonexclusive);
		    XSetWMProtocols(XtDisplay(file_msg_popup),
				    XtWindow(file_msg_popup),
				    &wm_delete_window, 1);
		    grabbed = True;
		} else {
		    XtPopup(file_msg_popup, XtGrabNone);
		    XSetWMProtocols(XtDisplay(file_msg_popup),
				    XtWindow(file_msg_popup),
				    &wm_delete_window, 1);
		    grabbed = False;
		}
	    }
	    /* ensure that the most recent colormap is installed */
	    set_cmap(XtWindow(file_msg_popup));
	    file_msg_is_popped = True;
	    return;
	}

	file_msg_is_popped = True;
	FirstArg(XtNx, 0);
	NextArg(XtNy, 0);
	NextArg(XtNcolormap, tool_cm);
	NextArg(XtNtitle, "Xfig: Error messages");
	file_msg_popup = XtCreatePopupShell("file_msg",
					transientShellWidgetClass,
					tool, Args, ArgCount);
	XtOverrideTranslations(file_msg_popup,
			XtParseTranslationTable(file_msg_translations));
	XtAppAddActions(tool_app, file_msg_actions, XtNumber(file_msg_actions));

	file_msg_panel = XtCreateManagedWidget("file_msg_panel", formWidgetClass,
					   file_msg_popup, NULL, ZERO);
	FirstArg(XtNwidth, 500);
	NextArg(XtNheight, 200);
	NextArg(XtNeditType, XawtextRead);
	NextArg(XtNdisplayCaret, False);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNscrollHorizontal, XawtextScrollWhenNeeded);
	NextArg(XtNscrollVertical, XawtextScrollAlways);
	file_msg_win = XtCreateManagedWidget("file_msg_win", asciiTextWidgetClass,
					     file_msg_panel, Args, ArgCount);

	FirstArg(XtNlabel, "Dismiss");
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfromVert, file_msg_win);
	file_msg_dismiss = XtCreateManagedWidget("dismiss", commandWidgetClass,
				       file_msg_panel, Args, ArgCount);
	XtAddEventHandler(file_msg_dismiss, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)file_msg_panel_dismiss, (XtPointer) NULL);

	FirstArg(XtNlabel, "Clear");
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfromVert, file_msg_win);
	NextArg(XtNfromHoriz, file_msg_dismiss);
	file_msg_dismiss = XtCreateManagedWidget("clear", commandWidgetClass,
				       file_msg_panel, Args, ArgCount);
	XtAddEventHandler(file_msg_dismiss, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)clear_file_message, (XtPointer) NULL);

	if (popup_up)
		{
		XtPopup(file_msg_popup, XtGrabNonexclusive);
    		XSetWMProtocols(XtDisplay(file_msg_popup),
				XtWindow(file_msg_popup),
			       	&wm_delete_window, 1);
		grabbed = True;
		}
	else
		{
		XtPopup(file_msg_popup, XtGrabNone);
    		XSetWMProtocols(XtDisplay(file_msg_popup),
				XtWindow(file_msg_popup),
			       	&wm_delete_window, 1);
		grabbed = False;
		}
	/* insure that the most recent colormap is installed */
	set_cmap(XtWindow(file_msg_popup));
}

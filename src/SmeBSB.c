/* $XConsortium: SmeBSB.c,v 1.19 94/04/17 20:12:49 kaleb Exp $ */

/* This is being included with the xfig distribution to ensure proper
 * includes for Xaw3d, if used.  It is unchanged from the X Consortium.
 */

/*
Copyright (c) 1989, 1994  X Consortium

 Any party obtaining a copy of these files is granted, free of charge, a
 full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 nonexclusive right and license to deal in this software and
 documentation files (the "Software"), including without limitation the
 rights to use, copy, modify, merge, publish and/or distribute copies of
 the Software, and to permit persons who receive copies from any such
 party to do so, with the only requirement being that this copyright
 notice remain intact.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 */

/*
 * SmeBSB.c - Source code file for BSB Menu Entry object.
 *
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <string.h>

#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xutil.h>

/* needed for abs() */
#ifdef X_NOT_STDC_ENV
#else
#include <stdlib.h>
#endif /* X_NOT_STDC_ENV */

#include <X11/Xmu/Drawing.h>

#ifdef XAW3D
#include <X11/Xaw3d/XawInit.h>
#else
#include <X11/Xaw/XawInit.h>
#endif /* XAW3D */

#include "SmeBSBP.h"
#include "figx.h"


#define ONE_HUNDRED 100

#define offset(field) XtOffsetOf(SmeBSBRec, sme_bsb.field)

static XtResource resources[] = {
  {XtNlabel,  XtCLabel, XtRString, sizeof(String),
     offset(label), XtRString, NULL},
  {XtNvertSpace,  XtCVertSpace, XtRInt, sizeof(int),
     offset(vert_space), XtRImmediate, (XtPointer) 25},
  {XtNleftBitmap, XtCLeftBitmap, XtRBitmap, sizeof(Pixmap),
     offset(left_bitmap), XtRImmediate, (XtPointer)None},
  {XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
     offset(justify), XtRImmediate, (XtPointer) XtJustifyLeft},
  {XtNrightBitmap, XtCRightBitmap, XtRBitmap, sizeof(Pixmap),
     offset(right_bitmap), XtRImmediate, (XtPointer)None},
  {XtNleftMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(left_margin), XtRImmediate, (XtPointer) 4},
  {XtNrightMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(right_margin), XtRImmediate, (XtPointer) 4},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     offset(foreground), XtRString, XtDefaultForeground},
  {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     offset(font), XtRString, XtDefaultFont},
  {XtNfontSet,  XtCFontSet, XtRFontSet, sizeof(XFontSet ),
     offset(fontset),XtRString, XtDefaultFontSet},
};
#undef offset

/*
 * Semi Public function definitions.
 */

static void Redisplay(Widget w, XEvent *event, Region region), Destroy(Widget w), Initialize(Widget request, Widget new, ArgList args, Cardinal *num_args), FlipColors(Widget w);
static void ClassInitialize(void);
static Boolean SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args);
static XtGeometryResult QueryGeometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *return_val);

/*
 * Private Function Definitions.
 */

static void GetDefaultSize(Widget w, Dimension *width, Dimension *height), DrawBitmaps(Widget w, GC gc), GetBitmapInfo(Widget w, Boolean is_left);
static void CreateGCs(Widget w), DestroyGCs(Widget w);

#ifdef XAW3D
#define superclass (&smeThreeDClassRec)
#else
#define superclass (&smeClassRec)
#endif /* XAW3D */

SmeBSBClassRec smeBSBClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "SmeBSB",
    /* size               */    sizeof(SmeBSBRec),
    /* class_initializer  */	ClassInitialize,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    ZERO,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    NULL,
    /* expose             */    Redisplay,
    /* set_values         */    SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    NULL,
    /* query_geometry	  */    QueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* SimpleMenuClass Fields */
    /* highlight          */	FlipColors,
    /* unhighlight        */	FlipColors,
    /* notify             */	XtInheritNotify,
    /* extension	  */	NULL
  },
#ifdef XAW3D
  {
    /* ThreeDClass Fields */
    /* shadowdraw         */    XtInheritXawSme3dShadowDraw
  },
#endif /* XAW3D */
  {
    /* BSBClass Fields */
    /* extension	  */    NULL
  }
};

WidgetClass smeBSBObjectClass = (WidgetClass) &smeBSBClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*	Function Name: ClassInitialize
 *	Description: Initializes the SmeBSBObject.
 *	Arguments: none.
 *	Returns: none.
 */

static void
ClassInitialize(void)
{
    XawInitializeWidgetSet();
    XtAddConverter( XtRString, XtRJustify, XmuCvtStringToJustify,
		    (XtConvertArgList)NULL, (Cardinal)0 );
}

/*      Function Name: Initialize
 *      Description: Initializes the simple menu widget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 */

/* ARGSUSED */
static void
Initialize(Widget request, Widget new, ArgList args, Cardinal *num_args)
{
    SmeBSBObject entry = (SmeBSBObject) new;

    if (entry->sme_bsb.label == NULL)
	entry->sme_bsb.label = XtName(new);
    else
	entry->sme_bsb.label = XtNewString( entry->sme_bsb.label );

    GetDefaultSize(new, &(entry->rectangle.width), &(entry->rectangle.height));
    CreateGCs(new);

    entry->sme_bsb.left_bitmap_width = entry->sme_bsb.left_bitmap_height = 0;
    entry->sme_bsb.right_bitmap_width = entry->sme_bsb.right_bitmap_height = 0;

    GetBitmapInfo(new, TRUE);	/* Left Bitmap Info */
    GetBitmapInfo(new, FALSE);	/* Right Bitmap Info */
}

/*      Function Name: Destroy
 *      Description: Called at destroy time, cleans up.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
Destroy(Widget w)
{
    SmeBSBObject entry = (SmeBSBObject) w;

    DestroyGCs(w);
    if (entry->sme_bsb.label != XtName(w))
	XtFree(entry->sme_bsb.label);
}

/*      Function Name: Redisplay
 *      Description: Redisplays the contents of the widget.
 *      Arguments: w - the simple menu widget.
 *                 event - the X event that caused this redisplay.
 *                 region - the region the needs to be repainted.
 *      Returns: none.
 */

/* ARGSUSED */
static void
Redisplay(Widget w, XEvent *event, Region region)
{
    GC gc;
    SmeBSBObject entry = (SmeBSBObject) w;
    Dimension s;
    int	font_ascent = 0, font_descent = 0, y_loc;
    int	fontset_ascent = 0, fontset_descent = 0;
    XFontSetExtents *ext = XExtentsOfFontSet(entry->sme_bsb.fontset);

#ifdef XAW3D
    s = entry->sme_threeD.shadow_width;
#else
    s = 0;
#endif /* XAW3D */

    entry->sme_bsb.set_values_area_cleared = FALSE;
#if (XtVersion >= 11006)
    if ( entry->sme.international == True ) {
        fontset_ascent = abs(ext->max_ink_extent.y);
        fontset_descent = ext->max_ink_extent.height - fontset_ascent;
    } else { /*else, compute size from font like R5*/
        font_ascent = entry->sme_bsb.font->max_bounds.ascent;
        font_descent = entry->sme_bsb.font->max_bounds.descent;
    }
#else
    font_ascent = entry->sme_bsb.font->max_bounds.ascent;
    font_descent = entry->sme_bsb.font->max_bounds.descent;
#endif /* XtVersion R6 */
    y_loc = entry->rectangle.y;


    if (XtIsSensitive(w) && XtIsSensitive( XtParent(w) ) ) {
#ifndef XAW3D
	if ( w == XawSimpleMenuGetActiveEntry(XtParent(w)) ) {
	    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),
			   entry->sme_bsb.norm_gc, 0, y_loc,
			   (unsigned int) entry->rectangle.width,
			   (unsigned int) entry->rectangle.height);
	    gc = entry->sme_bsb.rev_gc;
	}
	else
#endif /* XAW3D */
	    gc = entry->sme_bsb.norm_gc;
    }
    else
	gc = entry->sme_bsb.norm_gray_gc;

    if (entry->sme_bsb.label != NULL) {
	int x_loc = entry->sme_bsb.left_margin;
	int len = strlen(entry->sme_bsb.label);
	char * label = entry->sme_bsb.label;

	switch(entry->sme_bsb.justify) {
	    int width, t_width;

	case XtJustifyCenter:
#if (XtVersion >= 11006)
            if ( entry->sme.international == True ) {
	        t_width = XmbTextEscapement(entry->sme_bsb.fontset,label,len);
                width = entry->rectangle.width - (entry->sme_bsb.left_margin +
					      entry->sme_bsb.right_margin);
            } else
#endif /* XtVersion >= 11006 */
	    {
	        t_width = XTextWidth(entry->sme_bsb.font, label, len);
	        width = entry->rectangle.width - (entry->sme_bsb.left_margin +
					      entry->sme_bsb.right_margin);
            }
	    x_loc += (width - t_width)/2;
	    break;
	case XtJustifyRight:
#if (XtVersion >= 11006)
            if ( entry->sme.international == True ) {
                t_width = XmbTextEscapement(entry->sme_bsb.fontset,label,len);
                x_loc = entry->rectangle.width - ( entry->sme_bsb.right_margin
						 + t_width );
            } else
#endif /* XtVersion >= 11006 */
	    {
	        t_width = XTextWidth(entry->sme_bsb.font, label, len);
	        x_loc = entry->rectangle.width - ( entry->sme_bsb.right_margin
						 + t_width );
            }
	    break;
	case XtJustifyLeft:
	default:
	    break;
	}


	/* this will center the text in the gadget top-to-bottom */

#if (XtVersion >= 11006)
        if ( entry->sme.international==True ) {
            y_loc += ((int)entry->rectangle.height -
		  (fontset_ascent + fontset_descent)) / 2 + fontset_ascent;

            XmbDrawString(XtDisplayOfObject(w), XtWindowOfObject(w),
                entry->sme_bsb.fontset, gc, x_loc + s, y_loc, label, len);
        } else
#endif /* XtVersion >= 11006 */
	{
            y_loc += ((int)entry->rectangle.height -
		  (font_ascent + font_descent)) / 2 + font_ascent;

            XDrawString(XtDisplayOfObject(w), XtWindowOfObject(w), gc,
		    x_loc + s, y_loc, label, len);
        }
    }

    DrawBitmaps(w, gc);
}


/*      Function Name: SetValues
 *      Description: Relayout the menu when one of the resources is changed.
 *      Arguments: current - current state of the widget.
 *                 request - what was requested.
 *                 new - what the widget will become.
 *      Returns: none
 */

/* ARGSUSED */
static Boolean
SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
    SmeBSBObject entry = (SmeBSBObject) new;
    SmeBSBObject old_entry = (SmeBSBObject) current;
    Boolean ret_val = FALSE;

    if (old_entry->sme_bsb.label != entry->sme_bsb.label) {
        if (old_entry->sme_bsb.label != XtName( new ) )
	    XtFree( (char *) old_entry->sme_bsb.label );

	if (entry->sme_bsb.label != XtName(new) )
	    entry->sme_bsb.label = XtNewString( entry->sme_bsb.label );

	ret_val = True;
    }

    if (entry->rectangle.sensitive != old_entry->rectangle.sensitive )
	ret_val = TRUE;

    if (entry->sme_bsb.left_bitmap != old_entry->sme_bsb.left_bitmap) {
	GetBitmapInfo(new, TRUE);
	ret_val = TRUE;
    }

    if (entry->sme_bsb.right_bitmap != old_entry->sme_bsb.right_bitmap) {
	GetBitmapInfo(new, FALSE);
	ret_val = TRUE;
    }

#if (XtVersion >= 11006)
    if (  (	(old_entry->sme_bsb.font != entry->sme_bsb.font) &&
	(old_entry->sme.international == False )	          ) ||
	(old_entry->sme_bsb.foreground != entry->sme_bsb.foreground) )
#else
    if (  (old_entry->sme_bsb.font != entry->sme_bsb.font) ||
	(old_entry->sme_bsb.foreground != entry->sme_bsb.foreground) )
#endif /* XtVersion >= 11006 */
	{
	    DestroyGCs(current);
	    CreateGCs(new);
	    ret_val = TRUE;
	}

#if (XtVersion >= 11006)
    if ( ( old_entry->sme_bsb.fontset != entry->sme_bsb.fontset) &&
				(old_entry->sme.international == True ) )
        /* DONT changes the GCs, because the fontset is not in them. */
        ret_val = TRUE;
#endif /* XtVersion >= 11006 */

    if (ret_val) {
	GetDefaultSize(new,
		       &(entry->rectangle.width), &(entry->rectangle.height));
	entry->sme_bsb.set_values_area_cleared = TRUE;
    }
    return(ret_val);
}

/*	Function Name: QueryGeometry.
 *	Description: Returns the preferred geometry for this widget.
 *	Arguments: w - the menu entry object.
 *                 itended, return_val - the intended and return geometry info.
 *	Returns: A Geometry Result.
 *
 * See the Intrinsics manual for details on what this function is for.
 *
 * I just return the height and width of the label plus the margins.
 */

static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *return_val)
{
    SmeBSBObject entry = (SmeBSBObject) w;
    Dimension width, height;
    XtGeometryResult ret_val = XtGeometryYes;
    XtGeometryMask mode = intended->request_mode;

    GetDefaultSize(w, &width, &height );

    if ( ((mode & CWWidth) && (intended->width != width)) ||
	 !(mode & CWWidth) ) {
	return_val->request_mode |= CWWidth;
	return_val->width = width;
	ret_val = XtGeometryAlmost;
    }

    if ( ((mode & CWHeight) && (intended->height != height)) ||
	 !(mode & CWHeight) ) {
	return_val->request_mode |= CWHeight;
	return_val->height = height;
	ret_val = XtGeometryAlmost;
    }

    if (ret_val == XtGeometryAlmost) {
	mode = return_val->request_mode;

	if ( ((mode & CWWidth) && (width == entry->rectangle.width)) &&
	     ((mode & CWHeight) && (height == entry->rectangle.height)) )
	    return(XtGeometryNo);
    }

    return(ret_val);
}

/*      Function Name: FlipColors
 *      Description: Invert the colors of the current entry.
 *      Arguments: w - the bsb menu entry widget.
 *      Returns: none.
 */

static void
FlipColors(Widget w)
{
    SmeBSBObject entry = (SmeBSBObject) w;
    SmeBSBObjectClass oclass = (SmeBSBObjectClass) XtClass (w);

#ifdef XAW3D
    if (entry->sme_bsb.set_values_area_cleared) {
	entry->sme_threeD.shadowed = False;
	return;
    }

    if (entry->sme_threeD.shadow_width > 0) {
	entry->sme_threeD.shadowed ^= True;
	(*oclass->sme_threeD_class.shadowdraw) (w);
    } else
#endif /* XAW3D */
	XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),
		   entry->sme_bsb.invert_gc,
		   0, (int) entry->rectangle.y,
		   (unsigned int) entry->rectangle.width,
		   (unsigned int) entry->rectangle.height);
}

/************************************************************
 *
 * Private Functions.
 *
 ************************************************************/

/*	Function Name: GetDefaultSize
 *	Description: Calculates the Default (preferred) size of
 *                   this menu entry.
 *	Arguments: w - the menu entry widget.
 *                 width, height - default sizes (RETURNED).
 *	Returns: none.
 */

static void
GetDefaultSize(Widget w, Dimension *width, Dimension *height)
{
    SmeBSBObject entry = (SmeBSBObject) w;

#if (XtVersion >= 11006)
    if ( entry->sme.international == True ) {
        XFontSetExtents *ext = XExtentsOfFontSet(entry->sme_bsb.fontset);
        if (entry->sme_bsb.label == NULL)
	    *width = 0;
        else
	    *width = XmbTextEscapement(entry->sme_bsb.fontset, entry->sme_bsb.label,
			    strlen(entry->sme_bsb.label));
        *width += entry->sme_bsb.left_margin + entry->sme_bsb.right_margin;
#ifdef XAW3D
        *width += (2 * entry->sme_threeD.shadow_width);
#endif /* XAW3D */
        *height = ext->max_ink_extent.height;
        *height = ((int)*height * ( ONE_HUNDRED +
			        entry->sme_bsb.vert_space )) / ONE_HUNDRED;
#ifdef XAW3D
        *height += (2 * entry->sme_threeD.shadow_width);
#endif /* XAW3D */
    }
    else
#endif /* XtVersion >= 11006 */
    {
        if (entry->sme_bsb.label == NULL)
	    *width = 0;
        else
	    *width = XTextWidth(entry->sme_bsb.font, entry->sme_bsb.label,
			    strlen(entry->sme_bsb.label));

        *width += entry->sme_bsb.left_margin + entry->sme_bsb.right_margin;
#ifdef XAW3D
        *width += (2 * entry->sme_threeD.shadow_width);
#endif /* XAW3D */

        *height = (entry->sme_bsb.font->max_bounds.ascent +
	       entry->sme_bsb.font->max_bounds.descent);

        *height = ((int)*height * ( ONE_HUNDRED +
			        entry->sme_bsb.vert_space )) / ONE_HUNDRED;
#ifdef XAW3D
        *height += (2 * entry->sme_threeD.shadow_width);
#endif /* XAW3D */
    }
}

/*      Function Name: DrawBitmaps
 *      Description: Draws left and right bitmaps.
 *      Arguments: w - the simple menu widget.
 *                 gc - graphics context to use for drawing.
 *      Returns: none
 */

static void
DrawBitmaps(Widget w, GC gc)
{
    int x_loc, y_loc;
    SmeBSBObject entry = (SmeBSBObject) w;

    if ( (entry->sme_bsb.left_bitmap == None) &&
	 (entry->sme_bsb.right_bitmap == None) ) return;

/*
 * Draw Left Bitmap.
 */

  if (entry->sme_bsb.left_bitmap != None) {
    x_loc = (int)(entry->sme_bsb.left_margin -
		      entry->sme_bsb.left_bitmap_width) / 2;
#ifdef XAW3D
    x_loc += entry->sme_threeD.shadow_width;
#endif /* XAW3D */

    y_loc = entry->rectangle.y +
		(int)(entry->rectangle.height -
		      entry->sme_bsb.left_bitmap_height) / 2;

    XCopyPlane(XtDisplayOfObject(w), entry->sme_bsb.left_bitmap,
	       XtWindowOfObject(w), gc, 0, 0,
	       entry->sme_bsb.left_bitmap_width,
	       entry->sme_bsb.left_bitmap_height, x_loc, y_loc, 1);
  }

/*
 * Draw Right Bitmap.
 */


  if (entry->sme_bsb.right_bitmap != None) {
    x_loc = entry->rectangle.width -
		(int)(entry->sme_bsb.right_margin +
		      entry->sme_bsb.right_bitmap_width) / 2;
#ifdef XAW3D
    x_loc -= entry->sme_threeD.shadow_width;
#endif /* XAW3D */

    y_loc = entry->rectangle.y +
		(int)(entry->rectangle.height -
		      entry->sme_bsb.right_bitmap_height) / 2;

    XCopyPlane(XtDisplayOfObject(w), entry->sme_bsb.right_bitmap,
	       XtWindowOfObject(w), gc, 0, 0,
	       entry->sme_bsb.right_bitmap_width,
	       entry->sme_bsb.right_bitmap_height, x_loc, y_loc, 1);
  }
}

/*      Function Name: GetBitmapInfo
 *      Description: Gets the bitmap information from either of the bitmaps.
 *      Arguments: w - the bsb menu entry widget.
 *                 is_left - TRUE if we are testing left bitmap,
 *                           FALSE if we are testing the right bitmap.
 *      Returns: none
 */

static void
GetBitmapInfo(Widget w, Boolean is_left)
{
    SmeBSBObject entry = (SmeBSBObject) w;
    unsigned int depth, bw;
    Window root;
    int x, y;
    unsigned int width, height;
    char buf[BUFSIZ];

    if (is_left) {
	if (entry->sme_bsb.left_bitmap != None) {
	    if (!XGetGeometry(XtDisplayOfObject(w),
			      entry->sme_bsb.left_bitmap, &root,
			      &x, &y, &width, &height, &bw, &depth)) {
		(void) sprintf(buf, "Xaw SmeBSB Object: %s %s \"%s\".", "Could not",
			"get Left Bitmap geometry information for menu entry ",
			XtName(w));
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    if (depth != 1) {
		(void) sprintf(buf, "Xaw SmeBSB Object: %s \"%s\"%s.",
			"Left Bitmap of entry ",
			XtName(w), " is not one bit deep.");
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    entry->sme_bsb.left_bitmap_width = (Dimension) width;
	    entry->sme_bsb.left_bitmap_height = (Dimension) height;
	}
    }
    else if (entry->sme_bsb.right_bitmap != None) {
	if (!XGetGeometry(XtDisplayOfObject(w),
			  entry->sme_bsb.right_bitmap, &root,
			  &x, &y, &width, &height, &bw, &depth)) {
	    (void) sprintf(buf, "Xaw SmeBSB Object: %s %s \"%s\".", "Could not",
		    "get Right Bitmap geometry information for menu entry ",
		    XtName(w));
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	if (depth != 1) {
	    (void) sprintf(buf, "Xaw SmeBSB Object: %s \"%s\"%s.",
		    "Right Bitmap of entry ", XtName(w),
		    " is not one bit deep.");
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	entry->sme_bsb.right_bitmap_width = (Dimension) width;
	entry->sme_bsb.right_bitmap_height = (Dimension) height;
    }
}

/*      Function Name: CreateGCs
 *      Description: Creates all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
CreateGCs(Widget w)
{
    SmeBSBObject entry = (SmeBSBObject) w;
    XGCValues values;
    XtGCMask mask, mask_i18n;

    values.foreground = XtParent(w)->core.background_pixel;
    values.background = entry->sme_bsb.foreground;
    values.font = entry->sme_bsb.font->fid;
    values.graphics_exposures = FALSE;
    mask      = GCForeground | GCBackground | GCGraphicsExposures | GCFont;
    mask_i18n = GCForeground | GCBackground | GCGraphicsExposures;
#if (XtVersion >= 11006)
    if ( entry->sme.international == True )
        entry->sme_bsb.rev_gc = XtAllocateGC(w, 0, mask_i18n, &values, GCFont, 0 );
    else
#endif /* XtVersion >= 11006 */
        entry->sme_bsb.rev_gc = XtGetGC(w, mask, &values);

    values.foreground = entry->sme_bsb.foreground;
    values.background = XtParent(w)->core.background_pixel;
#if (XtVersion >= 11006)
    if ( entry->sme.international == True )
        entry->sme_bsb.norm_gc = XtAllocateGC(w, 0, mask_i18n, &values, GCFont, 0 );
    else
#endif /* XtVersion >= 11006 */
        entry->sme_bsb.norm_gc = XtGetGC(w, mask, &values);

    values.fill_style = FillTiled;
    values.tile   = XmuCreateStippledPixmap(XtScreenOfObject(w),
					    entry->sme_bsb.foreground,
					    XtParent(w)->core.background_pixel,
					    XtParent(w)->core.depth);
    values.graphics_exposures = FALSE;
    mask |= GCTile | GCFillStyle;
#if (XtVersion >= 11006)
    if ( entry->sme.international == True )
        entry->sme_bsb.norm_gray_gc = XtAllocateGC(w, 0, mask_i18n, &values, GCFont, 0 );
    else
#endif /* XtVersion >= 11006 */
        entry->sme_bsb.norm_gray_gc = XtGetGC(w, mask, &values);

    values.foreground ^= values.background;
    values.background = 0;
    values.function = GXxor;
    mask = GCForeground | GCBackground | GCGraphicsExposures | GCFunction;
    entry->sme_bsb.invert_gc = XtGetGC(w, mask, &values);
}

/*      Function Name: DestroyGCs
 *      Description: Removes all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
DestroyGCs(Widget w)
{
    SmeBSBObject entry = (SmeBSBObject) w;

    XtReleaseGC(w, entry->sme_bsb.norm_gc);
    XtReleaseGC(w, entry->sme_bsb.norm_gray_gc);
    XtReleaseGC(w, entry->sme_bsb.rev_gc);
    XtReleaseGC(w, entry->sme_bsb.invert_gc);
}

#ifdef apollo

/*
 * The apollo compiler that we have optomizes out my code for
 * FlipColors() since it is static. and no one executes it in this
 * file.  I am setting the function pointer into the class structure so
 * that it can be called by my parent who will tell me to when to
 * highlight and unhighlight.
 */

void _XawSmeBSBApolloHack ()
{
    FlipColors();
}
#endif /* apollo */

/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1989-2000 by Brian V. Smith
 * Parts Copyright (c) 1990 by Digital Equipment Corporation. All Rights Reserved.
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1990 by Digital Equipment Corporation
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
 * Original xdir code:
 *
 *	Created: 13 Aug 88
 *
 *	Win Treese
 *	Cambridge Research Lab
 *	Digital Equipment Corporation
 *	treese@crl.dec.com
 *
 *	    COPYRIGHT 1990
 *	  DIGITAL EQUIPMENT CORPORATION
 *	   MAYNARD, MASSACHUSETTS
 *	  ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE
 * FOR ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED
 * WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 *
 */

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "w_browse.h"
#include "w_dir.h"
#include "w_drawprim.h"		/* for max_char_height */
#include "w_export.h"
#include "w_file.h"
#include "w_listwidget.h"
#include "w_setup.h"
#include "w_util.h"
#ifdef HAVE_NO_DIRENT
#include <sys/dir.h>
#else
#include <dirent.h>
#endif

static char	CurrentSelectionName[PATH_MAX];
static int	file_entry_cnt, dir_entry_cnt;
static char   **file_list, **dir_list;
static char   **filelist, **dirlist;
static char    *dirmask;

static Widget	hidden;
static Boolean	show_hidden = False;

/* Functions */

void		DoChangeDir(),
		SetDir(),
		Rescan(),
		CallbackRescan();

static void	ParentDir();

/* Static variables */

DeclareStaticArgs(15);
static String	dir_translations =
		    "<Key>Return: SetDir()\n\
		    Ctrl<Key>X: EmptyTextKey()\n\
		    <Key>F18: PastePanelKey()";

static String	list_panel_translations =
		    "<Btn3Up>: ParentDir()";

static String	mask_text_translations =
		    "<Key>Return: Rescan()\n\
		    Ctrl<Key>J: Rescan()\n\
		    Ctrl<Key>M: Rescan()";

static XtActionsRec actionTable[] = {
    {"ParentDir", (XtActionProc)ParentDir},
    {"SetDir", (XtActionProc)SetDir},
    {"Rescan", (XtActionProc)Rescan},
};

/* Function:	FileSelected() is called when the user selects a file.
 *		Set the global variable "CurrentSelectionName"
 *		and set either the export or file panel file name, whichever is popped up
 *		Also, for the file popup, show a preview of the figure.
 * Arguments:	Standard Xt callback arguments.
 * Returns:	Nothing.
 * Notes:
 */

void
FileSelected(w, client_data, call_data)
    Widget	    w;
    XtPointer	    client_data;
    XtPointer	    call_data;
{
    XawListReturnStruct *ret_struct = (XawListReturnStruct *) call_data;

    strcpy(CurrentSelectionName, ret_struct->string);
    FirstArg(XtNstring, CurrentSelectionName);
    if (browse_up) {
	SetValues(browse_selfile);
		XawTextSetInsertionPoint(browse_selfile, strlen(CurrentSelectionName));
    } else if (file_up) {
	SetValues(file_selfile);
		XawTextSetInsertionPoint(file_selfile, strlen(CurrentSelectionName));
	/* and show a preview of the figure in the preview canvas */
	preview_figure(CurrentSelectionName, file_popup, preview_widget,
			preview_size, preview_pixmap);
    } else if (export_up) {
	SetValues(exp_selfile);
		XawTextSetInsertionPoint(exp_selfile, strlen(CurrentSelectionName));
    }
    /* if nothing is selected it probably means that the user was impatient and
	double clicked the file name again while it was still loading the first */
}

/* Function:	DirSelected() is called when the user selects a directory.
 *
 * Arguments:	Standard Xt callback arguments.
 * Returns:	Nothing.
 * Notes:
 */

void
DirSelected(w, client_data, call_data)
    Widget	    w;
    XtPointer	    client_data;
    XtPointer	    call_data;
{
    XawListReturnStruct *ret_struct = (XawListReturnStruct *) call_data;

    strcpy(CurrentSelectionName, ret_struct->string);
    DoChangeDir(CurrentSelectionName);
}

void
ShowHidden(w, client_data, ret_val)
    Widget	    w;
    XtPointer	    client_data;
    XtPointer	    ret_val;
{
    show_hidden = !show_hidden;
    FirstArg(XtNlabel, show_hidden? "Hide Hidden": "Show Hidden");
    SetValues(hidden);
    /* make it rescan the current directory */
    DoChangeDir(".");
}

void
GoHome(w, client_data, ret_val)
    Widget	    w;
    XtPointer	    client_data;
    XtPointer	    ret_val;
{
     parseuserpath("~",cur_dir);
     DoChangeDir(cur_dir);
}

/*
   come here when the user presses return in the directory path widget
   Get the current string from the widget and set the current directory to that
   Also, copy the dir to the current directory widget in the file popup
*/

/* Function:  SetDir() changes to the parent directory.
 * Arguments: Standard Xt action arguments.
 * Returns:   Nothing.
 * Notes:
 */

void
SetDir(widget, event, params, num_params)
    Widget	    widget;
    XEvent	   *event;
    String	   *params;
    Cardinal	   *num_params;
{
    char	   *ndir;

    /* get the string from the widget */
    FirstArg(XtNstring, &ndir);
    if (browse_up) {
	GetValues(browse_dir);
	strcpy(browse_cur_dir,ndir);
    } else if (file_up) {
	GetValues(file_dir);
    } else if (export_up) {
	GetValues(exp_dir);
	strcpy(export_cur_dir,ndir);	/* save in global var */
    }
    /* if there is a ~ in the directory, parse the username */
    if (ndir[0]=='~') {
	char longdir[PATH_MAX];
	parseuserpath(ndir,longdir);
	ndir=longdir;
    }
    DoChangeDir(ndir);
}

/* make the full path from ~/partialpath */
parseuserpath(path,longpath)
char *path,*longpath;
{
    char	  *p;
    struct passwd *who;

    /* this user's home */
    if (strlen(path)==1 || path[1]=='/') {
	strcpy(longpath,getenv("HOME"));
	if (strlen(path)==1)		/* nothing after the ~, we have the full path */
		return;
	strcat(longpath,&path[1]);	/* append the rest of the path */
	return;
    }
    /* another user name after ~ */
    strcpy(longpath,&path[1]);
    p=strchr(longpath,'/');
    if (p)
	    *p='\0';
    who = getpwnam(longpath);
    if (!who) {
	file_msg("No such user: %s",longpath);
	strcpy(longpath,path);
    } else {
	strcpy(longpath,who->pw_dir);
	p=strchr(path,'/');
	if (p)
		strcat(longpath,p);	/* attach stuff after the / */
    }
}

static Boolean      actions_added=False;

/* the file_exp parm just changes the vertical offset for the Rescan button */
/* make the filename list file_width wide */

void
create_dirinfo(file_exp, parent, below, ret_beside, ret_below,
	       mask_w, dir_w, flist_w, dlist_w, file_width, file_panel)
    Boolean	    file_exp;
    Widget	    parent, below, *ret_beside, *ret_below, *mask_w, *dir_w,
		   *flist_w, *dlist_w;
    int		    file_width;
    Boolean	    file_panel;

{
    Widget	    w,dir_alt,home;
    Widget	    file_viewport;
    Widget	    dir_viewport;
    XFontStruct	   *temp_font;
    int		    char_ht,char_wd;

    dir_entry_cnt = NENTRIES;
    file_entry_cnt = NENTRIES;
    filelist = (char **) calloc(file_entry_cnt, sizeof(char *));
    dirlist = (char **) calloc(dir_entry_cnt, sizeof(char *));

    get_directory(cur_dir);

    if (file_panel) {
	FirstArg(XtNlabel, "Fig files");
    } else {
	FirstArg(XtNlabel, " Alternatives");
    }
    NextArg(XtNfromVert, below);
    NextArg(XtNborderWidth, 0);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    w = XtCreateManagedWidget("file_alt_label", labelWidgetClass,
			      parent, Args, ArgCount);

    FirstArg(XtNfont, &temp_font);
    GetValues(w);
    char_ht = max_char_height(temp_font) + 2;
    char_wd = char_width(temp_font) + 2;

    /* make a viewport to hold the list widget of filenames */
    FirstArg(XtNallowVert, True);
    if (file_panel) {
	/* for the file panel, put the viewport below the Alternatives label */
	NextArg(XtNfromVert, w);
	NextArg(XtNheight, char_ht * 15);	/* show first 15 filenames */
    } else {
	/* for the export or browse panel, put the viewport beside the Alternatives label */
	NextArg(XtNfromVert, below);
	NextArg(XtNfromHoriz, w);
	NextArg(XtNheight, char_ht * 10);	/* show 10 lines */
    }
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNwidth, file_width);
    NextArg(XtNtop, XtChainTop);	/* chain the viewport resizes fully */
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainRight);
    file_viewport = XtCreateManagedWidget("vport", viewportWidgetClass,
					  parent, Args, ArgCount);

    /* label for filename mask */

    FirstArg(XtNlabel, "Filename Mask");
    NextArg(XtNborderWidth, 0);
    NextArg(XtNfromVert, file_viewport);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    w = XtCreateManagedWidget("mask_label", labelWidgetClass, 
				parent, Args, ArgCount);

    /* text widget for the filename mask */

    FirstArg(XtNeditType, XawtextEdit);
    NextArg(XtNscrollHorizontal, XawtextScrollNever);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNscrollVertical, XawtextScrollNever);
    NextArg(XtNresize, XawtextResizeWidth);
    NextArg(XtNwidth, file_panel? F_FILE_WIDTH: E_FILE_WIDTH);
    NextArg(XtNfromHoriz, w);
    NextArg(XtNfromVert, file_viewport);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainRight);
    *mask_w = XtCreateManagedWidget("mask", asciiTextWidgetClass, 
					parent, Args, ArgCount);
    XtOverrideTranslations(*mask_w, XtParseTranslationTable(mask_text_translations));

    /* get the first directory listing */

    FirstArg(XtNstring, &dirmask);
    GetValues(*mask_w);
    if (MakeFileList(cur_dir, dirmask, &dir_list, &file_list) == False)
	file_msg("No files in directory?");

    FirstArg(XtNlabel, "  Current Dir");
    NextArg(XtNborderWidth, 0);
    NextArg(XtNfromVert, *mask_w);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    w = XtCreateManagedWidget("dir_label", labelWidgetClass,
			      parent, Args, ArgCount);

    FirstArg(XtNstring, cur_dir);
    NextArg(XtNinsertPosition, strlen(cur_dir));
    NextArg(XtNheight, char_ht * 2);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNscrollHorizontal, XawtextScrollWhenNeeded);
    NextArg(XtNeditType, XawtextEdit);
    NextArg(XtNfromVert, *mask_w);
    NextArg(XtNfromHoriz, w);
    NextArg(XtNwidth, file_panel? F_FILE_WIDTH: E_FILE_WIDTH);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainRight);
    *dir_w = XtCreateManagedWidget("dir_name", asciiTextWidgetClass,
				   parent, Args, ArgCount);

    XtOverrideTranslations(*dir_w, XtParseTranslationTable(dir_translations));

    /* directory alternatives */

    FirstArg(XtNlabel, "  Directories");
    NextArg(XtNborderWidth, 0);
    NextArg(XtNfromVert, *dir_w);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    dir_alt = XtCreateManagedWidget("dir_alt_label", labelWidgetClass,
			      parent, Args, ArgCount);

    /* put a Home button to the left of the list of directories */
    FirstArg(XtNlabel, "Home");
    NextArg(XtNfromVert, dir_alt);
    NextArg(XtNfromHoriz, dir_alt);
    NextArg(XtNhorizDistance, -(char_wd * 5));
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    home = XtCreateManagedWidget("home", commandWidgetClass, parent, Args, ArgCount);
    XtAddCallback(home, XtNcallback, GoHome, (XtPointer) NULL);

    /* put a button for showing/hiding hidden files below the Home button */

    FirstArg(XtNlabel, show_hidden? "Hide Hidden": "Show Hidden");
    NextArg(XtNfromVert, home);
    NextArg(XtNfromHoriz, dir_alt);
    NextArg(XtNhorizDistance, (int) -(char_wd * 10.5));
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    hidden = XtCreateManagedWidget("hidden", commandWidgetClass, 
				parent, Args, ArgCount);
    XtAddCallback(hidden, XtNcallback, ShowHidden, (XtPointer) NULL);

    FirstArg(XtNallowVert, True);
    NextArg(XtNfromHoriz, dir_alt);
    NextArg(XtNfromVert, *dir_w);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNwidth, file_panel? F_FILE_WIDTH: E_FILE_WIDTH);
    NextArg(XtNheight, char_ht * 5);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainRight);
    dir_viewport = XtCreateManagedWidget("dirvport", viewportWidgetClass,
					 parent, Args, ArgCount);

    FirstArg(XtNlist, file_list);
    /* for file panel use only one column */
    if (file_panel) {
	NextArg(XtNforceColumns, True);
	NextArg(XtNdefaultColumns, 1);
    }
    *flist_w = XtCreateManagedWidget("file_list_panel", figListWidgetClass,
				     file_viewport, Args, ArgCount);
    XtAddCallback(*flist_w, XtNcallback, FileSelected, (XtPointer) NULL);
    XtOverrideTranslations(*flist_w,
			   XtParseTranslationTable(list_panel_translations));

    FirstArg(XtNlist, dir_list);
    *dlist_w = XtCreateManagedWidget("dir_list_panel", figListWidgetClass,
				     dir_viewport, Args, ArgCount);

    XtOverrideTranslations(*dlist_w,
			   XtParseTranslationTable(list_panel_translations));
    XtAddCallback(*dlist_w, XtNcallback, DirSelected, (XtPointer) NULL);

    if (!actions_added) {
	XtAppAddActions(tool_app, actionTable, XtNumber(actionTable));
	actions_added = True;
    }

    FirstArg(XtNlabel, "Rescan");
    NextArg(XtNfromVert, dir_viewport);
    NextArg(XtNvertDistance, 15);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNhorizDistance, 45);
    NextArg(XtNheight, 25);
    NextArg(XtNtop, XtChainBottom);
    NextArg(XtNbottom, XtChainBottom);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    w = XtCreateManagedWidget("rescan", commandWidgetClass, parent,
			      Args, ArgCount);
    XtAddCallback(w, XtNcallback, CallbackRescan, (XtPointer) NULL);

    /* install accelerators so they can be used from each window */
    XtInstallAccelerators(parent, w);
    XtInstallAccelerators(*flist_w, parent);
    XtInstallAccelerators(*dlist_w, parent);

    *ret_beside = w;
    *ret_below = dir_viewport;
    return;
}

/* Function:	SPComp() compares two string pointers for qsort().
 * Arguments:	s1, s2: strings to be compared.
 * Returns:	Value of strcmp().
 * Notes:
 */

static int
SPComp(s1, s2)
    char	  **s1, **s2;
{
    return (strcmp(*s1, *s2));
}

#define MAX_MASKS 20

Boolean
MakeFileList(dir_name, mask, dir_list, file_list)
    char	   *dir_name;
    char	   *mask, ***dir_list, ***file_list;
{
    DIR		  *dirp;
    DIRSTRUCT	  *dp;
    char	 **cur_file, **cur_directory;
    char	 **last_file, **last_dir;
    int		   nmasks,i;
    char	  *wild[MAX_MASKS],*cmask;
    Boolean	   match;

    set_temp_cursor(wait_cursor);
    cur_file = filelist;
    cur_directory = dirlist;
    last_file = filelist + file_entry_cnt - 1;
    last_dir = dirlist + dir_entry_cnt - 1;

    dirp = opendir(dir_name);
    if (dirp == NULL) {
	reset_cursor();
	*file_list = filelist;
	*file_list[0]="";
	*dir_list = dirlist;
	*dir_list[0]="..";
	return False;
    }
    /* process any events to ensure cursor is set to wait_cursor */
    /*
     * don't do this inside the following loop because this procedure could
     * be re-entered if the user presses (e.g.) rescan
     */
    app_flush();

    /* make copy of mask */
    cmask = strdup(mask);
    if ((cmask == NULL) || (*cmask == '\0'))
	strcpy(cmask,"*");
    wild[0] = strtok(cmask," \t");
    nmasks = 1;
    while ((wild[nmasks]=strtok((char*) NULL, " \t")) && nmasks < MAX_MASKS)
	nmasks++;
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	/* skip over '.' (current dir) */
	if (!strcmp(dp->d_name, "."))
	    continue;
	/* if don't want to see the hidden files (beginning with ".") skip them */
	if (!show_hidden && dp->d_name[0]=='.' && strcmp(dp->d_name,".."))
	    continue;

	if (IsDirectory(dir_name, dp->d_name)) {
	    *cur_directory++ = strdup(dp->d_name);
	    if (cur_directory == last_dir) {	/* out of space, make more */
		dirlist = (char **) realloc(dirlist,
					2 * dir_entry_cnt * sizeof(char *));
		cur_directory = dirlist + dir_entry_cnt - 1;
		dir_entry_cnt = 2 * dir_entry_cnt;
		last_dir = dirlist + dir_entry_cnt - 1;
	    }
	} else {
	    /* check if matches regular expression */
	    match=False;
	    for (i=0; i<nmasks; i++) {
		if (wild_match(dp->d_name, wild[i])) {
		    match = True;
		    break;
		}
	    }
	    if (!match)
		continue;	/* no, do next */
	    if (wild[i][0] == '*' && dp->d_name[0] == '.')
		continue;	/* skip files with leading . */
	    *cur_file++ = strdup(dp->d_name);
	    if (cur_file == last_file) {	/* out of space, make more */
		filelist = (char **) realloc(filelist,
				       2 * file_entry_cnt * sizeof(char *));
		cur_file = filelist + file_entry_cnt - 1;
		file_entry_cnt = 2 * file_entry_cnt;
		last_file = filelist + file_entry_cnt - 1;
	    }
	}
    }
    *cur_file = NULL;
    *cur_directory = NULL;
    if (cur_file != filelist)
	qsort(filelist, cur_file - filelist, sizeof(char *), (int(*)())SPComp);
    if (cur_directory != dirlist)
	qsort(dirlist, cur_directory - dirlist, sizeof(char *), (int(*)())SPComp);
    *file_list = filelist;
    *dir_list = dirlist;
    reset_cursor();
    closedir(dirp);
    free(cmask);	/* free copy of mask */
    return True;
}

/* Function:	ParentDir() changes to the parent directory.
 * Arguments:	Standard Xt action arguments.
 * Returns:	Nothing.
 * Notes:
 */

static void
ParentDir(w, event, params, num_params)
    Widget	    w;
    XEvent*	    event;
    String*	    params;
    Cardinal*	    num_params;
{
    DoChangeDir("..");
}

/* Function:	DoChangeDir() actually changes the directory and changes
 *		the list widget values to the new listing.
 * Arguments:	dir:	Pathname of new directory.
 * Returns:	Nothing.
 * Notes:
 *	NULL for dir means to rebuild the file list for the current directory
 *	(as in an update to the directory or change in filename filter).
 */

void
DoChangeDir(dir)
    char	   *dir;
{
    char	   *p;
    char	    ndir[PATH_MAX], tmpdir[PATH_MAX];

    
    strcpy(ndir, cur_dir);
    if (dir != NULL && dir[0] != '/') { /* relative path, prepend current dir */
	if (dir[strlen(dir) - 1] == '/')
	    dir[strlen(dir) - 1] = '\0';
	if (!strcmp(dir, "..")) {	/* Parent directory. */
	    if (*ndir == '\0')
		return;			/* no current directory, */
					/* can't do anything unless absolute path */
	    p = strrchr(ndir, '/');
	    *p = EOS;
	    if (ndir[0] == EOS)
		strcpy(ndir, "/");
	} else {
	    if (strcmp(ndir, "/"))	/* At the root already */
		strcat(ndir, "/");
	    strcat(ndir, dir);
	}
    } else {
	strcpy(ndir, dir);		/* abs path copy to ndir */
    }
    strcpy(tmpdir, cur_dir);		/* save cur_dir in case ndir is bad */
    strcpy(cur_dir, ndir);
    if (change_directory(cur_dir) != 0 ) {
	strcpy(cur_dir, tmpdir);	/* can't go there */
	return;
    } else if (MakeFileList(ndir, dirmask, &dirlist, &filelist) == False) {
	file_msg("Unable to list directory %s", ndir);
	strcpy(cur_dir, tmpdir);
	return;
    }

    FirstArg(XtNstring, cur_dir);
    /* update the current directory and file/dir list widgets */
    if (browse_up) {
	SetValues(browse_dir);
	strcpy(browse_cur_dir,cur_dir);	/* save in global var */
	XawTextSetInsertionPoint(browse_dir, strlen(cur_dir));
	NewList(browse_flist, filelist);
	NewList(browse_dlist, dirlist);
    } else if (file_up) {
	SetValues(file_dir);
	XawTextSetInsertionPoint(file_dir, strlen(cur_dir));
	NewList(file_flist,filelist);
	NewList(file_dlist,dirlist);
    } else if (export_up) {
	SetValues(exp_dir);
	strcpy(export_cur_dir,cur_dir);	/* save in global var */
	XawTextSetInsertionPoint(exp_dir, strlen(cur_dir));
	NewList(exp_flist, filelist);
	NewList(exp_dlist, dirlist);
    }
    CurrentSelectionName[0] = '\0';
}

void 
CallbackRescan(widget, closure, call_data)
    Widget    widget;
    XtPointer closure;
    XtPointer call_data;
{
     Rescan(0, 0, 0, 0);
}

void
Rescan(widget, event, params, num_params)
    Widget	widget;
    XEvent*	event;
    String*	params;
    Cardinal*	num_params;
{
    char	*dir;

    /*
     * get the mask string from the File or Export mask widget and put in
     * dirmask
     */
    if (browse_up) {
	FirstArg(XtNstring, &dirmask);
	GetValues(browse_mask);
	FirstArg(XtNstring, &dir);
	GetValues(browse_dir);
	if (change_directory(dir))	/* make sure we are there */
	    return;
	strcpy(browse_cur_dir,dir);	/* save in global var */
	(void) MakeFileList(dir, dirmask, &dir_list, &file_list);
	NewList(browse_flist, file_list);
	NewList(browse_dlist, dir_list);
    } else if (file_up) {
	FirstArg(XtNstring, &dirmask);
	GetValues(file_mask);
	FirstArg(XtNstring, &dir);
	GetValues(file_dir);
	if (change_directory(dir))	/* make sure we are there */
	    return;
	(void) MakeFileList(dir, dirmask, &dir_list, &file_list);
	NewList(file_flist,file_list);
	NewList(file_dlist,dir_list);
    } else if (export_up) {
	FirstArg(XtNstring, &dirmask);
	GetValues(exp_mask);
	FirstArg(XtNstring, &dir);
	GetValues(exp_dir);
	if (change_directory(dir))	/* make sure we are there */
	    return;
	strcpy(export_cur_dir,dir);	/* save in global var */
	(void) MakeFileList(dir, dirmask, &dir_list, &file_list);
	NewList(exp_flist, file_list);
	NewList(exp_dlist, dir_list);
    }
}

NewList(listwidget, list)
    Widget    listwidget;
    String   *list;
{

	/* delete scrollbars because of a bug in the viewport widget */
	FirstArg(XtNallowVert, False);
	NextArg(XtNallowHoriz, False);
	SetValues(XtParent(listwidget));

	/* install the new list */
	XawListChange(listwidget, list, 0, 0, True);

	/* re-enable the scrollbars */
	FirstArg(XtNallowVert, True);
	NextArg(XtNallowHoriz, True);
	SetValues(XtParent(listwidget));

	XawListChange(listwidget, list, 0, 0, True);
}

/* Function:	IsDirectory() tests to see if a pathname is a directory.
 * Arguments:	path:	Pathname of file to test.
 *		file:	Filename in path.
 * Returns:	True or False.
 * Notes:	False is returned if the directory is not accessible.
 */

Boolean
IsDirectory(path, file)
    char	   *path;
    char	   *file;
{
    char	    fullpath[PATH_MAX];
    struct stat	    statbuf;

    if (file == NULL)
	return (False);
    MakeFullPath(path, file, fullpath);
    if (stat(fullpath, &statbuf))	/* error, report that it is not a directory */
	return (False);
    if (statbuf.st_mode & S_IFDIR)
	return (True);
    else
	return (False);
}

/* Function:	MakeFullPath() creates the full pathname for the given file.
 * Arguments:	filename:	Name of the file in question.
 *		pathname:	Buffer for full name.
 * Returns:	Nothing.
 * Notes:
 */

void
MakeFullPath(root, filename, pathname)
    char	   *root;
    char	   *filename;
    char	   *pathname;
{
    strcpy(pathname, root);
    strcat(pathname, "/");
    strcat(pathname, filename);
}

/* wildmatch.c - Unix-style command line wildcards

   This procedure is in the public domain.

   After that, it is just as if the operating system had expanded the
   arguments, except that they are not sorted.	The program name and all
   arguments that are expanded from wildcards are lowercased.

   Syntax for wildcards:
   *		Matches zero or more of any character (except a '.' at
		the beginning of a name).
   ?		Matches any single character.
   [r3z]	Matches 'r', '3', or 'z'.
   [a-d]	Matches a single character in the range 'a' through 'd'.
   [!a-d]	Matches any single character except a character in the
		range 'a' through 'd'.

   The period between the filename root and its extension need not be
   given explicitly.  Thus, the pattern `a*e' will match 'abacus.exe'
   and 'axyz.e' as well as 'apple'.  Comparisons are not case sensitive.

   The wild_match code was written by Rich Salz, rsalz@bbn.com,
   posted to net.sources in November, 1986.

   The code connecting the two is by Mike Slomin, bellcore!lcuxa!mike2,
   posted to comp.sys.ibm.pc in November, 1988.

   Major performance enhancements and bug fixes, and source cleanup,
   by David MacKenzie, djm@ai.mit.edu. */

/* Shell-style pattern matching for ?, \, [], and * characters.
   I'm putting this replacement in the public domain.

   Written by Rich $alz, mirror!rs, Wed Nov 26 19:03:17 EST 1986. */

/* The character that inverts a character class; '!' or '^'. */
#define INVERT '!'

static int	star();

/* Return nonzero if `string' matches Unix-style wildcard pattern
   `pattern'; zero if not. */

int
wild_match(string, pattern)
    char	   *string, *pattern;
{
    int		    prev;	/* Previous character in character class. */
    int		    matched;	/* If 1, character class has been matched. */
    int		    reverse;	/* If 1, character class is inverted. */

    for (; *pattern; string++, pattern++)
	switch (*pattern) {
	case '\\':
	    /* Literal match with following character; fall through. */
	    pattern++;
	default:
	    if (*string != *pattern)
		return 0;
	    continue;
	case '?':
	    /* Match anything. */
	    if (*string == '\0')
		return 0;
	    continue;
	case '*':
	    /* Trailing star matches everything. */
	    return *++pattern ? star(string, pattern) : 1;
	case '[':
	    /* Check for inverse character class. */
	    reverse = pattern[1] == INVERT;
	    if (reverse)
		pattern++;
	    for (prev = 256, matched = 0; *++pattern && *pattern != ']';
		 prev = *pattern)
		if (*pattern == '-'
		    ? *string <= *++pattern && *string >= prev
		    : *string == *pattern)
		    matched = 1;
	    if (matched == reverse)
		return 0;
	    continue;
	}

    return *string == '\0';
}

static int
star(string, pattern)
    char	   *string, *pattern;
{
    while (wild_match(string, pattern) == 0)
	if (*++string == '\0')
	    return 0;
    return 1;
}

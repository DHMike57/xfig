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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "u_error.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>     /* includes X11/Xlib.h, which includes X11/X.h */

#include "resources.h"
#include "main.h"
#include "mode.h"

#include "f_save.h"
#include "f_util.h"
#include "w_cmdpanel.h"

#define MAXERRORS 6
#define MAXERRMSGLEN 512


static int	error_cnt = 0;


void error_handler(int err_sig)
{
    fprintf(stderr,"\nxfig%s: ", PACKAGE_VERSION);
    switch (err_sig) {
    case SIGHUP:
	fprintf(stderr, "SIGHUP signal trapped\n");
	break;
    case SIGFPE:
	fprintf(stderr, "SIGFPE signal trapped\n");
	break;
#ifdef SIGBUS
    case SIGBUS:
	fprintf(stderr, "SIGBUS signal trapped\n");
	break;
#endif /* SIGBUS */
    case SIGSEGV:
	fprintf(stderr, "SIGSEGV signal trapped\n");
	break;
    default:
	fprintf(stderr, "Unknown signal (%d)\n", err_sig);
	break;
    }
    emergency_quit(True);
}

int X_error_handler(Display *d, XErrorEvent *err_ev)
{
	(void)d;
    char	    err_msg[MAXERRMSGLEN];
    char	    ernum[10];

    /* uninstall error handlers so we don't recurse if another error happens! */
    XSetErrorHandler(NULL);
    XSetIOErrorHandler((XIOErrorHandler) NULL);
    if (!err_ev)
	return 0;
    XGetErrorText(tool_d, (int)(err_ev->error_code), err_msg, MAXERRMSGLEN - 1);
    (void) fprintf(stderr,
	   "xfig%s: X error trapped - error message follows:\n%s\n",
		PACKAGE_VERSION, err_msg);
    (void) sprintf(ernum, "%d", (int)err_ev->request_code);
    XGetErrorDatabaseText(tool_d, "XRequest", ernum, "<Unknown>", err_msg,
		MAXERRMSGLEN);
    (void) fprintf(stderr, "Request code: %s\n",err_msg);
    emergency_quit(True);
    return 0;
}

void emergency_quit(Boolean abortflag)
{
    if (++error_cnt > MAXERRORS) {
	fprintf(stderr, "xfig: too many errors - giving up.\n");
	exit(-1);
    }
    signal(SIGHUP, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
#ifdef SIGBUS
    signal(SIGBUS, SIG_DFL);
#endif /* SIGBUS */
    signal(SIGSEGV, SIG_DFL);

    aborting = abortflag;
    if (figure_modified && !emptyfigure()) {
	fprintf(stderr, "xfig: attempting to save figure\n");
	if (emergency_save("SAVE.fig") == -1)
	    if (emergency_save(strcat(TMPDIR,"/SAVE.fig")) == -1)
		fprintf(stderr, "xfig: unable to save figure\n");
    } else
	fprintf(stderr, "xfig: Figure is empty or not modified - exiting without saving.\n");

    goodbye(abortflag);	/* finish up and exit */
}

/* ARGSUSED */
void
my_quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	(void)params;
	(void)num_params;
    if (event && event->type == ClientMessage &&
#ifdef WHEN_SAVE_YOURSELF_IS_FIXED
	((event->xclient.data.l[0] != wm_protocols[0]) &&
	 (event->xclient.data.l[0] != wm_protocols[1])))
#else
	((Atom)event->xclient.data.l[0] != wm_protocols[0]))
#endif /* WHEN_SAVE_YOURSELF_IS_FIXED */
    {
	return;
    }
    /* quit after asking whether user wants to save figure */
    quit(w, 0, 0);
}

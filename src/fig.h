/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
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

#ifndef FIG_H
#define FIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#include <errno.h>
#include <sys/types.h>	/* for stat structure */
#include <sys/stat.h>

#if defined HAVE_DECL_S_IFDIR && !HAVE_DECL_S_IFDIR
#define S_IFDIR _S_IFDIR
#endif
#if defined HAVE_DECL_S_IWRITE && !HAVE_DECL_S_IWRITE
#define S_IWRITE _S_IWRITE
#endif

#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> /* info autoconf: On Darwin, stdio.h is a prerequisite. */
#include <signal.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xfuncs.h>
#ifdef I18N
#include <locale.h>
#endif

#include "xfig_math.h"

#endif /* FIG_H */

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

#ifndef U_SPAWN_H
#define U_SPAWN_H

#if defined HAVE_CONFIG_H && !defined VERSION
#include "config.h"		/* restrict */
#endif

/* spawn the process argv[0], writing to the open file descriptor fdout */
extern int	spawn_writefd(char *const argv[restrict], int fdout);
extern int	spawn_popen_r(char *const argv[restrict]);
extern int	spawn_pclose_r(int fdread);
#endif

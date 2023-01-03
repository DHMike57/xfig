/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2023 by Thomas Loimer
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
#include "u_print.h"

#include <errno.h>
#include <fcntl.h>		/* creat(), open() */
#ifdef I18N
#include <locale.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <unistd.h>
#include <X11/Intrinsic.h>

#include "resources.h"
#include "mode.h"
#include "f_save.h"
#include "f_util.h"
#include "u_colors.h"
#include "u_spawn.h"
#include "u_create.h"		/* new_string */
#include "w_cursor.h"
#include "w_layers.h"
#include "w_msgpanel.h"
#include "w_util.h"

Boolean	print_all_layers = True;
Boolean	bound_active_layers = False;
Boolean	print_hpgl_pcl_switch;
Boolean	hpgl_specified_font;
Boolean	pdf_pagemode;
int	preview_type;

static int	exec_prcmd(char *command, char *msg);
static void	build_layer_list (char *layers);
static void	append_group (char *list, char *num, int first, int last);

/*
 * Protect a string by enclosing it in apostrophes. Escape any apostrophes
 * in the string with a backslash ('\').
 * Beware!  The string returned by this function is static and is
 * reused the next time the function is called!
 */
static char *
shell_protect_string(char *string)
{
	static char *buf = 0;
	static int buflen = 0;
	int len = 2 * strlen(string) + 1;
	char *cp, *cp2;

	if (strlen(string) == 0)
		return string;

	if (! buf) {
		buf = XtMalloc(len);
		buflen = len;
	}
	else if (buflen < len) {
		buf = XtRealloc(buf, len);
		buflen = len;
	}

	cp2 = buf;
	*cp2++ = '\'';
	for (cp = string; *cp; cp++) {
		if (*cp == '\'') {
			/* an apostrophe in the string, close quotes,
			   add \' and open again */
			*cp2++ = '\'';
			*cp2++ = '\\';
			*cp2++ = '\'';
		}
		*cp2++ = *cp;
	}
	*cp2++ = '\'';

	*cp2 = '\0';

	return(buf);
}

/*
 * Write all objects to a temporary fig file.
 * The temporary file is created in TMPDIR, using the template.
 * A random string of length seven is appended to template.
 * The name of the temporary file is returned in the string tmpfile.
 * The length of tmpfile must not exceed len.
 */
static int
write_tmpfigfile(char *tmpfile, size_t len, char *template)
{
	int	fd;

	snprintf(tmpfile, len, "%s/%s.XXXXXX", TMPDIR, template);
	warnexist = False;
	if ((fd = mkstemp(tmpfile)) == -1) {
		file_msg("Cannot open temp file %s: %s\n",
				tmpfile, strerror(errno));
		return -1;
	}
	close(fd);

	init_write_tmpfile();
	if (write_file(tmpfile, False)) {
		end_write_tmpfile();
		return -1;
	}
	end_write_tmpfile();

	return 0;
}

/*
 * Write the export language to args[2].
 */
static void
start_argumentlist(char *arg[restrict], char *argbuf[restrict],
		const size_t bufsize, int *restrict a, int *restrict b,
		char *layers)
{
	/* a refers to the index of arg[], b to the index of argbuf[] */
	*b = -1;
	arg[0] = fig2dev_cmd;
	arg[1] = "-L";
	*a = 2;	/* arg[2] will be the output language */
#ifdef I18N
	if (appres.international)
		arg[++*a] = appres.fig2dev_localize_option;
#endif
	if (appres.magnification < 99.99 | appres.magnification > 100.01) {
		int	n;
		arg[++*a] = "-m";
		n = snprintf(argbuf[++*b], bufsize,
				"%.4g", appres.magnification/100.);
		arg[++*a] = argbuf[*b];
		if ((size_t)n >= bufsize)
			file_msg("Unable to write full magnification %.4g, "
					"only %zd characters available",
					appres.magnification/100., bufsize);
	}
	if (!print_all_layers) {
		arg[++*a] = "-D";
		arg[++*a] = layers;
		if (bound_active_layers)
			arg[++*a] = "-K";
	}
}

static void
border_arg(char *args[restrict], char *const argbuf[restrict],
		const size_t bufsize, int *restrict a, int *restrict b,
		int border)
{
	int	n;
	args[++*a] = "-b";
	n = snprintf(argbuf[++*b], bufsize, "%d", border);
	args[++*a] = argbuf[*b];
	if ((size_t)n >= bufsize)
		file_msg("Border thickness %d incorrectly written: %s",
				border, argbuf[*b]);
}

static int
spawn_prcmd(char *const args[restrict], const char *restrict outfile)
{
	int	fd;
	int	fdout;
	int	stat;

	if ((fdout = open(outfile, O_CREAT | O_WRONLY | O_TRUNC,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
					S_IROTH | S_IWOTH)) == -1) {
		file_msg("Cannot open file %s: %s", outfile, strerror(errno));
		return -1;
	}

	/* All the commands below do their own error reporting */

	if ((fd = spawn_popen_fd(args, "w", fdout)) < 0) {
		close(fdout);
		return -1;
	}
	stat = write_fd(fd);
	fd = spawn_pclose(fd);
	if (stat || fd)
		return -1;
	return 0;
}

/*
 * Construct the initial portion of the conversion command string for
 * language lang and write it to cmd. Use the layer option given in layers.
 * Return the number of chars written to cmd.
 * This function mainly exists, because the #ifdef I18N etc. confused
 * the automatic indenting.
 */
static int
start_exportcmd(char *cmd, char *lang, char *layers)
{
	int	n;

	n = sprintf(cmd, "%s -L %s", fig2dev_cmd, lang);
#ifdef I18N
	if (appres.international)
		n += sprintf(cmd + n, " %s", appres.fig2dev_localize_option);
#endif
	if (appres.magnification < 99.99 | appres.magnification > 100.01)
		n += sprintf(cmd + n, " -m %.4g", appres.magnification/100.);

	/* add the -D list if user doesn't want all layers printed */
	if (!print_all_layers) {
		n += sprintf(cmd + n, " %s", layers);
		/* if doesn't want bounding box of whole figure */
		if (bound_active_layers)
			n += sprintf(cmd + n, " -K");
	}

	return n;
}

void
print_to_printer(char *printer, char *backgrnd, float mag,
		Boolean print_all_layers, Boolean bound_active_layers,
		char *grid, char *params)
{
	(void)mag;
	(void)print_all_layers;
	(void)bound_active_layers;
	char	layers[PATH_MAX];
	char	syspr[2*PATH_MAX+200];
	char	prcmd[2*PATH_MAX+200];
	char	tmpcmd[255];
	char	tmpfile[PATH_MAX];
	char	*name;
	int	n;

	if (write_tmpfigfile(tmpfile, sizeof(tmpfile), "xfig-print"))
		return;

	/* if the user only wants the active layers, build that list */
	build_layer_list(layers);

	if (strlen(cur_filename) == 0)
		name = tmpfile;
	else
		name = shell_protect_string(cur_filename);

#ifdef I18N
	/* set the numeric locale to C so we get decimal points for numbers */
	setlocale(LC_NUMERIC, "C");
#endif
	n = start_exportcmd(tmpcmd, "ps", layers);
	n += sprintf(tmpcmd + n, " -z %s -n %s %s",
			paper_sizes[appres.papersize].sname, name,
			appres.landscape ? "-l xxx" : "-p xxx");

	if (appres.correct_font_size)
		n += sprintf(tmpcmd + n, " -F");

	if (!appres.multiple && !appres.flushleft)
		n += sprintf(tmpcmd + n, " -c");

	if (appres.multiple)
		n += sprintf(tmpcmd + n, " -M");

	if (grid[0] && strcasecmp(grid,"none") != 0)
		n += sprintf(tmpcmd + n, " -G %s", grid);

	if (backgrnd[0])		/* must escape the #rrggbb color spec */
		n += sprintf(tmpcmd + n, " -g \\%s", backgrnd);

	/* make the print command with no filename (it will be in stdin) */
	gen_print_cmd(syspr, "", printer, params);

	/* make up the whole translate/print command */
	sprintf(prcmd, "%s %s | %s", tmpcmd, tmpfile, syspr);
#ifdef I18N
	/* reset to original locale */
	setlocale(LC_NUMERIC, "");
#endif /* I18N */

	if (exec_prcmd(prcmd, "PRINT") == 0) {
		if (emptyname(printer))
			put_msg(
	"Printing on default printer with %s paper size in %s mode ... done",
				paper_sizes[appres.papersize].sname,
				appres.landscape ? "LANDSCAPE" : "PORTRAIT");
		else
			put_msg(
	"Printing on \"%s\" with %s paper size in %s mode ... done",
				printer, paper_sizes[appres.papersize].sname,
				appres.landscape ? "LANDSCAPE" : "PORTRAIT");
	}

	remove(tmpfile);
}

static void
strsub(char *prcmd, char *find, char *repl, char *result, int global)
{
	char *loc;

	do {
		loc = strstr(prcmd, find);
		if (loc == NULL)
			break;

		while ((prcmd != loc) && *prcmd)
			/* copy prcmd into result up to loc */
			*result++ = *prcmd++;
		strcpy(result, repl);
		result += strlen(repl);
		prcmd += strlen(find);
	} while (global);

	strcpy(result, prcmd);
}

/* xoff, yoff, and border are in postscript points (1/72 inch) */
int
print_to_file(char *file, int xoff, int yoff, char *backgrnd, char *transparent,
	      Boolean use_transp_backg, int border, char *grid)
{
	char	layers[PATH_MAX];
	const char	dummy[] = "x";
	char	*outfile, *name;
	char	*tmp_name = NULL;
	char	*suf;
	char	*args[36];
	char	argbuf[5][16];
	int	a;	/* args counter */
	int	b;	/* argbuf counter */
	size_t	bufsize = sizeof argbuf[1];

	/* if file exists, ask if ok */
	if (!ok_to_write(file, "EXPORT"))
		return 1;

	outfile = strdup(file);
	if (strlen(cur_filename) == 0)
		name = file;
	else
		name = cur_filename;

	put_msg("Exporting to file \"%s\" in %s mode ...     ",
			file, appres.landscape ? "LANDSCAPE" : "PORTRAIT");
	app_flush();		/* make sure message gets displayed */

	/*
	 * print_to_file is called from w_export.c where the current directory
	 * is set to cur_export_dir, but write_file() writes picture paths
	 * relative to cur_file_dir; Hence, for the spawned fig2dev command to
	 * find the images, go there.
	 */
	change_directory(cur_file_dir);

	/* if the user only wants the active layers, build that list */
	build_layer_list(layers);

#ifdef I18N
	/* set the numeric locale to C so we get decimal points for numbers */
	setlocale(LC_NUMERIC, "C");
#endif

	start_argumentlist(args, (char **)argbuf, bufsize, &a, &b, layers);

	/* args[2] points to the language */
	args[2] = lang_items[cur_exp_lang];

	/* Options common to PostScript, PDF and EPS output */
	if (cur_exp_lang == LANG_PS || cur_exp_lang == LANG_PDF ||
		    cur_exp_lang == LANG_EPS || cur_exp_lang == LANG_PSTEX ||
		    cur_exp_lang == LANG_PDFTEX || cur_exp_lang == LANG_PSPDF ||
		    cur_exp_lang == LANG_PSPDFTEX ) {

		args[++a] = "-F";
		args[++a] = "-n";
		args[++a] = name;

		if (backgrnd[0]) {
			args[++a] = "-g";
			args[++a] = backgrnd;
		}

		if (border > 0)
			border_arg(args, (char **)argbuf, bufsize,
					&a, &b, border);

		/* any grid spec */
		if (grid[0] && strcasecmp(grid, "none") != 0) {
			args[++a] = "-G";
			args[++a] = grid;
		}

		/* Options common to PS and PDF in PS-mode (not EPS-mode) */
		if (cur_exp_lang == LANG_PS ||
				(cur_exp_lang == LANG_PDF && pdf_pagemode)) {

			if (cur_exp_lang == LANG_PDF /* && pdf_pagemode */)
				args[++a] = "-P";

			args[++a] = "-z";
			args[++a] = paper_sizes[appres.papersize].sname;
			args[++a] = appres.landscape ? "-l" : "-p";
			args[++a] = (char *)dummy;

			if (xoff != 0) {
				int	n;
				args[++a] = "-x";
				n = snprintf(argbuf[++b], bufsize, "%d", xoff);
				args[++a] = argbuf[b];
				if ((size_t)n >= bufsize)
					file_msg("Too large x-shift %d, only "
						 "%zd characters possible",
							xoff, bufsize);
			}
			if (yoff != 0) {
				int	n;
				args[++a] = "-y";
				n = snprintf(argbuf[++b], bufsize, "%d", xoff);
				args[++a] = argbuf[b];
				if ((size_t)n >= bufsize)
					file_msg("Too large y-shift %d, only "
						 "%zd characters possible",
							yoff, bufsize);
			}

			if (appres.multiple) {
				args[++a] = "-M";
				if (appres.overlap)
					args[++a] = "-O";
			} else {
				if (!appres.flushleft)
					args[++a] = "-c";
			}
		}

		if (cur_exp_lang == LANG_PS || cur_exp_lang == LANG_EPS) {
			/* see preview_items in w_export.c */
			switch (preview_type) {
			case 1:				/* ASCII preview */
				args[++a] = "-A";
				break;
			case 2:				/* color tiff preview */
				args[++a] = "-C";
				args[++a] = (char *)dummy;
				break;
			case 3:				/* monochrome tiff */
				args[++a] = "-T";
				break;
			}
		}

		/* Languages which need a second or even third export. */
		if (cur_exp_lang == LANG_PSPDFTEX) {
			size_t		len = strlen(outfile);

			if (!(tmp_name = new_string(len + 4))) {
				free(outfile);
				return 1;
			}

			/* Options were already set above */
			/* first generate pstex postscript then pdftex PDF.  */
			strsub(outfile, ".", "_", tmp_name, 1);

#ifdef I18N
			/* reset to original locale */
			setlocale(LC_NUMERIC, "");
#endif

			/* make it suitable for pstex. */
			strcpy(tmp_name + len, ".eps");
			args[2] = lang_items[LANG_PSTEX];
			args[++a] = NULL;
			spawn_prcmd(args, tmp_name);

			/* make it suitable for pdftex. */
			strcpy(tmp_name + len, ".pdf");
			args[2] = lang_items[LANG_PDFTEX];
			spawn_prcmd(args, tmp_name);

			/* and then the tex code. */
#ifdef I18N
			setlocale(LC_NUMERIC, "C");
#endif
			start_argumentlist(args, (char **)argbuf, bufsize,
					&a, &b, layers);
			args[2] = "pstex_t";
			tmp_name[len] = '\0';
			args[++a] = "-p";
			args[++a] = tmp_name;

		/* PSTEX and PDFTEX */
		} else if (cur_exp_lang == LANG_PSTEX ||
				cur_exp_lang == LANG_PDFTEX) {
			size_t		len = strlen(outfile);

			/* Options were already set above
			    - output the first file */
			args[++a] = NULL;
			spawn_prcmd(args, outfile);

			/* now the text part */
			/* add "_t" to the output filename */
			if (!(tmp_name = new_string(len))) {
				free(outfile);
				return 1;
			}
			memcpy(tmp_name, outfile, len + 1);
			if (!realloc(outfile, len + 3)) {
				free(tmp_name);
				free(outfile);
				return 1;
			}
			strcpy(outfile + len, "_t");
#ifdef I18N
			setlocale(LC_NUMERIC, "C");
#endif
			start_argumentlist(args, (char **)argbuf, bufsize,
					&a, &b, layers);
			args[2] = "pstex_t";
			args[++a] = "-p";
			args[++a] = tmp_name;

			if (border > 0)
				border_arg(args, (char **)argbuf, bufsize,
						&a, &b, border);

		/* PSPDF */
		} else if (cur_exp_lang == LANG_PSPDF) {

			/* Output first file */
			args[2] = lang_items[LANG_EPS];
			args[++a] = NULL;
			spawn_prcmd(args, outfile);

#ifdef I18N
			setlocale(LC_NUMERIC, "C");
#endif
			start_argumentlist(args, (char **)argbuf, bufsize,
					&a, &b, layers);

			/* any grid spec */
			if (grid[0] && strcasecmp(grid, "none") != 0) {
				args[++a] = "-G";
				args[++a] = grid;
			}

			if (border > 0)
				border_arg(args, (char **)argbuf, bufsize,
						&a, &b, border);
			args[++a] = "-F";
			args[++a] = "-n";
			args[++a] = name;

			if (backgrnd[0]) {
				args[++a] = "-g";
				args[++a] = backgrnd;
			}

			/* now change the output file name to xxx.pdf */
			/* strip off current suffix, if any */
			if ((suf = strrchr(outfile, '.'))) {
				size_t	len = strlen(outfile);
				if (len - (suf-outfile) < 4 &&
						!realloc(outfile, len + 5)) {
					free(outfile);
					return 1;
				}
				sprintf(suf, ".pdf");
				args[++a] = outfile;
			} else {
				size_t	len = strlen(outfile);
				if (!(realloc(outfile, len + 5))) {
					free(outfile);
					return 1;
				}
				strcpy(outfile + len, ".pdf");
			}
		}

	} else if (cur_exp_lang == LANG_IBMGL) {

		if (hpgl_specified_font)
			args[++a] = "-F";

		if (print_hpgl_pcl_switch)
			args[++a] = "-k";

		args[++a] = "-z";
		args[++a] = paper_sizes[appres.papersize].sname;

		if (!appres.landscape)
			args[++a] = "-P";

	} else if (cur_exp_lang == LANG_MAP) {

		/* HTML map needs border option */
		if (border > 0)
			border_arg(args, (char **)argbuf, bufsize,
					&a, &b, border);

		args[++a] = outfile;

	} else if (cur_exp_lang == LANG_PCX || cur_exp_lang == LANG_PNG ||
			cur_exp_lang == LANG_TIFF || cur_exp_lang == LANG_XBM ||
#ifdef USE_XPM
			cur_exp_lang == LANG_XPM ||
#endif
			cur_exp_lang == LANG_PPM || cur_exp_lang == LANG_JPEG ||
			cur_exp_lang == LANG_GIF) {

		/* GIF must come before giving background option */
		if (cur_exp_lang == LANG_GIF) {
			/* select the transparent color, if any */
			if (transparent) {
				/* if user wants background transparent, set
				   the background to the transparrent color */
				if (use_transp_backg)
					backgrnd = transparent;
				args[++a] = "-t";
				args[++a] = transparent;
			}
		} else if (cur_exp_lang == LANG_JPEG) {
			/* set the image quality for JPEG export */
			/*
			 * DEF_JPEG_QUALITY is defined in resources.h. Make
			 * sure, it stays in sync with the value for
			 * jpeg_quality given in fig2dev/dev/genbitmaps.c
			 */
			if (appres.jpeg_quality != DEF_JPEG_QUALITY) {
				args[++a] = "-q";
				(void)snprintf(argbuf[++b], bufsize,
						"%d", appres.jpeg_quality);
				args[++a] = argbuf[b];
			}
		}

		/* bitmap formats need border option */
		if (border > 0)
			border_arg(args, (char **)argbuf, bufsize,
					&a, &b, border);

		if (appres.smooth_factor) {
			args[++a] = "-S";
			(void)snprintf(argbuf[++b], bufsize,
					"%d", appres.smooth_factor);
			args[++a] = argbuf[b];
		}

		args[++a] = "-F";

		if (backgrnd[0]) {
			args[++a] = "-g";
			args[++a] = backgrnd;
		}

	/* epic, eepic, eepicemu, latex, pictex */
	} else if (cur_exp_lang == LANG_EPIC || cur_exp_lang == LANG_EEPIC ||
			cur_exp_lang == LANG_EEPICEMU ||
			cur_exp_lang == LANG_LATEX ||
			cur_exp_lang == LANG_PICTEX ||
			cur_exp_lang == LANG_PICT2E ||
			cur_exp_lang == LANG_TIKZ) {

		args[++a] = "-E";
		(void)snprintf(argbuf[++b], bufsize, "%d", appres.encoding);

	/* TK or  PERL/TK */
	} else if (cur_exp_lang == LANG_TK || cur_exp_lang == LANG_PTK) {

		if (backgrnd[0]) {
			args[++a] = "-g";
			args[++a] = backgrnd;
		}

	} else if (cur_exp_lang == LANG_DXF) {

		if (!appres.landscape)
			args[++a] = "-P";

	}
	/* Nothing to do for everything else */

	/* make a busy cursor */
	set_temp_cursor(wait_cursor);

#ifdef I18N
	/* reset to original locale */
	setlocale(LC_NUMERIC, "");
#endif

	/* now execute fig2dev */
	args[++a] = NULL;
	if (!spawn_prcmd(args, outfile))
		put_msg("Export to \"%s\" done", file);

	/* and reset the cursor */
	reset_cursor();

	/* free tempnames */
	if (tmp_name)
		free(tmp_name);
	free(outfile);

	return 0;
}

void
gen_print_cmd(char *cmd, char *file, char *printer, char *pr_params)
{
    if (emptyname(printer)) {	/* send to default printer */
#if (defined(SYSV) || defined(SVR4)) && !defined(BSDLPR)
	sprintf(cmd, "lp %s %s",
		pr_params,
		shell_protect_string(file));
#else
	sprintf(cmd, "%s %s %s",
		access("/usr/bin/lp", X_OK)?"lpr":"lp",
		pr_params,
		shell_protect_string(file));
#endif /* (defined(SYSV) || defined(SVR4)) && !defined(BSDLPR) */
	put_msg("Printing on default printer with %s paper size in %s mode ...     ",
		paper_sizes[appres.papersize].sname,
		appres.landscape ? "LANDSCAPE" : "PORTRAIT");
    } else {
#if (defined(SYSV) || defined(SVR4)) && !defined(BSDLPR)
	sprintf(cmd, "lp %s -d%s %s",
		pr_params,
		shell_protect_string(printer),
		shell_protect_string(file));
#else
	sprintf(cmd, "%s %s %s%s %s",
		access("/usr/bin/lp", X_OK)?"lpr":"lp",
		pr_params,
		access("/usr/bin/lp", X_OK)?"-P":"-d",
		shell_protect_string(printer),
		shell_protect_string(file));
#endif /* (defined(SYSV) || defined(SVR4)) && !defined(BSDLPR) */
	put_msg("Printing on \"%s\" with %s paper size in %s mode ...     ",
		shell_protect_string(printer),
		paper_sizes[appres.papersize].sname,
		appres.landscape ? "LANDSCAPE" : "PORTRAIT");
    }
    app_flush();		/* make sure message gets displayed */
}

int
exec_prcmd(char *command, char *msg)
{
	char	errfname[PATH_MAX];
	FILE	*errfile;
	char	str[400];
	int	status, fd;

	/* make temp filename for any errors */
	snprintf(errfname, sizeof(errfname), "%s/xfig-export.XXXXXX", TMPDIR);
	if ((fd = mkstemp(errfname)) == -1) {
		file_msg("Can't open temp file %s: %s\n",
				errfname, strerror(errno));
		return 1;
	}
	close(fd);

	/* direct any output from fig2dev to this file */
	strcat(command, " 2> ");
	strcat(command, errfname);
	if (appres.DEBUG)
		fprintf(stderr,"Execing: %s\n",command);
	status=system(command);
	if (status != 0) {
		/* check if error file has anything in it */
		if ((errfile = fopen(errfname, "r")) == NULL) {
			file_msg("Error during %s. No messages available.",msg);
		} else {
			if (fgets(str,sizeof(str)-1,errfile) != NULL) {
				rewind(errfile);
				file_msg("Error during %s.  Messages:",msg);
				while (fgets(str,sizeof(str)-1,errfile)!=NULL) {
					/* remove trailing newlines */
					str[strlen(str)-1] = '\0';
					file_msg(" %s",str);
				}
			}
		}
		fclose(errfile);
	}
	remove(errfname);
	return status;
}

/*
   make an rgb string from color (e.g. #31ab12)
   if the color is < 0, make empty string
*/
void
make_rgb_string(int color, char *rgb_string)
{
	if (color >= 0) {
		sprintf(rgb_string, "#%02x%02x%02x",
				getred(color)>>8,
				getgreen(color)>>8,
				getblue(color)>>8);
	} else {
		rgb_string[0] = '\0';	/* no background wanted by user */
	}
}

/* make up the -D option to fig2dev if user wants to print only active layers */

static void
build_layer_list(char *layers)
{
	char	list[PATH_MAX], notlist[PATH_MAX], num[10];
	int	layer, len, notlen;
	int	firstyes, lastyes = 0, firstno, lastno = 0;

	layers[0] = '\0';

	if (print_all_layers)
		return;

	list[0] = notlist[0] = '\0';
	len = notlen = 0;

	/* build up two lists - layers TO print and layers to NOT print */
	/* use the smaller of the two in the final command */

	firstyes = firstno = -1;
	for (layer=min_depth; layer<=max_depth; layer++) {
		if (active_layers[layer] && object_depths[layer]) {
			if (firstyes == -1)
				firstyes = lastyes = layer;
			/* see if there is a contiguous set */
			if (layer-lastyes <= 1) {
				lastyes = layer;	/* so far, yes */
				continue;
			}
			append_group(list, num, firstyes, lastyes);
			firstyes = lastyes = layer;
			if (len+strlen(list) >= PATH_MAX-5)
				continue;   /* list is too long, don't append */
			strcat(list,num);
			len += strlen(list)+1;
		} else if (object_depths[layer]) {
			if (firstno == -1)
				firstno = lastno = layer;
			/* see if there is a contiguous set */
			if (layer-lastno <= 1) {
				lastno = layer;		/* so far, yes */
				continue;
			}
			if (firstno == -1)
				firstno = layer;
			append_group(notlist, num, firstno, lastno);
			firstno = lastno = layer;
			if (notlen+strlen(notlist) >= PATH_MAX-5)
				continue;   /* list is too long, don't append */
			strcat(notlist,num);
			notlen += strlen(notlist)+1;
		}
	}
	if (firstyes != -1) {
		append_group(list, num, firstyes, lastyes);
		if (len+strlen(list) < PATH_MAX-5) {
			strcat(list,num);
			len += strlen(list)+1;
		}
	}
	if (firstno != -1) {
		append_group(notlist, num, firstno, lastno);
		if (notlen+strlen(notlist) < PATH_MAX-5) {
			strcat(notlist,num);
			notlen += strlen(notlist)+1;
		}
	}
	if (len < notlen && firstyes != -1) {
		/* use list of layers TO print */
		sprintf(layers," -D +%s ",list);
	} else if (firstno != -1){
		/* use list of layers to NOT print */
		sprintf(layers," -D -%s ",notlist);
	}
}

void
append_group(char *list, char *num, int first, int last)
{
	if (list[0])
		strcat(list,",");
	if (first==last)
		sprintf(num,"%0d",first);
	else
		sprintf(num,"%0d:%d",first,last);
}

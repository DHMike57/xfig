## Run automake on this file to produce Makefile.in

# FIG : Facility for Interactive Generation of figures
# Copyright (c) 1985-1988 by Supoj Sutanthavibul
# Parts Copyright (c) 1989-2015 by Brian V. Smith
# Parts Copyright (c) 1991 by Paul King
# Parts Copyright (c) 2016-2021 by Thomas Loimer
#
# Any party obtaining a copy of these files is granted, free of charge, a
# full and unrestricted irrevocable, world-wide, paid up, royalty-free,
# nonexclusive right and license to deal in this software and documentation
# files (the "Software"), including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense and/or sell copies
# of the Software, and to permit persons who receive copies from any such
# party to do so, with the only requirement being that the above copyright
# and this permission notice remain intact.
#
#
# doc/Makefile.am
# Author: Thomas Loimer, 2016, 2021

dist_doc_DATA = FORMAT1.3 FORMAT1.4 FORMAT2.0 FORMAT2.1 FORMAT3.0 \
	FORMAT3.1 FORMAT3.2 xfig-howto.pdf xfig_ref_en.pdf

doc_DATA = xfig_man.html

man1_MANS = xfig.man

EXTRA_DIST  = html MAKEPS

CLEANFILES = html/images/xfig-title.fig

MAINTAINERCLEANFILES = Makefile.in

# Below, $< is not fully portable, see info autoconf "Portable Make",
# use $? instead. man is sensitive to spaces after the options.
# Probably, groff -Thtml -mman xfig.man > $@ is more portable?

# Create the logo, xfig-title.png, which is included at the top of each html
# page and which shows the version string, at distribution time.
# Also create xfig_man.html.in at distribution time.
# Users should be able to build xfig without fig2dev or man installed.
dist-hook: $(srcdir)/html/images/xfig-title.fig.in $(top_srcdir)/version.m4 \
		$(srcdir)/xfig.man.in
	chmod u+w "$(distdir)" "$(distdir)/html" "$(distdir)/html/images"
	-chmod u+w "$(distdir)/xfig_man.html.in"	\
		"$(distdir)/html/images/xfig-title.png"
	test -r html/images/xfig-title.fig || \
		{ cd .. && $(MAKE) doc/html/images/xfig-title.fig; }
	fig2dev -L png -S4 html/images/xfig-title.fig	\
			>"$(distdir)/html/images/xfig-title.png"
	rm -f $(distdir)/$(CLEANFILES)
	man -Thtml -l "$(srcdir)/xfig.man.in" >"$(distdir)/xfig_man.html.in"

# To build in the development tree, xfig_man.html.in is required, and already at
# configuration time. Hence, a make rule comes too late. Anyway, a version of
# xfig_man.html.in still lingers around. In addition, provide a rule for the
# file. Unusually, this is a rule for a file in the source tree.
$(srcdir)/xfig_man.html.in: $(srcdir)/xfig.man.in
	man -Thtml -l "$(srcdir)/xfig.man.in" >"$(srcdir)/xfig_man.html.in"

install-data-hook:
	cp -Rp "$(srcdir)/html" "$(DESTDIR)$(htmldir)" \
	&& chmod -R u+rwX "$(DESTDIR)$(htmldir)"
	cd "$(DESTDIR)$(htmldir)/html" \
	&& rm -f ja_JP.ujis && $(LN_S) japanese ja_JP.ujis
	cd "$(DESTDIR)$(htmldir)/html/images" && \
		rm -f xfig-title.fig xfig-title.fig.in

uninstall-local:
	rm -rf "$(DESTDIR)$(htmldir)/html"

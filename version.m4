# Version information
# This file is included by configure.ac.

m4_define([XFIG_VERSION], [3.2.6])

dnl m4_define must be called before AC_INIT - but, I believe,
dnl shell-variables can only be defined after a call to AC_INIT.
dnl Therefore, define RELEASEDATE as a macro.
m4_define([RELEASEDATE], [Aug 2016])

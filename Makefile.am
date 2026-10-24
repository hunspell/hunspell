ACLOCAL_AMFLAGS = -I m4

if BUILD_TOOLS
SUBDIRS= po src man tests
else
SUBDIRS= src man
endif

pkgconfdir = $(libdir)/pkgconfig
pkgconf_DATA = hunspell.pc

EXTRA_DIST = license.myspell license.hunspell \
        ChangeLog.O COPYING.LESSER COPYING.MPL hunspell.pc.in CMakeLists.txt

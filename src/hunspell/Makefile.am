lib_LTLIBRARIES = libhunspell-1.7.la
libhunspell_1_7_includedir = $(includedir)/hunspell
libhunspell_1_7_la_SOURCES=affentry.cxx affentry.hxx affixmgr.cxx affixmgr.hxx \
                           atypes.hxx baseaffix.hxx csutil.cxx csutil.hxx \
                           filemgr.cxx filemgr.hxx hashmgr.cxx hashmgr.hxx \
                           htypes.hxx hunspell.cxx hunspell.h hunspell.hxx \
                           hunzip.cxx hunzip.hxx langnum.hxx \
                           phonet.cxx phonet.hxx replist.cxx replist.hxx \
                           suggestmgr.cxx suggestmgr.hxx utf_info.hxx w_char.hxx
nodist_libhunspell_1_7_la_SOURCES=hunvisapi.h


libhunspell_1_7_include_HEADERS=hunspell.hxx hunspell.h hunvisapi.h \
		                w_char.hxx atypes.hxx

# libtool version-info: current:revision:age
#  - source changed, no API change:        r += 1
#  - public API added:                     c += 1; r = 0; a += 1
#  - public API removed or signature change: c += 1; r = 0; a = 0
# Linux SONAME suffix is (current - age). See RELEASING.md.
libhunspell_1_7_la_LDFLAGS=-no-undefined -version-info 1:0:1

AM_CXXFLAGS=$(CFLAG_VISIBILITY) -DBUILDING_LIBHUNSPELL $(HUNSPELL_STATIC_DEFINE)

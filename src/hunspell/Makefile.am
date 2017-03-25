lib_LTLIBRARIES = libhunspell-1.6.la
libhunspell_1_6_includedir = $(includedir)/hunspell
libhunspell_1_6_la_SOURCES=affentry.cxx affixmgr.cxx csutil.cxx \
		     hashmgr.cxx hunspell.cxx suggestmgr.cxx \
	             phonet.cxx filemgr.cxx hunzip.cxx replist.cxx \
	             affentry.hxx htypes.hxx affixmgr.hxx \
	             csutil.hxx atypes.hxx suggestmgr.hxx \
	             baseaffix.hxx hashmgr.hxx langnum.hxx \
	             phonet.hxx filemgr.hxx hunzip.hxx replist.hxx

libhunspell_1_6_include_HEADERS=hunspell.hxx hunspell.h hunvisapi.h \
		                w_char.hxx atypes.hxx csutil.hxx htypes.hxx

libhunspell_1_6_la_DEPENDENCIES=utf_info.cxx
libhunspell_1_6_la_LDFLAGS=-no-undefined -version-info 0:1:0

AM_CXXFLAGS=$(CFLAG_VISIBILITY) -DBUILDING_LIBHUNSPELL

EXTRA_DIST=utf_info.cxx

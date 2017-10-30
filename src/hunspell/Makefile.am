lib_LTLIBRARIES = libhunspell-1.6.la
libhunspell_1_6_includedir = $(includedir)/hunspell
libhunspell_1_6_la_SOURCES=affentry.cxx affentry.hxx affixmgr.cxx affixmgr.hxx \
                           atypes.hxx baseaffix.hxx csutil.cxx csutil.hxx \
                           filemgr.cxx filemgr.hxx hashmgr.cxx hashmgr.hxx \
                           htypes.hxx hunspell.cxx hunspell.h hunspell.hxx \
                           hunvisapi.h hunzip.cxx hunzip.hxx langnum.hxx \
                           phonet.cxx phonet.hxx replist.cxx replist.hxx \
                           suggestmgr.cxx suggestmgr.hxx utf_info.hxx w_char.hxx


libhunspell_1_6_include_HEADERS=hunspell.hxx hunspell.h hunvisapi.h \
		                w_char.hxx atypes.hxx

libhunspell_1_6_la_LDFLAGS=-no-undefined -version-info 0:1:0

AM_CXXFLAGS=$(CFLAG_VISIBILITY) -DBUILDING_LIBHUNSPELL

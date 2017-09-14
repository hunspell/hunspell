# this will be noinst until ready for release
noinst_LIBRARIES = libhunspell2.a
noinst_PROGRAMS = hun2

AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14 $(CODE_COVERAGE_CXXFLAGS)
LDADD = $(CODE_COVERAGE_LIBS)
LIBADD = $(CODE_COVERAGE_LIBS)

libhunspell2_a_SOURCES=aff_manager.cxx aff_manager.hxx dic_manager.cxx \
dic_manager.hxx dict_finder.cxx dict_finder.hxx hunspell.hxx \
locale_utils.hxx locale_utils.cxx stream_utils.hxx  \
string_utils.hxx
# libhunspell2_a_LIBADD = $(LIBADD)

hun2_SOURCES = main.cxx
hun2_LDADD = $(LDADD) libhunspell2.a

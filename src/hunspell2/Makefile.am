# this will be noinst until ready for release
noinst_LIBRARIES = libhunspell2.a
noinst_PROGRAMS = hun2

AM_CPPFLAGS = $(BOOST_CPPFLAGS)   $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14          $(CODE_COVERAGE_CXXFLAGS)
AM_LDFLAGS  = $(BOOST_LOCALE_LDFLAGS)
LDADD       = $(BOOST_LOCALE_LIBS) $(CODE_COVERAGE_LIBS)
LIBADD      = $(BOOST_LOCALE_LIBS) $(CODE_COVERAGE_LIBS)

libhunspell2_a_SOURCES=\
aff_data.cxx     aff_data.hxx     \
dic_data.cxx     dic_data.hxx     \
                 dictionary.hxx   \
finder.cxx       finder.hxx       \
locale_utils.cxx locale_utils.hxx \
                 stream_utils.hxx \
                 string_utils.hxx

hun2_SOURCES = main.cxx
hun2_LDADD = libhunspell2.a $(LDADD)

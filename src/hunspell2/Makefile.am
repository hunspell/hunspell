# this will be noinst until ready for release
# then should be bin
noinst_PROGRAMS=hun2

@CODE_COVERAGE_RULES@
LDADD = $(CODE_COVERAGE_LIBS)
AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14 $(CODE_COVERAGE_CXXFLAGS)

# CXXFLAGS is different than AM_CXXFLAGS. default values is -g -O2
if CODE_COVERAGE_ENABLED
	CXXFLAGS =
endif


hun2_SOURCES=aff_manager.cxx aff_manager.hxx dic_manager.cxx \
dic_manager.hxx dict_finder.cxx dict_finder.hxx hunspell.hxx \
locale_utils.hxx locale_utils.cxx main.cxx stream_utils.hxx  \
string_utils.hxx

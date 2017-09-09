override PACKAGE_STRING = Hunspell 2.0.0

XFAIL_TESTS = @XFAILED@

check_PROGRAMS=string_utils_test

AM_CXXFLAGS = -std=c++14
AM_CPPFLAGS = -I$(top_srcdir)/src/hunspell2
string_utils_test_SOURCES=string_utils_test.cxx

TESTS =	\
$(check_PROGRAMS)

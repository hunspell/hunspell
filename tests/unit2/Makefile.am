override PACKAGE_STRING = Hunspell 2.0.0

XFAIL_TESTS = @XFAILED@

AM_CXXFLAGS = -std=c++14 -I$(top_srcdir)/src/hunspell2
LDADD=-lcppunit -ldl

check_PROGRAMS=locale_utils_test string_utils_test
# Next line is needed to build executable from Qt Creator.
noinst_PROGRAMS=locale_utils_test string_utils_test

locale_utils_test_SOURCES=locale_utils_test.cxx \
$(top_srcdir)/src/hunspell2/locale_utils.hxx \
$(top_srcdir)/src/hunspell2/locale_utils.cxx

string_utils_test_SOURCES=string_utils_test.cxx \
$(top_srcdir)/src/hunspell2/string_utils.hxx


TESTS =	\
$(check_PROGRAMS)

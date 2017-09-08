## Process this file with automake to create Makefile.in

noinst_PROGRAMS=string_utils_test
AM_CXXFLAGS = -std=c++14
AM_CPPFLAGS = -I$(top_srcdir)/src/hunspell2
string_utils_test_SOURCES=string_utils_test.cxx string_utils.hxx



XFAIL_TESTS = @XFAILED@

TESTS =	\
simple.test \
simplenonascii.test

distclean-local:
	-rm -rf testSubDir
        
EXTRA_DIST = \
test.sh \
simple.aff \
simple.dic \
simple.good \
simple.wrong \
simple.test \
simplenonascii.aff \
simplenonascii.dic \
simplenonascii.good \
simplenonascii.wrong \
simplenonascii.test

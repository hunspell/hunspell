## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

check_PROGRAMS=string_utils_test
AM_CXXFLAGS = -std=c++14
AM_CPPFLAGS = -I$(top_srcdir)/src/hunspell2
string_utils_test_SOURCES=string_utils_test.cxx



XFAIL_TESTS = @XFAILED@

TESTS =	\
simple.test \
simplenonascii.test \
string_utils_test

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

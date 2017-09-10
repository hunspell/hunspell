## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

check_PROGRAMS=string_utils_test
AM_CXXFLAGS = -std=c++14
AM_CPPFLAGS = -I$(top_srcdir)/src/hunspell2
string_utils_test_SOURCES=string_utils_test.cxx

XFAIL_TESTS = @XFAILED@

TEST_EXTENSIONS = .dic
AM_TESTS_ENVIRONMENT = HUNSPELL=../../src/hunspell2/hun2; export HUNSPELL;
DIC_LOG_COMPILER = ../test.sh

TESTS =	\
simple.dic \
simplenonascii.dic \
$(check_PROGRAMS)

clean-local:
	-rm -rf testSubDir
        
EXTRA_DIST = \
test.sh \
simple.aff \
simple.dic \
simple.good \
simple.wrong \
simplenonascii.aff \
simplenonascii.dic \
simplenonascii.good \
simplenonascii.wrong 

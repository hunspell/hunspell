## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

XFAIL_TESTS = @XFAILED@

TEST_EXTENSIONS = .dic
AM_TESTS_ENVIRONMENT = HUNSPELL=../../src/hunspell2/hun2; export HUNSPELL;
DIC_LOG_COMPILER = ../test.sh

TESTS =	\
simple.dic \
simplenonascii.dic

clean-local:
	-rm -rf testSubDir
        
EXTRA_DIST = \
simple.aff \
simple.dic \
simple.good \
simple.wrong \
simplenonascii.aff \
simplenonascii.dic \
simplenonascii.good \
simplenonascii.wrong

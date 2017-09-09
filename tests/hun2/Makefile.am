## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

XFAIL_TESTS = @XFAILED@

TEST_EXTENSIONS = .dic
DIC_LOG_COMPILER = ./test.sh

TESTS =	\
simple.dic \
simpleline.dic \
simplenonascii.dic

distclean-local:
	-rm -rf testSubDir
        
EXTRA_DIST = \
test.sh \
simple.aff \
simple.dic \
simple.good \
simple.wrong \
simpleline.aff \
simpleline.dic \
simpleline.good \
simpleline.wrong \
simplenonascii.aff \
simplenonascii.dic \
simplenonascii.good \
simplenonascii.wrong

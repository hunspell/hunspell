## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

XFAIL_TESTS = @XFAILED@

TESTS =	\
simple.test \
simplenonascii.test \
$(check_PROGRAMS)

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

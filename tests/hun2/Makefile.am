## Process this file with automake to create Makefile.in

override PACKAGE_STRING = Hunspell 2.0.0

TEST_EXTENSIONS = .dic
AM_TESTS_ENVIRONMENT = export HUNSPELL=$(top_builddir)/src/hunspell2/hun2; \
                       export LIBTOOL=$(top_builddir)/libtool;
DIC_LOG_COMPILER = $(SHELL) $(top_srcdir)/tests/test.sh

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

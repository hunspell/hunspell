# Please, leave the comments in this file as it is a lot of work to fine tune.

override PACKAGE_STRING = Hunspell 2.0.0 unit tests

# The -I argument is needed to have initial top-level make working.
# It prevents errors such as:
#     xyz.o: In function `CppUnit::AutoRegisterSuite<LocaleUtilsTest>::~AutoRegisterSuite()':
#     /usr/include/cppunit/extensions/AutoRegisterSuite.h:50: undefined reference to `CppUnit::TestFactoryRegistry::isValid()'"
AM_CXXFLAGS = -std=c++14 -I../../src/hunspell2 $(CPPUNIT_CFLAGS)

LDADD = $(CPPUNIT_LIBS)

check_PROGRAMS = \
locale_utils_test \
string_utils_test

# Next setting is needed to build executables from Qt Creator.
noinst_PROGRAMS = $(check_PROGRAMS)

# Rebuilding locale_utils_test can be tested with:
#     touch ../../src/hunspell2/locale_utils.hxx && make
#     touch ../../src/hunspell2/locale_utils.cxx && make
locale_utils_test_SOURCES = locale_utils_test.cxx
# Next setting has $(CPPUNIT_LIBS) in order to link without errors.
# program specific LDADD overrides global
locale_utils_test_LDADD = ../../src/hunspell2/locale_utils.o $(CPPUNIT_LIBS)

# Rebuilding string_utils_test can be tested with:
#     touch ../../src/hunspell2/string_utils.hxx && make
string_utils_test_SOURCES = string_utils_test.cxx

# NOTE: older rules for locale_utils_test are below. Because of
# AUTOMAKE_OPTIONS = subdir-objects # they do work in the same way and even
# result in the object files and executables of the same size, no object
# files for source files from other directories are made and rebuiling works too.
##locale_utils_test_SOURCES = \
##../../src/hunspell2/locale_utils.cxx \
##locale_utils_test.cxx
# Any header files involved, with or without code in them, do not need to be listed
# above as long as their related source files are listed.

TESTS =	$(check_PROGRAMS)






# Code coverage reporting is defined below. The command gcov is part of the gcc package.  The command
# gcovr is part of the gcovr package and the commands lcov and genhtml are part of package lcov. They
# can be installed with:
#     $ sudo apt-get install lcov gcovr

# THIS IS CURRENTLY WORK IN PROGRES!!

# See also:
# https://gcc.gnu.org/onlinedocs/gcc/Invoking-Gcov.html
# https://www.gnu.org/software/autoconf-archive/ax_code_coverage.html
# https://github.com/eiichiroi/autotools-unittest/blob/master/Makefile.am.coverage
# https://github.com/ebassi/glib-controller/blob/master/build/Makefile.am.gcov
# https://gitlab.gnome.org/GNOME/json-glib/blob/449656c68a3d6e0d8b96c36414b475e78b9cc272/build/autotools/Makefile.am.gcov

#FIXME Fix condition below, if we really need this test.
#if ENABLE_COV

#FIXME The following is needed in order to report inside the code that is being tested.
# Better to clear the next setting completely. (Didn't know how, so simple set to redundent value.)
locale_utils_test_LDADD = $(CPPUNIT_LIBS)
locale_utils_test_SOURCES = \
../../src/hunspell2/locale_utils.cxx \
locale_utils_test.cxx
# Any header files involved, with or without code in them, do not need to be listed
# above as long as their related source files are listed. This was tested specifically and
# independently for code coverage reporting too.

# The option --coverage is synonym for -fprofile-arcs -ftest-coverage (when compiling) and -lgcov
# (when linking). The option -O0 disables optimiation set at a higher level, here -O2 is disabled.
AM_CXXFLAGS += --coverage -O0
AM_LDFLAGS = --coverage -O0

#FIXME Run automatically code coverage testing tool gcov. The make check generates the *.gcda fieles
# but not the *.gcno files. The command below, generate both.
# Run manually
#     $ gcov locale_utils_test.o; gcov string_utils_test.o
# or run manually (could in Makefile.am use the $check_PROGRAMS variable)
#     $ gcov locale_utils_test; gcov string_utils_test
# Run manually make cov-report-html afterwards and open in a browser
#     $ firefox coverage/index.html coverage2/index.html &

.PHONY: cov-reset
cov-reset:
	@rm -fr coverage coverage2
	@find . -name "*.gcda" -exec rm {} \;
	@find . -name "*.gcno" -exec rm {} \;
	@lcov --directory . --zerocounters

.PHONY: cov-report
cov-report:
# perhaps ../../ or only .
	@gcovr -r ../..

.PHONY: cov-report-html
cov-report-html: coverage
	@mkdir -p coverage coverage2
	@lcov --compat-libtool --directory . --capture --output-file coverage/app.info
	@genhtml -o coverage/ coverage/app.info 
# Some examples to filter
#	@lcov --extract coverage/app.info '*.cxx' --output-file coverage/app2.info
#	@genhtml -o coverage/ coverage/app2.info 
#	@lcov --remove coverage/app2.info cmdline.c --output-file coverage/app3.info
#	@genhtml -o coverage/ coverage/app3.info 
# For test purposes, here outout to different directory, ideally as just above in same directory.
	@lcov --extract coverage/app.info '*hunspell*' --output-file coverage2/app2.info
	@genhtml -o coverage2/ coverage2/app2.info 

cov:
	@make cov-report

clean-local:
	@make cov-reset 

.PHONY: coverage
coverage:
	@make check
	@make cov-report

#FIXME Below is disabled to exclude from building on CI.
#check:
#	@make cov

#FIXME Uncomment or remove when #if ENABLE_COV has been fixed.
#endif

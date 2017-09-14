# Please, leave the comments in this file as it is a lot of work to fine tune.

override PACKAGE_STRING = Hunspell 2.0.0 unit tests


@CODE_COVERAGE_RULES@
AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14 -I../../src/hunspell2 $(CPPUNIT_CFLAGS) $(CODE_COVERAGE_CXXFLAGS)
LDADD = ../../src/hunspell2/libhunspell2.a $(CPPUNIT_LIBS) $(CODE_COVERAGE_LIBS)

check_PROGRAMS = \
locale_utils_test \
string_utils_test

# Next setting is needed to build executables from Qt Creator.
noinst_PROGRAMS = $(check_PROGRAMS)

# Rebuilding locale_utils_test can be tested with:
#     touch ../../src/hunspell2/locale_utils.hxx && make
#     touch ../../src/hunspell2/locale_utils.cxx && make
locale_utils_test_SOURCES = locale_utils_test.cxx

# Rebuilding string_utils_test can be tested with:
#     touch ../../src/hunspell2/string_utils.hxx && make
string_utils_test_SOURCES = string_utils_test.cxx

TESTS =	$(check_PROGRAMS)

# to run coverage
# ./configure --enable-code-coverage
# cd tests/unit2 && make check-code-coverage

# AUTOMAKE_OPTIONS = subdir-objects # use this if you add a .cxx file from another directory
# that creates and object in that other directory
# we shoud not use this, let it stay for now

# Coverage should be enabled per whole project, not per unit tests.
# Now the same object files from ../../src/hunspell2 have to be build twice with different CXXFLAGS
# can create trouble

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

.PHONY: cov-reset
cov-reset:
	-rm -fr coverage coverage2
	-find . -name "*.gcda" -exec rm {} \;
	-find . -name "*.gcno" -exec rm {} \;
	-lcov --directory . --zerocounters

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

cov: cov-report

clean-local: cov-reset

.PHONY: coverage
coverage:
	@make check
	@make cov-report

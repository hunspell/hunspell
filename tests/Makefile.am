SUBDIRS = suggestiontest hun2 v1cmdline

override PACKAGE_STRING = Hunspell 2.0.0 unit tests


@CODE_COVERAGE_RULES@
AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14 -I../src/hunspell2 $(CPPUNIT_CFLAGS) $(CODE_COVERAGE_CXXFLAGS)
LDADD = ../src/hunspell2/libhunspell2.a $(CPPUNIT_LIBS) $(CODE_COVERAGE_LIBS)

check_PROGRAMS = \
locale_utils_test \
string_utils_test

# Next setting is needed to build executables from Qt Creator.
noinst_PROGRAMS = $(check_PROGRAMS)

locale_utils_test_SOURCES = locale_utils_test.cxx

string_utils_test_SOURCES = string_utils_test.cxx

TESTS =	$(check_PROGRAMS)

# to run coverage
# ./configure --enable-code-coverage
# cd tests && make check-code-coverage

# AUTOMAKE_OPTIONS = subdir-objects # use this if you add a .cxx file from another directory
# that creates and object in that other directory
# we shoud not use this, let it stay for now

# Coverage should be enabled per whole project, not per unit tests.
# Now the same object files from ../src/hunspell2 have to be build twice with different CXXFLAGS
# can create trouble

EXTRA_DIST = test.sh

SUBDIRS = suggestiontest hun2 v1cmdline

override PACKAGE_STRING = Hunspell 2.0.0 unit tests

AM_CPPFLAGS = -I../src/hunspell2 $(BOOST_CPPFLAGS) $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = -std=c++14 $(CODE_COVERAGE_CXXFLAGS)
AM_LDFLAGS  = $(BOOST_LOCALE_LDFLAGS)
LDADD = ../src/hunspell2/libhunspell2.a $(BOOST_LOCALE_LIBS) $(CODE_COVERAGE_LIBS)


BUILT_SOURCES = catch.hpp catch_reporter_tap.hpp
catch_url = https://raw.githubusercontent.com/philsquared/Catch/v1.10.0
catch.hpp:
	wget $(catch_url)/single_include/catch.hpp

catch_reporter_tap.hpp:
	wget $(catch_url)/include/reporters/catch_reporter_tap.hpp

CLEANFILES = catch.hpp catch_reporter_tap.hpp

TEST_EXTENSIONS = .catch
CATCH_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/tap-driver.sh
CATCH_LOG_COMPILER = $(SHELL) $(srcdir)/catch-runner.sh

check_PROGRAMS = \
ch.catch

# Next setting is needed to build executables from Qt Creator.
noinst_PROGRAMS = $(check_PROGRAMS)

TESTS =	$(check_PROGRAMS)

ch_catch_SOURCES = \
locale_utils_test.cxx \
string_utils_test.cxx \
aff_data_test.cxx \
finder_test.cxx \
catch_main.cxx

nodist_ch_catch_SOURCES = $(BUILT_SOURCES)


# to run coverage, cd to root
# ./configure --enable-code-coverage
# make check-code-coverage

EXTRA_DIST = test.sh catch-runner.sh

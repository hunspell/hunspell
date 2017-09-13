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
# result in the object files and executables of the same size and no
# object files for source files from other directories. and proper rebuiling.
##locale_utils_test_SOURCES = \
##../../src/hunspell2/locale_utils.cxx \
##locale_utils_test.cxx

TESTS =	$(check_PROGRAMS)

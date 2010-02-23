bin_PROGRAMS=analyze chmorph example hunspell munch unmunch hzip hunzip

INCLUDES=-I${top_srcdir}/src/hunspell -I${top_srcdir}/src/parsers

hzip_SOURCES=hzip.c
hunzip_SOURCES=hunzip.cxx
hunzip_LDADD = ../hunspell/libhunspell-1.2.la

munch_SOURCES=munch.c
unmunch_SOURCES=unmunch.c
include_HEADERS=munch.h unmunch.h

example_SOURCES=example.cxx
example_LDADD = ../hunspell/libhunspell-1.2.la

hunspell_SOURCES=hunspell.cxx
hunspell_LDADD = @LIBINTL@ ../hunspell/libhunspell-1.2.la \
	../parsers/libparsers.a @CURSESLIB@ @READLINELIB@

analyze_SOURCES=analyze.cxx
analyze_LDADD = ../hunspell/libhunspell-1.2.la

chmorph_SOURCES=chmorph.cxx
chmorph_LDADD = ../hunspell/libhunspell-1.2.la ../parsers/libparsers.a

noinst_PROGRAMS=example

EXTRA_DIST=makealias affixcompress wordforms

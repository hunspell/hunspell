bin_PROGRAMS=hunspell-analyze hunspell-chmorph hunspell hunspell-munch hunspell-unmunch hzip hunzip

AM_CPPFLAGS=-I${top_builddir}/src/hunspell -I${top_srcdir}/src/hunspell -I${top_srcdir}/src/parsers  -DDATADIR=\"$(datadir)\"

hzip_SOURCES=hzip.cxx
hunzip_SOURCES=hunzip.cxx
hunzip_LDADD = ../hunspell/libhunspell-1.7.la

hunspell_munch_SOURCES=munch.cxx munch.h
hunspell_unmunch_SOURCES=unmunch.cxx unmunch.h

example_SOURCES=example.cxx
example_LDADD = ../hunspell/libhunspell-1.7.la

hunspell_SOURCES=hunspell.cxx
nodist_hunspell_SOURCES= ../../config.h
hunspell_LDADD = @LIBINTL@ @LIBICONV@ ../parsers/libparsers.a \
	../hunspell/libhunspell-1.7.la @CURSESLIB@ @READLINELIB@

hunspell_analyze_SOURCES=analyze.cxx
hunspell_analyze_LDADD = ../hunspell/libhunspell-1.7.la

hunspell_chmorph_SOURCES=chmorph.cxx
hunspell_chmorph_LDADD = ../parsers/libparsers.a ../hunspell/libhunspell-1.7.la

noinst_PROGRAMS=example 

dist_bin_SCRIPTS=makealias affixcompress wordforms ispellaff2myspell wordlist2hunspell

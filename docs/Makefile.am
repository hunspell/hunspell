dist_man_MANS = hunspell.1 hunspell.3 hunspell.5 hzip.1 hunzip.1

if HAVE_RONN
SUFFIXES = .md
.md:
	ronn -r $<
endif

EXTRA_DIST = hunspell.1.md hunspell.5.md README.md
SUBDIRS=hu

# if config.h is missing, first run: autoreconf -vfi; ./configure

#TODO move all this to several Makefile.am files
#TODO integrate with Qt https://sourceforge.net/projects/qtprojecttool/files/
#TODO add more checkers such as ver++, krazy, cpplint, cloc

cd ..

cppcheck --check-config \
--enable=all \
--suppress=missingIncludeSystem \
-I. \
-Isrc/hunspell2 \
-Isrc/parsers \
-I/usr/include/thunderbird \
src 2> checks/cppcheck_checkconfig.txt

cppcheck \
--enable=all \
--suppress=missingIncludeSystem \
-I. \
-Isrc/hunspell2 \
-Isrc/parsers \
-I/usr/include/thunderbird \
src 2> checks/cppcheck.txt

cppcheck \
--xml-version=2 \
--enable=all \
--suppress=missingIncludeSystem \
-I. \
-Isrc/hunspell2 \
-Isrc/parsers \
-I/usr/include/thunderbird \
src 2> checks/cppcheck.xml

cd checks

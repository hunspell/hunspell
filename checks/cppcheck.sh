# If header file config.h is missing, first run: autoreconf -vfi; ./configure

cd ..

#cppcheck --check-config \
#--enable=all --suppress=missingIncludeSystem -Isrc/hunspell2 src/hunspell2/*.[ch]xx tests/*.[ch]xx \
#2> checks/cppcheck_checkconfig.txt

cppcheck \
--enable=all --suppress=missingIncludeSystem -Isrc/hunspell2 src/hunspell2/*.[ch]xx tests/*.[ch]xx \
2> checks/cppcheck.txt

#cppcheck \
#--xml-version=2 \
#--enable=all --suppress=missingIncludeSystem -Isrc/hunspell2 src/hunspell2/*.[ch]xx tests/*.[ch]xx \
#2> checks/cppcheck.xml

cd checks

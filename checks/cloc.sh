cd ..

cloc \
--3 --by-file src/hunspell2 tests/*.[ch]xx tests/*.am \
> checks/cloc.txt

#cloc \
#--3 --by-file src/hunspell2 tests/*.[ch]xx tests/*.am \
#--xml > checks/cloc.xml

cd checks

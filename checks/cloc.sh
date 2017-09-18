cd ..

cloc \
--3 --by-file src/hunspell2/*.?xx tests/*.?xx \
> checks/cloc.txt

cloc \
--3 --by-file src/hunspell2/*.?xx tests/*.?xx \
--xml > checks/cloc.xml

cd checks

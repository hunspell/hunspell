# if config.h is missing, first run: autoreconf -vfi; ./configure

#TODO move all this to several Makefile.am files
#TODO integrate with Qt https://sourceforge.net/projects/qtprojecttool/files/
#TODO add more checkers such as ver++, krazy, cpplint, cloc

if [ -e cppcheck ]
then
    rm -rf cppcheck/*
else
    mkdir cppcheck
fi

for i in `echo src/hunspell2 src/parsers src/tools`
do
    mkdir -p cppcheck/$i
    cd $i
    for j in `ls *.cxx`
    do
        echo $i/$j >> ../../cppcheck/log

        cppcheck --check-config \
        --xml-version=2 \
        --enable=all \
        --suppress=missingIncludeSystem \
        -I../.. \
        -I../../src/hunspell2 \
        -I../../src/parsers \
        -I/usr/include/thunderbird \
        $j 2> ../../cppcheck/$i/$j.checkconfig.xml

        cppcheck \
        --xml-version=2 \
        --enable=all \
        --suppress=missingIncludeSystem \
        -I../.. \
        -I../../src/hunspell2 \
        -I../../src/parsers \
        -I/usr/include/thunderbird \
        $j 2> ../../cppcheck/$i/$j.xml >> ../../cppcheck/log
    done
    cd ../..
done

#!/usr/bin/env bash

set -e

if [ -z "${OUT}" ] || [ -z "${SRC}" ] || [ -z "${WORK}" ]; then
    echo "OUT, SRC or WORK not set - script expects to be called inside oss-fuzz build env"
    exit 1
fi

# The oss-fuzz base image (Ubuntu Focal) has gettext 0.19, but
# configure.ac requires 0.20. Fuzzers don't need NLS, so skip
# autopoint and disable NLS entirely.
AUTOPOINT=true autoreconf -vfi
./configure --disable-shared --enable-static --disable-nls
make clean
make -j$(nproc)
$CXX $CXXFLAGS -o $OUT/fuzzer -I./src/ $LIB_FUZZING_ENGINE ./src/tools/fuzzer.cxx ./src/hunspell/.libs/libhunspell-1.7.a
cp -f ./src/tools/fuzzer.options $OUT/
cp -f ./src/tools/persdicfuzzer.options $OUT/
cp -f ./src/tools/parserfuzzer.options $OUT/
$CXX $CXXFLAGS -o $OUT/affdicfuzzer -I./src/ $LIB_FUZZING_ENGINE ./src/tools/affdicfuzzer.cxx ./src/hunspell/.libs/libhunspell-1.7.a
$CXX $CXXFLAGS -o $OUT/persdicfuzzer -I./src/ $LIB_FUZZING_ENGINE ./src/tools/persdicfuzzer.cxx ./src/hunspell/.libs/libhunspell-1.7.a
$CXX $CXXFLAGS -o $OUT/parserfuzzer -I./src/ $LIB_FUZZING_ENGINE ./src/tools/parserfuzzer.cxx ./src/parsers/libparsers.a ./src/hunspell/.libs/libhunspell-1.7.a

#dic/aff combos to test
cp -f ./tests/arabic.* $OUT/
cp -f ./tests/checkcompoundpattern*.* $OUT/
cp -f ./tests/korean.* $OUT/
cp -f ./tests/utf8*.* $OUT/

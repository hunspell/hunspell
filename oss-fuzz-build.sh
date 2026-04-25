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

#dic/aff combos to test - one representative per feature to keep the
#fuzzer's loop-over-all-dicts cost bounded
cp -f ./tests/alias.* $OUT/
cp -f ./tests/arabic.* $OUT/
cp -f ./tests/break.* $OUT/
cp -f ./tests/checkcompoundpattern.* $OUT/
cp -f ./tests/circumfix.* $OUT/
cp -f ./tests/complexprefixes.* $OUT/
cp -f ./tests/compoundflag.* $OUT/
cp -f ./tests/hu.* $OUT/
cp -f ./tests/iconv.* $OUT/
cp -f ./tests/ignore.* $OUT/
cp -f ./tests/keepcase.* $OUT/
cp -f ./tests/korean.* $OUT/
cp -f ./tests/maputf.* $OUT/
cp -f ./tests/needaffix.* $OUT/
cp -f ./tests/oconv.* $OUT/
cp -f ./tests/phone.* $OUT/
cp -f ./tests/reputf.* $OUT/
cp -f ./tests/utf8.* $OUT/
cp -f ./tests/warn.* $OUT/

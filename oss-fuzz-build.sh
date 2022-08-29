#!/usr/bin/env bash

set -e

if [ -z "${OUT}" ] || [ -z "${SRC}" ] || [ -z "${WORK}" ]; then
    echo "OUT, SRC or WORK not set - script expects to be called inside oss-fuzz build env"
    exit 1
fi

autoreconf -vfi
./configure --disable-shared --enable-static
make clean
make -j$(nproc)
$CXX $CXXFLAGS -o $OUT/fuzzer -I./src/ $LIB_FUZZING_ENGINE ./src/tools/fuzzer.cxx ./src/hunspell/.libs/libhunspell-1.7.a

#dic/aff combos to test
cp -f ./tests/arabic.* $OUT/
cp -f ./tests/checkcompoundpattern*.* $OUT/
cp -f ./tests/korean.* $OUT/
cp -f ./tests/utf8*.* $OUT/

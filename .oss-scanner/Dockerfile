# OSS Scanner build for Hunspell (https://github.com/hunspell/hunspell).
#
# Two builds of the same checkout:
#
#   /src/build-asan   clang, AddressSanitizer and UndefinedBehaviorSanitizer, -O1 -g. The library, the
#                     hunspell command line tool, and every fuzz target in src/tools/*fuzzer.cxx linked as
#                     a standalone binary in /src/build-asan/fuzz/ that runs the files named on its command
#                     line and exits.
#   /src              a plain -O2 -g build with frame pointers, installed to /usr/local, with make check run.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates git python3 make gawk autoconf automake libtool gettext autopoint \
      clang libclang-rt-18-dev llvm g++ gdb valgrind \
    && rm -rf /var/lib/apt/lists/*

# scanner contract: the checkout lives inside the image, at /src
COPY . /src
WORKDIR /src

RUN autoreconf -vfi

# Sanitizer build, out of tree so the plain build below can configure in place.
RUN mkdir /src/build-asan && cd /src/build-asan \
    && CC=clang CXX=clang++ \
       CFLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined" \
       CXXFLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined" \
       LDFLAGS="-fsanitize=address,undefined" \
       ../configure --disable-shared --enable-static --disable-nls \
    && make -j"$(nproc)" \
    && mkdir fuzz \
    && for f in /src/src/tools/*fuzzer.cxx; do \
         name=$(basename "$f" .cxx); \
         extra=; [ "$name" = parserfuzzer ] && extra=src/parsers/libparsers.a; \
         clang++ -O1 -g -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined \
           -I/src/src -I/src/build-asan -o fuzz/"$name" "$f" $extra src/hunspell/.libs/libhunspell-1.7.a \
           || exit 1; \
       done

# Plain build, as distributions build it.
RUN CFLAGS="-O2 -g -fno-omit-frame-pointer" CXXFLAGS="-O2 -g -fno-omit-frame-pointer" \
       ./configure --disable-nls \
    && make -j"$(nproc)" && make install && ldconfig

RUN make -j"$(nproc)" check || echo "WARNING: make check reported failures"

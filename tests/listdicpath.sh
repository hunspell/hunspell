#!/bin/sh
# hunspell -D lists the dictionaries at the top of each search path folder, and none of the ones
# in its subfolders. Hyphenation tables, hyph_*.dic, are not dictionaries.

[ "$HUNSPELL" = "" ] && HUNSPELL="$(dirname "$0")"/../src/tools/hunspell
[ "$LIBTOOL" = "" ] && LIBTOOL="$(dirname "$0")"/../libtool

DIR=./testSubDir/listdicpath
rm -rf "$DIR"
mkdir -p "$DIR/sub"
touch "$DIR/top.dic" "$DIR/hyph_top.dic" "$DIR/sub/nested.dic"
ln -s .. "$DIR/sub/loop"

LIST=$(DICPATH="$DIR" $LIBTOOL --mode=execute "$HUNSPELL" -D -a /dev/null 2>&1 >/dev/null)

if ! echo "$LIST" | grep -qx "$DIR/top"; then
    echo "listdicpath: $DIR/top is not listed"
    echo "$LIST"
    exit 1
fi
if echo "$LIST" | grep -q "^$DIR/sub"; then
    echo "listdicpath: a dictionary in a subfolder is listed"
    echo "$LIST"
    exit 1
fi
if echo "$LIST" | grep -q "^$DIR/hyph_"; then
    echo "listdicpath: a hyphenation table is listed as a dictionary"
    echo "$LIST"
    exit 1
fi
exit 0

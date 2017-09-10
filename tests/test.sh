#!/bin/bash
# Copyright 2016-2017 Dimitrij Mijoski
#
# This file is part of Hunspell-2.
#
# Hunspell-2 is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hunspell-2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Hunspell-2.	If not, see <http://www.gnu.org/licenses/>.
#
# Hunspell 2 is based on Hunspell v1 and MySpell.
# Hunspell v1 is Copyright (C) 2002-2017 Németh László
# MySpell is Copyright (C) 2002 Kevin Hendricks.

# set -x # uncomment for debugging

export LC_ALL="C"

function check_valgrind_log () {
if [[ "$VALGRIND" != "" && -f $TEMPDIR/test.pid* ]]; then
	log=$(ls $TEMPDIR/test.pid*)
	if ! grep -q 'ERROR SUMMARY: 0 error' $log; then
		echo "Fail in $NAME $1 checking detected by Valgrind"
		echo "$log Valgrind log file moved to $TEMPDIR/badlogs"
		mv $log $TEMPDIR/badlogs
		exit 1
	fi
	if grep -q 'LEAK SUMMARY' $log; then
		echo "Memory leak in $NAME $1 checking detected by Valgrind"
		echo "$log Valgrind log file moved to $TEMPDIR/badlogs"
		mv $log $TEMPDIR/badlogs
		exit 1
	fi
	rm -f $log
fi
}

TESTDIR=.
TEMPDIR=$TESTDIR/testSubDir
NAME=$(basename "$1" .dic)
shift
ENCODING=UTF-8 #io encoding passed with -i
if [[ "$1" == "-i" && -n "$2" ]]; then
	ENCODING=$2
	shift 2
fi
shopt -s expand_aliases

[[ "$HUNSPELL" == "" ]] && HUNSPELL=$(dirname $0)/../src/tools/hunspell
ANALYZE=$(dirname $0)/../src/tools/analyze
LIBTOOL=$(dirname $0)/../libtool
alias hunspell='"$LIBTOOL" --mode=execute "$HUNSPELL"'
alias analyze='"$LIBTOOL" --mode=execute "$ANALYZE"'

if [[ "$VALGRIND" != "" ]]; then
	mkdir $TEMPDIR 2> /dev/null
	rm -f $TEMPDIR/test.pid*
	mkdir $TEMPDIR/badlogs 2> /dev/null
	alias hunspell='"$LIBTOOL" --mode=execute valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid "$HUNSPELL"'
	alias analyze='"$LIBTOOL" --mode=execute valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid "$ANALYZE"'
fi

CR=$(printf "\r")

in_dict="$TESTDIR/$NAME"

# Tests good words
in_file="$in_dict.good"

if [[ -f $in_file ]]; then
	out=$(hunspell -l -i $ENCODING $* -d $in_dict < $in_file \
	      | tr -d "$CR")
	if [[ $out != "" ]]; then
		echo "============================================="
		echo "Fail in $NAME.good. Good words recognised as wrong:"
		echo "$out"
		exit 1
	fi
fi

check_valgrind_log "good words"

# Tests bad words
in_file="$in_dict.wrong"

if [[ -f $in_file ]]; then
	out=$(hunspell -G -i $ENCODING $* -d $in_dict < "$in_file" \
	      | tr -d "$CR") #strip carige return for mingw builds
	if [[ "$out" != "" ]]; then
		echo "============================================="
		echo "Fail in $NAME.wrong. Bad words recognised as good:"
		echo "$out"
		exit 1
	fi
fi

check_valgrind_log "bad words"

# Tests morphological analysis
in_file="$in_dict.good"
expected_file="$in_dict.morph"

if [[ -f $expected_file ]]; then
	#in=$(sed 's/	$//' "$in_file") #passes without this.
	out=$(analyze $in_dict.aff $in_dict.dic $in_file \
	      | tr -d "$CR") #strip carige return for mingw builds
	expected=$(<$expected_file)
	if [[ "$out" != "$expected" ]]; then
		echo "============================================="
		echo "Fail in $NAME.morph. Bad analysis?"
		diff $expected_file <(echo "$out") | grep '^<' | sed 's/^..//'
		exit 1
	fi
fi

check_valgrind_log "morphological analysis"

# Tests suggestions
in_file=$in_dict.wrong
expected_file=$in_dict.sug

if [[ -f $expected_file ]]; then
	out=$(hunspell -i $ENCODING $* -a -d $in_dict <$in_file | \
              grep -a '^&' | sed 's/^[^:]*: //')
	expected=$(<$expected_file) 
	if [[ "$out" != "$expected" ]]; then
		echo "============================================="
		echo "Fail in $NAME.sug. Bad suggestion?"
		diff $expected_file <(echo "$out")
		exit 1
	fi
fi

check_valgrind_log "suggestion"

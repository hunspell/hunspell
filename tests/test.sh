#!/bin/bash
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is Hunspell, based on MySpell.
#
# The Initial Developers of the Original Code are
# Kevin Hendricks (MySpell) and Németh László (Hunspell).
# Portions created by the Initial Developers are Copyright (C) 2002-2005
# the Initial Developers. All Rights Reserved.
#
# Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
# Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
# Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
# Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
# Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

# set -x # uncomment for debugging

export LC_ALL="C"

function check_valgrind_log () {
if [[ "$VALGRIND" != "" ]]; then
  if [[ -f $TEMPDIR/test.pid* ]]; then
    log=`ls $TEMPDIR/test.pid*`
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

if [ ! -d $TEMPDIR ]; then
  mkdir $TEMPDIR
fi

shopt -s expand_aliases

alias hunspell='../libtool --mode=execute -dlopen ../src/hunspell/.libs/libhunspell*.la ../src/tools/hunspell'
alias analyze='../libtool --mode=execute -dlopen ../src/hunspell/.libs/libhunspell*.la ../src/tools/analyze'

if [[ "$VALGRIND" != "" ]]; then
  rm -f $TEMPDIR/test.pid*
  if [[ ! -d $TEMPDIR/badlogs ]]; then
    mkdir $TEMPDIR/badlogs
  fi

  alias hunspell='../libtool --mode=execute -dlopen ../src/hunspell/.libs/libhunspell*.la valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid ../src/tools/hunspell'
  alias analyze='../libtool --mode=execute -dlopen ../src/hunspell/.libs/libhunspell*.la valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid ../src/tools/analyze'
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
    if [[ "$out" != "" ]] >/dev/null; then
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
    in=$(sed 's/	$//' "$in_file")
    out=$(analyze $in_dict.aff $in_dict.dic <(echo "$in") \
          | tr -d "$CR") #strip carige return for mingw builds
    expected=$(<$expected_file)
    if [[ "$out" != "$expected" ]]; then
        echo "============================================="
        echo "Fail in $NAME.morph. Bad analysis?"
        diff $expected_file <(echo "$out")  | grep '^<' | sed 's/^..//'
        exit 1
    fi
fi

check_valgrind_log "morphological analysis"

# Tests suggestions
in_file=$in_dict.wrong
expected_file=$in_dict.sug

if [[ -f $expected_file ]]; then
    out=$(hunspell -i $ENCODING $* -a -d $in_dict <$in_file | grep -a '^&' | \
        sed 's/^[^:]*: //')
    expected=$(<$expected_file) 
    if [[ "$out" != "$expected" ]]; then
        echo "============================================="
        echo "Fail in $NAME.sug. Bad suggestion?"
        diff $expected_file <(echo "$out")
        exit 1
    fi
fi

check_valgrind_log "suggestion"

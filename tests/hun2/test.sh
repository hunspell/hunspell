#!/usr/bin/env bash
BIN=../../src/hunspell2/hun2
FAILED=0
PASSED=0

TST='no arguments'
echo 'Test: '$TST
EXE=$BIN
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi
#FIXME Error message
#   Dictionary  not found.
# should be
#   Requires dictionary.

TST='incomplete option'
ARG=-
echo 'Test: '$TST
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi
#FIXME Error message
#   Dictionary  not found.
# should be
#   Incomplete argument.
# or
#   Unrecognized option: '-'
#   Invalid arguments

TST='invalid option'
ARG=-X
echo 'Test: '$TST
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

TST='non-ASCII invalid option'
ARG=-ĳ
echo 'Test: '$TST
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi
#FIXME Error message
#   Unrecognized option: '-�'
#   Unrecognized option: '-�'
#   Invalid arguments
# should be
#   Unrecognized option: '-ĳ'
#   Invalid arguments

TST='list dictionaries'
echo 'Test: '$TST
ARG=-D
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=0
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

TST='non-existing system dictionary'
echo 'Test: '$TST
ARG=-d\ xy
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

TST='no-existing non-ASCII system dictionary'
echo 'Test: '$TST
ARG=-d\ ĳn
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

TST='non-existing system dictionary'
echo 'Test: '$TST
ARG=-d\ /usr/share/hunspell/xy
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

echo 'Test: '$TST
ARG=-d\ /usr/share/hunspell/ĳn
EXE=$BIN\ $ARG
echo '  Command: '$EXE
COR=1
OUT=`$EXE 2>/tmp/stderr`
ERR=`cat /tmp/stderr`
echo '  Stdout: '$OUT
echo '  Stderr: '$ERR
$EXE 2>/dev/null >/dev/null
if [ $? != $COR ]; then
    FAILED=$((FAILED+1))
    echo '  Failed'
else
    PASSED=$((PASSED+1))
    echo '  Passed'
fi

#echo 'Test: '$TST
#ARG=-d\ en_US\ test-en_US.txt
#EXE=$BIN\ $ARG
#echo '  Command: '$EXE
#COR=0
#OUT=`$EXE 2>/tmp/stderr`
#ERR=`cat /tmp/stderr`
#echo '  Stdout: '$OUT
#echo '  Stderr: '$ERR
#$EXE 2>/dev/null >/dev/null
#if [ $? != $COR ]; then
#    FAILED=$((FAILED+1))
#    echo '  Failed'
#else
#    PASSED=$((PASSED+1))
#    echo '  Passed'
#fi

echo 'Tests failed: '$FAILED
echo 'Tests passed: '$PASSED

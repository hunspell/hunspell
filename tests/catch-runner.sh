#!/bin/sh
T="$1"
shift
"$T" -s -r tap "$@"

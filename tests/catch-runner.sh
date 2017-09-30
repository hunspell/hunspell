#!/bin/bash
T="$1"
shift
"$T" -s -r tap "$@"

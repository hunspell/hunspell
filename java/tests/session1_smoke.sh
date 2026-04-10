#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/minimal.dic" <<'DIC'
2
hello
world
DIC

mkdir -p "$TMP_DIR/classes"
javac -d "$TMP_DIR/classes" $(find "$ROOT/src/main/java" -name '*.java')

printf 'hello\nwrong\n' | java -cp "$TMP_DIR/classes" org.hunspell.cli.HunspellTool -d "$TMP_DIR/minimal.dic" -l > "$TMP_DIR/out.txt"

if [[ "$(cat "$TMP_DIR/out.txt")" != "wrong" ]]; then
  echo "unexpected output"
  cat "$TMP_DIR/out.txt"
  exit 1
fi

echo "session1_smoke: pass"

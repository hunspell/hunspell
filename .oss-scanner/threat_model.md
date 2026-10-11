# Threat model

## What this project does and where untrusted input enters

Hunspell is a spell checking library with a small command line tool. Office suites, browsers,
mail clients and editors embed it.

The main untrusted input is the text being checked: words taken from a document or a web page,
passed to spell, suggest, analyze, stem and generate. A word can be any byte sequence in the
dictionary encoding, of any length. A word that starts with "<?xml" is read as an XML query by
spellml, so that parser is reached from ordinary input too.

Dictionaries (.aff and .dic files, .hz compressed files, and personal word lists) are a weaker
boundary. They are normally installed from a distribution package or an extension, so a
malicious dictionary needs the user to install it. Memory safety bugs in dictionary parsing are
still in scope, at a lower severity than the same bug reached from a word.

## Components that matter most and least

- Matters most: src/hunspell, the library: hunspell.cxx, affixmgr.cxx, affentry.cxx,
  suggestmgr.cxx, hashmgr.cxx, csutil.cxx, and hunzip.cxx for .hz dictionaries.
- Lower: src/parsers (text, HTML, XML, LaTeX and other input filters for the command line tool)
  and src/tools (the command line tools themselves).
- Out of scope: the msvc/ build files, po/ translations, and the man pages.

## How to exercise it

- /src/build-asan/fuzz/ holds the oss-fuzz targets built with AddressSanitizer and
  UndefinedBehaviorSanitizer. Each one runs the files named on its command line:
  - fuzzer: checks and suggests words against the test dictionaries copied next to it. Copy
    /src/tests/*.aff and *.dic into the current directory first.
  - affdicfuzzer: the first byte is the length of a word, the rest is split in half into an .aff
    file and a .dic file, then the word is checked and suggested.
  - persdicfuzzer: personal dictionary loading.
  - morphfuzzer: morphological analysis and generation.
  - parserfuzzer: the text parsers in src/parsers.
  - hzfuzzer: the .hz decoder.
  Several targets write their inputs to fixed names under /tmp.
- /usr/local/bin/hunspell is the command line tool. "hunspell -d <dir>/<name> -a" reads words
  on standard input.
- /src/tests holds about 170 small .aff and .dic pairs, each with .good, .wrong, .sug and
  .morph files of expected results. "make check" in /src runs them.

## How we rate severity

- An out of bounds write, use after free or double free reached from a checked word: critical.
- An out of bounds read reached from a checked word: high.
- The same bugs reached only through a crafted dictionary: one level lower.
- A null dereference, assertion failure or other clean crash: medium from a word, low from a
  dictionary.
- Slow checking, high memory use and timeouts: out of scope, as in SECURITY.md. oss-fuzz already
  reports these.
- Wrong spelling results or bad suggestions: not security issues.

## Anything to leave alone

- Bugs that need Hunspell::add, remove or add_with_affix called with a crafted word on a crafted
  dictionary are low. The common embedding applications do not call these with document text.
- Integer overflow that only wraps a count and is caught later without a memory error is low.
- Findings that oss-fuzz has already reported against the same commit need not be sent again.

## How reports and patches should look

A reproducer file and the sanitizer output are the most useful parts. A patch should be the
smallest change that fixes the bug, on master, in the style of the surrounding code.

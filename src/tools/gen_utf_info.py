#!/usr/bin/env python3
"""
Regenerate src/hunspell/utf_info.hxx from a Unicode UnicodeData.txt.

Usage:
    gen_utf_info.py UnicodeData.txt > src/hunspell/utf_info.hxx

Emits a fresh BMP table (U+0000 to U+FFFF), one
{ cletter, cupper, clower } entry per codepoint.

cletter is true when the codepoint's General_Category is one of:

    Lu Ll Lt Lm Lo  any kind of letter
    Mn Mc           combining marks (nonspacing and spacing)

Combining marks must be word characters: in Indic scripts the visible
matras are Mc, and treating them as word breakers shreds every word at
its first vowel sign (see issue #1057).

cupper / clower come from UnicodeData.txt fields 12 and 13 (Simple
Uppercase Mapping / Simple Lowercase Mapping). Where no mapping is
recorded, both default to the codepoint itself. Mappings that target
codepoints outside the BMP fall back to self, since the table is
indexed by 16-bit values.

UnicodeData.txt represents large contiguous blocks (CJK Ideographs,
Hangul Syllables) using <..., First> / <..., Last> markers; this
script expands them.

The download lives at:

    https://www.unicode.org/Public/<version>/ucd/UnicodeData.txt
"""

import sys

LETTER_LIKE = {'Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Mn', 'Mc'}

LICENSE_HEADER = """\
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * Copyright (C) 2002-2022 Németh László
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Hunspell is based on MySpell which is Copyright (C) 2002 Kevin Hendricks.
 *
 * Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
 * Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
 * Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
 * Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
 * Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
"""


def parse_unicode_data(path):
    """Yield (codepoint, general_category, upper, lower) for every assigned
    codepoint, expanding <..., First> / <..., Last> range markers."""
    pending_first = None
    with open(path, encoding='ascii') as f:
        for lineno, raw in enumerate(f, 1):
            fields = raw.rstrip('\n').split(';')
            if len(fields) < 14:
                continue
            cp = int(fields[0], 16)
            name = fields[1]
            gc = fields[2]
            up = int(fields[12], 16) if fields[12] else cp
            lo = int(fields[13], 16) if fields[13] else cp

            if name.endswith(', First>'):
                pending_first = (cp, gc)
                continue
            if name.endswith(', Last>'):
                if pending_first is None:
                    sys.exit(f"{path}:{lineno}: 'Last' marker without 'First'")
                first_cp, first_gc = pending_first
                if first_gc != gc:
                    sys.exit(f"{path}:{lineno}: range gc mismatch")
                for c in range(first_cp, cp + 1):
                    yield c, gc, c, c
                pending_first = None
                continue
            yield cp, gc, up, lo
    if pending_first is not None:
        sys.exit(f"{path}: unterminated range starting at U+{pending_first[0]:04X}")


def build_table(unicode_data_path):
    table = [(False, c, c) for c in range(0x10000)]
    for cp, gc, up, lo in parse_unicode_data(unicode_data_path):
        if cp >= 0x10000:
            continue
        if up >= 0x10000:
            up = cp
        if lo >= 0x10000:
            lo = cp
        table[cp] = (gc in LETTER_LIKE, up, lo)
    return table


def emit(table, out):
    out.write(LICENSE_HEADER)
    out.write("\n")
    out.write("// Unicode character encoding information\n")
    out.write("struct unicode_info {\n")
    out.write("  bool cletter;\n")
    out.write("  unsigned short cupper;\n")
    out.write("  unsigned short clower;\n")
    out.write("};\n")
    out.write("\n")
    out.write("/* fields: Unicode isletter, toupper, tolower */\n")
    out.write("// clang-format off\n")
    out.write("static const struct unicode_info utf_tbl[] = {\n")
    for cp, (is_letter, up, lo) in enumerate(table):
        flag = 'true' if is_letter else 'false'
        out.write(
            f"/* 0x{cp:04x} */ {{ {flag}, 0x{up:04x}, 0x{lo:04x} }},\n"
        )
    out.write("};\n")
    out.write("// clang-format on\n")


def main(argv):
    if len(argv) != 2:
        sys.exit("usage: gen_utf_info.py UnicodeData.txt > utf_info.hxx")
    table = build_table(argv[1])
    emit(table, sys.stdout)


if __name__ == '__main__':
    main(sys.argv)

/* Copyright 2002-2017 Németh László
 *
 * This file is part of Hunspell.
 *
 * Hunspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hunspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "htmlparser.hxx"

#ifndef W32
using namespace std;
#endif

static const char* PATTERN[][2] = {{"<script", "</script>"},
                                   {"<style", "</style>"},
                                   {"<code", "</code>"},
                                   {"<samp", "</samp>"},
                                   {"<kbd", "</kbd>"},
                                   {"<var", "</var>"},
                                   {"<listing", "</listing>"},
                                   {"<address", "</address>"},
                                   {"<pre", "</pre>"},
                                   {"<!--", "-->"},
                                   {"<[cdata[", "]]>"},  // XML comment
                                   {"<", ">"}};

#define PATTERN_LEN (sizeof(PATTERN) / (sizeof(char*) * 2))

static const char* PATTERN2[][2] = {
    {"<img", "alt="},  // ALT and TITLE attrib handled spec.
    {"<img", "title="},
    {"<a ", "title="}};

#define PATTERN_LEN2 (sizeof(PATTERN2) / (sizeof(char*) * 2))

HTMLParser::HTMLParser(const char* wordchars)
  : XMLParser(wordchars) {
}

HTMLParser::HTMLParser(const w_char* wordchars, int len)
  : XMLParser(wordchars, len) {
}

bool HTMLParser::next_token(std::string& t) {
  return XMLParser::next_token(PATTERN, PATTERN_LEN, PATTERN2, PATTERN_LEN2, t);
}

HTMLParser::~HTMLParser() {}

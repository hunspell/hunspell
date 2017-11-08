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
#include "odfparser.hxx"

#ifndef W32
using namespace std;
#endif

static const char* PATTERN[][2] = {
    {"<office:meta>", "</office:meta>"},
    {"<office:settings>", "</office:settings>"},
    {"<office:binary-data>", "</office:binary-data>"},
    {"<!--", "-->"},
    {"<[cdata[", "]]>"},  // XML comment
    {"<", ">"}};

#define PATTERN_LEN (sizeof(PATTERN) / (sizeof(char*) * 2))

static const char* (*PATTERN2)[2] = NULL;

#define PATTERN_LEN2 0

ODFParser::ODFParser(const char* wordchars)
  : XMLParser(wordchars) {
}

ODFParser::ODFParser(const w_char* wordchars, int len)
  : XMLParser(wordchars, len) {
}

bool ODFParser::next_token(std::string& t) {
  return XMLParser::next_token(PATTERN, PATTERN_LEN, PATTERN2, PATTERN_LEN2, t);
}

ODFParser::~ODFParser() {}

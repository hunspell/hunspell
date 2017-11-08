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
#include "firstparser.hxx"

#ifndef W32
using namespace std;
#endif

FirstParser::FirstParser(const char* wordchars)
  : TextParser(wordchars) {
}

FirstParser::~FirstParser() {}

bool FirstParser::next_token(std::string& t) {
  t.clear();
  const size_t tabpos = line[actual].find('\t');
  if (tabpos != std::string::npos && tabpos > token) {
    token = tabpos;
    t = line[actual].substr(0, tabpos);
    return true;
  }
  return false;
}

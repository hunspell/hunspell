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
#include "manparser.hxx"

#ifndef W32
using namespace std;
#endif

ManParser::ManParser(const char* wordchars)
  : TextParser(wordchars) {
}

ManParser::ManParser(const w_char* wordchars, int len)
  : TextParser(wordchars, len) {
}

ManParser::~ManParser() {}

bool ManParser::next_token(std::string& t) {
  for (;;) {
    switch (state) {
      case 1:  // command arguments
        if (line[actual][head] == ' ')
          state = 2;
        break;
      case 0:  // dot in begin of line
        if (line[actual][0] == '.') {
          state = 1;
          break;
        } else {
          state = 2;
        }
      // no break
      case 2:  // non word chars
        if (is_wordchar(line[actual].c_str() + head)) {
          state = 3;
          token = head;
        } else if ((line[actual][head] == '\\') &&
                   (line[actual][head + 1] == 'f') &&
                   (line[actual][head + 2] != '\0')) {
          head += 2;
        }
        break;
      case 3:  // wordchar
        if (!is_wordchar(line[actual].c_str() + head)) {
          state = 2;
          if (alloc_token(token, &head, t))
            return true;
        }
        break;
    }
    if (next_char(line[actual].c_str(), &head)) {
      state = 0;
      return false;
    }
  }
}

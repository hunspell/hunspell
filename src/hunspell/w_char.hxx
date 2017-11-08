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

#ifndef W_CHAR_HXX_
#define W_CHAR_HXX_

#include <string>

#ifndef GCC
struct w_char {
#else
struct __attribute__((packed)) w_char {
#endif
  unsigned char l;
  unsigned char h;

  friend bool operator<(const w_char a, const w_char b) {
    unsigned short a_idx = (a.h << 8) + a.l;
    unsigned short b_idx = (b.h << 8) + b.l;
    return a_idx < b_idx;
  }

  friend bool operator==(const w_char a, const w_char b) {
    return (((a).l == (b).l) && ((a).h == (b).h));
  }

  friend bool operator!=(const w_char a, const w_char b) {
    return !(a == b);;
  }
};

// two character arrays
struct replentry {
  std::string pattern;
  std::string outstrings[4]; // med, ini, fin, isol
};

#endif

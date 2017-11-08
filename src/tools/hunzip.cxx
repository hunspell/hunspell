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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../hunspell/hunzip.hxx"

#define DESC                                                 \
  "hunzip - decompress a hzip file to the standard output\n" \
  "Usage: hunzip file.hz [password]\n"

int main(int argc, char** argv) {
  if (argc == 1 || strcmp(argv[1], "-h") == 0) {
    fprintf(stderr, DESC);
    return 1;
  }
  Hunzip h(argv[1], (argc > 2) ? argv[2] : NULL);
  if (!h.is_open())
    return 0;
  std::string s;
  while (h.getline(s))
    printf("%s", s.c_str());
  return 0;
}

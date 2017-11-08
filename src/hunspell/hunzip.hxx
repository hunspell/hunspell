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

/* hunzip: file decompression for sorted dictionaries with optional encryption,
 * algorithm: prefix-suffix encoding and 16-bit Huffman encoding */

#ifndef HUNZIP_HXX_
#define HUNZIP_HXX_

#include "hunvisapi.h"

#include <stdio.h>
#include <fstream>
#include <vector>

#define BUFSIZE 65536
#define HZIP_EXTENSION ".hz"

#define MSG_OPEN "error: %s: cannot open\n"
#define MSG_FORMAT "error: %s: not in hzip format\n"
#define MSG_MEMORY "error: %s: missing memory\n"
#define MSG_KEY "error: %s: missing or bad password\n"

struct bit {
  unsigned char c[2];
  int v[2];
};

class LIBHUNSPELL_DLL_EXPORTED Hunzip {
 private:
  Hunzip(const Hunzip&);
  Hunzip& operator=(const Hunzip&);

 protected:
  char* filename;
  std::ifstream fin;
  int bufsiz, lastbit, inc, inbits, outc;
  std::vector<bit> dec;     // code table
  char in[BUFSIZE];         // input buffer
  char out[BUFSIZE + 1];    // Huffman-decoded buffer
  char line[BUFSIZE + 50];  // decoded line
  int getcode(const char* key);
  int getbuf();
  int fail(const char* err, const char* par);

 public:
  Hunzip(const char* filename, const char* key = NULL);
  ~Hunzip();
  bool is_open() { return fin.is_open(); }
  bool getline(std::string& dest);
};

#endif

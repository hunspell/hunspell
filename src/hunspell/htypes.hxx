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

#ifndef HTYPES_HXX_
#define HTYPES_HXX_

#define ROTATE_LEN 5

#define ROTATE(v, q) \
  (v) = ((v) << (q)) | (((v) >> (32 - q)) & ((1 << (q)) - 1));

// hentry options
#define H_OPT (1 << 0)
#define H_OPT_ALIASM (1 << 1)
#define H_OPT_PHON (1 << 2)

// see also csutil.hxx
#define HENTRY_WORD(h) &(h->word[0])

// approx. number  of user defined words
#define USERWORD 1000

struct hentry {
  unsigned char blen;    // word length in bytes
  unsigned char clen;    // word length in characters (different for UTF-8 enc.)
  short alen;            // length of affix flag vector
  unsigned short* astr;  // affix flag vector
  struct hentry* next;   // next word with same hash code
  struct hentry* next_homonym;  // next homonym word (with same hash code)
  char var;      // variable fields (only for special pronounciation yet)
  char word[1];  // variable-length word (8-bit or UTF-8 encoding)
};

#endif

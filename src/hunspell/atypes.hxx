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

#ifndef ATYPES_HXX_
#define ATYPES_HXX_

#ifndef HUNSPELL_WARNING
#include <stdio.h>
#ifdef HUNSPELL_WARNING_ON
#define HUNSPELL_WARNING fprintf
#else
// empty inline function to switch off warnings (instead of the C99 standard
// variadic macros)
static inline void HUNSPELL_WARNING(FILE*, const char*, ...) {}
#endif
#endif

// HUNSTEM def.
#define HUNSTEM

#include "w_char.hxx"
#include <algorithm>
#include <string>
#include <vector>

#define SETSIZE 256
#define CONTSIZE 65536

// AffEntry options
#define aeXPRODUCT (1 << 0)
#define aeUTF8 (1 << 1)
#define aeALIASF (1 << 2)
#define aeALIASM (1 << 3)
#define aeLONGCOND (1 << 4)

// compound options
#define IN_CPD_NOT 0
#define IN_CPD_BEGIN 1
#define IN_CPD_END 2
#define IN_CPD_OTHER 3

// info options
#define SPELL_COMPOUND (1 << 0)
#define SPELL_FORBIDDEN (1 << 1)
#define SPELL_ALLCAP (1 << 2)
#define SPELL_NOCAP (1 << 3)
#define SPELL_INITCAP (1 << 4)
#define SPELL_ORIGCAP (1 << 5)
#define SPELL_WARN (1 << 6)

#define MINCPDLEN 3
#define MAXCOMPOUND 10
#define MAXCONDLEN 20
#define MAXCONDLEN_1 (MAXCONDLEN - sizeof(char*))

#define MAXACC 1000

#define FLAG unsigned short
#define FLAG_NULL 0x00
#define FREE_FLAG(a) a = 0

#define TESTAFF(a, b, c) (std::binary_search(a, a + c, b))

struct guessword {
  char* word;
  bool allow;
  char* orig;
};

typedef std::vector<std::string> mapentry;
typedef std::vector<FLAG> flagentry;

struct patentry {
  std::string pattern;
  std::string pattern2;
  std::string pattern3;
  FLAG cond;
  FLAG cond2;
  patentry()
    : cond(FLAG_NULL)
    , cond2(FLAG_NULL) {
  }
};

#endif

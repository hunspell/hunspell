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

#ifndef BASEAFF_HXX_
#define BASEAFF_HXX_

#include <string>

class AffEntry {
 private:
  AffEntry(const AffEntry&);
  AffEntry& operator=(const AffEntry&);

 public:
  AffEntry()
      : numconds(0),
        opts(0),
        aflag(0),
        morphcode(0),
        contclass(NULL),
        contclasslen(0) {}
  virtual ~AffEntry();
  std::string appnd;
  std::string strip;
  unsigned char numconds;
  char opts;
  unsigned short aflag;
  union {
    char conds[MAXCONDLEN];
    struct {
      char conds1[MAXCONDLEN_1];
      char* conds2;
    } l;
  } c;
  char* morphcode;
  unsigned short* contclass;
  short contclasslen;
};

#endif

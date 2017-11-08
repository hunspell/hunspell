/* Copyright 2002-2017 Németh László
 * Copyright 2002 Kevin B. Hendricks, Stratford, Ontario, Canada
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

/* string replacement list class */
#ifndef REPLIST_HXX_
#define REPLIST_HXX_

#include "w_char.hxx"

#include <string>
#include <vector>

class RepList {
 private:
  RepList(const RepList&);
  RepList& operator=(const RepList&);

 protected:
  replentry** dat;
  int size;
  int pos;

 public:
  explicit RepList(int n);
  ~RepList();

  int add(const std::string& pat1, const std::string& pat2);
  replentry* item(int n);
  int find(const char* word);
  std::string replace(const char* word, int n, bool atstart);
  bool conv(const std::string& word, std::string& dest);
};
#endif

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits>

#include "replist.hxx"
#include "csutil.hxx"

RepList::RepList(int n) {
  dat = (replentry**)malloc(sizeof(replentry*) * n);
  if (dat == 0)
    size = 0;
  else
    size = n;
  pos = 0;
}

RepList::~RepList() {
  for (int i = 0; i < pos; i++) {
    delete dat[i];
  }
  free(dat);
}

replentry* RepList::item(int n) {
  return dat[n];
}

int RepList::find(const char* word) {
  int p1 = 0;
  int p2 = pos - 1;
  int ret = -1;
  while (p1 <= p2) {
    int m = ((unsigned)p1 + (unsigned)p2) >> 1;
    int c = strncmp(word, dat[m]->pattern.c_str(), dat[m]->pattern.size());
    if (c < 0)
      p2 = m - 1;
    else if (c > 0)
      p1 = m + 1;
    else {      // scan in the right half for a longer match
      ret = m;
      p1 = m + 1;
    }
  }
  return ret;
}

std::string RepList::replace(const char* word, int ind, bool atstart) {
  int type = atstart ? 1 : 0;
  if (ind < 0)
    return std::string();
  if (strlen(word) == dat[ind]->pattern.size())
    type = atstart ? 3 : 2;
  while (type && dat[ind]->outstrings[type].empty())
    type = (type == 2 && !atstart) ? 0 : type - 1;
  return dat[ind]->outstrings[type];
}

int RepList::add(const std::string& in_pat1, const std::string& pat2) {
  if (pos >= size || in_pat1.empty() || pat2.empty()) {
    return 1;
  }
  // analyse word context
  int type = 0;
  std::string pat1(in_pat1);
  if (pat1[0] == '_') {
    pat1.erase(0, 1);
    type = 1;
  }
  if (!pat1.empty() && pat1[pat1.size() - 1] == '_') {
    type = type + 2;
    pat1.erase(pat1.size() - 1);
  }
  mystrrep(pat1, "_", " ");

  // find existing entry
  int m = find(pat1.c_str());
  if (m >= 0 && dat[m]->pattern == pat1) {
    // since already used
    dat[m]->outstrings[type] = pat2;
    mystrrep(dat[m]->outstrings[type], "_", " ");
    return 0;
  }

  // make a new entry if none exists
  replentry* r = new replentry;
  if (r == NULL)
    return 1;
  r->pattern = pat1;
  r->outstrings[type] = pat2;
  mystrrep(r->outstrings[type], "_", " ");
  dat[pos++] = r;
  // sort to the right place in the list
  int i;
  for (i = pos - 1; i > 0; i--) {
    if (strcmp(r->pattern.c_str(), dat[i - 1]->pattern.c_str()) < 0) {
      dat[i] = dat[i - 1];
    } else
      break;
  }
  dat[i] = r;
  return 0;
}

bool RepList::conv(const std::string& in_word, std::string& dest) {
  dest.clear();

  size_t wordlen = in_word.size();
  const char* word = in_word.c_str();

  bool change = false;
  for (size_t i = 0; i < wordlen; ++i) {
    int n = find(word + i);
    std::string l = replace(word + i, n, i == 0);
    if (!l.empty()) {
      dest.append(l);
      i += dat[n]->pattern.size() - 1;
      change = true;
    } else {
      dest.push_back(word[i]);
    }
  }

  return change;
}


/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 *
 * Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
 * Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
 * Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
 * Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
 * Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/*
 * Copyright 2002 Kevin B. Hendricks, Stratford, Ontario, Canada
 * And Contributors.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY KEVIN B. HENDRICKS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * KEVIN B. HENDRICKS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
    free(dat[i]->pattern);
    for (int j = 0; j < 4; ++j)
        free(dat[i]->outstrings[j]);
    free(dat[i]);
  }
  free(dat);
}

int RepList::get_pos() {
  return pos;
}

replentry* RepList::item(int n) {
  return dat[n];
}

int RepList::find(const char* word) {
  int p1 = 0;
  int p2 = pos - 1;
  while (p1 <= p2) {
    int m = (p1 + p2) / 2;
    int c = strncmp(word, dat[m]->pattern, dat[m]->plen);
    if (c < 0)
      p2 = m - 1;
    else if (c > 0)
      p1 = m + 1;
    else {      // scan back for a longer match
      for (p1 = m - 1; p1 >= 0; --p1)
        if (!strncmp(word, dat[p1]->pattern, dat[p1]->plen))
          m = p1;
        else if (dat[p1]->plen < dat[m]->plen)
          break;
      return m;
    }
  }
  return -1;
}

char *RepList::replace(const char* word, int ind, bool atstart) {
  int type = atstart ? 1 : 0;
  if (ind < 0)
    return NULL;
  if (strlen(word) == dat[ind]->plen)
    type = atstart ? 3 : 2;
  while (type && !dat[ind]->outstrings[type])
    type = (type == 2 && !atstart) ? 0 : type - 1;
  return dat[ind]->outstrings[type];
}

int RepList::add(char* pat1, char* pat2) {
  if (pos >= size || pat1 == NULL || pat2 == NULL) {
    if (pat1) free(pat1);
    if (pat2) free(pat2);
    return 1;
  }
  // analyse word context
  int type = 0;
  if (*pat1 == '_') {
    ++pat1;
    type = 1;
  }
  if (pat1[strlen(pat1) - 1] == '_') {
    pat1[strlen(pat1) - 1] = 0;
    type = type + 2;
  }
  pat1 = mystrrep(pat1, "_", " ");

  // find existing entry
  int m = find(pat1);
  if (m >= 0 && !strcmp(dat[m]->pattern, pat1)) {
    free(pat1);     // since already used
    dat[m]->outstrings[type] = mystrrep(pat2, "_", " ");
    return 0;
  }

  // make a new entry if none exists
  replentry* r = (replentry*)calloc(1, sizeof(replentry));
  if (r == NULL)
    return 1;
  r->pattern = pat1;
  r->plen = strlen(pat1);
  r->outstrings[type] = mystrrep(pat2, "_", " ");
  dat[pos++] = r;
  // sort to the right place in the list
  int i;
  for (i = pos - 1; i > 0; i--) {
    int c = strncmp(r->pattern, dat[i-1]->pattern, dat[i-1]->plen);
    if (c > 0)
      break;
    else if (c == 0) { // subpatterns match. Patterns can't be identical since would catch earlier
      for (int j = i - 2; j && !strncmp(dat[i-1]->pattern, dat[j]->pattern, dat[i-1]->plen); --j)
        if (dat[j]->plen > r->plen ||
              (dat[j]->plen == r->plen && strncmp(dat[j]->pattern, r->pattern, r->plen) > 0)) {
          i = j;
          break;
        }
      break;
    }
  }
  memmove(dat + i + 1, dat + i, (pos - i - 1) * sizeof(replentry *));
  dat[i] = r;
  return 0;
}

int RepList::conv(const char* word, char* dest, size_t destsize) {
  size_t stl = 0;
  int change = 0;
  for (size_t i = 0; i < strlen(word); i++) {
    int n = find(word + i);
    char* l = replace(word + i, n, i == 0);
    if (l) {
      size_t replen = strlen(l);
      if (stl + replen >= destsize)
        return -1;
      strcpy(dest + stl, l);
      stl += replen;
      i += dat[n]->plen - 1;
      change = 1;
    } else {
      if (stl + 1 >= destsize)
        return -1;
      dest[stl++] = word[i];
    }
  }
  dest[stl] = '\0';
  return change;
}

bool RepList::conv(const char* word, std::string& dest) {
  dest.clear();

  bool change = false;
  for (size_t i = 0; i < strlen(word); i++) {
    int n = find(word + i);
    char *l = replace(word + i, n, i == 0);
    if (l) {
      dest.append(l);
      i += dat[n]->plen - 1;
      change = true;
    } else {
      dest.push_back(word[i]);
    }
  }
  return change;
}


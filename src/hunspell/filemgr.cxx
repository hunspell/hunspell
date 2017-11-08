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

#include "filemgr.hxx"
#include "csutil.hxx"

int FileMgr::fail(const char* err, const char* par) {
  fprintf(stderr, err, par);
  return -1;
}

FileMgr::FileMgr(const char* file, const char* key) : hin(NULL), linenum(0) {
  in[0] = '\0';

  myopen(fin, file, std::ios_base::in);
  if (!fin.is_open()) {
    // check hzipped file
    std::string st(file);
    st.append(HZIP_EXTENSION);
    hin = new Hunzip(st.c_str(), key);
  }
  if (!fin.is_open() && !hin->is_open())
    fail(MSG_OPEN, file);
}

FileMgr::~FileMgr() {
  delete hin;
}

bool FileMgr::getline(std::string& dest) {
  bool ret = false;
  ++linenum;
  if (fin.is_open()) {
    ret = static_cast<bool>(std::getline(fin, dest));
  } else if (hin->is_open()) {
    ret = hin->getline(dest);
  }
  if (!ret) {
    --linenum;
  }
  return ret;
}

int FileMgr::getlinenum() {
  return linenum;
}

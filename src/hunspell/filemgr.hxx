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

/* file manager class - read lines of files [filename] OR [filename.hz] */
#ifndef FILEMGR_HXX_
#define FILEMGR_HXX_

#include "hunzip.hxx"
#include <stdio.h>
#include <string>
#include <fstream>

class FileMgr {
 private:
  FileMgr(const FileMgr&);
  FileMgr& operator=(const FileMgr&);

 protected:
  std::ifstream fin;
  Hunzip* hin;
  char in[BUFSIZE + 50];  // input buffer
  int fail(const char* err, const char* par);
  int linenum;

 public:
  FileMgr(const char* filename, const char* key = NULL);
  ~FileMgr();
  bool getline(std::string&);
  int getlinenum();
};
#endif

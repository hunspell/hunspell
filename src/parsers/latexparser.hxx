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

#ifndef LATEXPARSER_HXX_
#define LATEXPARSER_HXX_

#include "textparser.hxx"

/*
 * HTML Parser
 *
 */

class LaTeXParser : public TextParser {
  int pattern_num;  // number of comment
  int depth;        // depth of blocks
  int arg;          // arguments's number
  int opt;          // optional argument attrib.

 public:
  explicit LaTeXParser(const char* wc);
  LaTeXParser(const w_char* wordchars, int len);
  virtual ~LaTeXParser();

  virtual bool next_token(std::string&);

 private:
  int look_pattern(int col);
};

#endif

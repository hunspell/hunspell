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

#ifndef XMLPARSER_HXX_
#define XMLPARSER_HXX_

#include "textparser.hxx"

/*
 * XML Parser
 *
 */

class XMLParser : public TextParser {
 public:
  explicit XMLParser(const char* wc);
  XMLParser(const w_char* wordchars, int len);
  bool next_token(const char* p[][2],
                  unsigned int len,
                  const char* p2[][2],
                  unsigned int len2,
                  std::string&);
  virtual bool next_token(std::string&);
  int change_token(const char* word);
  virtual ~XMLParser();

 private:
  int look_pattern(const char* p[][2], unsigned int len, int column);
  int pattern_num;
  int pattern2_num;
  int prevstate;
  int checkattr;
  char quotmark;
};

#endif

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

#ifndef HTMLPARSER_HXX_
#define HTMLPARSER_HXX_

#include "xmlparser.hxx"

/*
 * HTML Parser
 *
 */

class HTMLParser : public XMLParser {
 public:
  explicit HTMLParser(const char* wc);
  HTMLParser(const w_char* wordchars, int len);
  virtual bool next_token(std::string&);
  virtual ~HTMLParser();
};

#endif

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

#ifndef ODFPARSER_HXX_
#define ODFPARSER_HXX_

#include "xmlparser.hxx"

/*
 * HTML Parser
 *
 */

class ODFParser : public XMLParser {
 public:
  explicit ODFParser(const char* wc);
  ODFParser(const w_char* wordchars, int len);
  virtual bool next_token(std::string&);
  virtual ~ODFParser();
};

#endif

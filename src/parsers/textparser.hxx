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

#ifndef TEXTPARSER_HXX_
#define TEXTPARSER_HXX_

// set sum of actual and previous lines
#define MAXPREVLINE 4

#ifndef MAXLNLEN
#define MAXLNLEN 8192
#endif

#include "../hunspell/w_char.hxx"

#include <vector>

/*
 * Base Text Parser
 *
 */

class TextParser {
 protected:
  std::vector<int> wordcharacters;// for detection of the word boundaries
  std::string line[MAXPREVLINE];  // parsed and previous lines
  std::vector<bool> urlline;      // mask for url detection
  int checkurl;
  int actual;  // actual line
  size_t head; // head position
  size_t token;// begin of token
  int state;   // state of automata
  int utf8;    // UTF-8 character encoding
  int next_char(const char* line, size_t* pos);
  const w_char* wordchars_utf16;
  int wclen;

 public:
  TextParser(const w_char* wordchars, int len);
  explicit TextParser(const char* wc);
  virtual ~TextParser();

  void put_line(const char* line);
  std::string get_line() const;
  std::string get_prevline(int n) const;
  virtual bool next_token(std::string&);
  virtual int change_token(const char* word);
  void set_url_checking(int check);

  size_t get_tokenpos();
  int is_wordchar(const char* w);
  inline int is_utf8() { return utf8; }
  const char* get_latin1(const char* s);
  char* next_char();
  int tokenize_urls();
  void check_urls();
  int get_url(size_t token_pos, size_t* head);
  bool alloc_token(size_t token, size_t* head, std::string& out);
private:
  void init(const char*);
  void init(const w_char* wordchars, int len);
};

#endif

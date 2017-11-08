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

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "textparser.hxx"

#include <algorithm>

#ifndef W32
using namespace std;
#endif

// ISO-8859-1 HTML character entities

static const char* LATIN1[] = {
    "&Agrave;", "&Atilde;", "&Aring;",  "&AElig;",  "&Egrave;", "&Ecirc;",
    "&Igrave;", "&Iuml;",   "&ETH;",    "&Ntilde;", "&Ograve;", "&Oslash;",
    "&Ugrave;", "&THORN;",  "&agrave;", "&atilde;", "&aring;",  "&aelig;",
    "&egrave;", "&ecirc;",  "&igrave;", "&iuml;",   "&eth;",    "&ntilde;",
    "&ograve;", "&oslash;", "&ugrave;", "&thorn;",  "&yuml;"};

#define LATIN1_LEN (sizeof(LATIN1) / sizeof(char*))

#define ENTITY_APOS "&apos;"
#define UTF8_APOS "\xe2\x80\x99"
#define APOSTROPHE "'"

TextParser::TextParser(const char* wordchars) {
  init(wordchars);
}

TextParser::TextParser(const w_char* wordchars, int len) {
  init(wordchars, len);
}

TextParser::~TextParser() {}

int TextParser::is_wordchar(const char* w) {
  if (*w == '\0')
    return 0;
  if (utf8) {
    std::vector<w_char> wc;
    unsigned short idx;
    u8_u16(wc, w);
    if (wc.empty())
        return 0;
    idx = (wc[0].h << 8) + wc[0].l;
    return (unicodeisalpha(idx) ||
            (wordchars_utf16 &&
             std::binary_search(wordchars_utf16, wordchars_utf16 + wclen, wc[0])));
  } else {
    return wordcharacters[(*w + 256) % 256];
  }
}

const char* TextParser::get_latin1(const char* s) {
  if (s[0] == '&') {
    unsigned int i = 0;
    while ((i < LATIN1_LEN) && strncmp(LATIN1[i], s, strlen(LATIN1[i])))
      i++;
    if (i != LATIN1_LEN)
      return LATIN1[i];
  }
  return NULL;
}

void TextParser::init(const char* wordchars) {
  actual = 0;
  head = 0;
  token = 0;
  state = 0;
  utf8 = 0;
  checkurl = 0;
  wordchars_utf16 = NULL;
  wclen = 0;
  wordcharacters.resize(256, 0);
  if (!wordchars)
    wordchars = "qwertzuiopasdfghjklyxcvbnmQWERTZUIOPASDFGHJKLYXCVBNM";
  for (unsigned int j = 0; j < strlen(wordchars); ++j) {
    wordcharacters[(wordchars[j] + 256) % 256] = 1;
  }
}

void TextParser::init(const w_char* wc, int len) {
  actual = 0;
  head = 0;
  token = 0;
  state = 0;
  utf8 = 1;
  checkurl = 0;
  wordchars_utf16 = wc;
  wclen = len;
}

int TextParser::next_char(const char* ln, size_t* pos) {
  if (*(ln + *pos) == '\0')
    return 1;
  if (utf8) {
    if (*(ln + *pos) >> 7) {
      // jump to next UTF-8 character
      for ((*pos)++; (*(ln + *pos) & 0xc0) == 0x80; (*pos)++)
        ;
    } else {
      (*pos)++;
    }
  } else
    (*pos)++;
  return 0;
}

void TextParser::put_line(const char* word) {
  actual = (actual + 1) % MAXPREVLINE;
  line[actual].assign(word);
  token = 0;
  head = 0;
  check_urls();
}

std::string TextParser::get_prevline(int n) const {
  return line[(actual + MAXPREVLINE - n) % MAXPREVLINE];
}

std::string TextParser::get_line() const {
  return get_prevline(0);
}

bool TextParser::next_token(std::string &t) {
  const char* latin1;

  for (;;) {
    switch (state) {
      case 0:  // non word chars
        if (is_wordchar(line[actual].c_str() + head)) {
          state = 1;
          token = head;
        } else if ((latin1 = get_latin1(line[actual].c_str() + head))) {
          state = 1;
          token = head;
          head += strlen(latin1);
        }
        break;
      case 1:  // wordchar
        if ((latin1 = get_latin1(line[actual].c_str() + head))) {
          head += strlen(latin1);
        } else if ((is_wordchar((char*)APOSTROPHE) ||
                    (is_utf8() && is_wordchar((char*)UTF8_APOS))) &&
                   !line[actual].empty() && line[actual][head] == '\'' &&
                   is_wordchar(line[actual].c_str() + head + 1)) {
          head++;
        } else if (is_utf8() &&
                   is_wordchar((char*)APOSTROPHE) &&  // add Unicode apostrophe
                                                      // to the WORDCHARS, if
                                                      // needed
                   strncmp(line[actual].c_str() + head, UTF8_APOS, strlen(UTF8_APOS)) ==
                       0 &&
                   is_wordchar(line[actual].c_str() + head + strlen(UTF8_APOS))) {
          head += strlen(UTF8_APOS) - 1;
        } else if (!is_wordchar(line[actual].c_str() + head)) {
          state = 0;
          if (alloc_token(token, &head, t))
            return true;
        }
        break;
    }
    if (next_char(line[actual].c_str(), &head))
      return false;
  }
}

size_t TextParser::get_tokenpos() {
  return token;
}

int TextParser::change_token(const char* word) {
  if (word) {
    std::string remainder(line[actual].substr(head));
    line[actual].resize(token);
    line[actual].append(word);
    line[actual].append(remainder);
    head = token;
    return 1;
  }
  return 0;
}

void TextParser::check_urls() {
  urlline.resize(line[actual].size() + 1);
  int url_state = 0;
  size_t url_head = 0;
  size_t url_token = 0;
  int url = 0;
  for (;;) {
    switch (url_state) {
      case 0:  // non word chars
        if (is_wordchar(line[actual].c_str() + url_head)) {
          url_state = 1;
          url_token = url_head;
          // Unix path
        } else if (line[actual][url_head] == '/') {
          url_state = 1;
          url_token = url_head;
          url = 1;
        }
        break;
      case 1:  // wordchar
        char ch = line[actual][url_head];
        // e-mail address
        if ((ch == '@') ||
            // MS-DOS, Windows path
            (strncmp(line[actual].c_str() + url_head, ":\\", 2) == 0) ||
            // URL
            (strncmp(line[actual].c_str() + url_head, "://", 3) == 0)) {
          url = 1;
        } else if (!(is_wordchar(line[actual].c_str() + url_head) || (ch == '-') ||
                     (ch == '_') || (ch == '\\') || (ch == '.') ||
                     (ch == ':') || (ch == '/') || (ch == '~') || (ch == '%') ||
                     (ch == '*') || (ch == '$') || (ch == '[') || (ch == ']') ||
                     (ch == '?') || (ch == '!') ||
                     ((ch >= '0') && (ch <= '9')))) {
          url_state = 0;
          if (url == 1) {
            for (size_t i = url_token; i < url_head; ++i) {
              urlline[i] = true;
            }
          }
          url = 0;
        }
        break;
    }
    urlline[url_head] = false;
    if (next_char(line[actual].c_str(), &url_head))
      return;
  }
}

int TextParser::get_url(size_t token_pos, size_t* hd) {
  for (size_t i = *hd; i < line[actual].size() && urlline[i]; i++, (*hd)++)
    ;
  return checkurl ? 0 : urlline[token_pos];
}

void TextParser::set_url_checking(int check) {
  checkurl = check;
}

bool TextParser::alloc_token(size_t tokn, size_t* hd, std::string& t) {
  size_t url_head = *hd;
  if (get_url(tokn, &url_head))
    return false;
  t = line[actual].substr(tokn, *hd - tokn);
  // remove colon for Finnish and Swedish language
  if (!t.empty() && t[t.size() - 1] == ':') {
    t.resize(t.size() - 1);
    if (t.empty()) {
      return false;
    }
  }
  return true;
}

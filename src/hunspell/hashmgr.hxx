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

#ifndef HASHMGR_HXX_
#define HASHMGR_HXX_

#include <stdio.h>
#include <string>
#include <vector>

#include "htypes.hxx"
#include "filemgr.hxx"
#include "w_char.hxx"

enum flag { FLAG_CHAR, FLAG_LONG, FLAG_NUM, FLAG_UNI };

class HashMgr {
  int tablesize;
  struct hentry** tableptr;
  flag flag_mode;
  int complexprefixes;
  int utf8;
  unsigned short forbiddenword;
  int langnum;
  std::string enc;
  std::string lang;
  struct cs_info* csconv;
  std::string ignorechars;
  std::vector<w_char> ignorechars_utf16;
  int numaliasf;  // flag vector `compression' with aliases
  unsigned short** aliasf;
  unsigned short* aliasflen;
  int numaliasm;  // morphological desciption `compression' with aliases
  char** aliasm;

 public:
  HashMgr(const char* tpath, const char* apath, const char* key = NULL);
  ~HashMgr();

  struct hentry* lookup(const char*) const;
  int hash(const char*) const;
  struct hentry* walk_hashtable(int& col, struct hentry* hp) const;

  int add(const std::string& word);
  int add_with_affix(const std::string& word, const std::string& pattern);
  int remove(const std::string& word);
  int decode_flags(unsigned short** result, const std::string& flags, FileMgr* af) const;
  bool decode_flags(std::vector<unsigned short>& result, const std::string& flags, FileMgr* af) const;
  unsigned short decode_flag(const char* flag) const;
  char* encode_flag(unsigned short flag) const;
  int is_aliasf() const;
  int get_aliasf(int index, unsigned short** fvec, FileMgr* af) const;
  int is_aliasm() const;
  char* get_aliasm(int index) const;

 private:
  int get_clen_and_captype(const std::string& word, int* captype);
  int get_clen_and_captype(const std::string& word, int* captype, std::vector<w_char> &workbuf);
  int load_tables(const char* tpath, const char* key);
  int add_word(const std::string& word,
               int wcl,
               unsigned short* ap,
               int al,
               const std::string* desc,
               bool onlyupcase);
  int load_config(const char* affpath, const char* key);
  bool parse_aliasf(const std::string& line, FileMgr* af);
  int add_hidden_capitalized_word(const std::string& word,
                                  int wcl,
                                  unsigned short* flags,
                                  int al,
                                  const std::string* dp,
                                  int captype);
  bool parse_aliasm(const std::string& line, FileMgr* af);
  int remove_forbidden_flag(const std::string& word);
};

#endif

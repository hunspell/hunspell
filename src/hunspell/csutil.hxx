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

#ifndef CSUTIL_HXX_
#define CSUTIL_HXX_

#include "hunvisapi.h"

// First some base level utility routines

#include <fstream>
#include <string>
#include <vector>
#include <string.h>
#include "w_char.hxx"
#include "htypes.hxx"

#ifdef MOZILLA_CLIENT
#include "nscore.h"  // for mozalloc headers
#endif

// casing
#define NOCAP 0
#define INITCAP 1
#define ALLCAP 2
#define HUHCAP 3
#define HUHINITCAP 4

// default encoding and keystring
#define SPELL_ENCODING "ISO8859-1"
#define SPELL_KEYSTRING "qwertyuiop|asdfghjkl|zxcvbnm"

// default morphological fields
#define MORPH_STEM "st:"
#define MORPH_ALLOMORPH "al:"
#define MORPH_POS "po:"
#define MORPH_DERI_PFX "dp:"
#define MORPH_INFL_PFX "ip:"
#define MORPH_TERM_PFX "tp:"
#define MORPH_DERI_SFX "ds:"
#define MORPH_INFL_SFX "is:"
#define MORPH_TERM_SFX "ts:"
#define MORPH_SURF_PFX "sp:"
#define MORPH_FREQ "fr:"
#define MORPH_PHON "ph:"
#define MORPH_HYPH "hy:"
#define MORPH_PART "pa:"
#define MORPH_FLAG "fl:"
#define MORPH_HENTRY "_H:"
#define MORPH_TAG_LEN strlen(MORPH_STEM)

#define MSEP_FLD ' '
#define MSEP_REC '\n'
#define MSEP_ALT '\v'

// default flags
#define DEFAULTFLAGS 65510
#define FORBIDDENWORD 65510
#define ONLYUPCASEFLAG 65511

// fix long pathname problem of WIN32 by using w_char std::fstream::open override
LIBHUNSPELL_DLL_EXPORTED void myopen(std::ifstream& stream, const char* path,
                                     std::ios_base::openmode mode);

// convert UTF-16 characters to UTF-8
LIBHUNSPELL_DLL_EXPORTED std::string& u16_u8(std::string& dest,
                                             const std::vector<w_char>& src);

// convert UTF-8 characters to UTF-16
LIBHUNSPELL_DLL_EXPORTED int u8_u16(std::vector<w_char>& dest,
                                    const std::string& src);

// remove end of line char(s)
LIBHUNSPELL_DLL_EXPORTED void mychomp(std::string& s);

// duplicate string
LIBHUNSPELL_DLL_EXPORTED char* mystrdup(const char* s);

// parse into tokens with char delimiter
LIBHUNSPELL_DLL_EXPORTED std::string::const_iterator mystrsep(const std::string &str,
                                                              std::string::const_iterator& start);

// replace pat by rep in word and return word
LIBHUNSPELL_DLL_EXPORTED std::string& mystrrep(std::string& str,
                                               const std::string& search,
                                               const std::string& replace);

// append s to ends of every lines in text
LIBHUNSPELL_DLL_EXPORTED std::string& strlinecat(std::string& str,
                                                 const std::string& apd);

// tokenize into lines with new line
LIBHUNSPELL_DLL_EXPORTED std::vector<std::string> line_tok(const std::string& text,
                                                           char breakchar);

// tokenize into lines with new line and uniq in place
LIBHUNSPELL_DLL_EXPORTED void line_uniq(std::string& text, char breakchar);

LIBHUNSPELL_DLL_EXPORTED void line_uniq_app(std::string& text, char breakchar);

// reverse word
LIBHUNSPELL_DLL_EXPORTED size_t reverseword(std::string& word);

// reverse word
LIBHUNSPELL_DLL_EXPORTED size_t reverseword_utf(std::string&);

// remove duplicates
LIBHUNSPELL_DLL_EXPORTED void uniqlist(std::vector<std::string>& list);

// character encoding information
struct cs_info {
  unsigned char ccase;
  unsigned char clower;
  unsigned char cupper;
};

LIBHUNSPELL_DLL_EXPORTED void initialize_utf_tbl();
LIBHUNSPELL_DLL_EXPORTED void free_utf_tbl();
LIBHUNSPELL_DLL_EXPORTED unsigned short unicodetoupper(unsigned short c,
                                                       int langnum);
LIBHUNSPELL_DLL_EXPORTED w_char upper_utf(w_char u, int langnum);
LIBHUNSPELL_DLL_EXPORTED w_char lower_utf(w_char u, int langnum);
LIBHUNSPELL_DLL_EXPORTED unsigned short unicodetolower(unsigned short c,
                                                       int langnum);
LIBHUNSPELL_DLL_EXPORTED int unicodeisalpha(unsigned short c);

LIBHUNSPELL_DLL_EXPORTED struct cs_info* get_current_cs(const std::string& es);

// get language identifiers of language codes
LIBHUNSPELL_DLL_EXPORTED int get_lang_num(const std::string& lang);

// get characters of the given 8bit encoding with lower- and uppercase forms
LIBHUNSPELL_DLL_EXPORTED std::string get_casechars(const char* enc);

// convert std::string to all caps
LIBHUNSPELL_DLL_EXPORTED std::string& mkallcap(std::string& s,
                                               const struct cs_info* csconv);

// convert null terminated string to all little
LIBHUNSPELL_DLL_EXPORTED std::string& mkallsmall(std::string& s,
                                                 const struct cs_info* csconv);

// convert first letter of string to little
LIBHUNSPELL_DLL_EXPORTED std::string& mkinitsmall(std::string& s,
                                                 const struct cs_info* csconv);

// convert first letter of string to capital
LIBHUNSPELL_DLL_EXPORTED std::string& mkinitcap(std::string& s,
                                                const struct cs_info* csconv);

// convert first letter of UTF-8 string to capital
LIBHUNSPELL_DLL_EXPORTED std::vector<w_char>&
mkinitcap_utf(std::vector<w_char>& u, int langnum);

// convert UTF-8 string to little
LIBHUNSPELL_DLL_EXPORTED std::vector<w_char>&
mkallsmall_utf(std::vector<w_char>& u, int langnum);

// convert first letter of UTF-8 string to little
LIBHUNSPELL_DLL_EXPORTED std::vector<w_char>&
mkinitsmall_utf(std::vector<w_char>& u, int langnum);

// convert UTF-8 string to capital
LIBHUNSPELL_DLL_EXPORTED std::vector<w_char>&
mkallcap_utf(std::vector<w_char>& u, int langnum);

// get type of capitalization
LIBHUNSPELL_DLL_EXPORTED int get_captype(const std::string& q, cs_info*);

// get type of capitalization (UTF-8)
LIBHUNSPELL_DLL_EXPORTED int get_captype_utf8(const std::vector<w_char>& q, int langnum);

// strip all ignored characters in the string
LIBHUNSPELL_DLL_EXPORTED size_t remove_ignored_chars_utf(
    std::string& word,
    const std::vector<w_char>& ignored_chars);

// strip all ignored characters in the string
LIBHUNSPELL_DLL_EXPORTED size_t remove_ignored_chars(
    std::string& word,
    const std::string& ignored_chars);

LIBHUNSPELL_DLL_EXPORTED bool parse_string(const std::string& line,
                                           std::string& out,
                                           int ln);

LIBHUNSPELL_DLL_EXPORTED bool parse_array(const std::string& line,
                                          std::string& out,
                                          std::vector<w_char>& out_utf16,
                                          int utf8,
                                          int ln);

LIBHUNSPELL_DLL_EXPORTED int fieldlen(const char* r);

LIBHUNSPELL_DLL_EXPORTED bool copy_field(std::string& dest,
                                         const std::string& morph,
                                         const std::string& var);

// conversion function for protected memory
LIBHUNSPELL_DLL_EXPORTED void store_pointer(char* dest, char* source);

// conversion function for protected memory
LIBHUNSPELL_DLL_EXPORTED char* get_stored_pointer(const char* s);

// hash entry macros
LIBHUNSPELL_DLL_EXPORTED inline char* HENTRY_DATA(struct hentry* h) {
  char* ret;
  if (!h->var)
    ret = NULL;
  else if (h->var & H_OPT_ALIASM)
    ret = get_stored_pointer(HENTRY_WORD(h) + h->blen + 1);
  else
    ret = HENTRY_WORD(h) + h->blen + 1;
  return ret;
}

LIBHUNSPELL_DLL_EXPORTED inline const char* HENTRY_DATA(
    const struct hentry* h) {
  const char* ret;
  if (!h->var)
    ret = NULL;
  else if (h->var & H_OPT_ALIASM)
    ret = get_stored_pointer(HENTRY_WORD(h) + h->blen + 1);
  else
    ret = HENTRY_WORD(h) + h->blen + 1;
  return ret;
}

// NULL-free version for warning-free OOo build
LIBHUNSPELL_DLL_EXPORTED inline const char* HENTRY_DATA2(
    const struct hentry* h) {
  const char* ret;
  if (!h->var)
    ret = "";
  else if (h->var & H_OPT_ALIASM)
    ret = get_stored_pointer(HENTRY_WORD(h) + h->blen + 1);
  else
    ret = HENTRY_WORD(h) + h->blen + 1;
  return ret;
}

LIBHUNSPELL_DLL_EXPORTED inline char* HENTRY_FIND(struct hentry* h,
                                                  const char* p) {
  return (HENTRY_DATA(h) ? strstr(HENTRY_DATA(h), p) : NULL);
}

#endif

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

#ifndef AFFIXMGR_HXX_
#define AFFIXMGR_HXX_

#include <stdio.h>

#include <string>
#include <vector>

#include "atypes.hxx"
#include "baseaffix.hxx"
#include "hashmgr.hxx"
#include "phonet.hxx"
#include "replist.hxx"

// check flag duplication
#define dupSFX (1 << 0)
#define dupPFX (1 << 1)

class PfxEntry;
class SfxEntry;

class AffixMgr {
  PfxEntry* pStart[SETSIZE];
  SfxEntry* sStart[SETSIZE];
  PfxEntry* pFlag[SETSIZE];
  SfxEntry* sFlag[SETSIZE];
  const std::vector<HashMgr*>& alldic;
  const HashMgr* pHMgr;
  std::string keystring;
  std::string trystring;
  std::string encoding;
  struct cs_info* csconv;
  int utf8;
  int complexprefixes;
  FLAG compoundflag;
  FLAG compoundbegin;
  FLAG compoundmiddle;
  FLAG compoundend;
  FLAG compoundroot;
  FLAG compoundforbidflag;
  FLAG compoundpermitflag;
  int compoundmoresuffixes;
  int checkcompounddup;
  int checkcompoundrep;
  int checkcompoundcase;
  int checkcompoundtriple;
  int simplifiedtriple;
  FLAG forbiddenword;
  FLAG nosuggest;
  FLAG nongramsuggest;
  FLAG needaffix;
  int cpdmin;
  bool parsedrep;
  std::vector<replentry> reptable;
  RepList* iconvtable;
  RepList* oconvtable;
  bool parsedmaptable;
  std::vector<mapentry> maptable;
  bool parsedbreaktable;
  std::vector<std::string> breaktable;
  bool parsedcheckcpd;
  std::vector<patentry> checkcpdtable;
  int simplifiedcpd;
  bool parseddefcpd;
  std::vector<flagentry> defcpdtable;
  phonetable* phone;
  int maxngramsugs;
  int maxcpdsugs;
  int maxdiff;
  int onlymaxdiff;
  int nosplitsugs;
  int sugswithdots;
  int cpdwordmax;
  int cpdmaxsyllable;
  std::string cpdvowels; // vowels (for calculating of Hungarian compounding limit,
  std::vector<w_char> cpdvowels_utf16; //vowels for UTF-8 encoding
  std::string cpdsyllablenum; // syllable count incrementing flag
  const char* pfxappnd;  // BUG: not stateless
  const char* sfxappnd;  // BUG: not stateless
  int sfxextra;          // BUG: not stateless
  FLAG sfxflag;          // BUG: not stateless
  char* derived;         // BUG: not stateless
  SfxEntry* sfx;         // BUG: not stateless
  PfxEntry* pfx;         // BUG: not stateless
  int checknum;
  std::string wordchars; // letters + spec. word characters
  std::vector<w_char> wordchars_utf16;
  std::string ignorechars; // letters + spec. word characters
  std::vector<w_char> ignorechars_utf16;
  std::string version;   // affix and dictionary file version string
  std::string lang;	 // language
  int langnum;
  FLAG lemma_present;
  FLAG circumfix;
  FLAG onlyincompound;
  FLAG keepcase;
  FLAG forceucase;
  FLAG warn;
  int forbidwarn;
  FLAG substandard;
  int checksharps;
  int fullstrip;

  int havecontclass;           // boolean variable
  char contclasses[CONTSIZE];  // flags of possible continuing classes (twofold
                               // affix)

 public:
  AffixMgr(const char* affpath, const std::vector<HashMgr*>& ptr, const char* key = NULL);
  ~AffixMgr();
  struct hentry* affix_check(const char* word,
                             int len,
                             const unsigned short needflag = (unsigned short)0,
                             char in_compound = IN_CPD_NOT);
  struct hentry* prefix_check(const char* word,
                              int len,
                              char in_compound,
                              const FLAG needflag = FLAG_NULL);
  inline int isSubset(const char* s1, const char* s2);
  struct hentry* prefix_check_twosfx(const char* word,
                                     int len,
                                     char in_compound,
                                     const FLAG needflag = FLAG_NULL);
  inline int isRevSubset(const char* s1, const char* end_of_s2, int len);
  struct hentry* suffix_check(const char* word,
                              int len,
                              int sfxopts,
                              PfxEntry* ppfx,
                              const FLAG cclass = FLAG_NULL,
                              const FLAG needflag = FLAG_NULL,
                              char in_compound = IN_CPD_NOT);
  struct hentry* suffix_check_twosfx(const char* word,
                                     int len,
                                     int sfxopts,
                                     PfxEntry* ppfx,
                                     const FLAG needflag = FLAG_NULL);

  std::string affix_check_morph(const char* word,
                                int len,
                                const FLAG needflag = FLAG_NULL,
                                char in_compound = IN_CPD_NOT);
  std::string prefix_check_morph(const char* word,
                                 int len,
                                 char in_compound,
                                 const FLAG needflag = FLAG_NULL);
  std::string suffix_check_morph(const char* word,
                                 int len,
                                 int sfxopts,
                                 PfxEntry* ppfx,
                                 const FLAG cclass = FLAG_NULL,
                                 const FLAG needflag = FLAG_NULL,
                                 char in_compound = IN_CPD_NOT);

  std::string prefix_check_twosfx_morph(const char* word,
                                        int len,
                                        char in_compound,
                                        const FLAG needflag = FLAG_NULL);
  std::string suffix_check_twosfx_morph(const char* word,
                                        int len,
                                        int sfxopts,
                                        PfxEntry* ppfx,
                                        const FLAG needflag = FLAG_NULL);

  std::string morphgen(const char* ts,
                       int wl,
                       const unsigned short* ap,
                       unsigned short al,
                       const char* morph,
                       const char* targetmorph,
                       int level);

  int expand_rootword(struct guessword* wlst,
                      int maxn,
                      const char* ts,
                      int wl,
                      const unsigned short* ap,
                      unsigned short al,
                      const char* bad,
                      int,
                      const char*);

  short get_syllable(const std::string& word);
  int cpdrep_check(const char* word, int len);
  int cpdpat_check(const char* word,
                   int len,
                   hentry* r1,
                   hentry* r2,
                   const char affixed);
  int defcpd_check(hentry*** words,
                   short wnum,
                   hentry* rv,
                   hentry** rwords,
                   char all);
  int cpdcase_check(const char* word, int len);
  inline int candidate_check(const char* word, int len);
  void setcminmax(int* cmin, int* cmax, const char* word, int len);
  struct hentry* compound_check(const std::string& word,
                                short wordnum,
                                short numsyllable,
                                short maxwordnum,
                                short wnum,
                                hentry** words,
                                hentry** rwords,
                                char hu_mov_rule,
                                char is_sug,
                                int* info);

  int compound_check_morph(const char* word,
                           int len,
                           short wordnum,
                           short numsyllable,
                           short maxwordnum,
                           short wnum,
                           hentry** words,
                           hentry** rwords,
                           char hu_mov_rule,
                           std::string& result,
                           const std::string* partresult);

  std::vector<std::string> get_suffix_words(short unsigned* suff,
                       int len,
                       const char* root_word);

  struct hentry* lookup(const char* word);
  const std::vector<replentry>& get_reptable() const;
  RepList* get_iconvtable() const;
  RepList* get_oconvtable() const;
  struct phonetable* get_phonetable() const;
  const std::vector<mapentry>& get_maptable() const;
  const std::vector<std::string>& get_breaktable() const;
  const std::string& get_encoding();
  int get_langnum() const;
  char* get_key_string();
  char* get_try_string() const;
  const std::string& get_wordchars() const;
  const std::vector<w_char>& get_wordchars_utf16() const;
  const char* get_ignore() const;
  const std::vector<w_char>& get_ignore_utf16() const;
  int get_compound() const;
  FLAG get_compoundflag() const;
  FLAG get_forbiddenword() const;
  FLAG get_nosuggest() const;
  FLAG get_nongramsuggest() const;
  FLAG get_needaffix() const;
  FLAG get_onlyincompound() const;
  const char* get_derived() const;
  const std::string& get_version() const;
  int have_contclass() const;
  int get_utf8() const;
  int get_complexprefixes() const;
  char* get_suffixed(char) const;
  int get_maxngramsugs() const;
  int get_maxcpdsugs() const;
  int get_maxdiff() const;
  int get_onlymaxdiff() const;
  int get_nosplitsugs() const;
  int get_sugswithdots(void) const;
  FLAG get_keepcase(void) const;
  FLAG get_forceucase(void) const;
  FLAG get_warn(void) const;
  int get_forbidwarn(void) const;
  int get_checksharps(void) const;
  char* encode_flag(unsigned short aflag) const;
  int get_fullstrip() const;

 private:
  int parse_file(const char* affpath, const char* key);
  bool parse_flag(const std::string& line, unsigned short* out, FileMgr* af);
  bool parse_num(const std::string& line, int* out, FileMgr* af);
  bool parse_cpdsyllable(const std::string& line, FileMgr* af);
  bool parse_reptable(const std::string& line, FileMgr* af);
  bool parse_convtable(const std::string& line,
                      FileMgr* af,
                      RepList** rl,
                      const std::string& keyword);
  bool parse_phonetable(const std::string& line, FileMgr* af);
  bool parse_maptable(const std::string& line, FileMgr* af);
  bool parse_breaktable(const std::string& line, FileMgr* af);
  bool parse_checkcpdtable(const std::string& line, FileMgr* af);
  bool parse_defcpdtable(const std::string& line, FileMgr* af);
  bool parse_affix(const std::string& line, const char at, FileMgr* af, char* dupflags);

  void reverse_condition(std::string&);
  std::string& debugflag(std::string& result, unsigned short flag);
  int condlen(const char*);
  int encodeit(AffEntry& entry, const char* cs);
  int build_pfxtree(PfxEntry* pfxptr);
  int build_sfxtree(SfxEntry* sfxptr);
  int process_pfx_order();
  int process_sfx_order();
  PfxEntry* process_pfx_in_order(PfxEntry* ptr, PfxEntry* nptr);
  SfxEntry* process_sfx_in_order(SfxEntry* ptr, SfxEntry* nptr);
  int process_pfx_tree_to_list();
  int process_sfx_tree_to_list();
  int redundant_condition(char, const char* strip, int stripl, const char* cond, int);
  void finishFileMgr(FileMgr* afflst);
};

#endif

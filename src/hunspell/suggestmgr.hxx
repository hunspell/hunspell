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

#ifndef SUGGESTMGR_HXX_
#define SUGGESTMGR_HXX_

#define MAX_ROOTS 100
#define MAX_WORDS 100
#define MAX_GUESS 200
#define MAXNGRAMSUGS 4
#define MAXPHONSUGS 2
#define MAXCOMPOUNDSUGS 3

// timelimit: max ~1/4 sec (process time on Linux) for a time consuming function
#define TIMELIMIT (CLOCKS_PER_SEC / 4)
#define MINTIMER 100
#define MAXPLUSTIMER 100

#define NGRAM_LONGER_WORSE (1 << 0)
#define NGRAM_ANY_MISMATCH (1 << 1)
#define NGRAM_LOWERING (1 << 2)
#define NGRAM_WEIGHTED (1 << 3)

#include "atypes.hxx"
#include "affixmgr.hxx"
#include "hashmgr.hxx"
#include "langnum.hxx"
#include <time.h>

enum { LCS_UP, LCS_LEFT, LCS_UPLEFT };

class SuggestMgr {
 private:
  SuggestMgr(const SuggestMgr&);
  SuggestMgr& operator=(const SuggestMgr&);

 private:
  char* ckey;
  size_t ckeyl;
  std::vector<w_char> ckey_utf;

  char* ctry;
  size_t ctryl;
  std::vector<w_char> ctry_utf;

  AffixMgr* pAMgr;
  unsigned int maxSug;
  struct cs_info* csconv;
  int utf8;
  int langnum;
  int nosplitsugs;
  int maxngramsugs;
  int maxcpdsugs;
  int complexprefixes;

 public:
  SuggestMgr(const char* tryme, unsigned int maxn, AffixMgr* aptr);
  ~SuggestMgr();

  void suggest(std::vector<std::string>& slst, const char* word, int* onlycmpdsug);
  void ngsuggest(std::vector<std::string>& slst, const char* word, const std::vector<HashMgr*>& rHMgr);

  std::string suggest_morph(const std::string& word);
  std::string suggest_gen(const std::vector<std::string>& pl, const std::string& pattern);

 private:
  void testsug(std::vector<std::string>& wlst,
               const std::string& candidate,
               int cpdsuggest,
               int* timer,
               clock_t* timelimit);
  int checkword(const std::string& word, int, int*, clock_t*);
  int check_forbidden(const char*, int);

  void capchars(std::vector<std::string>&, const char*, int);
  int replchars(std::vector<std::string>&, const char*, int);
  int doubletwochars(std::vector<std::string>&, const char*, int);
  int forgotchar(std::vector<std::string>&, const char*, int);
  int swapchar(std::vector<std::string>&, const char*, int);
  int longswapchar(std::vector<std::string>&, const char*, int);
  int movechar(std::vector<std::string>&, const char*, int);
  int extrachar(std::vector<std::string>&, const char*, int);
  int badcharkey(std::vector<std::string>&, const char*, int);
  int badchar(std::vector<std::string>&, const char*, int);
  int twowords(std::vector<std::string>&, const char*, int);

  void capchars_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int doubletwochars_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int forgotchar_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int extrachar_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int badcharkey_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int badchar_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int swapchar_utf(std::vector<std::string>&, const w_char*, int wl, int);
  int longswapchar_utf(std::vector<std::string>&, const w_char*, int, int);
  int movechar_utf(std::vector<std::string>&, const w_char*, int, int);

  int mapchars(std::vector<std::string>&, const char*, int);
  int map_related(const char*,
                  std::string&,
                  int,
                  std::vector<std::string>& wlst,
                  int,
                  const std::vector<mapentry>&,
                  int*,
                  clock_t*);
  int ngram(int n, const std::vector<w_char>& su1,
            const std::vector<w_char>& su2, int opt);
  int ngram(int n, const std::string& s1, const std::string& s2, int opt);
  int mystrlen(const char* word);
  int leftcommonsubstring(const std::vector<w_char>& su1,
                          const std::vector<w_char>& su2);
  int leftcommonsubstring(const char* s1, const char* s2);
  int commoncharacterpositions(const char* s1, const char* s2, int* is_swap);
  void bubblesort(char** rwd, char** rwd2, int* rsc, int n);
  void lcs(const char* s, const char* s2, int* l1, int* l2, char** result);
  int lcslen(const char* s, const char* s2);
  int lcslen(const std::string& s, const std::string& s2);
  std::string suggest_hentry_gen(hentry* rv, const char* pattern);
};

#endif

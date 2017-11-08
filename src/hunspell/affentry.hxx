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

#ifndef AFFIX_HXX_
#define AFFIX_HXX_

#include "atypes.hxx"
#include "baseaffix.hxx"
#include "affixmgr.hxx"

/* A Prefix Entry  */

class PfxEntry : public AffEntry {
 private:
  PfxEntry(const PfxEntry&);
  PfxEntry& operator=(const PfxEntry&);

 private:
  AffixMgr* pmyMgr;

  PfxEntry* next;
  PfxEntry* nexteq;
  PfxEntry* nextne;
  PfxEntry* flgnxt;

 public:
  explicit PfxEntry(AffixMgr* pmgr);

  bool allowCross() const { return ((opts & aeXPRODUCT) != 0); }
  struct hentry* checkword(const char* word,
                           int len,
                           char in_compound,
                           const FLAG needflag = FLAG_NULL);

  struct hentry* check_twosfx(const char* word,
                              int len,
                              char in_compound,
                              const FLAG needflag = FLAG_NULL);

  std::string check_morph(const char* word,
                          int len,
                          char in_compound,
                          const FLAG needflag = FLAG_NULL);

  std::string check_twosfx_morph(const char* word,
                                 int len,
                                 char in_compound,
                                 const FLAG needflag = FLAG_NULL);

  FLAG getFlag() { return aflag; }
  const char* getKey() { return appnd.c_str(); }
  std::string add(const char* word, size_t len);

  inline short getKeyLen() { return appnd.size(); }

  inline const char* getMorph() { return morphcode; }

  inline const unsigned short* getCont() { return contclass; }
  inline short getContLen() { return contclasslen; }

  inline PfxEntry* getNext() { return next; }
  inline PfxEntry* getNextNE() { return nextne; }
  inline PfxEntry* getNextEQ() { return nexteq; }
  inline PfxEntry* getFlgNxt() { return flgnxt; }

  inline void setNext(PfxEntry* ptr) { next = ptr; }
  inline void setNextNE(PfxEntry* ptr) { nextne = ptr; }
  inline void setNextEQ(PfxEntry* ptr) { nexteq = ptr; }
  inline void setFlgNxt(PfxEntry* ptr) { flgnxt = ptr; }

  inline char* nextchar(char* p);
  inline int test_condition(const char* st);
};

/* A Suffix Entry */

class SfxEntry : public AffEntry {
 private:
  SfxEntry(const SfxEntry&);
  SfxEntry& operator=(const SfxEntry&);

 private:
  AffixMgr* pmyMgr;
  std::string rappnd;

  SfxEntry* next;
  SfxEntry* nexteq;
  SfxEntry* nextne;
  SfxEntry* flgnxt;

  SfxEntry* l_morph;
  SfxEntry* r_morph;
  SfxEntry* eq_morph;

 public:
  explicit SfxEntry(AffixMgr* pmgr);

  bool allowCross() const { return ((opts & aeXPRODUCT) != 0); }
  struct hentry* checkword(const char* word,
                           int len,
                           int optflags,
                           PfxEntry* ppfx,
                           const FLAG cclass,
                           const FLAG needflag,
                           const FLAG badflag);

  struct hentry* check_twosfx(const char* word,
                              int len,
                              int optflags,
                              PfxEntry* ppfx,
                              const FLAG needflag = FLAG_NULL);

  std::string check_twosfx_morph(const char* word,
                                 int len,
                                 int optflags,
                                 PfxEntry* ppfx,
                                 const FLAG needflag = FLAG_NULL);
  struct hentry* get_next_homonym(struct hentry* he);
  struct hentry* get_next_homonym(struct hentry* word,
                                  int optflags,
                                  PfxEntry* ppfx,
                                  const FLAG cclass,
                                  const FLAG needflag);

  FLAG getFlag() { return aflag; }
  const char* getKey() { return rappnd.c_str(); }
  std::string add(const char* word, size_t len);

  inline const char* getMorph() { return morphcode; }

  inline const unsigned short* getCont() { return contclass; }
  inline short getContLen() { return contclasslen; }
  inline const char* getAffix() { return appnd.c_str(); }

  inline short getKeyLen() { return appnd.size(); }

  inline SfxEntry* getNext() { return next; }
  inline SfxEntry* getNextNE() { return nextne; }
  inline SfxEntry* getNextEQ() { return nexteq; }

  inline SfxEntry* getLM() { return l_morph; }
  inline SfxEntry* getRM() { return r_morph; }
  inline SfxEntry* getEQM() { return eq_morph; }
  inline SfxEntry* getFlgNxt() { return flgnxt; }

  inline void setNext(SfxEntry* ptr) { next = ptr; }
  inline void setNextNE(SfxEntry* ptr) { nextne = ptr; }
  inline void setNextEQ(SfxEntry* ptr) { nexteq = ptr; }
  inline void setFlgNxt(SfxEntry* ptr) { flgnxt = ptr; }
  void initReverseWord();

  inline char* nextchar(char* p);
  inline int test_condition(const char* st, const char* begin);
};

#endif

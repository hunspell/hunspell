/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 *
 * Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
 * Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
 * Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
 * Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
 * Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/*
 * Copyright 2002 Kevin B. Hendricks, Stratford, Ontario, Canada
 * And Contributors.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY KEVIN B. HENDRICKS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * KEVIN B. HENDRICKS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hunspell.hxx"
#include "hunspell.h"
#ifndef MOZILLA_CLIENT
#include "config.h"
#endif
#include "csutil.hxx"

#include <limits>
#include <string>

Hunspell::Hunspell(const char* affpath, const char* dpath, const char* key) {
  encoding = NULL;
  csconv = NULL;
  utf8 = 0;
  complexprefixes = 0;
  affixpath = mystrdup(affpath);
  maxdic = 0;

  /* first set up the hash manager */
  pHMgr[0] = new HashMgr(dpath, affpath, key);
  if (pHMgr[0])
    maxdic = 1;

  /* next set up the affix manager */
  /* it needs access to the hash manager lookup methods */
  pAMgr = new AffixMgr(affpath, pHMgr, &maxdic, key);

  /* get the preferred try string and the dictionary */
  /* encoding from the Affix Manager for that dictionary */
  char* try_string = pAMgr->get_try_string();
  encoding = pAMgr->get_encoding();
  langnum = pAMgr->get_langnum();
  utf8 = pAMgr->get_utf8();
  if (!utf8)
    csconv = get_current_cs(encoding);
  complexprefixes = pAMgr->get_complexprefixes();
  wordbreak = pAMgr->get_breaktable();

  /* and finally set up the suggestion manager */
  pSMgr = new SuggestMgr(try_string, MAXSUGGESTION, pAMgr);
  if (try_string)
    free(try_string);
}

Hunspell::~Hunspell() {
  delete pSMgr;
  delete pAMgr;
  for (int i = 0; i < maxdic; i++)
    delete pHMgr[i];
  maxdic = 0;
  pSMgr = NULL;
  pAMgr = NULL;
#ifdef MOZILLA_CLIENT
  delete[] csconv;
#endif
  csconv = NULL;
  if (encoding)
    free(encoding);
  encoding = NULL;
  if (affixpath)
    free(affixpath);
  affixpath = NULL;
}

// load extra dictionaries
int Hunspell::add_dic(const char* dpath, const char* key) {
  if (maxdic == MAXDIC || !affixpath)
    return 1;
  pHMgr[maxdic] = new HashMgr(dpath, affixpath, key);
  if (pHMgr[maxdic])
    maxdic++;
  else
    return 1;
  return 0;
}

// make a copy of src at destination while removing all leading
// blanks and removing any trailing periods after recording
// their presence with the abbreviation flag
// also since already going through character by character,
// set the capitalization type
// return the length of the "cleaned" (and UTF-8 encoded) word

int Hunspell::cleanword2(char* dest,
                         const char* src,
                         w_char* dest_utf,
                         int* nc,
                         int* pcaptype,
                         int* pabbrev) {
  unsigned char* p = (unsigned char*)dest;
  const unsigned char* q = (const unsigned char*)src;

  // first skip over any leading blanks
  while ((*q != '\0') && (*q == ' '))
    q++;

  // now strip off any trailing periods (recording their presence)
  *pabbrev = 0;
  int nl = strlen((const char*)q);
  while ((nl > 0) && (*(q + nl - 1) == '.')) {
    nl--;
    (*pabbrev)++;
  }

  // if no characters are left it can't be capitalized
  if (nl <= 0) {
    *pcaptype = NOCAP;
    *p = '\0';
    return 0;
  }

  strncpy(dest, (char*)q, nl);
  *(dest + nl) = '\0';
  nl = strlen(dest);
  if (utf8) {
    *nc = u8_u16(dest_utf, MAXWORDLEN, dest);
    // don't check too long words
    if (*nc >= MAXWORDLEN)
      return 0;
    if (*nc == -1) {  // big Unicode character (non BMP area)
      *pcaptype = NOCAP;
      return nl;
    }
    *pcaptype = get_captype_utf8(dest_utf, *nc, langnum);
  } else {
    *pcaptype = get_captype(dest, nl, csconv);
    *nc = nl;
  }
  return nl;
}

int Hunspell::cleanword(char* dest,
                        const char* src,
                        int* pcaptype,
                        int* pabbrev) {
  unsigned char* p = (unsigned char*)dest;
  const unsigned char* q = (const unsigned char*)src;
  int firstcap = 0;

  // first skip over any leading blanks
  while ((*q != '\0') && (*q == ' '))
    q++;

  // now strip off any trailing periods (recording their presence)
  *pabbrev = 0;
  int nl = strlen((const char*)q);
  while ((nl > 0) && (*(q + nl - 1) == '.')) {
    nl--;
    (*pabbrev)++;
  }

  // if no characters are left it can't be capitalized
  if (nl <= 0) {
    *pcaptype = NOCAP;
    *p = '\0';
    return 0;
  }

  // now determine the capitalization type of the first nl letters
  int ncap = 0;
  int nneutral = 0;
  int nc = 0;

  if (!utf8) {
    while (nl > 0) {
      nc++;
      if (csconv[(*q)].ccase)
        ncap++;
      if (csconv[(*q)].cupper == csconv[(*q)].clower)
        nneutral++;
      *p++ = *q++;
      nl--;
    }
    // remember to terminate the destination string
    *p = '\0';
    firstcap = csconv[(unsigned char)(*dest)].ccase;
  } else {
    unsigned short idx;
    w_char t[MAXWORDLEN];
    nc = u8_u16(t, MAXWORDLEN, src);
    for (int i = 0; i < nc; i++) {
      idx = (t[i].h << 8) + t[i].l;
      unsigned short low = unicodetolower(idx, langnum);
      if (idx != low)
        ncap++;
      if (unicodetoupper(idx, langnum) == low)
        nneutral++;
    }
    u16_u8(dest, MAXWORDUTF8LEN, t, nc);
    if (ncap) {
      idx = (t[0].h << 8) + t[0].l;
      firstcap = (idx != unicodetolower(idx, langnum));
    }
  }

  // now finally set the captype
  if (ncap == 0) {
    *pcaptype = NOCAP;
  } else if ((ncap == 1) && firstcap) {
    *pcaptype = INITCAP;
  } else if ((ncap == nc) || ((ncap + nneutral) == nc)) {
    *pcaptype = ALLCAP;
  } else if ((ncap > 1) && firstcap) {
    *pcaptype = HUHINITCAP;
  } else {
    *pcaptype = HUHCAP;
  }
  return strlen(dest);
}

void Hunspell::mkallcap(char* p) {
  if (utf8) {
    w_char u[MAXWORDLEN];
    int nc = u8_u16(u, MAXWORDLEN, p);
    unsigned short idx;
    for (int i = 0; i < nc; i++) {
      idx = (u[i].h << 8) + u[i].l;
      if (idx != unicodetoupper(idx, langnum)) {
        u[i].h = (unsigned char)(unicodetoupper(idx, langnum) >> 8);
        u[i].l = (unsigned char)(unicodetoupper(idx, langnum) & 0x00FF);
      }
    }
    u16_u8(p, MAXWORDUTF8LEN, u, nc);
  } else {
    while (*p != '\0') {
      *p = csconv[((unsigned char)*p)].cupper;
      p++;
    }
  }
}

int Hunspell::mkallcap2(char* p, w_char* u, int nc) {
  if (utf8) {
    unsigned short idx;
    for (int i = 0; i < nc; i++) {
      idx = (u[i].h << 8) + u[i].l;
      unsigned short up = unicodetoupper(idx, langnum);
      if (idx != up) {
        u[i].h = (unsigned char)(up >> 8);
        u[i].l = (unsigned char)(up & 0x00FF);
      }
    }
    u16_u8(p, MAXWORDUTF8LEN, u, nc);
    return strlen(p);
  } else {
    while (*p != '\0') {
      *p = csconv[((unsigned char)*p)].cupper;
      p++;
    }
  }
  return nc;
}

void Hunspell::mkallsmall(char* p) {
  while (*p != '\0') {
    *p = csconv[((unsigned char)*p)].clower;
    p++;
  }
}

int Hunspell::mkallsmall2(char* p, w_char* u, int nc) {
  if (utf8) {
    unsigned short idx;
    for (int i = 0; i < nc; i++) {
      idx = (u[i].h << 8) + u[i].l;
      unsigned short low = unicodetolower(idx, langnum);
      if (idx != low) {
        u[i].h = (unsigned char)(low >> 8);
        u[i].l = (unsigned char)(low & 0x00FF);
      }
    }
    u16_u8(p, MAXWORDUTF8LEN, u, nc);
    return strlen(p);
  } else {
    while (*p != '\0') {
      *p = csconv[((unsigned char)*p)].clower;
      p++;
    }
  }
  return nc;
}

int Hunspell::mkallsmall2(std::string& u8, std::vector<w_char>& u16) {
  if (utf8) {
    ::mkallsmall_utf(u16, langnum);
    u16_u8(u8, u16);
  } else {
    ::mkallsmall(u8, csconv);
  }
  return u8.size();
}

// convert UTF-8 sharp S codes to latin 1
std::string Hunspell::sharps_u8_l1(const std::string& source) {
  std::string dest(source);
  mystrrep(dest, "\xC3\x9F", "\xDF");
  return dest;
}

// recursive search for right ss - sharp s permutations
hentry* Hunspell::spellsharps(std::string& base,
                              size_t n_pos,
                              int n,
                              int repnum,
                              int* info,
                              char** root) {
  size_t pos = base.find("ss", n_pos);
  if (pos != std::string::npos && (n < MAXSHARPS)) {
    base[pos] = '\xC3';
    base[pos + 1] = '\x9F';
    hentry* h = spellsharps(base, pos + 2, n + 1, repnum + 1, info, root);
    if (h)
      return h;
    base[pos] = 's';
    base[pos + 1] = 's';
    h = spellsharps(base, pos + 2, n + 1, repnum, info, root);
    if (h)
      return h;
  } else if (repnum > 0) {
    if (utf8)
      return checkword(base.c_str(), info, root);
    std::string tmp(sharps_u8_l1(base));
    return checkword(tmp.c_str(), info, root);
  }
  return NULL;
}

int Hunspell::is_keepcase(const hentry* rv) {
  return pAMgr && rv->astr && pAMgr->get_keepcase() &&
         TESTAFF(rv->astr, pAMgr->get_keepcase(), rv->alen);
}

/* insert a word to the beginning of the suggestion array and return ns */
int Hunspell::insert_sug(char*** slst, const char* word, int ns) {
  if (!*slst)
    return ns;
  char* dup = mystrdup(word);
  if (!dup)
    return ns;
  if (ns == MAXSUGGESTION) {
    ns--;
    free((*slst)[ns]);
  }
  for (int k = ns; k > 0; k--)
    (*slst)[k] = (*slst)[k - 1];
  (*slst)[0] = dup;
  return ns + 1;
}

int Hunspell::spell(const char* word, int* info, char** root) {
  struct hentry* rv = NULL;

  int info2 = 0;
  if (!info)
    info = &info2;
  else
    *info = 0;

  // Hunspell supports XML input of the simplified API (see manual)
  if (strcmp(word, SPELL_XML) == 0)
    return 1;
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return 0;
  } else {
    if (nc >= MAXWORDLEN)
      return 0;
  }
  int captype = NOCAP;
  int abbv = 0;
  int wl = 0;

  std::string scw;
  std::vector<w_char> sunicw;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  {
    char cw[MAXWORDUTF8LEN];
    w_char unicw[MAXWORDLEN];
    char wspace[MAXWORDUTF8LEN];

    int convstatus = rl ? rl->conv(word, wspace, MAXWORDUTF8LEN) : 0;
    if (convstatus < 0)
      return 0;
    else if (convstatus > 0)
      wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
    else
      wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

    scw = std::string(cw);
    sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
  }

#ifdef MOZILLA_CLIENT
  // accept the abbreviated words without dots
  // workaround for the incomplete tokenization of Mozilla
  abbv = 1;
#endif

  if (wl == 0 || maxdic == 0)
    return 1;
  if (root)
    *root = NULL;

  // allow numbers with dots, dashes and commas (but forbid double separators:
  // "..", "--" etc.)
  enum { NBEGIN, NNUM, NSEP };
  int nstate = NBEGIN;
  int i;

  for (i = 0; (i < wl); i++) {
    if ((scw[i] <= '9') && (scw[i] >= '0')) {
      nstate = NNUM;
    } else if ((scw[i] == ',') || (scw[i] == '.') || (scw[i] == '-')) {
      if ((nstate == NSEP) || (i == 0))
        break;
      nstate = NSEP;
    } else
      break;
  }
  if ((i == wl) && (nstate == NNUM))
    return 1;

  switch (captype) {
    case HUHCAP:
    /* FALLTHROUGH */
    case HUHINITCAP:
      *info += SPELL_ORIGCAP;
    /* FALLTHROUGH */
    case NOCAP:
      rv = checkword(scw.c_str(), info, root);
      if ((abbv) && !(rv)) {
        std::string u8buffer(scw);
        u8buffer.push_back('.');
        rv = checkword(u8buffer.c_str(), info, root);
      }
      break;
    case ALLCAP: {
      *info += SPELL_ORIGCAP;
      rv = checkword(scw.c_str(), info, root);
      if (rv)
        break;
      if (abbv) {
        std::string u8buffer(scw);
        u8buffer.push_back('.');
        rv = checkword(u8buffer.c_str(), info, root);
        if (rv)
          break;
      }
      // Spec. prefix handling for Catalan, French, Italian:
      // prefixes separated by apostrophe (SANT'ELIA -> Sant'+Elia).
      size_t apos = pAMgr ? scw.find('\'') : std::string::npos;
      if (apos != std::string::npos) {
        mkallsmall2(scw, sunicw);
        std::string part1 = scw.substr(0, apos+1);
        std::string part2 = scw.substr(apos+1);
        if (utf8) {
          std::vector<w_char> part1u, part2u;
          u8_u16(part1u, part1);
          u8_u16(part2u, part2);
          mkinitcap2(part2, part2u);
          scw = part1 + part2;
          sunicw = part1u;
          sunicw.insert(sunicw.end(), part2u.begin(), part2u.end());
          rv = checkword(scw.c_str(), info, root);
          if (rv)
            break;
        } else {
          mkinitcap2(part2, sunicw);
          scw = part1 + part2;
          rv = checkword(scw.c_str(), info, root);
          if (rv)
            break;
        }
        mkinitcap2(scw, sunicw);
        rv = checkword(scw.c_str(), info, root);
        if (rv)
          break;
      }
      if (pAMgr && pAMgr->get_checksharps() && scw.find("SS") != std::string::npos) {

        mkallsmall2(scw, sunicw);
        std::string u8buffer(scw);
        rv = spellsharps(u8buffer, 0, 0, 0, info, root);
        if (!rv) {
          mkinitcap2(scw, sunicw);
          rv = spellsharps(scw, 0, 0, 0, info, root);
        }
        if ((abbv) && !(rv)) {
          u8buffer.push_back('.');
          rv = spellsharps(u8buffer, 0, 0, 0, info, root);
          if (!rv) {
            u8buffer = std::string(scw);
            u8buffer.push_back('.');
            rv = spellsharps(u8buffer, 0, 0, 0, info, root);
          }
        }
        if (rv)
          break;
      }
    }
    case INITCAP: {

      *info += SPELL_ORIGCAP;
      mkallsmall2(scw, sunicw);
      std::string u8buffer(scw);
      mkinitcap2(scw, sunicw);
      if (captype == INITCAP)
        *info += SPELL_INITCAP;
      rv = checkword(scw.c_str(), info, root);
      if (captype == INITCAP)
        *info -= SPELL_INITCAP;
      // forbid bad capitalization
      // (for example, ijs -> Ijs instead of IJs in Dutch)
      // use explicit forms in dic: Ijs/F (F = FORBIDDENWORD flag)
      if (*info & SPELL_FORBIDDEN) {
        rv = NULL;
        break;
      }
      if (rv && is_keepcase(rv) && (captype == ALLCAP))
        rv = NULL;
      if (rv)
        break;

      rv = checkword(u8buffer.c_str(), info, root);
      if (abbv && !rv) {
        u8buffer.push_back('.');
        rv = checkword(u8buffer.c_str(), info, root);
        if (!rv) {
          u8buffer = scw;
          u8buffer.push_back('.');
          if (captype == INITCAP)
            *info += SPELL_INITCAP;
          rv = checkword(u8buffer.c_str(), info, root);
          if (captype == INITCAP)
            *info -= SPELL_INITCAP;
          if (rv && is_keepcase(rv) && (captype == ALLCAP))
            rv = NULL;
          break;
        }
      }
      if (rv && is_keepcase(rv) &&
          ((captype == ALLCAP) ||
           // if CHECKSHARPS: KEEPCASE words with \xDF  are allowed
           // in INITCAP form, too.
           !(pAMgr->get_checksharps() &&
             ((utf8 && u8buffer.find("\xC3\x9F") != std::string::npos) ||
              (!utf8 && u8buffer.find('\xDF') != std::string::npos)))))
        rv = NULL;
      break;
    }
  }

  if (rv) {
    if (pAMgr && pAMgr->get_warn() && rv->astr &&
        TESTAFF(rv->astr, pAMgr->get_warn(), rv->alen)) {
      *info += SPELL_WARN;
      if (pAMgr->get_forbidwarn())
        return 0;
      return HUNSPELL_OK_WARN;
    }
    return HUNSPELL_OK;
  }

  // recursive breaking at break points
  if (wordbreak) {

    int nbr = 0;
    wl = scw.size();
    int numbreak = pAMgr ? pAMgr->get_numbreak() : 0;

    // calculate break points for recursion limit
    for (int j = 0; j < numbreak; j++) {
      size_t len = strlen(wordbreak[j]);
      size_t pos = 0;
      while ((pos = scw.find(wordbreak[j], pos, len)) != std::string::npos) {
        ++nbr;
        pos += len;
      }
    }
    if (nbr >= 10)
      return 0;

    // check boundary patterns (^begin and end$)
    for (int j = 0; j < numbreak; j++) {
      int plen = strlen(wordbreak[j]);
      if (plen == 1 || plen > wl)
        continue;

      if (wordbreak[j][0] == '^' &&
          scw.compare(0, plen - 1, wordbreak[j] + 1, plen -1) == 0 && spell(scw.c_str() + plen - 1))
        return 1;

      if (wordbreak[j][plen - 1] == '$' &&
          scw.compare(wl - plen + 1, plen - 1, wordbreak[j], plen - 1) == 0) {
        char r = scw[wl - plen + 1];
        scw[wl - plen + 1] = '\0';
        if (spell(scw.c_str()))
          return 1;
        scw[wl - plen + 1] = r;
      }
    }

    // other patterns
    for (int j = 0; j < numbreak; j++) {
      int plen = strlen(wordbreak[j]);
      size_t found = scw.find(wordbreak[j]);
      if ((found > 0) && (found < wl - plen)) {
        if (!spell(scw.c_str() + found + plen))
          continue;
        char r = scw[found];
        scw[found] = '\0';
        // examine 2 sides of the break point
        if (spell(scw.c_str()))
          return 1;
        scw[found] = r;

        // LANG_hu: spec. dash rule
        if (langnum == LANG_hu && strcmp(wordbreak[j], "-") == 0) {
          r = scw[found + 1];
          scw[found + 1] = '\0';
          if (spell(scw.c_str()))
            return 1;  // check the first part with dash
          scw[found + 1] = r;
        }
        // end of LANG specific region
      }
    }
  }

  return 0;
}

struct hentry* Hunspell::checkword(const char* w, int* info, char** root) {
  struct hentry* he = NULL;
  bool usebuffer = false;
  int len, i;
  std::string w2;
  const char* word;

  char* ignoredchars = pAMgr ? pAMgr->get_ignore() : NULL;
  if (ignoredchars != NULL) {
    w2.assign(w);
    if (utf8) {
      int ignoredchars_utf16_len;
      unsigned short* ignoredchars_utf16 =
          pAMgr->get_ignore_utf16(&ignoredchars_utf16_len);
      remove_ignored_chars_utf(w2, ignoredchars_utf16, ignoredchars_utf16_len);
    } else {
      remove_ignored_chars(w2, ignoredchars);
    }
    word = w2.c_str();
    usebuffer = true;
  } else
    word = w;

  len = strlen(word);

  if (!len)
    return NULL;

  // word reversing wrapper for complex prefixes
  if (complexprefixes) {
    if (!usebuffer) {
      w2.assign(word);
      usebuffer = true;
    }
    if (utf8)
      reverseword_utf(w2);
    else
      reverseword(w2);
  }

  if (usebuffer) {
    word = w2.c_str();
  }

  // look word in hash table
  for (i = 0; (i < maxdic) && !he; i++) {
    he = (pHMgr[i])->lookup(word);

    // check forbidden and onlyincompound words
    if ((he) && (he->astr) && (pAMgr) &&
        TESTAFF(he->astr, pAMgr->get_forbiddenword(), he->alen)) {
      if (info)
        *info += SPELL_FORBIDDEN;
      // LANG_hu section: set dash information for suggestions
      if (langnum == LANG_hu) {
        if (pAMgr->get_compoundflag() &&
            TESTAFF(he->astr, pAMgr->get_compoundflag(), he->alen)) {
          if (info)
            *info += SPELL_COMPOUND;
        }
      }
      return NULL;
    }

    // he = next not needaffix, onlyincompound homonym or onlyupcase word
    while (he && (he->astr) && pAMgr &&
           ((pAMgr->get_needaffix() &&
             TESTAFF(he->astr, pAMgr->get_needaffix(), he->alen)) ||
            (pAMgr->get_onlyincompound() &&
             TESTAFF(he->astr, pAMgr->get_onlyincompound(), he->alen)) ||
            (info && (*info & SPELL_INITCAP) &&
             TESTAFF(he->astr, ONLYUPCASEFLAG, he->alen))))
      he = he->next_homonym;
  }

  // check with affixes
  if (!he && pAMgr) {
    // try stripping off affixes */
    he = pAMgr->affix_check(word, len, 0);

    // check compound restriction and onlyupcase
    if (he && he->astr &&
        ((pAMgr->get_onlyincompound() &&
          TESTAFF(he->astr, pAMgr->get_onlyincompound(), he->alen)) ||
         (info && (*info & SPELL_INITCAP) &&
          TESTAFF(he->astr, ONLYUPCASEFLAG, he->alen)))) {
      he = NULL;
    }

    if (he) {
      if ((he->astr) && (pAMgr) &&
          TESTAFF(he->astr, pAMgr->get_forbiddenword(), he->alen)) {
        if (info)
          *info += SPELL_FORBIDDEN;
        return NULL;
      }
      if (root) {
        *root = mystrdup(he->word);
        if (*root && complexprefixes) {
          if (utf8)
            reverseword_utf(*root);
          else
            reverseword(*root);
        }
      }
      // try check compound word
    } else if (pAMgr->get_compound()) {
      he = pAMgr->compound_check(word, len, 0, 0, 100, 0, NULL, 0, 0, info);
      // LANG_hu section: `moving rule' with last dash
      if ((!he) && (langnum == LANG_hu) && (word[len - 1] == '-')) {
        char* dup = mystrdup(word);
        if (!dup)
          return NULL;
        dup[len - 1] = '\0';
        he = pAMgr->compound_check(dup, len - 1, -5, 0, 100, 0, NULL, 1, 0,
                                   info);
        free(dup);
      }
      // end of LANG speficic region
      if (he) {
        if (root) {
          *root = mystrdup(he->word);
          if (*root && complexprefixes) {
            if (utf8)
              reverseword_utf(*root);
            else
              reverseword(*root);
          }
        }
        if (info)
          *info += SPELL_COMPOUND;
      }
    }
  }

  return he;
}

int Hunspell::suggest(char*** slst, const char* word) {
  int onlycmpdsug = 0;
  char cw[MAXWORDUTF8LEN];
  if (!pSMgr || maxdic == 0)
    return 0;
  w_char unicw[MAXWORDLEN];
  *slst = NULL;
  // process XML input of the simplified API (see manual)
  if (strncmp(word, SPELL_XML, sizeof(SPELL_XML) - 3) == 0) {
    return spellml(slst, word);
  }
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return 0;
  } else {
    if (nc >= MAXWORDLEN)
      return 0;
  }
  int captype = 0;
  int abbv = 0;
  int wl = 0;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  {
    char wspace[MAXWORDUTF8LEN];
    int convstatus = rl ? rl->conv(word, wspace, MAXWORDUTF8LEN) : 0;
    if (convstatus < 0)
      return 0;
    else if (convstatus > 0)
      wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
    else
      wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

    if (wl == 0)
      return 0;
  }

  int ns = 0;
  int capwords = 0;

  // check capitalized form for FORCEUCASE
  if (pAMgr && captype == NOCAP && pAMgr->get_forceucase()) {
    int info = SPELL_ORIGCAP;
    char** wlst;
    if (checkword(cw, &info, NULL)) {
      if (*slst) {
        wlst = *slst;
      } else {
        wlst = (char**)malloc(MAXSUGGESTION * sizeof(char*));
        if (wlst == NULL)
          return -1;
        *slst = wlst;
        for (int i = 0; i < MAXSUGGESTION; i++) {
          wlst[i] = NULL;
        }
      }
      std::string form(cw);
      mkinitcap(form);
      wlst[0] = mystrdup(form.c_str());
      return 1;
    }
  }

  bool new_string_synced = false;
  std::string scw;
  std::vector<w_char> sunicw;

  switch (captype) {
    case NOCAP: {

      if (!new_string_synced) {
        scw = std::string(cw);
        sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
        new_string_synced = true;
      }

      ns = pSMgr->suggest(slst, scw.c_str(), ns, &onlycmpdsug);
      break;
    }

    case INITCAP: {

      if (!new_string_synced) {
        scw = std::string(cw);
        sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
        new_string_synced = true;
      }

      capwords = 1;
      ns = pSMgr->suggest(slst, scw.c_str(), ns, &onlycmpdsug);
      if (ns == -1)
        break;
      std::string wspace(scw);
      mkallsmall2(wspace, sunicw);
      ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
      break;
    }
    case HUHINITCAP:
      capwords = 1;
    case HUHCAP: {

      if (!new_string_synced) {
        scw = std::string(cw);
        sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
        new_string_synced = true;
      }

      ns = pSMgr->suggest(slst, scw.c_str(), ns, &onlycmpdsug);
      if (ns != -1) {
        // something.The -> something. The
        size_t dot_pos = scw.find('.');
        if (dot_pos != std::string::npos) {
          std::string postdot = scw.substr(dot_pos + 1);
          int captype_;
          if (utf8) {
            std::vector<w_char> postdotu;
            u8_u16(postdotu, postdot);
            captype_ = get_captype_utf8(postdotu.data(), postdotu.size(), langnum);
          } else {
            captype_ = get_captype(postdot.c_str(), postdot.size(), csconv);
          }
          if (captype_ == INITCAP) {
            std::string str(cw);
            str.insert(dot_pos + 1, 1, ' ');
            ns = insert_sug(slst, str.c_str(), ns);
          }
        }

        std::string wspace;

        if (captype == HUHINITCAP) {
          // TheOpenOffice.org -> The OpenOffice.org
          wspace = scw;
          mkinitsmall2(wspace, sunicw);
          ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
        }
        wspace = scw;
        mkallsmall2(wspace, sunicw);
        if (spell(wspace.c_str()))
          ns = insert_sug(slst, wspace.c_str(), ns);
        int prevns = ns;
        ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
        if (captype == HUHINITCAP) {
          mkinitcap2(wspace, sunicw);
          if (spell(wspace.c_str()))
            ns = insert_sug(slst, wspace.c_str(), ns);
          ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
        }
        // aNew -> "a New" (instead of "a new")
        for (int j = prevns; j < ns; j++) {
          char* space = strchr((*slst)[j], ' ');
          if (space) {
            int slen = strlen(space + 1);
            // different case after space (need capitalisation)
            if ((slen < wl) && strcmp(cw + wl - slen, space + 1)) {
              w_char w[MAXWORDLEN];
              int wc = 0;
              char* r = (*slst)[j];
              if (utf8)
                wc = u8_u16(w, MAXWORDLEN, space + 1);
              mkinitcap2(space + 1, w, wc);
              // set as first suggestion
              for (int k = j; k > 0; k--)
                (*slst)[k] = (*slst)[k - 1];
              (*slst)[0] = r;
            }
          }
        }
      }
      break;
    }

    case ALLCAP: {

      if (!new_string_synced) {
        scw = std::string(cw);
        sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
        new_string_synced = true;
      }

      std::string wspace(scw);
      mkallsmall2(wspace, sunicw);
      ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
      if (ns == -1)
        break;
      if (pAMgr && pAMgr->get_keepcase() && spell(wspace.c_str()))
        ns = insert_sug(slst, wspace.c_str(), ns);
      mkinitcap2(wspace, sunicw);
      ns = pSMgr->suggest(slst, wspace.c_str(), ns, &onlycmpdsug);
      for (int j = 0; j < ns; j++) {
        mkallcap((*slst)[j]);
        if (pAMgr && pAMgr->get_checksharps()) {
          char* pos;
          if (utf8) {
            pos = strstr((*slst)[j], "\xC3\x9F");
            while (pos) {
              *pos = 'S';
              *(pos + 1) = 'S';
              pos = strstr(pos + 2, "\xC3\x9F");
            }
          } else {
            pos = strchr((*slst)[j], '\xDF');
            while (pos) {
              (*slst)[j] = (char*)realloc((*slst)[j], strlen((*slst)[j]) + 2);
              mystrrep((*slst)[j], "\xDF", "SS");
              pos = strchr((*slst)[j], '\xDF');
            }
          }
        }
      }
      break;
    }
  }

  if (!new_string_synced) {
    scw = std::string(cw);
    sunicw = std::vector<w_char>(unicw, unicw + (utf8 ? (nc > -1 ? nc : 0) : 0));
  }

  // LANG_hu section: replace '-' with ' ' in Hungarian
  if (langnum == LANG_hu) {
    for (int j = 0; j < ns; j++) {
      char* pos = strchr((*slst)[j], '-');
      if (pos) {
        int info;
        char w[MAXWORDUTF8LEN];
        *pos = '\0';
        strcpy(w, (*slst)[j]);
        strcat(w, pos + 1);
        (void)spell(w, &info, NULL);
        if ((info & SPELL_COMPOUND) && (info & SPELL_FORBIDDEN)) {
          *pos = ' ';
        } else
          *pos = '-';
      }
    }
  }
  // END OF LANG_hu section

  // try ngram approach since found nothing or only compound words
  if (pAMgr && (ns == 0 || onlycmpdsug) && (pAMgr->get_maxngramsugs() != 0) &&
      (*slst)) {
    switch (captype) {
      case NOCAP: {
        ns = pSMgr->ngsuggest(*slst, scw.c_str(), ns, pHMgr, maxdic);
        break;
      }
      case HUHINITCAP:
        capwords = 1;
      case HUHCAP: {
        std::string wspace(scw);
        mkallsmall2(wspace, sunicw);
        ns = pSMgr->ngsuggest(*slst, wspace.c_str(), ns, pHMgr, maxdic);
        break;
      }
      case INITCAP: {
        capwords = 1;
        std::string wspace(scw);
        mkallsmall2(wspace, sunicw);
        ns = pSMgr->ngsuggest(*slst, wspace.c_str(), ns, pHMgr, maxdic);
        break;
      }
      case ALLCAP: {
        std::string wspace(scw);
        mkallsmall2(wspace, sunicw);
        int oldns = ns;
        ns = pSMgr->ngsuggest(*slst, wspace.c_str(), ns, pHMgr, maxdic);
        for (int j = oldns; j < ns; j++)
          mkallcap((*slst)[j]);
        break;
      }
    }
  }

  // try dash suggestion (Afo-American -> Afro-American)
  size_t dash_pos = scw.find('-');
  if (dash_pos != std::string::npos) {
    int nodashsug = 1;
    if (*slst) {
      for (int j = 0; j < ns && nodashsug == 1; j++) {
        if (strchr((*slst)[j], '-'))
          nodashsug = 0;
      }
    }

    size_t prev_pos = 0;
    bool last = false;

    while (nodashsug && !last) {
      if (dash_pos == scw.size())
        last = 1;
      std::string chunk = scw.substr(prev_pos, dash_pos - prev_pos);
      if (!spell(chunk.c_str())) {
        char** nlst = NULL;
        int nn = suggest(&nlst, chunk.c_str());
        for (int j = nn - 1; j >= 0; j--) {
          std::string wspace = scw.substr(0, prev_pos);
          wspace.append(nlst[j]);
          if (!last) {
            wspace.append("-");
            wspace.append(scw.substr(dash_pos + 1));
          }
          ns = insert_sug(slst, wspace.c_str(), ns);
          free(nlst[j]);
        }
        if (nlst != NULL)
          free(nlst);
        nodashsug = 0;
      }
      if (!last) {
        prev_pos = dash_pos + 1;
        dash_pos = scw.find('-', prev_pos);
      }
      if (dash_pos == std::string::npos)
        dash_pos = scw.size();
    }
  }

  // word reversing wrapper for complex prefixes
  if (complexprefixes) {
    for (int j = 0; j < ns; j++) {
      if (utf8)
        reverseword_utf((*slst)[j]);
      else
        reverseword((*slst)[j]);
    }
  }

  // capitalize
  if (capwords)
    for (int j = 0; j < ns; j++) {
      std::string form((*slst)[j]);
      free((*slst)[j]);
      mkinitcap(form);
      (*slst)[j] = mystrdup(form.c_str());
    }

  // expand suggestions with dot(s)
  if (abbv && pAMgr && pAMgr->get_sugswithdots()) {
    for (int j = 0; j < ns; j++) {
      (*slst)[j] = (char*)realloc((*slst)[j], strlen((*slst)[j]) + 1 + abbv);
      strcat((*slst)[j], word + strlen(word) - abbv);
    }
  }

  // remove bad capitalized and forbidden forms
  if (pAMgr && (pAMgr->get_keepcase() || pAMgr->get_forbiddenword())) {
    switch (captype) {
      case INITCAP:
      case ALLCAP: {
        int l = 0;
        for (int j = 0; j < ns; j++) {
          if (!strchr((*slst)[j], ' ') && !spell((*slst)[j])) {
            char s[MAXSWUTF8L];
            w_char w[MAXSWL];
            int len;
            if (utf8) {
              len = u8_u16(w, MAXSWL, (*slst)[j]);
            } else {
              strcpy(s, (*slst)[j]);
              len = strlen(s);
            }
            mkallsmall2(s, w, len);
            free((*slst)[j]);
            if (spell(s)) {
              (*slst)[l] = mystrdup(s);
              if ((*slst)[l])
                l++;
            } else {
              mkinitcap2(s, w, len);
              if (spell(s)) {
                (*slst)[l] = mystrdup(s);
                if ((*slst)[l])
                  l++;
              }
            }
          } else {
            (*slst)[l] = (*slst)[j];
            l++;
          }
        }
        ns = l;
      }
    }
  }

  // remove duplications
  int l = 0;
  for (int j = 0; j < ns; j++) {
    (*slst)[l] = (*slst)[j];
    for (int k = 0; k < l; k++) {
      if (strcmp((*slst)[k], (*slst)[j]) == 0) {
        free((*slst)[j]);
        l--;
        break;
      }
    }
    l++;
  }
  ns = l;

  // output conversion
  rl = (pAMgr) ? pAMgr->get_oconvtable() : NULL;
  for (int j = 0; rl && j < ns; j++) {
    char wspace[MAXWORDUTF8LEN];
    if (rl->conv((*slst)[j], wspace, MAXWORDUTF8LEN) > 0) {
      free((*slst)[j]);
      (*slst)[j] = mystrdup(wspace);
    }
  }

  // if suggestions removed by nosuggest, onlyincompound parameters
  if (l == 0 && *slst) {
    free(*slst);
    *slst = NULL;
  }
  return l;
}

void Hunspell::free_list(char*** slst, int n) {
  freelist(slst, n);
}

char* Hunspell::get_dic_encoding() {
  return encoding;
}

#ifdef HUNSPELL_EXPERIMENTAL
// XXX UTF-8 support is OK?
int Hunspell::suggest_auto(char*** slst, const char* word) {
  char cw[MAXWORDUTF8LEN];
  char wspace[MAXWORDUTF8LEN];
  if (!pSMgr || maxdic == 0)
    return 0;
  w_char unicw[MAXWORDLEN];
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return 0;
  } else {
    if (nc >= MAXWORDLEN)
      return 0;
  }
  int captype = 0;
  int abbv = 0;
  int wl = 0;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  int convstatus = rl ? rl->conv(word, wspace) : 0;
  if (convstatus < 0)
    return 0;
  else if (convstatus > 0)
    wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
  else
    wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

  if (wl == 0)
    return 0;
  int ns = 0;
  *slst = NULL;  // HU, nsug in pSMgr->suggest

  switch (captype) {
    case NOCAP: {
      ns = pSMgr->suggest_auto(slst, cw, ns);
      if (ns > 0)
        break;
      break;
    }

    case INITCAP: {
      memcpy(wspace, cw, (wl + 1));
      mkallsmall2(wspace, unicw, nc);
      ns = pSMgr->suggest_auto(slst, wspace, ns);
      for (int j = 0; j < ns; j++)
        mkinitcap((*slst)[j]);
      ns = pSMgr->suggest_auto(slst, cw, ns);
      break;
    }

    case HUHINITCAP:
    case HUHCAP: {
      ns = pSMgr->suggest_auto(slst, cw, ns);
      if (ns == 0) {
        memcpy(wspace, cw, (wl + 1));
        mkallsmall2(wspace, unicw, nc);
        ns = pSMgr->suggest_auto(slst, wspace, ns);
      }
      break;
    }

    case ALLCAP: {
      memcpy(wspace, cw, (wl + 1));
      mkallsmall2(wspace, unicw, nc);
      ns = pSMgr->suggest_auto(slst, wspace, ns);

      mkinitcap(wspace);
      ns = pSMgr->suggest_auto(slst, wspace, ns);

      for (int j = 0; j < ns; j++)
        mkallcap((*slst)[j]);
      break;
    }
  }

  // word reversing wrapper for complex prefixes
  if (complexprefixes) {
    for (int j = 0; j < ns; j++) {
      if (utf8)
        reverseword_utf((*slst)[j]);
      else
        reverseword((*slst)[j]);
    }
  }

  // expand suggestions with dot(s)
  if (abbv && pAMgr && pAMgr->get_sugswithdots()) {
    for (int j = 0; j < ns; j++) {
      (*slst)[j] = (char*)realloc((*slst)[j], strlen((*slst)[j]) + 1 + abbv);
      strcat((*slst)[j], word + strlen(word) - abbv);
    }
  }

  // LANG_hu section: replace '-' with ' ' in Hungarian
  if (langnum == LANG_hu) {
    for (int j = 0; j < ns; j++) {
      char* pos = strchr((*slst)[j], '-');
      if (pos) {
        int info;
        char w[MAXWORDUTF8LEN];
        *pos = '\0';
        strcpy(w, (*slst)[j]);
        strcat(w, pos + 1);
        spell(w, &info, NULL);
        if ((info & SPELL_COMPOUND) && (info & SPELL_FORBIDDEN)) {
          *pos = ' ';
        } else
          *pos = '-';
      }
    }
  }

  // output conversion
  rl = (pAMgr) ? pAMgr->get_oconvtable() : NULL;
  for (int j = 0; rl && j < ns; j++) {
    if (rl->conv((*slst)[j], wspace) > 0) {
      free((*slst)[j]);
      (*slst)[j] = mystrdup(wspace);
    }
  }

  // END OF LANG_hu section
  return ns;
}
#endif

int Hunspell::stem(char*** slst, char** desc, int n) {
  char result[MAXLNLEN];
  char result2[MAXLNLEN];
  *slst = NULL;
  if (n == 0)
    return 0;
  *result2 = '\0';
  for (int i = 0; i < n; i++) {
    *result = '\0';
    // add compound word parts (except the last one)
    char* s = (char*)desc[i];
    char* part = strstr(s, MORPH_PART);
    if (part) {
      char* nextpart = strstr(part + 1, MORPH_PART);
      while (nextpart) {
        copy_field(result + strlen(result), part, MORPH_PART);
        part = nextpart;
        nextpart = strstr(part + 1, MORPH_PART);
      }
      s = part;
    }

    char** pl;
    std::string tok(s);
    size_t alt = 0;
    while ((alt = tok.find(" | ", alt)) != std::string::npos) {
      tok[alt + 1] = MSEP_ALT;
    }
    int pln = line_tok(tok.c_str(), &pl, MSEP_ALT);
    for (int k = 0; k < pln; k++) {
      // add derivational suffixes
      if (strstr(pl[k], MORPH_DERI_SFX)) {
        // remove inflectional suffixes
        char* is = strstr(pl[k], MORPH_INFL_SFX);
        if (is)
          *is = '\0';
        char* sg = pSMgr->suggest_gen(&(pl[k]), 1, pl[k]);
        if (sg) {
          char** gen;
          int genl = line_tok(sg, &gen, MSEP_REC);
          free(sg);
          for (int j = 0; j < genl; j++) {
            sprintf(result2 + strlen(result2), "%c%s%s", MSEP_REC, result,
                    gen[j]);
          }
          freelist(&gen, genl);
        }
      } else {
        sprintf(result2 + strlen(result2), "%c%s", MSEP_REC, result);
        if (strstr(pl[k], MORPH_SURF_PFX)) {
          copy_field(result2 + strlen(result2), pl[k], MORPH_SURF_PFX);
        }
        copy_field(result2 + strlen(result2), pl[k], MORPH_STEM);
      }
    }
    freelist(&pl, pln);
  }
  int sln = line_tok(result2, slst, MSEP_REC);
  return uniqlist(*slst, sln);
}

int Hunspell::stem(char*** slst, const char* word) {
  char** pl;
  int pln = analyze(&pl, word);
  int pln2 = stem(slst, pl, pln);
  freelist(&pl, pln);
  return pln2;
}

#ifdef HUNSPELL_EXPERIMENTAL
int Hunspell::suggest_pos_stems(char*** slst, const char* word) {
  char cw[MAXWORDUTF8LEN];
  char wspace[MAXWORDUTF8LEN];
  if (!pSMgr || maxdic == 0)
    return 0;
  w_char unicw[MAXWORDLEN];
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return 0;
  } else {
    if (nc >= MAXWORDLEN)
      return 0;
  }
  int captype = 0;
  int abbv = 0;
  int wl = 0;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  int convstatus = rl ? rl->conv(word, wspace) : 0;
  if (convstatus < 0)
    return 0;
  else if (convstatus > 0)
    wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
  else
    wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

  if (wl == 0)
    return 0;

  int ns = 0;  // ns=0 = normalized input

  *slst = NULL;  // HU, nsug in pSMgr->suggest

  switch (captype) {
    case HUHCAP:
    case NOCAP: {
      ns = pSMgr->suggest_pos_stems(slst, cw, ns);

      if ((abbv) && (ns == 0)) {
        memcpy(wspace, cw, wl);
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        ns = pSMgr->suggest_pos_stems(slst, wspace, ns);
      }

      break;
    }

    case INITCAP: {
      ns = pSMgr->suggest_pos_stems(slst, cw, ns);

      if (ns == 0 || ((*slst)[0][0] == '#')) {
        memcpy(wspace, cw, (wl + 1));
        mkallsmall2(wspace, unicw, nc);
        ns = pSMgr->suggest_pos_stems(slst, wspace, ns);
      }

      break;
    }

    case ALLCAP: {
      ns = pSMgr->suggest_pos_stems(slst, cw, ns);
      if (ns != 0)
        break;

      memcpy(wspace, cw, (wl + 1));
      mkallsmall2(wspace, unicw, nc);
      ns = pSMgr->suggest_pos_stems(slst, wspace, ns);

      if (ns == 0) {
        mkinitcap(wspace);
        ns = pSMgr->suggest_pos_stems(slst, wspace, ns);
      }
      break;
    }
  }

  // output conversion
  rl = (pAMgr) ? pAMgr->get_oconvtable() : NULL;
  for (int j = 0; rl && j < ns; j++) {
    if (rl->conv((*slst)[j], wspace) > 0) {
      free((*slst)[j]);
      (*slst)[j] = mystrdup(wspace);
    }
  }

  return ns;
}
#endif  // END OF HUNSPELL_EXPERIMENTAL CODE

const char* Hunspell::get_wordchars() {
  return pAMgr->get_wordchars();
}

unsigned short* Hunspell::get_wordchars_utf16(int* len) {
  return pAMgr->get_wordchars_utf16(len);
}

void Hunspell::mkinitcap(std::string& u8) {
  if (utf8) {
    std::vector<w_char> u16;
    u8_u16(u16, u8);
    ::mkinitcap_utf(u16, langnum);
    u16_u8(u8, u16);
  } else {
    ::mkinitcap(u8, csconv);
  }
}

int Hunspell::mkinitcap2(char* p, w_char* u, int nc) {
  if (!utf8) {
    if (*p != '\0')
      *p = csconv[((unsigned char)*p)].cupper;
  } else if (nc > 0) {
    unsigned short i = unicodetoupper((u[0].h << 8) + u[0].l, langnum);
    u[0].h = (unsigned char)(i >> 8);
    u[0].l = (unsigned char)(i & 0x00FF);
    u16_u8(p, MAXWORDUTF8LEN, u, nc);
    return strlen(p);
  }
  return nc;
}

int Hunspell::mkinitcap2(std::string& u8, std::vector<w_char>& u16) {
  if (utf8) {
    ::mkinitcap_utf(u16, langnum);
    u16_u8(u8, u16);
  } else {
    ::mkinitcap(u8, csconv);
  }
  return u8.size();
}

int Hunspell::mkinitsmall2(std::string& u8, std::vector<w_char>& u16) {
  if (utf8) {
    ::mkinitsmall_utf(u16, langnum);
    u16_u8(u8, u16);
  } else {
    ::mkinitsmall(u8, csconv);
  }
  return u8.size();
}

int Hunspell::add(const char* word) {
  if (pHMgr[0])
    return (pHMgr[0])->add(word);
  return 0;
}

int Hunspell::add_with_affix(const char* word, const char* example) {
  if (pHMgr[0])
    return (pHMgr[0])->add_with_affix(word, example);
  return 0;
}

int Hunspell::remove(const char* word) {
  if (pHMgr[0])
    return (pHMgr[0])->remove(word);
  return 0;
}

const char* Hunspell::get_version() {
  return pAMgr->get_version();
}

struct cs_info* Hunspell::get_csconv() {
  return csconv;
}

void Hunspell::cat_result(char* result, char* st) {
  if (st) {
    if (*result)
      mystrcat(result, "\n", MAXLNLEN);
    mystrcat(result, st, MAXLNLEN);
    free(st);
  }
}

int Hunspell::analyze(char*** slst, const char* word) {
  char cw[MAXWORDUTF8LEN];
  char wspace[MAXWORDUTF8LEN];
  w_char unicw[MAXWORDLEN];
  int wl2 = 0;
  *slst = NULL;
  if (!pSMgr || maxdic == 0)
    return 0;
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return 0;
  } else {
    if (nc >= MAXWORDLEN)
      return 0;
  }
  int captype = 0;
  int abbv = 0;
  int wl = 0;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  int convstatus = rl ? rl->conv(word, wspace, MAXWORDUTF8LEN) : 0;
  if (convstatus < 0)
    return 0;
  else if (convstatus > 0)
    wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
  else
    wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

  if (wl == 0) {
    if (abbv) {
      for (wl = 0; wl < abbv; wl++)
        cw[wl] = '.';
      cw[wl] = '\0';
      abbv = 0;
    } else
      return 0;
  }

  char result[MAXLNLEN];
  char* st = NULL;

  *result = '\0';

  int n = 0;
  int n2 = 0;
  int n3 = 0;

  // test numbers
  // LANG_hu section: set dash information for suggestions
  if (langnum == LANG_hu) {
    while ((n < wl) && (((cw[n] <= '9') && (cw[n] >= '0')) ||
                        (((cw[n] == '.') || (cw[n] == ',')) && (n > 0)))) {
      n++;
      if ((cw[n] == '.') || (cw[n] == ',')) {
        if (((n2 == 0) && (n > 3)) ||
            ((n2 > 0) && ((cw[n - 1] == '.') || (cw[n - 1] == ','))))
          break;
        n2++;
        n3 = n;
      }
    }

    if ((n == wl) && (n3 > 0) && (n - n3 > 3))
      return 0;
    if ((n == wl) || ((n > 0) && ((cw[n] == '%') || (cw[n] == '\xB0')) &&
                      checkword(cw + n, NULL, NULL))) {
      mystrcat(result, cw, MAXLNLEN);
      result[n - 1] = '\0';
      if (n == wl)
        cat_result(result, pSMgr->suggest_morph(cw + n - 1));
      else {
        char sign = cw[n];
        cw[n] = '\0';
        cat_result(result, pSMgr->suggest_morph(cw + n - 1));
        mystrcat(result, "+", MAXLNLEN);  // XXX SPEC. MORPHCODE
        cw[n] = sign;
        cat_result(result, pSMgr->suggest_morph(cw + n));
      }
      return line_tok(result, slst, MSEP_REC);
    }
  }
  // END OF LANG_hu section

  switch (captype) {
    case HUHCAP:
    case HUHINITCAP:
    case NOCAP: {
      cat_result(result, pSMgr->suggest_morph(cw));
      if (abbv) {
        memcpy(wspace, cw, wl);
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        cat_result(result, pSMgr->suggest_morph(wspace));
      }
      break;
    }
    case INITCAP: {
      wl = mkallsmall2(cw, unicw, nc);
      memcpy(wspace, cw, (wl + 1));
      wl2 = mkinitcap2(cw, unicw, nc);
      cat_result(result, pSMgr->suggest_morph(wspace));
      cat_result(result, pSMgr->suggest_morph(cw));
      if (abbv) {
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        cat_result(result, pSMgr->suggest_morph(wspace));

        memcpy(wspace, cw, wl2);
        *(wspace + wl2) = '.';
        *(wspace + wl2 + 1) = '\0';

        cat_result(result, pSMgr->suggest_morph(wspace));
      }
      break;
    }
    case ALLCAP: {
      cat_result(result, pSMgr->suggest_morph(cw));
      if (abbv) {
        memcpy(wspace, cw, wl);
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        cat_result(result, pSMgr->suggest_morph(cw));
      }
      wl = mkallsmall2(cw, unicw, nc);
      memcpy(wspace, cw, (wl + 1));
      wl2 = mkinitcap2(cw, unicw, nc);

      cat_result(result, pSMgr->suggest_morph(wspace));
      cat_result(result, pSMgr->suggest_morph(cw));
      if (abbv) {
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        cat_result(result, pSMgr->suggest_morph(wspace));

        memcpy(wspace, cw, wl2);
        *(wspace + wl2) = '.';
        *(wspace + wl2 + 1) = '\0';

        cat_result(result, pSMgr->suggest_morph(wspace));
      }
      break;
    }
  }

  if (*result) {
    // word reversing wrapper for complex prefixes
    if (complexprefixes) {
      if (utf8)
        reverseword_utf(result);
      else
        reverseword(result);
    }
    return line_tok(result, slst, MSEP_REC);
  }

  // compound word with dash (HU) I18n
  char* dash = NULL;
  int nresult = 0;
  // LANG_hu section: set dash information for suggestions
  if (langnum == LANG_hu)
    dash = (char*)strchr(cw, '-');
  if ((langnum == LANG_hu) && dash) {
    *dash = '\0';
    // examine 2 sides of the dash
    if (dash[1] == '\0') {  // base word ending with dash
      if (spell(cw)) {
        char* p = pSMgr->suggest_morph(cw);
        if (p) {
          int ret = line_tok(p, slst, MSEP_REC);
          free(p);
          return ret;
        }
      }
    } else if ((dash[1] == 'e') && (dash[2] == '\0')) {  // XXX (HU) -e hat.
      if (spell(cw) && (spell("-e"))) {
        st = pSMgr->suggest_morph(cw);
        if (st) {
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        mystrcat(result, "+", MAXLNLEN);  // XXX spec. separator in MORPHCODE
        st = pSMgr->suggest_morph("-e");
        if (st) {
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        return line_tok(result, slst, MSEP_REC);
      }
    } else {
      // first word ending with dash: word- XXX ???
      char r2 = *(dash + 1);
      dash[0] = '-';
      dash[1] = '\0';
      nresult = spell(cw);
      dash[1] = r2;
      dash[0] = '\0';
      if (nresult && spell(dash + 1) &&
          ((strlen(dash + 1) > 1) || ((dash[1] > '0') && (dash[1] < '9')))) {
        st = pSMgr->suggest_morph(cw);
        if (st) {
          mystrcat(result, st, MAXLNLEN);
          free(st);
          mystrcat(result, "+", MAXLNLEN);  // XXX spec. separator in MORPHCODE
        }
        st = pSMgr->suggest_morph(dash + 1);
        if (st) {
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        return line_tok(result, slst, MSEP_REC);
      }
    }
    // affixed number in correct word
    if (nresult && (dash > cw) &&
        (((*(dash - 1) <= '9') && (*(dash - 1) >= '0')) ||
         (*(dash - 1) == '.'))) {
      *dash = '-';
      n = 1;
      if (*(dash - n) == '.')
        n++;
      // search first not a number character to left from dash
      while (((dash - n) >= cw) && ((*(dash - n) == '0') || (n < 3)) &&
             (n < 6)) {
        n++;
      }
      if ((dash - n) < cw)
        n--;
      // numbers: valami1000000-hoz
      // examine 100000-hoz, 10000-hoz 1000-hoz, 10-hoz,
      // 56-hoz, 6-hoz
      for (; n >= 1; n--) {
        if ((*(dash - n) >= '0') && (*(dash - n) <= '9') &&
            checkword(dash - n, NULL, NULL)) {
          mystrcat(result, cw, MAXLNLEN);
          result[dash - cw - n] = '\0';
          st = pSMgr->suggest_morph(dash - n);
          if (st) {
            mystrcat(result, st, MAXLNLEN);
            free(st);
          }
          return line_tok(result, slst, MSEP_REC);
        }
      }
    }
  }
  return 0;
}

int Hunspell::generate(char*** slst, const char* word, char** pl, int pln) {
  *slst = NULL;
  if (!pSMgr || !pln)
    return 0;
  char** pl2;
  int pl2n = analyze(&pl2, word);
  int captype = 0;
  int abbv = 0;
  char cw[MAXWORDUTF8LEN];
  cleanword(cw, word, &captype, &abbv);
  char result[MAXLNLEN];
  *result = '\0';

  for (int i = 0; i < pln; i++) {
    cat_result(result, pSMgr->suggest_gen(pl2, pl2n, pl[i]));
  }
  freelist(&pl2, pl2n);

  if (*result) {
    // allcap
    if (captype == ALLCAP)
      mkallcap(result);

    // line split
    int linenum = line_tok(result, slst, MSEP_REC);

    // capitalize
    if (captype == INITCAP || captype == HUHINITCAP) {
      for (int j = 0; j < linenum; j++) {
        std::string form((*slst)[j]);
        free((*slst)[j]);
        mkinitcap(form);
        (*slst)[j] = mystrdup(form.c_str());
      }
    }

    // temporary filtering of prefix related errors (eg.
    // generate("undrinkable", "eats") --> "undrinkables" and "*undrinks")

    int r = 0;
    for (int j = 0; j < linenum; j++) {
      if (!spell((*slst)[j])) {
        free((*slst)[j]);
        (*slst)[j] = NULL;
      } else {
        if (r < j)
          (*slst)[r] = (*slst)[j];
        r++;
      }
    }
    if (r > 0)
      return r;
    free(*slst);
    *slst = NULL;
  }
  return 0;
}

int Hunspell::generate(char*** slst, const char* word, const char* pattern) {
  char** pl;
  int pln = analyze(&pl, pattern);
  int n = generate(slst, word, pl, pln);
  freelist(&pl, pln);
  return uniqlist(*slst, n);
}

// minimal XML parser functions
int Hunspell::get_xml_par(char* dest, const char* par, int max) {
  char* d = dest;
  if (!par)
    return 0;
  char end = *par;
  char* dmax = dest + max;
  if (end == '>')
    end = '<';
  else if (end != '\'' && end != '"')
    return 0;  // bad XML
  for (par++; d < dmax && *par != '\0' && *par != end; par++, d++)
    *d = *par;
  *d = '\0';
  mystrrep(dest, "&lt;", "<");
  mystrrep(dest, "&amp;", "&");
  return (int)(d - dest);
}

int Hunspell::get_langnum() const {
  return langnum;
}

int Hunspell::input_conv(const char* word, char* dest, size_t destsize) {
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  return (rl && rl->conv(word, dest, destsize) > 0);
}

// return the beginning of the element (attr == NULL) or the attribute
const char* Hunspell::get_xml_pos(const char* s, const char* attr) {
  const char* end = strchr(s, '>');
  const char* p = s;
  if (attr == NULL)
    return end;
  do {
    p = strstr(p, attr);
    if (!p || p >= end)
      return 0;
  } while (*(p - 1) != ' ' && *(p - 1) != '\n');
  return p + strlen(attr);
}

int Hunspell::check_xml_par(const char* q,
                            const char* attr,
                            const char* value) {
  char cw[MAXWORDUTF8LEN];
  if (get_xml_par(cw, get_xml_pos(q, attr), MAXWORDUTF8LEN - 1) &&
      strcmp(cw, value) == 0)
    return 1;
  return 0;
}

int Hunspell::get_xml_list(char*** slst, char* list, const char* tag) {
  int n = 0;
  char* p;
  if (!list)
    return 0;
  for (p = list; ((p = strstr(p, tag)) != NULL); p++)
    n++;
  if (n == 0)
    return 0;
  *slst = (char**)malloc(sizeof(char*) * n);
  if (!*slst)
    return 0;
  for (p = list, n = 0; ((p = strstr(p, tag)) != NULL); p++, n++) {
    int l = strlen(p);
    (*slst)[n] = (char*)malloc(l + 1);
    if (!(*slst)[n])
      return n;
    if (!get_xml_par((*slst)[n], p + strlen(tag) - 1, l)) {
      free((*slst)[n]);
      break;
    }
  }
  return n;
}

int Hunspell::spellml(char*** slst, const char* word) {
  char *q, *q2;
  char cw[MAXWORDUTF8LEN], cw2[MAXWORDUTF8LEN];
  q = (char*)strstr(word, "<query");
  if (!q)
    return 0;  // bad XML input
  q2 = strchr(q, '>');
  if (!q2)
    return 0;  // bad XML input
  q2 = strstr(q2, "<word");
  if (!q2)
    return 0;  // bad XML input
  if (check_xml_par(q, "type=", "analyze")) {
    int n = 0;
    if (get_xml_par(cw, strchr(q2, '>'), MAXWORDUTF8LEN - 10))
      n = analyze(slst, cw);
    if (n == 0)
      return 0;
    // convert the result to <code><a>ana1</a><a>ana2</a></code> format
    std::string r;
    r.append("<code>");
    for (int i = 0; i < n; i++) {
      r.append("<a>");

      std::string entry((*slst)[i]);
      free((*slst)[i]);
      mystrrep(entry, "\t", " ");
      mystrrep(entry, "&", "&amp;");
      mystrrep(entry, "<", "&lt;");
      r.append(entry);

      r.append("</a>");
    }
    r.append("</code>");
    (*slst)[0] = mystrdup(r.c_str());
    return 1;
  } else if (check_xml_par(q, "type=", "stem")) {
    if (get_xml_par(cw, strchr(q2, '>'), MAXWORDUTF8LEN - 1))
      return stem(slst, cw);
  } else if (check_xml_par(q, "type=", "generate")) {
    int n = get_xml_par(cw, strchr(q2, '>'), MAXWORDUTF8LEN - 1);
    if (n == 0)
      return 0;
    char* q3 = strstr(q2 + 1, "<word");
    if (q3) {
      if (get_xml_par(cw2, strchr(q3, '>'), MAXWORDUTF8LEN - 1)) {
        return generate(slst, cw, cw2);
      }
    } else {
      if ((q2 = strstr(q2 + 1, "<code")) != NULL) {
        char** slst2;
        if ((n = get_xml_list(&slst2, strchr(q2, '>'), "<a>")) != 0) {
          int n2 = generate(slst, cw, slst2, n);
          freelist(&slst2, n);
          return uniqlist(*slst, n2);
        }
        freelist(&slst2, n);
      }
    }
  }
  return 0;
}

#ifdef HUNSPELL_EXPERIMENTAL
// XXX is UTF-8 support OK?
char* Hunspell::morph_with_correction(const char* word) {
  char cw[MAXWORDUTF8LEN];
  char wspace[MAXWORDUTF8LEN];
  if (!pSMgr || maxdic == 0)
    return NULL;
  w_char unicw[MAXWORDLEN];
  int nc = strlen(word);
  if (utf8) {
    if (nc >= MAXWORDUTF8LEN)
      return NULL;
  } else {
    if (nc >= MAXWORDLEN)
      return NULL;
  }
  int captype = 0;
  int abbv = 0;
  int wl = 0;

  // input conversion
  RepList* rl = (pAMgr) ? pAMgr->get_iconvtable() : NULL;
  int convstatus = rl ? rl->conv(word, wspace) : 0;
  if (convstatus < 0)
    return 0;
  else if (convstatus > 0)
    wl = cleanword2(cw, wspace, unicw, &nc, &captype, &abbv);
  else
    wl = cleanword2(cw, word, unicw, &nc, &captype, &abbv);

  if (wl == 0)
    return NULL;

  char result[MAXLNLEN];
  char* st = NULL;

  *result = '\0';

  switch (captype) {
    case NOCAP: {
      st = pSMgr->suggest_morph_for_spelling_error(cw);
      if (st) {
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      if (abbv) {
        memcpy(wspace, cw, wl);
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          if (*result)
            mystrcat(result, "\n", MAXLNLEN);
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
      }
      break;
    }
    case INITCAP: {
      memcpy(wspace, cw, (wl + 1));
      mkallsmall2(wspace, unicw, nc);
      st = pSMgr->suggest_morph_for_spelling_error(wspace);
      if (st) {
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      st = pSMgr->suggest_morph_for_spelling_error(cw);
      if (st) {
        if (*result)
          mystrcat(result, "\n", MAXLNLEN);
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      if (abbv) {
        memcpy(wspace, cw, wl);
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        mkallsmall2(wspace, unicw, nc);
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          if (*result)
            mystrcat(result, "\n", MAXLNLEN);
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        mkinitcap(wspace);
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          if (*result)
            mystrcat(result, "\n", MAXLNLEN);
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
      }
      break;
    }
    case HUHCAP: {
      st = pSMgr->suggest_morph_for_spelling_error(cw);
      if (st) {
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      memcpy(wspace, cw, (wl + 1));
      mkallsmall2(wspace, unicw, nc);
      st = pSMgr->suggest_morph_for_spelling_error(wspace);
      if (st) {
        if (*result)
          mystrcat(result, "\n", MAXLNLEN);
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      break;
    }
    case ALLCAP: {
      memcpy(wspace, cw, (wl + 1));
      st = pSMgr->suggest_morph_for_spelling_error(wspace);
      if (st) {
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      mkallsmall2(wspace, unicw, nc);
      st = pSMgr->suggest_morph_for_spelling_error(wspace);
      if (st) {
        if (*result)
          mystrcat(result, "\n", MAXLNLEN);
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      mkinitcap(wspace);
      st = pSMgr->suggest_morph_for_spelling_error(wspace);
      if (st) {
        if (*result)
          mystrcat(result, "\n", MAXLNLEN);
        mystrcat(result, st, MAXLNLEN);
        free(st);
      }
      if (abbv) {
        memcpy(wspace, cw, (wl + 1));
        *(wspace + wl) = '.';
        *(wspace + wl + 1) = '\0';
        if (*result)
          mystrcat(result, "\n", MAXLNLEN);
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        mkallsmall2(wspace, unicw, nc);
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          if (*result)
            mystrcat(result, "\n", MAXLNLEN);
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
        mkinitcap(wspace);
        st = pSMgr->suggest_morph_for_spelling_error(wspace);
        if (st) {
          if (*result)
            mystrcat(result, "\n", MAXLNLEN);
          mystrcat(result, st, MAXLNLEN);
          free(st);
        }
      }
      break;
    }
  }

  if (*result)
    return mystrdup(result);
  return NULL;
}

#endif  // END OF HUNSPELL_EXPERIMENTAL CODE

Hunhandle* Hunspell_create(const char* affpath, const char* dpath) {
  return (Hunhandle*)(new Hunspell(affpath, dpath));
}

Hunhandle* Hunspell_create_key(const char* affpath,
                               const char* dpath,
                               const char* key) {
  return (Hunhandle*)(new Hunspell(affpath, dpath, key));
}

void Hunspell_destroy(Hunhandle* pHunspell) {
  delete (Hunspell*)(pHunspell);
}

int Hunspell_add_dic(Hunhandle* pHunspell, const char* dpath) {
  return ((Hunspell*)pHunspell)->add_dic(dpath);
}

int Hunspell_spell(Hunhandle* pHunspell, const char* word) {
  return ((Hunspell*)pHunspell)->spell(word);
}

char* Hunspell_get_dic_encoding(Hunhandle* pHunspell) {
  return ((Hunspell*)pHunspell)->get_dic_encoding();
}

int Hunspell_suggest(Hunhandle* pHunspell, char*** slst, const char* word) {
  return ((Hunspell*)pHunspell)->suggest(slst, word);
}

int Hunspell_analyze(Hunhandle* pHunspell, char*** slst, const char* word) {
  return ((Hunspell*)pHunspell)->analyze(slst, word);
}

int Hunspell_stem(Hunhandle* pHunspell, char*** slst, const char* word) {
  return ((Hunspell*)pHunspell)->stem(slst, word);
}

int Hunspell_stem2(Hunhandle* pHunspell, char*** slst, char** desc, int n) {
  return ((Hunspell*)pHunspell)->stem(slst, desc, n);
}

int Hunspell_generate(Hunhandle* pHunspell,
                      char*** slst,
                      const char* word,
                      const char* word2) {
  return ((Hunspell*)pHunspell)->generate(slst, word, word2);
}

int Hunspell_generate2(Hunhandle* pHunspell,
                       char*** slst,
                       const char* word,
                       char** desc,
                       int n) {
  return ((Hunspell*)pHunspell)->generate(slst, word, desc, n);
}

/* functions for run-time modification of the dictionary */

/* add word to the run-time dictionary */

int Hunspell_add(Hunhandle* pHunspell, const char* word) {
  return ((Hunspell*)pHunspell)->add(word);
}

/* add word to the run-time dictionary with affix flags of
 * the example (a dictionary word): Hunspell will recognize
 * affixed forms of the new word, too.
 */

int Hunspell_add_with_affix(Hunhandle* pHunspell,
                            const char* word,
                            const char* example) {
  return ((Hunspell*)pHunspell)->add_with_affix(word, example);
}

/* remove word from the run-time dictionary */

int Hunspell_remove(Hunhandle* pHunspell, const char* word) {
  return ((Hunspell*)pHunspell)->remove(word);
}

void Hunspell_free_list(Hunhandle*, char*** slst, int n) {
  freelist(slst, n);
}

int Hunspell::suffix_suggest(char*** slst, const char* root_word) {
  struct hentry* he = NULL;
  int len;
  std::string w2;
  const char* word;
  char* ignoredchars = pAMgr->get_ignore();
  if (ignoredchars != NULL) {
    w2.assign(root_word);
    if (utf8) {
      int ignoredchars_utf16_len;
      unsigned short* ignoredchars_utf16 =
          pAMgr->get_ignore_utf16(&ignoredchars_utf16_len);
      remove_ignored_chars_utf(w2, ignoredchars_utf16, ignoredchars_utf16_len);
    } else {
      remove_ignored_chars(w2, ignoredchars);
    }
    word = w2.c_str();
  } else
    word = root_word;

  len = strlen(word);

  if (!len)
    return 0;

  char** wlst = (char**)malloc(MAXSUGGESTION * sizeof(char*));
  if (wlst == NULL)
    return -1;
  *slst = wlst;
  for (int i = 0; i < MAXSUGGESTION; i++) {
    wlst[i] = NULL;
  }

  for (int i = 0; (i < maxdic) && !he; i++) {
    he = (pHMgr[i])->lookup(word);
  }
  if (he) {
    return pAMgr->get_suffix_words(he->astr, he->alen, root_word, *slst);
  }
  return 0;
}

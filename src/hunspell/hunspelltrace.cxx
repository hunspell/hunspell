/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * Copyright (C) 2002-2022 Németh László
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
 * Hunspell is based on MySpell which is Copyright (C) 2002 Kevin Hendricks.
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

#include "hunspelltrace.hxx"

#include <cstdarg>
#include <cstdio>
#include <string>

#include "affentry.hxx"
#include "affixmgr.hxx"

void trace(const TraceCtx& context, const char* format, ...) {
  va_list args;
  va_start(args, format);
  va_list measure;
  va_copy(measure, args);
  int len = vsnprintf(nullptr, 0, format, measure);
  va_end(measure);
  std::string line;
  if (len > 0) {
    // vsnprintf writes its own terminator, so make room for it and drop it
    // again.
    line.resize(len + 1);
    vsnprintf(&line[0], len + 1, format, args);
    line.pop_back();
  }
  va_end(args);
  context.emit(line);
}

std::string trace_flags(const AffixMgr* pAMgr,
                        const unsigned short* astr,
                        int alen) {
  if (!pAMgr || !astr || alen <= 0)
    return "(none)";
  std::string result;
  for (int i = 0; i < alen; ++i) {
    if (i)
      result.push_back(',');
    result.append(pAMgr->encode_flag(astr[i]));
  }
  return result;
}

void trace_affix(const TraceCtx& context,
                 const char* verb,
                 const AffixMgr* pAMgr,
                 const AffEntry& entry) {
  // the rule file wrote a condition the stripping already forces, so hunspell
  // dropped it. Say so, or the dot looks like the file's own text
  const char* redundant =
      (entry.opts & aeREDUNDANTCOND) != 0 ? " redundant=Y" : "";

  trace(context,
        "%s flag=%s strip=\"%s\" add=\"%s\" cont=%s cond=\"%s\"%s at=aff:%d"
        " xprod=%c hdr=aff:%d",
        verb, pAMgr->encode_flag(entry.aflag).c_str(), entry.strip.c_str(),
        entry.appnd.c_str(),
        trace_flags(pAMgr, entry.contclass, entry.contclasslen).c_str(),
        entry.get_condition().c_str(), redundant, entry.line, entry.xprod,
        entry.headerline);
}

void trace_test(const TraceCtx& context,
                const char* name,
                const AffixMgr* pAMgr,
                unsigned short flag,
                const char* where,
                const unsigned short* astr,
                int alen,
                const char* outcome) {
  trace(context, "test %s flag=%s in=%s have=%s -> %s", name,
        pAMgr->encode_flag(flag).c_str(), where,
        trace_flags(pAMgr, astr, alen).c_str(), outcome);
}

void trace_form(const TraceCtx& context,
                const AffixMgr* pAMgr,
                const std::string& surface,
                const char* stem,
                PfxEntry* pfx,
                SfxEntry* sfx) {
  std::string line = "form \"" + surface + "\" = ";
  if (pfx)
    line += "pfx(" + pAMgr->encode_flag(pfx->getFlag()) + ":\"" +
            pfx->getKey() + "\") + ";
  line += '"';
  line += stem;
  line += '"';
  if (sfx)
    line += " + sfx(" + pAMgr->encode_flag(sfx->getFlag()) + ":\"" +
            sfx->getAffix() + "\")";
  context.emit(line);
}

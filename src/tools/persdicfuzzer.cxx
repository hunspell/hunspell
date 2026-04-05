/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// Fuzzer for personal dictionary operations

#include <hunspell/hunspell.hxx>
#include <fstream>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const char* data, size_t size)
{
    if (size < 4)
        return 0;

    // first byte: length of operations sequence
    unsigned char opslen = data[0];
    if (opslen > 64)
        opslen = 64;
    ++data;
    --size;
    if (opslen > size)
        return 0;
    const char* ops = data;
    data += opslen;
    size -= opslen;

    // split remainder into aff and dic
    size_t affsize = size / 2;
    size_t dicsize = size - affsize;

    std::ofstream aff("/tmp/perstest.aff", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    aff.write(data, affsize);
    aff.close();

    std::ofstream dic("/tmp/perstest.dic", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    dic.write(data + affsize, dicsize);
    dic.close();

    Hunspell dict("/tmp/perstest.aff", "/tmp/perstest.dic");

    // collect words from the dic for use as examples in add_with_affix
    std::string dictwords(data + affsize, dicsize);

    // extract short words from the fuzz input to use as operations
    std::vector<std::string> words;
    std::string current;
    for (size_t i = 0; i < dicsize; ++i) {
        char c = dictwords[i];
        if (c == '\n' || c == '\r' || c == '/') {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty())
        words.push_back(current);

    // add some fixed words to ensure we have enough
    words.push_back("test");
    words.push_back("example");

    // execute operations driven by the fuzz input
    for (unsigned char i = 0; i < opslen; ++i) {
        unsigned char op = ops[i];
        size_t widx = (op >> 2) % words.size();
        size_t widx2 = (op >> 4) % words.size();
        switch (op & 3) {
            case 0:
                dict.spell(words[widx]);
                break;
            case 1:
                dict.add(words[widx]);
                break;
            case 2:
                dict.remove(words[widx]);
                break;
            case 3:
                dict.add_with_affix(words[widx], words[widx2]);
                break;
        }
    }

    // final spell check of a limited number of words to trigger any use-after-free
    for (size_t j = 0; j < words.size() && j < 5; ++j)
        dict.spell(words[j]);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

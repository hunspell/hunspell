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

// Fuzzer for TextParser operations

#include "../parsers/textparser.hxx"
#include <string>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const char* data, size_t size)
{
    if (size < 2)
        return 0;

    // first byte: flags (utf8 mode, url checking)
    unsigned char flags = data[0];
    ++data;
    --size;

    // second byte: number of replacement strings that follow
    unsigned char nreplacements = data[0];
    ++data;
    --size;

    // extract replacement strings (null-terminated within the data)
    std::vector<std::string> replacements;
    size_t pos = 0;
    for (unsigned char i = 0; i < nreplacements && pos < size; ++i) {
        size_t end = pos;
        while (end < size && data[end] != '\0')
            ++end;
        size_t len = end - pos;
        if (len > 0 && len <= MAXLNLEN)
            replacements.emplace_back(data + pos, len);
        pos = end + 1;  // skip null terminator
    }

    if (pos >= size)
        return 0;

    // remaining data is the line to parse
    std::string line(data + pos, size - pos);
    // ensure no embedded nulls in the line
    for (auto& c : line) {
        if (c == '\0')
            c = ' ';
    }

    TextParser parser(flags & 1 ? "qwertzuiopasdfghjklyxcvbnmQWERTZUIOPASDFGHJKLYXCVBNM'" : NULL);
    parser.set_url_checking(flags & 2 ? 1 : 0);
    parser.put_line(line.c_str());

    std::string token;
    size_t repidx = 0;
    while (parser.next_token(token)) {
        // simulate auto-correct: replace some tokens with (potentially longer) strings
        if (!replacements.empty()) {
            // stop replacing if line has grown too large to avoid OOM
            if (parser.get_line().size() > MAXLNLEN)
                break;
            parser.change_token(replacements[repidx % replacements.size()].c_str());
            ++repidx;
            // change_token resets head to token start, so consume the
            // replacement to avoid re-replacing it in an infinite loop
            if (!parser.next_token(token))
                break;
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

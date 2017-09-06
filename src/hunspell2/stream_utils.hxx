/* Copyright 2016-2017 Dimitrij Mijoski
 *
 * This file is part of Hunspell-2.
 *
 * Hunspell-2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hunspell-2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell-2.	If not, see <http://www.gnu.org/licenses/>.
 *
 * Hunspell 2 is based on Hunspell v1 and MySpell.
 * Hunspell v1 is Copyright (C) 2002-2017 Németh László
 * MySpell is Copyright (C) 2002 Kevin Hendricks.
 */

/**
 * @file stream_utils.hxx
 * @brief C++ stream functions.
 */

#ifndef STREAM_UTILS_HXX
#define STREAM_UTILS_HXX

#include <istream>
#include <string>
#include <vector>

namespace Hunspell {

inline void reset_failbit_istream(std::istream& in)
{
        in.clear(in.rdstate() & ~in.failbit);
}

inline bool read_to_slash_or_space(std::istream& in, std::string& out)
{
        in >> std::ws;
        int c;
        bool readSomething = false;
        while ((c = in.get()) != std::istream::traits_type::eof() &&
               !isspace((char)c, in.getloc()) && c != '/') {
                out.push_back(c);
                readSomething = true;
        }
        bool slash = c == '/';
        if (readSomething || slash) {
                reset_failbit_istream(in);
        }
        return slash;
}

inline bool read_to_slash(std::istream& in, std::string& out)
{
        in >> std::ws;
        int c;
        bool readSomething = false;
        while ((c = in.get()) != std::istream::traits_type::eof() && c != '/') {
                out.push_back(c);
                readSomething = true;
        }
        bool slash = c == '/';
        if (readSomething || slash) {
                reset_failbit_istream(in);
        }
        return slash;
}

inline void parse_morhological_fields(std::istream& in,
                                      std::vector<std::string>& vecOut)
{
        if (!in.good()) {
                return;
        }

        std::string morph;
        while (in >> morph) {
                vecOut.push_back(morph);
        }
        reset_failbit_istream(in);
}

}

#endif // STREAM_UTILS_HXX

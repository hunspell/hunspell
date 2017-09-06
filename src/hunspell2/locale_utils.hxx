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
 * @file locale_utils.hxx
 * @brief String transformations that depend on locales and enocdings.
 */

#ifndef LOCALE_UTILS_HXX
#define LOCALE_UTILS_HXX

#include <codecvt>
#include <istream>
#include <locale>
#include <string>

namespace Hunspell {

using utf8_to_ucs2_converter =
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t>;

inline void toupper(std::string& s, const std::locale& loc /* = std::locale()*/)
{
	for (auto& c : s)
		c = std::toupper(c, loc);
}

unsigned char utf8_low_level(unsigned char state, char in, char32_t* out,
                             bool* too_short_err);

bool validate_utf8(const std::string& s);
}

#endif // LOCALE_UTILS_HXX

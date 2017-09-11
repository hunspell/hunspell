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

template <class CharT>
auto toupper(std::basic_string<CharT>& s, const std::locale& loc) -> void
{
	auto& f = std::use_facet<std::ctype<char>>(loc);
	f.toupper(&s[0], &s[s.size()]);
}

auto decode_utf8(const std::string& s) -> std::u32string;
auto validate_utf8(const std::string& s) -> bool;

auto is_ascii(char c) -> bool;
auto is_all_ascii(const std::string& s) -> bool;
auto ascii_to_ucs2_skip_invalid(const std::string& s) -> std::u16string;

auto latin1_to_ucs2(const std::string& s) -> std::u16string;
auto latin1_to_u32(const std::string& s) -> std::u32string;

auto is_bmp(char32_t c) -> bool;
// auto is_non_bmp(char32_t c) -> bool;
auto is_all_bmp(const std::u32string& s) -> bool;
auto u32_to_ucs2_skip_non_bmp(const std::u32string& s) -> std::u16string;
}

#endif // LOCALE_UTILS_HXX

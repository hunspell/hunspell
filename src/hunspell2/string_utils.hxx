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

/*!
 * \file string_utils.hxx
 * \brief String algorithms not dependent on locale.
 */

#ifndef HUNSPELL_STRING_UTILS_HXX
#define HUNSPELL_STRING_UTILS_HXX

#include <iterator>
#include <string>
#include <vector>

namespace hunspell {

/*!
 * Splits string on seperator.
 *
 * \param s a string to split.
 * \param sep char or string that acts as separator to split on.
 * \param out start of the output range to which append separated strings.
 * \return The iterator that indicates the end of the output range.
 */
template <class CharT, class CharOrStr, class OutIt>
auto split(const std::basic_string<CharT>& s, CharOrStr sep, OutIt out)
{
	using size_type = typename std::basic_string<CharT>::size_type;
	size_type i1 = 0;
	size_type i2;
	do {
		i2 = s.find(sep, i1);
		*out++ = s.substr(i1, i2 - i1);
		i1 = i2 + 1;
		// i2 gets s.npos after the last separator
		// lenth of i2-i1 will always go past the end
		// yet that is defined
	} while (i2 != s.npos);
	return out;
}

template <class CharT, class CharOrStr>
auto split_v(const std::basic_string<CharT>& s, CharOrStr sep,
             std::vector<std::basic_string<CharT>>& v)
{
	v.clear();
	split(s, sep, std::back_inserter(v));
}

/*!
 * Splits string on any seperator.
 *
 * \param s a string to split.
 * \param sep string holding all separators to split on.
 * \param out start of the output range to which append separated strings.
 * \return The iterator that indicates the end of the output range.
 */
template <class CharT, class CharOrStr, class OutIt>
auto split_on_any_of(const std::basic_string<CharT>& s, CharOrStr sep,
                     OutIt out)
{
	using size_type = typename std::basic_string<CharT>::size_type;
	size_type i1 = 0;
	size_type i2;
	do {
		i2 = s.find_first_of(sep, i1);
		*out++ = s.substr(i1, i2 - i1);
		i1 = i2 + 1;
		// i2 gets s.npos after the last separator
		// lenth of i2-i1 will always go past the end
		// yet that is defined
	} while (i2 != s.npos);
	return out;
}

/*!
 * Splits string on first seperator.
 *
 * \param s a string to split.
 * \param sep char or string that acts as separator to split on.
 * \return The string that has been split off.
 */
template <class CharT, class CharOrStr>
auto split_first(const std::basic_string<CharT>& s, CharOrStr sep)
    -> std::basic_string<CharT>
{
	auto index = s.find(sep);
	return s.substr(0, index);
}
}
#endif

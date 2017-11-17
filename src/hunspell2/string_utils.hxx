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
 * String algorithms not dependent on locale.
 */

#ifndef HUNSPELL_STRING_UTILS_HXX
#define HUNSPELL_STRING_UTILS_HXX

#include <algorithm>
#include <iterator>
#include <locale>
#include <string>
#include <vector>

namespace hunspell {

/*!
 * Splits string on seperator.
 *
 * Consecutive separators are treated as separate and will emit empty strings.
 *
 * \param s a string to split.
 * \param sep char or string that acts as separator to split on.
 * \param out start of the output range where separated strings are appended.
 * \return iterator that indicates the end of the output range.
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

/*!
 * Splits string on seperator, output to vector of strings.
 *
 * See split().
 *
 * \param s in string
 * \param sep in separator
 * \param v out vector. The vector is first cleared.
 */
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
 * Consecutive separators are treated as separate and will emit empty strings.
 *
 * \param s a string to split.
 * \param sep string holding all separators to split on.
 * \param out start of the output range where separated strings are appended.
 * \return iterator that indicates the end of the output range.
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
 * Gets the first token of a splitted string.
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

/*!
 * Splits on whitespace.
 *
 * Consecutive whitespace is treated as single separator. Behaves same as
 * Python's split called without separator argument.
 *
 * \param s a string to split.
 * \param out start of the output range where separated strings are appended.
 * \param loc locale object that takes care of what is whitespace
 * \return iterator that indicates the end of the output range.
 */
template <class CharT, class OutIt>
auto split_on_whitespace(const std::basic_string<CharT>& s, OutIt out,
                         const std::locale& loc = std::locale()) -> OutIt
{
	auto& f = std::use_facet<std::ctype<CharT>>(loc);
	auto isspace = [&](auto& c) { return f.is(std::ctype_base::space, c); };
	auto i1 = begin(s);
	auto endd = end(s);
	do {
		auto i2 = std::find_if_not(i1, endd, isspace);
		if (i2 == endd)
			break;
		auto i3 = std::find_if(i2, endd, isspace);
		*out++ = std::basic_string<CharT>(i2, i3);
		i1 = i3;
	} while (i1 != endd);
	return out;
}

/*!
 * Splits on whitespace, outputs to vector of strings.
 *
 * See split_on_whitespace().
 *
 * \param s in string
 * \param v out vector. The vector is first cleared.
 * \param loc in locale
 */
template <class CharT>
auto split_on_whitespace_v(const std::basic_string<CharT>& s,
                           std::vector<std::basic_string<CharT>>& v,
                           const std::locale& loc = std::locale()) -> void
{
	v.clear();
	split_on_whitespace(s, back_inserter(v), loc);
}
}
#endif

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

#include "locale_utils.hxx"

#include <algorithm>
#include <limits>
#include <type_traits>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace Hunspell {

using namespace std;

namespace {
// const unsigned char shift[] = {0, 6, 0, 0, 0, /**/ 0, 0, 0, 0};
const unsigned char mask[] = {0xff,      0x3f, 0x1f, 0x0f, 0x07,
                              /**/ 0x03, 0x01, 0x00, 0x00};
const unsigned char next_state[][9] = {{0, 4, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 0, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 1, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 2, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 4, 1, 2, 3, 4, 4, 4, 4}};
}

auto utf8_low_level(unsigned char state, char in, char32_t* out,
                    bool* too_short_err) -> unsigned char
{

	unsigned cc = (unsigned char)in; // do not delete the cast
	constexpr auto to_shift = numeric_limits<unsigned>::digits - 8;
#ifdef __GNUC__
	unsigned cc_shifted = cc << to_shift;
	int clz = __builtin_clz(~cc_shifted); // gcc only.
#elif _MSC_VER
	unsigned long cc_shifted = cc << to_shift;
	unsigned long clz;
	BitScanReverse(&clz, ~cc_shifted);
	clz = numeric_limits<unsigned long>::digits - 1 - clz;
#else
	int clz;
	// note the operator presedence
	// all parenthesis are necessary
	if ((cc & 0x80) == 0)
		clz = 0;
	else if ((cc & 0x40) == 0)
		clz = 1;
	else if ((cc & 0x20) == 0)
		clz = 2;
	else if ((cc & 0x10) == 0)
		clz = 3;
	else if ((cc & 0x08) == 0)
		clz = 4;
	else
		clz = 5;
#endif
	if (clz == 1)
		*out <<= 6;
	*out |= cc & mask[clz];
	//*out = (*out << shift[clz]) | (cc & mask[clz]);
	*too_short_err = state >= 1 && state <= 3 && clz != 1;
	return next_state[state][clz];
}

auto validate_utf8(const std::string& s) -> bool
{
	unsigned char state = 0;
	char32_t cp = 0;
	bool err;
	for (auto& c : s) {
		state = utf8_low_level(state, c, &cp, &err);
		if (state == 0) {
			cp = 0;
		}
		if (err || state == 4)
			return false;
	}
	return state == 0;
}

auto decode_utf8(const string& s) -> u32string
{
	unsigned char state = 0;
	char32_t cp = 0;
	bool err;
	constexpr auto REP_CH = U'\uFFFD';
	u32string ret(s.size(), 0);

	auto i = ret.begin();

	for (auto& c : s) {
		state = utf8_low_level(state, c, &cp, &err);
		if (err) {
			*i++ = REP_CH;
		}
		if (state == 0) {
			*i++ = cp;
			cp = 0;
		}
		else if (state == 4) {
			*i++ = REP_CH;
			cp = 0;
		}
	}
	if (state != 0 && state != 4)
		*i++ = REP_CH;
	ret.erase(i, ret.end());
	return ret;
}

auto is_ascii(char c) -> bool { return unsigned char(c) <= 127; }

auto is_all_ascii(const string& s) -> bool
{
	return all_of(s.begin(), s.end(), is_ascii);
}

auto ascii_to_ucs2_skip_invalid(const string& s) -> u16string
{
	u16string ret(s.size(), 0);
	auto i = ret.begin();
	i = copy_if(s.begin(), s.end(), i, is_bmp);
	ret.erase(i, ret.end());
	return ret;
}

template <class CharT>
auto widen_latin1(char c) -> CharT
{
	return (unsigned char)c;
}

auto latin1_to_ucs2(const string& s) -> u16string
{
	u16string ret(s.size(), 0);
	transform(s.begin(), s.end(), ret.begin(), widen_latin1<char16_t>);
}

auto latin1_to_u32(const string& s) -> u32string
{
	u32string ret(s.size(), 0);
	transform(s.begin(), s.end(), ret.begin(), widen_latin1<char32_t>);
}

auto is_bmp(char32_t c) -> bool { return c <= 0xFFFF; }

// auto is_non_bmp(char32_t c) -> bool { return c > 0xFFFF; }

auto is_all_bmp(const u32string& s) -> bool
{
	return all_of(s.begin(), s.end(), is_bmp);
}

auto u32_to_ucs2_skip_non_bmp(const u32string& s) -> u16string
{
	u16string ret(s.size(), 0);
	auto i = ret.begin();
	i = copy_if(s.begin(), s.end(), i, is_bmp);
	ret.erase(i, ret.end());
	return ret;
}
}

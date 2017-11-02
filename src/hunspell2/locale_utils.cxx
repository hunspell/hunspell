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

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace Hunspell {

using namespace std;

#ifdef __GNUC__
#define likely(expr) __builtin_expect(!!(expr), 1)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#endif

namespace {
// const unsigned char shift[] = {0, 6, 0, 0, 0, /**/ 0, 0, 0, 0};
const unsigned char mask[] = {0xff, 0x3f, 0x1f, 0x0f, 0x07, /**/
                              0x03, 0x01, 0x00, 0x00};

// for decoding
const unsigned char next_state[][9] = {{0, 4, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 0, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 1, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 2, 1, 2, 3, 4, 4, 4, 4},
                                       {0, 4, 1, 2, 3, 4, 4, 4, 4}};

// for validating
const unsigned char next_state2[][9] = {{0, 4, 1, 2, 3, 4, 4, 4, 4},
                                        {4, 0, 4, 4, 4, 4, 4, 4, 4},
                                        {4, 1, 4, 4, 4, 4, 4, 4, 4},
                                        {4, 2, 4, 4, 4, 4, 4, 4, 4},
                                        {4, 4, 4, 4, 4, 4, 4, 4, 4}};

auto inline count_leading_ones(unsigned char c)
{
#ifdef __GNUC__
	unsigned cc = c;
	unsigned cc_shifted = cc << (numeric_limits<unsigned>::digits - 8);
	return __builtin_clz(~cc_shifted); // gcc only.
#elif _MSC_VER
	using ulong = unsigned long;
	ulong cc = c;
	ulong cc_shifted = cc << (numeric_limits<ulong>::digits - 8);
	ulong clz;
	BitScanReverse(&clz, ~cc_shifted);
	clz = numeric_limits<ulong>::digits - 1 - clz;
	return clz;
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
	return clz;
#endif
}
}

/*!
 * \brief Finite state transducer used for decoding UTF-8 stream.
 *
 * Formally, the state is a pair (state, cp) and
 * output is a triplet (is_cp_ready, out_cp, too_short_error).
 * This function mutates the state and returns that third element of the output.
 *
 * Initial state is (0, 0).
 *
 *  - is_cp_ready <=> state == 0 so we do not return it since it is
 *    directly derived from the UPDATED state (after the call).
 *  - out_cp = cp for states 0 to 3, otherwise it is FFFD.
 *    We do not return out_cp for same reason.
 *
 * We only return too_short_err.
 *
 * If too_short_err is true, we should generally emit FFFD in the output
 * stream. Then we check the state.
 * Whenever it goes to state = 0, emit out_cp = cp,   reset cp = 0.
 * Whenever it goes to state = 4, emit out_cp = FFFD, reset cp = 0.
 *
 * At the end of the input stream, we should check if the state is 1, 2 or 3
 * which indicates that too_short_err happend. FFFD should be emitted.
 *
 * \param state In out parametar, first part of the state pair.
 * \param cp In out parametar, the code point in the state pair.
 * \param in Input byte.
 * \return true if too short sequence error happend. False otherwise.
 */
auto inline utf8_decode_dfst(unsigned char& state, char32_t& cp,
                             unsigned char in) -> bool
{
	char32_t cc = in; // do not delete the cast
	auto clz = count_leading_ones(in);
	//*cp = (*cp << shift[clz]) | (cc & mask[clz]);
	if (clz == 1)
		cp <<= 6;
	cp |= cc & mask[clz];

	//(state & 3)!=0 <=> state >=1 && state <= 3
	auto too_short_err = (state & 3) && clz != 1;
	state = next_state[state][clz];
	return too_short_err;
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
		err = utf8_decode_dfst(state, cp, c);
		if (unlikely(err)) {
			*i++ = REP_CH;
		}
		if (state == 0) {
			*i++ = cp;
			cp = 0;
		}
		else if (unlikely(state == 4)) {
			*i++ = REP_CH;
			cp = 0;
		}
	}
	if (unlikely(state & 3))
		*i++ = REP_CH;
	ret.erase(i, ret.end());
	return ret;
}

auto inline utf8_validate_dfa(unsigned char state, char in) -> unsigned char
{
	auto clz = count_leading_ones(in);
	return next_state2[state][clz];
}

auto validate_utf8(const std::string& s) -> bool
{
	unsigned char state = 0;
	for (auto& c : s) {
		state = utf8_validate_dfa(state, c);
		if (unlikely(state == 4))
			return false;
	}
	return state == 0;
}

auto is_ascii(char c) -> bool { return (unsigned char)c <= 127; }

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
	return ret;
}

auto latin1_to_u32(const string& s) -> u32string
{
	u32string ret(s.size(), 0);
	transform(s.begin(), s.end(), ret.begin(), widen_latin1<char32_t>);
	return ret;
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

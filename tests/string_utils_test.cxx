/* Copyright 2017 Sander van Geloven
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell-2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Hunspell 2 is based on Hunspell v1 and MySpell.
 * Hunspell v1 is Copyright (C) 2002-2017 Németh László
 * MySpell is Copyright (C) 2002 Kevin Hendricks.
 */

#include "catch.hpp"

#include <iostream>

#include "../src/hunspell2/string_utils.hxx"

using namespace std;
using namespace hunspell;

TEST_CASE("method split", "[string_utils]")
{
	auto in = string(";abc;;qwe;zxc;");
	auto exp = vector<string>{"", "abc", "", "qwe", "zxc", ""};
	auto out = vector<string>();
	split(in, ';', back_inserter(out));
	CHECK(exp == out);
}

TEST_CASE("method split_on_any_of", "[string_utils]")
{
	auto in = string("^abc;.qwe/zxc/");
	auto exp = vector<string>{"", "abc", "", "qwe", "zxc", ""};
	auto out = vector<string>();
	split_on_any_of(in, string(".;^/"), back_inserter(out));
	CHECK(exp == out);
}

TEST_CASE("method split_first", "[string_utils]")
{
	auto in = string("first\tsecond");
	auto first = string("first");
	auto out = split_first(in, '\t');
	CHECK(first == out);

	in = string("first");
	out = split_first(in, '\t');
	CHECK(first == out);

	in = string("\tsecond");
	first = string("");
	out = split_first(in, '\t');
	CHECK(first == out);

	in = string("");
	first = string("");
	out = split_first(in, '\t');
	CHECK(first == out);
}

TEST_CASE("method split_on_whitespace", "[string_utils]")
{
	auto in = string("   qwe ert  \tasd ");
	auto exp = vector<string>{"qwe", "ert", "asd"};
	auto out = vector<string>();
	split_on_whitespace_v(in, out);
	CHECK(exp == out);

	in = string("   \t\r\n  ");
	exp = vector<string>{};
	// out = vector<string>();
	split_on_whitespace_v(in, out);
	CHECK(exp == out);
}

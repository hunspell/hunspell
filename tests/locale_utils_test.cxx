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

#include "../src/hunspell2/locale_utils.hxx"

using namespace std;
using namespace hunspell;

TEST_CASE("Testing locale_utils", "[locale_utils]")
{
	SECTION("Testing decode_utf8")
	{
		CHECK(U"" == decode_utf8(string("")));
		// TODO Omit constructor string("...") without risk for UTF-8?
		CHECK(U"azĳß«" == decode_utf8(string("azĳß«")));
		CHECK(U"日  Ӥ" != decode_utf8(string("Ӥ日本に")));
		// TODO need counter example
	}

	SECTION("Testing validate_utf8")
	{
		CHECK(validate_utf8(string("")));
		CHECK(validate_utf8(string("the brown fox~")));
		CHECK(validate_utf8(string("Ӥ日本に")));
		// TODO need counter example
	}

	SECTION("Testing is_ascii")
	{
		CHECK(is_ascii('a'));
		CHECK(is_ascii('\t'));
		// Results in warning "multi-character character constant
		// [-Wmultichar]"
		// FIXME Add this to Makefile.am for only this source file.
		//        CHECK_FALSE(is_ascii('Ӥ'));
	}

	SECTION("Testing is_all_ascii")
	{
		CHECK(is_all_ascii(string("")));
		CHECK(is_all_ascii(string("the brown fox~")));
		CHECK_FALSE(is_all_ascii(string("brown foxĳӤ")));
	}

	SECTION("Testing ascii_to_ucs2_skip_invalid")
	{
		CHECK(u"ABC" == ascii_to_ucs2_skip_invalid("ABC"));
		CHECK(u"ABC" == ascii_to_ucs2_skip_invalid("ABCĳӤ日"));
	}

	SECTION("Testing latin1_to_ucs2")
	{
		CHECK(u"" == latin1_to_ucs2(string("")));
		CHECK(u"abc" == latin1_to_ucs2(string("abc")));
		// QUESTION Is next line OK?
		CHECK(u"²¿ýþÿ" != latin1_to_ucs2(string("²¿ýþÿ")));
		CHECK(u"Ӥ日本に" != latin1_to_ucs2(string("Ӥ日本に")));
	}

	SECTION("Testing latin1_to_u32")
	{
		CHECK(U"" == latin1_to_u32(string("")));
		CHECK(U"abc~" == latin1_to_u32(string("abc~")));
		// QUESTION Is next line OK?
		CHECK(U"²¿ýþÿ" != latin1_to_u32(string("²¿ýþÿ")));
		CHECK(U"Ӥ日本に" != latin1_to_u32(string("Ӥ日本に")));
	}

	SECTION("Testing is_all_bmp") { CHECK(true == is_all_bmp(U"abcýþÿӤ")); }

	SECTION("Testing u32_to_ucs2_skip_non_bmp")
	{
		CHECK(u"ABC" == u32_to_ucs2_skip_non_bmp(U"ABC"));
	}
}

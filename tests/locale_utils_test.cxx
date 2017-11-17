/* Copyright 2017 Sander van Geloven
 *
 * This file is part of Hunspell.
 *
 * Hunspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hunspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "catch.hpp"

#include <iostream>

#include "../src/hunspell2/locale_utils.hxx"

using namespace std;
using namespace hunspell;

TEST_CASE("header locale_utils", "[locale_utils]")
{
	SECTION("method decode_utf8")
	{
		CHECK(U"" == decode_utf8(string("")));
		// Omit constructor string("...") without risk for UTF-8?
		CHECK(U"azĳß«" == decode_utf8(string("azĳß«")));
		CHECK(U"日  Ӥ" != decode_utf8(string("Ӥ日本に")));
		// need counter example too
	}

	SECTION("method validate_utf8")
	{
		CHECK(validate_utf8(string("")));
		CHECK(validate_utf8(string("the brown fox~")));
		CHECK(validate_utf8(string("Ӥ日本に")));
		// need counter example too
	}

	SECTION("method is_ascii")
	{
		CHECK(is_ascii('a'));
		CHECK(is_ascii('\t'));
		// Results in warning "multi-character character constant
		// [-Wmultichar]"
		// FIXME Add this to Makefile.am for only this source file.
		//        CHECK_FALSE(is_ascii('Ӥ'));
	}

	SECTION("method is_all_ascii")
	{
		CHECK(is_all_ascii(string("")));
		CHECK(is_all_ascii(string("the brown fox~")));
		CHECK_FALSE(is_all_ascii(string("brown foxĳӤ")));
	}

	SECTION("method ascii_to_ucs2_skip_invalid")
	{
		CHECK(u"ABC" == ascii_to_ucs2_skip_invalid("ABC"));
		CHECK(u"ABC" == ascii_to_ucs2_skip_invalid("ABCĳӤ日"));
	}

	SECTION("method latin1_to_ucs2")
	{
		CHECK(u"" == latin1_to_ucs2(string("")));
		CHECK(u"abc" == latin1_to_ucs2(string("abc")));
		// QUESTION Is next line OK?
		CHECK(u"²¿ýþÿ" != latin1_to_ucs2(string("²¿ýþÿ")));
		CHECK(u"Ӥ日本に" != latin1_to_ucs2(string("Ӥ日本に")));
	}

	SECTION("method latin1_to_u32")
	{
		CHECK(U"" == latin1_to_u32(string("")));
		CHECK(U"abc~" == latin1_to_u32(string("abc~")));
		// QUESTION Is next line OK?
		CHECK(U"²¿ýþÿ" != latin1_to_u32(string("²¿ýþÿ")));
		CHECK(U"Ӥ日本に" != latin1_to_u32(string("Ӥ日本に")));
	}

	SECTION("method is_all_bmp") { CHECK(true == is_all_bmp(U"abcýþÿӤ")); }

	SECTION("method u32_to_ucs2_skip_non_bmp")
	{
		CHECK(u"ABC" == u32_to_ucs2_skip_non_bmp(U"ABC"));
	}
}

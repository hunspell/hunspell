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

#include "../src/hunspell2/aff_data.hxx"

using namespace std;
using namespace hunspell;

TEST_CASE("method get_locale_name", "[aff_data]")
{
	CHECK("en_US.ISO8859-1" == get_locale_name("", "", ""));
	CHECK("en_US.UTF-8" == get_locale_name("", "UTF-8", ""));
	CHECK(".ISO8859-1" == get_locale_name("", "", "somefilename"));
	CHECK(".UTF-8" == get_locale_name("", "UTF-8", "somefilename"));
	CHECK("nl_NL.ISO8859-1" == get_locale_name("nl_NL", "", ""));
	CHECK("nl_NL.UTF-8" == get_locale_name("nl_NL", "UTF-8", ""));
	CHECK("nl_NL.ISO8859-1" ==
	      get_locale_name("nl_NL", "", "somefilename"));
	CHECK("nl_NL.UTF-8" ==
	      get_locale_name("nl_NL", "UTF-8", "somefilename"));
}

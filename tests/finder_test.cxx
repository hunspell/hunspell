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

#include "../src/hunspell2/finder.hxx"

using namespace std;
using namespace hunspell;

TEST_CASE("class Finder", "[finder]")
{
	auto f = Finder();
	//	f.add_default_paths();
	//	f.add_libreoffice_paths();
	//	f.add_mozilla_paths();
	//	f.add_apacheopenoffice_paths();
	f.search_dictionaries();
	f.get_all_paths();
	f.get_all_dictionaries();
	//	CHECK(0 == 0);
}

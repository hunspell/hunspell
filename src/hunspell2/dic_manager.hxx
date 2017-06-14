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

#ifndef HUNSPELL_DIC_MANAGER_HXX
#define HUNSPELL_DIC_MANAGER_HXX

#include "aff_manager.hxx"
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace hunspell {

struct dic_data {
	// word and flag vector
	// efficient for short flag vectors
	// for long flag vectors like in Korean dict
	// we should keep pointers to the string in the affix aliases vector
	// for now we will leave it like this
	std::unordered_map<std::string, std::u16string> words;

	// word and morphological data
	// we keep them separate because morph data is generally absent
	std::unordered_map<std::string, std::vector<std::string>> morph_data;

	// methods
	// parses the dic data to hashtable
	auto parse(std::istream& in, const aff_data& aff) -> bool;
};
}

#endif

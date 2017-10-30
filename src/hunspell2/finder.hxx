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

#ifndef HUNSPELL_DIC_FINDER_HXX
#define HUNSPELL_DIC_FINDER_HXX
#include <string>
#include <utility>
#include <vector>

namespace Hunspell {

class Finder {
	using string = std::string;
	template <class T>
	using vector = std::vector<T>;
	template <class T, class U>
	using pair = std::pair<T, U>;

	vector<string> paths;
	vector<pair<string, string>> dictionaries;

      public:
	auto add_default_paths() -> void;
	auto add_mozilla_paths() -> void;
	auto add_libreoffice_paths() -> void;
	auto add_apacheopenoffice_paths() -> void;
	auto search_dictionaries() -> void;

	auto get_all_paths() const -> const vector<string>& { return paths; }
	auto get_all_dictionaries() const -> const vector<pair<string, string>>&
	{
		return dictionaries;
	}

	auto get_dictionary(const string& dict) const -> string;
};
}

#endif

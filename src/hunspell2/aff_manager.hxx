/* Copyright 2016 Dimitrij Mijoski
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
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 */
 
#ifndef HUNSPELL_AFF_MANAGER_HXX
#define HUNSPELL_AFF_MANAGER_HXX

#include <string>
#include <vector>
#include <utility>
#include <iostream>

namespace hunspell
{
enum flag_type_t {single_char, double_char, number, utf_8};

struct aff_data
{
	using string = std::string;
	using u16string = std::u16string;
	template <class T>
	using vector = std::vector<T>;
	template <class T, class U>
	using pair = std::pair<T, U>;
	
	string encoding;
	flag_type_t flag_type;
	bool complex_prefixes;
	string language_code;
	string ignore_chars;
	std::vector<u16string> flag_aliases;
	std::vector<u16string> morphological_aliases;
	
	//suggestion options
	string keyboard_layout;
	string try_chars;
	char16_t nosuggest_flag;
	short max_compound_suggestions;
	short max_ngram_suggestions;
	signed char max_diff_factor;
	bool only_max_diff;
	bool no_split_suggestions;
	bool suggest_with_dots;
	vector<pair<string, string>> replacements;
	vector<pair<string, string>> map_related_chars;
	vector<pair<string, string>> phonetic_replacements;
	char16_t warn_flag;
	bool forbid_warn_flag;
	
	//compouding options
	
	//affix options
	
	//methods
	bool parse(std::istream& in);
};

}

#endif

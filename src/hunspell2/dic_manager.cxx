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

#include "dic_manager.hxx"

#include "string_utils.hxx"
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>

namespace hunspell {

using namespace std;

bool dic_data::parse(std::istream& in, const aff_data& aff)
{
	size_t approximate_size;
	if (in >> approximate_size) {
		words.reserve(approximate_size);
		in.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else {
		return false;
	}
	string line;
	string word;
	string morph;
	vector<string> morphs;
	u16string flags;
	int line_number = 0;
	istringstream ss;
	utf8_to_ucs2_converter cv;
	while (getline(in, line)) {
		ss.str(line);
		ss.clear();
		word.clear();
		morph.clear();
		flags.clear();
		morphs.clear();
		if (line.find('/') == line.npos) {
			//no slash, treat word until first space
			ss >> word;
			if (ss.fail()) {
				//probably all whitespace
				continue;
			}
		}
		else { //slash found, word untill slash
			read_to_slash(ss, word);
			if (ss.fail() || word.empty()) {
				continue;
			}
			if (aff.flag_aliases.empty()) {
				flags = aff.decode_flags(ss, cv);
			}
			else {
				size_t flag_alias_idx;
				ss >> flag_alias_idx;
				if (ss.fail()
				        || flag_alias_idx >
				        aff.flag_aliases.size()) {
					continue;
				}
				flags = aff.flag_aliases[flag_alias_idx-1];
			}
		}
		while (ss >> morph) {
			morphs.push_back(morph);
		}
		words[word].append(flags);
		if (morphs.size()) {
			auto& vec = morph_data[word];
			vec.insert(vec.end(), morphs.begin(), morphs.end());
		}
	}
	for (auto& wd: words) {
		//sort unique flag vectors
		auto& vec = wd.second;
		sort(vec.begin(), vec.end());
		vec.erase(unique(vec.begin(), vec.end()), vec.end());
	}
}
}

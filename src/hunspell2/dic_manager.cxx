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

#include "dic_manager.hxx"

#include "stream_utils.hxx"
#include "string_utils.hxx"
#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

namespace Hunspell {

using namespace std;

auto Dic_data::parse(istream& in, const Aff_data& aff) -> bool
{
	size_t line_number = 1;
	size_t approximate_size;
	istringstream ss;
	string line;

	// locale must be "C", see note in Aff_data::parse()
	in.imbue(locale::classic());
	ss.imbue(locale::classic());
	if (!getline(in, line)) {
		return false;
	}
	if (aff.encoding == "UTF-8" && !validate_utf8(line)) {
		cerr << "Invalid utf in dic file" << endl;
	}
	ss.str(line);
	if (ss >> approximate_size) {
		words.reserve(approximate_size);
	}
	else {
		return false;
	}

	string word;
	string morph;
	vector<string> morphs;
	u16string flags;

	while (getline(in, line)) {
		line_number++;
		ss.str(line);
		ss.clear();
		word.clear();
		morph.clear();
		flags.clear();
		morphs.clear();

		if (aff.encoding == "UTF-8" && !validate_utf8(line)) {
			cerr << "Invalid utf in dic file" << endl;
		}
		if (line.find('/') == line.npos) {
			// no slash, treat word until first space
			ss >> word;
			if (ss.fail()) {
				// probably all whitespace
				continue;
			}
		}
		else { // slash found, word untill slash
			read_to_slash(ss, word);
			if (ss.fail() || word.empty()) {
				continue;
			}
			if (aff.flag_aliases.empty()) {
				flags = aff.decode_flags(ss);
			}
			else {
				size_t flag_alias_idx;
				ss >> flag_alias_idx;
				if (ss.fail() ||
				    flag_alias_idx > aff.flag_aliases.size()) {
					continue;
				}
				flags = aff.flag_aliases[flag_alias_idx - 1];
			}
		}
		parse_morhological_fields(ss, morphs);
		words[word].append(flags);
		if (morphs.size()) {
			auto& vec = morph_data[word];
			vec.insert(vec.end(), morphs.begin(), morphs.end());
		}
	}
	for (auto& wd : words) {
		// sort unique flag vectors
		auto& vec = wd.second;
		sort(vec.begin(), vec.end());
		vec.erase(unique(vec.begin(), vec.end()), vec.end());
	}
	return in.eof(); // success if we reached eof
}
}

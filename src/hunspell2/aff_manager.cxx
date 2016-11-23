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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.	If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 */

#include "aff_manager.hxx"

#include <string>
#include <vector>
#include <istream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <cctype>

#include <iterator>

namespace hunspell {

using namespace std;

namespace {
inline void toupper_ascii(string& s)
{
	for (auto& c: s) c = toupper(c);
}

template <class T, class Func>
void parse_vector_of_T(istream& in, const string& command,
	unordered_map<string, int>& counts, vector<T>& vec, Func factory)
{
	auto dat = counts.find(command);
	if (dat == counts.end()) {
		//first line
		int a;
		in >> a;
		counts[command] = a;
	}
	else if (dat->second) {
		vec.emplace_back();
		if (factory(in, vec.back()) == false) {
			vec.pop_back();
		}
		dat->second--;
	}
}
}

// Expects that there are flags in the stream.
// If there are no flags in the stream (eg, stream is at eof)
// or if the format of the flags is incorrect the stream failbit will be set.
std::u16string decode_flags(std::istream& in, flag_type_t t, bool utf8)
{
	u16string ret;
	switch(t) {
	case single_char:
		if (utf8) {
			
		}
		else {
			string s;
			in >> s;
			ret = u16string(s.begin(), s.end());
		}
		break;
	case double_char:
		if (utf8) {
			
		}
		else {
			string s;
			in >> s;
			auto i = s.begin();
			auto e = s.end();
			if (s.size() | 1) {
				--e;
			}
			for(; i!=e; i+=2) {
				char16_t c1 = *i, c2 = *(i+1);
				ret.push_back(c1 << 8 & c2);
			}
			if (i != s.end()) {
				ret.push_back(*i);
			}
		}
		break;
	case number:
		unsigned short flag;
		if (in >> flag) {
			ret.push_back(flag);
		}else {
			//err no flag at all
			break;
		}
		while (in.peek() == ',') {
			in.get();
			if (in >> flag) {
				ret.push_back(flag);
			} else {
				//err, comma and no number after that
				break;
			}
		}
		
		break;
	}
	
	return ret;
}

bool aff_data::parse(std::istream& in)
{
	unordered_map<string, string*> command_strings = {
		{"SET", &encoding},
		{"LANG", &language_code},
		{"IGNORE", &ignore_chars},

		{"KEY", &keyboard_layout},
		{"TRY", &try_chars},

		{"WORDCHARS", &wordchars}
	};

	unordered_map<string, bool*> command_bools = {
		{"COMPLEXPREFIXES", &complex_prefixes},


		{"ONLYMAXDIFF", &only_max_diff},
		{"NOSPLITSUGS", &no_split_suggestions},
		{"SUGSWITHDOTS", &suggest_with_dots},
		{"FORBIDWARN", &forbid_warn},

		{"COMPOUNDMORESUFFIXES", &compound_more_suffixes},
		{"CHECKCOMPOUNDDUP", &compound_check_up},
		{"CHECKCOMPOUNDREP", &compound_check_rep},
		{"CHECKCOMPOUNDCASE", &compound_check_case},
		{"CHECKCOMPOUNDTRIPLE", &compound_check_triple},
		{"SIMPLIFIEDTRIPLE", &compound_simplified_triple},

		{"FULLSTRIP", &fullstrip},
		{"CHECKSHARPS", &checksharps}
	};

	unordered_map<string, vector<string>*> command_vec_str = {
		{"BREAK", &break_patterns},
		{"COMPOUNDRULE", &compound_rules}
	};

	unordered_map<string, short*> command_shorts = {
		{"MAXCPDSUGS", &max_compound_suggestions},
		{"MAXNGRAMSUGS", &max_ngram_suggestions},
		{"MAXDIFF", &max_diff_factor},

		{"COMPOUNDMIN", &compoud_minimum},
		{"COMPOUNDWORDMAX", &compound_word_max}
	};

	unordered_map<string, vector<pair<string,string>>*> command_vec_pair = {
		{"REP", &replacements},
		{"MAP", &map_related_chars},
		{"PHONE", &phonetic_replacements},
		{"ICONV", &input_conversion},
		{"OCONV", &output_conversion}
	};

	unordered_map<string, char16_t*> command_flag = {
		{"NOSUGGEST", &nosuggest_flag},
		{"WARN", &warn_flag},

		{"COMPOUNDFLAG", &compound_flag},
		{"COMPOUNDBEGIN", &compound_begin_flag},
		{"COMPOUNDLAST", &compound_last_flag},
		{"COMPOUNDMIDDLE", &compound_middle_flag},
		{"ONLYINCOMPOUND", &compound_onlyin_flag},
		{"COMPOUNDPERMITFLAG", &compound_permit_flag},
		{"COMPOUNDFORBIDFLAG", &compound_forbid_flag},
		{"COMPOUNDROOT", &compound_root_flag},
		{"FORCEUCASE", &compound_force_uppercase},

		{"CIRCUMFIX", &circumfix_flag},
		{"FORBIDDENWORD", &forbiddenword_flag},
		{"KEEPCASE", &keepcase_flag},
		{"NEEDAFFIX", &need_affix_flag},
		{"SUBSTANDARD", &substandard_flag}
	};

	//keeps count for each vector
	unordered_map<string, int> cmd_with_vec_cnt;
	string line;
	string command;
	flag_type = flag_type_t::single_char;
	while (getline(in, line)) {
		istringstream ss(line);
		ss >> ws;
		if (ss.eof() || ss.peek() == '#') {
				continue; //skip comment or empty lines
		}
		ss >> command;
		toupper_ascii(command);
		ss >> ws;
		if (command == "PFX" || command == "SFX") {

		}
		else if (command_strings.count(command)) {
			auto& str = *command_strings[command];
			ss >> str;
			if (&str == &encoding){
				toupper_ascii(str);
			}
		}
		else if (command_bools.count(command)) {
			*command_bools[command] = true;
		}
		else if (command_shorts.count(command)) {
			ss >> *command_shorts[command];
		}
		else if (command_flag.count(command)) {
			//parse flag
		}
		else if (command_vec_str.count(command)) {
			auto& vec = *command_vec_str[command];
			auto func = [&](istream& in, string& p) {
				return (bool)(in >> p);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt,
				vec, func);
		}
		else if (command_vec_pair.count(command)) {
			auto& vec = *command_vec_pair[command];
			auto func = [&](istream& in, pair<string, string>& p) {
				return (bool)(in >> p.first >> p.second);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt,
				vec, func);
		}
		else if (command == "FLAG") {
			string p;
			ss >> p;
			toupper_ascii(p);
			if (p == "LONG") flag_type = double_char;
			else if (p == "NUM") flag_type = number;
			//else if (p == "UTF-8") flag_type = utf_8;
		}
	//{"AF", &flag_aliases},
	//{"AM", &morphological_aliases},
	//{"FLAG", &flag_type},

	//{"CHECKCOMPOUNDPATTERN", &compound_check_patterns},
	//{"COMPOUNDSYLLABLE", &compound_syllable_max}, //compound_syllable_vowels
	//{"SYLLABLENUM", &compound_syllable_num},
	//{"PFX", &prefixes},
	//{"SFX", &suffixes},
	}

	return in.eof(); // success if we reached eof
}

}

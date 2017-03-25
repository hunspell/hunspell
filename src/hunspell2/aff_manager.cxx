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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell-2.	If not, see <http://www.gnu.org/licenses/>.
 *
 * Hunspell 2 is based on Hunspell v1 and MySpell.
 * Hunspell v1 is Copyright (C) 2002-2017 Németh László
 * MySpell is Copyright (C) 2002 Kevin Hendricks.
 */

#include "aff_manager.hxx"

#include "string_utils.hxx"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace hunspell {

using namespace std;

namespace {

template <class T, class Func>
auto parse_vector_of_T(istream& in, const string& command,
                       unordered_map<string, int>& counts, vector<T>& vec,
                       Func parseLineFunc) -> void
{
	auto dat = counts.find(command);
	if (dat == counts.end()) {
		// first line
		int a;
		in >> a;
		if (!in || a < 0) {
			a = 0; // err
		}
		counts[command] = a;
	}
	else if (dat->second) {
		vec.emplace_back();
		parseLineFunc(in, vec.back());
		if (in.fail()) {
			vec.pop_back();
		}
		dat->second--;
	}
	else {
		cerr << "Hunspell warning: extra entries of " << command
		     << '\n';
	}
}

// Expects that there are flags in the stream.
// If there are no flags in the stream (eg, stream is at eof)
// or if the format of the flags is incorrect the stream failbit will be set.
auto decode_flags(std::istream& in, flag_type_t t, utf8_to_ucs2_converter& cv)
    -> u16string
{
	string s;
	u16string ret;
	// utf8 to ucs-2 converter. flags can be only in BPM
	// wstring_convert<codecvt_utf8<char16_t>,char16_t> cv;
	switch (t) {
	case single_char_flag:
		in >> s;
		ret.resize(s.size());
		transform(s.begin(), s.end(), ret.begin(),
		          cast_lambda<unsigned char>());
		break;
	case double_char_flag: {
		in >> s;
		auto i = s.begin();
		auto e = s.end();
		if (s.size() & 1) {
			--e;
		}
		for (; i != e; i += 2) {
			char16_t c1 = (unsigned char)*i;
			char16_t c2 = (unsigned char)*(i + 1);
			ret.push_back((c1 << 8) | c2);
		}
		if (i != s.end()) {
			ret.push_back((unsigned char)*i);
		}
		break;
	}
	case number_flag:
		unsigned short flag;
		if (in >> flag) {
			ret.push_back(flag);
		}
		else {
			// err no flag at all
			cerr << "Hunspell error: missing flag\n";
			break;
		}
		// peek can set failbit
		while (in.good() && in.peek() == ',') {
			in.get();
			if (in >> flag) {
				ret.push_back(flag);
			}
			else {
				// err, comma and no number after that
				cerr << "Hunspell error: long flag, no number "
				        "after comma\n";
				break;
			}
		}

		break;
	case utf8_flag:
		ret = cv.from_bytes(s);
		break;
	}
	return ret;
}

auto parse_affix(istream& ss, string& command, vector<aff_data::affix>& vec,
                 unordered_map<string, pair<bool, int>>& cmd_affix,
                 utf8_to_ucs2_converter& cv, aff_data& thiss) -> void
{
	char16_t f = thiss.decode_single_flag(ss, cv);
	if (f == 0) {
		// err
		return;
	}
	char f1 = f & 0xff;
	char f2 = (f >> 8) & 0xff;
	command.push_back(f1);
	command.push_back(f2);
	auto dat = cmd_affix.find(command);
	// note: the current affix parser does not allow the same flag
	// to be used once with cross product and again witohut
	// one flag is tied to one cross product value
	if (dat == cmd_affix.end()) {
		char cross_char; // 'Y' or 'N'
		int cnt;
		ss >> cross_char >> cnt;
		bool cross = cross_char == 'Y';
		if (!ss || cnt < 0) {
			cnt = 0; // err
		}
		cmd_affix[command] = make_pair(cross, cnt);
	}
	else if (dat->second.second) {
		vec.emplace_back();
		auto& elem = vec.back();
		elem.flag = f;
		elem.cross_product = dat->second.first;
		ss >> elem.stripping;
		if (read_to_slash_or_space(ss, elem.affix)) {
			elem.new_flags = thiss.decode_flags(ss, cv);
		}
		ss >> elem.condition;
		if (ss.fail()) {
			vec.pop_back();
		}
		else {
			parse_morhological_fields(ss,
			                          elem.morphological_fields);
		}
		dat->second.second--;
	}
	else {
		cerr << "Hunspell warning: extra entries of "
		     << command.substr(0, 3) << '\n';
	}
}
}

auto aff_data::decode_flags(istream& in, utf8_to_ucs2_converter& cv) const
    -> u16string
{
	return hunspell::decode_flags(in, flag_type, cv);
}

auto aff_data::decode_single_flag(istream& in, utf8_to_ucs2_converter& cv) const
    -> char16_t
{
	auto flags = decode_flags(in, cv);
	if (flags.size()) {
		return flags.front();
	}
	return 0;
}

auto aff_data::parse(std::istream& in) -> bool
{
	unordered_map<string, string*> command_strings = {
	    {"SET", &encoding},        {"LANG", &language_code},
	    {"IGNORE", &ignore_chars},

	    {"KEY", &keyboard_layout}, {"TRY", &try_chars},

	    {"WORDCHARS", &wordchars}};

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
	    {"CHECKSHARPS", &checksharps}};

	unordered_map<string, vector<string>*> command_vec_str = {
	    {"BREAK", &break_patterns},
	    {"MAP", &map_related_chars}, // maybe add special parsing code
	    {"COMPOUNDRULE", &compound_rules}};

	unordered_map<string, short*> command_shorts = {
	    {"MAXCPDSUGS", &max_compound_suggestions},
	    {"MAXNGRAMSUGS", &max_ngram_suggestions},
	    {"MAXDIFF", &max_diff_factor},

	    {"COMPOUNDMIN", &compoud_minimum},
	    {"COMPOUNDWORDMAX", &compound_word_max}};

	unordered_map<string, vector<pair<string, string>>*> command_vec_pair =
	    {{"REP", &replacements},
	     {"PHONE", &phonetic_replacements},
	     {"ICONV", &input_conversion},
	     {"OCONV", &output_conversion}};

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
	    {"SUBSTANDARD", &substandard_flag}};

	// keeps count for each vector
	unordered_map<string, int> cmd_with_vec_cnt;
	unordered_map<string, pair<bool, int>> cmd_affix;
	utf8_to_ucs2_converter cv;
	string line;
	string command;
	int line_number = 0;
	flag_type = single_char_flag;
	while (getline(in, line)) {
		line_number++;
		istringstream ss(line);
		ss >> ws;
		if (ss.eof() || ss.peek() == '#') {
			continue; // skip comment or empty lines
		}
		ss >> command;
		toupper_ascii(command);
		ss >> ws;
		if (command == "PFX" || command == "SFX") {
			auto& vec = command[0] == 'P' ? prefixes : suffixes;
			parse_affix(ss, command, vec, cmd_affix, cv, *this);
		}
		else if (command_strings.count(command)) {
			auto& str = *command_strings[command];
			ss >> str;
			if (&str == &encoding) {
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
			*command_flag[command] = decode_single_flag(ss, cv);
		}
		else if (command_vec_str.count(command)) {
			auto& vec = *command_vec_str[command];
			auto func = [&](istream& in, string& p) { in >> p; };
			parse_vector_of_T(ss, command, cmd_with_vec_cnt, vec,
			                  func);
		}
		else if (command_vec_pair.count(command)) {
			auto& vec = *command_vec_pair[command];
			auto func = [&](istream& in, pair<string, string>& p) {
				in >> p.first >> p.second;
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt, vec,
			                  func);
		}
		else if (command == "FLAG") {
			string p;
			ss >> p;
			toupper_ascii(p);
			if (p == "LONG")
				flag_type = double_char_flag;
			else if (p == "NUM")
				flag_type = number_flag;
			else if (p == "UTF-8")
				flag_type = utf8_flag;
		}
		else if (command == "AF") {
			auto& vec = flag_aliases;
			auto func = [&](istream& inn, u16string& p) {
				p = decode_flags(inn, cv);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt, vec,
			                  func);
		}
		else if (command == "AM") {
			auto& vec = morphological_aliases;
			parse_vector_of_T(ss, command, cmd_with_vec_cnt, vec,
			                  parse_morhological_fields);
		}
		else if (command == "CHECKCOMPOUNDPATTERN") {
			auto& vec = compound_check_patterns;
			auto func = [&](istream& in,
			                compound_check_pattern& p) {
				if (read_to_slash_or_space(in, p.end_chars)) {
					p.end_flag = decode_single_flag(in, cv);
				}
				if (read_to_slash_or_space(in, p.begin_chars)) {
					p.begin_flag =
					    decode_single_flag(in, cv);
				}
				if (in.fail()) {
					return;
				}
				in >> p.replacement;
				reset_failbit_istream(in);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt, vec,
			                  func);
		}
		else if (command == "COMPOUNDSYLLABLE") {
			ss >> compound_syllable_max >> compound_syllable_vowels;
		}
		else if (command == "SYLLABLENUM") {
			compound_syllable_num = decode_flags(ss, cv);
		}
		if (ss.fail()) {
			cerr << "Hunspell aff error in line " << line_number
			     << ": " << line << endl;
		}
	}

	return in.eof(); // success if we reached eof
}
}

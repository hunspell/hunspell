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
#include <algorithm>

#include <cctype>


namespace hunspell {

using namespace std;

namespace {
inline void toupper_ascii(string& s)
{
	for (auto& c: s) c = toupper(c);
}

void reset_failbit_istream(istream& in)
{
	in.clear(in.rdstate() & ~in.failbit);
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
		if (!in || a < 0) {
			a = 0; //err
		}
		counts[command] = a;
	}
	else if (dat->second) {
		vec.emplace_back();
		factory(in, vec.back());
		if (in.fail()) {
			vec.pop_back();
		}
		dat->second--;
	}
}

template <class InStrType>
void decode_flags_double_char(InStrType s, u16string& out)
{
	auto i = s.begin();
	auto e = s.end();
	if (s.size() | 1) {
		--e;
	}
	for(; i!=e; i+=2) {
		char16_t c1 = (unsigned char)*i;
		char16_t c2 = (unsigned char)*(i+1);
		out.push_back((c1 << 8) | c2);
	}
	if (i != s.end()) {
		out.push_back((unsigned char)*i);
	}
}

template <class To>
struct cast_lambda
{
	template <class From>
	To operator()(From& f) const
	{
		return static_cast<To>(f);
	}
};


// Expects that there are flags in the stream.
// If there are no flags in the stream (eg, stream is at eof)
// or if the format of the flags is incorrect the stream failbit will be set.
std::u16string decode_flags(std::istream& in, flag_type_t t, bool utf8,
                            aff_data::utf8_to_ucs2_converter& cv)
{
	string s;
	u16string ret;
	//utf8 to ucs-2 converter. flags can be only in BPM
	//wstring_convert<codecvt_utf8<char16_t>,char16_t> cv;
	switch (t) {
	case single_char:
		in >> s;
		if (utf8) {
			ret = cv.from_bytes(s);
		} else {
			ret.resize(s.size());
			transform(s.begin(), s.end(), ret.begin(),
			          cast_lambda<unsigned char>());
		}
		break;
	case double_char:
		in >> s;
		if (utf8) {
			u16string tmp = cv.from_bytes(s);
			decode_flags_double_char(tmp, ret);
		} else {
			decode_flags_double_char(s, ret);
		}
		break;
	case number:
		unsigned short flag;
		if (in >> flag) {
			ret.push_back(flag);
		} else {
			//err no flag at all
			break;
		}
		//peek can set failbit
		while (in.good() && in.peek() == ',') {
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

bool read_until_slash(istream& in, string& out)
{
	in >> ws;
	int c;
	bool readSomething = false;
	while ((c = in.get()) != istream::traits_type::eof()
	        && !isspace((char)c, in.getloc()) && c != '/') {
		out.push_back(c);
		readSomething = true;
	}
	bool slash = c == '/';
	if (readSomething || slash) {
		reset_failbit_istream(in);
	}
	return slash;
}

void parse_affix(istream& ss, string& command, vector<aff_data::affix>& vec,
                 unordered_map<string, pair<bool, int>>& cmd_affix,
                 aff_data::utf8_to_ucs2_converter& cv,
                 aff_data& thiss)
{
	char16_t f = thiss.decode_single_flag(ss, cv);
	if (f == 0) {
		//err
		return;
	}
	char f1 = f & 0xff;
	char f2 = (f>>8) & 0xff;
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
			cnt = 0; //err
		}
		cmd_affix[command] = make_pair(cross, cnt);
	} else if (dat->second.second) {
		vec.emplace_back();
		auto& elem = vec.back();
		elem.flag = f;
		elem.cross_product = dat->second.first;
		ss >> elem.stripping;
		if (read_until_slash(ss, elem.affix)) {
			elem.new_flags = thiss.decode_flags(ss, cv);
		}
		ss >> elem.condition;
		string morph;
		if (ss.fail()) {
			vec.pop_back();
		} else {
			while (ss >> morph) {
				elem.morphological_fields.push_back(morph);
			}
			reset_failbit_istream(ss);
		}
		dat->second.second--;
	}
}

}

u16string aff_data::decode_flags(istream& in, utf8_to_ucs2_converter& cv)
{
	return hunspell::decode_flags(in, flag_type, encoding == "UTF-8", cv);
}

char16_t aff_data::decode_single_flag(istream& in, utf8_to_ucs2_converter& cv)
{
	auto flags = decode_flags(in, cv);
	if (flags.size()) {
		return flags.front();
	}
	return 0;
}

//u16string aff_data::decode_flags(istream& in)
//{
//	utf8_to_ucs2_converter cv;
//	return decode_flags(in, cv);
//}

//char16_t aff_data::decode_single_flag(istream& in)
//{
//	utf8_to_ucs2_converter cv;
//	return decode_single_flag(in, cv);
//}

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
		{"MAP", &map_related_chars}, //maybe add special parsing code
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
	unordered_map<string, pair<bool, int>> cmd_affix;
	utf8_to_ucs2_converter cv;
	string line;
	string command;
	int line_number = 0;
	flag_type = flag_type_t::single_char;
	while (getline(in, line)) {
		line_number++;
		istringstream ss(line);
		ss >> ws;
		if (ss.eof() || ss.peek() == '#') {
			continue; //skip comment or empty lines
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
			auto func = [&](istream& in, string& p) {
				in >> p;
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt,
			                  vec, func);
		}
		else if (command_vec_pair.count(command)) {
			auto& vec = *command_vec_pair[command];
			auto func = [&](istream& in, pair<string, string>& p) {
				in >> p.first >> p.second;
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
		else if (command == "AF") {
			auto& vec = flag_aliases;
			auto func = [&](istream& inn, u16string& p) {
				p = decode_flags(inn, cv);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt,
			                  vec, func);
		}
		else if (command == "AM") {
//{"AM", &morphological_aliases},
		}
		else if (command == "CHECKCOMPOUNDPATTERN") {
			auto& vec = compound_check_patterns;
			auto func =
			[&](istream& in, compound_check_pattern& p) {
				if (read_until_slash(in, p.end_chars)) {
					p.end_flag = decode_single_flag(in, cv);
				}
				if (read_until_slash(in, p.begin_chars)) {
					p.begin_flag =
					    decode_single_flag(in, cv);
				}
				if (in.fail()) {
					return;
				}
				in >> p.replacement;
				reset_failbit_istream(in);
			};
			parse_vector_of_T(ss, command, cmd_with_vec_cnt,
			                  vec, func);
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

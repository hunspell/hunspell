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
 * along with Hunspell-2. If not, see <http://www.gnu.org/licenses/>.
 *
 * Hunspell 2 is based on Hunspell v1 and MySpell.
 * Hunspell v1 is Copyright (C) 2002-2017 Németh László
 * MySpell is Copyright (C) 2002 Kevin Hendricks.
 */

#include "aff_data.hxx"

#include "stream_utils.hxx"
#include "string_utils.hxx"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <boost/locale.hpp>

namespace hunspell {

using namespace std;

template <class T, class Func>
auto parse_vector_of_T(/* in */ istream& in, /* in */ size_t line_num,
                       /* in */ const string& command,
                       /* in out */ unordered_map<string, int>& counts,
                       /* in out */ vector<T>& vec,
                       /* in */ Func parseLineFunc) -> void
{
	auto dat = counts.find(command);
	if (dat == counts.end()) {
		// first line
		size_t a;
		in >> a;
		if (in.fail()) {
			a = 0; // err
			cerr << "Hunspell error: a vector command (series of "
			        "of similar commands) has no count. Ignoring "
			        "all of them.\n";
		}
		counts[command] = a;
	}
	else if (dat->second) {
		vec.emplace_back();
		parseLineFunc(in, vec.back());
		if (in.fail()) {
			vec.pop_back();
			cerr << "Hunspell error: single entry of a vector "
			        "command (series of "
			        "of similar commands) is invalid.\n";
		}
		dat->second--;
	}
	else {
		cerr << "Hunspell warning: extra entries of " << command
		     << '\n';
		cerr << "Hunspell warning in line " << line_num << endl;
	}
}

// Expects that there are flags in the stream.
// If there are no flags in the stream (eg, stream is at eof)
// or if the format of the flags is incorrect the stream failbit will be set.
auto decode_flags(/* in */ istream& in, /* in */ size_t line_num,
                  /* in */ Flag_type_t t, /* in */ const Encoding& enc)
    -> u16string
{
	string s;
	u16string ret;
	const auto err_message = "Hunspell warning: bytes above 127 in UTF-8 "
	                         "stream should not be treated alone as "
	                         "flags. Please update dictionary to use "
	                         "FLAG UTF-8 and make the file valid UTF-8.";
	switch (t) {
	case FLAG_SINGLE_CHAR:
		in >> s;
		if (in.fail()) {
			// err no flag at all
			cerr << "Hunspell error: missing flag\n";
			break;
		}
		if (enc.is_utf8() && !is_all_ascii(s)) {
			cerr << err_message << '\n';
			cerr << "Hunspell warning in line " << line_num
			     << "\n\n";
			// This error will be triggered in Hungarian.
			// Version 1 passed this, it just read a
			// single byte even if the stream utf-8.
			// Hungarian dictionary explited this
			// bug/feature, resulting it's file to be
			// mixed utf-8 and latin2.
			// In v2 we will make this to work, with
			// a warning.
		}
		ret = latin1_to_ucs2(s);
		break;
	case FLAG_DOUBLE_CHAR: {
		in >> s;
		if (in.fail()) {
			// err no flag at all
			cerr << "Hunspell error: missing flag\n";
			break;
		}
		if (enc.is_utf8() && !is_all_ascii(s)) {
			cerr << err_message << endl;
			cerr << "Hunspell warning in line " << line_num << endl;
		}
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
	case FLAG_NUMBER:
		unsigned short flag;
		in >> flag;
		if (in.fail()) {
			// err no flag at all
			cerr << "Hunspell error: missing flag\n";
			break;
		}
		ret.push_back(flag);
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
	case FLAG_UTF8: {
		in >> s;
		if (!enc.is_utf8()) {
			// err
			cerr << "Hunspell error: File encoding is not UTF-8, "
			        "yet flags are\n";
		}
		if (in.fail()) {
			// err no flag at all
			cerr << "Hunspell error: missing flag\n";
			break;
		}
		auto u32flags = decode_utf8(s);
		if (!is_all_bmp(u32flags)) {
			cerr << "Hunspell warning: flags must be in BMP. "
			        "Skipping non-BMP\n"
			     << "Hunspell warning in line " << line_num << endl;
		}
		ret = u32_to_ucs2_skip_non_bmp(u32flags);
		break;
	}
	}
	return ret;
}

auto decode_single_flag(/* in */ istream& in, /* in */ size_t line_num,
                        /* in */ Flag_type_t t, /* in */ Encoding enc)
    -> char16_t
{
	auto flags = decode_flags(in, line_num, t, enc);
	if (flags.size()) {
		return flags.front();
	}
	return 0;
}

auto parse_affix(/* in */ istream& in, /* in */ size_t line_num,
                 /* in out */ string& command,
                 /* in */ Flag_type_t t, /* in */ Encoding enc,
                 /* in out */ vector<Aff_data::affix>& vec,
                 /* in out */ unordered_map<string, pair<bool, int>>& cmd_affix)
    -> void
{
	char16_t f = decode_single_flag(in, line_num, t, enc);
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
		size_t cnt;
		in >> cross_char >> cnt;
		bool cross = cross_char == 'Y';
		if (in.fail()) {
			cnt = 0; // err
			cerr << "Hunspell error: a SFX/PFX header command is "
			        "invalid. Missing count or cross product\n";
		}
		cmd_affix[command] = make_pair(cross, cnt);
	}
	else if (dat->second.second) {
		vec.emplace_back();
		auto& elem = vec.back();
		elem.flag = f;
		elem.cross_product = dat->second.first;
		in >> elem.stripping;
		if (read_to_slash_or_space(in, elem.affix)) {
			elem.new_flags = decode_flags(in, line_num, t, enc);
		}
		in >> elem.condition;
		if (in.fail()) {
			vec.pop_back();
		}
		else {
			parse_morhological_fields(in,
			                          elem.morphological_fields);
		}
		dat->second.second--;
	}
	else {
		cerr << "Hunspell warning: extra entries of "
		     << command.substr(0, 3) << '\n'
		     << "Hunspell warning in line " << line_num << endl;
	}
}

auto parse_flag_type(/* in */ istream& in, /* in */ size_t line_num,
                     /* in out */ Flag_type_t& flag_type) -> void
{
	(void)line_num;
	string p;
	in >> p;
	toupper(p, in.getloc());
	if (p == "LONG")
		flag_type = FLAG_DOUBLE_CHAR;
	else if (p == "NUM")
		flag_type = FLAG_NUMBER;
	else if (p == "UTF-8")
		flag_type = FLAG_UTF8;
	else
		cerr << "Hunspell error: unknown FLAG type\n";
}

auto parse_morhological_fields(/* in */ istream& in,
                               /* in out */ vector<string>& vecOut) -> void
{
	if (!in.good()) {
		return;
	}

	std::string morph;
	while (in >> morph) {
		vecOut.push_back(morph);
	}
	reset_failbit_istream(in);
}

auto Aff_data::decode_flags(istream& in, size_t line_num) const -> u16string
{
	return hunspell::decode_flags(in, line_num, flag_type, encoding);
}

auto Aff_data::decode_single_flag(istream& in, size_t line_num) const
    -> char16_t
{
	return hunspell::decode_single_flag(in, line_num, flag_type, encoding);
}

auto get_locale_name(string lang, string enc, const string& filename) -> string
{
	if (enc.empty())
		enc = "ISO8859-1";
	if (lang.empty()) {
		if (filename.empty()) {
			lang = "en_US";
		}
	}
	return lang + "." + enc;
}

/*!
 * Returns a cleaned version of a word by removing any leading spaces and
 * trailing periods. It also returns the count the removed trailing periods and
 * the capitalization type of the cleaned word.
 *
 * \param dst destination holding the cleaned word.
 * \param src source holding the word to clean.
 * \param captype capitalisation type that has been detected.
 * \param abbrev number of removed trainling periods.
 * \return length of the cleaned word.
 */
auto clean_word(string& dst, const string& src, size_t* captype, size_t* abbrev)
    -> int
{
	// TODO implement
	dst = "blah";
	*captype = CAP_NO;
	*abbrev = (size_t)3;
	return 4;
}

/*!
 * Returns capitalization type for a word.
 *
 * \param word word for which capitalization is determined.
 * \return capitalization type.
 */
auto get_cap_type(const std::string& word) -> int
{
	// TODO implement
	return CAP_NO;
}

auto Aff_data::parse(istream& in) -> bool
{
	unordered_map<string, string*> command_strings = {
	    {"LANG", &language_code},
	    {"IGNORE", &ignore_chars},

	    {"KEY", &keyboard_layout},
	    {"TRY", &try_chars},

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
	auto cmd_with_vec_cnt = unordered_map<string, int>();
	auto cmd_affix = unordered_map<string, pair<bool, int>>();
	auto line = string();
	auto command = string();
	size_t line_num = 0;
	auto ss = istringstream();

	boost::locale::generator locale_generator;
	// auto loc = locale_generator("en_US.us-ascii");
	auto loc = locale::classic();
	// while parsing, the streams must have plain ascii locale without
	// any special number separator otherwise istream >> int might fail
	// due to thousands separator.
	// "C" locale can be used assuming it is US-ASCII,
	// but also boost en_US.us-ascii will do, avoiding any assumptions
	in.imbue(loc);
	ss.imbue(loc);

	flag_type = FLAG_SINGLE_CHAR;
	while (getline(in, line)) {
		line_num++;

		if (encoding.is_utf8() && !validate_utf8(line)) {
			cerr << "Hunspell warning: invalid utf in aff file\n";
			// Hungarian will triger this, contains mixed
			// utf-8 and latin2. See note in decode_flags().
		}

		ss.str(line);
		ss.clear();
		ss >> ws;
		if (ss.eof() || ss.peek() == '#') {
			continue; // skip comment or empty lines
		}
		ss >> command;
		toupper(command, ss.getloc());
		ss >> ws;
		if (command == "PFX" || command == "SFX") {
			auto& vec = command[0] == 'P' ? prefixes : suffixes;
			parse_affix(ss, line_num, command, flag_type, encoding,
			            vec, cmd_affix);
		}
		else if (command_strings.count(command)) {
			auto& str = *command_strings[command];
			if (str.empty())
				ss >> str;
			else
				cerr << "Hunspell warning: "
				        "Setting "
				     << command
				     << " more than once. Ignoring.\n"
				     << "Hunspell warning in line " << line_num
				     << '\n';
		}
		else if (command_bools.count(command)) {
			*command_bools[command] = true;
		}
		else if (command_shorts.count(command)) {
			ss >> *command_shorts[command];
		}
		else if (command_flag.count(command)) {
			*command_flag[command] =
			    decode_single_flag(ss, line_num);
		}
		else if (command_vec_str.count(command)) {
			auto& vec = *command_vec_str[command];
			auto func = [&](istream& in, string& p) { in >> p; };
			parse_vector_of_T(ss, line_num, command,
			                  cmd_with_vec_cnt, vec, func);
		}
		else if (command_vec_pair.count(command)) {
			auto& vec = *command_vec_pair[command];
			auto func = [&](istream& in, pair<string, string>& p) {
				in >> p.first >> p.second;
			};
			parse_vector_of_T(ss, line_num, command,
			                  cmd_with_vec_cnt, vec, func);
		}
		else if (command == "SET") {
			if (encoding.empty()) {
				auto str = string();
				ss >> str;
				encoding = str;
			}
			else {
				cerr << "Hunspell warning: "
				        "Setting "
				     << command
				     << " more than once. Ignoring.\n"
				     << "Hunspell warning in line " << line_num
				     << '\n';
			}
		}
		else if (command == "FLAG") {
			parse_flag_type(ss, line_num, flag_type);
		}
		else if (command == "AF") {
			auto& vec = flag_aliases;
			auto func = [&](istream& inn, u16string& p) {
				p = decode_flags(inn, line_num);
			};
			parse_vector_of_T(ss, line_num, command,
			                  cmd_with_vec_cnt, vec, func);
		}
		else if (command == "AM") {
			auto& vec = morphological_aliases;
			parse_vector_of_T(ss, line_num, command,
			                  cmd_with_vec_cnt, vec,
			                  parse_morhological_fields);
		}
		else if (command == "CHECKCOMPOUNDPATTERN") {
			auto& vec = compound_check_patterns;
			auto func = [&](istream& in,
			                compound_check_pattern& p) {
				if (read_to_slash_or_space(in, p.end_chars)) {
					p.end_flag =
					    decode_single_flag(in, line_num);
				}
				if (read_to_slash_or_space(in, p.begin_chars)) {
					p.begin_flag =
					    decode_single_flag(in, line_num);
				}
				if (in.fail()) {
					return;
				}
				in >> p.replacement;
				reset_failbit_istream(in);
			};
			parse_vector_of_T(ss, line_num, command,
			                  cmd_with_vec_cnt, vec, func);
		}
		else if (command == "COMPOUNDSYLLABLE") {
			ss >> compound_syllable_max >> compound_syllable_vowels;
		}
		else if (command == "SYLLABLENUM") {
			compound_syllable_num = decode_flags(ss, line_num);
		}
		if (ss.fail()) {
			cerr << "Hunspell aff error in line " << line_num
			     << ": " << line << endl;
		}
	}
	locale_aff = locale_generator(get_locale_name(language_code, encoding));
	cerr.flush();
	return in.eof(); // success if we reached eof
}
}

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

#ifndef HUNSPELL_AFF_MANAGER_HXX
#define HUNSPELL_AFF_MANAGER_HXX

#include "string_utils.hxx"
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace hunspell {

enum flag_type_t { single_char_flag, double_char_flag, number_flag, utf8_flag };

struct aff_data {
	using string = std::string;
	using u16string = std::u16string;
	using istream = std::istream;
	template <class T>
	using vector = std::vector<T>;
	template <class T, class U>
	using pair = std::pair<T, U>;

	string encoding;
	flag_type_t flag_type;
	bool complex_prefixes;
	string language_code;
	string ignore_chars;
	vector<u16string> flag_aliases;
	vector<vector<string>> morphological_aliases;

	// suggestion options
	string keyboard_layout;
	string try_chars;
	char16_t nosuggest_flag;
	short max_compound_suggestions;
	short max_ngram_suggestions;
	short max_diff_factor;
	bool only_max_diff;
	bool no_split_suggestions;
	bool suggest_with_dots;
	vector<pair<string, string>> replacements;
	vector<string> map_related_chars;
	vector<pair<string, string>> phonetic_replacements;
	char16_t warn_flag;
	bool forbid_warn;

	// compouding options
	vector<string> break_patterns;
	vector<string> compound_rules;
	short compoud_minimum;
	char16_t compound_flag;
	char16_t compound_begin_flag;
	char16_t compound_last_flag;
	char16_t compound_middle_flag;
	char16_t compound_onlyin_flag;
	char16_t compound_permit_flag;
	char16_t compound_forbid_flag;
	bool compound_more_suffixes;
	char16_t compound_root_flag;
	short compound_word_max;
	bool compound_check_up;
	bool compound_check_rep;
	bool compound_check_case;
	bool compound_check_triple;
	bool compound_simplified_triple;

	struct compound_check_pattern {
		string end_chars;
		char16_t end_flag;
		string begin_chars;
		char16_t begin_flag;
		string replacement;
	};
	vector<compound_check_pattern> compound_check_patterns;
	char16_t compound_force_uppercase;
	short compound_syllable_max;
	string compound_syllable_vowels;
	u16string compound_syllable_num;

	// affix creation
	struct affix {
		char16_t flag;
		bool cross_product;
		string stripping;
		string affix;
		u16string new_flags;
		string condition;
		vector<string> morphological_fields;
	};
	vector<affix> prefixes;
	vector<affix> suffixes;

	// others
	char16_t circumfix_flag;
	char16_t forbiddenword_flag;
	bool fullstrip;
	char16_t keepcase_flag;
	vector<pair<string, string>> input_conversion;
	vector<pair<string, string>> output_conversion;
	char16_t need_affix_flag;
	char16_t substandard_flag;
	string wordchars;
	bool checksharps;

	// methods
	auto parse(std::istream& in) -> bool;

	auto decode_flags(istream& in, utf8_to_ucs2_converter& cv) const
	    -> u16string;

	// u16string decode_flags(istream& in);

	auto decode_single_flag(istream& in, utf8_to_ucs2_converter& cv) const
	    -> char16_t;

	// char16_t decode_single_flag(istream& in);
};
}

#endif

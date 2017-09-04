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

#include "aff_manager.hxx"
#include "dic_manager.hxx"
#include "dict_finder.hxx"
#include "hunspell.hxx"
#include <clocale>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#if defined(__MINGW32__) || defined(__unix__) || defined(__unix) ||            \
    (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif

using namespace std;
using namespace Hunspell;

enum Mode {
	DEFAULT_MODE,
	PIPE_MODE,
	CURSES_MODE,
	MISSPELLED_WORDS_MODE,
	MISSPELLED_LINES_MODE,
	CORRECT_WORDS_MODE,
	CORRECT_LINES_MODE,
	LIST_DICTIONARIES_MODE
};

struct Args_t {
	Mode mode = DEFAULT_MODE;
	string dictionary;
	vector<string> other_dicts;
	vector<string> files;
	bool error = false;
	Args_t() {}
	Args_t(int argc, char* argv[]) { parse_args(argc, argv); }
	auto parse_args(int argc, char* argv[]) -> void;
};

auto Args_t::parse_args(int argc, char* argv[]) -> void
{
// usage
// hunspell [-a] [-d dict_NAME]... file_name...
// hunspell -l|-G [-L] [-d dict_NAME]... file_name...
// hunspell -D
#if defined(_POSIX_VERSION) || defined(__MINGW32__)
	int c;
	bool lines = false;
	while ((c = getopt(argc, argv, ":d:DGLl")) != -1) {
		switch (c) {
		case 'd':
			if (dictionary.empty()) {
				dictionary = optarg;
			}
			else {
				other_dicts.emplace_back(optarg);
			}
			break;
		case 'D':
			if (mode != DEFAULT_MODE)
				error = 1;
			mode = LIST_DICTIONARIES_MODE;
			break;
		case 'G':
			if (mode != DEFAULT_MODE)
				error = 1;
			if (lines)
				mode = CORRECT_LINES_MODE;
			else
				mode = CORRECT_WORDS_MODE;
			break;
		case 'l':
			if (mode != DEFAULT_MODE)
				error = 1;
			if (lines)
				mode = MISSPELLED_LINES_MODE;
			else
				mode = MISSPELLED_WORDS_MODE;
			break;
		case 'L':
			lines = true;
			if (mode == MISSPELLED_WORDS_MODE)
				mode = MISSPELLED_LINES_MODE;
			else if (mode == CORRECT_WORDS_MODE)
				mode = CORRECT_LINES_MODE;
			else if (mode != DEFAULT_MODE)
				error = 1;
			break;
		case ':': /* -d without operand */
			cerr << "Option -" << (char)optopt
			     << " requires an operand\n";
			error = 1;
			break;
		case '?':
			cerr << "Unrecognized option: '-" << (char)optopt
			     << "'\n";
			error = 1;
			break;
		}
	}
	files.insert(files.end(), argv + optind, argv + argc);
	if (lines && mode == DEFAULT_MODE) {
		//in v1 this defaults to MISSPELLED_LINES_MODE
		//we will make it error here
		error = 1;
	}
	if (error) {
		cerr << "Invalid arguments" << endl;
	}
#endif
}

int main(int argc, char* argv[])
{
	auto args = Args_t(argc, argv);
	if (args.error) {
		return 1;
	}
	auto v = Hunspell::get_default_search_directories();
	Hunspell::get_mozilla_directories(v);
	Hunspell::get_libreoffice_directories(v);
	auto dics = Hunspell::search_dirs_for_dicts(v);
	if (args.mode == LIST_DICTIONARIES_MODE) {
		for (auto& a : v) {
			cout << a << endl;
		}
		for (auto& a : dics) {
			cout << a.first << '\t' << a.second << endl;
		}
		return 0;
	}

	if (args.dictionary.empty()) {
		//try and load default dictionary
		return 0;
	}
	string filename;
	for (auto& a : dics) {
		if (a.first == args.dictionary) {
			filename = a.second;
			break;
		}
	}
	if (filename.empty()) {
		cerr << "Dictionary " << args.dictionary
		     << " not found." << endl;
		return 1;
	}

	locale::global(locale(""));
	cin.imbue(locale());
	setlocale(LC_ALL, "");
	Hunspell::Dictionary dic(filename);
	string word;

	while (cin >> word) {
		auto res = dic.spell_narrow_input(word);
		switch (res) {
		case bad_word:
			cout << '&' << endl;
			break;
		case good_word:
			cout << '*' << endl;
			break;
		case affixed_good_word:
			cout << '+' << endl;
			break;
		case compound_good_word:
			cout << '-' << endl;
			break;
		}
	}
	/*
	ifstream affstream(filename + ".aff");
	ifstream dicstream(filename + ".dic");
	hunspell::aff_data aff;
	aff.parse(affstream);
	hunspell::dic_data dic;
	dic.parse(dicstream, aff);
	std::cout << aff.encoding << endl;
	std::cout << aff.try_chars << endl;
	for (auto& a : aff.compound_rules) {
	        cout << a << endl;
	}
	for (auto& a : aff.suffixes) {
	        cout << (char)a.flag << ' ' << (a.cross_product ? 'Y' : 'N')
	             << ' ' << a.stripping << ' ' << a.affix
	             << (a.new_flags.size() ? "/ " : " ") << a.condition;
	        cout << endl;
	}
	for (auto& wd : dic.words) {
	        cout << wd.first;
	        if (wd.second.size()) {
	                cout << '/';
	                for (auto& flag : wd.second) {
	                        cout << flag << ',';
	                }
	        }
	        cout << endl;
	}
	*/
	return 0;
}

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
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#if defined(__MINGW32__) || defined(__unix__) || defined(__unix) ||            \
    (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif

using namespace std;

struct args_t {
	unordered_map<char, string> options;
	vector<string> operands;
};

auto parse_args(int argc, char* argv[]) -> args_t
{
// usage
// hunspell -d dict [-l|-G]
// hunspell [-D]
#if defined(_POSIX_VERSION) || defined(__MINGW32__)
	unordered_map<char, string> ret;
	int c;
	int errflg = 0;
	while ((c = getopt(argc, argv, ":d:DGl")) != -1) {
		switch (c) {
		case 'd':
			ret[c] = optarg;
			break;
		case 'D':
		case 'G':
		case 'l':
			ret[c];
			break;
		case ':': /* -d without operand */
			ret[c] += optopt;
			cerr << "Option -" << (char)optopt
			     << " requires an operand\n";
			errflg++;
			break;
		case '?':
			ret[c] += optopt;
			cerr << "Unrecognized option: '-" << (char)optopt
			     << "'\n";
			errflg++;
			break;
		}
	}
	return {ret, vector<string>(argv + optind, argv + argc)};
#else
	return {};
#endif
}

int main(int argc, char* argv[])
{
	auto args1 = parse_args(argc, argv);
	auto& args = args1.options;
	auto v = hunspell::get_default_search_directories();
	hunspell::get_mozilla_directories(v);
	hunspell::get_libreoffice_directories(v);
	auto dics = hunspell::search_dirs_for_dicts(v);
	if (args.empty() || args.count('D')) {
		for (auto& a : v) {
			cout << a << endl;
		}
		for (auto& a : dics) {
			cout << a.first << '\t' << a.second << endl;
		}
		return 0;
	}

	if (args.count('d') == 0) {
		return 0;
	}
	string filename;
	for (auto& a : dics) {
		if (a.first == args['d']) {
			filename = a.second;
			break;
		}
	}
	if (filename.empty()) {
		return 1;
	}
	/*
	locale::global(locale(""));
	cin.imbue(locale());
	hunspell::hunspell dic(filename);
	string word;
	if (args.count('l')) {
	        while (cin >> word) {
	                auto res = dic.spell_narrow_input(word);
	                switch (res) {
	                case bad_word:
	                        cout << word << '\n';
	                }
	        }
	}
	else if (args.count('G')) {
	        while (cin >> word) {
	                auto res = dic.spell_narrow_input(word);
	                switch (res) {
	                case bad_word:
	                        break;
	                default:
	                        cout << word << '\n';
	                }
	        }
	}
	else {
	        while (cin >> word) {
	                auto res = dic.spell_narrow_input(word);
	                switch (res) {
	                case bad_word:
	                case good_word:
	                case affixed_good_word:
	                case compound_good_word:
	                }
	        }
	}
	*/
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
	return 0;
}

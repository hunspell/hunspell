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
	LIST_DICTIONARIES_MODE,
	LINES_MODE,
    HELP_MODE,
    VERSION_MODE,
    ERROR_MODE
};

struct Args_t {
	Mode mode = DEFAULT_MODE;
	string dictionary;
	vector<string> other_dicts;
	vector<string> files;

	Args_t() {}
	Args_t(int argc, char* argv[]) { parse_args(argc, argv); }
	auto parse_args(int argc, char* argv[]) -> void;
	auto fail() -> bool { return mode == ERROR_MODE; }
};

auto Args_t::parse_args(int argc, char* argv[]) -> void
{
// usage
// hunspell [-a] [-d dict_NAME]... file_name...
// hunspell -l|-G [-L] [-d dict_NAME]... file_name...
// hunspell -D
// hunspell -h
// hunspell -v
//TODO support --help
#if defined(_POSIX_VERSION) || defined(__MINGW32__)
	int c;
	// The program can run in various modes depending on the
	// command line options. mode is FSM state, this while loop is FSM.
    while ((c = getopt(argc, argv, ":d:aDGLlhv")) != -1) {
		switch (c) {
		case 'd':
			if (dictionary.empty())
				dictionary = optarg;
			else
				other_dicts.emplace_back(optarg);

			break;
		case 'a':
			if (mode == DEFAULT_MODE)
				mode = PIPE_MODE;
			else
				mode = ERROR_MODE;

			break;
		case 'D':
			if (mode == DEFAULT_MODE)
				mode = LIST_DICTIONARIES_MODE;
			else
				mode = ERROR_MODE;

			break;
		case 'G':
			if (mode == DEFAULT_MODE)
				mode = CORRECT_WORDS_MODE;
			else if (mode == LINES_MODE)
				mode = CORRECT_LINES_MODE;
			else
				mode = ERROR_MODE;

			break;
		case 'l':
			if (mode == DEFAULT_MODE)
				mode = MISSPELLED_WORDS_MODE;
			else if (mode == LINES_MODE)
				mode = MISSPELLED_LINES_MODE;
			else
				mode = ERROR_MODE;

			break;
		case 'L':
			if (mode == DEFAULT_MODE)
				mode = LINES_MODE;
			else if (mode == MISSPELLED_WORDS_MODE)
				mode = MISSPELLED_LINES_MODE;
			else if (mode == CORRECT_WORDS_MODE)
				mode = CORRECT_LINES_MODE;
			else
				mode = ERROR_MODE;

			break;
        case 'h':
            if (mode == DEFAULT_MODE)
                mode = HELP_MODE;
            else
                mode = ERROR_MODE;

            break;
        case 'v':
            if (mode == DEFAULT_MODE)
                mode = VERSION_MODE;
            else
                mode = ERROR_MODE;

            break;
        case ':':
			cerr << "Option -" << (char)optopt
			     << " requires an operand\n";
			mode = ERROR_MODE;
			break;
		case '?':
			cerr << "Unrecognized option: '-" << (char)optopt
			     << "'\n";
			mode = ERROR_MODE;
			break;
		}
	}
	files.insert(files.end(), argv + optind, argv + argc);
	if (mode == LINES_MODE) {
		// in v1 this defaults to MISSPELLED_LINES_MODE
		// we will make it error here
		mode = ERROR_MODE;
	}
	if (mode == ERROR_MODE) {
		cerr << "Invalid arguments" << endl;
	}
#endif
}

int main(int argc, char* argv[])
{
	auto args = Args_t(argc, argv);
	if (args.fail()) {
		return 1;
	}

	// locale::global(locale(""));
	cin.imbue(locale(""));
	setlocale(LC_ALL, "");

	auto f = Finder();
	f.add_default_directories();
	f.add_libreoffice_directories();
	f.add_mozilla_directories();
	f.search_dictionaries();
	auto filename = f.get_dictionary(args.dictionary);
    if (args.mode == HELP_MODE) {
        cout << "Usage: hunspell [OPTION]... [FILE]..." << endl;
        cout << "Check spelling of each FILE. Without FILE, check standard input." << endl;
        cout << endl;
        // note there are tab characters used below
        cout << "TODO" << endl;
        cout << "  -d d[,d2,...]	use d (d2 etc.) dictionaries" << endl;
        cout << "  -D		show available dictionaries" << endl;
        cout << "TODO" << endl;
        cout << "  -h, --help	display this help and exit" << endl;
        cout << endl;
        // note there are only spaces used below
        cout << "Example: hunspell -d en_US file.txt    # interactive spelling" << endl;
        cout << "TODO" << endl;
        cout << endl;
        cout << "Bug reports: http://hunspell.github.io/" << endl;
        return 0;
    } else if (args.mode == VERSION_MODE) {
        cout << "Hunspell " << "2.0.0" << endl;//FIXME should get version via API from library
//TODO        cout << gettext("\nCopyright (C) 2002-2014 L\303\241szl\303\263 "
//                "N\303\251meth. License: MPL/GPL/LGPL.\n\n"
//                "Based on OpenOffice.org's Myspell library.\n"
//                "Myspell's copyright (C) Kevin Hendricks, 2001-2002, "
//                "License: BSD.") << endl;
//TODO cout << endl;
//TODO cout << gettext("This is free software; see the source for "
//                        "copying conditions.  There is NO\n"
//                        "warranty; not even for MERCHANTABILITY or "
//                        "FITNESS FOR A PARTICULAR PURPOSE,\n"
//                        "to the extent permitted by law.") << endl;
        return 0;
    } else if (args.mode == LIST_DICTIONARIES_MODE) {
        cout << "SEARCH PATHS:\n";
		for (auto& a : f.get_all_directories()) {
			cout << a << '\n';
		}
		cout << "AVAILABLE DICTIONARIES:\n";
		for (auto& a : f.get_all_dictionaries()) {
			cout << a.first << '\t' << a.second << '\n';
		}
		cout << "LOADED DICTIONARY:\n" << filename << endl;
		return 0;
    } else if (args.mode == DEFAULT_MODE) {
        if (filename.empty()) {
            cerr << "Dictionary " << args.dictionary << " not found."
                 << endl;
            return 1;
        }
        Hunspell::Dictionary dic(filename);
        string word;
        if (args.files.empty()) {
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
        } else {
            for (vector<string>::iterator file_name = args.files.begin(); file_name < args.files.end(); ++file_name) {
                ifstream input_file(file_name->c_str());
                if (!input_file.is_open()) {
                  cerr << "Can't open " << file_name->c_str() << endl;
                  return 1;
                }
                while (getline(input_file, word)) {
                    //TODO below is only temporary for development purposes
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
            }
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

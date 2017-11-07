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

#include "dictionary.hxx"
#include "finder.hxx"
#include "string_utils.hxx"
#include <clocale>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <unordered_map>

#include <boost/locale.hpp>

#if defined(__MINGW32__) || defined(__unix__) || defined(__unix) ||            \
    (defined(__APPLE__) && defined(__MACH__))
#include <getopt.h>
#include <unistd.h>
#endif

using namespace std;
using namespace hunspell;

enum Mode {
	DEFAULT_MODE,
	MISSPELLED_WORDS_MODE,
	MISSPELLED_LINES_MODE,
	CORRECT_WORDS_MODE,
	CORRECT_LINES_MODE,
	LINES_MODE,
	LIST_DICTIONARIES_MODE,
	HELP_MODE,
	VERSION_MODE,
	ERROR_MODE
};

struct Args_t {
	Mode mode = DEFAULT_MODE;
	string dictionary;
	string encoding;
	vector<string> other_dicts;
	vector<string> files;

	Args_t() {}
	Args_t(int argc, char* argv[]) { parse_args(argc, argv); }
	auto parse_args(int argc, char* argv[]) -> void;
	auto fail() -> bool { return mode == ERROR_MODE; }
};

/*!
 * Parses command line arguments and result is stored in mode, dictionary,
 * other_dicts and files.
 *
 * \param argc the total number of command line arguments.
 * \param argv all the individual command linen arguments.
 */
auto Args_t::parse_args(int argc, char* argv[]) -> void
{
// See POSIX Utility argument syntax
// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
#if defined(_POSIX_VERSION) || defined(__MINGW32__)
	int c;
	// The program can run in various modes depending on the
	// command line options. mode is FSM state, this while loop is FSM.
	const char* shortopts = ":d:i:DGLlhv";
	const struct option longopts[] = {
	    {"version", 0, 0, 'v'}, {"help", 0, 0, 'h'}, {NULL, 0, 0, 0},
	};
	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 'd':
			if (dictionary.empty())
				dictionary = optarg;
			else
				cerr << "WARNING: Detected not yet supported "
				        "other dictionary "
				     << optarg << '\n';
			other_dicts.emplace_back(optarg);

			break;
		case 'i':
			encoding = optarg;

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
			cerr << "Unrecognized option: '-" << (char)optopt << "'"
			     << '\n';
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
#endif
}

/*!
 * Prints help information to standard output.
 */
auto print_help() -> void
{
	cout << "Usage:\n"
	        "\n"
	        "hun2 [-d dict_NAME] [-i enc] [file_name]...\n"
	        "hun2 -l|-G [-L] [-d dict_NAME] [-i enc] [file_name]...\n"
	        "hun2 -D|-h|--help|-v|--version\n"
	        "\n"
	        "Check spelling of each FILE. Without FILE, check\n"
	        "standard input.\n"
	        "\n"
	        "  -d di_CT      use di_CT dictionary. Only one dictionary is\n"
	        "                supported.\n"
	        "  -D            show available dictionaries\n"
	        "  TODO\n"
	        "  -i enc        input encoding\n"
	        "  -l            print only misspelled words or lines\n"
	        "  -G            print only correct words or lines\n"
	        "  -L            lines mode\n"
	        "  -h, --help    display this help and exit\n"
	        "  -v, --version print version number\n"
	        "\n"
	        "Example: hun2 -d en_US file.txt\n"
	        "\n"
	        "Bug reports: http://hunspell.github.io/\n";
}

/*!
 * Prints the version number to standard output.
 */
auto print_version() -> void
{
	cout << "Hunspell "
	     << "2.0.0" << '\n';
	// FIXME should get version via API or (better?) from config.h
	// TODO print copyright and licence, LGPL v3
}

/*!
 * Lists dictionary paths and available dictionaries on the system to standard
 * output.
 *
 * \param f a finder for search paths and located dictionary.
 */
auto list_dictionaries(Finder& f) -> void
{
	if (f.get_all_paths().empty()) {
		cout << "No search paths available" << '\n';
	}
	else {
		cout << "Search paths:" << '\n';
		for (auto& p : f.get_all_paths()) {
			cout << p << '\n';
		}
	}

	// Even if no search paths are available, still report on available
	// dictionaries.
	if (f.get_all_dictionaries().empty()) {
		cout << "No dictionaries available\n";
	}
	else {
		cout << "Available dictionaries:\n";
		for (auto& d : f.get_all_dictionaries()) {
			cout << d.first << '\t' << d.second << '\n';
		}
	}
}

/*!
 * Checks spelling for input stream with on each line a single word to check
 * and report on respective line in output correctness of that word with a
 * single character.
 *
 * \param in the input stream to check spelling for with a word on each line.
 * \param out the output stream to report spelling correctness on the respective
 * lines.
 * \param dic the dictionary to use.
 */
auto normal_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		switch (res) {
		case bad_word:
			out << '&' << '\n';
			break;
		case good_word:
			out << '*' << '\n';
			break;
		case affixed_good_word:
			out << '+' << '\n';
			break;
		case compound_good_word:
			out << '-' << '\n';
			break;
		}
	}
}

/*!
 * Prints misspelled words from an input stream to an output stream.
 *
 * \param in the input stream with a word on each line.
 * \param out the output stream with on each line only misspelled words.
 * \param dic the dictionary to use.
 */
auto misspelled_word_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		if (res == bad_word)
			out << word << '\n';
	}
}

auto correct_word_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		if (res != bad_word)
			out << word << '\n';
	}
}

auto misspelled_line_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto line = string();
	auto words = vector<string>();
	while (getline(in, line)) {
		auto print = false;
		split_on_whitespace_v(line, words, in.getloc());
		for (auto& word : words) {
			auto res = dic.spell(word, in.getloc());
			if (res == bad_word) {
				print = true;
				break;
			}
		}
		if (print)
			out << line << '\n';
	}
}

auto correct_line_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto line = string();
	auto words = vector<string>();
	while (getline(in, line)) {
		auto print = true;
		split_on_whitespace_v(line, words, in.getloc());
		for (auto& word : words) {
			auto res = dic.spell(word, in.getloc());
			if (res == bad_word) {
				print = false;
				break;
			}
		}
		if (print)
			out << line << '\n';
	}
}

auto diagnose_dic_and_aff(Aff_data& aff, Dic_data& dic)
{
	cout << aff.encoding << '\n';
	cout << aff.try_chars << '\n';
	for (auto& a : aff.compound_rules) {
		cout << a << '\n';
	}
	for (auto& a : aff.suffixes) {
		cout << (char)a.flag << ' ' << (a.cross_product ? 'Y' : 'N')
		     << ' ' << a.stripping << ' ' << a.affix
		     << (a.new_flags.size() ? "/ " : " ") << a.condition;
		cout << '\n';
	}
	for (auto& wd : dic.words) {
		cout << wd.first;
		if (wd.second.size()) {
			cout << '/';
			for (auto& flag : wd.second) {
				cout << flag << ',';
			}
		}
		cout << '\n';
	}
}

namespace std {
ostream& operator<<(ostream& out, const locale& loc)
{
	auto& f = use_facet<boost::locale::info>(loc);
	out << "name=" << f.name() << ", lang=" << f.language()
	    << ", country=" << f.country() << ", enc=" << f.encoding();
	return out;
}
}

int main(int argc, char* argv[])
{
	// may spedup io. after this, don't use C printf, scanf etc.
	ios_base::sync_with_stdio(false);

	auto args = Args_t(argc, argv);
	if (args.fail()) {
		return 1;
	}
	boost::locale::generator gen;
	auto loc = gen("");
	if (args.encoding.empty()) {
		cin.imbue(loc);
	}
	else {
		cin.imbue(gen("en_US." + args.encoding));
	}
	cout.imbue(loc);
	cerr.imbue(loc);
	clog.imbue(loc);
	setlocale(LC_CTYPE, "");

	switch (args.mode) {
	case HELP_MODE:
		print_help();
		return 0;
	case VERSION_MODE:
		print_version();
		return 0;
	case ERROR_MODE:
		cerr << "Invalid arguments, try 'hun2 --help' for more "
		        "information.\n";
		return 1;
	default:
		break;
	}
	clog << "INFO: Input  locale " << cin.getloc() << '\n';
	clog << "INFO: Output locale " << cout.getloc() << '\n';

	auto f = Finder();
	f.add_default_paths();
	f.add_libreoffice_paths();
	f.add_mozilla_paths();
	f.add_apacheopenoffice_paths();
	f.search_dictionaries();

	if (args.mode == LIST_DICTIONARIES_MODE) {
		list_dictionaries(f);
		return 0;
	}
	if (args.dictionary.empty()) {
		// infer dictionary from locale
		auto& info = use_facet<boost::locale::info>(loc);
		args.dictionary = info.language();
		auto c = info.country();
		if (!c.empty()) {
			args.dictionary += '_';
			args.dictionary += c;
		}
	}
	auto filename = f.get_dictionary(args.dictionary);
	if (filename.empty()) {
		cerr << "Dictionary " << args.dictionary << " not found.\n";
		return 1;
	}
	clog << "INFO: Pointed dictionary " << filename << ".{dic,aff}\n";
	hunspell::Dictionary dic(filename);
	auto loop_function = normal_loop;
	switch (args.mode) {
	case DEFAULT_MODE:
		// loop_function = normal_loop;
		break;
	case MISSPELLED_WORDS_MODE:
		loop_function = misspelled_word_loop;
		break;
	case MISSPELLED_LINES_MODE:
		loop_function = misspelled_line_loop;
		break;
	case CORRECT_WORDS_MODE:
		loop_function = correct_word_loop;
		break;
	case CORRECT_LINES_MODE:
		loop_function = correct_line_loop;
		break;
	default:
		break;
	}

	if (args.files.empty()) {
		loop_function(cin, cout, dic);
	}
	else {
		for (auto& file_name : args.files) {
			ifstream in(file_name.c_str());
			if (!in.is_open()) {
				cerr << "Can't open " << file_name << '\n';
				return 1;
			}
			in.imbue(cin.getloc());
			loop_function(in, cout, dic);
		}
	}
	return 0;
}

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
#include "string_utils.hxx"
#include <clocale>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <locale>
#include <string>
#include <unordered_map>

#include <boost/locale.hpp>

#if defined(__MINGW32__) || defined(__unix__) || defined(__unix) ||            \
    (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif

using namespace std;
using namespace Hunspell;

enum Mode {
	DEFAULT_MODE,
	PIPE_MODE,
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
	bool first_of_tsv = false;
	string dictionary;
	string encoding;
	vector<string> other_dicts;
	vector<string> files;

	Args_t() {}
	Args_t(int argc, char* argv[]) { parse_args(argc, argv); }
	auto parse_args(int argc, char* argv[]) -> void;
	auto fail() -> bool { return mode == ERROR_MODE; }
};

/**
 * Parses command line arguments and result is stored in mode, dictionary,
 * other_dicts and files.
 *
 * \param argc the total number of command line arguments.
 * \param argv all the individual command linen arguments.
 */
auto Args_t::parse_args(int argc, char* argv[]) -> void
{
// usage
// hunspell [-d dict_NAME]... [-i enc] [-1] file_name...
// hunspell -l|-G [-L] [-d dict_NAME]... [-i enc] file_name...
// hunspell -D|-h|-v
// TODO support --help
//
// See POSIX Utility argument syntax
// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
#if defined(_POSIX_VERSION) || defined(__MINGW32__)
	int c;
	// The program can run in various modes depending on the
	// command line options. mode is FSM state, this while loop is FSM.
	const char* shortopts = ":d:i:1aDGHLOP::Xlhtv";
	const struct option longopts[] = {
	    {"version", 0, 0, 'v'}, {"help", 0, 0, 'h'}, {NULL, 0, 0, 0},
	};
	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case '1':
			first_of_tsv = true;

			break;
		case 'd':
			if (dictionary.empty())
				dictionary = optarg;
			else
				cerr << "WARNING: Detected not yet supported "
				        "other dictionary "
				     << optarg << endl;
			other_dicts.emplace_back(optarg);

			break;
		case 'i':
			encoding = optarg;

			break;
		case 'H':
			cerr << "ERROR: Deprecated option \"-H\" HTML input "
			        "file format"
			     << endl;
			mode = ERROR_MODE;

			break;
		case 'O':
			cerr << "ERROR: Deprecated option \"-O\" OpenDocument "
			        "(ODF or Flat ODF) input file format"
			     << endl;
			mode = ERROR_MODE;

			break;
		case 'P':
			cerr << "ERROR: Deprecated option \"-P\" set password "
			        "for encrypted dictionaries"
			     << endl;
			mode = ERROR_MODE;

			break;
		case 't':
			cerr << "ERROR: Deprecated option \"-t\" TeX/LaTeX "
			        "input file format"
			     << endl;
			mode = ERROR_MODE;

			break;
		case 'X':
			cerr << "ERROR: Deprecated option \"-X\" XML input "
			        "file format"
			     << endl;
			mode = ERROR_MODE;

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
			     << " requires an operand" << endl;
			mode = ERROR_MODE;
			break;
		case '?':
			cerr << "Unrecognized option: '-" << (char)optopt << "'"
			     << endl;
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

/*!
 * Prints help information to standard output.
 */
auto print_help() -> void
{
	cout << "Usage: hunspell [OPTION]... [FILE]...\n"
	        "Check spelling of each FILE. Without FILE, check\n"
	        "standard input.\n\n"
	        "  -1          check only first field in lines (delimiter = "
	        "tabulator)\n"
	        "              (-1 is not supported in line modes)\n"
	        "  -d di_CT    use di_CT dictionary. You can specify\n"
	        "              -d multiple times. \n"
	        "  -D          show available dictionaries\n"
	        "  TODO\n"
	        "  -i enc      input encoding\n"
	        "  -l          print only misspelled words or lines\n"
	        "  -G          print only correct words or lines\n"
	        "  -L          lines mode\n"
	        "  -h          display this help and exit\n"
	        "  -v          print version number\n"
	        "Example: hunspell -d en_US file.txt    # interactive "
	        "spelling\n"
	        "Bug reports: http://hunspell.github.io/"
	     << endl;
}

/*!
 * Prints the version number to standard output.
 */
auto print_version() -> void
{
	cout << "Hunspell "
	     << "2.0.0" << endl;
	// FIXME should get version via API or (better?) from config.h
	// TODO print copyright and licence, LGPL v3
}

/**
 * Lists dictionary paths and available dictionaries on the system to standard
 * output.
 *
 * \param f a finder for search paths and located dictionary.
 */
auto list_dictionaries(Finder& f) -> void
{
	if (f.get_all_paths().empty()) {
		cout << "No search paths available" << endl;
	}
	else {
		cout << "Search paths:" << endl;
		for (auto& p : f.get_all_paths()) {
			cout << p << endl;
		}
	}

	// Even if no search paths are available, still report on available
	// dictionaries.
	if (f.get_all_dictionaries().empty()) {
		cout << "No dictionaries available" << endl;
	}
	else {
		cout << "Available dictionaries:" << endl;
		for (auto& d : f.get_all_dictionaries()) {
			cout << d.first << '\t' << d.second << endl;
		}
	}
}

auto normal_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		switch (res) {
		case bad_word:
			out << '&' << endl;
			break;
		case good_word:
			out << '*' << endl;
			break;
		case affixed_good_word:
			out << '+' << endl;
			break;
		case compound_good_word:
			out << '-' << endl;
			break;
		}
	}
}

auto normal_tsv_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto line = string();
	auto word = string();
	while (getline(in, line)) {
		word = split_first(line, '\t');
		auto res = dic.spell(word, in.getloc());
		switch (res) {
		case bad_word:
			out << '&' << endl;
			break;
		case good_word:
			out << '*' << endl;
			break;
		case affixed_good_word:
			out << '+' << endl;
			break;
		case compound_good_word:
			out << '-' << endl;
			break;
		}
	}
}

auto misspelled_word_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		if (res == bad_word)
			out << word << endl;
	}
}

auto correct_word_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto word = string();
	while (in >> word) {
		auto res = dic.spell(word, in.getloc());
		if (res == good_word)
			out << word << endl;
	}
}

auto misspelled_line_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto line = string();
	auto words = vector<string>();
	while (getline(in, line)) {
		auto print = false;
		split_v(line, ' ', words);
		for (auto& word : words) {
			auto res = dic.spell(word, in.getloc());
			if (res == bad_word) {
				print = true;
				break;
			}
		}
		if (print)
			out << line << endl;
	}
}

auto correct_line_loop(istream& in, ostream& out, Dictionary& dic)
{
	auto line = string();
	auto words = vector<string>();
	while (getline(in, line)) {
		auto print = true;
		split_v(line, ' ', words);
		for (auto& word : words) {
			auto res = dic.spell(word, in.getloc());
			if (res == bad_word) {
				print = false;
				break;
			}
		}
		if (print)
			out << line << endl;
	}
}

auto diagnose_dic_and_aff(Aff_data& aff, Dic_data& dic)
{
	cout << aff.encoding << endl;
	cout << aff.try_chars << endl;
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
}

namespace std {
ostream& operator<<(ostream& out, const locale& loc)
{
	auto& f = use_facet<boost::locale::info>(loc);
	out << "name=" << f.name() << ", lang=" << f.language()
	    << ", enc=" << f.encoding();
	return out;
}
}

int main(int argc, char* argv[])
{
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
	clog << "INFO: Input  locale " << cin.getloc() << endl;
	clog << "INFO: Output locale " << cout.getloc() << endl;

	switch (args.mode) {
	case HELP_MODE:
		print_help();
		return 0;
	case VERSION_MODE:
		print_version();
		return 0;
	case ERROR_MODE:
		return 1;
	default:
		break;
	}

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

	auto filename = f.get_dictionary(args.dictionary);
	if (filename.empty()) {
		cerr << "Dictionary " << args.dictionary << " not found."
		     << endl;
		return 1;
	}
	clog << "INFO: Pointed dictionary " << filename << ".{dic,aff}" << endl;
	Hunspell::Dictionary dic(filename);
	auto loop_function = normal_loop;
	switch (args.mode) {
	case DEFAULT_MODE:
		if (args.first_of_tsv)
			loop_function = normal_tsv_loop;
		else
			loop_function = normal_loop;
		break;
	case PIPE_MODE:
		cerr << "ERROR: pipe mode unimplelemed, will behave"
		        "same as normal mode"
		     << endl;
		if (args.first_of_tsv)
			loop_function = normal_tsv_loop;
		else
			loop_function = normal_loop;
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
				cerr << "Can't open " << file_name << endl;
				return 1;
			}
			in.imbue(cin.getloc());
			loop_function(in, cout, dic);
		}
	}
	return 0;
}

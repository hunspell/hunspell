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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
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
#include "dic_manager.hxx"
#include "dict_finder.hxx"
#include <iostream>
#include <fstream>

using namespace std;


int main(int argc, char * argv[])
{
	auto v = hunspell::get_default_search_directories();
	hunspell::get_mozilla_directories(v);
	hunspell::get_libreoffice_directories(v);
	for (auto& a: v) {
		cout << a << endl;
	}
	auto dics = hunspell::search_dirs_for_dicts(v);
	for (auto& a: dics) {
		cout << a.first << '\t' << a.second << endl;
	}
	if (argc < 2) {
		return 0;
	}
	string filename;
	for (auto& a: dics) {
		if (a.first == argv[1]) {
			filename = a.second;
			break;
		}
	}
	if (filename.empty()) {
		return 1;
	}
	ifstream affstream(filename + ".aff");
	ifstream dicstream(filename + ".dic");
	hunspell::aff_data aff;
	aff.parse(affstream);
	hunspell::dic_data dic;
	dic.parse(dicstream, aff);
	std::cout << aff.encoding << endl;
	std::cout << aff.try_chars << endl;
	for (auto& a: aff.compound_rules) {
		cout << a << endl;
	}
	for (auto& a: aff.suffixes) {
		cout << (char)a.flag << ' '
		     << (a.cross_product ? 'Y' : 'N') << ' '
		     << a.stripping << ' '
		     << a.affix << (a.new_flags.size() ? "/ " : " ")
		     << a.condition << endl;
	}
	for (auto& wd: dic.words) {
		cout << wd.first;
		if (wd.second.size()) {
			cout << '/';
			for (auto& flag: wd.second) {
				cout << flag << ',';
			}
		}
		cout << endl;
	}
	return 0;
}

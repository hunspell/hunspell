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

#include <cctype>

#include <iterator>

namespace hunspell {

using namespace std;

bool aff_data::parse(std::istream& in)
{
	string line;
	string command;
	string parametar_line;
	while (getline(in, line)) {
		stringstream ss(line);
		ss >> ws;
		if (ss.eof() || ss.peek() == '#') {
				continue; //skip comment or empty lines
		}
		ss >> command;
		for (auto & c: command) c = toupper(c);
		ss >> ws;
		//getline(ss, parametar_line);
		//parse_line(command, parametar_line)
	}

	return in.eof(); // success if we reached eof
}

}

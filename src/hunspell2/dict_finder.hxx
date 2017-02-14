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
 * along with Hunspell-2.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 */

#ifndef HUNSPELL_DIC_FINDER_HXX
#define HUNSPELL_DIC_FINDER_HXX
#include <string>
#include <utility>
#include <vector>

namespace hunspell {

std::vector<std::string> get_default_search_directories();

void get_mozilla_directories(std::vector<std::string>& out);

void get_libreoffice_directories(std::vector<std::string>& out);

std::vector<std::pair<std::string, std::string>>
search_dirs_for_dicts(const std::vector<std::string>& dirs);
}
#endif

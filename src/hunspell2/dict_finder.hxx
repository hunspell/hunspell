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

#ifndef HUNSPELL_DIC_FINDER_HXX
#define HUNSPELL_DIC_FINDER_HXX
#include <string>
#include <utility>
#include <vector>

namespace hunspell {

auto get_default_search_directories() -> std::vector<std::string>;

auto get_mozilla_directories(std::vector<std::string>& out) -> void;

auto get_libreoffice_directories(std::vector<std::string>& out) -> void;

auto search_dirs_for_dicts(const std::vector<std::string>& dirs)
    -> std::vector<std::pair<std::string, std::string>>;
}
#endif

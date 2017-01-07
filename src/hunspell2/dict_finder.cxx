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

#include "dict_finder.hxx"

#include <array>
#include <iterator>
#include <unordered_set>

#include <sys/types.h>
#include <dirent.h>

using namespace std;

namespace hunspell {

template <class OutIt>
OutIt get_default_search_directories(OutIt out)
{
	*out = ".";
	++out;
	char * dicpath = getenv("DICPATH");
	if (dicpath) {
		*out = dicpath;
		++out;
	}
	char * home = getenv("HOME");
	array<string, 3> prefixes = {
		home ? string(home)+"/.local/" : "/",
		"/usr/local/",
		"/usr/" };
	array<const char*, 3> dirs =
		{ "share/hunspell", "share/myspell", "share/myspell/dicts" };
	for (auto& dir: dirs) {
		for (auto& prefix : prefixes) {
			*out = prefix + dir;
			out++;
		}
	}
	*out = "/Library/Spelling"; //for osx
	++out;
	return out;
}

vector<string> get_default_search_directories()
{
	vector<string> v;
	get_default_search_directories(back_inserter(v));
	return v;
}

struct Directory {
	DIR *dp;
	Directory(): dp(nullptr) {}
	Directory(const Directory& d) = delete;
	void operator=(const Directory& d) = delete;
	bool open(const string& dirname) {
		if (dp) {
			closedir(dp);
		}
		dp = opendir(dirname.c_str());
		return dp;
	}
	void close() {
		(void) closedir(dp);
	}
	~Directory()
	{
		close();
	}
	
};

template <class OutIt>
OutIt search_dir_for_dicts(const string& dir, OutIt out)
{
	//DIR* dp = opendir(dir.c_str()); //wrapped in RAII class
	Directory d; //this is the RAII class
	if (d.open(dir) == false) {
		return out;
	}
	struct dirent *ep;     
	unordered_set<string> dics;
	string file_name;
	while (ep = readdir (d.dp)) {
		file_name = ep->d_name;
		auto sz = file_name.size();
		if (sz < 4) {
			continue;
		}
		if (file_name.find(".dic", sz-4) != file_name.npos) {
			dics.insert(file_name);
			file_name.resize(sz-4);
			file_name += ".aff";
			if (dics.count(file_name)) {
				file_name.resize(sz-4);
				auto full_path = dir + '/' + file_name;
				*out = make_pair(file_name, full_path);
				out++;
			}
		}
		else if (file_name.find(".aff", sz-4) != file_name.npos) {
			dics.insert(file_name);
			file_name.resize(sz-4);
			file_name += ".dic";
			if (dics.count(file_name)) {
				file_name.resize(sz-4);
				auto full_path = dir + '/' + file_name;
				*out = make_pair(file_name, full_path);
				out++;
			}
		}
	}
	return out;
}

vector<pair<string, string>>
search_dirs_for_dicts(const vector<string>& dirs) {

	vector<pair<string, string>> v;	
	for(auto& dir: dirs) {
		search_dir_for_dicts(dir, back_inserter(v));
	}
	return v;
}

}

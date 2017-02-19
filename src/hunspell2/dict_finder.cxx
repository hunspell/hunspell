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

#include "dict_finder.hxx"

#include <algorithm>
#include <array>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <utility>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) ||               \
                         (defined(__APPLE__) && defined(__MACH__)))
#include <unistd.h>
#ifdef _POSIX_VERSION
#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#elif defined(__MINGW32__)
// Windows with Mingw. _WIN32 is also defined
// mingw unistd.h does not define _POSIX_VERSION, as expected
#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>

#elif defined(_WIN32)
// windows with non Mingw, usually MSVC
// #include <windows.h> // not the most pleasent header 
#endif

using namespace std;

namespace hunspell {

template <class CharT, class OutIt>
auto split(const basic_string<CharT>& s, CharT sep, OutIt out) -> OutIt
{
	basic_istringstream<CharT> is(s);
	basic_string<CharT> out_str;
	while (getline(is, out_str, sep)) {
		*out = out_str;
		++out;
	}
	return out;
}

template <class OutIt>
auto get_default_search_directories(OutIt out) -> OutIt
{
	*out = ".";
	++out;
	char* dicpath = getenv("DICPATH");
	if (dicpath) {
		out = split(string(dicpath), ':', out);
	}
	char* home = getenv("HOME");
#if defined(_POSIX_VERSION)
	array<string, 3> prefixes = {home ? string(home) + "/.local/" : "/",
	                             "/usr/local/", "/usr/"};
	array<const char*, 3> dirs = {"share/hunspell", "share/myspell",
	                              "share/myspell/dicts"};
	for (auto& dir : dirs) {
		for (auto& prefix : prefixes) {
			*out = prefix + dir;
			++out;
		}
	}
#endif
#if defined(__APPLE__) && defined(__MACH__)
	string osx = "/Library/Spelling";
	if (home) {
		*out++ = home + osx;
	}
	*out++ = osx;
#endif
#if defined(_WIN32)
	*out++ = "%LOCALAPPDATA%/hunspell";
	*out++ = "%PROGRAMDATA%/hunspell";
#endif
	return out;
}

auto get_default_search_directories() -> vector<string>
{
	vector<string> v;
	get_default_search_directories(back_inserter(v));
	return v;
}

#if defined(_POSIX_VERSION) || defined(__MINGW32__)
class Globber {
private:
	glob_t globdata;
	int ret;
public:
	Globber(const char* pattern) : globdata{}
	{
		ret = ::glob(pattern, 0, nullptr, &globdata);
	}
	Globber(const string& pattern) : Globber(pattern.c_str()) {}
	auto glob(const char* pattern) -> bool
	{
		globfree(&globdata);
		ret = ::glob(pattern, 0, nullptr, &globdata);
		return ret == 0;
	}
	auto glob(const string& pattern) -> bool
	{
		return glob(pattern.c_str());
	}
	auto begin() -> char** { return globdata.gl_pathv; }
	auto end() -> char** { return begin() + globdata.gl_pathc; }
	template <class OutIt>
	auto copy_glob_paths(OutIt out) -> OutIt
	{
		if (ret == 0) {
			out = copy(begin(), end(), out);
		}
		return out;
	}
	~Globber() { globfree(&globdata); }
};
#else
//unimplemented
class Globber {
	Globber(const char* pattern) {}
	Globber(const string& pattern) {}
	auto glob(const char* pattern) -> bool {return false;}
	auto begin() -> char** { return nullptr; }
	auto end() -> char** { return nullptr; }
	template <class OutIt>
	auto copy_glob_paths(OutIt out) -> OutIt {return out;}
};
#endif

template <class OutIt>
auto get_mozilla_directories(OutIt out) -> OutIt
{
#ifdef _POSIX_VERSION
	// add Mozilla global directory
	array<const char*, 2> dirs = {"/usr/local/lib/firefox/dictionaries",
	                              "/usr/lib/firefox/dictionaries"};
	struct stat dir_stat;
	for (auto& dir : dirs) {
		if (lstat(dir, &dir_stat) == 0) {
			if (S_ISDIR(dir_stat.st_mode)) {
				*out++ = dir;
			}
			// if SYMLINK do not add
		}
	}
#elif defined(_WIN32)
	// the bellow code should cover the majority of cases
	// otherwise we should query the registry
	*out++ "%PROGRAMFILES%/Firefox/dictionaries";
	*out++ "%PROGRAMFILES(x86)%/Firefox/dictionaries";
#endif

	string moz;
#ifdef _POSIX_VERSION
	// add Mozilla user directory
	char* home = getenv("HOME");
	if (home == nullptr) {
		return out;
	}
	moz = home;
	moz += "/.mozilla/firefox/*/extensions/*/dictionaries";
#elif defined(_WIN32)
	moz = "%APPDATA%/Mozilla/Firefox/Profiles/*/extensions/*/dictionaries";
#endif
	Globber g(moz);
	out = g.copy_glob_paths(out);
	return out;
}

auto get_mozilla_directories(vector<string>& out) -> void
{
	get_mozilla_directories(back_inserter(out));
}

template <class OutIt>
auto get_libreoffice_directories(OutIt out) -> OutIt
{
	// add Libreoffice global directories

	array<const char*, 3> prefixes = {"/usr/local/lib/libreoffice",
	                                  "/usr/lib/libreoffice",
	                                  "/opt/libreoffice*"};
	for (auto& p : prefixes) {
		Globber g(string(p) + "/share/extensions/dict-*");
		out = g.copy_glob_paths(out);
	}

	char* home = getenv("HOME");
	if (home == nullptr) {
		return out;
	}

	string lo_user_glob = home;
	lo_user_glob += "/.config/libreoffice/?/user/uno_packages/cache"
	                "/uno_packages/*/*.oxt/";

	Globber g(lo_user_glob + "dictionaries");
	out = g.copy_glob_paths(out);

	g.glob(lo_user_glob + "*.aff");
	string path_str;
	for (auto& path : g) {
		path_str = path;
		path_str.erase(path_str.rfind('/'));
		*out = std::move(path_str);
		++out;
	}
	return out;
}

auto get_libreoffice_directories(std::vector<std::string>& out) -> void
{
	get_libreoffice_directories(back_inserter(out));
}

struct Directory {
	DIR* dp;
	Directory() : dp(nullptr) {}
	Directory(const Directory& d) = delete;
	void operator=(const Directory& d) = delete;
	auto open(const string& dirname) -> bool
	{
		if (dp) {
			(void)closedir(dp);
		}
		dp = opendir(dirname.c_str());
		return dp;
	}
	auto close() -> void
	{
		(void)closedir(dp);
		dp = nullptr;
	}
	~Directory() { close(); }
};

template <class OutIt>
auto search_dir_for_dicts(const string& dir, OutIt out) -> OutIt
{
	// DIR* dp = opendir(dir.c_str()); //wrapped in RAII class
	Directory d; // this is the RAII class
	if (d.open(dir) == false) {
		return out;
	}
	struct dirent ent;
	struct dirent* ent_p = nullptr;
	unordered_set<string> dics;
	string file_name;
	while (readdir_r(d.dp, &ent, &ent_p) == 0 && ent_p) {
		file_name = ent_p->d_name;
		auto sz = file_name.size();
		if (sz < 4) {
			continue;
		}
		if (file_name.find(".dic", sz - 4) != file_name.npos) {
			dics.insert(file_name);
			file_name.resize(sz - 4);
			file_name += ".aff";
			if (dics.count(file_name)) {
				file_name.resize(sz - 4);
				auto full_path = dir + '/' + file_name;
				*out = make_pair(file_name, full_path);
				out++;
			}
		}
		else if (file_name.find(".aff", sz - 4) != file_name.npos) {
			dics.insert(file_name);
			file_name.resize(sz - 4);
			file_name += ".dic";
			if (dics.count(file_name)) {
				file_name.resize(sz - 4);
				auto full_path = dir + '/' + file_name;
				*out = make_pair(file_name, full_path);
				out++;
			}
		}
	}
	return out;
}

auto search_dirs_for_dicts(const vector<string>& dirs)
    -> vector<pair<string, string>>
{

	vector<pair<string, string>> v;
	for (auto& dir : dirs) {
		search_dir_for_dicts(dir, back_inserter(v));
	}
	return v;
}
}

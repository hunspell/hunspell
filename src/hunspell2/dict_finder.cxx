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

const char PATHSEP = ':';

#elif defined(_WIN32)

#ifdef __MINGW32__
#include <dirent.h>
//#include <glob.h> //not present in mingw-w64. present in vanilla mingw
#include <sys/stat.h>
#include <sys/types.h>
#endif //__MINGW32__

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

#ifdef _WIN32
const char PATHSEP = ';';
#else
const char PATHSEP = ':';
#endif

template <class OutIt>
auto get_default_search_directories(OutIt out) -> OutIt
{
	*out++ = ".";
	char* dicpath = getenv("DICPATH");
	if (dicpath) {
		out = split(string(dicpath), PATHSEP, out);
	}
	*out++ = "/mingw64/share/hunspell";
	char* home = getenv("HOME");
#ifdef _POSIX_VERSION
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
#ifdef _WIN32
	array<char*, 2> winpaths = {getenv("LOCALAPPDATA"),
	                            getenv("PROGRAMDATA")};
	for (auto& p : winpaths) {
		if (p) {
			*out++ = string(p) + "/hunspell";
		}
	}
#endif
	return out;
}

auto get_default_search_directories() -> vector<string>
{
	vector<string> v;
	get_default_search_directories(back_inserter(v));
	return v;
}

#ifdef _POSIX_VERSION
class Globber {
      private:
	glob_t g;
	int ret;

      public:
	Globber(const char* pattern) : g{}
	{
		ret = ::glob(pattern, 0, nullptr, &g);
	}
	Globber(const string& pattern) : Globber(pattern.c_str()) {}
	auto glob(const char* pattern) -> bool
	{
		globfree(&g);
		ret = ::glob(pattern, 0, nullptr, &g);
		return ret == 0;
	}
	auto glob(const string& pattern) -> bool
	{
		return glob(pattern.c_str());
	}
	auto begin() -> const char* const* { return g.gl_pathv; }
	auto end() -> const char* const* { return begin() + g.gl_pathc; }
	template <class OutIt>
	auto copy_glob_paths(OutIt out) -> OutIt
	{
		if (ret == 0) {
			out = copy(begin(), end(), out);
		}
		return out;
	}
	~Globber() { globfree(&g); }
};
#else
// unimplemented
struct Globber {
	Globber(const char* pattern) {}
	Globber(const string& pattern) {}
	auto glob(const char* pattern) -> bool { return false; }
	auto glob(const string& pattern) -> bool { return false; }
	auto begin() -> char** { return nullptr; }
	auto end() -> char** { return nullptr; }
	template <class OutIt>
	auto copy_glob_paths(OutIt out) -> OutIt
	{
		return out;
	}
};
#endif

template <class OutIt>
auto get_mozilla_directories(OutIt out) -> OutIt
{
#ifdef _POSIX_VERSION
	// add Mozilla linux global directory
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

	// add Mozilla linux user directory
	char* home = getenv("HOME");
	if (home == nullptr) {
		return out;
	}
	string moz = home;
	moz += "/.mozilla/firefox/*/extensions/*/dictionaries";
	Globber g(moz);
	out = g.copy_glob_paths(out);

#elif defined(_WIN32)
	// add Mozilla windows global directory
	array<char*, 2> winpaths = {getenv("PROGRAMFILES"),
	                            getenv("PROGRAMFILES(x86)")};
	for (auto& p : winpaths) {
		if (p) {
			*out++ = string(p) + "/Mozilla Firefox/dictionaries";
		}
	}
	// add Mozilla windows local directory
	char* home = getenv("APPDATA");
	if (home == nullptr) {
		return out;
	}
	string moz = home;
	moz += "/Mozilla/Firefox/Profiles/*/extensions/*/dictionaries";
	Globber g(moz);
	out = g.copy_glob_paths(out);
#endif
	return out;
}

auto get_mozilla_directories(vector<string>& out) -> void
{
	get_mozilla_directories(back_inserter(out));
}

template <class OutIt>
auto get_libreoffice_directories(OutIt out) -> OutIt
{
	string lo_user_glob;
#ifdef _POSIX_VERSION
	// add Libreoffice linux global directories
	array<const char*, 3> prefixes = {"/usr/local/lib/libreoffice",
	                                  "/usr/lib/libreoffice",
	                                  "/opt/libreoffice*"};
	for (auto& p : prefixes) {
		Globber g(string(p) + "/share/extensions/dict-*");
		out = g.copy_glob_paths(out);
	}

	// add Libreoffice linux local

	char* home = getenv("HOME");
	if (home == nullptr) {
		return out;
	}
	lo_user_glob = home;
	lo_user_glob += "/.config/libreoffice/?/user/uno_packages/cache"
	                "/uno_packages/*/*.oxt/";
#elif defined(_WIN32)
	// add Libreoffice windows global directories
	array<char*, 2> prefixes = {getenv("PROGRAMFILES"),
	                            getenv("PROGRAMFILES(x86)")};
	for (auto& p : prefixes) {
		if (p == nullptr) {
			continue;
		}
		Globber g(string(p) + "Libre Office ?/share/extensions/dict-*");
		out = g.copy_glob_paths(out);
	}

	char* home = getenv("APPDATA");
	if (home == nullptr) {
		return out;
	}
	lo_user_glob = home;
	lo_user_glob += "/libreoffice/?/user/uno_packages/cache"
	                "/uno_packages/*/*.oxt/";
#else
	return out;
#endif
	// finish adding LO user directory dicts (linux and windows)
	Globber g(lo_user_glob + "dictionaries");
	out = g.copy_glob_paths(out);

	g.glob(lo_user_glob + "*.aff");
	string path_str;
	for (auto& path : g) {
		path_str = path;
		path_str.erase(path_str.rfind('/'));
		*out = path_str;
		++out;
	}
	return out;
}

auto get_libreoffice_directories(std::vector<std::string>& out) -> void
{
	get_libreoffice_directories(back_inserter(out));
}

#if defined(_POSIX_VERSION) || defined(__MINGW32__)
class Directory {
	DIR* dp = nullptr;
#ifdef _POSIX_VERSION
	struct dirent ent;
#endif
	struct dirent* ent_p = nullptr;

      public:
	Directory() {}
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
	auto next() -> bool
	{
#ifdef _POSIX_VERSION
		return readdir_r(dp, &ent, &ent_p) == 0 && ent_p;
#else
		return (ent_p = readdir(dp));
#endif
	}
	auto entry_name() -> const char* { return ent_p->d_name; }
	auto close() -> void
	{
		(void)closedir(dp);
		dp = nullptr;
	}
	~Directory() { close(); }
};
#else
struct Directory()
{
	Directory() {}
	Directory(const Directory& d) = delete;
	void operator=(const Directory& d) = delete;
	auto open(const string& dirname)->bool { return false; }
	auto next()->bool { return false; }
	auto entry_name()->const char* { return nullptr; }
	auto close() {}
}
#endif

template <class OutIt>
auto search_dir_for_dicts(const string& dir, OutIt out) -> OutIt
{
	Directory d;
	if (d.open(dir) == false) {
		return out;
	}
	unordered_set<string> dics;
	string file_name;
	while (d.next()) {
		file_name = d.entry_name();
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

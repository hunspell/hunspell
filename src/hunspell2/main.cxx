#include "aff_manager.hxx"
#include "dic_manager.hxx"
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
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

std::vector<std::string> get_default_search_directories()
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
OutIt search_dir_for_dicts(const std::string& dir, OutIt out)
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

int main(int argc, char * argv[])
{
	auto v = hunspell::get_default_search_directories();
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

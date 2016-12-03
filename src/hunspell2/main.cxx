#include "aff_manager.hxx"
#include "dic_manager.hxx"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char * argv[])
{
	string filename;
	if (argc > 1) {
		filename = argv[1];
		if (filename.find(".aff", filename.size()-4) != string::npos
		        || filename.find(".dic",
		                         filename.size()-4) != string::npos) {
			filename.resize(filename.size()-4);
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

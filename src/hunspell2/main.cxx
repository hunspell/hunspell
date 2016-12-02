#include "aff_manager.hxx"

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char * argv[])
{
	hunspell::aff_data aff;
	if (argc > 1) {
		auto f = std::ifstream(argv[1]);
		aff.parse(f);
	}

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
	return 0;
}

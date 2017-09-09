#include <iostream>

#include <string_utils.hxx>

using namespace std;
using namespace Hunspell;

int main()
{
	auto in = string(";abc;;qwe;zxc;");
	auto out = vector<string>();
	auto expected = vector<string>{"", "abc", "", "qwe", "zxc", ""};
	split(in, ';', back_inserter(out));

	if (out != expected)
		return 1;

	in = string("^abc;.qwe/zxc/");
	out = vector<string>();
	split_on_any_of(in, ".;^/", back_inserter(out));

	if (out != expected)
		return 1;

	return 0;
}

//
//
// FILE TO DEVELOP AND TEST UNIT TESTING
//
//

#include "gtest/gtest.h"
#include <iostream>

#include "string_utils.hxx"

using namespace std;
using namespace Hunspell;

/*
out = split(string(dicpath), PATHSEP, out);


TEST (SplitTest, PositiveNos) {
    //TODO out
    ASSERT_EQ (TODO, split(string("first:second"), ':', out));
    ASSERT_EQ (TODO, split(string("first;second"), ';', out));
    ASSERT_EQ (TODO, split(string("first:second"), ":", out));
    ASSERT_EQ (TODO, split(string("first;second"), ";", out));
}
*/

int main(int argc, char* argv[])
{
	vector<string> ztrings;

	ztrings.insert(ztrings.end(), string("end"));
	ztrings.insert(ztrings.begin(), string("begin"));

	for (vector<string>::iterator ztring = ztrings.begin();
	     ztring < ztrings.end(); ++ztring) {
		cout << ztring->c_str() << endl;
	}

	//    ztrings = split(string("asdf;qwer"), ';', ztrings);

	for (auto& ztring : ztrings) {
		cout << ztring.c_str() << endl;
	}

	return 0;
}

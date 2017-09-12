#include <iostream>

#include "../../src/hunspell2/string_utils.hxx"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace std;
using namespace Hunspell;

class StringUtilsTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(StringUtilsTest);
	CPPUNIT_TEST(split_Semicolon);
	CPPUNIT_TEST(splitAnyOf_DotSemicolonCarotSlashDoublequote);
	CPPUNIT_TEST(splitFirst_Tab);
	CPPUNIT_TEST_SUITE_END();

      private:
	vector<string> exp;

      public:
	//    StringUtilsTest()
	//    {
	//    }

	auto setUp() -> void
	{
		exp = vector<string>{"", "abc", "", "qwe", "zxc", ""};
	}

	//  auto tearDown() -> void
	//  {
	//  }

	auto split_Semicolon() -> void
	{
		auto in = string(";abc;;qwe;zxc;");
		auto out = vector<string>();
		split(in, ';', back_inserter(out));
		CPPUNIT_ASSERT(exp == out);
	}

	auto splitAnyOf_DotSemicolonCarotSlashDoublequote() -> void
	{
		auto in = string("^abc;.qwe/zxc/");
		auto out = vector<string>();
		split_on_any_of(in, ".;^/", back_inserter(out));
		CPPUNIT_ASSERT(exp == out);
	}

	auto splitFirst_Tab() -> void
	{
		auto in = string("first\tsecond");
		auto first = string("first");
		auto out = split_first(in, '\t');
		CPPUNIT_ASSERT(first == out);

		in = string("first");
		out = split_first(in, '\t');
		CPPUNIT_ASSERT(first == out);

		in = string("\tsecond");
		first = string("");
		out = split_first(in, '\t');
		CPPUNIT_ASSERT(first == out);

		in = string("");
		first = string("");
		out = split_first(in, '\t');
		CPPUNIT_ASSERT(first == out);
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(StringUtilsTest);

int main(int argc, char* argv[])
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry& registry =
	    CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());

	// Change the default outputter to a compiler error format outputter
	runner.setOutputter(
	    new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	return !runner.run();
}

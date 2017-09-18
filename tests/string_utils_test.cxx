/* Copyright 2017 Sander van Geloven
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

#include <iostream>

#include "../src/hunspell2/string_utils.hxx"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace std;
using namespace Hunspell;

class StringUtilsTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(StringUtilsTest);
	CPPUNIT_TEST(test_split);
	CPPUNIT_TEST(test_split_on_any_of);
	CPPUNIT_TEST(test_split_first);
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

	auto test_split() -> void
	{
		auto in = string(";abc;;qwe;zxc;");
		auto out = vector<string>();
		split(in, ';', back_inserter(out));
		CPPUNIT_ASSERT(exp == out);
	}

	auto test_split_on_any_of() -> void
	{
		auto in = string("^abc;.qwe/zxc/");
		auto out = vector<string>();
		split_on_any_of(in, string(".;^/"), back_inserter(out));
		CPPUNIT_ASSERT(exp == out);
	}

	auto test_split_first() -> void
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

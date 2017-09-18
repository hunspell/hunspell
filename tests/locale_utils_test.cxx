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

#include "../src/hunspell2/locale_utils.hxx"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace std;
using namespace Hunspell;

class LocaleUtilsTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LocaleUtilsTest);
	CPPUNIT_TEST(test_decode_utf8);
	CPPUNIT_TEST(test_validate_utf8);
	CPPUNIT_TEST(test_is_ascii);
	CPPUNIT_TEST(test_is_all_ascii);
	CPPUNIT_TEST(test_latin1_to_ucs2);
	CPPUNIT_TEST(test_latin1_to_u32);
	CPPUNIT_TEST(test_is_all_bmp);
	CPPUNIT_TEST_SUITE_END();

      private:
	vector<string> exp;

      public:
	//    LocaleUtilsTest()
	//    {
	//    }

	//	auto setUp() -> void
	//	{
	//	}

	//  auto tearDown() -> void
	//  {
	//  }

	auto test_decode_utf8() -> void
	{
		CPPUNIT_ASSERT(U"" == decode_utf8(string("")));
		CPPUNIT_ASSERT(U"azĳß«" == decode_utf8(string("azĳß«")));
		CPPUNIT_ASSERT(U"日  Ӥ" != decode_utf8(string("Ӥ日本に")));
	}

	auto test_validate_utf8() -> void
	{
		CPPUNIT_ASSERT(true == validate_utf8(string("")));
		CPPUNIT_ASSERT(true == validate_utf8(string("the brown fox~")));
		CPPUNIT_ASSERT(true == validate_utf8(string("Ӥ日本に")));
	}

	auto test_is_ascii() -> void
	{
		CPPUNIT_ASSERT(true == is_ascii('a'));
		// Results in warning "multi-character character constant
		// [-Wmultichar]"
		// FIXME Add this to Makefile.am for only this source file.
		//        CPPUNIT_ASSERT(true != is_ascii('Ӥ'));
	}

	auto test_is_all_ascii() -> void
	{
		CPPUNIT_ASSERT(true == is_all_ascii(string("")));
		CPPUNIT_ASSERT(true == is_all_ascii(string("the brown fox~")));
		CPPUNIT_ASSERT(false ==
		               is_all_ascii(string("the brown foxĳӤ")));
	}

	auto test_latin1_to_ucs2() -> void
	{
		CPPUNIT_ASSERT(u"" == latin1_to_ucs2(string("")));
		CPPUNIT_ASSERT(u"abc" == latin1_to_ucs2(string("abc")));
		CPPUNIT_ASSERT(
		    u"²¿ýþÿ" !=
		    latin1_to_ucs2(string("²¿ýþÿ"))); // QUESTION Is this OK?
		CPPUNIT_ASSERT(u"Ӥ日本に" != latin1_to_ucs2(string("Ӥ日本に")));
	}

	auto test_latin1_to_u32() -> void
	{
		CPPUNIT_ASSERT(U"" == latin1_to_u32(string("")));
		CPPUNIT_ASSERT(U"abc~" == latin1_to_u32(string("abc~")));
		CPPUNIT_ASSERT(
		    U"²¿ýþÿ" !=
		    latin1_to_u32(string("²¿ýþÿ"))); // QUESTION Is this OK?
		CPPUNIT_ASSERT(U"Ӥ日本に" != latin1_to_u32(string("Ӥ日本に")));
	}

	auto test_is_all_bmp() -> void
	{
		CPPUNIT_ASSERT(true == is_all_bmp(U"abcýþÿӤ"));
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(LocaleUtilsTest);

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

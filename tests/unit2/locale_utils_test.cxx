#include <iostream>

#include "../../src/hunspell2/locale_utils.hxx"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace std;
using namespace Hunspell;

class LocaleUtilsTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LocaleUtilsTest);
	CPPUNIT_TEST(isAscii_Ascii);
	CPPUNIT_TEST(isAscii_NotAscii);
	CPPUNIT_TEST(isAllAscii_Ascii);
	CPPUNIT_TEST(isAllAscii_NotAscii);
	CPPUNIT_TEST(validateUtf8_Utf8);
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

	auto isAscii_Ascii() -> void { CPPUNIT_ASSERT(true == is_ascii('a')); }

	auto isAscii_NotAscii() -> void
	{
		CPPUNIT_ASSERT(true == is_ascii('a'));
	}
	auto isAllAscii_Ascii() -> void
	{
		CPPUNIT_ASSERT(true == is_all_ascii(string("the brown fox")));
	}

	auto isAllAscii_NotAscii() -> void
	{
		CPPUNIT_ASSERT(false == is_all_ascii(string("the brown foxĳ")));
	}

	auto validateUtf8_Utf8() -> void
	{
		CPPUNIT_ASSERT(true == validate_utf8(string("the brown fox")));
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

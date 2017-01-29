#include <string>
#include <locale>

namespace hunspell {

enum spell_result {
	bad_word, good_word, affixed_good_word, compound_good_word
};

class hunspell {

	using string = std::string;
	using u16string = std::u16string;
	using locale = std::locale;

	/**
	 (1) This should be called when the input and the dictionary
	 are in the same encoding and that encoding is single byte encoding.
	*/
	spell_result
	spell_singlechar_input_singlechar_dict(const string& word);


	/**
	 (2) This should be called when the input and the dictionary
	 are in the same encoding and that encoding UTF-8.
	*/
	spell_result
	spell_u8_input_u8_dict(const string& word);


	/*
	 (3) This should be called when the input is UTF-8 string
	 and the dictionary is byte encoding. Lossy conversion should happend
	 UTF-8 to single byte, and then (1) should be called.
	*/
	spell_result
	spell_u8_input_singlechar_dict(const string& word);

	/*
	 (4) This should be called when the input is
	 single-byte narow OR multi-byte narrow string.
	 and the dictionary is UTF-8
	 Locale object should help identify the encoding of the input string.
	 Generally, a loseless conversion should happend from narrow string
	 to UTF-8, and (2) should be called.

	 One can do narrow -> u16 -> u8 like this:
	 get old C locale,
	 set C locale to loc,
	 call mbrtoc16,
	 revert old C locale,
	 then codecvt<char16_t, char, mbstate_t>

	 We might completely exclude this variant.
	*/
	spell_result
	spell_narrow_input_u8_dict(const string& word, const locale& loc);

	/**
	 (5) This should be called when the input and the dictionary
	 are in the same encoding which can be single byte or UTF-8.
	 Simply calls (1) or (2).
	 This is the same as spell() in v1.
	*/
	spell_result
	spell(const string& word);

	/**
	 (6) Unknown narrow input
	 If dict is utf-8 call (4)
	 Else do narrow -> u16 -> u8 and call (3).

	 We might completely exclude this variant.
	*/
	spell_result
	spell_narrow_input(const string& word, const locale& loc);

	/**
	 (7) UTF-8 input. Will delegate either to (2) or (3).
	*/
	spell_result
	spell_u8_input(const string& word);


	/* All the major work is done in (1) and (2). The rest just do
	some conversions and delegate to them.
	And (1) and (2) can be implemented with a single generic method.

	template <class ConversionIterator>
	spell(ConversionIterator start, ConversionIterator end, string s)

	(1) will simply call this
	with ConversionIterator set to string::iterator

	(2) will call this with u8_u32 on the fly conversion iterator.
	*/

	/**
	 (8)
	*/
	spell_result
	spell_u16_input_singlechar_dict(const u16string& word);

	/**
	 (9)
	*/
	spell_result
	spell_u16_input_u8_dict(const u16string& word);

	/**
	 (10)
	*/
	spell_result
	spell_u16_input(const u16string& word);
};

}

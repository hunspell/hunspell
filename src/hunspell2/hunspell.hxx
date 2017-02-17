#include <string>

namespace hunspell {

enum spell_result {
	bad_word,
	good_word,
	affixed_good_word,
	compound_good_word
};

class hunspell {

      public:
	using string = std::string;
	using u16string = std::u16string;

      private:
	/* (0)
	All the major work is done here.
	(1) and (2) are the lowest level specializations.
	The rest just do some conversions and delegate to them.

	(1) will simply call this
	with ConversionIterator set to string::iterator

	(2) will call this with u8_u32 on the fly conversion iterator.
	*/
	template <class ConvIter>
	auto spell(ConvIter start, ConvIter end, const string& s)
	    -> spell_result;

	/**
	 (1) This should be called when the input and the dictionary
	 are in the same encoding and that encoding is single byte encoding.
	*/
	auto spell_singlechar_input_singlechar_dict(const string& word)
	    -> spell_result;

	/**
	 (2) This should be called when the input and the dictionary
	 are in the same encoding and that encoding UTF-8.
	*/
	auto spell_u8_input_u8_dict(const string& word) -> spell_result;

	/*
	 (3) This should be called when the input is UTF-8 string
	 and the dictionary is byte encoding. Lossy conversion should happend
	 UTF-8 to single byte, and then (1) should be called.
	*/
	auto spell_u8_input_singlechar_dict(const string& word) -> spell_result;

	/*
	 (4) This should be called when the input is
	 single-byte narow OR multi-byte narrow string.
	 and the dictionary is UTF-8
	 The input can be anything so we must use some info about the input
	 encoding, a C locale od C++ locale object.

	 One can do narrow -> u16 -> u8 like this:
	 get old C locale,
	 set C locale to loc,
	 call mbrtoc16,
	 revert old C locale,
	 then codecvt<char16_t, char, mbstate_t>

	 There is no C++ way to go mbr to u16, we're limited to mbrtoc16.

	 For that reason we will do similar conversion,
	 but in a more high level public funcrion.
	 Tis function should be UNUSED.
	*/
	// spell_result spell_narrow_input_u8_dict(const string& word);

      public:
	/**
	 (5) This should be called when the input and the dictionary
	 are in the same encoding which can be single byte or UTF-8.
	 Simply calls (1) or (2).
	 This is the same as spell() in v1.
	*/
	auto spell(const string& word) -> spell_result;

	/**
	 (6) Unknown narrow input (single byte or multi byte).
	 Use current C locale and mbrtoc16 to convert it to known.
	 Do a conversion mbr -> u16 -> u8.
	 Use mbrtoc16, codecvt<char16_t, char, mbstate_t>
	 We can check if the the current locale is already utf-8 to skip this.

	 Once we know we have a u8 string, just call (7).

	 This should be the recomended way to interface with the command line
	 utility. Before calling this function, one should call
	 setlocale(LC_ALL, "") or locale::global(locale("")).
	 If we use std::cin, we should imbue it with cin.imbue(locale())
	*/
	auto spell_narrow_input(const string& word) -> spell_result;

	/**
	 (7) UTF-8 input. Will delegate either to (2) or (3).
	*/
	auto spell_u8_input(const string& word) -> spell_result;

      private:
	/** (8) */
	auto spell_u16_input_singlechar_dict(const u16string& word)
	    -> spell_result;

	/** (9) */
	auto spell_u16_input_u8_dict(const u16string& word) -> spell_result;

      public:
	/** (10) */
	auto spell_u16_input(const u16string& word) -> spell_result;
};
}

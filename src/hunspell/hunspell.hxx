/* Copyright 2002-2017 Németh László
 * Copyright 2002 Kevin B. Hendricks, Stratford, Ontario, Canada
 *
 * This file is part of Hunspell.
 *
 * Hunspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hunspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MYSPELLMGR_HXX_
#define MYSPELLMGR_HXX_

#include "hunvisapi.h"
#include "w_char.hxx"
#include "atypes.hxx"
#include <string>
#include <vector>

#define SPELL_XML "<?xml?>"

#define MAXSUGGESTION 15
#define MAXSHARPS 5

#ifndef MAXWORDLEN
#define MAXWORDLEN 100
#endif

#if defined __GNUC__ && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#  define H_DEPRECATED __attribute__((__deprecated__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define H_DEPRECATED __declspec(deprecated)
#else
#  define H_DEPRECATED
#endif

class HunspellImpl;

class LIBHUNSPELL_DLL_EXPORTED Hunspell {
 private:
  Hunspell(const Hunspell&);
  Hunspell& operator=(const Hunspell&);

 private:
  HunspellImpl* m_Impl;

 public:
  /* Hunspell(aff, dic) - constructor of Hunspell class
   * input: path of affix file and dictionary file
   *
   * In WIN32 environment, use UTF-8 encoded paths started with the long path
   * prefix \\\\?\\ to handle system-independent character encoding and very
   * long path names (without the long path prefix Hunspell will use fopen()
   * with system-dependent character encoding instead of _wfopen()).
   */
  Hunspell(const char* affpath, const char* dpath, const char* key = NULL);
  ~Hunspell();

  /* load extra dictionaries (only dic files) */
  int add_dic(const char* dpath, const char* key = NULL);

  /* spell(word) - spellcheck word
   * output: false = bad word, true = good word
   *
   * plus output:
   *   info: information bit array, fields:
   *     SPELL_COMPOUND  = a compound word
   *     SPELL_FORBIDDEN = an explicit forbidden word
   *   root: root (stem), when input is a word with affix(es)
   */
  bool spell(const std::string& word, int* info = NULL, std::string* root = NULL);
  H_DEPRECATED int spell(const char* word, int* info = NULL, char** root = NULL);

  /* suggest(suggestions, word) - search suggestions
   * input: pointer to an array of strings pointer and the (bad) word
   *   array of strings pointer (here *slst) may not be initialized
   * output: number of suggestions in string array, and suggestions in
   *   a newly allocated array of strings (*slts will be NULL when number
   *   of suggestion equals 0.)
   */
  std::vector<std::string> suggest(const std::string& word);
  H_DEPRECATED int suggest(char*** slst, const char* word);

  /* Suggest words from suffix rules
   * suffix_suggest(suggestions, root_word)
   * input: pointer to an array of strings pointer and the  word
   *   array of strings pointer (here *slst) may not be initialized
   * output: number of suggestions in string array, and suggestions in
   *   a newly allocated array of strings (*slts will be NULL when number
   *   of suggestion equals 0.)
   */
  std::vector<std::string> suffix_suggest(const std::string& root_word);
  H_DEPRECATED int suffix_suggest(char*** slst, const char* root_word);

  /* deallocate suggestion lists */
  H_DEPRECATED void free_list(char*** slst, int n);

  const std::string& get_dict_encoding() const;
  char* get_dic_encoding();

  /* morphological functions */

  /* analyze(result, word) - morphological analysis of the word */
  std::vector<std::string> analyze(const std::string& word);
  H_DEPRECATED int analyze(char*** slst, const char* word);

  /* stem(word) - stemmer function */
  std::vector<std::string> stem(const std::string& word);
  H_DEPRECATED int stem(char*** slst, const char* word);

  /* stem(analysis, n) - get stems from a morph. analysis
   * example:
   * char ** result, result2;
   * int n1 = analyze(&result, "words");
   * int n2 = stem(&result2, result, n1);
   */
  std::vector<std::string> stem(const std::vector<std::string>& morph);
  H_DEPRECATED int stem(char*** slst, char** morph, int n);

  /* generate(result, word, word2) - morphological generation by example(s) */
  std::vector<std::string> generate(const std::string& word, const std::string& word2);
  H_DEPRECATED int generate(char*** slst, const char* word, const char* word2);

  /* generate(result, word, desc, n) - generation by morph. description(s)
   * example:
   * char ** result;
   * char * affix = "is:plural"; // description depends from dictionaries, too
   * int n = generate(&result, "word", &affix, 1);
   * for (int i = 0; i < n; i++) printf("%s\n", result[i]);
   */
  std::vector<std::string> generate(const std::string& word, const std::vector<std::string>& pl);
  H_DEPRECATED int generate(char*** slst, const char* word, char** desc, int n);

  /* functions for run-time modification of the dictionary */

  /* add word to the run-time dictionary */

  int add(const std::string& word);

  /* add word to the run-time dictionary with affix flags of
   * the example (a dictionary word): Hunspell will recognize
   * affixed forms of the new word, too.
   */

  int add_with_affix(const std::string& word, const std::string& example);

  /* remove word from the run-time dictionary */

  int remove(const std::string& word);

  /* other */

  /* get extra word characters definied in affix file for tokenization */
  const char* get_wordchars() const;
  const std::string& get_wordchars_cpp() const;
  const std::vector<w_char>& get_wordchars_utf16() const;

  struct cs_info* get_csconv();
  
  const char* get_version() const;
  const std::string& get_version_cpp() const;

  int get_langnum() const;

  /* need for putdic */
  bool input_conv(const std::string& word, std::string& dest);
  H_DEPRECATED int input_conv(const char* word, char* dest, size_t destsize);
};

#endif

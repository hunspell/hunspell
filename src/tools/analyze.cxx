/* Copyright 2002-2017 Németh László
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

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "../hunspell/hunspell.hxx"

#ifndef WIN32
using namespace std;
#endif

int main(int, char** argv) {
  /* first parse the command line options */

  for (int i = 1; i < 3; ++i)
    if (!argv[i]) {
      fprintf(stderr, "correct syntax is:\nanalyze affix_file");
      fprintf(stderr, " dictionary_file file_of_words_to_check\n");
      fprintf(stderr, "use two words per line for morphological generation\n");
      exit(1);
    }

  /* open the words to check list */

  FILE* wtclst = fopen(argv[3], "r");
  if (!wtclst) {
    fprintf(stderr, "Error - could not open file to check\n");
    exit(1);
  }

  Hunspell* pMS = new Hunspell(argv[1], argv[2]);
  char buf[100];
  while (fgets(buf, sizeof(buf), wtclst)) {
    buf[strcspn(buf, "\n")] = 0;
    if (*buf == '\0')
      continue;
    // morphgen demo
    char* s = strchr(buf, ' ');
    if (s) {
      *s = '\0';
      std::vector<std::string> result = pMS->generate(buf, s + 1);
      for (size_t i = 0; i < result.size(); ++i) {
        fprintf(stdout, "generate(%s, %s) = %s\n", buf, s + 1, result[i].c_str());
      }
      if (result.empty())
        fprintf(stdout, "generate(%s, %s) = NO DATA\n", buf, s + 1);
    } else {
      int dp = pMS->spell(std::string(buf));
      fprintf(stdout, "> %s\n", buf);
      if (dp) {
        std::vector<std::string> result = pMS->analyze(buf);
        for (size_t i = 0; i < result.size(); ++i) {
          fprintf(stdout, "analyze(%s) = %s\n", buf, result[i].c_str());
        }
        result = pMS->stem(buf);
        for (size_t i = 0; i < result.size(); ++i) {
          fprintf(stdout, "stem(%s) = %s\n", buf, result[i].c_str());
        }
      } else {
        fprintf(stdout, "Unknown word.\n");
      }
    }
  }
  delete pMS;
  fclose(wtclst);
  return 0;
}

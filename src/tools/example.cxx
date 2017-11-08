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
#include <fstream>

#include "../hunspell/hunspell.hxx"

using namespace std;

int main(int argc, char** argv) {

  /* first parse the command line options */

  if (argc < 4) {
    fprintf(stderr, "example (now it works with more dictionary files):\n");
    fprintf(stderr,
            "example affix_file dictionary_file(s) file_of_words_to_check\n");
    exit(1);
  }

  /* open the words to check list */
  std::ifstream wtclst(argv[argc - 1], std::ios_base::in);
  if (!wtclst.is_open()) {
    fprintf(stderr, "Error - could not open file of words to check\n");
    exit(1);
  }

  Hunspell* pMS = new Hunspell(argv[1], argv[2]);

  // load extra dictionaries
  if (argc > 4)
    for (int k = 3; k < argc - 1; ++k)
      pMS->add_dic(argv[k]);

  std::string buf;
  while (std::getline(wtclst, buf)) {
    int dp = pMS->spell(buf);
    if (dp) {
      fprintf(stdout, "\"%s\" is okay\n", buf.c_str());
      fprintf(stdout, "\n");
    } else {
      fprintf(stdout, "\"%s\" is incorrect!\n", buf.c_str());
      fprintf(stdout, "   suggestions:\n");
      std::vector<std::string> wlst = pMS->suggest(buf.c_str());
      for (size_t i = 0; i < wlst.size(); ++i) {
        fprintf(stdout, "    ...\"%s\"\n", wlst[i].c_str());
      }
      fprintf(stdout, "\n");
    }
    // for the same of testing this code path
    // do an analysis here and throw away the results
    pMS->analyze(buf);
  }

  delete pMS;
  return 0;
}

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
#include "../parsers/textparser.hxx"

#ifndef W32
using namespace std;
#endif

int main(int, char** argv) {
  FILE* f;

  /* first parse the command line options */

  for (int i = 1; i < 6; i++)
    if (!argv[i]) {
      fprintf(
          stderr,
          "chmorph - change affixes by morphological analysis and generation\n"
          "correct syntax is:\nchmorph affix_file "
          "dictionary_file file_to_convert STRING1 STRING2\n"
          "STRINGS may be arbitrary parts of the morphological descriptions\n"
          "example: chmorph hu.aff hu.dic hu.txt SG_2 SG_3 "
          " (convert informal Hungarian second person texts to formal third "
          "person texts)\n");
      exit(1);
    }

  /* open the words to check list */

  f = fopen(argv[3], "r");
  if (!f) {
    fprintf(stderr, "Error - could not open file to check\n");
    exit(1);
  }

  Hunspell* pMS = new Hunspell(argv[1], argv[2]);
  TextParser* p = new TextParser(
      "qwertzuiopasdfghjklyxcvbnmQWERTZUIOPASDFGHJKLYXCVBNM");

  char buf[MAXLNLEN];

  while (fgets(buf, MAXLNLEN, f)) {
    p->put_line(buf);
    std::string next;
    while (p->next_token(next)) {
      std::vector<std::string> pl = pMS->analyze(next);
      if (!pl.empty()) {
        int gen = 0;
        for (size_t i = 0; i < pl.size(); ++i) {
          const char* pos = strstr(pl[i].c_str(), argv[4]);
          if (pos) {
            std::string r(pl[i], pos - pl[i].c_str());
            r.append(argv[5]);
            r.append(pos + strlen(argv[4]));
            pl[i] = r;
            gen = 1;
          }
        }
        if (gen) {
          std::vector<std::string> pl2 = pMS->generate(next, pl);
          if (!pl2.empty()) {
            p->change_token(pl2[0].c_str());
            // jump over the (possibly un)modified word
            (void)p->next_token(next);
          }
        }
      }
    }
    fprintf(stdout, "%s\n", p->get_line().c_str());
  }

  delete p;
  fclose(f);
  return 0;
}

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

#include "textparser.hxx"
#include "htmlparser.hxx"
#include "latexparser.hxx"
#include "xmlparser.hxx"

#ifndef W32
using namespace std;
#endif

int main(int argc, char** argv) {
  FILE* f;
  /* first parse the command line options */

  if (argc < 2) {
    fprintf(stderr, "correct syntax is:\n");
    fprintf(stderr, "testparser file\n");
    fprintf(stderr, "example: testparser /dev/stdin\n");
    exit(1);
  }

  /* open the words to check list */
  f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "Error - could not open file of words to check\n");
    exit(1);
  }

  TextParser* p = new TextParser(
      "qwertzuiopasdfghjklyxcvbnmQWERTZUIOPASDFGHJKLYXCVBNM");

  char buf[MAXLNLEN];

  while (fgets(buf, MAXLNLEN, f)) {
    p->put_line(buf);
    p->set_url_checking(1);
    std::string next;
    while (p->next_token(next)) {
      fprintf(stdout, "token: %s\n", next.c_str());
    }
  }

  delete p;
  fclose(f);
  return 0;
}

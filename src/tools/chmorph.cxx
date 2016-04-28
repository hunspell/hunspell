/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
 *
 * Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
 * Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
 * Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
 * Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
 * Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "hunspell.hxx"
#include "textparser.hxx"

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
      "qwertzuiopasdfghjklyxcvbnméáúõûóüöíQWERTZUIOPASDFGHJKLYXCVBNMÍÉÁÕÚÖÜÓÛ");

  char buf[MAXLNLEN];
  char* next;

  while (fgets(buf, MAXLNLEN, f)) {
    p->put_line(buf);
    while ((next = p->next_token())) {
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
            free(next);
            next = p->next_token();
          }
        }
      }
      free(next);
    }
    fprintf(stdout, "%s\n", p->get_line());
  }

  delete p;
  fclose(f);
  return 0;
}

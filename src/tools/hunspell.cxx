/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// glibc < 3.0 (for mkstemp)
#ifndef __USE_MISC
#define __USE_MISC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include "config.h"
#include "hunspell.hxx"
#include "csutil.hxx"

#ifndef HUNSPELL_EXTRA
#define suggest_auto suggest
#endif

#define HUNSPELL_VERSION VERSION
#define INPUTLEN 50

#define HUNSPELL_PIPE_HEADING                                                  \
  "@(#) International Ispell Version 3.2.06 (but really Hunspell " VERSION ")" \
                                                                           "\n"
#define HUNSPELL_HEADING "Hunspell "
#define ODF_EXT "odt|ott|odp|otp|odg|otg|ods|ots"
#define ENTITY_APOS "&apos;"
#define UTF8_APOS "\xe2\x80\x99"

// for debugging only
//#define LOG

#define DEFAULTDICNAME "default"

#ifdef WIN32

#define LIBDIR "C:\\Hunspell\\"
#define USEROOODIR "Application Data\\OpenOffice.org 2\\user\\wordbook"
#define OOODIR                                                 \
  "C:\\Program files\\OpenOffice.org 2.4\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.3\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.2\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.1\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.0\\share\\dict\\ooo\\"
#define HOME "%USERPROFILE%\\"
#define DICBASENAME "hunspell_"
#define LOGFILE "C:\\Hunspell\\log"
#define DIRSEPCH '\\'
#define DIRSEP "\\"
#define PATHSEP ";"

#include "textparser.hxx"
#include "htmlparser.hxx"
#include "latexparser.hxx"
#include "manparser.hxx"
#include "firstparser.hxx"
#include "xmlparser.hxx"
#include "odfparser.hxx"

#else

// Not Windows
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include "textparser.hxx"
#include "htmlparser.hxx"
#include "latexparser.hxx"
#include "manparser.hxx"
#include "firstparser.hxx"
#include "xmlparser.hxx"
#include "odfparser.hxx"

#define LIBDIR                \
  "/usr/share/hunspell:"      \
  "/usr/share/myspell:"       \
  "/usr/share/myspell/dicts:" \
  "/Library/Spelling"
#define USEROOODIR                    \
  ".openoffice.org/3/user/wordbook:"  \
  ".openoffice.org2/user/wordbook:"   \
  ".openoffice.org2.0/user/wordbook:" \
  "Library/Spelling"
#define OOODIR                                       \
  "/opt/openoffice.org/basis3.0/share/dict/ooo:"     \
  "/usr/lib/openoffice.org/basis3.0/share/dict/ooo:" \
  "/opt/openoffice.org2.4/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.4/share/dict/ooo:"       \
  "/opt/openoffice.org2.3/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.3/share/dict/ooo:"       \
  "/opt/openoffice.org2.2/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.2/share/dict/ooo:"       \
  "/opt/openoffice.org2.1/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.1/share/dict/ooo:"       \
  "/opt/openoffice.org2.0/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.0/share/dict/ooo"
#define HOME getenv("HOME")
#define DICBASENAME ".hunspell_"
#define LOGFILE "/tmp/hunspell.log"
#define DIRSEPCH '/'
#define DIRSEP "/"
#define PATHSEP ":"
#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#include <errno.h>
char text_conv[MAXLNLEN];
#endif

#if ENABLE_NLS
#ifdef HAVE_LOCALE_H
#include <locale.h>
#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif
#endif
#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#else
#include <../../intl/libintl.h>
#endif
#else
#define gettext
#undef HAVE_LOCALE_H
#undef HAVE_LIBINTL_H
#endif

#ifdef HAVE_CURSES_H
#ifdef HAVE_NCURSESW_H
#include <ncurses.h>
#else
#include <curses.h>
#endif
#endif

#ifdef HAVE_READLINE
#include <readline/readline.h>
#else
#define readline scanline
#endif

extern char* mystrdup(const char* s);
extern char* mystrrep(const char* s);

// file formats:

enum { FMT_TEXT, FMT_LATEX, FMT_HTML, FMT_MAN, FMT_FIRST, FMT_XML, FMT_ODF };

struct wordlist {
  char* word;
  wordlist* next;
};

// global variables

char* wordchars = NULL;
char* dicpath = NULL;
int wordchars_len;
const w_char* wordchars_utf16 = NULL;
bool wordchars_utf16_free = false;
int wordchars_utf16_len;
char* dicname = NULL;
char* privdicname = NULL;
const char* currentfilename = NULL;

int modified;  // modified file sign
enum {
  NORMAL,
  BADWORD,     // print only bad words
  WORDFILTER,  // print only bad words from 1 word/line input
  BADLINE,     // print only lines with bad words
  STEM,        // stem input words
  ANALYZE,     // analyze input words
  PIPE,        // print only stars for LyX compatibility
  AUTO0,       // search typical error (based on SuggestMgr::suggest_auto())
  AUTO,        // automatic spelling to standard output
  AUTO2,       // automatic spelling to standard output with sed log
  AUTO3,
  SUFFIX  // print suffixes that can be attached to a given word
};        // automatic spelling to standard output with gcc error format
int filter_mode = NORMAL;
int printgood = 0;  // print only good words and lines
int showpath = 0;   // show detected path of the dictionary
int checkurl = 0;   // check URLs and mail addresses
int checkapos = 0;  // force typographic apostrophe
int warn = 0;  // warn potential mistakes (dictionary words with WARN flags)
const char* ui_enc = NULL;  // locale character encoding (default for I/O)
const char* io_enc = NULL;  // I/O character encoding

#define DMAX 10  // maximal count of loaded dictionaries

const char* dic_enc[DMAX];  // dictionary encoding
char* path = NULL;
int dmax = 0;  // dictionary count

// functions

#ifdef HAVE_ICONV
static const char* fix_encoding_name(const char* enc) {
  if (strcmp(enc, "TIS620-2533") == 0)
    enc = "TIS620";
  return enc;
}
#endif

/* change character encoding */
std::string& chenc(std::string& st, const char* enc1, const char* enc2) {
#ifndef HAVE_ICONV
  return st;
#else
  if (st.empty())
    return st;

  if (!enc1 || !enc2 || strcmp(enc1, enc2) == 0)
    return st;

  std::string out(st.size(), std::string::value_type());
  size_t c1(st.size());
  size_t c2(out.size());
  char* source(const_cast<char*>(&st[0]));
  char* dest(const_cast<char*>(&out[0]));
  iconv_t conv = iconv_open(fix_encoding_name(enc2), fix_encoding_name(enc1));
  if (conv == (iconv_t)-1) {
    fprintf(stderr, gettext("error - iconv_open: %s -> %s\n"), enc2, enc1);
  } else {
    size_t res;
    while ((res = iconv(conv, &source, &c1, &dest, &c2)) == size_t(-1)) {
      if (errno == E2BIG) {
        out.resize(out.size() + (c2 += c1));

        dest = const_cast<char*>(&out[0]) + out.size() - c2;
      } else
        break;
    }
    if (res == (size_t)-1) {
      fprintf(stderr, gettext("error - iconv: %s -> %s\n"), enc2, enc1);
    }
    iconv_close(conv);
    out.resize(dest - &out[0]);
    st = out;
  }

  return st;
#endif
}

/* change character encoding */
char* chenc(char* st, const char* enc1, const char* enc2) {
  char* out = st;
#ifdef HAVE_ICONV
  if (enc1 && enc2 && strcmp(enc1, enc2) != 0) {
    size_t c1 = strlen(st) + 1;
    size_t c2 = MAXLNLEN;
    char* source = st;
    char* dest = text_conv;
    iconv_t conv = iconv_open(fix_encoding_name(enc2), fix_encoding_name(enc1));
    if (conv == (iconv_t)-1) {
      fprintf(stderr, gettext("error - iconv_open: %s -> %s\n"), enc2, enc1);
    } else {
      size_t res = iconv(conv, (ICONV_CONST char**)&source, &c1, &dest, &c2);
      iconv_close(conv);
      if (res != (size_t)-1)
        out = text_conv;
    }
  }
#endif
  return out;
}

TextParser* get_parser(int format, const char* extension, Hunspell* pMS) {
  TextParser* p = NULL;
  int io_utf8 = 0;
  char* denc = pMS->get_dic_encoding();
#ifdef HAVE_ICONV
  initialize_utf_tbl();  // also need for 8-bit tokenization
  if (io_enc) {
    if ((strcmp(io_enc, "UTF-8") == 0) || (strcmp(io_enc, "utf-8") == 0) ||
        (strcmp(io_enc, "UTF8") == 0) || (strcmp(io_enc, "utf8") == 0)) {
      io_utf8 = 1;
      io_enc = "UTF-8";
    }
  } else if (ui_enc) {
    io_enc = ui_enc;
    if (strcmp(ui_enc, "UTF-8") == 0)
      io_utf8 = 1;
  } else {
    io_enc = denc;
    if (strcmp(denc, "UTF-8") == 0)
      io_utf8 = 1;
  }

  if (io_utf8) {
    wordchars_utf16 = pMS->get_wordchars_utf16(&wordchars_utf16_len);
    if ((strcmp(denc, "UTF-8") != 0) && pMS->get_wordchars()) {
      char* wchars = (char*)pMS->get_wordchars();
      int wlen = strlen(wchars);
      size_t c1 = wlen;
      size_t c2 = MAXLNLEN;
      char* dest = text_conv;
      iconv_t conv = iconv_open("UTF-8", fix_encoding_name(denc));
      if (conv == (iconv_t)-1) {
        fprintf(stderr, gettext("error - iconv_open: UTF-8 -> %s\n"), denc);
        wordchars_utf16 = NULL;
        wordchars_utf16_len = 0;
      } else {
        iconv(conv, (ICONV_CONST char**)&wchars, &c1, &dest, &c2);
        iconv_close(conv);
        w_char* new_wordchars_utf16 =
            (w_char*)malloc(sizeof(w_char) * wlen);
        int n = u8_u16(new_wordchars_utf16, wlen, text_conv);
        if (n > 0)
          std::sort(new_wordchars_utf16, new_wordchars_utf16 + n);
        wordchars_utf16 = new_wordchars_utf16;
        wordchars_utf16_len = n;
        wordchars_utf16_free = true;
      }
    }
  } else {
    // 8-bit input encoding
    // detect letters by unicodeisalpha() for tokenization
    char letters[MAXLNLEN];
    char* pletters = letters;
    char ch[2];
    char u8[10];
    *pletters = '\0';
    iconv_t conv = iconv_open("UTF-8", fix_encoding_name(io_enc));
    if (conv == (iconv_t)-1) {
      fprintf(stderr, gettext("error - iconv_open: UTF-8 -> %s\n"), io_enc);
    } else {
      for (int i = 32; i < 256; i++) {
        size_t c1 = 1;
        size_t c2 = 10;
        char* dest = u8;
        u8[0] = '\0';
        char* ch8bit = ch;
        ch[0] = (char)i;
        ch[1] = '\0';
        size_t res = iconv(conv, (ICONV_CONST char**)&ch8bit, &c1, &dest, &c2);
        if (res != (size_t)-1) {
          unsigned short idx;
          w_char w;
          w.l = 0;
          w.h = 0;
          u8_u16(&w, 1, u8);
          idx = (w.h << 8) + w.l;
          if (unicodeisalpha(idx)) {
            *pletters = (char)i;
            pletters++;
          }
        }
      }
      iconv_close(conv);
    }
    *pletters = '\0';

    // UTF-8 wordchars -> 8 bit wordchars
    int len = 0;
    char* wchars = (char*)pMS->get_wordchars();
    if (wchars) {
      if ((strcmp(denc, "UTF-8") == 0)) {
        pMS->get_wordchars_utf16(&len);
      } else {
        len = strlen(wchars);
      }
      char* dest = letters + strlen(letters);  // append wordchars
      size_t c1 = len + 1;
      size_t c2 = len + 1;
      iconv_t conv =
          iconv_open(fix_encoding_name(io_enc), fix_encoding_name(denc));
      if (conv == (iconv_t)-1) {
        fprintf(stderr, gettext("error - iconv_open: %s -> %s\n"), io_enc,
                denc);
      } else {
        iconv(conv, (ICONV_CONST char**)&wchars, &c1, &dest, &c2);
        iconv_close(conv);
        *dest = '\0';
      }
    }
    if (*letters)
      wordchars = mystrdup(letters);
  }
#else
  if (strcmp(denc, "UTF-8") == 0) {
    wordchars_utf16 = pMS->get_wordchars_utf16(&wordchars_utf16_len);
    io_utf8 = 1;
  } else {
    char* casechars = get_casechars(denc);
    wordchars = (char*)pMS->get_wordchars();
    if (casechars && wordchars) {
      casechars =
          (char*)realloc(casechars, strlen(casechars) + strlen(wordchars) + 1);
      strcat(casechars, wordchars);
    }
    wordchars = casechars;
  }
  io_enc = denc;
#endif

  if (io_utf8) {
    switch (format) {
      case FMT_LATEX:
        p = new LaTeXParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_HTML:
        p = new HTMLParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_MAN:
        p = new ManParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_XML:
        p = new XMLParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_ODF:
        p = new ODFParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_FIRST:
        p = new FirstParser(wordchars);
    }
  } else {
    switch (format) {
      case FMT_LATEX:
        p = new LaTeXParser(wordchars);
        break;
      case FMT_HTML:
        p = new HTMLParser(wordchars);
        break;
      case FMT_MAN:
        p = new ManParser(wordchars);
        break;
      case FMT_XML:
        p = new XMLParser(wordchars);
        break;
      case FMT_ODF:
        p = new ODFParser(wordchars);
        break;
      case FMT_FIRST:
        p = new FirstParser(wordchars);
    }
  }

  if ((!p) && (extension)) {
    if ((strcmp(extension, "html") == 0) || (strcmp(extension, "htm") == 0) ||
        (strcmp(extension, "xhtml") == 0)) {
      if (io_utf8) {
        p = new HTMLParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new HTMLParser(wordchars);
      }
    } else if ((strcmp(extension, "xml") == 0)) {
      if (io_utf8) {
        p = new XMLParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new XMLParser(wordchars);
      }
    } else if (((strlen(extension) == 3) &&
                (strstr(ODF_EXT, extension) != NULL)) ||
               ((strlen(extension) == 4) && (extension[0] == 'f') &&
                (strstr(ODF_EXT, extension + 1) != NULL))) {
      if (io_utf8) {
        p = new ODFParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new ODFParser(wordchars);
      }
    } else if (((extension[0] > '0') && (extension[0] <= '9'))) {
      if (io_utf8) {
        p = new ManParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new ManParser(wordchars);
      }
    } else if ((strcmp(extension, "tex") == 0)) {
      if (io_utf8) {
        p = new LaTeXParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new LaTeXParser(wordchars);
      }
    }
  }
  if (!p) {
    if (io_utf8) {
      p = new TextParser(wordchars_utf16, wordchars_utf16_len);
    } else {
      p = new TextParser(wordchars);
    }
  }
  p->set_url_checking(checkurl);
  return p;
}

#ifdef LOG
void log(char* message) {
  FILE* f = fopen(LOGFILE, "a");
  if (f) {
    fprintf(f, "%s\n", message);
    fclose(f);
  } else {
    fprintf(stderr, "Logfile...");
  }
}
#endif

int putdic(char* word, Hunspell* pMS) {
  char* w;
  char buf[MAXLNLEN];

  word = chenc(word, ui_enc, dic_enc[0]);

  if (pMS->input_conv(word, buf, MAXLNLEN))
    word = buf;

  int ret(0);

  if ((w = strstr(word + 1, "/")) == NULL) {
    if (*word == '*')
      ret = pMS->remove(word + 1);
    else
      ret = pMS->add(word);
  } else {
    char c;
    c = *w;
    *w = '\0';
    if (*(w + 1) == '/') {
      ret = pMS->add_with_affix(word, w + 2);  // word//pattern (back comp.)
    } else {
      ret = pMS->add_with_affix(word, w + 1);  // word/pattern
    }
    *w = c;
  }
  return ret;
}

void load_privdic(const char* filename, Hunspell* pMS) {
  char buf[MAXLNLEN];
  FILE* dic = fopen(filename, "r");
  if (dic) {
    while (fgets(buf, MAXLNLEN, dic)) {
      buf[strcspn(buf, "\n")] = 0;
      putdic(buf, pMS);
    }
    fclose(dic);
  }
}

int exist(const char* filename) {
  FILE* f = fopen(filename, "r");
  if (f) {
    fclose(f);
    return 1;
  }
  return 0;
}

int save_privdic(char* filename, char* filename2, wordlist* w) {
  wordlist* r;
  FILE* dic = fopen(filename, "r");
  if (dic) {
    fclose(dic);
    dic = fopen(filename, "a");
  } else {
    dic = fopen(filename2, "a");
  }
  if (!dic)
    return 0;
  while (w != NULL) {
    char* word = chenc(w->word, io_enc, ui_enc);
    fprintf(dic, "%s\n", word);
#ifdef LOG
    log(word);
    log("\n");
#endif
    r = w;
    free(w->word);
    w = w->next;
    free(r);
  }
  fclose(dic);
  return 1;
}

char* basename(char* s, char c) {
  char* p = s + strlen(s);
  while ((*p != c) && (p != s))
    p--;
  if (*p == c)
    p++;
  return p;
}

#ifdef HAVE_CURSES_H
char* scanline(char* message) {
  char input[INPUTLEN];
  printw(message);
  echo();
  getnstr(input, INPUTLEN);
  noecho();
  return mystrdup(input);
}
#endif

// check words in the dictionaries (and set first checked dictionary)
int check(Hunspell** pMS, int* d, char* token, int* info, char** root) {
  for (int i = 0; i < dmax; ++i) {
    std::string buf(token);
    chenc(buf, io_enc, dic_enc[*d]);
    mystrrep(buf, ENTITY_APOS, "'");
    if (checkapos && buf.find('\'') != std::string::npos)
      return 0;
    // 8-bit encoded dictionaries need ASCII apostrophes (eg. English
    // dictionaries)
    if (strcmp(dic_enc[*d], "UTF-8") != 0)
      mystrrep(buf, UTF8_APOS, "'");
    if ((pMS[*d]->spell(buf.c_str(), info, root) &&
         !(warn && (*info & SPELL_WARN))) ||
        // UTF-8 encoded dictionaries with ASCII apostrophes, but without ICONV
        // support,
        // need also ASCII apostrophes (eg. French dictionaries)
        ((strcmp(dic_enc[*d], "UTF-8") == 0) &&
         buf.find(UTF8_APOS) != std::string::npos &&
         pMS[*d]->spell(mystrrep(buf, UTF8_APOS, "'").c_str(), info, root) &&
         !(warn && (*info & SPELL_WARN)))) {
      return 1;
    }
    if (++(*d) == dmax)
      *d = 0;
  }
  return 0;
}

static int is_zipped_odf(TextParser* parser, const char* extension) {
  // ODFParser and not flat ODF
  return dynamic_cast<ODFParser*>(parser) && (extension && extension[0] != 'f');
}

static bool secure_filename(const char* filename) {
  const char* hasapostrophe = strchr(filename, '\'');
  if (hasapostrophe)
    return false;
  return true;
}

static void freewordlist(wordlist* w) {
  while (w != NULL) {
    wordlist* r = w;
    free(w->word);
    w = w->next;
    free(r);
  }
}

void pipe_interface(Hunspell** pMS, int format, FILE* fileid, char* filename) {
  char buf[MAXLNLEN];
  char* buf2;
  wordlist* dicwords = NULL;
  char* token;
  int pos;
  int bad;
  int lineno = 0;
  int terse_mode = 0;
  int verbose_mode = 0;
  int d = 0;
  char* odftmpdir;

  char* extension = (filename) ? basename(filename, '.') : NULL;
  TextParser* parser = get_parser(format, extension, pMS[0]);

  bool bZippedOdf = is_zipped_odf(parser, extension);

  // access content.xml of ODF
  if (bZippedOdf) {
    odftmpdir = tmpnam(NULL);
    // break 1-line XML of zipped ODT documents at </style:style> and </text:p>
    // to avoid tokenization problems (fgets could stop within an XML tag)
    sprintf(buf,
            "mkdir %s && unzip -p '%s' content.xml | sed "
            "'s/\\(<\\/text:p>\\|<\\/style:style>\\)\\(.\\)/\\1\\\n\\2/g' "
            ">%s/content.xml",
            odftmpdir, filename, odftmpdir);
    if (!secure_filename(filename) || system(buf) != 0) {
      if (secure_filename(filename))
        perror(gettext("Can't open inputfile"));
      else
        fprintf(stderr, gettext("Can't open %s.\n"), filename);
      exit(1);
    }
    sprintf(buf, "%s/content.xml", odftmpdir);
    fileid = fopen(buf, "r");
    if (fileid == NULL) {
      perror(gettext("Can't open inputfile"));
      exit(1);
    }
  }

  if (filter_mode == NORMAL) {
    fprintf(stdout, gettext(HUNSPELL_HEADING));
    fprintf(stdout, HUNSPELL_VERSION);
    if (pMS[0]->get_version())
      fprintf(stdout, " - %s", pMS[0]->get_version());
    fprintf(stdout, "\n");
    fflush(stdout);
  }

nextline:
  while (fgets(buf, MAXLNLEN, fileid)) {
    buf[strcspn(buf, "\n")] = 0;
    lineno++;
#ifdef LOG
    log(buf);
#endif
    bad = 0;
    pos = 0;

    // execute commands
    if (filter_mode == PIPE) {
      pos = -1;
      switch (buf[0]) {
        case '%': {
          verbose_mode = terse_mode = 0;
          break;
        }
        case '!': {
          terse_mode = 1;
          break;
        }
        case '`': {
          verbose_mode = 1;
          break;
        }
        case '+': {
          delete parser;
          parser = get_parser(FMT_LATEX, NULL, pMS[0]);
          parser->set_url_checking(checkurl);
          break;
        }
        case '-': {
          delete parser;
          parser = get_parser(format, NULL, pMS[0]);
          break;
        }
        case '@': {
          putdic(buf + 1, pMS[d]);
          break;
        }
        case '*': {
          struct wordlist* i =
              (struct wordlist*)malloc(sizeof(struct wordlist));
          i->word = mystrdup(buf + 1);
          i->next = dicwords;
          dicwords = i;
          putdic(buf + 1, pMS[d]);
          break;
        }
        case '#': {
          if (HOME) {
            strncpy(buf, HOME, MAXLNLEN - 1);
            buf[MAXLNLEN - 1] = '\0';
          } else {
            fprintf(stderr, gettext("error - missing HOME variable\n"));
            continue;
          }
#ifndef WIN32
          strcat(buf, "/");
#endif
          buf2 = buf + strlen(buf);
          if (!privdicname) {
            strcat(buf, DICBASENAME);
            strcat(buf, basename(dicname, DIRSEPCH));
          } else {
            strcat(buf, privdicname);
          }
          if (save_privdic(buf2, buf, dicwords)) {
            dicwords = NULL;
          }
          break;
        }
        case '^': {
          pos = 1;
          break;
        }

        default: {
          pos = 0;
          break;
        }

      }  // end switch
    }    // end filter_mode == PIPE

    if (pos >= 0) {
      parser->put_line(buf + pos);
      while ((token = parser->next_token())) {
        token = mystrrep(token, ENTITY_APOS, "'");
        switch (filter_mode) {
          case BADWORD: {
            if (!check(pMS, &d, token, NULL, NULL)) {
              bad = 1;
              if (!printgood)
                fprintf(stdout, "%s\n", token);
            } else {
              if (printgood)
                fprintf(stdout, "%s\n", token);
            }
            free(token);
            continue;
          }

          case WORDFILTER: {
            if (!check(pMS, &d, token, NULL, NULL)) {
              if (!printgood)
                fprintf(stdout, "%s\n", buf);
            } else {
              if (printgood)
                fprintf(stdout, "%s\n", buf);
            }
            free(token);
            goto nextline;
          }

          case BADLINE: {
            if (!check(pMS, &d, token, NULL, NULL)) {
              bad = 1;
            }
            free(token);
            continue;
          }

          case AUTO0:
          case AUTO:
          case AUTO2:
          case AUTO3: {
            FILE* f = (filter_mode == AUTO) ? stderr : stdout;
            if (!check(pMS, &d, token, NULL, NULL)) {
              char** wlst = NULL;
              bad = 1;
              int ns =
                  pMS[d]->suggest_auto(&wlst, chenc(token, io_enc, dic_enc[d]));
              if (ns > 0) {
                parser->change_token(chenc(wlst[0], dic_enc[d], io_enc));
                if (filter_mode == AUTO3) {
                  fprintf(f, "%s:%d: Locate: %s | Try: %s\n", currentfilename,
                          lineno, token, chenc(wlst[0], dic_enc[d], io_enc));
                } else if (filter_mode == AUTO2) {
                  fprintf(f, "%ds/%s/%s/g; # %s\n", lineno, token,
                          chenc(wlst[0], dic_enc[d], io_enc), buf);
                } else {
                  fprintf(f, gettext("Line %d: %s -> "), lineno,
                          chenc(token, io_enc, ui_enc));
                  fprintf(f, "%s\n", chenc(wlst[0], dic_enc[d], ui_enc));
                }
              }
              pMS[d]->free_list(&wlst, ns);
            }
            free(token);
            continue;
          }

          case STEM: {
            char** result;
            int n = pMS[d]->stem(&result, chenc(token, io_enc, dic_enc[d]));
            for (int i = 0; i < n; i++) {
              fprintf(stdout, "%s %s\n", token,
                      chenc(result[i], dic_enc[d], ui_enc));
            }
            pMS[d]->free_list(&result, n);
            if (n == 0 && token[strlen(token) - 1] == '.') {
              token[strlen(token) - 1] = '\0';
              n = pMS[d]->stem(&result, token);
              for (int i = 0; i < n; i++) {
                fprintf(stdout, "%s %s\n", token,
                        chenc(result[i], dic_enc[d], ui_enc));
              }
              pMS[d]->free_list(&result, n);
            }
            if (n == 0)
              fprintf(stdout, "%s\n", chenc(token, dic_enc[d], ui_enc));
            fprintf(stdout, "\n");
            free(token);
            continue;
          }

          case SUFFIX: {
            char** wlst = NULL;
            int ns = pMS[d]->suffix_suggest(&wlst, token);
            for (int j = 0; j < ns; j++) {
              fprintf(stdout, "Suffix Suggestions are %s \n",
                      chenc(wlst[j], dic_enc[d], io_enc));
            }
            fflush(stdout);
            pMS[d]->free_list(&wlst, ns);
            free(token);
            continue;
          }
          case ANALYZE: {
            char** result;
            int n = pMS[d]->analyze(&result, chenc(token, io_enc, dic_enc[d]));
            for (int i = 0; i < n; i++) {
              fprintf(stdout, "%s %s\n", token,
                      chenc(result[i], dic_enc[d], ui_enc));
            }
            pMS[d]->free_list(&result, n);
            if (n == 0 && token[strlen(token) - 1] == '.') {
              token[strlen(token) - 1] = '\0';
              n = pMS[d]->analyze(&result, token);
              for (int i = 0; i < n; i++) {
                fprintf(stdout, "%s %s\n", token,
                        chenc(result[i], dic_enc[d], ui_enc));
              }
              pMS[d]->free_list(&result, n);
            }
            if (n == 0)
              fprintf(stdout, "%s\n", chenc(token, dic_enc[d], ui_enc));
            fprintf(stdout, "\n");
            free(token);
            continue;
          }

          case PIPE: {
            int info;
            char* root = NULL;
            if (check(pMS, &d, token, &info, &root)) {
              if (!terse_mode) {
                if (verbose_mode)
                  fprintf(stdout, "* %s\n", token);
                else
                  fprintf(stdout, "*\n");
              }
              fflush(stdout);
            } else {
              char** wlst = NULL;
              int byte_offset = parser->get_tokenpos() + pos;
              int char_offset = 0;
              if (strcmp(io_enc, "UTF-8") == 0) {
                for (int i = 0; i < byte_offset; i++) {
                  if ((buf[i] & 0xc0) != 0x80)
                    char_offset++;
                }
              } else {
                char_offset = byte_offset;
              }
              int ns = pMS[d]->suggest(&wlst, chenc(token, io_enc, dic_enc[d]));
              if (ns == 0) {
                fprintf(stdout, "# %s %d", token, char_offset);
              } else {
                fprintf(stdout, "& %s %d %d: ", token, ns, char_offset);
                fprintf(stdout, "%s", chenc(wlst[0], dic_enc[d], io_enc));
              }
              for (int j = 1; j < ns; j++) {
                fprintf(stdout, ", %s", chenc(wlst[j], dic_enc[d], io_enc));
              }
              pMS[d]->free_list(&wlst, ns);
              fprintf(stdout, "\n");
              fflush(stdout);
            }
            if (root)
              free(root);
            free(token);
            continue;
          }
          case NORMAL: {
            int info;
            char* root = NULL;
            if (check(pMS, &d, token, &info, &root)) {
              if (info & SPELL_COMPOUND) {
                fprintf(stdout, "-\n");
              } else if (root) {
                fprintf(stdout, "+ %s\n", chenc(root, dic_enc[d], ui_enc));
              } else {
                fprintf(stdout, "*\n");
              }
              fflush(stdout);
              if (root)
                free(root);
            } else {
              char** wlst = NULL;
              int byte_offset = parser->get_tokenpos() + pos;
              int char_offset = 0;
              if (strcmp(io_enc, "UTF-8") == 0) {
                for (int i = 0; i < byte_offset; i++) {
                  if ((buf[i] & 0xc0) != 0x80)
                    char_offset++;
                }
              } else {
                char_offset = byte_offset;
              }
              int ns = pMS[d]->suggest(&wlst, chenc(token, io_enc, dic_enc[d]));
              if (ns == 0) {
                fprintf(stdout, "# %s %d", chenc(token, io_enc, ui_enc),
                        char_offset);
              } else {
                fprintf(stdout, "& %s %d %d: ", chenc(token, io_enc, ui_enc),
                        ns, char_offset);
                fprintf(stdout, "%s", chenc(wlst[0], dic_enc[d], ui_enc));
              }
              for (int j = 1; j < ns; j++) {
                fprintf(stdout, ", %s", chenc(wlst[j], dic_enc[d], ui_enc));
              }
              pMS[d]->free_list(&wlst, ns);
              fprintf(stdout, "\n");
              fflush(stdout);
            }
            free(token);
          }
        }
      }

      switch (filter_mode) {
        case AUTO: {
          char* pLine = parser->get_line();
          fprintf(stdout, "%s\n", pLine);
          free(pLine);
          break;
        }

        case BADLINE: {
          if (((printgood) && (!bad)) || (!printgood && (bad)))
            fprintf(stdout, "%s\n", buf);
          break;
        }

        case PIPE:
        case NORMAL: {
          fprintf(stdout, "\n");
          fflush(stdout);
          break;
        }
      }
    }  // if
  }    // while

  if (bZippedOdf) {
    fclose(fileid);
    sprintf(buf, "rm %s/content.xml; rmdir %s", odftmpdir, odftmpdir);
    if (system(buf) != 0)
      perror("write failed");
  }

  delete (parser);
  freewordlist(dicwords);

}  // pipe_interface

#ifdef HAVE_READLINE

#ifdef HAVE_CURSES_H
static const char* rltext;

// set base text of input line
static int set_rltext() {
  if (rltext) {
    rl_insert_text(rltext);
    rltext = NULL;
    rl_startup_hook = (rl_hook_func_t*)NULL;
  }
  return 0;
}

#endif

// Readline escape
static int rl_escape(int count, int key) {
  rl_delete_text(0, rl_end);
  rl_done = 1;
  return 0;
}
#endif

#ifdef HAVE_CURSES_H
int expand_tab(char* dest, char* src, int limit) {
  int i = 0;
  int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
  int chpos = 0;
  for (int j = 0; (i < limit) && (src[j] != '\0') && (src[j] != '\r'); j++) {
    dest[i] = src[j];
    if (src[j] == '\t') {
      int end = 8 - (chpos % 8);
      for (int k = 0; k < end; k++) {
        dest[i] = ' ';
        i++;
        chpos++;
      }
    } else {
      i++;
      if (!u8 || (src[j] & 0xc0) != 0x80)
        chpos++;
    }
  }
  dest[i] = '\0';
  return chpos;
}

// UTF-8-aware version of strncpy (but output is always null terminated)
// What we should deal in is cursor position cells in a terminal emulator,
// i.e. the number of visual columns occupied like wcwidth/wcswidth does
// What we're really current doing is to deal in the number of characters,
// like mbstowcs which isn't quite correct, but close enough for western
// text in UTF-8
void strncpyu8(char* dest, const char* src, int begin, int n) {
  if (n) {
    int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
    for (int i = 0; i < begin + n;) {
      if (!*src)
        break;  // source is at it's end
      if (!u8 || (*src & 0xc0) != 0x80)
        i++;            // new character
      if (i > begin) {  // copy char (w/ utf-8 bytes)
        *dest++ = *src++;
        while (u8 && (*src & 0xc0) == 0x80)
          *dest++ = *src++;
      } else {  // skip char (w/ utf-8 bytes)
        ++src;
        while (u8 && (*src & 0xc0) == 0x80)
          ++src;
      }
    }
  }
  *dest = '\0';
}

// See strncpyu8 for gotchas
int strlenu8(const char* src) {
  int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
  int i = 0;
  while (*src) {
    if (!u8 || (*src & 0xc0) != 0x80)
      i++;
    ++src;
  }
  return i;
}

void dialogscreen(TextParser* parser,
                  char* token,
                  char* filename,
                  int forbidden,
                  char** wlst,
                  int ns) {
  int x, y;
  char line[MAXLNLEN];
  char line2[MAXLNLEN];
  getmaxyx(stdscr, y, x);
  clear();

  if (forbidden & SPELL_FORBIDDEN)
    printw(gettext("FORBIDDEN!"));
  else if (forbidden & SPELL_WARN)
    printw(gettext("Spelling mistake?"));

  printw(gettext("\t%s\t\tFile: %s\n\n"), chenc(token, io_enc, ui_enc),
         filename);

  // handle long lines and tabulators

  char lines[MAXPREVLINE][MAXLNLEN];
  char* pPrevLine;
  for (int i = 0; i < MAXPREVLINE; i++) {
    pPrevLine = parser->get_prevline(i);
    expand_tab(lines[i], chenc(pPrevLine, io_enc, ui_enc), MAXLNLEN);
    free(pPrevLine);
  }

  pPrevLine = parser->get_prevline(0);
  strncpy(line, pPrevLine, parser->get_tokenpos());
  free(pPrevLine);
  line[parser->get_tokenpos()] = '\0';
  int tokenbeg = expand_tab(line2, chenc(line, io_enc, ui_enc), MAXLNLEN);

  pPrevLine = parser->get_prevline(0);
  strncpy(line, pPrevLine, parser->get_tokenpos() + strlen(token));
  free(pPrevLine);
  line[parser->get_tokenpos() + strlen(token)] = '\0';
  int tokenend = expand_tab(line2, chenc(line, io_enc, ui_enc), MAXLNLEN);

  int rowindex = (tokenend - 1) / x;
  int beginrow = rowindex - tokenbeg / x;
  if (beginrow >= MAXPREVLINE)
    beginrow = MAXPREVLINE - 1;

  int ri = rowindex;
  int prevline = 0;

  for (int i = 0; i < MAXPREVLINE; i++) {
    strncpyu8(line, lines[prevline], x * rowindex, x);
    mvprintw(MAXPREVLINE + 1 - i, 0, "%s", line);
    rowindex--;
    if (rowindex == -1) {
      prevline++;
      rowindex = strlenu8(lines[prevline]) / x;
    }
  }

  strncpyu8(line, lines[0], x * (ri - beginrow), tokenbeg % x);
  mvprintw(MAXPREVLINE + 1 - beginrow, 0, "%s", line);
  attron(A_REVERSE);
  printw("%s", chenc(token, io_enc, ui_enc));
  attroff(A_REVERSE);

  mvprintw(MAXPREVLINE + 2, 0, "\n");
  for (int i = 0; i < ns; i++) {
    if ((ns > 10) && (i < 10)) {
      printw(" 0%d: %s\n", i, chenc(wlst[i], io_enc, ui_enc));
    } else {
      printw(" %d: %s\n", i, chenc(wlst[i], io_enc, ui_enc));
    }
  }

  /* TRANSLATORS: the capital letters are shortcuts, mark one letter similarly
     in your translation and translate the standalone letter accordingly later
     */
  mvprintw(y - 3, 0, "%s\n", gettext("\n[SPACE] R)epl A)ccept I)nsert U)ncap "
                                     "S)tem Q)uit e(X)it or ? for help\n"));
}

char* lower_first_char(char* token, const char* io_enc, int langnum) {
  const char* utf8str = chenc(token, io_enc, "UTF-8");
  int max = strlen(utf8str);
  w_char* u = new w_char[max];
  int len = u8_u16(u, max, utf8str);
  unsigned short idx = (u[0].h << 8) + u[0].l;
  idx = unicodetolower(idx, langnum);
  u[0].h = (unsigned char)(idx >> 8);
  u[0].l = (unsigned char)(idx & 0x00FF);
  char* scratch = (char*)malloc(max + 1 + 4);
  u16_u8(scratch, max + 4, u, len);
  delete[] u;
  char* result = chenc(scratch, "UTF-8", io_enc);
  if (result != scratch) {
    free(scratch);
    result = mystrdup(result);
  }
  return result;
}

// for terminal interface
int dialog(TextParser* parser,
           Hunspell* pMS,
           char* token,
           char* filename,
           char** wlst,
           int ns,
           int forbidden) {
  char buf[MAXLNLEN];
  char* buf2;
  wordlist* dicwords = NULL;
  int c;

  dialogscreen(parser, token, filename, forbidden, wlst, ns);

  char firstletter = '\0';

  while ((c = getch())) {
    switch (c) {
      case '0':
      case '1':
        if ((firstletter == '\0') && (ns > 10)) {
          firstletter = c;
          break;
        }
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        modified = 1;
        if ((firstletter != '\0') && (firstletter == '1')) {
          c += 10;
        }
        c -= '0';
        if (c >= ns)
          break;
        if (checkapos) {
          strcpy(buf, wlst[c]);
          parser->change_token(mystrrep(buf, "'", UTF8_APOS));
        } else {
          parser->change_token(wlst[c]);
        }
        freewordlist(dicwords);
        return 0;
      case ' ':
        freewordlist(dicwords);
        return 0;
      case '?':
        clear();
        printw(gettext(
            "Whenever a word is found that is not in the dictionary\n"
            "it is printed on the first line of the screen.  If the "
            "dictionary\n"
            "contains any similar words, they are listed with a number\n"
            "next to each one.  You have the option of replacing the word\n"
            "completely, or choosing one of the suggested words.\n"));
        printw(gettext("\nCommands are:\n\n"));
        printw(gettext("R	Replace the misspelled word completely.\n"));
        printw(gettext("Space	Accept the word this time only.\n"));
        printw(
            gettext("A	Accept the word for the rest of this session.\n"));
        printw(gettext(
            "I	Accept the word, and put it in your private dictionary.\n"));
        printw(gettext(
            "U	Accept and add lowercase version to private dictionary.\n"));
        printw(
            gettext("S\tAsk a stem and a model word and store them in the "
                    "private dictionary.\n"
                    "\tThe stem will be accepted also with the affixes of the "
                    "model word.\n"));
        printw(gettext("0-n	Replace with one of the suggested words.\n"));
        printw(gettext(
            "X	Write the rest of this file, ignoring misspellings, and start "
            "next file.\n"));
        printw(
            gettext("Q	Quit immediately. Asks for confirmation. Leaves file "
                    "unchanged.\n"));
        printw(gettext("^Z	Suspend program. Restart with fg command.\n"));
        printw(gettext("?	Show this help screen.\n"));
        printw(gettext("\n-- Type space to continue -- \n"));
        while (getch() != ' ')
          ;
      // fall-through
      case 12: {
        dialogscreen(parser, token, filename, forbidden, wlst, ns);
        break;
      }
      default: {
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "R)epl" before */
        if (c == (gettext("r"))[0]) {
          char i[MAXLNLEN];
          char* temp;

          modified = 1;

#ifdef HAVE_READLINE
          endwin();
          rltext = "";
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          temp = readline(gettext("Replace with: "));
#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            break;
          }

          strncpy(i, temp, MAXLNLEN - 1);
          i[MAXLNLEN - 1] = '\0';
          free(temp);
          parser->change_token(checkapos ? mystrrep(i, "'", UTF8_APOS) : i);

          freewordlist(dicwords);
          return 2;  // replace
        }
        /* TRANSLATORS: translate these letters according to the shortcut letter
           used
           previously in the  translation of "U)ncap" and I)nsert before */
        int u_key = gettext("u")[0];
        int i_key = gettext("i")[0];

        if (c == u_key || c == i_key) {
          struct wordlist* i =
              (struct wordlist*)malloc(sizeof(struct wordlist));
          i->word = (c == i_key)
                        ? mystrdup(token)
                        : lower_first_char(token, io_enc, pMS->get_langnum());
          i->next = dicwords;
          dicwords = i;
          // save
          if (HOME) {
            strncpy(buf, HOME, MAXLNLEN - 1);
            buf[MAXLNLEN - 1] = '\0';
          } else {
            fprintf(stderr, gettext("error - missing HOME variable\n"));
            break;
          }
#ifndef WIN32
          strcat(buf, "/");
#endif
          buf2 = buf + strlen(buf);
          if (!privdicname) {
            strcat(buf, DICBASENAME);
            strcat(buf, basename(dicname, DIRSEPCH));
          } else {
            strcat(buf, privdicname);
          }
          if (save_privdic(buf2, buf, dicwords)) {
            dicwords = NULL;
          } else {
            fprintf(stderr, gettext("Cannot update personal dictionary."));
            break;
          }
        }  // no break
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "U)ncap" and I)nsert before */
        if ((c == (gettext("u"))[0]) || (c == (gettext("i"))[0]) ||
            (c == (gettext("a"))[0])) {
          modified = 1;
          putdic(token, pMS);
          freewordlist(dicwords);
          return 0;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "S)tem" before */
        if (c == (gettext("s"))[0]) {
          modified = 1;

          char w[MAXLNLEN], w2[MAXLNLEN], w3[MAXLNLEN];
          char* temp;

          strncpy(w, token, MAXLNLEN - 1);
          token[MAXLNLEN - 1] = '\0';
          temp = basename(w, '-');
          if (w < temp) {
            *(temp - 1) = '\0';
          } else {
#ifdef HUNSPELL_EXPERIMENTAL
            char** poslst = NULL;
            int ps = pMS->suggest_pos_stems(&poslst, token);
            if (ps > 0) {
              strcpy(buf, poslst[0]);
              for (int i = 0; i < ps; i++) {
                if (strlen(poslst[i]) <= strlen(buf))
                  strcpy(buf, poslst[i]);
                free(poslst[i]);
              }
              strcpy(w, buf);
            }
            if (poslst)
              free(poslst);
#endif
          }

#ifdef HAVE_READLINE
          endwin();
          rltext = w;
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          temp = readline(gettext("New word (stem): "));

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
#ifdef HAVE_READLINE
            initscr();
            cbreak();
#endif
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            break;
          }

          strncpy(w, temp, MAXLNLEN - 1);
          w[MAXLNLEN - 1] = '\0';
          free(temp);

#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif
          dialogscreen(parser, token, filename, forbidden, wlst, ns);
          refresh();

#ifdef HAVE_READLINE
          endwin();
          rltext = "";
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          temp = readline(gettext("Model word (a similar dictionary word): "));

#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            break;
          }

          strncpy(w2, temp, MAXLNLEN - 1);
          temp[MAXLNLEN - 1] = '\0';
          free(temp);

          if (strlen(w) + strlen(w2) + 2 < MAXLNLEN) {
            sprintf(w3, "%s/%s", w, w2);
          } else
            break;

          if (!putdic(w3, pMS)) {
            struct wordlist* i =
                (struct wordlist*)malloc(sizeof(struct wordlist));
            i->word = mystrdup(w3);
            i->next = dicwords;
            dicwords = i;

            if (strlen(w) + strlen(w2) + 4 < MAXLNLEN) {
              sprintf(w3, "%s-/%s-", w, w2);
              if (putdic(w3, pMS)) {
                struct wordlist* i =
                    (struct wordlist*)malloc(sizeof(struct wordlist));
                i->word = mystrdup(w3);
                i->next = dicwords;
                dicwords = i;
              }
            }
            // save

            if (HOME) {
              strncpy(buf, HOME, MAXLNLEN - 1);
              buf[MAXLNLEN - 1] = '\0';
            } else {
              fprintf(stderr, gettext("error - missing HOME variable\n"));
              continue;
            }
#ifndef WIN32
            strcat(buf, "/");
#endif
            buf2 = buf + strlen(buf);
            if (!privdicname) {
              strcat(buf, DICBASENAME);
              strcat(buf, basename(dicname, DIRSEPCH));
            } else {
              strcat(buf, privdicname);
            }
            if (save_privdic(buf2, buf, dicwords)) {
              dicwords = NULL;
            } else {
              fprintf(stderr, gettext("Cannot update personal dictionary."));
              break;
            }

          } else {
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            printw(gettext(
                "Model word must be in the dictionary. Press any key!"));
            getch();
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            break;
          }
          freewordlist(dicwords);
          return 0;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "e(X)it" before */
        if (c == (gettext("x"))[0]) {
          freewordlist(dicwords);
          return 1;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "Q)uit" before */
        if (c == (gettext("q"))[0]) {
          if (modified) {
            printw(
                gettext("Are you sure you want to throw away your changes? "));
            /* TRANSLATORS: translate this letter according to the shortcut
             * letter y)es */
            if (getch() == (gettext("y"))[0]) {
              freewordlist(dicwords);
              return -1;
            }
            dialogscreen(parser, token, filename, forbidden, wlst, ns);
            break;
          } else {
            freewordlist(dicwords);
            return -1;
          }
        }
      }
    }
  }
  freewordlist(dicwords);
  return 0;
}

int interactive_line(TextParser* parser,
                     Hunspell** pMS,
                     char* filename,
                     FILE* tempfile) {
  char* token;
  int dialogexit = 0;
  int info = 0;
  int d = 0;
  while ((token = parser->next_token())) {
    if (!check(pMS, &d, token, &info, NULL)) {
      dialogscreen(parser, token, filename, info, NULL, 0);  // preview
      refresh();
      char** wlst = NULL;
      std::string buf(token);
      int ns = pMS[d]->suggest(
          &wlst,
          mystrrep(chenc(buf, io_enc, dic_enc[d]), ENTITY_APOS, "'").c_str());
      if (ns == 0) {
        dialogexit = dialog(parser, pMS[d], token, filename, wlst, ns, info);
      } else {
        for (int j = 0; j < ns; j++) {
          char d2io[MAXLNLEN];
          strcpy(d2io, chenc(wlst[j], dic_enc[d], io_enc));
          wlst[j] = (char*)realloc(wlst[j], strlen(d2io) + 1);
          strcpy(wlst[j], d2io);
        }
        dialogexit = dialog(parser, pMS[d], token, filename, wlst, ns, info);
      }
      for (int j = 0; j < ns; j++) {
        free(wlst[j]);
      }
      free(wlst);
    }
    free(token);
    if ((dialogexit == -1) || (dialogexit == 1))
      goto ki2;
  }

ki2:
  fprintf(tempfile, "%s", token = parser->get_line());
  free(token);
  return dialogexit;
}

void interactive_interface(Hunspell** pMS, char* filename, int format) {
  char buf[MAXLNLEN];
  char* odffilename = NULL;
  char* odftempdir;  // external zip works only with temporary directories
                     // (option -j)

  FILE* text = fopen(filename, "r");
  if (!text) {
    perror(gettext("Can't open inputfile"));
    endwin();
    exit(1);
  }

  int dialogexit;
  int check = 1;

  char* extension = basename(filename, '.');
  TextParser* parser = get_parser(format, extension, pMS[0]);

  bool bZippedOdf = is_zipped_odf(parser, extension);
  // access content.xml of ODF
  if (bZippedOdf) {
    odftempdir = tmpnam(NULL);
    fclose(text);
    // break 1-line XML of zipped ODT documents at </style:style> and </text:p>
    // to avoid tokenization problems (fgets could stop within an XML tag)
    sprintf(buf,
            "mkdir %s && unzip -p '%s' content.xml | sed "
            "'s/\\(<\\/text:p>\\|<\\/style:style>\\)\\(.\\)/\\1\\\n\\2/g' "
            ">%s/content.xml",
            odftempdir, filename, odftempdir);
    if (!secure_filename(filename) || system(buf) != 0) {
      if (secure_filename(filename))
        perror(gettext("Can't open inputfile"));
      else
        fprintf(stderr, gettext("Can't open %s.\n"), filename);
      endwin();
      exit(1);
    }
    odffilename = filename;
    sprintf(buf, "%s/content.xml", odftempdir);
    filename = mystrdup(buf);
    text = fopen(filename, "r");
    if (!text) {
      perror(gettext("Can't open inputfile"));
      endwin();
      exit(1);
    }
  }

  FILE* tempfile = tmpfile();

  if (!tempfile) {
    perror(gettext("Can't create tempfile"));
    delete parser;
    fclose(text);
    endwin();
    exit(1);
  }

  while (fgets(buf, MAXLNLEN, text)) {
    if (check) {
      parser->put_line(buf);
      dialogexit = interactive_line(
          parser, pMS, odffilename ? odffilename : filename, tempfile);
      switch (dialogexit) {
        case -1: {
          clear();
          refresh();
          fclose(tempfile);  // automatically deleted when closed
          if (bZippedOdf) {
            sprintf(buf, "rm %s; rmdir %s", filename, odftempdir);
            if (system(buf) != 0)
              perror("write failed");
            free(filename);
          }
          endwin();
          exit(0);
        }
        case 1: {
          check = 0;
        }
      }
    } else {
      fprintf(tempfile, "%s", buf);
    }
  }
  fclose(text);

  if (modified) {
    rewind(tempfile);
    text = fopen(filename, "wb");
    if (text == NULL)
      perror(gettext("Can't open outputfile"));
    else {
      size_t n;
      while ((n = fread(buf, 1, MAXLNLEN, tempfile)) > 0) {
        if (fwrite(buf, 1, n, text) != n)
          perror("write failed");
      }
      fclose(text);
      if (bZippedOdf) {
        sprintf(buf, "zip -j '%s' %s", odffilename, filename);
        if (system(buf) != 0)
          perror("write failed");
      }
    }
  }

  if (bZippedOdf) {
    sprintf(buf, "rm %s; rmdir %s", filename, odftempdir);
    if (system(buf) != 0)
      perror("write failed");
    free(filename);
  }

  delete parser;
  fclose(tempfile);  // automatically deleted when closed
}

#endif

char* add(char* dest, const char* st) {
  if (!dest) {
    dest = mystrdup(st);
  } else {
    dest = (char*)realloc(dest, strlen(dest) + strlen(st) + 1);
    strcat(dest, st);
  }
  return dest;
}

char* exist2(char* dir, int len, const char* name, const char* ext) {
  std::string buf;
  const char* sep = (len == 0) ? "" : DIRSEP;
  buf.assign(dir, len);
  buf.append(sep);
  buf.append(name);
  buf.append(ext);
  if (exist(buf.c_str()))
    return mystrdup(buf.c_str());
  buf.append(HZIP_EXTENSION);
  if (exist(buf.c_str())) {
    buf.erase(buf.size() - strlen(HZIP_EXTENSION));
    return mystrdup(buf.c_str());
  }
  return NULL;
}

#ifndef WIN32
int listdicpath(char* dir, int len) {
  std::string buf;
  const char* sep = (len == 0) ? "" : DIRSEP;
  buf.assign(dir, len);
  buf.append(sep);
  DIR* d = opendir(buf.c_str());
  if (!d)
    return 0;
  struct dirent* de;
  while ((de = readdir(d))) {
    len = strlen(de->d_name);
    if ((len > 4 && strcmp(de->d_name + len - 4, ".dic") == 0) ||
        (len > 7 && strcmp(de->d_name + len - 7, ".dic.hz") == 0)) {
      char* s = mystrdup(de->d_name);
      s[len - ((s[len - 1] == 'z') ? 7 : 4)] = '\0';
      fprintf(stderr, "%s%s\n", buf.c_str(), s);
      free(s);
    }
  }
  closedir(d);
  return 1;
}
#endif

// search existing path for file "name + ext"
char* search(char* begin, char* name, const char* ext) {
  char* end = begin;
  while (1) {
    while (!((*end == *PATHSEP) || (*end == '\0')))
      end++;
    char* res = NULL;
    if (name) {
      res = exist2(begin, end - begin, name, ext);
    } else {
#ifndef WIN32
      listdicpath(begin, end - begin);
#endif
    }
    if ((*end == '\0') || res)
      return res;
    end++;
    begin = end;
  }
}

int main(int argc, char** argv) {
  std::string buf;
  Hunspell* pMS[DMAX];
  char* key = NULL;
  int arg_files = -1;  // first filename argumentum position in argv
  int format = FMT_TEXT;
  int argstate = 0;

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
  setlocale(LC_ALL, "");
  textdomain("hunspell");
#ifdef HAVE_LANGINFO_CODESET
  ui_enc = nl_langinfo(CODESET);
#endif
#endif
#endif

#ifdef HAVE_READLINE
  rl_set_key("", rl_escape, rl_get_keymap());
  rl_bind_key('\t', rl_insert);
#endif

#ifdef LOG
  log("START");
#endif

  for (int i = 1; i < argc; i++) {
#ifdef LOG
    log(argv[i]);
#endif

    if (argstate == 1) {
      if (dicname)
        free(dicname);
      dicname = mystrdup(argv[i]);
      argstate = 0;
    } else if (argstate == 2) {
      if (privdicname)
        free(privdicname);
      privdicname = mystrdup(argv[i]);
      argstate = 0;
    } else if (argstate == 3) {
      io_enc = argv[i];
      argstate = 0;
    } else if (argstate == 4) {
      key = argv[i];
      argstate = 0;
    } else if (strcmp(argv[i], "-d") == 0)
      argstate = 1;
    else if (strcmp(argv[i], "-p") == 0)
      argstate = 2;
    else if (strcmp(argv[i], "-i") == 0)
      argstate = 3;
    else if (strcmp(argv[i], "-P") == 0)
      argstate = 4;
    else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
      fprintf(stderr, gettext("Usage: hunspell [OPTION]... [FILE]...\n"));
      fprintf(stderr, gettext("Check spelling of each FILE. Without FILE, "
                              "check standard input.\n\n"));
      fprintf(stderr, gettext("  -1\t\tcheck only first field in lines "
                              "(delimiter = tabulator)\n"));
      fprintf(stderr, gettext("  -a\t\tIspell's pipe interface\n"));
      fprintf(stderr, gettext("  --check-url\tcheck URLs, e-mail addresses and "
                              "directory paths\n"));
      fprintf(
          stderr,
          gettext(
              "  --check-apostrophe\tcheck Unicode typographic apostrophe\n"));
      fprintf(stderr,
              gettext("  -d d[,d2,...]\tuse d (d2 etc.) dictionaries\n"));
      fprintf(stderr, gettext("  -D\t\tshow available dictionaries\n"));
      fprintf(stderr, gettext("  -G\t\tprint only correct words or lines\n"));
      fprintf(stderr, gettext("  -h, --help\tdisplay this help and exit\n"));
      fprintf(stderr, gettext("  -H\t\tHTML input file format\n"));
      fprintf(stderr, gettext("  -i enc\tinput encoding\n"));
      fprintf(stderr, gettext("  -l\t\tprint misspelled words\n"));
      fprintf(stderr, gettext("  -L\t\tprint lines with misspelled words\n"));
      fprintf(stderr,
              gettext("  -m \t\tanalyze the words of the input text\n"));
      fprintf(stderr, gettext("  -n\t\tnroff/troff input file format\n"));
      fprintf(
          stderr,
          gettext(
              "  -O\t\tOpenDocument (ODF or Flat ODF) input file format\n"));
      fprintf(stderr, gettext("  -p dict\tset dict custom dictionary\n"));
      fprintf(stderr,
              gettext("  -r\t\twarn of the potential mistakes (rare words)\n"));
      fprintf(
          stderr,
          gettext("  -P password\tset password for encrypted dictionaries\n"));
      fprintf(stderr, gettext("  -s \t\tstem the words of the input text\n"));
      fprintf(stderr, gettext("  -S \t\tsuffix words of the input text\n"));
      fprintf(stderr, gettext("  -t\t\tTeX/LaTeX input file format\n"));
      // experimental functions: missing Unicode support
      //			fprintf(stderr,gettext("  -u\t\tshow typical
      //misspellings\n"));
      //			fprintf(stderr,gettext("  -u2\t\tprint typical
      //misspellings in sed format\n"));
      //			fprintf(stderr,gettext("  -u3\t\tprint typical
      //misspellings in gcc error format\n"));
      //			fprintf(stderr,gettext("  -U\t\tautomatic
      //correction of typical misspellings to stdout\n"));
      fprintf(stderr, gettext("  -v, --version\tprint version number\n"));
      fprintf(stderr,
              gettext("  -vv\t\tprint Ispell compatible version number\n"));
      fprintf(stderr, gettext("  -w\t\tprint misspelled words (= lines) from "
                              "one word/line input.\n"));
      fprintf(stderr, gettext("  -X\t\tXML input file format\n\n"));
      fprintf(
          stderr,
          gettext(
              "Example: hunspell -d en_US file.txt    # interactive spelling\n"
              "         hunspell -i utf-8 file.txt    # check UTF-8 encoded "
              "file\n"
              "         hunspell -l *.odt             # print misspelled words "
              "of ODF files\n\n"
              "         # Quick fix of ODF documents by personal dictionary "
              "creation\n\n"
              "         # 1 Make a reduced list from misspelled and unknown "
              "words:\n\n"
              "         hunspell -l *.odt | sort | uniq >words\n\n"
              "         # 2 Delete misspelled words of the file by a text "
              "editor.\n"
              "         # 3 Use this personal dictionary to fix the deleted "
              "words:\n\n"
              "         hunspell -p words *.odt\n\n"));
      fprintf(stderr, gettext("Bug reports: http://hunspell.github.io/\n"));
      exit(0);
    } else if ((strcmp(argv[i], "-vv") == 0) || (strcmp(argv[i], "-v") == 0) ||
               (strcmp(argv[i], "--version") == 0)) {
      fprintf(stdout, gettext(HUNSPELL_PIPE_HEADING));
      fprintf(stdout, "\n");
      if (strcmp(argv[i], "-vv") != 0) {
        fprintf(stdout,
                gettext("\nCopyright (C) 2002-2014 L\303\241szl\303\263 "
                        "N\303\251meth. License: MPL/GPL/LGPL.\n\n"
                        "Based on OpenOffice.org's Myspell library.\n"
                        "Myspell's copyright (C) Kevin Hendricks, 2001-2002, "
                        "License: BSD.\n\n"));
        fprintf(stdout, gettext("This is free software; see the source for "
                                "copying conditions.  There is NO\n"
                                "warranty; not even for MERCHANTABILITY or "
                                "FITNESS FOR A PARTICULAR PURPOSE,\n"
                                "to the extent permitted by law.\n"));
      }
      exit(0);
    } else if ((strcmp(argv[i], "-a") == 0)) {
      filter_mode = PIPE;
    } else if ((strcmp(argv[i], "-m") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Make possible root/affix combinations that aren't in the
       dictionary.
       hunspell: Analyze the words of the input text
      */
      if (filter_mode != PIPE)
        filter_mode = ANALYZE;
    } else if ((strcmp(argv[i], "-s") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Stop itself with a SIGTSTP signal after each line of input.
       hunspell: Stem the words of the input text
      */
      if (filter_mode != PIPE)
        filter_mode = STEM;
    } else if ((strcmp(argv[i], "-S") == 0)) {
      if (filter_mode != PIPE)
        filter_mode = SUFFIX;
    } else if ((strcmp(argv[i], "-t") == 0)) {
      format = FMT_LATEX;
    } else if ((strcmp(argv[i], "-n") == 0)) {
      format = FMT_MAN;
    } else if ((strcmp(argv[i], "-H") == 0)) {
      format = FMT_HTML;
    } else if ((strcmp(argv[i], "-X") == 0)) {
      format = FMT_XML;
    } else if ((strcmp(argv[i], "-O") == 0)) {
      format = FMT_ODF;
    } else if ((strcmp(argv[i], "-l") == 0)) {
      filter_mode = BADWORD;
    } else if ((strcmp(argv[i], "-w") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Specify additional characters that can be part of a word.
       hunspell: Print misspelled words (= lines) from one word/line input
      */
      if (filter_mode != PIPE)
        filter_mode = WORDFILTER;
    } else if ((strcmp(argv[i], "-L") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Number of lines of context to be shown at the bottom of the
       screen
       hunspell: Print lines with misspelled words
      */
      if (filter_mode != PIPE)
        filter_mode = BADLINE;
    } else if ((strcmp(argv[i], "-u") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Show typical misspellings
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO0;
    } else if ((strcmp(argv[i], "-U") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Automatic correction of typical misspellings to stdout
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO;
    } else if ((strcmp(argv[i], "-u2") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Print typical misspellings in sed format
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO2;
    } else if ((strcmp(argv[i], "-u3") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Print typical misspellings in gcc error format
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO3;
    } else if ((strcmp(argv[i], "-G") == 0)) {
      printgood = 1;
    } else if ((strcmp(argv[i], "-1") == 0)) {
      format = FMT_FIRST;
    } else if ((strcmp(argv[i], "-D") == 0)) {
      showpath = 1;
    } else if ((strcmp(argv[i], "-r") == 0)) {
      warn = 1;
    } else if ((strcmp(argv[i], "--check-url") == 0)) {
      checkurl = 1;
    } else if ((strcmp(argv[i], "--check-apostrophe") == 0)) {
      checkapos = 1;
    } else if ((arg_files == -1) &&
               ((argv[i][0] != '-') && (argv[i][0] != '\0'))) {
      arg_files = i;
      if (!exist(argv[i])) {  // first check (before time-consuming dic. load)
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
#ifdef HAVE_CURSES_H
        endwin();
#endif
        exit(1);
      }
    }
  }

  if (printgood && (filter_mode == NORMAL))
    filter_mode = BADWORD;

  if (!dicname) {
    if (!(dicname = getenv("DICTIONARY"))) {
      /*
       * Search in order of LC_ALL, LC_MESSAGES &
       * LANG
      */
      const char* tests[] = {"LC_ALL", "LC_MESSAGES", "LANG"};
      for (size_t i = 0; i < sizeof(tests) / sizeof(const char*); ++i) {
        if ((dicname = getenv(tests[i])) && strcmp(dicname, "") != 0) {
          dicname = mystrdup(dicname);
          char* dot = strchr(dicname, '.');
          if (dot)
            *dot = '\0';
          char* at = strchr(dicname, '@');
          if (at)
            *at = '\0';
          break;
        }
      }

      if (dicname &&
          ((strcmp(dicname, "C") == 0) || (strcmp(dicname, "POSIX") == 0))) {
        free(dicname);
        dicname = mystrdup("en_US");
      }

      if (!dicname) {
        dicname = mystrdup(DEFAULTDICNAME);
      }
    } else {
      dicname = mystrdup(dicname);
    }
  }
  path = add(mystrdup("."), PATHSEP);  // <- check path in local directory
  path = add(path, PATHSEP);           // <- check path in root directory
  if (getenv("DICPATH"))
    path = add(add(path, getenv("DICPATH")), PATHSEP);
  path = add(add(path, LIBDIR), PATHSEP);
  if (HOME)
    path = add(add(add(add(path, HOME), DIRSEP), USEROOODIR), PATHSEP);
  path = add(path, OOODIR);

  if (showpath) {
    fprintf(stderr, gettext("SEARCH PATH:\n%s\n"), path);
    fprintf(
        stderr,
        gettext(
            "AVAILABLE DICTIONARIES (path is not mandatory for -d option):\n"));
    search(path, NULL, NULL);
  }

  if (!privdicname)
    privdicname = mystrdup(getenv("WORDLIST"));

  char* dicplus = strchr(dicname, ',');
  if (dicplus)
    *dicplus = '\0';
  char* aff = search(path, dicname, ".aff");
  char* dic = search(path, dicname, ".dic");
  if (aff && dic) {
    if (showpath) {
      fprintf(stderr, gettext("LOADED DICTIONARY:\n%s\n%s\n"), aff, dic);
    }
    pMS[0] = new Hunspell(aff, dic, key);
    dic_enc[0] = pMS[0]->get_dic_encoding();
    dmax = 1;
    while (dicplus) {
      char* dicname2 = dicplus + 1;
      dicplus = strchr(dicname2, ',');
      if (dicplus)
        *dicplus = '\0';
      free(aff);
      free(dic);
      aff = search(path, dicname2, ".aff");
      dic = search(path, dicname2, ".dic");
      if (aff && dic) {
        if (dmax < DMAX) {
          pMS[dmax] = new Hunspell(aff, dic, key);
          dic_enc[dmax] = pMS[dmax]->get_dic_encoding();
          dmax++;
        } else
          fprintf(stderr, gettext("error - %s exceeds dictionary limit.\n"),
                  dicname2);
      } else if (dic)
        pMS[dmax - 1]->add_dic(dic);
    }
  } else {
    fprintf(stderr, gettext("Can't open affix or dictionary files for "
                            "dictionary named \"%s\".\n"),
            dicname);
    exit(1);
  }

  /* open the private dictionaries */
  if (HOME) {
    buf.assign(HOME);
#ifndef WIN32
    buf.append("/");
#endif
    buf.append(DICBASENAME);
    buf.append(basename(dicname, DIRSEPCH));
    load_privdic(buf.c_str(), pMS[0]);
    buf.assign(HOME);
#ifndef WIN32
    buf.append("/");
#endif
    if (!privdicname) {
      buf.assign(DICBASENAME);
      buf.append(basename(dicname, DIRSEPCH));
      load_privdic(buf.c_str(), pMS[0]);
    } else {
      buf.append(privdicname);
      load_privdic(buf.c_str(), pMS[0]);
      buf.assign(privdicname);
      load_privdic(buf.c_str(), pMS[0]);
    }
  }

  /*
     If in pipe mode, output pipe mode version string only when
     hunspell has properly been started.
     Emacs and may be others relies in the English version format.
     Do not gettextize.
  */
  if (filter_mode == PIPE) {
    fprintf(stdout, HUNSPELL_PIPE_HEADING);
    fflush(stdout);
  }

  if (arg_files == -1) {
    pipe_interface(pMS, format, stdin, NULL);
  } else if (filter_mode != NORMAL) {
    for (int i = arg_files; i < argc; i++) {
      if (exist(argv[i])) {
        modified = 0;
        currentfilename = argv[i];
        FILE* f = fopen(argv[i], "r");
        pipe_interface(pMS, format, f, argv[i]);
        fclose(f);
      } else {
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
        exit(1);
      }
    }
  } else if (filter_mode == NORMAL) {
#ifdef HAVE_CURSES_H
    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);

    for (int i = arg_files; i < argc; i++) {
      if (exist(argv[i])) {
        modified = 0;
        interactive_interface(pMS, argv[i], format);
      } else {
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
        endwin();
        exit(1);
      }
    }

    clear();
    refresh();
    endwin();
#else
    fprintf(
        stderr,
        gettext(
            "Hunspell has been compiled without Ncurses user interface.\n"));
#endif
  }

  if (dicname)
    free(dicname);
  if (privdicname)
    free(privdicname);
  if (path)
    free(path);
  if (aff)
    free(aff);
  if (dic)
    free(dic);
  if (wordchars)
    free(wordchars);
  if (wordchars_utf16_free)
    free(const_cast<w_char*>(wordchars_utf16));
#ifdef HAVE_ICONV
  free_utf_tbl();
#endif
  for (int i = 0; i < dmax; i++)
    delete pMS[i];
  return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef LANGNUM_HXX_
#define LANGNUM_HXX_

/*
 language numbers for language specific codes
 see https://wiki.openoffice.org/w/index.php?title=Languages&oldid=230199
*/

enum {
  LANG_ar = 96,
  LANG_az = 100,  // custom number
  LANG_bg = 41,
  LANG_ca = 37,
  LANG_crh = 102, // custom number
  LANG_cs = 42,
  LANG_da = 45,
  LANG_de = 49,
  LANG_el = 30,
  LANG_en = 01,
  LANG_es = 34,
  LANG_eu = 10,
  LANG_fr = 02,
  LANG_gl = 38,
  LANG_hr = 78,
  LANG_hu = 36,
  LANG_it = 39,
  LANG_la = 99,   // custom number
  LANG_lv = 101,  // custom number
  LANG_nl = 31,
  LANG_pl = 48,
  LANG_pt = 03,
  LANG_ru = 07,
  LANG_sv = 50,
  LANG_tr = 90,
  LANG_uk = 80,
  LANG_xx = 999
};

#endif

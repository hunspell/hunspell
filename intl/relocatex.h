/* 
   Copyright (C) 2006 Free Software Foundation, Inc.
   This file is part of the GnuWin C Library.

   The GnuWin C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GnuWin C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GnuWin32 C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef __RELOCATE_H__
#define __RELOCATE_H__ 1

/* #include <libc-dll.h> */

#ifdef __cplusplus
extern "C" {
#endif

char *relocaten (const char *ModuleName, const char *path);
char *relocaten2 (const char *ModuleName, const char *installdir, const char *path);
char *relocatenx (const char *ModuleName, const char *installdir, const char *path);
char *relocate2 (const char *installdir, const char *path);
char *relocatex (const char *installdir, const char *path);

#ifdef __cplusplus
}
#endif

//#endif /*  __GW32__ */

#endif /* __RELOCATE_H__ */

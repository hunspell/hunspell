/* Provide relocatable packages.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */


/* Specification.  */
#include <errno.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
/* #include <path.h> */
#include "relocatex.h"
#include "canonicalize.h"
/* #include <gw32.h> */


#if defined _WIN32 || defined __WIN32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
//# define __GW32__
//# include <winx/errnox.h>
#endif
#define set_werrno

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
  /* Win32, Cygwin, OS/2, DOS */
# define ISDIRSEP(C) ((C) == '/' || (C) == '\\')
#else
  /* Unix */
# define ISDIRSEP(C) ((C) == '/')
#endif

/* Original installation prefix.  */
static char *orig_prefix = NULL;
static size_t orig_prefix_len = 0;
/* Current installation prefix.  */
static char *curr_prefix = NULL;
static size_t curr_prefix_len = 0;
/* These prefixes do not end in a slash.  Anything that will be concatenated
   to them must start with a slash.  */


int win2posixpath (const char *winpath, char *posixpath)
{
	strcpy (posixpath, winpath);
	win2unixpath (posixpath);
	return 0;
}


/* Sets the original and the current installation prefix of this module.
   Relocation simply replaces a pathname starting with the original prefix
   by the corresponding pathname with the current prefix instead.  Both
   prefixes should be directory names without trailing slash (i.e. use ""
   instead of "/").  */
static char *
set_orig_prefix (const char *orig_prefix_arg)
{
      char *memory;
//	  printf ("orig_prefix_arg: %s\n", orig_prefix_arg);
	  if (!orig_prefix_arg) {
		orig_prefix = NULL;
		orig_prefix_len = 0;
		return NULL;
	  }
	  if (orig_prefix)
		  free (orig_prefix);

	  memory = canonicalize_file_name (orig_prefix_arg);
//	  printf ("memory: %s\n", memory);
//	  memory = (char *) malloc (orig_prefix_len + 1);
      if (!memory) {
	  	set_werrno;
		orig_prefix = NULL;
		orig_prefix_len = 0;
		return NULL;
      }
	  win2unixpath (memory);
//	  win2posixpath (orig_prefix_arg, memory);
	  orig_prefix = memory;
	  orig_prefix_len = strlen (orig_prefix);
//	  printf ("orig_prefix: %s\n", orig_prefix);
	  if (ISDIRSEP (orig_prefix[orig_prefix_len-1])) {
	  	orig_prefix[orig_prefix_len-1] = '\0';
	  	orig_prefix_len--;
	  }
//	  printf ("orig_prefix: %s\n", orig_prefix);
//	  printf ("orig_prefix_len: %d\n", orig_prefix_len);
	  return orig_prefix;
}

#if defined __WIN32__
static char *
set_current_prefix (const char *ModuleName)
{
	LPTSTR curr_prefix_arg, q, lpFilePart;
	DWORD len;
	int nDIRSEP = 0;

	if (curr_prefix)
		free (curr_prefix);
	curr_prefix_arg = malloc (MAX_PATH * sizeof (TCHAR));
	if (!curr_prefix_arg) {
		set_werrno;
		curr_prefix = NULL;
		curr_prefix_len = 0;
		return NULL;
	}		
	if (ModuleName) {
//		printf ("ModuleName:  %s\n", ModuleName);
		len = SearchPath (NULL, ModuleName, ".DLL", MAX_PATH, curr_prefix_arg, &lpFilePart);
		if (len) {
//			printf ("ModulePath:  %s\n", curr_prefix_arg);
//			printf ("FilePart:    %s\n", lpFilePart);
		}
	}
	if (!ModuleName || !len) {
		len = GetModuleFileName (NULL, curr_prefix_arg, MAX_PATH);
		if (!len) {
			set_werrno;
			curr_prefix = NULL;
			curr_prefix_len = 0;
			return NULL;
		}
	}
//		strncpy (curr_prefix_arg, ModuleName, MAX_PATH);
//	  printf ("curr_prefix_arg: %s\n", curr_prefix_arg);
	win2posixpath (curr_prefix_arg, curr_prefix_arg);
	curr_prefix = curr_prefix_arg;
	q = curr_prefix_arg + len - 1;
	/* strip name of executable and its directory */
	while (!ISDIRSEP (*q) && (q > curr_prefix_arg) && nDIRSEP < 2) {
		q--;
		if (ISDIRSEP (*q)) {
			*q = '\0';
			nDIRSEP++;
		}
	}
	curr_prefix_len = q - curr_prefix_arg; 
//	printf ("curr_prefix: %s\n", curr_prefix);
//	printf ("curr_prefix_len: %d\n", curr_prefix_len);
	return curr_prefix;
}

char *getshortpath (const char *longpath)
{
	char *shortpath = NULL;
	DWORD len, res;
	
//	printf ("longpath: %s\n", longpath);
	len = GetShortPathName(longpath, shortpath, 0);
//	printf ("len: %ld\n", len);
	if (!len) {
//		WinErr ("getshortpath: len = 0");
		set_werrno;
		return (char *) longpath;
	}
	shortpath = (char *) malloc (len + 1);
	if (!shortpath) {
//		WinErr ("getshortpath: malloc");
		set_werrno;
		return (char *) longpath;
	}
	res = GetShortPathName(longpath, shortpath, len);
//	printf ("res: %ld\n", res);
	if (!res) {
//		WinErr ("getshortpath: res = 0");
		free (shortpath);
		set_werrno;
		return (char *) longpath;
	}
//	printf ("shortpath: %s\n", shortpath);
	return shortpath;
}

char *relocaten (const char *ModuleName, const char *path)
{
	char *relative_path, *relocated_path, *relocated_short_path;
	int relative_path_len;
	
	if (!curr_prefix)
		set_current_prefix (ModuleName);
//	printf ("path:                 %s\n", path);
//	printf ("orig_prefix:          %s\n", orig_prefix);
//	printf ("curr_prefix:          %s\n", curr_prefix);
//	if (strncmp (orig_prefix, path, orig_prefix_len))
//	if (strcmp (orig_prefix, path))
//		return (char *) path;
	relative_path = (char *) path + orig_prefix_len;
//	printf ("relative_path:        %s\n", relative_path);
	relative_path_len = strlen (relative_path);
	relocated_path = malloc (curr_prefix_len + relative_path_len + 1);
	strcpy (relocated_path, curr_prefix);
	strcat (relocated_path, relative_path);
//	printf ("relocated_path:       %s\n", relocated_path);
	relocated_short_path = getshortpath (relocated_path);
//	printf ("relocated_short_path: %s\n", relocated_short_path);
	if (relocated_short_path) {
		if (relocated_short_path != relocated_path)
			free (relocated_path);
		return relocated_short_path;
	} else
		return relocated_path;
}

#else // __WIN32__
char *relocaten (const char *ModuleName, const char *path)
{
	// dummy function for Unix/Linux
	return (char *)path;
}
#endif

char *relocaten2 (const char *ModuleName, const char *installdir, const char *path)
{
	set_orig_prefix (installdir);
	return relocaten (ModuleName, path);
}

char *relocatenx (const char *ModuleName, const char *installdir, const char *path)
{
	char *p;

	set_orig_prefix (installdir);
	if (access (path, R_OK))
		p = relocaten (ModuleName, path);
	else
		p = (char *) path;
//	printf ("relocatenx: %s\n", p);
	return p;
}

char *relocate2 (const char *installdir, const char *path)
{
	return relocaten2 (NULL, installdir, path);
}

char *relocatex (const char *installdir, const char *path)
{
	return relocatenx (NULL, installdir, path);
}

char *relocatepx (const char *cprefix, const char *installdir, const char *path)
{
	if (curr_prefix)
		free (curr_prefix);
	curr_prefix = strdup (cprefix);
	return relocatex (installdir, path);
}

static char *get_orig_prefix (void)
{
	return orig_prefix;
}

static char *get_curr_prefix (void)
{
	return curr_prefix;
}

static char *set_curr_prefix (const char *ModuleName)
{
	if (curr_prefix)
		free (curr_prefix);
	set_current_prefix (ModuleName);
	return curr_prefix;
}

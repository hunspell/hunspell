/* Return the canonical absolute name of a given file.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <malloc.h>
#ifdef __WIN32__
# include <stdio.h>
# include <windows.h>
//# include <gw32.h>
#endif /* __WIN32__ */
#include "canonicalize.h"

#ifndef MAXSYMLINKS
# define MAXSYMLINKS 20
#endif

#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

#  ifdef VMS
    /* We want the directory in Unix syntax, not in VMS syntax.  */
#   define __getcwd(buf, max) getcwd (buf, max, 0)
#  else
#   define __getcwd getcwd
#  endif

#define weak_alias(local, symbol)

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
  /* Win32, Cygwin, OS/2, DOS */
# define ISDIRSEP(C) ((C) == '/' || (C) == '\\')
#else
  /* Unix */
# define ISDIRSEP(C) ((C) == '/')
#endif

#ifdef __WIN32__
char *win2unixpath (char *FileName)
{
	char *s = FileName;
	while (*s) {
		if (*s == '\\')
			*s = '/';
		*s++;
	}
	return FileName;
}
#endif

/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/') or symlinks.  All path components must exist.  If
   RESOLVED is null, the result is malloc'd; otherwise, if the
   canonical name is PATH_MAX chars or more, returns null with `errno'
   set to ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars,
   returns the name in RESOLVED.  If the name cannot be resolved and
   RESOLVED is non-NULL, it contains the path of the first component
   that cannot be resolved.  If the path can be resolved, RESOLVED
   holds the same value as the value returned.
   RESOLVED must be at least PATH_MAX long */

static char *
canonicalize (const char *name, char *resolved)
{
  char *rpath, *dest, *extra_buf = NULL;
  const char *start, *end, *rpath_limit;
  long int path_max;
  int num_links = 0, old_errno;

  if (name == NULL)
    {
      /* As per Single Unix Specification V2 we must return an error if
	 either parameter is a null pointer.  We extend this to allow
	 the RESOLVED parameter to be NULL in case the we are expected to
	 allocate the room for the return value.  */
      __set_errno (EINVAL);
      return NULL;
    }

  if (name[0] == '\0')
    {
      /* As per Single Unix Specification V2 we must return an error if
	 the name argument points to an empty string.  */
      __set_errno (ENOENT);
      return NULL;
    }
#ifdef __WIN32__
	{
	char *lpFilePart;
	int len;
//  fprintf(stderr, "name: %s\n", name);
	rpath = resolved ? __builtin_alloca (MAX_PATH) : malloc (MAX_PATH);
//	unix2winpath (name);
//  fprintf(stderr, "name: %s\n", name);
	len = GetFullPathName(name, MAX_PATH, rpath, &lpFilePart);
	/* GetFullPathName returns bogus paths for *nix-style paths, like
	* /foo/bar - it just prepends current drive to them. Keep them
	* intact (they need to be for relocation to work!).
	*/
	if (name[0] == '/') {
		strncpy (rpath, name, MAX_PATH - 1);
		rpath[MAX_PATH - 1] = '\0';
		len = strlen (rpath);
	}
//  fprintf(stderr, "rpath: %s\n", rpath);
	if (len == 0) {
		//set_werrno;
		return NULL;
	}
	if (len > MAX_PATH)	{
		if (resolved)
			__set_errno(ENAMETOOLONG);
		else {
			rpath = realloc(rpath, len + 2);
			GetFullPathName(name, len, rpath, &lpFilePart);
//  fprintf(stderr, "rpath: %s\n", rpath);
		}
	}
//	if ( ISDIRSEP(name[strlen(name)]) && !ISDIRSEP(rpath[len]) ) {
//		rpath[len] = '\\';
//		rpath[len + 1] = 0;
//	}
	old_errno = errno;
	//if (!access (rpath, D_OK) && !ISDIRSEP(rpath[len - 1]) ){
	if (!access (rpath, R_OK) && !ISDIRSEP(rpath[len - 1]) ){
		rpath[len] = '\\';
		rpath[len + 1] = 0;
	}
	errno = old_errno;
	win2unixpath (rpath);
//  fprintf(stderr, "rpath: %s\n", rpath);
	return resolved ? strcpy(resolved, rpath) : rpath ;
	}
#else /* __WIN32__ */

#ifdef PATH_MAX
  path_max = PATH_MAX;
#else
  path_max = pathconf (name, _PC_PATH_MAX);
  if (path_max <= 0)
    path_max = 1024;
#endif

  rpath = resolved ? __builtin_alloca (path_max) : malloc (path_max);
  rpath_limit = rpath + path_max;

  if (name[0] != '/')
    {
      if (!__getcwd (rpath, path_max))
	{
	  rpath[0] = '\0';
	  goto error;
	}
      dest = strchr (rpath, '\0');
    }
  else
    {
      rpath[0] = '/';
      dest = rpath + 1;
    }

  for (start = end = name; *start; start = end)
    {
#ifdef _LIBC
      struct stat64 st;
#else
      struct stat st;
#endif
      int n;

      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
	++start;

      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
	/* Nothing.  */;

      if (end - start == 0)
	break;
      else if (end - start == 1 && start[0] == '.')
	/* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
	{
	  /* Back up to previous component, ignore if at root already.  */
	  if (dest > rpath + 1)
	    while ((--dest)[-1] != '/');
	}
      else
	{
	  size_t new_size;

	  if (dest[-1] != '/')
	    *dest++ = '/';

	  if (dest + (end - start) >= rpath_limit)
	    {
	      ptrdiff_t dest_offset = dest - rpath;

	      if (resolved)
		{
		  __set_errno (ENAMETOOLONG);
		  if (dest > rpath + 1)
		    dest--;
		  *dest = '\0';
		  goto error;
		}
	      new_size = rpath_limit - rpath;
	      if (end - start + 1 > path_max)
		new_size += end - start + 1;
	      else
		new_size += path_max;
	      rpath = realloc (rpath, new_size);
	      rpath_limit = rpath + new_size;
	      if (rpath == NULL)
		return NULL;

	      dest = rpath + dest_offset;
	    }

#ifdef _LIBC
	  dest = __mempcpy (dest, start, end - start);
#else
	  memcpy (dest, start, end - start);
	  dest += end - start;
#endif
	  *dest = '\0';

#ifdef _LIBC
	  if (__lxstat64 (_STAT_VER, rpath, &st) < 0)
#else
	  if (lstat (rpath, &st) < 0)
#endif
	    goto error;

#if HAVE_READLINK
	  if (S_ISLNK (st.st_mode))
	    {
	      char *buf = __builtin_alloca (path_max);
	      size_t len;

	      if (++num_links > MAXSYMLINKS)
		{
		  __set_errno (ELOOP);
		  goto error;
		}

	      n = __readlink (rpath, buf, path_max);
	      if (n < 0)
		goto error;
	      buf[n] = '\0';

	      if (!extra_buf)
		extra_buf = __builtin_alloca (path_max);

	      len = strlen (end);
	      if ((long int) (n + len) >= path_max)
		{
		  __set_errno (ENAMETOOLONG);
		  goto error;
		}

	      /* Careful here, end may be a pointer into extra_buf... */
	      memmove (&extra_buf[n], end, len + 1);
	      name = end = memcpy (extra_buf, buf, n);

	      if (buf[0] == '/')
		dest = rpath + 1;	/* It's an absolute symlink */
	      else
		/* Back up to previous component, ignore if at root already: */
		if (dest > rpath + 1)
		  while ((--dest)[-1] != '/');
	    }
#endif
	}
    }
  if (dest > rpath + 1 && dest[-1] == '/')
    --dest;
  *dest = '\0';

  return resolved ? memcpy (resolved, rpath, dest - rpath + 1) : rpath;

error:
  if (resolved)
    strcpy (resolved, rpath);
  else
    free (rpath);
  return NULL;

#endif /* __WIN32__ */
}


char *
__realpath (const char *name, char *resolved)
{
  if (resolved == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  return canonicalize (name, resolved);
}
weak_alias (__realpath, realpath)


char *
__canonicalize_file_name (const char *name)
{
  return canonicalize (name, NULL);
}
weak_alias (__canonicalize_file_name, canonicalize_file_name)

char *
canonicalize_file_name (const char *name)
{
  return canonicalize (name, NULL);
}

// Xconq
// String Functions

// $Id: string.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999        Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief String functions.
    \ingroup grp_util
*/

#include "util/base.h"
#include <cstdarg>

#include "util/constants.h"
#include "util/memory.h"
#include "util/string.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

char *
copy_string(char *str)
{
    int len = strlen(str);
    char *rslt;

    rslt = (char *)xmalloc(len + 1);
    strcpy(rslt, str);
#ifdef DEBUGGING
    copymalloc += len + 1;
#endif
    return rslt;
}

//! Pointer to buffer for blank padding.
/*! Allocated memory is never implicitly freed. */
static char *padbuf;

char *
pad_blanks(char *str, int n)
{
    char *pb;
    int i;

    if (padbuf == NULL)
      padbuf = (char *)xmalloc(BUFSIZE);
    pb = padbuf;
    while (*str && pb < padbuf + BUFSIZE - 2) {
	*pb++ = *str++;
	if (*str) {
	    for (i = 0; i < n; i++)
	      *pb++ = ' ';
	}
    }
    *pb = '\0';
    return padbuf;
}

int
iindex(int ch, char *str)
{
    int i;

    if (str == NULL)
      return (-1);
    for (i = 0; str[i] != '\0'; ++i)
      if (ch == str[i])
	return i;
    return (-1);
}

void
tprintf(char *buf, char *str, ...)
{
    va_list ap;
    char line[TBUFSIZE];

    va_start(ap, str);
    vsnprintf(line, TBUFSIZE, str, ap);
    strcat(buf, line);
    va_end(ap);
}


void
tnprintf(char *buf, int n, char *str, ...)
{
    va_list ap;
    int n1 = n - strlen(buf);
    char line[TBUFSIZE];

    if (n1 > 0) {
	va_start(ap, str);
	vsnprintf(line, TBUFSIZE, str, ap);
	strncat(buf, line, n1);
	va_end(ap);
    }
}

void
vtprintf(char *buf, char *str, va_list ap)
{
    char line[TBUFSIZE];

    vsnprintf(line, TBUFSIZE, str, ap);
    strcat(buf, line);
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END


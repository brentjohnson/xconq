// xConq
// String Functions

// $Id: string.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999	    Stanley T. Shebs
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
    \brief String functions and constants.
    \ingroup grp_util
*/

#ifndef XCONQ_STRING_H
#define XCONQ_STRING_H

#include "util/base.h"

#ifndef isspace
//! Is character whitespace?
/*! Defined if system doesn't provide this function/macro. */
#define isspace(c) \
        ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
#endif

//! Make a character lowercase, if it is uppercase.
#define lowercase(c) (isupper(c) ? tolower(c) : (c))
//! Make a character uppercase, if it is lowercase.
#define uppercase(c) (islower(c) ? toupper(c) : (c))

//! Is string empty?
#define empty_string(s) \
	((s) == NULL || s[0] == '\0')

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

//! Primary string printing buffer.
extern char spbuf[];

//! Clone a string into fresh memory and return pointer.
/*! The new space is never implicitly freed. */
extern char *copy_string(char *str);

//! Insert given number of blanks between each char of string.
extern char *pad_blanks(char *str, int n);

//! Get a *numeric* index into a string. 
/*! More useful than ptr, in Xconq. Return -1 on failed search. */
extern int iindex(int ch, char *str);

//! Formatted append a variable number of args to a buffer.
extern void tprintf(char *buf, char *str, ...);
//! Formatted, truncated append a variable number of args to a buffer.
extern void tnprintf(char *buf, int n, char *str, ...);
//! Formatted append a varargs list to a buffer.
extern void vtprintf(char *buf, char *str, va_list ap);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_STRING_H

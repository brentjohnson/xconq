// xConq
// String synthesis support.

// $Id: desc.cc,v 1.1 2006/05/22 23:43:54 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
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
    \brief String synthesis support.
    \ingroup grp_gdl
    \todo Get rid of this file in the GDL group.
          Functionality should be in kernel.
*/


#include "gdl/desc.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Variables: Buffers

//! Pluralization buffer.
static char *pluralbuf;

// Word Conversions

char *
ordinal_suffix(int n)
{
    if (n % 100 == 11 || n % 100 == 12 || n % 100 == 13) {
	return "th";
    } else {
	switch (n % 10) {
	  case 1:   return "st";
	  case 2:   return "nd";
	  case 3:   return "rd";
	  default:  return "th";
	}
    }
}

// Word Modifications

char *
plural_form(char *word)
{
    char endch = ' ', nextend = ' ';
    int len;

    if (word == NULL) {
	run_warning("plural_form given NULL string");
	pluralbuf[0] = '\0';
	return pluralbuf;
    }
    len = strlen(word);
    if (len > 0)
      endch = word[len - 1];
    if (len > 1)
      nextend = word[len - 2];
    if (endch == 'h' || endch == 's' || (endch == 'e' && nextend == 's')) {
	sprintf(pluralbuf, "%s", word);
    } else {
	sprintf(pluralbuf, "%ss", word);
    }
    return pluralbuf;
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

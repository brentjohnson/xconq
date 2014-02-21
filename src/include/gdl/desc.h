// xConq
// String synthesis support.

// $Id: desc.h,v 1.1 2006/05/22 23:43:55 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2003-2006   Eric A. McDonald

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

#ifndef XCONQ_GDL_DESC_H
#define XCONQ_GDL_DESC_H

#include "gdl/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

//! Given a number, figure out what suffix should go with it.
/*! \todo Provide localization. */
extern char *ordinal_suffix(int n);

//! Pluralize given word.
/*!
    Pluralize a word, attempting to be smart about various
    possibilities that don't have a different plural form (such as
    "Chinese" and "Swiss").

    \todo There should probably be a test for when to add "es" instead of "s".
    \todo Provide localization.
*/
extern char *plural_form(char *word);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_DESC_H

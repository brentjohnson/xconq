// xConq
// Parameter Box

// $Id: parambox.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 2004	    Eric A. McDonald
  Copyright (C) 2006	    Eric A. McDonald

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
    \brief Parameter boxes to aid search algorithms.
    \ingroup grp_util

    \todo Get rid of this, and replace with better object-oriented and 
	  templated solution.
*/

#ifndef XCONQ_PARAMBOX_H
#define XCONQ_PARAMBOX_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

/* Enumeration of parameter box types. */
enum ParamBoxType {
    PBOX_TYPE_NONE = 0,
    PBOX_TYPE_UNIT,			/* unit.h */
    PBOX_TYPE_UNIT_AT,			/* unit.h */
    PBOX_TYPE_UNIT_UNIT,		/* unit.h */
    PBOX_TYPE_UNIT_SIDE,		/* unit.h */
    PBOX_TYPE_UNIT_SIDE_SEERS,		/* unit.h */
    PBOX_TYPE_UNIT_UNIT_SEERS,		/* unit.h */
    PBOX_TYPE_SIDE,			/* side.h */
    PBOX_TYPE_COUNT			/* (Always last. Not a type.) */
};

/* Base struct for parameter boxen. */
/* (The parameter box is an alternative to using a function with
    an ellipsis and varargs. Having these "abbreviations" available can
    be more useful than parsing a varargs list. These are particularly
    helpful when used with predicate functions in search algorithms.) */
struct ParamBox {
  protected:
    ParamBoxType pboxtype;
    ParamBox() { pboxtype = PBOX_TYPE_NONE; }
  public:
    ParamBoxType get_type() { return pboxtype; }
};

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifdef XCONQ_PARAMBOX_H

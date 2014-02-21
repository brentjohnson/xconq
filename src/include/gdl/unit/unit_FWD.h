// xConq
// Foward declarations and typedefs related to units.

// $Id: unit_FWD.h,v 1.1 2006/05/29 22:43:09 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

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
    \brief Foward declarations and typedefs related to units.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_UNIT_FWD_H
#define XCONQ_GDL_UNIT_UNIT_FWD_H

#include "gdl/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Forward declarations of unit structs.

struct a_unit;
struct a_unit_extras;
struct a_unit_view;

// Typedefs of unit structs.

typedef a_unit      Unit;

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_UNIT_FWD_H

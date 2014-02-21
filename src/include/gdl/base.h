// xConq
// Foundation for GDL

// $Id: base.h,v 1.1 2006/04/07 02:39:10 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 2005-2006    Eric A. McDonald

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

//! \defgroup grp_gdl GDL

/*! \file
    \brief Lays foundation for entities in GDL namespace.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_BASE_H
#define XCONQ_GDL_BASE_H

#include "basetypes/basetypes.h"
#include "util/util.h"

#ifdef NAMESPACE_GDL_BEGIN
#undef NAMESPACE_GDL_BEGIN
#endif
#ifdef NAMESPACE_GDL_END
#undef NAMESPACE_GDL_END
#endif
//! Begin namespace.
#define NAMESPACE_GDL_BEGIN	namespace GDL {
//! End namespace.
#define NAMESPACE_GDL_END	}

#ifdef USING_NAMESPACE_GDL
#undef USING_NAMESPACE_GDL
#endif
//! Use namespace.
#define USING_NAMESPACE_GDL    using namespace xconq::GDL;

NAMESPACE_XCONQ_BEGIN

/*!
    \brief GDL classes and functions.
    \ingroup grp_gdl

    The GDL (Game Design Language) namepsace contains object classes and 
    functions that provide serialization/deserialization of GDL objects
    to/from game definition/save files and manipulation of GDL objects.
*/

NAMESPACE_GDL_BEGIN
NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

USING_NAMESPACE_BASETYPE
USING_NAMESPACE_UTILITY

#endif // ifndef XCONQ_GDL_BASE_H

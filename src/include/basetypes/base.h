// xConq
// Foundation for Base Types

// $Id: base.h,v 1.1 2006/04/07 01:03:23 eric_mcdonald Exp $

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

//! \defgroup grp_types Types

/*! \file
    \brief Lays foundation for entities in BaseType namespace.
    \ingroup grp_types
*/

#ifndef XCONQ_BASETYPE_BASE_H
#define XCONQ_BASETYPE_BASE_H

#include "projbase.h"

#ifdef NAMESPACE_BASETYPE_BEGIN
#undef NAMESPACE_BASETYPE_BEGIN
#endif
#ifdef NAMESPACE_BASETYPE_END
#undef NAMESPACE_BASETYPE_END
#endif
//! Begin namespace.
#define NAMESPACE_BASETYPE_BEGIN    namespace BaseType {
//! End namespace.
#define NAMESPACE_BASETYPE_END	    }

#ifdef USING_NAMESPACE_BASETYPE
#undef USING_NAMESPACE_BASETYPE
#endif
//! Use namespace.
#define USING_NAMESPACE_BASETYPE    using namespace xconq::BaseType;

NAMESPACE_XCONQ_BEGIN

/*!
    \brief Base types and typename aliases.
    \ingroup grp_types

    The BaseType namespace contains typedefs of all useful primitives, 
    plus aliasing macros for various STL and Boost data types.
*/

NAMESPACE_BASETYPE_BEGIN
NAMESPACE_BASETYPE_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_BASETYPE_BASE_H

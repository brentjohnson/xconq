// xConq
// Typedefs and Alias Macros for Common STL and Boost Types

// $Id: aliases.h,v 1.1 2006/04/07 01:03:23 eric_mcdonald Exp $

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

/*! \file
    \brief Collection of typedefs and alias macros for common STL and Boost types.
    \ingroup grp_types

    \todo Add typedefs for other common vectors.
    \todo Add typedefs for common complex types.
    \todo Add alias macros for common STL and Boost types.
*/

#ifndef XCONQ_BASETYPE_ALIASES_H
#define XCONQ_BASETYPE_ALIASES_H

#include "basetypes/primitives.h"
#include <complex>
#include <vector>
#include <list>

NAMESPACE_XCONQ_BEGIN
NAMESPACE_BASETYPE_BEGIN

/*! \brief Vector of Booleans

    Storage: STL Vector\n
    Representation: Vector of Booleans
*/
typedef std::vector < Bool >				Bool_v;

/*! \brief Pointer to Vector of Booleans

    Storage: STL Vector\n
    Representation: Pointer to Vector of Booleans
*/
typedef std::vector < Bool > *				Bool_vp;

/*! \brief Pointer to Const Vector of Const Booleans

    Storage: STL Vector\n
    Representation: Pointer to Const Vector of Const Booleans
*/
typedef const std::vector < const Bool > *		Bool_cvcp;

/*! \brief Reference to Vector of Booleans

    Storage: STL Vector\n
    Representation: Reference to Vector of Booleans
*/
typedef std::vector < Bool > &				Bool_vr;

/*! \brief Reference to Const Vector of Const Booleans

    Storage: STL Vector\n
    Representation: Reference to Const Vector of Const Booleans
*/
typedef const std::vector < const Bool > &		Bool_cvcr;

NAMESPACE_BASETYPE_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_BASETYPE_ALIASES_H

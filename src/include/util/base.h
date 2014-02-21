// xConq
// Foundation for Utilities

// $Id: base.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

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

//! \defgroup grp_util Utilities

/*! \file
    \brief Lays foundation for entities in Utility namespace.
    \ingroup grp_util
*/

#ifndef XCONQ_UTILITY_BASE_H
#define XCONQ_UTILITY_BASE_H

#include "basetypes/basetypes.h"

#ifdef NAMESPACE_UTILITY_BEGIN
#undef NAMESPACE_UTILITY_BEGIN
#endif
#ifdef NAMESPACE_UTILITY_END
#undef NAMESPACE_UTILITY_END
#endif
//! Begin namespace.
#define NAMESPACE_UTILITY_BEGIN    namespace Utility {
//! End namespace.
#define NAMESPACE_UTILITY_END	   }

#ifdef USING_NAMESPACE_UTILITY
#undef USING_NAMESPACE_UTILITY
#endif
//! Use namespace.
#define USING_NAMESPACE_UTILITY    using namespace xconq::Utility;

NAMESPACE_XCONQ_BEGIN

/*!
    \brief Utility functions.
    \ingroup grp_util

    The Utility namespace contains utility functions that provide 
    abstraction from platform-specific details, as well as replacement 
    functions for some system or C library facilities such as 
    pseudo-random number generation.
*/

NAMESPACE_UTILITY_BEGIN
NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

USING_NAMESPACE_BASETYPE

#endif // ifndef XCONQ_UTILITY_BASE_H

// xConq
// Foundation for Media Handling

// $Id: base.h,v 1.1 2006/05/14 16:11:38 eric_mcdonald Exp $

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

//! \defgroup grp_media Media

/*! \file
    \brief Lays foundation for entities in Media namespace.
    \ingroup grp_media
*/

#ifndef XCONQ_MEDIA_BASE_H
#define XCONQ_MEDIA_BASE_H

#include "basetypes/basetypes.h"
#include "util/util.h"
#include "gdl/lisp.h"

#ifdef NAMESPACE_MEDIA_BEGIN
#undef NAMESPACE_MEDIA_BEGIN
#endif
#ifdef NAMESPACE_MEDIA_END
#undef NAMESPACE_MEDIA_END
#endif
//! Begin namespace.
#define NAMESPACE_MEDIA_BEGIN	    namespace Media {
//! End namespace.
#define NAMESPACE_MEDIA_END	    }

#ifdef USING_NAMESPACE_MEDIA
#undef USING_NAMESPACE_MEDIA
#endif
//! Use namespace.
#define USING_NAMESPACE_MEDIA	    using namespace xconq::Media;

NAMESPACE_XCONQ_BEGIN

/*!
    \brief Media-handling functions.
    \ingroup grp_media

    The Media namespace contains object classes and functions related 
    to the handling of various media such as graphics and sounds. 
    The focus is on loading/saving, manipulating, and converting their 
    data, but not displaying. Thus, entities in this namespace are 
    safe for, and intended for, use in the game kernel.
*/

NAMESPACE_MEDIA_BEGIN
NAMESPACE_MEDIA_END
NAMESPACE_XCONQ_END

USING_NAMESPACE_BASETYPE
USING_NAMESPACE_UTILITY
USING_NAMESPACE_GDL

#endif // ifndef XCONQ_MEDIA_BASE_H

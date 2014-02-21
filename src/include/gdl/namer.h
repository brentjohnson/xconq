// xConq
// Lifecycle management and ser/deser of namers.

// $Id: namer.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of namers.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_NAMER_H
#define XCONQ_GDL_NAMER_H

#include "gdl/lisp.h"
#include "gdl/side/side_FWD.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Operations

//! Given list of utype-string pairs for side, set unit namers appropriately.
extern void merge_unit_namers(Side *side, Obj *lis);

// Lifecycle Management

//! Make namer from dictionary and namer method.
extern Obj *make_namer(Obj *sym, Obj *meth);

// GDL I/O

//! Read namer from GDL form.
extern void interp_namer(Obj *form);

//! Serialize namer to GDL.
extern void write_namers(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_NAMER_H

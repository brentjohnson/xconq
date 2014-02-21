// xConq
// Lifecycle management and ser/deser of past units.

// $Id: pastunit.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of past units.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_PASTUNIT_H
#define XCONQ_GDL_UNIT_PASTUNIT_H

#include "gdl/lisp.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Unit that is no longer in playing area.
typedef struct a_pastunit {
    //! Unit type.
    short type;
    //! Unique ID number.
    int id;
    //! Name, if given.
    char *name;
    //! Ordinal of type of unit on side.
    int number;
    //! X-position of departure from playing area.
    short x;
    //! Y-position of departure from playing area.
    short y;
    //! Z-position of departure from playing area.
    short z;
    //! Side on which unit departed from playing area.
    struct a_side *side;
    //! Next past unit.
    struct a_pastunit *next;
} PastUnit;

// Global Variables

//! List of all past units.
extern PastUnit *past_unit_list;

// Queries

//! Get past unit from ID.
extern PastUnit *find_past_unit(int n);

//! Return a handle for past unit on given side.
/*!
    Build a short phrase describing a given past unit to a given side,
    basically consisting of indication of unit's side, then of unit
    itself.
*/
extern char *past_unit_handle(Side *side, PastUnit *past_unit);

// Lifecycle Management

//! Create past unit of given type and ID.
extern PastUnit *create_past_unit(int type, int id);

// GDL I/O

//! Read past unit (one that is not in playing area) from GDL form.
extern void interp_past_unit(Obj *form);

//! Serialize past unit to GDL.
extern void write_past_unit(PastUnit *pastunit);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_PASTUNIT_H

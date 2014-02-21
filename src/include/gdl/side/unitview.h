// xConq
// Lifecycle management and ser/deser of per-side unit views.

// $Id: unitview.h,v 1.1 2006/05/30 02:34:14 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of per-side unit views.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_SIDE_UNITVIEW_H
#define XCONQ_GDL_SIDE_UNITVIEW_H

#include "gdl/lisp.h"
#include "gdl/unit/unitview.h"
#include "gdl/side/side_FWD.h"

// Macro Functions: Type Conversion

//! Get actual unit being viewed, if possible.
#define view_unit(uv) \
  (((uv)->unit != NULL && (uv)->id == (uv)->unit->id) ? (uv)->unit : NULL)

// Iterator Macros

//! Next neighboring unit view for given unit view on given side.
#define nextview(s,x,y,uv) \
    ((uv)->transport == NULL ? \
        unit_view_next((s), (x), (y), (uv)) : \
        (uv)->nexthere)

//! Iterate over all unit views at given cell.
#define for_all_view_stack(s,x,y,uv) \
  for ((uv) = unit_view_at(s,x,y); (uv) != NULL; \
        (uv) = unit_view_next(s,x,y,uv))

//! Iterate through all unit views in cell and their occupants.
/*!
    This nifty little macro will climb the occupant : nexthere tree and
    follow all branches four levels down to find all the occs within
    occs within occs within units in the stack.
*/
#define for_all_view_stack_with_occs(s, x,y,uv)  \
  for ((uv) = unit_view_at((s), (x), (y)); \
       (uv) != NULL; \
       (uv) = ((uv)->occupant != NULL ? \
        (uv)->occupant : \
	(nextview((s), (x), (y), (uv)) != NULL ? \
	   nextview((s), (x), (y), (uv)) : \
	   ((uv)->transport != NULL && \
	    nextview((s), (x), (y), (uv)->transport) != NULL ? \
	       nextview((s), (x), (y), (uv)->transport) : \
	           ((uv)->transport != NULL && \
		    (uv)->transport->transport != NULL && \
		    nextview((s), (x), (y), (uv)->transport->transport) != NULL ? \
		      nextview((s), (x), (y), (uv)->transport->transport) : \
  		      ((uv)->transport != NULL && \
		       (uv)->transport->transport != NULL && \
		       (uv)->transport->transport->transport != NULL && \
		       nextview((s), (x), (y), (uv)->transport->transport->transport) != NULL ? \
		          nextview((s), (x), (y), (uv)->transport->transport->transport) : NULL))))))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Queries

//! Get unit view that given side sees at given cell?
extern struct a_unit_view *unit_view_at(Side *side, int x, int y);
//! Get next unit view in list of unit views at give cell for given cell.
extern struct a_unit_view *unit_view_next(
    Side *side, int x, int y, struct a_unit_view *uview);
//! Look for an existing unit view that is tied to the given unit.
extern UnitView *find_unit_view(Side *side, Unit *unit);

// Images

//! Bind unit view to image for its unit type.
extern void set_unit_view_image(UnitView *uview);

// Lifecycle Management

//! Add raw view of unit to side's knowledge.
extern void add_unit_view_raw(Side *side, UnitView *uview, int x, int y);
//! Add view of unit to side's knowledge.
extern UnitView *add_unit_view(Side *side, Unit *unit);

//! Remove unit view from side.
extern int remove_unit_view(Side *side, UnitView *olduview);

// GDL I/O

//! Read unit views for given side from GDL list.
/*!
    Given a list of views of units, make unit view objects and plug
    them into the side's list of unit views.
*/
extern void interp_unit_views(Side *side, Obj *vlist);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SIDE_UNITVIEW_H

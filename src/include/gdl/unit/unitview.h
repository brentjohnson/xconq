// xConq
// Lifecycle management and serialization/deserialization of unit views.

// $Id: unitview.h,v 1.1 2006/05/29 22:43:09 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Lifecycle management and serialization/deserialization of unit views.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_UNITVIEW_H
#define XCONQ_GDL_UNIT_UNITVIEW_H

#include "gdl/unit/unit_FWD.h"

// Iterator Macros

//! Iterate over all unit views.
#define for_all_unit_views(v)  \
    for (v = viewlist; v != NULL; v = v->vnext)

//! Iterate over views of occupants.
#define for_all_occupant_views(uv,v) \
  for (v = (uv)->occupant; v != NULL; v = v->nexthere)

//!  Iterate over unit views and their occupants.
/*!
    Iterate through all occupants including occupents within occupents within
    occupents within a unit view.
    This nifty little macro will climb the occupant : nexthere tree and
    follow all branches three levels below (uv) to find all the occs
    within occs within occs within (uv).  The test for (var) !=
    (uv)->nexthere is to stop the macro from climbing up above (uv).
*/
#define for_all_occupant_views_with_occs(uv,var)  \
  for ((var) = (uv)->occupant; \
       (var) != NULL && (var) != (uv)->nexthere; \
  	 (var) = ((var)->occupant != NULL ? \
  	 	        (var)->occupant : \
  	 	      ((var)->nexthere != NULL ? \
  	 	        (var)->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->nexthere != NULL ? \
  		        (var)->transport->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->transport != NULL && \
  		        (var)->transport->transport->nexthere != NULL ? \
  		        (var)->transport->transport->nexthere : NULL)))))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Unit view.
/*!
    The unit view is a partial mirror of an actual unit, including only
    the data that might be known to another side seeing the unit from a
    distance.
*/
typedef struct a_unit_view {
    //! Number of observing side.
    short observer;
    //! Unit type.
    short type;
    //! Number of owning side.
    short siden;
    //! Name of unit.
    char *name;
    //! GDL image family.
    struct a_image_family *imf;
    //! Name of GDL image, if given.
    char *image_name;
    //! Advanced unit: abstract size.
    short size;
    //! X-position of view.
    short x;
    //! Y-position of view.
    short y;
    //! True, if unit appears to be completed.
    short complete;
    //! Turn on which view was last updated.
    short date;
    //! ID of actual unit, if known.
    int id;
    //! Actual unit, if ID valid.
    struct a_unit *unit;
    //! View of container unit, if any.
    struct a_unit_view *transport;
    //! Views of first occupant, if any.
    struct a_unit_view *occupant;
    //! View of next neighbor, if any.
    struct a_unit_view *nexthere;
    //! Next view in hash bucket.
    struct a_unit_view *nextinhash;
    //! Next view in global list.
    struct a_unit_view *vnext;
} UnitView;

// Global Variables: Unit View Pools

//! Global linked list of all unit views.
extern UnitView *viewlist;
//! Number of views in unit views list.
extern int numunitviews;

// Queries

//! Determine if unit view is in correct position amongst other unit views.
/*!
    Find out if an uview is in the correct position in a stack (cell or
    transport), relative to another uview.
*/
extern int is_at_correct_uvstack_position(UnitView *uview, UnitView *uview2);

// Lifecycle Management

//! Create basic unit view.
extern UnitView *create_bare_unit_view(void);

//! Initialize and fill out an uview from unit data.
extern void fill_out_uview(Unit *unit, Side *side, UnitView *uview);

//! Update unit's master (side-independent) unit view.
extern void update_master_uview(Unit *unit, int lookabove);

//! Flush a particular unit view.
/*!
    Keep it clean - hit all links to other places.  Some might not be
    strictly necessary, but this is not an area to take chances with.
*/
extern void flush_one_view(UnitView *uview);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_UNITVIEW_H

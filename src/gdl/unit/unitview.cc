// xConq
// Lifecycle management and serialization/deserialization of unit views.

// $Id: unitview.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2005-2006   Eric A. McDonald

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

#include "gdl/types.h"
#include "gdl/unit/unit.h"
#include "gdl/side/side.h"

// Local Constant Macros

#ifndef INITMAXVIEWS
//! Number of unit views to allocate in a block.
#define INITMAXVIEWS 200
#endif

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

UnitView *viewlist = NULL;
int numunitviews = 0;

// Local Variables

//! List of available unit views.
static UnitView *freeviews;

// Local Function Declarations: Lifecycle Management

//! Allocate block of unit view objects to work with.
static void allocate_unit_view_block(void);

// Queries

int
is_at_correct_uvstack_position(UnitView *uview, UnitView *uview2)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int uso = -1, u2so = -1;

    assert_error(uview, "Attempted to place a NULL uview in a uvstack");
    assert_error(uview2, "Attempted to compare an uview in a NULL uvstack");
    u = uview->type;
    u2 = uview2->type;
    uso = u_stack_order(u);
    u2so = u_stack_order(u2);
    /* Enforce side ordering 1st. */
    /*! \todo Add support for holding units belonging to the dside first in
	      the stack, last in the stack, or in the natural side order.
	      Currently, only the natural side order is supported. */
    if (uview->siden > uview2->siden)
      return FALSE;
    if (uview->siden < uview2->siden)
      return TRUE;
    /* Enforce stack ordering 2nd. */
    if (uso < u2so)
      return FALSE;
    if (uso > u2so)
      return TRUE;
    /* Enforce utype ordering 3rd. */
    if (u > u2)
      return FALSE;
    if (u < u2)
      return TRUE;
    /* Enforce uview ID ordering 4th. */
    if (uview->id < uview2->id)
      return TRUE;
    return FALSE;
}

// Lifecycle Management

static
void
allocate_unit_view_block(void)
{
    int i;
    UnitView *viewblock = (UnitView *) xmalloc(INITMAXVIEWS * sizeof(UnitView));

    for (i = 0; i < INITMAXVIEWS; ++i) {
        viewblock[i].id = -1;
        viewblock[i].nexthere = &viewblock[i+1];
    }
    viewblock[INITMAXVIEWS-1].nexthere = NULL;
    freeviews = viewblock;
    Dprintf("Allocated space for %d unit views.\n", INITMAXVIEWS);
}

UnitView *
create_bare_unit_view(void)
{
    UnitView *newview;

    /* If our free list is empty, go and get some more units. */
    if (freeviews == NULL) {
	allocate_unit_view_block();
    }
    /* Take the first unit off the free list. */
    newview = freeviews;
    freeviews = freeviews->nexthere;
    /* ...but an invalid id. */
    newview->id = -1;
    return newview;
}

void
fill_out_uview(Unit *unit, Side *side, UnitView *uview)
{
    /* Sanity checks. */
    assert_error(unit, "Attempted to fill out an unit view with a NULL unit");
    assert_error(uview, "Attempted to fill out a NULL unit view");
    /* Fill out uview. */
    uview->observer = (side ? side->id : -1);
    uview->unit = unit;
    uview->x = unit->x;
    uview->y = unit->y;
    uview->siden = unit->side->id;
    uview->type = unit->type;
    uview->size = unit->size;
    uview->id = unit->id;
    uview->imf = unit->imf;
    uview->image_name = unit->image_name;
    uview->complete = completed(unit);
    uview->occupant = NULL;
    uview->transport = NULL;
    uview->nextinhash = NULL;
    uview->nexthere = NULL;
    uview->vnext = NULL;
}

void
update_master_uview(Unit *unit, int lookabove)
{
    UnitView *uview = NULL;

    /* Return immediately if no unit to update. */
    if (!unit)
      return;
    /* If we are to look above and the unit has a transport,
       then update the transport's view first. Once the top of the transport
       chain is reached, then occs (including this unit) will be filled out,
       and so there is no need to duplicate work. Thus, return afterword. */
    if (lookabove && unit->transport) {
	update_master_uview(unit->transport, TRUE);
	return;
    }
    /* Allocate the uview, if it does already exist. */
    uview = unit->uview;
    if (!uview) {
	uview = (UnitView *)xmalloc(sizeof(UnitView));
	unit->uview = uview;
    }
    /* Fill out the uview from unit. */
    fill_out_uview(unit, NULL, uview);
    /* Link with transport, if any. */
    uview->transport = (unit->transport ? unit->transport->uview : NULL);
    /* Recurse into and link with first occ, if any. */
    /* This will update all of the occ's occs and next neighbors. */
    if (unit->occupant)
      update_master_uview(unit->occupant, FALSE);
    uview->occupant = (unit->occupant ? unit->occupant->uview : NULL);
    /* Recurse into and link with next neighbor, if any. */
    /* This will update all of the neighbor's occs and next neighbors. */
    /* The 'nextinhash' field is also updated with the value of the next
       neighbor so that 'unit_view_next' will not be broken. */
    if (unit->nexthere)
      update_master_uview(unit->nexthere, FALSE);
    uview->nexthere = (unit->nexthere ? unit->nexthere->uview : NULL);
    uview->nextinhash = uview->nexthere;
}

void
flush_one_view(UnitView *uview)
{
    uview->occupant = NULL;
    uview->transport = NULL;
    uview->nextinhash = NULL;
    uview->vnext = NULL;
    /* Add it on the front of the list of available views. */
    uview->nexthere = freeviews;
    freeviews = uview;
    --numunitviews;
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

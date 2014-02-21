// xConq
// Lifecycle management and ser/deser of per-side unit views.

// $Id: unitview.cc,v 1.1 2006/05/30 02:34:13 eric_mcdonald Exp $

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

#include "gdl/ui.h"
#include "gdl/types.h"
#include "gdl/gvars.h"
#include "gdl/side/side.h"
#include "gdl/gamearea/area.h"

// Local Constant Macros

//! Size of hash table for unit views.
#define VIEW_HASH_SIZE 257

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Function Declarations: Lifecycle Management

//! Add unit view to the given transport's list of unit views.
static void add_uview_to_uvstack(UnitView *uview, UnitView *tsptview);

// Queries

UnitView *
unit_view_at(Side *side, int x, int y)
{
    int hash;
    UnitView *uv;

    /* This might be called during synthesis, before sides are set up. */
    if (side->unit_views == NULL)
      return NULL;
    hash = (wrapx(x) ^ y) % VIEW_HASH_SIZE;
    for (uv = side->unit_views[hash]; uv != NULL; uv = uv->nextinhash) {
	if (wrapx(x) == uv->x
	    && y == uv->y
	    && uv->transport == NULL) {
		return uv;
    	}
    }
    return NULL;
}

UnitView *
unit_view_next(Side *side, int x, int y, UnitView *uview)
{
    UnitView *uv2;

    /* If we are looking among the side views. */
    if (side) {
	for (uv2 = uview->nextinhash; uv2 != NULL; uv2 = uv2->nextinhash) {
	    if (wrapx(x) == uv2->x && y == uv2->y && uv2->transport == NULL)
	      return uv2;
	}
    }
    /* If we are looking among master views. */
    else
      return uview->nexthere;
    return NULL;
}

UnitView *
find_unit_view(Side *side, Unit *unit)
{
    UnitView *uv, *ov;
    int i;

    if (!side) {
	update_master_uview(unit, FALSE);
	return unit->uview;
    }
    if (side->unit_views == NULL) {
	return NULL;
    }
    for (i = 0; i < VIEW_HASH_SIZE; ++i) {
	for (uv = side->unit_views[i]; uv != NULL; uv = uv->nextinhash) {
	    if (uv->id == unit->id) {
		return uv;
	    }
	    /* Also check all the occupant views. */
	    for_all_occupant_views_with_occs(uv, ov) {
		if (ov->id == unit->id) {
		    return ov;
		}
	    }
	}
    }
    return NULL;
}

// Images

void
set_unit_view_image(UnitView *uview)
{
    int u = uview->type, choice, i = 0;
    ImageFamily *imf;

    /* Some interfaces don't have images. */
    if (!uimages) {
    	return;
    }
    /* Use the preassigned image if available. */
    if (!empty_string(uview->image_name)) {
    	imf = find_imf(uview->image_name);
    	if (imf) {
	    uview->imf = imf;
	    uview->image_name = imf->name;
	    return;
    	}
    }
    /* Else pick a random image from the list for this unit type. */
    choice = xrandom(numuimages[u]);
    for (imf = uimages[u]; imf != NULL; imf = imf->next) {
	if (i == choice) {
	    uview->imf = imf;
	    uview->image_name = imf->name;
	    break;
    	}
	++i;
    }
}

// Lifecycle Management

void
add_unit_view_raw(Side *side, UnitView *uview, int x, int y)
{
    int hash;
    UnitView *uv2, *prevview;

    if (side->unit_views == NULL)
      side->unit_views =
	(UnitView **) xmalloc(VIEW_HASH_SIZE * sizeof(UnitView *));
    prevview = NULL;
    hash = (wrapx(x) ^ y) % VIEW_HASH_SIZE;
    for (uv2 = side->unit_views[hash]; uv2 != NULL; uv2 = uv2->nextinhash) {
	if (is_at_correct_uvstack_position(uview, uv2)) {
	    break;
	}
	prevview = uv2;
    }
    uview->nextinhash = uv2;
    if (prevview == NULL) {
	side->unit_views[hash] = uview;
    } else {
	prevview->nextinhash = uview;
    }
    uview->x = x;
    uview->y = y;
}

static
void
add_uview_to_uvstack(UnitView *uview, UnitView *tsptview)
{
    UnitView *topview = NULL, *occview = NULL, *nextview = NULL,
	     *prevview = NULL;

    assert_error(uview, "Attempted to add a NULL uview to a uvstack");
    assert_error(tsptview,
		 "Attempted to add an uview to a NULL transport uvstack");
    topview = tsptview->occupant;
    if (topview) {
	/* Insert uview into the occupant list at its correct position. */
	for_all_occupant_views(tsptview, occview) {
	    if (is_at_correct_uvstack_position(uview, occview)) {
		nextview = occview;
#if (0)
		if (uview == nextview->nexthere)
		  nextview->nexthere = NULL;
#endif
		if (occview == topview)
		  topview = uview;
		break;
	    }
	    prevview = occview;
	}
	if (prevview != NULL) {
	    prevview->nexthere = uview;
	}
    } else {
	topview = uview;
    }
    uview->nexthere = nextview;
    tsptview->occupant = topview;
}

UnitView *
add_unit_view(Side *side, Unit *unit)
{
    int changed = FALSE, rehash = FALSE, x, y;
    UnitView *tranview = NULL, *uview = NULL, *occview = NULL;
    Unit *occ;

    /* We need to create any transport view first to avoid an infinite
       loop between transport and occupant. */
    if (unit->transport) {
	tranview = find_unit_view(side, unit->transport);
	if (tranview == NULL) {
	    /* Recursive. Will climb up transport tree if necessary. */
	    tranview = add_unit_view(side, unit->transport);
	    /* Since occupant views also get generated,
	       we can return after this. */
	    return find_unit_view(side, unit);
    	}
    }
    /* Now we check if our unit has a view. */
    uview = find_unit_view(side, unit);
    if (uview) {
    	x = uview->x;
    	y = uview->y;
	/* Blast the view if it is in the wrong location,
	   or if it has the wrong transport. */
	if (x != unit->x
	    || y != unit->y
	    || (uview->transport && !unit->transport)
	    || (!uview->transport && unit->transport)
	    || (uview->transport && unit->transport
	    	&& uview->transport->id != unit->transport->id)) {
		remove_unit_view(side, uview);
		uview = NULL;
		/* Update the old cell display. */
		update_cell_display(side, x, y, UPDATE_ALWAYS);
	}
    }
    /* We need a new unit view. */
    if (uview == NULL) {
    	++numunitviews;
	uview = create_bare_unit_view();
	fill_out_uview(unit, side, uview);
#if (0)
	uview->observer = side->id;
	uview->unit = unit;
	uview->type = unit->type;
	uview->size = unit->size;
	uview->id = unit->id;
	uview->imf = unit->imf;
	uview->image_name = unit->image_name;
	uview->complete = completed(unit);
	uview->occupant = NULL;
	uview->transport = tranview;
	uview->nextinhash = NULL;
	uview->nexthere = NULL;
#endif
	uview->transport = tranview;
	/* Insert it into the global list. */
	uview->vnext = viewlist;
	viewlist = uview;
	/* Splice uview into the hash table, but only if it is the image of a
	   top unit. */
	if (unit->transport == NULL) {
	    add_unit_view_raw(side, uview, unit->x, unit->y);
	/* Else splice uview into the occupant list of its transport view. */
	} else {
	    add_uview_to_uvstack(uview, tranview);
	}
	changed = TRUE;
    }
    if (uview->type != unit->type) {
	uview->type = unit->type;
	changed = TRUE;
    }
    if (uview->siden != unit->side->id) {
	uview->siden = unit->side->id;
	changed = TRUE;
    }
    if (uview->name != unit->name) {
	uview->name = unit->name;
	changed = TRUE;
    }
    if (uview->imf != unit->imf) {
	uview->imf = unit->imf;
	changed = TRUE;
    }
    if (uview->image_name != unit->image_name) {
	uview->image_name = unit->image_name;
	changed = TRUE;
    }
    if (uview->size != unit->size) {
	uview->size = unit->size;
	changed = TRUE;
    }
    if (uview->complete != completed(unit)) {
	uview->complete = completed(unit);
	changed = TRUE;
    }
    /* Clean out any stale occupant views. */
    for_all_occupant_views(uview, occview) {
	remove_unit_view(side, occview);
    	changed = TRUE;
    }
    /* Add any occupant views that should be added. */
    if (unit->occupant && occupants_visible(side, unit)) {
	uview->occupant = NULL;
	for_all_occupants(unit, occ)
	  add_unit_view(side, occ);
	if (uview->occupant)
	  changed = TRUE;
    }
    /* Irrespective of whether any view content changed, we now know
       that the view of this unit is current, so date it. */
    uview->date = g_turn();
    return (changed ? uview : NULL);
}

int
remove_unit_view(Side *side, UnitView *olduview)
{
    int hash;
    UnitView *uv = NULL, *prevview = NULL;

    if (side->unit_views == NULL)
      return FALSE;
    prevview = uv = NULL;
    /* Splice the unit out of any occupant list. */
    if (olduview->transport) {
    	for_all_occupant_views(olduview->transport, uv) {
	    if (olduview == uv) {
		if (prevview == NULL) {
		    olduview->transport->occupant = uv->nexthere;
		} else {
		    prevview->nexthere = uv->nexthere;
		}
		break;
	    }
	    prevview = uv;
     	}
    /* Else splice out the view from the hash table. */
    } else {
	hash = (wrapx(olduview->x) ^ olduview->y) % VIEW_HASH_SIZE;
	for (uv = side->unit_views[hash]; uv != NULL; uv = uv->nextinhash) {
	    if (uv && uv == olduview) {
		if (prevview == NULL) {
		    side->unit_views[hash] = uv->nextinhash;
		} else {
		    prevview->nextinhash = uv->nextinhash;
		}
		break;
	    }
	    prevview = uv;
	}
    }
    /* Then clean out the occupant views. */
    for_all_occupant_views(olduview, uv) {
    	remove_unit_view(side, uv);
    }
    /* Finally mark the view as garbage. But don't hit its links yet,
       since we might be traversing an occupant list here. */
    olduview->id = -1;
    return TRUE;
}

// GDL I/O

void
interp_unit_views(Side *side, Obj *vlist)
{
    int u;
    Obj *rest, *viewform, *props;
    UnitView *uview;

    for_all_list(vlist, rest) {
	viewform = car(rest);
	u = NONUTYPE;
	if (symbolp(car(viewform))) {
	    u = utype_from_symbol(car(viewform));
	}
	if (u == NONUTYPE) {
	    read_warning("bad uview");
	    return;
	}
	uview = (UnitView *) xmalloc(sizeof(UnitView));
	/* Insert it into the global list. */
	uview->vnext = viewlist;
	viewlist = uview;
	uview->type = u;
	props = cdr(viewform);
	if (!consp(car(props))) {
	    uview->siden = c_number(eval(car(props)));
	    props = cdr(props);
	}
	if (symbolp(car(props))) {
             uview->name = c_string(car(props));
         	    props = cdr(props);
         }
	if (numberp(car(props))) {
	    uview->size = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->x = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->y = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->complete = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->date = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->id = c_number(car(props));
	    props = cdr(props);
	}
	if (symbolp(car(props))) {
            uview->image_name = c_string(car(props));
	    props = cdr(props);
         }
	add_unit_view_raw(side, uview, uview->x, uview->y);
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

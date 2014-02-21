// xConq
// Lifecycle management and ser/deser of sides.

// $Id: side.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of sides.
    \ingroup grp_gdl
*/

#include "gdl/kernel.h"
#include "gdl/ui.h"
#include "gdl/gvars.h"
#include "gdl/tables.h"
#include "gdl/desc.h"
#include "gdl/dir.h"
#include "gdl/namer.h"
#include "gdl/player.h"
#include "gdl/side/side.h"
#include "gdl/history.h"
#include "gdl/score.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

// Function Macros: Queries

//! What is elevation at given cell, if elevations layer defined?
#define checked_elev_at(x, y) (elevations_defined() ? elev_at(x, y) : 0)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Side *sidelist;
Side *lastside;
int numsides;
int numtotsides;

AI_ops *all_ai_ops;

// Global Variables: Uniques

Side *indepside;

// Global Variables: Game Setup

Obj *side_defaults;

// Global Variables: Counters

int nextsideid;

// Global Variables: Behavior Options

//! To compress layer output or not to compress layer output?
int tmpcompress;

// Global Variables: Buffers

//! Scratch side.
Side *tmpside;

// Local Variables

//! Names of the default colors for the various sides.
/*!
    Used for solid color emblems. These colors have been carefully
    tested so that they stand out against most backgrounds and also can be
    easily distinguished from each other. The indepside default emblem is
    set to "none" since it usually goes without emblem.
*/
static char *default_colornames[MAXSIDES + 1] = {
	"lemon-chiffon",
	"blue",
	"red",
	"yellow",
	"green",
	"dark-orange",
	"deep-pink",
	"cyan",
	"purple",
	"tomato",
	"violet",
	"dodger-blue",
	"light-sea-green",
	"rosy-brown",
	"slate-blue",
	"firebrick"
};

//! Which color names have been used thus far?
/*! Default colors used so far by any non-removed side. Stores the
    number of the side using the color, or else zero.
*/
static short default_colornames_used[MAXSIDES + 1];

// Local Variables: Buffers

//! Buffer used for readable side description (for debugging).
static char *sidedesigbuf = NULL;

// Local Variables: Visibility and Vision

//! Do side people always see what's in their cells?
static int people_always_see;
//! Do side people have chances to see what's in their cells?
static int any_people_see_chances = -1;
//! Are there any seeing chances at all in game?
static int any_see_chances = -1;
//! What is the max range out of all seeing chances?
static int max_see_chance_range;
//! Any chance to mistake type of units in game?
static int any_see_mistake_chances = -1;
//! what is max range out of all seeing mistake chances?
static int max_see_mistake_range;

//! Area coverage: x-position temporary.
static int tmpx0;
//! Area coverage: y-position temporary.
static int tmpy0;
//! Area coverage: z-position temporary.
static int tmpz0;
//! Area coverage: x-position temporary.
static int tmpnx;
//! Area coverage: y-position temporary.
static int tmpny;
//! Area coverage: z-position temporary.
static int tmpnz;

//! Suppress side effects of seeing cells.
/*!
    Set this flag to redo coverage without doing all the side effects
    that come with see_cell.
*/
static int suppress_see_cell;

//! Suppress waking behavior from seeing cells.
/*!
    Set this flag to initialize views without waking any units.  This
    is necessary when restoring a game, since we construct the view
    using this code, but don't want to modify the units' plans from
    what they were when the game was saved.
*/
static int suppress_see_wakeup;

// Local Function Declarations: Validation

static void check_name_uniqueness(Side *side, char *str, char *kind);

// Local Function Declarations: Visibility and Vision

//! Determine a mistaken type for given unit type.
/*!
    Given a unit type, return a similar-looking type that it might be
    confused with.
*/
static int mistaken_type(int u2);
//! Possibly mistake the type of an unit being viewed.
/*!
    If the viewing unit might possibly misidentify the type of unit it
    saw, compute the chance and replace the view's type with one of the
    possible types that it could be mistaken for.
*/
static void mistake_view(Side *side, Unit *seer, UnitView *uview);
//! Calculate if one unit sees another, and whether vision is perfect or not.
/*!
    Check if one unit sees another,
    and flag whether that vision is clear or not.
*/
static int see_unit(Unit *seer, Unit *tosee, SeerNode *seers, int *numseers);
//! Calculate how many occupants of given side unit can see other given unit.
/*!
    Iterate 'see_unit' over all the occs of a transport.
    \note Can be used as a search predicate.
*/
static int side_occs_see_unit(int x, int y, int *numseers, ParamBox *parambox);
//! Calculate how many side units in given cell see given unit.
/*!
    Iterate 'see_unit' over all the units in a cell.
    \note Can be used as a search predicate.
*/
static int side_ustack_see_unit(
    int x, int y, int *numseers, ParamBox *parambox);

//! Initialize visible elevation from given position.
static void init_visible_elevation(int x, int y);
//! Initialize visible elevation at given position.
static void init_visible_elevation_2(int x, int y);
//! Calculate visible elevation from given position.
static void calc_visible_elevation(int x, int y);
//! Calculate visible elevation at given position.
static void calc_visible_elevation_2(int x, int y);

//! Cover area for given side on account of given unit.
static void cover_area_1(
    Side *side, struct a_unit *unit, int x0, int y0, int x1, int y1);

//! Can given side see any materials in given cell?
static int see_materials(Side *side, int x, int y);
//! Can given side see any weather in given cell?
static int see_weather(Side *side, int x, int y);
//! Determine if there is any possibility of uncertain sighting.
/*!
    Determine whether there is any possibility of an uncertain sighting,
    and cache the conclusion.
    \note 'any_see_chances' will be only be set if a see chance is not 100%.
	  Thus, if all the see chances are 0, it will be set. The variable
	  name is somewhat misleading.
*/
static void compute_see_chances(void);

// Local Function Declarations: Game Setup

//! Return default color name for side.
static char *pick_default_colorname(Side *side);

// Local Function Declarations: GDL I/O

//! Read given side's attack stats from GDL list.
static void interp_atkstats_list(Side *side, Obj *lis);
//! Read given side's hit stats from GDL list.
static void interp_hitstats_list(Side *side, Obj *lis);

// Queries

Side *
side_n(int n)
{
    Side *side;

    for_all_sides(side)
      if (side->id == n)
        return side;
    return NULL;
}

Side *
find_side_by_name(char *str)
{
    Side *side;

    if (empty_string(str))
      return NULL;
    for_all_sides(side) {
	if (!empty_string(side->name) && strcmp(side->name, str) == 0)
	  return side;
	if (!empty_string(side->noun) && strcmp(side->noun, str) == 0)
	  return side;
	if (!empty_string(side->pluralnoun) && strcmp(side->pluralnoun, str) == 0)
	  return side;
	if (!empty_string(side->adjective) && strcmp(side->adjective, str) == 0)
	  return side;
    }
    return NULL;
}

int
side_number(Side *side)
{
    return side->id;
}

char *
side_name(Side *side)
{
    return (side->name ? side->name :
	    (side->adjective ? side->adjective :
	     (side->pluralnoun ? side->pluralnoun :
	      (side->noun ? side->noun :
	       (char *)""))));
}

char *
side_adjective(Side *side)
{
    return (side->adjective ? side->adjective :
	    (side->noun ? side->noun :
	     (side->pluralnoun ? side->pluralnoun :
	      (side->name ? side->name :
	       (char *)""))));
}

char *
side_desig(Side *side)
{
    Unit *unit;

    if (sidedesigbuf == NULL)
      sidedesigbuf = (char *)xmalloc(BUFSIZE);
    if (side != NULL) {
	/* The side name is all capitalized in the output. Why? */
	sprintf(sidedesigbuf, "s%d %s", side_number(side), side_name(side));
	if (side->self_unit) {
	    unit = find_unit_dead_or_alive(side->self_unit->id);
	    if (in_play(unit)) {
		if (!mobile(unit->type) || u_advanced(unit->type)) {
		    tprintf(sidedesigbuf, " (capital %s %s)",
			    (side->ingame ? "is" : "was"),
			    short_unit_handle(unit));
		} else {
		    tprintf(sidedesigbuf, " (leader %s %s)",
			    (side->ingame ? "is" : "was"),
			    short_unit_handle(unit));
		}
	    }
	}
    } else {
	sprintf(sidedesigbuf, "nullside");
    }
    return sidedesigbuf;
}

int
trusted_side(Side *side1, Side *side2)
{
    if (side1 == side2)
      return TRUE;
    if (side1 == NULL || side2 == NULL || side1->trusts == NULL)
      return FALSE;
    return (side1->trusts[side2->id]);
}

int
side_controls_side(Side *side, Side *side2)
{
    return (side == side2 || side2->controlled_by == side);
}

int
allied_side(Side *side1, Side *side2)
{
    if (trusted_side(side1, side2))
      return TRUE;
    return FALSE;
}

int
side_owns_viewer_in_unit(Side *side, Unit *unit)
{
    Unit *occ = NULL;

    assert_error(side, "Attempted to access a NULL side");
    if (unit->occupant == NULL)
      return FALSE;
    for_all_occupants(unit, occ) {
	/* (TODO: Account for transport adding vision range to occ.) */
	if ((occ->side == side) && (0 <= u_vision_range(unit->type)))
	  return TRUE;
	if (occ->occupant) {
	    if (side_owns_viewer_in_unit(side, occ))
	      return TRUE;
	}
    }
    return FALSE;
}

int
name_in_use(Side *side, char *str)
{
    Side *side2;

    if (empty_string(str))
      return FALSE;
    for_all_sides(side2) {
	if (side2 != side) {
	    if ((side2->name && strcmp(str, side2->name) == 0)
		|| (side2->noun && strcmp(str, side2->noun) == 0)
		|| (side2->pluralnoun && strcmp(str, side2->pluralnoun) == 0)
		|| (side2->adjective && strcmp(str, side2->adjective) == 0)
		|| (side2->colorscheme && strcmp(str, side2->colorscheme) == 0)
		)
	      return TRUE;
	}
    }
    return FALSE;
}

// Validation

static
void
check_name_uniqueness(Side *side, char *str, char *kind)
{
    if (name_in_use(side, str)) {
	init_warning("Side %s `%s' is already in use", kind, str);
    }
}

// Visibility and Vision

static
int
mistaken_type(int u2)
{
    int u3, totalweight, randval;

    totalweight = 0;
    for_all_unit_types(u3) {
	totalweight += uu_looks_like(u2, u3);
    }
    /* If nothing that it looks like, return original type. */
    if (totalweight == 0)
      return u2;
    randval = xrandom(totalweight);
    totalweight = 0;
    for_all_unit_types(u3) {
	totalweight += uu_looks_like(u2, u3);
	if (randval < totalweight)
	  return u3;
    }
    return u2;
}

static
void
mistake_view(Side *side, Unit *seer, UnitView *uview)
{
    assert_warning_return(seer, "Attempted to see with a NULL unit",);
    assert_warning_return(side, "Attempted to access a NULL side",);
    assert_warning_return(uview, "Attempted to see a NULL unit view",);
    uview->type = mistaken_type(uview->type);
    uview->image_name = NULL;
    uview->imf = NULL;
    set_unit_view_image(uview);
}

int
see_unit(Unit *seer, Unit *tosee, SeerNode *seers, int *numseers)
{
    int dist = -1;
    int u = NONUTYPE, us = NONUTYPE;

    assert_error(seers, "Attempted to access a NULL array in seeing code");
    assert_error(numseers && (*numseers >= 0),
		 "Attempted to access a bad counter in seeing code");
    assert_warning_return(in_play(seer),
			  "Attempted to access an out-of-play unit", FALSE);
    assert_warning_return(in_play(tosee),
			  "Attempted to access an out-of-play unit", FALSE);
    if (seer == tosee)
      return FALSE;
    if (seer->side == tosee->side)
      return FALSE;
    u = tosee->type;
    us = seer->type;
    dist = distance(tosee->x, tosee->y, seer->x, seer->y);
    if (probability(see_chance(seer, tosee))) {
	seers[*numseers].seer = seer;
	if ((uu_see_mistake_range_min(us, u) <= dist)
	    && uu_see_mistake(us, u)) {
	    if (xrandom(10000) < uu_see_mistake(us, u))
	      seers[*numseers].mistakes = TRUE;
	}
	++(*numseers);
	return TRUE;
    }
    return FALSE;
}

static
int
side_ustack_see_unit(int x, int y, int *numseers, ParamBox *parambox)
{
    ParamBoxUnitSideSeers *paramboxus = NULL;
    ParamBoxUnitUnitSeers *paramboxuu = NULL;
    int foundseer = FALSE;
    Unit *seer = NULL;

    assert_warning_return(in_area(x, y), "Attempted to use illegal coordinates",
			  FALSE);
    assert_warning_return(parambox, "Attempted to access a NULL parambox",
			  FALSE);
    assert_warning_return(PBOX_TYPE_UNIT_SIDE_SEERS == parambox->get_type(),
			  "Attempted to use wrong type of parambox",
			  FALSE);
    paramboxus = (ParamBoxUnitSideSeers *)parambox;
    assert_warning_return(paramboxus->unit,
			  "Could not check visibility of an unit", FALSE);
    assert_warning_return(paramboxus->side,
			  "Attempted to access a NULL side", FALSE);
    assert_warning_return(numseers && (*numseers >= 0),
			  "Invalid parambox data encountered", FALSE);
    assert_warning_return(paramboxus->seers,
			  "Attempted to access a NULL array", FALSE);
    /* If no possible seers, then return now. */
    if (!unit_at(x, y))
      return FALSE;
    /* Iterate over all stack members. */
    for_all_stack(x, y, seer) {
	/* Paranoia. Skip any out-of-play units in stack. */
	if (!in_play(seer))
	  continue;
	/* HACK: We need to pass more information into the recursive routine.
	   The seer may not be of the same side as the side that is supposed
	   to be seeing. For now we must assume that an occ of the correct
	   side is not lurking in an unit of the wrong side. */
	if (seer->side != paramboxus->side) {
	    continue;
	}
	/* Descend depth-first before checking anything else. A seer of the
	   correct side may be lurking in a transport of an incorrect side. */
	if (seer->occupant) {
	    paramboxuu =
		(ParamBoxUnitUnitSeers *)xmalloc(sizeof(ParamBoxUnitUnitSeers));
	    paramboxuu->unit1 = paramboxus->unit;
	    paramboxuu->unit2 = seer;
	    paramboxuu->seers = paramboxus->seers;
	    if (side_occs_see_unit(x, y, numseers, (ParamBox *)paramboxuu))
	      foundseer = TRUE;
	    if (paramboxuu)
	      free(paramboxuu);
	}
	/*! \todo Put the side filter back in the correct order once the
		  proper side info is being passed to 'side_occs_see_unit'. */
#if (0)
	/* Skip any units not on given side. */
	/* (TODO: Also account for trusted sides.) */
	if (seer->side != paramboxus->side) {
	    continue;
	}
#endif
	/* Try seeing with the current seer. */
	if (see_unit(seer, paramboxus->unit, paramboxus->seers, numseers))
	  foundseer = TRUE;
	/* Check if we should conitnue evaluating. */
	if (*numseers >= cover(paramboxus->side, x, y))
	  break;
    }
    /* return foundseer; */
    return FALSE;
}

static
int
side_occs_see_unit(int x, int y, int *numseers, ParamBox *parambox)
{
    ParamBoxUnitUnitSeers *paramboxuu = NULL;
    ParamBoxUnitUnitSeers *paramboxuu2 = NULL;
    int foundseer = FALSE;
    Unit *seer = NULL;

    assert_warning_return(in_area(x, y), "Attempted to use illegal coordinates",
			  FALSE);
    assert_warning_return(parambox, "Attempted to access a NULL parambox",
			  FALSE);
#if (0)
    assert_warning_return(PBOX_TYPE_UNIT_UNIT_SEERS == parambox->get_type(),
			  "Attempted to use wrong type of parambox",
			  FALSE);
#endif
    paramboxuu = (ParamBoxUnitUnitSeers *)parambox;
    assert_warning_return(paramboxuu->unit1,
			  "Could not check visibility of an unit", FALSE);
    assert_warning_return(paramboxuu->unit2,
			  "Attempted to access a NULL unit", FALSE);
    assert_warning_return(numseers && (*numseers >= 0),
			  "Invalid parambox data encountered", FALSE);
    assert_warning_return(paramboxuu->seers,
			  "Attempted to access a NULL array", FALSE);
    /* If no possible seers, then return now. */
    if (!paramboxuu->unit2->occupant)
      return FALSE;
    /* Iterate over all occupants. */
    for_all_occupants(paramboxuu->unit2, seer) {
	/* Paranoia. Skip any out-of-play unit. */
	if (!in_play(seer))
	  continue;
	/* HACK: We need to pass more information into the recursive routine.
	   The seer may not be of the same side as the side that is supposed
	   to be seeing. For now we must assume that an occ of the correct
	   side is not lurking in an unit of the wrong side. */
	if (seer->side != paramboxuu->unit2->side) {
	    continue;
	}
	/* Descend depth-first before checking anything else. A seer of the
	   correct side may be lurking in a transport of an incorrect side. */
	if (seer->occupant) {
	    paramboxuu2 =
		(ParamBoxUnitUnitSeers *)xmalloc(sizeof(ParamBoxUnitUnitSeers));
	    paramboxuu2->unit1 = paramboxuu->unit1;
	    paramboxuu2->unit2 = seer;
	    paramboxuu2->seers = paramboxuu->seers;
	    if (side_occs_see_unit(x, y, numseers, (ParamBox *)paramboxuu2))
	      foundseer = TRUE;
	    if (paramboxuu2)
	      free(paramboxuu2);
	}
	/*! \todo Put the side filter back in the correct order once the
		  proper side info is being passed to 'side_occs_see_unit'. */
#if (0)
	/* Skip any units not on given side. */
	/* (TODO: Also account for trusted sides.) */
	if (seer->side != paramboxuu->unit2->side) {
	    continue;
	}
#endif
	/* Try seeing with the current seer. */
	if (see_unit(seer, paramboxuu->unit1, paramboxuu->seers, numseers))
	  foundseer = TRUE;
	/* Check if we should conitnue evaluating. */
	if (*numseers >= cover(paramboxuu->unit2->side, x, y))
	  break;
    }
    /* return foundseer; */
    return FALSE;
}

static
void
init_visible_elevation(int x, int y)
{
    int elev = checked_elev_at(x, y);

    set_tmp2_at(x, y, elev - tmpz0);
}

static
void
init_visible_elevation_2(int x, int y)
{
    int elev = checked_elev_at(x, y);

    set_tmp3_at(x, y, elev - tmpnz);
}

static
void
calc_visible_elevation(int x, int y)
{
    int dir, x1, y1, elev, tmp, tmpa, tmpb;
    int adjelev = 9999, viselev, dist, dist1, cellwid = area.cellwidth;

    elev = checked_elev_at(x, y);
    dist = distance(x, y, tmpx0, tmpy0);
    if (cellwid <= 0)
      cellwid = 1;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    dist1 = distance(x1, y1, tmpx0, tmpy0);
	    if (dist1 < dist) {
		tmpa = tmp2_at(x1, y1);
		/* Account for the screening effect of the elevation
                   difference. */
		/* (dist1 will never be zero) */
		tmpa = (tmpa * dist * cellwid) / (dist1 * cellwid);
		tmpb = checked_elev_at(x1, y1)
		  + t_thickness(terrain_at(x1, y1))
		  - tmpz0;
		tmpb = (tmpb * dist * cellwid) / (dist1 * cellwid);
		tmp = max(tmpa, tmpb);
		adjelev = min(adjelev, tmp + tmpz0);
	    }
	}
    }
    viselev = max(adjelev, elev);
    set_tmp2_at(x, y, viselev - tmpz0);
}

static
void
calc_visible_elevation_2(int x, int y)
{
    int dir, x1, y1, elev, tmp, tmpa, tmpb;
    int adjelev = 9999, viselev, dist, dist1, cellwid = area.cellwidth;

    elev = checked_elev_at(x, y);
    dist = distance(x, y, tmpnx, tmpny);
    if (cellwid <= 0)
      cellwid = 1;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    dist1 = distance(x1, y1, tmpnx, tmpny);
	    if (dist1 < dist) {
		tmpa = tmp3_at(x1, y1);
		/* Account for the screening effect of the elevation
                   difference. */
		/* (dist1 will never be zero) */
		tmpa = (tmpa * dist * cellwid) / (dist1 * cellwid);
		tmpb = checked_elev_at(x1, y1)
		  + t_thickness(terrain_at(x1, y1))
		  - tmpnz;
		tmpb = (tmpb * dist * cellwid) / (dist1 * cellwid);
		tmp = max(tmpa, tmpb);
		adjelev = min(adjelev, tmp + tmpnz);
	    }
	}
    }
    viselev = max(adjelev, elev);
    set_tmp3_at(x, y, viselev - tmpnz);
}

int
side_sees_unit(Side *side, Unit *unit)
{
    int x = -1, y = -1;

    if (unit == NULL)
      return FALSE;
    /* If the side is omniscient. */
    if (side->see_all || side->show_all)
      return TRUE;
    /* If the unit is 'see-always' and on known terrain in the arena. */
    x = unit->x;  y = unit->y;
    if (u_see_always(unit->type) && inside_area(x, y)
	&& (UNSEEN != terrain_view(side, x, y)))
      return TRUE;
    /* If the side controls the unit's side. */
    if (side_controls_side(side, unit->side))
      return TRUE;
    /* If the unit's side trusts the side. */
    if (trusted_side(unit->side, side))
      return TRUE;
    /* If the game is over. */
    if (endofgame)
      return TRUE;
    /* If the side's people can see the unit. */
    if (people_always_see && people_sides_defined()
	&& (people_side_at(unit->x, unit->y) == side->id))
      return TRUE;
    /* Else, side cannot see. */
    return FALSE;
}

int
occupants_visible(Side *side, Unit *unit)
{
    if (unit->occupant == NULL)
      return FALSE;
    /* If the side is omniscient. */
    if (side->see_all)
      return TRUE;
    /* If the side sees the transport and the occupants are to be revealed. */
    if (side_sees_unit(side, unit) && u_see_occupants(unit->type))
      return TRUE;
    /* If the side owns a seeing unit in the transport. */
    /* (TODO: Handle the case of a trusted unit providing info.) */
    if (side_owns_viewer_in_unit(side, unit))
      return TRUE;
    return FALSE;
}

static
void
cover_area_1(Side *side, Unit *unit, int x0, int y0, int nx, int ny)
{
    int u = unit->type, range0, nrange, range, x, y, x1, y1, x2, y2;
    int y1c, y2c, cov, los, r;
    int xmin, ymin, xmax, ymax, oldcov, newcov, anychanges;

    if (side->coverage == NULL)
      return;
    range0 = nrange = u_vision_range(u);
    /* Adjust for the effects of nighttime on vision range. */
    if (in_area(x0, y0)) {
	if (night_at(x0, y0)) {
	    range0 =
	      (range0 * ut_vision_night_effect(u, terrain_at(x0, y0))) / 100;
	}
    } else {
	range0 = 0;
    }
    if (in_area(nx, ny)) {
	if (night_at(nx, ny)) {
	    nrange =
	      (nrange * ut_vision_night_effect(u, terrain_at(nx, ny))) / 100;
	}
    } else {
	nrange = 0;
    }
    range = max(range0, nrange);
    allocate_area_scratch(1);
    anychanges = FALSE;
    /* First, set the union of the from and to areas to the existing
       coverage. */
    /* These may be outside the area - necessary since units may be able
       to see farther in x than the height of the area. */
    /* Compute the maximum bounds that may be affected. */
    if (y0 >= 0) {
	if (ny >= 0) {
	    ymin = min(y0, ny);
	    ymax = max(y0, ny);
	} else {
	    ymin = ymax = y0;
	}
    } else if (ny >= 0) {
	ymin = ymax = ny;
    }
    if (x0 >= 0) {
	if (nx >= 0) {
	    xmin = min(x0, nx);
	    xmax = max(x0, nx);
	} else {
	    xmin = xmax = x0;
	}
    } else if (nx >= 0) {
	xmin = xmax = nx;
    }
    if (any_los) {
	/* Need extra scratch layers, will be used for visible
           elevation cache. */
	allocate_area_scratch(3);
    }
    los = FALSE;
    /* (should also adjust for effect of clouds here) */
    if (!u_can_see_behind(u)) {
	los = TRUE;
	/* Compute the minimum elevation for visibility at each cell. */
	if (in_area(x0, y0)) {
	    tmpx0 = x0;  tmpy0 = y0;
	    tmpz0 = checked_elev_at(x0, y0)
		+ unit_alt(unit)
		+ ut_eye_height(u, terrain_at(x0, y0));
	    apply_to_area(x0, y0, range0, init_visible_elevation);
	    /* Leave own and adj cells alone, they will always be visible. */
	    for (r = 2; r <= range0; ++r) {
		apply_to_ring(x0, y0, r, r, calc_visible_elevation);
	    }
	    /* We now have a layer indicating how high things must be
               to be visible. */
	}
	if (in_area(nx, ny)) {
	    tmpnx = nx;  tmpny = ny;
	    tmpnz = checked_elev_at(nx, ny)
		+ unit_alt(unit)
		+ ut_eye_height(u, terrain_at(nx, ny));
	    apply_to_area(nx, ny, nrange, init_visible_elevation_2);
	    /* Leave own and adj cells alone, they will always be visible. */
	    for (r = 2; r <= nrange; ++r) {
		apply_to_ring(nx, ny, r, r, calc_visible_elevation_2);
	    }
	    /* We now have another layer, indicating how high things must be
               to be visible from the new location. */
	}
    }
    /* Copy the current coverage into the tmp layer. */
    y1 = y1c = ymin - range;
    y2 = y2c = ymax + range;
    /* Clip the iteration bounds. */
    if (y1c < 0)
      y1c = 0;
    if (y2c > area.height - 1)
      y2c = area.height - 1;
    for (y = y1c; y <= y2c; ++y) {
	x1 = xmin - range;
	x2 = xmax + range;
	for (x = x1; x <= x2; ++x) {
	    if (in_area(x, y)) {
		set_tmp1_at(x, y, cover(side, x, y));
	    }
	}
    }
    /* Decrement coverage around the old location. */
    if (in_area(x0, y0)) {
	y1 = y1c = y0 - range0;
	y2 = y2c = y0 + range0;
	/* Clip the iteration bounds. */
	if (y1c < 0)
	  y1c = 0;
	if (y2c > area.height - 1)
	  y2c = area.height - 1;
	for (y = y1c; y <= y2c; ++y) {
	    x1 = x0 - (y < y0 ? (y - y1) : range0);
	    x2 = x0 + (y > y0 ? (y2 - y) : range0);
	    for (x = x1; x <= x2; ++x) {
		if (in_area(x, y)) {
		    if (!los
			|| ((tmp2_at(x, y) + tmpz0)
			    <= (checked_elev_at(x, y)
				+ t_thickness(terrain_at(x, y))))) {
			cov = tmp1_at(x, y) - 1;
			/* Should never go negative, detect if so. */
			if (cov < 0) {
			    Dprintf("Negative coverage for %s at %d,%d\n",
				    side_desig(side), wrapx(x), y);
			}
			set_tmp1_at(x, y, cov);
		    }
		    if (los && (alt_cover(side, x, y) == (tmp2_at(x, y) + tmpz0)))
		      /* this unit set the min, should recalc alt
                         coverage now */;
		}
	    }
	}
    }
    /* Increment coverage around the new location. */
    if (in_area(nx, ny)) {
	y1 = y1c = ny - nrange;
	y2 = y2c = ny + nrange;
	/* Clip the iteration bounds. */
	if (y1c < 0)
	  y1c = 0;
	if (y2c > area.height - 1)
	  y2c = area.height - 1;
	for (y = y1c; y <= y2c; ++y) {
	    x1 = nx - (y < ny ? (y - y1) : nrange);
	    x2 = nx + (y > ny ? (y2 - y) : nrange);
	    for (x = x1; x <= x2; ++x) {
		if (in_area(x, y)) {
		    if (!los
			|| ((tmp3_at(x, y) + tmpnz)
			    <= (checked_elev_at(x, y)
				+ t_thickness(terrain_at(x, y))))) {
			cov = tmp1_at(x, y) + 1;
			set_tmp1_at(x, y, cov);
		    }
		    if (los)
		      set_alt_cover(side, x, y,
				    min(alt_cover(side, x, y),
					(tmp3_at(x, y) + tmpnz)));
		}
	    }
	}
    }
    /* Now update the actual coverage.  Do this over an area that includes
       both the decrement and increment changes. */
    y1 = y1c = ymin - range;
    y2 = y2c = ymax + range;
    /* Clip the iteration bounds. */
    if (y1c < 0)
      y1c = 0;
    if (y2c > area.height - 1)
      y2c = area.height - 1;
    for (y = y1c; y <= y2c; ++y) {
	x1 = xmin - range;
	x2 = xmax + range;
	for (x = x1; x <= x2; ++x) {
	    if (in_area(x, y)) {
		oldcov = cover(side, x, y);
		newcov = tmp1_at(x, y);
		if (newcov != oldcov) {
		    set_cover(side, x, y, newcov);
		    /* Skip over unit view updating if we're just repairing
		       the coverage layer. */
		    if (suppress_see_cell)
		      continue;
		    if (newcov > oldcov
			&& see_cell(side, x, y)
			&& !suppress_see_wakeup) {
				react_to_seen_unit(side, unit, x, y);
		    }
		    if ((newcov > 0 && oldcov == 0)
			|| (newcov == 0 && oldcov > 0)
			|| (DebugG && newcov != oldcov)) {
				update_cell_display(side, x, y, UPDATE_COVER);
		    }
		    anychanges = TRUE;
		}
	    }
	}
    }
    /* If we're seeing new things, make sure they're on the display. */
    if (anychanges) {
	flush_display_buffers(side);
	/* Also flush the unit view list. */
	flush_stale_views();
    }
}

void
cover_area(
    Side *side,
    Unit *unit,
    Unit *oldtransport,
    int x0, int y0,
    int nx, int ny)
{
    Side *side2;

    if (side != NULL
	&& completed(unit)
	&& (oldtransport == NULL
	   || uu_occ_vision(unit->type, oldtransport->type) > 0)) {
	if (side->ingame) {
	    /* Active sides keep their allies informed.  Note that
	       this will effectively leak information to allies of
	       allies, even if the unit's side is not an ally
	       directly. */
	    for_all_sides(side2) {
		if (trusted_side(side, side2)
		    || side2->see_all) {
		    cover_area_1(side2, unit, x0, y0, nx, ny);
		}
	    }
	} else {
	    /* Inactive sides just have their own data maintained
	       (not clear if this is critical, but seems sensible) */
	    cover_area_1(side, unit, x0, y0, nx, ny);
	}
    }
}

int
see_cell(Side *side, int x, int y)
{
    int update, updatet, chance, curview;
    int m, mupdate, wupdate, flags, domore, rslt, sawthis;
    Unit *unit;
    UnitView *uview, *newuview;
    SeerNode seers [BUFSIZE];
    int numseers = 0, i = 0, seenclearly = FALSE, cellcover = 0;
    ParamBoxUnitSideSeers paramboxus;

    if (!in_area(x, y))
      return FALSE;
    update = updatet = rslt = FALSE;
    if (cover(side, x, y) > 0) {
    	/* Always update our knowledge of the terrain. */
    	curview = terrain_view(side, x, y);
 	if (curview == UNSEEN || !g_see_terrain_always()) {
	    set_terrain_view(side, x, y, buildtview(terrain_at(x, y)));
	    if (!g_see_terrain_always()) {
		set_terrain_view_date(side, x, y, g_turn());
	    }
	    update = updatet = TRUE;
	    if (side->see_all) {
		add_cover(side, x, y, 1);
	    } else {
		for_all_stack(x, y, unit) {
		    if (u_see_always(unit->type)) {
			add_cover(side, x, y, 1);
			break;
		    }
		}
	    }
	}
	if (any_material_views) {
	    mupdate = see_materials(side, x, y);
	    if (mupdate)
	      update = TRUE;
	}
	if (temperatures_defined() || clouds_defined() || winds_defined()) {
	    wupdate = see_weather(side, x, y);
	    if (wupdate)
	      update = TRUE;
	}
	if (any_see_chances < 0)
	  compute_see_chances();
	/* Get rid of any unit views that are no longer valid here. */
	domore = TRUE;
	while (domore) {
	    domore = FALSE;
	    for_all_view_stack(side, x, y, uview) {
		/* Conditionally flush unit views. */
		/*! \note Improve this code to consider view flushing
			  granularity. */
		unit = view_unit(uview);
		if (!unit || (uview->x != unit->x) || (uview->y != unit->y)
		    || (uview->transport
			&& (view_unit(uview->transport) != unit->transport))
		    || (!u_see_always(uview->type)
			&& (uview->date < g_turn()))) {
		    if (remove_unit_view(side, uview)) {
			update = TRUE;
		    }
		    domore = TRUE;
		    break;
		}
	    }
	}
	/* Iterate over all units-to-see in a given location. */
	for_all_stack(x, y, unit) {
	    /* Reset the flag that says whether we've spotted this
	       particular unit in the stack. */
	    sawthis = FALSE;
	    /* If any definite sightings. */
	    if (side_sees_unit(side, unit)) {
		newuview = add_unit_view(side, unit);
		if (newuview)
		  update = TRUE;
		rslt = sawthis = TRUE;
	    /* If any possible sightings. */
	    /* All see chances may be perfect by default ('any_see_chances'
	       will be FALSE), but terrain, night, etc... modifications still
	       take place, and mistaken viewings can still occur. */
	    }
	    else {
		numseers = 0;
		cellcover = cover(side, x, y);
#if (0)
		/* Allocate array of max possible seers. */
		if (cellcover > 0)
		  seers = (SeerNode *)xmalloc(cellcover * sizeof(SeerNode));
#endif
		/* Reset the seers array. */
		memset(seers, 0, BUFSIZE*sizeof(SeerNode));
		/* Setup parambox to iterate through ustacks. */
		paramboxus.unit = unit;
		paramboxus.side = side;
		paramboxus.seers = seers;
		/* Look in the same cell. */
		if (cellcover)
		  side_ustack_see_unit(x, y, &numseers,
				       (ParamBox *)(&paramboxus));
		/* Look in other cells out to the maximum vision range. */
		/* (NOTE: There should probably be a performance choice
		    here. If the max vision range is sufficiently large, then
		    it may be cheaper to iterate through the side units
		    instead of searching the area.) */
		if ((numseers < cellcover)
		    && (0 < max_see_chance_range)) {
		    limited_search_around(x, y, max_see_chance_range,
					  side_ustack_see_unit, 1, &numseers,
					  cellcover, (ParamBox *)&paramboxus);
		}
		/* Let the caller know that something is seen. */
		if (numseers)
		  rslt = TRUE;
		seenclearly = FALSE;
		/* Now that we have gathered up all the seers, start
		   building unmistaken unit views. */
		for (i = 0; i < numseers; ++i) {
		    /* Skip over mistaken views. */
		    if (seers[i].mistakes)
		      continue;
		    /* Construct an unit view. */
		    /* If coordinated vision, then we build the new uview
		       without hesitation. */
		    /* (NOTE: Currently, uncoordinated vision is not supported
			by the Xconq uview code.) */
		    if (1 /* (TODO: Replace with appropriate flag.) */) {
			newuview = add_unit_view(side, unit);
			seenclearly = TRUE;
		    }
		    /* If this is a new unit view, then a display update is
		       needed. */
		    /* (NOTE: This machinery will probably need to be
			changed if uncoordinated vision or advanced fog-of-war
			make it into Xconq.) */
		    if (newuview)
		      update = TRUE;
		    /* If coordinated vision, then any single accurate
		       sighting will be sufficient, and we don't need to
		       build additional unit views. */
		    if (1 /* (TODO: Replace with appropriate flag.) */)
		      break;
		} /* for all unmistaken views */
		/* If no one saw anything clearly, then investigate the
		   possibility of mistaken views. */
		for (i = 0; i < numseers; ++i) {
		    /* If coordinated vision and something else already
		       clearly seen, then what are we doing here? Get out. */
		    if (1 /* (TODO: Replace with appropriate flag.) */
			&& seenclearly)
		      break;
		    /* If coordinated vision and we are still in this loop,
		       then all seers must be mistaken about what they see. */
		    if (1 /* (TODO: Replace with appropriate flag.) */) {
			newuview = add_unit_view(side, unit);
			/* If this is a new unit view, then a display update is
			   needed. */
			/* (NOTE: This machinery will probably need to be
			    changed if uncoordinated vision or advanced
			    fog-of-war make it into Xconq.) */
			if (newuview)
			  update = TRUE;
			/* If no advanced fog-of-war, then we pick one of the
			   seers at random and agree upon its view of things,
			   and don't bother with other possible views. */
			if (newuview
			    && 1 /* (TODO: Replace with appropriate flag.) */) {
			    /* Mistake the view. */
			    mistake_view(side, seers[xrandom(numseers)].seer,
					 newuview);
			    break;
			}
			/* (TODO: Handle advanced fog-of-war.) */
		    } /* Coordinated vision. */
		    /* Skip over unmistaken views. */
		    if (!(seers[i].mistakes))
		      continue;
		    /* (TODO: Handle uncoordinated vision.) */
		} /* for all mistaken views */
#if (0)
		/* Deallocate array of seers if necessary. */
		if (seers) {
		    free(seers);
		    seers = NULL;
		}
#endif
	    }
	} /* for all stack units */
	/* Check if any populations in the cell see something. */
	/* 'side_sees_unit' will detect an unit if the people of a side
	    always see and are present in a cell. */
	if (!people_always_see && any_people_see_chances
	    && people_sides_defined()
	    && people_side_at(x, y) == side->id
	    && any_cell_materials_defined()) {
	    for_all_stack(x, y, unit) {
		for_all_material_types(m) {
		    if (cell_material_defined(m)
			&& material_at(x, y, m) > 0) {
			chance = um_people_see(unit->type, m);
			if (probability(chance)) {
			    newuview = add_unit_view(side, unit);
			    if (newuview)
			      update = TRUE;
			    /* Proceed to consider the next unit. */
			    break;
			}
		    }
		}
   	    }
	}
    }
    /* If there was any change in what was visible, tell the display. */
    if (update) {
	flags = UPDATE_ALWAYS;
	/* If the view of the terrain changed, adjacent cells probably
	   need to be redrawn as well. */
	if (updatet)
	  flags |= UPDATE_ADJ;
	update_cell_display(side, x, y, flags);
    }
    /* Indicate whether any units were seen at this location, even if
       no display updates were needed. */
    return rslt;
}

static
int
see_materials(Side *side, int x, int y)
{
    int m, curview, update;

    update = FALSE;
    if (!any_material_views)
      return update;
    for_all_material_types(m) {
	if (any_material_views_by_m[m]) {
	    curview = material_view(side, x, y, m);
	    if (curview != material_at(x, y, m)) {
		set_material_view(side, x, y, m, material_at(x, y, m));
		update = TRUE;
	    }
	    /* Even if the data didn't change, our information is
	       now up-to-date. */
	    if (1)
	      set_material_view_date(side, x, y, m, g_turn());
	}
    }
    return update;
}

static
int
see_weather(Side *side, int x, int y)
{
    int curview, update;

    update = FALSE;
    if (temperatures_defined()) {
	curview = temperature_view(side, x, y);
	if (curview != temperature_at(x, y)) {
	    set_temperature_view(side, x, y, temperature_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_temperature_view_date(side, x, y, g_turn());
    }
    if (clouds_defined()) {
	curview = cloud_view(side, x, y);
	if (curview != raw_cloud_at(x, y)) {
	    set_cloud_view(side, x, y, raw_cloud_at(x, y));
	    update = TRUE;
	}
	curview = cloud_bottom_view(side, x, y);
	if (cloud_bottoms_defined()
	    && (curview != raw_cloud_bottom_at(x, y))) {
	    set_cloud_bottom_view(side, x, y, raw_cloud_bottom_at(x, y));
	    update = TRUE;
	}
	curview = cloud_height_view(side, x, y);
	if (cloud_heights_defined()
	    && (curview != raw_cloud_height_at(x, y))) {
	    set_cloud_height_view(side, x, y, raw_cloud_height_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_cloud_view_date(side, x, y, g_turn());
	/* Only need one date for the three layers of view data. */
    }
    if (winds_defined()) {
	curview = wind_view(side, x, y);
	if (curview != raw_wind_at(x, y)) {
	    set_wind_view(side, x, y, raw_wind_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_wind_view_date(side, x, y, g_turn());
    }
    return update;
}

void
compute_see_chances(void)
{
    int u1, u2, u3, m1;

    any_see_chances = FALSE;
    any_people_see_chances = FALSE;
    people_always_see = TRUE;
    max_see_chance_range = -1;
    any_see_mistake_chances = FALSE;
    max_see_mistake_range = -1;
    for_all_unit_types(u2) {
	for_all_unit_types(u1) {
	    if (uu_see_at(u1, u2) != 100) {
		any_see_chances = TRUE;
		max_see_chance_range = max(max_see_chance_range, 0);
	    }
	    if (uu_see_adj(u1, u2) != 100) {
		any_see_chances = TRUE;
		max_see_chance_range = max(max_see_chance_range, 1);
	    }
	    if (uu_see(u1, u2) != 100) {
		any_see_chances = TRUE;
	    }
	    max_see_chance_range = max(max_see_chance_range,
				       u_vision_range(u1));
	    if (uu_see_mistake(u1, u2) > 0) {
		for_all_unit_types(u3) {
		    if (uu_looks_like(u2, u3) > 0) {
			any_see_mistake_chances = TRUE;
			break;
		    }
		}
		max_see_mistake_range = max(max_see_mistake_range,
					    u_vision_range(u1));
	    }
	}
	for_all_material_types(m1) {
	    if (m_people(m1) > 0 && um_people_see(u2, m1) > 0) {
		any_people_see_chances = TRUE;
		if (um_people_see(u2, m1) < 100)
		  people_always_see = FALSE;
	    }
	}
	if (people_sides_defined())
	  any_people_see_chances = TRUE;
    }
}

void
wake_unit(Side *side, Unit *unit, int forcewakeoccs)
{
    Unit *occ;

    assert_warning_return(in_play(unit),
			  "Attempted to wake an invalid unit.", );
    /* (should test that side is permitted to wake) */
    /* Wake the unit. */
    if (unit->plan) {
	unit->plan->asleep = FALSE;
	unit->plan->reserve = FALSE;
	update_unit_display(side, unit, TRUE);
    }
    /* Try waking any occupants. */
    if (forcewakeoccs) {
	for_all_occupants(unit, occ)
	  wake_unit(side, occ, forcewakeoccs);
    }
}

void
selectively_wake_unit(Side *side, Unit *unit, int wakeoccs, int forcewakeoccs)
{
    Unit *occ = NULL;

    assert_warning_return(in_play(unit),
			  "Attempted to wake an invalid unit.", );
    /* If on a transport, then probably transport already
       has awakened us, but just to cover all the cases... */
    if ((unit->transport)
	&& uu_can_recursively_wake(unit->transport->type,
				   unit->type))
      wake_unit(unit->side, unit, forcewakeoccs);
    /* Else not on a transport. */
    else if (!(unit->transport))
      wake_unit(unit->side, unit, forcewakeoccs);
    /* Now wake any occs, if so desired. */
    /* Note that if 'forcewakeoccs' is set, then 'wake_unit' will
       recursively wake _all_ occs, and so the following code would be
       somewhat redundant. */
    /* (Need valid test case for this, so that it can be tested and
	enabled.) */
#if (0)
    if (!forcewakeoccs && wakeoccs) {
	for_all_occupants(unit, occ) {
	    if ((unit->side == occ->side)
		&& uu_can_recursively_wake(unit->type, occ->type))
	      selectively_wake_unit(unit->side, occ, TRUE, FALSE);
	}
    }
#endif
}

void
react_to_seen_unit(Side *side, Unit *unit, int x, int y)
{
    int eu;
    Unit *eunit;
    UnitView *uview;
    Side *es;

    /* (The g_see_all() branch appears to be unused, because
	'calc_coverage' is only called when g_see_all() is not true.) */
    if (g_see_all() /* see real unit */) {
    	/* (should look at all of stack if can be mixed) */
	if ((eunit = unit_at(x, y)) != NULL) {
	    if (unit->plan && !allied_side(eunit->side, side)) {
		/* should do a more general alarm */
		selectively_wake_unit(unit->side, unit, TRUE, FALSE);
	    }
	}
    } else if (side->coverage != NULL) {
	uview = unit_view_at(side, x, y);
    	if (uview != NULL) {
    	    eu = uview->type;
    	    es = side_n(uview->siden);
    	    /* react only to certain utypes? */
	    if (unit->plan && !allied_side(es, side)) {
		/* should do a more general alarm */
		selectively_wake_unit(unit->side, unit, TRUE, FALSE);
	    }
    	}
    } else {
    	/* ??? */
    }
}

void
all_see_leave(Unit *unit, int x, int y, int inopen)
{
    Side *side;
    int always = u_see_always(unit->type), domore, update;
    UnitView *uview;

    for_all_sides(side) {
	if (side_sees_unit(side, unit)
		   && in_area(unit->x, unit->y)) {
	    see_cell(side, x, y);
	} else if (side_tracking_unit(side, unit)) {
	    see_cell(side, x, y);
	} else if (cover(side, x, y) > 0) {
	    see_cell(side, x, y);
	} else {
	    if (always && terrain_view(side, x, y) != UNSEEN) {
		see_cell(side, x, y);
		if (cover(side, x, y) > 0)
		  add_cover(side, x, y, -1);
		if (side->alt_coverage)
		  /* (should recalc alt coverage, since was 0) */;
	    }
	    /* Won't be called twice, because view coverage is 0 now. */
	    if (inopen) {
		see_cell(side, x, y);
	    }
	    /* special hack to flush images we *know* are garbage */
	    if (cover(side, x, y) < 1) {
		domore = TRUE;
		update = FALSE;
		while (domore) {
		    domore = FALSE;
		    for_all_view_stack_with_occs(side, x, y, uview) {
			if (side->id == uview->siden) {
			    remove_unit_view(side, uview);
			    domore = TRUE;
			    update = TRUE;
			    break;
			}
		    }
		}
		if (update)
		  update_cell_display(side, x, y, UPDATE_ALWAYS);
	    }
	}
    }
}

void
flush_stale_views(void)
{
    UnitView *uview, *prevview, *nextuv;
    int n = 0;

    if (viewlist == NULL)
      return;
    uview = viewlist;
    while (uview->id == -1) {
	nextuv = uview->vnext;
	flush_one_view(uview);
	uview = nextuv;
	if (uview == NULL)
	  break;
    }
    viewlist = uview;
    prevview = NULL;
    for_all_unit_views(uview) {
	if (uview->id == -1) {
	    nextuv = uview->vnext;
	    prevview->vnext = uview->vnext;
	    flush_one_view(uview);
	    uview = prevview;
	    ++n;
	} else {
	    prevview = uview;
	}
    }
    Dprintf("%d stale unit views flushed.\n", n);
    Dprintf("%d unit views left.\n", numunitviews);
}

// Lifecycle Management

Side *
create_side(void)
{
    int u, m;
    Side *newside;

    /* Too many sides. */
    if (numsides >=  g_sides_max()) {
	run_error("Cannot have more than %d sides total!", g_sides_max());
    }
    /* Prevent many crashes and much hair-pulling. */
    if (numutypes <= 0) {
	/* (Should probably be regarded as an init_error; however, this
	    segment of code may be executed "late".) */
	run_error("Unit types must be defined before defining sides!");
    }
    /* Just fill in the names if dealing with indepside. */
    if (nextsideid == 0) {
    	newside = indepside;
	newside->name = "Independents";
	newside->noun = "Independent";
	newside->pluralnoun = "Independents";
	newside->adjective = "independent";
	newside->colorscheme = "black,black,white";
	newside->emblemname = "none";
    } else {
	/* Else allocate a new side object and increase numsides. */
	newside = (Side *) xmalloc(sizeof(Side));
	/* numsides does not include indepside, thus increment it here. */
    	++numsides;
    }
    /* Fill in various side slots.  Only those with non-zero/non-NULL
       defaults need have anything done to them. */
    newside->id = nextsideid++;
    /* Always start sides IN the game. */
    newside->ingame = TRUE;
    /* Note that "everingame" is only set at the beginning of a turn. */
    /* Set up the relationships with other sides. */
    newside->knows_about = ALLSIDES; /* for now */
    newside->trusts = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    newside->trades = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    /* Set up per-unit-type slots. */
    newside->counts = (short *) xmalloc(numutypes * sizeof(short));
    newside->tech = (short *) xmalloc(numutypes * sizeof(short));
    newside->inittech = (short *) xmalloc(numutypes * sizeof(short));
    newside->numunits = (short *) xmalloc(numutypes * sizeof(short));
    newside->default_color = pick_default_colorname(newside);
    if (newside != indepside) {
	newside->emblemname = newside->default_color;
    }
    for_all_unit_types(u) {
	/* Start unit numbering at 1, not 0. */
	newside->counts[u] = 1;
    }
    newside->priority = -1;
    /* All sides should auto-finish by default. */
    newside->autofinish = TRUE;
    /* True by default, players should disable manually. */
    newside->willingtosave = TRUE;
    /* Put valid Lisp data into slots that need it. */
    newside->symbol = lispnil;
    newside->instructions = lispnil;
    newside->rawscores = lispnil;
    newside->aidata = lispnil;
    newside->uidata = lispnil;
    newside->possible_units = lispnil;
    newside->startx = newside->starty = -1;
    newside->init_center_x = newside->init_center_y = -1;
    newside->gaincounts =
	(short *) xmalloc(numutypes * num_gain_reasons * sizeof(short));
    newside->losscounts =
	(short *) xmalloc(numutypes * num_loss_reasons * sizeof(short));
    newside->atkstats = (long **) xmalloc(numutypes * sizeof(long *));
    newside->hitstats = (long **) xmalloc(numutypes * sizeof(long *));
    /* Necessary to enable AI control toggling of selected units! */
    newside->prefixarg = -1;
    if (numatypes > 0) {
	newside->advance = (short *) xmalloc(numatypes * sizeof(short));
	newside->canresearch = (short *) xmalloc(numatypes * sizeof(short));
	newside->research_precluded =
	    (short *)xmalloc(numatypes * sizeof(short));
	newside->research_topic = NOADVANCE;
	newside->autoresearch = FALSE;
	newside->research_goal = NONATYPE;
    }
    newside->canbuild = (short *) xmalloc(numutypes * sizeof(short));
    newside->cancarry = (short *) xmalloc(numutypes * sizeof(short));
    newside->candevelop = (short *) xmalloc(numutypes * sizeof(short));
    // Fill out side action limitations.
    // Note that indepside cannot act be default.
    newside->could_act_with = (short *) xmalloc(numutypes * sizeof(short));
    if (indepside != newside)
	memset(newside->could_act_with, TRUE, numutypes * sizeof(short));
    newside->could_construct = (short *) xmalloc(numutypes *sizeof(short));
    memset(newside->could_construct, TRUE, numutypes * sizeof(short));
    newside->could_develop = (short *) xmalloc(numutypes *sizeof(short));
    memset(newside->could_develop, TRUE, numutypes * sizeof(short));
    newside->could_research = (short *) xmalloc(numatypes *sizeof(short));
    memset(newside->could_research, TRUE, numatypes * sizeof(short));
    /* We can't update the canbuild vector yet, because its contents
       may depend on side class, which isn't set yet. */
    if (nummtypes > 0) {
	/* Set up the side's supply of materials. */
	newside->treasury = (long *) xmalloc(nummtypes * sizeof(long));
	for_all_material_types(m) {
	    if (side_has_treasury(newside, m)) {
		newside->treasury[m] = m_initial_treasury(m);
	    }
	}
	/* Make space for default conversions and copy them over. */
	newside->c_rates = (short *) xmalloc(nummtypes * sizeof(short));
	{
	    int m1, m2;

	    /* First initilaize m. */
	    m = NONMTYPE;
	    /* For now, assume only one m. */
	    for_all_material_types(m1) {
		for_all_material_types(m2) {
		    if (mm_conversion(m1, m2) > 0) {
			m = m1;
			break;
		    }
		}
	    }
	    if (m != NONMTYPE) {
		for_all_material_types(m2) {
		    newside->c_rates[m2] = mm_conversion_default(m, m2);
		}
	    }
	}
    }
    newside->controlled_by_id = -1;
    newside->playerid = -1;
    /* Link in at the end of the list of sides. */
    newside->next = NULL;
    /* Important to avoid the indepside->next = indepside loop! */
    if (newside != indepside) {
	lastside->next = newside;
	lastside = newside;
    }
    init_side_unithead(newside);
    /* numtotsides also includes indepside. */
    ++numtotsides;
    return newside;
}

// Game Setup

void
init_side_unithead(Side *side)
{
    if (side->unithead == NULL) {
	side->unithead = create_bare_unit(0);
	side->unithead->next = side->unithead;
	side->unithead->prev = side->unithead;
    }
}

static char *
pick_default_colorname(Side *side)
{
	int i;

	if (side == indepside) {
		return default_colornames[0];
	}
	/* We start at 1 since default_colors[0] is reserved for
	indepside, whether or not it is present in the game. */
	for (i = 1; i <= MAXSIDES +1; i++) {
		if (default_colornames_used[i] == FALSE) {
			default_colornames_used[i] = TRUE;
			return default_colornames[i];
		}
	}
	/* Should never happen, but humor the compiler. */
	return "black";
}

// GDL I/O

static
void
interp_atkstats_list(Side *side, Obj *lis)
{
    int u;
    Obj *rest, *head;

    for_all_list(lis, rest) {
    	head = car(rest);
    	if (consp(head) && symbolp(car(head))) {
	    u = utype_from_symbol(car(head));
	    if (!is_unit_type(u)) {
		init_warning("non unit type");
		continue;
	    }
	    if (side->atkstats[u] == NULL)
	      side->atkstats[u] = (long *) xmalloc(numutypes * sizeof(long));
	    interp_long_array(side->atkstats[u], cdr(head), numutypes);
	}
    }
}

static
void
interp_hitstats_list(Side *side, Obj *lis)
{
    int u;
    Obj *rest, *head;

    for_all_list(lis, rest) {
    	head = car(rest);
    	if (consp(head) && symbolp(car(head))) {
	    u = utype_from_symbol(car(head));
	    if (!is_unit_type(u)) {
		init_warning("non unit type");
		continue;
	    }
	    if (side->hitstats[u] == NULL)
	      side->hitstats[u] = (long *) xmalloc(numutypes * sizeof(long));
	    interp_long_array(side->hitstats[u], cdr(head), numutypes);
	}
    }
}

void
fill_in_side(Side *side, Obj *props, int userdata)
{
    int numval, isnumber, permission, u;
    char *propname, *strval = NULL, *checkuniq;
    Obj *bdg, *rest, *propval;

    /* Use this macro with every slot that the user may not arbitrarily
       change when the game starts up. */
#define CHECK_PERMISSION  \
	    if (userdata) {  \
		permission = FALSE;  \
		break;  \
	    }

    for (; props != lispnil; props = cdr(props)) {
	permission = TRUE;
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (symbolp(propval))
	  propval = eval(propval);
	numval = eval_number(propval, &isnumber);
	if (stringp(propval))
	  strval = c_string(propval);
	rest = cdr(bdg);
	checkuniq = NULL;
	switch (keyword_code(propname)) {
	  case K_NAME:
	    side->name = strval;
	    checkuniq = "name";
	    break;
	  case K_LONG_NAME:
	    side->longname = strval;
	    checkuniq = "long name";
	    break;
	  case K_SHORT_NAME:
	    side->shortname = strval;
	    checkuniq = "short name";
	    break;
	  case K_NOUN:
	    side->noun = strval;
	    checkuniq = "noun";
	    break;
	  case K_PLURAL_NOUN:
	    side->pluralnoun = strval;
	    checkuniq = "plural noun";
	    break;
	  case K_ADJECTIVE:
	    side->adjective = strval;
	    checkuniq = "adjective";
	    break;
	  case K_COLOR:
	    side->colorscheme = strval;
	    checkuniq = "color";
	    break;
	  case K_EMBLEM_NAME:
	    side->emblemname = strval;
	    checkuniq = "emblem";
	    break;
	  case K_UNIT_NAMERS:
	    // Allocate space if not already done so.
	    if (side->unitnamers == NULL)
	      side->unitnamers = (char **) xmalloc(numutypes * sizeof(char *));
	    merge_unit_namers(side, rest);
	    break;
	  case K_FEATURE_NAMERS:
	    // An empty list means to clear the side's list,
	    //	otherwise we "merge" by gluing onto the front.
	    if (rest == lispnil)
	      side->featurenamers = lispnil;
	    else
	      side->featurenamers = append_two_lists(rest, side->featurenamers);
	    break;
	  case K_NAMES_LOCKED:
	    side->nameslocked = numval;
	    break;
	  case K_UNITS:
	    side->possible_units = propval;
	    break;
	  case K_CLASS:
	    CHECK_PERMISSION;
	    side->sideclass = strval;
	    break;
	  case K_SELF_UNIT:
	    CHECK_PERMISSION;
	    side->self_unit_id = numval;
	    break;
	  case K_CONTROLLED_BY:
	    CHECK_PERMISSION;
	    side->controlled_by_id = numval;
	    break;
	  case K_TRUSTS:
	    interp_side_value_list(side->trusts, rest);
	    break;
	  case K_TRADES:
	    interp_side_value_list(side->trades, rest);
	    break;
	  case K_START_WITH:
	    CHECK_PERMISSION;
	    if (side->startwith == NULL)
	      side->startwith = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->startwith, rest);
	    break;
	  case K_NEXT_NUMBERS:
	    CHECK_PERMISSION;
	    if (side->counts == NULL)
	      side->counts = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->counts, rest);
	    break;
	  case K_TECH:
	    CHECK_PERMISSION;
	    if (side->tech == NULL)
	      side->tech = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->tech, rest);
	    break;
	  case K_INIT_TECH:
	    CHECK_PERMISSION;
	    if (side->inittech == NULL)
	      side->inittech = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->inittech, rest);
	    break;
	  case K_ACTION_PRIORITIES:
	    CHECK_PERMISSION;
	    if (side->action_priorities == NULL) {
		side->action_priorities = (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->action_priorities[u] = -1;
	    }
	    interp_utype_value_list(side->action_priorities, rest);
	    break;
	  case K_ALREADY_SEEN:
	    CHECK_PERMISSION;
	    if (side->already_seen == NULL) {
		side->already_seen = (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->already_seen[u] = -1;
	    }
	    interp_utype_value_list(side->already_seen, rest);
	    break;
	  case K_ALREADY_SEEN_INDEPENDENT:
	    CHECK_PERMISSION;
	    if (side->already_seen_indep == NULL) {
		side->already_seen_indep =
		    (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->already_seen_indep[u] = -1;
	    }
	    interp_utype_value_list(side->already_seen_indep, rest);
	    break;
	  case K_DEFAULT_DOCTRINE:
	    read_default_doctrine(side, rest);
	    break;
	  case K_DOCTRINES:
	    read_utype_doctrine(side, rest);
	    break;
	  case K_DOCTRINES_LOCKED:
	    side->doctrines_locked = numval;
	    break;
	  case K_TERRAIN_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_terrain_view);
	    break;
	  case K_TERRAIN_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_terrain_view_date);
	    break;
	  case K_AUX_TERRAIN_VIEW:
	    CHECK_PERMISSION;
	    read_aux_terrain_view_layer(side, rest, fn_set_aux_terrain_view);
	    break;
	  case K_AUX_TERRAIN_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_aux_terrain_view_layer(
		side, rest, fn_set_aux_terrain_view_date);
	    break;
	  case K_UNIT_VIEWS:
	    CHECK_PERMISSION;
	    interp_unit_views(side, rest);
	    side->unit_view_restored = TRUE;
	    break;
	  case K_MATERIAL_VIEW:
	    CHECK_PERMISSION;
	    read_material_view_layer(side, rest, fn_set_material_view);
	    break;
	  case K_MATERIAL_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_material_view_layer(side, rest, fn_set_material_view_date);
	    break;
	  case K_TEMPERATURE_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_temp_view);
	    break;
	  case K_TEMPERATURE_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_temp_view_date);
	    break;
	  case K_CLOUD_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_view);
	    break;
	  case K_CLOUD_BOTTOM_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_bottom_view);
	    break;
	  case K_CLOUD_HEIGHT_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_height_view);
	    break;
	  case K_CLOUD_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_view_date);
	    break;
	  case K_WIND_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_wind_view);
	    break;
	  case K_WIND_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_wind_view_date);
	    break;
	  case K_ACTIVE:
	    CHECK_PERMISSION;
	    side->ingame = numval;
	    break;
	  case K_EVER_ACTIVE:
	    CHECK_PERMISSION;
	    side->everingame = numval;
	    break;
	  case K_PRIORITY:
	    CHECK_PERMISSION;
	    side->priority = numval;
	    break;
	  case K_STATUS:
	    CHECK_PERMISSION;
	    side->status = numval;
	    break;
	  case K_SCORES:
	    CHECK_PERMISSION;
	    /* The data in this slot will be interpreted later,
	       after scorekeepers exist. */
	    side->rawscores = rest;
	    break;
	  case K_WILLING_TO_DRAW:
	    side->willingtodraw = numval;
	    break;
	  case K_FINISHED_TURN:
	    side->finishedturn = numval;
	    break;
	  case K_TURN_TIME_USED:
	    CHECK_PERMISSION;
	    side->turntimeused = numval;
	    break;
	  case K_TOTAL_TIME_USED:
	    CHECK_PERMISSION;
	    side->totaltimeused = numval;
	    break;
	  case K_TIMEOUTS:
	    CHECK_PERMISSION;
	    side->timeouts = numval;
	    break;
	  case K_TIMEOUTS_USED:
	    CHECK_PERMISSION;
	    side->timeoutsused = numval;
	    break;
	  case K_ADVANTAGE:
	    CHECK_PERMISSION;
	    side->advantage = numval;
	    break;
	  case K_ADVANTAGE_MIN:
	    CHECK_PERMISSION;
	    side->minadvantage = numval;
	    break;
	  case K_ADVANTAGE_MAX:
	    CHECK_PERMISSION;
	    side->maxadvantage = numval;
	    break;
	  case K_INITIAL_CENTER_AT:
	    /* (should support unit names, etc here also) */
	    side->init_center_x = numval;
	    side->init_center_y = c_number(caddr(bdg));
	    break;
	  case K_PLAYER:
	    side->playerid = numval;
	    break;
	  case K_INSTRUCTIONS:
	    /* The instructions are a list of strings. */
	    side->instructions = propval;
	    break;
	  case K_GAIN_COUNTS:
	    CHECK_PERMISSION;
	    interp_short_array(
		side->gaincounts, rest, numutypes * num_gain_reasons);
	    break;
	  case K_LOSS_COUNTS:
	    CHECK_PERMISSION;
	    interp_short_array(
		side->losscounts, rest, numutypes * num_loss_reasons);
	    break;
	  case K_ATTACK_STATS:
	    CHECK_PERMISSION;
	    interp_atkstats_list(side, rest);
	    break;
	  case K_HIT_STATS:
	    CHECK_PERMISSION;
	    interp_hitstats_list(side, rest);
	    break;
	  case K_AI_DATA:
	    /* The data in this slot will be interpreted when the AI is
	       ready to look at it. */
	    side->aidata = rest;
	    break;
	  case K_UI_DATA:
	    /* The data in this slot will be interpreted when the interface
	       is ready to look at it. */
	    side->uidata = rest;
	    break;
	  case K_STANDING_ORDER:
	    interp_standing_order(side, rest);
	    break;
	  case K_CURRENT_ADVANCE:
	    side->research_topic = numval;
	    break;
	  case K_ADVANCE_GOAL:
	    side->research_goal = numval;
	    break;
	  case K_ADVANCES_DONE:
	    interp_atype_value_list(side->advance, rest);
	    break;
	  case K_TREASURY:
	    interp_treasury_list(side->treasury, rest);
	    break;
	  case K_COULD_ACT_WITH:
	    interp_utype_value_list(side->could_act_with, rest);
	    break;
	  case K_COULD_CONSTRUCT:
	    interp_utype_value_list(side->could_construct, rest);
	    break;
	  case K_COULD_DEVELOP:
	    interp_utype_value_list(side->could_develop, rest);
	    break;
	  case K_COULD_RESEARCH:
	    interp_atype_value_list(side->could_research, rest);
	    break;
	  default:
	    unknown_property("side", side_desig(side), propname);
	}
	if (!permission) {
	    read_warning("No permission to set property `%s'", propname);
	}
	if (checkuniq != NULL) {
	    check_name_uniqueness(side, strval, checkuniq);
	}
    }
    /* Calculate the plural form of the noun for units if it was not
       explicitly defined. */
    if (side->noun != NULL && side->pluralnoun == NULL) {
	side->pluralnoun = copy_string(plural_form(side->noun));
    }
}

void
interp_side(Obj *form, Side *side)
{
    int id = -1;
    Obj *ident = lispnil, *sym = lispnil, *props = cdr(form);

    // Get optional side identifier, if there is one.
    if (!listp(car(props))) {
	ident = car(props);
	props = cdr(props);
    }
    // Get side from numeric ID.
    if (numberp(ident)) {
	id = c_number(ident);
	side = side_n(id);
    }
    // Maybe get side from symbolic ID.
    else if (symbolp(ident)) {
	if (boundp(ident) && numberp(symbol_value(ident))) {
	    id = c_number(symbol_value(ident));
	    side = side_n(id);
	} else {
	    sym = ident;
	}
    }
    // Maybe get side from string ID.
    else if (stringp(ident)) {
	side = find_side_by_name(c_string(ident));
    }
    // Else, we want to create a new side.
    else;
    // Additionally get the side symbol, if we haven't already.
    if (symbolp(car(props)) && (lispnil == sym)) {
	sym = car(props);
	props = cdr(props);
    }
#if (0) // Do we want to do this or not?
    // Require the 'add' keyword, if we are modifying an existing side.
    if (side && nextsideid) {
	if (!match_keyword(car(props), K_ADD))
	    syntax_error(car(props), "attempting to redefine an existing side");
    }
#endif
    // If indepside (which always exists) was read from the form,
    //	make sure it is completed if necessary.
    if ((side == indepside) && !nextsideid) {
    	// Does not actually create indepside, but fills in its slots.
    	side = create_side();
    }
    // Else if no side was looked up, then we are dealing with new side.
    else if (!side) {
	// If the side that was read from the form does not exist yet
        // we have to create it.
    	// Make sure indepside is completed first. This has to be done
    	// now and not earlier since we cannot allocate all its slots
    	// until we know numutypes etc. which we just got from reading
    	// this form. */
    	if (nextsideid == 0) {
    	    /* Does not actually create indepside, but fills in its slots. */
	    create_side();
    	}
    	// Then create (and fill in) the new side.
	side = create_side();
	// Replace with the read-in id if it was given.
	if (id >= 0)
	  side->id = id;
    }
    // Bind optional symbol to side.
    if (sym != lispnil) {
	// Record the symbol as going with this side.
	side->symbol = sym;
	// Also let it eval to the side's id, warning about conflicts.
	if (boundp(sym))
	  read_warning("Symbol `%s' has been bound already, overwriting",
		       c_string(sym));
	setq(sym, new_number(side->id));
    }
    // Apply the current side defaults first.
    fill_in_side(side, side_defaults, FALSE);
    // Now fill in from the explicitly specified properties.
    fill_in_side(side, props, FALSE);
    Dprintf("  Got side %s\n", side_desig(side));
}

void
interp_side_value_list(short *arr, Obj *lis)
{
    int s = 0;
    Obj *rest, *head;

    if (arr == NULL)
      run_error("null array for side value list?");
    for_all_list(lis, rest) {
    	head = car(rest);
	if (numberp(head)) {
	    if (s > g_sides_max())
	      break;
	    arr[s++] = c_number(head);
	} else if (symbolp(head)) {
	    int s2 = c_number(eval(head));

	    if (between(1, s2, g_sides_max()))
	      arr[s2] = TRUE;
	    else
	      read_warning("bad side spec");
	} else if (consp(head)) {
	    Obj *sidespec = car(head);
	    int s2, val2 = c_number(cadr(head));

	    if (numberp(sidespec) || symbolp(sidespec)) {
		s2 = c_number(eval(sidespec));
		if (between(1, s2, g_sides_max()))
		  arr[s2] = val2;
		else
		  read_warning("bad side spec");
	    } else if (consp(sidespec)) {
	    	read_warning("not implemented");
	    } else {
	    	read_warning("not implemented");
	    }
	} else {
	    read_warning("not implemented");
	}
    }
}

void
ai_save_state(Side *side)
{
    Obj *(*fn)(Side *side), *state = lispnil;

    fn = all_ai_ops[side->aitype].to_save_state;
    if (fn)
      state = (*fn)(side);
    /* Don't bother if there's no AI state to mess with. */
    if (side->aidata == lispnil && state == lispnil)
      return;
    side->aidata =
        replace_at_key(side->aidata, side->player->aitypename, state);
}

void
write_side_view(Side *side, int compress)
{
    int t, m, x, y;
    UnitView *uview;

    tmpside = side;
    tmpcompress = compress;
    if (side->terrview)
      write_one_side_view_layer(K_TERRAIN_VIEW, fn_terrain_view);
    if (!g_see_terrain_always()) {
	if (side->terrview)
	  write_one_side_view_layer(K_TERRAIN_VIEW_DATES,
				    fn_terrain_view_date);
	if (numcelltypes < numttypes) {
	    for_all_terrain_types(t) {
		if (!t_is_cell(t)) {
		    tmpttype = t;
		    if (side->auxterrview[t])
		      write_one_side_view_layer(K_AUX_TERRAIN_VIEW,
						fn_aux_terrain_view);
	/* Caused crashes since the code that allocates this layer in init_view
	is currently commented out. */
#if 0
		    if (side->auxterrviewdate[t])
		      write_one_side_view_layer(K_AUX_TERRAIN_VIEW_DATES,
						fn_aux_terrain_view_date);
#endif
		}
	    }
        }
    }
    if (side->materialview) {
	for_all_material_types(m) {
	    if (1) {
		tmpmtype = m;
		if (side->materialview[m])
		  write_one_side_view_layer(K_MATERIAL_VIEW, fn_material_view);
		if (side->materialviewdate[m])
		  write_one_side_view_layer(K_MATERIAL_VIEW_DATES,
					    fn_material_view_date);
	    }
	}
    }
    /* Write out unit view objects. */
    if (!side->see_all && side->unit_views != NULL) {
	newline_form();
	start_form(key(K_SIDE));
	add_num_to_form(side->id);
	space_form();
	start_form(key(K_UNIT_VIEWS));
	newline_form();
	for_all_cells(x, y) {
	    for_all_view_stack_with_occs(side, x, y, uview) {
		/* (should filter out side's own units?) */
		space_form();
		space_form();
		start_form(shortest_escaped_name(uview->type));
		add_num_to_form(uview->siden);
		add_to_form(uview->name);
		add_num_to_form(uview->size);
		add_num_to_form(uview->x);
		add_num_to_form(uview->y);
		add_num_to_form(uview->complete);
		add_num_to_form(uview->date);
		add_num_to_form(uview->id);
		add_to_form(uview->image_name);
		end_form();
		newline_form();
	    }
	}
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (!g_see_weather_always()) {
	if (side->tempview) {
	    write_one_side_view_layer(K_TEMPERATURE_VIEW, fn_temp_view);
	    write_one_side_view_layer(K_TEMPERATURE_VIEW_DATES,
				      fn_temp_view_date);
	}
	if (side->cloudview) {
	    write_one_side_view_layer(K_CLOUD_VIEW, fn_cloud_view);
	    write_one_side_view_layer(K_CLOUD_BOTTOM_VIEW,
				      fn_cloud_bottom_view);
	    write_one_side_view_layer(K_CLOUD_HEIGHT_VIEW,
				      fn_cloud_height_view);
	    write_one_side_view_layer(K_CLOUD_VIEW_DATES, fn_cloud_view_date);
	}
	if (side->windview) {
	    write_one_side_view_layer(K_WIND_VIEW, fn_wind_view);
	    write_one_side_view_layer(K_WIND_VIEW_DATES, fn_wind_view_date);
	}
    }
}

void
write_side_properties(Side *side)
{
    int i, u, u2, anyudoctrines;

    write_str_prop(key(K_NAME), side->name, "", FALSE, TRUE);
    write_str_prop(key(K_LONG_NAME), side->longname, "", FALSE, TRUE);
    write_str_prop(key(K_SHORT_NAME), side->shortname, "", FALSE, TRUE);
    write_str_prop(key(K_NOUN), side->noun, "", FALSE, TRUE);
    write_str_prop(key(K_PLURAL_NOUN), side->pluralnoun, "", FALSE, TRUE);
    write_str_prop(key(K_ADJECTIVE), side->adjective, "", FALSE, TRUE);
    write_str_prop(key(K_COLOR), side->colorscheme, "", FALSE, TRUE);
    write_str_prop(key(K_EMBLEM_NAME), side->emblemname, "", FALSE, TRUE);
    write_utype_string_list(key(K_UNIT_NAMERS), side->unitnamers, "", TRUE);
    write_lisp_prop(
	key(K_UNITS), side->possible_units, lispnil, FALSE, FALSE, TRUE);
    write_str_prop(key(K_CLASS), side->sideclass, "", FALSE, TRUE);
    write_bool_prop(key(K_ACTIVE), side->ingame, 1, FALSE, TRUE);
    /* side->everingame is only interesting if it differs from ingame,
       so use the value of ingame as the default. */
    write_bool_prop(key(K_EVER_ACTIVE), side->everingame, side->ingame,
		    FALSE, TRUE);
    write_num_prop(key(K_PRIORITY), side->priority, -1, FALSE, TRUE);
    write_num_prop(key(K_STATUS), side->status, 0, FALSE, TRUE);
    if (!debugging_state_sync)
      write_num_prop(key(K_TURN_TIME_USED), side->turntimeused, 0, FALSE, TRUE);
    if (!debugging_state_sync)
      write_num_prop(
	key(K_TOTAL_TIME_USED), side->totaltimeused, 0, FALSE, TRUE);
    write_num_prop(key(K_TIMEOUTS), side->timeouts, 0, FALSE, TRUE);
    write_num_prop(key(K_TIMEOUTS_USED), side->timeoutsused, 0, FALSE, TRUE);
    write_num_prop(key(K_FINISHED_TURN), side->finishedturn, 0, FALSE, TRUE);
    write_num_prop(key(K_WILLING_TO_DRAW), side->willingtodraw, 0, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE), side->advantage, 1, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE_MIN), side->minadvantage, 1, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE_MAX), side->maxadvantage, 1, FALSE, TRUE);
    write_num_prop(
	key(K_CONTROLLED_BY),
	(side->controlled_by ? side_number(side->controlled_by) : -1),
	-1, FALSE, TRUE);
    write_num_prop(
	key(K_SELF_UNIT),
	(side->self_unit ? side->self_unit->id : 0), 0, FALSE, TRUE);
    write_num_prop(
	key(K_PLAYER), (side->player ? side->player->id : -1), -1, FALSE, TRUE);
    write_num_prop(key(K_DEFAULT_DOCTRINE),
    		   (side->default_doctrine ? side->default_doctrine->id : 0),
		   0, FALSE, TRUE);
    write_num_prop(key(K_CURRENT_ADVANCE), side->research_topic, NOADVANCE,
		   FALSE, TRUE);
    write_num_prop(key(K_ADVANCE_GOAL), side->research_goal, NONATYPE, FALSE,
		   TRUE);
    write_atype_value_list(key(K_ADVANCES_DONE), side->advance, 0, TRUE);
    write_treasury_list(key(K_TREASURY), side->treasury, 0, TRUE);
    anyudoctrines = FALSE;
    if (side->udoctrine != NULL) {
	for_all_unit_types(u) {
	    if (side->udoctrine[u] != side->default_doctrine) {
		anyudoctrines = TRUE;
		break;
	    }
	}
    }
    if (anyudoctrines) {
	space_form();
	start_form(key(K_DOCTRINES));
	for_all_unit_types(u) {
	    if (side->udoctrine[u] != side->default_doctrine) {
		space_form();
		start_form(shortest_escaped_name(u));
		if (side->udoctrine[u]->name)
		  add_to_form(escaped_symbol(side->udoctrine[u]->name));
		else
		  add_num_to_form(side->udoctrine[u]->id);
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    write_side_value_list(key(K_TRUSTS), side->trusts, FALSE, TRUE);
    write_side_value_list(key(K_TRADES), side->trades, FALSE, TRUE);
    write_utype_value_list(key(K_START_WITH), side->startwith, 0, TRUE);
    write_utype_value_list(key(K_NEXT_NUMBERS), side->counts, 0, TRUE);
    write_utype_value_list(key(K_TECH), side->tech, 0, TRUE);
    write_utype_value_list(key(K_INIT_TECH), side->inittech, 0, TRUE);
    write_utype_value_list(key(K_ALREADY_SEEN), side->already_seen, -1, TRUE);
    write_utype_value_list(
	key(K_ALREADY_SEEN_INDEPENDENT), side->already_seen_indep, -1, TRUE);
    write_standing_orders(side);
    if (side->scores) {
	space_form();
	start_form(key(K_SCORES));
	for (i = 0; i < numscores; ++i) {
	    add_num_to_form(side->scores[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    write_lisp_prop(key(K_INSTRUCTIONS), side->instructions,
		    lispnil, FALSE, FALSE, TRUE);
    /* Write out statistics. */
    if (side->gaincounts != NULL) {
	space_form();
	start_form(key(K_GAIN_COUNTS));
	for (i = 0; i < numutypes * num_gain_reasons; ++i) {
	    add_num_to_form(side->gaincounts[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->losscounts != NULL) {
	space_form();
	start_form(key(K_LOSS_COUNTS));
	for (i = 0; i < numutypes * num_loss_reasons; ++i) {
	    add_num_to_form(side->losscounts[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->atkstats != NULL) {
	space_form();
	start_form(key(K_ATTACK_STATS));
	for_all_unit_types(u) {
	    if (side->atkstats[u] != NULL) {
		newline_form();
		space_form();
		space_form();
		space_form();
		start_form(shortest_escaped_name(u));
		for_all_unit_types(u2) {
		    add_num_to_form(side_atkstats(side, u, u2));
		}
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->hitstats != NULL) {
	space_form();
	start_form(key(K_HIT_STATS));
	for_all_unit_types(u) {
	    if (side->hitstats[u] != NULL) {
		newline_form();
		space_form();
		space_form();
		space_form();
		start_form(shortest_escaped_name(u));
		for_all_unit_types(u2) {
		    add_num_to_form(side_hitstats(side, u, u2));
		}
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->could_act_with)
	write_utype_value_list(
	    key(K_COULD_ACT_WITH), side->could_act_with, FALSE, TRUE);
    if (side->could_construct)
	write_utype_value_list(
	    key(K_COULD_CONSTRUCT), side->could_construct, FALSE, TRUE);
    if (side->could_develop)
	write_utype_value_list(
	    key(K_COULD_DEVELOP), side->could_develop, FALSE, TRUE);
    if (side->could_research)
	write_atype_value_list(
	    key(K_COULD_RESEARCH), side->could_research, FALSE, TRUE);
    /* Have the AI paste its useful state into distinct element of
       side->aidata. */
    if (side_has_local_ai(side)) {
	ai_save_state(side);
    }
    write_lisp_prop(key(K_AI_DATA), side->aidata, lispnil, FALSE, TRUE, TRUE);
    write_lisp_prop(key(K_UI_DATA), side->uidata, lispnil, FALSE, TRUE, TRUE);
}

void
write_sides(Module *module)
{
    Side *side;

    newline_form();
    add_to_form(";");
    add_num_to_form(numtotsides);
    add_to_form("sides including the independent 0 side.");
    newline_form();
    Dprintf("Will try to write %d sides ...\n", numtotsides);
    for_all_real_sides(side) {
	start_form(key(K_SIDE));
	add_num_to_form(side->id);
	if (symbolp(side->symbol))
	  add_to_form(escaped_symbol(c_string(side->symbol)));
	newline_form();
	space_form();
	write_side_properties(side);
	space_form();
	end_form();
	newline_form();
	if (module->def_all || module->def_side_views)
	  write_side_view(side, module->compress_layers);
	Dprintf("  Wrote side %s\n", side_desig(side));
    }
    /* Even though the indepside is another side, the properties of
       independent units are a separate type of form. */
    start_form(key(K_INDEPENDENT_UNITS));
    newline_form();
    write_side_properties(indepside);
    space_form();
    end_form();
    newline_form();
    Dprintf("  Wrote independent unit properties\n");
    Dprintf("... Done writing sides\n");
}

void
write_side_value_list(char *name, short *arr, int dflt, int addnewline)
{
    int s, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for (s = 0; s <= numsides; ++s) {
	if (arr[s] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for (s = 0; s <= numsides; ++s) {
	add_num_to_form(arr[s]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

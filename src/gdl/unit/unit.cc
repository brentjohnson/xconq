// xConq
// Lifecycle management and ser/deser of units.

// $Id: unit.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1986-1989   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of units.
    \ingroup grp_gdl
*/

#include "gdl/ui.h"
#include "gdl/gvars.h"
#include "gdl/tables.h"
#include "gdl/desc.h"
#include "gdl/media/imf.h"
#include "gdl/unit/unit.h"
#include "gdl/side/side.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Constant Macros

//! Number of units to allocate in a block of units.
/*! This is not a limit, just sets initial allocation of unit objects
    and how many additional to get if more are needed.  This can be
    tweaked for more frequent but smaller allocation, or less frequent
    but larger and possibly space-wasting allocation.
*/
#ifndef INITMAXUNITS
#define INITMAXUNITS 200
#endif

//! Number of unit description buffers to rotate.
/*! We use several and rotate among them; this is so multiple calls in
    a single printf will work as expected.  Not elegant, but simple and
    sufficient.

    \todo Do something better than rotate a small handful of buffers
	  and pretend that we are reentrant.
*/
#define NUMSHORTBUFS 3

// Global Variables

int numunits;
int numliveunits;

// Global Variables: Images

int *numuimages = NULL;
ImageFamily **uimages = NULL;

// Global Variables: Advanced Unit Defaults

short default_size = 1;
short default_usedcells = 0;
short default_maxcells = 1;
long default_population = 1;

// Local Variables

//! The list of available units.
static Unit *freeunits;

//! Total numbers of live units, by type.
static int *numlivebytype;

//! The next number to use for a unit id.
static int nextid = 1;

//! Default z-axis position for unit.
static short default_unit_z = -1;
//! Default transport unit ID for unit.
static short default_transport_id = -1;
//! Default side ID for unit.
static short default_unit_side_number = -1;
//! Default original side ID for unit.
static short default_unit_origside_number = -1;
//! Default construction points for unit.
static short default_unit_cp = -1;
//! Default hitpoints for unit.
static short default_unit_hp = -1;
//! Default combat experience for unit.
static short default_unit_cxp = -1;
//! Default tooling array for unit.
static short *default_tooling;
//! Advanced unit: default advance being researched.
static short default_curadvance = -1;

//! Default unit x-coordinate offset.
static int uxoffset = 0;
//! Default unit y-coordinate offset.
static int uyoffset = 0;

// Local Variables: Buffers

//! Current unit designator buffer number.
static int curshortbuf;
//! Array of unit designator buffers.
static char *shortbufs[NUMSHORTBUFS] = { NULL, NULL, NULL };

//! Unit handle buffer.
static char *unitbuf = NULL;

//! Array of types that occupy cell.
static int *toc_numtypes;

// Local Function Declarations: Queries

//! Get GDL spec of unit by name.
static Obj *find_unit_spec_by_name(char *name);
//! Get GDL spec of unit by number.
static Obj *find_unit_spec_by_number(int num);

// Local Function Declarations: Sorting

static int compare_units(Unit *unit1, Unit *unit2);

// Local Function Declarations: Position Manipulation

//! Leave cell, and do for any occupants as well.
/*!
    When leaving, remove view coverage, record old position, and then
    trash the old coordinates just in case.  Catches many bugs.  Do
    this for all the occupants as well.
*/
static void leave_cell_aux(Unit *unit, Unit *oldtransport);
//! Remove unit from list of units in cell.
static void remove_unit_from_ustack(Unit *unit);

// Local Function Declarations: Lifecycle Management

//! Grab a block of unit objects to work with.
static void allocate_unit_block(void);

//! Insert unit into list of units.
static void insert_unit(Unit *unithead, Unit *unit);

//! Remove unit from list of units.
static void delete_unit(Unit *unit);

//! Flush dead unit.
/*!
    Keep it clean - hit all links to other places.  Some might not be
    strictly necessary, but this is not an area to take chances with.
*/
static void flush_one_unit(Unit *unit);

// Queries

char *
short_unit_handle(Unit *unit)
{
    int u;

    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    u = unit->type;
    /* Use the name alone if the unit is named, or else use optional
       ordinal and the shortest type name available. */
    if (!empty_string(unit->name)) {
	strcpy(unitbuf, unit->name);
    } else {
	unitbuf[0] = '\0';
	if (unit->number > 0) {
	    sprintf(unitbuf, "%d%s ",
		    unit->number, ordinal_suffix(unit->number));
	}
	if (!empty_string(u_short_name(u)))
	  strcat(unitbuf, u_short_name(u));
	else
	  strcat(unitbuf, u_type_name(u));
    }
    return unitbuf;
}

char *
medium_long_unit_handle(Unit *unit)
{
    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    strcpy(unitbuf, side_adjective(unit->side));
    strcat(unitbuf, " ");
    /* If this unit is a self unit, say so. */
    if (unit->side != NULL && unit == unit->side->self_unit) {
	if (!mobile(unit->type) || u_advanced(unit->type)) {
		strcat(unitbuf, "capital ");
	} else {
		strcat(unitbuf, "leader ");
	}
	/* If the unit has a name, write its name. */
	if (!empty_string(unit->name)) {
		strcat(unitbuf, unit->name);
		/* If the unit has a number, write it followed by the type. */
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number),
			u_type_name(unit->type));
		/* Else just write the unit type. */
	} else {
		strcat(unitbuf, u_type_name(unit->type));
	}
    } else {
	/* If the unit has a name, write its type followed by the name. */
	if (!empty_string(unit->name)) {
		strcat(unitbuf, u_type_name(unit->type));
		strcat(unitbuf, " ");
		strcat(unitbuf, unit->name);
		/* If the unit has a number, write it followed by the type. */
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number),
			u_type_name(unit->type));
		/* Else just write the unit type. */
	} else {
		strcat(unitbuf, u_type_name(unit->type));
	}
    }
    return unitbuf;
}

char *
unit_desig(Unit *unit)
{
    int i;
    char *shortbuf;

    /* Allocate if not yet done so. */
    for (i = 0; i < NUMSHORTBUFS; ++i) {
	if (shortbufs[i] == NULL)
	  shortbufs[i] = (char *)xmalloc(BUFSIZE);
    }
    /* Note that we test here, so that unit_desig(NULL) can be used
       to allocate any space that this routine might need later. */
    if (unit == NULL)
      return "no unit";
    shortbuf = shortbufs[curshortbuf];
    curshortbuf = (curshortbuf + 1) % NUMSHORTBUFS;
    if (unit->id == -1) {
	sprintf(shortbuf, "s%d head", side_number(unit->side));
	return shortbuf;
    } else if (is_unit_type(unit->type)) {
	sprintf(shortbuf, "s%d %s %d (%d,%d",
		side_number(unit->side), shortest_unique_name(unit->type),
		unit->id, unit->x, unit->y);
	if (unit->z != 0)
	  tprintf(shortbuf, ",%d", unit->z);
	if (unit->transport)
	  tprintf(shortbuf, ",in%d", unit->transport->id);
	strcat(shortbuf, ")");  /* close out the unit location */
	return shortbuf;
    } else {
	return "!garbage unit!";
    }
}

char *
apparent_unit_handle(Side *side, Unit *unit, Side *side2)
{
    char *utypename, *fmtstr, smallbuf[40], sidebuf[100];
    Side *side3;
    Obj *frest, *fmt1;

    /* This should be impossible, be really obvious if it happens. */
    if (side2 == NULL)
      return "null side2 in apparent_unit_handle?";
    /* Make sure our working space exists. */
    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    unitbuf[0] = '\0';
    /* Decide how to identify the side.  If the unit's original side
       is not its current side, list both of them. */
    side3 = NULL;
    if (unit->origside != NULL && side2 != unit->origside)
      side3 = unit->origside;
    sidebuf[0] = '\0';
    if (side2 == side) {
	strcat(sidebuf, "your");
    } else {
	/* If the side adjective is a genitive (ends in 's, s' or z')
	   we should skip the definite article. */
	int len = strlen(side_adjective(side2));
	char *end = side_adjective(side2) + len - 2;

	if (strcmp(end, "'s") != 0
	    && strcmp(end, "s'") != 0
	    && strcmp(end, "z'") != 0)
	  strcat(sidebuf, "the ");
	strcat(sidebuf, side_adjective(side2));
    }

	/* Generates a lot of extra text of limited interest to the player. */
#if 0
    if (side3 != NULL) {
	if (side3 == side) {
	    strcat(sidebuf, " (formerly your)");
	} else if (side3 == indepside) {
	    /* Don't add anything for captured independents.  While
	       technically there's no reason not to do this, in
	       practice a side will often have many captured indeps,
	       and this keeps the text from getting too cluttered. */
	} else {
	    strcat(sidebuf, " (formerly ");
	    strcat(sidebuf, side_adjective(side3));
	    strcat(sidebuf, ")");
	}
    }
#endif

    /* Now add the unit's unique description. */
    utypename = u_type_name(unit->type);
    /* If we have special formatting info, interpret it. */
    if (u_desc_format(unit->type) != lispnil) {
	for_all_list(u_desc_format(unit->type), frest) {
	    fmt1 = car(frest);
	    if (stringp(fmt1)) {
		/* Append strings verbatim. */
		strcat(unitbuf, c_string(fmt1));
	    } else if (symbolp(fmt1)) {
		/* Symbols indicate the types of data to format and
                   output. */
		fmtstr = c_string(fmt1);
		if (strcmp(fmtstr, "name") == 0) {
		    strcat(unitbuf, (unit->name ? unit->name : "anon"));
		} else if (strcmp(fmtstr, "position") == 0) {
		    sprintf(smallbuf, "%d,%d", unit->x, unit->y);
		    strcat(unitbuf, smallbuf);
		} else if (strcmp(fmtstr, "side") == 0) {
		    strcat(unitbuf, sidebuf);
		} else if (strcmp(fmtstr, "type") == 0) {
		    strcat(unitbuf, utypename);
		} else if (strcmp(fmtstr, "side-name") == 0) {
		    strcat(unitbuf, side_name(unit->side));
		} else if (strcmp(fmtstr, "side-adjective") == 0) {
		    strcat(unitbuf, side_adjective(unit->side));
		} else {
		    strcat(unitbuf, "??description-format??");
		}
	    } else {
		strcat(unitbuf, "??description-format??");
	    }
	}
	return unitbuf;
    } else {
	strcat(unitbuf, sidebuf);
	strcat(unitbuf, " ");
    }
    /* If this unit is a self unit, say so. */
    if (unit->side != NULL && unit == unit->side->self_unit) {
    	if (!mobile(unit->type) || u_advanced(unit->type)) {
	  	strcat(unitbuf, "capital ");
	} else {
		strcat(unitbuf, "leader ");
    	}
	if (unit->name) {
		tprintf(unitbuf, "%s", unit->name);
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number), utypename);
	} else {
		strcat(unitbuf, utypename);
	}
    /* Default formats for units. */
    } else {
	    if (unit->name) {
		tprintf(unitbuf, "%s %s", utypename, unit->name);
	    } else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number), utypename);
	    } else {
		strcat(unitbuf, utypename);
	    }
    }
    return unitbuf;
}

char *
unit_handle(Side *side, Unit *unit)
{
    Side *side2 = NULL;

    if (unit != NULL)
      side2 = unit->side;
    return apparent_unit_handle(side, unit, side2);
}

static
Obj *
find_unit_spec_by_name(char *name)
{
    Obj *rest, *spec, *props, *bdg, *val;
    char *propname;

    for_all_list(unit_specs, rest) {
	spec = car(car(rest));
	for_all_list(spec, props) {
	    bdg = car(props);
	    if (consp(bdg)) {
		PARSE_PROPERTY_RETURN(bdg, propname, val, lispnil);
		if (keyword_code(propname) == K_N) {
		    if (strcmp(name, c_string(val)) == 0)
		      return car(rest);
		}
	    }
	}
    }
    return lispnil;
}

static
Obj *
find_unit_spec_by_number(int num)
{
    Obj *rest, *spec, *props, *bdg, *val;
    char *propname;

    for_all_list(unit_specs, rest) {
	spec = car(car(rest));
	for_all_list(spec, props) {
	    bdg = car(props);
	    if (consp(bdg)) {
		PARSE_PROPERTY_RETURN(bdg, propname, val, lispnil);
		if (keyword_code(propname) == K_NB) {
		    if (num == c_number(val))
		      return car(rest);
		}
	    }
	}
    }
    return lispnil;
}

Unit *
find_unit(int n)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->id == n && alive(unit))
	  return unit;
    }
    return NULL;
}

Unit *
find_unit_dead_or_alive(int n)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->id == n)
	  return unit;
    }
    return NULL;
}

Unit *
find_unit_by_name(char *nm)
{
    Unit *unit;

    if (nm == NULL)
      return NULL;
    for_all_units(unit) {
	if (unit->name != NULL && strcmp(unit->name, nm) == 0)
	  return unit;
    }
    return NULL;
}

Unit *
find_unit_by_number(int nb)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->number == nb)
	  return unit;
    }
    return NULL;
}

Unit *
find_unit_by_symbol(Obj *sym)
{
    Unit *unit;

    if (sym == lispnil)
      return NULL;
    for_all_units(unit) {
	if (equal(unit_symbol(unit), sym))
	  return unit;
    }
    return NULL;
}

int
unit_trusts_unit(Unit *unit1, Unit *unit2)
{
    return (unit1->side == unit2->side
	    || trusted_side(unit1->side, unit2->side));
}

int
type_survives_in_terrain(int u, int t)
{
	if (ut_vanishes_on(u, t))
	    return FALSE;
	if (ut_wrecks_on(u, t))
	    return FALSE;
	return TRUE;
}

int
type_survives_in_cell(int u, int x, int y)
{
	int c;

	/* First check if there is a safe connection to sit on. */
	for_all_connection_types(c) {
		if (aux_terrain_defined(c)
		    && any_connections_at(x, y, c)
		    && type_survives_in_terrain(u, c)) {
			return TRUE;
		}
	}
	/* If not, check if the ground terrain itself is safe. */
	if (type_survives_in_terrain(u, terrain_at(x, y))) {
		return TRUE;
	}
	return FALSE;
}

int
type_can_occupy_cell(int u, int x, int y)
{
    int t = terrain_at(x, y), u2, u3, numthistype = 0, fullness = 0;
    Unit *unit2;

    if (t_capacity(t) <= 0 && ut_capacity_x(u, t) <= 0) {
	return FALSE;
    }
    if (toc_numtypes == NULL) {
	toc_numtypes = (int *) xmalloc(numutypes * sizeof(int));
    }
    for_all_unit_types(u3) {
	toc_numtypes[u3] = 0;
    }
    for_all_stack(x, y, unit2) {
	u2 = unit2->type;
	++toc_numtypes[u2];
	if (u2 == u) {
	    ++numthistype;
	}
	/* Only count against fullness if exclusive capacity exceeded. */
	if (toc_numtypes[u2] > ut_capacity_x(u2, t)) {
	    fullness += ut_size(u2, t);
	}
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= ut_capacity_x(u, t)) {
	return TRUE;
    }
    /* Otherwise decide on the basis of fullness. */
    if (fullness + ut_size(u, t) <= t_capacity(t)) {
    	return TRUE;
    }
    return FALSE;
}

int
type_can_occupy(int u, Unit *transport)
{
    int utransport = transport->type, u3, o, space = 0;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int numfacilities = 0;
    int nummobiles = 0;
    int ucap, uucap;
    Unit *occ;

    /* Don't allow occupation of incomplete transports unless the unit is
       of a type that can help complete or the transport self-builds. Note:
       this test is questionable, but keep for now after fixing self-build
       bug. */
    if (!completed(transport)
        && uu_acp_to_build(u, utransport) < 1
        && u_cp_per_self_build(utransport) < 1) {
	return FALSE;
    }
    ucap = u_capacity(utransport);
    uucap = uu_capacity_x(utransport, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;
    for_all_unit_types(u3)
      tmp_u_array[u3] = 0;
    /* Compute the transport's fullness. */
    for_all_occupants(transport, occ) {
    	o = occ->type;
	++numalltypes;
	++tmp_u_array[occ->type];
	if (o == u)
	  ++numthistype;
	if (u_facility(o))
	  ++numfacilities;
	if (mobile(o))
	  ++nummobiles;
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tmp_u_array[o] > uu_capacity_x(utransport, o)) {
	    occvolume += uu_size(o, utransport);
	}
    }

    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */

    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(utransport, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(utransport, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(utransport) >= 0
	    && u_facility(u)
	    && numfacilities + 1 > u_facility_total_max(utransport))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(utransport) >= 0
	    && mobile(u)
	    && nummobiles + 1 > u_mobile_total_max(utransport))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(utransport) >= 0
        && numalltypes + 1 > u_occ_total_max(utransport))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (occvolume + uu_size(u, utransport) <= ucap);
    return space;
}

int
can_occupy(Unit *unit, Unit *transport)
{
    /* Intercept nonsensical arguments. */
    if (transport == unit)
      return FALSE;
    return type_can_occupy(unit->type, transport);
}

// Sorting

static
int
compare_units(Unit *unit1, Unit *unit2)
{
    if (unit1->type != unit2->type)
      return (unit1->type - unit2->type);
    if (unit1->name && unit2->name == NULL)
      return -1;
    if (unit1->name == NULL && unit2->name)
      return 1;
    if (unit1->name && unit2->name)
      return strcmp(unit1->name, unit2->name);
    if (unit1->number != unit2->number)
      return (unit1->number - unit2->number);
    /* Ids impose a total ordering. */
    return (unit1->id - unit2->id);
}

void
sort_units(int byidonly)
{
    int flips;
    int passes = 0;
    register Unit *unit, *nextunit;
    Side *side;

    for_all_sides(side) {
	passes = 0;
	flips = TRUE;
	while (flips) {
	    flips = FALSE;
	    for_all_side_units(side, unit) {
		if (unit->next != side->unithead
		    && (byidonly ? ((unit->id - unit->next->id) > 0)
				 : (compare_units(unit, unit->next) > 0))) {
		    flips = TRUE;
		    /* Reorder the units by fiddling with their links. */
		    nextunit = unit->next;
		    unit->prev->next = nextunit;
		    nextunit->next->prev = unit;
		    nextunit->prev = unit->prev;
		    unit->next = nextunit->next;
		    nextunit->next = unit;
		    unit->prev = nextunit;
		}
		++passes;
	    }
	}
    }
    Dprintf("Sorting passes = %d\n", passes);
}

// Vision and Visibility

int
see_chance(int u, int u2, int dist)
{
    assert_error(is_unit_type(u), "Invalid unit type encountered");
    assert_error(is_unit_type(u2), "Invalid unit type encountered");
    if (u_vision_range(u) < dist)
      return 0;
    if (u_see_always(u2))
      return 100;
    switch (dist) {
      case 0:
	return uu_see_at(u, u2);
      case 1:
	return uu_see_adj(u, u2);
      default:
	return uu_see(u, u2);
    }
    return 0;
}

int
see_chance(Unit *seer, Unit *tosee)
{
    int dist = -1, chance = 0;
    int u = NONUTYPE, u2 = NONUTYPE;
    int t = NONTTYPE, ta = NONTTYPE;
    int tv = UNSEEN;

    assert_error(in_play(seer), "Attempted to access an out-of-play unit.");
    assert_error(in_play(tosee), "Attempted to access an out-of-play unit.");
    tv = terrain_view(seer->side, tosee->x, tosee->y);
    if (UNSEEN == tv)
      return 0;
    t = vterrain(tv);
    u = seer->type;
    u2 = tosee->type;
    /* If visibility is 0 in any terrain, then the chance of seeing is 0. */
    /* (TODO: Implement 'ut_adds_visibility' and 'ut_multiplies_visibility'.) */
    if (0 >= ut_visibility(u2, t))
      return 0;
    if (any_aux_terrain_defined()) {
	for_all_aux_terrain_types(ta) {
	    if (aux_terrain_defined(ta)
		&& aux_terrain_at(tosee->x, tosee->y, ta)
		&& (0 >= ut_visibility(u2, ta)))
	      return 0;
	}
    }
    /* (TODO: Implement 'uu_transport_adds_vision',
	'uu_transport_multiples_vision', 'uu_occ_adds_vision', and
	'uu_occ_multiples_vision'.) */
    if (seer->transport && (0 == uu_occ_vision(u, seer->transport->type)))
      return 0;
    dist = distance(seer->x, seer->y, tosee->x, tosee->y);
    /* Basic see-chance between 2 units, accounting for distance. */
    chance = see_chance(u, u2, dist);
    /* Terrain modifications to see-chance. */
    chance = (chance * ut_visibility(u2, t)) / 100;
    if (any_aux_terrain_defined()) {
	for_all_aux_terrain_types(ta) {
	    if (aux_terrain_defined(ta)
		&& aux_terrain_at(tosee->x, tosee->y, ta))
	      chance = (chance * ut_visibility(u2, ta)) / 100;
	}
    }
    /* Transport modifications to see-chance. */
    /* (TODO: Implement 'uu_transport_adds_visibility',
	'uu_transport_multiplies_visibility', 'uu_occ_adds_visibility',
	and 'uu_occ_multiplies_visibility'.) */
    if (seer->transport)
      chance = (chance * uu_occ_vision(u, seer->transport->type)) / 100;
    /* Lighting modification to see-chance. */
    /* (TODO: Should test for twilight effects.) */
    /* (TODO: Implement 'ut_night_adds_visibility' and
	'ut_night_multiplies_visibility'.) */
    if (night_at(tosee->x, tosee->y)) {
	if (((ut_vision_night_effect(u, t) * u_vision_range(u)) / 100) < dist)
	  chance = 0;
    }
    return chance;
}

// Position Manipulation

void
leave_cell(Unit *unit)
{
    int ux = unit->x, uy = unit->y;
    Unit *transport = unit->transport;

    if (ux < 0 || uy < 0) {
	/* Sometimes called twice */
    } else if (transport != NULL) {
	leave_transport(unit);
	leave_cell_aux(unit, transport);
	all_see_leave(unit, ux, uy, FALSE);
	/* not all_see_cell here because can't see inside transports */
	update_unit_display(transport->side, transport, TRUE);
    } else {
	remove_unit_from_ustack(unit);
	/* Now bash the coords. */
	leave_cell_aux(unit, NULL);
	/* Now let everybody observe that the unit is gone. */
	all_see_leave(unit, ux, uy, TRUE);
    }
}

static
void
leave_cell_aux(Unit *unit, Unit *oldtransport)
{
    Unit *occ;

    if (unit->x < 0 && unit->y < 0)
      run_warning("unit %s has already left the cell", unit_desig(unit));
    /* Stash the old coords. */
    unit->prevx = unit->x;  unit->prevy = unit->y;
    /* Set to a recognizable value. */
    unit->x = -1;  unit->y = -1;
    /* Make any occupants leave too. */
    for_all_occupants(unit, occ) {
	leave_cell_aux(occ, unit);
    }
    /* Decrement viewing coverage around our old location. */
    cover_area(unit->side, unit, oldtransport, unit->prevx, unit->prevy, -1, -1);
}

static
void
remove_unit_from_ustack(Unit *unit)
{
    int ux = unit->x, uy = unit->y;
    Unit *other;

    /* Unsplice ourselves from the list of units in this cell. */
    if (unit == unit_at(ux, uy)) {
	set_unit_at(ux, uy, unit->nexthere);
    } else {
	for_all_stack(ux, uy, other) {
	    if (unit == other->nexthere) {
		other->nexthere = other->nexthere->nexthere;
		break;
	    }
	}
    }
    /* Bash this now-spurious link. */
    unit->nexthere = NULL;
}

// Lifecycle Management

static void
allocate_unit_block(void)
{
    int i;
    Unit *unitblock = (Unit *) xmalloc(INITMAXUNITS * sizeof(Unit));

    for (i = 0; i < INITMAXUNITS; ++i) {
        unitblock[i].id = -1;
        unitblock[i].next = &unitblock[i+1];
    }
    unitblock[INITMAXUNITS-1].next = NULL;
    freeunits = unitblock;
    Dprintf("Allocated space for %d units.\n", INITMAXUNITS);
}

Unit *
create_bare_unit(int type)
{
    Unit *newunit;

    /* If our free list is empty, go and get some more units. */
    if (freeunits == NULL) {
	allocate_unit_block();
    }
    /* Take the first unit off the free list. */
    newunit = freeunits;
    freeunits = freeunits->next;
    /* Give it a valid type... */
    newunit->type = type;
    /* ...but an invalid id. */
    newunit->id = -1;
    return newunit;
}

Unit *
create_unit(int type, int makebrains)
{
    int i, m;
    Unit *newunit;

    if (numlivebytype == NULL)
      numlivebytype = (int *) xmalloc(numutypes * sizeof(int));
    /* Test whether we've hit any designer-specified limits. */
    if ((u_type_in_game_max(type) >= 0
	 && numlivebytype[type] >= u_type_in_game_max(type))
	|| (g_units_in_game_max() >= 0
	    && numliveunits >= g_units_in_game_max())) {
	return NULL;
    }
    /* Allocate the unit object.  Xconq will fail instead of returning null. */
    newunit = create_bare_unit(type);
    /* Set the master uview to NULL; it will be allocated and updated as
       needed. */
    newunit->uview = NULL;
    /* Init all the slots to distinguishable values.  The unit is not
       necessarily newly allocated, so we have to hit all of its slots. */
    /* Note that we don't check for overflow of unit ids, since it's
       highly unlikely that a single game would ever create 2 billion
       units. */
    newunit->id = nextid++;
    newunit->name = NULL;
    /* Number == 0 means unit is unnumbered. */
    newunit->number = 0;
    /* If during launch, images may not be loaded yet. */
    if (numuimages) {
    	set_unit_image(newunit);
    }
    /* Outside the world. */
    newunit->x = newunit->y = -1;
    /* At ground level. */
    newunit->z = 0;
    /* Units default to being independent. */
    newunit->side = indepside;
    newunit->origside = indepside;
    /* Create at max hp, let others reduce if necessary. */
    newunit->hp = newunit->hp2 = u_hp(type);
    /* Create fully functional, let other routines set incompleteness. */
    newunit->cp = u_cp(type);
    /* Zero the combat experience. */
    newunit->cxp = 0;
    /* Zero the morale. */
    newunit->morale = 0;
    /* Not in a transport. */
    newunit->transport = NULL;
    /* Note that the space never needs to be freed. */
    if (newunit->supply == NULL && nummtypes > 0) {
        newunit->supply = (long *) xmalloc(nummtypes * sizeof(long));
    }
    /* Always zero out all the supply values. */
    for_all_material_types(m)
      newunit->supply[m] = 0;
    /* Allocate cache for units last production. */
    if (nummtypes > 0)
        newunit->production = (short *) xmalloc(nummtypes * sizeof(short));
    /* Always zero out all the production values. */
    for_all_material_types(m)
       newunit->production[m] = 0;
    /* Will allocate tooling state when actually needed. */
    newunit->tooling = NULL;
    /* Will allocate opinions when actually needed. */
    newunit->opinions = NULL;
    if (makebrains) {
	init_unit_actorstate(newunit, TRUE);
	init_unit_plan(newunit);
    } else {
	newunit->act = NULL;
	newunit->plan = NULL;
    }
    if (newunit->extras != NULL)
      init_unit_extras(newunit);
    newunit->occupant = NULL;
    newunit->nexthere = NULL;
    newunit->creation_id = 0;
    /* Glue this unit into the list of independent units. */
    newunit->next = newunit;
    newunit->prev = newunit;
    insert_unit(indepside->unithead, newunit);
    newunit->unext = unitlist;
    unitlist = newunit;
    /* Init various other slots. */
    newunit->prevx = newunit->prevy = -1;
    newunit->transport_id = lispnil;
    newunit->aihook = NULL;
    /* Advanced unit support. */
    newunit->size = default_size;
    newunit->reach = u_reach(type);
    newunit->usedcells = default_usedcells;
    newunit->maxcells = default_maxcells;
    newunit->population = default_population;
    newunit->curadvance = default_curadvance;
    newunit->cp_stash = 0;
    newunit->researchdone = FALSE;
    newunit->busy = FALSE;
    /* Add to the global unit counts. */
    ++numunits;
    ++numliveunits;
    ++(numlivebytype[type]);
    return newunit;
}

void
insert_unit(Unit *unithead, Unit *unit)
{
    unit->next = unithead->next;
    unit->prev = unithead;
    unithead->next->prev = unit;
    unithead->next = unit;
}

void
delete_unit(Unit *unit)
{
    unit->next->prev = unit->prev;
    unit->prev->next = unit->next;
}

void
init_unit_extras(Unit *unit)
{
    if (unit->extras == NULL)
      unit->extras = (UnitExtras *) xmalloc(sizeof(UnitExtras));
    /* Each slot must get the same value as the accessor would return
       if the structure had not been allocated. */
    unit->extras->point_value = -1;
    unit->extras->appear = -1;
    unit->extras->appear_var_x = -1;
    unit->extras->appear_var_y = -1;
    unit->extras->disappear = -1;
    unit->extras->priority = -1;
    unit->extras->sym = lispnil;
    unit->extras->sides = lispnil;
}

void
init_supply(Unit *unit)
{
    int m, u = unit->type;

    for_all_material_types(m) {
	unit->supply[m] = min(um_storage_x(u, m), um_initial(u, m));
    }
}

void
init_unit_tooling(Unit *unit)
{
    unit->tooling = (short *) xmalloc(numutypes * sizeof(short));
}

void
init_unit_opinions(Unit *unit, int nsides)
{
    int i;
    short *temp;

    temp = NULL;
    if (u_opinion_min(unit->type) != u_opinion_max(unit->type)) {
	if (unit->opinions != NULL && nsides > numsides) {
	    temp = unit->opinions;
	    unit->opinions = NULL;
	}
	if (unit->opinions == NULL)
	  unit->opinions = (short *) xmalloc(nsides * sizeof(short));
	/* Opinions are now all neutral. */
	if (temp != NULL) {
	    /* Copy over old opinions. */
	    for (i = 0; i < numsides; ++i)
	      unit->opinions[i] = temp[i];
	    free(temp);
	}
    } else {
	if (unit->opinions != NULL)
	  free(unit->opinions);
	unit->opinions = NULL;
    }
}

static
void
flush_one_unit(Unit *unit)
{
    unit->id = -1;
    unit->imf = NULL;
    unit->image_name = NULL;
    unit->occupant = NULL;
    unit->transport = NULL;
    unit->nexthere = NULL;
    unit->prev = NULL;
    unit->unext = NULL;
    /* Add it on the front of the list of available units. */
    unit->next = freeunits;
    freeunits = unit;
}

void
flush_dead_units(void)
{
    Unit *unit, *prevunit, *nextunit;

    if (unitlist == NULL)
      return;
    unit = unitlist;
    while (!alive(unit)) {
	nextunit = unit->unext;
	delete_unit(unit);
	flush_one_unit(unit);
	unit = nextunit;
	if (unit == NULL)
	  break;
    }
    unitlist = unit;
    /* Since the first unit of unitlist is guaranteed live now,
       we know that prevunit will always be set correctly;
       but mollify insufficiently intelligent compilers. */
    prevunit = NULL;
    for_all_units(unit) {
	if (!alive(unit)) {
	    nextunit = unit->unext;
	    prevunit->unext = unit->unext;
	    delete_unit(unit);
	    flush_one_unit(unit);
	    unit = prevunit;
	} else {
	    prevunit = unit;
	}
    }
}

// Images

void
set_unit_image(Unit *unit)
{
    int u = unit->type, choice, i = 0;
    ImageFamily *imf;

    /* Some interfaces don't have images. */
    if (!uimages) {
    	return;
    }
    /* Use the preassigned image if available. */
    if (!empty_string(unit->image_name)) {
    	imf = find_imf(unit->image_name);
    	if (imf) {
	    unit->imf = imf;
	    return;
    	}
    }
    /* Else pick a random image from the list for this unit type. */
    choice = xrandom(numuimages[u]);
    for (imf = uimages[u]; imf != NULL; imf = imf->next) {
	if (i == choice) {
	    unit->imf = imf;
	    unit->image_name = imf->name;
	    break;
    	}
	++i;
    }
}

// Sides

void
set_unit_side(Unit *unit, Side *side)
{
    int u = unit->type;
    Side *oldside, *newside;

    /* Might not have anything to do. */
    if (unit->side == side)
      return;
    /* Subtract from the counts for the ex-side. */
    oldside = unit->side;
    if (oldside->numunits)
      --(oldside->numunits[u]);
    /* Set the unit's slot. */
    /* Note that indep units have a NULL side, even though there
	   is an actual side object for independents. */
    unit->side = side;
    /* Make sure this unit is off anybody else's list. */
    delete_unit(unit);
    newside = side;
    insert_unit(newside->unithead, unit);
    /* Add to counts for the side. */
    if (newside->numunits)
      ++(newside->numunits[u]);
    /* Invalidate both sides' point value caches. */
    oldside->point_value_valid = FALSE;
    newside->point_value_valid = FALSE;
    /* Bump the tech level if owning this type helps. */
    if (side != NULL
	&& side->tech[u] < u_tech_from_ownership(u)) {
	side->tech[u] = u_tech_from_ownership(u);
	/* (should update any displays of tech - how to ensure?) */
    }
}

void
set_unit_origside(Unit *unit, Side *side)
{
    unit->origside = side;
}

// GDL I/O

void
interp_unit_defaults(Obj *form)
{
    int u, numval, wasnum, variablelength;
    Obj *props = form, *bdg, *val;
    char *propname;

    default_unit_spec = form;
    if (match_keyword(car(props), K_RESET)) {
	/* Reset all the tweakable defaults. */
	uxoffset = 0, uyoffset = 0;
	default_unit_side_number = -1;
	default_unit_origside_number = -1;
	default_unit_cp = -1;
	default_unit_hp = -1;
	default_unit_cxp = -1;
	default_unit_z = -1;
	default_transport_id = -1;
	if (default_tooling != NULL) {
	    for_all_unit_types(u)
	      default_tooling[u] = -1;
	}
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	numval = 0;
	wasnum = FALSE;
	if (numberp(val)) {
	    numval = c_number(val);
	    wasnum = TRUE;
	}
	variablelength = FALSE;
	/* Note that not all unit slots can get default values. */
	switch (keyword_code(propname)) {
	  case K_AT:
	    uxoffset = numval;
	    uyoffset = c_number(caddr(bdg));
	    /* The property is not really variable-length, but mollify
               the error check. */
	    variablelength = TRUE;
	    break;
	  case K_S:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    default_unit_side_number = numval;
	    break;
	  case K_OS:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    default_unit_origside_number = numval;
	    break;
	  case K_CP:
	    default_unit_cp = numval;
	    break;
	  case K_HP:
	    default_unit_hp = numval;
	    break;
	  case K_CXP:
	    default_unit_cxp = numval;
	    break;
	  case K_TP:
	    disallow_more_unit_types();
	    if (default_tooling == NULL)
	      default_tooling = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(default_tooling, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_IN:
	    default_transport_id = numval;
	    break;
	  case K_Z:
	    default_unit_z = numval;
	    break;
	  default:
	    unknown_property("unit-defaults", "", propname);
	}
	if (!variablelength && cddr(bdg) != lispnil)
	  read_warning("Extra junk in a default %s property, ignoring",
		       propname);
    }
}

Unit *
interp_unit(Obj *form)
{
    int u, u2, tp_varies, numval, numval2, wasnum, nuid = 0, variablelength,
unitnum;
    int nusn = -1, nuosn = -1;
    char *propname, *unitname;
    Obj *head = car(form), *props = cdr(form), *bdg, *val, *bdgrest, *val2;
    Obj *save, *unitspec;
    Unit *unit, *unit2;
    extern int nextid;

    if (!g_create_units()) {
	/* Stash the unit form and current defaults for later use. */
	Dprintf("Saving the unit spec ");
	Dprintlisp(form);
	Dprintf("\n");
	save = cons(cons(form, cons(default_unit_spec, lispnil)), lispnil);
	if (unit_specs != lispnil) {
	    set_cdr(last_unit_spec, save);
	    last_unit_spec = save;
	} else {
	    unit_specs = last_unit_spec = save;
	}
	return NULL;
    }
    Dprintf("Reading a unit from ");
    Dprintlisp(form);
    Dprintf("\n");
    if (symbolp(head)) {
	u = utype_from_symbol(head);
     	if (u != NONUTYPE) {
	    unit = create_unit(u, FALSE);
	    canaddmtype = FALSE;  /* (Why?) */
	    if (unit == NULL) {
		read_warning("Failed to create a unit, skipping the form");
		return NULL;
	    }
	} else {
	    read_warning("\"%s\" not a known unit type, skipping the form",
			 c_string(head));
	    return NULL;
	}
    } else if (stringp(head)) {
	unitname = c_string(head);
	unit = find_unit_by_name(unitname);
	if (unit == NULL) {
	    if (unit_specs != lispnil) {
		unitspec = find_unit_spec_by_name(unitname);
		if (unitspec != lispnil) {
		    /* First set up the saved defaults. */
		    interp_unit_defaults(cadr(unitspec));
		    unit = interp_unit(car(unitspec));
		    /* Restore the current defaults. */
		    interp_unit_defaults(default_unit_spec);
		    if (unit == NULL) {
			read_warning("Failed to create unit from saved spec");
			return NULL;
		    }
		} else {
		    read_warning("No unit spec named \"%s\", skipping the form",
				 unitname);
		    return NULL;
		}
	    } else {
		read_warning("No unit named \"%s\", skipping the form",
			     unitname);
		return NULL;
	    }
    	}
    } else if (numberp(head)) {
	unitnum = c_number(head);
	unit = find_unit_by_number(unitnum);
	if (unit == NULL) {
	    if (unit_specs != lispnil) {
		unitspec = find_unit_spec_by_number(unitnum);
		if (unitspec != lispnil) {
		    /* First set up the saved defaults. */
		    interp_unit_defaults(cadr(unitspec));
		    unit = interp_unit(car(unitspec));
		    /* Restore the current defaults. */
		    interp_unit_defaults(default_unit_spec);
		    if (unit == NULL) {
			read_warning("Failed to create unit from saved spec");
			return NULL;
		    }
		} else {
		    read_warning("No unit spec numbered %d, skipping the form",
				 unitnum);
		    return NULL;
		}
	    } else {
		read_warning("No unit numbered %d, skipping the form",
			     unitnum);
		return NULL;
	    }
    	}
    }
    /* At this point we're guaranteed to have a unit to work with. */
    /* Modify the unit according to current defaults. */
    if (default_unit_side_number >= 0)
      nusn = default_unit_side_number;
    if (default_unit_origside_number >= 0)
      nuosn = default_unit_origside_number;
    if (default_unit_cp >= 0)
      unit->cp = default_unit_cp;
    if (default_unit_hp >= 0)
      unit->hp = unit->hp2 = default_unit_hp;
    if (default_unit_cxp >= 0)
      unit->cxp = default_unit_cxp;
    /* First default supplies using the generic supply initialize. */
    init_supply(unit);
    /* Fill in any default toolings that might have been set up. */
    if (default_tooling != NULL) {
	tp_varies = FALSE;
	for_all_unit_types(u2) {
	    if (default_tooling[u2] > 0) {
		tp_varies = TRUE;
		break;
	    }
	}
	if (tp_varies) {
	    if (unit->tooling == NULL)
	      init_unit_tooling(unit);
	    for_all_unit_types(u2)
	      unit->tooling[u] = default_tooling[u];
	}
    }
    /* Peel off fixed-position properties, if they're supplied. */
    if (numberp(car(props))) {
	unit->prevx = c_number(car(props)) + uxoffset - area.fullx;
	props = cdr(props);
    }
    if (numberp(car(props))) {
	unit->prevy = c_number(car(props)) + uyoffset - area.fully;
	props = cdr(props);
    }
    if (props != lispnil && !consp(car(props))) {
	nusn = c_number(eval(car(props)));
	props = cdr(props);
    }
    /* Now crunch through optional stuff.  The unit's properties must *already*
       be correct. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY_RETURN(bdg, propname, val, unit);
	numval = 0;
	wasnum = FALSE;
	if (numberp(val)) {
	    numval = c_number(val);
	    wasnum = TRUE;
	}
	variablelength = FALSE;
	switch (keyword_code(propname)) {
	  case K_N:
	    unit->name = c_string(val);
	    break;
	  case K_IMAGE_NAME:
	    unit->image_name = c_string(val);
	    break;
	  case K_SHARP:
	    nuid = numval;
	    break;
	  case K_S:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    nusn = numval;
	    break;
	  case K_OS:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    nuosn = numval;
	    break;
	  case K_AT:
	    if (numberp(caddr(bdg))) {
		numval2 = c_number(caddr(bdg));
	    } else {
		type_error(caddr(bdg), "not a number");
		numval2 = 0;
	    }
	    unit->prevx = numval + uxoffset - area.fullx;
	    unit->prevy = numval2 + uyoffset - area.fully;
	    variablelength = TRUE;
	    break;
	  case K_NB:
	    unit->number = numval;
	    break;
	  case K_CP:
	    unit->cp = numval;
	    break;
	  case K_HP:
	    /* Note if the unit is read in as damaged, we don't want
	       damage reckoning to replace with a default hp2. */
	    unit->hp = unit->hp2 = numval;
	    break;
	  case K_CXP:
	    unit->cxp = numval;
	    break;
	  case K_MO:
	    unit->morale = numval;
	    break;
	  case K_TRK:
	    interp_side_mask_list(&(unit->tracking), cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_M:
	    interp_treasury_list(unit->supply, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_TP:
	    if (unit->tooling == NULL)
	      init_unit_tooling(unit);
	    interp_utype_value_list(unit->tooling, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_OPINIONS:
	    if (unit->opinions == NULL)
	      init_unit_opinions(unit, numsides);
	    if (unit->opinions != NULL) {
		interp_side_value_list(unit->opinions, cdr(bdg));
	    } else {
		read_warning("Unit %s cannot have opinions, ignoring attempt to set",
			     unit_desig(unit));
	    }
	    variablelength = TRUE;
	    break;
	  case K_IN:
	    unit->transport_id = val;
	    break;
	  case K_ACP:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE, numval);
	    if (unit->act != NULL)
	      unit->act->acp = numval;
	    else
	      read_warning("Unit %s cannot have acp, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_ACP0:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE, numval);
	    if (unit->act != NULL)
	      unit->act->initacp = numval;
	    else
	      read_warning("Unit %s cannot have acp0, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_AM:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE);
	    if (unit->act != NULL)
	      unit->act->actualmoves = numval;
	    else
	      read_warning("Unit %s cannot have am, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_A:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE);
	    if (unit->act != NULL)
	      interp_action(&(unit->act->nextaction), val);
	    else
	      read_warning("Unit %s cannot have action, ignoring attempt to set",
			   unit_desig(unit));
	    variablelength = TRUE;
	    break;
	  case K_PLAN:
	    interp_unit_plan(unit, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_Z:
	    unit->z = numval;
	    break;
	  case K_SIZE:
	    unit->size = numval;
	    break;
	  case K_REACH:
	    unit->reach = numval;
	    break;
	  case K_USEDCELLS:
	    unit->usedcells = numval;
	    break;
	  case K_MAXCELLS:
	    unit->maxcells = numval;
	    break;
	  case K_CURADVANCE:
	    unit->curadvance = numval;
	    break;
	  case K_PRODUCTION:
	    interp_mtype_value_list(unit->production, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_POPULATION:
	    unit->population = numval;
	    break;
	  case K_CP_STASH:
	    unit->cp_stash = numval;
	    break;
          case K_CREATION_ID:
            unit->creation_id = numval;
            break;
	  case K_POINT_VALUE:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->point_value = numval;
	    break;
	  case K_APPEAR:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->appear = numval;
	    /* Get the xy variation if supplied. */
	    bdgrest = cddr(bdg);
	    if (bdgrest != lispnil) {
		val2 = car(bdgrest);
		if (numberp(val2)) {
		    unit->extras->appear_var_x = c_number(val2);
		    bdgrest = cdr(bdgrest);
		    val2 = car(bdgrest);
		    if (numberp(val2)) {
			unit->extras->appear_var_y = c_number(val2);
		    }
		}
	    }
	    variablelength = TRUE;
	    break;
	  case K_DISAPPEAR:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->disappear = numval;
	    break;
	  case K_PRIORITY:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->priority = numval;
	    break;
	  case K_SYM:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->sym = val;
	    break;
	  case K_SIDES:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->sides = cdr(bdg);
	    variablelength = TRUE;
	    break;
	  default:
	    unknown_property("unit", unit_desig(unit), propname);
	}
	if (!variablelength && cddr(bdg) != lispnil)
	  read_warning("Extra junk in the %s property of %s, ignoring",
		       propname, unit_desig(unit));
    }
    /* If the unit id was given, assign it to the unit, avoiding
       duplication. */
    if (nuid > 0) {
    	/* If this id is already in use by some other unit, complain. */
    	unit2 = find_unit(nuid);
    	if (unit2 != NULL && unit2 != unit)
	  init_error("Id %d already in use by %s", nuid, unit_desig(unit2));
     	/* Guaranteed distinct, safe to use. */
	unit->id = nuid;
	/* Ensure that future random ids won't step on this one. */
	nextid = max(nextid, nuid + 1);
    }
    if (nusn >= 0) {
	/* (should check that this is an allowed side?) */
	set_unit_side(unit, side_n(nusn));
    }
    if (nuosn >= 0) {
	/* (should check that this is an allowed side?) */
	set_unit_origside(unit, side_n(nuosn));
    } else {
	set_unit_origside(unit, unit->side);
    }
    Dprintf("  Got %s\n", unit_desig(unit));
    return unit;
}

void
write_unit_properties(Unit *unit)
{
    write_str_prop(key(K_IMAGE_NAME), unit->image_name, NULL, FALSE, FALSE);
    write_num_prop(key(K_NB), unit->number, 0, FALSE, FALSE);
    write_num_prop(key(K_HP), unit->hp, u_hp(unit->type), FALSE, FALSE);
    write_num_prop(key(K_CP), unit->cp, u_cp(unit->type), FALSE, FALSE);
    write_num_prop(key(K_CXP), unit->cxp, 0, FALSE, FALSE);
    write_num_prop(key(K_MO), unit->morale, 0, FALSE, FALSE);
    write_num_prop(key(K_TRK), unit->tracking, NOSIDES, FALSE, FALSE);
    write_utype_value_list(key(K_TP), unit->tooling, 0, FALSE);
    write_side_value_list(key(K_OPINIONS), unit->opinions, 0, FALSE);
    /* Getting the defaults right for the supply list requires more
       cleverness, alway write for now. */
    write_treasury_list(key(K_M), unit->supply, -1, FALSE);
    write_num_prop(key(K_POINT_VALUE), unit_point_value(unit), -1, FALSE,
                   FALSE);
    write_num_prop(key(K_APPEAR), unit_appear_turn(unit), -1, FALSE, FALSE);
    /* (should do appear x,y also here?) */
    write_num_prop(key(K_DISAPPEAR), unit_disappear_turn(unit), -1, FALSE,
                   FALSE);
    write_lisp_prop(key(K_SIDES), unit_sides(unit), lispnil, FALSE, TRUE,
                    FALSE);
    /* Advanced unit support. */
    if (!u_advanced(unit->type))
      return;
    write_num_prop(key(K_SIZE), unit->size, 0, FALSE, FALSE);
    write_num_prop(key(K_REACH), unit->reach, 0, FALSE, FALSE);
    write_num_prop(key(K_USEDCELLS), unit->usedcells, 0, FALSE, FALSE);
    write_num_prop(key(K_MAXCELLS), unit->maxcells, 1, FALSE, FALSE);
    write_num_prop(key(K_CURADVANCE), unit->curadvance, -1, FALSE, FALSE);
    write_num_prop(key(K_POPULATION), unit->population, 1, FALSE, FALSE);
    write_mtype_value_list(key(K_PRODUCTION), unit->production, 0, FALSE);
    write_num_prop(key(K_CP_STASH), unit->cp_stash, FALSE, FALSE, FALSE);
    write_num_prop(key(K_CREATION_ID), unit->creation_id, 0, FALSE, FALSE);
}

void
write_units(Module *module)
{
    int x0, y0, x, y, numtowrite;
    Unit *unit;
    Side *loopside;

    /* Make sure no dead units get saved. */
    flush_dead_units();
    /* Make a consistent ordering. */
    sort_units(module->def_all || module->def_unit_ids);
    numtowrite = 0;
    for_all_sides(loopside) {
	for_all_side_units(loopside, unit) {
	    if (alive(unit))
	      ++numtowrite;
	}
    }
    add_to_form(";");
    add_num_to_form(numtowrite);
    add_to_form(" units");
    newline_form();
    /* Need to write out the defaults being assumed subsequently. */
    /* maybe use those in postprocessing. */
    start_form(key(K_UNIT_DEFAULTS));
    end_form();
    newline_form();
    Dprintf("Writing %d units ...\n", numliveunits);
    for_all_sides(loopside) {
	for_all_side_units(loopside, unit) {
	    if (alive(unit)) {
		/* K_AT always written */
		/* K_S always written */
		/* If the unit will appear later, must write out that
		   later position, possibly mapped to a new place if
		   the map is being reshaped. */
		if (unit->cp < 0 && unit_appear_turn(unit) >= 0) {
		    x0 = (- unit->prevx);  y0 = (- unit->prevy);
		} else {
		    x0 = unit->x;  y0 = unit->y;
		}
		if (doreshape) {
		    reshaped_point(x0, y0, &x, &y);
		} else {
		    x = x0;  y = y0;
		}
		/* If these were negative values made positive for the
		   purposes of reshaping, make them negative again. */
		if (unit->cp < 0 && unit_appear_turn(unit) >= 0) {
		    x = (- x);  y = (- y);
		}
		start_form(shortest_escaped_name(unit->type));
		add_num_to_form(x);
		add_num_to_form(y);
		if (unit->side != NULL && unit->side->symbol != lispnil)
		  add_to_form(escaped_symbol(c_string(unit->side->symbol)));
		else
		  add_num_to_form(side_number(unit->side));
		write_num_prop(key(K_Z), unit->z, 0, FALSE, FALSE);
		write_str_prop(key(K_N), unit->name, NULL, FALSE, FALSE);
		write_num_prop(key(K_OS), side_number(unit->origside),
			       side_number(unit->side), FALSE, FALSE);
		/* Maybe write the unit's id. */
		if (module->def_all
		    || module->def_unit_ids
		    || (unit->occupant
			&& unit->name == NULL
			&& unit_symbol(unit) == lispnil))
		  write_num_prop(key(K_SHARP), unit->id, 0, FALSE, FALSE);
		/* Need this to get back into the right transport. */
		if (unit->transport) {
		    if (unit_symbol(unit->transport) != lispnil
		        	&& !module->def_all
			&& !module->def_unit_ids) {
			write_lisp_prop(key(K_IN),
					unit_symbol(unit->transport),
					lispnil, FALSE, TRUE, FALSE);
		    } else if (unit->transport->name
		        	       && !module->def_all
			       && !module->def_unit_ids) {
			write_str_prop(key(K_IN), unit->transport->name,
				       NULL, FALSE, FALSE);
		    } else {
			write_num_prop(key(K_IN), unit->transport->id,
				       0, FALSE, FALSE);
		    }
		}
		/* Always need this, because occupants may reference. */
		write_lisp_prop(key(K_SYM), unit_symbol(unit), lispnil,
				FALSE, TRUE, FALSE);
		/* Write optional info about the units. */
		if (module->def_all || module->def_unit_props)
		  write_unit_properties(unit);
		if (module->def_all || module->def_unit_acts)
		  write_unit_act(unit);
		if (module->def_all || module->def_unit_plans)
		  write_unit_plan(unit);
		/* close the unit out */
		end_form();
		newline_form();
		Dprintf("Wrote %s\n", unit_desig(unit));
	    }
	}
	newline_form();
    }
    Dprintf("... Done writing units.\n");
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

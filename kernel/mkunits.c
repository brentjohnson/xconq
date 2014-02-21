/* Unit generation for Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

using namespace Xconq;

static void add_preexisting_units(Side *side, int runs);
static int country_is_complete(Side *side, int runs);
static int average_numunits(int u);
static void count_cells(int x, int y);
static int good_place(int cx, int cy);
static void find_a_place(Side *side);
static int valid_unit_place(int x, int y);
static int possible_unit_place(int x, int y);
static int find_unit_place(int u, int cx, int cy, int *xp, int *yp);
static void mung_terrain(int x, int y, int u);
static Unit *find_occupant_place(Side *side, int u, int x0, int y0);
static int at_country_units_max(Side *side, int u);
static void grant_initial_occs(Unit *unit);
static void place_unit(
    int u, Side *side, int x, int y, Unit *transport, 
    int announce = TRUE, int complete = TRUE);
static void expand_countries(void);
static void expand_country(int x, int y);
static void set_people_on_side(int x, int y);
static int not_too_crowded(int u, int x, int y);

/* Tmp array for counting terrain. */

static int *numcells;

static int *favorite;

static int *totnumcells;

/* This is the number of country placements that could not be found to meet
   all the constraints. */

static int badcountryplaces = 0;

/* This says whether terrain alteration is permissible to make country
   placements work.  This only happens when normal placement starts to fail. */

static int mungterrain;

static int baseradius;

static int tmpradius;

static int curmindistance;
static int curmaxdistance;

static int sideprogress;
int sidedeltahalf;

static int growth;

static int totnumtodo;
static int totnumdone;

/* Place all the units belonging to countries. */

int any_side_possible_units = -1;

int
make_countries(int calls, int runs)
{
    int alldone, x0, y0, u, t, i, tot, x, y;
    int advantage, sideadvantage, favor;
    int *canbeinopen;  /* true if type need not be occ at start */
    int *numtodo, *numindeptodo, *numleft, *numindepleft, totleft;
    int numlisted, numfails;
    int checkmins = FALSE;
    int dopeoplesides = FALSE, dopeopleindeps = FALSE;
    int numdone = 0;
    Unit *unit, *transport;
    Side *side;
    char tmpbuf2[BUFSIZE];

    /* Run this always, unless something important is missing. */
    if (!terrain_defined())
      return FALSE;
    /* Add any units already in the world that should belong to a side. */
    for_all_sides(side) {
	add_preexisting_units(side, runs);
    }
    /* Scan countries to see if done already. */
    alldone = TRUE;
    for_all_sides(side) {
	if (!country_is_complete(side, runs)) {
	    alldone = FALSE;
	    break;
	}
    }
    if (alldone)
      return FALSE;
    canbeinopen = (int *) xmalloc(numutypes * sizeof(int));
    numtodo = (int *) xmalloc(numutypes * sizeof(int));
    numindeptodo = (int *) xmalloc(numutypes * sizeof(int));
    numleft = (int *) xmalloc(numutypes * sizeof(int));
    numindepleft = (int *) xmalloc(numutypes * sizeof(int));
    /* Calculate whether the minimum required terrain types are present. */
    totnumcells = (int *) xmalloc(numttypes * sizeof(int));
    for_all_terrain_types(t) {
    	if (t_country_min(t) > 0)
    	  checkmins = TRUE;
    }
    if (checkmins) {
    	/* Edge cells are useless for country placement. */
    	for_all_interior_cells(x, y) {
    	    ++totnumcells[(int) terrain_at(x, y)];
    	}
    	for_all_terrain_types(t) {
    	    if (t_country_min(t) * numsides > totnumcells[t]) {
    	    	init_warning("Not enough %s for all %d sides",
			     t_type_name(t), numsides);
    	    	/* Don't error out, might be extenuating circumstances. */
    	    }
    	}
    }
    announce_lengthy_process("Making countries");
    /* Precompute some important info */
    numcells = (int *) xmalloc(numttypes * sizeof(int));
    favorite = (int *) xmalloc(numutypes * sizeof(int));
    tot = 0;
    for_all_unit_types(u) {
	canbeinopen[u] = FALSE;
	favorite[u] = NONTTYPE;
	favor = 0;
	for_all_terrain_types(t) {
	    if (ut_favored(u, t) > 0)
	      canbeinopen[u] = TRUE;
	    if (ut_favored(u, t) > favor) {
		favorite[u] = t;
		favor = ut_favored(u, t);
	    }
	}
	if (canbeinopen[u]) {
	    tot += u_start_with(u) + u_indep_near_start(u);
	}
    }
    /* Make space for people sides if we're going to have any. */
    if (!people_sides_defined()) {

    /* Always allocate area.peopleside since expand_country uses it even
    if dopeoplesides and dopeopleindeps both are FALSE. */
#if 0    
	for_all_terrain_types(t) {
	    if (t_country_people(t) > 0)
	      dopeoplesides = TRUE;
	    if (t_indep_people(t) > 0)
	      dopeopleindeps = TRUE;
	}
	if (dopeoplesides || dopeopleindeps) {
	    allocate_area_people_sides();
	}
#else
	allocate_area_people_sides();
#endif
    }
    if (g_radius_min() <= 0) {
	/* If no radius specified, pick something plausible. */
	baseradius = max(1, isqrt(3 * tot) / 2);
    } else {
	baseradius = g_radius_min();
    }
    curmindistance = g_separation_min();
    curmaxdistance = g_separation_max();
    badcountryplaces = 0;
    for_all_sides(side) {
	sideprogress = (100 * numdone++) / numsides;
	sidedeltahalf = (100 / numsides) / 2;
	announce_progress(sideprogress);
	if (!country_is_complete(side, runs)) {
	    sideadvantage = max(1, side->advantage);
	    advantage = actual_advantage(side);
	    tmpradius = baseradius;
	    if (advantage > 1)
	      tmpradius = (baseradius * isqrt(advantage * 100) + 9) / 10;
	    /* Discover or generate the country's center. */
	    mungterrain = FALSE;
	    find_a_place(side);
	    x0 = side->startx;  y0 = side->starty;
	    Dprintf("%s starts around %d,%d\n", side_desig(side), x0, y0);
	    announce_progress(sideprogress + sidedeltahalf);
	    totnumtodo = totnumdone = 0;
	    /* Calculate how many units of each type we will get, both
	       the ones that belong to the side, and any independent
	       units that should start out nearby. */
	    for_all_unit_types(u) {
		numtodo[u] = numindeptodo[u] = 0;
		/* Note that the numbers to create are primarily
		   governed by unit type properties and the sides'
		   advantages, but we also need not to put units on
		   disallowed sides. */
		if (type_allowed_on_side(u, side)) {
		    if (runs > 0) {
			/* If the side is just coming into an ongoing
			   game, give it an average number of each
			   type that the existing sides started with,
			   so that the new side has some parity. */
			if (u_start_with(u) > 0 || u_indep_near_start(u) > 0)
			  numtodo[u] =
			    (average_numunits(u) * advantage) / sideadvantage;
		    } else {
			numtodo[u] =
			  (u_start_with(u) * advantage) / sideadvantage;
		    }
		    totnumtodo += numtodo[u];
		}
		if (type_allowed_on_side(u, indepside)) {
		    numindeptodo[u] =
		      (u_indep_near_start(u) * advantage) / sideadvantage;
		    totnumtodo += numindeptodo[u];
		}
		numleft[u] = numindepleft[u] = 0;
	    }
	    /* First place the units belonging to the side initially. */
	    for_all_unit_types(u) {
		if (canbeinopen[u]) {
		    for (i = 0; i < numtodo[u]; ++i) {
			if (find_unit_place(u, x0, y0, &x, &y)) {
			    place_unit(u, side, x, y, NULL);
			} else {
			    /* If can't find places for this type,
			       give up and go on to the next type. */
		            numleft[u] = numtodo[u] - i;
			    break;
			}
		    }
		}
	    }
	    /* Now do independent units in the initial country area. */
	    for_all_unit_types(u) {
		for (i = 0; i < numindeptodo[u]; ++i) {
		    if (find_unit_place(u, x0, y0, &x, &y)) {
			place_unit(u, indepside, x, y, NULL);
		    } else {
			/* If can't find places for this type, give up
			   and go on to the next type. */
			numindepleft[u] = numindeptodo[u] - i;
			break;
		    }
		}
	    }
	    /* Now do units that have to be occupants.  Note that if
	       occupants are allowed in independent units, then they
	       might be placed in an independent rather than an owned
	       unit.  We have to trust that the game designer intended
	       this to happen... */
	    for_all_unit_types(u) {
		if (!canbeinopen[u]) {
		    for (i = 0; i < numtodo[u]; ++i) {
			if ((transport = find_occupant_place(side, u, x0, y0))
			    != NULL) {
			    place_unit(u, side, x0, y0, transport);
			} else {
		   	    numleft[u] = numtodo[u] - i;
			    break;
			}
		    }
		}
	    }
	    /* Now warn about what couldn't be placed. */
	    totleft = numlisted = numfails = 0;
	    tmpbuf2[0] = '\0';
	    for_all_unit_types(u) {
		totleft += numleft[u];
		if (numleft[u] > 0 && numlisted < 5) {
		    ++numfails;
                       strncat(tmpbuf2, (numfails == 1 ? "" : " "), 
			       BUFSIZE - strlen(tmpbuf2));
		    strncat(tmpbuf2, u_type_name(u), BUFSIZE - strlen(tmpbuf2));
		}
	    }
	    if (totleft > 0) {
		char tmpbuf3[BUFSIZE];

		init_warning("could not put %d units in %s country (%s%s)",
			     totleft, shortest_side_title(side, tmpbuf3),
			     tmpbuf2, (numfails >= 5 ? " etc" : ""));
	    }
	}
	/* Set the side of the people in this country. */
	tmpside = side;
	apply_to_area(side->startx, side->starty, tmpradius + 1,
		      set_people_on_side);
	/* Make sure each unit has people there, if allowed at all. */
	for_all_side_units(side, unit) {
	    if (inside_area(unit->x, unit->y)
	        && t_country_people(terrain_at(unit->x, unit->y)) > 0) {
		set_people_side_at(unit->x, unit->y, side_number(side));
	    }
	}
    }
    finish_lengthy_process();
    /* Grow each country out to a maximum radius. */
    expand_countries();
    
    /* This was a major source of network lockups during launch
    of the game. So what if somebody got a bad deal? Life's tough. */
#if 0
    /* Warn about any difficulties encountered. */
    if (badcountryplaces > 0) {
    	init_warning("%d of %d sides have undesirable locations",
		     badcountryplaces, numsides);
    }
#endif

    free(canbeinopen);
    free(numtodo);
    free(numindeptodo);
    free(numleft);
    free(numindepleft);
    return TRUE;
}

/* Test to see whether the side's pre-existing setup already suffices
   as a country in this game. */

static int
country_is_complete(Side *side, int runs)
{
    int u, totunits;
    Unit *unit;
    int sideadvantage = max(1, side->advantage);
    int advantage = actual_advantage(side);

    /* Don't make any more independent units than preassigned. This prevents 
    the formation of a third side in games like flattop.g and duel.g. */
    if (side == indepside)
      return TRUE;
    /* If side no longer in game, then we don't want to do country
       synth on it. */
    if (!side->ingame)
      return TRUE;
    totunits = 0;
    for_all_unit_types(u)
      tmp_u_array[u] = 0;
    /* Count up all and only the completed and present units. */
    for_all_side_units(side, unit) {
    	if (in_play(unit)) {
	    ++totunits;
	    ++tmp_u_array[unit->type];
	}
    }
    /* See if we're re-running side generation for the benefit of
       adding a new side, use the total counts of units to distinguish
       the new side from the old sides. */
    if (runs > 0)
      return (totunits > 0);
    for_all_unit_types(u) {
	if (tmp_u_array[u] < ((u_start_with(u) * advantage) / sideadvantage))
	  return FALSE;
    }
    return TRUE;
}

static void
add_preexisting_units(Side *side, int runs)
{
    int tries, total;
    Obj *possunits, *origpossunits, *chosen, *lis, *rest, *head;
    Unit *unit;

    if (any_side_possible_units < 0) {
	any_side_possible_units = FALSE;
	for_all_units(unit) {
	    lis = unit_sides(unit);
	    if (lis != lispnil) {
		any_side_possible_units = TRUE;
	    }
	}
    }
    if (!country_is_complete(side, runs)) {
	if ((side->possible_units != lispnil && consp(side->possible_units))
	    || any_side_possible_units) {
	    /* Collect a list of possible units for this side. */
	    possunits = origpossunits = side->possible_units;
	    if (any_side_possible_units) {
		for_all_units(unit) {
		    lis = unit_sides(unit);
		    if ((stringp(lis) || symbolp(lis))
			&& side == find_side_by_name(c_string(lis))) {
			possunits = cons(new_number(unit->id), possunits);
		    } else if (listp(lis)) {
			for_all_list(lis, rest) {
			    head = car(rest);
			    if ((stringp(head) || symbolp(lis))
				&& side == find_side_by_name(c_string(head))) {
				possunits = cons(new_number(unit->id), possunits);
			    }
			    /* Note that we ignore bogus-looking data
			       in the list, it may be side info for
			       some other game. */
			}
		    } else {
			init_warning("Unit %s sides prop invalid, ignoring",
				     unit_desig(unit));
		    }
		}
	    }
	    tries = 1000;
	    /* (should pay attention to need for unit type before collecting) */
	    while (possunits != lispnil && !country_is_complete(side, runs)) {
		/* Just to ensure no infinite loop... */
		if (--tries < 0)
		  break;
		total = 0;
		chosen = choose_from_weighted_list(possunits, &total, FALSE);
		possunits = remove_from_list(chosen, possunits);
		if (Debug) {
		    Dprintf("For %s, chose ", side_desig(side));
		    Dprintlisp(chosen);
		    Dprintf(" from ");
		    Dprintlisp(origpossunits);
		    Dprintf(" possible units\n");
		}
		if (stringp(chosen)) {
		    unit = find_unit_by_name(c_string(chosen));
		} else if (numberp(chosen)) {
		    unit = find_unit(c_number(chosen));
		} else {
		    unit = NULL;
		}
		if (unit != NULL) {
		    set_unit_side(unit, side);
		    set_unit_origside(unit, side);
		    init_supply(unit);
		}
	    }
	}
    }
}

/* Compute the average number of units of a given type that are currently
   in the game. */

static int
average_numunits(int u)
{
    int count = 0, nsides = 0;
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	if (side->ingame) {
	    ++nsides;
	    for_all_side_units(side, unit) {
		if (in_play(unit) && unit->type == u)
		  ++count;
	    }
	}
    }
    return (count / (nsides > 1 ? nsides - 1 : 1));
}

/* Count the cell as being of a particular type. */

static void
count_cells(int x, int y)
{
    ++numcells[(int) terrain_at(x, y)];
}

/* Test whether a given location is desirable for a country.  It
   should be in the right distance range from other countries, and
   have enough of the right sorts of terrain. */

static int
good_place(int cx, int cy)
{
    int toofar = TRUE, notfirst = FALSE, px, py, dist, t;
    Side *side;

    /* Check the candidate position against the other countries' positions. */
    for_all_sides(side) {
	px = side->startx;  py = side->starty;
	if (inside_area(px, py)) {
	    notfirst = TRUE;
	    dist = distance(cx, cy, px, py);
	    /* Min separation default allows any min separation. */
	    if (dist < curmindistance)
	      return FALSE;
	    /* Default max separation says pos can never be too far away. */
	    if (curmaxdistance < 0 || dist < curmaxdistance)
	      toofar = FALSE;
	}
    }
    if (toofar && notfirst)
      return FALSE;
    /* Count the types of terrain in the country. */
    for_all_terrain_types(t)
      numcells[t] = 0;
    apply_to_area(cx, cy, tmpradius, count_cells);
    /* Check against our upper and lower limits on terrain. */
    for_all_terrain_types(t) {
    	if (numcells[t] < t_country_min(t))
    	  return FALSE;
	if (t_country_max(t) >= 0
	    && numcells[t] > t_country_max(t))
	  return FALSE;
    }
    return TRUE;
}

/* Work hard to find a place for a side's country.  First make some random
   trials, then start searching from the center of the area outwards.
   Then just pick a place and plan to patch up the terrain later. */

static void
find_a_place(Side *side)
{
    int tries, x, y, maxtries = area.numcells / 5;
    Unit *unit;

    Dprintf("%s country ", side_desig(side));
    /* First see if any of our units is already at a good location
       to put the whole country. */
    /* (should choose randomly from units first?) */
    for_all_side_units(side, unit) {
	if (in_play(unit) && good_place(unit->x, unit->y)) {
	    side->startx = unit->x;  side->starty = unit->y;
	    Dprintf("placed at unit %s\n", unit_desig(unit));
	    return;
	}
    }
    /* Then try any of the independent units.  Probability of choosing
       one is inversely proportional to the total number of units. */
    for_all_side_units(indepside, unit) {
	if (in_play(unit)
	    && good_place(unit->x, unit->y)
	    && probability(max(1, 100 / numunits))) {
	    side->startx = unit->x;  side->starty = unit->y;
	    Dprintf("placed at indep unit %s\n", unit_desig(unit));
	    return;
	}
    }
    /* Then try some random locations. */
    for (tries = 0; tries < maxtries; ++tries) {
	if (tries % 10 == 0) {
	    announce_progress(sideprogress +
			      ((sidedeltahalf / 2) * tries) / maxtries);
	}
	random_point(&x, &y);
	/* Filter out points that are right on the edge.  If that's
	   where the only valid starting places are, the exhaustive
	   search will still find them. */
	if (between(2, y, area.height - 2)
	    && between(2, x, area.width - 2)
	    && good_place(x, y)) { 
	    side->startx = x;  side->starty = y;
	    Dprintf("placed on try %d\n", tries);
	    return;
	}
    }
    /* Then search exhaustively, starting from the center of the area. */
    if (search_around(area.width / 2, area.halfheight,
		      area.width / 2 + area.halfheight,
		      good_place, &x, &y, 1)) {
	side->startx = x;  side->starty = y;
	Dprintf("placed after search\n");
	return;
    }
    /* This should be a place in the area no matter what. */
    random_point(&x, &y);
    side->startx = x;  side->starty = y;
    Dprintf("placed randomly\n");
    ++badcountryplaces;
    /* Since placement has become difficult, we get permission to alter
       the terrain if necessary, while placing units. */
    mungterrain = TRUE;
}    

/* The basic conditions that *must* be met by an initial unit placement. */

static int
valid_unit_place(int x, int y)
{
    int t = terrain_at(x, y);

    return (inside_area(x, y)
	    && ut_favored(tmputype, t) > 0
	    && type_survives_in_cell(tmputype, x, y)
	    && type_can_occupy_cell(tmputype, x, y));
}

static int
possible_unit_place(int x, int y)
{
    return (inside_area(x, y) && unit_at(x, y) == NULL);
}

/* Find a place somewhere in the designated area, following constraints
   on terrain and adjacency.  Returns success of the search. */

static int
find_unit_place(int u, int cx, int cy, int *xp, int *yp)
{
    int tries, x, y;
    int maxtries = tmpradius * tmpradius * 5;

    Dprintf("%s place found ", u_type_name(u));
    tmputype = u;
    for (tries = 0; tries < maxtries; ++tries) {
	if (random_point_near(cx, cy, tmpradius, &x, &y)) {
	    if (mungterrain
	    	&& !valid_unit_place(x, y)
		&& possible_unit_place(x, y)
		&& not_too_crowded(u, x, y)
		&& favorite[u] != NONTTYPE
		&& probability(ut_favored(u, favorite[u]))) {
		Dprintf("(by munging) ");
		mung_terrain(x, y, u);
	    }
	    if (valid_unit_place(x, y)
		&& not_too_crowded(u, x, y)
		&& probability(ut_favored(u, terrain_at(x, y)))) {
	    	*xp = x;  *yp = y;
		Dprintf("on try %d of %d\n", tries, maxtries);
		return TRUE;
	    }
	}
    }
    /* Random points aren't working, switch to exhaustive search. */
    if (search_around(cx, cy, tmpradius, valid_unit_place, &x, &y, 1)
	&& not_too_crowded(u, x, y)) {
	*xp = x;  *yp = y;
	Dprintf("after search\n");
	return TRUE;

    }
    /* Search again, just find a location that we can alter to suit. */
    if (favorite[u] != NONTTYPE) {
        if (search_around(cx, cy, tmpradius, possible_unit_place, &x, &y, 1)
	    && not_too_crowded(u, x, y)) {
             mung_terrain(x, y, u);
	    *xp = x;  *yp = y;
	    Dprintf("(by munging) after search\n");
	    return TRUE;
	}
    }
    Dprintf("- NOT!\n");
    return FALSE;
}

static void
mung_terrain(int x, int y, int u)
{
    int dir, x1, y1;

    /* Alter the terrain to be compatible with the given unit type. */
    set_terrain_at(x, y, favorite[u]);
    /* Mung some empty adjacent cells also, improves appearance. */
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    if (unit_at(x1, y1) == NULL && flip_coin()) {
		set_terrain_at(x1, y1, favorite[u]);
	    }
	}
    }
}

/* Find a unit for an occupant of the given type. */

static Unit *
find_occupant_place(Side *side, int u, int x0, int y0)
{
    Unit *transport;

    /* (should cast about randomly first, then do the full search) */
    for_all_side_units(side, transport) {
	if (is_unit(transport)
	    && inside_area(transport->x, transport->y)
	    && type_can_occupy(u, transport)) {
	    return transport;
	}
    }
    /* (should be able to place inside indep units in country) */
    return NULL;
}

static void
grant_initial_occs(Unit *unit)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int i = -1, imax = -1;

    assert(in_play(unit));
    if (!in_play(unit)) 
	init_error("Attempted to access an out-of-play unit");
    u = unit->type;
    side = unit->side;
    for_all_unit_types(u2) {
	if (!uu_complete_occs_on_completion(u, u2)
            && !uu_alt_complete_occs_on_completion(u, u2)
            && !uu_incomplete_occs_on_completion(u, u2)
            && !uu_alt_incomplete_occs_on_completion(u, u2))
            continue;
	if (!type_can_occupy(u2, unit)) {
            init_warning(
"Cannot fit any implicitly created %s units into just placed %s unit",
                        u_type_name(u2), u_type_name(u));
            continue;
        }
	// Determine number of complete occs-on-completion.
        imax = prob_fraction(uu_complete_occs_on_completion(u, u2));
        imax += max(0, roll_dice1(uu_alt_complete_occs_on_completion(u, u2)));
        // Try to add complete occs-on-completion.
        for (i = 0; i < imax; ++i) {
            if (!type_can_occupy(u2, unit)) {
                init_warning(
"Cannot fit any more implictly created %s units into just placed %s unit",
                            u_type_name(u2), u_type_name(u));
                break;
            }
	    place_unit(u2, side, unit->x, unit->y, unit, FALSE);
	} // for all complete occs
	// Determine number of incomplete occs-on-completion.
        imax = prob_fraction(uu_incomplete_occs_on_completion(u, u2));
        imax += max(0, roll_dice1(uu_alt_incomplete_occs_on_completion(u, u2)));
        // Try to add incomplete occs-on-completion.
        for (i = 0; i < imax; ++i) {
            if (!type_can_occupy(u2, unit)) {
                init_warning(
"Cannot fit any more implictly created %s units into just placed %s unit",
                            u_type_name(u2), u_type_name(u));
                break;
            }
	    place_unit(u2, side, unit->x, unit->y, unit, FALSE, FALSE);
	} // for all incomplete occs
    } // for all unit types
}

/* Given a type, position, and optional transport, put down a
   newly-created unit.  We assume the caller has already checked
   that the type is allowed to be owned by the given side. */

static void
place_unit(
    int u, Side *side, int x, int y, Unit *transport, 
    int announce, int complete)
{
    Unit *unit;

    unit = create_unit(u, TRUE);
    /* If we failed to create the unit, but didn't die due to memory
       exhaustion, we must have gone past a designer-specified limit
       on the numbers of units; so just return quietly. */
    if (unit == NULL)
      return;
    set_unit_side(unit, side);
    set_unit_origside(unit, side);
    init_supply(unit);
    if (!complete)
      unit->cp = 0;
    if (transport != NULL)
      enter_transport(unit, transport);
    else
      enter_cell(unit, x, y);
    grant_initial_occs(unit);
    if (announce)
	announce_progress(sideprogress
			  + sidedeltahalf
			  + (sidedeltahalf * (++totnumdone)) / totnumtodo);
}

/* Country expansion works by iterating over increasing distances from
   the initial location, adding compatible cells to the country. */

static void
expand_countries(void)
{
    int i, maxradius, growth, x, y, dir, x1, y1;
    int pop, pop2, majoritypop, broken;
    Side *side;

    announce_lengthy_process("Growing countries");
    maxradius = min(area.width, g_radius_max());
    for_all_sides(side)
      side->finalradius = maxradius;
    for (i = baseradius; i < maxradius; ++i) {
    	announce_progress((100 * (i - baseradius)) / (maxradius - baseradius));
	for_all_sides(side) {
	    /* If side still allowed to grow, expand it. */
	    if (side->finalradius == maxradius) {
	    	tmpside = side;
	    	growth = 0;
	    	apply_to_ring(side->startx, side->starty, i - 2, i,
			      expand_country);
		/* If no actual growth happened in a step, the country
		   might have reached its natural size. */
	    	if (growth == 0 && probability(g_growth_stop())) {
		    side->finalradius = i;
	    	}
	    }
	}
    }
    /* Do a couple "consolidation" steps to fill in "jaggies". */
    for (i = 0; i < 2; ++i) {
	for_all_sides(side) {
	    tmpside = side;
	    apply_to_ring(side->startx, side->starty,
			  baseradius, side->finalradius - 1,
			  expand_country);
	}
    }
    /* Also remove isolated populations surrounded by one other side. */
    if (people_sides_defined()) {
	for_all_interior_cells(x, y) {
	    if ((pop = people_side_at(x, y)) != NOBODY) {
		majoritypop = NOBODY;
		broken = FALSE;
		for_all_directions(dir) {
		    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			if ((pop2 = people_side_at(x1, y1)) == pop) {
			    broken = TRUE;
			    break;
			} else if (majoritypop == NOBODY) {
			    majoritypop = pop2;
			} else if (pop2 != majoritypop) {
			    broken = TRUE;
			    break;
			}
		    }
		}
		if (broken)
		  break;
		if (majoritypop != NOBODY)
		  set_people_side_at(x, y, majoritypop);
	    }
	}
    }
    finish_lengthy_process();
}

/* Expand a country into the given cell. */

static void
expand_country(int x, int y)
{
    int u, t = terrain_at(x, y), ours, theirs, prob, prob2;
    int advantage, sideadvantage;
    Unit *unit;
    
    if (people_side_at(x, y) == NOBODY) {
	prob = t_country_growth(t);
    } else {
	prob = t_country_takeover(t);
    }
    /* The cell should become known even if not occupied, but we can't
       do that here because view structures don't exist yet, so just
       leave for now. */
    if (!probability(prob))
      return;
    /* Claim the cell, attempt to own it. */
    ours = theirs = FALSE;
    if (unit_at(x, y) == NULL) {
	/* If completely empty, we claim it. */
	ours = TRUE;
	for_all_unit_types(u) {
	    tmputype = u;
	    sideadvantage = max(1, tmpside->advantage);
	    advantage = actual_advantage(tmpside);
	    /* Note that while theoretically this multiplication won't
	       work if the product is > 100%, in practice the growth
	       probs are going to be well under 10%. */
	    prob2 = advantage * u_unit_growth(u);
	    if (xrandom(10000) < prob2
		&& !at_country_units_max(tmpside, u)
		&& probability(ut_favored(u, t))
		&& valid_unit_place(x, y)
		&& not_too_crowded(u, x, y)) {
		unit = create_unit(u, TRUE);
		if (unit != NULL) {
		    set_unit_side(unit, tmpside);
		    set_unit_origside(unit, tmpside);
		    init_supply(unit);
		    enter_cell(unit, x, y);
		    break;
		}
	    }
	    prob2 = advantage * u_indep_growth(u);
	    if (xrandom(10000) < prob2
		&& !at_country_units_max(tmpside, u)
		&& probability(ut_favored(u, t))
		&& valid_unit_place(x, y)
		&& not_too_crowded(u, x, y)) {
		unit = create_unit(u, TRUE);
		if (unit != NULL) {
		    init_supply(unit);
		    enter_cell(unit, x, y);
		    break;
		}
	    }
	}
    } else {
	/* Somebody is here already. */
	for_all_stack(x, y, unit) {
	    if (unit->side != tmpside) {
		if (unit->side != NULL) {
		    /* Maybe take over another side's unit. */
		    if (xrandom(10000) < u_unit_takeover(unit->type)
			&& !at_country_units_max(tmpside, unit->type)
			) {
			set_unit_side(unit, tmpside);
			set_unit_origside(unit, tmpside);
			ours = TRUE;
		    } else {
			/* Indicate that we don't have possession of every
			   unit in this cell. */
			theirs = TRUE;
		    }
		} else {
		    /* Maybe take over an independent unit. */
		    if (xrandom(10000) < u_indep_takeover(unit->type)
			&& !at_country_units_max(tmpside, unit->type)
			) {
			set_unit_side(unit, tmpside);
			set_unit_origside(unit, tmpside);
			ours = TRUE;
		    }
		}
	    }
	}
    }
    /* If cell is claimed, and has no opposition, then country gets it. */
    if (ours && !theirs && probability(t_country_people(t))) {
	set_people_side_at(x, y, side_number(tmpside));
    }
    /* Count this cell as one we expanded into. */
    ++growth;
}

static int
at_country_units_max(Side *side, int u)
{
    int themax = u_country_units_max(u);

    if (themax < 0)
      return FALSE;
    /* (should adjust for advantage?) */
    return (themax <= side->numunits[u]);
}

/* A method that scatters independent units over the world. */

/* (should consider reviving the old rule that if indep density was
   nonzero, at least one unit of that type was guaranteed to be
   placed) */

int
make_independent_units(int calls, int runs)
{
    int u, t, x, y;
    int doanytype = FALSE, doindeppeople = FALSE;
    Unit *unit;

    /* Only ever run this once. */
    if (runs > 0)
      return FALSE;
    /* Can't do anything without some terrain. */
    if (!terrain_defined())
      return FALSE;
    /* Decide which types will be put down. */
    for_all_unit_types(u) {
    	tmp_u_array[u] = FALSE;
    	for_all_terrain_types(t) {
    	    if (ut_indep_density(u, t) > 0 && !ut_vanishes_on(u, t)) {
    	    	tmp_u_array[u] = TRUE;
    	    	doanytype = TRUE;
    	    	break;
    	    }
    	}
    }
    /* Make space for people sides if we're going to have any. */
    if (!people_sides_defined()) {
	for_all_terrain_types(t) {
	    if (t_indep_people(t) > 0)
	      doindeppeople = TRUE;
	}
	if (doindeppeople) {
	    allocate_area_people_sides();
	}
    }
    /* If no units or peoples to do, don't waste time going through
       the world. */
    if (!doanytype && !doindeppeople)
      return FALSE;
    announce_lengthy_process("Making independents");
    /* Now apply the process to each cell individually. */
    for_all_interior_cells(x, y) {
    	/* Progress is approx proportional to "column" being worked on. */
    	if (y == 1 && x % 5 == 0)
    	  announce_progress((x * 100) / area.width);
	t = terrain_at(x, y);
	for_all_unit_types(u) {
	    if (tmp_u_array[u]) {
		if (ut_indep_density(u, t) > xrandom(10000)) {
	    	    if (type_survives_in_cell(u, x, y)
			&& type_can_occupy_cell(u, x, y)
			&& not_too_crowded(u, x, y)) {
			unit = create_unit(u, TRUE);
			if (unit != NULL) {
			    init_supply(unit);
			    enter_cell(unit, x, y);
		    	}
		    }
		}
	    }
	}
	if (doindeppeople
	    /* Don't put indeps on top of people already on a side. */
	    && people_side_at(x, y) == NOBODY
	    && probability(t_indep_people(t))) {
	    /* 0 represents indep people present */
	    set_people_side_at(x, y, 0);
	}
    }
    finish_lengthy_process();
    return TRUE;
}

/* Set the people inside the country to be on the country's side.
   Flip a coin along the edge of the country, to make the border more
   interesting.  Also don't always overwrite existing people. */

static void
set_people_on_side(int x, int y)
{
    if ((distance(tmpside->startx, tmpside->starty, x, y) <= tmpradius
         || flip_coin())
        && probability(t_country_people(terrain_at(x, y)))
        && (people_side_at(x, y) == NOBODY || flip_coin())) {
	set_people_side_at(x, y, side_number(tmpside));
    }
}

/* Check if minimal unit separation rules permit us to put a new
unit at (x, y). */

static int
not_too_crowded(int u, int x, int y)
{
	int n = 0, x1, y1, u2, dist;
	Unit *unit;
	
	/* First check how far we need to look. */
	for_all_unit_types(u2) {
		/* In the unlikely case that somebody made an asymmetric table, 
		we check the relation both ways ... */
		n = max(n, max(uu_separation_min(u, u2), uu_separation_min(u2, u)));
	}
	/* Then scan the surrounding cells for potentially blocking units. */
	for_all_cells_within_range(x, y, n, x1, y1) {
		if (!inside_area(x1, y1))
		    continue;
		if (!unit_at(x1, y1))
		    continue;
		dist = distance(x, y, x1, y1);
		for_all_stack_with_occs(x1, y1, unit) {
			/* Again, we check the relation both ways ... */
			if (uu_separation_min(u, unit->type) > dist
			    || uu_separation_min(unit->type, u) > dist) {
				return FALSE;
			}
		}
	}	
	return TRUE;	
}

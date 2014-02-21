/* Road generation for Xconq.
   Copyright (C) 1991-1997, 1999-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* A road generation method that connects specified types of units
   and favors specified types of terrain. */

/* (should add some favoring of nearby rather than totally random units?) */

#include "conq.h"

static int unit_sees_other_unit(Unit *unit1, Unit *unit2);
static int road_at(int x, int y);
static int find_adj_inside_area(int x, int y, int *xp, int *yp);
static void lay_road_between(int x1, int y1, int x2, int y2);
static int test_road_segment(int x, int y, int dir);
static int sort_road_segments(int x, int y, int *dirchoices, int numchoices);
static int compare_road_directions(const void *a0, const void *a1);
static int lay_road_segment(int x, int y, int dir, int choice, int numchoices);

static short roadtype;

static int *doroads;

static short terrs[NUMDIRS];

/* Note that these arrays have to be int-sized. */

static int xs[NUMDIRS], ys[NUMDIRS];

int
make_roads(int calls, int runs)
{
    int doanyroads = FALSE, keyval;
    int i, n, t, u1, u2, ex1, ey1, ex2, ey2, iex1, iey1, iex2, iey2, rx, ry;
    Unit *unit1, *unit2;

    /* Look for a suitable terrain type. */
    /* (should add ability to run on several road types) */
    roadtype = NONTTYPE;
    keyval = c_number(symbol_value(intern_symbol(keyword_name(K_ROAD_X))));
    for_all_terrain_types(t) {
	if (t_is_connection(t)
	    && !aux_terrain_defined(t)
	    && t_subtype_x(t) == keyval) {
	    roadtype = t;
	    break;
	}
    }
    if (roadtype == NONTTYPE)
      return FALSE;
    if (doroads == NULL)
      doroads = (int *) xmalloc(numutypes * sizeof(int));
    /* Compute which unit types will have roads. */
    for_all_unit_types(u1)
      doroads[u1] = FALSE;
    for_all_unit_types(u1) {
	for_all_unit_types(u2) {
	    if (uu_road_chance(u1, u2) > 0) {
		doroads[u1] = TRUE;
		doanyroads = TRUE;
		break;
	    }
	}
	if (u_road_to_edge_chance(u1) > 0) {
	    doroads[u1] = TRUE;
	    doanyroads = TRUE;
	}
    }
    if (g_edge_road_density() > 0)
      doanyroads = TRUE;
    /* Don't run if road chance always zero. */
    if (!doanyroads)
      return FALSE;
    /* Now we're ready to get down to work. */
    allocate_area_aux_terrain(roadtype);
    announce_lengthy_process("Laying down roads");
    /* (should be able to announce progress by guessing at number of roads) */
    /* Run roads between units. */
    for_all_units(unit1) {
	if (in_play(unit1) && doroads[unit1->type]) {
	    for_all_units(unit2) {
		if (unit1 != unit2
		    && in_play(unit2)
		    && (g_see_all()
			|| unit1->side == unit2->side
			|| (unit_sees_other_unit(unit1, unit2)
			    && unit_sees_other_unit(unit2, unit1)))
		    && probability(uu_road_chance(unit1->type, unit2->type))) {
		    lay_road_between(unit1->x, unit1->y, unit2->x, unit2->y);
		}
	    }
	}
    }
    /* Run roads from units to the edge of the world. */
    for_all_units(unit1) {
	if (in_play(unit1) && doroads[unit1->type]) {
	    if (probability(u_road_to_edge_chance(unit1->type))) {
		random_edge_point(&ex1, &ey1);
		lay_road_between(unit1->x, unit1->y, ex1, ey1);
	    }
	}
    }
    /* Run roads that go from one edge to another. */
    if (g_edge_road_density() > 0) {
	n = (g_edge_road_density() * (2 * (area.width + area.height))) / 10000;
	/* Ensure at least one road. */
	n = max(n, 1);
	for (i = 0; i < n; ++i) {
	    random_edge_point(&ex1, &ey1);
	    find_adj_inside_area(ex1, ey1, &iex1, &iey1);
	    random_edge_point(&ex2, &ey2);
	    find_adj_inside_area(ex2, ey2, &iex2, &iey2);
	    lay_road_between(iex1, iey1, iex2, iey2);
	}
    }
    /* Run spurs from unroaded units to existing roads. */
    for_all_units(unit1) {
	if (in_play(unit1)
	    && !road_at(unit1->x, unit1->y)
	    && u_spur_chance(unit1->type) > 0
	    && u_spur_range(unit1->type) > 0) {
	    if (probability(u_spur_chance(unit1->type))) {
		if (search_around(unit1->x, unit1->y, u_spur_range(unit1->type),
				  road_at, &rx, &ry, 1)) {
		    lay_road_between(unit1->x, unit1->y, rx, ry);
		}
	    }
	}
    }
    finish_lengthy_process();
    return TRUE;
}

/* Test whether we expect that units on different sides can see each
   other.  This has special cases so this works during game setup,
   when the usual unit view layers may not be accurate yet. */

static int
unit_sees_other_unit(Unit *unit1, Unit *unit2)
{
    Side *side = unit1->side;

    /* Independent units are considered to know about all other units
       (not realistic, but no one will notice, since sides with players
       must also be able to see the indep unit before they will have
       a road going to it) */
    if (indep(unit1))
      return TRUE;
    /* Run a normal unit view test to see if we see anything. */
    if (unit_view_at(side, unit2->x, unit2->y) != NULL)
      return TRUE;
    /* If the unit view hasn't been set up yet, but we anticipate that the
       unit in question will be visible. */
    if (side->terrview != NULL
	&& terrain_view(side, unit2->x, unit2->y) != UNSEEN
	&& (u_see_always(unit2->type) || already_seen_chance(side, unit2) >= 100))
      return TRUE;
    /* The people will report all units. */
    if (people_sides_defined()
	&& people_side_at(unit2->x, unit2->y) == side->id)
      return TRUE;
    return FALSE;
}

static int
road_at(int x, int y)
{
    return aux_terrain_at(x, y, roadtype);
}

/* Find an interior point adjacent to this one (presumed but not
   required to be on the edge).  This one just picks the first found,
   which is OK for the road generation usage. */

static int
find_adj_inside_area(int x, int y, int *xp, int *yp)
{
    int dir, x1, y1;

    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    *xp = x1;  *yp = y1;
	    return TRUE;
	}
    }
    return FALSE;
}

/* Do the whole path. */

static short ultx, ulty;

static void
lay_road_between(int x1, int y1, int x2, int y2)
{
    ultx = x2;  ulty = y2;
    apply_to_path(x1, y1, x2, y2, test_road_segment, sort_road_segments,
		  lay_road_segment, FALSE);
}

static int
test_road_segment(int x, int y, int dir)
{
    int x1, y1;

    if (point_in_dir(x, y, dir, &x1, &y1)) {
	return (tt_road_into_chance(terrain_at(x, y), terrain_at(x1, y1)) > 0);
    }
    return FALSE;
}

/* Given a location and a list of directions, sort the directions in
   order of desirability. */

static short basex, basey;

static int
sort_road_segments(int x, int y, int *dirchoices, int numchoices)
{
    int i, tmp, i0 = 0, i1 = 1, compar;

    basex = x;  basey = y;
    if (numchoices > 1) {
	/* Cache direction and terrain data. */
	tmpttype = terrain_at(x, y);
	for (i = 0; i < numchoices; ++i) { 
	    point_in_dir(x, y, dirchoices[i], &(xs[i]), &(ys[i]));
	    terrs[i] = terrain_at(xs[i], ys[i]);
	}
	if (numchoices == 2) {
	    compar = compare_road_directions(&i0, &i1);
	    if (compar > 0 || (compar == 0 && flip_coin())) {
		tmp = dirchoices[0];
		dirchoices[0] = dirchoices[1];
		dirchoices[1] = tmp;
	    }
	} else if (numchoices > 2) {
	    qsort(dirchoices, numchoices, sizeof(int),
		  compare_road_directions);
	    /* Randomize the top two choices if they're equivalent. */
	    if (compare_road_directions(&i0, &i1) == 0 && flip_coin()) {
		tmp = dirchoices[0];
		dirchoices[0] = dirchoices[1];
		dirchoices[1] = tmp;
	    }
	}
    }
    return numchoices;
}

/* This compares the desirability of two different directions.  This
   is somewhat tricky, because it should return < 0 if i0 designates a
   BETTER direction than i1. */

static int
compare_road_directions(CONST void *a0, CONST void *a1)
{
    int i0, i1, t0, t1, tmp0, tmp1, chance0, chance1, sum0, sum1, dir, x1, y1;
    int dist0, dist1, dist, chance;
    Unit *unit;

    i0 = *((int *) a0);  i1 = *((int *) a1);
    t0 = terrs[i0];  t1 = terrs[i1];
    /* Prefer the direction that already has a road. */
    tmp0 = (aux_terrain_at(xs[i0], ys[i0], roadtype) ? 1 : 0);
    tmp1 = (aux_terrain_at(xs[i1], ys[i1], roadtype) ? 1 : 0);
    if (tmp0 != tmp1)
      return (tmp1 - tmp0);
    /* Prefer the direction that already has a road-connected unit. */
    tmp0 = tmp1 = 0;
    for_all_stack(xs[i0], ys[i0], unit) {
	if (doroads[unit->type]) {
	    tmp0 = 1;
	    break;
	}
    }
    for_all_stack(xs[i1], ys[i1], unit) {
	if (doroads[unit->type]) {
	    tmp1 = 1;
	    break;
	}
    }
    if (tmp0 != tmp1)
      return (tmp1 - tmp0);
    /* Prefer the direction whose terrain is more favorable for a road. */
    dist0 = distance(xs[i0], ys[i0], ultx, ulty);
    chance0 = tt_road_into_chance(tmpttype, t0);
    sum0 = chance0;
    for_all_directions(dir) {
	if (interior_point_in_dir(xs[i0], ys[i0], dir, &x1, &y1)) {
	    dist = distance(x1, y1, ultx, ulty);
	    if (dist <= dist0) {
		sum0 += chance0;
		chance = tt_road_into_chance(t0, terrain_at(x1, y1));
		sum0 += (dist < dist0 ? 3 : 1) * chance;
	    }
	}
    }
    dist1 = distance(xs[i1], ys[i1], ultx, ulty);
    chance1 = tt_road_into_chance(tmpttype, t1);
    sum1 = chance1;
    for_all_directions(dir) {
	if (interior_point_in_dir(xs[i1], ys[i1], dir, &x1, &y1)) {
	    dist = distance(x1, y1, ultx, ulty);
	    if (dist <= dist1) {
		sum1 += chance1;
		chance = tt_road_into_chance(t1, terrain_at(x1, y1));
		sum1 += (dist < dist1 ? 3 : 1) * chance;
	    }
	}
    }
#if 0 /* use for debugging lookahead */
    if (Debug) {
	int weighta, weightb, signa, signb;

	weighta = chance1 - chance0;
	signa = (weighta > 0 ? 1 : (weighta < 0 ? -1 : 0));
	weightb = sum1 - sum0;
	signb = (weightb > 0 ? 1 : (weightb < 0 ? -1 : 0));
	if (signa != signb) {
	    Dprintf("Adj terrain affects road choice at %d,%d;\n", basex, basey);
	    Dprintf("  to %d,%d base weight is %d, adj terrain makes %d\n", xs[i0], ys[i0], chance0, sum0);
	    Dprintf("  to %d,%d base weight is %d, adj terrain makes %d\n", xs[i1], ys[i1], chance1, sum1);
	}
    }
#endif
    return (sum1 - sum0);
}

/* Lay down a single bit of road, tending to choose directions with more
   favorable underlying terrain and merging with already-existing roads. */

static int
lay_road_segment(int x, int y, int d, int choice, int numchoices)
{
    int x1, y1, roadchance, foundroad = FALSE, foundunit = FALSE;
    Unit *unit;

    if (interior_point_in_dir(x, y, d, &x1, &y1)) {
	/* See if any road already in the other end of this conn. */
	if (aux_terrain_at(x1, y1, roadtype))
	  foundroad = TRUE;
	/* See if any road-connected units there. */
	for_all_stack(x1, y1, unit) {
	    if (doroads[unit->type]) {
		foundunit = TRUE;
		break;
	    }
	}
    	roadchance = tt_road_into_chance(terrain_at(x, y), terrain_at(x1, y1));
        if (roadchance > 0) {
            if (probability(roadchance) || foundroad || foundunit) {
		set_connection_at(x, y, d, roadtype, TRUE);
		/* Return whether to keep going or stop because of
                   shared road. */
		/* Note that stopping at a road-connected unit is not
		   desirable, because we know (foundroad being false),
		   that it does not already have a road running to it,
		   and so we need to finish getting this one to where
		   it is supposed to go. */
		return (foundroad ? -1 : 1);
	    }
	}
    }
    /* Try next. */
    return 0;
}

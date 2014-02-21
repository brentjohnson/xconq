/* Tactical Analysis for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Tactical Analysis for AIs

    Part of the AI API, Level 3. 

    Provides tactical level AI implementation.

    \note Nothing in this file should be required for AI implementation.

*/

#include "conq.h"
#include "kernel.h"
#include "kpublic.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"

namespace Xconq {
namespace AI {

/* Task Inspection */

int
n_plan_to_construct_at(Side *side, int x, int y, short *p_utypes)
{
    int n = 0;
    Unit *unit = NULL;
    Task *agenda = NULL;
    Side *side2 = NULL;
    int u2 = NONUTYPE;

    assert_error(side,
		 "AI Task Inspector: Attempted to access a NULL side");
    assert_warning_return(inside_area(x, y),
			  "AI Task Inspector: Inquiry about out-of-arena cell",
			   0);
    for_all_sides(side2) {
	if ((side2 != side) && !allied_side(side2, side))
	    continue;
	for_all_side_units(side2, unit) {
	    agenda = (unit->plan ? unit->plan->tasks : NULL);
	    if (!agenda)
		continue;
	    // If movement involved, see what next task is.
	    if ((agenda->type == TASK_MOVE_TO) || (agenda->type == TASK_OCCUPY))
		agenda = agenda->next;
	    if (!agenda)
		continue;
	    if (agenda->type != TASK_CONSTRUCT)
		continue;
	    // Skip, if construction to occur in transport.
	    if (agenda->args[2] != -1)
		continue;
	    // If construction location matches location of interest...
	    if ((agenda->args[3] == x) && (agenda->args[4] == y)) {
		if (p_utypes) {
		    for_all_unit_types(u2) {
			if (p_utypes[u2] && (u2 == agenda->args[0])) {
			    ++n;
			    break;
			}
		    }
		}
		else
		    ++n;
	    }
	}
    }
    return n;
}

/* Resource Contention Assessment */

int
resource_contention_with_any(int u, int x, int y, Side *side)
{
    static short *p_advanced;

    Side *side2 = NULL;
    Unit *unit2 = NULL;
    int ux = -1, uy = -1, dist = -1;
    int userid = -1;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u),
"AI Resource Contention Assessor: Encountered invalid unit type");
    assert_error(side,
"AI Resource Contention Assessor: Attempted to access a NULL side");
    assert_error(inside_area(x, y),
"AI Resource Contention Assessor: Attempted to access resource contention " \
"at an out-of-area location");

    // Initialize advanced utypes array, if necessary.
    // TODO: Should already have this precalculated somewhere.
    if (!p_advanced) {
	p_advanced = (short *)xmalloc(numutypes * sizeof(short));
	for_all_unit_types(u2)
	    p_advanced[u2] = u_advanced(u2);
    }
    // Inspect all friendly units for contention.
    for_all_sides(side2) {
	if ((side2 != side) && !allied_side(side2, side))
	    continue;
	// Is anyone using or will be using the cell?
	if (user_defined() && u_advanced(u)) {
	    // TODO: Should consider reach of highest immobile utype on 
	    //	utype's evolutionary ladder.
	    for_all_cells_within_range(x, y, u_reach(u), ux, uy) {
		if (!inside_area(ux, uy))
		    continue;
		if (n_plan_to_construct_at(side2, ux, uy, p_advanced))
		    return TRUE;
		userid = user_at(ux, uy);
		if (NOUSER == userid)
		    continue;
		// TODO: Need user views so that we do not inspect enemy usage.
		unit2 = find_unit(userid);
		if (!in_play(unit2))
		    continue;
		if (!allied_side(unit2->side, side))
		    continue;
		return TRUE;
	    } // for all cells within range
	}
	// Are min separation distance reqs met?
	for_all_side_units(side2, unit2) {
	    u2 = unit2->type;
	    dist = distance(x, y, unit2->x, unit2->y);
	    // TODO: Make separation distance be TableUU rather than gvar.
	    if (u_advanced(u) && u_advanced(u2)) {
		if (dist < g_ai_advanced_unit_separation())
		    return TRUE;
	    }
	}
    } // for all sides
    return FALSE;
}

/* Offensive/Defensive Assessment */

int
choose_best_hit_method(
    int hitmethod, int fhm, int ahm, int dhm, int ftm, int atm, int dtm)
{
    if (HIT_METHOD_NONE == hitmethod)
	return hitmethod;
    // Filter out any valid hit methods not good enough.
    if (INT_MAX == ftm)
	hitmethod &= ~HIT_METHOD_FIRE;
    if (INT_MAX == atm)
	hitmethod &= ~HIT_METHOD_ATTACK;
    if (INT_MAX == dtm)
	hitmethod &= ~HIT_METHOD_DETONATE;
    // Determine best hit method.
    if (hitmethod & HIT_METHOD_FIRE) {
	if ((ftm > atm) || ((ftm == atm) && (fhm > ahm)))
	    hitmethod &= ~HIT_METHOD_FIRE;
	if ((ftm > dtm) || ((ftm == dtm) && (fhm > dhm)))
	    hitmethod &= ~HIT_METHOD_FIRE;
    }
    if (hitmethod & HIT_METHOD_ATTACK) {
	if ((atm > ftm) || ((atm == ftm) && (ahm > fhm)))
	    hitmethod &= ~HIT_METHOD_ATTACK;
	if ((atm > dtm) || ((atm == dtm) && (ahm > dhm)))
	    hitmethod &= ~HIT_METHOD_ATTACK;
    }
    if (hitmethod & HIT_METHOD_DETONATE) {
	if ((dtm > ftm) || ((dtm == ftm) && (dhm > fhm)))
	    hitmethod &= ~HIT_METHOD_DETONATE;
	if ((dtm > atm) || ((dtm == atm) && (dhm > ahm)))
	    hitmethod &= ~HIT_METHOD_DETONATE;
    }
    // If both fire and attack are tied, then choose one or the other.
    if ((hitmethod & HIT_METHOD_FIRE) && (hitmethod & HIT_METHOD_ATTACK)) {
	if (flip_coin())
	    hitmethod = HIT_METHOD_FIRE;
	else
	    hitmethod = HIT_METHOD_ATTACK;
    }
    // Cleanse all other bits.
    if (hitmethod & HIT_METHOD_FIRE)
	hitmethod = HIT_METHOD_FIRE;
    if (hitmethod & HIT_METHOD_ATTACK)
	hitmethod = HIT_METHOD_ATTACK;
    if (hitmethod & HIT_METHOD_DETONATE)
	hitmethod = HIT_METHOD_DETONATE;
    return hitmethod;
} 

int
choose_best_hit_method(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int dist = -1;
    int hitmethod = HIT_METHOD_NONE;
    int fhm = INT_MAX, ahm = INT_MAX, dhm = INT_MAX;
    int ftm = INT_MAX, atm = INT_MAX, dtm = INT_MAX;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    if (!is_active(unit))
	return HIT_METHOD_NONE;
    u = unit->type;
    u2 = uview->type;
    dist = distance(unit->x, unit->y, uview->x, uview->y);
    // Determine which hit methods are valid.
    hitmethod |= ((dist <= fire_range(u, u2)) ? HIT_METHOD_FIRE : 0);
    hitmethod |= ((dist <= attack_range(u, u2)) ? HIT_METHOD_ATTACK : 0);
    hitmethod |= ((dist <= detonate_urange(u, u2)) ? HIT_METHOD_DETONATE : 0);
    // Gather some info on each hit valid hit method.
    if (hitmethod & HIT_METHOD_FIRE) {
	fhm = firings_mean_to_destroy(u, uview);
	ftm = fire_turns_mean_to_destroy(u, uview);
    }
    if (hitmethod & HIT_METHOD_ATTACK) {
	ahm = attacks_mean_to_destroy(u, uview);
	atm = attack_turns_mean_to_destroy(u, uview);
    }
    if (hitmethod & HIT_METHOD_DETONATE) {
	dhm = detonations_mean_to_destroy(u, uview);
	dtm = detonate_turns_mean_to_destroy(u, uview);
    }
    return 
	choose_best_hit_method(hitmethod, fhm, ahm, dhm, ftm, atm, dtm);
}

int
guess_best_hit_method_used_by(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int dist = -1;
    int hitmethod = HIT_METHOD_NONE;
    int fhm = INT_MAX, ahm = INT_MAX, dhm = INT_MAX;
    int ftm = INT_MAX, atm = INT_MAX, dtm = INT_MAX;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    dist = distance(unit->x, unit->y, uview->x, uview->y);
    // Determine which hit methods are valid.
    hitmethod |= ((dist <= fire_range(u2, u)) ? HIT_METHOD_FIRE : 0);
    hitmethod |= ((dist <= attack_range(u2, u)) ? HIT_METHOD_ATTACK : 0);
    hitmethod |= ((dist <= detonate_urange(u2, u)) ? HIT_METHOD_DETONATE : 0);
    // Gather some info on each valid hit method.
    if (hitmethod & HIT_METHOD_FIRE) {
	fhm = firings_mean_to_destroy(u2, unit);
	ftm = fire_turns_mean_to_destroy(u2, unit);
    }
    if (hitmethod & HIT_METHOD_ATTACK) {
	ahm = attacks_mean_to_destroy(u2, unit);
	atm = attack_turns_mean_to_destroy(u2, unit);
    }
    if (hitmethod & HIT_METHOD_DETONATE) {
	dhm = detonations_mean_to_destroy(u2, unit);
	dtm = detonate_turns_mean_to_destroy(u2, unit);
    }
    return 
	choose_best_hit_method(hitmethod, fhm, ahm, dhm, ftm, atm, dtm);
}

int
choose_best_capture_method(
    int capmethod, int fcm, int acm, int dccm, int ftm, int atm, int dctm)
{
    if (CAPTURE_METHOD_NONE == capmethod)
	return capmethod;
    // Filter out any valid hit methods not good enough.
    if (INT_MAX == fcm)
	capmethod &= ~CAPTURE_METHOD_FIRE;
    if (INT_MAX == acm)
	capmethod &= ~CAPTURE_METHOD_ATTACK;
    if (INT_MAX == dccm)
	capmethod &= ~CAPTURE_METHOD_CAPTURE;
    // Determine best hit method.
    if (capmethod & CAPTURE_METHOD_FIRE) {
	if ((ftm > atm) || ((ftm == atm) && (fcm > acm)))
	    capmethod &= ~CAPTURE_METHOD_FIRE;
	if ((ftm > dctm) || ((ftm == dctm) && (fcm > dccm)))
	    capmethod &= ~CAPTURE_METHOD_FIRE;
    }
    if (capmethod & CAPTURE_METHOD_ATTACK) {
	if ((atm > ftm) || ((atm == ftm) && (acm > fcm)))
	    capmethod &= ~CAPTURE_METHOD_ATTACK;
	if ((atm > dctm) || ((atm == dctm) && (acm > dccm)))
	    capmethod &= ~CAPTURE_METHOD_ATTACK;
    }
    if (capmethod & CAPTURE_METHOD_DETONATE) {
	if ((dctm > ftm) || ((dctm == ftm) && (dccm > fcm)))
	    capmethod &= ~CAPTURE_METHOD_DETONATE;
	if ((dctm > atm) || ((dctm == atm) && (dccm > acm)))
	    capmethod &= ~CAPTURE_METHOD_DETONATE;
    }
    // Always prefer direct capture.
    if (capmethod & CAPTURE_METHOD_CAPTURE)
	capmethod = CAPTURE_METHOD_CAPTURE;
    // If both fire and attack are tied, then choose one or the other.
    if ((capmethod & CAPTURE_METHOD_FIRE) 
	&& (capmethod & CAPTURE_METHOD_ATTACK)) {
	if (flip_coin())
	    capmethod = CAPTURE_METHOD_FIRE;
	else
	    capmethod = CAPTURE_METHOD_ATTACK;
    }
    // Cleanse all other bits.
    if (capmethod & CAPTURE_METHOD_FIRE)
	capmethod = CAPTURE_METHOD_FIRE;
    if (capmethod & CAPTURE_METHOD_ATTACK)
	capmethod = CAPTURE_METHOD_ATTACK;
    return capmethod;
}

int
choose_best_capture_method(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side2 = NULL;
    int dist = -1;
    int capmethod = CAPTURE_METHOD_NONE;
    int fcm = INT_MAX, acm = INT_MAX, dccm = INT_MAX;
    int ftm = INT_MAX, atm = INT_MAX, dctm = INT_MAX;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    if (!is_active(unit))
	return HIT_METHOD_NONE;
    u = unit->type;
    u2 = uview->type;
    side2 = side_n(uview->siden);
    dist = distance(unit->x, unit->y, uview->x, uview->y);
    // Determine which capture methods are valid.
    capmethod |= ((dist <= fire_range(u, u2)) ? CAPTURE_METHOD_FIRE : 0);
    capmethod |= ((dist <= attack_range(u, u2)) ? CAPTURE_METHOD_ATTACK : 0);
    // TODO: Consider detonation as capture method.
    capmethod |= 
	((dist <= direct_capture_range(u, u2, side2)) 
	    ? CAPTURE_METHOD_CAPTURE : 0);
    // Gather some info on each valid capture method.
    if (capmethod & CAPTURE_METHOD_FIRE) {
	fcm = firings_mean_to_capture(u, uview);
	ftm = fire_turns_mean_to_capture(u, uview);
    }
    if (capmethod & CAPTURE_METHOD_ATTACK) {
	acm = attacks_mean_to_capture(u, uview);
	atm = attack_turns_mean_to_capture(u, uview);
    }
    if (capmethod & CAPTURE_METHOD_CAPTURE) {
	dccm = attempts_mean_to_capture(u, uview);
	dctm = capture_turns_mean_to_capture(u, uview);
    }
    return 
	choose_best_capture_method(capmethod, fcm, acm, dccm, ftm, atm, dctm);
}

int
guess_best_capture_method_used_by(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int dist = -1;
    int capmethod = CAPTURE_METHOD_NONE;
    int fcm = INT_MAX, acm = INT_MAX, dccm = INT_MAX;
    int ftm = INT_MAX, atm = INT_MAX, dctm = INT_MAX;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    side = unit->side;
    dist = distance(unit->x, unit->y, uview->x, uview->y);
    // Determine which capture methods are valid.
    capmethod |= ((dist <= fire_range(u2, u)) ? CAPTURE_METHOD_FIRE : 0);
    capmethod |= ((dist <= attack_range(u2, u)) ? CAPTURE_METHOD_ATTACK : 0);
    // TODO: Consider detonation as capture method.
    capmethod |= 
	((dist <= direct_capture_range(u2, u, side)) 
	    ? CAPTURE_METHOD_CAPTURE : 0);
    // Gather some info on each valid capture method.
    if (capmethod & CAPTURE_METHOD_FIRE) {
	fcm = firings_mean_to_capture(u2, unit);
	ftm = fire_turns_mean_to_capture(u2, unit);
    }
    if (capmethod & CAPTURE_METHOD_ATTACK) {
	acm = attacks_mean_to_capture(u2, unit);
	atm = attack_turns_mean_to_capture(u2, unit);
    }
    if (capmethod & CAPTURE_METHOD_CAPTURE) {
	dccm = attempts_mean_to_capture(u2, unit);
	dctm = capture_turns_mean_to_capture(u2, unit);
    }
    return 
	choose_best_capture_method(capmethod, fcm, acm, dccm, ftm, atm, dctm);
}

int
turns_to_defeat_best_ratio(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int hitmethod = HIT_METHOD_NONE, hitmethod2 = HIT_METHOD_NONE;
    int capmethod = CAPTURE_METHOD_NONE, capmethod2 = CAPTURE_METHOD_NONE;
    int ht = INT_MAX, ht2 = INT_MAX, ct = INT_MAX, ct2 = INT_MAX;
    int dt = INT_MAX, dt2 = INT_MAX;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    hitmethod = choose_best_hit_method(unit, uview);
    hitmethod2 = guess_best_hit_method_used_by(unit, uview);
    capmethod = choose_best_capture_method(unit, uview);
    capmethod2 = guess_best_capture_method_used_by(unit, uview);
    switch (hitmethod) {
      case HIT_METHOD_FIRE:
	ht = fire_turns_mean_to_destroy(u, uview);
	break;
      case HIT_METHOD_ATTACK:
	ht = attack_turns_mean_to_destroy(u, uview);
	break;
      case HIT_METHOD_DETONATE:
	ht = detonate_turns_mean_to_destroy(u, uview);
	break;
      default:
	ht = INT_MAX;
    }
    switch (hitmethod2) {
      case HIT_METHOD_FIRE:
	ht2 = fire_turns_mean_to_destroy(u2, unit);
	break;
      case HIT_METHOD_ATTACK:
	ht2 = attack_turns_mean_to_destroy(u2, unit);
	break;
      case HIT_METHOD_DETONATE:
	ht2 = detonate_turns_mean_to_destroy(u2, unit);
	break;
      default:
	ht2 = INT_MAX;
    }
    switch (capmethod) {
      case CAPTURE_METHOD_FIRE:
	ct = fire_turns_mean_to_capture(u, uview);
	break;
      case CAPTURE_METHOD_ATTACK:
	ct = attack_turns_mean_to_capture(u, uview);
	break;
      case CAPTURE_METHOD_CAPTURE:
	ct = capture_turns_mean_to_capture(u, uview);
	break;
      default:
	ct = INT_MAX;
    }
    switch (capmethod2) {
      case CAPTURE_METHOD_FIRE:
	ct2 = fire_turns_mean_to_capture(u2, unit);
	break;
      case CAPTURE_METHOD_ATTACK:
	ct2 = attack_turns_mean_to_capture(u2, unit);
	break;
      case CAPTURE_METHOD_CAPTURE:
	ct2 = capture_turns_mean_to_capture(u2, unit);
	break;
      default:
	ct2 = INT_MAX;
    }
    dt = min(ht, ct);
    dt2 = min(ht2, ct2);
    return dt / dt2;
}

/* Threat Assessment and Handling */

int
seems_safe_against(Unit *unit, UnitView *uview)
{
    Side *side = NULL, *side2 = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int dist = -1;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    side = unit->side;
    side2 = side_n(uview->siden);
    dist = distance(unit->x, unit->y, uview->x, uview->y);
    if ((side == side2) && (unit == query_unit_from_uview(uview)))
	return TRUE;
    // TODO: Figure out how to deal with transports of explosive units.
    //	Probably need to consider relative worths.
    if (allied_side(side, side2) && (dist > detonate_urange(u2, u)))
	return TRUE;
    if (dist > hit_range(u2, u))
	return TRUE;
    if (!could_hit(u2, u))
	return TRUE;
    if (!could_damage(u2, u))
	return TRUE;
    // TODO: Maybe u2 cannot destroy u, but what about effects on ACP, etc...?
    //	We need to go through the ACP and speed damage inerpolation lists, 
    //	and determine if we could be left incapacitated or immobilized.
    //	Also, we need to consider if we detonate when hit, 
    //	and if and how this affects occs, neighbors, etc....
    // TODO: Find out if occs or neighbors totally protect us.
    return FALSE;
}

int
is_frightened_by(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    Unit *occ = NULL;
    UnitView *uvocc = NULL;
    int numfears = 0;
    int wr = 0;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    // What does unit think of uview's occs?
    for_all_occupant_views(uview, uvocc) 
	numfears += is_frightened_by(unit, uvocc);
    // What do unit's own occs think about the uview?
    for_all_occupants(unit, occ) 
	numfears += is_frightened_by(occ, uview);
    if (seems_safe_against(unit, uview))
	return numfears;
    // Two units of equal worth will have a fear threshold of 2.
    // TODO: Allow machine learning to adjust this threshold.
    // TODO: Replace call to 'total_worth' with an effective total worth.
    if (0 < total_worth(u))
	wr = (2 * total_worth(u2)) / total_worth(u);
    else
	wr = INT_MAX;
    if (turns_to_defeat_best_ratio(unit, uview) >= wr)
	++numfears;
    return numfears;
}

int
is_harassed_by(Unit *unit, UnitView *uview)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    Unit *occ = NULL;
    UnitView *uvocc = NULL;
    int numhars = 0;

    assert_error(in_play(unit),
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    assert_error(uview,
"AI Tactical Assessor: Attempted to access a NULL unit view");
    u = unit->type;
    u2 = uview->type;
    // What does unit think of uview's occs?
    for_all_occupant_views(uview, uvocc) 
	numhars += is_harassed_by(unit, uvocc);
    // What do unit's own occs think about the uview?
    for_all_occupants(unit, occ) 
	numhars += is_harassed_by(occ, uview);
    if (seems_safe_against(unit, uview))
	return numhars;
    ++numhars;
    return numhars;
}

int
choose_flee_direction(
    Unit *unit, int feardirs [NUMDIRS + 1], int hardirs [NUMDIRS + 1])
{
    int u = NONUTYPE;
    Side *side = NULL;
    Z8 dir = -1, dirbest = -1, dirscorebest = 0;
    int x = -1, y = -1;
    Z8 fleedirs [NUMDIRS], fleedirsgood [NUMDIRS], fleedirsbest [NUMDIRS];
    int canflee = TRUE, canfleewell = TRUE;
    int runlen = 0;

    assert_error(in_play(unit), 
"AI Tactical Assessor: Attempted to access an out-of-play unit");
    memset(fleedirs, 0, NUMDIRS*sizeof(Z8));
    memset(fleedirsgood, 0, NUMDIRS*sizeof(Z8));
    memset(fleedirsbest, 0, NUMDIRS*sizeof(Z8));
    u = unit->type;
    side = unit->side;
    // Find out which dirs, if any, unit can flee in.
    for_all_directions(dir) {
	xy_in_dir(unit->x, unit->y, dir, x, y);
	if (!valid(can_survive_on_known(unit, x, y)))
	    continue;
	if (!feardirs[dir]) {
	    fleedirs[dir] = (feardirs[opposite_dir(dir)] ? 2 : 1);
	    canflee = TRUE;
	}
	if (fleedirs[dir] && !hardirs[dir]) {
	    fleedirsgood[dir] = fleedirs[dir];
	    canfleewell = TRUE;
	}
    }
    if (!canflee)
	return -1;
    // Find out which dirs, if any, are best to flee in.
    if (canfleewell) {
	runlen = 0;
	for_all_directions(dir) {
	    if (fleedirsgood[dir])
		++runlen;
	    if (!fleedirsgood[dir] && runlen) {
		dirbest = dir - runlen/2 - 1;
		fleedirsbest[dirbest] = fleedirs[dirbest] * runlen;
		runlen = 0;
	    }
	}
	if (NUMDIRS == runlen)
	    return xrandom(NUMDIRS);
	// Second pass for wraparound run length.
	if (runlen) {
	    for_all_directions(dir) {
		fleedirsbest[dir] = 0;
		if (fleedirsgood[dir])
		    ++runlen;
		if (!fleedirsgood[dir]) {
		    dirbest = dir - runlen/2 - 1;
		    if (0 > dirbest)
			dirbest = NUMDIRS + dirbest;
		    fleedirsbest[dirbest] = fleedirs[dirbest] * runlen;
		    break;
		}
	    }
	}
	dirbest = -1;
	dirscorebest = 0;
	for_all_directions(dir) {
	    if ((fleedirsbest[dir] > dirscorebest) 
		|| ((fleedirsbest[dir] == dirscorebest)
		    && flip_coin())) {
		dirbest = dir;
		dirscorebest = fleedirsbest[dir];
	    }
	}
	return dirbest;
    } // if any good flee directions
    // Now, try any valid flee direction.
    runlen = 0;
    for_all_directions(dir) {
	if (fleedirs[dir])
	    ++runlen;
	if (!fleedirs[dir] && runlen) {
	    dirbest = dir - runlen/2 - 1;
	    fleedirsbest[dirbest] = fleedirs[dirbest] * runlen;
	    runlen = 0;
	}
    }
    if (NUMDIRS == runlen)
	return xrandom(NUMDIRS);
    // Second pass for wraparound run length.
    if (runlen) {
	for_all_directions(dir) {
	    fleedirsbest[dir] = 0;
	    if (fleedirs[dir])
		++runlen;
	    if (!fleedirs[dir]) {
		dirbest = dir - runlen/2 - 1;
		if (0 > dirbest)
		    dirbest = NUMDIRS + dirbest;
		fleedirsbest[dirbest] = fleedirs[dirbest] * runlen;
		break;
	    }
	}
    }
    dirbest = -1;
    dirscorebest = 0;
    for_all_directions(dir) {
	if ((fleedirsbest[dir] > dirscorebest) 
	    || ((fleedirsbest[dir] == dirscorebest)
		&& flip_coin())) {
	    dirbest = dir;
	    dirscorebest = fleedirsbest[dir];
	}
    }
    return dirbest;
}

TacticalConcern
handle_immediate_danger(Unit *unit)
{
    TacticalConcern taccon = TC_NONE;
    int u = NONUTYPE;
    Side *side = NULL;
    UnitView *uvstack = NULL, *uview = NULL;
    int x = -1, y = -1, range = -1;
    int feardirs [NUMDIRS + 1], hardirs [NUMDIRS + 1];
    int numfears = 0, numhars = 0, numfearstot = 0, numharstot = 0;
    int fleedir = -1;
    
    assert_error(is_active(unit), 
"AI Tactical Assessor: Attempted to access an inactive unit");
    memset(feardirs, 0, (NUMDIRS + 1)*sizeof(int));
    memset(hardirs, 0, (NUMDIRS + 1)*sizeof(int));
    u = unit->type;
    side = unit->side;
    // Think about worst case: 
    //	stepping into sights/blast radius of enemy with longest hit range.
    range = hit_range_max() + (u_mobile(u) ? u_move_range(u) : 0);
    for_all_cells_within_range(unit->x, unit->y, range, x, y) {
	if (!inside_area(x, y))
	    continue;
	if (UNSEEN == terrain_view(side, x, y))
	    continue;
	uvstack = (side->see_all ? query_uvstack_at(x, y) 
				 : unit_view_at(side, x, y));
	for_all_uvstack(uvstack, uview) {
	    if ((numfears = is_frightened_by(unit, uview))) {
		numfearstot += numfears;
		taccon = TC_DANGER_IMMEDIATE;
		if ((x == unit->x) && (y == unit->y))
		    feardirs[NUMDIRS] += numfears;
		else
		    feardirs[approx_dir(x - unit->x, y - unit->y)] += numfears;
	    }
	    else if ((numhars = is_harassed_by(unit, uview))) {
		numharstot += numhars;
		taccon = TC_DANGER_IMMEDIATE;
		if ((x == unit->x) && (y == unit->y))
		    hardirs[NUMDIRS] += numhars;
		else
		    hardirs[approx_dir(x - unit->x, y - unit->y)] += numhars;
	    }
	} // for all uvstack
    } // for all cells in range
    if (!numfearstot && !numharstot)
	return TC_NONE;
    if (u_mobile(u) && numfearstot) {
	if (NODIR != 
	    (fleedir = choose_flee_direction(unit, feardirs, hardirs)))
	    net_push_move_dir_task(unit, fleedir, 1);
	// TODO: Probably should just delay, and keep checking back.
	else
	    net_push_sentry_task(unit, 1);
	return taccon;
    }
    if (!u_mobile(u) || (1 == numfearstot)) {
	// TODO: Consider tactical construction.
    }
    // TODO: Respond to harassment.
    return taccon;
}

/* Overall Tactical Assessment and Handling */

/*!
    \todo Implement handling of occ danger.
    \todo Implement handling of targets of opportunity.
*/

TacticalConcern
handle_tactical_situation(Unit *unit)
{
    TacticalConcern taccon = TC_NONE;

    assert_error(is_active(unit), "Attempted to access an inactive unit");
    /* Garrisons */
    /* Threat Assessment */
    // Handle immediate danger to unit.
    if ((taccon = handle_immediate_danger(unit)))
	return taccon;
    /* Targets of Opportunity */
    // TODO: Implement.
    return taccon;
}

} // namespace Xconq::AI
} // namespace Xconq

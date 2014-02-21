/* Operational Roles and Analysis for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Operational Roles and Analysis for AIs

    Part of the AI API, Level 4. 

    Provides an operational level AI implementation.

    \note Nothing in this file should be required for AI implementation.

*/

#include "conq.h"
#include "kernel.h"
#include "kpublic.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"
#include "aioprt.h"

// NOTE: For now, we also include 'ai.h' for its theater stuff.
// TODO: We should probably move the theater stuff to this file.
#include "ai.h"

namespace Xconq {
namespace AI {

/* TEMPORARY: Side-related AI management.
    Should be moved to new files in the future. */

//! Create/init master AI structure associated with a side.

void
create_side_ai(Side *side)
{
    assert_error(side, "Attempted to access a NULL side");
    side->master_ai = (AI_Side *)xmalloc(sizeof(AI_Side));
    side->master_ai->side = side;
}

//! Return master AI structure associated with a side.

AI_Side *
get_side_ai(Side *side)
{
    assert_error(side, "Attempted to access a NULL side");
    return side->master_ai;
}

/* Management of operational roles. */

//! Allocate a new block of oproles in pool.

void
allocate_oproles(Side *side, int nodesnum)
{
    AI_Side *ai = NULL;
    OpRole *newblock = NULL;
    int i = 0;

    assert_error(side, "Attempted to access a NULL side");
    assert_error(get_side_ai(side), 
		 "Attempted to access a side without a master AI");
    assert_error(0 < nodesnum, 
		 "Attempted to allocate 0 or fewer operation roles");
    ai = get_side_ai(side);
    /* Allocate new block of oproles. */
    newblock = (OpRole *)xmalloc(nodesnum * sizeof(OpRole));
    /* Fill out new block of oprole info nodes. */
    for (i = 0; i < (nodesnum - 1); ++i) {
	newblock[i].next = &(newblock[i+1]);
    }
    /* If side's pool is empty, then assign block to it. */
    if (!ai->oproles_free) 
      ai->oproles_free = newblock;
    /* Else prepend new block to existing pool of free nodes. */
    else {
	newblock[nodesnum - 1].next = ai->oproles_free;
	ai->oproles_free = newblock;
    }
}

//! Acquire oprole.

OpRole *
acquire_oprole(Side *side, int id, OpRole_Type type)
{
    AI_Side *ai = NULL;
    OpRole *oprole = NULL, *oprole2 = NULL;

    assert_error(side, "Attempted to access a NULL side");
    assert_error(get_side_ai(side), 
		 "Attempted to access a side without a master AI");
    ai = get_side_ai(side);
    /* Allocate a new block of free nodes, if necessary. */
    if (!ai->oproles || !ai->oproles_free)
      allocate_oproles(side);
    /* Get oprole. */
    oprole = ai->oproles_free;
    ai->oproles_free = oprole->next;
    /* Insert it into correct location in side's oprole pool. */
    for_all_oproles(side, oprole2) {
	/* Somewhere in the list... */
	if (oprole2->next 
	    && between(oprole2->id, id, oprole2->next->id)) {
	    oprole->next = oprole2->next;
	    oprole2->next = oprole;
	    break;
	}
	/* At beginning of list... */
	else if (oprole2->id > id) {
	    oprole->next = oprole2;
	    ai->oproles = oprole;
	    break;
	}
	/* At end of list... */
	else if (!oprole2->next) {
	    oprole2->next = oprole;
	    oprole->next = NULL;
	    break;
	}
    }
    /* Maybe the list was empty. Make node head of list, if so. */
    if (!ai->oproles) {
	ai->oproles = oprole;
	oprole->next = NULL;
    }
    /* Allocate hash buckets, if needed. */
    if (!ai->oprole_buckets)
      ai->oprole_buckets =
	(OpRole **)xmalloc(OR_Total * sizeof(OpRole *));
    /* Find proper place in bucket. */
    for_all_oproles_by_type(side, type, oprole2) {
	/* Somewhere in the list... */
	if (oprole2->next_by_type 
	    && between(oprole2->id, id, oprole2->next_by_type->id)) {
	    oprole->next_by_type = oprole2->next_by_type;
	    oprole2->next_by_type = oprole;
	    break;
	}
	/* At beginning of list... */
	else if (oprole2->id > id) {
	    oprole->next_by_type = oprole2;
	    ai->oprole_buckets[type] = oprole;
	    break;
	}
	/* At end of list... */
	else if (!oprole2->next_by_type) {
	    oprole2->next_by_type = oprole;
	    oprole->next_by_type = NULL;
	    break;
	}
    }
    /* Maybe the bucket was empty. Make node head of bucket, if so. */
    if (!ai->oprole_buckets[type]) {
	ai->oprole_buckets[type] = oprole;
	oprole->next_by_type = NULL;
    }
    /* Finish filling out the oprole info node. */
    oprole->id = id;
    oprole->type = type;
    oprole->side = side;
    return oprole;
}

void
release_oprole(OpRole *oprole)
{
    AI_Side *ai = NULL;
    OpRole *oprole2 = NULL;
    Side *side = NULL;

    assert_error(oprole, "Attempted to release a NULL oprole info node");
    side = oprole->side;
    ai = get_side_ai(side);
    /* Remove from hash bucket. */
    for_all_oproles_by_type(side, oprole->type, oprole2) {
	/* If first in bucket... */
	if (oprole2->id == oprole->id) {
	    ai->oprole_buckets[oprole->type] = oprole->next_by_type;
	    break;
	}
	/* If next in bucket... */
	else if (oprole2->next_by_type 
		 && (oprole2->next_by_type->id == oprole->id)) {
	    oprole2->next_by_type = oprole->next_by_type;
	    break;
	}
    }
    oprole->next_by_type = NULL;
    // Remove from list.
    for_all_oproles(side, oprole2) {
	// If first in list... 
	if (oprole2->id == oprole->id) {
	    ai->oproles = oprole->next;
	    break;
	}
	// If next in list... 
	else if (oprole2->next && (oprole2->next->id == oprole->id)) {
	    oprole2->next = oprole->next;
	    break;
	}
    }
    oprole->next = NULL;
    // Add back to pool of unused nodes.
    if (ai->oproles_free) {
	oprole->next = ai->oproles_free;
	ai->oproles_free = oprole;
    }
    else 
      ai->oproles_free = oprole;
    // Reset ID and type.
    oprole->id = -1;
    oprole->type = OR_NONE;
}

OpRole *
find_oprole(Side *side, int id)
{
    OpRole *oprole = NULL;

    assert_error(side, "Attempted to access a NULL side");
    for_all_oproles(side, oprole) {
	if (oprole->id == id)
	  return oprole;
    }
    return NULL;
}

/* Handling of Operational Roles */

#if (0)
/*
    \todo Implement.
*/

void
handle_shuttle_oprole(OpRole *oprole)
{
    Unit *unit = NULL, *occ = NULL;
    Side *side = NULL;
    UnitView *uv = NULL, *uvocc = NULL;

    /* Get out if the oprole is invalid or inactive. */
    assert_warning_return(oprole, "Attempted to evaluate invalid oprole",);
    if (OR_SHUTTLE != oprole->type)
      return;
    /* Find unit associated with oprole. */
    unit = find_unit(oprole->id);
    assert_warning_return(in_play(unit),
			  "Attempted to manipulate an out-of-play unit",);
    /* Get oprole side. Need not be unit side. */
    side = oprole->side;
    /* Find uview associated with unit. */
    uv = find_unit_view(side, unit);
    assert_warning_return(uv, "Attempted to access a NULL uview",);
    // TODO: Implement.
#if (0)
    /** Threat Assessment **/
    /* Scan for immediate threats to shuttle. */
    if (in_immediate_danger(unit)) {
	retreat_from_danger(unit);
	return;
    }
    /* Scan for immediate threats to shuttle's occs. */
    for_all_occupant_views(uv, uvocc) {
	if (uvocc->siden != side->id)
	  continue;
	occ = view_unit(uvocc);
	if (in_immediate_danger(occ)) {
	    retreat_from_occ_danger(unit, occ);
	    return;
	}
    }
    /* (TODO: Scan for shuttle threats along path.) */
    /* (TODO: Scan for occ threats along path.) */
    /** TODO: Embark/Disembark Opportunities **/
    /** TODO: Pathfind/Move-To Next Embark/Disembark Point **/
#endif
}
#endif

/* Construction */

int
choose_utype_to_construct(OpRole *oprole, int *uscore)
{
    static int *uscores = NULL;

    int rslt = A_ANY_OK;
    Unit *unit = NULL;
    Side *side = NULL;
    int u = NONUTYPE, u2 = NONUTYPE, uc = NONUTYPE;
    int cp = 0, cppt = 0, tp = 0, tppt = 0, ttc = -1;
    int uscorestot = 0, luckynum = -1;

    // Get out if the oprole is invalid or inactive.
    assert_warning_return(oprole, "AI: Attempted to handle invalid oprole", 
			  uc);
    // Find unit associated with oprole.
    unit = find_unit(oprole->id);
    assert_warning_return(in_play(unit),
			  "AI: Attempted to manipulate an out-of-play unit", 
			  uc);
    // Useful info.
    side = oprole->side;
    u = unit->type;
    // Initialize utype score array, if necessary.
    if (!uscores)
	uscores = (int *)xmalloc(numutypes * sizeof(int));
    // Iterate through utypes, and find out which we can create and score them.
    for_all_unit_types(u2) {
	uscores[u2] = -1;
	// Skip if...
	// ...cannot create.
	rslt = can_create_common(unit, unit, u2, unit->x, unit->y);
	if (!valid(rslt) && (A_CONSTRUCT_NO_TOOLING != rslt))
	    continue;
	// ...if immobile constructor and no access to suitable cells.
	// TODO: Alter this test not to depend on info we should not have.
	if (!u_mobile(u) 
	    && (u_naval_mobile(u2) && !u_air_mobile(u2)) 
	    && !suitable_port(unit))
	    continue;
	// Estimate turns to complete.
	ttc = 0;
	// TODO: Estimate whether creation would add another turn.
	// Add CP contribution to turns to complete.
	cp = u_cp(u2) - uu_creation_cp(u, u2);
	if (0 < cp) {
	    cppt = cp_per_turn_est(unit, u2);
	    if (0 < cppt) {
		ttc += cp / cppt;
		if (cp % cppt)
		    ++ttc;
	    }
	}
	// Add TP contribution to turns to complete.
	tp = uu_tp_max(u, u2) - (unit->tooling ? unit->tooling[u2] : 0);
	if (0 < tp) {
	    tppt = tp_per_turn_est(unit, u2);
	    if (0 < tppt) {
		ttc += tp / tppt;
		if (tp % tppt)
		    ++ttc;
	    }
	}
	// Calculate score.
	// TODO: Should consider things such as:
	//	    (a) Potential self-unit?
	//	    (b) Essential for victory?
	//	    (c) Needed in constructor's theater?
	//	    (d) Needed in any theater?
	//	 Possibly roll such tests into enhanced total worth function.
	// Total worth of u2 divided by estimated turns to complete it.
	uscores[u2] = total_worth(u2) / (ttc + 1);
	uscorestot += uscores[u2];
    }
    // Randomly pick an utype to construct.
    luckynum = xrandom(uscorestot);
    uscorestot = 0;
    for_all_unit_types(u2) {
	// Skip unscored utypes.
	if (0 > uscores[u2])
	    continue;
	// Rebuild total uscores, and find what was picked.
	uscorestot += uscores[u2];
	if (luckynum < uscorestot) {
	    uc = u2;
	    if (uscore)
		*uscore = uscores[u2];
	    break;
	}
    }
    return uc;
}

Unit *
choose_unit_to_build_upon(OpRole *oprole, int *uscore)
{
    Unit *unit = NULL, *unit2 = NULL, *unitc = NULL;
    Side *side = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int mybrate = 0, brate = 0;
    int dist = 0, mdist = 0;
    int cpreq = 0;
    int eta = 0, ttc = -1;
    int worth = 0, score = 0, scorebest = -1;

    assert_warning_return(
	oprole, "AI: Attempted to handle invalid oprole", NULL);
    // Find unit associated with oprole.
    unit = find_unit(oprole->id);
    assert_warning_return(
	in_play(unit), "AI: Attempted to manipulate an out-of-play unit", 
        NULL);
    side = oprole->side;
    u = unit->type;
    // Find unit on our side that is most worthy of being built upon, 
    //	accounting for its relative completeness, distance from builder, 
    //	and worth.
    for_all_side_units(side, unit2) {
	if (unit == unit2)
	    continue;
	if (!in_play(unit2))
	    continue;
	if (completed(unit2))
	    continue;
	u2 = unit2->type;
	mybrate = cp_per_turn_est(unit, u2);
	if (0 >= mybrate)
	    continue;
	if (!valid(can_build(unit, unit, unit2)))
	    continue;
	dist = distance(unit->x, unit->y, unit2->x, unit2->y);
	// Calculate distance outside of build range.
	// Note that range < 0 increases move dist.
	mdist = dist - uu_build_range(u, u2);
	if (!u_mobile(u) && (mdist > 0))
	    continue;
	cpreq = u_cp(u2) - unit2->cp;
	eta = (u_mobile(u) ? mdist / mp_per_turn_max(u) : 0);
	// Estimated CP per turn from current builders.
	// Note: Does not consider potential builders in transit, 
	//  who may contribute before this potential builder arrives.
	brate = max(0, cp_gained_per_turn_est(unit2, side));
	// Estimate CP left to complete after arrival.
	cpreq -= eta * brate;
	if (0 >= cpreq)
	    continue;
	brate += mybrate;
	ttc = (cpreq / brate);
	if (cpreq % brate)
	    ++ttc;
	// Effective worth of incomplete unit.
	// TODO: Handle other worth modifiers, such as victory conditions.
	// TODO: Penalize score for out-of-theater units.
	worth = total_worth(u2);
	// Score completion importance.
	score = worth / ttc;
	// Update best scores and unit placeholders, if necessary.
	if (score > scorebest) {
	    scorebest = score;
	    unitc = unit2;
	}
    } // for all side units
    if (uscore)
	*uscore = scorebest;
    return unitc;
}

Unit *
choose_unit_to_repair(OpRole *oprole, int *uscore)
{
    Unit *unit = NULL, *unit2 = NULL, *unitr = NULL;
    Side *side = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int myrrate = 0, rrate = 0;
    int dist = 0, mdist = 0, rrange = -1;
    int hpreq = 0, hpalarm = 0;
    int eta = 0, ttc = -1;
    int worth = 0, score = 0, scorebest = -1;

    assert_warning_return(
	oprole, "AI: Attempted to handle invalid oprole", NULL);
    // Find unit associated with oprole.
    unit = find_unit(oprole->id);
    assert_warning_return(
	in_play(unit), "AI: Attempted to manipulate an out-of-play unit", 
        NULL);
    side = oprole->side;
    u = unit->type;
    // Find unit on our side that is most worthy of being built upon, 
    //	accounting for its relative completeness, distance from builder, 
    //	and worth.
    for_all_side_units(side, unit2) {
	if (unit == unit2)
	    continue;
	if (!in_play(unit2))
	    continue;
	if (!completed(unit2))
	    continue;
	u2 = unit2->type;
	hpalarm = (u_hp(u2) * unit_doctrine(unit2)->repair_percent) / 100;
	if (unit2->hp > hpalarm)
	    continue;
	myrrate = hp_per_turn_est(unit, u2);
	if (0 >= myrrate)
	    continue;
	if (!valid(can_repair(unit, unit, unit2))
	    && !valid(can_auto_repair(unit, unit2)))
	    continue;
	dist = distance(unit->x, unit->y, unit2->x, unit2->y);
	rrange = max(uu_repair_range(u, u2), uu_auto_repair_range(u, u2));
	// Calculate distance outside of build range.
	// Note that range < 0 increases move dist.
	mdist = dist - rrange;
	if (!u_mobile(u) && (mdist > 0))
	    continue;
	hpreq = (u_hp(u2) * unit_doctrine(unit2)->repair_complete) / 100
		- unit2->hp;
	eta = (u_mobile(u) ? mdist / mp_per_turn_max(u) : 0);
	// Estimated HP per turn from current repairers.
	// Note: Does not consider potential repairers in transit, 
	//  who may contribute before this potential repairer arrives.
	rrate = max(0, hp_gained_per_turn_est(unit2, side));
	// Estimate CP left to complete after arrival.
	hpreq -= eta * rrate;
	if (0 >= hpreq)
	    continue;
	rrate += myrrate;
	ttc = (hpreq / rrate);
	if (hpreq % rrate)
	    ++ttc;
	// Effective worth of incomplete unit.
	// TODO: Handle other worth modifiers, such as victory conditions.
	// TODO: Penalize score for out-of-theater units.
	worth = total_worth(u2);
	// Score completion importance.
	score = worth / ttc;
	// Update best scores and unit placeholders, if necessary.
	if (score > scorebest) {
	    scorebest = score;
	    unitr = unit2;
	}
    } // for all side units
    if (uscore)
	*uscore = scorebest;
    return unitr;
}

Unit *
choose_transport_to_construct_in(
    int u2, Side *side, UnitView *uvtspt, int upchain)
{
    Unit *tsptbest = NULL;
    UnitView *uvtspt2 = NULL;

    assert_error(is_unit_type(u2),
		 "AI: Encountered an invalid unit type to construct");
    assert_error(side,
		 "AI: Attempted to access a NULL side");
    assert_error(uvtspt,
		 "AI: Attempted to access a NULL unit view");
    // If we are allowed to search upwards, then do so.
    // Note: We pick the broadest transport to construct in, 
    //	hoping that we will be able to enclose other things.
    //	Technically, the action code does not support this yet.
    if (upchain) {
	for (uvtspt2 = uvtspt->transport; uvtspt2; 
	     uvtspt2 = uvtspt2->transport) {
	    // We cannot construct in an enemy transport.
	    // TODO: Think about "constructing" plague, etc... inside an enemy.
	    if (enemy_side(side, side_n(uvtspt2->siden))) 
		continue;
	    // Does new unit appear likely to fit in transport?
	    if (!valid(can_be_in(u2, side, uvtspt2)))
		continue;
	    // Mark this as our best transport.
	    tsptbest = query_unit_from_uview(uvtspt2);
	}
	if (tsptbest)
	    return tsptbest;
    }
    // Get out, if target transport is an enemy.
    if (enemy_side(side, side_n(uvtspt->siden)))
	return NULL;
    // Search downwards.
    // Note: We pick the narrowest transport to construct in,
    //	to minimize the amount of space being wasted.
    for_all_occupant_views(uvtspt, uvtspt2) {
	// We cannot construct in an enemy transport.
	if (enemy_side(side, side_n(uvtspt2->siden)))
	    continue;
	// Skip mobile transports.
	//  We don't want them moving away while we are trying to construct.
	if (u_mobile(uvtspt2->type))
	    continue;
	// Depth-first recurse into occs.
	if (uvtspt2->occupant)
	    tsptbest = choose_transport_to_construct_in(u2, side, uvtspt2);
	// Does new unit appear likely to fit in transport?
	if (!valid(can_be_in(u2, side, uvtspt2)))
	    continue;
	// Return the first occ that the new unit can apparently fit in.
	return query_unit_from_uview(uvtspt2);
    }
    // Does new unit appear likely to fit in target transport?
    if (valid(can_be_in(u2, side, uvtspt)))
	return query_unit_from_uview(uvtspt);
    return NULL;
}

Task *
generate_construction_task(Unit *unit, int u2)
{
    Unit *transport = NULL, *tsptbest = NULL;
    int u = NONUTYPE;
    Side *side = NULL;
    UnitView *uvstack = NULL, *uview = NULL;
    int crange = -1, range = -1, sfx = -1, sfy = -1, x = -1, y = -1;
    int dist = -1, xbest = -1, ybest = -1;
    int score = -1, scorebest = -1;
    Theater *theater = NULL;
    int xypopularity = 0;

    assert_warning_return(
	in_play(unit), "AI: Attempted to manipulate an out-of-play unit", 
        NULL);
    // Useful info.
    side = unit->side;
    u = unit->type;
    uview = query_uvstack_from_unit(unit);
    // Calculate the search range and origin point.
    crange = min(uu_create_range(u, u2), uu_build_range(u, u2));
    // If mobile unit,
    //	and construction can performed outside of trans-occ chain, 
    //	then try theater-wide+ search...
    if (u_mobile(u) && (0 <= crange)) {
	// Get the theater that the unit is in.
	// We deliberately allow ourselves to "bleed" into other theaters.
	//  This allows the constructive processes to diffuse without the 
	//  need for an explicit theater transfer for the unit.
	if ((theater = theater_at(side, unit->x, unit->y))) {
	    range = 
		crange 
		+ (max(theater->xmax - theater->xmin, 
		       theater->ymax - theater->ymin)*125)/100;
	    sfx = theater->x;  sfy = theater->y;
	}
	// Else, fall back on the unit's tactical range.
	else {
	    range = crange + u_ai_tactical_range(u);
	    sfx = unit->x;  sfy = unit->y;
	}
    }
    // Else, we use limited range.
    else {
	range = crange;
	sfx = unit->x;  sfy = unit->y;
    }
    // Try the transport-occ chain first, because it is probably quicker.
    // Find transport in transport-occ chain to construct in.
    // Note that this need not be the constructor or its transport.
    transport = choose_transport_to_construct_in(u2, side, uview, TRUE);
    // If a transport was found, then try to set a construct task in it.
    if (transport) {
	net_set_construct_task(unit, u2, 1, transport->id, -1, -1);
	if (unit->plan)
	    return unit->plan->tasks;
    }
    else {
	// If restricted to transport-occ chain, then we are done.
	if (0 > range) 
	    return NULL;
    }
    // Search all cells in range, and determine best one.
    for_all_cells_within_range(sfx, sfy, range, x, y) {
	score = 0;
	transport = NULL;
	// Skip, if outside of world.
	if (!inside_area(x, y))
	    continue;
	// Rummage through potential transports in cell.
	// Note: Even if u2 cannot see cell and cannot survive on it,
	//  we may be able to construct it inside something we have there.
	uvstack = query_uvstack_at(x, y);
	for_all_uvstack(uvstack, uview) {
	    transport = 
		choose_transport_to_construct_in(u2, side, uview);
	    // TODO: Score all potential transports.
	    if (transport)
		break;
	}
	// Can survive on cell without transport?
	// Note: Rejects cells that we cannot see.
	// Note: Rejects cells that are too full.
	if (!transport && !valid(can_survive_on_known(u2, side, x, y))) 
	    continue;
	// TODO: Maybe create deferred construction tasks, 
	//  if moving friendly mobile units would allow construction at.
	// If another unit plans on constructing in the given cell,
	//  and we plan on constructing there, then reject it.
	if ((xypopularity = n_plan_to_construct_at(side, x, y)))
	    continue;
	// If there would be resource contention with other units...
	if (resource_contention_with_any(u2, x, y, side))
	    continue;
	// If advanced utype cannot meet size goal at location.
	if (u_advanced(u2) && !could_meet_size_goal(u2, x, y))
	    continue;
	dist = distance(unit->x, unit->y, x, y);
	score += isqrt(range - (dist - crange));
	// TODO: Additional scoring.
	if ((score > scorebest) || ((score == scorebest) && flip_coin())) {
	    scorebest = score;
	    tsptbest = transport;
	    xbest = x;  ybest = y;
	}
    } // for all cells within range
    // If nothing was found, then get out.
    if (!in_play(tsptbest) && !inside_area(xbest, ybest))
	return NULL;
    // Setup tasks based on best cell or transport.
    // Construct in a transport.
    if (tsptbest) 
	net_set_construct_task(unit, u2, 1, tsptbest->id, -1, -1);
    // Construct in a cell.
    else 
	net_set_construct_task(unit, u2, 1, -1, xbest, ybest);
    // Move there first, if necessary.
    if (dist > crange)
	net_push_move_to_task(unit, xbest, ybest, crange);
    if (unit->plan)
	return unit->plan->tasks;
    return NULL;
}

int
choose_construction_or_repair(OpRole *oprole)
{
    Unit *unit = NULL, *unitb = NULL, *unitr = NULL;
    Side *side = NULL;
    int u = NONUTYPE, uc = NONUTYPE;
    int uscore = -1, bscore = -1, rscore = -1; 
    int dist = -1, rrange = -1;

    // Get out if the oprole is invalid or inactive.
    assert_warning_return(
	oprole, "AI: Attempted to handle invalid oprole", NONUTYPE);
    // Find unit associated with oprole.
    unit = find_unit(oprole->id);
    assert_warning_return(
	in_play(unit), "AI: Attempted to manipulate an out-of-play unit", 
        NONUTYPE);
    // Useful info.
    side = oprole->side;
    u = unit->type;
    // Little machine player debug blurb.
    DMprintf(
	"%s is looking for something to construct or repair...\n",
	medium_long_unit_handle(unit));
    // Choose a new utype to construct.
    //	We may or may not actually construct it, 
    uc = choose_utype_to_construct(oprole, &uscore);
    // Choose an unit to complete, if one is available.
    //	We may or may not actually complete it.
    unitb = choose_unit_to_build_upon(oprole, &bscore);
    if (bscore < uscore) {
	unitb = NULL;
	bscore = -1;
    }
    // Choose an unit to repair to its repair threshold.
    // TODO: Need to think about auto-repair.
    //	With auto-repair, it is conceivable that we could move into range 
    //	of many units needing repair and heal them all simultaneously.
    //	Probably we should test how many other units could be auto-repaired 
    //	if we were at the position of the unit needing auto-repair.
    //	This is somewhat crude, but decent and not too expensive.
    //	Once at the location or near the primary unit being auto-repaired, 
    //	the repair task could be smart enough to position the repairer to 
    //	optimize auto-repair coverage. This kind of intelligence is not 
    //	critical to have at this juncture, but something to keep in mind.
    //	If auto-repairer is immobile, then ignore auto-repair.
    unitr = choose_unit_to_repair(oprole, &rscore);
    if (rscore < uscore) {
	unitr = NULL;
	rscore = -1;
    }
    /* Decide what to do. */
    // Should we construct new unit?
    if ((uscore > bscore) && (uscore > rscore)
	&& generate_construction_task(unit, uc)) {
	DMprintf("\t...and decided to construct a new %s.\n", 
		 u_type_name(uc));
    }
    // Should we resume building on existing unit?
    else if (bscore > rscore) {
	uc = unitb->type;
	dist = distance(unit->x, unit->y, unitb->x, unitb->y);
	net_set_build_task(unit, unitb->id, u_cp(uc));
	// TODO: Handle tooling up, if necessary.
	if (0 < dist)
	    net_push_move_to_task(
		unit, unitb->x, unitb->y, 
		(-1 < uu_build_range(u, uc)) ? uu_build_range(u, uc) : 0);
	DMprintf(
	    "\t...and found %s to complete.\n", 
	    medium_long_unit_handle(unitb));
    }
    // Should we repair an unit to sufficient health?
    else if (rscore >= 0) {
	uc = unitr->type;
	dist = distance(unit->x, unit->y, unitr->x, unitr->y);
	net_set_repair_task(
	    unit, unitr->id, 
	    (u_hp(uc) * unit_doctrine(unitr)->repair_complete) / 100);
	rrange = max(uu_repair_range(u, uc), uu_auto_repair_range(u, uc));
	rrange = (0 > rrange ? 0 : rrange);
	if (0 < dist)
	    net_push_move_to_task(unit, unitr->x, unitr->y, rrange);
	DMprintf(
	    "\t...and found %s to repair up to %d%% of full (%d) HP.\n", 
	    medium_long_unit_handle(unitr), 
	    unit_doctrine(unitr)->repair_complete, u_hp(uc));
    }
    // Else, we could not find anything to do.
    else {
	uc = NONUTYPE;
	DMprintf("\t...and found nothing.\n");
    }
    return uc;
}

OpRole_Outcome
handle_constructor_oprole(OpRole *oprole)
{
    Unit *unit = NULL;
    Side *side = NULL;
    Task *tasks = NULL;

    // Get out if the oprole is invalid or inactive.
    assert_warning_return(oprole, "AI: Attempted to evaluate invalid oprole",
			  ORO_INVALID);
    if (OR_CONSTRUCTOR != oprole->type)
	return ORO_INVALID;
    // Find unit associated with oprole.
    unit = find_unit(oprole->id);
    assert_warning_return(in_play(unit),
			  "AI: Attempted to manipulate an out-of-play unit",
			  ORO_FAILED);
    side = oprole->side;
    tasks = (unit->plan ? unit->plan->tasks : NULL);
    // Evaluate tactical situation first.
    //	Includes tactical construction.
    if (TC_NONE != handle_tactical_situation(unit))
	return ORO_HANDLED_TACTICAL;
    // If there are existing tasks to run, then let them run.
    if (tasks)
	return ORO_OK;
    // Choose what to construct next.
    if (NONUTYPE != choose_construction_or_repair(oprole))
	return ORO_OK;
    // TODO: If no construction available, 
    //		then perhaps we can decide to fall back to another oprole type.
    return ORO_FAILED;
}

//! Handle an operational role.

void
handle_oprole(OpRole *oprole)
{
    Unit *unit = NULL;
    OpRole_Outcome oro = ORO_OK;

    // Get out if the oprole is invalid or inactive.
    assert_warning_return(oprole, "Attempted to evaluate invalid oprole",);
    if (OR_NONE == oprole->type) {
	release_oprole(oprole);
	return;
    }
    // Remove oproles for dead units.
    unit = find_unit(oprole->id);
    if (!unit) {
	release_oprole(oprole);
	return;
    }
    // TODO: If too many failures, then release oprole and put unit in reserve.
    // Switch to appropriate evaluator, as necessary.
    switch (oprole->type) {
      case OR_SHUTTLE:
#if (0)
	oro = handle_shuttle_oprole(oprole);
#endif
	break;
      case OR_CONSTRUCTOR:
	oro = handle_constructor_oprole(oprole);
	break;
      default:
	run_warning("AI: Attempted to evaluate unknown oprole type; weird");
	return;
    }
    // Increment oprole execution counter.
    ++oprole->execs_this_turn;
    // Handle oprole outcomes.
    switch (oro) {
	case ORO_OK: 
	case ORO_HANDLED_TACTICAL:
	    break;
	case ORO_FAILED:
	case ORO_INVALID:
	default:
	    ++oprole->fails_this_turn;
	    // If too many failures, 
	    //	then put unit in reserve and release oprole.
	    // Note that each oprole should handle failover to other oproles.
	    if ((10 <= oprole->fails_this_turn) 
		&& (oprole->fails_this_turn 
		    >= ((oprole->execs_this_turn * 9) / 10))) {
		if (in_play(unit))
		    net_set_unit_reserve(unit->side, unit, TRUE, FALSE);
		release_oprole(oprole);
	    }
	    break;
    }
}

//! Handle all operational roles on a side.

void
handle_oproles(Side *side)
{
    OpRole *oprole = NULL;

    assert_error(side, "Attempted to access a NULL side");
    assert_error(get_side_ai(side), 
		 "Attempted to access non-existent master AI struct");
    for_all_oproles(side, oprole) {
	if (OR_NONE == oprole->type)
	  continue;
	handle_oprole(oprole);
    }
}

} // namespace Xconq::AI
} // namespace Xconq

/* Unit and Unit Type Analysis and Status Functions for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Unit and Unit Type Analysis and Status Function for AIs

    Part of the AI API, Level 1. 

    Provides useful functions that an AI implementation may use to analyze
    and to learn the status of units and unit types.

    The functions in this file provide information regarding units and 
    unit types that is not directly available from the kernel functions, 
    but is useful for decision-making.

    There are various kinds of functions in this file.
    'can_*' functions test an unit's inclusive-or side's current state to 
    see if a given type of action or meta-action can be performed.
    'could_*' functions disregard current state, and ask hypothetical 
    questions based on type. These functions can be regarded as lower level 
    than the 'can_*' functions.

    \note Nothing in this file should be required for AI implementation.
    \todo Import most of the functions in 'aiutil.c' into this file.
    \todo Make use of caching, where applicable.

*/

#include "conq.h"
#include "kernel.h"
#include "aiunit.h"

using namespace Xconq;
using namespace Xconq::AI;

//! Could u see?

int
could_see(int u)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (0 > u_vision_range(u))
      return FALSE;
    return TRUE;
}

int
can_be_on(int u, Side *side, int x, int y, int *p_without)
{
    static int *p_ucapx;

    int tv = UNSEEN;
    int t = NONTTYPE;
    UnitView *uvstack = NULL, *uview = NULL;
    int sztot = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    /* Cannot be outside playing area. */
    if (!inside_area(x, y))
      return A_MOVE_CANNOT_LEAVE_WORLD;
    /* Acquire side's terrain view. */
    tv = terrain_view(side, x, y);
    /* If cell is unseen, we assume we can be there. */
    if (UNSEEN == tv)
      return A_ANY_OK;
    /* Convert view to ttype. */
    t = vterrain(tv);
    /* Could cell ever hold utype? */
    if (!could_be_on(u, t))
      /* (Should return A_ANY_TOO_LARGE.) */
      return A_ANY_CANNOT_DO;
    // Initialize extra-capcity array, if necessary.
    if (!p_ucapx)
	p_ucapx = (int *)xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u2)
	p_ucapx[u2] = ut_capacity_x(u2, t);
    // Fake capacities for exclusions.
    if (p_without) {
	for_all_unit_types(u2) {
	    if (0 < ut_capacity_x(u2, t)) {
		p_ucapx[u2] += min(p_without[u2], ut_capacity_x(u2, t));
		p_without[u2] = max(0, p_without[u2] - ut_capacity_x(u2, t));
	    }
	    sztot -= min(p_without[u2] * ut_size(u2, t), t_capacity(t));
	}
    }
    /* Can cell hold utype given the uvstack there? */
    if (g_see_all() || side->see_all)
      uvstack = query_uvstack_at(x, y);
    else
      uvstack = unit_view_at(side, x, y);
    for_all_uvstack(uvstack, uview) {
	u2 = uview->type;
	if (0 < p_ucapx[u2])
	  --p_ucapx[u2];
	else {
	    sztot += ut_size(u2, t);
	    if (sztot >= t_capacity(t))
	      return A_MOVE_DEST_FULL;
	}
    }
    sztot += ut_size(u, t);
    if (!p_ucapx[u] && (sztot > t_capacity(t)))
      return A_MOVE_DEST_FULL;
    // TODO: Consider all known blocking ZOCs?
    return A_ANY_OK;
}

int
can_be_on_known(int u, Side *side, int x, int y, int *p_without)
{
    int rslt = A_ANY_OK;
    int tv = UNSEEN;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    /* Can be on given cell? */
    if (!valid(rslt = can_be_on(u, side, x, y, p_without)))
      return rslt;
    /* Acquire side's terrain view. */
    tv = terrain_view(side, x, y);
    /* Cell cannot be unknown to side. */
    if (UNSEEN == tv)
      return A_ANY_ERROR;
    return A_ANY_OK;
}

/*! \todo Consider whether u could ever get in u2 to start with. */

int
could_be_occupant_of(int u, int u2)
{
    int vol = 0;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    if (0 == u_occ_total_max(u2))
	return FALSE;
    if (0 == uu_occ_max(u2, u))
	return FALSE;
    vol = uu_size(u, u2);
    if (vol < u_capacity(u2))
      return TRUE;
    if (vol < uu_capacity_x(u2, u))
      return TRUE;
    return FALSE;
}

int
could_be_occupant(int u)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    for_all_unit_types(u2) {
	if (could_be_occupant_of(u, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_be_in(int u2, Side *side, UnitView *uvtspt)
{
    static int *p_ucapx;

    UnitView *uview = NULL;
    int u = NONUTYPE, u3 = NONUTYPE;
    int sztot = 0, numoccs = 0, numsame = 0;

    assert_error(is_unit_type(u2),
		 "AI Volume Check: Encountered invalid unit type");
    assert_error(side,
		 "AI Volume Check: Attempted to access a NULL side");
    assert_error(uvtspt,
		 "AI Volume Check: Attempted to access a NULL unit view");
    u = uvtspt->type;
    // Check permissions.
    if (!trusted_side(side, side_n(uvtspt->siden)))
	return A_ANY_CANNOT_DO;
    if (!uvtspt->complete) {
	// If unit cannot occupy incomplete transport...
	if (!uu_can_occupy_incomplete(u2, u))
	    return A_MOVE_DEST_FULL;
    }
    // Initialize extra-capcity array, if necessary.
    if (!p_ucapx)
	p_ucapx = (int *)xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u3)
	p_ucapx[u3] = uu_capacity_x(u, u3);
    // Determine if there appears to be space for new unit.
    for_all_occupant_views(uvtspt, uview) {
	u3 = uview->type;
	++numoccs;
	if (0 < p_ucapx[u3])
	    --p_ucapx[u3];
	else {
	    sztot += uu_size(u3, u);
	    if (sztot >= u_capacity(u))
		return A_MOVE_DEST_FULL;
	    if (u3 == u2)
		++numsame;
	}
    }
    if ((0 <= u_occ_total_max(u)) && (numoccs >= u_occ_total_max(u)))
	return A_MOVE_DEST_FULL;
    if ((0 <= uu_occ_max(u, u2)) && (numsame >= uu_occ_max(u, u2)))
	return A_MOVE_DEST_FULL;
    sztot += uu_size(u2, u);
    if (!p_ucapx[u2] && (sztot > u_capacity(u)))
	return A_MOVE_DEST_FULL;
    return A_ANY_OK;
}

int
longest_economic_input(int u, int u2, int m)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    assert_error(is_material_type(m), 
		 "Expected a material type but did not get one");
    /* Check if the receiver can hold the material. */
    if (!um_storage_x(u, m))
      return INT_MAX;
    /* Check if the sender can hold the materials. */
    if (!um_storage_x(u2, m))
      return -1;
    /* Check if input is possible. */
    if ((0 > um_inlength(u, m)) || (0 > um_outlength(u2, m)))
      return -1;
    /* Return reciever's in-length, if it is <= than sender's outlength. */
    if (um_outlength(u2, m) >= um_inlength(u, m))
      return um_inlength(u, m);
    /* Else, return sender's out-length. */
    return um_outlength(u2, m);
}

//! Longest direct economic input from u2 to u.

int
longest_economic_input(int u, int u2)
{
    int ll = INT_MAX;
    int m = NONMTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    for_all_material_types(m) 
      ll = min(ll, longest_economic_input(u, u2, m));
    return ll;
}

//! Cache of longest direct economic inputs.
int *cv__longest_economic_inputs = NULL;

//! Longest direct economic input from any utype to u.

int
longest_economic_input(int u)
{
    int ll = -1;
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    /* Calculate everything if value is not cached. */
    if (!cv__longest_economic_inputs) {
	cv__longest_economic_inputs = (int *)xmalloc(numutypes * sizeof(int));
	/* Calculate for each utype. */
	for_all_unit_types(u2) {
	    ll = -1;
	    /* Calculate against each utype. */
	    for_all_unit_types(u3)
	      ll = max(ll, longest_economic_input(u2, u3));
	    cv__longest_economic_inputs[u] = ll;
	}
    }
    /* Return cached value. */
    return cv__longest_economic_inputs[u];
}

//! Materials productivity on given known cell for given utype and side.

void
productivity_on_known(int *p_mtypes, int u, Side *side, int x, int y)
{
    int x1 = -1, y1 = -1, dir = -1, ptivity = 0, ptivityadj = 0;
    int tv = UNSEEN;
    int t = NONTTYPE;
    int m = NONMTYPE;

    assert_error(p_mtypes, "Attempted to access a NULL materials array");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    for_all_material_types(m)
      p_mtypes[m] = 0;
    /* Cannot produce anything outside the playing area. */
    if (!inside_area(x, y))
      return;
    /* Return 0 if cell is not known. */
    tv = terrain_view(side, x, y);
    if (tv == terrain_view(side, x, y))
      return;
    t = vterrain(tv);
    /* Determine productivity for each material. */
    for_all_material_types(m) {
	ptivity = ut_productivity(u, t);
	ptivity = max(ptivity, um_productivity_min(u, m));
	ptivity = min(ptivity, um_productivity_max(u, m));
	ptivityadj = 0;
	for_all_directions(dir) {
            if (point_in_dir(x, y, dir, &x1, &y1)) {
                tv = terrain_view(side, x1, y1);
                if (UNSEEN == tv)
                  continue;
                ptivityadj =
                    max(ptivityadj, ut_productivity_adj(u, vterrain(tv)));
            }
        }
	p_mtypes[m] = ptivity + ptivityadj;
    }
}

//! Can u on a given side survive on a given known cell?

int
can_survive_on_known(int u, Side *side, int x, int y, int *p_without)
{
    int rslt = A_ANY_OK;
    int tv = UNSEEN;
    int t = NONTTYPE;
    int m = NONMTYPE;
    int amtprod = 0;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    /* Cannot survive in a place where we simply cannot be. */
    if (!valid(rslt = can_be_on_known(u, side, x, y, p_without)))
      return rslt;
    /* Acquire side's terrain view. */
    tv = terrain_view(side, x, y);
    /* Convert view to ttype. */
    t = vterrain(tv);
    /* If u can have accidents on, vanish on, or wreck on, 
	then we say that it cannot survive on. */
    if (ut_accident_vanish(u, t) 
	|| ut_vanishes_on(u, t) || ut_wrecks_on(u, t))
      return A_ANY_CANNOT_DO;
    /* If consumption > production, and u can starve, 
	then we have also have trouble. */
    productivity_on_known(tmp_m_array, u, side, x, y);
    for_all_material_types(m) {
	if (0 >= um_hp_per_starve(u, m))
	  continue;
	amtprod = (um_base_production(u, m) * tmp_m_array[m]) / 100;
	if (um_base_consumption(u, m) > amtprod) {
	    if (!mobile(u))
		return A_ANY_NO_MATERIAL;
	    // TODO: Consider immobile in-length suppliers for immobile unit.
	    // TODO: Consider cell in-length suppliers for immobile unit.
	    // TODO: Consider treasury for any unit.
	}
    }
    return A_ANY_OK;
}

//! Can a given unit survive a given known cell?

int
can_survive_on_known(Unit *survivor, int x, int y, int *p_without)
{
    assert_error(in_play(survivor), "Attempted to access an out-of-play unit");
    return 
	can_survive_on_known(survivor->type, survivor->side, x, y, p_without);
}

//! Can a given unit refuel another given unit with a given fuel?

int
can_refuel(Unit *supplier, Unit *demander, int m)
{
    int rslt = A_ANY_CANNOT_DO;
    int ud = NONUTYPE, us = NONUTYPE;

    assert_error(in_play(supplier), "Attempted to access an out-of-play unit");
    assert_error(in_play(demander), "Attempted to access an out-of-play unit");
    assert_error(is_material_type(m), 
		 "Attempted to reference an invalid mtype");
    ud = demander->type;
    us = supplier->type;
    if (0 >= um_consumption_per_move(ud, m))
      return A_ANY_OK;
    if (0 < supplier->supply[m]) 
      return A_ANY_OK;
    else {
	if (!um_storage_x(us, m))
	  return A_ANY_CANNOT_DO;
	else
	  return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

int
can_create_completed_unit(Unit *actor, Unit *creator, int u3)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(is_active(actor), 
		 "AI: Attempted to access an inactive actor");
    assert_error(in_play(creator), 
		 "AI: Attempted to access an inactive creator");
    assert_error(is_unit_type(u3), 
		 "AI: Encountered an invalid unit type");
    rslt = can_create_common(actor, creator, u3, creator->x, creator->y);
    if (valid(rslt) || (A_CONSTRUCT_NO_TOOLING == rslt)) {
	if (uu_creation_cp(creator->type, u3) >= u_cp(u3))
	  return A_ANY_OK;
    }
    return rslt;
}

int 
can_construct_any(int u, Side *side)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    for_all_unit_types(u2) {
	if (side_can_build(side, u2)
	    && (could_create(u, u2) || could_build(u, u2)))
	  return A_ANY_OK;
    }
    return A_ANY_CANNOT_DO;
}

int
can_construct(Unit *actor, Unit *constructor, int u3)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(is_active(actor), 
		 "AI: Attempted to access an inactive actor");
    assert_error(is_active(constructor), 
		 "AI: Attempted to access an inactive constructor");
    assert_error(is_unit_type(u3), 
		 "AI: Encountered an invalid unit type");
    rslt = 
	can_create_common(
	    actor, constructor, u3, constructor->x, constructor->y);
    if (valid(rslt) || (A_CONSTRUCT_NO_TOOLING == rslt)) { 
	rslt = can_create_completed_unit(actor, constructor, u3);
	if (valid(rslt) || (A_CONSTRUCT_NO_TOOLING == rslt))
	  return A_ANY_OK;
	else {
	    rslt = can_build(actor, constructor, u3);
	    if (valid(rslt) || (A_CONSTRUCT_NO_TOOLING == rslt))
	      return A_ANY_OK;
	}
    }
    return rslt;
}

int
can_construct_any(Unit *actor, Unit *constructor)
{
    int rslt = A_ANY_CANNOT_DO;
    int u3 = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), 
		 "AI: Attempted to access an inactive actor");
    assert_error(in_play(constructor), 
		 "AI: Attempted to access an inactive constructor");
    for_all_unit_types(u3) {
	if (valid(rslt = can_construct(actor, constructor, u3)))
	  return rslt;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

int
can_repair_any(Unit *actor, Unit *repairer)
{
    int rslt = A_ANY_CANNOT_DO;
    int u3 = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), 
		 "AI: Attempted to access an inactive actor");
    assert_error(in_play(repairer), 
		 "AI: Attempted to access an inactive repairer");
    for_all_unit_types(u3) {
	if (valid(rslt = can_repair(actor, repairer, u3)))
	  return rslt;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

/* Combat Questions */

int
fire_damage_mean(int u, int u2)
{
    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid firer unit type");
    assert_error(is_unit_type(u2), 
"AI Combat Assessor: Encountered an invalid defender unit type");
    if (!could_fire_at(u, u2))
	return 0;
    return dice1_roll_mean((DiceRep)fire_damage(u, u2));
}

int
attack_damage_mean(int u, int u2)
{
    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid attacker unit type");
    assert_error(is_unit_type(u2), 
"AI Combat Assessor: Encountered an invalid defender unit type");
    if (!could_attack(u, u2))
	return 0;
    return dice1_roll_mean((DiceRep)uu_damage(u, u2));
}

int
detonate_0dist_damage_mean(int u, int u2)
{
    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid detonator unit type");
    assert_error(is_unit_type(u2), 
"AI Combat Assessor: Encountered an invalid detonation victim unit type");
    if (!could_damage_by_0dist_detonation(u, u2))
	return 0;
    return dice1_roll_mean((DiceRep)uu_detonation_damage_at(u, u2));
}

int
detonate_ranged_damage_mean(int u, int u2)
{
    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid detonator unit type");
    assert_error(is_unit_type(u2), 
"AI Combat Assessor: Encountered an invalid detonation victim unit type");
    if (!could_damage_by_ranged_detonation(u, u2))
	return 0;
    return dice1_roll_mean((DiceRep)uu_detonation_damage_adj(u, u2));
}

int
detonate_damage_mean(int u, int u2)
{
    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid detonator unit type");
    assert_error(is_unit_type(u2), 
"AI Combat Assessor: Encountered an invalid detonation victim unit type");
    return ((detonate_0dist_damage_mean(u, u2) 
	    + detonate_ranged_damage_mean(u, u2)) / 2);
}

int
n_detonations_available(int u)
{
    int hppd = 0, dets = 0;

    assert_error(is_unit_type(u), 
"AI Combat Assessor: Encountered an invalid detonator unit type");
    hppd = u_hp_per_detonation(u);
    if (!hppd)
	return INT_MAX;
    dets = u_hp(u) / hppd;
    if (u_hp(u) % hppd)
      ++dets;
    return dets;
}

int
could_counterattack(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_attack(u, u2) && (0 < uu_counterattack(u2, u)));
}

int
could_countercapture(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    return (could_capture(u, u2, side) && (0 < uu_countercapture(u2, u)));
}

int
could_prevent_capture_of(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    /* Could u, as an occ of u2, prevent u2's capture? */
    if (could_be_occupant_of(u, u2)) {
	if (g_prot_resists_capture()) {
	    if (!uu_protection(u, u2))
	      return TRUE;
	}
	if (!uu_occ_allows_capture_of(u, u2))
	  return TRUE;
    }
    /* Could u, as a neighbor of u2, prevent u2's capture? */
    if (g_prot_resists_capture()) {
	if (!uu_stack_protection(u, u2))
	  return TRUE;
	if (!uu_cellwide_protection_for(u, u2))
	  return TRUE;
    }
    if (!uu_stack_neighbor_allows_capture_of(u, u2))
      return TRUE;
    if (!uu_any_neighbor_allows_capture_of(u, u2))
      return TRUE;
    return FALSE;
}

//! Could u prevent capture by u2?

int
could_prevent_capture_by(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    /* Could u as an occ of some utype prevent capture by u2? */
    if (!uu_occ_allows_capture_by(u, u2))
      return TRUE;
    /* Could u, as a neighbor of u2, prevent u2's capture? */
    if (!uu_stack_neighbor_allows_capture_by(u, u2))
      return TRUE;
    if (g_prot_resists_capture()) {
	if (!uu_cellwide_protection_against(u, u2))
	  return TRUE;
    }
    if (!uu_any_neighbor_allows_capture_by(u, u2))
      return TRUE;
    return FALSE;
}

//! Could u damage u2 by firing?

int
could_damage_by_fire(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_fire_at(u, u2) && (0 < fire_damage(u, u2)));
}

//! Could u damage u2 by attacks?

int
could_damage_by_attacks(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_attack(u, u2) && (0 < uu_damage(u, u2)));
}

//! Could u destroy u2 by firing?

int
could_destroy_by_fire(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_damage_by_fire(u, u2) && !uu_hp_min(u, u2));
}

//! Can a given unit destroy a given utype by firing?

int
can_destroy_by_fire(Unit *actor, Unit *destroyer, int u)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), 
		 "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (could_destroy_by_fire(destroyer->type, u)
	&& can_fire_at(actor, destroyer, u))
      return A_ANY_OK;
    return rslt;
}

//! Can a given unit destroy >= 1 enemy utypes by firing?

int
can_destroy_any_by_fire(Unit *actor, Unit *destroyer)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (valid(rslt = can_destroy_by_fire(actor, destroyer, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

//! Could u destroy u2 by attacking?

int
could_destroy_by_attacks(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_damage_by_attacks(u, u2) && !uu_hp_min(u, u2));
}

//! Can a given unit destroy a given utype by attacks?

int
can_destroy_by_attacks(Unit *actor, Unit *destroyer, int u)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (could_destroy_by_attacks(destroyer->type, u)
	&& can_attack(actor, destroyer, u))
      return A_ANY_OK;
    return rslt;
}

//! Can a given unit destroy >= 1 enemy utypes by attacks?

int
can_destroy_any_by_attacks(Unit *actor, Unit *destroyer)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (valid(rslt = can_destroy_by_attacks(actor, destroyer, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u damage u2 by combat?

int
could_damage_by_combat(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_damage_by_fire(u, u2) || could_damage_by_attacks(u, u2));
}

//! Can a given unit destroy >= 1 enemy utypes through combat?

int
can_destroy_any_by_combat(Unit *acpsrc, Unit *destroyer)
{
    int rslt = A_ANY_CANNOT_DO;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(acpsrc), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), 
		 "Attempted to access an out-of-play unit");
    if (valid(rslt = can_destroy_any_by_fire(acpsrc, destroyer)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    if (valid(rslt = can_destroy_any_by_attacks(acpsrc, destroyer)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

int
could_damage_by_detonations(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (uu_detonation_damage_at(u, u2) 
	    || uu_detonation_damage_adj(u, u2));
}

int
could_destroy_by_detonations(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_damage_by_detonations(u, u2) && !uu_hp_min(u, u2));
}

int
can_destroy_any_by_detonations(int u, Side *side)
{
    int rslt = A_ANY_CANNOT_DO;
    int u2 = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    for_all_unit_types(u2) {
	if (could_destroy_by_detonations(u, u2))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

int
can_destroy_any_by_detonations(Unit *acpsrc, Unit *exploder)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(acpsrc), "Attempted to access an out-of-play unit");
    assert_error(in_play(exploder), 
		 "Attempted to access an out-of-play unit");
    if (!valid(rslt = can_detonate(acpsrc, exploder)))
      return rslt;
    return can_destroy_any_by_detonations(exploder->type, exploder->side);
}

//! Could u damage u2 by any means?

int
could_damage(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_damage_by_combat(u, u2) 
	    || could_damage_by_detonations(u, u2));
}

//! Could u damage >= 1 enemy utypes by any means?

int
could_damage_any(int u)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    for_all_unit_types(u2) {
	if (could_damage(u, u2))
	  return TRUE;
    }
    return FALSE;
}

//! Can a given unit destroy >= 1 enemy utypes by any means?

int
can_destroy_any(Unit *acpsrc, Unit *destroyer)
{
    int rslt = A_ANY_CANNOT_DO;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(acpsrc), "Attempted to access an out-of-play unit");
    assert_error(in_play(destroyer), 
		 "Attempted to access an out-of-play unit");
    if (valid(rslt = can_destroy_any_by_combat(acpsrc, destroyer)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    if (valid(rslt = can_destroy_any_by_detonations(acpsrc, destroyer)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return rslt;
}

//! Can a given unit capture a given utype by fire?

int
can_capture_by_fire(Unit *actor, Unit *captor, int u)
{
    int rslt = A_ANY_CANNOT_DO;
    Side *side = NULL;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(captor), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (!could_fire_at(captor->type, u))
      return A_ANY_CANNOT_DO;
    for_all_sides(side) {
	if (valid(rslt = can_capture(actor, captor, u, side)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit capture a given utype by attacks?

int
can_capture_by_attacks(Unit *actor, Unit *captor, int u)
{
    int rslt = A_ANY_CANNOT_DO;
    Side *side = NULL;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(captor), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (!could_attack(captor->type, u))
      return A_ANY_CANNOT_DO;
    for_all_sides(side) {
	if (valid(rslt = can_capture(actor, captor, u, side)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit capture >= 1 enemy utypes in any way?

int
can_capture_any(Unit *actor, Unit *captor)
{
    int rslt = A_ANY_CANNOT_DO;
    Side *side = NULL;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(captor), "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	for_all_sides(side) {
	    if (valid(rslt = can_capture(actor, captor, u, side)))
	      return A_ANY_OK;
	    else if (A_ANY_NO_ACP == rslt)
	      noacp = TRUE;
	    else if (A_ANY_NO_MATERIAL == rslt)
	      nomat = TRUE;
	}
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit countercapture >= 1 enemy utypes in any way?

int
can_countercapture_any(Unit *actor, Unit *countercaptor)
{
    int rslt = A_ANY_CANNOT_DO;
    Side *side = NULL;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(countercaptor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	for_all_sides(side) {
	    if (could_countercapture(countercaptor->type, u, side)
		&& valid(rslt = can_capture(actor, countercaptor, u, side)))
	      return A_ANY_OK;
	    else if (A_ANY_NO_ACP == rslt)
	      noacp = TRUE;
	    else if (A_ANY_NO_MATERIAL == rslt)
	      nomat = TRUE;
	}
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit prevent capture of >= 1 friendly utypes?
/*! \todo Check if our side or ally possesses each utype? */

int
can_prevent_capture_of_any(Unit *actor, Unit *anticaptor)
{
    int u = NONUTYPE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(anticaptor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (could_prevent_capture_of(anticaptor->type, u))
	  return A_ANY_OK;
    }
    return A_ANY_CANNOT_DO;
}

//! Could u defeat u2 by fire?

int
could_defeat_by_fire(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    return (could_destroy_by_fire(u, u2) || could_capture_by_fire(u, u2, side));
}

//! Can a given unit defeat a given utype by fire?

int
can_defeat_by_fire(Unit *actor, Unit *firer, int u)
{
    int rslt = A_ANY_CANNOT_DO;
    int noacp = FALSE, nomat = FALSE;

    /* Sanity checks. */
    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(firer), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    /* Can destroy enemy utype by fire? */
    if (valid(rslt = can_destroy_by_fire(actor, firer, u)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Can capture enemy utype by fire? */
    if (valid(rslt = can_capture_by_fire(actor, firer, u)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Return proper result. */
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u defeat u2 by attacks?

int
could_defeat_by_attacks(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    return (could_destroy_by_attacks(u, u2) 
	    || could_capture_by_attacks(u, u2, side));
}

//! Can a given unit defeat a given utype with attacks?

int
can_defeat_by_attacks(Unit *actor, Unit *attacker, int u)
{
    int rslt = A_ANY_CANNOT_DO;
    int noacp = FALSE, nomat = FALSE;

    /* Sanity checks. */
    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(attacker), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    /* Can destroy enemy utype by attacks? */
    if (valid(rslt = can_destroy_by_attacks(actor, attacker, u)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Can capture enemy utype by attacks? */
    if (valid(rslt = can_capture_by_attacks(actor, attacker, u)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Return proper result. */
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u defeat u2 by counterattacks?

int
could_defeat_by_counterattacks(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    return (could_counterattack(u, u2) && could_defeat_by_attacks(u, u2, side));
}

//! Can a given unit defeat a given utype with counterattacks?

int
can_defeat_by_counterattacks(Unit *actor, Unit *counterattacker, int u)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(counterattacker), 
		 "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (could_counterattack(counterattacker->type, u)
	&& valid(rslt = can_defeat_by_attacks(actor, counterattacker, u)))
      return A_ANY_OK;
    return rslt;
}

//! Can a given unit defeat >= 1 enemy utypes with counterattacks?

int
can_defeat_any_by_counterattacks(Unit *actor, Unit *counterattacker)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(counterattacker), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (valid(
	    rslt = can_defeat_by_counterattacks(actor, counterattacker, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could an utype overwatch against another utype?

int
could_overwatch_against(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_fire_at(u, u2) && (0 < uu_zoo_range(u, u2)));
}

//! Could u overwatch against and defeat u2?

int
could_overwatch_against_and_defeat(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    return (could_overwatch_against(u, u2)
	    && could_defeat_by_fire(u, u2, side));
}

//! Can a given unit overwatch against and defeat a given utype?

int
can_overwatch_against_and_defeat(Unit *acpsrc, Unit *overwatcher, int u)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(acpsrc), "Attempted to access an out-of-play unit");
    assert_error(in_play(overwatcher), 
		 "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (could_overwatch_against(overwatcher->type, u)
	&& valid(rslt = can_defeat_by_fire(acpsrc, overwatcher, u)))
      return A_ANY_OK;
    return rslt;
}

//! Can a given unit provide defeating overwatch against >= 1 enemy utypes?

int
can_overwatch_against_and_defeat_any(Unit *acpsrc, Unit *overwatcher)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(acpsrc), "Attempted to access an out-of-play unit");
    assert_error(in_play(overwatcher), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (valid(
	    rslt = can_overwatch_against_and_defeat(acpsrc, overwatcher, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    /* Return proper result. */
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u defend against u2 in any way?
/*! \todo Support counterfire. */

int
could_defend_against(int u, int u2, Side *side)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    if (could_overwatch_against_and_defeat(u, u2, side))
      return TRUE;
    /* TODO: Support counterfire. */
    if (could_defeat_by_counterattacks(u, u2, side))
      return TRUE;
    if (could_countercapture(u, u2, side))
      return TRUE;
    if (could_prevent_capture_by(u, u2))
      return TRUE;
    return FALSE;
}

//! Could u defend against >= 1 enemy utypes in any way?

int
could_defend_against_any(int u)
{
    int u2 = NONUTYPE;
    Side *side = NULL;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    for_all_unit_types(u2) {
	for_all_sides(side) {
	    if (could_defend_against(u, u2, side))
	      return TRUE;
	}
    }
    return FALSE;
}

//! Can a given unit defend against >= 1 enemy utypes in any way?
/*! \todo Support counterfire. */

int
can_defend_against_any(Unit *actor, Unit *defender)
{
    int rslt = A_ANY_CANNOT_DO;
    int noacp = FALSE, nomat = FALSE;

    /* Sanity checks. */
    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(defender), "Attempted to access an out-of-play unit");
    /* Can overwatch against and defeat any enemies? */
    if (valid(rslt = can_overwatch_against_and_defeat_any(actor, defender)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* TODO: Counterfire. */
    /* Can defeat any enemies by counterattacking? */
    if (valid(rslt = can_defeat_any_by_counterattacks(actor, defender)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Can countercapture any enemies? */
    if (valid(rslt = can_countercapture_any(actor, defender)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Can prevent capture of any friendly units by any enemy units? */
    if (valid(rslt = can_prevent_capture_of_any(actor, defender)))
      return A_ANY_OK;
    else if (A_ANY_NO_ACP == rslt)
      noacp = TRUE;
    else if (A_ANY_NO_MATERIAL == rslt)
      nomat = TRUE;
    /* Return proper result. */
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u explore?

int
could_explore(int u)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    return ((-1 < u_vision_range(u)) && mobile(u));
}

//! Can a given unit explore?

int
can_explore(Unit *actor, Unit *explorer)
{
    int rslt = A_ANY_CANNOT_DO;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(explorer), "Attempted to access an out-of-play unit");
    if ((-1 < u_vision_range(explorer->type))
	&& valid(rslt = can_move(actor, explorer)))
      return A_ANY_OK;
    return rslt;
}

//! Can a given unit construct explorers?

int
can_construct_explorers(Unit *actor, Unit *constructor)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(constructor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (could_explore(u) 
	    && valid(rslt = can_construct(actor, constructor, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Could u colonize?

int
could_colonize(int u)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    for_all_unit_types(u2) {
	if (!could_create(u, u2))
	  continue;
	for_all_unit_types(u3) {
	    if (could_create(u2, u3))
	      return TRUE;
	}
    }
    return FALSE;
}

//! Can a given utype on a given side colonize?

int
can_colonize(int u, Side *side)
{
    int rslt = A_ANY_CANNOT_DO;
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(side, "Attempted to access a NULL side");
    for_all_unit_types(u2) {
	if (!side_can_build(side, u2))
	  continue;
	if (!could_create(u, u2))
	  continue;
	for_all_unit_types(u3) {
	    if (could_create(u2, u3))
	      return A_ANY_OK;
	}
    }
    return rslt;
}

//! Can a given unit colonize?

int 
can_colonize(Unit *actor, Unit *colonizer)
{
    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(colonizer), "Attempted to access an out-of-play unit");
    return can_colonize(colonizer->type, actor->side);
}

//! Can a given unit construct colonizers?

int
can_construct_colonizers(Unit *actor, Unit *constructor)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(constructor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (can_colonize(u, actor->side) 
	    && valid(rslt = can_construct(actor, constructor, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit construct any offensive units?

int
can_construct_offenders(Unit *actor, Unit *constructor)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(constructor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (could_damage_any(u) 
	    && valid(rslt = can_construct(actor, constructor, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

//! Can a given unit construct any defensive units?

int
can_construct_defenders(Unit *actor, Unit *constructor)
{
    int rslt = A_ANY_CANNOT_DO;
    int u = NONUTYPE;
    int noacp = FALSE, nomat = FALSE;

    assert_error(in_play(actor), "Attempted to access an out-of-play unit");
    assert_error(in_play(constructor), 
		 "Attempted to access an out-of-play unit");
    for_all_unit_types(u) {
	if (could_defend_against_any(u) 
	    && valid(rslt = can_construct(actor, constructor, u)))
	  return A_ANY_OK;
	else if (A_ANY_NO_ACP == rslt)
	  noacp = TRUE;
	else if (A_ANY_NO_MATERIAL == rslt)
	  nomat = TRUE;
    }
    if (noacp)
      return A_ANY_NO_ACP;
    if (nomat)
      return A_ANY_NO_MATERIAL;
    return A_ANY_CANNOT_DO;
}

/* Unit and Unit Type Analysis and Worth Functions for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Unit and Unit Type Analysis and Worth Functions for AIs

    Part of the AI API, Level 2. 

    Provides useful functions that an AI implementation may use to analyze
    and to assign a worth to units and unit types.

    The functions in this file provide evaluations regarding units and
    unit types that are useful in decision-making.

    \note Nothing in this file should be required for AI implementation.

*/

#include "conq.h"
#include "kernel.h"
#include "aiunit.h"
#include "aiunit2.h"

using namespace Xconq;
using namespace Xconq::AI;

/* Unit Vision and Detection Worths */

//! Basic worth as a seer.

int
seer_worth(int u)
{
    int worth = 0, nighteffect = 0;
    int u2 = NONUTYPE;
    int t = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    /* If utype could see... */
    if (could_see(u)) {
	/* Ability to see other utypes is basis. */
	for_all_unit_types(u2) {
	    worth += min(100, uu_see_at(u, u2));
	    if (0 < u_vision_range(u))
	      worth += 6 * min(100, uu_see_adj(u, u2));
	    if (1 < u_vision_range(u))
	      worth += 
		((radius_covers_n_cells(u_vision_range(u)) - 7) * 
		  min(100, uu_see(u, u2)));
	    if (0 < uu_see_mistake(u, u2) 
		&& (uu_see_mistake_range_min(u, u2) <= u_vision_range(u))) {
		worth -= 
		    ((radius_covers_n_cells(u_vision_range(u)) -
		      radius_covers_n_cells(uu_see_mistake_range_min(u, u2))) *
		     (uu_see_mistake(u, u2) / 200));
	    }
	}
	worth = max(0, worth / 100);
	// Night's effect on vision.
	for_all_terrain_types(t)
	  nighteffect += min(100, ut_vision_night_effect(u, t));
	nighteffect /= numttypes;
	worth -= ((worth / 2) * (100 - nighteffect)) / 100;
	worth = max(0, worth);
    }
    // Else utype could not see, then it is worthless as a seer. 
    else
      worth = 0;
    return max(0, worth);
}

//! Set basic worth as a seer.

void
set_seer_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aiseerworth = n;
}

//! Set basic worths as seers, if necessary.

void
maybe_set_seer_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_seer_worth) == u_ai_seer_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = seer_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_seer_worth(u, 
			 normalize_on_pmscale(tmp_u_array[u], worthmax, 10000));
    }
}

/* Size Estimates */

int
could_meet_size_goal(int u, int x, int y)
{
    static int *p_materials;

    int ux = -1, uy = -1;
    int m = NONMTYPE;
    int szmax = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Size Assessor: Encountered invalid unit type");
    assert_error(inside_area(x, y),
		 "AI Size Assessor: Cannot check for out-of-area coords");
    if (!u_advanced(u))
	return TRUE;
    // Allocate the materials storage array, if necessary.
    if (!p_materials)
	p_materials = (int *)xmalloc(nummtypes * sizeof(int));
    // Always reset the array.
    for_all_material_types(m)
	p_materials[m] = 0;
    // For all cells within reach, find out how much material is yielded up.
    for_all_cells_within_range(x, y, u_reach(u), ux, uy) {
	if (!inside_area(ux, uy))
	    continue;
	for_all_material_types(m) {
	    p_materials[m] += production_at(ux, uy, m);
	}
    }
    for_all_material_types(m) {
	if (0 < um_consumption_per_size(u, m))
	    szmax = min(szmax, p_materials[m] / um_consumption_per_size(u, m));
    }
    if (szmax < u_ai_minimal_size_goal(u))
	return FALSE;
    return TRUE;
}

/* ACP Estimates */

namespace Xconq {
    //! Cache for maximum ACP of utypes without enhancing effects.
    int *cv__acp_max_wo_effects = NULL;
    //! Cache for maximum ACP per turn of utypes without enhancing effects.
    int *cv__acp_per_turn_max_wo_effects = NULL;
}

//! Maximum ACP u can have without enhancing effects.

int
acp_max_wo_effects(int u)
{
    int acp = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    if (!Xconq::cv__acp_max_wo_effects) {
	Xconq::cv__acp_max_wo_effects = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    /* ACP-indep u2 has no ACP. */
	    if (u_acp_independent(u2)) 
	      continue;
	    /* Calculate ACP. */
	    if (uprop_i_default(u_acp_max) != u_acp_max(u2))
	      acp = u_acp_max(u2);
	    else if (uprop_i_default(u_acp_turn_max) != u_acp_turn_max(u2))
	      acp = u_acp_turn_max(u2);
	    else
	      acp = u_acp(u2);
	    acp = max(acp, u_acp_turn_min(u2));
	    acp = max(acp, u_acp_min(u2));
	    Xconq::cv__acp_max_wo_effects[u2] = acp;
	}
    }
    return Xconq::cv__acp_max_wo_effects[u];
}

//! Maximum ACP per turn u can have without enhancing effects.

int
acp_per_turn_max_wo_effects(int u)
{
    int acp = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    if (!Xconq::cv__acp_per_turn_max_wo_effects) {
	Xconq::cv__acp_per_turn_max_wo_effects = 
	    (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    /* ACP-indep u2 has no ACP. */
	    if (u_acp_independent(u2))
	      continue;
	    /* Calculate clipped basic ACP. */
	    acp = u_acp(u2);
	    if (uprop_i_default(u_acp_turn_max) != u_acp_turn_max(u2))
	      acp = min(acp, u_acp_turn_max(u2));
	    acp = max(acp, u_acp_turn_min(u2));
	    if (uprop_i_default(u_acp_max) != u_acp_max(u2))
	      acp = min(acp, u_acp_max(u2));
	    acp = max(acp, u_acp_min(u2));
	    Xconq::cv__acp_per_turn_max_wo_effects[u2] = acp;
	}
    }
    return Xconq::cv__acp_per_turn_max_wo_effects[u];
}

/* Speed Estimates */

namespace Xconq {
    //! Cache for maximum speed of utypes without enhancing effects.
    int *cv__speed_max_wo_effects = NULL;
}

//! Maximum speed u can have without enhancing effects.

int
speed_max_wo_effects(int u)
{
    int speed = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    if (!Xconq::cv__speed_max_wo_effects) {
	Xconq::cv__speed_max_wo_effects = 
	    (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    /* Immobile utype has no speed. */
	    if (!mobile(u2))
	      continue;
	    /* Calculate clipped basic speed. */
	    speed = u_speed(u2);
	    speed = min(speed, u_speed_max(u2));
	    speed = max(speed, u_speed_min(u2));
	    Xconq::cv__speed_max_wo_effects[u2] = speed;
	}
    }
    return Xconq::cv__speed_max_wo_effects[u];
}

/* MP Estimates */

namespace Xconq{
    //! Cache for maximum MP per turn of utypes.
    int *cv__mp_per_turn_max = NULL;
}

//! Maximum MP u can get per turn.

int
mp_per_turn_max(int u)
{
    int acpmax = 0, speedmax = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    if (!Xconq::cv__mp_per_turn_max) {
	Xconq::cv__mp_per_turn_max = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    /* Immobile utypes have 0 MP per turn. */
	    if (!mobile(u2))
	      continue;
	    /* Calculate modest max ACP per turn. */
	    acpmax = acp_per_turn_max_wo_effects(u2);
	    /* If u is ACP-indep, then tell it no (for now). */
	    if (u_acp_independent(u2) || (0 >= acpmax))
	      return 0;
	    /* Calculate modest max speed. */
	    speedmax = speed_max_wo_effects(u2);
	    /* Calculate max MP per turn. */
	    Xconq::cv__mp_per_turn_max[u2] = 
		(((acpmax * speedmax) / 100) + u_free_mp(u2));
	}
    }
    return Xconq::cv__mp_per_turn_max[u];
}

/* Number of Moves Estimates */

namespace Xconq{
    //! Cache of maximum moves utypes can make per turn on t.
    int **cv__moves_per_turn_max_on_t = NULL;
}

//! Maximum moves u can make per turn on t.

int
moves_per_turn_max(int u, int t)
{
    int moves = 0, mpmax = 0, cost = 0;
    int u2 = NONUTYPE;
    int t2 = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_terrain_type(t), 
		 "Attempted to manipulate an invalid ttype");
    if (!Xconq::cv__moves_per_turn_max_on_t) {
	Xconq::cv__moves_per_turn_max_on_t = 
	    (int **)xmalloc(numutypes * sizeof(int *));
	for_all_unit_types(u2)
	  moves_per_turn_max(u2, t);
    }
    if (!Xconq::cv__moves_per_turn_max_on_t[u]) {
	Xconq::cv__moves_per_turn_max_on_t[u] =
	    (int *)xmalloc(numttypes * sizeof(int));
	for_all_terrain_types(t2) {
	    /* Immobile units have 0 moves per turn. */
	    if (!mobile(u))
	      continue;
	    /* Calculate max MP per turn. */
	    mpmax = mp_per_turn_max(u);
	    /* Calculate enter+leave cost.
		Does not consider border/connector traversal. */
	    cost = ut_mp_to_enter(u, t2) + ut_mp_to_leave(u, t2);
	    /* Calculate moves per turn. */
	    /* If cost is apparently 0 or negative, 
		we still need to charge 1 MP per move. */
	    if (0 >= cost) 
	      moves = mpmax;
	    else
	      moves = mpmax / cost;
	    Xconq::cv__moves_per_turn_max_on_t[u][t2] = moves;
	}
    }
    return Xconq::cv__moves_per_turn_max_on_t[u][t];
}

namespace Xconq {
    //! Cache of max movement range of u if producing m on t.
    int ***cv__move_range_max_on_t_with_m = NULL;
    //! Cache of max movement range of u on t.
    int **cv__move_range_max_on_t = NULL;
}

//! Max movement range of u if producing m on t, and given an amount of m.
/* 
    If amount of m is negative, 
    then assume unit storage or side treasury capacity.
*/

int
move_range_max_on(int u, int t, int m, int amt)
{
    int range = INT_MAX, ptivity = 0, netprod = 0, moves = 0, consump = 0;
    int amt2 = 0;
    int u2 = NONUTYPE;
    int t2 = NONTTYPE;
    int m2 = NONMTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_terrain_type(t), 
		 "Attempted to manipulate an invalid ttype");
    assert_error(is_material_type(m), 
		 "Attempted to manipulate an invalid mtype");
    /* Prepare per-utype cache, if needed. */
    if ((0 > amt) && !Xconq::cv__move_range_max_on_t_with_m) {
	Xconq::cv__move_range_max_on_t_with_m = 
	    (int ***)xmalloc(numutypes * sizeof(int **));
	for_all_unit_types(u2)
	  move_range_max_on(u2, t, m, amt);
    }
    /* Prepare per-ttype cache, if needed. */
    if ((0 > amt) && !Xconq::cv__move_range_max_on_t_with_m[u]) {
	Xconq::cv__move_range_max_on_t_with_m[u] = 
	    (int **)xmalloc(numttypes * sizeof(int *));
	for_all_terrain_types(t2)
	  move_range_max_on(u, t2, m, amt);
    }
    /* Prepare per-mtype cache, if needed. */
    if ((0 > amt) && !Xconq::cv__move_range_max_on_t_with_m[u][t]) {
	Xconq::cv__move_range_max_on_t_with_m[u][t] = 
	    (int *)xmalloc(numttypes * sizeof(int));
	for_all_material_types(m2) {
	    if (could_take_from_treasury(u, m2))
	      amt2 = g_treasury_size();
	    else
	      amt2 = um_storage_x(u, m2);
	    Xconq::cv__move_range_max_on_t_with_m[u][t][m2] = 
		move_range_max_on(u, t, m2, amt2);
	}
    }
    /* Return cached result, if available. */
    if (0 > amt)
      return Xconq::cv__move_range_max_on_t_with_m[u][t][m];
    /* If u is immobile, then it has no operating range. */
    if (!mobile(u))
      return -1;
    /* If material is not consumed per turn or by movement, 
	then it does not affect the operating range. */
    if ((0 >= um_base_consumption(u, m)) 
	|| (0 >= um_consumption_per_move(u, m)))
      return INT_MAX;
    /* If u could never be on t, then it has no operating range on t. */
    if (!could_be_on(u, t))
      return -1;
    /* Net production at each stop on t. */
    ptivity = ut_productivity(u, t);
    ptivity = max(ptivity, um_productivity_min(u, m));
    ptivity = min(ptivity, um_productivity_max(u, m));
    netprod = (um_base_production(u, m) * ptivity * 2) / 100;
    netprod -= um_base_consumption(u, m);
    /* Max moves per turn on t. */
    moves = moves_per_turn_max(u, t);
    /* Consumption per turn. */
    consump = netprod + (moves * um_consumption_per_move(u, m));
    /* Calculate range. */
    range = (amt / consump) * moves * u_move_range(u);
    amt = amt % consump;
    if (amt)
      range += (amt / um_consumption_per_move(u, m)) * u_move_range(u);
    return range;
}

//! Max movement range of u on t, and given amounts of mtypes.
/* 
    If amount array is NULL, then pass -1 for each individual material check.
*/

int
move_range_max(int u, int t, int *amt)
{
    int range = INT_MAX;
    int u2 = NONUTYPE;
    int t2 = NONTTYPE;
    int m = NONMTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_terrain_type(t), 
		 "Attempted to manipulate an invalid ttype");
    /* Prepare per-utype cache, if needed. */
    if (!amt && !Xconq::cv__move_range_max_on_t) {
	Xconq::cv__move_range_max_on_t = 
	    (int **)xmalloc(numutypes * sizeof(int *));
	for_all_unit_types(u2)
	  move_range_max(u2, t, amt);
    }
    /* Prepare per-ttype cache, if needed. */
    if (!amt && !Xconq::cv__move_range_max_on_t[u]) {
	Xconq::cv__move_range_max_on_t[u] = 
	    (int *)xmalloc(numttypes * sizeof(int));
	for_all_material_types(m)
	  tmp_m_array[m] = -1;
	for_all_terrain_types(t2)
	  Xconq::cv__move_range_max_on_t[u][t2] = 
	    move_range_max(u, t2, tmp_m_array);
    }
    /* Return cached value, if available. */
    if (!amt)
      return Xconq::cv__move_range_max_on_t[u][t];
    /* If u is immobile, then it has no operating range. */
    if (!mobile(u))
      return -1;
    /* If u could never be on t, then it has no operating range on t. */
    if (!could_be_on(u, t))
      return -1;
    /* Determine range by most constrictive material. */
    for_all_material_types(m)
      range = min(range, move_range_max_on(u, t, m, amt[m]));
    return range;
}

//! Best movement range of u, given amounts of mtypes.

int
move_range_best(int u, int *amt)
{
    int range = -1;
    int t = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    for_all_terrain_types(t)
      range = max(range, move_range_max(u, t, amt));
    return range;
}

//! Worst movement range of u, given amounts of mtypes.

int
move_range_worst(int u, int *amt)
{
    int range = INT_MAX, range2 = -1;
    int t = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    for_all_terrain_types(t) {
	range2 = move_range_max(u, t, amt);
	if (0 > range2)
	  continue;
	range = min(range, range2);
    }
    return range;
}

//! Basic worth as a mover.

int
mover_worth(int u)
{
    int mp = 0, worth = 0, movesmean = 0, movesmax = INT_MAX;
    int availtcount = 0, safetcount = 0;
    int m = NONMTYPE;
    int t = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    /* If utype could move... */
    if (could_move(u, u)) {
	/* For ACP-dependent utypes, we use the basic MP-per-turn as a basis. */
	if (!u_acp_independent(u)) {
	    if (0 < u_speed(u)) {
		mp = (u_acp(u) * u_speed(u)) / 100;
	    }
	    else 
	      mp = 0;
	    /* Add in free MP. */
	    mp += u_free_mp(u);
	}
	/* Determine max material-limited moves per turn. */
	for_all_material_types(m) {
	    if (0 < um_consumption_per_move(u, m)) {
		if (0 < um_storage_x(u, m) && !um_takes_from_treasury(u, m))
		    movesmax = min(movesmax,
				   um_storage_x(u, m) / 
				    um_consumption_per_move(u, m));
	    }
	}
	/* For ACP-dependent utypes, 
	    determine mean terrain-limited moves per turn. */
	/* TODO: Weight by popularity of terrain. */
	if (!u_acp_independent(u)) {
	    for_all_terrain_types(t) {
		if (t_is_cell(t) || t_is_coating(t)) { 
		    if (0 > ut_mp_to_leave(u, t))
		      tmp_t_array[t] = 0;
		    /* Q: Is this correct? */
		    else if (0 > ut_mp_to_enter(u, t))
		      tmp_t_array[t] = 0;
		    else if ((0 < ut_mp_to_enter(u, t)) 
			     || (0 < ut_mp_to_leave(u, t)))
		      tmp_t_array[t] = 
			mp / (ut_mp_to_enter(u, t) + ut_mp_to_leave(u, t));
		    else
		      tmp_t_array[t] = mp;
		}
		else if (t_is_connection(t) || t_is_border(t)) {
		    if (0 < ut_mp_to_traverse(u, t))
		      tmp_t_array[t] = mp / ut_mp_to_traverse(u, t);
		    else if (0 > ut_mp_to_traverse(u, t))
		      tmp_t_array[t] = 0;
		    else
		      tmp_t_array[t] = mp;
		}
	    }
	    for_all_terrain_types(t) {
		movesmean += tmp_t_array[t];
		if (0 < tmp_t_array[t])
		  ++availtcount;
	    }
	    if (availtcount)
	      movesmean /= availtcount;
	    else
	      movesmean = 0;
	    movesmean = min(movesmax, movesmean);
	}
	/* Else, ACP-independent utype. */
	else
	  movesmean = movesmax;
	/* Prepare for further worth calcs. */
	worth = movesmean * 1000;
	/* Penalize for all ttypes in which 
	    accidents, wrecks, or disappearances may occur. */
	for_all_terrain_types(t) {
	    if ((0 < ut_accident_vanish(u, t))
		|| ((0 < ut_accident_hit(u, t)) 
		    && (0 < ut_accident_damage(u, t)))
		|| ut_vanishes_on(u, t) || ut_wrecks_on(u, t))
	      tmp_t_array[t] = 1;
	    else
	      tmp_t_array[t] = 0;
	}
	for_all_terrain_types(t)
	  safetcount += (1 - tmp_t_array[t]);
	worth = (worth * safetcount) / numttypes;
    }
    /* If utype could not move, then it is worthless as a mover. */
    else
      worth = 0;
    return max(0, worth);
}

//! Set basic worth as a mover.

void
set_mover_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aimoverworth = n;
}

//! Set basic worths as movers, if necessary.

void
maybe_set_mover_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_mover_worth) == u_ai_mover_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = mover_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_mover_worth(u, 
			  normalize_on_pmscale(tmp_u_array[u], 
					       worthmax, 10000));
    }
}

//! Basic worth as a depot.

int
depot_worth(int u)
{
    int worth = 0, penalties = 0;
    int m = NONMTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    for_all_material_types(m) {
	if (0 < um_storage_x(u, m)) {
	    penalties = 0;
	    /* Storage capacity is basis. */
	    tmp_m_array[m] = um_storage_x(u, m);
	    /* Penalize or reward for receiving range from other units. */
	    if (-1 == um_inlength(u, m))
	      penalties += 2;
	    else if (0 == um_inlength(u, m))
	      ++penalties;
	    else if (1 < um_inlength(u, m))
	      tmp_m_array[m] *= um_inlength(u, m);
	    /* Penalize or reward for receiving range from cells. */
	    if (1 == g_backdrop_model()) {
		if (-1 == um_tu_in_length(u, m))
		  penalties += 2;
		else if (0 == um_tu_in_length(u, m))
		  ++penalties;
		else if (1 < um_tu_in_length(u, m))
		  tmp_m_array[m] *= um_tu_in_length(u, m);
	    }
	    /* Apply penalties. */
	    if (penalties)
	      tmp_m_array[m] /= (penalties + 1);
	}
	else 
	  tmp_m_array[m] = 0;
    }
    /* Sum per-material worths into master worth. */
    for_all_material_types(m)
      worth += tmp_m_array[m];
    return max(0, worth);
}

//! Set basic worth as a depot.

void
set_depot_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aidepotworth = n;
}

//! Set basic worths as depots, if necessary.

void
maybe_set_depot_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_depot_worth) == u_ai_depot_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = depot_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_depot_worth(u, 
			  normalize_on_pmscale(tmp_u_array[u], 
					       worthmax, 10000));
    }
}

//! Basic worth as a distributor.

int
distributor_worth(int u)
{
    int worth = 0, penalties = 0;
    int m = NONMTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    for_all_material_types(m) {
	if (0 < um_storage_x(u, m)) {
	    penalties = 0;
	    /* Storage capacity is basis. */
	    tmp_m_array[m] = um_storage_x(u, m);
	    /* Penalize or reward for receiving range from other units. */
	    if (-1 == um_outlength(u, m))
	      penalties += 2;
	    else if (0 == um_outlength(u, m))
	      ++penalties;
	    else if (1 < um_outlength(u, m))
	      tmp_m_array[m] *= um_outlength(u, m);
	    /* Penalize or reward for receiving range from cells. */
	    if (1 == g_backdrop_model()) {
		if (-1 == um_ut_out_length(u, m))
		  penalties += 2;
		else if (0 == um_ut_out_length(u, m))
		  ++penalties;
		else if (1 < um_ut_out_length(u, m))
		  tmp_m_array[m] *= um_ut_out_length(u, m);
	    }
	    /* Apply penalties. */
	    if (penalties)
	      tmp_m_array[m] /= (penalties + 1);
	}
	else 
	  tmp_m_array[m] = 0;
    }
    /* Sum per-material worths into master worth. */
    for_all_material_types(m)
      worth += tmp_m_array[m];
    return max(0, worth);
}

//! Set basic worth as a distributor.

void
set_distributor_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aidistributorworth = n;
}

//! Set basic worths as distributors, if necessary.

void
maybe_set_distributor_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_distributor_worth) == 
	    u_ai_distributor_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = distributor_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_distributor_worth(u, 
				normalize_on_pmscale(tmp_u_array[u], 
						     worthmax, 10000));
    }
}

//! Basic worth as a producer.
/*! \todo Consider occupant consumption. */

int
producer_worth(int u)
{
    int worth = 0, ptivityavg = 0, ptivity = 0;
    int m = NONMTYPE;
    int t = NONTTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    /* Average productivity. */
    for_all_terrain_types(t) {
	tmp_t_array[t] = ut_productivity(u, t);
	tmp_t_array[t] += ut_productivity_adj(u, t);
    }
    ptivityavg = 0;
    for_all_terrain_types(t)
      ptivityavg += tmp_t_array[t];
    ptivityavg /= numttypes;
    /* Account for automatic base production, and explicit production. */
    for_all_material_types(m) {
	ptivity = max(ptivityavg, um_productivity_min(u, m));
	ptivity = min(ptivity, um_productivity_max(u, m));
	tmp_m_array[m] = (um_base_production(u, m) * ptivity) / 100;
	tmp_m_array[m] = max(0, tmp_m_array[m] - um_base_consumption(u, m));
	if (0 < um_acp_to_produce(u, m))
	  tmp_m_array[m] += 
	    (um_material_per_production(u, m) / um_acp_to_produce(u, m));
    }
    /* Account for automatic base production as an occupant. */
    if (could_be_occupant(u)) {
	for_all_material_types(m) {
	    tmp_m_array[m] += um_occ_production(u, m);
	    tmp_m_array[m] = 
		max(0, 
		    tmp_m_array[m] - 
		    ((um_base_consumption(u, m) * 
		      um_consumption_as_occupant(u, m)) / 100));
	}
    }
    /* Account for advanced unit production from terrain. */
    if (u_advanced(u)) {
	for_all_material_types(m) {
	    for_all_terrain_types(t)
	      tmp_m_array[m] += tm_prod_from_terrain(t, m);
	}
    }
    /* Sum all contributions into a worth. */
    for_all_material_types(m)
      worth += tmp_m_array[m];
    return max(0, worth);
}

//! Set basic worth as a producer.

void
set_producer_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aiproducerworth = n;
}

//! Set basic worths as producers, if necessary.

void
maybe_set_producer_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_producer_worth) == u_ai_producer_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = producer_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_producer_worth(u, 
			     normalize_on_pmscale(tmp_u_array[u], 
						  worthmax, 10000));
    }
}

//! Given unit's worth as a producer on a given known cell.

int
producer_worth_on_known(Unit *producer, int x, int y)
{
    int worth = 0;
    int u = NONUTYPE;
    int m = NONMTYPE;
    int t = NONTTYPE;

    assert_error(producer, "Attempted to access an out-of-play unit");
    assert_warning_return(inside_area(x, y), 
	"Attempted to access an out-of-area cell", 0);
    u = producer->type;
    /* Can producer survive on given known cell? */
    if (!valid(can_survive_on_known(producer, x, y)))
      return 0;
    /* Production at the given cell. */
    productivity_on_known(tmp_m_array, u, producer->side, x, y);
    for_all_material_types(m)
      tmp_m_array[m] *= um_base_production(u, m);
    for_all_material_types(m)
      worth += tmp_m_array[m];
    /* Account for advanced unit production from terrain. */
    t = vterrain(terrain_view(producer->side, x, y));
    if (u_advanced(u)) {
	for_all_material_types(m)
	  worth += production_at(x, y, m);
    }
    /* Subtract off consumption. */
    for_all_material_types(m)
      worth -= um_base_consumption(u, m);
    return max(0, worth);
}

//! Basic worth as a production enhancer.

int
prod_enhancer_worth(int u)
{
    int worth = 0, ucount = 0;
    int m = NONMTYPE;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    /* Basis for production enhancement worth. */
    for_all_material_types(m) {
	tmp_m_array[m] = 0;
	if (!um_occ_add_production(u, m) 
	    && (100 == um_occ_mult_production(u, m)))
	  continue;
	tmp_m_array[m] = 100 + (100 * um_occ_add_production(u, m));
	tmp_m_array[m] = 
	    (tmp_m_array[m] * um_occ_mult_production(u, m)) / 100;
    }
    for_all_material_types(m)
      worth += tmp_m_array[m];
    /* Multiply by number of utypes this could be occupant of. */
    for_all_unit_types(u2) {
	if (could_be_occupant_of(u, u2))
	  ++ucount;
    }
    worth *= ucount;
    return max(0, worth);
}

//! Set basic worth as a production enhancer.

void
set_prod_enhancer_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aiprodenhancerworth = n;
}

//! Set basic worths as production enhancers, if necessary.

void
maybe_set_prod_enhancer_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_prod_enhancer_worth) == 
	    u_ai_prod_enhancer_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = prod_enhancer_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_prod_enhancer_worth(u, 
				  normalize_on_pmscale(tmp_u_array[u], 
						       worthmax, 10000));
    }
}

int
base_worth_for(int u, int u2)
{
    int worth = 0;
    int m = NONMTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_unit_type(u2), "Attempted to manipulate an invalid utype");
    /* If u could neither be a depot nor a producer, 
	then it cannot be a base either. */
    if ((0 >= u_ai_depot_worth(u)) && (0 >= u_ai_producer_worth(u)))
      return 0;
    /* If u2 could never occupy u, then u cannot be a base for u2. */
    if (!could_be_occupant_of(u2, u))
      return 0;
    /* Determine which materials both u and u2 store. */
    /* We assume that if u2 is storing something, then it is for consumption. */
    for_all_material_types(m) {
	if (um_storage_x(u, m) && um_storage_x(u2, m))
	  tmp_m_array[m] = um_storage_x(u, m);
	else
	  tmp_m_array[m] = 0;
    }
    /* Calculate worth. */
    for_all_material_types(m)
      worth += tmp_m_array[m];
    worth = (worth * (u_ai_depot_worth(u) + u_ai_producer_worth(u))) / 10000;
    return max(0, worth);
}

void
set_base_worth_for(int u, int u2, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_unit_type(u2), "Attempted to manipulate an invalid utype");
    assert_error(between(TABLO, n, TABHI), 
		 "Attempted to set a table out-of-bounds");
    assert_error(uuaibaseworthfor, 
		 "Attempted to set value in unallocated table");
    uuaibaseworthfor[numutypes * u + u2] = n;
}

void
maybe_set_base_worths_for(void)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;
    int i = 0;

    if (!uuaibaseworthfor)
      mustcalc = TRUE;
    if (mustcalc) {
	for (i = 0; tabledefns[i].name; ++i) {
	    if (!strcmp("ai-base-worth-for", tabledefns[i].name)) {
		allocate_table(i, FALSE);
		break;
	    }
	}
	if (!tabledefns[i].table)
	  run_error("No 'ai-base-worth-for' table allocated");
	for_all_unit_types(u) {
	    for_all_unit_types(u2) {
		(tmp_uu_array[u])[u2] = base_worth_for(u, u2);
		worthmax = max(worthmax, (tmp_uu_array[u])[u2]);
	    }
	}
	for_all_unit_types(u) {
	    for_all_unit_types(u2) 
	      set_base_worth_for(u, u2, 
				 normalize_on_pmscale((tmp_uu_array[u])[u2],
						      worthmax, 10000));
	}
    }
}

int
base_worth(int u)
{
    int worth = 0;
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    for_all_unit_types(u2) 
      worth += uu_ai_base_worth_for(u, u2);
    return max(0, worth);
}

void
set_base_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aibaseworth = n;
}

void
maybe_set_base_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_base_worth) == u_ai_base_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = base_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_base_worth(u, 
			 normalize_on_pmscale(tmp_u_array[u], 
					      worthmax, 10000));
    }
}

/* Construction Questions */

int
can_create_in(Unit *actor, Unit *creator, int u3, Unit *transport)
{
    int rslt = A_ANY_OK;
    int u2 = NONUTYPE;
    int x = -1, y = -1;
    Side *side = NULL;
    UnitView *uvtransport = NULL, *uvubertransport = NULL;

    assert_error(in_play(transport), 
"AI Create Check: Attempted to access an out-of-play transport");
    x = transport->x;  y = transport->y;
    if (!valid(rslt = can_create_common(actor, creator, u3, x, y)))
	return rslt;
    u2 = creator->type;
    side = actor->side;
    uvtransport = find_unit_view(side->see_all ? NULL : side, transport);
    // If we cannot see the transport for some reason,
    //	then we cannot create in it.
    if (!uvtransport)
	return A_ANY_CANNOT_DO;
    // Handle the special case of merging.
    // TODO: Make this a separate action?
    if (u_advanced(transport->type)
	&& creator->transport && (creator->transport == transport)
	&& (transport->type == u3))
	return A_ANY_OK;
    // Check permissions up the transport chain.
    if (uvtransport->transport) {
	for (uvubertransport = uvtransport->transport;
	     uvubertransport->transport;
	     uvubertransport = uvubertransport->transport) {
	    if (!trusted_side(side, side_n(uvubertransport->siden)))
		return A_ANY_CANNOT_DO;
	}
    }
    // Check against transport's capacity.
    if (!valid(rslt = can_be_in(u3, side, uvtransport)))
	return rslt;
    return A_ANY_OK;
}

int
can_create_at(Unit *actor, Unit *creator, int u3, int x, int y)
{
    static int *p_without;

    int rslt = A_ANY_OK;
    int u2 = NONUTYPE;
    Side *side = NULL;
    int tv = UNSEEN;
    int t = NONTTYPE;

    if (!valid(rslt = can_create_common(actor, creator, u3, x, y)))
	return rslt;
    u2 = creator->type;
    side = actor->side;
    // Check if cell is seen.
    //	Do not attempt to create on an unseen cell, even if legal.
    tv = terrain_view(side, x, y);
    if (UNSEEN == tv)
	return A_ANY_CANNOT_DO;
    t = vterrain(tv);
    // Can new utype survive on cell?
    if (!valid(rslt = can_survive_on_known(u3, side, x, y))) {
	if (!((x == creator->x) && (y == creator->y)
	      && (rslt == A_MOVE_DEST_FULL)))
	    return rslt;
    }
    // Initialize the without-utypes array, if necessary.
    if (!p_without)
	p_without = (int *)xmalloc(numutypes * sizeof(int));
    // Clean out the without-utypes array, and creator utype to it.
    memset(p_without, 0, numutypes * sizeof(int));
    p_without[u2] = 1;
    // If creator is in same cell as future creation, 
    //	then we have other options.
    // Option 1: Creator is allowed to enter creation.
    // Option 2: Creator dies upon creation.
    if (!valid(rslt = can_survive_on_known(u3, side, x, y, p_without))) 
	return rslt;
    return A_ANY_OK;
}

/* Tooling Point Estimates */

int
tp_per_turn_est(Unit *unit, int u2)
{
    int tp = 0, toolups = INT_MAX, copert = 0;
    int u = NONUTYPE;
    int m = NONMTYPE;
    int mlimits = FALSE;
    Side *side = NULL;

    assert_warning_return(
	in_play(unit), "AI: Attempted to access an out-of-play unit", 0);
    assert_error(is_unit_type(u2), "AI: Encountered invalid unit type");
    u = unit->type;
    side = unit->side;
    // Quick test to see if we can even toolup for the given utype.
    if (0 >= uu_tp_per_toolup(u, u2))
	return 0;
    // Look at material limits on tooling up.
    for_all_material_types(m) {
	// Consumption per build.
	copert = um_consumption_per_tp(u2, m) * uu_tp_per_toolup(u, u2);
	copert += um_consumption_per_tooledup(u2, m);
	copert += um_consumption_per_toolup(u, m);
	// If this material is not consumed, then skip it.
	if (!copert)
	    continue;
	mlimits = TRUE;
	// If constructor takes from treasury, 
	//  then estimate treasury limit on tooling up,
	//  assuming this is the only constructor using the treasury for 
	//  the material in question.
	if (could_take_from_treasury(u, side, m))
	    toolups = min(toolups, side->treasury[m] / copert);
	// Else, the builder has only its own stock to use.
	// We assume that the builder always has about the amount of 
	//  supply that it currently has on hand.
	else
	    toolups = min(toolups, unit->supply[m] / copert);
    } // for all mtypes
    // If not ACP-indep and if build action actually costs ACP, 
    //	then ACP may affect number of builds.
    if (!u_acp_independent(u) && (0 < uu_acp_to_toolup(u, u2)))
	toolups = 
	    min(toolups, 
		(acp_per_turn_max_wo_effects(u) / uu_acp_to_toolup(u, u2)));
    // Estimate TP per turn.
    tp = toolups * uu_tp_per_toolup(u, u2);
    return tp;
}

/* Construction Point Estimates */

int
cp_per_turn_est(Unit *unit, int u2)
{
    int cp = 0, builds = INT_MAX, coperb = 0;
    int u = NONUTYPE;
    int m = NONMTYPE;
    int mlimits = FALSE;
    Side *side = NULL;

    assert_warning_return(
	in_play(unit), "AI: Attempted to access an out-of-play unit", 0);
    assert_error(is_unit_type(u2), "AI: Encountered invalid unit type");
    u = unit->type;
    side = unit->side;
    // Quick test to see if we can even build on the given utype.
    if (0 >= uu_cp_per_build(u, u2))
	return 0;
    // Look at material limits on building.
    for_all_material_types(m) {
	// Consumption per build.
	coperb = um_consumption_per_cp(u2, m) * uu_cp_per_build(u, u2);
	coperb += um_consumption_per_built(u2, m);
	coperb += um_consumption_per_build(u, m);
	// If this material is not consumed, then skip it.
	if (!coperb)
	    continue;
	mlimits = TRUE;
	// If builder takes from treasury, 
	//  then estimate treasury limit on building, 
	//  assuming this is the only builder using the treasury for 
	//  the material in question.
	if (could_take_from_treasury(u, side, m))
	    builds = min(builds, side->treasury[m] / coperb);
	// Else, the builder has only its own stock to use.
	// We assume that the builder always has about the amount of 
	//  supply that it currently has on hand.
	else
	    builds = min(builds, unit->supply[m] / coperb);
    } // for all mtypes
    // If not ACP-indep and if build action actually costs ACP, 
    //	then ACP may affect number of builds.
    if (!u_acp_independent(u) && (0 < uu_acp_to_build(u, u2)))
	builds = 
	    min(builds, 
		(acp_per_turn_max_wo_effects(u) / uu_acp_to_build(u, u2)));
    // Estimate CP per turn.
    cp = builds * uu_cp_per_build(u, u2);
    return cp;
}

int
cp_gained_per_turn_est(Unit *unit, Side *side)
{
    Unit *unit2 = NULL;
    Task *tasks2 = NULL;
    int id = -1;
    int u = NONUTYPE;
    int crate = 0;

    assert_error(in_play(unit), 
		 "AI: Attempted to manipulate an out-of-play unit");
    // Useful info.
    id = unit->id;
    u = unit->type;
    if (!side)
	side = unit->side;
    // No construction if the specified side is an enemy.
    // Technically, we should not even be able to iterate through the units.
    if (enemy_side(side, unit->side))
	return 0;
    // If this unit can build itself, then count that contribution.
    if (unit->cp > u_cp_to_self_build(u))
	crate += u_cp_per_self_build(u);
    // Find all current builders and tally their contributions.
    for_all_side_units(side, unit2) {
	if (unit == unit2)
	    continue;
	if (!is_active(unit2))
	    continue;
	tasks2 = (unit2->plan ? unit2->plan->tasks : NULL);
	if (!tasks2)
	    continue;
	if ((TASK_BUILD == tasks2->type)
	    && ((id == unit2->creation_id) || (id == tasks2->args[1])))
	    crate += cp_per_turn_est(unit2, u);
    }
    return crate;
}

/* Hitpoint Estimates */

int
hp_per_turn_est(Unit *unit, int u2)
{
    int hp = 0, repairs = INT_MAX, coperr = 0;
    int u = NONUTYPE;
    int m = NONMTYPE;
    int mlimits = FALSE;
    Side *side = NULL;

    assert_warning_return(
	in_play(unit), "AI: Attempted to access an out-of-play unit", 0);
    assert_error(is_unit_type(u2), "AI: Encountered invalid unit type");
    u = unit->type;
    side = unit->side;
    // Could we even repair the given utype?
    if (!could_repair(u, u2) && !could_auto_repair(u, u2))
	return 0;
    // Look at material limits on repair.
    for_all_material_types(m) {
	// Consumption per repair.
	coperr = um_consumption_per_repaired(u2, m);
	coperr += um_consumption_per_repair(u, m);
	// If this material is not consumed, then skip it.
	if (!coperr)
	    continue;
	mlimits = TRUE;
	// If repairer takes from treasury, 
	//  then estimate treasury limit on repairing, 
	//  assuming this is the only repairer using the treasury for 
	//  the material in question.
	if (could_take_from_treasury(u, side, m))
	    repairs = min(repairs, side->treasury[m] / coperr);
	// Else, the repairer has only its own stock to use.
	// We assume that the repairer always has about the amount of 
	//  supply that it currently has on hand.
	else
	    repairs = min(repairs, unit->supply[m] / coperr);
    } // for all mtypes
    // If not ACP-indep, explicit repair allowed, and action costs ACP,
    //	then ACP may affect number of repairs.
    if (!u_acp_independent(u) && could_repair(u, u2) 
	&& (0 < uu_acp_to_repair(u, u2)))
	repairs = 
	    min(repairs, 
		(acp_per_turn_max_wo_effects(u) / uu_acp_to_repair(u, u2)));
    // Estimate HP per turn.
    hp = repairs * (uu_hp_per_repair(u, u2) / 100);
    if (0 != (uu_hp_per_repair(u, u2) % 100))
	hp += ((repairs * (uu_hp_per_repair(u, u2) % 100)) / 100);
    return hp;
}

int
hp_gained_per_turn_est(Unit *unit, Side *side)
{
    Unit *unit2 = NULL;
    Task *tasks2 = NULL;
    int id = -1;
    int u = NONUTYPE, u2 = NONUTYPE;
    int rrate = 0, dist = 0;

    assert_error(in_play(unit), 
		 "AI: Attempted to manipulate an out-of-play unit");
    // Useful info.
    id = unit->id;
    u = unit->type;
    if (!side)
	side = unit->side;
    // No repair if the specified side is an enemy.
    // Technically, we should not even be able to iterate through the units.
    if (enemy_side(side, unit->side))
	return 0;
    // If this unit can repair itself, then count that contribution.
    if ((0 < u_hp_recovery(u)) && (unit->hp >= u_hp_to_recover(u))) {
	rrate += (u_hp_recovery(u) / 100);
	if (49 < (u_hp_recovery(u) % 100))
	    ++rrate;
    }
    // Iterate through all side units and sum their contributions.
    for_all_side_units(side, unit2) {
	if (!is_active(unit2))
	    continue;
	u2 = unit2->type;
	tasks2 = (unit2->plan ? unit2->plan->tasks : NULL);
	if (!tasks2)
	    continue;
	// Consider repairers dedicated to repairee.
	if ((TASK_REPAIR == tasks2->type) && (id == tasks2->args[0])) {
	    rrate += hp_per_turn_est(unit2, u);
	    continue;
	}
	// Consider auto-repairers in range of repairee.
	dist = distance(unit2->x, unit2->y, unit->x, unit->y);
	if ((dist <= uu_auto_repair_range(u2, u)) 
	    && (0 < uu_auto_repair(u2, u)))
	    rrate += uu_auto_repair(u2, u);
    }
    return rrate;
}

/* Combat Rate Estimates */

int
firings_per_turn_est(int u, int u2)
{
    int hits = INT_MAX;
    int acpcost = -1;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    if (!could_fire_at(u, u2))
	return 0;
    acpcost = (u_acp_independent(u) ? 0 : u_acp_to_fire(u));
    if (0 < acpcost) 
	hits = min(hits, acp_per_turn_max_wo_effects(u) / acpcost);
    // TODO: Handle materials restrictions.
    return hits;
}

int
attacks_per_turn_est(int u, int u2)
{
    int hits = INT_MAX;
    int acpcost = -1;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    if (!could_attack(u, u2))
	return 0;
    acpcost = (u_acp_independent(u) ? 0 : uu_acp_to_attack(u, u2));
    if (0 < acpcost) 
	hits = min(hits, acp_per_turn_max_wo_effects(u) / acpcost);
    // TODO: Handle materials restrictions.
    return hits;
}

int
detonations_per_turn_est(int u, int u2)
{
    int hits = INT_MAX;
    int acpcost = -1;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    if (!could_damage_by_detonation(u, u2))
	return 0;
    acpcost = (u_acp_independent(u) ? 0 : u_acp_to_detonate(u));
    if (0 < acpcost) 
	hits = min(hits, acp_per_turn_max_wo_effects(u) / acpcost);
    // TODO: Handle materials restrictions.
    return hits;
}

int
capture_attempts_per_turn_est(int u, int u2, Side *side2)
{
    int attempts = INT_MAX;
    int acpcost = -1;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid captor unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error(side2,
		 "AI Combat Assessor: Attempted to access a NULL side");
    if (!could_capture_by_capture(u, u2, side2))
	return 0;
    acpcost = (u_acp_independent(u) ? 0 : uu_acp_to_capture(u, u2));
    if (0 < acpcost) 
	attempts = min(attempts, acp_per_turn_max_wo_effects(u) / acpcost);
    // TODO: Handle materials restrictions.
    return attempts;
}

/* Hit Estimates */

int
firings_mean_to_destroy(int u, int u2, int hp2, int dm, int hcm)
{
    int dmgmean = 0, hitchance = 0, hitsmean = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error((0 < hp2),
		 "AI Combat Assessor: Cannot calculate with HP <= 0");
    if (!could_fire_at(u, u2))
	return INT_MAX;
    if (!could_destroy_by_fire(u, u2))
	return INT_MAX;
    dmgmean = (fire_damage_mean(u, u2) * dm) / 100;
    if (0 >= dmgmean)
	return INT_MAX;
    hitchance = min(100, (fire_hit_chance(u, u2) * hcm) / 100);
    dmgmean = (dmgmean * 100) / hitchance;
    hitsmean = hp2 / dmgmean;
    if (hp2 % dmgmean)
	++hitsmean;
    return hitsmean;
}

int
firings_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about a NULL defender");
    u2 = uview->type;
    // TODO: Calculate ablation and vulnerability.
    return firings_mean_to_destroy(u, u2, u_hp(u2));
}

int
firings_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    // TODO: Calculate ablation and vulnerability.
    return firings_mean_to_destroy(u2, u, unit->hp);
}

int
fire_turns_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int hm = INT_MAX, hpt = 0, tm = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    if (!could_fire_at(u, u2))
	return INT_MAX;
    if (!could_destroy_by_fire(u, u2))
	return INT_MAX;
    hm = firings_mean_to_destroy(u, uview);
    if (INT_MAX == hm)
	return INT_MAX;
    hpt = firings_per_turn_est(u, u2);
    if (0 >= hpt)
	return INT_MAX;
    tm = hm / hpt;
    if (hm % hpt)
	++tm;
    return tm;
}

int
fire_turns_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int hm2 = INT_MAX, hpt2 = 0, tm2 = INT_MAX;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    if (!could_fire_at(u2, u))
	return INT_MAX;
    if (!could_destroy_by_fire(u2, u))
	return INT_MAX;
    hm2 = firings_mean_to_destroy(u2, unit);
    if (INT_MAX == hm2)
	return INT_MAX;
    hpt2 = firings_per_turn_est(u2, u);
    if (0 >= hpt2)
	return INT_MAX;
    tm2 = hm2 / hpt2;
    if (hm2 % hpt2)
	++tm2;
    return tm2;
}

int
attacks_mean_to_destroy(int u, int u2, int hp2, int dm, int hcm)
{
    int admgmean = 0, ahitchance = 0, ddmgmean = 0;
    int dmgmean = 0, hitsmean = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error((0 < hp2),
		 "AI Combat Assessor: Cannot calculate with HP <= 0");
    if (!could_attack(u, u2))
	return INT_MAX;
    if (!could_destroy_by_attacks(u, u2))
	return INT_MAX;
    admgmean = (attack_damage_mean(u, u2) * dm) / 100;
    ahitchance = min(100, (uu_hit(u, u2) * hcm) / 100);
    admgmean = (admgmean * 100) / ahitchance;
    if (u_detonate_with_attack(u) && could_damage_by_detonation(u, u2))
	ddmgmean = (detonate_damage_mean(u, u2) * dm) / 100;
    dmgmean = admgmean + ddmgmean;
    if (0 >= dmgmean)
	return INT_MAX;
    hitsmean = hp2 / dmgmean;
    if (hp2 % dmgmean)
	++hitsmean;
    return hitsmean;
}

int
attacks_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about a NULL defender");
    u2 = uview->type;
    // TODO: Calculate damage and hit-chance modifiers.
    return attacks_mean_to_destroy(u, u2, u_hp(u2));
}

int
attacks_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    // TODO: Calculate damage and hit-chance modifiers.
    return attacks_mean_to_destroy(u2, u, unit->hp);
}

int
attack_turns_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int hm = INT_MAX, hpt = 0, tm = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    if (!could_attack(u, u2))
	return INT_MAX;
    if (!could_destroy_by_attacks(u, u2))
	return INT_MAX;
    hm = attacks_mean_to_destroy(u, uview);
    if (INT_MAX == hm)
	return INT_MAX;
    hpt = attacks_per_turn_est(u, u2);
    if (0 >= hpt)
	return INT_MAX;
    tm = hm / hpt;
    if (hm % hpt)
	++tm;
    return tm;
}

int
attack_turns_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int hm2 = INT_MAX, hpt2 = 0, tm2 = INT_MAX;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    if (!could_attack(u2, u))
	return INT_MAX;
    if (!could_destroy_by_attacks(u2, u))
	return INT_MAX;
    hm2 = attacks_mean_to_destroy(u2, unit);
    if (INT_MAX == hm2)
	return INT_MAX;
    hpt2 = attacks_per_turn_est(u2, u);
    if (0 >= hpt2)
	return INT_MAX;
    tm2 = hm2 / hpt2;
    if (hm2 % hpt2)
	++tm2;
    return tm2;
}

int
detonations_mean_to_destroy(int u, int u2, int hp2, int dm)
{
    int dmgmean = 0, hitsmean = INT_MAX;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid detonator unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error((0 < hp2),
		 "AI Combat Assessor: Cannot calculate with HP <= 0");
    if (!could_damage_by_detonation(u, u2))
	return INT_MAX;
    if (!could_destroy_by_detonations(u, u2))
	return INT_MAX;
    dmgmean = (detonate_damage_mean(u, u2) * dm) / 100;
    if (0 >= dmgmean)
	return INT_MAX;
    hitsmean = hp2 / dmgmean;
    if (hp2 % dmgmean)
	++hitsmean;
    return hitsmean;
}

int
detonations_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about a NULL defender");
    u2 = uview->type;
    // TODO: Calculate damage modifier.
    return detonations_mean_to_destroy(u, u2, u_hp(u2));
}

int
detonations_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    // TODO: Calculate damage modifier.
    return detonations_mean_to_destroy(u2, u, unit->hp);
}

int
detonate_turns_mean_to_destroy(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int hm = INT_MAX, hpt = 0, tm = INT_MAX;
    int detsavail = 0;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    if (!could_damage_by_detonation(u, u2))
	return INT_MAX;
    if (!could_destroy_by_detonations(u, u2))
	return INT_MAX;
    hm = detonations_mean_to_destroy(u, uview);
    if (INT_MAX == hm)
	return INT_MAX;
    detsavail = n_detonations_available(u);
    if (detsavail < hm)
	return INT_MAX;
    hpt = detonations_per_turn_est(u, u2);
    if (0 >= hpt)
	return INT_MAX;
    tm = hm / hpt;
    if (hm % hpt)
	++tm;
    return tm;
}

int
detonate_turns_mean_to_destroy(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int hm2 = INT_MAX, hpt2 = 0, tm2 = INT_MAX;
    int detsavail2 = 0;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid detonator unit type");
    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    if (!could_damage_by_detonation(u2, u))
	return INT_MAX;
    if (!could_destroy_by_detonations(u2, u))
	return INT_MAX;
    hm2 = detonations_mean_to_destroy(u2, unit);
    if (INT_MAX == hm2)
	return INT_MAX;
    detsavail2 = n_detonations_available(u2);
    if (detsavail2 < hm2)
	return INT_MAX;
    hpt2 = detonations_per_turn_est(u2, u);
    if (0 >= hpt2)
	return INT_MAX;
    tm2 = hm2 / hpt2;
    if (hm2 % hpt2)
	++tm2;
    return tm2;
}

/* Capture Estimates */

int
firings_mean_to_capture(int u, int u2, Side *side2, int ccm)
{
    int attempts = INT_MAX, capchance = 0;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error(side2,
		 "AI Combat Assessor: Attempted to access a NULL side");
    if (!could_fire_at(u, u2))
	return INT_MAX;
    if (!could_capture_by_fire(u, u2, side2))
	return INT_MAX;
    capchance = (fire_hit_chance(u, u2) * capture_chance(u, u2, side2)) / 100;
    capchance = (capchance * ccm) / 100;
    attempts = 100 / capchance;
    if (100 % capchance)
	++attempts;
    return attempts;
}

int
firings_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    Side *side2 = NULL;
    int attempts = INT_MAX;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    side2 = side_n(uview->siden);
    // TODO: Calculate hit and capture chance modifiers.
    attempts = firings_mean_to_capture(u, u2, side2);
    if (attempts >= firings_mean_to_destroy(u, uview))
	return INT_MAX;
    return attempts;
}

int
firings_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    Side *side = NULL;
    int attempts = INT_MAX;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    side = unit->side;
    // TODO: Calculate hit and capture chance modifiers.
    attempts = firings_mean_to_capture(u2, u, side);
    if (attempts >= firings_mean_to_destroy(u2, unit))
	return INT_MAX;
    return attempts;
}

int
fire_turns_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side2 = NULL;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    if (!could_fire_at(u, u2))
	return INT_MAX;
    if (!could_capture_by_fire(u, u2, side2))
	return INT_MAX;
    cm = firings_mean_to_capture(u, uview);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = firings_per_turn_est(u, u2);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

int
fire_turns_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side = NULL;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid firer unit type");
    assert_error(in_play(unit),
"AI Combat Assessor: Attempted to access out-of-play defender");
    u = unit->type;
    if (!could_fire_at(u2, u))
	return INT_MAX;
    if (!could_capture_by_fire(u2, u, side))
	return INT_MAX;
    cm = firings_mean_to_capture(u2, unit);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = firings_per_turn_est(u2, u);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

int
attacks_mean_to_capture(int u, int u2, Side *side2, int ccm)
{
    int attempts = INT_MAX, capchance = 0;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error(side2,
		 "AI Combat Assessor: Attempted to access a NULL side");
    if (!could_attack(u, u2))
	return INT_MAX;
    if (!could_capture_by_attacks(u, u2, side2))
	return INT_MAX;
    capchance = (uu_hit(u, u2) * capture_chance(u, u2, side2)) / 100;
    capchance = (capchance * ccm) / 100;
    attempts = 100 / capchance;
    if (100 % capchance)
	++attempts;
    return attempts;
}

int
attacks_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    Side *side2 = NULL;
    int attempts = INT_MAX;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    side2 = side_n(uview->siden);
    // TODO: Calculate hit and capture chance modifiers.
    attempts = attacks_mean_to_capture(u, u2, side2);
    if (attempts >= attacks_mean_to_destroy(u, uview))
	return INT_MAX;
    return attempts;
}

int
attacks_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    Side *side = NULL;
    int attempts = INT_MAX;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    side = unit->side;
    // TODO: Calculate hit and capture chance modifiers.
    attempts = attacks_mean_to_capture(u2, u, side);
    if (attempts >= attacks_mean_to_destroy(u2, unit))
	return INT_MAX;
    return attempts;
}

int
attack_turns_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side2 = NULL;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    if (!could_attack(u, u2))
	return INT_MAX;
    if (!could_capture_by_attacks(u, u2, side2))
	return INT_MAX;
    cm = attacks_mean_to_capture(u, uview);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = attacks_per_turn_est(u, u2);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

int
attack_turns_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side = NULL;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(in_play(unit),
"AI Combat Assessor: Attempted to access out-of-play defender");
    u = unit->type;
    side = unit->side;
    if (!could_attack(u2, u))
	return INT_MAX;
    if (!could_capture_by_attacks(u2, u, side))
	return INT_MAX;
    cm = attacks_mean_to_capture(u2, unit);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = attacks_per_turn_est(u2, u);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

int
attempts_mean_to_capture(int u, int u2, Side *side2, int ccm)
{
    int attempts = INT_MAX, capchance = 0;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid defender unit type");
    assert_error(side2,
		 "AI Combat Assessor: Attempted to access a NULL side");
    if (!could_capture(u, u2, side2))
	return INT_MAX;
    if (!could_capture_by_capture(u, u2, side2))
	return INT_MAX;
    capchance = (capture_chance(u, u2, side2) * ccm) / 100;
    attempts = 100 / capchance;
    if (100 % capchance)
	++attempts;
    return attempts;
}

int
attempts_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    Side *side2 = NULL;

    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    side2 = side_n(uview->siden);
    // TODO: Calculate hit and capture chance modifiers.
    return attempts_mean_to_capture(u, u2, side2);
}

int
attempts_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    Side *side = NULL;
    int attempts = INT_MAX;

    assert_error(in_play(unit),
		 "AI Combat Assessor: Thinking about an out-of-play defender");
    u = unit->type;
    side = unit->side;
    // TODO: Calculate hit and capture chance modifiers.
    attempts = attempts_mean_to_capture(u2, u, side);
    return attempts;
}

int
capture_turns_mean_to_capture(int u, UnitView *uview)
{
    int u2 = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side2 = NULL;

    assert_error(is_unit_type(u),
		 "AI Combat Assessor: Encountered invalid captor unit type");
    assert_error(uview,
		 "AI Combat Assessor: Thinking about an inactive defender");
    u2 = uview->type;
    side2 = side_n(uview->siden);
    if (!could_capture(u, u2, side2))
	return INT_MAX;
    if (!could_capture_by_capture(u, u2, side2))
	return INT_MAX;
    cm = attempts_mean_to_capture(u, uview);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = capture_attempts_per_turn_est(u, u2, side2);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

int
capture_turns_mean_to_capture(int u2, Unit *unit)
{
    int u = NONUTYPE;
    int cm = INT_MAX, cpt = 0, tm = INT_MAX;
    Side *side = NULL;

    assert_error(is_unit_type(u2),
		 "AI Combat Assessor: Encountered invalid captor unit type");
    assert_error(in_play(unit),
"AI Combat Assessor: Attempted to access out-of-play defender");
    u = unit->type;
    side = unit->side;
    if (!could_capture(u2, u, side))
	return INT_MAX;
    if (!could_capture_by_capture(u2, u, side))
	return INT_MAX;
    cm = attempts_mean_to_capture(u2, unit);
    if (INT_MAX == cm)
	return INT_MAX;
    cpt = capture_attempts_per_turn_est(u2, u, side);
    if (0 >= cpt)
	return INT_MAX;
    tm = cm / cpt;
    if (cm % cpt)
	++tm;
    return tm;
}

/* Worths as an explorer. */

int
explorer_worth(int u)
{
    int worth = 0;

    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    worth = (u_ai_seer_worth(u) * u_ai_mover_worth(u)) / 10000;
    return max(0, worth);
}

void
set_explorer_worth(int u, int n)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(between(PROPLO, n, PROPHI),
	"Attempted to set an uprop out-of-bounds");
    utypes[u].aiexplorerworth = n;
}

//! Set basic worths as explorers, if necessary.

void
maybe_set_explorer_worths(void)
{
    int u = NONUTYPE;
    int worthmax = 0;
    int mustcalc = FALSE;

    for_all_unit_types(u) {
	if (uprop_i_default(u_ai_explorer_worth) == u_ai_explorer_worth(u)) {
	    mustcalc = TRUE;
	    break;
	}
    }
    if (mustcalc) {
	for_all_unit_types(u) { 
	    tmp_u_array[u] = explorer_worth(u);
	    worthmax = max(worthmax, tmp_u_array[u]);
	}
	for_all_unit_types(u)
	  set_explorer_worth(u, 
			     normalize_on_pmscale(tmp_u_array[u], 
						  worthmax, 10000));
    }
}

/* Total Worths */

int
total_worth(int u)
{
    int worth = 0;

    assert_error(is_unit_type(u), "AI: Encountered an invalid unit type");
    worth += u_offensive_worth(u);
    worth += u_siege_worth(u);
    worth += u_defensive_worth(u);
    worth += u_ai_explorer_worth(u);
    worth += u_ai_prod_enhancer_worth(u);
    worth += u_ai_base_worth(u);
    worth += u_colonizer_worth(u);
    worth += u_colonization_support_worth(u);
    worth += u_base_construction_worth(u);
    worth += u_exploration_support_worth(u);
    worth += u_offensive_support_worth(u);
    worth += u_defensive_support_worth(u);
    return worth;
}

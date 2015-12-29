/* The combat-related actions of Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

using namespace Xconq;

static int one_attack(Unit *atker, Unit *defender);
static void fire_on_unit(Unit *atker, Unit *other);
static void attack_unit(Unit *atker, Unit *other);
static void maybe_hit_unit(Unit *atker, Unit *other, int fire, int fallsoff);
static void hit_unit(Unit *unit, int hit, Unit *atker);
static void model_1_attack(Unit *unit, int x, int y);
static int real_attack_value(Unit *unit);
static int real_defense_value(Unit *unit);

static void reckon_damage(int fire);
static void reckon_damage_here(int x, int y);
static void report_damage(Unit *unit, Unit *atker, Unit *mainunit, int fire);
static int will_report_damage(Unit *unit);
static void rescue_one_occupant(Unit *occ);
static int retreat_unit(Unit *unit, Unit *atker);
static int retreat_in_dir(Unit *unit, int dir);
static void attempt_to_capture_unit(Unit *atker, Unit *other);
static void capture_unit_2(Unit *unit, Unit *pris, Side *prevside);
static void capture_occupant(Unit *unit, Unit *pris, Unit *occ, Side *newside);
static void detonate_on_cell(int x, int y);
static void hit_unit_with_detonation(Unit *unit, int hit, Unit *atker);
static int found_blocking_elevation(int u, int ux, int uy, int uz, int u2, int u2x, int u2y, int u2z);
static Unit *mobile_enemy_threat(Unit *unit, int range);
static int enough_ammo_to_attack_unit(Unit *unit, Unit *other);
static int enough_ammo_to_fire_at_unit(Unit *unit, Unit *other);
static int enough_ammo_to_fire_one_round(Unit *unit);

static void damage_terrain(int u, int x, int y);
static int damaged_terrain_type(int t, int u);

/* We can't declare all the action functions as static because some of them
   are in other files, but don't let them be visible to all files. */

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,prepfn,netprepfn,DOFN,checkfn,ARGDECL,doc)  \
  extern int DOFN ARGDECL;

#include "action.def"

/* Destruction stuff. */

typedef enum {
    DESTRUCT_ORDINARY = 0, 
    DESTRUCT_VANISH
} DestructionResult;

static DestructionResult determine_destruction_result(Unit *unit);

/* Detonation stuff. */

int max_t_detonate_effect_range = -1;

int max_detonate_on_approach_range = -1;

/* Remember what the main units involved are, so display is handled
   relative to them and not to any occupants. */

static Unit *amain, *omain;

int numsoundplays;

/* Useful, little functions. */

//! Chance that u will hit u2 by firing.

int
fire_hit_chance(int u, int u2)
{
    return (uu_fire_hit(u, u2) != -1) ? uu_fire_hit(u, u2) : uu_hit(u, u2);
}

//! Damage that u does to u2 by firing.

int
fire_damage(int u, int u2)
{
    return (uu_fire_damage(u, u2) != -1) ? uu_fire_damage(u, u2)
                                         : uu_damage(u, u2);
}

//! Chance for u to capture independent u2.

int
indep_capture_chance(int u, int u2)
{
    return (uu_indep_capture(u, u2) != -1) ? uu_indep_capture(u, u2)
                                           : uu_capture(u, u2);
}

//! Chance for u to capture u2 on a given side?

int
capture_chance(int u, int u2, Side *side2)
{
    if (side2 != indepside)
      return uu_capture(u, u2);
    return (indep_capture_chance(u, u2));
}

/* Unit/utype status/analysis functions. */

/*! \todo Remove ACP restrictions. */

int
could_fire_at(int u, int u2)
{
    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    assert_error(is_unit_type(u2), "Expected an unit type but did not get one");
    if (!u_acp_independent(u) && !u_acp_to_fire(u))
      return FALSE;
    return ((g_combat_model() == 0) && (0 < fire_hit_chance(u, u2)));
}

int
could_fire_at_any(int u)
{
    static bool *p_could;

    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_unit_type(u), 
		"Combat Assessor: Encountered invalid firer unit type");
    if (!p_could) {
	p_could = (bool *)xmalloc(numutypes * sizeof(bool));
	for_all_unit_types(u2) {
	    for_all_unit_types(u3)
		p_could[u2] = could_fire_at(u2, u3);
	}
    }
    return p_could[u];
}

int
fire_range(int u, int u2)
{
    assert_error(is_unit_type(u),
"Fire Range Calculator: Encountered invalid firer unit type");
    assert_error(is_unit_type(u2),
"Fire Range Calculator: Encountered invalid target unit type");
    if (!could_fire_at(u, u2))
        return -1;
    switch (g_combat_model()) {
      case 0:
        if (u_range(u) < u_range_min(u))
            return -1;
        else
            return u_range(u);
      case 1:
        return -1;
      default:
        return -1;
    }
    return -1;
}

int
fire_range_best(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid firer unit type");
    assert_error(is_unit_type(u2),
"Combat Range Calculator: Encountered invalid target unit type");
    return min(u_hit_falloff_range(u), fire_range(u, u2));
}

int
fire_range_max(int u)
{
    static int *p_ranges;

    int u2 = NONUTYPE, u3 = NONUTYPE;
    int range = -1;

    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid firer unit type");
    if (!p_ranges) {
        p_ranges = (int *)xmalloc(numutypes * sizeof(int));
        for_all_unit_types(u2) {
            range = -1;
            for_all_unit_types(u3)
                range = max(range, fire_range(u2, u3));
            p_ranges[u2] = range;
        }
    }
    return p_ranges[u];
}

/*! 
    \todo Get rid of combat models. 
    \todo Get rid of ACP restrictions.
    \todo Consider 'consumption-per-fire' and 'hit-by'.
*/

int
can_fire(Unit *actor, Unit *firer)
{
    int rslt = A_ANY_OK;
    int acp = 0;
    int u2 = NONUTYPE;
    int m2 = NONMTYPE;

    /* In combat model 1, we can't attack units by firing. */
    if (g_combat_model() == 1)
      return A_ANY_ERROR;
    /* Can the ACP source and firer act? */
    if (!valid(rslt = can_act(actor, firer)))
      return rslt;
    u2 = firer->type;
    /* Is there enough ACP to fire? */
    acp = u_acp_to_fire(u2);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (!has_enough_acp(actor, acp))
      return A_ANY_NO_ACP;
    /* Check whether we can attack from inside a transport. */
    if (firer->transport 
	&& uu_occ_combat(u2, firer->transport->type) == 0)
      return A_ANY_OCC_CANNOT_DO;
    /* We have to have a minimum level of supply to be able to attack. */
    for_all_material_types(m2) {
	if (firer->supply[m2] < um_to_fire(u2, m2))
	  return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

int
can_fire_at(Unit *actor, Unit *firer, int u)
{
    int rslt = A_ANY_OK;

    assert_error(is_unit_type(u), "Expected an unit type but did not get one");
    if (!could_fire_at(firer->type, u))
      return A_ANY_CANNOT_DO;
    if (!valid(rslt = can_fire(actor, firer)))
      return rslt;
    return rslt;
}

int
can_fire_at_any(Unit *actor, Unit *firer)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE;

    for_all_unit_types(u) {
	if (valid(rslt = can_fire_at(actor, firer, u)))
	  return A_ANY_OK;
    }
    return A_ANY_CANNOT_DO;
}

/*! \todo Remove ACP restrictions.  */

int
could_attack(int u, int u2)
{
    if (!u_acp_independent(u) && !uu_acp_to_attack(u, u2))
      return FALSE;
    return (((g_combat_model() == 0) && uu_hit(u, u2))
            || ((g_combat_model() == 1) && u_attack(u)));
}

int
could_attack_any(int u)
{
    static bool *p_could;

    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_unit_type(u), 
		"Combat Assessor: Encountered invalid attacker unit type");
    if (!p_could) {
	p_could = (bool *)xmalloc(numutypes * sizeof(bool));
	for_all_unit_types(u2) {
	    for_all_unit_types(u3)
		p_could[u2] = could_attack(u, u2);
	}
    }
    return p_could[u];
}

int
attack_range(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid attacker unit type");
    assert_error(is_unit_type(u2),
"Combat Range Calculator: Encountered invalid defender unit type");
    if (!could_attack(u, u2))
        return -1;
    switch (g_combat_model()) {
      case 0:
        if (uu_attack_range(u, u2) < uu_attack_range_min(u, u2))
            return -1;
        else
            return uu_attack_range(u, u2);
      case 1:
        return 1;
      default:
        return -1;
    }
    return -1;
}

int
attack_range_max(int u)
{
    static int *p_ranges;

    int u2 = NONUTYPE, u3 = NONUTYPE;
    int range = -1;

    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid attacker unit type");
    if (!p_ranges) {
        p_ranges = (int *)xmalloc(numutypes * sizeof(int));
        for_all_unit_types(u2) {
            range = -1;
            for_all_unit_types(u3)
                range = max(range, attack_range(u2, u3));
            p_ranges[u2] = range;
        }
    }
    return p_ranges[u];
}

/*! 
    \todo Get rid of ACP restrictions.
    \todo Consider 'consumption-per-attack' and 'hit-by'.
*/

int
can_attack(Unit *actor, Unit *attacker, int u3)
{
    int rslt = A_ANY_OK;
    int acp = 0;
    int u2 = NONUTYPE;
    int m = NONMTYPE;

    if (!valid(rslt = can_act(actor, attacker)))
      return rslt;
    u2 = attacker->type;
    /* Is there enough ACP to attack? */
    acp = uu_acp_to_attack(u2, u3);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (!has_enough_acp(actor, acp))
      return A_ANY_NO_ACP;
    /* Check whether we can attack from inside a transport. */
    if (attacker->transport 
	&& uu_occ_combat(u2, attacker->transport->type) == 0)
      return A_ANY_OCC_CANNOT_DO;
    /* We have to have a minimum level of supply to be able to attack. */
    for_all_material_types(m) {
	if (attacker->supply[m] < um_to_attack(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

int
can_attack_any(Unit *actor, Unit *attacker)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE;

    for_all_unit_types(u) {
	if (valid(rslt = can_attack(actor, attacker, u)))
	  return A_ANY_OK;
    }
    return A_ANY_CANNOT_DO;
}

int
could_detonate(int u)
{
    // TODO: Remove ACP requirement for this action.
    // TODO: Think about detonation on hit, capture, etc....
    if (!u_acp_independent(u) && !u_acp_to_detonate(u))
	return FALSE;
    return TRUE;
}

int
could_damage_by_0dist_detonation(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Calculator: Encountered invalid detonator unit type");
    assert_error(is_unit_type(u2),
"Combat Calculator: Encountered invalid detonation victim unit type");
    if (!could_detonate(u))
        return FALSE;
    if (0 >= dice1_roll_min(uu_detonation_damage_at(u, u2)))
        return FALSE;
    return TRUE;
}

int
could_damage_by_ranged_detonation(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Calculator: Encountered invalid detonator unit type");
    assert_error(is_unit_type(u2),
"Combat Calculator: Encountered invalid detonation victim unit type");
    if (!could_detonate(u))
        return FALSE;
    if (0 >= dice1_roll_min(uu_detonation_damage_adj(u, u2)))
        return FALSE;
    if (!uu_detonation_range(u, u2))
	return FALSE;
    return TRUE;
}

int
could_damage_by_detonation(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Calculator: Encountered invalid detonator unit type");
    assert_error(is_unit_type(u2),
"Combat Calculator: Encountered invalid detonation victim unit type");
    return (could_damage_by_0dist_detonation(u, u2)
	    || could_damage_by_ranged_detonation(u, u2));
}

int
detonate_urange(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid detonator unit type");
    assert_error(is_unit_type(u2),
"Combat Range Calculator: Encountered invalid detonation victim unit type");
    if (!could_damage_by_0dist_detonation(u, u2)
	&& !could_damage_by_ranged_detonation(u, u2))
	return -1;
    return uu_detonation_range(u, u2);
}

int
detonate_urange_max(int u)
{
    static int *p_ranges;

    int u2 = NONUTYPE, u3 = NONUTYPE;
    int range = -1;

    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid detonator unit type");
    if (!p_ranges) {
	p_ranges = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    range = -1;
	    for_all_unit_types(u3)
		range = max(range, detonate_urange(u2, u3));
	    p_ranges[u2] = range;
	}
    }
    return p_ranges[u];
}

int
detonate_urange_max(void)
{
    static int range = -2;

    int u = NONUTYPE;

    if (-2 == range) {
	for_all_unit_types(u)
	    range = max(range, detonate_urange_max(u));
    }
    return range;
}

int
can_detonate(Unit *actor, Unit *detonator)
{
    int rslt = A_ANY_OK;
    int acp = 0;
    int u2 = NONUTYPE;

    /* Can actor and detonator act? */
    if (!valid(rslt = can_act(actor, detonator)))
      return rslt;
    u2 = detonator->type;
    /* The unit must actually be able to detonate. */
    if (!could_detonate(u2))
	return A_ANY_CANNOT_DO;
    acp = u_acp_to_detonate(u2);
    if (!has_enough_acp(actor, acp))
      return A_ANY_NO_ACP;
    return rslt;
}

int
could_hit(int u, int u2)
{
    assert_error(is_unit_type(u),
"Combat Assessor: Encountered invalid offensive unit type");
    return (could_attack(u, u2) 
	    || could_fire_at(u, u2)
	    || could_damage_by_detonation(u, u2));
}

int
hit_range(int u, int u2)
{
    int range = -1;

    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid offensive unit type");
    assert_error(is_unit_type(u2),
"Combat Range Calculator: Encountered invalid victim unit type");
    range = max(fire_range(u, u2), attack_range(u, u2));
    range = max(range, detonate_urange(u, u2));
    return range;
}

int
hit_range_max(int u)
{
    static int *p_ranges;

    int u2 = NONUTYPE;

    assert_error(is_unit_type(u),
"Combat Range Calculator: Encountered invalid offensive unit type");
    if (!p_ranges) {
        p_ranges = (int *)xmalloc(numutypes * sizeof(int));
        for_all_unit_types(u2) {
            p_ranges[u2] = max(fire_range_max(u2), attack_range_max(u2));
            p_ranges[u2] = max(p_ranges[u2], detonate_urange_max(u2));
        }
    }
    return p_ranges[u];
}

int
hit_range_max(void)
{
    static int range = -2;

    int u = NONUTYPE;

    if (-2 == range) {
        for_all_unit_types(u)
            range = max(range, hit_range_max(u));
    }
    return range;
}

//! Could u capture u2 by fire?
/* 
    \todo Get rid of combat models.
    \todo Remove ACP restrictions.
*/

int
could_capture_by_fire(int u, int u2, Side *oside)
{
    if (!u_acp_independent(u) && !u_acp_to_fire(u))
      return FALSE;
    if (g_combat_model() == 1)
      return FALSE;
    if (fire_hit_chance(u, u2)) 
      return (0 < capture_chance(u, u2, oside));
    return FALSE;
}

//! Could u capture u2 by attacks?
/* 
    \todo Get rid of combat models.
    \todo Remove ACP restrictions.
*/

int
could_capture_by_attacks(int u, int u2, Side *oside)
{
    if (!u_acp_independent(u) && !uu_acp_to_attack(u, u2))
      return FALSE;
    /* Since capture only can happen through overrun actions in 
	combat model 1, and such actions are blocked if u_attack(u) is 
	zero (see model_1_attack) we return 0. Exception: if there is
	an encounter result specified, we return the capture chance
	(if non-zero) since the encounter code is executed before the
	check of u_attack(u). */
    if ((g_combat_model() == 1) && !u_attack(u))
      return FALSE;
    if (((g_combat_model() == 0) && uu_hit(u, u2)) || (g_combat_model() == 1)) 
      return (0 < capture_chance(u, u2, oside));
    return FALSE;
}

//! Could u capture u2 by direct capture?
/* 
    \todo Get rid of combat models.
    \todo Remove ACP restrictions.
*/

int
could_capture_by_capture(int u, int u2, Side *oside)
{
    if (!u_acp_independent(u) && !uu_acp_to_capture(u, u2))
      return FALSE;
    if (g_combat_model() == 1)
      return FALSE;
    return (0 < capture_chance(u, u2, oside));
}

//! Could u capture u2 by any means?

int
could_capture(int u, int u2, Side *oside)
{
    return (could_capture_by_attacks(u, u2, oside)
            || could_capture_by_fire(u, u2, oside)
            || could_capture_by_capture(u, u2, oside));
}

//! Could the given utype capture any utypes by any means?

int
could_capture_any(int u)
{
    int u2 = NONUTYPE;
	
    for_all_unit_types(u2) {
	/* Capture can happen by both direct action and as result of an
	   attack in combat model 0. */
	if (g_combat_model() == 0) {
		if ((uu_acp_to_attack(u, u2) > 0 
		    || uu_acp_to_capture(u, u2) > 0)
		    && (uu_capture(u, u2) > 0 || uu_indep_capture(u, u2) > 0))
		  return TRUE;
	/* Capture can only happen as result of an attack in combat model 1. */
	} else if (g_combat_model() == 1) {
		if (uu_acp_to_attack(u, u2) > 0
		    && (uu_capture(u, u2) > 0 || uu_indep_capture(u, u2) > 0))
		  return TRUE;
	}
    }
    return FALSE;
}

int
direct_capture_range(int u, int u2, Side *side2)
{
    return 1;
}

int
can_capture(Unit *actor, Unit *captor, int u3, Side *eside)
{
    int rslt = A_ANY_OK;
    int acp = 0;
    int u2 = NONUTYPE;
    int m = NONMTYPE;

    /* Can actor and captor act? */
    if (!valid(rslt = can_act(actor, captor)))
      return rslt;
    u2 = captor->type;
    /* We can't capture units on our side. */
    if (actor->side == eside)
      return A_ANY_ERROR;    
    /* Enough ACP to capture? */
    acp = uu_acp_to_capture(u2, u3);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (!has_enough_acp(actor, acp))
      return A_ANY_NO_ACP;
    /* Is there even a chance to capture the enemy utype? */
    if (capture_chance(u2, u3, eside) == 0)
      return A_ANY_CANNOT_DO;
    /* We have to have a minimum level of supply to be able to capture. */
    /* (attack material table not really appropriate) */
    for_all_material_types(m) {
	if (captor->supply[m] < um_to_attack(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

/* Attack action. */

/* This is an attack on a given unit at a given level of commitment. */

int
prep_attack_action(Unit *unit, Unit *unit2, Unit *defender, int n)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_ATTACK;
    unit->act->nextaction.args[0] = defender->id;
    unit->act->nextaction.args[1] = n;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_attack_action(Unit *unit, Unit *unit2, Unit *defender, int n)
{
    int u2 = unit2->type, u3 = defender->type;
    int withdrawchance, surrenderchance;

    action_point(unit2->side, defender->x, defender->y);
    action_point(defender->side, defender->x, defender->y);
    /* Defender might be a type that can sneak away to avoid attack. */
    withdrawchance = uu_withdraw_per_attack(u2, u3);
    if (withdrawchance > 0) {
	if (probability(withdrawchance)) {
	    if (retreat_unit(defender, unit2)) {
    		if (alive(unit))
    		  use_up_acp(unit, uu_acp_to_attack(u2, u3));
		return A_ANY_DONE;
	    }
	}
    }
    /* Defender might instead choose to surrender right off. */
    surrenderchance = uu_surrender_per_attack(u2, u3);
    if (surrenderchance > 0
	&& unit2->side
	&& unit2->side->tech[u3] >= u_tech_to_own(u3)) {
	if (probability(surrenderchance)) {
	    capture_unit(unit2, defender, H_UNIT_CAPTURED);
	    if (alive(unit))
	      use_up_acp(unit, uu_acp_to_attack(u2, u3));
	    return A_ANY_DONE;
	}
    }
    /* Carry out a normal attack. */
    one_attack(unit, defender);
    if (alive(unit))
      use_up_acp(unit, uu_acp_to_attack(u2, u3));
    /* The defender in an attack has to take time to defend itself. */
    if (alive(defender)) 
      use_up_acp(defender, uu_acp_to_defend(u2, u3));
    return A_ANY_DONE;
}

int
check_attack_action(Unit *unit, Unit *unit2, Unit *defender, int n)
{
    int u, u2, u3, u2x, u2y, dfx, dfy, dist;
    int rslt = A_ANY_OK;

    /* In combat model 1, we can't attack units directly. */
    if (g_combat_model() == 1)
      return A_ANY_ERROR;
    if (!in_play(defender))
      return A_ANY_ERROR;
    /* We can't attack ourselves. */
    if (unit2 == defender)
      return A_ANY_ERROR;
    if (!valid(rslt = can_attack(unit, unit2, defender->type)))
      return rslt;
    if (!indep(unit2) && unit2->side == defender->side)
      return A_ANY_ERROR;
    u = unit->type;
    u2 = unit2->type;
    u3 = defender->type;
    u2x = unit2->x;  u2y = unit2->y;
    dfx = defender->x;  dfy = defender->y;
    dist = distance(u2x, u2y, dfx, dfy);
    if (dist < uu_attack_range_min(u2, u3))
      return A_ANY_TOO_NEAR;
    if (dist > uu_attack_range(u2, u3))
      return A_ANY_TOO_FAR;
    if (uu_hit(u2, u3) <= 0)
      return A_ATTACK_CANNOT_HIT;
    /* (should prorate ammo needs by intensity of attack) */
    if (!enough_ammo_to_attack_unit(unit2, defender))
      return A_ANY_NO_AMMO;
    /* Allow attacks even if zero damage, this amounts to "harassment". */
    return A_ANY_OK;
}

/* Overrun action. */

/* Overrun is an attempt to occupy a given cell that may include attempts
   to attack and/or capture any units in the way. */

int
prep_overrun_action(Unit *unit, Unit *unit2, int x, int y, int z, int n)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_OVERRUN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.args[3] = n;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_overrun_action(Unit *unit, Unit *unit2, int x, int y, int z, int n)
{
    int u, u2, u3, acpused, mpcost, acpcost, speed, ox, oy, oz;
    int withdrawchance, surrenderchance, gotonext;
    Unit *defender;

    u = unit->type;  u2 = unit2->type;
    ox = unit2->x;  oy = unit2->y;  oz = unit2->z;
    action_point(unit2->side, x, y);
    acpused = 0;
    switch(g_combat_model()) {
      case 0:
	/* Attack every defender in turn. */
	for_all_stack(x, y, defender) {
	    u3 = defender->type;
	    /* Don't attack any of our buddies. */
	    if (unit_trusts_unit(unit2, defender))
	      continue;
	    action_point(defender->side, x, y);
	    gotonext = FALSE;
	    /* Defender might be a type that can sneak away to avoid
               attack. */
	    withdrawchance = uu_withdraw_per_attack(u2, u3);
	    if (withdrawchance > 0) {
		if (probability(withdrawchance)) {
		    if (retreat_unit(defender, unit2)) {
			gotonext = TRUE;
		    }
		}
	    }
	    /* Defender might instead choose to surrender right off. */
	    surrenderchance = uu_surrender_per_attack(u2, u3);
	    if (surrenderchance > 0
		&& unit2->side
		&& unit2->side->tech[u3] >= u_tech_to_own(u3)) {
		if (probability(surrenderchance)) {
		    capture_unit(unit2, defender, H_UNIT_CAPTURED);
		    gotonext = TRUE;
		}
	    }
            /* If any units were captured, the captor may have become 
               a garrison. If so, then the captor is considered dead. 
               Check for this before trying to fire/attack from (-1,-1)!
               (If the last unit in the stack is the one garrisoned, 
               then this is a kind of Pyrrhic victory. For the 
               time being, we will count this as a failure so that major 
               surgery does not have to be done on the overrun code.)
            */
            if (!alive(unit2))
              return A_OVERRUN_FAILED;
	    /* (should automatically prefer direct attack if better odds) */
	    if (valid(check_fire_at_action(unit, unit, defender, -1))) {
		do_fire_at_action(unit, unit, defender, -1); 
		acpused += u_acp_to_fire(u2);
		/* Unlike the case below, target acp consumption has
		   already been dealt with in fire_on_unit, which is
		   called by do_fire_at_action. */
	    }
	    else if (valid(check_attack_action(unit, unit, defender, 100))) {
		if (!gotonext)
		  one_attack(unit2, defender);
		if (alive(unit))
		  use_up_acp(unit, uu_acp_to_attack(u2, u3));
		acpused += uu_acp_to_attack(u2, u3);
		/* The target of an attack has to take time to defend
		   itself. */
		if (!gotonext && alive(defender)) {
		    use_up_acp(defender, uu_acp_to_defend(u, u3));
		}
	    }
	}
	if (!alive(unit2))
	  return A_OVERRUN_FAILED;
	if (in_blocking_zoc(unit2, x, y, z))
	  return A_OVERRUN_FAILED;
	/* Try to enter the cleared cell now - might still have
	   friendlies filling it up already, so check first. We don't
	   check for type_survives_in_cell, so suicide by deadly
	   terrain remains possible. */
	if (type_can_occupy_cell(unit2->type, x, y)) {
	    mpcost = move_unit(unit2, x, y);
	    /* Note that we'll say the action succeeded even if
	       the cell did not have enough room for us to actually
	       be in it, which is a little weird. */
	    /* Now add up any extra movement costs of entering the new cell. */
	    mpcost += zoc_move_cost(unit2, ox, oy, oz);
	    acpcost = 0;
	    speed = unit_speed(unit2, x, y);
	    if (speed > 0)
	      acpcost = (mpcost * 100) / speed;
	    acpcost -= acpused;
	    if (acpcost > 0) {
		/* Take the movement cost out of the moving unit if
                   possible. */
		use_up_acp(unit2, acpcost);
	    }
	    /* Count the unit as having actually moved. */
	    if (unit2->act)
	      ++(unit2->act->actualmoves);
	}
      break;
      case 1:
	model_1_attack(unit2, x, y);
	break;
      default:
	break;
    }
    return A_OVERRUN_SUCCEEDED;
}

int
check_overrun_action(Unit *unit, Unit *unit2, int x, int y, int z, int n)
{
    int u, u2, u2x, u2y, u2z, u3, totcost, speed, mpavail, m;
    Unit *defender;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!inside_area(x, y))
      return A_ANY_ERROR;
    if (n == 0)
      return A_ANY_ERROR;
    u = unit->type;
    u2 = unit2->type;
    if (!has_enough_acp(unit, 1))
      return A_ANY_NO_ACP;
    /* Check whether we can attack from inside a transport. */
    if (unit2->transport && uu_occ_combat(u2, unit2->transport->type) == 0)
      return A_ANY_OCC_CANNOT_DO;
    u2x = unit2->x;  u2y = unit2->y;  u2z = unit2->z;
    /* We have to be in the same cell or an adjacent one. */
    if (!between(0, distance(u2x, u2y, x, y), 1))
      return A_ANY_TOO_FAR;
    /* Now start looking at the move costs. */
    u3 = (unit2->transport ? unit2->transport->type : NONUTYPE);
    totcost = total_move_cost(u2, u3, u2x, u2y, u2z, x, y, u2z);
    speed = unit_speed(unit2, x, y);
    mpavail = (unit->act->acp * speed) / 100;
    /* Zero mp always disallows movement, unless intra-cell. */
    if (mpavail <= 0 && !(u2x == x && u2y == y /*&& u2z == u2z*/))
      return A_MOVE_NO_MP;
    /* The free mp might get us enough moves, so add it before comparing. */
    if (mpavail + u_free_mp(u2) < totcost)
      return A_MOVE_NO_MP;
    /* We have to have a minimum level of supply to be able to attack. */
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_attack(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    switch(g_combat_model()) {
      case 0:
	for_all_stack(x, y, defender) {
	    /* If we can't attack the unit, then we can't possibly overrun. */
	    if (uu_acp_to_attack(u2, defender->type) == 0)
	      return A_OVERRUN_CANNOT_HIT;
	    /* (should test if units here can be attacked en masse) */
	    /* (should prorate ammo needs by intensity of overrun) */
	    if (!enough_ammo_to_attack_unit(unit2, defender))
	      return A_ANY_NO_AMMO;
	}
	break;
      case 1:
	/* Assume we can always attempt to overrun */
	break;
      default:
	return A_ANY_ERROR;
    }
    return A_ANY_OK;
}

/* Return true if the attacker defeated the defender, and can therefore
   try to move into the defender's old position. */

static int
one_attack(Unit *atker, Unit *defender)
{
    int ua = atker->type, ud = defender->type;
    int ax = atker->x, ay = atker->y, ox = defender->x, oy = defender->y;
    int counter;
    Side *as = atker->side, *os = defender->side;
    SideMask observers;

    amain = atker;  omain = defender;
    attack_unit(atker, defender);
    /* Do a counterattack if appropriate.  We must be able to counterattack,
       and have enough acp to do so. */
    counter = uu_counterattack(ua, ud);
    if (counter > 0
	&& has_enough_acp(defender, uu_acp_to_defend(ua, ud))
	&& alive(atker)) {
	/* (should use value to set strength/commitment of counterattack) */
	attack_unit(defender, atker);
    }
    if (0 /* recording attacks */) {
	observers = add_side_to_set(atker->side, NOSIDES);
	observers = add_side_to_set(defender->side, observers);
	/* (should let other watching sides see event also) */
	record_event(H_UNIT_ASSAULTED, observers, atker->id, defender->id,
		     ox, oy);
    }
    reckon_damage(FALSE);
#if (0)
    see_exact(as, ax, ay);
    see_exact(as, ox, oy);
    see_exact(os, ax, ay);
    see_exact(os, ox, oy);
#else
    see_cell(as, ax, ay);
    see_cell(as, ox, oy);
    see_cell(os, ax, ay);
    see_cell(os, ox, oy);
#endif
    update_cell_display(as, ax, ay, UPDATE_ALWAYS);
    update_cell_display(as, ox, oy, UPDATE_ALWAYS);
    update_cell_display(os, ax, ay, UPDATE_ALWAYS);
    update_cell_display(os, ox, oy, UPDATE_ALWAYS);
    all_see_cell(ax, ay);
    all_see_cell(ox, oy);
    if (!alive(atker))
      return FALSE;
    attempt_to_capture_unit(atker, defender);
    /* If the defender was not captured, it might turn the tables! */
    /* (Note that we cannot cache the attacker's and defender's types,
       because the type might have changed due to damage, and we want
       to know if the new type might countercapture.) */
    if (alive(defender)
	&& alive(atker)
	&& defender->side == os
	&& uu_countercapture(atker->type, defender->type) > 0) {
	attempt_to_capture_unit(defender, atker);
    }
    return (alive(atker) && unit_at(ox, oy) == NULL);
}

/* Implement "combat model 1", which emulates Civ-type games.  Combat
   is multi-round, ending in the demise of one or the other combatant,
   plus anything stacked with it. */

static void
model_1_attack(Unit *unit, int x, int y)
{
    int u = unit->type, u2, u3, d, maxdef, def, att, winround, limit, dmg;
    Obj *choice, *chtype, *chutype, *chside;
    Unit *unit2, *defender, *occ, *defocc, *victim;
    char *hitmovietype;
    Side *side3;
    SideMask sidemask;    
    int weightstotal = 0;

    for_all_stack(x, y, unit2) {
	u2 = unit2->type;
	if (u_encounter_result(u2) != lispnil) {
	    /* The encounter result overrides any other outcome. */
	    choice = choose_from_weighted_list(u_encounter_result(u2),
					       &weightstotal, FALSE);
	    if (is_quoted_lisp(choice))
	      choice = eval(choice);
	    if (consp(choice))
	      chtype = car(choice);
	    else
	      chtype = choice;
	    if (match_keyword(chtype, K_UNIT)) {
		/* Dig out a unit type spec. */
		chutype = cadr(choice);
		if (consp(chutype)) {
		    chside = cadr(chutype);
		    chutype = car(chutype);
		} else {
		    chside = lispnil;
		}
		u3 = utype_from_name(c_string(chutype));
		/* Pick out a side if one was specified. */
		if (numberp(chside)) {
		    side3 = side_n(c_number(chside));
		} else {
		    side3 = unit->side;
		}
		/* Change the unit's type. */
		change_unit_type(unit2, u3, H_UNIT_TYPE_CHANGED, side3);
		/* Changing the type will kill the unit if there is no room
		for the new type. */
		if (alive(unit2)) {
			unit2->hp = unit2->hp2 = u_hp(u3);
			/* We still need to explicitly change the unit side, 
			   if the new type is allowed on the same side as 
			   the old type. The 'newside' argument to 
			   'change_unit_type' is only used when the new type 
			   cannot be on the same side as the old type. */
			change_unit_side(unit2, side3, -1, unit);
			/* The unit is changing side voluntarily, so set the
			   original side also. */
			set_unit_origside(unit2, unit2->side);
		}
	    } else if (match_keyword(chtype, K_VANISH)) {
		kill_unit(unit2, H_UNIT_VANISHED);
	    } else {
		run_warning("Unknown encounter type %s", c_string(chtype));
	    }
	    if (consp(choice) && stringp(caddr(choice))) {
		notify(unit->side, "%s", c_string(caddr(choice)));
	    }
	    return;
	}
    }
    /* Non-combat units can't do anything. Note: this means that unarmed
    units (settlers etc) cannot capture empty cities, since this happens by
    overrun action. Should perhaps allow to proceed if attacking an empty
    advanced type? However, in that case capture_chance must be changed,
    since it right now disallows capture by unarmed units. */
    if (u_attack(u) == 0)
      return;
    /* Choose the defender of a stack. */
    maxdef = -1;
    defender = NULL;
    for_all_stack(x, y, unit2) {
	/* Don't attack any of our buddies. */
	if (unit_trusts_unit(unit, unit2))
	  continue;
	/* Identify the best defender. */
	def = real_defense_value(unit2);
	if (def > maxdef) {
	    maxdef = def;
	    defender = unit2;
	}
    }
    /* If the location is empty for some reason, escape quietly. */
    if (defender == NULL)
      return;
    /* For cities, find a defender. */
    if (u_advanced(defender->type)) {
        /* We don't want defenseless occs (facilities) to count as defenders,
         * so initialize maxdefs to 0 instead of -1. */
	maxdef = 0;
	defocc = NULL;
	for_all_occupants(defender, occ) {
	    u2 = occ->type;
	    def = u_defend(u2);
	    if (def > maxdef) {
		maxdef = def;
		defocc = occ;
	    }
	}
	/* Cities with no defenders can be captured directly. */
	if (defocc == NULL) {
	    attempt_to_capture_unit(unit, defender);
	    return;
	}
	defender = defocc;
    }
    d = defender->type;
    limit = 100;
    while (alive(unit) && alive(defender) && limit-- > 0) {
	att = real_attack_value(unit);
	def = real_defense_value(defender);
	winround = probability((att * 100) / (att + def));
	if (winround) {
	    dmg = max(0, roll_dice1((DiceRep)uu_damage(u, d)));
	    victim = defender;
	    defender->hp2 -= dmg;
	} else {
	    dmg = max(0, roll_dice1((DiceRep)uu_damage(d, u)));
	    victim = unit;
	    unit->hp2 -= dmg;
	}
	if (defender->hp2 <= 0) {
	    report_combat(unit, defender, "destroy");
	} else if (unit->hp2 <= 0) {
	    report_combat(defender, unit, "destroy");
	}
	/* Pick the hit movie to show. */
	hitmovietype = (char *)((victim->hp2 <= 0) ? "death" : "hit-short");
	/* Make up the list of sides that will see it. */
	sidemask = NOSIDES;
	for_all_sides(side3) {
	    /* Let all sides that can see the attacker's cell see it. */
	    if (units_visible(side3, victim->x, victim->y))
	      sidemask = add_side_to_set(side3, sidemask);
	}
	/* Show the movie. */
	for_all_sides(side3) {
	    if (side_in_set(side3, sidemask))
	      schedule_movie(side3, hitmovietype, victim->id);
	}
	play_movies(ALLSIDES);
	damage_unit(unit, combat_dmg, defender);
	damage_unit(defender, combat_dmg, unit);
    }
    /* The whole stack dies if its defender dies. */
    if (!alive(defender) && defender->transport == NULL) {
	for_all_stack(x, y, unit2) {
	    if (!u_advanced(unit2->type)) {
		/* (should except spies etc?) */
		unit2->hp2 = 0;
		report_combat(unit, unit2, "destroy");
	    }
	}
	reckon_damage_here(x, y);
	/* But capture cities instead of destroying them. */
	for_all_stack(x, y, unit2) {
	    if (u_advanced(unit2->type)) {
		attempt_to_capture_unit(unit, unit2);
	    }
	}
	/* Occupants of city may be damaged. */
	reckon_damage_here(x, y);
    }
}

/* The real attack value of a unit after adjustments for combat exp, 
terrain effects, occupants etc. */

int
real_attack_value(Unit *unit)
{
	Unit *unit2;
	int t, u, att, effect;
	
	u = unit->type;
	att = u_attack(u);
	/* First factor in combat experience. */
	if (u_cxp_max(u) > 0 && unit->cxp > 0) {
	    effect = u_full_cxp_affects_attack(u);
	    if (effect != 100) {
		att *= (100 + (effect - 100) * unit->cxp / u_cxp_max(u)) / 100;
	    }
	}
	/* Then factor in terrain effects. */
	t = terrain_at(unit->x, unit->y);
	effect = ut_terrain_affects_attack(u, t);
	if (effect != 100) {
	    att *= effect / 100;
	}
	/* Then factor in effect of transport. */
	if (unit->transport) {
	    if (is_active(unit->transport)) {
		effect = uu_transport_affects_attack(unit->transport->type, u);	
		if (effect != 100) {
		    att *= effect / 100;
		}
	    }
	}	
	/* Then factor in effect of occupants. */
	for_all_occupants(unit, unit2) {
		if (is_active(unit2)) {
		    effect = uu_occ_affects_attack(unit2->type, u);
		    if (effect != 100) {
			att *= effect / 100;
		    }
		}
	}	
	/* Then factor in effect of friendly units in the same cell. */
	for_all_stack_with_occs(unit->x, unit->y, unit2) {
	    if (is_active(unit2)
		&& trusted_side(unit2->side, unit->side)) {
		effect = uu_neighbour_affects_attack(unit2->type, u);
		if (effect != 100) {
			att *= effect / 100;
		}
	    }
	}
	return att;
}


/* The real defense value of a unit after adjustments for combat exp, 
terrain effects, occupants etc. */

int
real_defense_value(Unit *unit)
{
	Unit *unit2;
	int t, u, def, effect;
	
	u = unit->type;
	def = u_defend(u);
	/* First factor in combat experience. */
	if (u_cxp_max(u) > 0 && unit->cxp > 0) {
	    effect = u_full_cxp_affects_defense(u);
	    if (effect != 100) {
		def *= (100 + (effect - 100) * unit->cxp / u_cxp_max(u)) / 100;
	    }
	}
	/* Then factor in terrain effects. */
	t = terrain_at(unit->x, unit->y);
	effect = ut_terrain_affects_defense(u, t);
	if (effect != 100) {
	    def *= effect / 100;
	}
	/* Then factor in effect of transport. */
	if (unit->transport) {
	    if (is_active(unit->transport)) {
		effect = uu_transport_affects_defense(unit->transport->type, u);
		if (effect != 100) {
		    def *= effect / 100;
		}
	    }
	}	
	/* Then factor in effect of occupants. */
	for_all_occupants(unit, unit2) {
	    if (is_active(unit2)) {
		effect = uu_occ_affects_defense(unit2->type, u);
		if (effect != 100) {
		    def *= effect / 100;
		}
	    }
	}	
	/* Then factor in effect of friendly units in the same cell. */
	for_all_stack_with_occs(unit->x, unit->y, unit2) {
	    if (is_active(unit2)
		&& trusted_side(unit2->side, unit->side)) {
		effect = uu_neighbour_affects_defense(unit2->type, u);
		if (effect != 100) {
		    def *= effect / 100;
		}
	    }
	}
	return def;
}


/* Fire-at action. */

/* Shooting at a given unit. */

int
prep_fire_at_action(Unit *unit, Unit *unit2, Unit *unit3, int m)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL || unit3 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_FIRE_AT;
    unit->act->nextaction.args[0] = unit3->id;
    unit->act->nextaction.args[1] = m;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_fire_at_action(Unit *unit, Unit *unit2, Unit *unit3, int m)
{
    int ux = unit->x, uy = unit->y, ox, oy, oz;
    SideMask sidemask;
    Side *side;

    action_point(unit2->side, unit3->x, unit3->y);
    action_point(unit3->side, unit3->x, unit3->y);
    /* Make up the list of sides that will see the fire. */
    sidemask = NOSIDES;
    for_all_sides(side) {
	/* Let all sides that can see the attacker's cell see it. */
	if (units_visible(side, unit2->x, unit2->y))
	  sidemask = add_side_to_set(side, sidemask);
	/* Let all sides that can see the unit3's cell see it. */
	if (units_visible(side, unit3->x, unit3->y))
	  sidemask = add_side_to_set(side, sidemask);
    }
    /* Show the fire. */
    for_all_sides(side) {
	if (side_in_set(side, sidemask))
	  update_fire_at_display(side, unit, unit3, m, TRUE);
    }
    ox = unit3->x;  oy = unit3->y;  oz = unit3->z;
    amain = unit;  omain = unit3;
    fire_on_unit(unit, unit3);
    reckon_damage(TRUE);
    if (alive(unit))
      use_up_acp(unit, u_acp_to_fire(unit2->type));
    /*	if (alive(unit3)) use_up_acp(unit3, 1); */
    /* Each side sees what happened to its own unit. */
    update_unit_display(unit2->side, unit2, TRUE);
    if (unit != unit2)
      update_unit_display(unit->side, unit, TRUE);
    update_unit_display(unit3->side, unit3, TRUE);
    /* The attacking side also sees the remote cell. */
    update_cell_display(unit2->side, ox, oy, UPDATE_ALWAYS);
    update_cell_display(unit3->side, ox, oy, UPDATE_ALWAYS);
    /* Victim might see something in attacker's cell. */
    update_cell_display(unit3->side, ux, uy, UPDATE_ALWAYS);
    /* Actually, everybody might be seeing the combat. */
    all_see_cell(ux, uy);
    all_see_cell(ox, oy);
    // Capture can occur as a result of firing,
    //	but only if victim is in or adjacent to ZOC.
    if (distance(unit2->x, unit2->y, unit3->x, unit3->y) 
	<= (uu_zoc_range(unit2->type, unit3->type) + 1)) {
	Side *os = unit3->side;

	attempt_to_capture_unit(unit2, unit3);
	/* If the unit3 was not captured, it might turn the tables! */
	/* (Note that we cannot cache the attacker's and unit3's
	   types, because the type might have changed due to damage,
	   and we want to know if the new type might countercapture.)  */
	if (alive(unit3)
	    && alive(unit2)
	    && unit3->side == os
	    && uu_countercapture(unit2->type, unit3->type) > 0) {
	    attempt_to_capture_unit(unit3, unit2);
	}
    }
    /* Always expend the ammo (but only if m is a valid mtype). */
    return A_ANY_DONE;
}

/* Test a fire action for plausibility. */

int
check_fire_at_action(Unit *unit, Unit *unit2, Unit *unit3, int m)
{
    int u, u2, u3, ux, uy, uz, dist;
    int rslt = A_ANY_OK;

    if (!valid(rslt = can_fire(unit, unit2)))
      return rslt;
    if (!in_play(unit3))
      return A_ANY_ERROR;
    /* We can't attack ourselves. */
    if (unit2 == unit3)
      return A_ANY_ERROR;
    u = unit->type; u2 = unit2->type;  u3 = unit3->type;
    ux = unit->x;  uy = unit->y;  uz = unit->z;
    /* Check that target is in range. */
    dist = distance(ux, uy, unit3->x, unit3->y);
    if (dist > u_range(u2))
      return A_ANY_TOO_FAR;
    if (dist < u_range_min(u2))
      return A_ANY_TOO_NEAR;
    if (!indep(unit2) && unit2->side == unit3->side)
      return A_ANY_ERROR;
    /* Attackers won't fire at anything they know they can't hit. */
    if (fire_hit_chance(u2, u3) <= 0)
      return A_FIRE_CANNOT_HIT;
    /* Check intervening elevations. */
    if (found_blocking_elevation(u2, ux, uy, uz, u3,
				 unit3->x, unit3->y, unit3->z))
      return A_FIRE_BLOCKED;
    /* Check for enough of right kind of ammo 
	(this feature is not used anywhere in the code. 
	(-1) is always passed as m). */
    if (is_material_type(m)) {
    	if (unit->supply[m] == 0) {
		return A_ANY_NO_AMMO;
    	}
    } else {
	if (!enough_ammo_to_fire_at_unit(unit2, unit3))
	    return A_ANY_NO_AMMO;
    }
    return A_ANY_OK;
}

/* Fire-into action. */

/* Shooting at a given location. */

int
prep_fire_into_action(Unit *unit, Unit *unit2, int x, int y, int z, int m)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_FIRE_INTO;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.args[3] = m;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* One can always shoot, if the cell is visible, but there might not
   not be anything to hit!  No counterattacks when shooting, and the
   results might not be visible to the shooter. */

int
do_fire_into_action(Unit *unit, Unit *unit2, int x, int y, int z, int m)
{
    int ux = unit->x, uy = unit->y, ox, oy, oz;
    SideMask sidemask;
    Unit *other;
    Side *side;
    
    /* Make up the list of sides that will see the fire. */
    sidemask = NOSIDES;
    for_all_sides(side) {
	/* Let all sides that can see the attacker's cell see it. */
	if (units_visible(side, unit2->x, unit2->y))
	  sidemask = add_side_to_set(side, sidemask);
	/* Let all sides that can see the fired-into cell see it. */
	if (units_visible(side, x, y))
	  sidemask = add_side_to_set(side, sidemask);
    }
    /* Show the fire. */
    for_all_sides(side) {
	if (side_in_set(side, sidemask))
		update_fire_into_display(side, unit2, x, y, z, m, TRUE);
    }
    /* If any units at target, hit them. */
    for_all_stack(x, y, other) {
	ox = other->x;  oy = other->y;  oz = other->z;
	amain = unit;  omain = other;
	fire_on_unit(unit2, other);
	reckon_damage(TRUE);
	/* Each side sees what happened to its unit that is being hit. */
	update_unit_display(other->side, other, TRUE);
	/* The attacking side also sees the remote cell. */
	update_cell_display(unit->side, ox, oy, UPDATE_ALWAYS);
	update_cell_display(other->side, ox, oy, UPDATE_ALWAYS);
	/* Victim might see something in attacker's cell. */
	update_cell_display(other->side, ux, uy, UPDATE_ALWAYS);
	/* Actually, everybody might be seeing the combat. */
	all_see_cell(ux, uy);
	all_see_cell(ox, oy);
	/* don't take moves though! */
    }
    /* Firing side gets just one update. */
    update_unit_display(unit2->side, unit2, TRUE);
    if (unit != unit2)
      update_unit_display(unit->side, unit, TRUE);
    if (alive(unit))
      use_up_acp(unit, u_acp_to_fire(unit2->type));
    /* Always expend the ammo (but only if m is a valid material). */
    /* We're always "successful", even though the bombardment may have
       had little or no actual effect. */
    return A_ANY_DONE;
}

/* Test a shoot action for plausibility. */

int
check_fire_into_action(Unit *unit, Unit *unit2, int x, int y, int z, int m)
{
    int u, u2, u2x, u2y, u2z, dist;
    int rslt = A_ANY_OK;

    if (!valid(rslt = can_fire(unit, unit2)))
      return rslt;
    u2x = unit2->x;  u2y = unit2->y;  u2z = unit2->z;
    /* Check that target location is meaningful. */
    if (!inside_area(x, y))
      return A_FIRE_INTO_OUTSIDE_WORLD;
    u = unit->type;
    u2 = unit2->type;
    /* Check that target is in range. */
    dist = distance(u2x, u2y, x, y);
    if (dist > u_range(u2))
      return A_ANY_TOO_FAR;
    if (dist < u_range_min(u2))
      return A_ANY_TOO_NEAR;
    /* Check intervening elevations and terrain. */
    if (found_blocking_elevation(u2, u2x, u2y, u2z, NONUTYPE, x, y, z))
      return A_FIRE_BLOCKED;
    /* Check for enough of right kind of ammo. */
    if (is_material_type(m)) {
    	if (unit->supply[m] == 0)
    	  return A_ANY_NO_AMMO;
    } else {
	if (!enough_ammo_to_fire_one_round(unit2))
	    return A_ANY_NO_AMMO;
    }
    return A_ANY_OK;
}

static int tmpx0, tmpy0, tmpe0, tmpe1, tmpdist01, cellwid, fangle;

static int elevation_blocks(int x, int y);

static int
elevation_blocks(int x, int y)
{
    int interpol, celldist0, dist0, el;

    celldist0 = distance(tmpx0, tmpy0, x, y);
    dist0 = celldist0 * cellwid;
    interpol = (tmpe1 - tmpe0) - (fangle * tmpdist01) / 100;
    /* The following is * dist0^2 / tmpdist01^2, but arranged to avoid
       overflow. */
    interpol *= dist0;
    interpol /= tmpdist01;
    interpol *= dist0;
    interpol /= tmpdist01;
    interpol += (fangle * dist0) / 100;
    interpol += tmpe0;
    el = (elevations_defined() ? elev_at(x, y) : 0);
    el += t_thickness(terrain_at(x, y));
    Dprintf("Arc at %d: %d, elev %d\n", celldist0, interpol, el);
    return (el > interpol);
}

int
found_blocking_elevation(int u, int ux, int uy, int uz,
			 int u2, int u2x, int u2y, int u2z)
{
    int t, weaponheight, bodyheight, rslt, x1, y1;

    /* Note that a flat world with no elevation defined could
       still have thick terrain that screens. */
    /* (should detect absence of thick terrain and return immed) */
    /* Adjacent cells can't be screened by elevation. */
    /* (should accommodate possibility that target is at top of
       cliff in adj and back away from its edge, thus screened;
       need to recog cliffs vs slopes in terrain) */
    if (distance(ux, uy, u2x, u2y) <= 1)
      return FALSE;
    tmpx0 = ux;  tmpy0 = uy;
    t = terrain_at(ux, uy);
    tmpe0 = (elevations_defined() ? elev_at(ux, uy) : 0);
    weaponheight = ut_weapon_height(u, t);
    tmpe0 += weaponheight;
    tmpe1 = (elevations_defined() ? elev_at(u2x, u2y) : 0);
    bodyheight = ut_body_height(u, t);
    tmpe1 += bodyheight;
    cellwid = area.cellwidth;
    if (cellwid <= 0)
      cellwid = 1;
    fangle = u_fire_angle_max(u);
    tmpdist01 = distance(ux, uy, u2x, u2y) * cellwid;
    Dprintf("Checking arc: %d,%d el %d -> %d,%d el %d\n", ux, uy, tmpe0, u2x, u2y, tmpe1);
    rslt = search_straight_line(ux, uy, u2x, u2y, elevation_blocks, &x1, &y1);
    if (rslt)
      Dprintf("blocked at %d,%d\n", x1, y1);
    return rslt;
}
 
static void
fire_on_unit(Unit *atker, Unit *other)
{
    int m, dist = distance(atker->x, atker->y, other->x, other->y), consump;
    int a = NONUTYPE, o = NONUTYPE;

    if (alive(atker) && alive(other)) {
	a = atker->type;  o = other->type;
	if (enough_ammo_to_fire_at_unit(atker, other)) {
	    maybe_hit_unit(atker, other, TRUE, (dist > u_hit_falloff_range(a)));
	    if (alive(atker)) {
		for_all_material_types(m) {
		    consump = um_hit_by(o, m);
		    if (consump > 0) {
			consump *= um_consumption_per_fire(a, m);
			if (consump < 0) 
			  consump = 
			    um_hit_by(o, m) * um_consumption_per_attack(a, m);
			atker->supply[m] -= consump;
		    }
		}
	    }
	    /* The *victim* can lose acp. */
	    if (alive(other))
	      use_up_acp(other, uu_acp_to_be_fired_on(o, atker->type));
	}
    }
    /* (should ping victim to see if it wants to respond to the attack?) */
}

/* Test to see if enough ammo is available to make the attack.  Need
   enough of *all* types - semi-bogus but too complicated otherwise?  */

int
enough_ammo_to_attack_unit(Unit *unit, Unit *other)
{
    int m, hitby;

    for_all_material_types(m) {
        hitby = um_hit_by(other->type, m);
        if (0 < hitby)
            if (unit->supply[m] < \
              hitby * um_consumption_per_attack(unit->type, m))
                return FALSE;
    }
    return TRUE;
}

int
enough_ammo_to_fire_at_unit(Unit *unit, Unit *other)
{
    int m, hitby, consump;

    for_all_material_types(m) {
        hitby = um_hit_by(other->type, m);
        if (0 < hitby) {
            consump = hitby * um_consumption_per_fire(unit->type, m);
            if (0 > consump)
                consump = hitby * um_consumption_per_attack(unit->type, m);
            if (unit->supply[m] < consump) 
                return FALSE;
        }
    }
    return TRUE;
}

/* Since we don't know what we are firing at, we test if we have enough of
all ammo types required to fire at any target that we can hit. */

int
enough_ammo_to_fire_one_round(Unit *unit)
{
    int m;

    for_all_material_types(m) {
	if (unit->supply[m] < um_consumption_per_fire(unit->type, m))
	  return FALSE;
    }
    return TRUE;
}

/* Single attack, no counterattack.  Check and use ammo - usage
   independent of outcome, but types used depend on unit types
   involved. */

static void
attack_unit(Unit *atker, Unit *other)
{
    int m, hitby;
    int a = NONUTYPE, o = NONUTYPE;

    if (alive(atker) && alive(other)) {
	a = atker->type;  o = other->type;
	if (enough_ammo_to_attack_unit(atker, other)) {
	    maybe_hit_unit(atker, other, FALSE, FALSE);
	    if (alive(atker)) {
		for_all_material_types(m) {
		    hitby = um_hit_by(o, m);
		    if (hitby > 0) {
		      atker->supply[m] -=
			um_consumption_per_attack(a, m) * hitby;
		    }
		}
	    }
	}
    }
    /* (should ping victim to see if it wants to respond to the attack?) */
}

/* Make a single hit and maybe hit some passengers also.  Power of hit
   is constant, but chance is affected by terrain, quality, and
   occupants' protective abilities.  If a hit is successful, it may
   have consequences on the defender's occupants, but limited by the
   protection that the transport provides. */

static void
maybe_hit_unit(Unit *atker, Unit *other, int fire, int fallsoff)
{
    int chance, t, hit = 0, a = atker->type, o = other->type;
    int teffect, cxpeffect, cxpmax, effect, prot;
    int dist = 0, distdiff = 0, rangefoff = 0, rangemax = 0, hitrdist = 0, 
        hitrfoff = 0, hitrmax = 0;
    int retrchance = 0;
    DiceRep dmgspec;
    char *hitmovietype;
    Unit *occ, *unit2;
    Side *side3;
    SideMask sidemask;    

    Dprintf("%s tries to hit %s", unit_desig(atker), unit_desig(other));
    if (fire)
      chance = fire_hit_chance(a, o);
    else
      chance = uu_hit(a, o);
    /* Combat experience tends to raise the hit chance, so do that
       first, reduces roundoff weirdnesses later. */
    cxpmax = u_cxp_max(a);
    if (cxpmax > 0 && atker->cxp > 0) {
	cxpeffect = uu_hit_cxp(a, o);
	if (cxpeffect != 100) {
	    effect = 100 + (atker->cxp * (cxpeffect - 100)) / cxpmax;
	    chance = (chance * effect) / 100;
	}
    }
    /* (should modify due to cxp of defender too) */
    /* Account for terrain effects. */
    t = terrain_at(atker->x, atker->y);
    if (fire && ut_fire_attack_terrain_effect(a, t) != -1)
      teffect = ut_fire_attack_terrain_effect(a, t);
    else
      teffect = ut_attack_terrain_effect(a, t);
    chance = (chance * teffect) / 100;
    t = terrain_at(other->x, other->y);
    if (fire && ut_fire_defend_terrain_effect(o, t) != -1)
      teffect = ut_fire_defend_terrain_effect(o, t);
    else
      teffect = ut_defend_terrain_effect(o, t);
    chance = (chance * teffect) / 100;
    /* Also account for aux terrain effects. */
    /* (should also account for direction of attack in the case of 
        borders, and possibly connectors as well) */
    if (any_aux_terrain_defined()) {
        for_all_aux_terrain_types(t) {
            if (aux_terrain_defined(t)) {
                if (aux_terrain_at(atker->x, atker->y, t)) {
                    if (fire && ut_fire_attack_terrain_effect(a, t) != -1)
                      teffect = ut_fire_attack_terrain_effect(a, t);
                    else
                      teffect = ut_attack_terrain_effect(a, t);
                    chance = (chance * teffect) / 100;
                }
                if (aux_terrain_at(other->x, other->y, t)) {
                    if (fire && ut_fire_defend_terrain_effect(o, t) != -1)
                      teffect = ut_fire_defend_terrain_effect(o, t);
                    else
                      teffect = ut_defend_terrain_effect(o, t);
                    chance = (chance * teffect) / 100; 
                }
            }
        } 
    }
    /* Account for protection by occupants. */
    for_all_occupants(other, occ) {
	if (in_play(occ) && completed(occ)) {
	    prot = uu_protection(occ->type, o);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    /* Account for protection by neighbours. */
    for_all_stack(other->x, other->y, unit2) {
	if (unit2 != other
	    && in_play(unit2)
	    && completed(unit2)
	    && unit2->side == other->side) {
	    prot = uu_stack_protection(unit2->type, o);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    /* Note that this code differs from that above in that it treats
       all units in the same cell equally, whether occs, suboccs or
       cellmates. This also means that one occ can protect another occ
       in the same transport. Moreover, the protective unit also
       protects itself. These rules simulate real situations such as
       when triple-A protects all nearby units (including itself)
       against bombers, or when a city wall protects all other
       occupants against ground attack. */
    for_all_stack_with_occs(other->x, other->y, unit2) {
	if (is_active(unit2)
	    /* We also extend protection to our buddies! */
	    && trusted_side(unit2->side, other->side)) {
	    /* This is when a unit, such as triple-A, extends unique
	       protection against a specific attacker, such as
	       bombers, to all other units in the cell. */
	    prot = uu_cellwide_protection_against(unit2->type, a);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	    /* This is when a unit (such as a garrison) specifically
	       protects a second unit, such as a fort (but not other
	       nearby units), against all forms of attack. It thus
	       works the same way as uu_protection and
	       uu_stack_protection. */
	    prot = uu_cellwide_protection_for(unit2->type, o);	    
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    /* Determine how distance affects ranged fire.
       First, perform a [sanity] check to make sure that we are only 
       executing this segment if the unit is firing and the effect of 
       that fire falls off over distance.
       Second, calculate distance to target, and the difference, 
       distdiff, between this and the falloff range.
       Third, calculate linear interpolant (slope of falloff effect) by 
       determining the difference between rangemax and rangefoff and 
       the difference between hitrmax and hitrfoff.
       Finally, interpolate the falloff effect, provided distdiff. */
    if (fire && fallsoff) {
        dist = distance(atker->x, atker->y, other->x, other->y);
        rangefoff = u_hit_falloff_range(a);
        distdiff = dist - rangefoff;
        if (0 < distdiff) {
            rangemax = u_range(a);
            hitrfoff = fire_hit_chance(a, o);
            hitrmax = (uu_hit_max_range_effect(a, o) * hitrfoff) / 100; 
            hitrdist = 
                hitrfoff + 
                (distdiff * (hitrmax - hitrfoff)) / (rangemax - rangefoff);
            Dprintf(", fire distance is %d", dist);
            Dprintf(", fire falloff modifier is %d%%", hitrdist);
            chance = (chance * hitrdist) / 100;
        }
        /* else no falloff */
    }
    Dprintf(", probability of hit is %d%%", chance);
    /* Compute the hit itself. */
    if (probability(chance)) {
	if (fire)
          dmgspec = (DiceRep)fire_damage(a, o);
	else
	  dmgspec = (DiceRep)uu_damage(a, o);
    	/* Account for attacker's experience. */
	if (cxpmax > 0 && atker->cxp > 0) {
	    cxpeffect = uu_damage_cxp(a, o);
	    if (cxpeffect != 100) {
		effect = 100 + (atker->cxp * (cxpeffect - 100)) / cxpmax;
		dmgspec = multiply_dice1(dmgspec, effect);
	    }
	}
	hit = max(0, roll_dice1(dmgspec));
    }
    if (hit > 0) {
    	Dprintf(", damage will be %d hp", hit);
    } else {
    	Dprintf(", missed");
    }
    /* (should record a raw statistic?) */
    /* Ablation is a chance for occupants or stack to take part of a
       hit themselves. */
    if (hit > 0) {
	/* (should decide how ablation computed) */
    }
    /* Affect the hitting unit's morale.  Note that morale is still reduced
       even if the hit becomes 0 because of a successful retreat. */
    if (hit > 0) {
	change_morale(atker, 1, uu_morale_hit(a, o));
    }
    if (hit > 0) {
	retrchance = uu_retreat_chance(a, o);
	/* Chance of a retreat rises to 100% as morale goes to 0. */
	if (u_morale_max(o) > 0 && retrchance < 100)
	  retrchance = 100 - ((100 - retrchance) * other->morale) / 
		       u_morale_max(o);
	if (probability(retrchance)) {
	    if (retreat_unit(other, atker)) {
	    	/* It is possible that the unit died while trying to retreat 
		   into terrain where it vanishes. If so, we don't want to 
		   continue, since calling maybe_hit_unit recursively for an 
		   occupant will crash the game due to stack iteration for 
		   (-1,-1).  Nor do we want to report its retreat since its 
		   demise already has been reported. */
	    	if (!alive(other)) {
		    return;
	    	}
		report_combat(atker, other, "retreat");
		hit = 0; /* should only be reduced hit, may still be > 0 */
	    }
	}
    }
    hit_unit(other, hit, atker);
    /* Pick the hit movie to show. */
    hitmovietype =
      (char *)((hit >= other->hp) ? "death" : ((hit > 0) ? "hit" : "miss"));
    /* Make up the list of sides that will see it. */
    sidemask = NOSIDES;
    for_all_sides(side3) {
	/* Let all sides that can see the attacker's cell see it. */
	if (units_visible(side3, atker->x, atker->y))
	  sidemask = add_side_to_set(side3, sidemask);
	/* Let all sides that can see the defender's cell see it. */
	if (units_visible(side3, other->x, other->y))
	  sidemask = add_side_to_set(side3, sidemask);
    }
    /* Show the movie. */
    for_all_sides(side3) {
	if (side_in_set(side3, sidemask))
	  schedule_movie(side3, hitmovietype, other->id);
    }
    Dprintf("\n");
    /* Recurse into occupants, maybe hit them too.  */
    /* It would really make sense to check for hit > 0 here and only
    hit occupants if the transport was hit. However, this will not
    work since some games require that occupants can be hit even
    if we miss the transport. */
    for_all_occupants(other, occ) {
	if (is_active(other)) {
	    if (probability(uu_protection(o, occ->type)))
	      maybe_hit_unit(atker, occ, fire, fallsoff);
    	} else {
	    /* No protection by incomplete transport. */
	    maybe_hit_unit(atker, occ, fire, fallsoff);
	}
    }
    /* Some units might detonate automatically upon scoring a melee hit. */
    if (!fire && (0 < hit)
        && in_play(atker)
        && probability(u_detonate_with_attack(atker->type))
        && !was_detonated(atker)) {
    	detonate_unit(atker, atker->x, atker->y, atker->z);
    }
    /* We get combat experience only if there could have been some damage. */
    if (chance > 0) {
    	if (in_play(atker) && (atker->cxp < u_cxp_max(a)))
    	  atker->cxp += uu_cxp_per_combat(a, o);
    	if (in_play(other) && (other->cxp < u_cxp_max(o)))
    	  other->cxp += uu_cxp_per_combat(o, a);
    	/* Occupants already gained their experience in the recursive call. */
    }
}

/* Do the hit itself. */

static void
hit_unit(Unit *unit, int hit, Unit *atker)
{
    int u = unit->type, u2, hpmin, tpdmg, tp;
    Side *aside;
    int gain = 0, loss = 0;
    int m = NONMTYPE;
    DiceRep dmgspec = 0;

    /* Some units might detonate automatically upon being hit. */
    if (hit > 0
        && atker != NULL
        && probability(uu_detonate_on_hit(u, atker->type))
        && !was_detonated(unit)) {
    	detonate_unit(unit, unit->x, unit->y, unit->z);
    	/* If the detonating unit still exists, then continue on to
    	   normal damage computation. */
    }
    if (hit > 0) {
	/* Record the loss of hp. */
	unit->hp2 -= hit;
	if (atker != NULL) {
	    /* Attacker might not be able to do any more damage.  Note
	       that the positioning of this code is such that all the
	       usual side effects of combat happen, but the victim
	       doesn't get any more worse off than it is already. */
	    hpmin = uu_hp_min(atker->type, u);
	    if (hpmin > 0 && hpmin > unit->hp2)
	      unit->hp2 = hpmin;
	    /* Affect the morale of the unit being hit. */
	    change_morale(unit, -1, uu_morale_hit_by(atker->type, u));
	}
	/* Collateral damage may include loss of tooling. */
	if (unit->tooling && 1 /* any_tp_damage */) {
	    for_all_unit_types(u2) {
		dmgspec = (DiceRep)uu_tp_damage(u, u2);
		tpdmg = max(0, roll_dice1(dmgspec));
		if (tpdmg != 0) {
		    tp = unit->tooling[u2] - tpdmg;
		    tp = max(0, min(tp, uu_tp_max(u, u2)));
		    unit->tooling[u2] = tp;
		}
	    }
	}
    }
    /* Maybe record for statistical analysis. */
    /* (this is only useful if code always goes through here - is that true?) */
    if (atker != NULL) {
	aside = atker->side;
	u2 = atker->type;
	if (aside->atkstats[u2] == NULL)
	  aside->atkstats[u2] = (long *) xmalloc(numutypes * sizeof(long));
	if (aside->hitstats[u2] == NULL)
	  aside->hitstats[u2] = (long *) xmalloc(numutypes * sizeof(long));
	++((aside->atkstats[u2])[u]);
	(aside->hitstats[u2])[u] += hit;
    }
    /* Some units may detonate automatically just before dying. */
    if (hit > 0 && unit->hp2 <= 0
        && probability(u_detonate_on_death(u))
        && !was_detonated(unit)) {
	detonate_unit(unit, unit->x, unit->y, unit->z);
    }
    /* Treasury bonus and penalty due to unit destruction */
    if (hit > 0 && unit->hp2 <= 0) {
        for_all_material_types(m) {
            if (atker && side_has_treasury(atker->side, m)) {
		gain = um_treasury_gain_per_destroy(atker->type, m);
		atker->side->treasury[m] += gain;
		/* Clip treasury to bounds, if necessary. */
		atker->side->treasury[m] = 
		    min(atker->side->treasury[m], g_treasury_size());
		atker->side->treasury[m] = max(atker->side->treasury[m], 0);
	    }
            if (side_has_treasury(unit->side, m)) {
		loss = um_treasury_loss_per_destroyed(unit->type, m);
		unit->side->treasury[m] -= loss;
		/* Clip treasury to bounds, if necessary. */
		unit->side->treasury[m] = 
		    min(unit->side->treasury[m], g_treasury_size());
		unit->side->treasury[m] = max(unit->side->treasury[m], 0);
	    }
        }  
    }
    /* CxP reward based on unit destruction */
    if (atker && hit > 0 && unit->hp2 <= 0 && (atker->cxp < u_cxp_max(u))) 
      atker->cxp += uu_cxp_per_destroy(u, u2);
    /* Should wake the unit if it receives a tap from enemy. */
    /* (Perhaps we should worry about waking occupants according to 
       recursive waking rules?) */
    if ((hit > 0) && is_active(unit))
      wake_unit(unit->side, unit, FALSE);
}

/* Hits on the main units have to be done later, so that mutual
   destruction works properly.  This function also does all the notifying. */

/* (What if occupants change type when killed, but transport vanishes?) */

static void
reckon_damage(int fire)
{
    int ax = -1, ay = -1, ox = -1, oy = -1, range = -1;
    int o = NONUTYPE, a = NONUTYPE;

    /* Entertain everybody. */
    play_movies(ALLSIDES);
    /* Report the damage in more detail, now before the actual damage
       is taken (which may cause many units to disappear). */
    /* Normally we report the defender and then the attacker's damage,
       but if the defender dies, we report its counterattack results
       first and its death second. */
    if (!(omain->hp2 <= 0 && amain->hp2 > 0)) {
	report_damage(omain, amain, omain, fire);
	if (!fire && will_report_damage(amain))
	  report_damage(amain, omain, amain, fire);
    } else {
	if (!fire)
	  report_damage(amain, omain, amain, fire);
	report_damage(omain, amain, omain, fire);
    }
    a = amain->type;
    o = omain->type;
    ax = amain->x;  ay = amain->y;
    ox = omain->x;  oy = omain->y;
    damage_unit(omain, combat_dmg, amain);
    damage_unit(amain, combat_dmg, omain);
    /* If the attacker or defender may have detonated, 
	then look around to see what else was damaged. */
    range = max(range, detonate_urange_max());
    range = max(range, max_t_detonate_effect_range);
    if ((-1 < range) && u_detonate_with_attack(amain->type))
      reckon_damage_around(ax, ay, range, amain);
    if ((-1 < range)
	&& (uu_detonate_on_capture(o, a) || uu_detonate_on_hit(o, a)
	    || u_detonate_on_death(o))) 
      reckon_damage_around(ox, oy, range, omain);
}

/*! \todo Evil var for search. Replace with parambox instead. */
Unit *tmp_unit_detonator = NULL;

static void
reckon_damage_here(int x, int y)
{
    Unit *unit = NULL, *nextunit = NULL;

    /* NOTE: Use special iterator because units may get pulled out from 
	under us while we are iterating. */
    for (unit = unit_at(x, y); unit; unit = nextunit) {
	nextunit = unit->nexthere;
	damage_unit(unit, combat_dmg, tmp_unit_detonator);
    }
}

void
reckon_damage_around(int x, int y, int r, Unit *detonator)
{
    tmp_unit_detonator = detonator;
    if (r > 0) {
	apply_to_area(x, y, r, reckon_damage_here);
    } else {
	reckon_damage_here(x, y);
    }
    tmp_unit_detonator = NULL;
}

/* Given the units involved in combat, decide how to report it.  The
   strings like "destroy" and "miss" are symbolic, don't normally
   appear in the final output. */

static void
report_damage(Unit *unit, Unit *atker, Unit *mainunit, int fire)
{
    Unit *occ;

    /* Don't report supposed actions by dead units. */
    if (!alive(atker))
      return;
    if (unit->hp2 <= 0) {
	if (unit == mainunit) {
	    report_combat(atker, unit, "destroy");
	} else {
	    report_combat(atker, unit, "destroy-occupant");
	}
    } else if (unit->hp2 < unit->hp) {
	if (unit == mainunit) {
	    report_combat(atker, unit, "hit");
	} else {
	    report_combat(atker, unit, "hit-occupant");
	}
    } else {
	if (unit == mainunit) {
	    if (fire) {
	    	report_combat(atker, unit, "miss-fire");
	    } else {
	    	report_combat(atker, unit, "miss-attack");
	    }	
	} else {
	    report_combat(atker, unit, "miss-occupant");
	}
    }
    for_all_occupants(unit, occ) {
	report_damage(occ, atker, mainunit, fire);
    }
}

/* Return true if the given unit or one of its occupants has been
   damaged. */

static int
will_report_damage(Unit *unit)
{
    Unit *occ;

    if (unit->hp2 < unit->hp)
      return TRUE;
    for_all_occupants(unit, occ) {
	if (will_report_damage(occ))
	  return TRUE;
    }
    return FALSE;
}

/* Make the intended damage become real, and do any consequences. */
/* (should include agent unit with hevts if appropriate) */
void
damage_unit(Unit *unit, enum damage_reasons dmgreason, Unit *agent)
{
    int newacp;
    HistEventType hevttype;
    Obj *dameff;
    Unit *occ;
    DestructionResult drslt = DESTRUCT_ORDINARY;
    int fkillunit = FALSE;
    int u = NONUTYPE, u2 = NONUTYPE, t = NONTTYPE, m = NONMTYPE, ms = NONMTYPE;

    assert_error(unit, "Tried to damage a null unit.");
    /* Prepare table lookups. */
    u = unit->type;
    /* (At this point, detonation damage still counts as combat damage.) */
#if (0)
    if (dmgreason == combat_dmg) {
	assert_error(agent, "Receiving damage from a null unit?!");
	u2 = agent->type;
    }
#endif
    if (agent)
      u2 = agent->type;
    t = terrain_at(unit->x, unit->y);
    for_all_material_types(m) {
	if ((unit->supply[m] <= 0) && (um_hp_per_starve(u, m) > 0)) {
	    ms = m;
	    break;
	}
    }
    /* Process all the occupants first. */
    for_all_occupants(unit, occ) {
	damage_unit(occ, dmgreason, agent);
    }
    /* If no damage was recorded, just return. */
    if (unit->hp2 == unit->hp) {
	set_was_detonated(unit, FALSE);
	return;
    }
    /* If unit is to die, do the consequences. */
    if (unit->hp2 <= 0) {
	/* Decide whether the unit should vanish or wreck. */
	drslt = determine_destruction_result(unit);
	switch (drslt) {
	  case DESTRUCT_VANISH:
	    fkillunit = TRUE;
	    break;
	  case DESTRUCT_ORDINARY:
	    fkillunit = (u_wrecked_type(unit->type) == NONUTYPE);
	    if ((dmgreason == starvation_dmg) && is_material_type(ms))
	      fkillunit = fkillunit 
			  && (um_wrecked_type_if_starved(u, ms) == NONUTYPE);
	    else if ((dmgreason == combat_dmg) && is_unit_type(u2))
	      fkillunit = fkillunit
			  && (uu_wrecked_type_if_killed(u, u2) == NONUTYPE);
	    else if ((dmgreason == attrition_dmg) && is_terrain_type(t))
	      fkillunit = fkillunit
			  && (ut_wrecked_type_if_attrited(u, t) == NONUTYPE);
	    /* (Could add a case for dying in accidents.) */
	    break;
	  default:
	    fkillunit = TRUE;
	    run_warning("Unknown unit destruction result encountered.");
	} /* switch (drslt) */
	/* Make the unit vanish forthwith. */
	if (fkillunit) {
	    rescue_occupants(unit);
	    switch (dmgreason) {
	      case combat_dmg:
		hevttype = H_UNIT_KILLED;
		break;
	      case accident_dmg:
		hevttype = H_UNIT_DIED_IN_ACCIDENT;
		break;
	      case attrition_dmg:
		hevttype = H_UNIT_DIED_FROM_ATTRITION;
		break;
	      case starvation_dmg:
		hevttype = H_UNIT_STARVED;
		break;
	      default:
		hevttype = (HistEventType)0;
	    }
	    kill_unit(unit, hevttype);
	/* Wreck the unit. */
	} else {
	    switch (dmgreason) {
	      case combat_dmg:
		hevttype = H_UNIT_WRECKED;
		wreck_unit(unit, hevttype, WRECK_TYPE_KILLED, u2, agent);
		break;
	      case accident_dmg:
		hevttype = H_UNIT_WRECKED_IN_ACCIDENT;
		wreck_unit(unit, hevttype, WRECK_TYPE_UNSPECIFIED, -1, NULL);
		break;
	      case attrition_dmg:
		hevttype = H_UNIT_WRECKED_FROM_ATTRITION;
		wreck_unit(unit, hevttype, WRECK_TYPE_ATTRITED, t, NULL);
		break;
	      case starvation_dmg:
		hevttype = H_UNIT_STARVED;
		wreck_unit(unit, hevttype, WRECK_TYPE_STARVED, ms, NULL);
		break;
	      default:
		hevttype = (HistEventType)0;
		wreck_unit(unit, hevttype, WRECK_TYPE_UNSPECIFIED, -1, NULL);
	    }
	}
    } else {
	record_event(H_UNIT_DAMAGED, add_side_to_set(unit->side, NOSIDES),
		     unit->id, unit->hp, unit->hp2);
	/* Change the unit's hp. */
	unit->hp = unit->hp2;
	/* Perhaps adjust the acp down. */
	if (unit->act != NULL
	    && unit->act->acp > 0
	    && (dameff = u_acp_damage_effect(unit->type)) != lispnil) {
	    newacp = damaged_acp(unit, dameff);
	    /* The damaged acp limits the remaining acp, rather than trying
	       to do some sort of proportional adjustment, which would be
	       hard to get right. */
	    /* (should account for occupant effects on acp) */
	    unit->act->acp = min(unit->act->acp, newacp);
	}
    }
    /* Clear any detonation flag that might have been set. */
    if (alive(unit))
      set_was_detonated(unit, FALSE);
    /* Let the unit's owner know about all this. */
    update_unit_display(unit->side, unit, TRUE);
}

/* Figure out what the result of destroying the unit is. */

static DestructionResult
determine_destruction_result (Unit *unit)
{
    DestructionResult drslt = DESTRUCT_ORDINARY;
    int u = NONUTYPE;
    Obj *choice = NULL, *chtype = NULL;
    int weightstotal = 0;

    assert_error(unit, "Tried to manipulate a null unit.");
    u = unit->type;
    if (!completed(unit))
      return DESTRUCT_VANISH;
    if (lispnil != u_destruction_result(u)) {
	choice = choose_from_weighted_list(u_destruction_result(u), 
					   &weightstotal, FALSE);
	chtype = consp(choice) ? car(choice) : choice;
	/* Make the unit unconditionally vanish. */
	if (match_keyword(chtype, K_VANISH)) {
	    drslt = DESTRUCT_VANISH;
	}
	/* Lookup the result in the appropriate table. */
	else if ((lispnil == chtype) || match_keyword(chtype, K_TABLE)) {
	    /* Use the ordinary result. */
	}
	else
	  run_warning("Unknown destruction result encountered.");
    }
    return drslt;
}

/* Occupants may be able to avoid the fate of their transport. */

void
rescue_occupants(Unit *unit)
{
    Unit *occ;

    for_all_occupants(unit, occ) {
	if (probability(uu_occ_escape(unit->type, occ->type))) {
	    rescue_one_occupant(occ);
	}
    }
}

static void
rescue_one_occupant(Unit *occ)
{
    int rslt, dir, tmp, nx, ny;
    Unit *other;

    /* Try to escape into another unit in this cell. */
    for_all_stack(occ->x, occ->y, other) {
	if (other != occ->transport) {
	    rslt = check_enter_action(occ, occ, other);
	    if (valid(rslt)) {
		do_enter_action(occ, occ, other);
		return;
	    }
	}
    }
    /* Try to move into an adjacent cell. */
    for_all_directions_randomly(dir, tmp) {
	if (interior_point_in_dir(occ->x, occ->y, dir, &nx, &ny)) {
	    if (retreat_in_dir(occ, dir))
	      return;
	}
    }
}

/* Retreat is a special kind of movement that a unit uses to avoid
   damage during combat. It bypasses some of the normal move rules. */

static int
retreat_unit(Unit *unit, Unit *atker)
{
    int dir;
    extern int retreating_from;

    /* First check that this type is able to retreat. */
    if (unit->act == NULL || !mobile(unit->type))
      return FALSE;
    retreating_from = atker->type;
    if (unit->x == atker->x && unit->y == atker->y) {
    	dir = random_dir();
    } else {
    	dir = approx_dir(unit->x - atker->x, unit->y - atker->y);
    }
    if (retreat_in_dir(unit, dir))
      return TRUE;
    if (flip_coin()) {
    	if (retreat_in_dir(unit, left_dir(dir)))
    	  return TRUE;
    	if (retreat_in_dir(unit, right_dir(dir)))
    	  return TRUE;
    } else {
    	if (retreat_in_dir(unit, right_dir(dir)))
    	  return TRUE;
    	if (retreat_in_dir(unit, left_dir(dir)))
    	  return TRUE;
    }
    retreating_from = NONUTYPE;
    return FALSE;
}

/* Try to beat a retreat in the given direction. */

static int
retreat_in_dir(Unit *unit, int dir)
{
    int nx, ny, rslt;
    Unit *other;
    extern int retreating;
    extern int retreating_from;

    /* (should it be possible for a unit to retreat out of the world?) */
    if (!interior_point_in_dir(unit->x, unit->y, dir, &nx, &ny))
      return FALSE;
    retreating = TRUE;
    rslt = check_move_action(unit, unit, nx, ny, unit->z);
    if (valid(rslt)) {
	unit->act->acp += uu_acp_retreat(unit->type, retreating_from);
	do_move_action(unit, unit, nx, ny, unit->z);
	retreating = FALSE;
	retreating_from = NONUTYPE;
	return TRUE;
    }
    /* No luck moving; see if there's a friendly unit to enter. */
    for_all_stack(nx, ny, other) {
	rslt = check_enter_action(unit, unit, other);
	if (valid(rslt)) {
	    unit->act->acp += uu_acp_retreat(unit->type, retreating_from);
	    do_enter_action(unit, unit, other);
	    retreating = FALSE;
	    retreating_from = NONUTYPE;
	    return TRUE;
	}
    }
    return FALSE;
}

/* Capture action. */

/* Prepare a capture action to be executed later. */

int
prep_capture_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_CAPTURE;
    unit->act->nextaction.args[0] = unit3->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* Execute a capture action. */

int
do_capture_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    int rslt;

    attempt_to_capture_unit(unit2, unit3);
    use_up_acp(unit, uu_acp_to_capture(unit2->type, unit3->type));
    if (unit3->side == unit2->side)
      rslt = A_CAPTURE_SUCCEEDED;
    else
      rslt = A_CAPTURE_FAILED;
    return rslt;
}

/* Check the validity of a capture action. */

int
check_capture_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    int rslt = A_ANY_OK;

    /* In combat model 1, we can't capture units directly. */
    if (g_combat_model() == 1)
      return A_ANY_ERROR;
    if (!in_play(unit3))
      return A_ANY_ERROR;
    if (!valid(rslt = can_capture(unit, unit2, unit3->type, unit3->side)))
      return rslt;
    /* We can't capture ourselves. */
    if (unit2 == unit3)
      return A_ANY_ERROR;
    if (distance(unit2->x, unit2->y, unit3->x, unit3->y) > 1)
      return A_ANY_TOO_FAR;
    return A_ANY_OK;
}

/* Handle capture possibility and repulse/slaughter. */

/* The chance to capture an enemy is modified by several factors.
   Neutrals have a different chance to be captured, and presence of
   occupants should also has an effect.  Can't capture anything that is
   on a kind of terrain that the capturer can't go on, unless victim has
   "bridge effect". */

/* (Need a little better treatment of committed assaults, where lack of
   success == death.) */

static void
attempt_to_capture_unit(Unit *atker, Unit *other)
{
    int a = atker->type, o = other->type, chance, prot;
    int ox = other->x, oy = other->y;
    Unit *occ, *unit2;
    Side *as = atker->side, *os = other->side;
    
    /* Low-tech sides are not going to succeed at even touching high-tech
       types, let alone capturing them. */
    if (atker->side
	&& atker->side->tech[o] < u_tech_to_own(o))
      return;
    chance = capture_chance(a, o, other->side);
    if (alive(atker) && alive(other) && chance > 0) {
	if (impassable(atker, ox, oy) && !uu_bridge(o, a))
	  return;
	/* Can possibly detonate on *any* attempt to capture! */
	if (probability(uu_detonate_on_capture(o, a))
	    && !was_detonated(other)) {
	    detonate_unit(other, other->x, other->y, other->z);
	    /* Might not be possible to capture anything anymore. */
	    if (!alive(atker) || !alive(other))
	      return;
	    /* Types of units might have changed, recalc things. */
	    a = atker->type;  o = other->type;
	    as = atker->side;  os = other->side;
	    chance = capture_chance(a, o, other->side);
	}
	/* Occupants can protect the transport. */
	for_all_occupants(other, occ) {
	    if (is_active(occ)) {
		if (g_prot_resists_capture()) {
	    	    prot = uu_protection(occ->type, o);
		    if (prot != 100)
		      chance = (chance * prot) / 100;
		}
		prot = uu_occ_allows_capture_by(occ->type, a);
	    	if (prot != 100)
		  chance = (chance * prot) / 100;
		prot = uu_occ_allows_capture_of(occ->type, o);
	    	if (prot != 100)
		  chance = (chance * prot) / 100;
	    }
	}
	/* Neighbors can protect neighbors. */
	for_all_stack(other->x, other->y, unit2) {
	    if (unit2 != other
		&& in_play(unit2)
		&& completed(unit2)
		&& unit2->side == other->side) {
		if (g_prot_resists_capture()) {
		    prot = uu_stack_protection(unit2->type, o);
		    if (prot != 100)
		      chance = (chance * prot) / 100;
		}
		prot = uu_stack_neighbor_allows_capture_by(unit2->type, a);
		if (prot != 100)
		  chance = (chance * prot) / 100;
		prot = uu_stack_neighbor_allows_capture_of(unit2->type, o);
		if (prot != 100)
		  chance = (chance * prot) / 100;
	    }
	}
	/* Note that this code differs from that above in that it
	   treats all units in the same cell equally, whether occs,
	   suboccs or part of the stack. This also means that one occ
	   can protect another occ in the same transport. Moreover,
	   the protective unit also protects itself. These rules
	   simulate real situations such as when triple-A protects all
	   nearby units (including itself) against bombers, or when a
	   city wall protects all other occupants against ground
	   attack. */
	for_all_stack_with_occs(other->x, other->y, unit2) {
	    if (is_active(unit2)
		/* We also extend protection to our buddies! */
		&& trusted_side(unit2->side, other->side)) {
		/* This is when a unit, such as triple-A, provides
		   unique protection against a specific attacker, such
		   as bombers, to all other units in the cell. */
		if (g_prot_resists_capture()) {
		    prot = uu_cellwide_protection_against(unit2->type, a);
		    if (prot != 100)
		      chance = (chance * prot) / 100;
		}
		prot = uu_any_neighbor_allows_capture_by(unit2->type, a);
		if (prot != 100)
		  chance = (chance * prot) / 100;
		/* This is when a unit (such as a garrison)
		   specifically protects a second unit, such as a fort
		   (but not other nearby units), against all forms of
		   attack. It thus works the same way as uu_protection
		   and uu_stack_protection. */
		if (g_prot_resists_capture()) {
		    prot = uu_cellwide_protection_for(unit2->type, o);
		    if (prot != 100)
		      chance = (chance * prot) / 100;
		}
		prot = uu_any_neighbor_allows_capture_of(unit2->type, o);
		if (prot != 100)
		  chance = (chance * prot) / 100;
	    }
	}
	/* Test whether the capture actually happens. */
	if (probability(chance)) {
	    capture_unit(atker, other, H_UNIT_CAPTURED);
	} else if (atker->transport != NULL && 
		   (impassable(atker, ox, oy) ||
		    impassable(atker, atker->x, atker->y))) {
	    /* was the capture attempt a one-way trip? */
	    /* (should fix the test above - needs to be more accurate) */
	    report_combat(atker, other, "resist/slaughter");
	    kill_unit(atker, 
		      H_UNIT_KILLED /* should be something appropriate */);
	} else {
	    report_combat(atker, other, "resist");
	    /* (should record failed attempt to capture?) */
	}
	if (chance > 0) {
	    if (atker->cxp < u_cxp_max(a))
	      atker->cxp += uu_cxp_per_capture(a, o);
	    /* (should not increment if side just changed?) */
	    if (other->cxp < u_cxp_max(o))
	      other->cxp += uu_cxp_per_capture(o, a);

	}
    }
}

/* There are many consequences of a unit being captured. */

void
capture_unit(Unit *unit, Unit *pris, int captype)
{
    int u = unit->type, px = pris->x, py = pris->y;
    Unit *occ;
    Side *ps = pris->side, *us = unit->side, *newside;
    int newtype = NONUTYPE;
    int m = NONMTYPE;
    int gain = 0, loss = 0;

    newside = unit->side;
    /* Return a unit to its original side if we are buds with that side. */
    if (pris->origside != newside && trusted_side(us, pris->origside))
      newside = pris->origside;
    /* Attempt to scuttle. */
    if (probability(uu_scuttle(pris->type, u))) {
	/* (should add terrain effect on success too) */
	/* (should characterize as a scuttle) */
	kill_unit(pris, H_UNIT_DISBANDED);
    }
    /* Attempt to change type if we are supposed to do so on capture. */
    /* If new type cannot be on new side, then the unit must die. */
    newtype = uu_changed_type_if_captured(pris->type, u);
    if (NONUTYPE != newtype) {
	if (!type_allowed_on_side(newtype, newside))
	  kill_unit(pris, H_UNIT_KILLED);
	else
	  change_unit_type(pris, newtype, H_UNIT_TYPE_CHANGED, newside);
    }
    /* If prisoner is not allowed on side, then make it go away. */
    if (!unit_allowed_on_side(pris, newside)) {
	kill_unit(pris, H_UNIT_KILLED);
    }
    /* Properly report capture outcome. */
    if (alive(pris)) {
	if (newside == pris->origside) {
	    report_combat(unit, pris, "liberate");
	} else {
	    report_combat(unit, pris, "capture");
	}
	/* Decide the fate of each occupant of our prisoner. */
	for_all_occupants(pris, occ) {
	    /* Don't try to capture occs from trusted sides. */
	    if (trusted_side(us, occ->side))
	      continue;
	    capture_occupant(unit, pris, occ, newside);
	}
	/* Treasury bonus and penalty due to unit capture */
	for_all_material_types(m) {                
	    if (side_has_treasury(unit->side, m)) {
		gain = um_treasury_gain_per_capture(unit->type, m);
		unit->side->treasury[m] += gain;
		/* Clip treasury to bounds, if necessary. */
		unit->side->treasury[m] = 
		    min(unit->side->treasury[m], g_treasury_size());
		unit->side->treasury[m] = max(unit->side->treasury[m], 0);
	    }
	    if (side_has_treasury(pris->side, m)) {
		loss = um_treasury_loss_per_captured(pris->type, m);
		pris->side->treasury[m] -= loss;
		/* Clip treasury to bounds, if necessary. */
		pris->side->treasury[m] = 
		    min(pris->side->treasury[m], g_treasury_size());
		pris->side->treasury[m] = max(pris->side->treasury[m], 0);
	    }
	}      
	/* The change of side itself.  This happens recursively to any
	   remaining occupants as well. */
	change_unit_side(pris, newside, captype, unit);
	/* Garrison the newly-captured unit with hp from the capturing unit. */
	garrison_unit(unit, pris);
        update_unit_display(unit->side, unit, TRUE);
	capture_unit_2(unit, pris, ps);
	/* The people at the new location may change sides immediately. */
	if (people_sides_defined()
	    && any_people_side_changes
	    && probability(people_surrender_chance(pris->type, px, py))) {
	    change_people_side_around(px, py, pris->type, unit->side);
	}
	if (control_sides_defined()) {
	    if (ut_control_range(pris->type, terrain_at(px, py)) >= 0) {
		change_control_side_around(px, py, pris->type, unit->side);
	    }
	}
         kick_out_enemy_users(unit->side, px, py);
	/* Occupy the captured unit if possible. */
	if(valid(check_enter_action(unit, unit, pris))) {
	    do_enter_action(unit, unit, pris);
	/* Else move into the same cell to guard it. */
    	} else if (valid(check_move_action(unit, unit, px, py, 0))) {
	    do_move_action(unit, unit, px, py, 0);
	}
    }

    /* Update everybody's view of the situation. */
    see_exact(ps, px, py);
    update_cell_display(ps, px, py, UPDATE_ALWAYS);
    all_see_cell(px, py);
}

/* Given that the main unit is going to be captured, decide what each occupant
   will do. */

static void
capture_occupant(Unit *unit, Unit *pris, Unit *occ, Side *newside)
{
    int u = unit->type, newtype = NONUTYPE;
    Unit *subocc;

    /* Side change will actually happen later. */
    if (probability(uu_occ_escape(u, occ->type))) {
	/* The occupant escapes, along with all its suboccupants. */
	/* Retreat is not a perfect model, but close enough for now. */
	if (retreat_unit(occ, unit)) {
		/* Only report an escape if successful. */
		report_combat(unit, occ, "escape");
    	}
    } else if (probability(uu_scuttle(occ->type, u))) {
	/* (should add terrain effect on success too) */
	/* (should characterize as a scuttle) */
	kill_unit(occ, H_UNIT_DISBANDED);
    /* Some occs may gracefully switch to their transport's new side. */
    } else if (u_match_transport_side(occ->type)) {
	newtype = uu_changed_type_if_captured(occ->type, u);
	if (NONUTYPE != newtype) {
	    if (type_allowed_on_side(newtype, newside))
	      change_unit_type(occ, newtype, H_UNIT_TYPE_CHANGED, newside);
	}
	for_all_occupants(occ, subocc) {
	    capture_occupant(unit, occ, subocc, newside);
	}
	if ((NONUTYPE != newtype) && !type_allowed_on_side(newtype, newside))
	  kill_unit(pris, H_UNIT_KILLED);
    /* Other occs may need to be forcefully captured. */
    } else if (capture_chance(u, occ->type, occ->side) > 0) {
	/* (TODO: Should use up ACP during capture of occs.) */
	for_all_occupants(occ, subocc) {
	    capture_occupant(unit, occ, subocc, newside);
	}
    } else {
	/* Occupant can't live as a prisoner, but suboccs might, so recurse
	   through them. */
	for_all_occupants(occ, subocc) {
	    capture_occupant(unit, occ, subocc, newside);
	}
	/* Any suboccupants that didn't escape will die. */
	/* (what if subocc captured tho? should move elsewhere) */
	kill_unit(occ, H_UNIT_KILLED);
    }
}

/* Do additional consequences of capture. */

static void
capture_unit_2(Unit *unit, Unit *pris, Side *prevside)
{
    int chance, x, y, notesee;
    Unit *occ, *unit2;
    Side *newside;
    Unit *checkalways;

    /* Our new unit's experience might be higher or lower, depending on what
       capture really means (change of crew perhaps). */
    pris->cxp = (pris->cxp * u_cxp_on_capture(pris->type)) / 100;
    pris->cxp = min(unit->cxp, u_cxp_max(pris->type));
    /* Getting captured is always bad for morale, but getting
       liberated is good. */
    if (pris->side == pris->origside)
      change_morale(pris, 1, ((pris->morale + 1) * 50) + 50);
    else
      pris->morale = pris->morale / 2;
    /* Clear any actions and plans. */
    /* Don't use init_unit_actorstate!  We just want to cancel any pending
       action, but leave all the acp values untouched. */
    if (pris->act)
      pris->act->nextaction.type = ACTION_NONE;
    /* (should probably adjust side's acp sums by new unit's amounts) */
    init_unit_plan(pris);
    /* Maybe get a pile of interesting information. */
    /* Independent units won't know anything about other independents
       though (they're independent, eh?). */
    newside = unit->side;
    if (newside && !newside->see_all && prevside != indepside) {
	notesee = FALSE;
	chance = u_see_terrain_captured(pris->type);
	if (!g_terrain_seen() && probability(chance)) {
	    for_all_cells(x, y) {
		if (terrain_view(prevside, x, y) != UNSEEN
		    && terrain_view(newside, x, y) == UNSEEN) {
		    set_terrain_view(newside, x, y,
				     terrain_view(prevside, x, y));

		    /* Now view see-always units. */
		    for_all_stack_with_occs(x, y, checkalways) {
			if (u_see_always(checkalways->type)) {
			    see_exact(newside, x, y);
			    add_cover(newside, x, y, 1);
			}
		    }

		    update_cell_display(newside, x, y, UPDATE_ALWAYS);
		    notesee = TRUE;
		}
	    }
	}
	if (1 /* any see others chance for this type */) {
	    for_all_side_units(prevside, unit2) {
		if (in_play(unit2)) {
		    chance = uu_see_others_captured(pris->type, unit2->type);
		    if (probability(chance)) {
			x = unit2->x; y = unit2->y;
			see_exact(newside, unit2->x, unit2->y);
			update_cell_display(newside, unit2->x, unit2->y,
					    UPDATE_ALWAYS);
			for_all_stack_with_occs(x, y, checkalways) {
			    if (u_see_always(checkalways->type)) {
				add_cover(newside, x, y, 1);
			    }
			}
			notesee = TRUE;
		    }
		}
	    }
	}
	/* Only say something to the capturing side if we actually got
	   any new information - after several captures, may not be
	   anything new to find out. */
	if (notesee) {
	    notify(newside, "Enemy information fell into your hands!");
	}
    }
    /* Likewise for occupants. */
    for_all_occupants(pris, occ) {
	capture_unit_2(unit, occ, prevside);
    }
}

/* A detonate action just blasts the vicinity indiscriminately. */

int
prep_detonate_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_DETONATE;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_detonate_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int u2 = unit2->type;

    detonate_unit(unit2, x, y, z);
    /* Note that if the maxrange is further than the actual range of this
       detonation, only just-damaged units will be looked at. */
    reckon_damage_around(x, y, detonate_urange_max(), unit2);
    /* Unit might have detonated outside its range of effect, so need
       this to make its own damage is accounted for. */
    if (alive(unit2))
      reckon_damage_around(unit2->x, unit2->y, 0, unit2);
    use_up_acp(unit, u_acp_to_detonate(u2));
    return A_ANY_DONE;
}

int
check_detonate_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int u, u2;
    int rslt = A_ANY_OK;

    if (!valid(rslt = can_detonate(unit, unit2)))
      return rslt;
    if (!inside_area(x, y))
      return A_ANY_ERROR;
    u = unit->type;
    u2 = unit2->type;
    /* Can only detonate in our own or an adjacent cell. */
    /* (In other words, the detonating unit doesn't get to teleport
       its detonation effects to any desired faraway location.) */
    if (distance(unit2->x, unit2->y, x, y) > 1)
      return A_ANY_TOO_FAR;
    return A_ANY_OK;
}

static int tmpdetx, tmpdety;

/* Actual detonation may occur by explicit action or automatically; this
   routine makes the detonation effects happen, pyrotechnics and all. */

int
detonate_unit(Unit *unit, int x, int y, int z)
{
    int u = unit->type, dir, x1, y1, dmg, maxrange;
    Unit *unit2;
    Side *side;

    if (max_t_detonate_effect_range < 0) {
	int u2, t, range;

	for_all_unit_types(u2) {
	    for_all_terrain_types(t) {
		range = ut_detonation_range(u2, t);
		max_t_detonate_effect_range =
		  max(range, max_t_detonate_effect_range);
	    }
	}
    }
    report_combat(unit, NULL, "detonate");
    for_all_sides(side) {
    	if (active_display(side)
	    && (side_sees_unit(side, unit)
		|| units_visible(side, unit->x, unit->y))) {
	    schedule_movie(side, "flash", unit->x, unit->y);
	    /* a hack */
	    if (strstr(u_type_name(u), "nuclear")
		|| strstr(u_type_name(u), "nuke"))
	      schedule_movie(side, "nuke", x, y);
    	}
    }
    set_was_detonated(unit, TRUE);
    /* Hit the detonating unit first. */
    hit_unit_with_detonation(unit, u_hp_per_detonation(u), NULL);
    /* Hit units at ground zero. */
    for_all_stack(x, y, unit2) {
    	if (unit2 != unit) {
	    hit_unit_with_detonation(
		unit2, 
		max(0, roll_dice1(uu_detonation_damage_at(u, unit2->type))), 
		unit);
	}
    }
    damage_terrain(u, x, y);
    /* Hit units and/or terrain in adjacent cells, if this is defined. */
    if (detonate_urange_max() >= 1) {
        for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		for_all_stack(x1, y1, unit2) {
		    if (0 >= uu_detonation_range(u, unit2->type))
		      continue;
		    dmg = 
			max(0, 
			    roll_dice1(
				uu_detonation_damage_adj(u, unit2->type)));
		    hit_unit_with_detonation(unit2, dmg, unit);
		}
	    }
	}
    }
    if (max_t_detonate_effect_range >= 1) {
        for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		if (0 >= ut_detonation_range(u, terrain_at(x1, y1)))
		  continue;
		damage_terrain(u, x1, y1);
	    }
	}
    }
    /* Hit units that are further away. */
    maxrange = max(detonate_urange_max(), max_t_detonate_effect_range);
    if (maxrange >= 2) {
	tmpunit = unit;
	tmpdetx = x;  tmpdety = y;
	apply_to_area(x, y, maxrange, detonate_on_cell);
    }
    /* (should test compatibility of any new terrain types with each other;
        only after changes over with) */
    /* Entertain everybody. */
    play_movies(ALLSIDES);
    return TRUE;
}

static void
detonate_on_cell(int x, int y)
{
    int dist, dmg, sdmg;
    Unit *unit2;

    dist = distance(tmpdetx, tmpdety, x, y);
    if (dist > 1 && dist <= detonate_urange_max()) {
	/* Since this code bypasses the occ recursion in
	   maybe_hit_unit, we should also hit all occupants directly
	   with the blast. */
	for_all_stack_with_occs(x, y, unit2) {
	    if (dist <= uu_detonation_range(tmpunit->type, unit2->type)) {
		dmg = 
		    max(0, 
			roll_dice1(
			    uu_detonation_damage_adj(tmpunit->type, 
						     unit2->type)));
		/* Reduce by inverse square of the distance. */
		sdmg = (dmg * 100) / (dist * dist);
		dmg = prob_fraction(sdmg);
		hit_unit_with_detonation(unit2, dmg, tmpunit);
	    }
	}
    }
    if (dist > 1 
	&& dist <= ut_detonation_range(tmpunit->type, terrain_at(x, y))) {
	damage_terrain(tmpunit->type, x, y);
    }
}

static void
hit_unit_with_detonation(Unit *unit, int hit, Unit *atker)
{
    char *hitmovietype;
    Side *side;

    hit_unit(unit, hit, atker);
    for_all_sides(side) {
    	if (active_display(side)
	    /* (should figure out visibility rules) */
	    && (g_see_all()
	        || side == unit->side
	        || 0 /* visible to other sides */)) {
	    hitmovietype = (char *)((hit >= unit->hp) ? "death" :
			    ((hit > 0) ? "hit" : "miss"));
	    schedule_movie(side, hitmovietype, unit->id);
    	}
    }
}

/* Do the effect of detonation on the terrain at the given location. */

void
damage_terrain(int u, int x, int y)
{
    int t, t2, dir;

    /* Damage the cell's terrain. */
    t = terrain_at(x, y);
    if (probability(ut_detonation_damage(u, t))) {
	t2 = damaged_terrain_type(t, u);
	if (t2 == NONTTYPE) {
	    run_warning(
"Possibly invalid damaged terrain type from undamaged terrain type '%s'.", 
			t_type_name(t));
	    return;
	} else if (t2 != t) {
	    change_terrain_type(x, y, t2);
	}
    }
    /* Apply to auxiliary terrain also. */
    if (1 /* if any aux terrain */) {
	for_all_terrain_types(t) {
	    switch (t_subtype(t)) {
	      case cellsubtype:
		/* We already did this one. */
	        break;
	      case bordersubtype:
		if (1 /* any damage possible */) {
		    for_all_directions(dir) {
			if (border_at(x, y, dir, t)
			    && probability(ut_detonation_damage(u, t))) {
			    t2 = damaged_terrain_type(t, u);
			    if (t2 == NONTTYPE) {
				set_border_at(x, y, dir, t, FALSE);
			    } else if (t2 != t) {
				set_border_at(x, y, dir, t, FALSE);
				set_border_at(x, y, dir, t2, TRUE);
				/* There is potentially a problem with
				   some game designs here; if the new
				   type t2 can also be damaged by the
				   detonation, then the loop here will
				   damage it in turn, at least if t2
				   *follows* t in the list of terrain
				   types.  Preventing this would
				   require a lot of buffering, so it's
				   left as a limitation for now. */
			    }
			}
		    }
		}
	        break;
	      case connectionsubtype:
		if (1 /* any damage possible */) {
		    for_all_directions(dir) {
			if (connection_at(x, y, dir, t)
			    && probability(ut_detonation_damage(u, t))) {
			    t2 = damaged_terrain_type(t, u);
			    if (t2 == NONTTYPE) {
				set_connection_at(x, y, dir, t, FALSE);
			    } else if (t2 != t) {
				set_connection_at(x, y, dir, t, FALSE);
				set_connection_at(x, y, dir, t2, TRUE);
				/* Same issue here as with border
                                   damage. */
			    }
			}
		    }
		}
	        break;
	      case coatingsubtype:
		/* don't know how to damage coatings yet */
	        break;
	    }
	}
    }
}

int
damaged_terrain_type(int t, int u)
{
    int t2, tot, othertot, test, sum, rslt;

    tot = othertot = 0;
    for_all_terrain_types(t2) {
	if (!ut_blasts_into_ttype(u, t2))
	    continue;
	if (t_subtype(t2) == t_subtype(t)) {
	    tot += tt_damaged_type(t, t2);
	} else if (t_subtype(t) != cellsubtype) {
	    othertot += tt_damaged_type(t, t2);
	}
    }
    rslt = NONTTYPE;
    if ((tot + othertot) > 0) {
    	test = xrandom(tot + othertot);
    	sum = 0;
    	for_all_terrain_types(t2) {
	    if (!ut_blasts_into_ttype(u, t2))
		continue;
	    if (t_subtype(t2) == t_subtype(t)) {
	    	sum += tt_damaged_type(t, t2);
		if (test < sum) {
		    rslt = t2;
		    break;
		}
	    }
    	}
    }
    /* Random values between tot and othertot will have
       fallen through the loop, and the rslt is NONTTYPE,
       which indicates that the terrain must be removed
       if possible. */
    /* Paranoia check */
    if (rslt != NONTTYPE && t_subtype(rslt) != t_subtype(t))
      run_error("badness in damaged_terrain_type");
    return rslt;
}

/* True if unit has enough occupants assigned to its defense. */

int
defended_by_occupants(Unit *unit)
{
    Unit *unit2;
    int defenders = 0;

    for_all_occupants(unit, unit2) {
	if (is_active(unit2)
	    && unit2->plan
	    && unit2->plan->maingoal
	    && unit2->plan->maingoal->type == GOAL_UNIT_OCCUPIED
	    && unit2->plan->maingoal->args[0] == unit->id
	    /* Occs that cannot defend the transport should not be
	       assigned to occupy it, but check anyway for ability to
	       protect. */
	    && occ_can_defend_transport(unit2->type, unit->type)) {
	    ++defenders;
	}
    }
    if (enough_to_garrison(unit, defenders))
      return TRUE;
    else 
      return FALSE;
}

/* True if first type can both occupy and protect second type. */

int
occ_can_defend_transport(int o, int t)
{
	/* Never true if transport cannot carry occupant. */
	if (!could_carry(t, o))
	    return FALSE;
	if (g_combat_model() == 0) {
		/* Specific protection of transport by its occupant.
		Note: zero protection is 100, full protection is 0! */
		if (uu_protection(o, t) < 100)
		    return TRUE;
		/* Cellwide protection of all units by occupant. */
		if (uu_cellwide_protection_for(o, t) < 100)
		    return TRUE;
	} else if (g_combat_model() == 1) {
		/* Advanced units are protected by any kind of occupant
		that has a positive defense value. */
		 if (u_advanced(t) && u_defend(o) > 0) {
			return TRUE;
		/* Non-advanced units are protected by this table. */ 
		} else if (!u_advanced(t)
			    /*	Note: More than 100 is protection. */
			     && uu_occ_affects_defense(o, t) > 100) {
			return TRUE;
		}
	}
	return FALSE;	
}

/* True if the specified number of defenders is sufficient to defend
   the unit. Note that this garrison does not have to exist! */

int
enough_to_garrison(Unit *unit, int defenders)
{
    int u = unit->type;
    Unit *unit2;
	
    /* We only worry about enemies within our tactical range. */	
    unit2 = mobile_enemy_threat(unit, u_ai_enemy_alert_range(u));
    /* We are threatened by the enemy! */
    if (unit2 && defenders < u_ai_war_garrison(u))
      return FALSE;
    else if (defenders < u_ai_peace_garrison(u))
      /* No enemy in sight. */ 
      return FALSE;
    else
      return TRUE;
}

/* Returns the first mobile enemy unit found within the specified
   range that can hit or capture our unit. */

Unit *
mobile_enemy_threat(Unit *unit, int range)
{
    int x, y;
    Unit *unit2;
	
    /* Go through all cells within the specified range. */
    for_all_cells_within_range(unit->x, unit->y, range, x, y) {
	if (!inside_area(x, y))
	  continue;
	if (!terrain_visible(unit->side, x, y))
	  continue;
	/* Important to count also occupants here. */
	for_all_stack_with_occs(x, y, unit2) {
	    /* Only count visible mobile enemy units that actually
	       threaten us. */
	    if (is_active(unit2)
		/* The NULL side rarely attacks anybody :-) */
		&& unit2->side
		&& mobile(unit2->type)
		&& side_sees_image(unit->side, unit2)
		&& !trusted_side(unit->side, unit2->side)
		&& (could_hit(unit2->type, unit->type)
		    || capture_chance(unit2->type, unit->type, unit->side) > 0)) {
		return unit2;
	    }
	}
    }
    return NULL;
}


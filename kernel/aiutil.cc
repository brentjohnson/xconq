/* Functions common to all AIs.
   Copyright (C) 2004 Eric A. McDonald

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file contains code used by all types of AIs and intelligent 
   UIs. While AI code is officially outside the kernel, the functions 
   here we need some definitions from kernel.h and so must include it. */

/* The functions contained herein should contain simple calculations 
   which produce strictly objective, factual results, but are not used 
   at the referee level. Functions that rank, score, or apply 
   probability do not belong here. */

#include "conq.h"
#include "kpublic.h"
#include "aiutil.h"
#include "aiunit.h"

using namespace Xconq;

/* Could u destroy u2 by attacking or firing? */
int
could_destroy(int u, int u2)
{
    return (could_damage(u, u2) && !uu_hp_min(u, u2));
}

/* What is the maximum damage that u can do to u2 by attacking? */
int
type_attack_damage_max(int u, int u2)
{
    if (!could_damage_by_attacks(u, u2))
      return 0;
    return dice1_roll_max(uu_damage(u, u2));
}

/* What is the minimum damage that u can do to u2 by attacking? */
int
type_attack_damage_min(int u, int u2)
{
    if (!could_damage_by_attacks(u, u2))
      return 0;
    return dice1_roll_min(uu_damage(u, u2));
}

/* What is the maximum damage that u can do to u2 by firing? */
int
type_fire_damage_max(int u, int u2)
{
    if (!could_damage_by_fire(u, u2))
      return 0;
    return dice1_roll_max(fire_damage(u, u2));
}

/* What is the minimum damage that u can do to u2 by firing? */
int
type_fire_damage_min(int u, int u2)
{
    if (!could_damage_by_fire(u, u2))
      return 0;
    return dice1_roll_min(fire_damage(u, u2));
}

/* What is the maximum damage that u can do to u2 by attacking or 
   firing? */
int
type_damage_max(int u, int u2)
{
    return max(type_attack_damage_max(u, u2), 
               type_fire_damage_max(u, u2));
}

/* What is the maximum striking range of u versus u2 when attacking or 
   firing? */
int
type_hit_range_max(int u, int u2)
{
    return max(attack_range(u, u2), fire_range(u, u2));
}

/* What is the ideal maximum striking range of u versus u2 when attacking? */
int
type_ideal_attack_range_max(int u, int u2)
{
    return attack_range(u, u2);
}

/* What is the ideal maximum striking range of u versus u2 when firing? */
int
type_ideal_fire_range_max(int u, int u2)
{
    if (g_combat_model() == 0) {
        if (u_hit_falloff_range(u) < u_range(u)) {
            if (u_hit_falloff_range(u) >= u_range_min(u))
              return u_hit_falloff_range(u);
            else
              return u_range_min(u);
        }
        else
          return u_range(u);
    }
    return 0; 
}

/* What is the ideal maximum striking range of u versus u2 when attacking or 
   firing? */
int
type_ideal_hit_range_max(int u, int u2)
{
    return max(type_ideal_attack_range_max(u, u2), 
               type_ideal_fire_range_max(u, u2));
}

/* What kind of hits can u do on u2? */
int
type_possible_damage_methods(int u, int u2)
{
    int dmgtypes = DAMAGE_TYPE_NONE;

    if (could_damage_by_attacks(u, u2))
      dmgtypes |= DAMAGE_TYPE_ATTACK;
    if (could_damage_by_fire(u, u2))
      dmgtypes |= DAMAGE_TYPE_FIRE;
    /* (Consider other ways for u to damage u2.) */
    return dmgtypes;
}

/* What is the occupancy status of a given unit view? */
OccStatus
occ_status(UnitView *uview)
{
    UnitView *uvocc = NULL;
    int u2 = uview->type;

    if (!type_can_have_occs(u2))
      return CANNOT_HAVE_OCCS;
    if (g_see_all() || u_see_occupants(u2)) {
        for_all_occupant_views(uview, uvocc) {
            if (uvocc)
              return DEFINITELY_HAS_OCCS;
        }
        if (!uvocc) {
            if (g_see_all())
              return DEFINITELY_HAS_NO_OCCS;
            else
              return MAYBE_HAS_OCCS;
        }
    }
    return MAYBE_HAS_OCCS;
}

/* Is the unit planning to hit a unit of the given type at the given 
   position? */
int
planning_to_hit_type_at(Unit *unit, int u, int x, int y)
{
    Task *task = NULL;
    Unit *unit2 = NULL;

    if (in_play(unit) && unit->plan) {
        for_all_tasks(unit->plan, task) {
	    if (TASK_HIT_UNIT != task->type)
		continue;
	    unit2 = find_unit(task->args[0]);
	    if (!in_play(unit2))
		continue;
            if ((unit2->x == x) && (unit2->y == y) && (unit2->type == u))
              return TRUE;
        }
    }
    return FALSE;
}

/* Is the unit planning to capture a unit of the given type at the given 
   position? */
int
planning_to_capture_type_at(Unit *unit, int u, int x, int y)
{
    Task *task = NULL;
    Unit *unit2 = NULL;

    if (in_play(unit) && unit->plan) {
        for_all_tasks(unit->plan, task) {
            if ((task->type == TASK_HIT_UNIT) || (task->type == TASK_CAPTURE)) {
		unit2 = find_unit(task->args[0]);
		if (!in_play(unit2))
		    continue;
		if ((unit2->x == x) && (unit2->y == y)
		    && (unit2->type == u)
		    && (uu_capture(unit->type, u) > 0))
		    return TRUE;
	    }
        }
    }
    return FALSE;
}

/* Is the side planning to hit a unit of the given type at the given 
   position? */
int
side_planning_to_hit_type_at(Side *side, int u, int x, int y)
{
    Unit *unit = NULL;

    for_all_side_units(side, unit) {
        if (planning_to_hit_type_at(unit, u, x, y))
          return TRUE;
    }
    return FALSE;
}

/* How many side units are planning to hit a unit of the given type at 
   the given position? */
int
n_planning_to_hit_type_at(Side *side, int u, int x, int y)
{
    Unit *unit = NULL;
    int counter = 0;

    for_all_side_units(side, unit) {
        if (planning_to_hit_type_at(unit, u, x, y))
          ++counter;
    }
    return counter;
}

/* Is the side planning to capture a unit of the given type at the given 
   position? */
int
side_planning_to_capture_type_at(Side *side, int u, int x, int y)
{
    Unit *unit = NULL;

    for_all_side_units(side, unit) {
        if (planning_to_capture_type_at(unit, u, x, y))
          return TRUE;
    }
    return FALSE;
}

/* How many side units are planning to capture a unit of the given type at 
   the given position? */
int
n_planning_to_capture_type_at(Side *side, int u, int x, int y)
{
    Unit *unit = NULL;
    int counter = 0;

    for_all_side_units(side, unit) {
        if (planning_to_capture_type_at(unit, u, x, y))
          ++counter;
    }
    return counter;
}

/* Is a given unit satisfied that it has enough of a given material? */

int
has_enough_of_material(Unit *unit, int m)
{
    int u = NONUTYPE;

    if (NONMTYPE == m)
      return TRUE;
    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          FALSE);
    if (!(unit->supply))
      return TRUE;
    u = unit->type;
    if (um_storage_x(u, m) == 0)
      return TRUE;
    if (unit->supply[m] > 
        ((um_storage_x(u, m) * unit_doctrine(unit)->resupply_percent) / 100))
      return TRUE;
    return FALSE;
}

/* How much of a given material is needed to satisfy a given unit? */

int
needs_n_of_material(Unit *unit, int m)
{
    int u = NONUTYPE;

    if (has_enough_of_material(unit, m))
      return 0;
    assert_warning_return(unit, "Unexpected null pointer encountered.", 0);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          0);
    u = unit->type;
    return (((um_storage_x(u, m) * unit_doctrine(unit)->resupply_percent) 
             / 100) - unit->supply[m]);
}

/* Does the given unit have the full amount of a given material? */

int
has_full_amount_of_material(Unit *unit, int m)
{
    int u = NONUTYPE;

    if (NONMTYPE == m)
      return TRUE;
    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          FALSE);
    if (!(unit->supply))
      return TRUE;
    u = unit->type;
    return (unit->supply[m] == um_storage_x(u, m));
}

/* How much of a given material is needed to fill a given unit? */

int
wants_n_of_material(Unit *unit, int m)
{
    int u = NONUTYPE;

    if (has_full_amount_of_material(unit, m))
      return 0;
    assert_warning_return(unit, "Unexpected null pointer encountered.", 0);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          0);
    u = unit->type;
    return (um_storage_x(u, m) - unit->supply[m]);
}

/* Would a given unit type starve without a given material? */

int
would_starve_without_material(int u, int m)
{
    if (m == NONMTYPE)
      return FALSE;
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          FALSE);
    return (um_hp_per_starve(u, m) > 0);
}

/* Will a given unit starve unless it has some of a given material? */

int
will_starve_wrt_material(Unit *unit, int m)
{
    int u = NONUTYPE;

    if (m == NONMTYPE)
      return FALSE;
    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          FALSE);
    if (!(unit->supply))
      return FALSE;
    u = unit->type;
    if (wants_n_of_material(unit, m) >= unit->supply[m])
      return would_starve_without_material(u, m);
    return FALSE;
}

/* How much of a given material can a given unit readily provide? */

int
can_donate_n_of_material(Unit *unit, int m)
{
    int u = NONUTYPE, mavail = 0;

    if (m == NONMTYPE)
      return 0;
    assert_warning_return(unit, "Unexpected null pointer encountered.", 0);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          0);
    if (!(unit->supply))
      return 0;
    u = unit->type;
    if (!um_storage_x(u, m))
      return 0;
    mavail = unit->supply[m] - 
             ((um_storage_x(u, m) * unit_doctrine(unit)->resupply_percent) /
              100);
    if (mavail < 0)
      return 0;
    return mavail;
}

/* How much of a given material can a given unit provide and still be able 
   to survive without acting for one turn? */

int
can_sacrifice_n_of_material(Unit *unit, int m)
{
    int u = NONUTYPE, mavail = 0;

    if (m == NONMTYPE)
      return 0;
    assert_warning_return(unit, "Unexpected null pointer encountered.", 0);
    assert_warning_return(m < nummtypes, 
                          "Reference to illegal material type.", 
                          0);
    if (!(unit->supply))
      return 0;
    u = unit->type;
    if (!um_storage_x(u, m))
      return 0;
    if (!would_starve_without_material(u, m))
      return unit->supply[m];
    mavail = unit->supply[m] - 
             (um_base_consumption(u, m) - um_base_production(u, m));
    if (mavail < 0)
      return 0;
    return mavail;
}

/* Is a given unit satisfied that it has enough of all materials? */

int
has_enough_of_all_materials(Unit *unit)
{
    int m = NONMTYPE;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply))
      return TRUE;
    for_all_material_types(m) {
        if (!has_enough_of_material(unit, m))
          return FALSE;
    }
    return TRUE;
}

/* Does a given unit have a full amount of all materials? */

int
has_full_amount_of_all_materials(Unit *unit)
{
    int m = NONMTYPE;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply))
      return TRUE;
    for_all_material_types(m) {
        if (!has_full_amount_of_material(unit, m))
          return FALSE;
    }
    return TRUE;
}

/* Will a given unit starve unless it receives some material? */

int
will_starve(Unit *unit)
{
    int m = NONMTYPE;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply))
      return FALSE;
    for_all_material_types(m) {
        if (will_starve_wrt_material(unit, m))
          return TRUE;
    }
    return FALSE;
}

/* Which material is needed to prevent starvation? */

int
critically_needed_material(Unit *unit)
{
    int m = NONMTYPE;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply))
      return NONMTYPE;
    for_all_material_types(m) {
        if (will_starve_wrt_material(unit, m))
          return m;
    }
    return NONMTYPE;
}

/* Which material is most needed by the given unit? */

int
most_needed_material(Unit *unit, int *nmost)
{
    int m = NONMTYPE, mmost = NONMTYPE;
    int n = 0, nmost2 = 0;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply)) 
      return NONMTYPE;
    for_all_material_types(m) {
        if (will_starve_wrt_material(unit, m)) {
            mmost = m;
            nmost2 = um_storage_x(unit->type, m) - unit->supply[m];
            break;
        }
        n = needs_n_of_material(unit, m);
        if (n > nmost2) {
            nmost2 = n;
            mmost = m;
        } 
    }
    if (nmost)
      *nmost = nmost2;
    return mmost;
}

/* Which material is most wanted by the given unit? */

int
most_wanted_material(Unit *unit, int *nmost)
{
    int m = NONMTYPE, mmost = NONMTYPE;
    int n = 0, nmost2 = 0;

    assert_warning_return(unit, "Unexpected null pointer encountered.", FALSE);
    if (!(unit->supply)) 
      return NONMTYPE;
    for_all_material_types(m) {
        if (will_starve_wrt_material(unit, m)) {
            mmost = m;
            nmost2 = um_storage_x(unit->type, m) - unit->supply[m];
            break;
        }
        n = wants_n_of_material(unit, m);
        if (n > nmost2) {
            nmost2 = n;
            mmost = m;
        } 
    }
    if (nmost)
      *nmost = nmost2;
    return mmost;
}

/* See if any known enemies in a given cell can exert a blocking ZOC against 
   a given unit to enter another given cell. */

int
can_be_blocked_by_any_known_enemy_at_if_at(int x, int y, int *counter, 
                                           ParamBox *parambox)
{
    ParamBoxUnitAt *bzocpbox = NULL;
    int nx = -1, ny = -1;
    int dist = -1, range = -1;
    Unit *unit = NULL, *unit2 = NULL;
    Side *side = NULL, *oside = NULL;
    UnitView *uview = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int t = NONTTYPE;

    bzocpbox = (ParamBoxUnitAt *)parambox;
    /* Sanity checks. */
    assert_warning_return(bzocpbox, 
                          "Function received unexpected null pointer", 
                          FALSE);
    assert_warning_return(PBOX_TYPE_UNIT_AT == bzocpbox->get_type(), 
                          "Function received illegal parameter", 
                          FALSE);
    unit = bzocpbox->unit;
    assert_warning_return(unit && in_play(unit),
                          "Function received bad parameter",
                          FALSE);
    if (max_zoc_range < 0)
      return FALSE;
    side = unit->side;
    if (!unit_view_at(side, x, y))
      return FALSE;
    u = unit->type;
    nx = bzocpbox->x; ny = bzocpbox->y;
    t = terrain_at(nx, ny);
    dist = distance(x, y, nx, ny);
    for_all_view_stack_with_occs(side, x, y, uview) {
        oside = side_n(uview->siden);
        if (!enemy_side(side, oside))
          continue;
        u2 = uview->type;
        unit2 = view_unit(uview);
        range = zoc_range(unit2, u);
        if (range < dist)
          continue; 
        if (is_active(unit2) && unit_blockable_by(unit, unit2)
            && ((UNSEEN != terrain_view(side, nx, ny)) ? ut_zoc_into(u2, t) 
                                                       : TRUE)
            && ((UNSEEN != terrain_view(side, x, y))
                 ? (ut_zoc_from_terrain(u2, terrain_at(x, y)) > 0) : TRUE))
            return TRUE;
    }
    return FALSE; 
}

/* See if any known enemies can exert a blocking ZOC against a given type 
   attempting to enter a given cell. */ 
int
can_be_blocked_by_any_known_enemy_if_at(int x, int y, int *counter, 
                                        ParamBox *parambox) 
{
    ParamBoxUnitAt *bzocpbox = NULL;
    Unit *unit = NULL;
    int dir = NODIR;
    int nx = -1, ny = -1;

    bzocpbox = (ParamBoxUnitAt *)parambox;
    /* Sanity checks. */
    assert_warning_return(bzocpbox, 
                          "Function received unexpected null pointer.", 
                          FALSE);
    assert_warning_return(PBOX_TYPE_UNIT_AT == bzocpbox->get_type(), 
                          "Function received illegal parameter.", 
                          FALSE);
    unit = bzocpbox->unit;
    assert_warning_return(unit && in_play(unit),
                          "Function received bad parameter.",
                          FALSE);
    if (max_zoc_range < 0)
      return FALSE;
    /* Search for blockers from the speculative position. */
    /* (Should do this with search_around, but that function could use 
        param box support first.) */
    bzocpbox->x = x; bzocpbox->y = y;
    for_all_directions(dir) {
        if (point_in_dir(x, y, dir, &nx, &ny)) {
            if (search_under_arc(nx, ny, dir, max_zoc_range, -1, 
                                 can_be_blocked_by_any_known_enemy_at_if_at, 
                                 (ParamBox *)parambox))
              return TRUE;
        }
    }
    return FALSE; 
}

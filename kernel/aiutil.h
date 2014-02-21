/* Definitions common to all AIs.
   Copyright (C) 2004 Eric A. McDonald

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file aiutil.h
 * Declaration of generic functions that may be of use to an AI or 
 * intelligent UI.
 */

#include "parambox.h"

typedef enum a_occ_status {
    CANNOT_HAVE_OCCS,
    MAYBE_HAS_OCCS,
    DEFINITELY_HAS_OCCS,
    DEFINITELY_HAS_NO_OCCS
} OccStatus;

#define DAMAGE_TYPE_NONE        0
#define DAMAGE_TYPE_ATTACK      1
#define DAMAGE_TYPE_FIRE        2
#define DAMAGE_TYPE_HIT         (DAMAGE_TYPE_ATTACK | DAMAGE_TYPE_FIRE)
#define DAMAGE_TYPE_EXPLODE     4
#define DAMAGE_TYPE_USE_OCC     8
#define DAMAGE_TYPE_USE_TSPT    16

#define VICTIM_NOTHING_SPECIAL  0
#define VICTIM_CAPTURABLE       1
#define VICTIM_SHAKEABLE        2
#define VICTIM_ENCOUNTERABLE    4

extern int could_damage(int u, int u2);
extern int could_destroy(int u, int u2);

extern int type_attack_damage_max(int u, int u2);
extern int type_attack_damage_min(int u, int u2);
extern int type_fire_damage_max(int u, int u2);
extern int type_fire_damage_min(int u, int u2);
extern int type_damage_max(int u, int u2);

extern int type_hit_range_max(int u, int u2);
extern int type_ideal_attack_range_max(int u, int u2);
extern int type_ideal_fire_range_max(int u, int u2);
extern int type_ideal_hit_range_max(int u, int u2);

extern int type_possible_damage_methods(int u, int u2);

extern OccStatus occ_status(UnitView *uview);

extern int planning_to_hit_type_at(Unit *unit, int u, int x, int y);
extern int planning_to_capture_type_at(Unit *unit, int u, int x, int y);
extern int side_planning_to_hit_type_at(Side *side, int u, int x, int y);
extern int n_planning_to_hit_type_at(Side *side, int u, int x, int y);
extern int side_planning_to_capture_type_at(Side *side, int u, int x, int y);
extern int n_planning_to_capture_type_at(Side *side, int u, int x, int y);

extern int has_enough_of_material(Unit *unit, int m);
extern int needs_n_of_material(Unit *unit, int m);
extern int has_full_amount_of_material(Unit *unit, int m);
extern int wants_n_of_material(Unit *unit, int m);
extern int would_starve_without_material(int u, int m);
extern int will_starve_wrt_material(Unit *unit, int m);
extern int can_donate_n_of_material(Unit *unit, int m);
extern int can_sacrifice_n_of_material(Unit *unit, int m);
extern int has_enough_of_all_materials(Unit *unit);
extern int has_full_amount_of_all_materials(Unit *unit);
extern int will_starve(Unit *unit);
extern int critically_needed_material(Unit *unit);
extern int most_needed_material(Unit *unit, int *nmost);
extern int most_wanted_material(Unit *unit, int *nmost);

extern int can_be_blocked_by_any_known_enemy_at_if_at(int x, int y, 
                                                      int *counter, 
                                                      ParamBox *parambox);
extern int can_be_blocked_by_any_known_enemy_if_at(int x, int y, int *counter, 
                                                   ParamBox *parambox);

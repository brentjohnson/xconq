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

    \note Nothing in this file should ever be required to implement an AI; 
	    everything here is optional.
    \todo Import most of the functions in 'aiutil.h' into this file.
    \todo Make use of caching, where applicable.

*/

/* Vision Questions */

//! Could u see?
extern int could_see(int u);

/* Occupancy Questions */

//! Can u on a given side be on a given cell?
extern int can_be_on(int u, Side *side, int x, int y, int *p_without = NULL);
//! Can u on a given side be on a given known cell?
extern int can_be_on_known(
    int u, Side *side, int x, int y, int *p_without = NULL);

//! Could u be an occupant of u2?
extern int could_be_occupant_of(int u, int u2);
//! Could u be an occupant of any utypes?
extern int could_be_occupant(int u);

//! Can u2 on the given side be an occupant of the given uview?
extern int can_be_in(int u2, Side *side, UnitView *uvtspt);

/* Economy Questions */

//! Longest direct economic input for a material from u2 to u.
extern int longest_economic_input(int u, int u2, int m);
//! Longest direct economic input from u2 to u.
extern int longest_economic_input(int u, int u2);
//! Cache of longest direct economic inputs.
extern int *cv__longest_economic_inputs;
//! Longest direct economic input from any utype to u.
extern int longest_economic_input(int u);

//! Materials productivity on given known cell for given utype and side.
extern void productivity_on_known(
    int *p_mtypes, int u, Side *side, int x, int y);

//! Can u on a given side survive on a given known cell?
extern int can_survive_on_known(
    int u, Side *side, int x, int y, int *p_without = NULL);
//! Can a given unit survive a given known cell?
extern int can_survive_on_known(
    Unit *survivor, int x, int y, int *p_without = NULL);

//! Can a given unit refuel another given unit with a given fuel?
extern int can_refuel(Unit *supplier, Unit *demander, int m);

/* Construction and Repair Questions */

//! Can a given unit complete an unit of a given type at creation?
extern int can_create_completed_unit(Unit *acpsrc, Unit *creator, int u2);
//! Can an utype on a given side construct >= 1 utypes?
extern int can_construct_any(int u, Side *side);
//! Can a given unit construct an unit of a given type?
extern int can_construct(Unit *acpsrc, Unit *constructor, int u2);
//! Can a given unit construct >= 1 utypes?
extern int can_construct_any(Unit *acpsrc, Unit *constructor);

//! Can given actor-repairer pair repair any utype?
extern int can_repair_any(Unit *actor, Unit *repairer);

/* Combat Questions */

//! Mean fire damage that u does to u2.
extern int fire_damage_mean(int u, int u2);

//! Mean attack damage that u does to u2.
extern int attack_damage_mean(int u, int u2);

//! Mean detonate damage that u does to u2 in same cell.
extern int detonate_0dist_damage_mean(int u, int u2);
//! Mean detonate damage that u does to u2 at range.
extern int detonate_ranged_damage_mean(int u, int u2);
//! Mean detonate damage that u does to u2.
extern int detonate_damage_mean(int u, int u2);
//! Number of detonations available to u before expiring.
extern int n_detonations_available(int u);

//! Could u counterattack u2?
extern int could_counterattack(int u, int u2);
//! Could u countercapture u2 on a given side?
extern int could_countercapture(int u, int u2, Side *side);
//! Could u prevent capture of u2?
extern int could_prevent_capture_of(int u, int u2);
//! Could u prevent capture by u2?
extern int could_prevent_capture_by(int u, int u2);
//! Could u damage u2 by firing?
extern int could_damage_by_fire(int u, int u2);
//! Could u damage u2 by attacks?
extern int could_damage_by_attacks(int u, int u2);
//! Could u destroy u2 by firing?
extern int could_destroy_by_fire(int u, int u2);
//! Can a given unit destroy a given utype by firing?
extern int can_destroy_by_fire(Unit *actor, Unit *destroyer, int u);
//! Can a given unit destroy >= 1 enemy utypes by firing?
extern int can_destroy_any_by_fire(Unit *actor, Unit *destroyer);
//! Could u destroy u2 by attacks?
extern int could_destroy_by_attacks(int u, int u2);
//! Can a given unit destroy a given utype by attacks?
extern int can_destroy_by_attacks(Unit *actor, Unit *destroyer, int u);
//! Can a given unit destroy >= 1 enemy utypes by attacks?
extern int can_destroy_any_by_attacks(Unit *actor, Unit *destroyer);
//! Could u damage u2 by combat?
extern int could_damage_by_combat(int u, int u2);
//! Can a given unit destroy >= 1 enemy utypes through combat?
extern int can_destroy_any_by_combat(Unit *acpsrc, Unit *destroyer);
//! Could u damage u2 through detonations?
extern int could_damage_by_detonations(int u, int u2);
//! Could u destroy u2 through detonations?
extern int could_destroy_by_detonations(int u, int u2);
//! Can an utype on a given side destroy >= 1 enemy utypes through detonations?
extern int can_destroy_any_by_detonations(int u, Side *side);
//! Can a given unit destroy >= 1 enemy utypes through detonations?
extern int can_destroy_any_by_detonations(Unit *acpsrc, Unit *exploder);
//! Could u damage u2 by any means?
extern int could_damage(int u, int u2);
//! Could u damage >= 1 enemy utypes by any means?
extern int could_damage_any(int u);
//! Can a given unit destroy >= 1 enemy utypes in any way?
extern int can_destroy_any(Unit *acpsrc, Unit *destroyer);
//! Can a given unit capture a given utype by fire?
extern int can_capture_by_fire(Unit *actor, Unit *captor, int u);
//! Can a given unit capture >= 1 enemy utypes in any way?
extern int can_capture_any(Unit *actor, Unit *captor);
//! Can a given unit countercapture >= 1 enemy utypes?
extern int can_countercapture_any(Unit *actor, Unit *countercaptor);
//! Can a given unit prevent capture by >= 1 enemy utypes?
extern int can_prevent_capture_by_any(Unit *actor, Unit *anticaptor);
//! Could u defeat u2 by fire?
extern int could_defeat_by_fire(int u, int u2, Side *side);
//! Can a given unit defeat a given utype by fire?
extern int can_defeat_by_fire(Unit *actor, Unit *firer, int u);
//! Could u defeat u2 by attacks?
extern int could_defeat_by_attacks(int u, int u2, Side *side);
//! Can a given unit defeat a given utype with attacks?
extern int can_defeat_by_attacks(Unit *actor, Unit *attacker, int u);
//! Could u defeat u2 by counterattacks?
extern int could_defeat_by_counterattacks(int u, int u2, Side *side);
//! Can a given unit defeat a given utype with counterattacks?
extern int can_defeat_by_counterattacks(
    Unit *actor, Unit *counterattacker, int u);
//! Can a given unit defeat >= 1 enemy utypes by counterattacks?
extern int can_defeat_any_by_counterattacks(
    Unit *actor, Unit *counterattacker);
//! Could u overwatch against u2?
extern int could_overwatch_against(int u, int u2, Side *side);
//! Could u overwatch against and defeat u2?
extern int could_overwatch_against_and_defeat(int u, int u2);
//! Can a given unit overwatch against and defeat a given utype?
extern int can_overwatch_against_and_defeat(
    Unit *actor, Unit *overwatcher, int u);
//! Can a given unit provide defeating overwatch against >= 1 enemy utypes?
extern int can_overwatch_against_and_defeat_any(
    Unit *actor, Unit *overwatcher);
//! Could u defend against u2 in any way?
extern int could_defend_against(int u, int u2, Side *side);
//! Could u defend against >= 1 enemy utypes in any way?
extern int could_defend_against_any(int u);
//! Can a given unit defend against >= 1 enemy utypes in any way?
extern int can_defend_against_any(Unit *actor, Unit *defender);

/* Exploration Questions */

//! Could u explore?
extern int could_explore(int u);
//! Can a given unit explore?
extern int can_explore(Unit *actor, Unit *explorer);

/* Advanced Construction Questions */

//! Can a given unit construct explorers?
extern int can_construct_explorers(Unit *actor, Unit *constructor);
//! Could u colonize?
extern int could_colonize(int u);
//! Can a given utype on a given side colonize?
extern int can_colonize(int u, Side *side);
//! Can a given unit colonize?
extern int can_colonize(Unit *actor, Unit *colonizer);
//! Can a given unit construct any colonizers?
extern int can_construct_colonizers(Unit *actor, Unit *constructor);
//! Can a given unit construct any offensive units?
extern int can_construct_offenders(Unit *actor, Unit *constructor);
//! Can a given unit construct any defensive units?
extern int can_construct_defenders(Unit *actor, Unit *constructor);

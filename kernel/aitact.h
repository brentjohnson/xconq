/* Tactical Analysis for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Tactical Analysis for AIs

    Part of the AI API, Level 3. 

    Provides a tactical level AI implementation.

    \note Nothing in this file should ever be required to implement an AI; 
	    everything here is optional.

*/

namespace Xconq {
namespace AI {

//! Enumeration of tactical concerns.
enum TacticalConcern {
    //! No tactical concern.
    TC_NONE = 0,
    //! Immediate danger.
    TC_DANGER_IMMEDIATE,
    //! Total number of tactical concern types.
    TC_Total
};

/* Task Inspection */

//! How many friendly units plan to construct at given location?
extern int n_plan_to_construct_at(
    Side *side, int x, int y, short *p_utypes = NULL);

/* Resource Contention Assessement */

//! Would any units be in contention with an utype at a given location?
extern int resource_contention_with_any(int u, int x, int y, Side *side);

/* Offensive/Defensive Assessment */

//! Given various combat stats, which hit method is preferred?
extern int choose_best_hit_method(
    int hitmethod, int fhm, int ahm, int dhm, int ftm, int atm, int dtm);
//! Which hit method should given unit use to hit given uview?
extern int choose_best_hit_method(Unit *unit, UnitView *uview);
//! Which hit method should given uview use to hit given unit?
extern int guess_best_hit_method_used_by(Unit *unit, UnitView *uview);

//! Given various combat stats, which capture method is preferred?
extern int choose_best_capture_method(
    int capmethod, int fcm, int acm, int dccm, int ftm, int atm, int dctm);
//! Which capture method should given unit use to capture given uview?
extern int choose_best_capture_method(Unit *unit, UnitView *uview);
//! Which capture method should given uview use ti hit given unit?
extern int guess_best_capture_method_used_by(Unit *unit, UnitView *uview);

/* Threat Assessment and Handling */

//! Does given unit seem safe from given uview?
extern int seems_safe_against(Unit *unit, UnitView *uview);
//! Is given unit deathly afraid of given uview?
extern int is_frightened_by(Unit *unit, UnitView *uview);
//! Is given unit possibly being harassed by given uview?
extern int is_harassed_by(Unit *unit, UnitView *uview);
//! Choose best direction, if any, to flee from fears.
extern int choose_flee_direction(
    Unit *unit, int feardirs [NUMDIRS + 1], int hardirs [NUMDIRS + 1]);

//! Handle immediate danger to given unit.
/*!
    Immediate danger is considered to be danger to the unit itself, 
    or one of its occupants. Danger can imply either frightful 
    endangerment or simple endangerment (harassment).
    Frightful endangerment means that the unit or one of its occs can be 
    quickly destroyed with no real hope of survival.
    Harassment means that unit or one of its occs can be destroyed, 
    but has a fairly decent fighting chance against the harasser.
*/
extern TacticalConcern handle_immediate_danger(Unit *unit);

/* Overall Tactical Assessment and Handling */

//! Handle tactical situation of given unit.
extern TacticalConcern handle_tactical_situation(Unit *unit);

} // namespace Xconq::AI
} // namespace Xconq

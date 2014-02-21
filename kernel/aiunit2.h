/* Unit and Unit Type Analysis and Status Functions for AIs
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

    \note Nothing in this file should ever be required to implement an AI; 
	    everything here is optional.

*/

/* Vision and Detection Worths */

//! Basic worth as a seer.
extern int seer_worth(int u);
//! Set basic worth as a seer.
extern void set_seer_worth(int u, int n);
//! Set basic worths as seers, if necessary.
extern void maybe_set_seer_worths(void);

/* Size Estimates */

//! Could u meet its size goal, if located at the given position?
/*! \todo Should consider contribution of side treasury. */
extern int could_meet_size_goal(int u, int x, int y);

/* ACP Estimates */

namespace Xconq {
    //! Cache for maximum ACP of utypes without enhancing effects.
    extern int *cv__acp_max_wo_effects;
    //! Cache for maximum ACP per turn of utypes without enhancing effects.
    extern int *cv__acp_per_turn_max_wo_effects;
}
//! Maximum ACP u can have without enhancing effects.
extern int acp_max_wo_effects(int u);
//! Maximum ACP per turn u can have without enhancing effects.
extern int acp_per_turn_max_wo_effects(int u);

/* Speed Estimates */

namespace Xconq {
    //! Cache for maximum speed of utypes without enhancing effects.
    extern int *cv__speed_max_wo_effects;
}
//! Maximum speed u can have without enhancing effects.
extern int speed_max_wo_effects(int u);

/* MP Estimates */

namespace Xconq {
    //! Cache for maximum MP per turn of utypes.
    extern int *cv__mp_per_turn_max;
}
//! Maximum MP u can get per turn.
extern int mp_per_turn_max(int u);

/* Number of Moves Estimates */

namespace Xconq {
    //! Cache of maximum moves utypes can make per turn on t.
    extern int **cv__moves_per_turn_max_on_t;
}
//! Maximum moves u can make per turn on t.
extern int moves_per_turn_max(int u, int t);

/* Movement Range Estimates */

namespace Xconq {
    //! Cache of max movement range of u if producing m on t.
    extern int ***cv__move_range_max_on_t_with_m;
    //! Cache of max movement range of u on t.
    extern int **cv__move_range_max_on_t;
}
//! Max movement range of u if producing m on t, and given an amount of m.
extern int move_range_max_on(int u, int t, int m, int amt);
//! Max movement range of u on t, and given amounts of mtypes.
extern int move_range_max(int u, int t, int *amt);
//! Best movement range of u, given amounts of mtypes.
extern int move_range_best(int u, int *amt);
//! Worst movement range of u, given amounts of mtypes.
extern int move_range_worst(int u, int *amt);

/* Movement Worths */

//! Basic worth as a mover.
extern int mover_worth(int u);
//! Set basic worth as a mover.
extern void set_mover_worth(int u, int n);
//! Set basic worths as movers, if necessary.
extern void maybe_set_mover_worths(void);

//! Basic worth as a depot.
extern int depot_worth(int u);
//! Set basic worth as a depot.
extern void set_depot_worth(int u, int n);
//! Set basic worths as depots, if necessary.
extern void maybe_set_depot_worths(void);

//! Basic worth as a distributor.
extern int distributor_worth(int u);
//! Set basic worth as a distributor.
extern void set_distributor_worth(int u, int n);
//! Set basic worths as distributors, if necessary.
extern void maybe_set_distributor_worths(void);

//! Basic worth as a producer.
extern int producer_worth(int u);
//! Set basic worth as a producer.
extern void set_producer_worth(int u, int n);
//! Set basic worths as producers, if necessary.
extern void maybe_set_producer_worths(void);
//! Given unit's worth as a producer on a given known cell.
extern int producer_worth_on_known(Unit *producer, int x, int y);

//! Basic worth as a production enhancer.
extern int prod_enhancer_worth(int u);
//! Set basic worth as a production enhancer.
extern void set_prod_enhancer_worth(int u, int n);
//! Set basic worths as production enhancers, if necessary.
extern void maybe_set_prod_enhancer_worths(void);

/* Worths as a base. */
/* A base is considered to be a depot inclusive-or producer that has 
    stored materials in common with another unit that can occupy it. */

//! Basic worth as a base for u2.
extern int base_worth_for(int u, int u2);
//! Set basic worth as a base for u2.
extern void set_base_worth_for(int u, int n);
//! Set basic worths as bases for utypes, if necessary.
extern void maybe_set_base_worths_for(void);

//! Basic worth as a base.
extern int base_worth(int u);
//! Set basic worth as a base.
extern void set_base_worth(int u, int n);
//! Set basic worths as bases, if necessary.
extern void maybe_set_base_worths(void);

/* Construction Questions */

//! Does a given side seem to be able to create u3 in a given transport?
extern int can_create_in(
    Unit *actor, Unit *creator, int u3, Unit *transport);
//! Does a given side seem to be able to create u3 at a given location?
extern int can_create_at(Unit *actor, Unit *creator, int u3, int x, int y);

/* Tooling Point Estimates */

//! How many TP for a given utype can a given unit add to itself per turn?
extern int tp_per_turn_est(Unit *unit, int u2);

/* Construction Point Estimates */

//! How many CP can a given unit add to a given utype per turn?
extern int cp_per_turn_est(Unit *unit, int u2);
//! How many CP can a given side add to a given unit per turn?
extern int cp_gained_per_turn_est(Unit *unit, Side *side);

/* Hitpoint Estimates */

//! How many HP can a given unit add to a given utype per turn?
extern int hp_per_turn_est(Unit *unit, int u2);
//! How many HP can a given side add to a given unit per turn?
extern int hp_gained_per_turn_est(Unit *unit, Side *side);

/* Combat Rate Estimates */

//! Estimated number of times u can fire at u2 in a turn.
extern int firings_per_turn_est(int u, int u2);
//! Estimated number of times u can attack u2 in a turn.
extern int attacks_per_turn_est(int u, int u2);
//! Estimated number of times u can detonate against u2 in a turn.
extern int detonations_per_turn_est(int u, int u2);
//! Estimated number of times u can attempt capture of u2 in a turn.
extern int capture_attempts_per_turn_est(int u, int u2, Side *side2);

/* Hit Estimates */

//! Mean number of firings for u to destroy u2 with given modifiers.
extern int firings_mean_to_destroy(
    int u, int u2, int hp2, int dm = 100, int hcm = 100);
//! Mean number of firings for u to destroy given uview.
extern int firings_mean_to_destroy(int u, UnitView *uview);
//! Mean number of firings for u2 to destroy given unit.
extern int firings_mean_to_destroy(int u2, Unit *unit);
//! Mean number of turns for u to destroy given uview by firing.
extern int fire_turns_mean_to_destroy(int u, UnitView *uview);
//! Mean number of turns for u2 to destroy given unit by firing.
extern int fire_turns_mean_to_destroy(int u2, Unit *unit);

//! Mean number of attacks for u to destroy u2 with given modifiers.
extern int attacks_mean_to_destroy(
    int u, int u2, int hp2, int dm = 100, int hcm = 100);
//! Mean number of attacks for u to destroy given uview.
extern int attacks_mean_to_destroy(int u, UnitView *uview);
//! Mean number of attacks for u2 to destroy given unit.
extern int attacks_mean_to_destroy(int u2, Unit *unit);
//! Mean number of turns for u to destroy given uview by attacks.
extern int attack_turns_mean_to_destroy(int u, UnitView *uview);
//! Mean number of turns for u2 to destroy given unit by attacks.
extern int attack_turns_mean_to_destroy(int u2, Unit *unit);

//! Mean number of detonations for u to destroy u2 with given modifiers.
extern int detonations_mean_to_destroy(
    int u, int u2, int hp2, int dm = 100);
//! Mean number of detonations for u to destroy given uview.
extern int detonations_mean_to_destroy(int u, UnitView *uview);
//! Mean number of detonations for u2 to destroy given unit.
extern int detonations_mean_to_destroy(int u2, Unit *unit);
//! Mean number of turns for u to destroy given uview by detonations.
extern int detonate_turns_mean_to_destroy(int u, UnitView *uview);
//! Mean number of turns for u2 to destroy given unit by detonations.
extern int detonate_turns_mean_to_destroy(int u2, Unit *unit);

/* Capture Estimates */

//! Mean number of attempts u needs to capture u2 by firing.
extern int firings_mean_to_capture(int u, int u2, Side *side2, int ccm = 100);
//! Mean number of attempts u needs to capture given uview by firing.
extern int firings_mean_to_capture(int u, UnitView *uview);
//! Mean number of attempts u2 needs to capture given unit by firing.
extern int firings_mean_to_capture(int u2, Unit *unit);
//! Mean number of turns u needs to capture given uview by firing.
extern int fire_turns_mean_to_capture(int u, UnitView *uview);
//! Mean number of turns u2 needs to capture given unit by firing.
extern int fire_turns_mean_to_capture(int u2, Unit *unit);

//! Mean number of attempts u needs to capture u2 by attacks.
extern int attacks_mean_to_capture(int u, int u2, Side *side2, int ccm = 100);
//! Mean number of attempts u needs to capture given uview by attacks.
extern int attacks_mean_to_capture(int u, UnitView *uview);
//! Mean number of attempts u2 needs to capture given unit by attacks.
extern int attacks_mean_to_capture(int u2, Unit *unit);
//! Mean number of turns u needs to capture given uview by attacks.
extern int attack_turns_mean_to_capture(int u, UnitView *uview);
//! Mean number of turns u2 needs to capture given unit by attacks.
extern int attack_turns_mean_to_capture(int u2, Unit *unit);

//! Mean number of attempts u needs to directly capture u2.
extern int attempts_mean_to_capture(int u, int u2, Side *side2, int ccm = 100);
//! Mean number of attempts u needs to directly capture given uview.
extern int attempts_mean_to_capture(int u, UnitView *uview);
//! Mean number of attempts u2 needs to directly capture given unit.
extern int attempts_mean_to_capture(int u2, Unit *unit);
//! Mean number of turns u needs to directly capture given uview.
extern int capture_turns_mean_to_capture(int u, UnitView *uview);
//! Mean number of turns u2 needs to directly capture given unit.
extern int capture_turns_mean_to_capture(int u2, Unit *unit);

/* Worths as an explorer. */
/* An explorer is considered to be a mobile unit which can see. */

//! Basic worth as an explorer.
/*!
    \note
    We could consider range like the old 'exploring_worth' function
    does. However, a fast, short-range explorer, such as a Fighter
    in the Default game, should not be undervalued against a slower, 
    longe-range explorer, such as a Bomber.
*/
extern int explorer_worth(int u);
//! Set basic worth as an explorer.
extern void set_explorer_worth(int u, int n);
//! Set basic worths as explorers, if necessary.
extern void maybe_set_explorer_worths(void);

/* Total Worths */

//! Sum of all the basic worths.
extern int total_worth(int u);

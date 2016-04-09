/* Functions common to all AIs.
   Copyright (C) 1992-1997, 1999-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file contains code used by all types of AIs, plus generic
   AI-running code.  While AI code is officially outside the kernel,
   the functions here we need some definitions from kernel.h and so
   must include it. */

#include "conq.h"
#include "kpublic.h"
#include "aiutil.h"
#include "ai.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aioprt.h"

using namespace Xconq;

extern void register_mplayer(AI_ops *ops);

/* Needed in set_side_ai. */

extern void force_replan(Unit *unit);

/* Used in plan.c. */

int ai_go_after_victim(Unit *unit, int range, int broadcast);

extern int taskexecs;
extern int need_ai_init_turn;
extern int need_ai_planning;
extern int need_ai_action_reaction;
extern int need_ai_for_new_side;
extern int need_ai_finish_movement;
extern int latest_action_x, latest_action_y;

static int numaitypes;

/* The array of all possible AI types. */

static AI_ops *all_ai_ops;

/* The number of AIs run by this program. */

static int num_local_ais = -1;

/* Moved here from mplayer.c etc. */

GameClass game_class = gc_none;
int bhw_max = 0;

static void ai_init_turn(Side *side);
static void ai_decide_plan(Side *side, Unit *unit);
static void ai_react_to_task_result(Side *side, Unit *unit, Task *task, TaskOutcome rslt);
static int ai_adjust_plan(Side *side, Unit *unit);
static void ai_finish_movement(Side *side);

static int ai_integrate_advance_worth(Side *side, int a, int *numdeps);
static void ai_pick_side_research_goal(Side *side);
static int advance_leads_to_goal(Side *side, int a);
static void ai_plan_research(Side *side);

static int basic_worth(int u);
static int offensive_worth(int u);
static int defensive_worth(int u);
static int colonizing_worth(int u);
static int facility_worth(int u);
static int random_worth(int u);
static int siege_worth(int u);

static int basic_hit_worth(int u, int e);
static int basic_fire_worth(int u, int e);
static int basic_capture_worth(int u, int e);
static int basic_transport_worth(int u, int e);
static void display_assessment(void);

static int is_base_for(int u1, int u2);
static int is_carrier_for(int u1, int u2);

static void set_u_is_base(int u, int n);
static void set_u_is_transport(int u, int n);
static void set_u_is_carrier(int u, int n);
static void set_u_is_base_builder(int u, int n);
static void set_u_can_make(int u, int n);
static void set_u_can_capture(int u, int n);
static void set_u_bw(int u, int n);

static void set_u_offensive_worth(int u, int n);
static void set_u_defensive_worth(int u, int n);
static void set_u_colonizer_worth(int u, int n);
static void set_u_facility_worth(int u, int n);
static void set_u_random_worth(int u, int n);
static void set_u_siege_worth(int u, int n);
static void set_u_colonization_support_worth(int u, int n);
static void set_u_base_construction_worth(int u, int n);
static void set_u_exploration_support_worth(int u, int n);
static void set_u_offensive_support_worth(int u, int n);
static void set_u_defensive_support_worth(int u, int n);

static void set_u_is_ground_mobile(int u, int n);
static void set_u_is_naval_mobile(int u, int n);
static void set_u_is_air_mobile(int u, int n);
static void set_u_is_advanced(int u, int n);
static void set_u_is_colonizer(int u, int n);
static void set_u_is_facility(int u, int n);
	
static void set_uu_bhw(int u1, int u2, int v);
static void set_uu_bfw(int u1, int u2, int v);
static void set_uu_bcw(int u1, int u2, int v);
static void set_uu_btw(int u1, int u2, int v);

static int cell_unknown(int x, int y);
static int enemies_present(int x, int y);

static int carryable(int u);

/* New action-reaction code. */

static void ai_react_to_action(Side *side, Unit *unit);
static void check_current_target(Unit *unit);
static void defensive_reaction(Unit *unit);
static int keep_defensive_goal(Unit *unit, Unit *unit2);
static int maybe_defend_own_transport(Unit *unit);
static Unit *undefended_neighbour(Unit *unit);
static int maybe_defend_other_transport(Unit *unit, Unit *unit2);
static void offensive_reaction(Unit *unit);
static int ai_go_after_captive(Unit *unit, int range);
static int ai_fire_at_opportunity(Unit *unit);
static int mp_collect_here(int x, int y);
static int compare_weights(const void *w1, const void *w2);
static int probably_explorable(Side *side, int x, int y, int u);
static int real_capture_chance(Unit *unit, Unit *unit2);
static int attack_can_damage_or_capture(Unit *unit, Unit *unit2);
static int fire_can_damage_or_capture(Unit *unit, Unit *unit2);
static int fire_can_damage(Unit *unit, Unit *unit2);

void
init_ai_types(void)
{
    /* Fill in the table of pointers to AI types. */
    all_ai_ops = (AI_ops *) xmalloc(MAXAITYPES * sizeof(AI_ops));
    numaitypes = 0;
    /* We leave all_ai_ops[0] unassigned so as to catch bad AI refs
       due to uninitialized side->aitype. */
    /* Call each AI's registration function. */
    register_mplayer(&(all_ai_ops[++numaitypes]));
}

/* Given an AI type name, find its numerical index. */

int
find_ai_type(const char *aitype)
{
    int i;

    for (i = 1; i <= numaitypes; ++i) {
	if (strcmp(all_ai_ops[i].name, aitype) == 0)
	  return i;
    }
    return 0;
}

/* Given a numerical index, return the name. */

const char *
ai_type_name(int n)
{
    if (!between(1, n, numaitypes))
      return NULL;
    return all_ai_ops[n].name;
}

/* Given an AI type name, return the next name in the list of types,
   or NULL if the name is the last in the list. */

const char *
next_ai_type_name(const char *aitype)
{
    int i;
    const char *name;

    if (aitype == NULL)
      return all_ai_ops[1].name;
    for (i = 1; i < numaitypes; ++i) {
	name = all_ai_ops[i].name;
	if (strcmp(name, aitype) == 0)
	  return all_ai_ops[i+1].name;
    }
    return NULL;
}

/* Given a numerical index, return the name. */

const char *
ai_type_help(int n)
{
    if (!between(1, n, numaitypes))
      return NULL;
    return all_ai_ops[n].help;
}

/* Given a side that wants to have an AI running its units, find and
   record the actual AI type that will be used, and run its
   initialization method. */

void
init_ai(Side *side)
{
    int i;
    const char *aitype;
    int (*test)(void);
    void (*fn)(Side *side);

    if (side_wants_ai(side)) {
	/* (should merge with same bit in set_side_ai) */
	if (strcmp(side->player->aitypename, "ai") == 0) {
	    /* (should use the "best" default for this game and side) */
	    side->player->aitypename = "mplayer";
	}
	/* If remote, do nothing beyond recording the name of the AI. */
	if (numremotes > 0 && side->player->rid != my_rid) {
	    side->rai = (RAI *) xmalloc(sizeof(RAI));
	    return;
	}
	/* Scan through the possible AI types. */
	for (i = 1; i <= numaitypes; ++i) {
	    aitype = all_ai_ops[i].name;
	    if (aitype != NULL
		&& strcmp(aitype, side->player->aitypename) == 0) {
		test = all_ai_ops[i].to_test_compat;
		if (test == NULL || (*test)()) {
		    /* Record the index of the AI type. */
		    side->aitype = i;
		    fn = all_ai_ops[i].to_init;
		    if (fn)
		      (*fn)(side);
		} else {
		    /* desired aitype incompatible - should complain */
		}
	    }
	}
	if (!side_has_ai(side)) {
	    init_warning("could not make an AI (type %s) for %s",
			 side->player->aitypename, side_desig(side));
	}
    }
}

/* Change the AI running a side. */

void
set_side_ai(Side *side, const char *aitype)
{
    Unit *unit;
    Side *side2;

    /* Pick the mplayer if we just asked for an ai without name. */
    if (!empty_string(aitype) && strcmp(aitype, "ai") == 0) {
	aitype = "mplayer";
    }
    /* Set the desired ai by name. */
    if (empty_string(aitype)) {
	side->player->aitypename = NULL;
    } else {
	side->player->aitypename = copy_string(aitype);
    }
    /* This is a remote ai. */
    if (numremotes > 0 && side->player->rid != my_rid) {
	/* Dump any existing rai. */
	if (side->rai != NULL) {
	    free(side->rai);
	    side->rai = NULL;
	}    
	/* Maybe allocate a new one. */
	if (!empty_string(aitype))
	    side->rai = (RAI *) xmalloc(sizeof(RAI));
    /* This is a local ai. */
    } else {
	/* Dump any existing ai. */
	if (side->ai != NULL) {
		free(side->ai);
		side->ai = NULL;
	}    
	/* Switching to human control. */
	if (empty_string(aitype)) {
	    DMprintf("%s switches to human control.\n", side_desig(side));
	    /* Switching to ai control. */
	} else {
	    DMprintf("%s switches to %s control.\n", 
		     side_desig(side), aitype);
	    /* We need a new ai. */
	    need_ai_for_new_side = TRUE;
	    /* We need to init the strategy code. */
	    need_ai_init_turn = TRUE;
	}
    }
    if (gameinited) {
	/* Replan everything, whether switching to ai or human control. */
	DMprintf("%s is replanning all units after ai switch.\n", 
		 side_desig(side));
	for_all_side_units(side, unit) {
	    if (is_active(unit)) {
		/* We do this locally on each computer to save time. */
		unit->aihook = NULL;
		force_replan(unit);
	    }
	}
    }
    /* Stimulate a recalculation later. */
    num_local_ais = -1;
    if (!empty_string(side->player->aitypename)) {
	notify_all("AI %s is running %s.", 
		   side->player->aitypename, short_side_title(side));
    } else {
	notify_all("No AI is running %s.", short_side_title(side));
    }
    /* Make sure the ai player info is updated. */
    for_all_sides(side2) {
	if (side_has_display(side2)) {
	    update_side_display(side2, side, TRUE);
	}
    }
}

void run_ai_plan_adjust(Side *side);

/* (should decide on useful return value and use, or make void) */

int
run_local_ai(int when, int maxplanning)
{
    Side *side;
    Unit *unit;
    const char *activity = "run_local_ai";

    /* Make sure we know how many local AIs are present. */
    if (num_local_ais < 0) {
	num_local_ais = 0;
	for_all_sides(side) {
	    if (side_has_local_ai(side))
	      ++num_local_ais;
	}
    }
    /* If no AIs to run, get out quickly. */
    if (num_local_ais == 0 && !need_ai_for_new_side)
      return 0;
    record_activity_start(activity, when);
#if 0	/* This caused buzzing by preventing passive units under AI 
	   control from getting necessary replanning. */
    /* Only do this routine at a certain rate. */
    if (current_play_rate > 0
	&& !n_ms_elapsed(60000 / current_play_rate)
	) {
	record_activity_end(activity, 1);
	return 0;
    }
#endif
    if (need_ai_for_new_side) {
	for_all_sides(side) {
	    if (side_wants_ai(side) && !side_has_ai(side)) {
		init_ai(side);
	    }
	}
	need_ai_for_new_side = FALSE;
    }
    /* This code does the following:
    1) Analyzes the game.
    2) Reviews goals, theaters and units.
    3) Updates strategy and unit plans.
    Executed at the start of each turn. */
    if (need_ai_init_turn) {
	for_all_sides(side) {
	    if (side_has_local_ai(side))
	      ai_init_turn(side);
	}
	need_ai_init_turn = FALSE;
    }
    /* This code finds a new plan for any unit with a passive plan. 
	Before that, however, ai_react_to_action is called to handle any 
	emergencies and also to ensure that peace garrisons are maintained.
	Executed after each plan execution, during forced replanning
	and at the start of each turn. */
    if (need_ai_planning) {
	for_all_sides(side) {
	    if (side_has_local_ai(side) 
		/* (should) and need planning for this side*/) {
		for_all_side_units(side, unit) {
		    if (is_active(unit)
			&& ai_controlled(unit)
			&& unit->plan
			&& !unit->plan->asleep
			/* Don't mess with acp-independent units. */
			&& !acp_indep(unit)) {
			ai_decide_plan(side, unit);
		    }
		}
	    }
	}
	need_ai_planning = FALSE;
    }
    /* This code handles side research. */
    if (numatypes > 0) {
	for_all_sides(side) {
	    if (side_has_local_ai(side) 
	        && side->research_topic == NOADVANCE) {
		ai_plan_research(side);
	    }
	}
    }
    /* Run evaluators for operational roles. */
    for_all_sides(side) {
	if (side_has_local_ai(side) && Xconq::AI::get_side_ai(side))
	  Xconq::AI::handle_oproles(side);
    }
    /* This code does two things:
       1) Reacts to failed move tasks as approprite.
       2) Reacts to failed resupply tasks by trying to build a depot.
       Executed after each task execution. */
    if (taskexecs > 0) {
	for_all_sides(side) {
	    if (side_has_local_ai(side)) {
		for_all_side_units(side, unit) {
		    if (is_active(unit)
			&& ai_controlled(unit)
			&& unit->plan
			&& &(unit->plan->last_task)
			&& ((&(unit->plan->last_task))->type == TASK_MOVE_TO
			     || (&(unit->plan->last_task))->type == 
				 TASK_RESUPPLY)
			&& unit->plan->last_task_outcome == TASK_FAILED) {
			ai_react_to_task_result(side, unit,
						&(unit->plan->last_task),
						unit->plan->last_task_outcome);
			/* Necessary to enable execution of the new task. */
			net_clear_task_outcome(side, unit);
		    }
		}
	    }
	}
    }
    /* This code does two things:
       1) Reevaluates the defense of cities and assigns or releases defenders.
       2) Sets (not pushes) hit tasks etc. for enemy units that appear nearby.
       Executed after certain actions. */
    if (need_ai_action_reaction) {
	for_all_sides(side) {
	    if (side_has_local_ai(side)) {
		for_all_side_units(side, unit) {
		    if (is_active(unit)
			&& ai_controlled(unit)
			&& (distance(unit->x, unit->y, 
				     latest_action_x, latest_action_y)
			    <= u_ai_tactical_range(unit->type))) {
			ai_react_to_action(side, unit);
		    }
		}
	    }
	}
	need_ai_action_reaction = FALSE;
    }
    /* This code does four things:
       1) Sets build tasks for stationary units with no tasks.
       2) Forces units with PLAN_IMPROVE and no tasks to replan.
       3) Forces units that are waiting for tasks to replan.
       4) Forces units with too many plan execs this turn into reserve.
       Executed each round. */ 
    if (1) {
	for_all_sides(side) {
	    if (side_has_local_ai(side) && !side->finishedturn) {
		run_ai_plan_adjust(side);
	    }
	}
    }
    /* This code does three things:
       1) Organizes pickup of any units already known to need transport.
       2) Checks if any other units need transport (rethink_plan).
       Executed at each end of turn. 
       The code used to test for !side->finishedturn, but this made execution
       impossible since need_ai_finish_movement is set only when all sides
       have finished their turns. */
    if (need_ai_finish_movement) {
	for_all_sides(side) {
	    if (side_has_local_ai(side)) {
		ai_finish_movement(side);
	    }
	}
	need_ai_finish_movement = FALSE;
    }
    record_activity_end(activity, 1);
    return 0;
}

void
run_ai_plan_adjust(Side *side)
{
    int curactor, domore;
    Unit *unit;

    if (side->actionvector == NULL)
      return;
    domore = TRUE;
    for (curactor = 0; curactor < side->actionvector->numunits; ++curactor) {
	unit = unit_in_vector(side->actionvector, curactor);
	if (ai_controlled(unit)
	    /* Don't mess with acp-independent units. */
	    && !acp_indep(unit)) {
	    /* Domore is now always true. Skip this test? */
	    domore = ai_adjust_plan(side, unit);
	    if (!domore)
	      return;
	}
    }
}

/* The following functions dispatch on AI type. */

void
ai_init_turn(Side *side)
{
    void (*fn)(Side *side);

    fn = all_ai_ops[side->aitype].to_init_turn;
    if (fn)
      (*fn)(side);
}

void
ai_decide_plan(Side *side, Unit *unit)
{
    void (*fn)(Side *side, Unit *unit);

    /* This is the replanning code from ai_react_to_action.
    It needs to be called at least once each turn even in the absence of any
    hostile actions, to make sure that peace garrisons are maintained. Not
    sure if this is the best place, though. */ 
    
    /* Note: executing this code at the start of each turn also makes the 
    "attack nearby enemy unit" code in mplayer_rethink_plan etc (called 
    by ai_finish_movement) unnecessary. */

/*    ai_react_to_action(side, unit); */

    fn = all_ai_ops[side->aitype].to_decide_plan;
    if (fn) {
      (*fn)(side, unit);
    }
    /* Code moved here to make the ai more responsive, particularly
    when turned on in the middle of a game, where the to_decide_plan
    call would erase any tasks set up by the first ai_react_to_action
    call. */
    ai_react_to_action(side, unit);
}

/* Forward a task result to the appropriate AI routine. */
/* (should make task and rslt args implicit) */

void
ai_react_to_task_result(Side *side, Unit *unit, Task *task, TaskOutcome rslt)
{
    void (*fn)(Side *side, Unit *unit, Task *task, TaskOutcome rslt);

    fn = all_ai_ops[side->aitype].to_react_to_task_result;
    if (fn)
      (*fn)(side, unit, task, rslt);
}

void
ai_react_to_new_side(Side *side, Side *side2)
{
    void (*fn)(Side *side, Side *side2);

    fn = all_ai_ops[side->aitype].to_react_to_new_side;
    if (fn)
      (*fn)(side, side2);
}

int
ai_adjust_plan(Side *side, Unit *unit)
{
    int (*fn)(Side *side, Unit *unit);

    fn = all_ai_ops[side->aitype].to_adjust_plan;
    if (fn)
      return (*fn)(side, unit);
    else
      return FALSE;
}

void
ai_finish_movement(Side *side)
{
    void (*fn)(Side *side);

    fn = all_ai_ops[side->aitype].to_finish_movement;
    if (fn)
      (*fn)(side);
}

/* Push a new goal onto the side's list of goals. */

/* (this should only add goals that are not already present) */

void
add_goal(Side *side, Goal *goal)
{
    if (ai(side)->numgoals < MAXGOALS) {
        ai(side)->goals[(ai(side)->numgoals)++] = goal;
        DMprintf("%s added %s\n", side_desig(side), goal_desig(goal));
    } else {
        DMprintf("%s has no room for %s\n",
                 side_desig(side), goal_desig(goal));
    }
}

/* Return any goal of the given type. */

Goal *
has_goal(Side *side, GoalType goaltype)
{
    int i;
    Goal *goal;

    for (i = 0; i < ai(side)->numgoals; ++i) {
        goal = ai(side)->goals[i];
        if (goal != NULL && goal->type == goaltype) {
            return goal;
        }
    }
    return NULL;
}

/* Return an unfulfilled goal of the given type. */

Goal *
has_unsatisfied_goal(Side *side, GoalType goaltype)
{
    int i;
    Goal *goal;

    for (i = 0; i < ai(side)->numgoals; ++i) {
        goal = ai(side)->goals[i];
        if (goal != NULL && goal->type == goaltype
            && goal_truth(side, goal) < 100) {
            return goal;
        }
    }
    return NULL;
}

void
ai_receive_message(Side *side, Side *sender, const char *str)
{
    /* First detect standard messages. */
    if (strcmp(str, "Eh?") == 0) {
	/* Don't respond, otherwise we might infinitely recurse. */
    } else if (allied_side(side, sender)) {
	/* (should say something) */
    } else {
	/* Detect insults and respond appropriately. */
	/* (should be case-insensitive) */
	if (strstr(str, "idiot")) {
	    net_send_message(side, add_side_to_set(sender, NOSIDES), "Loser!");
	} else if (strstr(str, "loser")) {
	    net_send_message(side, add_side_to_set(sender, NOSIDES), "Idiot!");
	} else {
	    /* No idea what the message was, be puzzled. */
	    net_send_message(side, add_side_to_set(sender, NOSIDES), "Eh?");
	}
    }
}

void
ai_save_state(Side *side)
{
    Obj *(*fn)(Side *side), *state = lispnil;

    fn = all_ai_ops[side->aitype].to_save_state;
    if (fn)
      state = (*fn)(side);
    /* Don't bother if there's no AI state to mess with. */
    if (side->aidata == lispnil && state == lispnil)
      return;
    side->aidata = replace_at_key(side->aidata, side->player->aitypename,
				  state);
}

/* Return a numerical value indicate the AI's use of the given location.
   For instance, each number could represent a region or theater of
   operations. */

int
ai_region_at(Side *side, int x, int y)
{
    int (*fn)(Side *side, int x, int y);

    fn = all_ai_ops[side->aitype].region_at;
    if (fn)
      return (*fn)(side, x, y);
    else
      return 0;
}

/* Provide textual information about the AI's usage or opinion of the
   given location. */

const char *
ai_at_desig(Side *side, int x, int y)
{
    const char *(*fn)(Side *side, int x, int y);

    fn = all_ai_ops[side->aitype].at_desig;
    if (fn)
      return (*fn)(side, x, y);
    else
      return NULL;
}

/* Set willingness to declare a draw. */

void
try_to_draw(Side *side, int flag, const char *ainame)
{
    /* If no change, nothing to say or do. */
    if (flag == side->willingtodraw)
      return;
    /* If there is a human player and the decision has not been
       delegated, allow the human to make the final decision. */
    if (side_has_display(side) && !side->ai_may_resign) {
	if (flag)
	  notify(side, "Your AI %s recommends being willing to draw.",
		 ainame);
	else
	  notify(side, "Your AI %s recommends not being willing to draw.",
		 ainame);
	return;
    }
    net_set_willing_to_draw(side, flag);
}

/* If an AI resigns, it tries to help its friends. */

void
give_up(Side *side, const char *ainame)
{
    Side *side1;

    /* The independents cannot resign unless played by a human. */
    if (side == indepside && !side_has_display(side))
      return;
    /* Can't resign if we've been forbidden to do so. */
    if (side_has_display(side) && !side->ai_may_resign) {
	/* But speak up about it. */
	notify(side, "Your AI %s recommends resignation", ainame);
	return;
    }
    /* Try to give away all of our units to an ally. */
    for_all_sides(side1) {
	if (side != side1 && allied_side(side, side1) && side1->ingame) {
	    if (side_has_display(side))
	      notify(side, "Your AI resigns, giving its assets to %s!",
		     short_side_title(side1));
	    net_resign_game(side, side1);
	    return;
	}
    }
    /* No allies left in game, leave everything to its fate. */
    if (side_has_display(side))
      notify(side, "Your AI resigns!");
    net_resign_game(side, NULL);
}

/* (should go elsewhere eventually?) */

/* Goal handling. */

GoalDefn goaldefns[] = {

#undef  DEF_GOAL
#define DEF_GOAL(NAME,DNAME,code,ARGTYPES) { NAME, DNAME, ARGTYPES },

#include "goal.def"

    { NULL, NULL, NULL }
};


/* General handling of goals. */

Goal *
create_goal(GoalType type, Side *side, int tf)
{
    Goal *goal = (Goal *) xmalloc(sizeof(Goal));

    goal->type = type;
    goal->side = side;
    goal->tf = tf;
    return goal;
}

int
cell_unknown(int x, int y)
{
    return !(tmpside->see_all || terrain_view(tmpside, x, y) != UNSEEN);
}

int
enemies_present(int x, int y)
{
    UnitView *uv;

    for_all_view_stack_with_occs(tmpside, x, y, uv) {
	if (!trusted_side(tmpside, side_n(uv->siden)))
	  return TRUE;
    }
    return FALSE;
}

/* Test a goal to see if it is true for side, as specified. */

int
goal_truth(Side *side, Goal *goal)
{
    int x, y, m, n;

    if (goal == NULL) return 0;
    switch (goal->type) {
      case GOAL_POSITIONS_KNOWN:
	/* what if no enemies present? then this is undefined? */
	/* should goals have preconditions or prerequisites? */
	return 0;
      case GOAL_VICINITY_KNOWN:
	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  cell_unknown, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      case GOAL_VICINITY_HELD:
      	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  enemies_present, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      case GOAL_CELL_OCCUPIED:
	return 0;
      case GOAL_UNIT_OCCUPIED:
	return 0;
      case GOAL_COLONIZE:
	return 0;
      case GOAL_HAS_UNIT_TYPE:
	return 0;
      case GOAL_HAS_UNIT_TYPE_NEAR:
	return 0;
      case GOAL_HAS_MATERIAL_TYPE:
	m = goal->args[0];
	n = goal->args[1];
	if (side->treasury[m] >= n)
	  return 100;
	else
	  return -100;
      default:
	case_panic("goal type", goal->type);
	return 0;
    }
}

/* (might eventually want another evaluator that guesses at another
   side's goals) */

char *goalbuf = NULL;

const char *
goal_desig(Goal *goal)
{
    int numargs, i, arg;
    const char *argtypes;

    if (goal == NULL)
      return "<null goal>";
    if (goalbuf == NULL)
      goalbuf = (char *)xmalloc(BUFSIZE);
    sprintf(goalbuf, "<goal s%d %s%s",
	    side_number(goal->side), (goal->tf ? "" : "not "),
	    goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	arg = goal->args[i];
	switch (argtypes[i]) {
	  case 'h':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  case 'm':
	    if (is_material_type(arg))
	      tprintf(goalbuf, " %s", m_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'S':
	    tprintf(goalbuf, " `%s'", side_desig(side_n(arg)));
	    break;
	  case 'u':
	    if (is_unit_type(arg))
	      tprintf(goalbuf, " %s", u_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'U':
	    tprintf(goalbuf, " `%s'", unit_desig(find_unit(arg)));
	    break;
	  case 'w':
	    tprintf(goalbuf, " %dx", arg);
	    break;
	  case 'x':
	    tprintf(goalbuf, " %d,", arg);
	    break;
	  case 'y':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  default:
	    tprintf(goalbuf, " %d", arg);
	    break;
	}
    }
    strcat(goalbuf, ">");
    return goalbuf;
}

/* Common AI code moved here from mplayer.c etc. */

/* Determine game type from name of included modules. */

GameClass
find_game_class(void)
{
    Module *m;

    for_all_modules(m) {
	if (strcmp(m->name, "time") == 0
	    || (m->origmodulename && strcmp(m->origmodulename, "time") == 0))
	  return gc_time;
    	else if (strcmp(m->name, "advanced") == 0
		 || (m->origmodulename
		     && strcmp(m->origmodulename, "advanced") == 0))
	  return gc_advanced;
    }
    return gc_standard;
}

/* For each unit, decide what it should be doing (if anything).  This is
   when a side takes the initiative;  a unit can also request info from
   its side when it is working on its individual plan. */

void
update_unit_plans(Side *side)
{
    Unit *unit;

    for_all_side_units(side, unit) {
	if (is_active(unit) && ai_controlled(unit)) {
	    ai_decide_plan(side, unit);
	}
    }
}

/* Randomly change a unit's plans.  (This is really more for
   debugging, exercising plan execution code in novel ways.) */

void
update_unit_plans_randomly(Side *side)
{
    Unit *unit;

    for_all_side_units(side, unit) {
	if (is_active(unit) && ai_controlled(unit)) {
	    if (probability(10)) {
		DMprintf("Randomly changed %s plan %s",
			 unit_desig(unit), plan_desig(unit->plan));
		net_set_unit_plan_type(side, unit, xrandom((int) NUMPLANTYPES));
		DMprintf("to plan %s\n", plan_desig(unit->plan));
	    }
	    /* (should add/remove goals randomly) */
	    if (probability(10)) {
		net_set_unit_reserve(side, unit, TRUE, FALSE);
	    }
	}
    }
}

/* This hook is triggered when certain actions are executed within the
   tactical range of our unit. */

static void
ai_react_to_action(Side *side, Unit *unit)
{
    /* Don't mess with units that are doing resupply or repair ... */
    if (unit->plan
	&& unit->plan->tasks
	&& (unit->plan->tasks->type == TASK_REPAIR
	     || unit->plan->tasks->type == TASK_RESUPPLY
	     /* ... or are on their way to resupply or repair. */
	     || (unit->plan->tasks->type == TASK_MOVE_TO
		&& unit->plan->tasks->next
		&& (unit->plan->tasks->next->type == TASK_REPAIR
		     || unit->plan->tasks->next->type == TASK_RESUPPLY)))) {
	return;
    }
    /* Don't mess with non-mobile units that are building something. */
    if (!mobile(unit->type) 
        && unit->plan
	&& unit->plan->tasks
	&& unit->plan->tasks->type == TASK_BUILD) {
	return;
    }
    /* First check that the unit's target (if it has one) is valid. */
    check_current_target(unit);
    /* Deal with tactical emergencies. */
    defensive_reaction(unit);
    offensive_reaction(unit);
}

/* Check that any offensive targets are still valid. Replan if not. */

static void
check_current_target(Unit *unit)
{
    int		x, y, u, target = FALSE, valid = FALSE;
    Plan	*plan = unit->plan;
    Side	*side = unit->side;
    UnitView 	*uview;
    Unit *unit2 = NULL, *unit3 = NULL;
    Task *agenda = NULL;
	
    agenda = plan->tasks;
    if (agenda && (agenda->type == TASK_MOVE_TO))
	agenda = agenda->next;
    /* Check if we are trying to hit or capture a unit. */
    if (agenda 
	&& ((agenda->type == TASK_HIT_UNIT) 
	    || (agenda->type == TASK_CAPTURE))) {
	unit3 = find_unit(agenda->args[0]);
	if (in_play(unit3)) {
	    target = TRUE;
	    x = unit3->x;
	    y = unit3->y;
	    u = unit3->type;
	}
    }
    /* If we have no target we are done. */
    if (!target)
	return;
    /* Check if we still can see the intended target in the assumed position. */
	if (side->see_all) {
	    for_all_stack_with_occs(x, y, unit2) {
		if (!trusted_side(side, unit2->side)
		    && unit2->type == u)
		    valid = TRUE;
	    }
	} else {
	    for_all_view_stack_with_occs(side, x, y, uview) {
		if (!trusted_side(side, side_n(uview->siden))
		    && uview->type == u)
		    valid = TRUE;
	    }
	}
	/* Blast the plan if we lack a valid target. */
	if (!valid) {
	    DMprintf("Forced replan for %s as target is no longer valid.\n", 
		     unit_desig(unit));
	    net_force_replan(unit);		
	}
}

/* Check that advanced units are adequately defended, but release any
   excess defenders. */

static void
defensive_reaction(Unit *unit)
{
    Plan *plan = unit->plan;
    Unit *unit2;

    /* First check if our unit is assigned to protecting another unit,
       and release it from that duty if it is no longer needed. This is 
       very important or all our units will soon be soaked up in this 
       kind of defense! */
    if (plan->maingoal 
        && plan->maingoal->type == GOAL_UNIT_OCCUPIED) {
	unit2 = find_unit(plan->maingoal->args[0]);
	/* Return if we are still busy with unit2. */
	if (keep_defensive_goal(unit, unit2))
	    return;
	/* We need a new plan and goal. */
	DMprintf("Forced replan: %s released from defending %s\n", 
		          unit_desig(unit), unit_desig(unit2));
	net_force_replan(unit);
    }
    /* Then check if our own transport needs defense. */
    if (maybe_defend_own_transport(unit))
        return;
    /* If not, check if any neigbour needs defense. */
    unit2 = undefended_neighbour(unit);
    if (unit2) {
	maybe_defend_other_transport(unit, unit2);
    }
}

/* Returns true if unit is busy defending or recapturing unit2, false
   if it needs a new goal. */

static int
keep_defensive_goal(Unit *unit, Unit *unit2)
{
    Plan *plan = unit->plan;
    int garrison = 0;
    Unit *unit3;
    Task *agenda = NULL;

    agenda = (plan ? plan->tasks : NULL);
    /* unit2 has been destroyed. */
    if (!in_play(unit2)) {
	return FALSE;
	/* We can't defend unit2 (should never happen). */
    } else if (!occ_can_defend_transport(unit->type, unit2->type)) {
	return FALSE;
	/* unit2 is held by the enemy. */
    } else if (enemy_side(unit->side, unit2->side) && agenda) {
	if (TASK_MOVE_TO == agenda->type)
	    agenda = agenda->next;
	if (agenda 
	    && ((TASK_HIT_UNIT == agenda->type)
		|| (TASK_CAPTURE == agenda->type))) {
	    unit3 = find_unit(agenda->args[0]);
	    if (!in_play(unit3))
		return FALSE;
	    if (unit2 == unit3)
		return TRUE;
	}
	/* Try to recapture unit2 directly if we have a decent chance. */
	if (real_capture_chance(unit, unit2) > 20) {
	    net_set_capture_task(
		unit, unit2->id, CAPTURE_METHOD_ANY, CAPTURE_TRIES_DFLT);
	    DMprintf("%s tries to recapture %s directly\n",
		     unit_desig(unit), unit_desig(unit2));
	    return TRUE;
	    /* Otherwise attack unit2 if possible. */
	} else if (attack_can_damage_or_capture(unit, unit2) 
		   || fire_can_damage_or_capture(unit, unit2)) {
	    net_set_hit_unit_task(
		unit, unit2->id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
	    DMprintf("%s tries to take %s back by attacking it\n",
		     unit_desig(unit), unit_desig(unit2));
	    return TRUE;
	}
	return FALSE;
    } else {
	/* Check if unit2 is adequately defended by OTHER occupants. */
	for_all_occupants(unit2, unit3) {
	    if (is_active(unit3)
		&& unit3 != unit
		&& unit3->plan
		&& unit3->plan->maingoal
		&& unit3->plan->maingoal->type == GOAL_UNIT_OCCUPIED
		&& unit3->plan->maingoal->args[0] == unit2->id
		/* Occs that cannot defend the transport should never
		   have been assigned to occupy it, but check
		   anyway. */
		&& occ_can_defend_transport(unit3->type, unit2->type)) {
		++garrison;
	    }
	}
	/* If so, release our unit from its duty. */
	if (enough_to_garrison(unit2, garrison)) {
	    return FALSE;
	/* Else make sure we have the occupy task set if we are
	   not there. */
	} else if (unit->transport != unit2
		   && plan->tasks
		   && plan->tasks->type != TASK_OCCUPY) {
	    net_set_occupy_task(unit, unit2);
	    return TRUE;
	    /* Else just keep defending unit2. */
	} else {
	    return TRUE;
	}			
    }
}

/* Returns true if we decide to defend it. */

static int
maybe_defend_own_transport(Unit *unit)
{
    Unit *unit2;
	  
    /* First assign our unit to protect its own city if needed. */
    unit2 = unit->transport;
    if (unit2 != NULL) {
	/* A city is a transport that can build. */
	if (u_can_make(unit2->type)
	/* u_can_make is much faster than can_build. */
	&& occ_can_defend_transport(unit->type, unit2->type)
	/* could_carry, which is called by occ_can_defend_transport,
	   is much faster than can_carry_type, which was used before. */
	&& !defended_by_occupants(unit2)) {
	assign_to_defend_unit(unit, unit2);
	DMprintf("%s assigned to defend own transport %s\n",
		 unit_desig(unit), unit_desig(unit2));
		return TRUE;
	}
    }
    return FALSE;
}

/* This code is twice as fast as the old code which used
   for_all_side_units. */

static Unit *
undefended_neighbour(Unit *unit)
{
    Unit *unit2;
    int x, y;

    for_all_cells_within_range(unit->x, unit->y, 
			       u_ai_enemy_alert_range(unit->type), x, y) {
	if (!inside_area(x, y))
	  continue;
	if (!terrain_visible(unit->side, x, y))
	  continue;
	for_all_stack(x, y, unit2) {
	    if (is_active(unit2)
		&& unit2->side == unit->side
		/* (u_can_make is much faster than can_build). */
		&& u_can_make(unit2->type)
		/* (could_carry, which is called by
		   occ_can_defend_transport, is much faster than
		   can_carry_type, which was used before). */
		&& occ_can_defend_transport(unit->type, unit2->type)
		&& !defended_by_occupants(unit2)) {
		return unit2;
	    }
	}
    }
    return NULL;
}

/* Returns true if we decide to defend it. */

static int
maybe_defend_other_transport(Unit *unit, Unit *unit2)
{
    Unit *unit3;
    int garrison = 0;

    /* Check if other defenders on their way are closer or same
       distance.  This prevents buzzing of flocks of defenders in and
       out of a city without adequate defense. */
    for_all_side_units(unit->side, unit3) {
	if (is_active(unit3)
	    /* Should never happen, but check. */
	    && unit3 != unit
	    && unit3->plan
	    && unit3->plan->maingoal
	    && unit3->plan->maingoal->type == GOAL_UNIT_OCCUPIED
	    && unit3->plan->maingoal->args[0] == unit2->id
	    && occ_can_defend_transport(unit3->type, unit2->type)
	    /* (should take unit speeds into consideration here). */
	    && (distance(unit2->x, unit2->y, unit3->x, unit3->y) <=
		distance(unit2->x, unit2->y, unit->x, unit->y))) {
	    /* This is really the potential garrison as soon as all
	       defenders get there. */
	    ++garrison;
	}							
    }
    /* If not enough defenders, assign our unit to the city's defense. */
    if (!enough_to_garrison(unit2, garrison)) { 
	assign_to_defend_unit(unit, unit2);
	DMprintf("%s assigned to defend neighbour transport %s\n",
		 unit_desig(unit), unit_desig(unit2));
	return TRUE;
    }				
    return FALSE;
}

/* Deal with tactical emergencies such as enemy units that suddenly
   appear nearby. */

static void
offensive_reaction(Unit *unit)
{
    Plan *plan = unit->plan;
    int range;
	
    /* Drop the current task and promptly attack any enemies within
       range.  This is very important or we will loose the tactical
       initiative! */

    /* But first check if we are assigned to defending another unit. */
    if (plan->maingoal
	&& plan->maingoal->type == GOAL_UNIT_OCCUPIED) {

	Unit *unit2 = find_unit(plan->maingoal->args[0]);
	/* Limit attack range to 1 if we are defending a unit. This
	   will in most cases assure that the defender stays inside
	   its protege, but an overrun action into an adjacent cell
	   may still cause the defender to stray outside. */
	if (unit->transport && unit->transport == unit2) {
	    range = 1;
	} else {
	    range = u_ai_tactical_range(unit->type);
	}
	/* Fall through: don't interfere if we are on our way to
           defend a city. */
	if (in_play(unit2)
	    && plan->tasks
	    && plan->tasks->type == TASK_OCCUPY
	    && plan->tasks->args[0] == unit2->id) {
	    return;
	}

    /* Also check if we are assigned to defending a specific cell. */
    } else if (plan->maingoal
	       && plan->maingoal->type == GOAL_CELL_OCCUPIED) {
	int x = plan->maingoal->args[0], y = plan->maingoal->args[1];

	/* Limit attack range to 1 in that case. */
	if (unit->x == x && unit->y == y) {
	    range = 1;
	} else {
	    range = u_ai_tactical_range(unit->type);
	}
	/* Fall through: don't interfere if we are on our way to
	   defend a cell. */
	if (inside_area(x, y)
	    && plan->tasks
	    && plan->tasks->type == TASK_MOVE_TO
	    && plan->tasks->args[0] == x
	    && plan->tasks->args[1] == y) {
	    return;
	}

    /* Use default tactical range in all other cases. */
    } else
      range = u_ai_tactical_range(unit->type);

    /* Fall-through: don't interfere if we already are in a fight. */
    if (plan->tasks
	&& (plan->tasks->type == TASK_HIT_UNIT
	    || plan->tasks->type == TASK_HIT_POSITION
	    || plan->tasks->type == TASK_CAPTURE
	    /* Don't forget about critical tasks. */
	    || plan->tasks->type == TASK_REPAIR
	    || plan->tasks->type == TASK_RESUPPLY
	    /* Or if we are chasing a victim. */
	    || (plan->tasks->type == TASK_MOVE_TO
		&& plan->tasks->next
		&& (plan->tasks->next->type == TASK_HIT_UNIT
		    || plan->tasks->next->type == TASK_HIT_POSITION
		    || plan->tasks->next->type == TASK_CAPTURE
		    /* Don't forget about critical tasks. */
		    || plan->tasks->next->type == TASK_REPAIR
		    || plan->tasks->next->type == TASK_RESUPPLY
		    ))))
      return;
    ai_go_after_victim(unit, range, TRUE);
}

extern int victim_id;
extern int victim_x, victim_y, victim_rating, victim_utype, victim_sidenum;
extern OccStatus victim_occstatus;
extern int victim_flags;
extern int victim_dmgtypes;

/* (Nasty hack. Should rearrange header stuff instead, or put 
    ai_go_after_victim in 'plan.c'.) */
extern void set_hit_unit_task(Unit *unit, int id, int hitmethod, int n);
extern void set_move_to_task(Unit *unit, int x, int y, int dist);
extern void push_move_to_task(Unit *unit, int x, int y, int dist);
extern void set_capture_task(Unit *unit, int id, int capmethod, int n);

int
ai_go_after_victim(Unit *unit, int range, int broadcast)
{
    int x = -1, y = -1, rslt = FALSE;
    int u1 = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL, *oside = NULL;
    int dist = 0, strikedist = 0, range2 = -1, amtavail = 0, rsplypct = 0;
    int m = NONMTYPE;

    u1 = unit->type;
    /* If we are immobile and cannot attack or fire, 
	then get out immediately. */
    if (!mobile(u1) && !could_attack_any(u1) && !could_fire_at_any(u1))
      return FALSE;
    /* If victim seeker makes a better transport than victimizer, 
       and it is now devoid of occs, then don't bother looking for 
       victims. */
    /*! \note This shunt probably belongs elsewhere, but it fine here for 
	      now, until some heavy improvement can be done on the AI. */
    if (u_is_transport(u1) && !unit->occupant
	&& (u_offensive_worth(u1) < u_defensive_worth(u1)))
      return FALSE;
    /* Clip the range based on a rough estimate of how much fuel we 
	actually have left. */
    /* (TODO: Should move range clipper to a generic function.) */
    range2 = area.maxdim;
    for_all_material_types(m) {
	if (0 >= um_consumption_per_move(u1, m))
	  continue;
	if (unit->side)
	  rsplypct = unit_doctrine(unit)->resupply_percent;
	else
	  rsplypct = 50;
	amtavail = unit->supply[m] - ((um_storage_x(u1, m) * rsplypct) / 100);
	if (amtavail < 0)
	  return FALSE;
	range2 = min(range2, 
		     (amtavail / um_consumption_per_move(u1, m)) * 
		     u_move_range(u1));
    }
    range = min(range, range2);
    /* Clip the range to the striking distance, if unit is on transport. */
    /* (TODO: Account for attack ranges > 1.) */
    if (unit->transport) {
	range2 = max((could_attack_any(u1) ? 1 : 0),
		     (could_fire_at_any(u1) ? u_range(u1) : 0));
	range = min(range, range2);
    }
    /* Perform search for a victim. */
    tmpunit = unit;
    DMprintf("%s (%s) seeking victim within radius %d... ",
	     unit_desig(unit), ai_type_name(unit->side->aitype), range);
    victim_rating = 0;
    rslt = limited_search_around(unit->x, unit->y, range, ai_victim_here, 
                                 &x, &y, 1, 5);
    run_ui_idler();
    if (victim_rating > 0) {
        rslt = TRUE;
        DMprintf("\n...and a victim is found!\n");
    }
    else {
        DMprintf("\n...and could not find one.\n");
        return FALSE;
    }
    u2 = victim_utype;
    side = unit->side;
    oside = side_n(victim_sidenum);
    x = victim_x; y = victim_y;
    dist = distance(unit->x, unit->y, x, y);
    strikedist = max((could_damage_by_fire(u1, u2) ?
                        type_ideal_fire_range_max(u1, u2) : 0),
                     (could_damage_by_attacks(u1, u2) ?
                        type_ideal_attack_range_max(u1, u2) : 1));
    if ((victim_flags & VICTIM_ENCOUNTERABLE) && (g_combat_model() == 1)
        && mobile(u1)) {
        if (broadcast) {
            net_set_hit_unit_task(
		unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
            net_push_move_to_task(unit, x, y, 1); 
        }
        else {
            set_hit_unit_task(
		unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
            push_move_to_task(unit, x, y, 1); 
        }
    }
    else if (victim_flags & VICTIM_CAPTURABLE) {
        if (g_combat_model() == 1) {
            if (broadcast) {
                net_set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  net_push_move_to_task(unit, x, y, 1);
            }
            else {
                set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  push_move_to_task(unit, x, y, 1);
            }
        }
        /* Should check to see whether direct capture or capture by hitting 
            is less expensive in terms of ACP, materials, etc.... */
        /* Should check to see if occs prevent direct capture. */
        else if (could_capture_by_attacks(u1, u2, oside) 
                || could_capture_by_fire(u1, u2, oside)) {
            if (broadcast) {
                net_set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  net_push_move_to_task(unit, x, y, 1);
            }
            else {
                set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  push_move_to_task(unit, x, y, 1);
            }
        }
        else if (could_capture_by_capture(u1, u2, oside) 
                 && (g_combat_model() == 0)) {
            if (broadcast) {
                net_set_capture_task(
		    unit, victim_id, CAPTURE_METHOD_ANY, CAPTURE_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  net_push_move_to_task(unit, x, y, 1);
            }
            else {
                set_capture_task(
		    unit, victim_id, CAPTURE_METHOD_ANY, CAPTURE_TRIES_DFLT);
                if (mobile(u1) && (dist > 1))
                  push_move_to_task(unit, x, y, 1);
            }
        }
        else {
            if (broadcast)
              net_set_move_to_task(unit, x, y, 1);
            else
              set_move_to_task(unit, x, y, 1);
        }
    } /* victim_flags & VICTIM_CAPTURABLE */
    else {
        if ((victim_dmgtypes & DAMAGE_TYPE_HIT)
            || (victim_flags & VICTIM_SHAKEABLE)) {
            if (broadcast) {
                net_set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (dist > strikedist)
                  net_push_move_to_task(unit, x, y, strikedist);
            }
            else {
                set_hit_unit_task(
		    unit, victim_id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
                if (dist > strikedist)
                  push_move_to_task(unit, x, y, strikedist);
            }
        }
        else if (mobile(u1)) {
            if (broadcast)
              net_set_move_to_task(unit, x, y, 0);
            else
              set_move_to_task(unit, x, y, 0);
        }
        else
          return FALSE;
    }
    return rslt;
}

#if (0)
/* The point of this new function is to limit the search for captives
   to a given range. */

static int
ai_go_after_captive(Unit *unit, int range)
{
    int x, y, rslt;

    tmpunit = unit;

    DMprintf("%s (%s) searching for useful capture within %d; found ",
	     unit_desig(unit), ai_type_name(unit->side->aitype), range);
    rslt = search_around(unit->x, unit->y, range, 
			 useful_captureable_here, &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
	net_set_capture_task(unit, x, y, tmputype, tmpside->id);
	if (unit->transport
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    net_set_move_to_task(unit->transport, x, y, 1);
	}
	return TRUE;
    } else {
	DMprintf("nothing\n");
	return FALSE;
    }
}

static int
ai_fire_at_opportunity(Unit *unit)
{
    int x, y, range, rslt;
    extern int target_x, target_y, target_rating, target_utype, target_sidenum;

    tmpunit = unit;
    range = u_range(unit->type);
    /* Look further for targets if we are mobile. */
    if (mobile(unit->type)) {
    	range += u_ai_tactical_range(unit->type);
    }
    target_rating = -9999;
    DMprintf("%s (%s) seeking target within %d; found ",
             unit_desig(unit), ai_type_name(unit->side->aitype), range);
    rslt = search_around(unit->x, unit->y, range, target_here, &x, &y, 1);
    if (rslt) {
	DMprintf("s%d %s at %d,%d\n", target_sidenum, 
                 u_type_name(target_utype), x, y);
	/* Set up a task to shoot at the unit found. */
	net_set_hit_unit_task(unit, x, y, target_utype, target_sidenum);
    } else if (target_rating > -9999) {
	DMprintf("s%d %s (rated %d) at %d,%d\n",
		 target_sidenum, u_type_name(target_utype), target_rating, 
                 x, y);
	/* Set up a task to shoot at the unit found. */
	net_set_hit_unit_task(unit, target_x, target_y, target_utype, 
                              target_sidenum);
    } else {
	DMprintf("nothing\n");
    }
    return rslt;
}
#endif

/* Return true if the unit can actually damage the other unit. */

int
attack_can_damage_or_capture(Unit *unit, Unit *unit2)
{
    Unit *occ;

    if (!alive(unit) || !alive(unit2))
      return FALSE;
    if (capture_chance(unit->type, unit2->type, unit2->side) > 0)
      return TRUE;
    /* (should check for right kind of ammo) */
    if (uu_hit(unit->type, unit2->type) <= 0)
      return FALSE;
    /* Also take into account if we can damage any occs. Differs from
       attack code in that only visible occs are considered. */
    /* Return TRUE if we can damage the unit itself. */
    if (uu_damage(unit->type, unit2->type) > 0)
      return TRUE;
    /* Else check if we can hit and damage any of its occs. */
    for_all_occs_with_occs(unit2, occ) {
	int immune = FALSE;
	Unit *transport = occ->transport;

	/* Skip occs that we can't hit or damage. */
	if (uu_hit(unit->type, occ->type <= 0)
	    || uu_damage(unit->type, occ->type <= 0))
	  continue;
	/* Skip occs that we cannot see (no cheating - not even for the AI!) */
	if (!side_sees_image(unit->side, occ))
	  continue;		    	
	/* For vulnerable occs, check recursively that we also can hit
	   all transports in the chain leading down to the occ. */
	while (transport) {
	    if (uu_hit(unit->type, transport->type) <= 0) {
		immune = TRUE;
		break;
	    }
	    transport = transport->transport;
	}
	/* If we found an occ that is not immune we are done. */
	if (!immune)
	  return TRUE;
    }    	
    /* We can't capture or hit anything. */
    return FALSE;
}

/* Return true if the unit can actually damage the other unit by
   firing at it. */

int
fire_can_damage(Unit *unit, Unit *unit2)
{
    Unit *occ;

    if (!alive(unit) || !alive(unit2))
      return FALSE;
    /* (should check for right kind of ammo) */
    if (fire_hit_chance(unit->type, unit2->type) <= 0)
      return FALSE;
    /* Also take into account if we can damage any occs. Differs from fire code
    in that only visible occs are considered. */
    /* Return TRUE if we can damage the unit itself. */
    if (fire_damage(unit->type, unit2->type) > 0)
	return TRUE;
    /* Else check if we can hit and damage any of its occs. */
    for_all_occs_with_occs(unit2, occ) {
	int 	immune = FALSE;
	Unit	*transport = occ->transport;

	/* Skip occs that we can't hit or damage. */
	if (fire_hit_chance(unit->type, occ->type) <= 0
	    || fire_damage(unit->type, occ->type) <= 0)
	    continue;
	/* Skip occs that we cannot see (no cheating - not even for the AI!) */
	if (!side_sees_image(unit->side, occ))
	  continue;		    	
	/* For vulnerable occs, check recursively that we also can 
	hit all transports in the chain leading down to the occ. */
	while (transport) {
	    if (fire_hit_chance(unit->type, transport->type) <= 0) {
		    immune = TRUE;
		    break;
	    }
	    transport = transport->transport;
	}
	/* If we found an occ that is not immune we are done. */
	if (!immune)
	  return TRUE;
    }    	
    /* We can't damage anything. */
    return FALSE;
}

int
fire_can_damage_or_capture(Unit *unit, Unit *unit2)
{
    if (!alive(unit) || !alive(unit2))
	return FALSE;
    /* Always return TRUE if we can capture the unit. */
    if (capture_chance(unit->type, unit2->type, unit2->side) > 0)
	return TRUE;
    /* Determine whether the fire can damage. */
    return fire_can_damage(unit, unit2);
}

/* Probability that one real unit will capture another real unit with
   protection taken into account.  Differs from real capture code in
   that only visible occupants are considered (no cheating!) */

int
real_capture_chance(Unit *unit, Unit *unit2)
{
    int chance, prot;
    int u2 = unit2->type;
    int u = unit->type;
    Unit *unit3;

    chance = capture_chance(unit->type, unit2->type, unit2->side);

    /* Occupants can protect the unit. */
    for_all_occupants(unit2, unit3) {
	if (is_active(unit3)
	    && side_sees_image(unit->side, unit3)) {
	    prot = uu_protection(unit3->type, u2);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    /* And so can its neighbours. */
    for_all_stack(unit2->x, unit2->y, unit3) {
	if (unit3 != unit2
	    && is_active(unit3)
	    && side_sees_image(unit->side, unit3)
	    && unit3->side == unit2->side) {
	    prot = uu_stack_protection(unit3->type, u2);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    for_all_stack_with_occs(unit2->x, unit2->y, unit3) {
	if (is_active(unit3)
	    && side_sees_image(unit->side, unit3)
	    /* We also extend protection to our buddies! */
	    && trusted_side(unit3->side, unit2->side)) {
	    /* This is when a unit, such as triple-A, provides unique
	       protection against a specific attacker, such as
	       bombers, to all other units in the cell. */
	    prot = uu_cellwide_protection_against(unit3->type, u);
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	    /* This is when a unit (such as a garrison) specifically
	       protects a second unit, such as a fort (but not other
	       nearby units), against all forms of attack. It thus
	       works the same way as uu_protection and
	       uu_stack_protection. */
	    prot = uu_cellwide_protection_for(unit3->type, u2);	    
	    if (prot != 100)
	      chance = (chance * prot) / 100;
	}
    }
    return chance;
}

/* Integrate total worth of an advance by recursively accounting for any 
   worth gained/lost by studying its precursor advances. */

static int
ai_integrate_advance_worth(Side *side, int a, int *numdeps)
{
    int a2 = NONATYPE, aval = 0;

    assert_error(side, "Attempted to access a NULL side");
    assert_error(is_advance_type(a), 
		 "Attempted to use an illegal advance type");
    if (side_can_research(side, a))
      return max(0, tmp_a_array[a]);
    if (has_advance(side, a))
      return 0;
    for_all_advance_types(a2) {
	if (aa_needed_to_research(a, a2)) {
	    if (has_advance(side, a2))
	      continue;
	    ++(*numdeps);
	    aval += ai_integrate_advance_worth(side, a2, numdeps);
	}
    }
    return aval;
}

/* Is the advance in question precluded by another advance? */

#if (0)
static int
is_precluded_advance(Side *side, int a)
{
    int a2 = NONATYPE;

    assert_error(side, "Attempted to manipulate a NULL side");
    assert_error(is_advance_type(a), 
		 "Attempted to use an illegal advance type");
    for_all_advance_types(a2) {
	if (aa_needed_to_research(a, a2)) {
	    if (aa_precludes(a2, a) && has_advance(side, a2))
	      return TRUE;
	    else {
		if (has_advance(side, a2))
		  return FALSE;
		else
		  return is_precluded_advance(side, a2);
	    }
	}
    }
    return FALSE;
}
#endif

/* Choose a research goal for a side. */

static void
ai_pick_side_research_goal(Side *side)
{
    int a = NONATYPE, a2 = NONATYPE;
    int u = NONUTYPE, aval = -1, uval = -1;
    int n = -1, ntot = 0, numdeps = 0;
    int *tmparray = NULL;
    Obj *newgoals = lispnil, *newgoal = lispnil;

    assert_error(side, "Attempted to manipulate a NULL side");
    /* If the side cannot research, then return. */
    if ((side == indepside) && (!g_indepside_can_research()
				|| !g_indepside_has_treasury()))
      return;
    /* If a research goal is already set, then simply return. */
    if (is_advance_type(side->research_goal) 
	&& !(side->research_precluded[side->research_goal]))
      return;
    /* If a set of initial advances is specified, then utilize it. */
    newgoals = g_ai_initial_research_goals();
    if (lispnil != newgoals) {
	if (consp(newgoals))
	  newgoal = 
	    choose_side_research_goal_from_weighted_list(newgoals, &ntot, side);
	else
	  newgoal = eval(newgoals);
    }
    if (lispnil != newgoal) {
	newgoal = eval(newgoal);
	if (atypep(newgoal)) {
	    if (is_advance_type(c_number(newgoal))) {
		side->research_goal = c_number(newgoal);
		return;
	    }
	    else
	      run_warning(
"Invalid research goal provided from list of initial research goals");
	}
	else
	  run_warning(
"Invalid research goal provided from list of initial research goals");
    }
    /* Prepare the advances scratch space. */
    memset(tmp_a_array, -1, numatypes * sizeof(int));
    /* Try to get a guesstimate of the worth of individual advances. */
    for_all_advance_types(a) {
	aval = 0;
	if (has_advance(side, a))
	  continue;
	if (side->research_precluded[a])
	  continue;
	for_all_advance_types(a2) {
	    if (aa_needed_to_research(a2, a))
	      ++aval;
	}
	for_all_unit_types(u) {
#if (0)
	    if (!type_ever_available(u, side))
	      continue;
#endif
	    uval = total_worth(u);
	    if (uval > 0) {
		aval += (uval * ua_needed_to_build(u, a));
		aval += (uval * ua_to_change_type(u, a));
	    }
	}
	/* (TODO: Consider boosts to materials production, etc....) */
	if (aval > 0) {
	    tmp_a_array[a] = aval;
	}
	else {
	    tmp_a_array[a] = -a_rp(a);
	}
    } /* for_all_advance_types */
    /* If cycle is present, then choose a research goal from the immediately 
       available advances only. */
    if (G_advances_graph_has_cycles) {
	for_all_advance_types(a) {
	    if (!side_can_research(side, a))
	      tmp_a_array[a] = 0;
	}
    }
    /* If no cycles, then integrate the worth of each advance. */
    else {
	tmparray = (int *)xmalloc(numatypes * sizeof(int));
	/* Integrate the worth of all dependent advances into each advance. */
	/* Also, rescale according distance from goal. */
	for_all_advance_types(a) {
	    numdeps = 0;
	    if (!(side->research_precluded[a]) && !has_advance(side, a))
	      tmparray[a] += ai_integrate_advance_worth(side, a, &numdeps);
	    if ((tmparray[a] > 0) && numdeps)
		tmparray[a] = (tmparray[a] / numdeps) + 1;
	}
	memcpy(tmp_a_array, tmparray, numatypes * sizeof(int));
	free(tmparray);
    }
    /* Knock intermediate advances out of contention. Only forking branches 
       and leaves of the tree should remain. */
    if (!G_advances_graph_has_cycles) {
    for_all_advance_types(a) {
	numdeps = 0;
	for_all_advance_types(a2) {
	    if (aa_needed_to_research(a2, a)) {
#if (0)
		++numdeps;
		break;
#endif
		if (numdeps > 1)
		  break;
	    }
	}
	/* Ignore leaves (numdeps == 0) or forks (numdeps > 1). */
	if (1 == numdeps)
	  tmp_a_array[a] = 0;
#if (0)
	if (numdeps)
	  tmp_a_array[a] = 0;
#endif
    }
    }
    /* Randomly select a research goal from weighted list. */
    ntot = 0;
    for_all_advance_types(a) {
	if (tmp_a_array[a] > 0)
	  ntot += tmp_a_array[a];
    }
    n = xrandom(ntot);
    ntot = 0;
    for_all_advance_types(a) {
	if (tmp_a_array[a] > 0) {
	    ntot += tmp_a_array[a];
	    if (ntot > n) {
		side->research_goal = a;
		return;
	    }
	}
    }
    Dprintf("Could not find a research goal for side %d.\n", side->id);
}

/* Determine if researching a particular topic leads to a given research 
   goal. */

static int
advance_leads_to_goal(Side *side, int a)
{
    int agoal = NONATYPE;

    assert_error(side, "Attempted to manipulate a NULL side");
    assert_error(is_advance_type(a), 
		 "Attempted to use an illegal advance type");
    assert_warning_return(is_advance_type(side->research_goal),
			  "Cannot seek unset advance goal", FALSE);
    if (G_advances_graph_has_cycles)
      return FALSE;
    agoal = side->research_goal;
    if (a == agoal)
      return TRUE;
    if (side->research_precluded[a])
      return FALSE;
    return get_packed_bool(G_advances_synopsis, agoal, a);
}

/* Pick a research topic. First set a resarch goal, if necessary. */

void
ai_pick_side_research(Side *side)
{
    int a = NONATYPE;

    assert_error(side, "Attempted to manipulate a NULL side");
    /* If something is already being researched, then continue with it. */
    if (is_advance_type(side->research_topic))
      return;
    /* Reset a research goal that may have been precluded for some reason. */
    if (is_advance_type(side->research_goal) 
	&& side->research_precluded[side->research_goal])
      side->research_goal = NONATYPE;
    /* Choose a research goal if one is not already in place. */
    if (!is_advance_type(side->research_goal)) 
      ai_pick_side_research_goal(side);
    /* Work towards the research goal, if one was chosen. */
    if (is_advance_type(side->research_goal)) {
	for_all_advance_types(a) {
	    if (side_can_research(side, a) 
		&& ((G_advances_graph_has_cycles && (a == side->research_goal))
		    || advance_leads_to_goal(side, a))) {
		side->research_topic = a;
		return;
	    }
	}
	Dprintf("Could not research towards goal for side %d.\n", side->id);
	side->research_goal = NONATYPE;
    }
    /* If no research goal or could not find an advance to get to the goal, 
       then pick first researchable advance. */
    for_all_advance_types(a) {
	if (side_can_research(side, a)) {
	    side->research_topic = a;
	    return;
	}
    }
}

/* Pick a good research topic for an AI-controlled side. */

static void
ai_plan_research(Side *side)
{
    ai_pick_side_research(side);
    net_set_side_research_topic(side, side->research_topic);
    net_set_side_research_goal(side, side->research_goal);
}

/* Return true if the given type should be used to collect the given
   type of material. */

int
need_this_type_to_collect(Side *side, int u, int m)
{
    return (strcmp(u_type_name(u), "villager") == 0);
}

int
mp_collect_here(int x, int y)
{
    int m2;

    if (can_extract_at(tmpunit, x, y, &m2)) {
	if (m2 == tmpmtype)
	  return TRUE;
    }
    if (can_load_at(tmpunit, x, y, &m2)) {
	if (m2 == tmpmtype)
	  return TRUE;
    }
    return FALSE;
}

/* Assign the given unit to the collection of the given type of
   material. */

void
assign_to_collection(Side *side, Unit *unit, int m)
{
    int x, y;

    tmpunit = unit;
    tmpmtype = m;
    search_around(unit->x, unit->y, 10, mp_collect_here, &x, &y, 1);
    net_set_unit_plan_type(side, unit, PLAN_IMPROVING);
    net_set_collect_task(unit, m, x, y);
    DMprintf("%s assigned to collecting %s at %d,%d\n",
	     unit_desig(unit), m_type_name(m), x, y);
}

/* Unit's goal in life will be to improve the world...
    ...or, at least itself. */

void
assign_to_improve(Side *side, Unit *unit)
{
    Goal *goal = NULL;

    goal = create_goal(GOAL_COLONIZE, side, TRUE);
    goal->args[0] = unit->type;
    net_set_unit_plan_type(side, unit, PLAN_IMPROVING);
    net_set_unit_main_goal(side, unit, goal);
    DMprintf("%s assigned to improve\n", unit_desig(unit));
}

void
assign_to_defense(Side *side, Unit *unit)
{
    Theater *theater;
    Goal *goal;

    net_set_unit_plan_type(side, unit, PLAN_DEFENSIVE);
    theater = theater_at(side, unit->x, unit->y);
    set_unit_theater(unit, theater);
    if (theater != NULL) {
	++(theater->numassigned[unit->type]);
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	goal->args[0] = theater->x;  goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(side, unit, goal);
	DMprintf("%s now assigned to defense in %s\n",
		 unit_desig(unit), theater->name);
    } else {
	DMprintf("%s now assigned to defense in no theater\n",
		 unit_desig(unit));
    }
}

#if 0

/* Assign unit to defend theater. Does NOT change the plan type. */

void
assign_to_defend_theater(Unit *unit, Theater *theater)
{
    	Goal 	*goal;

	/* Clear the task aganda. */
	net_clear_task_agenda(unit->side, unit);

	/* Set main goal to hold the vicinity of (x, y). */
	goal = create_goal(GOAL_VICINITY_HELD, unit->side, TRUE);
	goal->args[0] = theater->x;  
	goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(unit->side, unit, goal);
	DMprintf("%s now assigned to defense in %s\n",
		 unit_desig(unit), theater->name);
}
#endif

/* Assign unit to defend unit2. Does NOT change the plan type. */

void
assign_to_defend_unit(Unit *unit, Unit *unit2)
{
    Goal *goal;

    net_set_unit_plan_type(unit->side, unit, PLAN_DEFENSIVE);
    /* Set main goal to occupy unit2. */
    goal = create_goal(GOAL_UNIT_OCCUPIED, unit->side, TRUE);
    goal->args[0] = unit2->id;
    net_set_unit_main_goal(unit->side, unit, goal);
    DMprintf("%s assigned to occupy %s\n",
	     unit_desig(unit), unit_desig(unit2));
}

#if 0
/* Assign unit to defend cell at (x, y). */

void
assign_to_defend_cell(Unit *unit, int x, int y)
{
    	Goal 	*goal;

	net_set_unit_plan_type(unit->side, unit, PLAN_DEFENSIVE);
	/* Set main goal to occupy (x, y). */
	goal = create_goal(GOAL_CELL_OCCUPIED, unit->side, TRUE);
	goal->args[0] = x;
	goal->args[1] = y;
	net_set_unit_main_goal(unit->side, unit, goal);
	DMprintf("%s assigned to occupy cell at (%d, %d)\n",
	 		unit_desig(unit), x, y);
}
#endif

#if 0
/* Assign unit to defend the vicinity of (x, y). Does NOT change the
   plan type. */

void
assign_to_defend_vicinity(Unit *unit, int x, int y, int w, int h)
{
    	Goal 	*goal;

	/* Clear the task aganda. */
	net_clear_task_agenda(unit->side, unit);

	/* Set main goal to hold the vicinity of (x, y). */
	goal = create_goal(GOAL_VICINITY_HELD, unit->side, TRUE);
	goal->args[0] = x;  
	goal->args[1] = y;
	goal->args[2] = w;
	goal->args[3] = h;
	net_set_unit_main_goal(unit->side, unit, goal);
	DMprintf("%s now assigned to defense the vicinity of (%d, %d)\n",
		 unit_desig(unit), x, y);
}
#endif

int
compare_weights(CONST void *w1, CONST void *w2)
{
    return (((struct weightelt *) w2)->weight
	    - ((struct weightelt *) w1)->weight);
}

/* Return true if the given unit type has a chance to reach a position
   where it can view the given location, for instance an adjacent cell
   with safe terrain. */

static int
probably_explorable(Side *side, int x, int y, int u)
{
    int dir, x1, y1, tview, t;

    if (UNSEEN != terrain_view(side, x, y))
      return FALSE;
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    tview = terrain_view(side, x1, y1);
	    if (tview == UNSEEN)
	      return TRUE;
	    t = vterrain(tview);
	    if (could_be_on(u, t) && could_live_on(u, t))
	      return TRUE;
	}
    }
    return FALSE;
}

/* Set the unit up as an explorer and let it go. */

void
assign_to_exploration(Side *side, Unit *unit)
{
    int numweights = 0, weight, i, dist;
    struct weightelt weights[MAXTHEATERS];
    Theater *theater;

    /* Unit's goal in life will be to see that the world is all known. */
    net_set_unit_plan_type(side, unit, PLAN_EXPLORATORY);
    set_unit_theater(unit, NULL);
    /* Find the theater most in need of exploration. */
    for_all_theaters(side, theater) {
    	if (theater->size > 0 && theater->unexplored > 0) {
	    /* Weight by percentage of theater that is unknown. */
	    weight = (100 * theater->unexplored) / theater->size;
	    /* Downrate theaters that are far away. */
	    dist = distance(unit->x, unit->y, theater->x, theater->y)
	      - isqrt(theater->size) / 2;
	    if (dist < 0)
	      dist = 0;
	    weight /= max(1, (4 * dist) / area.maxdim);
	    /* Uprate the home front by a lot - it will become
	       completely known quickly, and then effectively drop out
	       of the list. */
	    if (theater == ai(side)->homefront)
	      weight *= 4;
	    /* Flatten out 10% variations, thus giving equal weight to
	       theaters of similar value. */
	    weight = 10 * (weight / 10);
	    weights[numweights].weight = weight;
	    weights[numweights].data = theater->id;
	    ++numweights;
    	}
    }
    if (numweights > 0) {
    	qsort(weights, numweights, sizeof(struct weightelt), compare_weights);
    	/* Choose randomly among theaters of equal weight. */
    	for (i = 0; i < numweights; ++i)
	  if (weights[i].weight < weights[0].weight)
	    break;
    	theater = ai(side)->theatertable[weights[xrandom(i)].data];
    } else {
    	theater = NULL;
    }
    assign_explorer_to_theater(side, unit, theater);
}

/* For a given unit in a given theater, find a location that it ought to
   try to explore.  Start by choosing random points in the theater, then
   switch to exhaustive search if necessary. */

void
assign_explorer_to_theater(Side *side, Unit *unit, Theater *theater)
{
    int x, y, w, h, sq, tries, found;
    Goal *goal;

    /* Undo any existing assignment.  Note that we always undo, even if
       new theater assignment fails, so that unit continues to be free
       for something else. */
    if (unit_theater(unit) != NULL) {
	--(unit_theater(unit)->numassigned[unit->type]);
	set_unit_theater(unit, NULL);
	/* (should release existing main goal?) */
    }    
    /* Try to find an unexplored cell in the given theater. */
    /* (TODO: Look for other explorers assigned to the theater, 
	and explore a random cell no less than a certain distance from them?) */
    if (theater != NULL) {
	tries = theater->size * 2;
	found = FALSE;
	while (tries-- > 0) {
	    /* Select a random point within the theater. */
	    x = theater->xmin;  y = theater->ymin;
	    x += (xrandom(theater->xmax - theater->xmin)
		  + xrandom(theater->xmax - theater->xmin)) / 2;
	    y += (xrandom(theater->ymax - theater->ymin)
		  + xrandom(theater->ymax - theater->ymin)) / 2;
	    /* See if the point is of any interest. */
	    if (inside_area(x, y)
		&& theater_at(side, x, y) == theater
		&& terrain_view(side, x, y) == UNSEEN
		&& probably_explorable(side, x, y, unit->type)) {
		found = TRUE;
		break;
	    }
	}
	if (!inside_area(x, y)) {
	    /* Now try iterating over entire theater. */
	    for (x = theater->xmin; x <= theater->xmax; ++x) {
		for (y = theater->ymin; y <= theater->ymax; ++y) {
		    /* See if the point is of any interest. */
		    if (inside_area(x, y)
			&& theater_at(side, x, y) == theater
			&& terrain_view(side, x, y) == UNSEEN
			&& probably_explorable(side, x, y, unit->type)) {
			found = TRUE;
			break;
		    }
		}
		if (found)
		  break;
	    }
	}
	if (!found) {
	    /* (should indicate failure in unit data somehow?) */
	    DMprintf("%s did not find anything to explore in %s\n",
		     unit_desig(unit), theater->name);
	    return;
	}
	set_unit_theater(unit, theater);
	++(theater->numassigned[unit->type]);
	/* Create a goal of having the whole vicinity known. */
	goal = create_goal(GOAL_VICINITY_KNOWN, side, TRUE);
	goal->args[0] = x;  goal->args[1] = y;
	sq = isqrt(theater->size);
	/* (should clip to theater boundary?) */
	w = h = sq / 2;
	goal->args[2] = w;  goal->args[3] = h;
	net_set_unit_main_goal(side, unit, goal);
	DMprintf("%s now assigned to explore in %s, %dx%d area around %d,%d\n",
		 unit_desig(unit), theater->name, w, h, x, y);
    }
}

/* Explorer constructors concentrate on building types that are good for
   exploration. */

void
assign_to_explorer_construction(Side *side, Unit *unit)
{
    /* Unit's goal in life will be to help see that the world is all known. */
    net_set_unit_plan_type(side, unit, PLAN_EXPLORATORY);
    DMprintf("%s assigned to explorer construction\n", unit_desig(unit));
}

void
assign_to_offense(Side *side, Unit *unit)
{
    int numweights = 0, weight;
    struct weightelt weights[MAXTHEATERS];
    Goal *goal;
    Theater *homefront, *theater;
    int w = -1, h = -1;

    net_set_unit_plan_type(side, unit, PLAN_OFFENSIVE);
    /* If there is a target, then deal with it before any goals. */
    if (ai_go_after_victim(unit, u_ai_tactical_range(unit->type), TRUE))
      return;
    /* If our home area is being threatened, assign the unit to it. */
    homefront = ai(side)->homefront;
    if (homefront != NULL && homefront->enemystrengthmin > 0) {
	set_unit_theater(unit, homefront);
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	goal->args[0] = homefront->x;  goal->args[1] = homefront->y;
	goal->args[2] = goal->args[3] = isqrt(homefront->size);
	net_set_unit_main_goal(side, unit, goal);
	DMprintf("%s assigned to offensive in the home front\n",
		 unit_desig(unit));
	return;
    }
    /* If the theater the unit is currently in is being threatened,
       assign the unit to it. */
    /* (should just increase it weight in next calculation?) */
    theater = theater_at(side, unit->x, unit->y);
    if (theater != NULL && theater->enemystrengthmin > 0) {
	set_unit_theater(unit, theater);
#if (0)
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	/* (should randomize?) */
	goal->args[0] = theater->x;  goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(side, unit, goal);
#else
	w = (theater->xmax - theater->xmin) / 2;
	h = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(side, unit, NULL);
	ai_go_after_victim(unit, min(w, h), TRUE);
#endif
	DMprintf("%s assigned to offensive in the theater where it's at now\n",
		 unit_desig(unit));
	return;
    }
    for_all_theaters(side, theater) {
    	if (theater->enemystrengthmin > 0 || theater->unexplored > 0) {
	    /* (should weight by strength relative to own units already 
		there) */
	    weight = theater->enemystrengthmax * 20;
	    /* Prefer not to send unit to farther-away theaters. */
	    if (distance(unit->x, unit->y, theater->x, theater->y) > 
		area.maxdim / 2) {
		weight /= 2;
	    }
	    /* Prefer theaters with more unknown territory. */
	    weight += (10 * theater->unexplored) / max(1, theater->size);
	    weights[numweights].weight = weight;
	    weights[numweights].data = theater->id;
	    ++numweights;
    	}
    }
    if (numweights > 0) {
    	qsort(weights, numweights, sizeof(struct weightelt), compare_weights);
    	theater = ai(side)->theatertable[weights[0].data];
    } else {
    	theater = theater_at(side, unit->x, unit->y);
    }
    set_unit_theater(unit, theater);
    if (theater != NULL) {
	++(theater->numassigned[unit->type]);
#if (0)
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	/* (should randomize?) */
	goal->args[0] = theater->x;  goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(side, unit, goal);
#else
	w = (theater->xmax - theater->xmin) / 2;
	h = (theater->ymax - theater->ymin) / 2;
	net_set_unit_main_goal(side, unit, NULL);
	net_set_move_to_task(unit, theater->x, theater->y, min(w, h));
#endif
	DMprintf("%s now assigned to offensive in %s",
		 unit_desig(unit), theater->name);
	if (numweights > 1) {
	    DMprintf(" (weight %d; runnerup was %s, weight %d)",
		     weights[0].weight,
		     (ai(side)->theatertable[weights[1].data])->name,
		     weights[1].weight);
	}
	DMprintf("\n");
    } else {
	DMprintf("%s now assigned to offensive in no theater\n",
		 unit_desig(unit));
    }
}

void
assign_to_offense_support(Side *side, Unit *unit)
{
    net_set_unit_plan_type(side, unit, PLAN_OFFENSIVE);
}

#if (0)
void
assign_to_colonization_support(Side *side, Unit *unit)
{
    net_set_unit_plan_type(side, unit, PLAN_COLONIZING);
}
#endif

void
assign_to_defense_support(Side *side, Unit *unit)
{
    net_set_unit_plan_type(side, unit, PLAN_DEFENSIVE);
}

/* For the given side and unit and plan, calculate the right type of
   unit to build. */

static int *pbt_prefs;
static int *pbt_fringe_terrain;
static int *pbt_enemy_types;
static int *pbt_num_to_transport;

static int type_can_leave_unit(int u, Unit *unit);

static int
type_can_leave_unit(int u, Unit *unit)
{
	int x = unit->x, y = unit->y, x1, y1, dir;
	int mpcost;
	
	for_all_directions(dir) {
		if (interior_point_in_dir(x, y, dir, &x1, &y1)
		    && type_survives_in_cell(u, x1, y1)
		    /* Blocking occupants may leave before we are done. */
		    && type_can_occupy_empty_cell(u, x1, y1)) {
			/* Now calculate the move cost. */
			mpcost = total_move_cost(u, unit->type, x, y, 0, x1, y1, 0);
			/* Check if we can have enough mps. */
			if (type_can_have_enough_mp(u, mpcost)) {
				return TRUE;
				break;
			}
		}
	}
	return FALSE;
}

int
suitable_port(Unit *unit)
{
    int x1, y1, dir, u = unit->type;
    
    for_all_directions(dir) {
	if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)
	    && t_liquid(terrain_at(x1, y1))
	    && aref(area.landsea_regions, x1, y1)->size 
	    		>= u_minimal_sea_for_docks(u)) {
	    	return TRUE;
	    	break;
	}
    }
    return FALSE;
}

int
preferred_build_type(Side *side, Unit *unit, int plantype)
{
    int u = unit->type, u2, u3, t, found = FALSE, canbuildtransport = FALSE;
    int x, y, dir, x1, y1, est, rslt, sumfringe, totfringe;
    Unit *unit2, *occ;
    UnitView *uview;
    Theater *theater;

    if (pbt_prefs == NULL)
      pbt_prefs = (int *) xmalloc(numutypes * sizeof(int));
    if (pbt_fringe_terrain == NULL)
      pbt_fringe_terrain = (int *) xmalloc(numttypes * sizeof(int));
    if (pbt_enemy_types == NULL)
      pbt_enemy_types = (int *) xmalloc(numutypes * sizeof(int));
    if (pbt_num_to_transport == NULL)
      pbt_num_to_transport = (int *) xmalloc(numutypes * sizeof(int));
    if (plantype == PLAN_EXPLORATORY) {
	/* Calculate the amount of each type of terrain at the edges
	   of the known world. */
	for_all_terrain_types(t)
	  pbt_fringe_terrain[t] = 0;
	for_all_cells(x, y) {
	    if (terrain_view(side, x, y) != UNSEEN) {
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)
			&& terrain_view(side, x1, y1) == UNSEEN) {
			++(pbt_fringe_terrain[(int) terrain_at(x, y)]);
			break;
		    }
		}
	    }
	}
    } else {
	/* should use estimated strengths instead? */
    	for_all_unit_types(u2)
    	  pbt_enemy_types[u2] = 0;
    	for_all_interior_cells(x, y) {
	    if (side->see_all) {
		/* Use new recursive macro instead that also sees occs
                   within occs. */
		for_all_stack_with_occs(x, y, unit2) {
		    if (!trusted_side(side, unit2->side))
		      ++pbt_enemy_types[unit2->type];
		}
	    } else {
		for_all_view_stack_with_occs(side, x, y, uview) {
		    if (!trusted_side(side, side_n(uview->siden)))
		      ++pbt_enemy_types[uview->type];
		}
	    }
    	}
    }
    /* Calculate a basic preference for each possible type. */
    for_all_unit_types(u2) {
	pbt_prefs[u2] = 0;
	est = est_completion_time(unit, u2);
	if (could_create(u, u2)
	    && est >= 0
	    && side_can_build(side, u2)) {
	    if (0 /* any demand in this unit's own theater */) {
	    } else if (need_more_transportation(side)) {
    		for_all_unit_types(u3) {
		    pbt_num_to_transport[u3] = 0;
    		}
	    	for_all_theaters(side, theater) {
		    for_all_unit_types(u3) {
			pbt_num_to_transport[u3] += theater->numtotransport[u3];
		    }
	    	}
    		for_all_unit_types(u3) {
		    if (pbt_num_to_transport[u3] > 0
			&& mobile(u2)
			&& could_carry(u2, u3)) {
			pbt_prefs[u2] += pbt_num_to_transport[u3];
			canbuildtransport = TRUE;
		    }
	    	}
	     }
	}
    }
    /* Check if any transports that we can build also can be used here. */
    if (canbuildtransport) {
	for_all_unit_types(u2) {
	    if (pbt_prefs[u2] > 0) {
		/* First check that we can at all leave the unit. */
		if (type_can_leave_unit(u2, unit)) {
		    /* Then check that any naval transports that we plan to
			build have more than just the city pond to swim in. */
		    if (u_air_mobile(u2)
			|| u_ground_mobile(u2)
			|| (u_naval_mobile(u2) 
			    && suitable_port(unit))) {
			    found = TRUE;
		    } else {
			pbt_prefs[u2] = 0;
		    }
		} else {
		    pbt_prefs[u2] = 0;
		}
	    }   
	}
    }
    if (!found) {
	for_all_unit_types(u2) {
	    pbt_prefs[u2] = 0;
	    est = est_completion_time(unit, u2);
	    if (could_create(u, u2)
		&& est >= 0
		&& side_can_build(side, u2)) {
		/* Prefer units by overall suitability for general plan. */
		if (plantype == PLAN_EXPLORATORY) {
		    /* Weight unit types by suitability for exploration around
		       the edges of the known area. */
		    sumfringe = totfringe = 0;
		    for_all_terrain_types(t) {
			totfringe += pbt_fringe_terrain[t];
			if (!terrain_always_impassable(u2, t))
			  sumfringe += pbt_fringe_terrain[t];
		    }
		    if (totfringe < 1)
		      sumfringe = totfringe = 1;
		    /* Scale - so 5% diffs in amt of crossable terrain
		       don't affect result. */
		    pbt_prefs[u2] = (20 * sumfringe) / totfringe;
		    /* Prefer types that are quicker to build. */
		    pbt_prefs[u2] /= max(1, est / 8);
		} else if (plantype == PLAN_IMPROVING) {
		    /* Eliminate all mobile units. */
		    if (mobile(u2))
		      pbt_prefs[u2] = 0;
		} else {
		    /* Weight unit type by effectiveness against known
                       enemies. */
		    for_all_unit_types(u3) {
			if (pbt_enemy_types[u3] > 0) {
			    if (uu_zz_bhw(u2, u3) > 0) {
				pbt_prefs[u2] +=
				  ((uu_zz_bhw(u2, u3) * 100) / bhw_max) * 
				    pbt_enemy_types[u3];
			    }
			    if (uu_zz_bcw(u2, u3) > 0) {
				pbt_prefs[u2] += 
				    uu_zz_bcw(u2, u3) * pbt_enemy_types[u3];
			    }
			}
		    }
		    /* Prefer types that are quicker to build. */
		    pbt_prefs[u2] /= max(1, est / 8);
		}
		if (pbt_prefs[u2] < 1)
		  pbt_prefs[u2] = 1;
	    }
	}
	/* Check that mobile units can leave. */
	for_all_unit_types(u2) {
	    if (pbt_prefs[u2] > 0 
		&& mobile(u2)
		/* We may want to build mobile defenders,
		     even if they cannot leave. */
		&& plantype != PLAN_DEFENSIVE) {
		/* First check that we can at all leave the unit. */
		if (type_can_leave_unit(u2, unit)) {
		    /* Then check that any naval units that we plan to
		    build have more than just the city pond to swim in. */
		    if (u_air_mobile(u2)
			|| u_ground_mobile(u2)
			|| (u_naval_mobile(u2) 
			    && suitable_port(unit))) {
			    found = TRUE;
		    } else {
			pbt_prefs[u2] = 0;
		    }
		} else {
		    pbt_prefs[u2] = 0;
		}
	    }   
	}
    }
    DMprintf("%s preferred build type is ", unit_desig(unit));
    /* Look for an existing incomplete occupant and prefer to build its type,
       if it is in the choices in the typelist. */
    for_all_occupants(unit, occ) {
	if (in_play(occ) && !completed(occ)) {
	    if (pbt_prefs[occ->type] > 0 && flip_coin()) {
		rslt = occ->type;
		DMprintf("%s (incomplete occupant)\n", u_type_name(rslt));
		return rslt;
	    }
	}
    }
    for_all_unit_types(u2)
      if (pbt_prefs[u2] < 0)
        pbt_prefs[u2] = 0;
    rslt = select_by_weight(pbt_prefs, numutypes);
    if (!is_unit_type(rslt))
      rslt = NONUTYPE;
    if (DebugM) {
	if (is_unit_type(rslt)) {
	    DMprintf("%s (choices were", u_type_name(rslt));
	    for_all_unit_types(u2) {
		if (pbt_prefs[u2] > 0)
		  DMprintf(" %s,%d", utype_name_n(u2, 1), pbt_prefs[u2]);
	    }
	    DMprintf(")");
	} else {
	    DMprintf("nothing (no choices)");
	}
	DMprintf("\n");
    }
    return rslt;
}

int
need_more_transportation(Side *side)
{
    int u3, u2, anytransport;
    Theater *theater;

    for_all_theaters(side, theater) {
	for_all_unit_types(u3) {
	    if (theater->numtotransport[u3] > 0) {
		anytransport = FALSE;
		for_all_unit_types(u2) {
		    if (theater->numassigned[u2] > 0
			&& mobile(u2)
			&& could_carry(u2, u3))
		      anytransport = TRUE;
		}
		if (!anytransport)
		  return TRUE;
	    }
	}
    }
    return FALSE;
}

int
need_explorers(Side *side)
{
    int s, numcontacted = 0, numfound = 0;

    if (side->see_all)
      return FALSE;
    if (g_terrain_seen())
      return FALSE;
    for (s = 1; s <= numsides; ++s) {
        if (s == side->id)
          continue;
        if (ai(side)->contacted[s])
          ++numcontacted;
        if (ai(side)->homefound[s])
          ++numfound;
    }
    if (numcontacted == 0) {
        /* If we've haven't found anybody, always explore. */
        return TRUE;
    } else if (numfound == 0) {
        /* If we've made contact but haven't found their home base,
           we still need to explore. */
        return TRUE;
    } else if (numfound < numsides - 1) {
        /* If we haven't found everybody's home base,
            we still need to explore. */
        return probability(max(100, 20 + (numfound * 100) / numsides));
    } else {
        /* If everybody has been found, then we likely have more
           pressing concerns; don't do as much exploration. */
        return probability(20);
    }
}

#if 0
/* Decide for the unit whether it should build a base to help other units. */

int
build_base_for_others(side, unit)
Side *side;
Unit *unit;
{
    int u = unit->type, u2, cando = FALSE;

    for_all_unit_types(u2) {
	if (uu_acp_to_create(u, u2) > 0
	    && (uu_creation_cp(u, u2) >= u_cp(u2)
	    && side_can_build(side, u2)
	        || uu_acp_to_build(u, u2) > 0)
	    && u_is_base(u2)
	    /* (should check if any advantage to building) */
	    ) {
	    cando = TRUE;
	    break;
	}
    }
    if (cando) {
	DMprintf("%s building %s as a base for others\n",
		 unit_desig(unit), u_type_name(u2));
	net_set_build_task(unit, u2, 1, 0, 0);
	return TRUE;
    }
    return FALSE;
}
#endif

int
build_depot_for_self(Unit *unit)
{
    int u = unit->type, u2, cando = FALSE;

    for_all_unit_types(u2) {
        if (valid(can_construct(unit, unit, u2))) {
            cando = TRUE;
            break;
        }
    }
    if (cando) {
	DMprintf("%s building %s as a depot for itself\n",
		 unit_desig(unit), u_type_name(u2));
	net_set_construct_task(unit, u2, 1, -1, unit->x, unit->y);
	return TRUE;
    }
    return FALSE;
}

int
can_develop_on(int u, int u2)
{
    int acp_res = uu_acp_to_develop(u, u2);

    if (acp_res < 1 || acp_res > type_max_acp(u))
      return 0;

    return 1;
}

/* code specific to the "time" game */
int 
needs_develop (Side *side, int u)
{
    int u2, i;

    if (game_class != gc_time)
      return 0;

    if (side->tech[u] >= u_tech_to_build(u) ||
	u_tech_max(u)  > u_tech_to_build(u))
      return 0;
    
    i = 0;
    for_all_unit_types(u2) {
	i += (could_create(u, u2) ? 1 : 0);
    }
    if (i < 2)
      return 0;

    return 1;
}

int
assign_to_develop_on(Side *side, Unit *unit, int u2)
{
    int lev = u_tech_to_build(u2);

    if (!can_develop_on(unit->type, u2)) {
	DMprintf("%s cannot develop on %s!\n",
	     unit_desig(unit), u_type_name(u2));
	return 0;
    }

    DMprintf("%s will develop for %s on %s (to level %d)\n",
	     unit_desig(unit), side_desig(side), u_type_name(u2), lev);

    net_set_unit_plan_type(side, unit, PLAN_IMPROVING);
    net_push_develop_task(unit, u2, lev);
    return 1;
}

/* (should account for impassability because of borders, etc) */

int
desired_direction_impassable(Unit *unit, int x, int y)
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t, numbaddirs = 0;

    numdirs = choose_move_dirs(unit, x, y, TRUE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	if (terrain_always_impassable(unit->type, t))
	  ++numbaddirs;
    }
    return (numbaddirs == numdirs);
}

/* Return true if a given unit could reach a given location by being
   carried on a transport of some sort. */

int
could_be_ferried(Unit *unit, int x, int y)
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t, u2;

    if (!carryable(unit->type))
      return FALSE;
    numdirs = choose_move_dirs(unit, x, y, FALSE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	/* See if there is a type that can carry us through via this
           direction. */
	for_all_unit_types(u2) {
	    if (could_carry(u2, unit->type)
	        && mobile(u2)
	        && !terrain_always_impassable(u2, t)
	        /* should also have "and this type is avail to us" */
	        ) {
	        return TRUE;
	    }
	}
    }
    return FALSE;
}

int
blocked_by_enemy(Unit *unit, int x, int y, int shortest)
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, numbaddirs = 0;
    Unit *unit2;

    numdirs = choose_move_dirs(unit, x, y, shortest, plausible_move_dir, NULL, dirs);
    /* No plausible dirs anyhow, so presence of enemy irrelevant. */
    if (numdirs == 0)
      return FALSE;
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	unit2 = unit_at(x1, y1);
	if (in_play(unit2) && !trusted_side(unit->side, unit2->side))
	  ++numbaddirs;
    }
    return (numbaddirs == numdirs);
}

int
attack_blockage(Side *side, Unit *unit, int x, int y, int shortest)
{
    int dirs[NUMDIRS], numdirs, i, x1, y1;
    Unit *unit2;

    numdirs = choose_move_dirs(unit, x, y, shortest, plausible_move_dir,
			       NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	unit2 = unit_at(x1, y1);
	if (in_play(unit2)
	    && !trusted_side(unit->side, unit2->side)
	    && uu_zz_bhw(unit->type, unit2->type) > 0
	    ) {
	    DMprintf("%s blocked, hitting enemy %s.", 
		     unit_desig(unit), unit_desig(unit2));
	    net_push_hit_unit_task(
		unit, unit2->id, HIT_METHOD_ANY, HIT_TRIES_DFLT);
	    return TRUE;
	}
    }
    return FALSE;
}

/* Detect an enemy unit in our own or an adjacent cell. */

int
enemy_close_by(Side *side, Unit *unit, int dist, int *xp, int *yp)
{
    int x = unit->x, y = unit->y, dir, x1, y1, victimcount = 0;
    extern int victim_x, victim_y, victim_rating;

    victim_rating = -9999;
    tmpunit = unit;
    ai_victim_here(x, y, &victimcount);
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    ai_victim_here(x1, y1, &victimcount);
	}
    }
    if (victim_rating > -9999) {
	*xp = victim_x;  *yp = victim_y;
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Note the recursion - should precalc this property. */

int
carryable(int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (could_carry(u2, u) && mobile(u2))
	  return TRUE;
    }
    return FALSE;
}

static short *accelerables = NULL;

int
accelerable(int u)
{
    int u1, u2;

    if (accelerables == NULL) {
	accelerables = (short *) xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u1) {	
	    for_all_unit_types(u2) {
		if (accelerator(u1, u2)) {
		    accelerables[u2] = TRUE;
		    break;
		}
	    }
	}
    }
    return accelerables[u];
}

/* True if u1 is a type that can move u2 faster by transporting it. */

int
accelerator(int u1, int u2)
{
    int t;

    if (could_carry(u1, u2)
	&& mobile(u1)) {
	if (type_max_acp(u1) * type_max_speed(u1) > 
	    type_max_acp(u2) * type_max_speed(u2)) {
	    /* The transport must be able to move on any type of terrain
	       accessible to the transportee. */
	    for_all_terrain_types(t) {
		if (terrain_always_impassable(u1, t)
		    && !terrain_always_impassable(u2, t)) {
		    return FALSE;
		}
	    }
	}
	return TRUE;
    }
    return FALSE;
}

/* Init used by all machine players.  Precompute useful information
   relating to unit types in general, and that usually gets referenced
   in inner loops. */

void
ai_init_shared()
{
    int u1, t, m1, numbuilders, tmp;
    int worthmax, n = 0;
    int cswtmp, cswmax, eswtmp, eswmax, oswtmp, oswmax, dswtmp, dswmax;
    Side *side = NULL;
    int u = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    
    /* Need 3 scratch layers for routefinding. */
    allocate_area_scratch(3);
    /* Recognize unit types that are bases */
    for_all_unit_types(u1) {
	set_u_is_base(u1, FALSE);
	tmp = FALSE;
	for_all_material_types(m1) {
	    if (um_base_production(u1, m1) > 0) {
		tmp = TRUE;
		break;
	    }
	}
	if (tmp) {
	    for_all_unit_types(u2) {
		if ((u1 != u2) && could_carry(u1,u2)) {
		    set_u_is_base(u1, TRUE);
		    continue;
		}
	    }
	}
    }
    /* Note that is_base_builder is set to the type of base that can */
    /* be built.  That means that unit zero can not be a base which */
    /* can be built. */
    for_all_unit_types(u1) {
	set_u_is_transport(u1, FALSE);
	set_u_is_carrier(u1, FALSE);
	set_u_is_base_builder(u1, FALSE);
	set_u_can_make(u1, FALSE);
	set_u_can_capture(u1, FALSE);
	set_u_is_ground_mobile(u1, FALSE);
	set_u_is_naval_mobile(u1, FALSE);
	set_u_is_air_mobile(u1, FALSE);
	set_u_is_colonizer(u1, FALSE);
	set_u_is_facility(u1, FALSE);
	/* DON'T set u_advanced to FALSE. It is already set by the
           game file. */
	numbuilders = 0;
	for_all_unit_types(u2) {
	    if (u_is_base(u2) &&
		could_create(u1, u2)) {
		set_u_is_base_builder(u1, TRUE);
	    }
	    if (type_max_speed(u1) > 0 && could_carry(u1, u2)) {
		set_u_is_transport(u1, TRUE);
	    }
	    if (could_create(u1,u2)) {
		set_u_can_make(u1, TRUE);
		/* A colonizer is a mobile builder of advanced units. */
		if (u_advanced(u2) && mobile(u1))
		  set_u_is_colonizer(u1, TRUE);
	    }
	    if (uu_capture(u1, u2) > 0 || uu_indep_capture(u1, u2) > 0) {
		set_u_can_capture(u1, TRUE);
	    }
	}
    }
    /* a carrier is a unit that is a mobile base, but that cannot
       move a passenger anywhere the passenger could not go itself. */
    for_all_unit_types(u1) {
	if (u_is_transport(u1)) {
	    set_u_is_carrier(u1, TRUE);
	    for_all_unit_types(u2) {
		if (could_carry(u1, u2)) {
		    for_all_terrain_types(t) {
			if (could_be_on(u1, t) && !could_be_on(u2, t))
			  set_u_is_carrier(u1, FALSE);
		    }
		}
	    }
	}
    }
    maybe_set_mover_worths();
    maybe_set_seer_worths();
    maybe_set_depot_worths();
    maybe_set_distributor_worths();
    maybe_set_producer_worths();
    maybe_set_prod_enhancer_worths();
    maybe_set_base_worths_for();
    maybe_set_base_worths();
    maybe_set_explorer_worths();
    for_all_unit_types(u) {
	set_u_bw(u, basic_worth(u));
	set_u_offensive_worth(u, offensive_worth(u));
	set_u_defensive_worth(u, defensive_worth(u));
	set_u_colonizer_worth(u, colonizing_worth(u));
	set_u_facility_worth(u, facility_worth(u));
	set_u_random_worth(u, random_worth(u));
	set_u_siege_worth(u, siege_worth(u));
	if (mobile(u)) {
	    /* Assume this is an air unit by default. */
	    set_u_is_air_mobile(u, TRUE);
	    for_all_terrain_types(t) {
	    	/* We only care about real terrain here. */
	    	if (t_is_connection(t) || t_is_border(t) || t_is_coating(t)) {
	    		continue;
	    	}
	         /* Check that our unit can have enough mps to move within 
	         the given terrain. */
		if (type_can_move_in_terrain(u, t)	
		    /* And that there is room for our unit in the terrain. */
		    && type_can_occupy_terrain(u, t)
		    /* And that it does not vanish or wreck in the terrain. */	
		    && type_survives_in_terrain(u, t)) {
		    /* It is naval mobile if it can enter at least one
		       liquid terrain. */
		    if (t_liquid(t))
		      set_u_is_naval_mobile(u, TRUE);
		    /* It is ground mobile if it can enter at least one
		       non-liquid terrain. */
		    if (!t_liquid(t))
		      set_u_is_ground_mobile(u, TRUE);
		    /* Air units should have zero size in all terrains. */
		    /* This is not always adhered to in the game modules. */
/*		    if (ut_size(u, t) > 0)
		      set_u_is_air_mobile(u, FALSE);
*/		} else {
		    /* Air units should be able to enter all terrains. */
		    set_u_is_air_mobile(u, FALSE);
	        }
	    }
	/* Kind of crude, but OK for now. */
	} else
	  set_u_is_facility(u, TRUE);
    }
    /* Normalize the worth values on a -10k to +10k scale. */
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_bw(u));
    for_all_unit_types(u)
      set_u_bw(u, normalize_on_pmscale(u_bw(u), worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_offensive_worth(u));
    for_all_unit_types(u)
      set_u_offensive_worth(u, 
			    normalize_on_pmscale(u_offensive_worth(u), 
						 worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_defensive_worth(u));
    for_all_unit_types(u)
      set_u_defensive_worth(u, 
			    normalize_on_pmscale(u_defensive_worth(u), 
						 worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_colonizer_worth(u));
    for_all_unit_types(u)
      set_u_colonizer_worth(u, 
			    normalize_on_pmscale(u_colonizer_worth(u), 
						 worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_facility_worth(u));
    for_all_unit_types(u)
      set_u_facility_worth(u, 
			   normalize_on_pmscale(u_facility_worth(u), 
						worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_random_worth(u));
    for_all_unit_types(u)
      set_u_random_worth(u, 
			 normalize_on_pmscale(u_random_worth(u), 
					      worthmax, 10000));
    worthmax = 0;
    for_all_unit_types(u)
      worthmax = max(worthmax, u_siege_worth(u));
    for_all_unit_types(u)
      set_u_siege_worth(u, 
			normalize_on_pmscale(u_siege_worth(u), 
					     worthmax, 10000));
    /* Colonization support worth. */
    cswmax = 0;
    for_all_unit_types(u) {
	cswtmp = 0;
	for_all_unit_types(u2) {
	    if (!could_create(u, u2))
	      continue;
	    if (could_colonize(u2))
	      cswtmp += max(0, u_colonizer_worth(u2));
	    for_all_unit_types(u3) {
		n = uu_complete_occs_on_completion(u2, u3) / 100;
		n += dice1_roll_min(uu_alt_complete_occs_on_completion(u2, u3));
		if (!n)
		    continue;
		if (could_colonize(u3))
		    cswtmp += max(0, u_colonizer_worth(u3));
	    }
	}
	cswmax = max(cswtmp, cswmax);
	tmp_u_array[u] = cswtmp;
    }
    for_all_unit_types(u) 
      set_u_colonization_support_worth(u,
				       normalize_on_pmscale(
					tmp_u_array[u], cswmax, 10000));
    // Base construction worth.
    cswmax = 0;
    for_all_unit_types(u) {
	cswtmp = 0;
	for_all_unit_types(u2) {
	    if (!could_create(u, u2))
	      continue;
	    if (0 < u_ai_base_worth(u2))
	      cswtmp += u_ai_base_worth(u2);
	}
	cswmax = max(cswtmp, cswmax);
	tmp_u_array[u] = cswtmp;
    }
    for_all_unit_types(u) 
      set_u_base_construction_worth(u,
				    normalize_on_pmscale(
					tmp_u_array[u], cswmax, 10000));
    /* Exploration support worth. */
    eswmax = 0;
    for_all_unit_types(u) {
	eswtmp = 0;
	for_all_unit_types(u2) {
	    if (!could_create(u, u2))
	      continue;
	    if (could_explore(u2))
	      eswtmp += max(0, u_ai_explorer_worth(u2));
	    for_all_unit_types(u3) {
		n = uu_complete_occs_on_completion(u2, u3) / 100;
		n += dice1_roll_min(uu_alt_complete_occs_on_completion(u2, u3));
		if (!n)
		    continue;
		if (could_explore(u3))
		    eswtmp += max(0, u_ai_explorer_worth(u3));
	    }
	}
	eswmax = max(eswtmp, eswmax);
	tmp_u_array[u] = eswtmp;
    }
    for_all_unit_types(u) 
      set_u_exploration_support_worth(u,
				      normalize_on_pmscale(
					tmp_u_array[u], eswmax, 10000));
    /* Offensive support worth. */
    oswmax = 0;
    for_all_unit_types(u) {
	oswtmp = 0;
	for_all_unit_types(u2) {
	    if (!could_create(u, u2))
	      continue;
	    if (could_damage_any(u2)) 
	      oswtmp += max(0, u_offensive_worth(u2));
	    if (could_capture_any(u2))
	      oswtmp += max(0, u_siege_worth(u2));
	    for_all_unit_types(u3) {
		n = uu_complete_occs_on_completion(u2, u3) / 100;
		n += dice1_roll_min(uu_alt_complete_occs_on_completion(u2, u3));
		if (!n)
		    continue;
		if (could_damage_any(u3))
		    oswtmp += max(0, u_offensive_worth(u3));
		if (could_capture_any(u3))
		    oswtmp += max(0, u_siege_worth(u3));
	    }
	}
	oswmax = max(oswtmp, oswmax);
	tmp_u_array[u] = oswtmp;
    }
    for_all_unit_types(u) 
      set_u_offensive_support_worth(u,
				    normalize_on_pmscale(
					tmp_u_array[u], oswmax, 10000));
    /* Defensive support worth. */
    dswmax = 0;
    for_all_unit_types(u) {
	dswtmp = 0;
	for_all_unit_types(u2) {
	    if (!could_create(u, u2))
	      continue;
	    if (could_defend_against_any(u2))
	      dswtmp += max(0, u_defensive_worth(u2));
	    for_all_unit_types(u3) {
		n = uu_complete_occs_on_completion(u2, u3) / 100;
		n += dice1_roll_min(uu_alt_complete_occs_on_completion(u2, u3));
		if (!n)
		    continue;
		if (could_defend_against_any(u3))
		    dswtmp += max(0, u_defensive_worth(u3));
	    }
	}
	dswmax = max(dswtmp, dswmax);
	tmp_u_array[u] = dswtmp;
    }
    for_all_unit_types(u) 
      set_u_defensive_support_worth(u,
				    normalize_on_pmscale(
					tmp_u_array[u], dswmax, 10000));
    /* UU worths. */
    for_all_unit_types(u) {
	for_all_unit_types(u2) {
	    set_uu_bhw(u, u2, basic_hit_worth(u, u2));
	    set_uu_bfw(u, u2, basic_fire_worth(u, u2));
	    set_uu_bcw(u, u2, basic_capture_worth(u, u2));
	    set_uu_btw(u, u2, basic_transport_worth(u, u2));
	    if (uu_zz_bhw(u, u2) > bhw_max)
	      bhw_max = uu_zz_bhw(u, u2);
	}
    }
    /* Normalize basic attack worths. */
    worthmax = 0;
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	    worthmax = max(worthmax, uu_zz_bhw(u, u2));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	  set_uu_bhw(u, u2,  
		     normalize_on_pmscale(uu_zz_bhw(u, u2), worthmax, 10000));
    }
    /* Normalize basic fire worths. */
    worthmax = 0;
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	    worthmax = max(worthmax, uu_zz_bfw(u, u2));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	  set_uu_bfw(u, u2,
		     normalize_on_pmscale(uu_zz_bfw(u, u2), worthmax, 10000));
    }
    /* Normalize basic capture worths. */
    worthmax = 0;
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	    worthmax = max(worthmax, uu_zz_bcw(u, u2));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	  set_uu_bcw(u, u2,
		     normalize_on_pmscale(uu_zz_bcw(u, u2), worthmax, 10000));
    }
    /* Normalize basic transport worths. */
    worthmax = 0;
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	    worthmax = max(worthmax, uu_zz_btw(u, u2));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2)
	  set_uu_btw(u, u2,
		     normalize_on_pmscale(uu_zz_btw(u, u2), worthmax, 10000));
    }
    /* Tell how things rated. */
    if (DebugM)
      display_assessment();
}

int basic_transport_worth(int u1, int u2);
void set_uu_btw(int u1, int u2, int v);

static int average_damage(int damage, int defender);

/* This function takes damage (any type) as input converts it into
an average damage in case it is a dice value. */

int
average_damage(int damage, int defender)
{
	int avgdamage = 0;

        avgdamage = dice1_roll_mean(damage);
	if (avgdamage > u_hp(defender))
		avgdamage = u_hp(defender);
	return avgdamage;
}

/* A crude estimate of the worth of having one type of unit. */

int
basic_worth(int u)
{
    int worth = 0, u2, r, range;
  
    worth += u_hp(u) * 10;
    for_all_unit_types(u2) {
	if (could_create(u, u2))
	  worth += (u_bw(u2) * (50)) / 1 /* uu_make(u, u2) */;
	/* (should account for shared capacity) */
	if (could_carry(u, u2))
	  worth += (1 + type_max_speed(u)) * uu_capacity_x(u, u2) *
	    (u_is_base(u) ? 10 : 1) * u_bw(u2) / 30;
    }
    range = 12345;
    for_all_material_types(r) {
	worth += um_base_production(u, r) * (u_is_base(u) ? 4 : 1);
	if (um_consumption_per_move(u, r) > 0)
	  range = min(range, 
                      um_storage_x(u, r) / 
                      max(1, um_consumption_per_move(u, r)));
	if (um_base_consumption(u, r) > 0) 
	  range =
	    min(range, 
                type_max_speed(u) * um_storage_x(u, r) / 
                max(1, um_base_consumption(u, r)));
    }
    worth += type_max_speed(u) * u_hp(u);
    worth += (range == 12345 ? area.maxdim : range)
      * u_hp(u) / max(1, 10 - type_max_speed(u));
    for_all_unit_types(u2) {
	worth += (worth * uu_capture(u, u2)) / 150;
    }
    worth = isqrt(worth);
    DMprintf("unit type %s basic worth %d \n ", u_type_name(u), worth);
    if (worth < 0)
        init_warning("%s has negative basic worth", u_type_name(u));
    return worth;
}

int
offensive_worth(int u)
{
    int totoffvalue = 0, offvalue = 0, defenders = 0, hit = 0, dmg = 0;
    int worth = 0;
    int u2 = NONUTYPE;

    if (g_combat_model() == 0) {
	for_all_unit_types(u2) {
	    if (uu_acp_to_attack(u, u2) > 0) {
		hit = uu_hit(u, u2);
		if (hit) {
		    offvalue = (hit * average_damage(uu_damage(u, u2), u2));
		    if (offvalue)
		      ++defenders;
		    totoffvalue += offvalue;
		}
	    }
	    if (u_acp_to_fire(u) > 0) {
                hit = fire_hit_chance(u, u2);
		if (hit) {
		    offvalue = hit * average_damage(fire_damage(u, u2), u2);
		    if (offvalue)
		      ++defenders;
		    totoffvalue += offvalue;
		}
	    }
	    if ((0 < u_acp_to_detonate(u)) || u_detonate_with_attack(u)) {
		offvalue = dice1_roll_mean(uu_detonation_damage_at(u, u2));
		if (0 < uu_detonation_range(u, u2))
		  offvalue += 
		    (radius_covers_n_cells(uu_detonation_range(u, u2) - 1) * 
		     (dice1_roll_mean(uu_detonation_damage_adj(u, u2)) / 2));
		hit = 0;
		if (0 < u_acp_to_detonate(u))
		  hit = 100;
		hit += min(100, u_detonate_with_attack(u));
		offvalue = (offvalue * hit) / 100;
		if (offvalue)
		  ++defenders;
		totoffvalue += offvalue;
	    }
	}
	if (defenders > 0)
	  totoffvalue /= defenders;
	worth = type_max_acp(u) * totoffvalue * u_range(u) * 
		type_max_speed(u) / 100;
	DMprintf("unit type %s offensive worth %d (combat model 0)\n ",
		 u_type_name(u), worth);
	return worth;
    } else if (g_combat_model() == 1) {
	worth = type_max_acp(u) * u_attack(u) * u_range(u) * 
		type_max_speed(u) / 100;
	DMprintf("unit type %s offensive worth %d (combat model 1)\n ",
		 u_type_name(u), worth);
	if (worth < 0)
	    init_warning("%s has negative offensive worth", u_type_name(u));
	return worth;
    } else return 1;
}

int
defensive_worth(int u)
{
    int defvalue = 0;
    int attackers = 0;
    int worth = 0;
    int u2, hit, dmg;

    if (g_combat_model() == 0) {
	for_all_unit_types(u2) {
	    if (uu_acp_to_attack(u2, u) > 0) {
		++attackers;
		defvalue += 100 * u_hp_max(u); 
		defvalue -= uu_hit(u2, u) * average_damage(uu_damage(u2, u), u);
	    }
	    if (u_acp_to_fire(u2) > 0) {
		++attackers;
                hit = fire_hit_chance(u2, u);
                dmg = average_damage(fire_damage(u2, u), u);
		defvalue += 100 * u_hp_max(u) - hit * dmg;
	    }
	}
	if (attackers > 0)
	  defvalue /= attackers;
	worth = type_max_acp(u) * defvalue / 10;
	DMprintf("unit type %s defensive worth %d (combat model 0)\n ", u_type_name(u), worth);
	return worth;
	/* Much simpler to calculate things in combat model 1 since attack and defense strengths are
	   absolute numbers. */
    } else if (g_combat_model() == 1) {
	worth = type_max_acp(u) * u_defend(u);
	DMprintf("unit type %s defensive worth %d (combat model 1)\n ", u_type_name(u), worth);
	if (worth < 0)
	    init_warning("%s has negative defensive worth", u_type_name(u));
	return worth;
    } else return 1;
}

#if (0)

int
exploring_worth(int u)
{

        /* It should also be considered what kind of terrain the unit can 
           move on, and how common that terrain is. */
        /* if ( g_synth_methods == what? */
            for_all_terrain_types(t) {
                if ( ( ! ut_vanishes_on(u, t) ) && ( ! ut_wrecks_on(u, t) ) ) {
                    /* Can explore this terrain without dying! */
                    mobility += ( ( t_alt_max(t) - t_alt_min(t) ) 
                    		   * ( t_wet_max(t) - t_wet_min(t) ) ) 
                    		   / ( ut_mp_to_enter(u, t) + 1);
                }
            }
    }

#endif

int
colonizing_worth(int u)
{
    int colvalue = 0;
    int worth = 0;
    int u2;
 
    /* A colonizer is a mobile builder of advanced units. */
    if (!mobile(u))
      return 0;
    for_all_unit_types(u2) {
	if (could_create(u, u2)
	    && (u_advanced(u2) || could_create(u2, u))) {
	    /* It is more valuable if it can build bigger cities. */
	    colvalue += u_reach(u2) + 1;
	}
    }    	
    worth = type_max_acp(u) * type_max_speed(u) * colvalue / 10;
    DMprintf("unit type %s colonizer worth %d \n ", u_type_name(u), worth);
    if (worth < 0)
	init_warning("%s has negative colonizer worth", u_type_name(u));
    return worth;
}

int
facility_worth(int u)
{
    int worth = 0;

    /* Kind of crude, but OK for now. */
    if (!mobile(u))
      worth = 1;

    DMprintf("unit type %s facility worth %d \n ", u_type_name(u), worth);
    if (worth < 0)
	init_warning("%s has negative facility worth", u_type_name(u));
    return worth;
}

int
siege_worth(int u)
{
    int siegevalue = 0;
    int worth = 0;
    int u2;
 
    for_all_unit_types(u2) {
	siegevalue += uu_capture(u, u2);
    }           
    siegevalue = siegevalue / numutypes;
 
    worth = type_max_acp(u) * u_range(u) * siegevalue; 

    DMprintf("unit type %s siege worth %d \n ", u_type_name(u), worth);
    if (worth < 0)
	init_warning("%s has negative siege worth", u_type_name(u));
    return worth;
}

/* Assign the same worth to all unit types. */

int
random_worth(int u)
{
    return 1;
}

/* A basic estimate of the payoff of one unit type attacking another
   type directly.  This is "context-free", does not account for
   overall goals etc. */

int
basic_hit_worth(int u, int e)
{
    int avgdamage, worth = 0, anti = 0, acp = 0, attacks = 0;

    if (could_attack(u, e)) {
	if (!u_acp_independent(u))
	  attacks = (u_acp(u) + u_free_acp(u)) / uu_acp_to_attack(u, e);
	else
	  attacks = 1;
	avgdamage = average_damage(uu_damage(u, e), e);
	worth = (uu_hit(u, e) * avgdamage * attacks) / u_hp(e);
    }
    if (could_counterattack(e, u)) {
	if (!u_acp_independent(e))
	  attacks = (u_acp(e) + u_free_acp(e)) / uu_acp_to_attack(e, u);
	else
	  attacks = 1;
	avgdamage = average_damage(uu_damage(e, u), u);
	anti = (uu_hit(e, u) * uu_counterattack(u, e)) / 100;
	anti = (anti * avgdamage * attacks) / u_hp(u);
    }
    worth -= anti;
    return worth;
}

int
basic_fire_worth(int u, int e)
{
    int avgdamage, worth = 0, firings = 0;

    if (could_fire_at(u, e)) {
	if (!u_acp_independent(u))
	  firings = (u_acp(u) + u_free_acp(u)) / u_acp_to_fire(u);
	else
	  firings = 1;
	avgdamage = average_damage(fire_damage(u, e), e);
	worth = (fire_hit_chance(u, e) * avgdamage * firings) / u_hp(e);
    }
    return worth;
}

/* A crude estimate of the payoff of one unit type trying to capture. */

int
basic_capture_worth(int u, int e)
{
    int acp, worth1 = 0, worth2 = 0, worth = 0;

    acp = max(uu_acp_to_capture(u, e), uu_acp_to_attack(u, e));
    if (acp < 1)
      return -9999;
    if (uu_capture(u, e) > 0) {
	worth1 = (uu_capture(u, e) * type_max_acp(u)) / acp;
    }
    if (uu_indep_capture(u, e) > 0) {
	worth2 = (uu_indep_capture(u, e) * type_max_acp(u)) / acp;
    }
    /* (should account for chance of dying in attempt) */
    worth = max(worth1, worth2);
    return worth;
}

int
basic_transport_worth(int u, int u2)
{
    int worth = 0;

    if (could_carry(u, u2)) {
	worth += 1;
    }
    return worth;
}

/* Display the results of our calculations. */

void
display_assessment(void)
{
    int u, u2;

    DMprintf("\nUnit Attributes:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s : base %d, transport %d, carrier %d, worth %d\n",
	       shortest_unique_name(u), u_is_base(u),
	       u_is_transport(u), u_is_carrier(u), u_bw(u));
	DMprintf("    Operate between ranges %d and %d\n", 
		 operating_range_worst(u), operating_range_best(u));
    }
    DMprintf("\nUnit vs Unit Combat:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s:", shortest_unique_name(u));
	for_all_unit_types(u2)
	  DMprintf("%5d", uu_zz_bhw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Fire:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s:", shortest_unique_name(u));
	for_all_unit_types(u2)
	  DMprintf("%5d", uu_zz_bfw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Capture:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s:", shortest_unique_name(u));
	for_all_unit_types(u2)
	  DMprintf(" %4d", uu_zz_bcw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Transport:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s:", shortest_unique_name(u));
	for_all_unit_types(u2)
	  DMprintf(" %4d", uu_zz_btw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\n");
}

int
is_base_for(int u1, int u2)
{
    return (type_max_speed(u1) == 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

int
is_carrier_for(int u1, int u2)
{
    return (type_max_speed(u1) > 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

/* Since *.def parameters don't have setters usually, we have to supply
   some here.  These are very sensitive to how the parameters are organized,
   and they don't do any checking, so you have to careful about using them. */

void set_u_is_base(int u, int n) {  utypes[u].is_base = n;  }
void set_u_is_transport(int u, int n) {  utypes[u].is_transport = n;  }
void set_u_is_carrier(int u, int n) {  utypes[u].is_carrier = n;  }
void set_u_is_base_builder(int u, int n) {  utypes[u].is_base_builder = n;  }
void set_u_can_make(int u, int n) {  utypes[u].can_make = n;  }
void set_u_can_capture(int u, int n) {  utypes[u].can_capture = n;  }
void set_u_bw(int u, int n) {  utypes[u].bw = n;  }

void set_u_offensive_worth(int u, int n) {  utypes[u].ow = n;  }
void set_u_defensive_worth(int u, int n) {  utypes[u].dw = n;  }
void set_u_colonizer_worth(int u, int n) {  utypes[u].cw = n;  }
void set_u_facility_worth(int u, int n) {  utypes[u].fw = n;  }
void set_u_random_worth(int u, int n) {  utypes[u].rw = n;  }
void set_u_siege_worth(int u, int n) {  utypes[u].sw = n;  }
void set_u_colonization_support_worth(int u, int n) {  utypes[u].csw = n;  }
void set_u_base_construction_worth(int u, int n) {  utypes[u].bcow = n;  }
void set_u_exploration_support_worth(int u, int n) {  utypes[u].esw = n;  }
void set_u_offensive_support_worth(int u, int n) {  utypes[u].osw = n;  }
void set_u_defensive_support_worth(int u, int n) {  utypes[u].dsw = n;  }

void set_u_is_ground_mobile(int u, int n) {  utypes[u].is_ground_mobile = n;  }
void set_u_is_naval_mobile(int u, int n) {  utypes[u].is_naval_mobile = n;  }
void set_u_is_air_mobile(int u, int n) {  utypes[u].is_air_mobile = n;  }
void set_u_is_advanced(int u, int n) {  utypes[u].is_advanced = n;  }
void set_u_is_colonizer(int u, int n) {  utypes[u].is_colonizer = n;  }
void set_u_is_facility(int u, int n) {  utypes[u].is_facility = n;  }
	
int bhwtab = -1;
int bfwtab = -1;
int bcwtab = -1;
int btwtab = -1;

void
set_uu_bhw(int u1, int u2, int v)
{
    if (bhwtab < 0) {
	for (bhwtab = 0; tabledefns[bhwtab].name != NULL; ++bhwtab) {
	    if (strcmp("zz-basic-hit-worth", tabledefns[bhwtab].name) == 0) {
		allocate_table(bhwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[bhwtab].table == NULL)
      run_error("no bhw table allocated");
    (*(tabledefns[bhwtab].table))[numutypes * u1 + u2] = v;
}

void
set_uu_bfw(int u1, int u2, int v)
{
    if (bfwtab < 0) {
	for (bfwtab = 0; tabledefns[bfwtab].name != NULL; ++bfwtab) {
	    if (strcmp("zz-basic-fire-worth", tabledefns[bfwtab].name) == 0) {
		allocate_table(bfwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[bfwtab].table == NULL)
      run_error("no bfw table allocated");
    (*(tabledefns[bfwtab].table))[numutypes * u1 + u2] = v;
}

void
set_uu_bcw(int u1, int u2, int v)
{
    if (bcwtab < 0) {
	for (bcwtab = 0; tabledefns[bcwtab].name != NULL; ++bcwtab) {
	    if (strcmp("zz-basic-capture-worth", tabledefns[bcwtab].name) == 0) {
		allocate_table(bcwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[bcwtab].table == NULL)
      run_error("no bcw table allocated");
    (*(tabledefns[bcwtab].table))[numutypes * u1 + u2] = v;
}


void
set_uu_btw(int u1, int u2, int v)
{
    if (btwtab < 0) {
	for (btwtab = 0; tabledefns[btwtab].name != NULL; ++btwtab) {
	    if (strcmp("zz-basic-transport-worth", tabledefns[btwtab].name) == 0) {
		allocate_table(btwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[btwtab].table == NULL)
      run_error("no btw table allocated");
    (*(tabledefns[btwtab].table))[numutypes * u1 + u2] = v;
}



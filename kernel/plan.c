/* Unit plan handling for Xconq.
   Copyright (C) 1991-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"
#include "aiutil.h"
#include "ai.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"
#include "aioprt.h"

using namespace Xconq;
using namespace Xconq::AI;

static void plan_passive(Unit *unit);
static void plan_offense(Unit *unit);
static void plan_defense(Unit *unit);
static void plan_exploration(Unit *unit);
static void plan_improve(Unit *unit);
static void wake_at(int x, int y);
static int resupply_if_low(Unit *unit);
static int rearm_if_low(Unit *unit);
static int plan_resupply(Unit *unit, int m);
static int repair_if_damaged(Unit *unit);
static int plan_repair(Unit *unit);
static int alternate_target_here(int x, int y);
static int do_for_occupants(Unit *unit);
static int ai_damage_ratio_vs_type(Unit *unit, int u2);
static int ai_score_potential_victim_occupants(
    UnitView *uview, OccStatus occstatus, int victimflags, int dmgtypes);
static int ai_score_potential_victim(
    UnitView *uview, OccStatus occstatus, int victimflags);
static int ai_type_choose_best_hit_method(int u, int u2);
static int ai_consider_capturing(
    Unit *unit, UnitView *uview, int dmgthresh, int qthresh);
static int ai_consider_shaking(
    Unit *unit, UnitView *uview, OccStatus occstatus);
extern int ai_go_after_victim(Unit *unit, int range, int broadcast);
static int fire_at_opportunity(Unit *unit);
static int explore_reachable_cell(Unit *unit, int range);
static int capture_useful_if_nearby(Unit *unit);
static int capture_indep_if_nearby(Unit *unit);
static void random_walk(Unit *unit);
static int worth_capturing(Side *side, int u2, Side *oside, int x, int y);
static int indep_captureable_here(int x, int y);
static int useful_type(Side *side, int u);
/* static int could_capture_any(int u); */
static Plan *create_plan(void);
static int might_be_captured(Unit *unit);
static int occupant_could_capture(Unit *unit, int etype);
static int can_capture_neighbor(Unit *unit);
static int occupant_can_capture_neighbor(Unit *unit);
static int find_closest_unit(Side *side, int x0, int y0, int maxdist,
			     int (*pred)(int x, int y), int *rxp, int *ryp);
static int reachable_unknown(int x, int y);
static int adj_known_ok_terrain(int x, int y, Side *side, int u);
static int normal_completion_time(int u, int u2);
static void maybe_set_materials_goal(Unit *unit, int u2);

#if 0
static int go_after_captive(Unit *unit, int range);
static int range_left(Unit *unit);
static int find_worths(int range);
static int attack_worth(Unit *unit, int e);
static int threat(Side *side, int u, int x0, int y0);
static int move_patrol(Unit *unit);
static int build_time(Unit *unit, int prod);
static int out_of_ammo(Unit *unit);
static int explorable_cell(int x, int y);
static int should_capture_maker(Unit *unit);
extern int adj_unit(int x, int y);
#endif

/* (should have a generic struct for all plan type attrs) */

const char *plantypenames[] = {

#undef  DEF_PLAN
#define DEF_PLAN(NAME,code) NAME,

#include "plan.def"

    NULL
};

/* Every unit that can act needs a plan object, types that can't act
   should have it cleared out.  Note that incomplete units are expected
   to be able to act in the future, so it's acceptable to run this for
   incomplete units to give them a plan. */

void
init_unit_plan(Unit *unit)
{
    if (can_be_actor(unit)) {
	/* Might already have a plan, so don't always realloc. */
	if (unit->plan == NULL) {
	    unit->plan = create_plan();
	}
	/* Put the plan into a default state, side will work it up later. */
	/* (should release goals also) */
	clear_task_agenda(unit);
	/* Zero the plan just as xmalloc would. */
	memset(unit->plan, 0, sizeof(Plan));
	unit->plan->type = PLAN_PASSIVE;
	unit->plan->creation_turn = g_turn();
	/* Allow AIs to make this unit do things. */
	unit->plan->aicontrol = TRUE;
	/* Enable supply alarms by default. */
	unit->plan->supply_alarm = TRUE;
	/* Clear the task outcome. */
	unit->plan->last_task_outcome = TASK_UNKNOWN;
    } else {
	/* Brainless units don't need anything, can free up plan. */
	if (unit->plan != NULL) {
	    free_plan(unit);
	}
	unit->plan = NULL;
    }
}

void
set_unit_plan_type(Side *side, Unit *unit, int type)
{
    int oldtype;

    if (unit->plan) {
	oldtype = unit->plan->type;
	if (type != oldtype) {
	    if (type == PLAN_NONE) {
		type = PLAN_PASSIVE;
		DMprintf( "Forced replan: %s lacks plan.\n", unit_desig(unit));
		force_replan(unit);
	    }
	    unit->plan->type = (PlanType)type;
	    clear_task_agenda(unit);
	    if (side != NULL)
	      update_unit_display(side, unit, TRUE);
	}
    }
}

void
set_unit_asleep(Side *side, Unit *unit, int flag, int recurse)
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->asleep;
	if (flag != oldflag) {
	    unit->plan->asleep = flag;
	    if (side != NULL)
	      update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_asleep(side, occ, flag, recurse);
    	}
    }
}

void
set_unit_reserve(Side *side, Unit *unit, int flag, int recurse)
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->reserve;
	if (flag != oldflag) {
	    unit->plan->reserve = flag;
	    if (side != NULL)
	      update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_reserve(side, occ, flag, recurse);
    	}
    }
}

void
set_unit_ai_control(Side *side, Unit *unit, int flag, int recurse)
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->aicontrol;
	if (flag != oldflag) {
	    unit->plan->aicontrol = flag;
	    if (side != NULL)
	      update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_ai_control(side, occ, flag, recurse);
    	}
    }
}

void
set_unit_curadvance(Side *side, Unit *unit, int a)
{    
	unit->curadvance = a;
}

void
set_unit_researchdone(Side *side, Unit *unit, int flag)
{    
	unit->researchdone = flag;
}

void
set_unit_main_goal(Side *side, Unit *unit, Goal *goal)
{
    if (unit->plan) {
	unit->plan->maingoal = goal;
    }
}

void
set_unit_waiting_for_transport(Side *side, Unit *unit, int flag)
{
    if (unit->plan) {
	unit->plan->waitingfortransport = flag;
    }
}

/* Execute the plan. */

int
execute_plan(Unit *unit)
{
    Plan *plan = unit->plan;

    if (!in_play(unit) || !completed(unit)) {
	DMprintf("%s shouldn't be planning yet\n", unit_desig(unit));
	return 0; 
    }
    DMprintf("%s using plan %s\n", unit_desig(unit), plan_desig(plan));
    /* Units that are asleep or in reserve do nothing. */
    /* (This never happens according to debugging). */
    if (plan->asleep || plan->reserve) {
      return 0;
    }
    if (plan->type == PLAN_PASSIVE && plan->execs_this_turn > 10) {
	DMprintf(" not found\n");
    }
    if (plan->execs_this_turn > 100) {
	DMprintf("%s executed plan 100 times this turn, going into reserve\n",
		 unit_desig(unit));
	plan->reserve = TRUE;
	return 1;
    }
    /* Unit actually has a plan, dispatch on its type. */
    switch (plan->type) {
      case PLAN_NONE:
	/* Unit has not gotten a plan yet, leave it alone. */
	break;
      case PLAN_PASSIVE:
	plan_passive(unit);
	break;
      case PLAN_OFFENSIVE:
	plan_offense(unit);
	break;
      case PLAN_DEFENSIVE:
	plan_defense(unit);
	break;
      case PLAN_EXPLORATORY:
	plan_exploration(unit);
	break;
      case PLAN_IMPROVING:
	plan_improve(unit);
	break;
      default:
	case_panic("plan type", plan->type);
	break;
    }
    ++(plan->execs_this_turn);
    run_ui_idler();
    return 1;
}

/* Check if the unit is in formation or not. */

int
is_in_formation(Unit *unit)
{
    int nx, ny, dist; 
    Plan *plan = unit->plan;
    Goal *goal;
    Unit *leader;

    goal = plan->formation;
    leader = plan->funit;
    if (!leader)
      return FALSE;
    if (!in_play(leader)
	|| !unit_trusts_unit(unit, leader)
	|| goal->args[0] != leader->id)
      return FALSE;
    nx = leader->x + goal->args[1];  ny = leader->y + goal->args[2];
    dist = goal->args[3];
    if (distance(unit->x, unit->y, nx, ny) > dist)
      return FALSE;
    return TRUE;
}

/* See if we're too far away from an assigned position, set a task
   to move back if so. */

int
move_into_formation(Unit *unit)
{
    int nx, ny, dist; 
    Plan *plan = unit->plan;
    Goal *goal;
    Unit *leader;

    leader = plan->funit;
    if (leader != NULL) {
	goal = plan->formation;
	/* Ensure that the leader is still someone we want to follow. */
	if (!in_play(leader)
	    || !unit_trusts_unit(unit, leader)
	    || goal->args[0] != leader->id) {
	    notify(unit->side, "%s leader is gone, cancelling formation",
		   unit_handle(unit->side, unit));
	    free(goal);
	    plan->formation = NULL;
	    plan->funit = NULL;
	    /* Unit is available to do something else. */
	    return FALSE;
	}
	nx = leader->x + goal->args[1];  ny = leader->y + goal->args[2];
	dist = goal->args[3];
	if (distance(unit->x, unit->y, nx, ny) > dist) {
	    /* (should perhaps insert after current task?) */
	    set_move_to_task(unit, nx, ny, dist);
	    return TRUE;
	}
    }
    return FALSE;
}

int task_is_in_agenda(Plan *plan, Task *task);

/* See if there are any standing orders that currently apply to the given unit,
   and schedule a task if so.  Return TRUE if a task was added. */

int
execute_standing_order(Unit *unit, int addtask)
{
    Unit *transport;
    Side *side = unit->side;
    StandingOrder *sorder;

    for (sorder = side->orders; sorder != NULL; sorder = sorder->next) {
	if (sorder->types[unit->type] && unit->plan) {
	    switch (sorder->condtype) {
	      case sorder_at:
		if (unit->x == sorder->a1 && unit->y == sorder->a2) {
		    /* If the task is already in the plan, don't do
		       anything. */
		    if (task_is_in_agenda(unit->plan, sorder->task))
		      return FALSE;
		    if (addtask)
		      add_task(unit, 0, clone_task(sorder->task));
		    return TRUE;
		}
		break;
	      case sorder_in:
		transport = unit->transport;
		if (transport != NULL && transport->id == sorder->a1) {
		    /* If the task is already in the plan, don't do
		       anything. */
		    if (task_is_in_agenda(unit->plan, sorder->task))
		      return FALSE;
		    if (addtask)
		      add_task(unit, 0, clone_task(sorder->task));
		    return TRUE;
		}
		break;
	      case sorder_near:
		if (distance(unit->x, unit->y, sorder->a1, sorder->a2) <= sorder->a3) {
		    /* If the task is already in the plan, don't do
		       anything. */
		    if (task_is_in_agenda(unit->plan, sorder->task))
		      return FALSE;
		    if (addtask)
		      add_task(unit, 0, clone_task(sorder->task));
		    return TRUE;
		}
		break;
	      default:
		run_warning("Unknown order condition type");
		break;
	    }
	}
    }
    return FALSE;
}

int tasks_match(Task *task1, Task *task2);

int
task_is_in_agenda(Plan *plan, Task *task)
{
    Task *task2;

    for (task2 = plan->tasks; task2 != NULL; task2 = task2->next) {
	if (tasks_match(task, task2))
	  return TRUE;
    }
    return FALSE;
}

int
tasks_match(Task *task1, Task *task2)
{
    int i;

    if (task1->type != task2->type)
      return FALSE;
    for (i = 0; i < MAXTASKARGS; ++i)
      if (task1->args[i] != task2->args[i])
	return FALSE;
    return TRUE;
}

/* Passive units just work from the task queue or else wait to be told
   what to do. */

static void
plan_passive(Unit *unit)
{
    Plan *plan = unit->plan;

    /* Handle AI-controlled passive plans. */
    if (ai_controlled(unit)) {
	if (resupply_if_low(unit)) {
	    return;
	}
	if (rearm_if_low(unit)) {
	    return;
	}
	if (repair_if_damaged(unit)) {
	    return;
	}
	// If mobile transport, then change tasks if occs want us to.
	do_for_occupants(unit);
	/* Execute any old tasks associated with this plan. Return. */
	if (unit->plan->tasks) {
	    execute_task(unit);
	    return;
	}
	/* Else mention that we are replanning. */
	else {
	    DMprintf("Forced replan: %s is ai-controlled & passive.\n", 
		     unit_desig(unit));
	    force_replan(unit);
	    return;
	}
    }
    /* Special-case human cities/towns in the intro game to automatically
       start producing infantry initially. */
    /* (would be more efficient to put in a once-per-turn location,
       should look for one) */
    if (g_turn() <= 1
	&& mainmodule != NULL
	&& ((mainmodule->name != NULL
	     && strcmp(mainmodule->name, INTRO_GAME) == 0)
	    || (mainmodule->origmodulename != NULL
		&& strcmp(mainmodule->origmodulename, INTRO_GAME) == 0))
	&& ((strcmp(u_type_name(unit->type), "city") == 0)
	    || (strcmp(u_type_name(unit->type), "town") == 0))) {
	push_construct_task(unit, 0, 99, unit->id, -1, -1);
    }
    if (plan->supply_is_low && plan->supply_alarm) {
	plan->supply_alarm = FALSE;
	if (0 /* auto resupply */) {
	    set_resupply_task(unit, NONMTYPE);
	} else if (plan->tasks
		   && (plan->tasks->type == TASK_RESUPPLY
		       || (plan->tasks->type == TASK_MOVE_TO
			   && plan->tasks->next
			   && plan->tasks->next->type == TASK_RESUPPLY))) {
	} else {           
	    clear_task_agenda(unit);
	    set_waiting_for_tasks(unit, TRUE);
	    notify(unit->side,
"Canceling task agenda for %s. Supply is low. Please provide guidance.",
                   medium_long_unit_handle(unit));
	}
    }
    if (plan->tasks) {
	/* (should check that doctrine being followed correctly) */
	execute_task(unit);
    } else if (unit->side
	       && unit->side->orders
	       && execute_standing_order(unit, TRUE)) {
	execute_task(unit);
    } else if (plan->formation && move_into_formation(unit)) {
	execute_task(unit);
    } else {
	/* Our goal is now to get guidance from the side. */
	set_waiting_for_tasks(unit, TRUE);
    }
}

/* A unit operating offensively advances and attacks when possible. */

int find_alternate_hit_target(Unit *unit, Task *task, int *xp, int *yp);

static void
plan_offense(Unit *unit)
{
    int u = unit->type;
    int x, y, w, h, range, x1, y1, nx, ny;
    Plan *plan = unit->plan;
    Task *lasttask;
    Unit *unit2;

    if (resupply_if_low(unit)) {
	return;
    }
    if (rearm_if_low(unit)) {
	return;
    }
    if (repair_if_damaged(unit)) {
	return;
    }
    /* Run any 'hit-unit' tasks. */
    if (plan->tasks) {
    	execute_task(unit);
    // TODO: Reacquire target, if possible.
#if (0)
    	if (plan->last_task_outcome == TASK_FAILED) {
	    lasttask = &(plan->last_task);
	    if (lasttask->type == TASK_HIT_UNIT
		&& lasttask->args[2] != NONUTYPE
		&& !target_visible(unit, lasttask)) {
		/* Target seems to have disappeared, look around for it. */
		DMprintf("%s hit target has disappeared, looking for it; ",
			 unit_desig(unit));
		if (find_alternate_hit_target(unit, lasttask, &nx, &ny)) {
		    if (plan->tasks
			&& plan->tasks->type == lasttask->type
			&& plan->tasks->args[0] == lasttask->args[0]
			&& plan->tasks->args[1] == lasttask->args[1]
			&& plan->tasks->args[2] == lasttask->args[2]
			&& plan->tasks->args[3] == lasttask->args[3]
			) {
			pop_task(plan);
		    }
		    push_hit_unit_task(unit, nx, ny, 
				       lasttask->args[2], lasttask->args[3]);
		    DMprintf(" found at %d,%d\n", nx, ny);
		} else {
		    DMprintf(" not found\n");
		}
	    }
	}
#endif
	/* Irrespective of what happened, 
	    we don't want to step on the task yet. */
	return;
    }
    /* Follow through with other details of plan. */
    if (plan->maingoal && mobile(u)) {
	switch (plan->maingoal->type) {
	  case GOAL_UNIT_OCCUPIED:
	    /* Move to occupy our goal if necessary. */
	    unit2 = find_unit(plan->maingoal->args[0]);
	    range = u_ai_tactical_range(unit->type);
	    if (ai_go_after_victim(unit, range, FALSE)); 
	    else if (in_play(unit2) && unit->transport != unit2)
	      set_occupy_task(unit, unit2);
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
	  case GOAL_CELL_OCCUPIED:
	    /* Move to occupy our goal if necessary. */
	    x = plan->maingoal->args[0];  
	    y = plan->maingoal->args[1];
	    range = u_ai_tactical_range(unit->type);
	    if (ai_go_after_victim(unit, range, FALSE)); 
	    else if (unit->x != x || unit->y != y)
	      set_move_to_task(unit, x, y, 0);
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
	  case GOAL_VICINITY_HELD:
	    x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
	    w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
	    if (distance(x, y, unit->x, unit->y) > max(w, h)) {
		/* Outside the goal area - move in towards it. */
	    	if (random_point_near(x, y, w / 2, &x1, &y1)) {
		    x = x1;  y = y1;
	    	}
		DMprintf("%s to go on offensive to %d,%d\n",
			 unit_desig(unit), x, y);
		set_move_to_task(unit, x, y, max(w, h) / 2);
#if (0)
		if (unit->transport
		    && mobile(unit->transport->type)
		    && unit->transport->plan) {
		    set_move_to_task(unit->transport, x, y, max(w, h) / 2);
		}
#endif
	    } else {
		range = max(w, h);
		range = min(u_ai_tactical_range(unit->type), range);
#if (0)
		range = min(real_operating_range_best(unit), range);
		/* No special goal, look for something to fight with. */
		/* Sometimes be willing to look a little farther out. */
		if (probability(50))
		  range *= 2;
#endif
		/* Try to let occs decide for us. */
		if (mobile(u) && do_for_occupants(unit)) {
		/* Found a victim to go after, fall through. */
		} else if (ai_go_after_victim(unit, range, FALSE)) {
		/*! \todo Should consider transferring to another theater. */
		} else {
		    /* Do a random walk instead of just sitting there. */
		    DMprintf("%s to walk randomly\n", unit_desig(unit));
		    random_walk(unit);
	    	}
	    }
	    break;
	  default:
	    DMprintf("offensive unit has some goal\n");
	    break;
	}
    } else if (mobile(u)) {
	/* Play it safe. Search every cell within the tactical range.
	    But don't search the whole world! */
	range = u_ai_tactical_range(u); 
#if (0)
	if (probability(50))
	  range = min(range, 2 * type_max_acp(u));
#endif
	if (do_for_occupants(unit));
	/* No special goal, but found something to fight with. */
	else if (ai_go_after_victim(unit, range, FALSE)); 
	/*! \todo Should consider transferring to another theater. */
	/* Else go into reserve. */
	else {
	    if (mobile(unit->type) && probability(50)
		&& ((0 < u_defensive_worth(unit->type))
		    || (0 < u_ai_explorer_worth(unit->type))))
	      force_replan(unit);
	    else
	      set_unit_reserve(unit->side, unit, TRUE, FALSE);
	    return;
	}
	/* should go to a "best location" if possible. */
	/* (should do a sentry task) */
    } else if (ai_go_after_victim(unit, u_ai_tactical_range(u), FALSE)); 
    if (plan->tasks) 
    	execute_task(unit);
    else {
	/* If we cannot find anything sensible to do, we force a replan. 
           This will force the AI to consider if this unit should be moved 
           to another theater of operations. */
	if (probability(20)) {
	    force_replan(unit);
	    DMprintf("%s found nothing to do offensively, replanning.\n", 
		     unit_desig(unit));
	}
	else
	  set_unit_reserve(unit->side, unit, TRUE, FALSE);
    }
}

/* Look through list of occupants to see if an occupant needs the
   transport to do something. */

int
do_for_occupants(Unit *unit)
{
    Unit *occ = NULL, *target = NULL;
    Goal *goal = NULL;
    Task *task = NULL;
    
    /* If transport is not AI-controlled, then do not muck with it. */
    if (!ai_controlled(unit))
      return FALSE;
    /* If transport is immobile, then don't try moving for occs. */
    if (!u_mobile(unit->type))
      return FALSE;
    /* Do whatever the occs are asking us to do. */
    for_all_occupants(unit, occ) {
	if (occ->plan) {
	    /* Get the occ towards its goal, if it has one. */
	    goal = occ->plan->maingoal;
	    if (goal != NULL
		&& goal->type == GOAL_VICINITY_HELD
		&& (distance(goal->args[0], goal->args[1], unit->x, unit->y)
		    > goal->args[2])) {
		unit->plan->type = PLAN_PASSIVE;
		unit->plan->maingoal = NULL;
		unit->plan->formation = NULL;
		set_move_to_task(unit, goal->args[0], goal->args[1],
				  max(goal->args[2] / 2, 1));
		DMprintf("%s will go where occupant %s wants to go (goal %s)\n",
			 unit_desig(unit), unit_desig(occ), goal_desig(goal));
		return TRUE;
	    }
	    /* If the occ does not have a goal, see if it has a task. */
	    for_all_tasks(occ->plan, task) {
		if (TASK_MOVE_TO == task->type)
		    continue;
		if (task->type == TASK_HIT_UNIT) {
		    target = find_unit(task->args[0]);
		    if (!in_play(target))
			continue;
		    if (((target->x != unit->x) || (target->y != unit->y))
			&& (distance(target->x, target->y, unit->x, unit->y) 
			   > 1)) {
			unit->plan->type = PLAN_PASSIVE;
			unit->plan->maingoal = NULL;
			unit->plan->formation = NULL;
			set_move_to_task(unit, target->x, target->y, 1);
			DMprintf(
    "%s will go where occupant %s wants to go (task %s)\n",
				 unit_desig(unit), unit_desig(occ), 
				 task_desig(task));
			return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

int
find_alternate_hit_target(Unit *unit, Task *task, int *xp, int *yp)
{
    int range;

    tmpunit = unit;
    tmputype = task->args[2];
    tmpside = side_n(task->args[3]);
    /* (should adjust search radius for speed?) */
    range = type_max_acp(tmputype) + 1;
    return search_around(task->args[0], task->args[1], range,
			 alternate_target_here, xp, yp, 1);
}

static int
alternate_target_here(int x, int y)
{
    UnitView *uview;

    for_all_view_stack_with_occs(tmpunit->side, x, y, uview) {
	if (uview->type == tmputype
	    && uview->siden == tmpside->id)
	  return TRUE;
    }
    return FALSE;
}

/* Defensive units don't go out looking for trouble, but they should
   react strongly to threats. */

static void
plan_defense(Unit *unit)
{
    int u = unit->type, range, x, y, w, h, x1, y1;
    Plan *plan = unit->plan;
    Unit *unit2;

    if (resupply_if_low(unit)) {
	return;
    }
    if (rearm_if_low(unit)) {
	return;
    }
    if (repair_if_damaged(unit)) {
	return;
    }
    if (plan->tasks) {
    	/* (should analyze and maybe decide to change task) */
    	execute_task(unit);
    	return;
    }
    /* Listen to what our occs are telling us. */
    if (mobile(unit->type) && do_for_occupants(unit)) {
	if (plan->tasks) {
	    execute_task(unit);
	    return;
	}
    }
    /* Proceed with normal planning. */
    if (plan->maingoal) {
	switch (plan->maingoal->type) {
  	  case GOAL_UNIT_OCCUPIED:
	    /* Move to occupy our goal if necessary. */
	    unit2 = find_unit(plan->maingoal->args[0]);
	    if (in_play(unit2) && unit->transport != unit2)
	      set_occupy_task(unit, unit2);
	    else if ((unit->transport == unit2)
		     && ai_go_after_victim(unit, 1, FALSE));
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
  	  case GOAL_CELL_OCCUPIED:
	    /* Move to occupy our goal if necessary. */
	    x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
	    if (unit->x != x || unit->y != y)
	      set_move_to_task(unit, x, y, 0);
	    else if ((unit->x == x) && (unit->y == y) 
		     && ai_go_after_victim(unit, 
					   u_ai_tactical_range(unit->type),
					   FALSE));
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
	  case GOAL_VICINITY_HELD:
	    x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
	    w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
	    if (distance(x, y, unit->x, unit->y) > max(w, h)) {
		/* Outside the goal area - move in towards it. */
	    	if (random_point_near(x, y, w / 2, &x1, &y1)) {
		    x = x1;  y = y1;
	    	}
		DMprintf("%s to go on defensive to %d,%d\n",
			 unit_desig(unit), x, y);
		set_move_to_task(unit, x, y, max(w, h) / 2);
		if (unit->transport
		    && mobile(unit->transport->type)
		    && unit->transport->plan) {
		    set_move_to_task(unit->transport, x, y, max(w, h) / 2);
		}
	    } else {
		range = max(w, h);
		range = min(u_ai_tactical_range(unit->type), range);
		/* No special goal, look for something to fight with. */
		/* Sometimes be willing to look a little farther out. */
		if (probability(50))
		  range *= 2;
		/* Occupants have decided for us, fall through. */
		if (do_for_occupants(unit));
		/* Found a victim to go after, fall through. */
		else if (ai_go_after_victim(unit, range, FALSE)); 
		/* Else go into reserve. */
		else 
		  set_unit_reserve(unit->side, unit, TRUE, FALSE);
		/*! \todo Should consider transferring to another theater. */
	    }
	    break;
	  default:
	    DMprintf("defensive unit has some goal\n");
	    break;
	}
	/* (might be able to defend by interposing self?) */
	return;
    }
    if (can_attack_any(unit, unit) || can_fire_at_any(unit, unit) 
	|| could_capture_any(unit->type)) {
	/* Use the tactical range. */ 
	if (ai_go_after_victim(unit, u_ai_tactical_range(unit->type), FALSE)) {
	    execute_task(unit);
	    return;
	}
	/* Nobody close by, just hang out, shifting around a bit
           occasionally. */
	if (mobile(unit->type) && probability(10)) {
	    if (random_point_near(unit->x, unit->y, type_max_acp(u), 
				  &x1, &y1)) {
		DMprintf("%s to shift defensive position to %d,%d\n",
			 unit_desig(unit), x1, y1);
		set_move_to_task(unit, x1, y1, 0);
		execute_task(unit);
		return;
	    }
	}
	/* Else go into reserve or replan. */
	else {
	    if (mobile(unit->type) && probability(50)
		&& ((0 < u_offensive_worth(unit->type))
		    || (0 < u_siege_worth(unit->type))
		    || (0 < u_ai_explorer_worth(unit->type))))
	      force_replan(unit);
	    else
	      set_unit_reserve(unit->side, unit, TRUE, FALSE);
	    return;
	}
    } else {
	if (is_active(unit))
	    acquire_oprole(unit->side, unit->id, OR_CONSTRUCTOR);
    }
    if (plan->tasks) {
	execute_task(unit);
    } else {
	    /* If we cannot find anything sensible to do, we force a replan. 
	       This will force the AI to consider 
		if this unit should be moved to another theater of 
		operations. */
	if (probability(20)) {
	    force_replan(unit);
	    DMprintf("%s found nothing to do defensively, replanning.\n", 
		     unit_desig(unit));
	}
	else
	  set_unit_reserve(unit->side, unit, TRUE, FALSE);
    }
}

#if (0) // HACKING NOTE: Saving a snippet we are still interested in.
static void
plan_colonize(Unit *unit)
{
	  case GOAL_COLONIZE:
	    x1 = -1;  y1 = -1;
	    u2 = plan->maingoal->args[0];
	    /* Try finding a good, unused spot to colonize. */
	    /* (TODO: Ensure that other side units don't have same idea.) */
	    search_around(unit->x, unit->y, u_ai_tactical_range(u), 
			  good_cell_to_colonize, &x1, &y1, 1);
	    if (inside_area(x1, y1) 
		&& valid(can_construct(unit, unit, u2))
                && (NODIR != 
			choose_move_direction(unit, x1, y1, 
					      distance(x1, y1, 
						       unit->x, unit->y)))) {
		set_construct_task(unit, u2, 1, -1, x1, y1);	
		push_move_to_task(unit, x1, y1, 0);
		DMprintf(
			 "%s moving to (%d, %d) to colonize by building %s\n", 
			 unit_desig(unit), x1, y1, u_type_name(u2));
	    }
	    /* If we have AI planning and need explorers, 
		then try exploring to a reachable unknown cell. */
	    else if (ai_controlled(unit) && need_explorers(unit->side) 
		     && (0 < u_ai_explorer_worth(u))
		     && explore_reachable_cell(unit, u_ai_tactical_range(u)));
	    /* Else, move in a random dir along a straight line. */
	    else
	      set_move_dir_task(unit, random_dir(), 
				u_ai_tactical_range(u) + 
				xrandom(u_ai_tactical_range(u) / 2));
	    break;
#endif

#if (0) // HACKING NOTE: Saving a snippet we are still interested in.
static void
plan_colonize_support(Unit *unit)
{
	if (ai_controlled(unit))
	  maybe_set_materials_goal(unit, u2);
}
#endif

//! Plan: Improve
/*!
    This plan is currently a catch-all for a number of things:
    (1) Preparing to change utype. [TODO]
    (2) Changing utype.
    (3) Building productivity-enhancing units. [TODO]
    (4) Developing new tech for the side. [TODO]

    \todo Move this code to a new 'aiplan.c' file. Replace with hook invocation.
    \todo Improve heuristics for selecting a new utype to change into.
    \todo Allow designer-provided weights for selecting utype to change into.
    \todo Write change-type prep analysis code.
*/

static void
plan_improve(Unit *unit)
{
    static int *p_uimprove;

    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int totchance = 0, luckynum = 0, uval = 0;
    UnitView *uview = NULL, *uvstack = NULL;
    Unit *transport = NULL;

    assert_error(in_play(unit), 
		 "Attempting to execute plan for out-of-play unit");
    assert_error(unit->plan, "Trying to run NULL plan");
    u = unit->type;
    side = unit->side;
    /* Always take care of the basics before trying other stuff. */
    if (resupply_if_low(unit)) {
	return;
    }
    if (rearm_if_low(unit)) {
	return;
    }
    if (repair_if_damaged(unit)) {
	return;
    }
    /* Execute any old tasks associated with this plan. Return. */
    if (unit->plan->tasks) {
    	execute_task(unit);
    	return;
    }
    // Initialize the improvement utypes array, if necessary.
    if (!p_uimprove) 
	p_uimprove = (int *)xmalloc(numutypes * sizeof(int));
    /* Could unit change type ever? */
    if (could_change_type(u)) {
	/* Sum the various worths of u. */
	uval = total_worth(u);
	uval = max(0, uval);
	/* Examine possible utypes to change into. */
	for_all_unit_types(u2) {
	    p_uimprove[u2] = 0;
	    if (!valid(can_change_type_to(u, u2, side)))
	      continue;
	    /* Sum the various worths of u2. */
	    p_uimprove[u2] = total_worth(u2);
	    p_uimprove[u2] = max(0, p_uimprove[u2]);
	    /* (TODO: Give bonus for utypes that are needed for victory.) */
	    /* (TODO: If u2 could change back to u,
			then we may need to think more about it.) */
	}
	/* Zero out entries that are probably not worthwhile. */
	for_all_unit_types(u2) {
	    if (uval >= p_uimprove[u2]) 
	      p_uimprove[u2] = 0;
	}
	/* Sum chances. */
	totchance = 0;
	for_all_unit_types(u2)
	  totchance += p_uimprove[u2];
	/* Run the lottery, if we should. */
	if (0 < totchance) {
	    luckynum = xrandom(totchance);
	    totchance = 0;
	    for_all_unit_types(u2) {
		totchance += p_uimprove[u2];
		if (luckynum < totchance)
		  break;
	    }
	    assert_warning_return(is_unit_type(u2), 
				  "Picked an invalid utype to change to",);
	    /* Could unit change type now? */
	    if (valid(can_change_type_to(unit, unit, u2))) {
		if (prep_change_type_action(unit, unit, u2)) 
		  return;
	    }
	    /* Else, find out what we can do to prepare for a change-type,
		if anything. Also determine whether the cost of 
		changing type is too high in relation to what we could 
		construct, etc... in the same estimated timeframe. */
	    else {
		/* (TODO: Implement change-type prep analysis.) */
	    }
	} /* run lottery */
    } /* could change-type */
    /* Can unit construct anything useful inside itself? */
    else if (can_construct_any(unit, unit)) {
	/* Pick out possible utypes to construct. */
	for_all_unit_types(u2) {
	    p_uimprove[u2] = 0;
	    if (!type_can_occupy(u2, unit))
	      continue;
	    if (!can_construct(unit, unit, u2))
	      continue;
	    if (0 < u_ai_prod_enhancer_worth(u2))
	      p_uimprove[u2] += u_ai_prod_enhancer_worth(u2);
	    /* (TODO: Handle other cases, such as protection enhancement.) */
	    totchance += p_uimprove[u2];
	}
	/* Run the lottery, if we should. */
	if (totchance) {
	    luckynum = xrandom(totchance);
	    totchance = 0;
	    for_all_unit_types(u2) {
		totchance += p_uimprove[u2];
		if (luckynum < totchance)
		  break;
	    }
	    assert_warning_return(is_unit_type(u2), 
				  "Picked an invalid utype to construct",);
	    // Rummage through potential transports in cell.
	    // Note: Even if u2 cannot see cell and cannot survive on it,
	    //  we may be able to construct it inside something we have there.
	    uvstack = query_uvstack_at(unit->x, unit->y);
	    for_all_uvstack(uvstack, uview) {
		transport =
		    choose_transport_to_construct_in(u2, unit->side, uview);
		// TODO: Score all potential transports.
		if (transport)
		    break;
	    }
	    // Can survive on cell without transport?
	    // Note: Rejects cells that we cannot see.
	    // Note: Rejects cells that are too full.
	    if (!transport 
		&& !valid(can_survive_on_known(
			    u2, unit->side, unit->x, unit->y))) {
		if (probability(20))
		    force_replan(unit);
		else
		    set_unit_reserve(unit->side, unit, TRUE, FALSE);
		return;
	    }
	    if (transport)
		set_construct_task(unit, u2, 1, transport->id, -1, -1);
	    else
		set_construct_task(unit, u2, 1, -1, unit->x, unit->y);
	} /* lottery */
    } /* can construct any */
    /* Else, no known improvements can be made. */
    else {
	if (probability(20)) {
	    force_replan(unit);
	    DMprintf("%s could not find a way to improve, replanning.\n",
		     unit_desig(unit));
	}
	else
	  set_unit_reserve(unit->side, unit, TRUE, FALSE);
    }
    /* Execute any new tasks associated with this plan. Return. */
    if (unit->plan->tasks) {
    	execute_task(unit);
    	return;
    }
}

static void
plan_exploration(Unit *unit)
{
    Unit *unit2;
    Plan *plan = unit->plan;
    int u = unit->type;
    int x, y, w, h, range, x1, y1;
    int u2 = NONUTYPE;

    /* If the world has no secrets, exploration is sort of pointless. */
    if (g_see_all()) {
    	force_replan(unit);
    	return;
    }
    if (resupply_if_low(unit)) {
	return;
    }
    if (rearm_if_low(unit)) {
	return;
    }
    if (repair_if_damaged(unit)) {
	return;
    }
    if (plan->tasks) {
    	/* (should see if a change of task is worthwhile) */
    	execute_task(unit);
    	return;
    }
    /* Listen to what our occs are telling us. */
    if (do_for_occupants(unit)) {
	if (plan->tasks) {
	    execute_task(unit);
	    return;
	}
    }
#if (0)
    /* If we are a colonizer, we might also want to try colonizing. */
    if (0 < u_colonizer_worth(u) && good_cell_to_colonize(unit->x, unit->y)) {
	for_all_unit_types(u2) {
	    if (valid(can_construct(unit, unit, u2)) && !unit->nexthere) {
		set_construct_task(unit, u2, 1, -1, unit->x, unit->y);
		DMprintf(
"%s decided to colonize while exploring; will build %s\n",
			 unit_desig(unit), u_type_name(u2));
		if (plan->tasks) {
		    /* (should see if a change of task is worthwhile) */
		    execute_task(unit);
		    return;
		}
	    }
	}
    }
#endif
    /* Else, get down to exploration business. */
    if (plan->maingoal) {
	switch (plan->maingoal->type) {
	  case GOAL_UNIT_OCCUPIED:
	    /* Move to occupy our goal if necessary. */
	    unit2 = find_unit(plan->maingoal->args[0]);
	    if (in_play(unit2) && unit->transport != unit2)
	      set_occupy_task(unit, unit2);
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
	  case GOAL_CELL_OCCUPIED:
	    /* Move to our goal if necessary. */
	    x = plan->maingoal->args[0];  
	    y = plan->maingoal->args[1];
	    if (unit->x != x || unit->y != y)
	      set_move_to_task(unit, x, y, 0);
	    else {
		free(plan->maingoal);
		plan->maingoal = NULL;
	    }
	    break;
	  case GOAL_VICINITY_KNOWN:
	  case GOAL_VICINITY_HELD:
	    if (mobile(u)) {
		x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
		w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
		if (distance(x, y, unit->x, unit->y) > max(w, h)) {
		    /* Out of the area, move into it. */
	    	    if (random_point_near(x, y, max(w, h) / 2, &x1, &y1)) {
	    		x = x1;  y = y1;
	    	    }
		    DMprintf("%s to explore towards %d,%d\n",
			     unit_desig(unit), x, y);
		    set_move_to_task(unit, x, y, max(w, h) / 2);
		} else {
		    /* Found a cell to explore. */
		    if (explore_reachable_cell(unit, max(w, h) + 2)) {
		    } else {
		    	if (flip_coin()) {
			    DMprintf("%s clearing goal\n", unit_desig(unit));
                            free(plan->maingoal);
			    plan->maingoal = NULL;
		    	}
			DMprintf("%s to walk randomly\n", unit_desig(unit));
			random_walk(unit);
		    }
		}
	    }
	    else
		random_walk(unit);
	    break;
	  default:
	    DMprintf("%s goal %s?\n",
		     unit_desig(unit), goal_desig(unit->plan->maingoal));
	    break;
	}
    } else {
	/* No specific goal, just poke around. */
	if (mobile(u)) {
	    range = area.maxdim / 2;
	    /* Found a cell to explore. */
	    if (explore_reachable_cell(unit, range)) {
	    } else {
		DMprintf("%s to walk randomly\n", unit_desig(unit));
		random_walk(unit);
	    }
	}
	else 
	    random_walk(unit);
    }
    if (plan->tasks) {
        execute_task(unit);
    } else {
	/* If we cannot find anything sensible to do, we force a replan. 
           This will force the AI to consider if this unit should be moved 
           to another theater of operations. */
	if (probability(20)) {
	    force_replan(unit);
	    DMprintf("%s found nothing to do exploring, replanning.\n", 
		     unit_desig(unit));
	}
	else
	  set_unit_reserve(unit->side, unit, TRUE, FALSE);
    }
}

/* These are used by AIs as well. */
/* (Should be transplanted to 'ai.c'?) */

int victim_id;
int victim_x, victim_y, victim_rating, victim_utype, victim_sidenum;
OccStatus victim_occstatus;
int victim_flags;
int victim_dmgtypes;

int
ai_type_choose_best_hit_method(int u1, int u2)
{
    int dmgtypes = DAMAGE_TYPE_NONE;
    int atkdmgmax1 = -1, firedmgmax1 = -1;
    int atkrngidl1 = -1, firerngidl1 = -1, hitrngmax2 = -1;
    int atkrngdelta = 0, firerngdelta = 0;
    int dmgdelta1 = 0;

    dmgtypes = type_possible_damage_methods(u1, u2); 
    if ((dmgtypes & DAMAGE_TYPE_ATTACK) && !(dmgtypes & DAMAGE_TYPE_FIRE))
      return DAMAGE_TYPE_ATTACK;
    if (!(dmgtypes & DAMAGE_TYPE_ATTACK) && (dmgtypes & DAMAGE_TYPE_FIRE))
      return DAMAGE_TYPE_FIRE;
    if (!(dmgtypes & DAMAGE_TYPE_ATTACK) && !(dmgtypes & DAMAGE_TYPE_FIRE))
      return DAMAGE_TYPE_NONE;
    atkdmgmax1 = type_attack_damage_max(u1, u2);
    firedmgmax1 = type_fire_damage_max(u1, u2);
    atkrngidl1 = type_ideal_attack_range_max(u1, u2);
    firerngidl1 = type_ideal_fire_range_max(u1, u2);
    hitrngmax2 = type_hit_range_max(u2, u1);
    atkrngdelta = atkrngidl1 - hitrngmax2;
    firerngdelta = firerngidl1 - hitrngmax2;
    dmgdelta1 = atkdmgmax1 - firedmgmax1;
    if (atkrngdelta == firerngdelta) {
        return ((dmgdelta1 >= 0) ? DAMAGE_TYPE_ATTACK : DAMAGE_TYPE_FIRE);
    }
    else if (atkrngdelta > firerngdelta) {
        if (dmgdelta1 >= 0)
          return DAMAGE_TYPE_ATTACK;
        else {
            if ((dmgdelta1 / (atkrngdelta - firerngdelta)) <= -2)
              return DAMAGE_TYPE_FIRE;
            else
              return DAMAGE_TYPE_ATTACK;
        }
    }
    else {
        if (dmgdelta1 <= 0)
          return DAMAGE_TYPE_FIRE;
        else {
            if ((dmgdelta1 / (atkrngdelta - firerngdelta)) <= -2)
              return DAMAGE_TYPE_ATTACK;
            else
              return DAMAGE_TYPE_FIRE;
        }
    }
    return dmgtypes;
}

int
ai_score_potential_victim_occupants(UnitView *uview, OccStatus occstatus, 
                                    int victimflags, 
                                    int dmgtypes)
{
    int rating = 0, occrating = 0;
    int u1 = NONUTYPE, u3 = NONUTYPE;
    int x = -1, y = -1;
    UnitView *uvocc = NULL;
    Side *side = NULL, *oside = NULL;

    u1 = tmpunit->type;
    side = tmpunit->side;
    x = uview->x; y = uview->y;
    /* Iterate through all known occupants. */
    for_all_occupant_views_with_occs(uview, uvocc) {
        occrating = 0;
        oside = side_n(uvocc->siden);
        u3 = uvocc->type;
        /* Penalty for thinking about enemy transport that contains 
           friendlies. */
        if (!enemy_side(side, oside)) {
            if ((dmgtypes & DAMAGE_TYPE_ATTACK) 
                && could_damage_by_attacks(u1, u3))
              rating -= uu_hit(u1, u3);
            else if ((dmgtypes & DAMAGE_TYPE_FIRE)
                && could_damage_by_fire(u1, u3))
              rating -= fire_hit_chance(u1, u3);
            continue;
        }
        /* Add in attack worth, if occ can be attacked. */
        if ((dmgtypes & DAMAGE_TYPE_ATTACK)
	    && could_damage_by_attacks(u1, u3)) {
            occrating += uu_hit(u1, u3);
        }
        /* Add in fire-at worth, if occ can be fired at. */
        else if ((dmgtypes & DAMAGE_TYPE_FIRE) 
                 && could_damage_by_fire(u1, u3)) {
            occrating += fire_hit_chance(u1, u3);
        }
        /* Add in capture worth, if occ can be captured. */
        if (victimflags & VICTIM_CAPTURABLE)
          occrating += capture_chance(u1, u3, oside);
        /* Adjust occrating if less than or equal to 0. */
        if (occrating <= 0) {
            rating += occrating;
            occrating = 2;
        }
        /* Add a bonus, if occ is a builder. */
        if (type_can_build(u3, oside))
          occrating += ((occrating * 50) / 100);
        /* Add a huge bonus, if occ might be self unit. */
        if (u_can_be_self(u3))
          occrating *= 3;
        /* Add occ rating to aggregate rating. */
        rating += occrating;
    }
    return rating;
}

int
ai_score_potential_victim(UnitView *uview, OccStatus occstatus, 
                          int victimflags)
{
    int rating = 0, baserating = 0, isqrating;
    int u1 = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    int x = -1, y = -1;
    Side *side = NULL, *oside = NULL;
    int dmgtypes = DAMAGE_TYPE_NONE;
    Unit *unit3 = NULL;
    int dmgratio = -1;
    int popularity = 0;
    int dist = INT_MAX, dist2 = INT_MAX;
    int strikedist = -1, oprange = -1, strikedist2 = -1;
    int costratio = 0, costratiomax = 0;
    int numbuildees = 0;

    u1 = tmpunit->type;
    side = tmpunit->side;
    u2 = uview->type;
    x = uview->x; y = uview->y;
    oside = side_n(uview->siden);
    dist = distance(tmpunit->x, tmpunit->y, x, y);
    strikedist = type_hit_range_max(u1, u2);
    oprange = (new_acp_for_turn(tmpunit) * type_max_speed(u1)) / 100;
    dmgtypes = type_possible_damage_methods(u1, u2); 
    /* Choose better method for u1 to hit u2, if necessary. */
    if ((dmgtypes & DAMAGE_TYPE_ATTACK) && (dmgtypes & DAMAGE_TYPE_FIRE))
      dmgtypes = ai_type_choose_best_hit_method(u1, u2);
    /* Add attack worth. */
    rating += max(0, uu_zz_bhw(u1, u2));
    /* Add fire worth. */
    rating += max(0, uu_zz_bfw(u1, u2));
    /* Add capture worth, if we can capture enemy. */
    if (victimflags & VICTIM_CAPTURABLE) 
      rating += uu_zz_bcw(u1, u2);
    /* Add other worths that boost our estimation of the enemy. */
    rating += u_colonization_support_worth(u2);
    rating += u_exploration_support_worth(u2);
    rating += u_offensive_support_worth(u2);
    rating += u_defensive_support_worth(u2);
    /* Add encounter worth, if we can encounter enemy. */
    if (victimflags & VICTIM_ENCOUNTERABLE)
      rating += 10000;
#if (0)
    /* Add a bonus, for each utype that an enemy can build. */
    for_all_unit_types(u3) {
        if (type_can_build_type(u2, oside, u3))
          ++numbuildees;
    }
    rating += ((numbuildees * 100) / numutypes);
#endif
    /* If score is initially <= 1, still give it a chance to build up. */
    if (rating <= 1)
      baserating = 2;
    else
      baserating = rating;
    /* Add huge bonus if enemy unit might that side's self unit. */
    if (u_can_be_self(u2)) {
        if (rating <= 0)
          rating += (baserating * 3);
        else
          rating *= 3;
    }
    /* Update baserating. */
    if (rating > baserating)
      baserating = rating;
    /* Add in total transport worth, if enemy can transport. */
    if (CANNOT_HAVE_OCCS != occstatus) {
        for_all_unit_types(u3) {
            rating += uu_zz_btw(u2, u3);
        }
    }
#if (0)
    /* Add in occupant ratings, if enemy does or may have occupants. */
    switch (occstatus) {
      case MAYBE_HAS_OCCS:
        rating += (baserating * 2);
        break;
      case DEFINITELY_HAS_OCCS:
        rating += 
	    ai_score_potential_victim_occupants(uview, occstatus, 
                                                victimflags, dmgtypes);
        break;
      case DEFINITELY_HAS_NO_OCCS:
      case CANNOT_HAVE_OCCS:
      default:
        break;
    }
#endif
    /* Add urgency bonus if enemy unit potentially threatens one of our 
       builders. */
    for_all_side_units(side, unit3) {
        u3 = unit3->type;
        if (type_can_build(u3, side)) {
            dist2 = distance(unit3->x, unit3->y, x, y);
            strikedist2 = (could_damage(u2, u3)) 
                           ? type_hit_range_max(u2, u3) : -1;
            if (dist2 <= strikedist2) {
                rating += ((baserating * 25) / 100);
                strikedist2 = (could_damage(u2, u3))
                               ? type_ideal_hit_range_max(u2, u3) : -1;
                if (dist2 <= strikedist2)
                    rating += ((baserating * 50) / 100);
            }
            /* Urgent. Attempt to stop possible capture or lift siege. */
            if ((dist2 <= 1) && could_capture(u2, u3, side))
              rating *= 2;
        }
    }
    /* Penalize, if other units on our side are already planning to mess 
       with the enemy. */
    if (side_planning_to_capture_type_at(side, u2, x, y))
      popularity = n_planning_to_capture_type_at(side, u2, x, y);
    if (side_planning_to_hit_type_at(side, u2, x, y))
      popularity += n_planning_to_hit_type_at(side, u2, x, y);
    if (planning_to_capture_type_at(tmpunit, u2, x, y)) 
      --popularity;
    if (planning_to_hit_type_at(tmpunit, u2, x, y))
      --popularity;
    popularity = isqrt(popularity);
    if (popularity && (rating > 0))
      rating /= popularity;
    /* Add a bonus if it is more costly than our unit. */
    if (u_cp(u1)) {
        /* Increase score according to cost ratio. */
        if (u_cp(u2))
          rating += ((baserating * (u_cp(u2) / u_cp(u1)) * 50) / 100);
    }
    /* Could add many other score modifiers, such as whether the potential 
        victim provides a critical material for a side, whether it is 
        guaranteed to be a side's self unit, or if it has a greater value to 
        the scorekeeper than our unit. */
    /* Also need to consider stack protection and cellwide protection, and 
        the strengths of any enemy units that may be supporting our victim. */
#if (0)
    dmgratio = ai_damage_ratio_vs_type(tmpunit, u2);
    /* Add a bonus, if enemy is outgunned by us. */
    if (!dmgratio)
      rating += ((baserating * 50) / 100);
    /* Update baserating. */
    if (rating > baserating)
      baserating = rating;
    if (baserating <= 0)
      baserating = 2;
    /* Add urgency bonus, if enemy can potentially hit or may actually be 
       hitting us, and we are equal or superior to it. */
    if ((dmgratio <= 1) && could_damage(u2, u1)) {
        /* If it can hit us at all, we should be concerned. */
        if (type_hit_range_max(u2, u1) >= dist)
          rating += ((baserating * 50) / 100);
        /* If it can hit us with full effect, then be extra concerned. */
        if (type_ideal_hit_range_max(u2, u1) >= dist)
          rating += ((baserating * 75) / 100); 
        /* If it is mobile, there is a greater chance that it moved in to 
           attack us or that it can pester us in the future. */
        /* (Should actually examine the unit's movement rate.) */
        if (mobile(u2))
          rating += ((baserating * 125) / 100);
        /* More likely to be hitting us if it is less vulnerable. */
        if (!(victimflags & (VICTIM_CAPTURABLE | VICTIM_ENCOUNTERABLE)))
          rating += ((baserating * 50) / 100);
        if (!u_advanced(u2) && !type_can_build(u2, oside))
          rating += ((baserating * 50) / 100);
        if (!u_can_be_self(u2))
          rating += ((baserating * 50) / 100);
    }
    /* Heavily penalize score, if enemy can seriously outgun us. */
    if ((dmgratio > 1) && !(victimflags & VICTIM_ENCOUNTERABLE)) {
        if (type_ideal_hit_range_max(u1, u2) <= 
            type_ideal_hit_range_max(u2, u1)) 
            rating -= (baserating - (baserating / dmgratio));
        else if (type_ideal_hit_range_max(u1, u2) <=
                 type_hit_range_max(u2, u1))
          rating -= (baserating - (baserating / (dmgratio / 2)));
        else if (victimflags & VICTIM_CAPTURABLE)
          rating -= (baserating - (baserating / dmgratio));
    }
    /* Update baserating. */
    if (rating > baserating)
      baserating = rating;
    if (baserating <= 0)
      baserating = 2;
#endif
    isqrating = rating / (isqrt(dist) + 1);
    /* Penalize score according to turns from unit. */
    /* Note that this code is _different_ in purpose than the general 
       distance penalizer later on. It penalizes according to a rough 
       estimate of turns from the enemy unit rather than distance from 
       the enemy unit. */
    if (oprange && (dist > strikedist) && ((dist - strikedist) > oprange))
      rating -= (baserating - 
		 (baserating / (((dist - strikedist) / oprange) + 1)));
    /* Add a 50% bonus if enemy is within striking distance. */
    if (dist <= strikedist) {
	rating += (rating * 50) / 100;
    }
    /* Cap penalty to inverse square root of distance. */
    if (rating < isqrating) 
      rating = isqrating;
    /* Penalize if uview is old. */
    if (!g_see_all() && !u_see_always(u2) && (1 < (g_turn() - uview->date)))
      rating /= (g_turn() - uview->date);
    return rating;
}

int
ai_victim_here(int x, int y, int *numvictims)
{
    int u1 = tmpunit->type, u2 = NONUTYPE, rating = 0, dist = 0;
    Side *side = tmpunit->side, *oside = NULL;
    UnitView *uview = NULL;
    Task *hittask = NULL;
    int strikedist = -1, moves = 0, omoves = 0;

    /* Should iterate also over visible occs? However, the code explicitly
	handles occs. */
    for_all_view_stack(side, x, y, uview) {
        int victimflags = VICTIM_NOTHING_SPECIAL;
        OccStatus occstatus = CANNOT_HAVE_OCCS;

	u2 = uview->type;
	oside = side_n(uview->siden);
        rating = 0;
        /* If it's a friendly, then obviously skip it. */
        if (!enemy_side(side, oside))
          continue;
        /* If there is an encounter result associated with it, 
           then treat it like a capture. */
        /* We can be smarter than this, but this helps us out in games 
            such as Civ2, where many of the encounter results for 
            Villages are capture-like. */
        if (u_encounter_result(u2) != lispnil) 
          victimflags |= VICTIM_ENCOUNTERABLE;
        /* If we cannot hit it, then skip it. */
        /* Currently model 1 does not allow for even the capture of 
           undefended units by attack == 0 units. If this changes, then 
           we will need to reassess this part of the code. */
        if (!could_hit(u1, u2) && !(victimflags & VICTIM_ENCOUNTERABLE))
          continue;
        /* If we are immobile and it is not in striking range, then skip it. */
        strikedist = type_hit_range_max(u1, u2);
        dist = distance(tmpunit->x, tmpunit->y, x, y);
        if (!mobile(u1) && (dist > strikedist))
          continue;
	/* If we are slower than it, and cannot reach it this turn. */
	if (tmpunit->act) {
	    omoves = (type_max_acp(u2) * type_max_speed(u2)) / 100;
	    moves = (tmpunit->act->acp * type_max_speed(u1)) / 100;
	    /* (Assumes move-range == 1.) */
	    if (dist > moves) {
		moves = (type_max_acp(u1) * type_max_speed(u1)) / 100;
		if (moves < omoves)
		  continue;
	    }
	}
        if (capture_chance(u1, u2, oside))
          victimflags |= VICTIM_CAPTURABLE;
        /* Record the occupancy status. */
        occstatus = occ_status(uview);
        /* If it cannot be captured or encountered, and... */
        if (!victimflags) { 
#if (0)
            /* ...if it can hurt us much worse than we can hurt it, 
               then skip it. */
            if (ai_damage_ratio_vs_type(tmpunit, u2) >= 2)
                continue;
#endif
            /* ...if it cannot have occs, and... */
            if (CANNOT_HAVE_OCCS == occstatus) {
                /* ...if we cannot destroy it, then skip it. */
                /* Though in theory, it might be useful to drain the 
                   enemy's ACP with supressing fire. */
                if (!could_destroy(u1, u2))
                  continue;
            } /* CANNOT_HAVE_OCCS */
        } /* !victimflags */
	/* If it cannot be captured or encountered, and... */
	if (!victimflags) {
             /* ...if it might hurt our occupants, then skip. */
             /* (This test could be improved.) */
	    if ((tmpunit->occupant != NULL) && could_damage(u2, u1))
		continue;
	    /* ...if it is worth capturing by our side, and... */
	    if (worth_capturing(side, u2, oside, x, y)
                && (u_advanced(u2) || type_can_build(u2, oside))) {
		/* ...if it belongs to indepside, and indepside has no player 
		and cannot build, then skip it. */
		if (oside == indepside
		    && !g_indepside_has_ai()) {
			continue;
		}
#if (0)
		/* Let 2 attackers pound a unit with known occs. */
		if (occstatus == DEFINITELY_HAS_OCCS
		    && (n_planning_to_hit_type_at(side, u2, x, y) 
		    	- planning_to_hit_type_at(tmpunit, u2, x, y)) > 1) {
			continue;
		/* Let 1 attacker pound a unit with possible occs. */
		} else if (occstatus == MAYBE_HAS_OCCS
		    && (n_planning_to_hit_type_at(side, u2, x, y) 
		    	- planning_to_hit_type_at(tmpunit, u2, x, y)) > 0) {
			continue;
		/* Don't go for units without occs. */
		} else if (occstatus == DEFINITELY_HAS_NO_OCCS) {
			continue;
		}
#endif
		/* ...if it is too fragile to be shaken down, then skip it. */
		if (!ai_consider_shaking(tmpunit, uview, occstatus)) {
			continue;
		} else {
			victimflags |= VICTIM_SHAKEABLE;
            	}
            } else { /* not worth_capturing */
                /* ...if we cannot destroy it, then skip it. */
                /* Though in theory, it might be useful to drain the 
                   enemy's ACP with supressing fire. */
		if (!could_destroy(u1, u2))
                      continue;
	    } /* worth_capturing */
         } /* !victimflags */
        /* Else, can capture it. */
        /* Score the potential victim. */
        rating = ai_score_potential_victim(uview, occstatus, victimflags);
        if (rating > victim_rating
            || (rating == victim_rating && flip_coin())) {
            /* See if we can get within striking distance. */
            if (mobile(u1) 
                && (dist > strikedist)
                && (NODIR == 
                    choose_move_direction(tmpunit, x, y, max(1, strikedist))))
              continue;
            /* Tag the new victim. */
            DMprintf("\n\t...considering %s at %d,%d; assigned rating %d",
                     u_type_name(u2), x, y, rating);
            if (victimflags)
              DMprintf("\n\t\tflags = { %s %s }",
                       (victimflags & VICTIM_CAPTURABLE) ? "capturable" : "",
                       (victimflags & VICTIM_SHAKEABLE) ? "shakeable" : "");
            if (occstatus)
              DMprintf("\n\t\toccupancy status = %s",
                       (occstatus == MAYBE_HAS_OCCS) ? "maybe_has_occs" : 
                       ((occstatus == DEFINITELY_HAS_OCCS) ? 
                            "definitely_has_occs" : "definitely_has_no_occs"));
	    victim_id = uview->id;
            victim_x = x;  victim_y = y;
            victim_rating = rating;
            victim_utype = u2;
            victim_sidenum = oside->id;
            victim_occstatus = occstatus;
            victim_flags = victimflags;
            victim_dmgtypes = type_possible_damage_methods(u1, u2); 
            /* Increment the victim counter. */
            if (numvictims)
              ++(*numvictims);
        }
    } /* for_all_view_stack */
    return FALSE;
}

/* Evaluates the feasibility of shaking up a transport. */
/* (A fancier version of this might take into account 
   protection a transport provides its occs and vice versa.) */

int
ai_consider_shaking(Unit *unit, UnitView *uview, OccStatus occstatus)
{
    int u1 = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    Side *side = NULL, *oside = NULL, *occside = NULL;
    int dmgmax1 = 0, occknocks = 0, occknocksbest = 0, tsptknocks = 0;
    UnitView *uvocc = NULL;

    u1 = unit->type;
    side = unit->side;
    u2 = uview->type;
    oside = side_n(uview->siden);
    dmgmax1 = type_damage_max(u1, u2);
    /* If we can damage it. */
    if (dmgmax1) {
        tsptknocks = u_hp_max(u2) / dmgmax1;
        occknocks = 0; /* No occ-knock jokes please. */
        /* I'm not occustomed to having my work knocked. */
        if (occstatus == DEFINITELY_HAS_OCCS) {
            for_all_occupant_views(uview, uvocc) {
                u3 = uvocc->type;
                occside = side_n(uvocc->siden);
                dmgmax1 = type_damage_max(u1, uvocc->type); 
                if (dmgmax1) {
                    if (enemy_side(side, occside))
                      occknocks += u_hp_max(uvocc->type) / dmgmax1;
                    else
                      occknocks += 2 * (u_hp_max(uvocc->type) / dmgmax1);
                }
                if (occknocks >= tsptknocks)
                  break;
            }
            occknocksbest = occknocks;
        }
        else if (occstatus == MAYBE_HAS_OCCS) {
            /* This is a fairly expensive test. The results 
               remain the same throughout the game, and 
               hence should be precomputed. */
            for_all_unit_types(u3) {
                if (type_can_occupy_empty_type(u3, u2)) {
                    dmgmax1 = type_damage_max(u1, u3);
                    occknocks = 0;
                    if (dmgmax1)
                      occknocks = u_hp_max(u3) / dmgmax1;
                    occknocksbest = max(occknocks, occknocksbest);
                    if (occknocksbest >= tsptknocks)
                      break;
                }
            } 
        }
        /* If transport it too fragile, then don't shake it. */
        if (((tsptknocks * (oside->udoctrine)[u2]->repair_percent) / 100) 
            <= occknocksbest)
          return FALSE;
        /* If transport is tough enough, 
           and if it maybe has occs, then maybe shake it. */
        else {
            if ((occstatus == MAYBE_HAS_OCCS) && probability(50))
              return FALSE;
        }
    } /* dmgmax1 */
    /* If we cannot damage the transport we always attack 
    it on the assumption that it is building occs that we can 
    damage (the calling code checks that type_can_build or 
    u_advanced is true). */
    return TRUE;
}

/* Evaluates the feasibility of attempting to capture an unit. */

int
ai_consider_capturing(Unit *unit, UnitView *uview, int dmgthresh, 
                           int qthresh)
{
    int u1 = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    Side *side = NULL, *oside = NULL;
    int dmgmax1 = 0, ourknocks = 0;
    int qprob = 0;
    int i = 0;
    int ncaptors = 0;

    u1 = unit->type;
    side = unit->side;
    u2 = uview->type;
    oside = side_n(uview->siden);
    /* Sanity check. Can we even capture it? */
    if (!could_capture(u1, u2, oside))
      return FALSE;
    /* Can we even own the potential captive? */
    if (!type_allowed_on_side(u2, side))
      return FALSE;
    /* Can we take the potential captive without being scathed? */
    if ((could_capture_by_attacks(u1, u2, oside)
        || could_capture_by_fire(u1, u2, oside))
        && (type_hit_range_max(u2, u1) < 1))
      return TRUE;
    /* If it is a self-unit, can any other unit on our side capture it, or 
       MUST we perform the task, no matter how unsavory? */
    /* (Should consider that destruction might be more palatable.) */
    if (u_can_be_self(u2)) {
        dmgthresh += 5;
        for_all_unit_types(u3) {
            if (u3 == u1)
              continue;
            if (!type_allowed_on_side(u3, side))
              continue;
            if (could_capture(u3, u2, oside)) {
                ++ncaptors;
                break;
            }
        }
        if (!ncaptors)
          return TRUE;
    } 
    /* If it is a builder, then be willing to take greater risks to 
       seize it. */
    if (type_can_build(u2, oside) || u_advanced(u2))
      ++dmgthresh;
    if (ai_damage_ratio_vs_type(unit, u2) >= dmgthresh) {
        dmgmax1 = type_damage_max(u2, u1);    
        /* If it can damage us... */
        if (dmgmax1) {
            ourknocks = unit->hp2 / dmgmax1;
            qprob = 100 - capture_chance(u1, u2, oside);
            if (qprob > 0) {
                for (i = 0; i < ourknocks; ++i) {
                    qprob = (qprob * qprob) / 100; 
                    if (qprob <= qthresh)
                        break;
                }
                /* (Should factor in the value of the target, 
                   and whether we could lose occs in a 
                   shootout/capture attempt.) */
                if (qprob > qthresh)
                  return FALSE;
            }
        } /* dmgmax1 */
        /* Else it cannot damage us, then we must be invincible to one 
           another. */
        else {
            if (!could_capture_by_capture(u1, u2, oside))
              return FALSE;
        } /* !dmgmax1 */
    } /* ai_damage_ratio_vs_type(unit, u2) */
    return TRUE;
}

/* A generic damage-ratio evaluator. Assumes maximal damage. */
/* (This function should be in 'ai.c'?) */

int
ai_damage_ratio_vs_type(Unit *unit, int u2)
{
    int atkval1 = 0, atkval2 = 0;
    int u1 = NONUTYPE;

    u1 = unit->type;
    /* Can it even hit or damage us? 
       And do we look like Freddy Kreuger to it? */
    if (could_destroy(u2, u1)) {
        /* (The difference in fire ranges and attack ranges should be 
           accounted for. Which is to ask: can it hit us before we can 
           hit it, or vice versa? And how hard are the hits?) */
        atkval1 = unit->hp2 * type_damage_max(u1, u2);
        if (atkval1) {
            atkval2 = u_hp_max(u2) * type_damage_max(u2, u1);
            if (!atkval2)
                return 0; /* u2 cannot hurt u1 by conventional means. */
            /* How much more can u2 hurt u1 than u1 can hurt u2? 
               If 0, then u1 is superior.
               If 1, then roughly equal.
               If >1, then u2 is superior. */
            return atkval2 / atkval1; 
        }
        else
            return INT_MAX; /* u1 cannot hurt u2 by conventional means. */
    }
    return 0; /* u2 cannot destroy u1 by conventional means. */
}

/* This decides whether a given unit type seen at a given location is worth
   trying to capture. */

int
worth_capturing(Side *side, int u2, Side *side2, int x, int y)
{
    int u, bestchance = 0;

    /* See how likely we are to be able to capture the type. */
    for_all_unit_types(u) {
	bestchance = max(capture_chance(u, u2, side2), bestchance);
    }
    return bestchance;
    /* (should account for other considerations too, like which types of 
       units we have) */
}

#if 0		/* Unused. */

/* The point of this new function is to limit the search for captives
   to a given range. */

int
go_after_captive(Unit *unit, int range)
{
    int x, y, rslt;

    tmpunit = unit;

    DMprintf("%s searching for useful capture within %d; found ",
	     unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, 
			 useful_captureable_here, &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
	set_capture_task(unit, x, y);
	if (unit->transport
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    set_move_to_task(unit->transport, x, y, 1);
	}
	return (execute_task(unit) != TASK_FAILED);
    } else {
	DMprintf("nothing\n");
	return FALSE;
    }
}

/* Given a location and a unit (in tmpunit), try to identify a target. */
/* (should move to place to share with AIs) */

int target_x, target_y, target_rating, target_utype, target_sidenum;

int
target_here(int x, int y)
{
    int u2 = NONUTYPE, rating, dist;
    Side *side = tmpunit->side, *oside = NULL;
    Unit *unit3;
    UnitView *uview;

    for_all_view_stack_with_occs(side, x, y, uview) {
	u2 = uview->type;
	oside = side_n(uview->siden);
	/* (should move all tests inside loop) */
	if (is_unit_type(u2)
	    && enemy_side(side, oside)
	    && could_hit(tmpunit->type, u2)
	    /* Also consider damage by fire. Moreover, if the unit can
	       carry occupants they may be vulnerable even though the
	       unit itself is not. */
         	&& (uu_damage(tmpunit->type, u2) > 0
		    || fire_damage(tmpunit->type, u2) > 0
		    || type_can_have_occs(u2))		
	    /* and have correct ammo */
	    && !side_planning_to_capture_type_at(side, u2, x, y)
	    ) {
	    rating = uu_zz_bfw(tmpunit->type, u2);
	    /* Further-away units are less interesting than closer ones. */
	    dist = distance(tmpunit->x, tmpunit->y, x, y);
	    if (dist > 0)
	      rating /= dist;
	    /* A larger city is more worth capturing. */
	    rating *= uview->size;
	    /* Real enemies are more important targets. */
	    if (oside != NULL)
	      rating *= 2;
	    /* Always attack units that threaten one of our own cities. */
	    for_all_side_units(side, unit3) {
		if (u_advanced(unit3->type)
		    && distance(unit3->x, unit3->y, x, y) < 5)
		  rating *= 5 - distance(unit3->x, unit3->y, x, y);
	    }
	    if (rating > target_rating
		|| (rating == target_rating && flip_coin())) {
		target_x = x;  target_y = y;
		target_rating = rating;
		target_utype = u2;
		target_sidenum = side_number(oside);
	    }
	}
    }
    return FALSE;
}

int
fire_at_opportunity(Unit *unit)
{
    int x, y, range, rslt;

    tmpunit = unit;
    range = u_range(unit->type);
    /* Look further for targets if we are mobile. */
    if (mobile(unit->type)) {
    	range += u_ai_tactical_range(unit->type);
    }
    target_rating = -9999;
    DMprintf("%s seeking target within %d; found ",
             unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, target_here, &x, &y, 1);
    if (rslt) {
	DMprintf("s%d %s at %d,%d\n",
		 target_sidenum, u_type_name(target_utype), x, y);
	/* Set up a task to shoot at the unit found. */
	set_hit_unit_task(unit, x, y, target_utype, target_sidenum);
    } else if (target_rating > -9999) {
	DMprintf("s%d %s (rated %d) at %d,%d\n",
		 target_sidenum, u_type_name(target_utype), target_rating,
		 x, y);
	/* Set up a task to shoot at the unit found. */
	set_hit_unit_task(unit, target_x, target_y, target_utype, target_sidenum);
    } else {
	DMprintf("nothing\n");
    }
    return rslt;
}

#endif

/* Find a unit task by task type. */

Task *
find_unit_task_by_type(Unit *unit, TaskType tt)
{
    Task *task = NULL;

    if (in_play(unit) && unit->plan) {
        for_all_tasks(unit->plan, task) {
            if (task->type == tt)
              return task;
        }
    }
    return NULL;
}

/* Check to see if our grand plans are at risk of being sideswiped by lack of
   supply, and set up a resupply task if so. */

int
resupply_if_low(Unit *unit)
{
    int m;

    /* Check if any supplies are below the trigger level. */
    m = low_on_supplies_one(unit);
    if (m != NONMTYPE) {
	return plan_resupply(unit, m);
    }
    return FALSE;
}

/* Return a type of essential material that the unit is running out of. */

int
low_on_supplies_one(Unit *unit)
{
    int u = unit->type, m;

    for_all_material_types(m) {
	if ((um_base_consumption(u, m) > 0 
		|| um_consumption_per_move(u, m) > 0)
	    && um_storage_x(u, m) > 0
	    && unit->supply[m] <= 
               (unit_doctrine(unit)->resupply_percent * um_storage_x(u, m)) 
                / 100) {
	    return m;
	}
    }
    return NONMTYPE;
}

int
rearm_if_low(Unit *unit)
{
	int m;
	
	/* Check if any ammo is below the trigger level. */
	m = low_on_ammo_one(unit);
    	if (m != NONMTYPE) {
		return plan_resupply(unit, m);
	}
	return FALSE;
}

/* Return a type of material that we want to use to attack or fire. */

int
low_on_ammo_one(Unit *unit)
{
    int u = unit->type, m, trigger;

    for_all_material_types(m) {
    	/* Skip materials that we cannot store. */
    	if (um_storage_x(u, m) <= 0) {
		continue;
    	}
    	/* Compute the rearming trigger level. */
    	trigger = 
	    (unit_doctrine(unit)->rearm_percent * um_storage_x(u, m)) / 100;
	if (could_fire_at_any(u)) {
		/* First consider consumables (ammo). */
		if ((um_consumption_per_fire(u, m) > 0
		    /* Used incorrectly instead of um_consumption_per_fire in 
		    	some games (should fix this). */
		    || um_consumption_per_attack(u, m) > 0)
		    /* We are low either if we are below the trigger,
			 or if we are unable to fire one more round. */
	    	    && unit->supply[m] <= 
		       max(
			um_consumption_per_fire(u, m) > 0 ?
	    	    	um_consumption_per_fire(u, m) :
	    	    	um_consumption_per_attack(u, m), trigger)) {
			return m;
		}
		/* Then consider special materials (tools & weapons). */
		if (um_to_fire(u, m) > 0
		    /* We are low either if we are below the trigger,
		        or if we are unable to fire at all. */
		    && unit->supply[m] <= max(um_to_fire(u, m), trigger)) {
			return m;
		}
	}
	if (could_attack_any(u)) {
		/* First consider consumables (ammo). */
		if (um_consumption_per_attack(u, m) > 0
		    /* We are low either if we are below the trigger,
			 or if we are unable to fire one more round. */
	    	    && unit->supply[m] <= 
		       max(um_consumption_per_attack(u, m), trigger)) {
			return m;
		}
		/* Then consider special materials (tools & weapons). */
		if (um_to_attack(u, m) > 0
		    /* We are low either if we are below the trigger,
			 or if we are unable to fire at all. */
		    && unit->supply[m] <= max(um_to_attack(u, m), trigger)) {
			return m;
		}
	}
    }
    return NONMTYPE;
}

/* Now common code called by resupply_if_low and rearm_if_low. This is
really a plan since it sets various tasks. */

int
plan_resupply(Unit *unit, int m)
{
    int u = unit->type, x = unit->x, y = unit->y, x1, y1, outcome, range;
    Task *curtask = unit->plan->tasks;

    /* Set up a resupply task if extraction of material from the
	terrains is possible. */
    if (valid(check_extract_action(unit, unit, x, y, m, 1))) {
	set_resupply_task(unit, m);
	outcome = execute_task(unit);
	/* Clear the task outcome if we failed so that we do not block
	repair tasks or normal plan execution. Also clear the agenda
	so that we do not execute the failed task again. */
	if (outcome == TASK_FAILED) {
	    clear_task_agenda(unit);
	    clear_task_outcome(unit);
	    return FALSE;
	} else {
	    return TRUE;
	}
    }
    /* Setting up other types of resupply tasks makes sense only for mobile
	units. A non-mobile unit cannot move to a supply source, and if supplies
	are available in the same cell, it will benefit from this even if it is 
	doing something else such as building. */
    if (!mobile(u)) {
	return FALSE;
    }
    /* Now set up things to look for resupply of firstlowm. */
    range = real_operating_range_best(unit);
    tmpside = unit->side;
    tmpunit = unit;
    if (lowm == NULL) {
	lowm = (int *) xmalloc(nummtypes * sizeof(int));
    }
    lowm[0] = m;
    numlow = 1;
    /* Try resupplying in place. 
	This is preferable to moving to a resupply point. */
    if (can_resupply_from_here(unit->x, unit->y) 
	|| (can_survive_on_known(unit->type, unit->side, unit->x, unit->y)
	    && can_auto_resupply_self(unit, lowm, numlow))) {
	set_resupply_task(unit, m);
	/* Execute the new resupply task. */
	outcome = execute_task(unit);
    }
    /* See if we are already moving to a supply source. */
    else if (curtask != NULL
	&& curtask->type == TASK_MOVE_TO
	&& can_resupply_from_here(curtask->args[0], curtask->args[1])) {
	/* Proceed with task execution. */
	outcome = execute_task(unit);
    /* See if we already have a resupply task for the same material. */
    } else if (curtask != NULL
	&& curtask->type == TASK_RESUPPLY
	&& curtask->args[0] == m) {
	/* Proceed with task execution. */
	outcome = execute_task(unit);
    /* Otherwise set up a task. */
    } else if (search_around(x, y, range, can_resupply_from_here, 
			     &x1, &y1, 1)) {
	DMprintf("%s low on %s, found resupply point at %d,%d.\n", 
		 unit_desig(unit), m_type_name(m), x1, y1);
	set_resupply_task(unit, m);
	/* Execute the new resupply task. */
	outcome = execute_task(unit);
    /* Return, but don't clear the task outcome or the agenda if a 
	resupply task could not be set, so that execution of any existing 
	task in the queue may proceed. */
    } else {
	DMprintf("%s low on %s, found no resupply point within range %d.\n", 
		 unit_desig(unit), m_type_name(m), range);
	return FALSE;
    }   		
    /* Clear the task outcome if we failed so that we do not block
    repair tasks or normal plan execution. Also clear the agenda
    so that we do not execute the failed task again. */
    if (outcome == TASK_FAILED) {
	clear_task_agenda(unit);
	clear_task_outcome(unit);
	return FALSE;
    } else {
	return TRUE;
    }
}

int
repair_if_damaged(Unit *unit)
{
    int u = NONUTYPE;
    Task *tasks = NULL;
    int hpgoal = -1;

    assert_error(in_play(unit),
		 "AI: Attempted to assess damage to out-of-play unit");
    u = unit->type;
    hpgoal = (u_hp(u) * unit_doctrine(unit)->repair_complete) / 100;
    // Is any repair needed?
    if (unit->hp 
	>= (u_hp_max(u) * unit_doctrine(unit)->repair_percent) / 100) 
	return FALSE;
    // Check if we are already working on repair or resupply,
    //	and do not interfere if so.
    tasks = (unit->plan ? unit->plan->tasks : NULL);
    if (tasks) {
	if ((TASK_MOVE_TO == tasks->type) || (TASK_OCCUPY == tasks->type))
	    tasks = tasks->next;
	if (tasks) {
	    if (TASK_RESUPPLY == tasks->type)
		return FALSE;
	    if ((TASK_REPAIR == tasks->type) && (unit->id != tasks->args[0]))
		return FALSE;
	}
    }
    // Set repair task on self, if one does not exist already.
    if (!tasks || (TASK_REPAIR != tasks->type))
	set_repair_task(unit, unit->id, hpgoal);
    // Execute the new or existing task.
    if (TASK_FAILED != execute_task(unit))
	return TRUE;
    return FALSE;
}

#if (0)
/* Look within a limited distance for any independent unit that could be
   captured, and set up tasks to go get it. */

int
capture_indep_if_nearby(Unit *unit)
{
    /* Use the unit's tactical range instead. */
    int	u = unit->type;
    int	range = u_ai_tactical_range(u);
    int x, y, rslt;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u))
      return FALSE;
    if (!could_capture_any(unit->type))
      return FALSE;
    tmpunit = unit;
    /* See if we're already doing such a task. */
    if (curtask != NULL
	&& ((curtask->type == TASK_MOVE_TO
	     && indep_captureable_here(curtask->args[0], curtask->args[1]))
	    || (curtask->type == TASK_CAPTURE
		&& indep_captureable_here(curtask->args[0], curtask->args[1]))))
      return FALSE;
    DMprintf("%s searching for easy capture within %d; found ",
	     unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, indep_captureable_here,
			 &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
	set_capture_task(unit, x, y, tmputype, tmpside->id);
	if (unit->transport
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    set_move_to_task(unit->transport, x, y, 1);
	}
	return (execute_task(unit) != TASK_FAILED);
    } else {
	DMprintf("nothing\n");
    }
    return FALSE;
}

int
indep_captureable_here(int x, int y)
{
    int u2;
    Side *side = tmpunit->side, *side2;
    UnitView *uview;

    /* Should perhaps iterate over occs also, so that they can be 
    captured even if the transport is not? */
    for_all_view_stack(side, x, y, uview) {
	u2 = uview->type;
	side2 = side_n(uview->siden);
	if (side2 == indepside
	    && side != indepside
	    && capture_chance(tmpunit->type, u2, side2) > 10) {
	  	tmputype = u2;
		tmpside = side2;
		return TRUE;
	}
    }
    return FALSE;
}

/* Look within a limited distance for any type of unit that would be
   good to own, and set up tasks to go get it. */

int
capture_useful_if_nearby(Unit *unit)
{
    /* Use the unit's tactical range instead. */
    int u = unit->type;
    int	range = u_ai_tactical_range(u);
    int x, y, rslt;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u))
      return FALSE;
    if (!could_capture_any(unit->type))
      return FALSE;
    tmpunit = unit;
    /* See if we're already doing such a task. */
    if (curtask != NULL
	&& ((curtask->type == TASK_MOVE_TO
	     && useful_captureable_here(curtask->args[0], curtask->args[1]))
	    || (curtask->type == TASK_CAPTURE
		&& useful_captureable_here(curtask->args[0], curtask->args[1]))))
      return FALSE;
    DMprintf("%s searching for useful capture within %d; found ",
	     unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, useful_captureable_here,
			 &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
        if (g_combat_model() == 0) {
            if (!could_capture_by_capture(unit->type, tmputype, tmpside))
              set_hit_unit_task(unit, x, y, tmputype, tmpside->id);
            else
	      set_capture_task(unit, x, y, tmputype, tmpside->id);
        }
        else if (g_combat_model() == 1) {
            set_hit_unit_task(unit, x, y, tmputype, tmpside->id);
        }
	if (unit->transport
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    set_move_to_task(unit->transport, x, y, 1);
	}
	return (execute_task(unit) != TASK_FAILED);
    } else {
	DMprintf("nothing\n");
    }
    return FALSE;
}

int
useful_captureable_here(int x, int y)
{
    int u2;
    Side *side = tmpunit->side, *side2;
    UnitView *uview;

    /* Should perhaps iterate over occs also, so that they can be 
    captured even if the transport is not? */
    for_all_view_stack(side, x, y, uview) {
	u2 = uview->type;
	side2 = side_n(uview->siden);
	if (!trusted_side(side, side2)
	    && capture_chance(tmpunit->type, u2, side2) > 0
	    && useful_type(side, u2)
	    ) {
	    tmputype = u2;
	    tmpside = side2;
	    return TRUE;
	}
    }
    return FALSE;
}

#endif

/* Return true if the given type of unit is useful in some way to the
   given side.  This is almost always true. */

int
useful_type(Side *side, int u)
{
    if (!type_allowed_on_side(u, side))
      return FALSE;
    return TRUE;
}

#if (0)
int
could_capture_any(int u)
{
    int u2;

    for_all_unit_types(u2) {
	if (uu_capture(u, u2) > 0 || uu_indep_capture(u, u2) > 0)
	  return TRUE;
	/* also check if u2 in game, on other side, etc? */
    }
    return FALSE;
}
#endif

/* This attempts to make some vaguely plausible arguments for an
   action, using the types of each arg as a guide.  It also generates
   *invalid* arguments occasionally, which tests error checking in the
   actions' code.  This is mainly useful for testing. */

void
make_plausible_random_args(const char *argtypestr, int i, int *args, Unit *unit)
{
    char argch;
    int	slen, arg;

    slen = strlen(argtypestr);
    while (i < slen && i < 10) {
	argch = argtypestr[i];
	switch (argch) {
	  case 'n':
	    arg = (flip_coin() ? xrandom(10) :
		   (flip_coin() ? xrandom(100) :
		    (xrandom(20000) - 10000)));
	    break;
	  case 'u':
	    /* Go a little outside range, so as to get some invalid types. */
	    arg = xrandom(numutypes + 2) - 1;
	    break;
	  case 'm':
	    arg = xrandom(nummtypes + 2) - 1;
	    break;
	  case 't':
	    arg = xrandom(numttypes + 2) - 1;
	    break;
	  case 'a':
	    arg = xrandom(numatypes + 2) - 1;
	    break;
	  case 'x':
	    arg = (unit != NULL && flip_coin() ? (unit->x + xrandom(5) - 2) :
		   (xrandom(area.width + 4) - 2));
	    break;
	  case 'y':
	    arg = (unit != NULL && flip_coin() ? (unit->y + xrandom(5) - 2) :
		   (xrandom(area.height + 4) - 2));
	    break;
	  case 'z':
	    arg = (flip_coin() ? 0 : xrandom(10));
	    break;
	  case 'd':
	    arg = random_dir();
	    break;
	  case 'U':
	    /* Cast around for a valid unit. */
	    while (find_unit(arg = xrandom(numunits)+1) == NULL
		   && probability(98)) {
		   ;
	    }
	    break;
	  case 'S':
	    arg = xrandom(numsides + 3) - 1;
	    break;
	  default:
	    run_warning("Garbled action arg type '%c'\n", argch);
	    arg = 0;
	    break;
	}
	args[i++] = arg;
    }
}

/* Random walking just attempts to move around. */

void
random_walk(Unit *unit)
{
    int dir = random_dir(), x1, y1, tries = 0;

    while (!interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
    	if (++tries > 500) {
	    run_warning("something is wrong");
	    break;
	}
	dir = random_dir();
    }
    set_move_to_task(unit, x1, y1, 0);
}

/* Record the unit as waiting for orders about what to do. */

void
set_waiting_for_tasks(Unit *unit, int flag)
{
    if (unit->plan->waitingfortasks == flag) {
        return;
    }
    unit->plan->waitingfortasks = flag;
    if (unit->side != NULL) {
        unit->side->numwaiting += (unit->plan->waitingfortasks ? 1 : -1);
        update_unit_display(unit->side, unit, FALSE);
    }
}

/* General routine to wake a unit up (and maybe all its cargo). */

void
wake_unit(Side *side, Unit *unit, int forcewakeoccs)
{
    Unit *occ;

    assert_warning_return(in_play(unit), 
			  "Attempted to wake an invalid unit.", );
    /* (should test that side is permitted to wake) */
    /* Wake the unit. */
    if (unit->plan) {
	unit->plan->asleep = FALSE;
	unit->plan->reserve = FALSE;
	update_unit_display(side, unit, TRUE);
    }
    /* Try waking any occupants. */
    if (forcewakeoccs) {
	for_all_occupants(unit, occ)
	  wake_unit(side, occ, forcewakeoccs);
    }
}

/* Conditionally attempt to wake up a unit based on restrictions on recursive 
   waking. */

void
selectively_wake_unit(Side *side, Unit *unit, int wakeoccs, int forcewakeoccs)
{
    Unit *occ = NULL;

    assert_warning_return(in_play(unit), 
			  "Attempted to wake an invalid unit.", );
    /* If on a transport, then probably transport already
       has awakened us, but just to cover all the cases... */
    if ((unit->transport)
	&& uu_can_recursively_wake(unit->transport->type,
				   unit->type))
      wake_unit(unit->side, unit, forcewakeoccs);
    /* Else not on a transport. */
    else if (!(unit->transport))
      wake_unit(unit->side, unit, forcewakeoccs);
    /* Now wake any occs, if so desired. */
    /* Note that if 'forcewakeoccs' is set, then 'wake_unit' will 
       recursively wake _all_ occs, and so the following code would be 
       somewhat redundant. */
    /* (Need valid test case for this, so that it can be tested and 
	enabled.) */
#if (0)
    if (!forcewakeoccs && wakeoccs) {
	for_all_occupants(unit, occ) {
	    if ((unit->side == occ->side)
		&& uu_can_recursively_wake(unit->type, occ->type))
	      selectively_wake_unit(unit->side, occ, TRUE, FALSE);
	}
    }
#endif
}

/* The area wakeup. */

static int tmpflag;

static void
wake_at(int x, int y)
{
    Unit *unit;

    for_all_stack(x, y, unit) {
	if (side_controls_unit(tmpside, unit)) {
	    wake_unit(tmpside, unit, tmpflag);
	}
    }
}

void
wake_area(Side *side, int x, int y, int n, int occs)
{
    tmpside = side;
    tmpflag = occs;
    apply_to_area(x, y, n, wake_at);
}

void
set_formation(Unit *unit, Unit *leader, int x, int y, int dist, int flex)
{
    Plan *plan = unit->plan;
    Goal *goal;

    if (plan == NULL)
      return;
    if (!in_play(unit))
      return;
    if (leader != NULL) {
	if (!in_play(leader))
	  return;
	goal = create_goal(GOAL_KEEP_FORMATION, unit->side, TRUE);
	goal->args[0] = leader->id;
	goal->args[1] = x;  goal->args[2] = y;
	goal->args[3] = dist;
	goal->args[4] = flex;
	plan->formation = goal;
	plan->funit = leader;
    } else {
	/* A NULL leader means to clear the formation goal. */
	plan->formation = NULL;
	plan->funit = NULL;
    }
}

void
delay_unit(Unit *unit, int flag)
{
    if (in_play(unit) && unit->plan) {
	unit->plan->delayed = flag;
    }
}

#if 0 	/* The four functions below are not used anywhere. */

/* Return the distance that we can go by shortest path before running out
   of important supplies.  Will return at least 1, since we can *always*
   move one cell to safety.  This is a worst-case routine, too complicated
   to worry about units getting refreshed by terrain or whatever. */

int
range_left(Unit *unit)
{
    int u = unit->type, m, least = 12345; /* bigger than any real value */
    
    for_all_material_types(m) {
	if (um_consumption_per_move(u, m) > 0) {
	    least = min(least, unit->supply[m] / um_consumption_per_move(u, m));
	}
#if 0
	/* This code is too pessimistic if no account taken of supply line or
	   production, so leave out for now. */
	if (um_base_consumption(u, m) > 0) {
	    tmp = (type_max_speed(u) * unit->supply[m]) / 
		  um_base_consumption(u, m);
	    least = min(least, tmp);
	}
#endif
    }
    return (least == 12345 ? 1 : least);
}

/* Estimate the goodness and badness of cells in the immediate vicinity. */

int
find_worths(range)
int range;
{
    return 0;
}

/* This is a heuristic estimation of the value of one unit type
   hitting on another.  Should take cost of production into account as
   well as the chance and significance of any effect. */

int
attack_worth(unit, e)
Unit *unit;
int e;
{
    int u = unit->type, worth;

    worth = uu_zz_bhw(u, e);
    /* Risk of death? */
/*    if (uu_damage(e, u) >= unit->hp)
	worth /= (could_capture(u, e) ? 1 : 4);
    if (could_capture(u, e)) worth *= 4; */
    return worth;
}

/* Support functions. */

/* Return true if the given position is threatened by the given unit type. */

int
threat(Side *side, int u, int x0, int y0)
{
#if 0
    int d, x, y, thr = 0;
    Side *side2;
    int view;

    for_all_directions(d) {
    	point_in_dir(x0, y0, d, &x, &y);
	view = 0 /* side_view(side, x, y) */;
	if (view != UNSEEN && view != EMPTY) {
	    side2 = side_n(vside(view));
	    if (allied_side(side, side2)) {
		if (uu_capture(u, vtype(view)) > 0) thr += 1000;
		if (uu_zz_bhw(u, vtype(view)) > 0) thr += 100;
	    }
	}
    }
    return thr;
#endif
    return 0;
}

#endif

void
pop_task(Plan *plan)
{
    Task *oldtask;

    if (plan->tasks) {
	oldtask = plan->tasks;
	plan->tasks = plan->tasks->next;
	free_task(oldtask);
    }
}

#if 0 	/* The two functions below are unused. */

/* Patrol just does move_to, but cycling waypoints around when the first */
/* one has been reached. */

int
move_patrol(Unit *unit)
{
#if 0
    int tx, ty;

    if (unit->plan->orders.rept-- > 0) {
	if (unit->x == unit->plan->orders.p.pt[0].x &&
	    unit->y == unit->plan->orders.p.pt[0].y) {
	    tx = unit->plan->orders.p.pt[0].x;
	    ty = unit->plan->orders.p.pt[0].y;
	    unit->plan->orders.p.pt[0].x = unit->plan->orders.p.pt[1].x;
	    unit->plan->orders.p.pt[0].y = unit->plan->orders.p.pt[1].y;
	    unit->plan->orders.p.pt[1].x = tx;
	    unit->plan->orders.p.pt[1].y = ty;
	}
	return move_to(unit, unit->plan->orders.p.pt[0].x, unit->plan->orders.p.pt[0].y,
		       (unit->plan->orders.flags & SHORTESTPATH));
    }
#endif
    return TRUE;
}

/* Basic routine to compute how long a unit will take to build something. */

int
build_time(Unit *unit, int prod)
{
    int schedule = 1 /* uu_make(unit->type, prod) */;
    int u, develop_delay = 0;

    /* Add penalty (or unpenalty!) for first unit of a type. */
    /* is "counts" a reliable way to test? */
    if (unit->side->counts[prod] <= 1) {
/*	develop_delay = ((schedule * u_develop(prod)) / 100);  */
	for_all_unit_types(u) {
	    if (unit->side->counts[u] > 1) {
		develop_delay -=
		  (1 /*uu_make(unit->type, u)*/ * 
		   uu_tech_crossover(prod, u)) / 100;
	    }
	    if (develop_delay > 0) {
		schedule += develop_delay;
	    }
	}
    }
    return schedule;
}

#endif

int
clear_task_agenda(Unit *unit)
{
    Plan	*plan = unit->plan;
    int		numcleared;
    Task	*oldtask;

    if (plan == NULL || plan->tasks == NULL)
	return 0;
    numcleared = 0;
    while (plan->tasks != NULL) {
    	oldtask = plan->tasks;
    	plan->tasks = plan->tasks->next;
    	free_task(oldtask);
    	++numcleared;
    }
    return numcleared;
}

void
clear_task_outcome(Unit *unit)
{
	if (unit->plan == NULL)
	    run_error("no plan here?");
	unit->plan->last_task_outcome = TASK_UNKNOWN;
}

Plan *
create_plan(void)
{
    Plan *plan = (Plan *) xmalloc(sizeof(Plan));
    return plan;
}

void 
free_plan(Unit *unit)
{
    Plan *plan = unit->plan;

    if (plan == NULL)
      run_error("no plan here?");
    /* Make tasks available for reallocation. */
    clear_task_agenda(unit);
    free(plan);
}

/* Describe a plan succinctly.  This is primarily for debugging, not
   for normal user display. */

char *planbuf = NULL;

char *
plan_desig(Plan *plan)
{
    Task *task;
    int extra = 0;

    if (planbuf == NULL)
      planbuf = (char *)xmalloc(1000);
    if (plan == NULL) {
	sprintf(planbuf, "no plan");
    } else if (plan->type == PLAN_NONE) {
	sprintf(planbuf, "unformed plan");
    } else {
	if (plan->tasks) {
	    tmpbuf[0] = '\0';
	    for_all_tasks(plan, task) {
		if (strlen(tmpbuf) < 100) {
		    strcat(tmpbuf, " ");
		    strcat(tmpbuf, task_desig(task));
		} else {
		    ++extra;
		}
	    }
	    if (extra > 0) {
		tprintf(tmpbuf, " ... %d more ...", extra);
	    }
	} else {
	    sprintf(tmpbuf, " no tasks");
	}
	sprintf(planbuf, "type %s %s",
		plantypenames[plan->type], goal_desig(plan->maingoal));
	if (plan->formation) {
	    strcat(planbuf, " ");
	    strcat(planbuf, goal_desig(plan->formation));
	}
	if (plan->asleep)
	  strcat(planbuf, " asleep");
	if (plan->reserve)
	  strcat(planbuf, " reserve");
	if (plan->delayed)
	  strcat(planbuf, " delayed");
	if (plan->waitingfortasks)
	  strcat(planbuf, " waiting");
	if (plan->supply_alarm)
	  strcat(planbuf, " supply_alarm");
	if (plan->supply_is_low)
	  strcat(planbuf, " supply_is_low");
	strcat(planbuf, tmpbuf);
    }
    return planbuf;
}

/* True if unit is in immediate danger of being captured. */
/* Needs check on capturer transport being seen. */

int
might_be_captured(Unit *unit)
{
    int d, x, y;
    Unit *unit2;

    for_all_directions(d) {
      if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
	if (((unit2 = unit_at(x, y)) != NULL) &&
	    (enemy_side(unit->side, unit2->side)) &&
	    (uu_capture(unit2->type, unit->type) > 0))
	      return TRUE;
      }
    }
    return FALSE;
}

/* Clear a unit's plan out. */

void
force_replan(Unit *unit)
{
    extern int need_ai_planning;

    if (unit->plan == NULL)
      return;
    unit->plan->type = PLAN_PASSIVE;
    clear_task_agenda(unit);
    unit->plan->maingoal = NULL;
    unit->plan->formation = NULL;
    unit->plan->funit = NULL;
    unit->plan->asleep = FALSE;
    unit->plan->reserve = FALSE;
    set_waiting_for_tasks(unit, FALSE);
    unit->plan->delayed = FALSE;
    unit->plan->last_task_outcome = TASK_UNKNOWN;
    need_ai_planning = TRUE;
}

/* Auxiliary functions for unit planning in Xconq. */

/* router flags */

#define SAMEPATH 1
#define EXPLORE_PATH 2

/* These macros are a cache used for planning purposes by machines. */

#define markloc(x, y) (set_tmp1_at(x, y, mark))

#define markedloc(x, y) (tmp1_at(x, y) == mark)

#define get_fromdir(x, y) (tmp2_at(x, y))

#define set_fromdir(x, y, dir) (set_tmp2_at(x, y, dir))

#define get_dist(x, y) (tmp3_at(x, y))

#define set_dist(x, y, d) (set_tmp3_at(x, y, d))

int
occupant_could_capture(Unit *unit, int u2)
{
    Unit *occ;

    for_all_occupants(unit, occ)
      if (uu_capture(occ->type, u2) > 0)
	return TRUE;
    return FALSE;
}

/* Check to see if there is anyone around to capture. */

int
can_capture_neighbor(Unit *unit)
{
    int d, x, y;
    Side *side2;
    UnitView *uview;

    for_all_directions(d) {
	if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
	    /* Should perhaps iterate over occs also, so that they can be 
	    captured even if the transport is not? */
	    for_all_view_stack(unit->side, x, y, uview) {
		side2 = side_n(uview->siden);
		if (!allied_side(unit->side, side2)) {
		    if (uu_capture(unit->type, uview->type) > 0) {
			/* need some other way to change move order quickly */
			return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

/* check if our first occupant can capture something.  Doesn't look at
   other occupants. */

int
occupant_can_capture_neighbor(Unit *unit)
{
    Unit *occ = unit->occupant;

    if (occ != NULL && has_acp_left(occ) && occ->side == unit->side) {
	if (can_capture_neighbor(occ)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* Find the closes unit, first prefering bases, and then transports. */

int
find_closest_unit(Side *side, int x0, int y0, int maxdist, int (*pred)(int x, int y)/*pred*/, int *rxp, int *ryp)
{
#if 0    
    Unit *unit;
    int u, dist;
    int found = FALSE;

    for_all_unit_types(u) {
	if (u_is_base(u)) {
	    for (unit = NULL /* side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit) &&
		    (dist = distance(x0, y0, unit->x, unit->y)) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
    for_all_unit_types(u) {
	if (!u_is_base(u) && u_is_transport(u)) {
	    for (unit = NULL /*side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit)
		    && distance(x0, y0, unit->x, unit->y) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
    /* (What's the point of finding a non-base/non-transport?) */
    for_all_unit_types(u) {
	if (!u_is_base(u) && !u_is_transport(u)) {
	    for (unit = NULL/*side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit)
		    && distance(x0, y0, unit->x, unit->y) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
#endif
    return FALSE;
}

#if 0		/* Unused. */

/* Returns the type of missing supplies. */

int
out_of_ammo(Unit *unit)
{
    int u = unit->type, m;

    for_all_material_types(m) {
	if (um_consumption_per_attack(u, m) > 0 && unit->supply[m] <= 0)
	    return m;
    }
    return (-1);
}

#endif

#if 0 	/* The two functions below are unused. */
int
usable_cell(Unit *unit, int x, int y)
{
    int u = unit->type;
    UnitView *uview;

    if (!could_live_on(u, terrain_at(x, y)))
      return FALSE;
    for_all_view_stack(unit->side, x, y, uview) {
	if (allied_side(uview->side, unit->side)
	    && could_carry(uview->type, u))
	  return TRUE;
    }
    return FALSE;
}

Task *explorechain;

int
explorable_cell(x, y)
int x, y;
{
    return (terrain_view(tmpside, x, y) == UNSEEN);
}

#endif

/* Test whether the given location is an unknown cell that we can get
   next to. */
/* (should consider testing "within vision range", but that might
   require LOS tests and be expensive) */

static int
reachable_unknown(int x, int y)
{
    /* Only interior cells are reachable. */
    if (!inside_area(x, y))
      return FALSE;
    if (terrain_view(tmpside, x, y) == UNSEEN) {
    	if (adj_known_ok_terrain(x, y, tmpside, tmpunit->type)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    } else {
	return FALSE;
    }
}

/* Test whether the given location has an adjacent cell that is ok for
   the given type to be out in the open. */

static int
adj_known_ok_terrain(int x, int y, Side *side, int u)
{
    int dir, x1, y1, t;

    if (!inside_area(x, y))
      return FALSE;
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    if (terrain_view(side, x1, y1) == UNSEEN)
	      continue;
	    t = terrain_at(x1, y1);
	    if (!terrain_always_impassable(u, t))
	      return TRUE;
	}
    }
    return FALSE;
}

/* Go to the nearest cell that we can see how to get to. */

static int
explore_reachable_cell(Unit *unit, int range)
{
    int x, y;

    if (g_see_all() || g_terrain_seen())
      return FALSE;
    tmpunit = unit;
    tmpside = unit->side;
    DMprintf("%s searching within %d for cell to explore -",
	     unit_desig(unit), range);
    if (search_around(unit->x, unit->y, range, reachable_unknown, &x, &y, 1)) {
	set_move_to_task(unit, x, y, 1);
	DMprintf("found one at %d,%d\n", x, y);
	return TRUE;
    }
    DMprintf("found nothing\n");
    return FALSE;
}

/* Estimate the usual number of turns to finish construction. */

int
normal_completion_time(int u, int u2)
{
    int acp = 0;

    acp = type_max_acp(u); /* (Watch for performance issues here.) */
    if (acp == 0 || uu_cp_per_build(u, u2) == 0)
      return (-1);
    return (u_cp(u2) - uu_creation_cp(u, u2)) /
      (uu_cp_per_build(u, u2) * acp);
}

/* Similar, but using a specific unit and also accounting for toolup
   time. */

int
est_completion_time(Unit *unit, int u2)
{
    int u, tooluptime, tp, acp;

    u = unit->type;
    if (!could_create(u, u2))
      return (-1);
    acp = type_max_acp(u); /* (Watch for performance issues here.) */
    tooluptime = 0;
    tp = (unit->tooling ? unit->tooling[u2] : 0);
    if (tp < uu_tp_to_build(u, u2)) {
	if (uu_acp_to_toolup(u, u2) < 1
	    || uu_tp_per_toolup(u, u2) <= 0
	    || acp <= 0)
	  return (-1);
	tooluptime = ((uu_tp_to_build(u, u2) - tp) * uu_acp_to_toolup(u, u2))
	 / (uu_tp_per_toolup(u, u2) * acp);
    }
    return tooluptime + normal_completion_time(unit->type, u2);
}

//! Maybe set side goal for a material, if needed for completion of u.

void
maybe_set_materials_goal(Unit *unit, int u2)
{
    int u = NONUTYPE;
    int m = NONMTYPE;
    Side *side = NULL;
    int conc = 0, cpb = 0;
    Goal *goal = NULL;

    assert_error(in_play(unit), "Attempted to access an out-of-play unit");
    assert_error(is_unit_type(u2), "Attempted to use an invalid utype");
    u = unit->type;
    side = unit->side;
    for_all_material_types(m) {
	conc = um_consumption_on_creation(u2, m);
	cpb = um_consumption_per_built(u2, m);
	cpb += (um_consumption_per_cp(u2, m) * uu_cp_per_build(u, u2));
	if (((0 < conc) || (0 < cpb)) && side_has_treasury(side, m)
	    && um_takes_from_treasury(u, m)) {
	    goal = create_goal(GOAL_HAS_MATERIAL_TYPE, side, TRUE);
	    goal->args[0] = m;
	    goal->args[1] = 3 * conc + cpb;
	    add_goal(side, goal);
	}
    }
}

/* A unit runs low on supplies at the halfway point.  Formula is the same
   no matter how/if occupants eat transports' supplies or whether 
   unit can borrow from treasury. Unless we could put in reservations 
   on the supplies, we would simply be making assumptions that might 
   not bear out when it actually comes time to consume the supplies. */

int
past_halfway_point(Unit *unit)
{
    int u = unit->type, m = NONMTYPE, rsplypct = -1;

    for_all_material_types(m) {
	if (((um_base_consumption(u, m) > 0)
	     || (um_consumption_per_move(u, m) > 0))
	    && (unit->transport == NULL)) {
            if (unit->side)
                rsplypct = unit_doctrine(unit)->resupply_percent;
            else
                rsplypct = 50;
            if (unit->supply[m] <= ((rsplypct * um_storage_x(u, m)) / 100))
                return TRUE;
	}
    }
    return FALSE;
}

// xConq
// Unit plan lifecycle management and ser/deser.

// $Id: plan.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1986-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Unit plan lifecycle management and ser/deser.
    \ingroup grp_gdl
*/

#include "gdl/gvars.h"
#include "gdl/unit/unit.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

char *plantypenames[] = {

#undef  DEF_PLAN
#define DEF_PLAN(NAME,code) NAME,

#include "gdl/unit/plan.def"

    NULL

};

// Local Functions: Queries

//! Get plan type from name.
static int lookup_plan_type(char *name);

// Local Functions: Lifecycle Management

//! Create new plan.
static Plan *create_plan(void);

// Queries

static
int
lookup_plan_type(char *name)
{
    int i;

    for (i = 0; plantypenames[i] != NULL; ++i)
      /* should get real enum */
      if (strcmp(name, plantypenames[i]) == 0)
	return i;
    return PLAN_NONE;
}

// Lifecycle Management

static
Plan *
create_plan(void)
{
    Plan *plan = (Plan *) xmalloc(sizeof(Plan));
    return plan;
}

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
free_plan(Unit *unit)
{
    Plan *plan = unit->plan;

    if (plan == NULL)
      run_error("no plan here?");
    /* Make tasks available for reallocation. */
    clear_task_agenda(unit);
    free(plan);
}

// Task Management

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

// GDL I/O

void
interp_unit_plan(Unit *unit, Obj *props)
{
    int isnumber, numval;
    Obj *bdg, *propval, *plantypesym, *val, *trest;
    char *propname;
    Goal *goal;
    Plan *plan;
    Task *task, *task1;

    if (unit->plan == NULL) {
	/* Create the plan explicitly, even if unit type doesn't allow it
	   (type might be changed later in the reading process). */
	plan = (Plan *) xmalloc(sizeof(Plan));
	/* From init_unit_plan: can't call it directly, might not behave
	   right (should fix to be callable from here - problem is that
	   other unit props such as cp might not be set right yet) */
	/* Allow AIs to make this unit do things. */
	plan->aicontrol = TRUE;
	/* Enable supply alarms by default. */
	plan->supply_alarm = TRUE;
	/* Attach to unit. */
	unit->plan = plan;
    }
    plantypesym = car(props);
    SYNTAX(props, symbolp(plantypesym), "plan type must be a symbol");
    plan->type = (PlanType)lookup_plan_type(c_string(plantypesym));
    props = cdr(props);
    val = car(props);
    if (numberp(val)) {
	plan->creation_turn = c_number(val);
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	numval = eval_number(propval, &isnumber);
	switch (keyword_code(propname)) {
	  case K_INITIAL_TURN:
	    plan->initial_turn = numval;
	    break;
	  case K_FINAL_TURN:
	    plan->final_turn = numval;
	    break;
	  case K_ASLEEP:
	    plan->asleep = numval;
	    break;
	  case K_RESERVE:
	    plan->reserve = numval;
	    break;
	  case K_DELAYED:
	    plan->delayed = numval;
	    break;
	  case K_WAIT:
	    plan->waitingfortasks = numval;
	    break;
	  case K_AI_CONTROL:
	    plan->aicontrol = numval;
	    break;
	  case K_SUPPLY_ALARM:
	    plan->supply_alarm = numval;
	    break;
	  case K_SUPPLY_IS_LOW:
	    plan->supply_is_low = numval;
	    break;
	  case K_WAIT_TRANSPORT:
	    plan->waitingfortransport = numval;
	    break;
	  case K_GOAL:
	    goal = interp_goal(cdr(bdg));
	    plan->maingoal = goal;
	    break;
	  case K_FORMATION:
	    goal = interp_goal(cdr(bdg));
	    plan->formation = goal;
	    /* Object patching will fill in plan's funit slot later. */
	    break;
	  case K_TASKS:
	    task1 = NULL;
	    for_all_list(cdr(bdg), trest) {
	    	task = interp_task(car(trest));
		if (task) {
		    /* Add tasks to the agenda in same order as saved. */
		    if (task1) {
			task1->next = task;
		    } else {
			plan->tasks = task;
		    }
		    task1 = task;
		}
	    }
	    break;
	  default:
	    unknown_property("unit plan", unit_desig(unit), propname);
	}
    }
}

void
write_unit_plan(Unit *unit)
{
    Task *task;
    Plan *plan = unit->plan;

    /* The plan is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (plan) {
	if (1) {
	    newline_form();
	    space_form();
	}
    	space_form();
    	start_form(key(K_PLAN));
    	add_to_form(plantypenames[plan->type]);
	add_num_to_form(plan->creation_turn);
	write_num_prop(key(K_INITIAL_TURN), plan->initial_turn, 0, FALSE, FALSE);
	write_num_prop(key(K_FINAL_TURN), plan->final_turn, 0, FALSE, FALSE);
	write_bool_prop(key(K_ASLEEP), plan->asleep, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_RESERVE), plan->reserve, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_DELAYED), plan->delayed, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_WAIT), plan->waitingfortasks, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_AI_CONTROL), plan->aicontrol, TRUE, FALSE, FALSE);
	write_bool_prop(key(K_SUPPLY_ALARM), plan->supply_alarm, TRUE, FALSE, FALSE);
	write_bool_prop(key(K_SUPPLY_IS_LOW), plan->supply_is_low, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_WAIT_TRANSPORT), plan->waitingfortransport, FALSE, FALSE, FALSE);
	if (plan->maingoal)
	  write_goal(plan->maingoal, K_GOAL);
	if (plan->formation)
	  write_goal(plan->formation, K_FORMATION);
	if (plan->tasks) {
    	    space_form();
    	    start_form(key(K_TASKS));
    	    for_all_tasks(plan, task) {
	    	space_form();
	    	write_task(task);
	    }
	    end_form();
	}
	end_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

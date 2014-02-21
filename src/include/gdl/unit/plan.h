// xConq
// Lifecycle management and serialization/deserialization of unit plans.

// $Id: plan.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Lifecycle management and serialization/deserialization of unit plans.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_PLAN_H
#define XCONQ_GDL_UNIT_PLAN_H

#include "gdl/lisp.h"
#include "gdl/unit/action.h"
#include "gdl/unit/task.h"
#include "gdl/unit/goal.h"

// Iterator Macros

#define for_all_tasks(plan,task)  \
  for (task = (plan)->tasks; task != NULL; task = task->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifndef DEF_PLAN
#undef  DEF_PLAN
#endif
#define DEF_PLAN(name,CODE) CODE,

//! Unit plan type.
/*!
    A plan is what a single unit uses to make decisions, both for itself and
    for any other units it commands.  Any unit that can act at all has a
    plan object.  A plan collects lots of unit behavior, but its most
    important structure is the task queue, which contains a list of what
    to do next, in order.
    Plan types distinguish several kinds of usages.
*/
typedef enum plantype {

#include "gdl/unit/plan.def"

    NUMPLANTYPES

} PlanType;

//! Unit plan.
typedef struct a_plan {
    //! Type of plan.
    PlanType type;
    //! Turn at which this plan was created on.
    short creation_turn;
    //! Turn at which this plan is to be executed.
    short initial_turn;
    //! Turn at which this plan is to be deactivated.
    short final_turn;
    //! True, if unit is to do nothing, unless awakened.
    short asleep;
    //! True, if unit is waiting to act until next turn.
    short reserve;
    //! True, if unit is waiting to act until later in turn.
    short delayed;
    //! True, if plan needs tasks.
    short waitingfortasks;
    //! True, if unit is under AI control.
    short aicontrol;
    //! True, if resupply alarm is armed.
    short supply_alarm;
    //! True, if unit needs to resupply.
    short supply_is_low;
    //! True, if unit is trying to hitch a ride.
    short waitingfortransport;
    //! Main goal associated with plan.
    struct a_goal *maingoal;
    //! Special goal: keep in formation.
    struct a_goal *formation;
    //! List of tasks.
    struct a_task *tasks;
    /* Not saved/restored. (little value, some trouble to do) */
    //! Formation leader, if there is one.
    struct a_unit *funit;
    //! Copy of last action attempted.
    Action lastaction;
    //! Outcome of last action attempted.
    short lastresult;
    //! Copy of last task executed.
    Task last_task;
    //! Outcome of last task executed.
    TaskOutcome last_task_outcome;
    //! How many times has plan executed this turn?
    short execs_this_turn;
} Plan;

// Global Variables

//! Array of plan type names.
extern char *plantypenames[];

// Lifecycle Management

//! Initialize unit's plan.
/*! Every unit that can act needs a plan object, types that can't act
    should have it cleared out.  Note that incomplete units are expected
    to be able to act in the future, so it's acceptable to run this for
    incomplete units to give them a plan.
*/
extern void init_unit_plan(Unit *unit);

//! Dissociate plan from unit and delete plan.
extern void free_plan(Unit *unit);

// Task Management

//! Delete all tasks in unit's plan.
extern int clear_task_agenda(Unit *unit);

// GDL I/O

//! Read unit plan from GDL form.
extern void interp_unit_plan(struct a_unit *unit, Obj *props);

//! Serialize unit plan to GDL.
extern void write_unit_plan(Unit *unit);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_PLAN_H

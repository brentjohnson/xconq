/* Unit task execution and general task functions.
   Copyright (C) 1992-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

/* (The following inclusion is only temporary if we end up proceeding
    with the extraction of all AI code from this file.) */
#include "aiutil.h"

#include "aiunit.h"
#include "aiunit2.h"

/* This is the number of tasks to allocate initially.  More will always be
   allocated as needed, so this should be a "reasonable" value. */

#ifndef INITMAXTASKS
#define INITMAXTASKS 100
#endif

enum choicestate {
    eitherway,
    leftthenright,
    rightthenleft,
    leftonly,
    rightonly
};

static int compare_directions(const void *a0, const void *a1);
static int test_for_buildable(int x, int y);
static int collect_test(int x, int y);
static int extractable_test(int x, int y);
static TaskOutcome execute_task_aux(Unit *unit, Task *task);

//! Perform a toolup subtask on behalf of a construct/build task.
static TaskOutcome do_toolup_subtask(Unit *constructor, int uc);

static TaskOutcome do_approach_subtask(Unit *unit, Task *task, int tx, int ty, int *statep);
static void allocate_task_block(void);
#if (0)
static Unit *repair_here(int x, int y);
#endif
static Unit *resupply_here(int x, int y);


/* Declare all the task functions. */

#undef  DEF_TASK
#define DEF_TASK(name,dname,code,argtypes,DOFN,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl)  \
  static TaskOutcome DOFN(Unit *unit, Task *task);

#include "task.def"

/* Array of descriptions of task types. */

TaskDefn taskdefns[] = {

#undef  DEF_TASK
#define DEF_TASK(NAME,DNAME,code,ARGTYPES,DOFN,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl)  \
  { NAME, DNAME, ARGTYPES, DOFN },

#include "task.def"

    { NULL, NULL, NULL }
};

/* The list of available task objects. */

Task *freetasks;

/* Pointer to a buffer that task debug info goes into. */

char *taskbuf;

static int tmpbuildutype;

static Unit *tmpbuilder, *tmpbuildunit;

/* Used by the resupply code. */

int *lowm = NULL;

int numlow = 0; 

static int tmpx, tmpy;

static int *foundm;

/* Allocate an initial collection of task objects. */

void
init_tasks(void)
{
    allocate_task_block();
}

/* Allocate a new block of tasks. */

void
allocate_task_block(void)
{
    int i;

    freetasks = (Task *) xmalloc(INITMAXTASKS * sizeof(Task));
    /* Chain the tasks together. */
    for (i = 0; i < INITMAXTASKS; ++i) {
	freetasks[i].next = &freetasks[i+1];
    }
    freetasks[INITMAXTASKS-1].next = NULL;
}

/* Create and return a new task. */

Task *
create_task(TaskType type)
{
    int i;
    Task *task;

    /* Scarf up some more memory if we need it. */
    if (freetasks == NULL) {
	allocate_task_block();
    }
    /* Peel off a task from the free list. */
    task = freetasks;
    freetasks = task->next;
    /* Reset its slots. */
    task->type = type;
    task->execnum = 0;
    task->retrynum = 0;
    for (i = 0; i < MAXTASKARGS; ++i)
      task->args[i] = 0;
    task->next = NULL;
    return task;
}

/* Make a copy of the given task. */

Task *
clone_task(Task *oldtask)
{
    int i;
    Task *newtask;

    newtask = create_task(oldtask->type);
    /* Probably not a good idea to copy exec/retry counts, skip them. */
    for (i = 0; i < MAXTASKARGS; ++i)
      newtask->args[i] = oldtask->args[i];
    return newtask;
}

/* The empty task always succeeds immediately. */

static TaskOutcome
do_none_task(Unit *unit, Task *task)
{
    return TASK_IS_COMPLETE;
}

static int
test_for_buildable(int x, int y)
{
    Unit *unit;

    for_all_stack(x, y, unit) {
	if (in_play(unit)
	    && !completed(unit)
	    && unit->type == tmpbuildutype
	    && unit->side == tmpbuilder->side) {
	    tmpbuildunit = unit;
	    return TRUE;
	}
    }
    return FALSE;
}

/* The build task handles the development, tooling up, creation, and
   completion for a given number of units of a given type. */

static TaskOutcome
do_build_task(Unit *builder, Task *task)
{
    int rslt = A_ANY_OK;
    int idc = -1;
    int uc = NONUTYPE;
    Unit *buildee = NULL;
    Side *side = NULL;

    assert_error(is_active(builder),
		 "AI: Attempted to build with an inactive unit");
    assert_warning_return(task, "AI: Attempted to run invalid construct task",
                          TASK_IS_COMPLETE);
    side = builder->side;
    // Attempt to retrieve buildee unit from ID.
    idc = task->args[0];
    buildee = find_unit(idc);
    if (!in_play(buildee))
	return TASK_FAILED;
    if (completed(buildee))
	return TASK_IS_COMPLETE;
    uc = buildee->type;
    builder->creation_id = -1;
    // If research or development is needed, then we should not be in this code.
    if (!side_can_build(side, uc))
        return TASK_FAILED;
    // We must handle tooling, if necessary.
    if (!has_enough_tooling(builder, uc))
	return do_toolup_subtask(builder, uc);
    // Try performing build action.
    if (valid(rslt = can_build(builder, builder, buildee))) {
	prep_build_action(builder, builder, buildee);
	return TASK_PREPPED_ACTION;
    }
    // Wait for more materials or ACP, if necessary.
    // TODO: Proactively accumulate materials, if possible.
    if ((A_ANY_NO_MATERIAL == rslt) || (A_ANY_NO_ACP == rslt)) {
        set_unit_reserve(builder->side, builder, TRUE, FALSE);
        return TASK_IS_INCOMPLETE;
    }
    return TASK_FAILED;
#if (0) // HACKING NOTE: Old devel logic. Keep around for now.
    /* See if our technology needs improvement in order to build this
       type. */
    if (is_unit_type(u2)
	&& u_tech_to_build(u2) > 0
	&& us->tech[u2] < u_tech_to_build(u2)) {
        if (uu_acp_to_develop(u, u2) > 0) {
	    push_develop_task(unit, u2, u_tech_to_build(u2));
	    return TASK_IS_INCOMPLETE;
        } else {
	    /* Can't do the necessary development. */
	    /* (should filter out when asking to build?) */
	    notify(unit->side,
		   "You need tech of %d to build %s, but are only at %d.",
		   u_tech_to_build(u2), u_type_name(u2), us->tech[u2]);
	    notify(unit->side,
		   "%s cannot develop %s, so build task failed.",
		   unit_handle(unit->side, unit), u_type_name(u2));
	    return TASK_FAILED;
        }
    }
#endif
}

Unit *
find_unit_to_complete(Unit *unit, Task *task)
{
    Unit *occ;
    int u = unit->type, nx, ny, range;
    int u2 = task->args[0];
    int x = unit->x, y = unit->y;

    /* Check out the unit supposedly in progress. */
    if (task->args[1] != 0) {
	occ = find_unit(task->args[1]);
	if (in_play(occ) && occ->type == u2 && !completed(occ)) 
	  return occ;
    }
    /* Maybe search for any appropriate incomplete occupants. */
    for_all_occupants(unit, occ) {
	if (in_play(occ)
	    && !completed(occ)
	    && occ->type == u2
	    && occ->side == unit->side) 
	  return occ;
    }
    /* Or else search for any appropriate incomplete units in this cell. */
    for_all_stack(x, y, occ) {
	if (in_play(occ)
	    && !completed(occ)
	    && occ->type == u2
	    && occ->side == unit->side) {
	    return occ;
    	}
    }
    /* Or else search nearby area. */
    if (is_unit_type(u2)) {
	range = uu_build_range(u, u2);
	if (range > 0) {
	    tmpbuilder = unit;
	    tmpbuildutype = u2;
	    if (search_around(x, y, range, test_for_buildable, &nx, &ny, 1)) {
	    	return tmpbuildunit;
	    }
	}
    }
    /* nothing found */
    return NULL;
}

/* This is a "pure development" task, with the sole objective of
   increasing technology. */

static TaskOutcome
do_develop_task(Unit *unit, Task *task)
{
    int u = unit->type;
    int u2 = task->args[0], lev = task->args[1];
    Side *us = unit->side;

    /* Independents can never ever do development. */
    if (us == NULL)
      return TASK_FAILED;
    if (us->tech[u2] > u_tech_max(u2))
      run_error("s%d tech for u%d is %d", side_number(us), u2, us->tech[u2]);
    if (us->tech[u2] >= lev)
      return TASK_IS_COMPLETE;
    if (uu_acp_to_develop(u, u2) <= 0)
      return TASK_FAILED;
    if (valid(check_develop_action(unit, unit, u2))) {
	prep_develop_action(unit, unit, u2);
	return TASK_PREPPED_ACTION;
    } else {
	/* We get three tries to develop before giving up. */
	return (task->execnum < 3 ? TASK_IS_INCOMPLETE : TASK_FAILED);
    }
}

/* This is to capture a given type/side of unit at a given place. */

static TaskOutcome
do_capture_task(Unit *unit, Task *task)
{
    int u = NONUTYPE, tu = NONUTYPE;
    Side *side = NULL, *tside = NULL;
    int n = -1;
    Unit *target = NULL;
    UnitView *uvtarget = NULL;
    int capmethod = CAPTURE_METHOD_ANY;
    int tx = -1, ty = -1, dist = -1;
    int cancapture = FALSE, canattack = FALSE, canfire = FALSE;

    n = task->args[2];
    // Get out, if we have exceeded number of capture attempts.
    if (task->execnum > n)
	return TASK_IS_COMPLETE;
    u = unit->type;
    side = unit->side;
    // NOTE: Looking up the actual unit is cheating.
    //  We need the uview by ID, not the unit.
    target = find_unit(task->args[0]);
    if (!in_play(target))
        return TASK_FAILED;
    uvtarget = find_unit_view(side, target);
    if (!uvtarget)
        return TASK_FAILED;
    capmethod = task->args[1];
    tx = uvtarget->x;  ty = uvtarget->y;
    tu = uvtarget->type;
    tside = side_n(uvtarget->siden);
    dist = distance(tx, ty, unit->x, unit->y);
    canattack = ((capmethod & CAPTURE_METHOD_ATTACK)
		 && valid(check_attack_action(unit, unit, target, 100)));
    canfire = ((capmethod & HIT_METHOD_FIRE)
	      && (dist <= (uu_zoc_range(u, tu) + 1))
              && valid(check_fire_at_action(unit, unit, target, -1)));
    cancapture = ((capmethod & CAPTURE_METHOD_CAPTURE)
		  && valid(check_capture_action(unit, unit, target)));
    if (cancapture) {
	prep_capture_action(unit, unit, target);
	return TASK_PREPPED_ACTION;
    }
    else if (canattack) {
	prep_attack_action(unit, unit, target, 100);
        return TASK_PREPPED_ACTION;
    }
    else if (canfire) {
        prep_fire_at_action(unit, unit, target, -1);
        return TASK_PREPPED_ACTION;
    }
    // TODO: Handle using transports or occs to perform capture.
    // TODO: Handle using detonations to perform capture.

#if (0)
    /* (should be able to say how hard to try) */
    tx = task->args[0];  ty = task->args[1];
    tu2 = task->args[2];
    ts2 = task->args[3];
    dist = distance(tx, ty, unit->x, unit->y);
    switch (dist) {
      case 0:
      case 1:
	for_all_stack(tx, ty, unit2) {
	    if ((ts2 >= 0 ?
		 (unit2->side->id == ts2) :
		 enemy_side(us, unit2->side))
		&& (tu2 == NONUTYPE || tu2 == unit2->type)) {
		if (valid(check_capture_action(unit, unit, unit2))) {
		    prep_capture_action(unit, unit, unit2);
		    return TASK_PREPPED_ACTION;
		} else if (valid(check_attack_action(unit, unit, unit2, 100))) {
		    prep_attack_action(unit, unit, unit2, 100);
		    return TASK_PREPPED_ACTION;
		} else {
		    /* We get several tries to capture before giving up. */
		    set_unit_reserve(unit->side, unit, TRUE, FALSE);
		    return (task->execnum < 5 ? TASK_IS_INCOMPLETE : TASK_FAILED);
		}
	    }
	}
	/* Nothing was here to capture. */
	return TASK_IS_COMPLETE;
      default:
	/* If on mobile transport, let it handle things. */
	if (unit->transport != NULL
	    && mobile(unit->transport->type)
	    /* and the transport is not blocked */
	    && flip_coin()) {
	    return TASK_IS_INCOMPLETE;
	}
	/* If out of range and can move, push a task to get closer
           (usually). */
	if (mobile(u) && probability(90)) {
	    push_move_to_task(unit, tx, ty, 1);
	    return TASK_IS_INCOMPLETE;
	}
	return TASK_FAILED;
    }
#endif
    return TASK_FAILED;
}

/* The disband task's purpose is to make the unit disappear, so just
   keep doing actions; when the unit goes away, so will the task. */

static TaskOutcome
do_disband_task(Unit *unit, Task *task)
{
    if (valid(check_disband_action(unit, unit))) {
	prep_disband_action(unit, unit);
	return TASK_PREPPED_ACTION;
    } else {
	/* (should try to find a nearby unit to do it?) */
	return (task->execnum < 5 ? TASK_IS_INCOMPLETE : TASK_FAILED);
    }
}

static TaskOutcome
do_hit_position_task(Unit *unit, Task *task)
{
    int u = unit->type, tx, ty, dist;
    UnitView *uview = NULL;

    /* (Temporary hack. Ask the planner to re-evaluate continuation of 
       this task. If it wants to, then it will issue a new one. If not, 
       then move on and do something more productive.) */
    /* (A better solution would be to add a new arg to the task that would 
       tell it the number of times to attempt execution before returning to 
       the planner for guidance.) */
    if (task->execnum > 3)
      return TASK_IS_COMPLETE;
    /* This is to hit a given place. */
    /* (ask for a number of hits?) */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    /* Make sure that we are not going to hit a friendly unit. */
    /* (What if we have unit with vision-range -1 in cell and they aren't 
       seen by any of our own units?) */
    for_all_view_stack_with_occs(unit->side, tx, ty, uview) {
        if (!enemy_side(unit->side, side_n(uview->siden)))
          return TASK_FAILED;
    } 
    /* Try performing a fire-into action. */
    if (valid(check_fire_into_action(unit, unit, tx, ty, 0, -1))) {
	prep_fire_into_action(unit, unit, tx, ty, 0, -1);
	return TASK_PREPPED_ACTION;
    } else if (mobile(u) && flip_coin()) {
	/* We're too far away to shoot directly, add a move-to task. */
	push_move_to_task(unit, tx, ty, max(1 /* attack range */, u_range(u)));
	return TASK_IS_INCOMPLETE;
    }
    return TASK_FAILED;
}

static TaskOutcome
do_hit_unit_task(Unit *unit, Task *task)
{
    int u = NONUTYPE, tu = NONUTYPE;
    Side *side = NULL, *tside = NULL;
    Unit *target = NULL;
    UnitView *uvtarget = NULL;
    int n = -1;
    int tx = -1, ty = -1, dist = -1;
    int hitmethod = HIT_METHOD_NONE;
    int canattack = FALSE, canfire = FALSE, candet = FALSE;

    n = task->args[2];
    // Get out, if we have exceeded number of hit attempts.
    if (task->execnum > n)
      return TASK_IS_COMPLETE;
    u = unit->type;
    side = unit->side;
    // NOTE: Looking up the actual unit is cheating.
    //	We need the uview by ID, not the unit.
    target = find_unit(task->args[0]);
    if (!in_play(target))
	return TASK_FAILED;
    uvtarget = find_unit_view(side, target);
    if (!uvtarget)
	return TASK_FAILED;
    hitmethod = task->args[1];
    tx = uvtarget->x;
    ty = uvtarget->y;
    tu = uvtarget->type;
    tside = side_n(uvtarget->siden);
    dist = distance(tx, ty, unit->x, unit->y);
    canattack = ((hitmethod & HIT_METHOD_ATTACK)
		&& valid(check_attack_action(unit, unit, target, 100)));
    canfire = ((hitmethod & HIT_METHOD_FIRE)
	      && valid(check_fire_at_action(unit, unit, target, -1)));
    candet = ((hitmethod & HIT_METHOD_DETONATE)
	     && valid(can_detonate(unit, unit))
	     && (dist >= uu_detonation_range(u, tu)));
    if (canattack) {
	prep_attack_action(unit, unit, target, 100);
	return TASK_PREPPED_ACTION;
    }
    else if (canfire) {
	prep_fire_at_action(unit, unit, target, -1);
	return TASK_PREPPED_ACTION;
    }
    else if (candet) {
	prep_detonate_action(unit, unit, tx, ty, unit->z);
	return TASK_PREPPED_ACTION;
    }
    // TODO: Handle using transports or occs to perform hit.

#if (0)
    if (dist <= 1) {
	if (can_attack_any(unit, unit)) {
	    for_all_view_stack(us, tx, ty, uview) {
		if (ts == uview->siden && tu == uview->type) {
		    unit2 = view_unit(uview);
		    if (unit2 
			&& valid(check_attack_action(unit, unit, unit2, 100))) {
			prep_attack_action(unit, unit, unit2, 100);
			return TASK_PREPPED_ACTION;
		    }
		}
	    }
	    /* Maybe we can overrun, but not if our cell is known to
	       be clear of enemies. */
	    if (dist > 0
	        && valid(
		    check_overrun_action(unit, unit, tx, ty, unit->z, 100))) {
		prep_overrun_action(unit, unit, tx, ty, unit->z, 100);
		return TASK_PREPPED_ACTION;
	    }
	}
	/* Might be able to fire at pointblank range. */
	if (can_fire_at_any(unit, unit) && dist >= u_range_min(u)) {
	    for_all_view_stack(us, tx, ty, uview) {
		if (ts == uview->siden && tu == uview->type) {
		    unit2 = view_unit(uview);
		    if (unit2 
			&& valid(check_fire_at_action(unit, unit, unit2, -1))) {
			prep_fire_at_action(unit, unit, unit2, -1);
			return TASK_PREPPED_ACTION;
		    }
		}
	    }
	}
	return TASK_FAILED;
    }
    if (dist < u_range_min(u)) {
	// TODO: Retreat from target, and then fire.
	return TASK_FAILED;
    }
    /* If we're within firing range, attempt to fire. */
    if (dist <= u_range(u)) {
	for_all_view_stack(us, tx, ty, uview) {
	    if (ts == uview->siden && tu == uview->type) {
		unit2 = view_unit(uview);
		if (unit2 
		    && valid(check_fire_at_action(unit, unit, unit2, -1))) {
		    prep_fire_at_action(unit, unit, unit2, -1);
		    return TASK_PREPPED_ACTION;
		}
	    }
	}
	return TASK_FAILED;	    
    }
    if (!target_visible(unit, task))
      return TASK_FAILED;
    /* If on mobile transport, let it handle things. */
    if (unit->transport != NULL
        && mobile(unit->transport->type)
	/* and the transport is not blocked */
        && flip_coin()) {
        return TASK_IS_INCOMPLETE;
    }
    /* If out of range and can move, push a task to get closer (maybe). */
    if (mobile(u) && flip_coin()) {
	movedist = max(1 /* attack range */, u_range(u));
	if (dist > movedist + u_acp(u) /* or dist that could be covered in 1-2 turns */) {
	    movedist = max(movedist, (dist - movedist) / 4);
	}
	push_move_to_task(unit, tx, ty, movedist);
	return TASK_IS_INCOMPLETE;
    }
#endif
    return TASK_FAILED;
}

int
target_visible(Unit *unit, Task *task)
{
    int tx, ty, tu, ts;
    Side *us = unit->side;
    UnitView *uview;

    tx = task->args[0];  ty = task->args[1];
    tu = task->args[2];  ts = task->args[3];

    for_all_view_stack_with_occs(us, tx, ty, uview) {
	if (ts == uview->siden
	    && (tu == NONUTYPE || tu == uview->type)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* Move in a straight line, go through things or stop rather than
   going around. */

static TaskOutcome
do_move_dir_task(Unit *unit, Task *task)
{
    int dir, tx, ty;
    Unit *unit2 = NULL;
    UnitView *uview = NULL;
    Side *side = NULL;
    int u = NONUTYPE;
    int rslt = A_ANY_CANNOT_DO;
    int curmp = 0, fullmp = 0;

    side = unit->side;
    u = unit->type;
    if ((task->args[1])-- > 0) {
	dir = task->args[0];
	/* Is the next cell even valid? */
	if (!point_in_dir(unit->x, unit->y, dir, &tx, &ty)) {
	    return TASK_FAILED;
	}
	/* Can we squeeze into the cell in any way, shape, or form? */
	if (!side_thinks_it_can_put_type_at(side, u, tx, ty))
	  return TASK_FAILED;
	/* Can the unit move into the cell? */
	if (valid(check_move_action(unit, unit, tx, ty, 0))) {
	    /* (Probably need to set reserve here for low ACP and MP cases, 
		like we do for the enter action below.) */
	    prep_move_action(unit, unit, tx, ty, 0);
	    return TASK_PREPPED_ACTION;
	} 
	/* Can the unit occupy another unit in the cell? */
	else if (unit_view_at(side, tx, ty)) {
	    for_all_view_stack(side, tx, ty, uview) {
		/* (A spy could enter untrusted unit...) */
		if (!trusted_side(side, side_n(uview->siden)))
		  continue;
		unit2 = view_unit(uview);
		rslt = check_enter_action(unit, unit, unit2);
		/* If not enough ACP left this turn, then wait. */
		if ((A_ANY_NO_ACP == rslt) 
		    && (can_have_enough_acp(unit, 
					    uu_acp_to_enter(u, uview->type)))) {
		    set_unit_reserve(side, unit, TRUE, FALSE);
		    return TASK_IS_INCOMPLETE;
		}
		/* If not enough MP left this turn, then wait. */
		else if (A_MOVE_NO_MP == rslt) {
		    if (can_be_actor(unit)) {
			curmp = (unit->act->acp * unit_speed(unit, tx, ty)) 
				/ 100 + u_free_mp(u);
			fullmp = ((total_acp_for_turn(unit) - u_acp_min(u)) *
				   unit_speed(unit, tx, ty)) / 100 + 
				  u_free_mp(u);
		    }
		    else {
		      curmp = 0; fullmp = 0;
		    }
		    if (fullmp > curmp) {
			set_unit_reserve(side, unit, TRUE, FALSE);
			return TASK_IS_INCOMPLETE;
		    }
		}
		/* Else, we can enter now, then do it. */
		else if (valid(rslt)) {
		    prep_enter_action(unit, unit, unit2);
		    return TASK_PREPPED_ACTION;
		}
	    } /* for_all_view_stack */
	    return TASK_FAILED;
	} 
	else {
	    return TASK_FAILED;
	}
    } /* if ((task->args[1])-- > 0) */
    /* Specified number of moves has already been executed. */
    else {
	return TASK_IS_COMPLETE;
    }
}

#if (0)
/* If we are sitting in the same cell or transport as a resupply source, 
   resupply is "free" for both us and the resupply source, and the 
   resupply source has some supplies that we could stock up on, then 
   swipe the supplies before moving on. */
int
stock_up_materials_for_free(Unit *unit)
{
    Unit *supplier = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int m = NONMTYPE;

    u = unit->type;
    if (!has_full_amount_of_all_materials(unit)) {
        for_all_stack(unit->x, unit->y, supplier) {
            if (enemy_side(unit->side, supplier->side))
              continue;
            if (!(supplier->supply))
              continue;
            u2 = supplier->type;
            for_all_material_types(m) {
    int dir;
                /* TODO: Finish implementing. */
            }
        }
    }
    return FALSE;
}
#endif

static int
could_directly_board_ferry(Unit * unit, Unit * transport)
{
    return (can_occupy(unit, transport) && 
	        valid(check_enter_action(unit, unit, transport)));
}

/* The move-to task is the main way for units to get from point A to
   point B.  In addition to the destination, the task has a required
   distance, so it will succeed if the unit is within that distance to
   the nominal destination. */

static TaskOutcome
do_move_to_task(Unit *unit, Task *task)
{
    int dist, tx, ty, check;
    Unit *unit2, *occ;
    int canceltask = FALSE;
    Task *tmptask = NULL;

    /* This task is to get to a designated location somehow. */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    if (dist <= task->args[3]) {
#if (0)
	/* If unit is a mobile transport, then don't get jittery. */
	if (unit->occupant && mobile(unit->type)) {
	    for_all_occupants(unit, occ) {
		if (has_acp_left(occ) && unit->plan 
		    && !unit->plan->reserve) {
		    /* delay_unit(unit, TRUE); */
		    return TASK_IS_INCOMPLETE;
		}
	    }
	}
#endif
	return TASK_IS_COMPLETE;
    }
#if (0)
    /* Try to top off supplies if supplier around and supply transfer 
       does not cost any ACP. */
    if (stock_up_materials_for_free(unit))
      return TASK_IS_INCOMPLETE;
#endif
    /* Abort the move-to, if it is associated with a resupply task that is 
       no longer necessary. Some games will run supply lines while a unit is 
       en route to resupply, and thereby negate the need to explicitly 
       move to a resupply point and resupply. */
    if (task->next && (TASK_RESUPPLY == task->next->type)) {
        if (NONMTYPE == task->next->args[0]) {
	    if (has_full_amount_of_all_materials(unit))
	      canceltask = TRUE;
	}
        else if (has_full_amount_of_material(unit, task->next->args[0]))
          canceltask = TRUE;
        if (canceltask) {
            tmptask = task->next;
            task->next = task->next->next;
            free_task(tmptask);
            /* Lie, so that we can hopefully replan or retask. */
            /* (Should have new TaskResult, TASK_CANCELLED.) */
            return TASK_IS_COMPLETE;
        }
    } 
#if (0)
    /* Abort the move-to, if it is not associated with a resupply 
	or entry task, and the unit is AI-controlled, 
	and the distance to the destination 
	is > the real operating range of the unit, 
	or unit has less fuel than its doctrine threshold, 
	and the unit is not in a transport. */
    if ((!task->next 
	 || (task->next
	     && ((task->next->type != TASK_RESUPPLY) 
		 && (task->next->type != TASK_OCCUPY))))
	&& ai_controlled(unit)
	&& (((dist - task->args[3]) > real_operating_range_best(unit))
	    || past_halfway_point(unit))
	&& !unit->transport) {
	clear_task_agenda(unit);
	/* Lie, so that we can hopefully replan or retask. */
	/* (Should have new TaskResult, TASK_CANCELLED.) */
	return TASK_IS_COMPLETE;
    }
#endif
    switch (dist) {
      case 0:
      /* We're there already, nothing more to do. */
      return TASK_IS_COMPLETE;
      case 1:
	/* Adjacent cell, do a single move. */
	/* But first, if there are units here already, prefer to
	   interact with them. */
	for_all_stack(tx, ty, unit2) {
	    /* If there's somebody that we can enter, prefer to do that. */
	    if (can_occupy(unit, unit2)
		&& valid(check_enter_action(unit, unit, unit2))) {
		prep_enter_action(unit, unit, unit2);
		return TASK_PREPPED_ACTION;
	    }
	    /* Perhaps an occupant... */
	    /* (We could use the recursive occupant test, but if
	       things are that complicated, the player should probably
	       exercise manual control here.) */
	    for_all_occupants(unit2, occ) {
		if (can_occupy(unit, occ)
		    && valid(check_enter_action(unit, unit, occ))) {
		    prep_enter_action(unit, unit, occ);
		    return TASK_PREPPED_ACTION;
		}
	    }
	}
#if 0 /* auto-attack on move should be player-controlled... */
		if (!trusted_side(unit->side, unit2->side)) {
		    /* This is probably not a good idea, combat odds not
		       taken into account. */
		    if (valid(check_attack_action(unit, unit, unit2, 100))) {
			prep_attack_action(unit, unit, unit2, 100);
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
		}
#endif
	/* Now try a basic move action. */
	if (valid(check = check_move_action(unit, unit, tx, ty, unit->z))) {
	    /* Moving into an empty cell. */
	    prep_move_action(unit, unit, tx, ty, unit->z);
	    return TASK_PREPPED_ACTION;
	} else {
	    /* If we're just short on mp, wait until the next turn to
               try to move. */
	    if (check == A_MOVE_NO_MP) {
		notify(unit->side,
		       "%s is resting until next turn.",
		       unit_handle(unit->side, unit));
		set_unit_reserve(unit->side, unit, TRUE, FALSE);
		return TASK_IS_INCOMPLETE;
	    }
	    Dprintf("%s move action fails check, result is %s\n",
		    unit_desig(unit), hevtdefns[check].name);
	    return TASK_FAILED;
	}
	break;
      default:
	if (dist <= u_move_range(unit->type)
	    && valid(check_move_action(unit, unit, tx, ty, unit->z))) {
	    prep_move_action(unit, unit, tx, ty, unit->z);
	    return TASK_PREPPED_ACTION;
	} else if (dist == 2
		   && valid(check_move_action(unit, unit, tx, ty, unit->z))) {
	    /* Border slide check. */
	    prep_move_action(unit, unit, tx, ty, unit->z);
	    return TASK_PREPPED_ACTION;
	} else {
	    /* Still some distance away, pick a way to go. */
	    return do_approach_subtask(unit, task, tx, ty, &(task->args[4]));
	}
    }
    return TASK_FAILED;
}

static TaskOutcome
do_approach_subtask(Unit *unit, Task *task, int tx, int ty, int *statep)
{
    int nx, ny, dirs[NUMDIRS], numdirs, i, numdirs2, check;
    Unit *unit2;

#if (0)
    /* If on mobile transport, let it handle things. */
    /* Comment: Umm, no. What if unit is trying to get off mobile transport? */
    if (unit->transport != NULL
	&& mobile(unit->transport->type)
	/* and the transport is not stuck */
	&& probability(95)) {
	set_unit_reserve(unit->side, unit, TRUE, FALSE);
	return TASK_IS_INCOMPLETE;
    }
#endif
    numdirs = 
	choose_move_dirs(unit, tx, ty, TRUE, plausible_move_dir, 
			 sort_directions, dirs);
    if (!numdirs)
      numdirs = 
	choose_move_dirs(unit, tx, ty, FALSE, plausible_move_dir, 
			 sort_directions, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &nx, &ny);
	for_all_stack(nx, ny, unit2) {
	    if (can_occupy(unit, unit2)) {
		if (valid(check_enter_action(unit, unit, unit2))) {
		    prep_enter_action(unit, unit, unit2);
		    /* We (probably) made forward progress, 
			so reopen choice of dirs. */
		    *statep = eitherway;
		    return TASK_PREPPED_ACTION;
		} else {
		    continue;
		}
	    } else if (!trusted_side(unit->side, unit2->side)) {
		if (unit->occupant) {
		    /* More important to find a way through. */
		    continue;
		} else {
		    /* This will encourage some re-evaluation. */
		    return TASK_FAILED;
		}
#if 0 /* the following is rarely a good idea */
		if (valid(check_attack_action(unit, unit, unit2, 100))) {
		    prep_attack_action(unit, unit, unit2, 100);
		    /* We (probably) made forward progress, so reopen choice of dirs. */
		    *statep = eitherway;
		    return TASK_PREPPED_ACTION;
		} else {
		    continue;
		}
#endif
	    }
	}
	if (valid(check_move_action(unit, unit, nx, ny, unit->z))) {
	    prep_move_action(unit, unit, nx, ny, unit->z);
	    /* We (probably) made forward progress, so reopen choice of dirs. */
	    *statep = eitherway;
	    return TASK_PREPPED_ACTION;
	}
    }
    /* Get both right and left non-decreasing dirs. */
    numdirs  = choose_move_dirs(unit, tx, ty, TRUE, NULL, NULL, dirs);
    numdirs2 = choose_move_dirs(unit, tx, ty, FALSE, NULL, NULL, dirs);
    for (i = numdirs; i < numdirs2; ++i) {
	if (plausible_move_dir(unit, dirs[i])) {
	    switch (*statep) {
	      case eitherway:
		if (i == numdirs)
		  *statep = leftonly /* leftthenright */;
		if (i == numdirs+1)
		  *statep = rightonly /* rightthenleft */;
		break;
#if 0
	      case leftthenright:
		if (i == numdirs)
		  *statep = rightonly;
		if (i == numdirs+1)
		  *statep = rightonly;
		continue;
		break;
	      case rightthenleft:
		if (i == numdirs+1)
		  *statep = leftonly;
		continue;
		break;
#endif
	      case leftonly:
		if (i == numdirs+1)
		  continue;
		break;
	      case rightonly:
		if (i == numdirs)
		  continue;
		break;
	      default:
		run_warning("Weird right/left state %d", *statep);
		*statep = leftonly;
		break;
	    }
	} else {
	    switch (*statep) {
	      case eitherway:
		if (i == numdirs)
		  *statep = rightonly;
		if (i == numdirs+1)
		  *statep = leftonly;
		continue;
		break;
#if 0
	      case leftthenright:
		if (i == numdirs)
		  *statep = rightonly;
		if (i == numdirs+1)
		  *statep = rightonly;
		continue;
		break;
	      case rightthenleft:
		if (i == numdirs+1)
		  *statep = leftonly;
		continue;
		break;
#endif
	      case leftonly:
		if (i == numdirs)
		  return TASK_FAILED;
		if (i == numdirs+1)
		  continue;
		break;
	      case rightonly:
		if (i == numdirs)
		  continue;
		if (i == numdirs+1)
		  return TASK_FAILED;
		break;
	      default:
		run_warning("Weird right/left state %d", *statep);
		*statep = leftonly;
		break;
	    }
	}
	point_in_dir(unit->x, unit->y, dirs[i], &nx, &ny);
	for_all_stack(nx, ny, unit2) {
	    if (can_occupy(unit, unit2)) {
		if (valid(check_enter_action(unit, unit, unit2))) {
		    prep_enter_action(unit, unit, unit2);
		    return TASK_PREPPED_ACTION;
		} else {
		    continue;
		}
	    } else if (!trusted_side(unit->side, unit2->side)) {
		if (unit->occupant) {
		    /* More important to find a way through. */
		    continue;
		} else {
		    /* This will encourage some re-evaluation. */
		    return TASK_FAILED;
		}
#if 0				/* the following is rarely a good idea */
		if (valid(check_attack_action(unit, unit, unit2, 100))) {
		    prep_attack_action(unit, unit, unit2, 100);
		    return TASK_PREPPED_ACTION;
		} else {
		    continue;
		}
#endif
	    }
	}
	if (valid(check = check_move_action(unit, unit, nx, ny, unit->z))) {
	    prep_move_action(unit, unit, nx, ny, unit->z);
	    return TASK_PREPPED_ACTION;
	}
	/* If we're just short on mp, wait until the next turn to try to move. */
	if (check == A_MOVE_NO_MP) {
	    if (unit->side)
	      notify(unit->side, "%s is resting until next turn.",
				 unit_handle(unit->side, unit));
	    set_unit_reserve(unit->side, unit, TRUE, FALSE);
	    return TASK_IS_INCOMPLETE;
	}
    }
    return TASK_FAILED;
}

static TaskOutcome
do_occupy_task(Unit *unit, Task *task)
{
    int dist;
    Unit *transport = NULL;
    int tx = -1;
    int ty = -1;

    transport = find_unit_dead_or_alive(task->args[0]);
    /* Transport may have left play in the interim between attempted task 
       executions.
    */
    if (!transport || !in_play(transport)) {
        if (transport) {
          DMprintf("%s attempted to enter out-of-play transport %s.\n", 
                    unit_desig(unit), unit_desig(transport));
        } else {
          DMprintf("%s attempted to enter a bogus transport with id %d.\n",
                   unit_desig(unit), task->args[0]);
        }
        return TASK_FAILED;
    }

    tx = transport->x;
    ty = transport->y;
    /* (should also fail if we don't know where transport is anymore) */
    if (unit->transport == transport) {
	return TASK_IS_COMPLETE;
    }
    dist = distance(unit->x, unit->y, transport->x, transport->y);
    if (dist <= 1) {
	if (valid(check_enter_action(unit, unit, transport))) {
	    prep_enter_action(unit, unit, transport);
	    return TASK_PREPPED_ACTION;
	} else {
	    /* Try a couple times, then fail if not working. */
	    return (task->execnum < 3 ? TASK_IS_INCOMPLETE : TASK_FAILED);
	}
    } else {
	/* Still some distance away, pick a way to go. */
	return do_approach_subtask(unit, task, transport->x, transport->y,
				   &(task->args[1]));
    }
}

/* Wait around for a particular unit.  Give up if the unit is not
   forthcoming. */

static TaskOutcome
do_pickup_task(Unit *unit, Task *task)
{
    Unit *occupant = NULL;

    occupant = find_unit_dead_or_alive(task->args[0]);
    /* Potential occupant may have left play in the interim between 
       attempted task executions. */
    if (!occupant || !in_play(occupant)) {
        if (occupant) {
          DMprintf("%s attempted to pickup out-of-play unit %s.\n",
                    unit_desig(unit), unit_desig(occupant));
        } else {
          DMprintf("%s attempted to pickup a bogus unit with id %d.\n",
                   unit_desig(unit), task->args[0]);
        }
        return TASK_FAILED;
    }
    wake_unit(occupant->side, occupant, FALSE);
    if (occupant->transport == unit)
      return TASK_IS_COMPLETE;
    if (distance(unit->x, unit->y, occupant->x, occupant->y) > 1)
      return do_approach_subtask(unit, task, occupant->x, occupant->y,
				 &(task->args[1]));
    if (task->execnum > 10) {
	/* Waiting around isn't working for us, give up.  If the
	   prospective occupant still needs us, we'll get another
	   call. */
	return TASK_FAILED;
    } else {
	if (valid(check_enter_action(occupant, occupant, unit))) {
	    prep_enter_action(occupant, occupant, unit);
	    return TASK_PREPPED_ACTION;
	} else if (valid(check_enter_action(unit, occupant, unit))) {
	    prep_enter_action(unit, occupant, unit);
	    return TASK_PREPPED_ACTION;
	} else {
	    return (task->execnum < 5 ? TASK_IS_INCOMPLETE : TASK_FAILED);
	}
    }
}

static TaskOutcome
do_produce_task(Unit *unit, Task *task)
{
    int m, tot, sofar, amt;

    m = task->args[0];
    if (!is_material_type(m)) {
	/* This may be an indication of code bogosity; warn? */
	return TASK_FAILED;
    }
    tot = task->args[1];
    sofar = task->args[2];
    if (sofar >= tot)
      return TASK_IS_COMPLETE;
    amt = um_material_per_production(unit->type, m);
    if (valid(check_produce_action(unit, unit, m, amt))) {
	task->args[2] += amt;
	prep_produce_action(unit, unit, m, amt);
	return TASK_PREPPED_ACTION;
    }
    return TASK_FAILED;
}

/* The collection task runs a series of extraction actions,
   transferring the collected material to another, given, unit. */

static int tmpm;

static Unit *collect_here(int x, int y);
static Unit *aux_collect_here(Unit *unit);

static TaskOutcome
do_collect_task(Unit *unit, Task *task)
{
    int m, x, y, dir, x1, y1, x2, y2, range;
    Unit *collector, *unit3;

    m = task->args[0];
    x = task->args[1];  y = task->args[2];
    /* Set up tmp globals early, all steps use these. */
    tmpunit = unit;
    tmpm = m;
    if (!is_material_type(m)) {
	/* This may be an indication of code bogosity; warn? */
	return TASK_FAILED;
    }
    /* If the unit is full, then arrange to dump the supply somewhere. */
    if (unit->supply[m] == um_storage_x(unit->type, m)) {
	for_all_directions(dir) {
	    if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
		if ((collector = collect_here(x1, y1)) != NULL) {
		    transfer_supply(unit, collector, m, unit->supply[m]);
		    if (unit->supply[m] == um_storage_x(unit->type, m))
		      return TASK_FAILED;
		    push_move_to_task(unit, task->args[1], task->args[2], 0);
		    return TASK_IS_INCOMPLETE;
		}
	    }
	}
	/* Compute how far out to look for a delivery point. */
	range = real_operating_range_best(unit);
	if (search_around(unit->x, unit->y, range, collect_test,
			  &x2, &y2, 1)) {
    	    /* (should find actual unit and chase it directly) */
	    push_move_to_task(unit, x2, y2, 1);
	    return TASK_IS_INCOMPLETE;
	} else {
	    /* Failure - sometimes just sit, but usually try something else. */
	    if (probability(10))
	      set_unit_reserve(unit->side, unit, TRUE, FALSE);
    	    /* (should be able to signal interface usefully somehow) */
	    return TASK_FAILED;
	}
    }
    if (distance(unit->x, unit->y, x, y) > 1) {
	push_move_to_task(unit, x, y, 1);
	return TASK_IS_INCOMPLETE;
    }
    if (valid(check_extract_action(unit, unit, x, y, m, 1))) {
	prep_extract_action(unit, unit, x, y, m, 1);
	return TASK_PREPPED_ACTION;
    }
    for_all_stack(x, y, unit3) {
	if (in_play(unit3)
	    && unit3->side == unit->side
	    && type_max_acp(unit3->type) == 0) {
	    if (valid(check_transfer_action(unit, unit3, m, 1, unit))) {
		prep_transfer_action(unit, unit3, m, 1, unit);
		return TASK_PREPPED_ACTION;
	    }
	}
    }
    /* We've run out of extractables in the immediate vicinity; look around
       for more. */
    tmpunit = unit;
    tmpm = m;
    if (search_around(x, y, 3, extractable_test, &x1, &y1, 1)) {
	push_move_to_task(unit, x1, y1, 1);
	task->args[1] = x1;  task->args[2] = y1;
	return TASK_IS_INCOMPLETE;
    }
    return TASK_FAILED;
}

static int
collect_test(int x, int y)
{
    return (collect_here(x, y) != NULL);
}

static Unit *
collect_here(int x, int y)
{
    Unit *unit, *collector;

    for_all_stack(x, y, unit) {
    	collector = aux_collect_here(unit);
    	if (collector)
    	  return collector;
    }
    return NULL;
}

static Unit *
aux_collect_here(Unit *unit)
{
    Unit *occ;

    /* what about allies? */
    if (unit != tmpunit && unit_trusts_unit(unit, tmpunit)) {
	if ((unit->supply[tmpm] < um_storage_x(unit->type, tmpm)
	     || um_gives_to_treasury(unit->type, tmpm))
	    /* this is a hack to prevent foragers from trying to
               deliver to each other */
	    && !mobile(unit->type))
	  return unit;
	/* (should also test for ability to transfer to side's storage) */
    }
    for_all_occupants(unit, occ) {
	if (aux_collect_here(occ)) {
	    return occ;
	}
    }
    return NULL;
}

static int
extractable_test(int x, int y)
{
    Unit *unit;

    /* (should only look at already-seen cells) */
    /* (should test if unit can extract from these places) */
    if (any_cell_materials_defined()
	&& cell_material_defined(tmpm)
	&& material_at(x, y, tmpm) > 0)
      return TRUE;
    for_all_stack(x, y, unit) {
	if (in_play(unit)
	    && indep(unit)
	    && unit->supply[tmpm] > 0)
	  return TRUE;
    }
    return FALSE;
}

/* This function uses a global approach in that it checks if the unit
and its destination are located in or are adjacent to the same region 
(sea or continent). Since the region area layer is precomputed during 
startup, this check is fast. There are certain simplistic assumptions 
built into the code, however. First, it is assumed that resupply and
repair points for naval units can be on land (i.e. in ports) but not vice 
versa. Second, it is assumed that naval units always can enter such
land-based resupply points, if they are adjacent to a liquid cell. */

/* This code does not handle units on board of transports. A ground
unit at sea will therefore fail to set a resupply or repair task since
it cannot find a land path to a supply point. An air unit on board of a
carrier will, however, be able to set a resupply task on land, since
it can get there by itself. Ultimately, we should implement a way for
occupants to set repair tasks and signal their transport where they
need to go. In some games, occupants will leech supplies off their 
transport, so the latter will evetually run out of supplies and set
its own resupply task. */

/* The code does not check if a path to the destination really exists
that our unit may follow. Posible problems: enemy units and hostile
terrain. The fact that u_ground_mobile is set for a unit does not
mean that it may traverse all possible ground terrain. Ultimately,
the code should test for paths as well. */

int
direct_access_to(int x, int y)
{
	int x1, y1, x2, y2;  
	int u = tmpunit->type;
	int ux = tmpunit->x;
	int uy = tmpunit->y;
	
	/* Airborne units always have access everywhere. */
	if (u_air_mobile(u)) {
		return TRUE;
	}
	/* Landborne units have access if both cells belong
	to the same non-liquid region. */
	if (u_ground_mobile(u)) {
		if (!t_liquid(terrain_at(ux, uy))
		    && !t_liquid(terrain_at(x, y))) {
			if (aref(area.landsea_regions, ux, uy) 
				== aref(area.landsea_regions, x, y)) {
				return TRUE;
			} 	
		}
	}
	/* Seaborne units have access if both cells are adjacent
	to the same liquid region. */
	if (u_naval_mobile(u)) {
		/* We are starting from a liquid cell. */
		if (t_liquid(terrain_at(ux, uy))) {
			/* Test if the destination belongs to the same region. */
			if (aref(area.landsea_regions, ux, uy) 
				== aref(area.landsea_regions, x, y)) {
				return TRUE;
			}
			/* Next test cells adjacent to the destination. */
			for_all_cells_within_range(x, y, 1, x2, y2) {
				if (aref(area.landsea_regions, ux, uy) 
					== aref(area.landsea_regions, x2, y2)) {
					return TRUE;
				}
			}
		/* We are starting from a land cell. */
		} else {
			/* Scan adjacent liquid cells. */
			for_all_cells_within_range(ux, uy, 1, x1, y1) {
				if (t_liquid(terrain_at(x1, y1))) {
					/* Test if the destination belongs to the same region. */
					if (aref(area.landsea_regions, x1, y1) 
						== aref(area.landsea_regions, x, y)) {
						return TRUE;
					}
					/* Next test cells adjacent to the destination. */
					for_all_cells_within_range(x, y, 1, x2, y2) {
						if (aref(area.landsea_regions, x1, y1) 
							== aref(area.landsea_regions, x2, y2)) {
							return TRUE;
						}
					}
				}
			}
		}
	}	
	return FALSE;
}

static TaskOutcome
do_toolup_subtask(Unit *constructor, int uc)
{
    int rslt = A_ANY_OK;

    assert_error(is_active(constructor), 
		 "AI: Attempted to toolup an inactive unit");
    assert_error(is_unit_type(uc),
		 "AI: Encountered and invalid unit type to toolup for");
    // If by some strange coincidence we are inside this function, 
    //	and constructor already has enough tooling, 
    //	then get out.
    if (has_enough_tooling(constructor, uc))
	return TASK_IS_INCOMPLETE;
    // If we cannot perform the toolup action, then the task must fail.
    if (!valid(rslt = 
	       can_toolup_for(constructor, constructor, uc))) {
	// TODO: Should notify side.
	return TASK_FAILED;
    }
    // Try to prep a toolup action.
    if (prep_toolup_action(constructor, constructor, uc))
	return TASK_PREPPED_ACTION;
    return TASK_FAILED;
}

static TaskOutcome
do_construct_task(Unit *constructor, Task *task)
{
    int rslt = A_ANY_OK;
    int uc = NONUTYPE;
    int run = 0, x = -1, y = -1;
    int transid = -1;
    Unit *creation = NULL, *transport = NULL;
    Side *side = NULL;
    Task *tasks = NULL;

    assert_error(is_active(constructor), 
		 "AI: Attempted to construct with an inactive unit");
    assert_warning_return(task, "AI: Attempted to run invalid construct task",
			  TASK_IS_COMPLETE);
    side = constructor->side;
    // Unpack task args.
    uc = task->args[0];
    run = task->args[1];
    // If run length <= 0, then we're done.
    if (0 >= run)
	return TASK_IS_COMPLETE;
    transid = task->args[2];
    // If no transport or out-of-play transport, then try getting coords.
    transport = find_unit(transid);
    if (!in_play(transport)) {
	x = task->args[3];  y = task->args[4];
	// If coords are also invalid, then task fails.
	if (!inside_area(x, y))
	    return TASK_FAILED;
    }
    // If constructor has its creation ID set, 
    //	then see if it matches an incomplete unit.
    //  If so, then decrement the construction run length,
    //  clear the creation ID, and push a build task, if necessary.
    creation = find_unit(constructor->creation_id);
    if (in_play(creation)) {
	constructor->creation_id = -1;
	--task->args[1];
	// Push or set build task, if creation is incomplete.
	// Push, if we have more construction planned.
	// Set, if this is last of construction run.
	if (task->args[1])
	    push_build_task(
		constructor, creation->id, u_cp(creation->type));
	else
	    set_build_task(
		constructor, creation->id, u_cp(creation->type));
	tasks = (constructor->plan ? constructor->plan->tasks : NULL);
	if (!tasks || (TASK_BUILD != tasks->type))
	    return TASK_FAILED;
	tasks->args[1] = creation->id;
	return TASK_IS_INCOMPLETE;
    }
    // Else, we try to proceed with creating an unit 
    //	in the desired cell or transport.
    // If research or development is needed, then we should not be in this code.
    if (!side_can_build(side, uc))
	return TASK_FAILED;
    // We must handle tooling, if necessary.
    if (!has_enough_tooling(constructor, uc))
	return do_toolup_subtask(constructor, uc);
    // If transport is valid, then try creating in it.
    if (in_play(transport)) {
	if (valid(rslt = 
		  can_create_in(constructor, constructor, uc, transport))) {
	    prep_create_in_action(constructor, constructor, uc, transport);
	    return TASK_PREPPED_ACTION;
	}
    }
    // Else, try creating in designated cell.
    else {
	if (valid(rslt = 
		  can_create_at(constructor, constructor, uc, x, y))) {
	    prep_create_at_action(constructor, constructor, uc, x, y, 0);
	    return TASK_PREPPED_ACTION;
	}
    }
    // Wait for more materials or ACP, if necessary.
    // TODO: Proactively accumulate materials, if possible.
    if ((A_ANY_NO_MATERIAL == rslt) || (A_ANY_NO_ACP == rslt)) {
	set_unit_reserve(constructor->side, constructor, TRUE, FALSE);
	return TASK_IS_INCOMPLETE;
    }
    return TASK_FAILED;
}

static TaskOutcome
do_repair_self_subtask(Unit *repairee, Task *task)
{
    Unit *unit2 = NULL, *unitbest = NULL;
    Side *side = NULL;
    Task *tasks2 = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    int tx = -1, ty = -1, dist = -1;
    int score = -1, scorebest = -1;
    
    assert_error(is_active(repairee), 
		 "AI: Attempted to repair with an inactive unit");
    assert_warning_return(task, "AI: Attempted to run invalid repair task",
			  TASK_IS_COMPLETE);
    // Useful info.
    side = repairee->side;
    u = repairee->type;
    tx = repairee->x;  ty = repairee->y;
    // Try hp-recovery mechanism.
    if (0 < u_hp_recovery(u)) {
	set_unit_reserve(side, repairee, TRUE, FALSE);
	return TASK_IS_INCOMPLETE;
    }
    // Check if anyone else is tasked with repairing us.
    for_all_side_units(side, unit2) {
	if (unit2 == repairee)
	    continue;
	// Skip inactive units.
	if (!is_active(unit2))
	    continue;
	// Skip taskless units.
	tasks2 = (unit2->plan ? unit2->plan->tasks : NULL);
	if (!tasks2)
	    continue;
	// Useful info.
	u2 = unit2->type;
	// If 'move-to' task, then inspect closer.
	if ((TASK_MOVE_TO == tasks2->type)
	    && (uu_auto_repair_range(u2, u)
		>= distance(tasks2->args[0], tasks2->args[1], tx, ty)))
	    tasks2 = tasks2->next;
	if ((TASK_MOVE_TO == tasks2->type)
	    && (uu_repair_range(u2, u)
		>= distance(tasks2->args[0], tasks2->args[1], tx, ty)))
	    tasks2 = tasks2->next;
	// If 'occupy' task, then inspect closer.
	if (TASK_OCCUPY == tasks2->type)
	    tasks2 = tasks2->next;
	// If not 'repair' task, then skip.
	if (TASK_REPAIR != tasks2->type)
	    continue;
	// Skip, if repairee is not the object of the repair.
	if (repairee->id != tasks2->args[0])
	    continue;
	// If we've made it this far, then we know enough.
	// Now we just sit back and let the other unit do the work.
	set_unit_reserve(side, repairee, TRUE, FALSE);
	return TASK_IS_INCOMPLETE;
    } // for all side units
    // If not auto-repair, then we report that the task failed.
    // TODO: Find an unit to explicitly repair us, and schedule the repair.
    if (!Xconq::any_auto_repair)
	return TASK_FAILED;
    // Try to find someone to auto-repair us.
    for_all_side_units(side, unit2) {
	if (unit2 == repairee)
	    continue;
	// Skip inactive units.
	if (!is_active(unit2))
	    continue;
	// Probably not worth chasing mobile units. Let them come to us.
	if (mobile(unit2->type))
	    continue;
	// Useful info.
	u2 = unit2->type;
	dist = distance(unit2->x, unit2->y, tx, ty);
	// Subtract auto-repair range from distance.
	// Note: For repairs that must occur inside a transport, 
	//  a penalty of 1 is effectively added.
	dist = max(0, dist - uu_auto_repair_range(u2, u));
	// If repairee is immobile,
	//   and potential repairer is out of range, then skip it.
	if (!mobile(u) && (0 < dist))
	    continue;
	// Score potential repairer.
	score = hp_per_turn_est(unit2, u) / isqrt(dist + 1);
	// Remember unit if it is best potential repairer.
	if (score > scorebest) {
	    scorebest = score;
	    unitbest = unit2;
	}
    } // for all side units
    if (0 < scorebest) {
	u2 = unitbest->type;
	// Calculate distance to be within repair range of best 
	//  potential repairer.
	dist = distance(unitbest->x, unitbest->y, repairee->x, repairee->y);
	dist = max(0, dist - uu_auto_repair_range(u2, u));
	// If distance to repairer > 0, then move there.
	// Note: A potential repairer with distance > 0 should never 
	//  have been selected, so this is not a concern.
	if (dist > 0) {
	    // Move within range of potential repairer.
	    if (0 <= uu_auto_repair_range(u2, u))
		// Move within auto-repair range of potential repairer.
		push_move_to_task(
		    repairee, unitbest->x, unitbest->y, 
		    uu_auto_repair_range(u2, u));
	    else {
		// Prepare to occupy potential repairer.
		// TODO: Technically, we need to search not only 
		//  the potential repairer but all occs under it to 
		//  find an opening that we can occupy.
		push_occupy_task(repairee, unitbest);
		// Move-to potential repairer before occupation of it.
		// NOTE: Move-to may not be necessary in all cases,
		//  but it cannot hurt.
		push_move_to_task(repairee, unitbest->x, unitbest->y, 1);
	    }
	}
	// Else, wait around.
	else 
	    set_unit_reserve(side, repairee, TRUE, FALSE);
	// Return the task as being incomplete.
	// If tasks were pushed onto the stack they will execute next.
	return TASK_IS_INCOMPLETE;
    } // 0 < scorebest
    return TASK_FAILED;
}

static TaskOutcome
do_repair_task(Unit *repairer, Task *task)
{
    TaskOutcome rslt = TASK_IS_INCOMPLETE;
    Unit *repairee = NULL;
    Side *side = NULL;
    int id = -1;
    int u = NONUTYPE, u2 = NONUTYPE;
    int hpgoal = -1;

    assert_error(is_active(repairer), 
		 "AI: Attempted to repair with an inactive unit");
    assert_warning_return(task, "AI: Attempted to run invalid repair task",
			  TASK_IS_COMPLETE);
    // Get repairee.
    id = task->args[0];
    repairee = find_unit(id);
    // Is repairee in play?
    if (!in_play(repairee))
	return TASK_FAILED;
    // Useful info.
    u = repairer->type;
    u2 = repairee->type;
    side = repairer->side;
    hpgoal = task->args[1];
    // If repair is finished, then indicate that the task is complete.
    if (repairee->hp >= hpgoal)
	return TASK_IS_COMPLETE;
    // Set up explicit repair action, if possible.
    if (valid(can_repair(repairer, repairer, repairee))) {
	prep_repair_action(repairer, repairer, repairee);
	return TASK_PREPPED_ACTION;
    }
    // Wait around for auto-repair, if possible.
    if (valid(can_auto_repair(repairer, repairee))) {
	// If repairer is repairee, then try to get repaired various ways.
	if ((repairer == repairee) 
	    && (TASK_FAILED != (rslt = do_repair_self_subtask(repairee, task))))
	    return rslt;
	else {
	    set_unit_reserve(side, repairer, TRUE, FALSE);
	    return TASK_IS_INCOMPLETE;
	}
    }
    // None of the above worked. Something is wrong.
    Dprintf("%s repair task failed!\n", unit_desig(repairer));
    return TASK_FAILED;
}

#if (0)
int
can_repair_from_here(int x, int y)
{
	int x1, y1, u = tmpunit->type, u2, range = 0, space = FALSE;
	Unit *unit;

	/* First test if auto-repair of this unit type is at all possible 
	    (should never get this far if not, but check anyway). */
	if (!will_be_auto_repaired
	    || !will_be_auto_repaired[u]) {
	    return FALSE;    
	}
	/* Then test if we can get there. */
	if (!direct_access_to(x, y)) {
	    return FALSE;
	}
	/* Also check if we can sit there. */
	if (!type_survives_in_cell(u, x, y)
	    || !type_can_occupy_cell(u, x, y)) {
	    /* If not, check if we can sit inside a unit at (x, y). */
	    for_all_stack_with_occs(x, y, unit) {
		/* We are testing for empty type on the optimistic assumption
		    that there will be some space when we get there. */
		if (type_can_occupy_empty_type(u, unit->type)) {
		    space = TRUE;
		    break;
		}
	    }
	    if (!space) {
		    return FALSE;
	    }
	}
	tmpx = x;
	tmpy = y;
	/* First check this cell. */
	if (repair_here(x, y)) {
	    return TRUE;
	}
	/* Compute how far out we need to search. */
	for_all_unit_types(u2) {
	    range = max(range, uu_auto_repair_range(u2, u));
	}
	/* Search adjacent cells. */
	if (search_around(
		x, y, range, (int (*)(int, int)) repair_here, &x1, &y1, 1)) {
	    return TRUE;
	}		
	return FALSE;
}

Unit *
repair_here(int x, int y)
{
    int u = tmpunit->type, range = 0;
    Unit *unit2;

    for_all_stack_with_occs(x, y, unit2) {
	/* Skip over inactive units. */
	if (!is_active(unit2)) {
		continue;
	}
	/* Skip over untrusting units. */
	if (!unit_trusts_unit(unit2, tmpunit)) {
		continue;
	}
	/* Skip over units that cannot repair us. */
	if (uu_auto_repair(unit2->type, u) <= 0) {
		continue;
	}
	range = uu_auto_repair_range(unit2->type, u);
	/* Test if unit2 can repair unit and is within auto repair range of 
	    (x, y). Force the auto repair range to 0 if it is set to -1 to 
	    signal that only occupants should be repaired. */
	if (distance(x, y, tmpx, tmpy) <= max(range, 0)) {
	    /* Check that tmpunit can occupy unit2 if this is required. 
		Note: we optimistically test for room in empty transport 
		since things may change once we get there. 
		Should eventually add code that kicks out other occs to 
		make room for damaged units that need repair. */
	    if (range < 0
		&& !type_can_occupy_empty_type(u, unit2->type)) {
		    continue;
	    }
	    /* If the auto-repair range is zero and we cannot enter either
		the cell or unit2, we also have a problem. */
	    if (range == 0
		&& (!type_survives_in_cell(u, x, y)
		    || !type_can_occupy_cell(u, x, y))
		&& !type_can_occupy_empty_type(u, unit2->type)) {
		    continue;
	    }
	    /* We assume that the calling code already has checked that 
		tmpunit can sit at (tmpx, tmpy). */
	    return unit2;
	}
    }
    return NULL;
}
#endif

/* Replenish our supplies, using one of several strategies, which as
   usual depends on the game, unit, terrain, etc.  Strategies include
   1) wait for supply line or own production to replenish, 2) move to
   productive terrain and then wait, 3) move within range of a
   supplier, and 4) request a supplier to approach. */

/* (should see if production actions would resupply, prep those actions) */

static TaskOutcome
do_resupply_task(Unit *unit, Task *task)
{
    int x, y, u = unit->type, m, range;
    int ux = unit->x, uy = unit->y;
    Unit *unit2;
#if (0)
    int amtavail = 0, amtwanted = 0;
#endif
    int i = 0;

    /* A unit acting randomly might have gotten this task, even if the 
    game has no materials or the unit no supply. Pretend that it was OK 
    and get out of here. */
    if (nummtypes == 0 || unit->supply == NULL)
      return TASK_IS_COMPLETE;
    tmpside = unit->side;
    tmpunit = unit;
    if (lowm == NULL)
      lowm = (int *) xmalloc(nummtypes * sizeof(int));
    numlow = 0;
    if (task->args[0] == NONMTYPE) {
	for_all_material_types(m) {
	    if (unit->supply[m] < um_storage_x(u, m)) {
		lowm[numlow++] = m;
	    }
	}
    } else {
	m = task->args[0];
	if (unit->supply[m] < um_storage_x(u, m)) {
	    lowm[numlow++] = m;
	}
    }
    /* We're all filled up, must be OK. */
    if (numlow == 0) {
    	return TASK_IS_COMPLETE;
    /* Set up an explicit extraction action if possible. We will still 
	benefit from any auto-resupply that is available.*/
    } else if (valid(check_extract_action(unit, unit, 
					  unit->x, unit->y, m, 1))) {
	prep_extract_action(unit, unit, unit->x, unit->y, m, 1);
	return TASK_PREPPED_ACTION;
    /* Should not just sit around when there may be more expedient ways to 
	get supplies that could let us get back into action the same turn. */
    } else if (can_auto_resupply_self(unit, lowm, numlow)) {
	set_unit_reserve(unit->side, unit, TRUE, FALSE);
    	return TASK_IS_INCOMPLETE;
    /* Check if we are already at a valid resupply point. */
    } else if (can_resupply_from_here(ux, uy)) {
	/* Try to enter any provider in the same cell. Note: this
	    is to ensure that aircraft lands on a carrier. If we already
	    have a transport, we stay put. */
	if (!unit->transport) {
	    for (i = 0; i < numlow; ++i) {
		tmpmtype = lowm[i];
		tmpx = ux;
		tmpy = uy;
		unit2 = resupply_here(ux, uy);
		if (unit2
		    && can_occupy(unit, unit2)) {
		    prep_enter_action(unit, unit, unit2);
		    return TASK_PREPPED_ACTION;
		}
	    }
	}
	/* Otherwise, just wait for the economy code to to its job. */
	set_unit_reserve(unit->side, unit, TRUE, FALSE);
	return TASK_IS_INCOMPLETE;
    } else {
	/* Compute how far out to look for a resupply point. */
	range = real_operating_range_best(unit);
	if (search_around(ux, uy, range, can_resupply_from_here, &x, &y, 1)) {
	    push_move_to_task(unit, x, y, 0);
	    DMprintf("Resupply task: %s is moving to (%d, %d).", 
		     unit_desig(unit), x, y);
	    return TASK_IS_INCOMPLETE;
	} else {
	    /* None of the above worked. Something is wrong. */
	    Dprintf("%s resupply task failed!\n", unit_desig(unit));
	    return TASK_FAILED;
	}
    }
}

int
can_resupply_from_here(int x, int y)
{
    int i, x1, y1, u = tmpunit->type, space = FALSE;
    Unit *unit;

    /* First test if we can get there. */
    if (!direct_access_to(x, y)) {
	return FALSE;
    }
    /* Also check if we can sit there. */
    if (!type_survives_in_cell(u, x, y)
	|| !type_can_occupy_cell(u, x, y)) {
	/* If not, check if we can sit inside a unit at (x, y). */
	for_all_stack_with_occs(x, y, unit) {
	    /* We are testing for empty type on the optimistic assumption
		that there will be some space when we get there. */
	    if (type_can_occupy_empty_type(u, unit->type)) {
		space = TRUE;
		break;
	    }
	}
	if (!space) {
	    return FALSE;
	}
    }
    if (foundm == NULL) {
	foundm = (int *) xmalloc(nummtypes * sizeof(int));
    }
    /* Search adjacent cells within our inlength for needed supplies. */
    for (i = 0; i < numlow; ++i) {
	foundm[i] = FALSE;
	tmpmtype = lowm[i];
	tmpx = x;
	tmpy = y;
	/* First search this cell. */
	if (resupply_here(x, y)) {
	    foundm[i] = TRUE;
	    continue;		
	}
	if (search_around(x, y, um_inlength(u, lowm[i]), 
	    (int (*)(int, int)) resupply_here, &x1, &y1, 1)){
	    foundm[i] = TRUE;
	    continue;
	}		
    }
    /* If we cannot resupply a needed material from here, we are done. */
    for (i = 0; i < numlow; ++i) {
	if (foundm[i] == FALSE) {
	    return FALSE; 
	}
    }
    return TRUE;
}

Unit *
resupply_here(int x, int y)
{
    int u, tu = tmpunit->type, range = 0;
    Unit *unit;

    for_all_stack_with_occs(x, y, unit) {
	u = unit->type;
	/* Can't resupply from ourselves. */
	if (unit == tmpunit) 
	  continue;
	/* Can't resupply from enemies. */
	if (!unit_trusts_unit(unit, tmpunit)) 
	  continue;
	/* The unit lacks our needed supply. */
	if (unit->supply[tmpmtype] == 0) 
	  continue;
	/* Compute the maximal possible supply range. */   
	range = min(um_outlength(u, tmpmtype), um_inlength(tu, tmpmtype));
	/* Resupply is impossible. */
	if (range < 0)
	  continue;
	/* We are out of range. */
	if (range < distance(unit->x, unit->y, tmpx, tmpy)) 
	  continue;
	/* Range zero and there is no room for us. We assume 
	    that it is necessary to enter the provider. */
	if (range == 0 && !can_occupy(tmpunit, unit)) 
	  continue;
	/* If range > 0 we assume that there is always room for 
	    us in a cell within range. */
	return unit;
    }
    return NULL;
}

/* Return true if our own automatic material production is *greater*
   than our consumption, for the given list of materials. */

int
can_auto_resupply_self(Unit *unit, int *materials, int numtypes)
{
    int u = unit->type, i, m, rslt = TRUE, t = terrain_at(unit->x, unit->y);

    for (i = 0; i < numtypes; ++i) {
	m = materials[i];
	if ((um_base_production(u, m) * ut_productivity(u, t))
	    <= um_base_consumption(u, m))
	  rslt = FALSE;
    }
    return rslt;
}

/* The sentry task puts the unit into reserve each turn for a given
   number of turns. */

static TaskOutcome
do_sentry_task(Unit *unit, Task *task)
{
    Task *nexttask = NULL;
    Unit *transport = NULL;

    if (task->next)
      nexttask = task->next;
    /* If we were waiting for a transport, and are now in it, 
	then don't wait. */
    if ((nexttask != NULL) && (nexttask->type == TASK_OCCUPY)) {
	transport = find_unit_dead_or_alive(nexttask->args[0]);
	if (in_play(transport) && (transport == unit->transport)) {
	    task->next = nexttask->next;
	    free_task(nexttask);
	}
	return TASK_IS_COMPLETE;
    }
    if (task->args[0] > 0) {
	set_unit_reserve(unit->side, unit, TRUE, FALSE);
	--(task->args[0]);
	return TASK_IS_INCOMPLETE;
    } else {
	/* Unit won't necessarily wake up, may just replan and
	   continue sleeping. */
	return TASK_IS_COMPLETE;
    }
}

/* This is the main routine for a unit to execute a task.  It
   basically consists of a dispatch to the execution code for each
   task type, and handling for a task's several possible outcomes.
   Note that a task does *not* directly invoke any actions; instead it
   will schedule ("prep") an action, which will be executed later by
   execute_action.  Therefore, it is possible for a task to succeed
   but the action to fail, although each task type's code tries to
   reduce the chances of this happening (not possible to prevent
   entirely - unit may become damaged and unable to do perform an
   action after the task had decided on that action). */

TaskOutcome
execute_task(Unit *unit)
{
    Plan *plan = unit->plan;
    TaskOutcome rslt;
    Task *task;
    extern int taskexecs;

    /* This should never happen. */
    if (unit->plan == NULL)
      run_error("???");
    /* If the unit is AI-run, don't do more than one task execution with
       it during each run step. */
    if (ai_controlled(unit)
         && plan->last_task_outcome != TASK_UNKNOWN
	&& plan->last_task_outcome != TASK_PREPPED_ACTION
	/* unless it is acp-independent ... */
	&& !acp_indep(unit))
      return unit->plan->last_task_outcome;
    task = plan->tasks;
    if (task == NULL)
      return TASK_UNKNOWN;
    ++taskexecs;
    rslt = execute_task_aux(unit, task);
    DMprintf("%s did task %s, ", unit_desig(unit), task_desig(task));
    /* Record it. */
    memcpy(&(unit->plan->last_task), task, sizeof(Task));
    unit->plan->last_task_outcome = rslt;
    /* Now look at what happened with task execution. */
    switch (rslt) {
      case TASK_UNKNOWN:
	DMprintf("???unknown outcome???");
	break;
      case TASK_FAILED:
        ++task->retrynum;
	DMprintf("failed try %d, ", task->retrynum);
	/* If a task fails, it might be because the task cannot be
	   completed, or just because conditions are temporarily
	   unfavorable, such as a passing unit blocking the way while
	   moving through.  So we need to retry a couple times at
	   least; the variables here control how hard to keep
	   trying. */
	/* (should be doctrine, since these affect human-run units too) */
	if (probability(g_ai_badtask_remove_chance()) 
	    || task->retrynum >= g_ai_badtask_max_retries()) {
	    pop_task(plan);
	    DMprintf("removed it");
	    /* We might be buzzing, so maybe go into reserve. */
	    if (probability(g_ai_badtask_reserve_chance())) {
		plan->reserve = TRUE;
	    	DMprintf(" and went into reserve");
	    }
	} else {
	    DMprintf("will retry");
	}
	break;
      case TASK_IS_INCOMPLETE:
	/* Leave the task alone. */
	DMprintf("incomplete");
	break;
      case TASK_PREPPED_ACTION:
	/* Mention the action that was prepared to execute. */
	DMprintf("prepped action %s", action_desig(&(unit->act->nextaction)));
	break;
      case TASK_IS_COMPLETE:
	/* Task completed successfully, get rid of it. */
	DMprintf("completed after %d executions", task->execnum);
	pop_task(plan);
	break;
      default:
	break;
    }
    DMprintf("\n");
    return rslt;
}

/* Perform a single given task. */

static TaskOutcome
execute_task_aux(Unit *unit, Task *task)
{
    TaskOutcome (*fn)(Unit *unit, Task *task);

    if (!alive(unit) || task == NULL)
      return TASK_UNKNOWN;
    DMprintf("%s doing task %s\n", unit_desig(unit), task_desig(task));
    if (task->type < 0 || task->type >= NUMTASKTYPES) {
	run_warning("Unknown task type %d", task->type);
    	return TASK_FAILED;
    }
    /* Count this execution. */
    ++task->execnum;
    /* Do it. */
    fn = taskdefns[task->type].exec;
    return (*fn)(unit, task);
}

/* Wrapper to 'choose_move_dirs' function for providing "backward" 
   compatibility to the 'choose_move_direction' function that some 
   functions scattered around the kernel came to depend upon. */
/* (The 'range' parameter is dead weight. Should we/can we do anything 
    meaningful with it in the new context?) */
int
choose_move_direction(Unit *unit, int x, int y, int range)
{
    int rslt = NODIR;
    int numdirs = 0;
    int dirs[NUMDIRS];
    int i = 0, ix, iy;

    /* Try to find a shortest path first. */
    numdirs = choose_move_dirs(unit, x, y, TRUE, plausible_move_dir, 
			       sort_directions, dirs);
#if (1)
    /* If that fails then try to find any path. */
    /* (Unfortunately, this is not compatible with 'do_approach_subtask'. 
	If this piece of code is enabled and called from an UI, 
	it may say a move is OK, but a movement task that is prepped may, 
	in fact, fail in do_approach_subtask. So, for the time being, we 
	must accept that certain legitimate paths will be rejected.) */
    if (!numdirs)
      numdirs = choose_move_dirs(unit, x, y, FALSE, plausible_move_dir, 
				 sort_directions, dirs);
#endif
    /* If that fails then either no path exists, or else our pathfinder 
       algorithm does not do a more thorough search such as with Astar. */
    if (!numdirs)
      return NODIR;
    for (i = 0; i < numdirs; ++i) {
	interior_point_in_dir(unit->x, unit->y, dirs[i], &ix, &iy);
	if (!side_thinks_it_can_put_type_at(unit->side, unit->type, ix, iy)) 
	  continue;
	/* (Should put other tests here to make this function emulate 
	    the 'choose_move_direction' from Peter's pathfinder as much as 
	    is sane.) */
	rslt = dirs[i];
	break;
    }
    return rslt;
}

/* This weird-looking routine computes next directions for moving to a
   given spot.  The number of directions ranges from 1 to 4, depending
   on whether there is a straight-line path to the dest, and whether we are
   required to take a direct path or are allowed to move in dirs that don't
   bring the unit any closer (we never increase our distance though).
   Some trickinesses:  if area wraps, must resolve ambiguity about
   getting to the same place going either direction (we pick shortest). */

int
choose_move_dirs(Unit *unit, int tx, int ty, int shortest,
		 int (*dirtest)(Unit *, int),
		 void (*dirsort)(Unit *, int *, int),
		 int *dirs)
{
    int dx, dxa, dy, dist, d1, d2, d3, d4, axis = -1, hextant = -1;
    int numdirs = 0, shortestnumdirs;

    dist = distance(unit->x, unit->y, tx, ty);
    dx = tx - unit->x;  dy = ty - unit->y;
    if (area.xwrap) {
	dxa = (tx + area.width) - unit->x;
	if (ABS(dx) > ABS(dxa))
	  dx = dxa;
	dxa = (tx - area.width) - unit->x;
	if (ABS(dx) > ABS(dxa))
	  dx = dxa;
    }
    if (dx == 0 && dy == 0) {
	return -1;
    }
    axis = hextant = -1;
    if (dx == 0) {
	axis = (dy > 0 ? NORTHEAST : SOUTHWEST);
    } else if (dy == 0) {
	axis = (dx > 0 ? EAST : WEST);
    } else if (dx == (0 - dy)) {
	axis = (dy > 0 ? NORTHWEST : SOUTHEAST);
    } else if (dx > 0) {
	hextant = (dy > 0 ? EAST :
		   (ABS(dx) > ABS(dy) ? SOUTHEAST : SOUTHWEST));
    } else {
	hextant = (dy < 0 ? WEST :
		   (ABS(dx) > ABS(dy) ? NORTHWEST : NORTHEAST));
    }
    if (axis >= 0) {
	d1 = d2 = axis;
	if (dirtest == NULL || (*dirtest)(unit, d1)) {
	    dirs[numdirs++] = d1;
	}
    }
    if (hextant >= 0) {
	d1 = left_dir(hextant);
	d2 = hextant;
	if (dirtest == NULL || (*dirtest)(unit, d1)) {
	    dirs[numdirs++] = d1;
	}
	if (dirtest == NULL || (*dirtest)(unit, d2)) {
	    dirs[numdirs++] = d2;
	}
    }
    /* Check on other properties of the two choices. */
    if (numdirs > 1 && dirsort != NULL) {
    	(*dirsort)(unit, dirs, numdirs);
    }
    if (dist > 1 && !shortest) {
	shortestnumdirs = numdirs;
    	d3 = left_dir(d1);
    	d4 = right_dir(d2);
	if (dirtest == NULL || (*dirtest)(unit, d3)) {
	    dirs[numdirs++] = d3;
	}
	if (dirtest == NULL || (*dirtest)(unit, d4)) {
	    dirs[numdirs++] = d4;
	}
	if (numdirs > shortestnumdirs + 1 && dirsort != NULL) {
	    (*dirsort)(unit, dirs + shortestnumdirs, numdirs - shortestnumdirs);
	}
    }
    return numdirs;
}

/* A heuristic test for whether the given direction is a good one
   to move in. */

int
plausible_move_dir(Unit *unit, int dir)
{
    int u = unit->type, ux = unit->x, uy = unit->y, u2, nx, ny, t, c, cost;
    int totcost, speed, maxmpavail;

    point_in_dir(ux, uy, dir, &nx, &ny);
    if (unit_at(nx, ny))
      return TRUE;
    t = terrain_at(nx, ny);
    /* Deadly terrain with no bridges is always implausible. */
    if ((ut_vanishes_on(u, t) || ut_wrecks_on(u, t))
        && !can_move_via_conn(unit, nx, ny))
      return FALSE;
    speed = unit_speed(unit, nx, ny);
    /* Try the easy test first. */
    if (ut_mp_to_enter(u, t) <= ((new_acp_for_turn(unit) * speed) / 100))
      return TRUE;
    u2 = (unit->transport ? unit->transport->type : NONUTYPE);
    totcost = total_move_cost(u, u2, ux, uy, 0, nx, ny, 0);
    /* Check if unit can possibly have enough mp for the move. */
    /* (We need to be careful here, since this function is not 
	actually privy to all knowledge. Should use type generic 
	functions instead.) */
    maxmpavail =
      (((u_acp_max(u) > 0 ? u_acp_max(u) : new_acp_for_turn(unit)) 
       - u_acp_min(u)) * speed) / 100
      + u_free_mp(u);
    if (maxmpavail >= totcost)
      return TRUE;
    /* Cross-country movement is not possible; try each connection
       type to see if it provides an alternative. */
    for_all_connection_types(c) {
	if (aux_terrain_defined(c)
	    && connection_at(ux, uy, dir, c)
	    && ((cost = ut_mp_to_traverse(u, c)) >= 0)) {
	    if ((ut_mp_to_enter(u, c) + cost + ut_mp_to_leave(u, c))
		<= ((new_acp_for_turn(unit) * speed) / 100))
	      return TRUE;
	}
    }
    return FALSE;
}

/* This compares the desirability of two different directions.  This is
   somewhat tricky, because it should return < 0 if i0 designates a BETTER
   direction than i1. */

int xs[NUMDIRS];
int ys[NUMDIRS];
int terrs[NUMDIRS];

static int
compare_directions(CONST void *a0, CONST void *a1)
{
    int i0, i1;
    int u = tmputype, t0, t1;
    int ux = tmpunit->x, uy = tmpunit->y, u2 = NONUTYPE;
    int cost0 = 0, cost1 = 0, s, ps0, ps1, surr0, surr1, rslt;
    int cs0, cs1;
    extern short *any_people_surrenders;

    i0 = *((int *) a0);  i1 = *((int *) a1);
    t0 = terrs[i0];  t1 = terrs[i1];
    if (tmpunit->transport)
      u2 = tmpunit->transport->type;
    /* Check the overall movement cost of each direction. */
    cost0 = total_move_cost(u, u2, ux, uy, 0, xs[i0], ys[i0], 0);
    cost1 = total_move_cost(u, u2, ux, uy, 0, xs[i1], ys[i1], 0);
    if (cost0 != cost1) {
	return cost0 - cost1;
    }
    if (1 /* not in supply */) {
	if ((rslt = ut_productivity(u, t1) - ut_productivity(u, t0)) != 0) {
	    return rslt;
	}
    }
    if ((rslt = ut_mp_to_leave(u, t1) - ut_mp_to_leave(u, t0)) != 0) {
	return rslt;
    }
    /* Chooser the safer terrain. */
    if ((rslt = ut_accident_hit(u, t1) - ut_accident_hit(u, t0)) != 0) {
	return rslt;
    }
    /* Choose the better-concealing terrain. */
    /* (should only do if limited visibility) */
    if ((rslt = ut_visibility(u, t1) - ut_visibility(u, t0)) != 0) {
	return rslt;
    }
    /* Prefer to go over cells that we can change to our side. */
    /* (should control via doctrine, this will reveal movements
       more often) */
    if (any_people_surrenders != NULL
	&& any_people_surrenders[u]
	&& people_sides_defined()) {
    	s = side_number(tmpunit->side);
    	ps0 = people_side_at(xs[i0], ys[i0]);
    	ps1 = people_side_at(xs[i1], ys[i1]);
    	surr0 = ut_people_surrender(u, t0)
	  * ((ps0 != NOBODY && s != ps0) ? 1 : 0);
    	surr1 = ut_people_surrender(u, t1)
	  * ((ps1 != NOBODY && s != ps1) ? 1 : 0);
    	if (surr0 != surr1) {
	    return surr1 - surr0;
    	}
    }
    if (control_sides_defined()) {
    	s = side_number(tmpunit->side);
    	cs0 = control_side_at(xs[i0], ys[i0]);
    	cs1 = control_side_at(xs[i1], ys[i1]);
	/* (should test trusted side) */
    	surr0 = ((ps0 != NOBODY && s != ps0) ? 1 : 0);
    	surr1 = ((ps1 != NOBODY && s != ps1) ? 1 : 0);
    	if (surr0 != surr1) {
	    return surr1 - surr0;
    	}
    }
    return 0;
}

void
sort_directions(Unit *unit, int *dirs, int numdirs)
{
    int i, tmp, i0 = 0, i1 = 1, compar;

    for (i = 0; i < numdirs; ++i) { 
	point_in_dir(unit->x, unit->y, dirs[i], &(xs[i]), &(ys[i]));
	terrs[i] = terrain_at(xs[i], ys[i]);
    }
    tmpunit = unit;
    tmputype = unit->type;
    if (numdirs == 2) {
	compar = compare_directions(&i0, &i1);
    	if (compar > 0 || (compar == 0 && flip_coin())) {
    	    tmp = dirs[0];  dirs[0] = dirs[1];  dirs[1] = tmp;
    	}
    } else if (numdirs > 2) {
    	qsort(dirs, numdirs, sizeof(int), compare_directions);
	if (compare_directions(&i0, &i1) == 0 && flip_coin()) {
	    tmp = dirs[0];  dirs[0] = dirs[1];  dirs[1] = tmp;
	}
    }
}

/* Put the given task back onto the list of free tasks. */

void 
free_task(Task *task)
{
    task->next = freetasks;
    freetasks = task;
}

void
add_task(Unit *unit, int pos, Task *task)
{
    int numcleared;
    Task *agenda = NULL;

    if (unit->plan == NULL && !completed(unit)) {
	init_unit_plan(unit);
    }
    if (!in_play(unit) || unit->plan == NULL) {
	run_warning("Trying to do %s task with bad %s",
		    task_desig(task), unit_desig(unit));
	return;
    }
    DMprintf("%s add task %s",
	    unit_desig(unit), task_desig(task));
    if (pos == CLEAR_AGENDA) {
	numcleared = clear_task_agenda(unit);
	DMprintf(" (cleared %d existing tasks)", numcleared);
    }
    DMprintf("\n");
    switch (pos) {
      // Put the task on the front of the agenda. 
      case ADD_TO_AGENDA_AS_LIFO:
      case CLEAR_AGENDA:
	task->next = unit->plan->tasks;
	unit->plan->tasks = task;
	break;
      // Put the task on the end of the agenda.
      case ADD_TO_AGENDA_AS_FIFO:
	task->next = NULL;
	agenda = unit->plan->tasks;
	if (!agenda)
	    unit->plan->tasks = task;
	else {
	    for (; agenda && agenda->next; agenda = agenda->next);
	    agenda->next = task;
	}
	break;
      default:
	run_error("Tried to add a task to a task agenda in a strange manner");
	break;
    }
    // Shouldn't be asleep any longer. 
    unit->plan->asleep = FALSE;
    // We're not in reserve.
    unit->plan->reserve = FALSE;
    // Presumably we're no longer waiting to be told what to do. 
    set_waiting_for_tasks(unit, FALSE);
    // Reflect all this on displays. 
    update_unit_display(unit->side, unit, FALSE);
}

/* This routine sets up a task to build a unit of the given type. */

Task *
create_build_task(Unit *unit, int id, int cp)
{
    Task *task = NULL;

    task = create_task(TASK_BUILD);
    task->args[0] = id;
    task->args[1] = cp;
    return task;
}

void
set_build_task(Unit *unit, int id, int cp)
{
    add_task(unit, CLEAR_AGENDA, create_build_task(unit, id, cp));
}

void
push_build_task(Unit *unit, int id, int cp)
{
    add_task(unit, 0, create_build_task(unit, id, cp));
}

Task *
create_capture_task(Unit *unit, int id, int capmethod, int n)
{
    Task *task = create_task(TASK_CAPTURE);

    task->args[0] = id;
    task->args[1] = capmethod;
    task->args[2] = n;
    return task;
}

void
set_capture_task(Unit *unit, int id, int capmethod, int n)
{
    add_task(unit, CLEAR_AGENDA, create_capture_task(unit, id, capmethod, n));
}

void
push_capture_task(Unit *unit, int id, int capmethod, int n)
{
    add_task(unit, 0, create_capture_task(unit, id, capmethod, n));
}

Task *
create_collect_task(Unit *unit, int m, int x, int y)
{
    Task *task = create_task(TASK_COLLECT);

    task->args[0] = m;
    task->args[1] = x;  task->args[2] = y;
    return task;
}

void
set_collect_task(Unit *unit, int m, int x, int y)
{
    add_task(unit, 0, create_collect_task(unit, m, x, y));
}

Task *
create_disband_task(Unit *unit)
{
    return create_task(TASK_DISBAND);
}

void
set_disband_task(Unit *unit)
{
    add_task(unit, CLEAR_AGENDA, create_disband_task(unit));
}

Task *
create_hit_unit_task(Unit *unit, int id, int hitmethod, int n)
{
    Task *task = create_task(TASK_HIT_UNIT);

    task->args[0] = id;
    task->args[1] = hitmethod;
    task->args[2] = n;
    return task;
}

void
set_hit_unit_task(Unit *unit, int id, int hitmethod, int n)
{
    add_task(unit, CLEAR_AGENDA, create_hit_unit_task(unit, id, hitmethod, n));
}

void
push_hit_unit_task(Unit *unit, int id, int hitmethod, int n)
{
    add_task(unit, 0, create_hit_unit_task(unit, id, hitmethod, n));
}

/* Create a task to move in a given direction for a given distance. */

Task *
create_move_dir_task(Unit *unit, int dir, int n)
{
    Task *task = create_task(TASK_MOVE_DIR);

    task->args[0] = dir;
    task->args[1] = n;
    return task;
}

/* Fill in the given unit with direction-moving orders. */

void
set_move_dir_task(Unit *unit, int dir, int n)
{
    add_task(unit, CLEAR_AGENDA, create_move_dir_task(unit, dir, n));
}

void
push_move_dir_task(Unit *unit, int dir, int n)
{
    add_task(unit, 0, create_move_dir_task(unit, dir, n));
}

Task *
create_move_to_task(Unit *unit, int x, int y, int dist)
{
    Task *task = create_task(TASK_MOVE_TO);

    /* Other (time-critical AI) code expects that our data is clean,
       so filter it. */
    if (!in_area(x, y)) {
	run_warning("move-to task with bad dest %d,%d, replacing", x, y);
	/* Replace with a location guaranteed to be valid. */
	x = area.width / 2;  y = area.height / 2;
    }
    task->args[0] = x;  task->args[1] = y;  task->args[2] = 0;
    task->args[3] = dist;
    task->args[4] = eitherway;
    return task;
}

void
set_move_to_task(Unit *unit, int x, int y, int dist)
{
    add_task(unit, CLEAR_AGENDA, create_move_to_task(unit, x, y, dist));
}

void
push_move_to_task(Unit *unit, int x, int y, int dist)
{
    add_task(unit, 0, create_move_to_task(unit, x, y, dist));
}

Task *
create_occupy_task(Unit *unit, Unit *transport)
{
    Task *task = create_task(TASK_OCCUPY);

    task->args[0] = transport->id;
    task->args[1] = eitherway;
    /* add a waiting period also? */
    return task;
}

void
set_occupy_task(Unit *unit, Unit *transport)
{
    add_task(unit, CLEAR_AGENDA, create_occupy_task(unit, transport));
}

void
push_occupy_task(Unit *unit, Unit *transport)
{
    add_task(unit, 0, create_occupy_task(unit, transport));
}

Task *
create_pickup_task(Unit *unit, Unit *occ)
{
    Task *task = create_task(TASK_PICKUP);

    task->args[0] = occ->id;
    /* add a waiting period also? */
    return task;
}

void
push_pickup_task(Unit *unit, Unit *occ)
{
    add_task(unit, 0, create_pickup_task(unit, occ));
}

Task *
create_produce_task(Unit *unit, int m, int n)
{
    Task *task = create_task(TASK_PRODUCE);

    task->args[0] = m;
    task->args[1] = n;
    /* Third arg is amount produced, which starts at 0. */
    return task;
}

void
push_produce_task(Unit *unit, int m, int n)
{
    add_task(unit, 0, create_produce_task(unit, m, n));
}

/* Construct Task */

Task *
create_construct_task(Unit *unit, int u, int run, int transid, int x, int y)
{
    Task *task = NULL;

    task = create_task(TASK_CONSTRUCT);
    task->args[0] = u;
    task->args[1] = run;
    task->args[2] = transid;
    task->args[3] = x;
    task->args[4] = y;
    return task;
}

void
set_construct_task(Unit *unit, int u, int run, int transid, int x, int y)
{
    add_task(
	unit, CLEAR_AGENDA, create_construct_task(unit, u, run, transid, x, y));
}

void
push_construct_task(Unit *unit, int u, int run, int transid, int x, int y)
{
    add_task(unit, 0, create_construct_task(unit, u, run, transid, x, y));
}

/* Repair Task */

Task *
create_repair_task(Unit *unit, int id, int hp)
{
    Task *task = NULL;

    task = create_task(TASK_REPAIR);
    task->args[0] = id;
    task->args[1] = hp;
    return task;
}

void
set_repair_task(Unit *unit, int id, int hp)
{
    add_task(unit, CLEAR_AGENDA, create_repair_task(unit, id, hp));
}

void
push_repair_task(Unit *unit, int id, int hp)
{
    add_task(unit, 0, create_repair_task(unit, id, hp));
}

/* This routine sets up a task to develop a unit of the given type. */

Task *
create_develop_task(Unit *unit, int u2, int n)
{
    Task *task = create_task(TASK_DEVELOP);

    task->args[0] = u2;
    task->args[1] = n;
    return task;
}

void
set_develop_task(Unit *unit, int u2, int techgoal)
{
    add_task(unit, CLEAR_AGENDA, create_develop_task(unit, u2, techgoal));
}

void
push_develop_task(Unit *unit, int u2, int techgoal)
{
    add_task(unit, 0, create_develop_task(unit, u2, techgoal));
}

Task *
create_resupply_task(Unit *unit, int m)
{
    Task *task = create_task(TASK_RESUPPLY);

    task->args[0] = m;
    return task;
}

void
set_resupply_task(Unit *unit, int m)
{
    add_task(unit, CLEAR_AGENDA, create_resupply_task(unit, m));
}

Task *
create_sentry_task(Unit *unit, int n)
{
    Task *task = create_task(TASK_SENTRY);

    task->args[0] = n;
    return task;
}

void
set_sentry_task(Unit *unit, int n)
{
    add_task(unit, CLEAR_AGENDA, create_sentry_task(unit, n));
}

void
push_sentry_task(Unit *unit, int n)
{
    add_task(unit, 0, create_sentry_task(unit, n));
}

extern int parse_location(Side *side, char *arg, int *xp, int *yp);

extern Unit *parse_unit(Side *side, char *arg);

/* Find a unit with the given name, either alive or dead. */

Unit *
parse_unit(Side *side, char *nm)
{
    Unit *unit;

    if (empty_string(nm))
      return NULL;
    for_all_side_units(side, unit) {
	if (alive(unit) && unit->name != NULL && strcmp(unit->name, nm) == 0)
	  return unit;
    }
    /* Under some circumstances, we can refer to other sides' units by name. */
    for_all_units(unit) {
	if (alive(unit)
	    && unit->side != side
	    && unit->name != NULL
	    && strcmp(unit->name, nm) == 0
	    && side_sees_image(side, unit))
	  return unit;
    }
    return NULL;
}

/* Given a textual description of a location, compute an x,y for it
   if possible. */

int
parse_location(Side *side, char *arg, int *xp, int *yp)
{
    char *arg2;
    Unit *unit;

    *xp = strtol(arg, &arg2, 10);
    if (arg != arg2 && *arg2 == ',') {
	*yp = strtol(arg2 + 1, &arg, 10);
	if (arg2 + 1 != arg)
	    return TRUE;
    } else if ((unit = parse_unit(side, arg)) != NULL) {
	*xp = unit->x;  *yp = unit->y;
	return TRUE;
    }
    notify(side, "location \"%s\" not recognized", arg);
    return FALSE;
}

/* Given a string describing a task that has been entered in
   by a player, generate a task object and return the rest
   of the string, if NULL if failure. */

const char *
parse_task(Side *side, const char *str, Task **taskp)
{
    int tasktype, i, x, y, n, dir, u, taskargs[MAXTASKARGS], numargs;
    char *arg, *arg2, substr[BUFSIZE];
    const char *rest;
    const char *argtypes;
    Unit *unit;

    *taskp = NULL;
    rest = get_next_arg(str, substr, &arg);
    /* Recognize special cases of task types first. */
    if (strcmp(arg, "nil") == 0 || strcmp(arg, "nothing") == 0) {
	/* NULL task with non-NULL return indicates order cancellation. */
	*taskp = NULL;
	return rest;
    } else if (strcmp(arg, "move-near") == 0) {
	rest = get_next_arg(rest, substr, &arg);
	if (parse_location(side, arg, &x, &y)) {
	    rest = get_next_arg(rest, substr, &arg);
	    n = strtol(arg, &arg2, 10);
	    *taskp = create_move_to_task(NULL, x, y, n);
	    return rest;
	}
    }
    tasktype = lookup_task_type(arg);
    if (tasktype < 0) {
	notify(side, "task type \"%s\" not recognized", arg);
	return NULL;
    }
    switch (tasktype) {
      case TASK_MOVE_TO:
	rest = get_next_arg(rest, substr, &arg);
	if (parse_location(side, arg, &x, &y)) {
	    *taskp = create_move_to_task(NULL, x, y, 0);
	    return rest;
	} else {
	    return NULL;
	}
	break;
      default:
	argtypes = taskdefns[tasktype].argtypes;
	numargs = strlen(argtypes);
	for (i = 0; i < numargs; ++i)
	  taskargs[i] = 0;
	rest = get_next_arg(rest, substr, &arg);
	for (i = 0; i < numargs; ++i) {
	    if (argtypes[i] == 'x' && argtypes[i+1] == 'y') {
		/* If there are two arguments that are together a position,
		   interpret both together. */
		if (parse_location(side, arg, &x, &y)) {
		    taskargs[i] = x;  taskargs[i + 1] = y;
		    ++i;
		} else {
		    return NULL;
		}
	    } else if (argtypes[i] == 'd') {
		const char *mydirchars = "ulnbhy"; /* (a local copy of ui.c thing) */
		/* Match on names or chars for directions. */
		for_all_directions(dir) {
		    if (strcmp(arg, dirnames[dir]) == 0) {
			taskargs[i] = dir;
			goto nextarg;
		    }
		    if (strlen(arg) == 1 && arg[0] == mydirchars[dir]) {
			taskargs[i] = dir;
			goto nextarg;
		    }
		}
		notify(side, "direction \"%s\" not recognized", arg);
	    } else if (argtypes[i] == 'u') {
		u = utype_from_name(arg);
		if (u != NONUTYPE) {
		    taskargs[i] = u;
		} else {
		    notify(side, "unit type \"%s\" not recognized", arg);
		}
	    } else if (argtypes[i] == 'U') {
		unit = parse_unit(side, arg);
		if (unit != NULL) {
		    taskargs[i] = unit->id;
		} else {
		    notify(side, "unit called \"%s\" not recognized", arg);
		}
	    } else {
		/* Just collect an integer and stuff it. */
		taskargs[i] = strtol(arg, &arg2, 10);
		if (arg == arg2) {
		    notify(side, "argument \"%s\" not recognized", arg);
		}
	    }
	  nextarg:
	    rest = get_next_arg(str, substr, &arg);
	    /* (should check for end of command or not?) */
	}
	*taskp = create_task((TaskType)tasktype);
	for (i = 0; i < numargs; ++i) {
	    (*taskp)->args[i] = taskargs[i];
	}
	return rest;
    }
}

/* Describe a task succinctly - use for debugging only. */

char *
task_desig(Task *task)
{
    int i, slen;
    const char *argtypes;

    if (taskbuf == NULL)
      taskbuf = (char *)xmalloc(BUFSIZE);
    if (task) {
	sprintf(taskbuf, "{%s", taskdefns[task->type].name);
	argtypes = taskdefns[task->type].argtypes;
	slen = strlen(argtypes);
	for (i = 0; i < slen; ++i) {
	    tprintf(taskbuf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	tprintf(taskbuf, " x %d", task->execnum);
	if (task->retrynum > 0) {
	    tprintf(taskbuf, " fail %d", task->retrynum);
	}
	strcat(taskbuf, "}");
    } else {
	sprintf(taskbuf, "no task");
    }
    return taskbuf;
}


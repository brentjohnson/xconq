/* The main simulation-running code in Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is the main simulation-running code. */

#include "conq.h"
#include "kernel.h"
#include "kpublic.h"

/* This is for storing the most recently executed action. */

Action *latest_action = NULL;

/* The last known position of the latest actor. */

int latest_action_x;
int latest_action_y;

static void compose_actionvectors(void);
static void init_movement(void);
static void clear_aicontrolled_outcomes(void);
static int move_some_units(int lim);
static int side_move_some_units(Side *side, int lim);
static int unit_still_acting(Unit *unit);
static int unit_still_acting_no_plan(Unit *unit);
static int move_one_unit_multiple(Unit *unit, int lim);
static int action_reaction_needed(int type, int rslt);
static int all_human_only_sides_finished(void);

static void test_for_game_start(void);
static void test_for_game_end(void);
static int all_sides_finished(void);
static void check_realtime(void);
static int exceeded_rt_for_game(void);
static int exceeded_rt_per_turn(void);
static int units_still_acting(Side *side);
static int unit_priority(Unit *unit);
static void auto_pick_new_plan(Unit *unit);

/* Advanced unit support. */

extern void allocate_used_cells(Unit *unit);	/* Used in move.c */

static void run_side_research(void);
static void run_population(Unit *unit);
static void run_research(Unit *unit);
static int auto_pick_new_build_task(Unit *unit);

static void take_all_from_treasury(Unit *unit);
static void give_all_to_treasury(Unit *unit);

/* Priority of sides that are now moving. */

int curpriority;

/* Priority of units that are now moving. */

int cur_unit_priority;

/* Highest and lowest possible priority of unit (smallest and largest
   number). */

int highest_unit_priority;
int lowest_unit_priority;

int maintimeout = -1;

int paused = FALSE;

/* State variables. */
/* (I don't think all of these are strictly necessary) */

/* This becomes TRUE the first time run_game is executed. */

int gameinited = FALSE;

/* This is true only before the game actually starts. */

int beforestart = TRUE;

/* This is true only at the beginning of a turn. */

int at_turn_start = FALSE;

/* This is true after the game is over. */

int endofgame = FALSE;

/* This is true when the program may actually exit. */

int ok_to_exit;

/* This is set FALSE whenever the game state changes, and TRUE whenever
   the game has been saved. */

int gamestatesafe = TRUE;

/* This is TRUE after the designer has been mucking around, or if
   networked versions are found to be inconsistent. */

int compromised = FALSE;

int in_run_game = FALSE;

/* The time at which the game actually starts. */

time_t game_start_in_real_time;

/* The point in the turn at which players can actually do things. */

time_t turn_play_start_in_real_time;

int planexecs;

/* The count of task executions during a call to run_game. */

int taskexecs;

/* The rate at which AIs play when acting more slowly (so as
   not to overwhelm human players), expressed as plan executions
   per minute.  0 means "as fast as possible". */

int slow_play_rate = 240;

/* The rate at which AIs play when acting more quickly (such
   as when all humans are done with their moves). */

int fast_play_rate = 0;

/* The current rate at which AIs are playing. */

int current_play_rate;

/* Debugging counts for when run_game does nothing many times. */

static int nothing_count;

/* True when an event occurs and we need to check scorekeepers. */

int need_post_event_scores;

/* Flags that other code uses to signal the AI code that it ought
   to consider running something. */

int need_ai_init_turn;

int need_ai_planning;

int need_ai_action_reaction;

int need_ai_for_new_side;

int need_ai_finish_movement;

int debugging_state_sync;

void
init_run(void)
{
    int u;
    Unit *unit;
    Side *side;

    highest_unit_priority = 9999;
    lowest_unit_priority = -1;
    for_all_unit_types(u) {
        highest_unit_priority =
	  min(highest_unit_priority, u_action_priority(u));
        lowest_unit_priority =
	  max(lowest_unit_priority, u_action_priority(u));
        for_all_sides(side) {
	    if (side->action_priorities) {
		highest_unit_priority =
		  min(highest_unit_priority, side->action_priorities[u]);
		lowest_unit_priority =
		  max(lowest_unit_priority, side->action_priorities[u]);
	    }
        }
    }
    for_all_units(unit) {
        if (unit->extras) {
	    highest_unit_priority =
	      min(highest_unit_priority, unit_extra_priority(unit));
	    lowest_unit_priority =
	      max(lowest_unit_priority, unit_extra_priority(unit));
        }
    }
}

/* This function does a (small, usually) amount of simulation, then
   returns.  It can be run multiple times at any time, will not go
   "too far".  It returns the number of actions that were actually
   performed. Other important state changes (such a side finishing its
   turn or the turn ending) are also counted as actions, so that this
   function's callers will know that something was done. */

static void save_run_state(const char *suffix);

int
run_game(int maxactions)
{
    int numacted, numother, runtime, numdone, bump;
    int last_taskexecs = taskexecs;
    long saved_randstate;
    time_t rungamestart, rungameend;
    Side *side;
    extern long randstate;
    const char *activity = "run_game";

    in_run_game = TRUE;
    record_activity_start(activity, maxactions);
    if (Debug)
      save_run_state("a");
    gameinited = TRUE;
    saved_randstate = randstate;
    time(&rungamestart);
    numacted = numother = planexecs = taskexecs = 0;
    need_ai_planning = FALSE;
    /* Make sure the action record is allocated. */
    if (latest_action == NULL)
      latest_action = (Action *) xmalloc(sizeof(Action));
    if (beforestart) {
	/* If we haven't started yet, see if it's time. */
	test_for_game_start();
	Dprintf("run_game: tested for game start.\n");
    }
    if (endofgame) {
	/* Nothing to do except wait for users to do exit commands. */
    	Dprintf("run_game: at end of game.\n");
    } else if (paused) {
	/* Don't do anything if we're paused. */
    	Dprintf("run_game: paused.\n");
    } else if (numdesigners > 0) {
	/* Don't try to run the game if any designers are around. */
    	Dprintf("run_game: designing.\n");
    } else if (!beforestart) {
	if (at_turn_start) {
	    if (midturnrestore)
	      run_restored_turn_start();
	    else
	      run_turn_start();
	    check_all_units();
	    init_movement();
	    cur_unit_priority = highest_unit_priority;
	    compose_actionvectors();
	    update_all_progress_displays("", -1);
	    /* Make sure the research window is popped up for sides
	    without a research task. */
	    for_all_sides(side) {
		    if (side_has_display(side)) {
			update_research_display(side);
		    }
	    }
	    /* Game might have been ended by new turn init. */
	    test_for_game_end();
	    if (endofgame) {
	    	Dprintf("run_game: game ended by new turn init.\n");
	    	goto run_game_return;
	    }
	    /* (should adjust this by recorded elapsed turn time) */
	    time(&turn_play_start_in_real_time);
	    at_turn_start = FALSE;
	    ++numother;
	    /* Might have local AIs that need to run, so give them a chance
	       now rather than after units start moving. */
	    need_ai_init_turn = TRUE;
	    need_ai_planning = TRUE;
	    goto run_game_return;
	}
	/* If this game is running in realtime, update all clock displays. */
	if (realtime_game()) {
	    for_all_sides(side) {
		if (side->ingame && side_has_display(side)) {
		    update_clock_display(side, TRUE);
		}
	    }
	}
	/* If all sides are done acting, end the turn.  This will never be true
	   right at the start of a turn. */
	if (all_sides_finished() || exceeded_rt_per_turn()) {
	    run_turn_end();
	    Dprintf("run_game: at turn end.\n");
	    at_turn_start = TRUE;
	    ++numother;
	    need_ai_finish_movement = TRUE;
	} else {
	    /* First do some side research. Note: we have to do this
	       here rather than during run_turn_start since we must
	       give human players and ais a chance to pick a new
	       research task if the current one is
	       completed. Otherwise, no side can ever finish more than
	       one advance each turn. */
	    run_side_research();
	    if (last_taskexecs > 0)
	      clear_aicontrolled_outcomes();
	    /* Move some units around. */
	    numacted += move_some_units(maxactions);
	    if (cur_unit_priority < lowest_unit_priority) {
		/* Handle prioritized movement. */
		bump = TRUE;
		for_all_sides(side) {
		    if (!side->finishedturn
			&& units_still_acting(side)) {
			bump = FALSE;
		    }
		}
		if (bump) {
		    Dprintf("run_game: increment unit priority to %d\n",
			    cur_unit_priority + 1);
		    ++cur_unit_priority;
		    compose_actionvectors();
		    ++numother;
		}
	    } else {
		/* Possibly finish some sides' turns. */
		for_all_sides(side) {
		    if (!side->finishedturn
			&& ((!units_still_acting(side)
			     && side->autofinish
			     && !is_designer(side))
			    /* Sides with no display or AI finish
                               automatically. */
			    || (!side_has_display(side)
				&& !side_has_ai(side)))) {
			Dprintf("run_game: %s auto-finishes.\n",
				side_desig(side));
			finish_turn(side);
			++numother;
		    }
		}
	    }
	}
	check_realtime();
	test_for_game_end();
    }
 run_game_return:
    if (need_post_event_scores)
      check_post_event_scores();
    numdone = numacted + planexecs + taskexecs + numother;
    if (Debug) {
	if (numdone > 0) {
	    if (nothing_count > 0) {
		Dprintf("run_game: Did nothing %d times\n", nothing_count);
		nothing_count = 0;
	    }
	    Dprintf("run_game #%d: %d/%d actions", g_run_serial_number(),
		    numacted, maxactions);
	    if (planexecs > 0)
	      Dprintf(", %d plan execs", planexecs);
	    if (taskexecs > 0)
	      Dprintf(", %d task execs", taskexecs);
	    if (numother > 0)
	      Dprintf(", %d other", numother);
	    /* (also number of units considered?) */
	    Dprintf("\n");
	    if (numremotes > 0)
	      Dprintf("run_game: Randstate started at %ld, is now %ld\n",
		      saved_randstate, randstate);
	    save_run_state("");
	} else {
	    if (nothing_count >= 1000) {
		Dprintf("run_game: Did nothing %d times\n", nothing_count);
		nothing_count = 0;
	    } else {
		++nothing_count;
	    }
	}
    }
    time(&rungameend);
    runtime = idifftime(rungameend, rungamestart);
    if (runtime > 0)
      Dprintf("run_game: runtime seconds = %d\n", runtime);
    if (Debug)
      save_run_state("b");
    in_run_game = FALSE;
    set_g_run_serial_number(g_run_serial_number() + 1);
    record_activity_end(activity, numdone);
    return numdone;
}

/* The following routine saves a copy of the game's state in a
   specially-named file, for the purposes of debugging networking sync
   problems.  This generates massive amounts of saved data very
   quickly, be careful using it! */

static int save_run_states = 0;

void
save_run_state(const char *suffix)
{
    if (numremotes > 0 && save_run_states) {
	sprintf(spbuf, "states/%dstate%06d%s",
		my_rid, g_run_serial_number(), suffix);
	debugging_state_sync = TRUE;
	write_entire_game_state(spbuf);
	debugging_state_sync = FALSE;
    }
}

/* See if game is ready to get underway for real.  Note that displays will
   work before the game has started, but game time doesn't move. */

static void
test_for_game_start(void)
{
    int anydisplays = FALSE;
    Side *side;

    /* We must have at least one unit on a side that is being displayed
       before the game can start for real. */
    for_all_sides(side) {
	if (side_has_display(side)) {
	    anydisplays = TRUE;
	}
	if (side_has_units(side) && side_has_display(side)) {
	    /* Now we're really ready to roll. */
	    beforestart = FALSE;
	    at_turn_start = TRUE;
	    if (midturnrestore) {
		record_event(H_GAME_RESTARTED, ALLSIDES);
	    } else {
		record_event(H_GAME_STARTED, ALLSIDES);
		set_g_elapsed_time(0);
	    }
	    /* Record the game as starting NOW in real time. */
	    time(&game_start_in_real_time);
	    /* Adjust by any recorded elapsed time. */
	    game_start_in_real_time -= g_elapsed_time();
	    /* No need to look at any more sides, just get on with the game. */
	    return;
	}
    }
    if (!anydisplays) {
	init_warning("No sides have a display");
    }
}

/* This routine looks to see if the game is completely over. */

static void
test_for_game_end(void)
{
    Side *side;

    /* Declare a draw if everybody is amenable. */
    if (all_others_willing_to_quit(NULL)) {
    	notify_all("All sides have agreed to declare a draw.");
    	all_sides_draw();
	end_the_game();
	return;
    }
    for_all_sides(side) {
    	/* If we have an active side being displayed, we're not done yet. */
	if (side->ingame && side_has_display(side))
	  return;
	/* (If no displayed sides have units, turns will whiz by) */
    }
    notify_all("All sides with displays are out of the game.");
    end_the_game();
}

/* This is true when all participating sides have finished their turn. */

static int
all_sides_finished(void)
{
    Side *side;

    for_all_sides(side) {
	if (side->ingame
	    && !side->finishedturn) {
	    return FALSE;
	}
    }
    return TRUE;
}

/* This is true when AIs should move more quickly. */

int
all_human_only_sides_finished(void)
{
    Side *side;

    for_all_sides(side) {
	if (side->ingame
	    && side_has_display(side)
	    && !side_has_ai(side)
	    && !side->finishedturn) {
	    return FALSE;
	}
    }
    return TRUE;
}

/* Call this from interfaces to check on realtime details without actually
   going into run_game.  Will call back to interface if necessary. */

void
check_realtime(void)
{
    Side *side;

    if (!realtime_game())
      return;
    if (exceeded_rt_for_game()) {
	notify_all("Time has run out!");
	end_the_game();
    }
    if (g_rt_per_side() > 0) {
	for_all_sides(side) {
	    if (side->ingame && side->totaltimeused > g_rt_per_side()) {
		remove_side_from_game(side);
	    }
	}
    }
}

int
exceeded_rt_for_game(void)
{
    time_t now;

    if (g_rt_for_game() <= 0)
      return FALSE;
    time(&now);
    /* Note that the game start time is already adjusted for any
       elapsed time recorded when the game was last saved. */
    return (idifftime(now, game_start_in_real_time) > g_rt_for_game());
}

int
exceeded_rt_per_turn(void)
{
    time_t now;

    if (g_rt_per_turn() <= 0)
      return FALSE;
    time(&now);
    return (idifftime(now, turn_play_start_in_real_time) > g_rt_per_turn());
}

/* This returns true if the given side is still wanting to do stuff. */

int
units_still_acting(Side *side)
{
    int curactor, a;
    Unit *unit;
    UnitVector *av = side->actionvector;

    if (!side->ingame)
      return FALSE;
    /* Test current actor first, most likely to be still acting. */
    if (side->curactor_pos < av->numunits) {
	unit = unit_in_vector(av, side->curactor_pos);
	if (unit_still_acting(unit) && side_controls_unit(side, unit))
	  return TRUE;
    }
    /* Now scan the whole action vector.  It's expected that all
       active units will be in this vector. */
    for (curactor = 0; curactor < av->numunits; ++curactor) {
	unit = unit_in_vector(av, curactor);
	if (unit_still_acting(unit) && side_controls_unit(side, unit))
	  return TRUE;
    }
    /* Try not to let a side not be researching something. */
    if (numatypes > 0
	&& g_side_can_research()
	&& side->research_topic == NOADVANCE) {
	/* Check that the side can research something. */
	for_all_advance_types(a) {
	    if (side_can_research(side, a)) {
		/* Popup the research dialog if the side has a 
		   display. */
		if (side_has_display(side)) {
		    update_research_display(side);
		}
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* Handle movement priorities and various flags. */

static void
init_movement(void)
{
    int i;
    Side *side;

    i = 1;
    curpriority = 9999;
#if (0)
    // If the indepside has no AI and cannot build or research, it's done.
    if (!g_indepside_can_research()
	&& !g_indepside_has_ai())
      indepside->finishedturn = TRUE;
#endif
    // If indepside has no AI, then its turn is finished.
    // TODO: Make this true of any side that is run by nobody.
    if (!g_indepside_has_ai())
	indepside->finishedturn = TRUE;
    for_all_sides(side) {
	if (side->ingame) {
	    /* Record that this side was active during at least one turn. */
	    side->everingame = TRUE;
	    /* No units are waiting for orders initially. */
	    side->numwaiting = 0;
	}
	side->turnstarttime = time(0);
	/* Didn't really do input, but useful to pretend so. */
	side->lasttime = time(0);
	/* Calculate side priorities; do here so future versions can
	   set priorities dynamically. */
	if (g_use_side_priority()) {
	    if (side->priority < 0) {
		side->priority = i++;
	    }
	}
	side->busy = FALSE;
	if (side_has_display(side))
	  update_action_display(side, TRUE);
    }
    /* Set independent units to move after units on sides. */
    if (g_use_side_priority()) {
	if (indepside->priority < 0) {
	    indepside->priority = i;
	}
	for_all_sides(side) {
	    if (!side->finishedturn && side->priority < curpriority)
	      curpriority = side->priority;
	}
    }
}

/* Create (if necessary) and fill in the action vectors, which are the
   arrays of units that will be moving during the turn. */

static void
compose_actionvectors(void)
{
    int priority;
    Unit *unit;
    Side *side, *side2;

    for_all_sides(side) {
	if (side->actionvector == NULL)
	  side->actionvector = make_unit_vector(max(numunits, 100));
	clear_unit_vector(side->actionvector);
	for_all_side_units(side, unit) {
	    if (unit->act 
	        && ((unit->act->initacp > 0)
	            || acp_indep(unit))) {
		priority = unit_priority(unit);
		if (priority == cur_unit_priority) {
		    side->actionvector =
		      add_unit_to_vector(side->actionvector, unit, 0);
		    /* Clear any delay flags. */
		    if (unit->plan)
		      unit->plan->delayed = FALSE;
		    /* Clear any busy flags. */
		    if (unit->busy) {
		    	/* The busy flag should have been cleared when the unit 
			   moved. If it is still set, it could mean that a 
			   broadcasted action was never received by the host 
		    	   and rebroadcasted. */
			Dprintf(
"Busy flag still set for %s at start of turn.\n", 
				unit_desig(unit));
			unit->busy = FALSE;
		    }
		}
	    }
	    if (unit->plan) {
		unit->plan->execs_this_turn = 0;
	    }
	}
	Dprintf("Action vector for %s has %d units, at priority %d\n",
		side_desig(side), side->actionvector->numunits,
		cur_unit_priority);
    }
    /* Inform sides with displays of how many units are ready to act. */
    for_all_sides(side) {
	if (side_has_display(side)) {
	    for_all_sides(side2) {
		update_side_display(side, side2, TRUE);
	    }
	}
    }
}

int
unit_priority(Unit *unit)
{
    int pri;
    Side *side = unit->side;

    pri = unit_extra_priority(unit);
    if (pri >= 0)
      return pri;
    if (side->action_priorities != NULL) {
	pri = side->action_priorities[unit->type];
	if (pri >= 0)
	  return pri;
    }
    return u_action_priority(unit->type);
}

static void
clear_aicontrolled_outcomes(void)
{
    Unit *unit;

    /* The human side is not always able to react between two
       runs. Premature loss of task outcomes causes interface
       problems. Since the only purpose of clearing outcomes is to
       send a signal to run_local_ai, we only clear them for
       ai-controlled units. */
    for_all_units(unit) {
	if (ai_controlled(unit))
	  clear_task_outcome(unit);
    }
}

/* Do some number of actions, up to a total of LIM per side.  Return
   the total number of actions performed.  If sides are moving in
   priority order (aka sequentially), only allow the side matching the
   current priority to do anything. */

static int
move_some_units(int lim)
{
    int num = 0, sidenum;
    Side *side;

    for_all_sides(side) {
	if ((g_use_side_priority() ?
	     curpriority == side->priority :
	     TRUE)) {
	    sidenum = side_move_some_units(side, lim);
	    num += sidenum;
	}
    }
    return num;
}

/* Do some number of actions. */

static int
side_move_some_units(Side *side, int lim)
{
    int num, foundanytomove, curactor0, curactor, numcouldact = 0, numfollowing;
    int numdelayed = 0;
    Unit *unit;
    UnitVector *av = side->actionvector;

    num = 0;
    curactor0 = 0;
    /* If we were here last while working on moving a particular unit,
       and it's still around, start with it. */
    if (side->curactor_pos < av->numunits
        && side->curactor == unit_in_vector(av, side->curactor_pos)
        && side->curactor != NULL
        && side->curactor_id == unit_in_vector(av, side->curactor_pos)->id)
      curactor0 = side->curactor_pos;
    /* (is this needed to maintain sync, or what??) */
    if (numremotes > 0)
      curactor0 = 0;
  try_again:
    foundanytomove = FALSE;
    numcouldact = 0;
    numdelayed = 0;
    numfollowing = 0;
    /* Iterate through all units in the action vector, starting with
       the "current" (most recently moved) unit. */
    /* Note that the action vector may get resized/reallocated as
       units act (such as when a unit is captured), so we can't cache
       it into a local var here. */
    for (curactor = curactor0; curactor < side->actionvector->numunits;
	 ++curactor) {
	unit = unit_in_vector(side->actionvector, curactor);
#if 0 /* use this for more intense debugging */
	Dprintf("Considering moving %s with plan %s\n",
		unit_desig(unit), plan_desig(unit->plan));
#endif
	/* If the unit cannot act anymore, then skip it forthwith. */
	if (!has_acp_left(unit))
	  continue;
	/* Count and skip over deliberately delayed units. */
	if (unit->plan && unit->plan->delayed) {
	    ++numcouldact;
	    ++numdelayed;
	    continue;
	}
	/* Count the units which are in formations and can still act. */
	if (unit->plan && unit->plan->formation && unit_still_acting(unit))
	  ++numfollowing;
	/* Default AIs to the slow play rate. */
	current_play_rate = slow_play_rate;
	/* AIs should play as fast as possible if turns are sequential,
	   or if the human players are all done moving for this turn. */
	if (g_use_side_priority() || all_human_only_sides_finished())
	  current_play_rate = fast_play_rate;
	/* If the unit is keeping formation, then give it a chance to
	   adjust its position, even if it's not "still acting". */
	if (is_active(unit)
	    && unit->side->ingame 
	    && !unit->side->finishedturn
	    && has_acp_left(unit)
	    && (unit->plan && unit->plan->formation
		&& !is_in_formation(unit))) {
	    num += move_one_unit_multiple(unit, lim - num);
	    foundanytomove = TRUE;
	}
	/* Execute standing orders. */
	if (unit->side->orders
	    && unit->plan
	    && unit->plan->tasks == NULL
	    && execute_standing_order(unit, FALSE)) {
	    /* We're not waiting because standing order execution will
	       shortly be allowed to fill in a task for real. */
	    set_waiting_for_tasks(unit, FALSE);
	    num += move_one_unit_multiple(unit, lim - num);
	    foundanytomove = TRUE;
	}
	/* Execute any tasks or pending actions that the unit has. */
	if (unit_still_acting(unit)
	    && (unit->plan && !unit->plan->waitingfortasks)) {
	    num += move_one_unit_multiple(unit, lim - num);
	    foundanytomove = TRUE;
	} else if (unit_still_acting_no_plan(unit)) {
	    if (has_pending_action(unit) || (unit->plan && unit->plan->tasks)) {
		num += move_one_unit_multiple(unit, lim - num);
		foundanytomove = TRUE;
	    }
	}
	/* See if we exceeded the limit on the number of moves that
	   we're allowed to do this time.  If so, memorize the unit
	   that we were working on and return. */
	if (num >= lim) {
	    if (foundanytomove && unit != NULL) {
		side->curactor_pos = curactor;
		side->curactor = unit;
		/* Important to remember the id, in case the unit dies
		   or changes side, invalidating the raw pointer. */
		side->curactor_id = unit->id;
	    } else {
		side->curactor_pos = 0;
		side->curactor = NULL;
		side->curactor_id = 0;
	    }
	    return num;
	}
	/* Increment the count of units that could act. */
	++numcouldact;
    }
    /* If started in middle of list, and didn't find anything to do,
       rescan from beginning. */
    if (!foundanytomove && curactor0 > 0) {
	curactor0 = 0;
	goto try_again;
    }
    /* See if any at all could act, and let the 'run_game' logic deal 
       with it. */
    if (!numcouldact) 
      return 0;
    /* Clear all the delay flags and rescan the action vector, if no other 
       non-delayed actors left. */
    if (!foundanytomove && (0 >= (numcouldact - numdelayed))) {
	av = side->actionvector;
	for (curactor = 0; curactor < av->numunits; ++curactor) {
	    unit = unit_in_vector(av, curactor);
	    if (unit->plan)
	      unit->plan->delayed = FALSE;
	}
	curactor0 = 0;
	goto try_again;
    }
    if (!foundanytomove && 0 /* not at max priority */) {
	/* (should recompose action vector for new unit priority?) */
    }
    /* Set waiting-for-tasks to true for any units in a formation, which 
       still are acting (have ACP, are not skipped, etc...). */
    if (numfollowing > 0) {
	for (curactor = 0; curactor < av->numunits; ++curactor) {
	    unit = unit_in_vector(av, curactor);
	    if (unit->plan && unit->plan->formation 
		&& unit_still_acting(unit) && !(unit->plan->tasks))
	      set_waiting_for_tasks(unit, TRUE);
	}
    }
    side->curactor_pos = 0;
    side->curactor = NULL;
    side->curactor_id = 0;
    return num;
}

/* Return true if a unit is not yet finished moving. */

static int
unit_still_acting(Unit *unit)
{
    /* Conditions that always rule out further action. */
    if (!is_active(unit))
      return FALSE;
    if (!unit->side->ingame)
      return FALSE;
    if (unit->side->finishedturn)
      return FALSE;
    if (!has_acp_left(unit))
      return FALSE;
    if (!unit->side->could_act_with[unit->type])
      return FALSE;
    if (unit->plan
	 && (unit->plan->asleep
	    || unit->plan->reserve))
      return FALSE;
    /* Conditions that require further action provided
       that it was not ruled out above. */
    // HACK: We need to check action allowance flags rather than doing this.
    if (acp_indep(unit))
      return TRUE;
    if (has_pending_action(unit))
      return TRUE;
    if (unit->plan)
      return TRUE;
    return FALSE;
}

static int
unit_still_acting_no_plan(Unit *unit)
{
    /* Conditions that always rule out further action. */
    if (!is_active(unit))
      return FALSE;
    if (!unit->side->ingame)
      return FALSE;
    if (unit->side->finishedturn)
      return FALSE;
    if (!has_acp_left(unit))
      return FALSE;
    if (!unit->side->could_act_with[unit->type])
      return FALSE;
    /* Conditions that require further action provided that it was not
       ruled out above. */
    // HACK: We need to check action allowance flags rather than doing this.
    if (acp_indep(unit))
      return TRUE;
    if (has_pending_action(unit))
      return TRUE;
    return FALSE;
}

/* Do a single unit's actions, up to the given limit or until it runs
   out of things it wants to do (or something happens to it). */

static int
move_one_unit_multiple(Unit *unit, int lim)
{
    int num = 0, buzz = 0, acp1, i;
    int rslt;

    /* If unit is incapable of acting right now, get out of here. */
    if (unit->act == NULL 
        || (unit->act->initacp < 1 
            && !acp_indep(unit)))
      return 0;
    acp1 = unit->act->acp;
    while (is_active(unit) && has_acp_left(unit)
	   && ((unit->plan && !unit->plan->asleep
		&& !unit->plan->reserve && !unit->plan->delayed)
	       || has_pending_action(unit))
	   && num < lim
	   && buzz < lim) {
#if (1) /* enable for more intense debugging */
	Dprintf("  Moving %s (%d/%d, buzz %d) with plan %s\n",
		unit_desig(unit), num, lim, buzz, plan_desig(unit->plan));
#endif
        /* Unit may be dead, and thus cannot complete the rest of its actions.
           Although kill_unit removes the unit from action vector, we may 
           not have had a chance to go back up to side_move_some_units to 
           discover this.
           Tell side_move_some_units to go find another unit to move.
           (Note that execute_action does check whether the unit is alive 
           or not, but why check in each called function when we can 
           nip things in the bud right here, right now? None of the 
           called functions can reasonably do anything with a dead unit.)
        */
        if (!alive(unit)) {
            Dprintf("Tried to act with dead unit, %s!\n", unit_desig(unit));
            break;
        }
#if (0)
	/* Skip over AI-run units that have executed a task during
	   this run step, so that the AI gets a chance to react to the
	   result. */
	if (ai_controlled(unit)
	    && unit->plan->last_task_outcome != TASK_UNKNOWN
	    && unit->plan->last_task_outcome != TASK_PREPPED_ACTION)
	  break;
#endif
	if (has_pending_action(unit)) {
	    /* Save the action now in case the actor dies or wrecks. */
	    latest_action->type = unit->act->nextaction.type;
	    for (i = 0; i < MAXACTIONARGS; ++i) {
	    	latest_action->args[i] = unit->act->nextaction.args[i];
	    }
	    latest_action->actee = unit->act->nextaction.actee;
	    latest_action->next = unit->act->nextaction.next;

	    /* Also save the latest known position of the actor. */
	    latest_action_x = unit->x;
	    latest_action_y = unit->y;
	    /* Execute the action directly. */
	    rslt = execute_action(unit, &(unit->act->nextaction));
	    /* Trigger ai reaction for certain actions. */ 
	    need_ai_action_reaction = 
		action_reaction_needed(latest_action->type, rslt);
	    /* Clear the action.  Note that the unit might have changed
	       to a non-acting type, so we have to check for act struct. */
	    if (unit->act)
	      unit->act->nextaction.type = ACTION_NONE;
	    /* Also clear the busy flag. */
	    unit->busy = FALSE;
	    /* In any case, the game state is irrevocably altered. */
	    gamestatesafe = FALSE;
	    ++num;
	} else if (unit->plan != NULL) {
	    /* Even units that are asleep, in reserve, etc must execute
	       any standing orders that apply. */
	    if (unit->side
		&& unit->side->orders
		&& unit->plan->tasks == NULL
		&& execute_standing_order(unit, TRUE)) {
		planexecs += execute_plan(unit);
		gamestatesafe = FALSE;
		++buzz;
	    }
	    /* Similarly for formations. */
	    if (unit->plan->formation && move_into_formation(unit)) {
		planexecs += execute_plan(unit);
		gamestatesafe = FALSE;
		++buzz;
	    }
#if 0	    
	    /* This replanning caused buzzing. It is sufficient to call 
               ai_decide_plan at the start of each turn and after 
               force_replan. Possibly, we should test for an empty task queue 
               here, and replan in that case only. However, the AI works fine 
               without any replanning. */

	    /* Flag so that we run AI code in the near future (after
	       run_game exits). */
	    if (side_has_ai(unit->side))
	      need_ai_planning = TRUE;
#endif
	    /* Get out of here if unit is set not to do anything on
	       its own. */
	    if (unit->plan->waitingfortasks
		|| unit->plan->asleep
		|| unit->plan->reserve
		|| unit->plan->delayed)
	      break;
	    /* Normal plan execution. */
	    planexecs += execute_plan(unit);
	    record_ms();
	    gamestatesafe = FALSE;
	    ++buzz;
	} else {
	    run_warning("Planless \"%s\" was asked to act", unit_desig(unit));
	    ++buzz;
	}
	/* If the unit is trying to do actions several times in this
	   loop and and none of them are succeeding, something is
	   wrong; blast the plan and eventually put the unit to
	   sleep, if the problem persists. */
	if (unit->act && unit->act->acp == acp1 && num > 1) {
	    /* Blast the action. */
	    unit->act->nextaction.type = ACTION_NONE;
	    /* Blast the plan. */
	    if (unit->plan)
	      unit->plan->type = PLAN_PASSIVE;
	    run_warning("\"%s\" trying multiple bad actions, clearing its plan",
			unit_desig(unit));
	}
    }
    return num;
}

/* Returns true if ais may need to react on a given action and result. */
	    
int
action_reaction_needed(int type, int rslt)
{
    /* Special success flags for captures, overruns and builds that
       just completed a unit. */
    if (rslt == A_CAPTURE_SUCCEEDED
	|| rslt == A_OVERRUN_SUCCEEDED
	|| rslt == A_BUILD_COMPLETED) {
	return TRUE;
	/* Any other action returns A_ANY_DONE on success. */
    } else if (rslt == A_ANY_DONE
	       /* These actions may change the tactical situation. */
	       && ((type == ACTION_MOVE
		    || type == ACTION_ENTER
		    || type == ACTION_DISBAND
		    || type == ACTION_CHANGE_TYPE
		    || type == ACTION_CHANGE_SIDE)
		   /* Also worry about offensive actions that kill or
		      capture a unit. */
		   ||((type == ACTION_ATTACK
		       || type == ACTION_FIRE_AT
		       || type == ACTION_FIRE_INTO
		       || type == ACTION_DETONATE)
		      && (history->prev->type == H_UNIT_KILLED
			  || history->prev->type == H_UNIT_CAPTURED)))) {
	return TRUE;
    }
    return FALSE;
}

/* This explicitly finishes out a side's activity for the turn. */

void
finish_turn(Side *side)
{
    int nextpriority;
    Side *side2, *side3;

    /* Flag the side as being done for this turn. */
    side->finishedturn = TRUE;
    /* Stop counting down our time consumption. */
    side->totaltimeused += (time(0) - side->turnstarttime);
    if (g_use_side_priority()) {
	nextpriority = 9999;
	for_all_sides(side2) {
	    if (!side2->finishedturn
		/* && side2->priority > curpriority */
		&& side2->priority < nextpriority) {
		nextpriority = side2->priority;
	    }
	    if (!side2->finishedturn && side2->priority < curpriority)
	      run_warning(
"%s not finished, but priority is %d, less than current %d",
			  side_desig(side2), side2->priority, curpriority);
	}
	if (nextpriority > curpriority)
	  curpriority = nextpriority;
    }
    /* Clue everybody in. */
    if (g_use_side_priority()) {
	/* Several sides may change, if current priority changes. */
	for_all_sides(side2) {
	    for_all_sides(side3) {
		update_side_display(side2, side3, TRUE);
	    }
	}
    } else {
	/* Only the turn-finishing side changes. */
	for_all_sides(side2) {
	    update_side_display(side2, side, TRUE);
	}
    }
    Dprintf("%s finished its turn.\n", side_desig(side));
}

void
set_play_rate(int slow, int fast)
{
    if (slow < 0 || fast < 0 || fast < slow) {
	run_warning("Bad play rates slow=%d fast=%d, ignoring", slow, fast);
	return;
    }
    slow_play_rate = slow;
    fast_play_rate = fast;
}

/* Resignation, possibly giving away any remaining units. */

void
resign_game(Side *side, Side *side2)
{
    /* Nothing to do if we're not in the game. */
    if (!side->ingame)
      return;
    notify_all_of_resignation(side, side2);
    side_loses(side, side2, -1);
}

/* This is true if there is any kind of realtime limit on the game. */

int
realtime_game(void)
{
    return (g_rt_for_game() > 0
    	    || g_rt_per_side() > 0
    	    || g_rt_per_turn() > 0);
}

/* Pass NULL to see if all sides are now willing to save the game. */

int
all_others_willing_to_save(Side *side)
{
    Side *side2;

    for_all_sides(side2) {
	if (side != side2 && !side2->willingtosave)
	  return FALSE; 
    }
    return TRUE;
}

/* Pass NULL to see if all sides are now willing to declare a draw. */

int
all_others_willing_to_quit(Side *side)
{
    Side *side2;

    for_all_sides(side2) {
	if (side != side2 && !side2->willingtodraw)
	  return FALSE; 
    }
    return TRUE;
}

/* This forces an end to the game directly. */

void
end_the_game(void)
{
    Side *side;

    Dprintf("The game is over.\n");
    /* Make sure everybody sees this. */
    notify_all("END OF THE GAME!");
    record_event(H_GAME_ENDED, ALLSIDES);
    /* Set the global that indicates the game is over for everybody. */
    endofgame = TRUE;
    end_history();
    /* (should compute final scoring) */
    record_into_scorefile();
    /* Done with internal state change, now echo onto displays. */
    for_all_sides(side) {
	/* Enable the display of everything in the world. */
	if (!side->see_all)
	  side->may_set_show_all = TRUE;
	/* ...by default display it all. */
	side->show_all = TRUE;
    	if (side_has_display(side)) {
    	    update_turn_display(side, TRUE);
    	    /* (should update side's view of all sides?) */
    	    update_side_display(side, side, TRUE);
	    /* Update legends and their positions as new terrain comes into view. */
	    place_legends(side);
	    /* Redraw legends in new positions. */
	    update_area_display(side);
    	}
    }
    dump_statistics();
    /* We've done everything the kernel needs to have done; it's now up to
       the interface to decide when to exit. */
    ok_to_exit = TRUE;
}

/* Run any per-side research activities. */

void
run_side_research(void)
{
    int a, m, consump, amt, totalweight = 0;
    long maxrp, rp; 
    Side *side;
    Obj *choice = NULL, *nextgoal = NULL;

    /* No research if there are no advances. */
    if (numatypes == 0)
      return;
    if (!g_side_can_research())
      return;
    for_all_sides(side) {
	/* The indepside might just be done here. */
	if (side == indepside 
	    && (g_indepside_can_research() != TRUE
 	        || g_indepside_has_treasury() != TRUE)) {
 	    continue;
 	}
	/* Our wise men are resting. Don't disturb them. */
    	if (side->research_topic == NONATYPE)
	  continue;
	/* Our wise men are waiting for something to do. */
	if (side->research_topic == NOADVANCE) {
	    if (side_has_ai(side)) {
		/* Do nothing. ai_plan_research will pick a new advance. */
	    } else if (side->autoresearch) {
		auto_pick_side_research(side);
	    }
	    /* We are not ready to proceed until the human player or
	       the computer has picked a new research topic. */
	    continue;
	}
	a = side->research_topic;
	/* Compute the maximum number of rps that our material limits
	   allow us to add. */
	maxrp = VARHI;
	for_all_material_types(m) {
	    consump = am_consumption_per_rp(a, m);
	    if (consump > 0) {
		/* Check that all required materials have a treasury. */
	    	if (!side_has_treasury(side, m))
		  return;
		rp = side->treasury[m] / consump;
		maxrp = min(maxrp, rp);
	    }
	}
	/* If research is not limited by materials something is wrong. */
	if (maxrp == VARHI) {
	    run_error("research is not limited by materials");
	}
	/* Don't spend more than needed to complete the advance! */
	maxrp = min(maxrp, a_rp(a) - side->advance[a]);
	/* Only proceed if we are still doing some research. */
	if (maxrp > 0) {
	    side->advance[a] += maxrp;
	    /* Make sure the research window is updated. */
	    if (side_has_display(side)) {
		update_research_display(side);
	    }
	    if (side->advance[a] >= a_rp(a)) {
		side->advance[a] = DONE;
		/* Update research and build vectors. */
		update_canresearch_vector(side);
		update_canbuild_vector(side);
		/* (TODO: Replace "Your wise men discover" with a custom 
		    string.) */
		notify(side, "Your wise men discover %s!", a_type_name(a));
		side->research_topic = NOADVANCE;
		nextgoal = lispnil;
		choice = lispnil;
		if (a == side->research_goal) {
		    side->research_goal = NONATYPE;
		    nextgoal = a_ai_next_goal(a);
		}
		if (nextgoal != lispnil) {
		    if (consp(nextgoal)) {
			choice = choose_side_research_goal_from_weighted_list(
				    nextgoal, &totalweight, side);
		    }
		    else {
			choice = eval(nextgoal);
		    }
		}
		if (choice != lispnil) {
		    if (symbolp(choice))
		      choice = eval_symbol(choice);
		    if (atypep(choice)) {
			if (is_advance_type(c_number(choice)))
			  side->research_goal = c_number(choice);
			else
			  run_warning(
"Invalid research goal provided by advance, %s, upon reserach completion",
				      a_type_name(a));
		    }
		    else
		      run_warning(
"Invalid research goal provided by advance, %s, upon reserach completion",
				  a_type_name(a));
		}
		if (side_has_ai(side)) {
		    /* Do nothing. ai_plan_research will pick a new advance. */
		} else if (side->autoresearch) {
		    auto_pick_side_research(side);
		}
		/* Make sure the research window is updated. */
		if (side_has_display(side)) {
		    update_research_display(side);
		}
	    }
	    /* Consume the materials. */
	    for_all_material_types(m) {
		consump = am_consumption_per_rp(a, m);
		if (consump > 0) {
		    amt = maxrp * consump;
		    side->treasury[m] -= amt;
		}
	    }
	}
    }
}

/* 	Advanced unit and advance code for Xconq.
     	Copyright (C) 1998 Hans Ronne.
	Copyright (C) 2004 Eric A. McDonald.
*/

void
update_canresearch_vector(Side *side)
{
    int a, a2;

    update_side_research_goal_availability(side);
    for_all_advance_types(a) {
	/* We can't research advances that are already done. */
	if (has_advance(side, a)) {
	    side->canresearch[a] = FALSE;
	    continue;
	}			
	side->canresearch[a] = TRUE;
	/* Test for required advances. */
	for_all_advance_types(a2) {
	    if (aa_needed_to_research(a, a2) && !has_advance(side, a2)) {
		side->canresearch[a] = FALSE;
		break;
	    }
	    if (aa_precludes(a2, a) && has_advance(side, a2)) {
		side->canresearch[a] = FALSE;
		break;
	    }
	}
    }
}

void
update_side_research_goal_availability(Side *side)
{
    int updated = TRUE;
    int a = NONATYPE, a2 = NONATYPE;

    assert_error(side, "Attempted to access a NULL side");
    while (updated) {
	updated = FALSE;
	for_all_advance_types(a) {
	    if (side->research_precluded[a])
	      continue;
	    for_all_advance_types(a2) {
		if ((aa_precludes(a2, a) && has_advance(side, a2)) 
		    || (side->research_precluded[a2] 
			&& aa_needed_to_research(a, a2))) {
			side->research_precluded[a] = TRUE;
			updated = TRUE;
			break;
		    }
	    } /* a2 */
	    if (updated)
	      break;
	} /* a */
    } /* while updated */
}

/* Call this whenever the list of buildable/developable types changes. */

void
update_canbuild_vector(Side *side)
{
    int a, u;

    for_all_unit_types(u) {
	/* Start out optimistic, then following code may disallow some
           types. */
	side->canbuild[u] = TRUE;
	side->candevelop[u] = TRUE;
	/* Units with zero cp are unbuildable by definition. */
	if (u_cp(u) == 0) {
	    side->canbuild[u] = FALSE;
	    side->candevelop[u] = FALSE;
	    continue;
	}
	/* First test if this unit type is at all allowed on our side. */
	if (!type_allowed_on_side(u, side)) {
	    side->canbuild[u] = FALSE;
	    side->candevelop[u] = FALSE;
	    continue;
	}
	/* Then test if we have the required tech to both build and own 
	   this type. */
	if (u_tech_to_build(u) > 0) {
	    /* Don't "continue" in this group, because we need to set
	       both build and develop cases correctly. */
	    if (side->tech[u] < u_tech_to_own(u)
		|| side->tech[u] < u_tech_to_build(u))
	      side->canbuild[u] = FALSE;
	    if (side->tech[u] >= u_tech_max(u))
	      side->candevelop[u] = FALSE;
	}
	/* Consider the effect of advances on ability to build/develop. */
	if (numatypes > 0) {
	    /* Test for an obsoleting advance. */
	    a = u_obsolete(u);
	    if (a != NONATYPE && has_advance(side, a)) {
		side->canbuild[u] = FALSE;
		side->candevelop[u] = FALSE;
		continue;
	    }
	    /* Then test for required advances. */
	    for_all_advance_types(a) {
		if (ua_needed_to_build(u, a) && !has_advance(side, a)) {
		    side->canbuild[u] = FALSE;
		    side->candevelop[u] = FALSE;
		    break;
		}
	    }
	}
    }
}

void
update_cancarry_vector(Side *side)
{
    Unit *unit;
    int u;

    for_all_unit_types(u) {
    	side->cancarry[u] = FALSE;
    	for_all_side_units(side, unit) {
	    if (mobile(unit->type)
		&& could_carry(unit->type, u)) {
		side->cancarry[u] = TRUE;
		break;
	    }
    	}
    }
}

static void
take_all_from_treasury(Unit *unit)
{
    int m, amount;
	
    /* Borrow materials from treasury if it exists. */
    for_all_material_types(m) {
	if (side_has_treasury(unit->side, m)
	    && um_takes_from_treasury(unit->type, m)) {
	    /* Don't exceed the unit's storage capacity unless there is no
	        capacity, in which case take the lot. */
	    if (um_storage_x(unit->type, m) <= 0)
	        amount = unit->side->treasury[m];
	    else
	        amount = min(unit->side->treasury[m], 
			     um_storage_x(unit->type, m) - unit->supply[m]);
	    unit->supply[m] += amount;
	    unit->side->treasury[m] -= amount; 			
	}
    }
}

static void
give_all_to_treasury(Unit *unit)
{
    int m, amount;
	
    /* Return all unused materials to treasury. */
    for_all_material_types(m) {
	if (side_has_treasury(unit->side, m)
	    && um_gives_to_treasury(unit->type, m)) {
	    amount = min(unit->supply[m], 
			 g_treasury_size() - unit->side->treasury[m]);
	    unit->side->treasury[m] += amount;
	    unit->supply[m] -= amount;		
	}
    }
}

/* Called from run_turn_start. */

void
run_people()
{
}

/* Main city emulating routine. Called by run_turn_start in run2.c. */

static int *rau_incrs;

void
run_advanced_units()
{
    Unit *unit, *unit2;
		
    for_all_units(unit) {
	Side	*side;
	int	a, m, x, y, oldsize;

	/* Only do this for cities. */
	if (!u_advanced(unit->type))
	  continue;

	/* Skip if this is an indep which cannot produce or consume materials. */
	if (indep(unit) &! g_indepside_has_economy())
	  continue;

	/* Units still under construction or off-area can't do anything. */
	if (!completed(unit) || !in_play(unit))
	  continue;

	/* Set size to 1 if not defined. */
	if (!unit->size)
	  unit->size = 1;
	oldsize = unit->size;

	/* Break here if no material types exist. */
	if (!nummtypes)
	  continue;

	/* Make sure landuse is optimize before collecting materials. */
	allocate_used_cells(unit);

	/* Zero the production cache. */
	for_all_material_types (m)
	    unit->production[m] = 0;
	/* Collect materials from cells controlled by the city. */
	for_all_cells_within_reach(unit, x, y) {
	    int m;
	    
	    if (!inside_area(x, y))
	  	continue;
	    if (user_at(x, y) == unit->id)
	      for_all_material_types(m)
		unit->production[m] += production_at(x, y, m);
	}
	/* Support for city improvements. */

	/* Note: an occupant or facility will typically have either an
	   additive or a multiplicative effect on material
	   production. However, it is possible for it to have
	   both. The default additive effect is zero, and the default
	   multiplicative effect is 1 (100 %). If the multiplicative
	   effect is set to zero, the occupant will prevent all
	   production of the specified material. A negative additive
	   effect means that the occupant is consuming some material
	   each turn. */

	for_all_material_types(m) {
	    
	    /* First compute additive effect of each occupant/facility. */
	    for_all_occupants(unit, unit2) {
	    	if (is_active(unit2)) {
	    	    unit->production[m] += um_occ_add_production(unit2->type, m);
	    	}
	    }
	    /* Then compute additive effect of each completed advance. */
	    if (unit->side) {
		for_all_advance_types(a)
		  if (unit->side->advance[a] == DONE)
		    unit->production[m] += am_adv_add_production(a, m);
	    }

	    /* Then factor in multiplicative effect (in %) of each
               occupant/facility. */
	    for_all_occupants(unit, unit2) {
	    	if (is_active(unit2)) {
		      unit->production[m] =
		        (unit->production[m] * um_occ_mult_production(unit2->type, m)) / 100;
	    	}
	    }
	    /* Then factor in multiplicative effect (in %) of each
               completed advance. */
	    if (unit->side) {
		for_all_advance_types(a)
		  if (unit->side->advance[a] == DONE)
		    unit->production[m] =
		      (unit->production[m] * am_adv_mult_production(a, m))
		      / 100;
	    }

	    {
		int m2, anyconversion = FALSE, tot = 0;

		for_all_material_types(m2) {
		    if (rau_incrs == NULL)
		      rau_incrs = (int *) xmalloc(nummtypes * sizeof(int));
		    rau_incrs[m2] = 0;
		    if (mm_conversion(m, m2) > 0
			&& unit->side->c_rates != NULL
			&& unit->side->c_rates[m2] > 0) {
			anyconversion = TRUE;
			tot += unit->side->c_rates[m2];
		    }
		}
		while (anyconversion && unit->production[m] > 0) {
		    for_all_material_types(m2) {
			if (mm_conversion(m, m2) > 0
			    && unit->side->c_rates != NULL
			    && unit->side->c_rates[m2] > 0) {
			    rau_incrs[m2] += unit->side->c_rates[m2];
			}
		    }
		    for_all_material_types(m2) {
			if (rau_incrs[m2] >= tot 
			    && unit->production[m] > 0) {
			    rau_incrs[m2] -= tot;
			    unit->production[m2] += 1;
			    unit->production[m] -= 1;
			}
		    }
		}
	    }
	}
	    /* Finally add production to unit supply. Check that we have room and 
	    transfer any excess materials to the treasury if possible. */
	for_all_material_types(m) {
	    int space, amount;

	    space = um_storage_x(unit->type, m) - unit->supply[m];
	    if (unit->production[m] <= space) {
		unit->supply[m] += unit->production[m];
	    } else {
	    	unit->supply[m] = um_storage_x(unit->type, m);
	    	amount = unit->production[m] - space;
		if (side_has_treasury(unit->side, m)
		    && um_gives_to_treasury(unit->type, m)) {
			amount = min(amount, g_treasury_size() - unit->side->treasury[m]);
			unit->side->treasury[m] += amount;
		}
	    }
	}
	/* Growth or starvation. */
	run_population(unit);
	/* Fixes crashes when the unit starved to death. */
	if (!in_play(unit))
	  continue;
	/* Optimize landuse again in case city size changed. */
	if (unit->size != oldsize) {
	    allocate_used_cells(unit);
	    /* Also reflect any changes of the city size text. */
	    for_all_sides(side) {
		if (side->see_all) {
		    update_cell_display(side, unit->x, unit->y, 
					UPDATE_ALWAYS);
		} else if (side_sees_unit(side, unit) 
		    || side_tracking_unit(side, unit)
		    || cover(side, unit->x, unit->y) > 0) {
			see_cell(side, unit->x, unit->y);
		}
	    }
	}
    }
}

/* Compute the total production of the given material at the given
   location within a city radius. */

int
production_at(int x, int y, int m)
{
    int prod = 0, t2 = NONTTYPE;

    prod = tm_prod_from_terrain(terrain_at(x, y), m);
    /* Add bonuses for all aux terrain types. */
    for_all_aux_terrain_types(t2) {
	if (aux_terrain_defined(t2)
	    && aux_terrain_at(x, y, t2)
	    && (tm_prod_from_terrain(t2, m) > 0)) {
	    prod += tm_prod_from_terrain(t2, m);
	}
    }
    return prod;
}

/* Starve or grow city population depending on food supply. */

void
run_population(Unit *unit)
{
    int starved = FALSE;
    int u = unit->type;
    int a, m, u2;
    Side *side = unit->side;

    /* If we have no people we are done. */
    if (!g_people())
        return;
    /* Loot the treasury. */
    take_all_from_treasury(unit);
    /* Consume necessary materials (food) in proportion to unit size. */
    for_all_material_types(m) {
	unit->supply[m] -= unit->size * um_consumption_per_size(u, m);
	if (unit->supply[m] < 0) {
	    /* Don't allow negative supply. */
	    unit->supply[m] = 0;
	    /* Trigger starvation. */
	    starved = TRUE;
	}
    }
    /* Return remaining materials to treasury. */
    give_all_to_treasury(unit);
    /* Starvation! (Size Loss) */
    if (starved && (unit->size > u_size_min(u))) {
	/* Shrink size by one. */
	unit->size -= 1;
	/* Run warning. */
	if (unit->size > 0) {
	    notify(side, "The people in %s are starving!",
		   unit_handle(side, unit));
	} else {
	    notify(side, "%s is no more. The people starved to death.", 
		   unit_handle(side, unit));
	    kill_unit(unit, H_UNIT_STARVED);
	}
	/* Done with handling starvation. */
	return;
    }
    /* Growth (Size Gain) */
    /* First check if we have reached our max size. */
    if (unit->size >= u_size_max(u)) 
      return;
    /* Then check if we have the required advances for the next size. */
    for_all_advance_types(a) {
	if (side->advance[a] != DONE 
	    && ua_size_limit_without_adv(u, a) <= unit->size) {
	    notify(side, "%s needs %s advance to grow in size.", 
		   unit_handle(side, unit), a_type_name(a));
	    return;
	}
    }
    /* Then check if we have the required facilities for the next size. */
    for_all_unit_types(u2) {
	if (uu_size_limit_without_occ(u, u2) <= unit->size) {
	    int	hasocc = FALSE;
	    Unit *unit2;

	    for_all_occupants(unit, unit2) {			
		if (is_active(unit2) && unit2->type == u2) {
		    hasocc = TRUE;
		    break;
		}
	    }
	    if (!hasocc) {
		notify(side, "%s needs a %s to grow in size.", 
		       unit_handle(side, unit), u_type_name(u2));
		return;
	    }
	}
    } 
    /* Loot the treasury again, in case storage space was limiting before 
	consumption. */
    take_all_from_treasury(unit);
    /* Now check if we have enough supplies left to grow in size. */
    /*! \todo Consider occupant effects on growth consumption. */
    for_all_material_types(m) {
	if (unit->supply[m] < unit->size * um_consumption_to_grow(u, m)) {
	    /* Return borrowed stuff to treasury. */
	    give_all_to_treasury(unit);     		
	    return;
	}
    }
    /* If we got this far we are ready for a size increase! */
    for_all_material_types(m) {
	int consump = unit->size * um_consumption_to_grow(u, m);
	/*! \todo Consider occupant effects on growth consumption. */
	/* Use up more supplies as required to prepare growth. */
	unit->supply[m] -= consump;
    }
    /* Then increase the size. */
    unit->size += 1;
    /* And brag about it. */
    notify(side, "%s prospers and reaches size %d.", 
	   unit_handle(side, unit), unit->size);
    /* Return borrowed stuff to treasury. */
    give_all_to_treasury(unit);     		
}

#if (0) // A leftover snippets worth considering.
void
run_construction(Unit *unit)
{			
    update_unit_acp_display(unit->side, unit, TRUE);
    /* Things have changed. */
    gamestatesafe = FALSE;
}
#endif

/* Do research until supply of its limiting material is gone. */

void
run_research(Unit *unit)
{
    int m, a, lim = PROPHI;

    /* Check if independent units are allowed to do research. */
    if (indep(unit) && !g_indepside_can_research())
	return; 
    /* First test if we are already done, and select new research in
       that case. */
    if (has_advance(unit->side, unit->curadvance) 
	|| unit->curadvance == NOADVANCE) {
	/* Set current advance to NOADVANCE to signal that we are idle. */
	unit->curadvance = NOADVANCE;
	/* Pick new research manually for human players. */
	if (side_has_display(unit->side)
	    && !side_has_ai(unit->side))
	  unit_research_dialog(unit);
	/* Pick new research automatically for AIs. */
	else
	  auto_pick_unit_research(unit);
    } 
    /* Only proceed if we now have a new advance. */
    a = unit->curadvance; 
    if (a == DONE)
      return;
    /* Loot the treasury. */
    take_all_from_treasury(unit);
    /* Loop until break. */
    while (1) {
	/* Find the material whose supply limits current research. */
	for_all_material_types(m) {
	    if (am_consumption_per_rp(a, m) > 0) {
		lim = min(lim, (int)((float) unit->supply[m] / 
			       am_consumption_per_rp(a, m)));
	    }
	}
	/* Don't do more research than needed to complete the advance though. */
	lim = min(lim,  a_rp(a) - unit->side->advance[a]);
	/* Then do research to the limit. */
	unit->side->advance[a] += lim;				
	/* Reduce all supplies accordingly. */		
	for_all_material_types(m) {
	    if (am_consumption_per_rp(a, m) > 0) {
		unit->supply[m] -= lim * am_consumption_per_rp(a, m);
	    }
	}
	/* Check if research has been completed. */
	if (unit->side->advance[a] >= a_rp(a)) {
	    Unit *unit2;

	    /* Set current advance to done. */
	    unit->side->advance[a] = DONE;
	    /* Update research and build vectors. */
	    update_canresearch_vector(unit->side);
	    update_canbuild_vector(unit->side);
	    /* Notify the units side about it. */
	    /* (TODO: Change "wise men in" to be a customizable string.) */
	    /* (TODO: Change "discover" to be a customizable string.) */
	    notify(unit->side, "Your wise men in %s discover %s!",
		   unit->name, a_type_name(a));
	    /* Pick new research for all involved units. */
	    if (side_has_display(unit->side) 
	          && !side_has_ai(unit->side)) {
		for_all_side_units(unit->side, unit2) {
		    if (!u_advanced(unit2->type))
			continue;
		    /* Reallocate units on auto that took part in this
		       advance. */
		    if (unit2->curadvance == a) { 
			auto_pick_unit_research(unit2);
		    /* Detect idle units on non-auto. */
		    } else if (unit2->curadvance == a) {
			unit2->curadvance = 0;
			unit_research_dialog(unit2);
		    }
    		}
	    } else {
		for_all_side_units(unit->side, unit2) {
		    if (!u_advanced(unit2->type))
		      continue;
		    if (unit2->curadvance == a) 
		      auto_pick_unit_research(unit2);
		}
	    }
	/* Research ended because we ran out of materials. */
	} else {
	    unit->researchdone = TRUE;
	    break;
    	}
    }
    /* Return all materials to treasury. */
    give_all_to_treasury(unit);
}

/* Free all used cells and redistribute them for optimal limiting
   material production (usually food) if the city belongs to an AI or
   is independent. If it belongs to a human player, only allocate new
   used cells automatically. Don't change old allocations that may
   differ from automatic ones intentionally.  */

void
allocate_used_cells(Unit *unit)
{
	int	x, y, a, i, m, mlim = 0, lim = 0;
	int u = unit->type;
	Unit *unit2;

	/* Set max used cells to unit size. */
	unit->maxcells = unit->size;
	if (u_use_own_cell(u))
	    ++(unit->maxcells);
	/* First compute additive effect of each occupant/facility. */
	for_all_occupants(unit, unit2) {
		if (is_active(unit2)) {
			unit->maxcells += uu_occ_add_maxcells(u, unit2->type);
		}
	}
	/* Then compute additive effect of completed advances. */
	for_all_advance_types(a) {
		if (has_advance(unit->side, a)) {
			unit->maxcells += ua_adv_add_maxcells(u, a);
		}
	}
	/* Then factor in multiplicative effect (in %) of each
	occupant/facility. */
	for_all_occupants(unit, unit2) {
		if (is_active(unit2)) {
			unit->maxcells = (unit->maxcells * uu_occ_mult_maxcells(u, unit2->type)) / 100;
		}
	}
	/* Then factor in multiplicative effect (in %) of completed advances. */
	for_all_advance_types(a) {
		if (has_advance(unit->side, a)) {
			unit->maxcells = (unit->maxcells * ua_adv_mult_maxcells(u, a)) / 100;
		}
	}
	/* Check the number of used cells. */
	unit->usedcells = 0;
	for_all_cells_within_reach(unit, x, y) {
		if (!inside_area(x, y))
	    	    continue;
		if (user_at(x, y) == unit->id)
		    unit->usedcells += 1;
	}
	/* Find the limiting material for population maintenance. */
	for_all_material_types(m) {
		if (um_consumption_per_size(u, m) > lim) {
			lim = um_consumption_per_size(u, m);
			mlim = m;
		}
	}

	/* Free all of this unit's doubtful cells within reach. */
	for_all_cells_within_reach(unit, x, y) {
		/* Skip if not inside area */
		if (!inside_area(x, y))
		    continue;
		/* Free cells used by the unit itself if it is independent or
		played by an AI. */
		if (user_at(x, y) == unit->id
		    && (indep(unit) || side_has_ai(unit->side))) {
			set_user_at(x, y, NOUSER);
			unit->usedcells -= 1;
		} else if (user_at(x, y) != NOUSER && unit_at(x, y) != NULL) {
			/* Also free cells used by enemy side if we have a unit in
			the cell. */
			for_all_stack(x, y, unit2) {
				if (unit2->side && unit2->side == unit->side) {
					kick_out_enemy_users(unit->side, x, y);
					break;
				}
			}
		}
	}
	/* May want to use own cell automatically. */
	if (u_use_own_cell(u) && user_at(unit->x, unit->y) == NOUSER) {
		set_user_at(unit->x, unit->y, unit->id);
		unit->usedcells += 1;
	}
	/* Then select new cells up to unit size */
	for (i = unit->usedcells + 1; i <= unit->maxcells; i++) {
		int prod, tmpprod = 0, tmpx = unit->x, tmpy = unit->y;

		for_all_cells_within_reach(unit, x, y) {
			/* Skip if not inside area */
			if (!inside_area(x, y))
			    continue;
			/* Skip if cell already is in use */
			if (user_at(x, y) != NOUSER)
			    continue;
			/* Skip if cell is invisible to unit */
			if (!terrain_visible(unit->side, x, y))
			    continue;
			/* Skip if untrusted side has a unit in the cell */
			if (unit_at(x, y) != NULL) {
				int enemy = FALSE;
				for_all_stack(x, y, unit2) {
					if (!trusted_side(unit->side, unit2->side)) {
						enemy = TRUE;
						break;
					}
				}
				if (enemy)
				    continue;
			}
			/* Make this tmp used cell if it produces more limiting
			material than previous tmp used cell or if the previous
			tmp cell is already in use. */
			prod = production_at(x, y, mlim);
			if ((prod > tmpprod) || (user_at(tmpx, tmpy) != NOUSER)) {
				tmpprod = prod;
				tmpx = x;  tmpy = y;
			}
		}
		/* Make tmp used cell permanently used if it is unused. */
		if (user_at(tmpx, tmpy) == NOUSER) {
			set_user_at(tmpx, tmpy, unit->id);
			unit->usedcells += 1;
		}
	}
}

/* Can't use a cell occupied by an enemy unit. */

void
kick_out_enemy_users(Side *side, int x, int y)
{
    Unit *unit;

    /* Nothing to do if no user layer. */
    if (!user_defined())
      return;
    /* Return if nobody is using the cell. */
    if (user_at(x, y) == NOUSER)
      return;
    /* Else find the user. */
    unit = find_unit(user_at(x, y));
    /* Return if the user belongs to the same or a friendly side. */
    if (in_play(unit) && trusted_side(unit->side, side))
      return;

    /* Mark the cell as unused. */
    set_user_at(x, y, NOUSER);
    /* Reduce the former user's cells. */
    if (in_play(unit))
      unit->usedcells -= 1;	
}

/* Now picks new advance at random rather than by number. */

static int *apr_type;

void
auto_pick_unit_research(Unit *unit)
{
    int a;
    int numtypes = 0;

    if (apr_type == NULL)
      apr_type = (int *) xmalloc(numutypes * sizeof(int));

    /* Get researchable advance types. */
    for_all_advance_types(a) {
	if (side_can_research(unit->side, a)) {
	    /* Add to list of researchable advances. */
	    apr_type[numtypes++] = a;
	}
    }
    if (numtypes)
      /* Pick one of them at random. */
      unit->curadvance = apr_type[xrandom(numtypes)];
    else
      unit->curadvance = NOADVANCE;
}

/* Try to pick a good advance to research. */

void
auto_pick_side_research(Side *side)
{
    ai_pick_side_research(side);
}

#if (0)
static int *apnbt_types;

int
auto_pick_new_build_task(Unit *unit)
{
    int enemy_city_near = FALSE;
    int x, y, x1, y1, u; 
    int city_has_shore = FALSE;
    int numtypes = 0;
    Side *side = unit->side;
    Unit *unit2;

    if (apnbt_types == NULL)
      apnbt_types = (int *) xmalloc(numutypes * sizeof(int));
    /* Zero the apnbt vector! */
    for_all_unit_types(u) {
	apnbt_types[u] = 0;
    }
    /* Reevaluate the current plan every time. */
    auto_pick_new_plan(unit);
    /* Check if the unit borders on a water cell. */ 	
    for_all_cells_within_range(unit->x, unit->y, 1, x, y) {
    	if (!inside_area(x, y))
    	    continue;
	if (t_liquid(terrain_at(x, y))) {
	    /* Also check if that water cell borders on another water cell.
	       (we don't want to build battleships in the city pond) */
	    for_all_cells_within_range(x, y, 1, x1, y1) {
	    	if (!inside_area(x1, y1))
	    	    continue;
		if (t_liquid(terrain_at(x1, y1))
		    /* Don't count the first water cell. */
		    && (x != x1 || y != y1)) {
		    city_has_shore = TRUE;
		    break;
		}
	    }
	    if (city_has_shore)
	      break;				
	}
    }		
    /* Check if we have an enemy city within our tactical range. */
    for_all_units(unit2) {
	if (u_advanced(unit2->type)
	    && distance(unit->x, unit->y, unit2->x, unit2->y)
	    <= u_ai_enemy_alert_range(unit->type)
	    && enemy_side(side, unit2->side)
	    /* Only count cities that we can actually see. */
	    && side_sees_image(side, unit2)) {
	    enemy_city_near = TRUE;
	    break;
	}
    }
    /* Calculate unit values. */	
    for_all_unit_types(u) {
	if (side_can_build(side, u)
	    && could_create(unit->type, u)) {
	    /* Don't build immobile units for which there is no room. */
	    if (!mobile(u) 
	        && !type_can_occupy(u, unit)) {
		continue;
	    }
	    /* Don't build ships (naval-only movers) 
		if we don't have a shore. */
	   if (u_naval_mobile(u)
	       && !city_has_shore
	       && !u_air_mobile(u)
	       && !u_ground_mobile(u)) {
		continue;
	    }
	    switch (unit->plan->type) {
	      case PLAN_IMPROVING:			
		apnbt_types[u] = u_facility_worth(u);
		break;					
	      case PLAN_EXPLORATORY:
		apnbt_types[u] = u_ai_explorer_worth(u);
		break;					
	      case PLAN_OFFENSIVE:
		if (enemy_city_near) {
		    /* Build 50% siege units. */
		    if (probability(50))
		    	apnbt_types[u] = u_siege_worth(u);
		    else	
		    	apnbt_types[u] = u_siege_worth(u);
		} else	{
		    /* Build only attack units. */
		    apnbt_types[u] = u_offensive_worth(u);
		}
		break;					
	      case PLAN_DEFENSIVE:
		apnbt_types[u] = occ_can_defend_transport(u, unit->type) 
		  * u_defensive_worth(u);
		break;					
	      case PLAN_NONE:
	      case PLAN_PASSIVE:
	      case NUMPLANTYPES:
		apnbt_types[u] = 0;
		break;					
	    }
	}
	if (apnbt_types[u] <= 0)
	  apnbt_types[u] = 0;
	else ++numtypes;
    }			
    /* Try again with random choice if we found nothing to build. */
    if (numtypes == 0) {
	for_all_unit_types(u) {
	    if (side_can_build(side, u)
		&& could_create(unit->type, u)) {
	         /* Don't build immobile units for which there is no room. */
	         if (!mobile(u) 
	             && !type_can_occupy(u, unit)) {
		     continue;
	         }
	         /* Don't build ships (naval-only movers) 
		    if we don't have a shore. */
	         if (u_naval_mobile(u)
	            && !city_has_shore
	            && !u_air_mobile(u)
	            && !u_ground_mobile(u)) {
		     continue;
	         }
		apnbt_types[u] = u_random_worth(u);
		if (apnbt_types[u] <= 0)
		  apnbt_types[u] = 0;
		else ++numtypes;
	    }
	}
    }
    if (numtypes > 0) {
	/* Use prefs weight to pick unit type to build. */
	u = select_by_weight(apnbt_types, numutypes);
	if (is_unit_type(u))
	  set_construct_task(unit, u, 1, -1, unit->x, unit->y); 
	else
	  u = NONUTYPE;
    } else
      u = NONUTYPE;
    if (u == NONUTYPE) {
    	set_unit_reserve(side, unit, TRUE, FALSE);
    	notify(side, "%s unable to pick build task, waiting one turn.", 
	       unit_handle(side, unit));
    }
    return u;	
}

/* This is a rudimentary AI for advanced units. Lots of room for
   improvement! */

void
auto_pick_new_plan(Unit *unit)
{
    int my_advanced = 0, my_colonizers = 0, my_total = 0;
    int enemy_advanced = 0, enemy_colonizers = 0, enemy_total = 0;
    int defenders = 0, facilities = 0, mobiles = 0, invisible_cells = 0;
    int u = unit->type;
    int x, y, u2;
    Unit *unit2;
			
    /* Init plan if necessary. */
    if (!unit->plan)
      init_unit_plan(unit);	
    /* First count our facilities, defenders and mobile types. */
    for_all_occs_with_occs(unit, unit2) {
	if (u_facility(unit2->type))
	  facilities += 1;
	if (is_active(unit2)
	    && occ_can_defend_transport(unit2->type, unit->type)) 
	  defenders += 1;
	if (mobile(unit2->type))
	  mobiles += 1;
    }		
    /* Always build defenders if we are undefended. */
    if (!defended_by_occupants(unit)
	&& can_build_defenders(unit->side, unit->type)) {
	set_unit_plan_type(unit->side, unit, PLAN_DEFENSIVE);
	return;
    }	
    /* Check out the tactical neighbourhood. */
    for_all_cells_within_range(unit->x, unit->y, 
			       u_ai_tactical_range(unit->type), x, y) {
	if (!inside_area(x, y))
	  continue;
	if (!terrain_visible(unit->side, x, y))
	  invisible_cells +=1;
	unit2 = NULL;
	/* Important to count also occupants here. */
	for_all_stack_with_occs(x, y, unit2) {
	    /* Only count real units. */
	    if (!is_active(unit2))
	      continue;
	    /* Only count units that we can see. */
	    if (!side_sees_image(unit->side, unit2))
	      continue;
	    u2 = unit2->type;
	    /* First count own units. */
	    if (unit->side == unit2->side) {
		if (u_advanced(u2))
		  my_advanced += 1;
		else if (u_colonizer_worth(u2))
		  my_colonizers += 1;
		my_total += 1;
		/* Then count hostile units. */
	    } else if (enemy_side(unit->side, unit2->side)){
		if (u_advanced(u2))
		  enemy_advanced += 1;
		else if (u_colonizer_worth(u2))
		  enemy_colonizers += 1;
		enemy_total += 1;
	    }		
	}
    }
    if (enemy_total > 0
	&& can_build_attackers(unit->side, unit->type)) {
	/* Always build attackers if any enemy unit is near. */
	set_unit_plan_type(unit->side, unit, PLAN_OFFENSIVE);
    } else if (invisible_cells
	       && can_build_explorers(unit->side, unit->type)) {
	/* Then build explorers if we have any invisible cells nearby. */
	set_unit_plan_type(unit->side, unit, PLAN_EXPLORATORY);
    } else if (flip_coin() && facilities < u_facility_total_max(u)
	       && can_build_facilities(unit->side, unit->type)) {
	/* Build 50% facilities if there is room left. */
	set_unit_plan_type(unit->side, unit, PLAN_IMPROVING);
    } else if (can_build_attackers(unit->side, unit->type)) {
	/* Else build offensive units. */	
	set_unit_plan_type(unit->side, unit, PLAN_OFFENSIVE);
    } else {
	set_unit_plan_type(unit->side, unit, PLAN_PASSIVE);
	set_unit_reserve(unit->side, unit, TRUE, FALSE);
    }
}
#endif

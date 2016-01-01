/* The main simulation-running code in Xconq, turn start and end calcs.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This code calculates what happens at the beginning and end of each turn. */

#include "conq.h"
#include "kernel.h"

using namespace Xconq;

/* Auto Repair */
extern int check_repair_action(Unit *actor, Unit *repairer, Unit *repairee);
extern int do_repair_action(Unit *actor, Unit *repairer, Unit *repairee);

extern int stop_apply;

static void vary_winds(void);

extern int gamestatesafe;

static int visible_to(Unit *unit, Unit *unit2);
static void add_new_sides_to_game(void);
static void run_tech_leakage(void);
static void cache_init_tech_levels(void);
static void run_tooling_attrition(void);
static void run_cp_attrition(void);
static void reset_all_reserves(void);
static void save_checkpoint(int curturn);
static void compute_moves(void);
static void finish_movement(void);
#if 0
static void test_agreements(void);
#endif
static void compute_sun(void);
static void run_sun(void);
static void compute_season(void);
static void run_environment(void);
static void mix_winds(void);
static void all_see_cell_weather(int x, int y);
static void run_spies(void);
static void run_accidents(void);
static void run_attrition(void);
static void run_revolts(void);
static void unit_revolt(Unit *unit, int force);
static void run_surrenders(void);
static void unit_surrender(Unit *unit);
static void maybe_surrender_to(Unit *unit, Unit *unit2);
static void run_self_builds(void);
static void run_environment_effects(void);
static void damage_unit_with_temperature(Unit *unit, int n);
static void run_people_side_changes(void);
static void run_appearances(void);
static void run_disappearances(void);

//! Handle auto-repair for all units in game.
static void run_auto_repair(void);
//! Auto-repair from within given transport.
static void auto_repair_from_in(Unit *transport);
//! Auto-repair from given location.
static void auto_repair_from_here(int x, int y);

static void run_morale_recovery(void);
static void run_auto_change_types(void);
static int season_effect(int u);
static void run_unit_fates(void);
static void run_detonation_accidents(void);
static void run_people_limits(void);
static void spy_on_location(int x, int y);

/* Set this flag to suppress normal cell updates. Should only be set if
all maps will be redrawn before any units can act. */

int suppress_update_cell_display = FALSE;

/* The number of the current turn within a year. */

int curyearpart = -1;

/* The season name for the current turn. */

const char *curseasonname;

/* The full date/season/day-night string that interfaces should display. */

char *curdatestr;

/* Do we want saves while playing? */

int want_checkpoints = FALSE;

/* How often to do saves while playing. */

int checkpoint_interval = 0;

static char *last_checkpoint_filename;

/* The number of new sides that have been requested to be added. */

int new_sides_requested;

char **players_requested;

/* True whenever the game has both day and night. */

int daynight = FALSE;

/* The location of the sun, as a position relative to the area.  The
   actual values may be far outside the area. */

int sunx, suny;

/* The sun's previous location. */

int lastsunx = -1, lastsuny = -1;

/* If the area is uniformly lighted, this is that value. */

int area_lighting = -1;

/* Flags indicating whether various sets of calculations need to be done. */

/* Typically we set to -1 to indicate that the value is uncomputed, then detect
   when the value is first needed and compute a true/false value, which we then
   use thereafter. */

short any_tooling_attrition = -1;

short any_cp_attrition = -1;

short any_self_builds = -1;

short any_environment_effects = -1;

short any_appearances = -1;

short any_disappearances = -1;

short any_people_side_changes = -1;

short *any_people_surrenders = NULL;

short any_morale_recovery = -1;

short any_tech_leakage = -1;

short any_detonation_accidents = -1;

short any_lost_vanish = -1;

short any_lost_wreck = -1;

short any_lost_surrender = -1;

short any_lost_revolt = -1;

short any_people_max = -1;

short any_spying = -1;

short any_annual_temp_change = -1;

short any_accidents = -1;

short any_attrition = -1;

short any_revolts = -1;

short any_surrenders = -1;

namespace Xconq {
    short any_hp_recovery = FALSE;
    short any_auto_repair = FALSE;
    short *cv__could_be_auto_repaired = NULL;
    short *cv__auto_repaired_range_max = NULL;
}

short any_auto_change_types = FALSE;

static short *surrender_ranges;

static char *any_lost_surrenders;

/* Do everything that would happen before movement in a turn. */

void
run_turn_start(void)
{
    int curturn;
    time_t turncalcstart, turncalcend;
    Side *side;

    /* Increment the turn number. */
    curturn = g_turn();
    ++curturn;
    set_g_turn(curturn);
    /* Add any sides that were requested to be added during the
       previous turn. */
    add_new_sides_to_game();
    /* Warn players if the game is nearly over. */
    if (curturn >= g_last_turn()) {
	if (g_extra_turn() > 0) {
	    notify_all("This may be the last turn in the game!");
	} else {
	    notify_all("This is the last turn in the game!");
	}
    }
    time(&turncalcstart);
    update_all_progress_displays("turn start calcs", -1);
    compute_season();
    Dprintf("##### TURN %d (%s) #####\n", curturn, curdatestr);
    /* Mark the change of turn even if only dumping AI and planning info. */
    if (!Debug)
      DMprintf("##### TURN %d (%s) #####\n", curturn, curdatestr);
    for_all_sides(side) {
	side->finishedturn = FALSE;
	update_turn_display(side, TRUE);
	if (realtime_game()) {
	    update_clock_display(side, TRUE);
	}
	/* Update legends and their positions as new terrain comes into view. */
	place_legends(side);
	/* Needed to update legends in games that have them, but probably a 
	good idea to redraw the maps each turn anyway. */
	update_area_display(side);
    }
    /* Do a variety of each-turn backdrop activities. */
    run_sun();
    run_environment();
    run_ui_idler(); /* Take a breather for UI events. */
    /* Advanced unit support. */
    run_advanced_units();
    /* Optionally run economy and supply code. */
    if (g_people())
      run_people();
    run_ui_idler(); /* Take a breather for UI events. */
    if (g_economy())
      run_economy();
    run_ui_idler(); /* Take a breather for UI events. */
    if (g_supply())
      run_supply();
    run_ui_idler(); /* Take a breather for UI events. */
    run_auto_repair();
    run_self_builds();
    run_morale_recovery();
    run_ui_idler(); /* Take a breather for UI events. */
    run_auto_change_types();
    run_appearances();
    run_accidents();
    run_attrition();
    run_revolts();
    run_surrenders();
    run_unit_fates();
    run_detonation_accidents();
    run_ui_idler(); /* Take a breather for UI events. */
    sort_units(TRUE);
    compute_moves();
    run_spies();
    run_tech_leakage();
    run_tooling_attrition();
    run_cp_attrition();
    cache_init_tech_levels();
    reset_all_reserves();
    gamestatesafe = FALSE;
    save_checkpoint(curturn);
    time(&turncalcend);
    Dprintf("%d seconds to calc at turn start\n",
	    idifftime(turncalcend, turncalcstart));
}

/* Do computations to start the first turn of a restored game. */

void
run_restored_turn_start(void)
{
    Side *side;

    compute_season();
    Dprintf("##### TURN %d (%s) #####\n", g_turn(), curdatestr);
    for_all_sides(side) {
	update_turn_display(side, TRUE);
	if (realtime_game()) {
	    update_clock_display(side, TRUE);
	}
    }
    compute_sun();
    sort_units(TRUE);
    /* We're done with restore-specific tweaks, turn the flag off. */
    midturnrestore = FALSE;
}

/* Do everything associated with the end of a turn. */

void
run_turn_end(void)
{
    update_all_progress_displays("turn end calcs", -1);
    finish_movement();
    run_all_consumption();
    run_environment_effects();
    run_people_side_changes();
    /* This should come after other people-related computations,
       since this only constrains generic overcrowding. */
    run_people_limits();
    flush_dead_units();
    flush_stale_views();
    check_post_turn_scores();
#if 0
    test_agreements();
#endif
    run_disappearances();
#ifdef DEBUGGING
    if (Debug)
      report_malloc();
#endif /* DEBUGGING */
    /* See if we've hit a preset end to the game. */
    if (g_turn() >= g_last_turn() && !probability(g_extra_turn())) {
	notify_all("This is the end of the last turn in the game!");
	end_the_game();
    }
}

/* Interfaces should call this to have another side added to the current
   game.  The actual addition happens during turn setup, so as not to
   risk confusing list traversals or AI calculations. */

void
request_additional_side(const char *playerspec)
{
    if (numsides + new_sides_requested + 1 <= g_sides_max()) {
	if (players_requested == NULL)
	  players_requested = 
	    (char **) xmalloc((MAXSIDES + 1) * sizeof(char *));
	if (empty_string(playerspec))
	  playerspec = ",ai";
	players_requested[new_sides_requested++] = copy_string(playerspec);
	notify_all(
"Will add a new side (player \"%s\") at the start of the next turn",
		   playerspec);
    } else {
	notify_all(
"Additional side requested (player \"%s\"), but not possible to add",
		   playerspec);
    }
}

/* Add the requested number of new sides into an ongoing game. */

static void
add_new_sides_to_game(void)
{
    int i;
    Side *side, *side2;
    Player *player;
    Unit *unit;
    extern int need_ai_for_new_side;

    if (new_sides_requested > 0) {
	for (i = 0; i < new_sides_requested; ++i) {
	    if (numsides >= g_sides_max())
	      break;
	    /* Grow side-referencing objects. */
	    for_all_units(unit) {
		if (unit->opinions != NULL) {
		    init_unit_opinions(unit, numsides + 1);
		}
	    }
	    side = make_up_a_side();
	    player = add_player();
	    parse_player_spec(player, players_requested[i]);
	    player->side = side;
	    side->player = player;
	    /* Set the player's advantage to be the side's advantage, if not
	       already set. */
	    if (player->advantage == 0) {
		player->advantage = side->advantage;
	    }
	    run_synth_methods();
	    init_doctrine(side);
	    init_self_unit(side);
	    if (g_use_side_priority()) {
		int maxpri = 0;

		for_all_sides(side2) {
		    if (side2->priority > maxpri)
		      maxpri = side2->priority;
		}
		side->priority = maxpri + 1;
		/* If the indepside's priority was set automatically to be
		   one up from regular sides', bump it up to be past the new
		   side's priority also. */
		if (indepside->priority == side->priority)
		  ++(indepside->priority);
	    }
	    /* Count all of the new side's units as initial gain. */
	    for_all_side_units(side, unit) {
		count_gain(side, unit->type, initial_gain);
	    }
	    /* Maybe record that we need to create an AI for the new
               side later. */
	    if (side_wants_ai(side)) {
		need_ai_for_new_side = TRUE;
	    }
	    for_all_sides(side2) {
		/* Give all other AIs a chance to rework internal data
                   structures. */
		/* (should move to run_local_ai?) */
		if (side_has_ai(side2) && side2 != side) {
		    ai_react_to_new_side(side2, side);
		}
		/* Add the new side to displays. */
		update_side_display(side2, side, TRUE);
		notify(side2, "A new side %s (played by %s) is in the game.",
		       side_desig(side), player_desig(player));
	    }
	}
	if (i > 0) {
	    /* Recalculate all view info for all sides; simpler than
	       trying to figure out what's really changed. */
	    really_reset_coverage();
	    reset_all_views();
	    compute_all_feature_centroids();
	    /* Redraw everything that's displayed. */
	    update_everything();
	}
	if (i < new_sides_requested) {
	    notify_all("Cannot create %d of the requested new sides",
		       new_sides_requested - i);
	}
    }
    /* We've handled everything we're going to, reset the counter. */
    new_sides_requested = 0;
}

/* Parse the syntax "[username][,ai][/config][@display][+advantage]".  The
   input string spec may be modified in place. */
/* (should move to side.c?) */
void
parse_player_spec(Player *player, char *spec)
{
    int commapos, slashpos, atpos, pluspos;

    if (spec != NULL && strcmp(spec, "*") != 0) {
	/* Extract (destructively) a trailing advantage specification. */
	pluspos = iindex('+', spec);
	if (pluspos >= 0) {
	    player->advantage = max(1, atoi(&(spec[pluspos + 1])));
	    spec[pluspos] = '\0';
	}
	/* Extract a trailing displayname if given. */
	atpos = iindex('@', spec);
	if (atpos >= 0) {
	    player->displayname = copy_string(spec + atpos + 1);
	    spec[atpos] = '\0';
	}
	/* Extract a trailing configuration name if given. */
	slashpos = iindex('/', spec);
	if (slashpos >= 0) {
	    player->configname = copy_string(spec + slashpos + 1);
	    spec[slashpos] = '\0';
	}
	/* Extract a trailing AI type if given. */
	commapos = iindex(',', spec);
	if (commapos >= 0) {
	    player->aitypename = copy_string(spec + commapos + 1);
	    spec[commapos] = '\0';
	}
	/* Just a plain old string left. */
	if (strlen(spec) > 0) {
	    if (atpos >= 0) {
		/* Display given separately, so this is a name. */
		player->name = copy_string(spec);
	    } else {
		player->displayname = copy_string(spec);
	    }
	}
    }
    canonicalize_player(player);
}

/* Compute the leakage of technology from one side to another. */

static void
run_tech_leakage(void)
{
    int u;
    Side *side, *side2;

    if (any_tech_leakage < 0) {
	any_tech_leakage = FALSE;
	for_all_unit_types(u) {
	    if (u_tech_leakage(u) > 0) {
		any_tech_leakage = TRUE;
		break;
	    }
	}
	Dprintf("Any tech leakage: %d\n", any_tech_leakage);
    }
    if (!any_tech_leakage)
      return;
    Dprintf("Running tech leakage\n");
    for_all_sides(side) {
	for_all_sides(side2) {
	    if (side != side2 /* (should) and some contact between sides */) {
		for_all_unit_types(u) {
		    if (side->tech[u] < side2->tech[u]
			&& u_tech_leakage(u) > 0) {
			side->tech[u] += prob_fraction(u_tech_leakage(u));
		    }
		}
	    }
	}
    }
}

/* Remember each side's tech levels before it does any develop actions
   during the turn.  This can be used to keep tech level from going up too
   fast if the player has lots of units doing development. */

static void
cache_init_tech_levels(void)
{
    int u;
    Side *side;

    if (using_tech_levels()) {
	for_all_sides(side) {
	    for_all_unit_types(u) {
		side->inittech[u] = side->tech[u];
	    }
	}
    }
}

/* Reduce some units' construction tooling randomly. */

static void
run_tooling_attrition(void)
{
    int u, u2, att;
    Unit *unit;

    /* Test whether tooling attrition is ever possible. */
    if (any_tooling_attrition < 0) {
	any_tooling_attrition = FALSE;
	for_all_unit_types(u) {
	    for_all_unit_types(u2) {
		if (uu_tp_attrition(u, u2) > 0) {
		    any_tooling_attrition = TRUE;
		    break;
		}
	    }
	    if (any_tooling_attrition)
	      break;
	}
	Dprintf("Any tooling attrition: %d\n", any_tooling_attrition);
    }
    if (!any_tooling_attrition)
      return;
    for_all_units(unit) {
	if (is_active(unit) && unit->tooling != NULL) {
	    for_all_unit_types(u2) {
		att = uu_tp_attrition(unit->type, u2);
		if (att > 0) {
		    unit->tooling[u2] -= prob_fraction(att);
		}
		if (unit->tooling[u2] < 0) unit->tooling[u2] = 0;
	    }
	}
    }
}

/* Reduce some incomplete units' cp. */

static void
run_cp_attrition(void)
{
    int u, att;
    Unit *unit;

    /* Test whether tooling attrition is ever possible. */
    if (any_cp_attrition < 0) {
	any_cp_attrition = FALSE;
	for_all_unit_types(u) {
	    if (u_cp_attrition(u) > 0) {
		any_cp_attrition = TRUE;
		break;
	    }
	}
	Dprintf("Any cp attrition: %d\n", any_cp_attrition);
    }
    if (!any_cp_attrition)
      return;
    Dprintf("Running cp attrition\n");
    for_all_units(unit) {
	if (alive(unit) && !completed(unit)) {
	    att = u_cp_attrition(unit->type);
	    if (att > 0 && xrandom(10000) < att) {
		--(unit->cp);
		if (unit->cp <= 0) {
		    kill_unit(unit, -1 /* for now */);
		}
	    }
	}
    }
}

/* At the beginning of a turn, there are no units in reserve. */

static void
reset_all_reserves(void)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->plan != NULL) {
	    unit->plan->reserve = FALSE;
	    unit->plan->delayed = FALSE;
	}
	unit->researchdone = FALSE;
    }
}

/* Record a checkpoint, which is just a saved game, saved
   automatically every n turns.  The code is slightly complicated
   by our desire to ensure successful saving before removing
   the previous checkpoint.  Note also that interfaces are not
   required to incorporate the turn number into the result of
   checkpoint_filename. */

static void
save_checkpoint(int curturn)
{
    const char *checkfname, *checkname;

    if (want_checkpoints
    	&& checkpoint_interval > 0 
    	&& curturn % checkpoint_interval == 0) {
	checkfname = checkpoint_filename(curturn);
	checkname = copy_string(find_name(checkfname));
	save_game(checkname);
	if (last_checkpoint_filename == NULL)
	  last_checkpoint_filename = (char *)xmalloc(BUFSIZE);
	if (strcmp(checkfname, last_checkpoint_filename) != 0)
	  remove_file(last_checkpoint_filename);
	strcpy(last_checkpoint_filename, checkfname);
	Dprintf("Saved checkpoint %s\n", checkfname);
    }
}

/* Compute moves and actions for all the units at once, put everybody that
   can do anything into a list. */

static void
compute_moves(void)
{
    int curturn = g_turn();
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	side->numacting = 0;
	side->numfinished = 0;
	for_all_side_units(side, unit) {
	    /* If (can_be_actor(unit) XOR unit->act), then reconcile the
               difference. These situations occur because some units
               are only actors under special circumstances. */
            if ((can_be_actor(unit) && !(unit->act))
                || (!can_be_actor(unit) && unit->act)) {
		init_unit_actorstate(unit, TRUE);
		/* Attempt to preserve existing plan. */
		if (!(unit->plan))
		  init_unit_plan(unit);
	    }
	    if (unit->act) {
		/* Unit acp is set to -1 to indicate uninitialization,
		   but acp is computed by adding to the previous acp,
		   so when starting a new game (as opposed to
		   restoring an old one), acp should be inited to
		   zero.  (This could maybe be done better.) */
		if (curturn == 1)
		  unit->act->acp = 0;
		set_unit_acp_for_turn(unit);
		update_unit_acp_display(side, unit, FALSE);
	    }
	}
    }
}

/* Compute the maximum amount of new ACP an unit can gain in a turn, all things 
   considered, if located in its present cell. */

int
new_acp_for_turn(Unit *unit)
{
    assert_error(unit, "Tried to access a null unit.");
    return new_acp_for_turn_if_at(unit, unit->x, unit->y);
}

/* Compute the maximum amount of new ACP an unit can gain in a turn, all things 
   considered, if located in any legitimate cell. */

int
new_acp_for_turn_if_at(Unit *unit, int x, int y)
{
    int u = NONUTYPE, t = NONTTYPE, acp = -1;
    int err = 0, mor = 0, moreff = 0, temp = 0, tempeff = 0;
    Unit *occ = NULL;

    /* Sanity checks. */
    assert_error(unit, "Tried to access a null unit.");
    /* Units still under construction or off-area can't do anything. */
    if (!completed(unit) || !inside_area(x, y)) {
	return 0;
    }
    u = unit->type;
    /* First compute how many action points are available. */
    /* Start with basic acp, normal or damaged as appropriate. */
    if (u_acp_damage_effect(u) != lispnil
	&& unit->hp < u_hp_max(u)) {
	acp = damaged_acp(unit, u_acp_damage_effect(u));
    } else {
	acp = u_acp(u);
    }
    /** Additive/subtractive effects. **/
    /* Occupants */
    for_all_occupants(unit, occ) {
	if (is_active(occ))
	  acp += uu_occ_adds_acp(u, occ->type);
    }
    /* Night Time */
    if (night_at(x, y)) {
    	/* (should account for unit being on a road at night, etc) */
	t = terrain_at(x, y);
    	acp += ut_night_adds_acp(u, t);
    }
    /* Shortcircuit the multiplicative effects if they would be useless. */
    if (acp <= 0)
      return 0;
    /** Multiplicative effects. **/
    /* Occupants */
    for_all_occupants(unit, occ) {
	if (is_active(occ))
	  acp = (acp * uu_occ_multiplies_acp(u, occ->type)) / 100;
    }
    /* Night Time */
    if (night_at(x, y)) {
    	/* (should account for unit being on a road at night, etc) */
	t = terrain_at(x, y);
    	acp = (acp * ut_night_multiplies_acp(u, t)) / 100;
    }
    /* Morale */
    if (u_acp_morale_effect(u) != lispnil) {
	mor = unit->morale;
	err = interpolate_in_list(mor, u_acp_morale_effect(u), &moreff);
	if (err == 0) {
	    acp = (acp * moreff) / 100;
	} else {
	    run_warning("Morale %d out of bounds for acp-morale-effect", mor);
	}
    }
    /* (TODO: Should add an ACP experience effect.) */
    /* Temperature */
    if (temperatures_defined() && u_acp_temp_effect(u) != lispnil) {
	temp = temperature_at(x, y);
	err = interpolate_in_list(temp, u_acp_temp_effect(u), &tempeff);
	if (err == 0) {
	    acp = (acp * tempeff) / 100;
	} else {
	    run_warning(
"Temperature %d out of bounds for acp-temperature-effect", temp);
	}
    }
    /* Season */
    if (u_acp_season_effect(u) != lispnil) {
    	acp = (acp * season_effect(u)) / 100;
    }
    /* Clip to upper and lower acp-per-turn limits. */
    acp = max(acp, u_acp_turn_min(u));
    if (u_acp_turn_max(u) >= 0)
      acp = min(acp, u_acp_turn_max(u));
    return acp;
}

/* Compute the maximum amount of ACP an unit can have for a turn, all things 
   considered, if in its present location. */

int
total_acp_for_turn(Unit *unit)
{
    int newacp = -1, turnacp = -1, minacp = -1, maxacp = -1;
    int u = NONUTYPE;

    /* Sanity checks. */
    assert_error(unit, "Tried to access a null unit.");
    if (!(unit->act))
      return 0;
    u = unit->type;
    /* Compute the new ACP total. */
    newacp = new_acp_for_turn(unit);
    turnacp = unit->act->acp + newacp;
    /* Clip the accumulated ACP to its limits. */
    minacp = u_acp_min(u);
    turnacp = max(turnacp, minacp);
    maxacp = ((u_acp_max(u) < 0) ? newacp : u_acp_max(u));
    turnacp = min(turnacp, maxacp);
    /* Return final value. */
    return turnacp;
}

/* Set the action points available to the unit this turn. */

void
set_unit_acp_for_turn(Unit *unit)
{
    /* Sanity checks. */
    assert_error(unit, "Tried to change the ACP of a null unit.");
    /* Units still under construction or off-area can't do anything. */
    if (!completed(unit) || !inside_area(unit->x, unit->y)) {
	unit->act->initacp = unit->act->acp = 0;
	return;
    }
    /* Get the total ACP for the turn. */
    unit->act->acp = unit->act->initacp = total_acp_for_turn(unit);
    /* Zero the counts of what actually got done. */
    unit->act->actualmoves = 0;
}

/* Compute and return the acp of a damaged unit, using a list of (hp
   acp) pairs and interpolating between them. */

int
damaged_acp(Unit *unit, Obj *effect)
{
    int u, err, rslt;

    u = unit->type;
    err = interpolate_in_list_ext(unit->hp, effect, 0, 0, 0, 0, u_hp(u),
				  u_acp(u), &rslt);
    if (err != 0) {
	run_warning("cannot get damaged acp for %s at hp %d, using %d",
		    u_type_name(u), u_acp(u));
	rslt = u_acp(u);
    }
    return rslt;
}

/* Compute the numeric "year part" and the textual "season name". */

static void
compute_season(void)
{
    Obj *names, *rest, *elt;

    curseasonname = NULL;
    if (world.yearlength > 1) {
	curyearpart = (g_turn() + g_initial_year_part()) % world.yearlength;
	/* Determine the name of the season, if defined. */
	names = g_season_names();
	if (names != NULL && names != lispnil && consp(names)) {
	    for_all_list(names, rest) {
		elt = car(rest);
		if (consp(elt)
		    && numberp(car(elt))
		    && numberp(cadr(elt))
		    && between(c_number(car(elt)),
			       curyearpart,
			       c_number(cadr(elt)))
		    && stringp(car(cddr(elt))))
		  curseasonname = c_string(car(cddr(elt)));
	    }
	}
    } else {
	curyearpart = 0;
    }
    /* Update the preformatted date/season string. */
    strcpy(curdatestr, absolute_date_string(g_turn()));
    if (curseasonname != NULL) {
	strcat(curdatestr, " (");
	strcat(curdatestr, curseasonname);
	strcat(curdatestr, ")");
    }
    /* Even though this has nothing to do with seasons, this is
       when the curdatestr is being updated. */
    if (area_lighting >= 0) {
	strcat(curdatestr, " (");
	strcat(curdatestr,
	       (area_lighting == 2 ? "day" :
		(area_lighting == 1 ? "twilight" : "night")));
	strcat(curdatestr, ")");
    }
}

static int
season_effect(int u)
{
    int err, rslt;

    if (curyearpart < 0)
      compute_season();
    err = interpolate_in_list(curyearpart, u_acp_season_effect(u), &rslt);
    if (err != 0) {
	rslt = 100;
    }
    return rslt;
}

#if 0

/* See how any agreements' terms are holding up. */

static void
test_agreements(void)
{
    Agreement *ag;

    for_all_agreements(ag) {
	if (ag->state == in_force) {
		/* what? */
	}
    }
}

#endif

/* Compute lighting-related data. */

static void
compute_sun(void)
{
    int curtime, highest, y = 0;

    switch (world.daylength) {
      /* Sun is at fixed position. */
      case 0:
	daynight = TRUE;
	sunx = area.sunx;  suny = area.suny;
	break;
      /* 1 turn == 1 day; no sun effects */
      case 1:
      	daynight = FALSE;
	break;
      /* n turns == 1 day */
      default:
	daynight = TRUE;
	/* If world has a appropriate circumference, the sun moves over
	   it at a regular pace. */
	if (world.circumference >= area.width) {
	    lastsunx = sunx;  lastsuny = suny;
	    curtime = ((g_turn() * 100 + g_initial_day_part())
		       % (world.daylength * 100));
	    /* Calculate the sun's x position. */
	    sunx = ((curtime * world.circumference) / (world.daylength * 100)
		    + area.sunx);
	    sunx %= world.circumference;
	}
	break;
    }
    switch (world.yearlength) {
      /* Sun is at fixed y position. */
      case 0: case 1:
	suny = area.suny;
	break;
      /* n > 1 turns == 1 year */
      default:
	lastsuny = suny;
	/* Find the highest latitude that direct sunlight can reach. 
	    Note that if axial tilt is negative, then it is equivalent to 
	    starting from the autumnal rather than the vernal equinox. */
	highest = (world.axial_tilt * (world.circumference / 4)) / 90;
	/* Approximate the y position of direct sunlight. */
	/* Find position in terms of year part. */
	y = g_turn() + g_initial_year_part();
	if (world.axial_tilt < 0)
	  y += (world.yearlength / 2);
	y %= world.yearlength;
	/* Descending node. */
	if ((y > ((1 * world.yearlength) / 4)) 
	    && (y <= ((3 * world.yearlength) / 4))) {
	    highest = -highest;
	    y -= (world.yearlength / 4);
	}
	/* Ascending node. */
	else
	  y += (world.yearlength / 4);
	y %= world.yearlength;
	/* Vertical position of direct sunlight over map. */
	suny = -highest + ((4 * highest * y) / world.yearlength);
	/* Adjust coords for area latitude. */
	suny -= area.latitude;
	break;
    }
    if (daynight) {
	Dprintf("Sun is now at %d,%d\n", sunx, suny);
    }
}

/* Compute the position of the sun for this turn. */

static void
run_sun(void)
{
    int x, y, oldlighting, anychange = FALSE, anyvariation = FALSE;
    Side *side;

    compute_sun();
    if (world.daylength <= 1 || world.circumference < area.width)
      return;
    oldlighting = area_lighting;
    area_lighting = -1;
    /* Find a cell whose lighting has changed; if so, update the
       whole area at once. */
    for_all_cells(x, y) {
	if (area_lighting < 0)
	  area_lighting = lighting(x, y, sunx, suny);
	if (!anychange
	    && (lighting(x, y, sunx, suny)
		!= lighting(x, y, lastsunx, lastsuny))) {
	    /* Night may impair units' vision, so redo coverage calcs. Lots of
	    cell updates get triggered here but since we are going to redraw all
	    maps before any unit can act we suppress them all. */
	    suppress_update_cell_display = TRUE;
	    really_reset_coverage();
	    suppress_update_cell_display = FALSE;
	    for_all_sides(side) {
		update_area_display(side);
	    }
	    /* Don't do this code again. */
	    anychange = TRUE;
	}
	/* If not all cells have the same lighting this turn, reset
	   the area_lighting value. */
	if (!anyvariation
	    && lighting(x, y, sunx, suny) != area_lighting) {
	    area_lighting = -1;
	    anyvariation = TRUE;
	}
    }
    /* Ensure that any new overall lighting value gets reflected on
       the screen. */
    if (area_lighting >= 0) {
	compute_season();
	for_all_sides(side) {
	    update_turn_display(side, TRUE);
	}
	/* Announce changes in area lighting. */
	if (area_lighting != oldlighting) {
	    switch (area_lighting) {
	      case 0:
		notify_all("Night has fallen.");
		break;
	      case 1:
		notify_all("Twilight.");
		break;
	      case 2:
		notify_all("The sun is up.");
		break;
	    }
	}
    }
}

static int num_key_points;

struct a_key_point {
    int x, y;
    int temp;
} *key_points;

static void calc_key_point_temps(int yearpart);
static int interpolate_temperature(int x, int y);

static void
calc_key_point_temps(int yearpart)
{
    int i, err, rslt;
    Obj *lis, *item, *loc;

    num_key_points = length(area.temp_year);
    if (num_key_points > 0 && key_points == NULL) {
	key_points =
	  (struct a_key_point *) xmalloc(num_key_points * sizeof(struct a_key_point));
    }
    i = 0;
    for_all_list(area.temp_year, lis) {
	item = car(lis);
	loc = car(item);
	if (consp(loc)) {
	    key_points[i].x = c_number(car(loc));
	    key_points[i].y = c_number(cadr(loc));
	}
	err = interpolate_in_list(yearpart, cdr(item), &rslt);
	if (err != 0) {
	    run_warning("Year part %d not within range of temp_year list",
			yearpart);
	    rslt = 0;
	}
	key_points[i].temp = rslt;
	++i;
    }
}

#if 1
static int
interpolate_temperature(int x, int y)
{
    int dbest, dnextbest, tbest, tnextbest, i, besti, nextbesti, d;

    if (num_key_points == 1)
      return key_points[0].temp;
    /* Find the closest and next-closest key points. */
    besti = 0;
    dbest = distance(x, y, key_points[besti].x, key_points[besti].y);
    nextbesti = -1;
    for (i = 1; i < num_key_points; ++i) {
	d = distance(x, y, key_points[i].x, key_points[i].y);
	if (d < dbest) {
	    nextbesti = besti;
	    dnextbest = dbest;
	    besti = i;
	    dbest = d;
	} else if (nextbesti < 0 || d < dnextbest) {
	    nextbesti = i;
	    dnextbest = d;
	}
    }
    tbest = key_points[besti].temp;
    tnextbest = key_points[nextbesti].temp;
    return ((tbest * dnextbest + tnextbest * dbest) / (dnextbest + dbest));
}
#endif

#if 0  /* experimental but losing */
static int
interpolate_temperature(int x, int y)
{
    int dbests[3], tbests[3], bests[3], i, j, num, dist;

    if (num_key_points < 3)
      return key_points[0].temp;
    /* Find the closest and next-closest key points. */
    bests[0] = 0;
    dbests[0] = distance(x, y, key_points[0].x, key_points[0].y);
    num = 1;
    for (i = 1; i < num_key_points; ++i) {
	dist = distance(x, y, key_points[i].x, key_points[i].y);
	for (j = 0; j < num; ++j) {
	    if (dist < dbests[j]) {
		dbests[j] = dist;
		bests[j] = i;
		break;
	    }
	}
	if (j == num && num < 3) {
	    dbests[j] = dist;
	    bests[j] = i;
	    ++num;
	}
    }
    for (j = 0; j < num; ++j) {
	tbests[j] = key_points[bests[j]].temp;
    }
    {
    int x21, y21, z21, x31, y31, z31, a, b, c, d, rslt;
    x21 = key_points[bests[1]].x - key_points[bests[0]].x;
    y21 = key_points[bests[1]].y - key_points[bests[0]].y;
    z21 = tbests[1] - tbests[0];
    x31 = key_points[bests[2]].x - key_points[bests[0]].x;
    y31 = key_points[bests[2]].y - key_points[bests[0]].y;
    z31 = tbests[2] - tbests[0];
    a = y21 * z31 - z21 * y31;
    b = z21 * x31 - x21 * z31;
    c = x21 * y31 - y21 * x31;
    d = - (key_points[bests[0]].x * a + key_points[bests[0]].y * b + tbests[0] * c);
    if (c == 0)
      return -99;
    rslt = - (a * x + b * y + d) / c;
    return rslt;
    }
}
#endif

/* Compute environment changes. */

static void
run_environment(void)
{
    int yrlen = world.yearlength, x, y, dir, t, celltemp, sum;
    int anychanges = FALSE;

    if (mintemp == maxtemp && !any_wind_variation_in_layer && !any_clouds)
      return;
    if (any_annual_temp_change < 0) {
	any_annual_temp_change = FALSE;
	if (yrlen > 0 && area.temp_year != lispnil) {
	    any_annual_temp_change = TRUE;
	}
    }
    if (any_annual_temp_change) {
	calc_key_point_temps(curyearpart);
    }
    /* The tmp1 layer will record where any weather changes occur. */
    for_all_cells(x, y)
      set_tmp1_at(x, y, 0);
    if (mintemp != maxtemp /* and any temperature changes */) {
	/* Compute the average temperature at each point in the world. */
	for_all_cells(x, y) {
	    /* Use the tmp2 layer to cache the previous value of the
               temperature. */
	    set_tmp2_at(x, y, temperature_at(x, y));
	    t = terrain_at(x, y);
	    if (any_annual_temp_change)
	      celltemp = interpolate_temperature(x, y);
	    else
	      celltemp = t_temp_avg(t);
	    /* Add in a random variation if specified. */
	    if (t_temp_variability(t) > 0) {
		celltemp += (xrandom(t_temp_variability(t))
			     - t_temp_variability(t)/2);
	    }
	    /* Higher elevations can be much colder. */
	    /* (In this pos, will influence lowlands via moderation -
	       realistic?) */
	    if (elevations_defined()
		&& g_temp_floor_elev() != 0
		&& elev_at(x, y) < g_temp_floor_elev()) {
		celltemp -=
		    ((celltemp - g_temp_floor()) * elev_at(x, y))
		    / g_temp_floor_elev();
	    }
	    /* Clip to terrain type's limits. */
	    if (celltemp < t_temp_min(t))
	      celltemp = t_temp_min(t);
	    if (celltemp > t_temp_max(t))
	      celltemp = t_temp_max(t);
	    /* Record the (unmoderated) temperature of the cell. */
	    set_temperature_at(x, y, celltemp);
	}
	/* Sometimes the scale of the world is such that neighboring cells
	   influence each other's temperatures. */
	if (g_temp_mod_range() > 0) {
	    /* only doing a range of 1... */
	    for_all_interior_cells(x, y) {
		sum = temperature_at(x, y);
		for_all_directions(dir)
		  sum += temperature_at(x+dirx[dir], y+diry[dir]);
		set_tmp3_at(x, y, sum / (NUMDIRS + 1));
	    }
	    for_all_interior_cells(x, y) {
		celltemp = tmp3_at(x, y);
		t = terrain_at(x, y);
		/* Clip to terrain type's limits. */
		if (celltemp < t_temp_min(t))
		  celltemp = t_temp_min(t);
		if (celltemp > t_temp_max(t))
		  celltemp = t_temp_max(t);
		set_temperature_at(x, y, celltemp);
	    }
	}
	/* Set a changed bit at any changed cells. */
 	for_all_cells(x, y) {
	    if (temperature_at(x, y) != tmp2_at(x, y))
	      set_tmp1_at(x, y, 1);
	    anychanges = TRUE;
	}
	if (numcoattypes > 0) {
	    /* (should make more generic) */
	    for_all_terrain_types(t) {
		if (t_subtype(t) == coatingsubtype) {
		    allocate_area_aux_terrain(t);
		}
	    }
	    for_all_cells(x, y) {
		for_all_terrain_types(t) {
		    if (strcmp(t_type_name(t), "snow") == 0) {
			int olddepth = aux_terrain_at(x, y, t), newdepth;
			if (temperature_at(x, y) <= 3) {
			    newdepth = 1;
			} else {
			    newdepth = 0;
			}
			if (newdepth > tt_coat_max(t, terrain_at(x, y)))
			  newdepth = tt_coat_max(t, terrain_at(x, y));
			if (newdepth < tt_coat_min(t, terrain_at(x, y)))
			  newdepth = tt_coat_min(t, terrain_at(x, y));
			if (newdepth != olddepth) {
			    set_aux_terrain_at(x, y, t, newdepth);
			    set_tmp1_at(x, y, 2);
			    anychanges = TRUE;
			}
		    } else if (strcmp(t_type_name(t), "mud") == 0) {
			int olddepth = aux_terrain_at(x, y, t), newdepth;
			if (between(4, temperature_at(x, y), 9)) {
			    newdepth = 1;
			} else {
			    newdepth = 0;
			}
			if (newdepth > tt_coat_max(t, terrain_at(x, y)))
			  newdepth = tt_coat_max(t, terrain_at(x, y));
			if (newdepth < tt_coat_min(t, terrain_at(x, y)))
			  newdepth = tt_coat_min(t, terrain_at(x, y));
			if (newdepth != olddepth) {
			    set_aux_terrain_at(x, y, t, newdepth);
			    set_tmp1_at(x, y, 2);
			    anychanges = TRUE;
			}
		    }
		}
	    }
	}
    }
    /* Use the tmp2 layer to cache the previous value of clouds. */
    if (any_clouds /* and any cloud changes */) {
	/* Save the previous state. */
	for_all_interior_cells(x, y) {
	    set_tmp2_at(x, y, raw_cloud_at(x, y));
	}
	/* Do completely random changes to clouds. */
	if (1 /* any random changes */) {
	    for_all_cells(x, y) {
		int cloud = raw_cloud_at(x, y), newcloud;
		int t = terrain_at(x, y);
		int anychange;

		anychange = FALSE;
		if (probability(10)) {
		    newcloud = cloud + (flip_coin() ? 1 : -1);
		    newcloud = max(newcloud, t_clouds_min(t));
		    newcloud = min(newcloud, t_clouds_max(t));
		    if (newcloud != cloud) {
			cloud = newcloud;
			anychange = TRUE;
		    }
		}
		if (anychange) {
		    set_raw_cloud_at(x, y, cloud);
		}
	    }
	}
	/* Let winds move clouds around. */
	if (winds_defined() /* and wind affects clouds */) {
	    int x1, y1, winddir, sum, count, newcloud, t, raw;

	    for_all_cells(x, y) {
		/* Existing cloud accounts for bulk of cloud present. */
		sum = 6 * 100 * raw_cloud_at(x, y);
		count = 6;
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			if (wind_force_at(x1, y1) > 0) {
			    winddir = wind_dir_at(x1, y1);
			    if (winddir == opposite_dir(dir)) {
				sum += 100 * raw_cloud_at(x1, y1);
				++count;
			    }
			}
		    }
		}
		raw = sum / count;
		newcloud = raw / 100;
		/* Round off the results.  Otherwise, all clouds will gradually
		   disappear as the effect of truncation accumulates. */
		if (raw % 100 > 50)
		    ++newcloud;
		t = terrain_at(x, y);
		newcloud = max(newcloud, t_clouds_min(t));
		newcloud = min(newcloud, t_clouds_max(t));
		set_tmp3_at(x, y, newcloud);
	    }
	    /* Copy over the buffered-up changes. */
	    for_all_interior_cells(x, y) {
		set_raw_cloud_at(x, y, tmp3_at(x, y));
	    }
	}
	/* Set a changed bit at any changed cells. */
	for_all_cells(x, y) {
	    if (raw_cloud_at(x, y) != tmp2_at(x, y))
	      set_tmp1_at(x, y, 1);
	    anychanges = TRUE;
	}
    }
    /* Do wind changes. */
    /* Use tmp tmp2 layer to cache old values for the wind. */
    if (any_wind_variation_in_layer) {
	/* Save the previous state. */
	for_all_interior_cells(x, y) {
	    set_tmp2_at(x, y, raw_wind_at(x, y));
	}
	vary_winds();
	if (g_wind_mix_range() > 0)
	  mix_winds();
	/* Set a changed bit at any changed cells. */
	for_all_cells(x, y) {
	    if (raw_wind_at(x, y) != tmp2_at(x, y))
	      set_tmp1_at(x, y, 1);
	    anychanges = TRUE;
	}
    }
    /* See if any displays should change and report if so. */
    if (anychanges) {

#if 1		/* Just redraw the map if there were any changes. */

    	Side *side;

    	for_all_sides(side) {    	
    		update_area_display(side);
	}
	
#else	/* This took forever to complete. No point in updating
		cells one by one since run_environment changes the
		whole map if it changes anything at all. */
		
	for_all_cells(x, y) {
	    /* A value of 2 means terrain changed, and so we need to
	       do a full update of the cell. */
	    if (tmp1_at(x, y) == 2) {
		Side *side;
		for_all_sides(side) {
		    update_cell_display(side, x, y,
					UPDATE_ALWAYS | UPDATE_ADJ);
		}
	    } else if (tmp1_at(x, y) == 1) {
		/* Just update if the side is displaying any weather
		   data. */
		all_see_cell_weather(x, y);
	    }
	}

#endif

    }
}

static void
vary_winds(void)
{
    int x, y, t, wdir, wforce, newwforce, anychange;

    for_all_interior_cells(x, y) {
	wdir = wind_dir_at(x, y);
	wforce = wind_force_at(x, y);
	t = terrain_at(x, y);
	anychange = FALSE;
	if (probability(t_wind_variability(t))) {
	    wdir = (flip_coin() ? right_dir(wdir) : left_dir(wdir));
	    anychange = TRUE;
	}
	if (probability(t_wind_force_variability(t))) {	
	    newwforce = wforce + (flip_coin() ? 1 : -1);
	    newwforce = max(newwforce, t_wind_force_min(t));
	    newwforce = min(newwforce, t_wind_force_max(t));
	    if (newwforce != wforce) {
		wforce = newwforce;
		anychange = TRUE;
	    }
	}
	if (anychange) {
	    set_wind_at(x, y, wdir, wforce);
	}
    }
}

static void
mix_winds(void)
{
    int num, i, x, y, dir, x1, y1, wdir, wforce, sumx, sumy, n, t;

    num = (area.width * area.height) / 6;

    for (i = 0; i < num; ++i) {
	random_point(&x, &y);
	wdir = wind_dir_at(x, y);
	wforce = wind_force_at(x, y);
	sumx = dirx[wdir] * wforce;  sumy = diry[wdir] * wforce;
	n = 1;
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		wdir = wind_dir_at(x1, y1);
		wforce = wind_force_at(x1, y1);
		sumx += dirx[wdir] * wforce;  sumy += diry[wdir] * wforce;
		++n;
	    }
	}
	/* Over time, the rounding-down effect of division would
	   causes all winds to diminish to zero.  Counteract by
	   incrementing the sum. */
	sumx += n - 1;  sumy += n - 1;
	sumx = sumx / n;  sumy = sumy / n;
	if (sumx == 0 && sumy == 0) {
	    wdir = random_dir();
	    wforce = 0;
	} else {
	    wdir = approx_dir(sumx, sumy);
	    wforce = distance(0, 0, sumx, sumy);
	}
	t = terrain_at(x, y);
	wforce = max(wforce, t_wind_force_min(t));
	wforce = min(wforce, t_wind_force_max(t));
	set_wind_at(x, y, wdir, wforce);
    }
}

/* Update all sides with all weather changes that have happened at
   the given location. */

static void
all_see_cell_weather(int x, int y)
{
    int oldview, mask;
    Side *side;

    for_all_sides(side) {
	mask = 0;
	if (side->see_all) {
	    mask = UPDATE_TEMP | UPDATE_CLOUDS | UPDATE_WINDS;
	} else if (g_see_weather_always()
		   ? (terrain_view(side, x, y) != UNSEEN)
		   : (cover(side, x, y) > 0)) {
	    if (temperatures_defined()) {
		oldview = temperature_view(side, x, y);
		if (oldview != temperature_at(x, y)) {
		    set_temperature_view(side, x, y, temperature_at(x, y));
		    mask |= UPDATE_TEMP;
		}
	    }
	    if (clouds_defined()) {
		oldview = cloud_view(side, x, y);
		if (oldview != raw_cloud_at(x, y)) {
		    set_cloud_view(side, x, y, raw_cloud_at(x, y));
		    mask |= UPDATE_CLOUDS;
		}
	    }
	    if (winds_defined()) {
		oldview = wind_view(side, x, y);
		if (oldview != raw_wind_at(x, y)) {
		    set_wind_view(side, x, y, raw_wind_at(x, y));
		    mask |= UPDATE_WINDS;
		}
	    }
	}
	if (mask != 0)
	  update_cell_display(side, x, y, mask);
    }
}

/* Given that the spying unit is going to get info about other units at this
   location, figure out just what it is going to see. */

static void
spy_on_location(int x, int y)
{
    int qual;
    Unit *unit2, *occ;

    for_all_stack(x, y, unit2) {
	if (unit2->side != tmpunit->side) {
	    qual = uu_spy_quality(tmpunit->type, unit2->type);
	    if (probability(qual)) {
		/* Spy got something, report it. */
		/* (should be more worked-out, dunno exactly how) */
		see_exact(tmpunit->side, x, y);
		for_all_occupants(unit2, occ) {
		    /* (should get info about occupants?) */
		}
		/* Might also be able to track the unit. */
		if (xrandom(10000) < uu_spy_track(tmpunit->type, unit2->type)) {
		    /* (should inform side of tracking?) */
		    add_side_to_set(tmpunit->side, unit2->tracking);
		}
	    }
	}
    }
}

/* Certain kinds of units can do spying for the side they're on. */

static void
run_spies(void)
{
    int u, chance;
    Unit *unit;

    if (any_spying < 0) {
	any_spying = FALSE;
	for_all_unit_types(u) {
	    if (u_spy_chance(u) > 0) {
		any_spying = TRUE;
		break;
	    }
	}
	/* But spying is pointless if everybody can see everything anyway. */
	if (g_see_all())
	  any_spying = FALSE;
	Dprintf("Any spying: %d\n", any_spying);
    }
    if (!any_spying)
      return;
    Dprintf("Running spies\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    u = unit->type;
	    chance = u_spy_chance(u);
	    if (chance > 0 && xrandom(10000) < chance) {
		/* Spying is successful, decide how much was seen. */
		tmpunit = unit;
		apply_to_area(unit->x, unit->y, u_spy_range(u), spy_on_location);
	    }
	}
    }
}

/* Test each unit that is out in the open to see if a terrain-related
   accident happens to it.  Accidents can either kill the unit instantly or
   just damage it. */

static void
run_accidents(void)
{
    int u, t, chance, hit;
    Unit *unit;

    if (any_accidents < 0) {
	any_accidents = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_accident_vanish(u, t) > 0
		    || ut_accident_hit(u, t) > 0) {
		    any_accidents = TRUE;
		    break;
		}
	    }
	    if (any_accidents)
	      break;
    	}
    }
    if (!any_accidents)
      return;
    Dprintf("Running accidents\n");
    for_all_units(unit) {
	if (in_play(unit) && unit->transport == NULL) {
	    u = unit->type;
	    t = terrain_at(unit->x, unit->y);
	    chance = ut_accident_vanish(u, t);
	    if (chance > 0 && xrandom(10000) < chance) {
	    	/* Kill the unit outright. */
		kill_unit(unit, H_UNIT_VANISHED);
		/* (should make a hevt) */
		continue;
	    }
	    chance = ut_accident_hit(u, t);
	    if (chance > 0 && xrandom(10000) < chance) {
		/* Damage the unit. */
		hit = max(0, roll_dice1((DiceRep)ut_accident_damage(u, t)));
		if (hit > 0) {
		    unit->hp2 -= hit;
		    damage_unit(unit, accident_dmg, NULL);
		}
	    }
	}
    }
}

/* Attrition only takes out a few hp at a time, but can be deadly...
   Note that attrition does not affect incomplete units - use cp-attrition
   for those. */

static void
run_attrition(void)
{
    int u, t, dmg;
    Unit *unit;

    if (any_attrition < 0) {
	any_attrition = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_attrition(u, t) > 0) {
		    any_attrition = TRUE;
		    break;
		}
	    }
	    if (any_attrition)
	      break;
    	}
    }
    if (!any_attrition)
      return;
    Dprintf("Running attrition\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    u = unit->type;
	    t = terrain_at(unit->x, unit->y);
	    if (ut_attrition(u, t) <= 0)
	      continue;
	    dmg = prob_fraction(ut_attrition(u, t));
	    /* This is like hit_unit but doesn't have other effects. */
	    unit->hp2 -= dmg;
	    damage_unit(unit, attrition_dmg, NULL);
	}
    }
}

/* Check each unit to see whether it revolts spontaneously.  While
   surrender is influenced by nearby units, revolt takes only the
   overall state of the world into account. */

static void
run_revolts(void)
{
    int u;
    Unit *unit;

    if (any_revolts < 0) {
	any_revolts = FALSE;
	for_all_unit_types(u) {
	    if (u_revolt(u) > 0 || u_revolt_opinion_min(u) > 0) {
		any_revolts = TRUE;
		break;
	    }
    	}
    }
    if (!any_revolts)
      return;
    Dprintf("Running revolts\n");
    for_all_units(unit) {
	if (in_play(unit)
	    && (u_revolt(unit->type) > 0
		|| (unit->opinions != NULL
		    && u_revolt_opinion_min(unit->type) > 0))) {
	    unit_revolt(unit, FALSE);
	}
    }
}

/* Test for and run a single revolt. */

static void
unit_revolt(Unit *unit, int force)
{
    int u = unit->type, ux = unit->x, uy = unit->y, chance, lo, opin;
    int sideweights[MAXSIDES+1], sn2, sum, n, i, psum;
    Side *oldside = unit->side, *newside, *side2;
    Unit *unit2;

    chance = u_revolt(u);
    if (unit->opinions != NULL
	&& u_opinion_min(u) < 0
	&& u_revolt_opinion_min(u) > 0) {
	lo = u_revolt_opinion_min(u);
	opin = unit_opinion(unit, unit->side);
	if (opin < 0)
	  chance += ((lo - chance) * (0 - opin)) / (0 - u_opinion_min(u));
    }
    if (force || xrandom(10000) < chance) {
	newside = oldside;
	if (oldside != unit->origside
	    && (unit->origside == indepside
		|| unit->origside->ingame)
	    && !trusted_side(oldside, unit->origside)
	    && unit_allowed_on_side(unit, unit->origside)
	    ) {
	    newside = unit->origside;
	} else if (unit->opinions != NULL) {
	    /* Find all the non-allied sides in the game and weight
	       each according to the unit's opinion of the side. */
	    sum = 0;
	    for_all_sides(side2) {
		sn2 = side_number(side2);
		sideweights[sn2] = 0;
		if ((side2 == NULL || side2 == indepside || side2->ingame)
		    && side2 != oldside
		    && !trusted_side(oldside, side2)
		    && unit_allowed_on_side(unit, side2)
		    /* Don't go over to a side that doesn't even know
                       we're there. */
		    && side_sees_image(side2, unit)
		    ) {
		    sideweights[sn2] =
		      unit_opinion(unit, side2) + u_opinion_min(u);
		}
		sum += sideweights[sn2];
	    }
	    /* Select one of the sides. */
	    if (sum > 0) {
		n = xrandom(sum);
		psum = 0;
		for (i = 0; i <= numsides; ++i) {
		    psum += sideweights[i];
		    if (n <= psum) {
			newside = side_n(i);
			break;
		    }
		}
	    }
	} else {
	    /* Find all the non-allied sides in the game and weight
	       each according to the number of units of the same type
	       present. */
	    sum = 0;
	    for_all_sides(side2) {
		sn2 = side_number(side2);
		sideweights[sn2] = 0;
		if ((side2 == NULL || side2 == indepside || side2->ingame)
		    && side2 != oldside
		    && !trusted_side(oldside, side2)
		    && unit_allowed_on_side(unit, side2)
		    /* Don't go over to a side that doesn't even know
                       we're there. */
		    && side_sees_image(side2, unit)
		    ) {
		    sideweights[sn2] = 1;
		    for_all_side_units(side2, unit2) {
			/* OK if not ideal to count real units, this
                           is just a weighting */
			if (in_play(unit2) && unit2->type == unit->type)
			  ++(sideweights[sn2]);
		    }
		}
		sum += sideweights[sn2];
	    }
	    /* Select one of the sides. */
	    if (sum > 0) {
		n = xrandom(sum);
		psum = 0;
		for (i = 0; i <= numsides; ++i) {
		    psum += sideweights[i];
		    if (n <= psum) {
			newside = side_n(i);
			break;
		    }
		}
	    }
	}
	/* Might not have been much of a revolt. */
	if (newside == oldside)
	  return;
	/* Tell the players what happened. */
	/* (should notify other players, if they're observing the unit?) */
	if (newside == indepside) {
	    notify(oldside, "%s revolts, becomes independent!",
		   unit_handle(oldside, unit));
	} else {
	    notify(oldside, "%s revolts, goes over to %s!",
		   unit_handle(oldside, unit), short_side_title(newside));
	    notify(newside, "%s revolts, comes over to your side!",
		   unit_handle(newside, unit));
	}
	change_unit_side(unit, newside, H_UNIT_REVOLTED, NULL);
	/* (should set new opinions and maybe morale) */
	/* Give the previous side a last view of the situation. */
	see_exact(oldside, ux, uy);
	update_cell_display(oldside, ux, uy, UPDATE_ALWAYS);
	all_see_cell(ux, uy);
    }
}

/* Test whether surrenders can happen in this game. */

static void
run_surrenders(void)
{
    int u1, u2, u3, range;
    Unit *unit;

    if (any_surrenders < 0) {
	any_surrenders = FALSE;    
	for_all_unit_types(u1) {
	    for_all_unit_types(u2) {
		if (uu_surrender_chance(u1, u2) > 0) {
		    any_surrenders = TRUE;
		    if (surrender_ranges == NULL) {
			surrender_ranges =
			  (short *) xmalloc(numutypes * sizeof(short));
			for_all_unit_types(u3)
			  surrender_ranges[u3] = -1;
		    }
		    range = uu_surrender_range(u1, u2);
		    surrender_ranges[u1] = max(range, surrender_ranges[u1]);
		}
    	    }
    	}
    }
    if (!any_surrenders)
      return;
    Dprintf("Running surrenders\n");
    /* For each unit, look for units nearby that might surrender to it. */
    for_all_units(unit) {
	if (in_play(unit)
	    /* For now anyway, nobody surrenders to independents. */
	    && !indep(unit)) {
	    unit_surrender(unit);
	}
    }
}

/* Units may surrender to enemy units that are visible nearby.
   Independents have to be treated specially, since they don't have a
   view to work from.  We sort of compute the view "on the fly". */

static void
unit_surrender(Unit *unit)
{
    int u = unit->type, dir, x1, y1, range /*, surrounded = TRUE */;
    Unit *unit2;

    range = surrender_ranges[u];
    if (range < 0) {
	/* This unit won't surrender, nothing to do. */
    } else if (range > 1) {
	run_warning("Surrender range of %d not supported, ignoring", range);
    } else {
	/* Range is 0 or 1; check other units in this cell. */
	for_all_stack(unit->x, unit->y, unit2) {
	    if (in_play(unit2)
		&& unit2->side != unit->side
		&& uu_surrender_chance(u, unit2->type) > 0
		&& visible_to(unit, unit2)) {
		maybe_surrender_to(unit, unit2);
	    }
	}
	/* Check on adjacent units. */
        if (range == 1) {
	    for_all_directions(dir) {
		if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
		    for_all_stack(x1, y1, unit2) {
			if (in_play(unit2)
			    && unit2->side != unit->side
			    && uu_surrender_chance(u, unit2->type) > 0
			    && visible_to(unit, unit2)) {
			    maybe_surrender_to(unit, unit2);
			}
		    }
		}
	    }
	}
    }
}

/* Calculate whether one unit is visible to another, even if the other
   is independent. */
/* (should rewrite this) */
int
visible_to(Unit *unit, Unit *unit2)
{
    if (g_see_all()) {
	return TRUE;
    } else if (side_sees_unit(unit->side, unit2)) {
	return TRUE;
    } else {
	/* (should be more careful to check see-chances) */
    	if (distance(unit->x, unit->y, unit2->x, unit2->y)
	    <= u_vision_range(unit->type))
    	  return TRUE;
    	else
    	  return FALSE;
    }
}

static void
maybe_surrender_to(Unit *unit, Unit *unit2)
{
    int chance;

    chance = uu_surrender_chance(unit->type, unit2->type);
    if (xrandom(10000) < chance) {
	capture_unit(unit, unit2, H_UNIT_SURRENDERED);
    }
}

/* Some types of units can become completed and grow to full size
   automatically when they get to a certain point. */

static void
run_self_builds(void)
{
    int u, cpper;
    Unit *unit;

    /* Precompute whether any self-building ever happens. */
    if (any_self_builds < 0) {
	any_self_builds = FALSE;
	for_all_unit_types(u) {
	    if (u_cp_per_self_build(u) > 0) {
		any_self_builds = TRUE;
		break;
	    }
	}
	Dprintf("Any self builds: %d\n", any_self_builds);
    }
    if (!any_self_builds)
      return;
    Dprintf("Running self builds\n");
    for_all_units(unit) {
	u = unit->type;
	if (in_play(unit)
	    && !completed(unit)
	    && (cpper = u_cp_per_self_build(u)) > 0
	    && unit->cp >= u_cp_to_self_build(u)) {
	    unit->cp += cpper;
	    if (unit->cp > u_cp(u))
	      unit->cp = u_cp(u);
	    if (completed(unit)) {
		make_unit_complete(unit);
	    } else {
	    	/* Let the player know that progress was made. */
		update_unit_display(unit->side, unit, TRUE);
	    }
	}
    }
}

static void
run_environment_effects(void)
{
    int err, dmg;
    Unit *unit;
    Obj *attrition;

    /* Precompute whether any environment effects happen. */
    if (any_environment_effects < 0) {
	int u;

	any_environment_effects = FALSE;
	for_all_unit_types(u) {
	    if (u_temp_attrition(u) != lispnil) {
		any_environment_effects = TRUE;
		break;
	    }
	}
	Dprintf("Any environment effects: %d\n", any_environment_effects);
    }
    if (!any_environment_effects)
      return;
    if (!temperatures_defined())
      return;
    Dprintf("Running environmental effects\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    attrition = u_temp_attrition(unit->type);
	    if (attrition != lispnil
		&& !(unit->transport != NULL
		     && uu_temp_protect(unit->transport->type, unit->type))) {
		err = interpolate_in_list(temperature_at(unit->x, unit->y), attrition, &dmg);
		if (err != 0) {
		    dmg = 0;
		}
		damage_unit_with_temperature(unit, dmg);
	    }
	    /* (should check for storm damage here?) */
	}
    }
}

static void
damage_unit_with_temperature(Unit *unit, int dmg)
{
    int n;

    n = prob_fraction(dmg);
    if (n >= unit->hp) {
	rescue_occupants(unit);
	kill_unit(unit, H_UNIT_DIED_FROM_TEMPERATURE);
    } else if (n > 0) {
	notify(unit->side, "%s loses %d HP due to the temperature",
		unit_handle(unit->side, unit), n);
	unit->hp -= n;
	unit->hp2 -= n;
	update_unit_display(unit->side, unit, TRUE);
    }
}

static void
run_people_side_changes(void)
{
    int x, y, u, t;
    Unit *unit;

    /* Precompute whether any people side changes can happen. */
    if (any_people_side_changes < 0) {
	any_people_side_changes = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_people_surrender(u, t) > 0) {
		    any_people_side_changes = TRUE;
		    break;
		}
	    }
	    if (any_people_side_changes)
	      break;
	}
	Dprintf("Any people side changes: %d\n", any_people_side_changes);
    }
    if (!any_people_side_changes)
      return;
    if (!people_sides_defined())
      return;
    /* Precompute a per-unit-type flag. */
    if (any_people_surrenders == NULL) {
	any_people_surrenders = (short *) xmalloc(numutypes * sizeof(short));
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_people_surrender(u, t) > 0) {
		    any_people_surrenders[u] = TRUE;
		    break;
		}
	    }
	}
    }
    Dprintf("Running people side changes\n");
    for_all_cells(x, y) {
	if (unit_at(x, y) != NULL) {
	    for_all_stack(x, y, unit) {
		/* The people here may change sides. */
	        u = unit->type;
		if (any_people_surrenders[u]
		    && probability(people_surrender_chance(u, x, y))) {
		    change_people_side_around(x, y, u, unit->side);
		}
	    }
	} else {
	    /* Unoccupied cells might see population revert. */
	    /* (this would need multiple-loyalty pops) */
	}
    }
}

int
people_surrender_chance(int u, int x, int y)
{
    int m, chance, peop;

    chance = ut_people_surrender(u, terrain_at(x, y));
    /* Modify the basic chance according to people types, if present. */
    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (m_people(m) > 0
		&& cell_material_defined(m)) {
		peop = material_at(x, y, m);
		if (peop > 0) {
		    chance = (chance * um_people_surrender(u, m)) / 100;
		}
	    }
	}
    }
    return chance;
}

void
change_people_side_around(int x, int y, int u, Side *side)
{
    int pop = people_side_at(x, y), s = side_number(side);
    Side *oldside, *side2;

    if (pop != NOBODY
        && pop != s
        && !trusted_side(side, side_n(pop))) {
        oldside = side_n(pop);
	set_people_side_at(x, y, s);
	if (side) {
	    for_all_sides(side2) {
		if (side == side2 || trusted_side(side, side2)) {
		    add_cover(side2, x, y, 1);
		}
	    }
	}
	update_cell_display_all_sides(x, y, UPDATE_ALWAYS | UPDATE_ADJ);
	/* Previous side(s) lose free coverage. */
	for_all_sides(side2) {
	    if (!trusted_side(side, side2)
		&& (oldside == side2 || trusted_side(oldside, side2))) {
		add_cover(side2, x, y, -1);
		/* Update coverage display. */
		update_cell_display(side2, x, y, UPDATE_COVER);
	   }
	}
    }
    /* (should add ability to change adjacent cells also) */
}

/* See if the numbers of individuals in a cell exceeds the max, and
   migrate or remove so as to bring the numbers back in line. */

static void
run_people_limits(void)
{
    int m, t, x, y, num, ratio, amt, newamt;

    /* Precompute whether there are any people limits. */    
    if (any_people_max < 0) {
	any_people_max = FALSE;
	for_all_terrain_types(t) {
	    if (t_people_max(t) >= 0) {
		any_people_max = TRUE;
		break;
	    }
	}
	Dprintf("Any people max: %d\n", any_people_max);
    }
    if (!any_people_max)
      return;
    if (!any_cell_materials_defined())
      return;
    Dprintf("Running people limits\n");
    for_all_cells(x, y) {
	t = terrain_at(x, y);
	if (t_people_max(t) >= 0) {
	    num = num_people_at(x, y);
	    if (num > t_people_max(t)) {
		/* Too many people here, trim them down. */
		/* Compute the ratio of limit to actual number.
		   (Note that actual number is guaranteed to be nonzero.) */
		ratio = (t_people_max(t) * 100) / num;
		for_all_material_types(m) {
		    if (m_people(m) > 0
			&& cell_material_defined(m)) {
			amt = material_at(x, y, m);
			if (amt > 0) {
			    newamt = (amt * ratio) / 100;
			    set_material_at(x, y, m, newamt);
			    /* (should update sides?) */
			}
		    }
		}
	    }
	}
    }
}

void
update_cell_display_all_sides(int x, int y, int flags)
{
    Side *side;

    for_all_sides(side) {
	/* We have no people/control view layers at the moment, so
	   always update if the terrain has been seen. */
	if (side->ingame && terrain_visible(side, x, y)) {
	    update_cell_display(side, x, y, flags);
	}
    }
}

/* See if it's time for any scheduled arrivals to appear. */

static void
run_appearances(void)
{
    char abuf[BUFSIZE];
    int curturn, nx, ny, nw, nh, nx0, ny0, nx1, ny1, tries;
    int need_rescan, scan_tries;
    Unit *unit, *transport;

    /* Precompute whether any units will appear at a given time. */
    if (any_appearances < 0) {
    	any_appearances = FALSE;
	for_all_units(unit) {
	    if (unit->cp < 0 && unit_appear_turn(unit) >= 0) {
		any_appearances = TRUE;
		break;
	    }
	}
	Dprintf("Any appearances: %d\n", any_appearances);
    }
    if (!any_appearances)
      return;
    Dprintf("Running appearances\n");
    curturn = g_turn();
    /* The unit may be blocked from appearing for some reason, such
       as a stacking limit, so scan the list of appearances several
       times. */
    need_rescan = TRUE;
    scan_tries = 5;
    while (need_rescan && scan_tries-- > 0) {
	need_rescan = FALSE;
	for_all_units(unit) {
	    /* See if now time for a unit to appear. */
	    if (unit->cp < 0
		&& unit_appear_turn(unit) >= 0
		&& unit_appear_turn(unit) <= curturn) {
		/* Set the unit to its correct cp. */
		unit->cp = (- unit->cp);
		/* Get the base location at which it will appear. */
		nx = nx0 = (- unit->prevx);  ny = ny0 = (- unit->prevy);
		nw = unit_appear_var_x(unit);  nh = unit_appear_var_y(unit);
		tries = ((nw >= 0 && nh >= 0) ? 100 : 1);
		while (tries-- > 0) {
		    if (nw >= 0 && nh >= 0
			&& random_point_in_area(nx0, ny0, nw, nh, &nx1, &ny1)) {
			nx = nx1;  ny = ny1;
		    }
		    /* Do the usual steps to place the unit. */
		    /* (should add case for appearing on conn - share with patch_obj_refs code?) */
		    if (inside_area(nx, ny)) {
			if (unit->transport != NULL) {
			    /* nothing to do?? */
			} else if (type_can_occupy_cell(unit->type, nx, ny)
			    && type_survives_in_cell(unit->type, nx, ny)) {
			    enter_cell(unit, nx, ny);
			    tries = 0;
			} else {
			    /* Search this cell for units to enter. */
			    for_all_stack(nx, ny, transport) {
			 	if (unit->side == transport->side
				    && can_occupy(unit, transport)) {
				    enter_transport(unit, transport);
				    tries = 0;
				    break;
				}
			    }
			}
		    }
		}
		if (inside_area(unit->x, unit->y)) {
		    init_unit_actorstate(unit, FALSE);
		    init_unit_plan(unit);
		    /* (should use generic event notification mech? */
		    destination_desc(abuf, unit->side, unit,
				     unit->x, unit->y, 0);
		    notify(unit->side, "%s has appeared at %s.",
			   unit_handle(unit->side, unit), abuf);
		} else {
		    /* We've got a problem, put the unit back. */
		    unit->cp = (- unit->cp);
		    /* Go around again, perhaps a plausible transport
		       appeared during this pass. */
		    need_rescan = TRUE;
		}
	    }
    	}
    }
}

/* Perform any prescheduled disappearances. */

static void
run_disappearances(void)
{
    char buf1[BUFSIZE], buf2[BUFSIZE];
    int curturn;
    Unit *unit;

    /* Precompute whether any units will disappear at a given time. */
    /* Note that this won't be right if any units created during the game
       must disappear, but there is no during-the-game way to create a unit
       that is scheduled to disappear, so this is no problem. */
    if (any_disappearances < 0) {
    	any_disappearances = FALSE;
	for_all_units(unit) {
	    if (unit_disappear_turn(unit) >= 0) {
		any_disappearances = TRUE;
		break;
	    }
	}
	Dprintf("Any disappearances: %d\n", any_disappearances);
    }
    if (!any_disappearances)
      return;
    Dprintf("Running disappearances\n");
    curturn = g_turn();
    for_all_units(unit) {
    	/* See if now time for a unit to disappear. */
    	if (in_play(unit)
	    && unit_disappear_turn(unit) >= 0
	    && unit_disappear_turn(unit) <= curturn) {
	    strcpy(buf1, unit_handle(unit->side, unit));
	    destination_desc(buf2, unit->side, unit, unit->x, unit->y, 0);
	    /* (should eject occupants first if possible) */
	    kill_unit(unit, H_UNIT_LEFT_WORLD);
	    /* (should use generic event notification mech? */
	    notify(unit->side, "%s has disappeared from %s.", buf1, buf2);
	}
    }
}

/* Some types of units can repair themselves or others without doing actions. */

namespace Xconq {
    //! Temp pointer to unit to be auto-repaired.
    Unit *auto_repair_unit = NULL;
}

static void
run_auto_repair(void)
{
    using namespace Xconq;

    Unit *unit = NULL, *transport = NULL;
    int u = NONUTYPE;
    int hpold = -1;
    int i = -1;

    if (!any_auto_repair && !any_hp_recovery)
      return;
    Dprintf("Running auto repair\n");
    // Go through all units and auto-repair as necessary.
    for_all_units(unit) {
	// Skip out-of-play units.
	if (!in_play(unit))
	    continue;
	// Useful info.
	u = unit->type;
	hpold = unit->hp;
	auto_repair_unit = unit;
	// Skip units that don't need repairs.
	if (unit->hp >= u_hp(u))
	    continue;
	// Skip units which cannot be auto-repaired,
	//  and which do not benefit from the hp-recovery mechanism.
	if (!cv__could_be_auto_repaired[u] && (0 >= u_hp_recovery(u)))
	    continue;
	// Run the hp-recovery mechanism.
	if (0 < u_hp_recovery(u)) {
	    if (valid(check_repair_action(unit, unit, unit)))
		do_repair_action(unit, unit, unit);
	}
	// If unit can only be auto-repaired inside another unit, 
	//  then skip ranged auto-repair attempts.
	if (0 > cv__auto_repaired_range_max[u]) {
	    // Traverse the transport pointers to top of stack.
	    transport = unit->transport ? unit->transport : unit;
	    for (; transport->transport; transport = transport->transport);
	    // Auto-repair from the top-level transport downwards.
	    auto_repair_from_in(transport);
	}
	// Else apply ranged auto-repair.
	// NOTE: We cannot apply to a fat ring because of the way the 
	//  'apply_to_ring' algorithm is set up. There would be a bias 
	//  towards one edge of the ring, if we did. We apply to concentric 
	//  thin rings instead.
	else {
	    auto_repair_from_here(unit->x, unit->y);
	    for (i = 1; (i <= cv__auto_repaired_range_max[u]) && !stop_apply; 
		 ++i)
		apply_to_ring(unit->x, unit->y, i, i, auto_repair_from_here);
	}
	// Inform the player if the unit's HP changed.
	if (unit->hp != hpold)
	    update_unit_display(unit->side, unit, TRUE);
    }
}

static void
auto_repair_from_in(Unit *transport)
{
    using namespace Xconq;

    Unit *occ = NULL, *repairee = NULL, *transport2 = NULL;
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(in_play(transport),
		 "Auto Repair: Attempted to repair within an out-of-play unit");
    repairee = auto_repair_unit;
    assert_error(in_play(repairee),
		 "Auto Repair: Attempted to repair an out-of-play unit");
    // Useful info.
    u3 = repairee->type;
    // Any repair necessary?
    if (repairee->hp == u_hp(u3)) {
	stop_apply = TRUE;
	return;
    }
    // Iterate over all occs.
    for_all_occupants(transport, occ) {
	u2 = occ->type;
	// Use occs to repair with first.
	// QUESTION: Should we disallow occs to repair their friends 
	//  from inside units belonging to untrusted sides?
	if (occ->occupant) {
	    // If auto-repair range < 0, then restrict to transport-occ chain.
	    if (0 > uu_auto_repair_range(u2, u3)) {
		// If occ is repairee...
		if (occ == repairee)
		    auto_repair_from_in(occ);
		else {
		    // If occ is in transport chain of repairee...
		    for (transport2 = repairee->transport; transport2; 
			 transport2 = transport2->transport) {
			if (transport2 == occ) {
			    auto_repair_from_in(occ);
			    break;
			}
		    }
		    // If repairee is in transport chain of occ...
		    if (!transport2) {
			for (transport2 = transport; transport2;
			     transport2 = transport2->transport) {
			    if (transport2 == repairee) {
				auto_repair_from_in(occ);
				break;
			    }
			}
		    }
		}
	    }
	    // Else, try the auto-repair.
	    else
		auto_repair_from_in(occ);
	}
	// Skip untrusted units.
	if (!trusted_side(occ->side, repairee->side))
	    continue;
	// Skip repairee, if already done.
	if ((occ == repairee) && (0 < u_hp_recovery(u2)))
	    continue;
	// Auto-repair, if possible.
	if (valid(check_repair_action(occ, occ, repairee)))
	    do_repair_action(occ, occ, repairee);
    } // for all occupants
}

static void
auto_repair_from_here(int x, int y)
{
    using namespace Xconq;

    Unit *repairer = NULL, *repairee = NULL;
    int u3 = NONUTYPE;

    repairee = auto_repair_unit;
    assert_error(in_play(repairee),
		 "Auto Repair: Attempted to repair an out-of-play unit");
    // Useful info.
    u3 = repairee->type;
    // Any repair necessary?
    if (repairee->hp == u_hp(u3)) {
	stop_apply = TRUE;
	return;
    }
    // Iterate through cell ustack.
    for_all_stack(x, y, repairer) {
	// Use occs to repair with first.
	// QUESTION: Should we allow occs to repair their friends 
	//  from inside units belonging to untrusted sides?
	if (repairer->occupant)
	    auto_repair_from_in(repairer);
	// Skip untrusted units.
	if (!trusted_side(repairer->side, repairee->side))
	    continue;
	// Skip repairee, if already done.
	if ((repairer == repairee) && (0 < u_hp_recovery(repairer->type)))
	    continue;
	// Auto-repair, if possible.
	if (valid(check_repair_action(repairer, repairer, repairee)))
	    do_repair_action(repairer, repairer, repairee);
    } // for all cell ustack
}

/* Some types of units recover lost morale spontaneously. */

static void
run_morale_recovery(void)
{
    int u, moralerecovery, moralemax;
    Unit *unit;

    /* Precompute whether any units ever recover morale. */
    if (any_morale_recovery < 0) {
	any_morale_recovery = FALSE;
    	for_all_unit_types(u) {
	    if (u_morale_recovery(u) > 0) {
		any_morale_recovery = TRUE;
		break;
	    }
	}
	Dprintf("Any morale recovery: %d\n", any_morale_recovery);
    }
    if (!any_morale_recovery)
      return;
    Dprintf("Running morale recovery\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    u = unit->type;
	    moralerecovery = u_morale_recovery(u);
	    moralemax = u_morale_max(u);
	    if (moralerecovery > 0 && unit->morale < moralemax) {
		/* Change the morale (using a routine that does all the
		   notification). */
		change_morale(unit, 1, prob_fraction(moralerecovery));
	    }
	}
    }
}

/* Auto upgrade/downgrade any units that can do so. */

static void
run_auto_change_types (void)
{
    Unit *unit = NULL;
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;

    if (!any_auto_change_types)
        return;
    Dprintf("Running auto unit type changes.\n");
    for_all_units(unit) {
        if (!is_active(unit))
          continue;
        u = unit->type;
        side = unit->side;
        if (is_unit_type(u2 = u_auto_upgrade_to(u)) 
            && (A_ANY_OK == check_change_type_action(unit, unit, u2))) {
	    /* If unit is in transport. */
	    if (unit->transport) {
		if (type_can_occupy_without(u2, unit->transport, unit)) {
		    notify(side, "%s changes into a %s.", 
			   unit_handle(side, unit), u_type_name(u2));
		    change_unit_type(unit, u2, H_UNIT_TYPE_CHANGED, NULL);
		    continue;
		}
		else {
		    notify(side, 
"%s could have changed into a %s, but there was no room.",
			   unit_handle(side, unit), u_type_name(u2));
		    continue;
		}
	    }
	    /* Else, unit is directly in the cell. */
	    else {
		if (type_can_occupy_cell_without(u2, unit->x, unit->y, unit)) {
		    notify(side, "%s changes into a %s.", 
			   unit_handle(side, unit), u_type_name(u2));
		    change_unit_type(unit, u2, H_UNIT_TYPE_CHANGED, NULL);
		    continue;
		}
		else {
		    notify(side, 
"%s could have changed into a %s, but there was no room.",
			   unit_handle(side, unit), u_type_name(u2));
		    continue;
		}
	    }
        }
    }
    return;
}

/* Decide what happens to units belonging to sides that have lost.
   Not all units will necessarily disappear right away, so this runs
   at each turn. */

static void lost_unit_surrender(Unit *unit);
static void maybe_surrender_lost_to(Unit *unit, Unit *unit2);

static void
run_unit_fates(void)
{
    int u, u1, u2, chance, anylost;
    Side *side, *origside;
    Unit *unit;

    if (any_lost_vanish < 0) {
	any_lost_vanish = FALSE;
	for_all_unit_types(u1) {
	    if (u_lost_vanish(u1) > 0) {
		any_lost_vanish = TRUE;
		break;
	    }
	}
    }
    if (any_lost_wreck < 0) {
	any_lost_wreck = FALSE;
	for_all_unit_types(u1) {
	    if (u_lost_wreck(u1) > 0) {
		any_lost_wreck = TRUE;
		break;
	    }
	}
    }
    if (any_lost_surrender < 0) {
	any_lost_surrender = FALSE;
	for_all_unit_types(u1) {
	    for_all_unit_types(u2) {
		if (uu_lost_surrender(u1, u2) > 0) {
		    any_lost_surrender = TRUE;
		    if (any_lost_surrenders == NULL)
		      any_lost_surrenders = (char *)xmalloc(numutypes);
		    any_lost_surrenders[u1] = TRUE;
		    break;
		}
	    }
	}
    }
    if (any_lost_revolt < 0) {
	any_lost_revolt = FALSE;
	for_all_unit_types(u1) {
	    if (u_lost_revolt(u1) > 0) {
		any_lost_revolt = TRUE;
		break;
	    }
	}
    }
    if (!(any_lost_vanish
	  || any_lost_wreck
	  || any_lost_surrender
	  || any_lost_revolt))
      return;
    /* Don't kick in until at least one side has lost. */
    anylost = FALSE;
    for_all_sides(side) {
	if (side_lost(side)) {
	    anylost = TRUE;
	    break;
	}
    }
    if (!anylost)
      return;
    /* Although it would be more efficient to only scan units on sides
       that have lost, units may be changing sides, so per-side list
       scanning will lose. */
    for_all_units(unit) {
	if (side_lost(unit->side)) {
	    if (in_play(unit)) {
		u = unit->type;
		if (any_lost_wreck) {
		    chance = u_lost_wreck(u);
		    if (chance > 0 && xrandom(10000) < chance) {
			wreck_unit(unit, H_UNIT_WRECKED, WRECK_TYPE_SIDE_LOST, 
				   -1, NULL);
		    }
		}
	    }
	    if (in_play(unit)) {
		u = unit->type;
		if (any_lost_vanish) {
		    chance = u_lost_vanish(u);
		    if (chance > 0 && xrandom(10000) < chance) {
			kill_unit(unit, H_UNIT_VANISHED);
		    }
		}
	    }
	    if (in_play(unit)) {
		origside = unit->side;
		if (any_lost_surrender && any_lost_surrenders[u]) {
		    lost_unit_surrender(unit);
		}
		if (any_lost_revolt && unit->side == origside) {
		    chance = u_lost_revolt(u);
		    if (chance > 0 && xrandom(10000) < chance) {
			unit_revolt(unit, TRUE);
		    }
		}
	    }
	}
    }
}

static void
lost_unit_surrender(Unit *unit)
{
    int u = unit->type, dir, x1, y1;
    Unit *unit2;

    for_all_stack(unit->x, unit->y, unit2) {
	if (in_play(unit2)
	    && unit2->side != unit->side
	    && uu_lost_surrender(u, unit2->type) > 0
	    && visible_to(unit, unit2)) {
	    maybe_surrender_lost_to(unit, unit2);
	}
    }
    /* Check on adjacent units. */
    for_all_directions(dir) {
	if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
	    for_all_stack(x1, y1, unit2) {
		if (in_play(unit2)
		    && unit2->side != unit->side
		    && uu_surrender_chance(u, unit2->type) > 0
		    && visible_to(unit, unit2)) {
		    maybe_surrender_lost_to(unit, unit2);
		}
	    }
	}
    }
}

static void
maybe_surrender_lost_to(Unit *unit, Unit *unit2)
{
    int chance;

    chance = uu_lost_surrender(unit->type, unit2->type);
    if (xrandom(10000) < chance) {
	capture_unit(unit, unit2, H_UNIT_SURRENDERED);
    }
}

static void
run_detonation_accidents(void)
{
    int u, t, x, y, z, chance;
    Unit *unit;

    /* Precompute whether accidental detonation can ever occur. */
    if (any_detonation_accidents < 0) {
	any_detonation_accidents = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_detonation_accident(u, t) > 0) {
		    any_detonation_accidents = TRUE;
		    break;
		}
	    }
	    if (any_detonation_accidents)
	      break;
	}
	Dprintf("Any detonation accidents: %d\n", any_detonation_accidents);
    }
    if (!any_detonation_accidents)
      return;
    Dprintf("Running detonation accidents\n");
    for_all_units(unit) {
	if (in_play(unit) && completed(unit)) {
	    x = unit->x;  y = unit->y;  z = unit->z;
	    t = terrain_at(x, y);
	    /* (should account for being an occupant?) */
	    chance = ut_detonation_accident(unit->type, t);
	    if (chance > 0 && xrandom(10000) < chance) {
		/* Detonate the unit right where it is. */
		detonate_unit(unit, x, y, z);
		reckon_damage_around(x, y, detonate_urange_max(), unit);
	    }
	}
    }
}

/* Take care of details that no longer require any interaction, at least
   none that can't wait until the next turn. */

static void
finish_movement(void)
{
    int lostacp;
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	if (Debug) {
	    lostacp = 0;
	    for_all_side_units(side, unit) {
		if (is_active(unit) && unit->act && unit->act->acp > 0) {
		    /* Note that we don't count acp-min here, would be
		       way too confusing. */
		    lostacp += unit->act->acp;
		}
	    }
	    if (lostacp > 0) {
		Dprintf("%s forfeited %d acp overall.\n",
			side_desig(side), lostacp);
	    }
	}
    }
    for_all_sides(side)  {
	update_side_display_all_sides(side, TRUE);
    }
}

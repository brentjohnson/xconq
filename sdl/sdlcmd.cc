/* Commands for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "conq.h"
#include "kpublic.h"
#include "sdlconq.h"
#include "print.h"
#include "aiunit.h"
#include "aiunit2.h"

void really_do_design(Side *side);

static void aux_add_terrain(Screen *screen, int cancel);
static void aux_add_terrain_2(Screen *screen, int cancel);
static void do_add_terrain_2(Screen *screen, int t);
static void aux_attack(Screen *screen, int cancel);
static void aux_build(Screen *screen, int cancel);
static void aux_build_2(Screen *screen, int cancel);
static void aux_change_type(Screen *screen, int cancel);
static void aux_distance(Screen *screen, int cancel);
static void aux_fire_at(Screen *screen, int cancel);
static void common_fire_at(Unit *unit, int x, int y);
static void aux_fire_into(Screen *screen, int cancel);
static void common_fire_into(Unit *unit, int x, int y);
static void aux_give_unit(Screen *screen, int cancel);
static void aux_move_dir(Screen *screen, Unit *unit);
static void aux_message(Screen *screen, int cancel);
static void aux_move_to(Screen *screen, int cancel);
static void aux_name(Screen *screen, int cancel);
static void aux_others(Screen *screen, int cancel);
static void aux_remove_terrain(Screen *screen, int cancel);
static void aux_remove_terrain_2(Screen *screen, int cancel);
static void do_remove_terrain_2(Screen *screen, int t);
static void aux_quit_resign(Screen *screen, int cancel);
static void aux_quit_resign_to(Screen *screen, int cancel);
static void aux_quit_exit(Screen *screen, int cancel);
static void aux_quit_save(Screen *screen, int cancel);
static void aux_quit_leave(Screen *screen, int cancel);
static void aux_resign(Screen *screen, int cancel);
static void aux_resign_to(Screen *screen, int cancel);
static void aux_set_formation(Screen *screen, int cancel);

static void aux_design(Screen *screen, int cancel);

/* Use these functions to check commands' preconditions. */

static int
during_game_only(void)
{
    if (endofgame) {
	cmd_error(dside, "Cannot do after game is over!");
	return FALSE;
    }
    return TRUE;
}

static int
require_unit(Screen *screen)
{
    if (screen == NULL) {
	beep();
	return FALSE;
    }
    if (!in_play(screen->curunit) || screen->curunit_id != screen->curunit->id) {
	screen->curunit = NULL;
	screen->curunit_id = 0;
	cmd_error(dside, "No current unit to command!");
	return FALSE;
    }
    return TRUE;
}

static int
require_own_unit_during_game(Screen *screen)
{
    if (!during_game_only())
      return FALSE;
    if (!require_unit(screen))
      return FALSE;
    if (!side_controls_unit(dside, screen->curunit)) {
	cmd_error(dside, "The unit is not yours to command!");
	return FALSE;
    }
    return TRUE;
}

/* This function tests that a unit that was originally specified as
   part of a command is still usable.  The unit may have died or
   changed sides since the command was first given. */

static int
unit_still_ok(Unit *unit)
{
    if (!in_play(unit)) {
	/* Note that this is not an error, because the player didn't
	   do anything wrong. */
	notify(dside, "Unit is gone!");
	return FALSE;
    }
    if (!side_controls_unit(dside, unit)) {
	notify(dside, "%s is no longer yours to command!",
	       unit_handle(dside, unit));
	return FALSE;
    }
    return TRUE;
}

static UnitVector *selvec;

UnitVector *
get_selected_units(Side *side)
{
    Screen *screen = ui->curscreen;

    if (selvec == NULL)
      selvec = make_unit_vector(2);
    clear_unit_vector(selvec);
    if (screen && in_play(screen->curunit) && screen->curunit_id == screen->curunit->id)
      selvec = add_unit_to_vector(selvec, screen->curunit, 0);
    return selvec;
}

int designed_on = FALSE;

/* Definitions of all the command functions. */

void
do_add_terrain(Side *side)
{
    Screen *screen = ui->curscreen;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;
    u = unit->type;
    numtypes = 0;
    for_all_terrain_types(t) {
	if (ut_acp_to_add_terrain(u, t) > 0) {
	    tmp_t_array[t] = TRUE;
	    ++numtypes;
	    tfirst = t;
	} else {
	    tmp_t_array[t] = FALSE;
	}
    }
    if (numtypes == 0) {
	cmd_error(dside, "%s cannot add or alter terrain!",
		  unit_handle(dside, unit));
    } else if (numtypes == 1) {
	screen->argunitid = unit->id;
	do_add_terrain_2(screen, tfirst);
    } else {
	screen->argunitid = unit->id;
	ask_terrain_type(screen, "Type to add:", tmp_t_array, aux_add_terrain);
    }
}

static void
aux_add_terrain(Screen *screen, int cancel)
{
    int t;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Type a key to select terrain type.");
    }
    if (grok_terrain_type(screen, &t)) {
	if (t != NONTTYPE) {
	    do_add_terrain_2(screen, t);
	}
    } else {
        if (screen->inpch != '?')
	  beep();
	/* Stay in this mode until we get it right. */
	screen->modalhandler = aux_add_terrain;
    }
}

/* This is like do_add_terrain, but with a terrain type given. */

static void
do_add_terrain_2(Screen *screen, int t)
{
    char abuf[100];

    screen->tmpt = t;
    sprintf(abuf, "Add %s where?", t_type_name(t));
    ask_position(screen, abuf, aux_add_terrain_2);
}

static void
aux_add_terrain_2(Screen *screen, int cancel)
{
    int x, y, dir;
    Unit *unit;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Pick a location to add %s.", t_type_name(screen->tmpt));
	screen->modalhandler = aux_add_terrain_2;
	return;
    }
    if (grok_position(screen, &x, &y, NULL)) {
	if (in_area(x, y)) {
	    unit = find_unit(screen->argunitid);
	    if (!unit_still_ok(unit))
	      return;
	    switch (t_subtype(screen->tmpt)) {
	      case cellsubtype:
		net_prep_alter_cell_action(unit, unit, x, y, screen->tmpt);
		break;
	      case bordersubtype:
	      case connectionsubtype:
		if (x == unit->x && y == unit->y) {
		    beep();
		    notify(dside, "Pick an adjacent location.");
		    screen->modalhandler = aux_add_terrain_2;
		}
		dir = closest_dir(x - unit->x, y - unit->y);
		net_prep_add_terrain_action(unit, unit, unit->x, unit->y,
					    dir, screen->tmpt);
		break;
	      case coatingsubtype:
		net_prep_add_terrain_action(unit, unit, unit->x, unit->y,
					    1, screen->tmpt);
		break;
	    }
	}
    } else {
	beep();
	screen->modalhandler = aux_add_terrain_2;
    }
}

void
do_attack(Side *side)
{
    Screen *screen = ui->curscreen;
    Unit *unit = screen->curunit;
    
    if (!require_own_unit_during_game(screen))
      return;
    screen->argunitid = unit->id;
    ask_position(screen, "Attack where?", aux_attack);
}

static void
aux_attack(Screen *screen, int cancel)
{
    int x, y;
    Unit *unit, *unit2;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Click on a location");
    }
    if (grok_position(screen, &x, &y, &unit2)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (unit2 != NULL) {
	    net_prep_attack_action(unit, unit, unit2, 100);
	} else {
	    cmd_error(dside, "Nothing to attack!");
	}
    } else {
        if (screen->inpch != '?')
	  beep();
	screen->modalhandler = aux_attack;
    }
}

void
do_build(Side *side)
{
    Screen *screen = ui->curscreen;
    int u, u2, numtypes, ufirst = NONUTYPE;
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;
    u = unit->type;
    if (!can_build(unit)) {
	cmd_error(dside, "%s can't build anything!", unit_handle(dside, unit));
	return;
    }
    numtypes = 0;
    for_all_unit_types(u2) {
	if (unit_can_build_type(unit, u2)) {
	    tmp_u_array[u2] = TRUE;
	    ++numtypes;
	    ufirst = u2;
	} else {
	    tmp_u_array[u2] = FALSE;
	}
    }
    if (unit->transport != NULL
	&& !uu_occ_can_build(unit->transport->type, u)
	&& !(numtypes == 1
	     && !completed(unit->transport)
	     && uu_acp_to_build(u, unit->transport->type) > 0)) {
	cmd_error(dside, "%s can't build anything while inside another unit!",
		  unit_handle(dside, unit));
	return;
    }
    switch (numtypes) {
      case 0:
	cmd_error(dside, "Nothing to build right now!");
	break;
      case 1:
	/* Only one type to build - do it. */
	maybe_ask_construct_location(screen, unit, ufirst);
	break;
      default:
	/* Player has to choose a type to build. */
	screen->argunitid = unit->id;
	ask_unit_type(screen, "Type to build:", tmp_u_array, aux_build);
	break;
    }
}

void
maybe_ask_construct_location(Screen *screen, Unit *unit, int u2)
{
    int u = NONUTYPE;
    char *msg = NULL;
    Unit *transport = NULL;

    assert_error(screen, "Attempted to access a NULL screen");
    assert_error(unit, "Attempted to manipulate a NULL unit");
    assert_error(is_unit_type(u2), 
		 "Attempted to construct an illegal unit type");
    u = unit->type;
    /* If the unit is mobile or can perform a ranged create. */
    if (u_mobile(u) || (0 < uu_create_range(u, u2))) {
	screen->argunitid = unit->id;
	screen->inptype = u2;
	msg = (char *)xmalloc(80 + strlen(u_type_name(u2)));
	sprintf(msg, "Build %s where?", u_type_name(u2));
	ask_position(screen, msg, aux_build_2);
	free(msg);
    /* Else attempt to create at unit's location. */
    } 
    else {
	if (valid(can_create_in(unit, unit, u2, unit))
	    && impl_build(
		dside, unit, u2, unit, unit->x, unit->y, screen->prefixarg)) {
	    reset_screen_input(screen);
	}
	else if (valid(can_create_at(unit, unit, u2, unit->x, unit->y))
		 && impl_build(
			dside, unit, u2, NULL, unit->x, unit->y,
			screen->prefixarg)) {
	    reset_screen_input(screen);
	}
    }
}

static void
aux_build(Screen *screen, int cancel)
{
    int u2;
    Unit *unit;

    if (cancel) {
	cancel_unit_type(screen);
	return;
    }
    if (screen->inpch == '?') {
	notify(dside, "Type a key or click on a unit type to select build (or hit <esc> to cancel).");
    }
    if (grok_unit_type(screen, &u2)) {
	/* Escape silently if the player cancelled. */
	if (u2 == NONUTYPE)
	  return;
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (mobile(unit->type)
	    || uu_create_range(unit->type, u2) > 0) {
	    /* For mobile and/or at-a-distance builders, also ask
	       where to build. */
	    char *msg;
	    screen->inptype = u2;
	    /* Provide feedback about which unit type the user picked.  */
	    msg = (char *)xmalloc(80 + strlen (u_type_name (u2)));
	    sprintf (msg, "Build %s where?", u_type_name (u2));
	    ask_position(screen, msg, aux_build_2);
	    free (msg);
	    return;
	}
	else {
	    if (valid(can_create_in(unit, unit, u2, unit))
		&& impl_build(
		    dside, unit, u2, unit, unit->x, unit->y, 
		    screen->prefixarg)) {
		reset_screen_input(screen);
	    }
	    else if (valid(can_create_at(unit, unit, u2, unit->x, unit->y))
		     && impl_build(
			    dside, unit, u2, NULL, unit->x, unit->y,
			    screen->prefixarg)) {
		reset_screen_input(screen);
	    }
	}
    } else {
	if (screen->inpch != '?') {
	    beep();
	    screen->modalhandler = aux_build;
	}
    }
}

static void
aux_build_2(Screen *screen, int cancel)
{
    int x, y;
    Unit *unit = NULL, *transport = NULL;

    if (cancel) {
	return;
    }
    if (screen->inpch == '?') {
	notify(dside, "Pick a location where you would like to build (or hit <esc> to cancel).");
	screen->modalhandler = aux_build_2;
	return;
    }
    if (grok_position(screen, &x, &y, &transport)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (impl_build(
		dside, unit, screen->inptype, transport, x, y, 
		screen->prefixarg)) {
	    reset_screen_input(screen);
	} else {
	    beep();
	    screen->modalhandler = aux_build_2;    
	}
    } else {
	notify(dside, "Invalid position. Please pick a new location.");
	beep();
	screen->modalhandler = aux_build_2;    
    }
}

void
do_change_type(Side *side)
{
    Screen *screen = ui->curscreen;
    int u = NONUTYPE, u2 = NONUTYPE, numtypes = 0, ufirst = NONUTYPE;
    Unit *unit = NULL;

    if (screen && screen->curunit)
      unit = screen->curunit;
    else
      return;
    if (!require_own_unit_during_game(screen))
      return;
    if (unit)
      u = unit->type; 
    else
      return;
    if (unit->side != dside) {
        cmd_error(dside, "%s is not your unit!", unit_handle(dside, unit));
        return;
    }
    for_all_unit_types(u2) {
        if (uu_acp_to_change_type(unit->type, u2)) {
            tmp_u_array[u2] = TRUE;
            ++numtypes;
            ufirst = u2;
        }
        else {
            tmp_u_array[u2] = FALSE;
        }
    }
    switch (numtypes) {
      case 0:
        cmd_error(dside, "Cannot change to any type right now!");
        break;
      case 1:
        /* Only one possible type to change to. Change to it. */
        if (!impl_change_type(dside, unit, ufirst)) {
            screen->inptype = NONUTYPE;
            return;
        }
        break;
      default:
        screen->argunitid = unit->id;
        ask_unit_type(screen, "Type to change into:", tmp_u_array,
                      aux_change_type);
        break;
    }
}

static void
aux_change_type(Screen *screen, int cancel)
{
    int u2 = NONUTYPE;
    Unit *unit = NULL;

    if (cancel) {
        cancel_unit_type(screen);
        return;
    }
    if (screen->inpch == '?') {
        notify(dside, "Type a key or click in the unit list to select new type (or hit <esc> to cancel).");
    }
    if (grok_unit_type(screen, &u2)) {
        /* Escape silently if the player cancelled. */
        if (u2 == NONUTYPE)
          return;
        unit = find_unit(screen->argunitid);
        if (!unit_still_ok(unit))
          return;
        if (!impl_change_type(dside, unit, u2)) {
            screen->inptype = NONUTYPE;
            return;
        }
    }
    else {
        if (screen->inpch != '?')
          beep();
        screen->modalhandler = aux_change_type;
    }
}

void
do_collect(Side *side)
{
    Screen *screen = ui->curscreen;
    Unit *unit = screen->curunit;
    int mtocollect;
    char *arg;
    const char *rest;

    if (!require_own_unit_during_game(screen))
      return;
    mtocollect = NONMTYPE;
    if (nummtypes == 0) {
	cmd_error(dside, "No materials to collect");
	return;
    }
    if (!empty_string(cmdargstr)) {
	rest = get_next_arg(cmdargstr, tmpbuf, &arg);
	mtocollect = mtype_from_name(arg);
    } else {
	cmd_error(dside, "No material name given");
	return;
    }
    if (!is_material_type(mtocollect)) {
	cmd_error(dside, "`%s' is not a recognized material name", arg);
	return;
    }
    if (unit->plan)
      net_set_collect_task(unit, mtocollect, unit->x, unit->y);
}

void
do_copying(Side *side)
{
    popup_help(dside, copying_help_node);
}

void
do_dir(Side *side)
{
    Screen *screen = ui->curscreen;

    if (!require_own_unit_during_game(screen))
      return;
    aux_move_dir(screen, screen->curunit);
}

void
do_dir_multiple(Side *side)
{
    Screen *screen = ui->curscreen;

    if (!require_own_unit_during_game(screen))
      return;
    screen->prefixarg = 9999;
    aux_move_dir(screen, screen->curunit);
}

/* Handle both single and multiple moves in the direction given by
   tmpkey. */

static void
aux_move_dir(Screen *screen, Unit *unit)
{
    int ndirs, dir, n = screen->prefixarg, x, y;
    HistEventType reason;
    char failbuf[BUFSIZE];

    if (!unit->act || !unit->plan) { /* use a more sophisticated test? */
	/* ??? can't act ??? */
	return;
    }
    ndirs = char_to_dir(tmpkey, &dir, NULL, NULL);
    if (ndirs < 1) {
	/* (would never occur in real life though...) */
	cmd_error(dside, "what direction is that?!?");
	return;
    }
    if (n > 1) {
	DGprintf("Ordering %s to move %d %s\n",
		 unit_desig(unit), n, dirnames[dir]);
	net_set_move_dir_task(unit, dir, n);
    } else {
	/* Moving directly into an adjacent cell, do now instead of
	   making a task. */
	if (!point_in_dir(unit->x, unit->y, dir, &x, &y)) {
	    /* (but what if leaving the world is allowed?) */
	    beep();
	    return;
	}
	if (!advance_into_cell(dside, unit, x, y, unit_at(x, y), &reason)) {
	    advance_failure_desc(failbuf, unit, reason);
	    notify(dside, "%s", failbuf);
	    beep();
	}
    }
}

void
do_distance(Side *side)
{
    Screen *screen = ui->curscreen;

    if (!require_unit(screen))
      return;
    screen->argunitid = screen->curunit->id;
    ask_position(screen, "Distance to where?", aux_distance);
}

static void
aux_distance(Screen *screen, int cancel)
{
    int x, y, dist;
    Unit *unit;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Pick a location to which you want the distance.");
	screen->modalhandler = aux_distance;
	return;
    }
    if (grok_position(screen, &x, &y, NULL)) {
	unit = find_unit(screen->argunitid);
	/* We don't care about the unit's ownership. */
	if (in_area(x, y) && unit != NULL) {
	    dist = distance(unit->x, unit->y, x, y);
	    notify(dside, "Distance from the current unit is %d cells.", dist);
	} else {
	    cmd_error(dside, "Measurement failed.");
	}
    } else {
        beep();
	screen->modalhandler = aux_distance;
    }
}

void
do_escape(Side *side)
{
}

void
do_fire(Side *side)
{
    Screen *screen = ui->curscreen;
    int sx, sy, x, y;
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;

    if (screen->inpch == '\0') {
	screen->argunitid = unit->id;
	ask_position(screen, "Fire at what unit?", aux_fire_at);
    } else {
	sx = screen->inpsx;  sy = screen->inpsy;
	if (nearest_cell(screen->map->main_vp, sx, sy, &x, &y, NULL, NULL)) {
	    common_fire_at(unit, x, y);
	} else {
	    cmd_error(dside, "Cannot fire outside the world!");
	}
    }
}

static void
aux_fire_at(Screen *screen, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(screen, &x, &y, NULL)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	common_fire_at(unit, x, y);
    } else {
	screen->modalhandler = aux_fire_at;
    }
}

static void
common_fire_at(Unit *unit, int x, int y)
{
    int rslt;
    Unit *other;

    /* (should only be able to target unit if covered...) */
    other = unit_at(x, y);
    if (other == NULL) {
	cmd_error(dside, "Nothing there to fire at!");
    } else if (other == unit) {
	cmd_error(dside, "You can't fire at yourself!");
    } else if (other->side == unit->side) {
	cmd_error(dside, "You can't fire at one of your own units!");
    } else {
	rslt = check_fire_at_action(unit, unit, other, -1);
	if (valid(rslt)) {
	    net_prep_fire_at_action(unit, unit, other, -1);
	} else {
	    /* (should say which unit was target) */
	    cmd_error(dside, "%s fire at unit not valid: %s",
		      unit_handle(dside, unit),
		      action_result_desc(rslt));
	}
    }
}

void
do_fire_into(Side *side)
{
    Screen *screen = ui->curscreen;
    int x = screen->inpx, y = screen->inpy;
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;

    if (screen->inpch == '\0') {
	screen->argunitid = unit->id;
	ask_position(screen, "Fire into which location?", aux_fire_into);
    } else {
	common_fire_into(unit, x, y);
    }
}

static void
aux_fire_into(Screen *screen, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(screen, &x, &y, NULL)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	common_fire_into(unit, x, y);
    } else {
	screen->modalhandler = aux_fire_into;
    }
}

static void
common_fire_into(Unit *unit, int x, int y)
{
    int rslt;

    if (!inside_area(x, y)) {
	cmd_error(dside, "Cannot fire outside the world!");
	return;
    }
    rslt = check_fire_into_action(unit, unit, x, y, 0, -1);
    if (valid(rslt)) {
	net_prep_fire_into_action(unit, unit, x, y, 0, -1);
    } else {
	cmd_error(dside, "%s fire into %d,%d not valid: %s",
		  unit_handle(dside, unit), x, y, action_result_desc(rslt));
    }
}

void
do_flash(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

/* Toggle the "follow-action" flag. */

void
do_follow_action(Side *side)
{
    Screen *screen = ui->curscreen;

    if (screen == NULL)
      return;
    if (!during_game_only())
      return;
    screen->follow_action = !screen->follow_action;
    if (screen->follow_action) {
	notify(dside, "Following the action on screen.");
    } else {
	notify(dside, "Not following the action on screen.");
    }
}

/* Give a unit to another side or make it independent. */

/* (but giving to indep should be tested, otherwise might kill unit) */

static void really_do_give_unit(Unit *unit, Side *side2);

void
do_give_unit(Side *side)
{
    Screen *screen = ui->curscreen;
    Side *side2;

    if (!require_own_unit_during_game(screen))
      return;
    if (between(0, screen->prefixarg, numsides)) {
	/* Interpret a prefix as the recipient side. */
	side2 = side_n(screen->prefixarg);
	really_do_give_unit(screen->curunit, side2);
    } else {
	ask_side(screen, "To whom do you wish to give the unit?", NULL,
		 aux_give_unit);
    }
}

static void
aux_give_unit(Screen *screen, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(screen, &side2)) {
	really_do_give_unit(screen->curunit, side2);
    } else {
	/* Iterate until grok_side is happy. */
	screen->modalhandler = aux_give_unit;
    }
}

static void
really_do_give_unit(Unit *unit, Side *side2)
{
    if (unit->side == side2)
      return;
#ifdef DESIGNERS
    if (is_designer(dside)) {
	net_designer_change_side(unit, side2);
	return;
    }
#endif /* DESIGNERS */
    if (0 /* unit is a type that cannot act */) {
	/* (should add case for nonacting units) */
    } else {
	/* (should check validity!) */
	if (side2 != NULL && side2 != indepside) {
	    notify(dside, "You give %s to %s.",
		   unit_handle(dside, unit), short_side_title(side2));
	    net_prep_change_side_action(unit, unit, side2);
	} else {
	    notify(dside, "You give %s its independence.",
		   unit_handle(dside, unit));
	    net_prep_change_side_action(unit, unit, indepside);
	}
    }
}

void
do_help(Side *side)
{
    popup_help(dside, NULL);
}

/* In general, the subcommands of "screen" are for the GUI to use. */

void
do_map(Side *side)
{
    Screen *screen = ui->curscreen;
    VP *vp;

    if (empty_string(cmdargstr))
      return;
    if (screen == NULL)
      return;
    vp = screen->map->main_vp;
    if (strncmp(cmdargstr, "contour-interval=", 17) == 0) {
	if (strcmp("?", cmdargstr+17) == 0) {
	    notify(dside, "Contour interval is %d.", vp->contour_interval);
	} else {
	    int n = strtol(cmdargstr+17, NULL, 10);
	    set_contour_interval(vp, n);
	    redraw_screen(screen);
	}
    } else if (strcmp(cmdargstr, "iso") == 0) {
    } else if (strcmp(cmdargstr, "meridians") == 0) {
	vp->draw_meridians = !vp->draw_meridians;
	if (screen->prefixarg > 0)
	  set_meridian_interval(vp, screen->prefixarg);
	redraw_screen(screen);
    } else if (strcmp(cmdargstr, "rotl") == 0) {
	notify(dside, "Now looking %s.", dirnames[vp->isodir]);
    } else if (strcmp(cmdargstr, "rotr") == 0) {
	notify(dside, "Now looking %s.", dirnames[vp->isodir]);
    } else if (strcmp(cmdargstr, "show-all") == 0) {
	if (!side->may_set_show_all) {
	    cmd_error(dside, "You are not permitted to see everything!");
	    return;
	}
	set_show_all(screen, !screen->show_all);
	redraw_screen(screen);
    } else if (strcmp(cmdargstr, "unit-colorize") == 0) {
	screen->colorize_units = !screen->colorize_units;
	redraw_screen(screen);
    } else if (*cmdargstr == '!') {
    } else {
	cmd_error(dside, "Screen command \"%s\" not recognized!", cmdargstr);
    }
}

/* Send a short (1 line) message to another player.  Some messages are
   recognized specially, causing various actions. */

void
do_message(Side *side)
{
    Screen *screen = ui->curscreen;
    char prompt[BUFSIZE];
    Side *side2;

    if (screen == NULL)
      return;
    side2 = side_n(screen->prefixarg);
    if (dside == side2) {
	sprintf(prompt, "Message to yourself: ");
    } else if (side2) {
	sprintf(prompt, "Message to %s: ", short_side_title(side2));
    } else {
	sprintf(prompt, "Broadcast to all: ");
    }
    screen->argside = side2;
    ask_string(screen, prompt, NULL, aux_message);
}

static void
aux_message(Screen *screen, int cancel)
{
    char *msg;
    SideMask sidemask;

    if (cancel)
      return;
    if (grok_string(screen, &msg)) {
	if (empty_string(msg)) {
	    notify(dside, "You keep your mouth shut.");
	    sidemask = NOSIDES;
	} else if (screen->argside == NULL) {
	    notify(dside, "You broadcast to everybody.", msg);
	    sidemask = ALLSIDES;
	} else {
	    notify(dside, "You send the message.");
	    sidemask = add_side_to_set(screen->argside, NOSIDES);
	}
	if (!empty_string(msg) && sidemask != NOSIDES)
	  net_send_message(dside, sidemask, msg);
    } else {
	screen->modalhandler = aux_message;
    }
}

/* Set unit to move to a given location.  */

/* The command proper. */

void
do_move_to(Side *side)
{
    Screen *screen = ui->curscreen;
    Unit *unit = screen->curunit;

    if (!is_designer(side)) {
	if (!mobile(unit->type)) {
	    cmd_error(side, "%s cannot move at all!", unit_handle(side, unit));
	    return;
	}
    }
    if (!require_own_unit_during_game(screen))
      return;
    screen->argunitid = unit->id;
    ask_position(screen, "Move to where?", aux_move_to);
}

static void
aux_move_to(Screen *screen, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(screen, &x, &y, NULL)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit)) {
	    return;
	}
	if (impl_move_to(dside, unit, x, y, 0)) {
	    return;
	} else {
	    beep();
	    screen->modalhandler = aux_move_to;
	}
    } else {
	beep();
	notify(dside, "Invalid position. Please pick a new destination.");
	screen->modalhandler = aux_move_to;
    }
}

/* Name/rename the current unit. */

void
do_name(Side *side)
{
    Screen *screen = ui->curscreen;
    char tmpbuf[BUFSIZE];
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;
    screen->argunitid = unit->id;
    sprintf(tmpbuf, "New name for %s:", unit_handle(dside, unit));
    ask_string(screen, tmpbuf, unit->name, aux_name);
}

static void
aux_name(Screen *screen, int cancel)
{
    char *name;
    Unit *unit;

    if (cancel)
      return;
    if (grok_string(screen, &name)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	net_set_unit_name(dside, unit, name);
    } else {
	screen->modalhandler = aux_name;
    }
}

/* Create a new screen, of standard size and zoom. */
/* (should clone last screen in list perhaps?) */

void
do_new_map(Side *side)
{
    cmd_error(dside, "Cannot have more than one SDL screen");
}

/* This is a command to examine all occupants and suboccupants, in an
   inorder fashion. */

/* Should have an option to open up a list window that shows everything
   all at once. */

void
do_occupant(Side *side)
{
    Screen *screen = ui->curscreen;
    Unit *nextocc;

    if (!require_unit(screen))
      return;
    nextocc = find_next_occupant(screen->curunit);
    if (nextocc != screen->curunit)
      set_current_unit(screen, nextocc);
    else
      beep();
}

void
do_orders_popup(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

void
do_other(Side *side)
{
    Screen *screen = ui->curscreen;

    if (screen == NULL)
      return;
    ask_string(screen, "Command:", "", aux_others);
}

static void
aux_others(Screen *screen, int cancel)
{
    char *cmd;

    if (cancel)
      return;
    if (grok_string(screen, &cmd)) {
	if (empty_string(cmd)) {
	    notify(dside, "No command.");
	} else if (strcmp(cmd, "?") == 0) {
#if 0
	    do_help(dside);
	    /* (should do with special jump routine) */
	    ui->curhelpnode = long_commands_help_node;
	    update_help(dside);
#endif
	} else {
	    execute_long_command(dside, cmd);
	}
    } else {
	screen->modalhandler = aux_others;
    }
}

void
do_print_view(Side *side)
{  
    double conv;
    PrintParameters *ps_pp;

    ps_pp = (PrintParameters *) xmalloc(sizeof(PrintParameters));

    init_ps_print(ps_pp);

    /* convert to cm or in */
    if (ps_pp->cm) {
	conv = 72 / 2.54;
    } else {
	conv = 72;
    }
    ps_pp->cell_size /= conv;
    ps_pp->cell_grid_width /= conv;
    ps_pp->border_width /= conv;
    ps_pp->connection_width /= conv;
    ps_pp->page_width /= conv;
    ps_pp->page_height /= conv;
    ps_pp->top_margin /= conv;
    ps_pp->bottom_margin /= conv;
    ps_pp->left_margin /= conv;
    ps_pp->right_margin /= conv;

    dump_ps_view(dside, ps_pp, "view.ps");
}

void
do_produce(Side *side)
{
    int m, n;
    Screen *screen = ui->curscreen;
    Unit *unit = screen->curunit;

    if (!require_unit(screen))
	return;

    if (!can_produce(unit)) {
	cmd_error(dside, "cannot do active production");
    }
    n = 9999;
    if (screen->prefixarg > 0)
	n = screen->prefixarg;
    /* Find the first produceable type and set up to produce it. */
    for_all_material_types(m) {
	if (um_acp_to_produce(unit->type, m) > 0) {
	    net_push_produce_task(unit, m, n);
	    return;
	}
    }
}

void
do_quit(Side *side)
{
    Screen *screen = ui->curscreen;

    if (screen == NULL || endofgame || beforestart) {
	/* If the game is over or never started, nothing to test or confirm. */
	exit_xconq();
    } else if (dside->ingame) {
	if (gamestatesafe
	    || all_others_willing_to_quit(dside)
	    || is_designer(dside)) {
	    /* For the above cases, there is no obstacle to quitting,
	       but confirm anyway, in case there was a slip of the
	       keyboard. */
	    ask_bool(screen, "Do you really want to quit?", FALSE, 
		     aux_quit_exit);
	} else if (keeping_score()) {
	    ask_bool(screen, "Do you want to save the game before quitting?",
		     FALSE, aux_quit_save);
	} else {
	    /* Everybody is just participating. */
	    ask_bool(screen, "Do you want to leave this game?", FALSE,
		     aux_quit_leave);
	}
    } else {
	/* We're already out of the game, not really anything to confirm. */
	/* (is this common to all interfaces?) */
	if (all_others_willing_to_quit(dside) || num_active_displays() == 1)
	  exit_xconq();
    }
}

/* (Have an extra confirm for designers not to lose unsaved work?) */

static void
aux_quit_exit(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	exit_xconq();
    } else {
	/* Nothing to do if we said not to exit. */
    }
}

static void
aux_quit_save(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	do_save(dside);
	/* Don't exit unless we know we saved successfully. */
	if (gamestatesafe)
	  exit_xconq();
    } else {
	ask_bool(screen, "You cannot quit without resigning; give up now?",
		 FALSE, aux_quit_resign);
    }
}

static void
aux_quit_resign(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	if (numsides > 2) {
	    /* (should suggest resigning to a trusted side) */
	    ask_side(screen, "To whom do you wish to surrender?", NULL,
		     aux_quit_resign_to);
	} else {
	    net_resign_game(dside, NULL);
	    exit_xconq();
	}
    }
}

static void
aux_quit_resign_to(Screen *screen, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(screen, &side2)) {
	net_resign_game(dside, side2);
	exit_xconq();
    } else {
	/* Iterate until grok_side is happy. */
	screen->modalhandler = aux_quit_resign_to;
    }
}

/* Do the act of leaving the game. */

static void
aux_quit_leave(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	/* (should probably structure differently, but how?) */
	remove_side_from_game(dside);
	exit_xconq();
    } else {
	/* Nothing to do if we said not to exit. */
    }
}

/* Center the screen on the current location. */

void
do_recenter(Side *side)
{
    Screen *screen = ui->curscreen;

    if (!require_unit(screen))
      return;
    recenter(screen->map, screen->curunit->x, screen->curunit->y);
}

/* Redraw everything using the same code as when windows need a redraw. */

void
do_refresh(Side *side)
{
    Screen *screen;

    reset_coverage();
    reset_all_views();
    ui->legends = NULL;
    compute_all_feature_centroids();
    place_legends(dside);
#if 0
    update_contour_intervals();
#endif
    for_all_screens(screen)
      redraw_screen(screen);
}

void
do_remove_terrain(Side *side)
{
    Screen *screen = ui->curscreen;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = screen->curunit;

    if (!require_own_unit_during_game(screen))
      return;
    u = unit->type;
    numtypes = 0;
    for_all_terrain_types(t) {
	if (ut_acp_to_remove_terrain(u, t) > 0) {
	    tmp_t_array[t] = TRUE;
	    ++numtypes;
	    tfirst = t;
	} else {
	    tmp_t_array[t] = FALSE;
	}
    }
    if (numtypes == 0) {
	cmd_error(dside, "%s cannot remove terrain!", unit_handle(side, unit));
    } else if (numtypes == 1) {
	screen->argunitid = unit->id;
	do_remove_terrain_2(screen, tfirst);
    } else {
	screen->argunitid = unit->id;
	ask_terrain_type(screen, "Type to remove:",
			 tmp_t_array, aux_remove_terrain);
    }
}

static void
aux_remove_terrain(Screen *screen, int cancel)
{
    int t;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Type a key to select terrain type to remove.");
    }
    if (grok_terrain_type(screen, &t)) {
	if (t != NONTTYPE) {
	    do_remove_terrain_2(screen, t);
	}
    } else {
        if (screen->inpch != '?')
	  beep();
	/* Stay in this mode until we get it right. */
	screen->modalhandler = aux_remove_terrain;
    }
}

/* This is like do_remove_terrain, but with a terrain type given. */

static void
do_remove_terrain_2(Screen *screen, int t)
{
    screen->tmpt = t;
    ask_position(screen, "Remove where?", aux_remove_terrain_2);
}

static void
aux_remove_terrain_2(Screen *screen, int cancel)
{
    int x, y, dir;
    Unit *unit;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Pick a location to remove %s.", t_type_name(screen->tmpt));
	screen->modalhandler = aux_remove_terrain_2;
	return;
    }
    if (grok_position(screen, &x, &y, NULL)) {
	if (in_area(x, y)) {
	    unit = find_unit(screen->argunitid);
	    if (!unit_still_ok(unit))
	      return;
	    switch (t_subtype(screen->tmpt)) {
	      case cellsubtype:
		cmd_error(dside, "can't remove cell terrain!");
		break;
	      case bordersubtype:
	      case connectionsubtype:
		dir = closest_dir(x - unit->x, y - unit->y);
		net_prep_remove_terrain_action(unit, unit, unit->x, unit->y, dir, screen->tmpt);
		break;
	      case coatingsubtype:
		net_prep_remove_terrain_action(unit, unit, unit->x, unit->y, 1, screen->tmpt);
		break;
	    }
	}
    } else {
	beep();     
	screen->modalhandler = aux_remove_terrain_2;
    }
}

void
do_repair(Side *side)
{
    Screen *screen = ui->curscreen;
    Unit *unit, *other;

    if (!require_own_unit_during_game(screen))
      return;
    unit = screen->curunit;
    if (valid(check_repair_action(unit, unit, unit))) {
	net_prep_repair_action(unit, unit, unit);
	return;
    }
    other = unit->transport;
    if (other != NULL
	&& valid(check_repair_action(other, other, unit))) {
	net_prep_repair_action(other, other, unit);
	return;
    }
    cmd_error(dside, "No repair possible right now");
}

void
do_resign(Side *side)
{
    Screen *screen = ui->curscreen;

    if (screen == NULL || endofgame) {
	notify(dside, "Game is already over.");
	beep();
    } else if (!side->ingame) {
	notify(dside, "You are already out of the game.");
	beep();
    } else {
	ask_bool(screen, "Are you sure you want to resign now?",
		 FALSE, aux_resign);
    }
}

static void
aux_resign(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	if (numsides > 2) {
	    /* (should suggest resigning to a trusted side) */
	    ask_side(screen, "To whom do you wish to surrender?",
		     NULL, aux_resign_to);
	} else {
	    net_resign_game(dside, NULL);
	}
    }
}

static void
aux_resign_to(Screen *screen, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(screen, &side2)) {
	net_resign_game(dside, side2);
    } else {
	/* Iterate until grok_side is happy. */
	screen->modalhandler = aux_resign_to;
    }
}

/* Stuff game state into a file. */

void
do_save(Side *side)
{
    if (!during_game_only())
      return;
    /* (TODO: Actually do the save.) */
    gamestatesafe = TRUE;
}

void
do_set_formation(Side *side)
{
    Screen *screen = ui->curscreen;

    if (!require_own_unit_during_game(screen))
      return;
    screen->argunitid = screen->curunit->id;
    ask_position(screen, "Form up on who?", aux_set_formation);
}

static void
aux_set_formation(Screen *screen, int cancel)
{
    int x, y;
    char ubuf[BUFSIZE];
    Unit *unit, *leader;

    if (cancel)
      return;
    if (screen->inpch == '?') {
	notify(dside, "Click on a unit that you want to follow.");
	screen->modalhandler = aux_set_formation;
	return;
    }
    if (grok_position(screen, &x, &y, NULL)) {
	unit = find_unit(screen->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (in_area(x, y)) {
	    strcpy(ubuf, unit_handle(dside, unit));
	    leader = unit_at(x, y);
	    if (in_play(leader)
		&& leader != unit
		&& trusted_side(unit->side, leader->side)) {
		net_set_formation(unit, leader,
				  unit->x - leader->x, unit->y - leader->y,
				  1, 1);
		notify(dside, "%s to keep formation with %s.",
		       ubuf, unit_handle(dside, leader));
	    } else if (unit->plan != NULL
		       && unit->plan->formation != NULL) {
		/* Assume this is to cancel the formation. */
		net_set_formation(unit, NULL, 0, 0, 0, 0);
		notify(dside, "%s no longer keeping formation.", ubuf);
	    } else {
		cmd_error(dside, "Nobody here to form on!");
	    }
	}
    } else {
	beep();
	screen->modalhandler = aux_set_formation;
    }
}

void
do_set_view_angle(Side *side)
{
  sscreen->map->main_vp->isometric = !sscreen->map->main_vp->isometric;
  redraw_screen(sscreen);
}

void
do_side_closeup(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

void
do_standing_orders(Side *side)
{
    int rslt;

    if (!during_game_only())
      return;
    if (cmdargstr) {
	rslt = parse_standing_order(dside, cmdargstr);
	if (rslt < 0)
	  cmd_error(dside, "Parse error");
    } else
      cmd_error(dside, "No arguments given.");
}

void
do_survey(Side *side)
{
    Screen *screen = ui->curscreen;

    if (screen == NULL)
      return;
    if (endofgame && screen->mode == survey_mode)
      return;
    if (screen->mode == survey_mode && !endofgame) {
	screen->mode = move_mode;
	if (is_designer(dside))
	  screen->mode = survey_mode; /* should be last design mode */
    } else if (screen->mode == move_mode) {
	screen->mode = survey_mode;
    } else if (is_designer(dside)) {
	screen->mode = survey_mode;
    } else {
	beep();
	return;
    }
    if (!is_designer(dside)) {
	screen->autoselect = !screen->autoselect;
	screen->move_on_click = !screen->move_on_click;
    }
    set_tool_cursor(screen);
    if (!screen->autoselect && screen->curunit != NULL)
      update_cell(screen, screen->curunit->x, screen->curunit->y);
    if (screen->mode == move_mode)
      ;
    else if (screen->mode == survey_mode)
      ;
    update_cursor(screen);
}

void
do_unit_closeup(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

void
do_up(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

/* Command to display the program version. */

void
do_version(Side *side)
{
    notify(dside, "Xconq version %s", version_string());
    notify(dside, "(c) %s", copyright_string());
}

void
do_warranty(Side *side)
{
    popup_help(dside, warranty_help_node);
}

/* Create a new world screen (a regular screen zoomed to display the whole
   world at once). */

void
do_world_map(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

void
do_zoom_in(Side *side)
{
    Map *map = sscreen->map;

    set_view_power(map->main_vp, min(6, map->main_vp->power + 1));
    recenter(map, map->main_vp->vcx, map->main_vp->vcy);
    redraw_screen(sscreen);
}

void
do_zoom_out(Side *side)
{
    Map *map = sscreen->map;

    set_view_power(map->main_vp, max(0, map->main_vp->power - 1));
    recenter(map, map->main_vp->vcx, map->main_vp->vcy);
    redraw_screen(sscreen);
}

#ifdef DESIGNERS

void
do_design(Side *side)
{
    Screen *screen = ui->curscreen;
    Screen *screen2;

    if (screen == NULL)
      return;
    if (!is_designer(dside)) {
	if (!designed_on) {
	    ask_bool(screen, "Do you really want to start designing?",
		     FALSE, aux_design);
	} else {
	    really_do_design(dside);
	}
    } else {
	net_become_nondesigner(dside);
	for_all_screens(screen2) {
	    set_show_all(screen2, side->show_all);
	    /* Force back to survey mode; would be perhaps spiffier to
	       switch back to pre-design mode, but given the effects
	       of design mode, not much reason to bother. */
	    screen->mode = survey_mode;
	    set_tool_cursor(screen);
	}
	/* Just redo every display. */
	do_refresh(dside);
    }
}

static void
aux_design(Screen *screen, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(screen)) {
	really_do_design(dside);
    } else {
	/* nothing to do if we said not to design */
    }
}

void
really_do_design(Side *side)
{
    Screen *screen;

    net_become_designer(dside);
    for_all_screens(screen) {
	set_show_all(screen, side->show_all);
	redraw_screen(screen);
	screen->autoselect = FALSE;
	screen->mode = survey_mode;
    }
}

#endif /* DESIGNERS */

#ifdef DEBUGGING

void
do_profile(Side *side)
{
    cmd_error(dside, "Not implemented.");
}

void
do_trace(Side *side)
{
    extern int activity_trace;

    if (activity_trace)
      dump_activity_trace();
    activity_trace = !activity_trace;
}

#endif /* DEBUGGING */

/* Generic command error feedback. */

void
cmd_error(Side *side, const char *fmt, ...)
{
    va_list ap;

    if (!empty_string(fmt)) {
	va_start(ap, fmt);
	vnotify(dside, fmt, ap);
	va_end(ap);
    }
    /* Only beep once, even if a command generates multiple error messages. */
    if (ui->beepcount++ < 1)
      beep();
}

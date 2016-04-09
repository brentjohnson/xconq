/* Commands for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "tkconq.h"
#include "print.h"
#include "aiunit.h"
#include "aiunit2.h"

extern void toggle_map_outline(Map *map);

void really_do_design(Side *side);

static void aux_add_terrain(Side *side, Map *map, int cancel);
static void aux_add_terrain_2(Side *side, Map *map, int cancel);
static void do_add_terrain_2(Side *side, Map *map, int t);
static void aux_attack(Side *side, Map *map, int cancel);
static void aux_build(Side *side, Map *map, int cancel);
static void aux_build_2(Side *side, Map *map, int cancel);
static void aux_change_type(Side *side, Map *map, int cancel);
static void aux_distance(Side *side, Map *map, int cancel);
static void aux_fire_at(Side *side, Map *map, int cancel);
static void common_fire_at(Unit *unit, int x, int y);
static void aux_fire_into(Side *side, Map *map, int cancel);
static void common_fire_into(Unit *unit, int x, int y);
static void aux_give_unit(Side *side, Map *map, int cancel);
static void aux_move_dir(Side *side, Map *map, Unit *unit);
static void aux_message(Side *side, Map *map, int cancel);
static void aux_move_to(Side *side, Map *map, int cancel);
static void aux_name(Side *side, Map *map, int cancel);
static void aux_others(Side *side, Map *map, int cancel);
static void aux_remove_terrain(Side *side, Map *map, int cancel);
static void aux_remove_terrain_2(Side *side, Map *map, int cancel);
static void do_remove_terrain_2(Side *side, Map *map, int t);
static void aux_quit_resign(Side *side, Map *map, int cancel);
static void aux_quit_resign_to(Side *side, Map *map, int cancel);
static void aux_quit_exit(Side *side, Map *map, int cancel);
static void aux_quit_save(Side *side, Map *map, int cancel);
static void aux_quit_leave(Side *side, Map *map, int cancel);
static void aux_resign(Side *side, Map *map, int cancel);
static void aux_resign_to(Side *side, Map *map, int cancel);
static void aux_set_formation(Side *side, Map *map, int cancel);

static void aux_design(Side *side, Map *map, int cancel);

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
require_unit(Map *map)
{
    if (map == NULL) {
	beep(dside);
	return FALSE;
    }
    if (!in_play(map->curunit) || map->curunit_id != map->curunit->id) {
	map->curunit = NULL;
	map->curunit_id = 0;
	cmd_error(dside, "No current unit to command!");
	return FALSE;
    }
    return TRUE;
}

static int
require_own_unit_during_game(Map *map)
{
    if (!during_game_only())
      return FALSE;
    if (!require_unit(map))
      return FALSE;
    if (!side_controls_unit(dside, map->curunit)) {
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
    Map *map = side->ui->curmap;

    if (selvec == NULL)
      selvec = make_unit_vector(2);
    clear_unit_vector(selvec);
    if (map && in_play(map->curunit) && map->curunit_id == map->curunit->id)
      selvec = add_unit_to_vector(selvec, map->curunit, 0);
    return selvec;
}

int designed_on = FALSE;

/* Definitions of all the command functions. */

void
do_add_terrain(Side *side)
{
    Map *map = side->ui->curmap;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
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
	cmd_error(side, "%s cannot add or alter terrain!",
		  unit_handle(side, unit));
    } else if (numtypes == 1) {
	map->argunitid = unit->id;
	do_add_terrain_2(side, map, tfirst);
    } else {
	map->argunitid = unit->id;
	ask_terrain_type(side, map, "Type to add:", tmp_t_array,
			 aux_add_terrain);
    }
}

static void
aux_add_terrain(Side *side, Map *map, int cancel)
{
    int t;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Type a key to select terrain type.");
    }
    if (grok_terrain_type(side, map, &t)) {
	if (t != NONTTYPE) {
	    do_add_terrain_2(side, map, t);
	}
    } else {
        if (map->inpch != '?')
	  beep(side);
	/* Stay in this mode until we get it right. */
	map->modalhandler = aux_add_terrain;
    }
}

/* This is like do_add_terrain, but with a terrain type given. */

static void
do_add_terrain_2(Side *side, Map *map, int t)
{
    char abuf[100];

    map->tmpt = t;
    sprintf(abuf, "Add %s where?", t_type_name(t));
    ask_position(side, map, abuf, aux_add_terrain_2);
}

static void
aux_add_terrain_2(Side *side, Map *map, int cancel)
{
    int x, y, dir;
    Unit *unit;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Pick a location to add %s.", t_type_name(map->tmpt));
	map->modalhandler = aux_add_terrain_2;
	return;
    }
    if (grok_position(side, map, &x, &y, NULL)) {
	if (in_area(x, y)) {
	    unit = find_unit(map->argunitid);
	    if (!unit_still_ok(unit))
	      return;
	    switch (t_subtype(map->tmpt)) {
	      case cellsubtype:
		net_prep_alter_cell_action(unit, unit, x, y, map->tmpt);
		break;
	      case bordersubtype:
	      case connectionsubtype:
		if (x == unit->x && y == unit->y) {
		    beep(side);
		    notify(side, "Pick an adjacent location.");
		    map->modalhandler = aux_add_terrain_2;
		}
		dir = closest_dir(x - unit->x, y - unit->y);
		net_prep_add_terrain_action(unit, unit, unit->x, unit->y,
					    dir, map->tmpt);
		break;
	      case coatingsubtype:
		net_prep_add_terrain_action(unit, unit, unit->x, unit->y,
					    1, map->tmpt);
		break;
	    }
	}
    } else {
	beep(side);
	map->modalhandler = aux_add_terrain_2;
    }
}

void
do_attack(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *unit = map->curunit;
    
    if (!require_own_unit_during_game(map))
      return;
    map->argunitid = unit->id;
    ask_position(side, map, "Attack where?", aux_attack);
}

static void
aux_attack(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit, *unit2;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Click on a location");
    }
    if (grok_position(side, map, &x, &y, &unit2)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (unit2 != NULL) {
	    net_prep_attack_action(unit, unit, unit2, 100);
	} else {
	    cmd_error(side, "Nothing to attack!");
	}
    } else {
        if (map->inpch != '?')
	  beep(side);
	map->modalhandler = aux_attack;
    }
}

void
do_build(Side *side)
{
    Map *map = side->ui->curmap;
    int u, u2, numtypes, ufirst = NONUTYPE;
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
      return;
    u = unit->type;
    if (!can_build(unit)) {
	cmd_error(side, "%s cannot build anything!", unit_handle(side, unit));
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
	&& !(!completed(unit->transport)
	     && uu_acp_to_build(u, unit->transport->type) > 0)) {
	cmd_error(side, "%s cannot build while inside another unit!",
		  unit_handle(side, unit));
	return;
    }
    map->prevmode = map->mode;
    map->mode = build_mode;
    switch (numtypes) {
      case 0:
	cmd_error(side, "%s cannot build anything right now!", 
			unit_handle(side, unit));
	break;
      case 1:
	/* Only one type to build - do it. */
	if (u_mobile(unit->type)
	    || uu_create_range(unit->type, ufirst) > 0) {
	    char *msg;
	    map->argunitid = unit->id;
	    map->inptype = ufirst;
	    msg = (char *)xmalloc(80 + strlen (u_type_name (ufirst)));
	    sprintf (msg, "Build %s where?", 
	    		u_type_name (ufirst));
	    ask_position(side, map, msg, aux_build_2);
	    free (msg);
	} else if (impl_build(side, unit, ufirst, unit, unit->x, unit->y, 
                              map->prefixarg)) {
	    map->inptype = NONUTYPE;
             map->prefixarg = -1;
             eval_tcl_cmd("clear_command_line %d", map->number);
             map->modalhandler = NULL;
             map->mode = map->prevmode;
	}
	break;
      default:
	/* Player has to choose a type to build. */
	map->argunitid = unit->id;
	ask_unit_type(side, map, "Type to build:", tmp_u_array, aux_build);
	break;
    }
}

static void
aux_build(Side *side, Map *map, int cancel)
{
    int u2;
    Unit *unit;

    if (cancel) {
	cancel_unit_type(side, map);
	map->inptype = NONUTYPE;
	map->prefixarg = -1;
	eval_tcl_cmd("clear_command_line %d", map->number);
	map->modalhandler = NULL;
	map->mode = map->prevmode;
	return;
    }
    if (map->inpch == '?') {
	notify(side, "Type a key or click on a unit type to select build (or hit <esc> to cancel).");
    }
    if (grok_unit_type(side, map, &u2)) {
	/* Escape silently if the player cancelled. */
	if (u2 == NONUTYPE) {
		return;
	}
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit)) {
		return;
	}
	if (u_mobile(unit->type)
	    || uu_create_range(unit->type, u2) > 0) {
		/* For mobile and/or at-a-distance builders, also ask
		where to build. */
		char *msg;
		map->inptype = u2;
		/* Provide feedback about which unit type the user picked.  */
		msg = (char *)xmalloc(80 + strlen (u_type_name (u2)));
		sprintf (msg, "Build %s where?", u_type_name (u2));
		ask_position(side, map, msg, aux_build_2);
		free (msg);
		return;
	} else {
		impl_build(
		    side, unit, u2, unit, unit->x, unit->y, map->prefixarg);	
		map->inptype = NONUTYPE;
		map->prefixarg = -1;
		eval_tcl_cmd("clear_command_line %d", map->number);
		map->modalhandler = NULL;
		map->mode = map->prevmode;
	}
    } else {
	if (map->inpch != '?') {
	    beep(side);
	    map->modalhandler = aux_build;
	}
    }
}

static void
aux_build_2(Side *side, Map *map, int cancel)
{
    Unit *unit = NULL, *transport = NULL;

    if (cancel) {
	map->inptype = NONUTYPE;
	map->prefixarg = -1;
	eval_tcl_cmd("clear_command_line %d", map->number);
	map->modalhandler = NULL;
	map->mode = map->prevmode;
	return;
    }
    if (map->inpch == '?') {
	notify(side, "Pick a location where you would like to build (or hit <esc> to cancel).");
	map->modalhandler = aux_build_2;
	return;
    }
    if (in_area(map->inpx, map->inpy)) {
	transport = map->inpunit;
	if (!in_play(transport) 
	    || (transport->x != map->inpx) 
	    || (transport->y != map->inpy))
	    transport = NULL;
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit)) {
		return;
	}
	if (impl_build(
		       side, unit, map->inptype, transport, 
		       map->inpx, map->inpy, map->prefixarg)) {
		map->inptype = NONUTYPE;
		map->prefixarg = -1;
		eval_tcl_cmd("ask_position_done %d", map->number);
		map->modalhandler = NULL;
		map->mode = map->prevmode;
	} else {
		beep(side);
		map->modalhandler = aux_build_2;    
	}
    } else {
	notify(side, "Invalid position. Please pick a new location.");
	beep(side);
	map->modalhandler = aux_build_2;    
    }
}

void
do_change_type(Side *side)
{
    Map *map = side->ui->curmap;
    int u = NONUTYPE, u2 = NONUTYPE, numtypes = 0, ufirst = NONUTYPE;
    Unit *unit = NULL;

    /* (Can this happen, and what should we say if it does, and where
        should we say it?) */
    if (map && map->curunit)
      unit = map->curunit;
    else
      return;
    if (!require_own_unit_during_game(map))
      return;
    /* (Is this a possibility?) */
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
            map->inptype = NONUTYPE;
            return;
        }
        break;
      default:
        map->argunitid = unit->id;
        ask_unit_type(dside, map, "Type to change into:", tmp_u_array,
                      aux_change_type);
        break;
    }
}

static void
aux_change_type(Side *side, Map *map, int cancel)
{
    int u2 = NONUTYPE;
    Unit *unit = NULL;

    if (cancel) {
        cancel_unit_type(dside, map);
        return;
    }
    if (map->inpch == '?') {
        notify(dside, "Type a key or click in the unit list to select new type (or hit <esc> to cancel).");
    }
    if (grok_unit_type(dside, map, &u2)) {
        /* Escape silently if the player cancelled. */
        if (u2 == NONUTYPE)
          return;
        unit = find_unit(map->argunitid);
        if (!unit_still_ok(unit))
          return;
        if (!impl_change_type(dside, unit, u2)) {
            map->inptype = NONUTYPE;
            return;
        }
    }
    else {
        if (map->inpch != '?')
          beep(side);
        map->modalhandler = aux_change_type;
    }
}

void
do_collect(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *unit = map->curunit;
    int mtocollect;
    char *arg;
    const char *rest;

    if (!require_own_unit_during_game(map))
      return;
    mtocollect = NONMTYPE;
    if (nummtypes == 0) {
	cmd_error(side, "No materials to collect");
	return;
    }
    if (!empty_string(cmdargstr)) {
	rest = get_next_arg(cmdargstr, tmpbuf, &arg);
	mtocollect = mtype_from_name(arg);
    } else {
	cmd_error(side, "No material name given");
	return;
    }
    if (!is_material_type(mtocollect)) {
	cmd_error(side, "`%s' is not a recognized material name", arg);
	return;
    }
    if (unit->plan)
      net_set_collect_task(unit, mtocollect, unit->x, unit->y);
}

void
do_copying(Side *side)
{
    popup_help(side, copying_help_node);
}

void
do_dir(Side *side)
{
    Map *map = side->ui->curmap;

    if (!require_own_unit_during_game(map))
      return;
    aux_move_dir(side, map, map->curunit);
}

void
do_dir_multiple(Side *side)
{
    Map *map = side->ui->curmap;

    if (!require_own_unit_during_game(map))
      return;
    map->prefixarg = 9999;
    aux_move_dir(side, map, map->curunit);
}

/* Handle both single and multiple moves in the direction given by
   tmpkey. */

static void
aux_move_dir(Side *side, Map *map, Unit *unit)
{
    int ndirs, dir, n = map->prefixarg, x, y;
    HistEventType reason;
    char failbuf[BUFSIZE];

    if (!unit->act || !unit->plan || !mobile(unit->type)) {
	notify(dside, "Cannot move this unit!");		
	notify(dside, "Please pick another unit.");		
	beep(side);
	return;
    }
    ndirs = char_to_dir(tmpkey, &dir, NULL, NULL);
    if (ndirs < 1) {
	/* (would never occur in real life though...) */
	cmd_error(side, "what direction is that?!?");
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
	    notify(dside, "Cannot move unit outside the map!");		
	    notify(dside, "Please pick a new destination.");		
	    beep(side);
	    return;
	}
	if (!advance_into_cell(side, unit, x, y, unit_at(x, y), &reason)) {
	    advance_failure_desc(failbuf, unit, reason);
	    if (!empty_string(failbuf)) {
		notify(dside, "%s", failbuf);
	    }
	    beep(side);
	}
    }
}

void
do_distance(Side *side)
{
    Map *map = side->ui->curmap;

    if (!require_unit(map))
      return;
    map->argunitid = map->curunit->id;
    ask_position(side, map, "Distance to where?", aux_distance);
}

static void
aux_distance(Side *side, Map *map, int cancel)
{
    int x, y, dist;
    Unit *unit;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Pick a location to which you want the distance.");
	map->modalhandler = aux_distance;
	return;
    }
    if (grok_position(side, map, &x, &y, NULL)) {
	unit = find_unit(map->argunitid);
	/* We don't care about the unit's ownership. */
	if (in_area(x, y) && unit != NULL) {
	    dist = distance(unit->x, unit->y, x, y);
	    notify(side, "Distance from the current unit is %d cells.", dist);
	} else {
	    cmd_error(side, "Measurement failed.");
	}
    } else {
        beep(side);
	map->modalhandler = aux_distance;
    }
}

void
do_escape(Side *side)
{
}

void
do_fire(Side *side)
{
    Map *map = side->ui->curmap;
    int sx, sy, x, y;
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
      return;

    if (map->inpch == '\0') {
	map->argunitid = unit->id;
	ask_position(side, map, "Fire at what unit?", aux_fire_at);
    } else {
	sx = map->inpsx;  sy = map->inpsy;

	if (nearest_cell(widget_vp(map), sx, sy, &x, &y, NULL, NULL)) {
	    common_fire_at(unit, x, y);
	} else {
	    cmd_error(dside, "Cannot fire outside the world!");
	}
    }
}

static void
aux_fire_at(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, NULL)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit))
	  return;
	common_fire_at(unit, x, y);
    } else {
	map->modalhandler = aux_fire_at;
    }
}

static void
common_fire_at(Unit *unit, int x, int y)
{
    int rslt;
    Unit *other = NULL;
    UnitView *uview = NULL;
    int methere = FALSE, minethere = FALSE;

    /* Check only those units which we can see. */
    for_all_view_stack_with_occs(unit->side, x, y, uview) {
        /* And which are not on our side. Note that we are allowed to 
           hit other trusted sides, though doing so may make them not 
           trusted for long. :-) */
        if (unit->side != side_n(uview->siden)) {
            other = uview->unit;
            /* Assume we have the right target. (Bad assumption, 
               but what else can one do with the info presently available?) */
            break;
        }
        /* If one of our units is in the cell, we make note of this, in 
           case there turns out to be no visible enemies there. */
        else {
            if (uview->unit == unit)
              methere = TRUE;
            else
              minethere = TRUE;
        }
    }
    /* Target may be a ghost (view of dead or moved unit), but the player 
       cannot know that, and hence we should proceed with a fire-into 
       action instead. */
    /* (This code should not be in the interface. Instead the referee 
        code in the kernel should accept UnitView * instead of 
        Unit *, and perform these checks for us, and guide the 
        fire-into action to be safer in such cases. Or perhaps just run 
        an attempted_fire_at procedure, which would expend ammo, draw the 
        fire lines, and do not much else.) */
    if (other && (!in_play(other) || (other->x != x) || (other->y != y))) {
        common_fire_into(unit, x, y);
        return;
    }
    if (other == NULL) {
        if (methere)
          cmd_error(dside, "You can't fire at yourself!");
        else if (minethere)
          cmd_error(dside, "You can't fire at one of your own units!");
        else
	  cmd_error(dside, "Apparently nothing there to fire at!");
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
    Map *map = side->ui->curmap;
    int x = map->inpx, y = map->inpy;
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
      return;

    if (map->inpch == '\0') {
	map->argunitid = unit->id;
	ask_position(side, map, "Fire into which location?", aux_fire_into);
    } else {
	common_fire_into(unit, x, y);
    }
}

static void
aux_fire_into(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, NULL)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit))
	  return;
	common_fire_into(unit, x, y);
    } else {
	map->modalhandler = aux_fire_into;
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
    cmd_error(side, "Not implemented.");
}

/* Toggle the "follow-action" flag. */

void
do_follow_action(Side *side)
{
    Map *map = side->ui->curmap;

    if (map == NULL)
      return;
    if (!during_game_only())
      return;
    map->follow_action = !map->follow_action;
    if (map->follow_action) {
	notify(side, "Following the action on map.");
    } else {
	notify(side, "Not following the action on map.");
    }
}

/* Give a unit to another side or make it independent. */

/* (but giving to indep should be tested, otherwise might kill unit) */

static void really_do_give_unit(Unit *unit, Side *side2);

void
do_give_unit(Side *side)
{
    Map *map = side->ui->curmap;
    Side *side2;

    if (!require_own_unit_during_game(map))
      return;
    if (between(0, map->prefixarg, numsides)) {
	/* Interpret a prefix as the recipient side. */
	side2 = side_n(map->prefixarg);
	really_do_give_unit(map->curunit, side2);
    } else {
	ask_side(side, map, "To whom do you wish to give the unit?", NULL,
		 aux_give_unit);
    }
}

static void
aux_give_unit(Side *side, Map *map, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(side, map, &side2)) {
	really_do_give_unit(map->curunit, side2);
    } else {
	/* Iterate until grok_side is happy. */
	map->modalhandler = aux_give_unit;
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
    popup_help(side, NULL);
}

/* In general, the subcommands of "map" are for the GUI to use. */

void
do_map(Side *side)
{
    Map *map = side->ui->curmap;
    VP *vp;

    if (empty_string(cmdargstr))
      return;
    if (map == NULL)
      return;
    vp = widget_vp(map);
    if (strncmp(cmdargstr, "contour-interval=", 17) == 0) {
	if (strcmp("?", cmdargstr+17) == 0) {
	    notify(side, "Contour interval is %d.", vp->contour_interval);
	} else {
	    int n = strtol(cmdargstr+17, NULL, 10);
	    set_contour_interval(vp, n);
	    redraw_map(map);
	}
    } else if (strcmp(cmdargstr, "iso") == 0) {
	eval_tcl_cmd("set_isometric %d %d %d",
		     map->number, !vp->isometric,
		     (map->prefixarg >= 0 ? map->prefixarg : vp->vertscale));
    } else if (strcmp(cmdargstr, "meridians") == 0) {
	vp->draw_meridians = !vp->draw_meridians;
	if (map->prefixarg > 0)
	  set_meridian_interval(vp, map->prefixarg);
	redraw_map(map);
    } else if (strcmp(cmdargstr, "rotl") == 0) {
	eval_tcl_cmd("set_iso_dir %d %d", map->number, left_dir(vp->isodir));
	notify(side, "Now looking %s.", dirnames[vp->isodir]);
    } else if (strcmp(cmdargstr, "rotr") == 0) {
	notify(side, "Now looking %s.", dirnames[vp->isodir]);
	eval_tcl_cmd("set_iso_dir %d %d", map->number, right_dir(vp->isodir));
    } else if (strcmp(cmdargstr, "show-all") == 0) {
	if (!side->may_set_show_all) {
	    cmd_error(side, "You are not permitted to see everything!");
	    return;
	}
	set_show_all(map, !map->show_all);
	redraw_map(map);
    } else if (strcmp(cmdargstr, "unit-colorize") == 0) {
	map->colorize_units = !map->colorize_units;
	redraw_map(map);
    } else if (*cmdargstr == '!') {
	eval_tcl_cmd("%s", cmdargstr + 1);
    } else {
	cmd_error(side, "Map command \"%s\" not recognized!", cmdargstr);
    }
}

/* Send a short (1 line) message to another player.  Some messages are
   recognized specially, causing various actions. */

void
do_message(Side *side)
{
    Map *map = side->ui->curmap;
    char prompt[BUFSIZE];
    Side *side2;

    if (map == NULL)
      return;
    side2 = side_n(map->prefixarg);
    if (side == side2) {
	sprintf(prompt, "Message to yourself: ");
    } else if (side2) {
	sprintf(prompt, "Message to %s: ", short_side_title(side2));
    } else {
	sprintf(prompt, "Broadcast to all: ");
    }
    map->argside = side2;
    ask_string(side, map, prompt, NULL, aux_message);
}

static void
aux_message(Side *side, Map *map, int cancel)
{
    char *msg;
    SideMask sidemask;

    if (cancel)
      return;
    if (grok_string(side, map, &msg)) {
	if (empty_string(msg)) {
	    notify(side, "You keep your mouth shut.");
	    sidemask = NOSIDES;
	} else if (map->argside == NULL) {
	    notify(side, "You broadcast to everybody.", msg);
	    sidemask = ALLSIDES;
	} else {
	    notify(side, "You send the message.");
	    sidemask = add_side_to_set(map->argside, NOSIDES);
	}
	if (!empty_string(msg) && sidemask != NOSIDES)
	  net_send_message(side, sidemask, msg);
    } else {
	map->modalhandler = aux_message;
    }
}

/* Set unit to move to a given location.  */

/* The command proper. */

void
do_move_to(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *unit = map->curunit;

    if (!is_designer(side)) {
	if (!mobile(unit->type)) {
	    cmd_error(side, "%s cannot move at all!", unit_handle(side, unit));
	    return;
	}
    }
    if (!require_own_unit_during_game(map))
      return;
    map->argunitid = unit->id;
    ask_position(side, map, "Move to where?", aux_move_to);
}

static void
aux_move_to(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, NULL)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit)) {
	    return;
	}
	if (impl_move_to(side, unit, x, y, 0)) {
	    return;
	} else {
	    beep(side);
	    map->modalhandler = aux_move_to;
	}
    } else {
	beep(side);
	notify(side, "Invalid position. Please pick a new destination.");
	map->modalhandler = aux_move_to;
    }
}

/* Name/rename the current unit. */

void
do_name(Side *side)
{
    Map *map = side->ui->curmap;
    char tmpnamebuf[BUFSIZE];
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
      return;
    map->argunitid = unit->id;
    sprintf(tmpnamebuf, "New name for %s:", unit_handle(side, unit));
    ask_string(side, map, tmpnamebuf, unit->name, aux_name);
}

static void
aux_name(Side *side, Map *map, int cancel)
{
    char *name;
    Unit *unit;

    if (cancel)
      return;
    if (grok_string(side, map, &name)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit))
	  return;
	net_set_unit_name(side, unit, name);
    } else {
	map->modalhandler = aux_name;
    }
}

/* Create a new map, of standard size and zoom. */
/* (should clone last map in list perhaps?) */

void
do_new_map(Side *side)
{
    create_map();
}

/* This is a command to examine all occupants and suboccupants, in an
   inorder fashion. */

/* Should have an option to open up a list window that shows everything
   all at once. */

void
do_occupant(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *nextocc;

    if (!require_unit(map))
      return;
    nextocc = find_next_occupant(map->curunit);
    if (nextocc != map->curunit)
      set_current_unit(map, nextocc);
    else
      beep(side);
}

void
do_orders_popup(Side *side)
{
    cmd_error(side, "Not implemented.");
}

void
do_other(Side *side)
{
    Map *map = side->ui->curmap;

    if (map == NULL)
      return;
    ask_string(side, map, "Command:", "", aux_others);
}

static void
aux_others(Side *side, Map *map, int cancel)
{
    char *cmd;

    if (cancel)
      return;
    if (grok_string(side, map, &cmd)) {
	if (empty_string(cmd)) {
	    notify(side, "No command.");
	} else if (strcmp(cmd, "?") == 0) {
#if 0
	    do_help(side);
	    /* (should do with special jump routine) */
	    side->ui->curhelpnode = long_commands_help_node;
	    update_help(side);
#endif
	} else {
	    execute_long_command(side, cmd);
	}
    } else {
	map->modalhandler = aux_others;
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
    Map *map = side->ui->curmap;
    Unit *unit = map->curunit;

    if (!require_unit(map))
	return;

    if (!can_produce(unit)) {
	cmd_error(side, "cannot do active production");
    }
    n = 9999;
    if (map->prefixarg > 0)
	n = map->prefixarg;
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
    Map *map = side->ui->curmap;

    if (map == NULL || endofgame || beforestart) {
	/* If the game is over or never started, nothing to test or confirm. */
	exit_xconq(side);
    } else if (side->ingame) {
	if (gamestatesafe
	    || all_others_willing_to_quit(side)
	    || is_designer(side)) {
	    /* For the above cases, there is no obstacle to quitting,
	       but confirm anyway, in case there was a slip of the
	       keyboard. */
	    ask_bool(side, map, "Do you really want to quit?", FALSE,
		     aux_quit_exit);
	} else if (keeping_score()) {
	    ask_bool(side, map,
		     "Do you want to save the game before quitting?",
		     FALSE, aux_quit_save);
	} else {
	    /* Everybody is just participating. */
	    ask_bool(side, map, "Do you want to leave this game?", FALSE,
		     aux_quit_leave);
	}
    } else {
	/* We're already out of the game, not really anything to confirm. */
	/* (is this common to all interfaces?) */
	if (all_others_willing_to_quit(side) || num_active_displays() == 1)
	  exit_xconq(side);
    }
}

/* (Have an extra confirm for designers not to lose unsaved work?) */

static void
aux_quit_exit(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	exit_xconq(side);
    } else {
	/* Nothing to do if we said not to exit. */
    }
}

static void
aux_quit_save(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	do_save(side);
	/* Don't exit unless we know we saved successfully. */
	if (gamestatesafe)
	  exit_xconq(side);
    } else {
	ask_bool(side, map, "You cannot quit without resigning. Give up now?",
		 FALSE, aux_quit_resign);
    }
}

static void
aux_quit_resign(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	if (numsides > 2) {
	    /* (should suggest resigning to a trusted side) */
	    ask_side(side, map, "To whom do you wish to surrender?", NULL,
		     aux_quit_resign_to);
	} else {
	    net_resign_game(side, NULL);
	    /* (should exit now, or go through std keep-going dialog?) */
	}
    }
}

static void
aux_quit_resign_to(Side *side, Map *map, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(side, map, &side2)) {
	net_resign_game(side, side2);
    } else {
	/* Iterate until grok_side is happy. */
	map->modalhandler = aux_quit_resign_to;
    }
}

/* Do the act of leaving the game. */

static void
aux_quit_leave(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	/* (should probably structure differently, but how?) */
	remove_side_from_game(side);
	exit_xconq(side);
    } else {
	/* Nothing to do if we said not to exit. */
    }
}

/* Center the screen on the current location. */

void
do_recenter(Side *side)
{
    Map *map = side->ui->curmap;

    if (!require_unit(map))
      return;
    recenter(map, map->curunit->x, map->curunit->y);
}

/* Redraw everything using the same code as when windows need a redraw. */

void
do_refresh(Side *side)
{
    Map *map;

    reset_coverage();
    reset_all_views();
    dside->ui->legends = NULL;
    compute_all_feature_centroids();
    place_legends(dside);
    update_contour_intervals();
    for_all_maps(map) {
	redraw_map(map);
    }
}

void
do_remove_terrain(Side *side)
{
    Map *map = side->ui->curmap;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = map->curunit;

    if (!require_own_unit_during_game(map))
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
	cmd_error(side, "%s cannot remove terrain!", unit_handle(side, unit));
    } else if (numtypes == 1) {
	map->argunitid = unit->id;
	do_remove_terrain_2(side, map, tfirst);
    } else {
	map->argunitid = unit->id;
	ask_terrain_type(side, map, "Type to remove:",
			 tmp_t_array, aux_remove_terrain);
    }
}

static void
aux_remove_terrain(Side *side, Map *map, int cancel)
{
    int t;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Type a key to select terrain type to remove.");
    }
    if (grok_terrain_type(side, map, &t)) {
	if (t != NONTTYPE) {
	    do_remove_terrain_2(side, map, t);
	}
    } else {
        if (map->inpch != '?')
	  beep(side);
	/* Stay in this mode until we get it right. */
	map->modalhandler = aux_remove_terrain;
    }
}

/* This is like do_remove_terrain, but with a terrain type given. */

static void
do_remove_terrain_2(Side *side, Map *map, int t)
{
    map->tmpt = t;
    ask_position(side, map, "Remove where?", aux_remove_terrain_2);
}

static void
aux_remove_terrain_2(Side *side, Map *map, int cancel)
{
    int x, y, dir;
    Unit *unit;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Pick a location to remove %s.", t_type_name(map->tmpt));
	map->modalhandler = aux_remove_terrain_2;
	return;
    }
    if (grok_position(side, map, &x, &y, NULL)) {
	if (in_area(x, y)) {
	    unit = find_unit(map->argunitid);
	    if (!unit_still_ok(unit))
	      return;
	    switch (t_subtype(map->tmpt)) {
	      case cellsubtype:
		cmd_error(side, "can't remove cell terrain!");
		break;
	      case bordersubtype:
	      case connectionsubtype:
		dir = closest_dir(x - unit->x, y - unit->y);
		net_prep_remove_terrain_action(unit, unit, unit->x, unit->y, dir, map->tmpt);
		break;
	      case coatingsubtype:
		net_prep_remove_terrain_action(unit, unit, unit->x, unit->y, 1, map->tmpt);
		break;
	    }
	}
    } else {
	beep(side);     
	map->modalhandler = aux_remove_terrain_2;
    }
}

void
do_repair(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *unit, *other;

    if (!require_own_unit_during_game(map))
      return;
    unit = map->curunit;
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
    cmd_error(side, "No repair possible right now");
}

void
do_resign(Side *side)
{
    Map *map = side->ui->curmap;

    if (map == NULL || endofgame) {
	notify(side, "Game is already over.");
	beep(side);
    } else if (!side->ingame) {
	notify(side, "You are already out of the game.");
	beep(side);
    } else {
	ask_bool(side, map, "Are you sure you want to resign now?",
		 FALSE, aux_resign);
    }
}

static void
aux_resign(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	if (numsides > 2) {
	    /* (should suggest resigning to a trusted side) */
	    ask_side(side, map, "To whom do you wish to surrender?", NULL,
		     aux_resign_to);
	} else {
	    net_resign_game(side, NULL);
	}
    }
}

static void
aux_resign_to(Side *side, Map *map, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(side, map, &side2)) {
	net_resign_game(side, side2);
    } else {
	/* Iterate until grok_side is happy. */
	map->modalhandler = aux_resign_to;
    }
}

/* Stuff game state into a file. */

void
do_save(Side *side)
{
    if (!during_game_only())
      return;
    eval_tcl_cmd("popup_game_save {%s} {%s}",
		 saved_game_filename(), (cmdargstr ? cmdargstr : "0"));
}

void
do_set_formation(Side *side)
{
    Map *map = side->ui->curmap;

    if (!require_own_unit_during_game(map))
      return;
    map->argunitid = map->curunit->id;
    ask_position(side, map, "Form up on who?", aux_set_formation);
}

static void
aux_set_formation(Side *side, Map *map, int cancel)
{
    int x, y;
    char ubuf[BUFSIZE];
    Unit *unit, *leader;
    int gap = 0;
    int tx = -1, ty = -1, lx = -1, ly = -1;

    if (cancel) {
	map->prefixarg = -1;
        eval_tcl_cmd("clear_command_line %d", map->number);
        map->modalhandler = NULL;
	return;
    }
    if (map->inpch == '?') {
	notify(side, "Click on a unit that you want to follow.");
	map->modalhandler = aux_set_formation;
	return;
    }
    if (grok_position(side, map, &x, &y, NULL)) {
	unit = find_unit(map->argunitid);
	if (!unit_still_ok(unit))
	  return;
	if (in_area(x, y)) {
	    strcpy(ubuf, unit_handle(side, unit));
	    leader = unit_at(x, y);
	    if (in_play(leader)) {
		if (leader == unit) {
		    notify(side, "A unit cannot keep formation with itself!");
		    beep(side);
		    map->prefixarg = -1;
		    return;
		}
		/* (Do we really want to exclude enemy units? Why can't the 
		    formation mechanism be used to pursue enemy units? :-) */
		if (!trusted_side(unit->side, leader->side)) {
		    notify(side, 
			   "You cannot keep formation with an enemy unit.");
		    beep(side);
		    map->prefixarg = -1;
		    return;
		}
		gap = (0 > map->prefixarg) ? 1 : map->prefixarg;
		lx = leader->x; ly = leader->y;
		tx = unit->x; ty = unit->y;
		/* Shrink the dist down to gap. */
		if (!gap) {
		    tx = lx; ty = ly;
		}
		else {
		    while (distance(tx, ty, lx, ly) > gap) {
			assert_warning_return(interior_point_in_dir(tx, ty, 
						   approx_dir(lx - tx, ly - ty),
								    &tx, &ty),
"An invalid interior point was chosen for a formation position.",);
		    }
		}
		/* Set the formation. */
		if ((tx != unit->x) || (ty != unit->y))
		  net_push_move_to_task(unit, tx, ty, 0);
		net_set_formation(unit, leader, tx - lx, ty - ly, gap, 1);
		notify(side, 
		       "%s to keep formation with %s within a distance of %d.",
		       ubuf, unit_handle(side, leader), gap);
	    } else if (unit->plan != NULL
		       && unit->plan->formation != NULL) {
		/* Assume this is to cancel the formation. */
		net_set_formation(unit, NULL, 0, 0, 0, 0);
		notify(side, "%s no longer keeping formation.", ubuf);
	    } else {
		cmd_error(side, "Nobody here to form on!");
	    }
	    map->prefixarg = -1;
	}
    } else {
	beep(side);
	map->modalhandler = aux_set_formation;
    }
}

void
do_set_view_angle(Side *side)
{
    Map *map = side->ui->curmap;
    VP *vp = widget_vp(map);

    if (map == NULL)
      return;
    eval_tcl_cmd("set_isometric %d %d %d",
		 map->number, !vp->isometric,
		 (map->prefixarg >= 0 ? map->prefixarg : vp->vertscale));
}

void
do_side_closeup(Side *side)
{
    cmd_error(side, "Not implemented.");
}

void
do_standing_orders(Side *side)
{
    int rslt;

    if (!during_game_only())
      return;
    if (cmdargstr) {
	rslt = parse_standing_order(side, cmdargstr);
	if (rslt < 0)
	  cmd_error(side, "Parse error");
    } else
      cmd_error(side, "No arguments given.");
}

void
do_survey(Side *side)
{
    Map *map = side->ui->curmap;

    if (map == NULL)
      return;
    if (endofgame && map->mode == survey_mode)
      return;
    if (map->mode == survey_mode && !endofgame) {
	map->mode = move_mode;
	if (is_designer(side))
	  map->mode = survey_mode; /* should be last design mode */
    } else if (map->mode == move_mode) {
	map->mode = survey_mode;
    } else if (is_designer(side)) {
	map->mode = survey_mode;
    } else {
	beep(side);
	return;
    }
    if (!is_designer(side)) {
	map->autoselect = !map->autoselect;
	map->move_on_click = !map->move_on_click;
    }
    set_tool_cursor(map, 0);
    if (!map->autoselect && map->curunit != NULL)
      update_cell(map, map->curunit->x, map->curunit->y);
    if (map->mode == move_mode)
      eval_tcl_cmd("update_mode %d move", map->number);
    else if (map->mode == survey_mode)
      eval_tcl_cmd("update_mode %d survey", map->number);
}

void
do_unit_closeup(Side *side)
{
    cmd_error(side, "Not implemented.");
}

void
do_up(Side *side)
{
    cmd_error(side, "Not implemented.");
}

/* Command to display the program version. */

void
do_version(Side *side)
{
    notify(side, "Xconq version %s", version_string());
    notify(side, "(c) %s", copyright_string());
}

void
do_warranty(Side *side)
{
    popup_help(side, warranty_help_node);
}

/* Create a new world map (a regular map zoomed to display the whole
   world at once). */

void
do_world_map(Side *side)
{
    cmd_error(side, "Not implemented.");
}

void
do_zoom_in(Side *side)
{
    Map *map = side->ui->curmap;

    eval_tcl_cmd("zoom_in_out %d 1", map->number);
}

void
do_zoom_out(Side *side)
{
    Map *map = side->ui->curmap;

    eval_tcl_cmd("zoom_in_out %d -1", map->number);
}

#ifdef DESIGNERS

void
do_design(Side *side)
{
    Map *map = side->ui->curmap;
    Map *map2;

    if (map == NULL)
      return;
    if (!is_designer(side)) {
	if (!designed_on) {
	    ask_bool(side, map, "Do you really want to start designing?",
		     FALSE, aux_design);
	} else {
	    really_do_design(side);
	}
    } else {
	net_become_nondesigner(side);
	eval_tcl_cmd("update_show_all_info %d", dside->may_set_show_all);
	for_all_maps(map2) {
	    set_show_all(map2, side->show_all);
	    /* Force back to survey mode; would be perhaps spiffier to
	       switch back to pre-design mode, but given the effects
	       of design mode, not much reason to bother. */
	    map2->mode = survey_mode;
	    set_tool_cursor(map2, 0);
	}
	/* Just redo every display. */
	do_refresh(side);
 	eval_tcl_cmd("dismiss_design_palette");
 	/* Finally focus on the current map. */
	eval_tcl_cmd("focus -force .m%d", map->number);
  }
}

static void
aux_design(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	really_do_design(side);
    } else {
	/* nothing to do if we said not to design */
    }
}

void
really_do_design(Side *side)
{
    Map *map;

    net_become_designer(side);
    eval_tcl_cmd("update_show_all_info %d", dside->may_set_show_all);
    for_all_maps(map) {
	set_show_all(map, side->show_all);
	redraw_map(map);
	map->autoselect = FALSE;
	map->mode = survey_mode;
	eval_tcl_cmd("update_mode %d survey", map->number);
    }
    eval_tcl_cmd("popup_design_palette");
}

#endif /* DESIGNERS */

#ifdef DEBUGGING

void
do_profile(Side *side)
{
    cmd_error(side, "Not implemented.");
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
	vnotify(side, fmt, ap);
	va_end(ap);
    }
    /* Only beep once, even if a command generates multiple error messages. */
    if (side->ui->beepcount++ < 1)
      beep(side);
}

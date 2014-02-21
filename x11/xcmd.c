/* Commands for the X11 interface to Xconq.
   Copyright (C) 1987-1989, 1991-1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "xtconq.h"
extern void create_orders_window(Side *side, Map *map);

static void aux_add_terrain(Side *side, Map *map, int cancel);
static void aux_add_terrain_2(Side *side, Map *map, int cancel);
static void do_add_terrain_2(Side *side, Map *map, int t);
static void aux_attack(Side *side, Map *map, int cancel);
static void aux_build(Side *side, Map *map, int cancel);
static void aux_change_type(Side *side, Map *map, int cancel);
static void aux_distance(Side *side, Map *map, int cancel);
static void aux_fire_at(Side *side, Map *map, int cancel);
static void aux_move_look(Side *side, Map *map);
static void aux_move_dir(Side *side, Map *map, Unit *unit);
static void aux_message(Side *side, Map *map, int cancel);
static void aux_move_to(Side *side, Map *map, int cancel);
static void aux_name(Side *side, Map *map, int cancel);
static void aux_others(Side *side, Map *map, int cancel);
static void aux_remove_terrain(Side *side, Map *map, int cancel);
static void aux_remove_terrain_2(Side *side, Map *map, int cancel);
static void do_remove_terrain_2(Side *side, Map *map, int t);
static void aux_resign(Side *side, Map *map, int cancel);
static void aux_resign_2(Side *side, Map *map, int cancel);
static void aux_leave_game(Side *side, Map *map, int cancel);
static void aux_kill_game(Side *side, Map *map, int cancel);
static void aux_resign_b(Side *side, Map *map, int cancel);
static void aux_save_2(Side *side, Map *map, int cancel);
static void aux_save_1(Side *side, Map *map, int cancel);
static void aux_save_1_1(Side *side, Map *map, int cancel);
static void aux_set_formation(Side *side, Map *map, int cancel);
static void save_the_game(Side *side);

/* Use this macro in any command if it requires a current unit. */

#define REQUIRE_UNIT(side, map)  \
  if (!in_play((map)->curunit)) {  \
    (map)->curunit = NULL;  \
    cmd_error((side), "No current unit to command!");  \
    return;  \
  }

UnitVector *
get_selected_units(Side *side)
{
    static UnitVector *selvec;

    if (selvec == NULL)
	selvec = make_unit_vector(2);
    clear_unit_vector(selvec);
    if (in_play(side->ui->curmap->curunit))
	selvec = add_unit_to_vector(selvec, side->ui->curmap->curunit, 0);
    return selvec;
}

/* Definitions of all the command functions. */

void
do_add_terrain(Side *side)
{
    Map *map = side->ui->curmap;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);
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
	cmd_error(side, "%s cannot add or alter terrain!", unit_handle(side, unit));
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
    map->tmpt = t;
    save_cur(side, map);
    ask_position(side, map, "Add/alter where? [click to set]",
		 aux_add_terrain_2);
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
    if (grok_position(side, map, &x, &y, &unit)) {
	if (in_area(x, y)) {
	    unit = find_unit(map->argunitid);
	    if (in_play(unit) && side_controls_unit(side, unit)) {
		switch (t_subtype(map->tmpt)) {
		  case cellsubtype:
		    net_prep_alter_cell_action(unit, unit, x, y, map->tmpt);
		    break;
		  case bordersubtype:
		  case connectionsubtype:
		    dir = closest_dir(x - unit->x, y - unit->y);
		    net_prep_add_terrain_action(unit, unit, unit->x, unit->y, dir, map->tmpt);
		    break;
		  case coatingsubtype:
		    net_prep_add_terrain_action(unit, unit, unit->x, unit->y, 1, map->tmpt);
		    break;
		}
	    }
	}
	restore_cur(side, map);
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
    
    REQUIRE_UNIT(side, map);
    save_cur(side, map);
    map->argunitid = unit->id;
    ask_position(side, map, "Attack where? [click to set]", aux_attack);
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
	if (in_play(unit) && side_controls_unit(side, unit)) {
	    if (unit2 != NULL) {
		net_prep_attack_action(unit, unit, unit2, 100);
	    } else {
	      /* (should attack anything in cell) */
	    }
	} else {
	    cmd_error(side, "unit disappeared!?");
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

    REQUIRE_UNIT(side, map);
    u = unit->type;
    if (!can_build(unit)) {
	cmd_error(side, "%s can't build anything!", unit_handle(side, unit));
	return;
    }
    numtypes = 0;
    for_all_unit_types(u2) {
	if (uu_acp_to_create(u, u2) > 0 && type_allowed_on_side(u, side)) {
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
	cmd_error(side, "%s can't build anything while inside another unit!",
		  unit_handle(side, unit));
	return;
    }
    switch (numtypes) {
      case 0:
	cmd_error(side, "%s cannot build anything right now!", 
			unit_handle(side, unit));
	break;
      case 1:
	/* Only one type to build - do it. */
	impl_build(side, unit, ufirst, 0, 0, map->prefixarg);
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

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Type a key or click on a unit type to select build (or hit <esc> to cancel).");
    }
    if (grok_unit_type(side, map, &u2)) {
	if (u2 != NONUTYPE) {
	    unit = find_unit(map->argunitid);
	    if (in_play(unit) && side_controls_unit(side, unit)) {
		impl_build(side, unit, u2, 0, 0, map->prefixarg);
	    } else {
		cmd_error(side, "unit disappeared!?");
	    }
	}
    } else {
        if (map->inpch != '?')
	  beep(side);
	map->modalhandler = aux_build;
    }
}

void
do_change_type(Side *side)
{
    Map *map = side->ui->curmap;
    int u = NONUTYPE, u2 = NONUTYPE, numtypes = 0, ufirst = NONUTYPE;
    Unit *unit = NULL;

    if (map && map->curunit)
      unit = map->curunit;
    else
      return;
    REQUIRE_UNIT(side, map);
    if (unit)
      u = unit->type;
    else
      return;
    if (unit->side != side) {
        cmd_error(side, "%s is not your unit!", unit_handle(side, unit));
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
        cmd_error(side, "Cannot change to any type right now!");
        break;
      case 1:
        /* Only one possible type to change to. Change to it. */
        if (!impl_change_type(side, unit, ufirst)) {
            map->inptype = NONUTYPE;
            return;
        }
        break;
      default:
        map->argunitid = unit->id;
        ask_unit_type(side, map, "Type to change into:", tmp_u_array,
                      aux_change_type);
        break;
    }
}

static void
aux_change_type(Side *side, Map* map, int cancel)
{
    int u2 = NONUTYPE;
    Unit *unit = NULL;

    if (cancel) {
        /* cancel_unit_type(side, map); */
        return;
    }
    if (map->inpch == '?') {
        notify(side, "Type a key or click in the unit list to select new type (or hit <esc> to cancel).");
    }
    if (grok_unit_type(side, map, &u2)) {
        /* Escape silently if the player cancelled. */
        if (u2 == NONUTYPE)
          return;
        unit = find_unit(map->argunitid);
        if (!in_play(unit) || !side_controls_unit(side, unit))
          return;
        if (!impl_change_type(side, unit, u2)) {
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

/* (should reindent) */
void
do_collect(Side *side)
{
  Map *map = side->ui->curmap;
  Unit *unit = map->curunit;
  int mtocollect;
  char *arg, *rest;

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
    popup_help(side);
    /* Go directly to the copying help node. */
    side->ui->curhelpnode = copying_help_node;
    update_help(side);
}

void
do_dir(Side *side)
{
    Map *map = side->ui->curmap;

    switch (map->curtool) {
      case looktool:
	if (map->prefixarg < 0)
	  map->prefixarg = 1;
	aux_move_look(side, map);
	break;
      case movetool:
      case unitmovetool:
	if (map->curunit != NULL) {
	    aux_move_dir(side, map, map->curunit);
	} else {
	    cmd_error(side, "No current unit to move!");
	}
	break;
    }
}

void
do_dir_multiple(Side *side)
{
    Map *map = side->ui->curmap;

    switch (map->curtool) {
      case looktool:
	if (map->prefixarg < 0)
	  map->prefixarg = 10;
	aux_move_look(side, map);
	break;
      case movetool:
      case unitmovetool:
	map->prefixarg = 9999;
	if (map->curunit != NULL) {
	    aux_move_dir(side, map, map->curunit);
	} else {
	    cmd_error(side, "No current unit to move!");
	}
	break;
    }
}

static void
aux_move_look(Side *side, Map *map)
{
    int ndirs, dir, nx, ny;
    Unit *unit;

    ndirs = char_to_dir(tmpkey, &dir, NULL, NULL);
    if (ndirs < 1) {
	cmd_error(side, "what direction is that?!?");
	return;
    }
    if (!point_in_dir_n(map->curx, map->cury, dir, map->prefixarg, &nx, &ny)) {
	beep(side);
	return;
    }
    /* (should share with move_look) */
    clear_current(side, map);
    unit = unit_at(nx, ny);
    if (unit != NULL 
	&& (side_controls_unit(side, unit) || side->see_all)) {
	set_current_unit(side, map, unit);
    } else {
	set_current_xy(side, map, nx, ny);
    }
}
  
static void
aux_move_dir(Side *side, Map *map, Unit *unit)
{
    int ndirs, dir, n = map->prefixarg, x, y;
    HistEventType reason;
    char failbuf[BUFSIZE];

    if (!unit->act || !unit->plan) { /* use a more sophisticated test? */
	/* ??? can't act ??? */
	return;
    }
    ndirs = char_to_dir(tmpkey, &dir, NULL, NULL);
    if (ndirs < 1) {
	cmd_error(side, "what direction is that?!?");
	return;
    }
    if (n > 1) {
	DGprintf("Ordering %s to move %d %s\n",
		 unit_desig(unit), n, dirnames[dir]);
	net_set_move_dir_task(unit, dir, n);
    } else {
	if (!point_in_dir(unit->x, unit->y, dir, &x, &y)) {
	    return;
	}
	if (!advance_into_cell(side, unit, x, y, unit_at(x, y), &reason)) {
	    advance_failure_desc(failbuf, unit, reason);
	    notify(side, "%s", failbuf);
	    beep(side);
	}
	if (in_play(unit)) {
	    map->curx = unit->x;  map->cury = unit->y;
	}
#if 0	/* (this probably should not be here, but check behavior) */
	/* make sure we don't wander too close to the edge... */
	x = map->curx - map->vx;  y = map->cury - map->vy;
	if (y < 3 || y > map->vh - 3) {
	    recenter(side, map, map->curx, map->cury);
	}
#endif
    }
}

void
do_distance(Side *side)
{
    Map *map = side->ui->curmap;

    save_cur(side, map);
    ask_position(side, map, "Distance to where? [click to set]", aux_distance);
}

static void
aux_distance(Side *side, Map *map, int cancel)
{
    int x, y, dist;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Pick a location to which you want the distance.");
	map->modalhandler = aux_distance;
	return;
    }
    if (grok_position(side, map, &x, &y, NULL)) {
	if (in_area(x, y)) {
	    dist = distance(map->savedcurx, map->savedcury, x, y);
	    notify(side, "Distance from the current location is %d cells.", dist);
	}
	restore_cur(side, map);
    } else {
        beep(side);
	map->modalhandler = aux_distance;
    }
}

void
do_escape(Side *side)
{
    /* (should do something?) */
}

void
do_fire(Side *side)
{
    Map *map = side->ui->curmap;
    int sx, sy, x, y, rslt;
    Map *map2;
    Unit *other;
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);

    if (map->frombutton) {
	map->argunitid = unit->id;
	save_cur(side, map);
	ask_position(side, map, "Fire at what unit? [click to set]",
		     aux_fire_at);
	return;
    }

    map2 = side->ui->mapdown;
    sx = side->ui->sxdown;  sy = side->ui->sydown;

    if (x_nearest_cell(side, map2, sx, sy, &x, &y)) {
	if (x != unit->x || y != unit->y) {
	    if (unit->act && unit->plan) { /* (should be more sophisticated?) */
		/* (should only be able to target unit if covered...) */
		other = unit_at(x, y);
		if (other != NULL) {
		    /* There's a unit to fire at. */
		    if (other->side == unit->side) {
			cmd_error(side, "You can't fire at one of your own units!");
		    } else {
			rslt = check_fire_at_action(unit, unit, other, -1);
			if (valid(rslt)) {
			    net_prep_fire_at_action(unit, unit, other, -1);
			} else {
			    /* (should say which unit was target) */
			    cmd_error(side, "%s fire at unit not valid: %s",
				      unit_handle(side, unit),
				      action_result_desc(rslt));
			}
		    }
		} else {
		    cmd_error(side, "Nothing there to fire at");
		}
	    }
	}
    }
}

static void
aux_fire_at(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit, *other;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, &unit)) {
	unit = find_unit(map->argunitid);
	if (x != unit->x || y != unit->y) {
	    if (unit->act && unit->plan) { /* (should be more sophisticated?) */
		other = unit_at(x, y);
		if (other != NULL) {
		    /* There's a unit to fire at. */
		    if (other->side == unit->side) {
			cmd_error(side, "You can't fire at one of your own units!");
		    } else if (valid(check_fire_at_action(unit, unit, other, -1))) {
			net_prep_fire_at_action(unit, unit, other, -1);
		    }
		} else {
		    cmd_error(side, "Nothing there to fire at");
		}
	    }
	}
	restore_cur(side, map);
    } else {
	map->modalhandler = aux_move_to;
    }
}

void
do_fire_into(Side *side)
{
    Map *map = side->ui->curmap;
    int sx, sy, x, y, rslt;
    Map *map2;
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);

    sx = side->ui->sxdown;
    sy = side->ui->sydown;
    map2 = side->ui->mapdown;

    if (x_nearest_cell(side, map2, sx, sy, &x, &y)) {
	if (x != unit->x || y != unit->y) {
	    rslt = check_fire_into_action(unit, unit, x, y, 0, -1);
	    if (valid(rslt)) {
		net_prep_fire_into_action(unit, unit, x, y, 0, -1);
	    } else {
		cmd_error(side, "%s fire into %d,%d not valid: %s",
			  unit_handle(side, unit), x, y,
			  action_result_desc(rslt));
	    }
	}
    }
}

void
do_flash(Side *side)
{
    Map *map = side->ui->curmap;
    int i;

    if (in_area(map->curx, map->cury)) {
	for (i = 0; i < 2; ++i) {
	    invert_unit_subarea(side, map, map->curx, map->cury);
	    /* (should wait a moment) */
	    invert_unit_subarea(side, map, map->curx, map->cury);
	    draw_row(side, map, map->curx, map->cury, 1, TRUE);
	    if (map->curunit) {
		draw_current(side, map);
	    }
	    invert_unit_subarea(side, map, map->curx, map->cury);
	    /* (should wait a moment) */
	    invert_unit_subarea(side, map, map->curx, map->cury);
	    draw_row(side, map, map->curx, map->cury, 1, TRUE);
	    if (map->curunit) {
		draw_current(side, map);
	    }
	}
    } else {
	beep(side);
    }
}

/* Toggle the "follow-action" flag. */

void
do_follow_action(Side *side)
{
    side->ui->follow_action = !side->ui->follow_action;
    if (side->ui->follow_action) {
	notify(side, "Following the action.");
    } else {
	notify(side, "Not following the action.");
    }
}

/* Give a unit to another side or make it independent. */

/* (but giving to indep should be tested, otherwise might kill unit) */

void
do_give_unit(Side *side)
{
    Map *map = side->ui->curmap;
    int u;
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);
    u = unit->type;
    if (/* u_change_side(u) || */ side->designer) {
/*	unit_changes_side(unit, side_n(n), CAPTURE, PRISONER);  */
	all_see_cell(unit->x, unit->y);
    } else {
	cmd_error(side, "You can't just give away %s!",
		  unit_handle(side, unit));
    }
}

void
do_help(Side *side)
{
    /* Compose the help node for commands and make it be the first one. */
    if (side->ui->curhelpnode == NULL) {
	key_commands_help_node =
	  add_help_node("commands", describe_key_commands, 0, first_help_node);
	side->ui->curhelpnode = first_help_node;
	long_commands_help_node =
	  add_help_node("long commands", describe_long_commands, 0,
			key_commands_help_node);
    }
    popup_help(side);
}

void
do_map(Side *side)
{
    int n;
    Map *map = side->ui->curmap;

    if (empty_string(cmdargstr))
      return;
    if (strcmp(cmdargstr, "close") == 0) {
	/* (should trash map and popdown) */
	notify(side, "can't close a map yet");
    } else if (strncmp(cmdargstr, "contour-color=", 14) == 0) {
	side->ui->contour_color = request_color(side, cmdargstr+14);
	do_refresh(side);
    } else if (strncmp(cmdargstr, "contour-interval=", 17) == 0) {
	if (strcmp("?", cmdargstr+17) == 0) {
	    notify(side, "Contour interval is %d.", map->vp->contour_interval);
	} else {
	    n = strtol(cmdargstr+17, NULL, 10);
	    set_contour_interval(map->vp, n);
	    do_refresh(side);
	}
    } else if (strcmp(cmdargstr, "meridians") == 0) {
	if (map->prefixarg > 0) {
	    side->ui->default_meridian_interval = map->prefixarg;
	    map->vp->meridian_interval = side->ui->default_meridian_interval;
	    map->vp->draw_meridians = TRUE;
	} else {
	    map->vp->draw_meridians = FALSE;
	}
	do_refresh(side);
    } else if (strcmp(cmdargstr, "mono") == 0) {
	side->ui->pref_solid_color_terrain = FALSE;
	do_refresh(side);
    } else if (strcmp(cmdargstr, "rv") == 0) {
	if (0) {
	    side->ui->bonw = !side->ui->bonw;
	    set_colors(side);
	    reset_color_state(side);
	    redraw(side);
	} else {
	    cmd_error(side, "Reverse video is only for monochrome!");
	}
    } else if (strcmp(cmdargstr, "solid") == 0) {
	side->ui->pref_solid_color_terrain = TRUE;
	do_refresh(side);
    } else {
	cmd_error(side, "Argument \"%s\" not understood");
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

    side2 = side_n(map->prefixarg);
    if (side == side2) {
	cmd_error(side, "You mumble to yourself.");
	return;
    }
    if (side2) {
	sprintf(prompt, "Say to %s: ", short_side_title(side2));
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
	    notify(side, "You send your message.");
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

    REQUIRE_UNIT(side, map);
    if (!is_designer(side)) {
	if (!mobile(unit->type)) {
	    cmd_error(side, "%s cannot move at all!", unit_handle(side, unit));
	    return;
	}
    }
    map->argunitid = unit->id;
    save_cur(side, map);
    ask_position(side, map, "Move to where? [click to set]",
		 aux_move_to);
}

static void
aux_move_to(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, &unit)) {
	unit = find_unit(map->argunitid);
	if (in_play(unit)) {
		if (impl_move_to(side, unit, x, y, 0)) {
		    return;
		} else {
		    beep(side);
		    map->modalhandler = aux_move_to;
		}
	}
	restore_cur(side, map);
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
    char tmpbuf[BUFSIZE];
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);
    map->argunitid = unit->id;
    sprintf(tmpbuf, "New name for %s:", unit_handle(side, unit));
    ask_string(side, map, tmpbuf, unit->name, aux_name);
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
	if (in_play(unit) && side_controls_unit(side, unit)) {
	    net_set_unit_name(side, unit, name);
	} else {
	    cmd_error(side, "Nothing here that could be named!");
	}
    } else {
	map->modalhandler = aux_name;
    }
}

/* Create a new map, of standard size and zoom. */
/* (should clone last map in list perhaps?) */

void
do_new_map(Side *side)
{
    Map *map2;

    map2 = create_map(side, 5, NULL);
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
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);
    nextocc = find_next_occupant(unit);
    if (nextocc != unit) {
	set_current_unit(side, map, nextocc);
    }
}

void
do_orders_popup(Side *side)
{
    Map *map = side->ui->curmap;

    create_orders_window(side, map);
}

void
do_other(Side *side)
{
    Map *map = side->ui->curmap;

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
	    do_help(side);
	    /* (should do with special jump routine) */
	    side->ui->curhelpnode = long_commands_help_node;
	    update_help(side);
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

    if (side->ui->ps_pp == NULL)
      side->ui->ps_pp = (PrintParameters *) xmalloc(sizeof(PrintParameters));

    init_ps_print(side->ui->ps_pp);

    /* convert to cm or in */
    if (side->ui->ps_pp->cm) {
	conv = 72 / 2.54;
    } else {
	conv = 72;
    }
    side->ui->ps_pp->cell_size /= conv;
    side->ui->ps_pp->cell_grid_width /= conv;
    side->ui->ps_pp->border_width /= conv;
    side->ui->ps_pp->connection_width /= conv;
    side->ui->ps_pp->page_width /= conv;
    side->ui->ps_pp->page_height /= conv;
    side->ui->ps_pp->top_margin /= conv;
    side->ui->ps_pp->bottom_margin /= conv;
    side->ui->ps_pp->left_margin /= conv;
    side->ui->ps_pp->right_margin /= conv;

    popup_print_setup_dialog(side);
}

void
do_produce(Side *side)
{
    notify(side, "can't produce materials by action yet");
}

void
do_quit(Side *side)
{
    Map *map = side->ui->curmap;

    if (map == NULL || endofgame || beforestart) {
	/* If the game is over or never started, nothing to test or confirm. */
	exit_xconq(side);
    } else if (side->ingame) {
	if (all_others_willing_to_quit(side)) {
	    /* Everbody else is willing to get out, but confirm us anyway. */
	    ask_bool(side, map, "Do you really want to quit?", FALSE,
		     aux_kill_game);
	} else {
	    if (1 /* outcome needs resolution */) {
		/* if not everybody willing to draw, then we have to resign */
		ask_bool(side, map, "Do you really want to give up?", FALSE,
			 aux_resign);
	    } else {
		/* Everybody is just participating. */
		ask_bool(side, map, "Do you want to leave this game?", FALSE,
			 aux_leave_game);
	    }
	}
    } else {
	/* We're already out of the game, not really anything to confirm. */
	/* (is this common to all interfaces?) */
	if (all_others_willing_to_quit(side)) {
	    exit_xconq(side);
	} else {
	    close_display(side);
	    if (num_active_displays() == 0) {
		exit_xconq(side);
	    }
	}
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
		     aux_resign_2);
	} else {
	    net_resign_game(side, NULL);
	}
    }
}

static void
aux_resign_2(Side *side, Map *map, int cancel)
{
    Side *side2;

    if (cancel)
      return;
    if (grok_side(side, map, &side2)) {
	net_resign_game(side, side2);
    } else {
	/* Iterate until grok_side is happy. */
	map->modalhandler = aux_resign_2;
    }
}

/* Do the act of leaving the game. */

static void
aux_leave_game(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	remove_side_from_game(side);
	/* Now go back and see what happens if we're not in the game. */ 
	do_quit(side);
    } else {
	/* nothing to do if we said not to exit */
    }
}

/* (Have an extra confirm for designers not to lose unsaved work?) */

static void
aux_kill_game(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	exit_xconq(side);
	/* Nothing to do if we said not to exit. */
    }
}

/* Center the screen on the current location. */

void
do_recenter(Side *side)
{
    Map *map = side->ui->curmap;

    recenter(side, map, map->curx, map->cury);
}

/* Redraw everything using the same code as when windows need a redraw. */

void
do_refresh(Side *side)
{
    Map *map = side->ui->curmap;

    redraw(side);
    draw_view_in_panner(side, map);
}

void
do_remove_terrain(Side *side)
{
    Map *map = side->ui->curmap;
    int u, t, numtypes, tfirst = NONTTYPE;
    Unit *unit = map->curunit;

    REQUIRE_UNIT(side, map);
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
    save_cur(side, map);
    ask_position(side, map, "Remove where? [click to set]",
		 aux_remove_terrain_2);
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
	    if (in_play(unit) && side_controls_unit(side, unit)) {
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
	}
	restore_cur(side, map);
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

    REQUIRE_UNIT(side, map);
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

    if (endofgame) {
	notify(side, "Game is already over.");
	beep(side);
    } else if (!side->ingame) {
	notify(side, "You are already out of the game.");
	beep(side);
    } else {
	ask_bool(side, map, "You really want to resign?", FALSE, aux_resign_b);
    }
}

/* This is semi-redundant with aux_resign. */

static void
aux_resign_b(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	net_resign_game(side, NULL);
    }
}

/* Stuff game state into a file.  By default, it goes into the current
   directory.  If designing a game, we can specify exactly which parts
   of the game state are to be written. */

/* The command proper just sets up different modal handlers, depending on
   whether we're building (and therefore saving a scenario/fragment), or
   saving as much game state as possible, for resumption later. */

void
do_save(Side *side)
{
    Map *map = side->ui->curmap;
    Side *side2;

    /* First check to see if anybody is in the middle of doing something
       (like renaming a unit) whose state would be lost. */
    for_all_sides(side2) {
	if (side2 != side && side2->busy) {
	    cmd_error(side, "The %s are busy, can't save right now.",
		      plural_form(side2->name));
	}
    }
#ifdef DESIGNERS
    if (side->designer) {
	ask_string(side, map, "Types of data to save?", "all",
		   aux_save_1);
	return;
    }
#endif /* DESIGNERS */
    if (0 /* (should be "savemustquit") */) {
	ask_bool(side, map, "You really want to save?", FALSE, aux_save_2);
    } else {
	save_the_game(side);
    }
}

/* Make a module appropriate to a save file, write the file, and leave. */

static void
aux_save_2(Side *side, Map *map, int cancel)
{
    if (cancel)
      return;
    if (grok_bool(side, map)) {
	save_the_game(side);
    }
}

static void
save_the_game(Side *side)
{
    char *savename = saved_game_filename();

    notify_all("Game will be saved to \"%s\" ...", savename);
    if (write_entire_game_state(savename)) {
	close_displays();
	/* this should be conditional? */
	exit(0);
    } else {
	cmd_error(side, "Can't open file \"%s\"!", savename);
    }
}


#ifdef DESIGNERS

static Module *tmpmodule;

static void
aux_save_1_1(Side *side, Map *map, int cancel)
{
    char *filenamespec;

    if (cancel)
      return;
    if (grok_string(side, map, &filenamespec)) {
	tmpmodule->filename = filenamespec;
	notify(side, "File will be written to \"%s\" ...",
	       tmpmodule->filename);
	if (write_game_module(tmpmodule, tmpmodule->filename)) {
	    notify(side, "Done writing to \"%s\".", tmpmodule->filename);
	} else {
	    cmd_error(side, "Can't open file \"%s\"!", tmpmodule->filename);
	}
    } else {
	map->modalhandler = aux_save_1_1;
    }
}

static void
aux_save_1(Side *side, Map *map, int cancel)
{
    int save;
    char *contentspec;

    if (cancel)
      return;
    if (grok_string(side, map, &contentspec)) {
	tmpmodule = create_game_module("game-data");
	/* Use the spec string to decide which pieces to save. */
	save = FALSE;
	if (strcmp(contentspec, "all") == 0) {
	    tmpmodule->def_all = TRUE;
	    tmpmodule->compress_layers = TRUE;
	    save = TRUE;
	} else if (strcmp(contentspec, "world") == 0) {
	    tmpmodule->def_world = TRUE;
	    tmpmodule->def_areas = TRUE;
	    tmpmodule->def_area_terrain = TRUE;
	    tmpmodule->def_area_misc = TRUE;
	    tmpmodule->def_area_weather = TRUE;
	    tmpmodule->def_area_material = TRUE;
	    save = TRUE;
	} else {
	    cmd_error(side, "Don't understand content spec \"%s\"!",
		      contentspec);
	    return;
	}
	if (save) {
	    ask_string(side, map, "Save data to where?", "game-data.g",
		       aux_save_1_1);
	} else {
	    notify(side, "Nothing requested to be saved.");
	}
    } else {
	map->modalhandler = aux_save_1;
    }
}

#endif /* DESIGNERS */

void
do_set_formation(Side *side)
{
    Map *map = side->ui->curmap;

    REQUIRE_UNIT(side, map);
    map->argunitid = map->curunit->id;
    save_cur(side, map);
    ask_position(side, map, "Form up on who? [click to set]",
		 aux_set_formation);
}

static void
aux_set_formation(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit, *leader;

    if (cancel)
      return;
    if (map->inpch == '?') {
	notify(side, "Click on a unit that you want to follow.");
	map->modalhandler = aux_set_formation;
	return;
    }
    if (grok_position(side, map, &x, &y, &unit)) {
	unit = find_unit(map->argunitid);
	if (in_play(unit) && in_area(x, y)) {
	    leader = unit_at(x, y);
	    if (in_play(leader)
		&& leader != unit
		&& trusted_side(unit->side, leader->side)) {
		net_set_formation(unit, leader, leader->x - unit->x, leader->y - unit->y, 1, 1);
	    } else {
		cmd_error(side, "Nobody here to form on!");
	    }
	}
	restore_cur(side, map);
    } else {
	beep(side);
	map->modalhandler = aux_set_formation;
    }
}

void
do_set_view_angle(Side *side)
{
    /* (should do something) */
}

void
do_side_closeup(Side *side)
{
    Map *map = side->ui->curmap;

    Unit *unit = map->curunit;
    SideCloseup *sidecloseup;

    REQUIRE_UNIT(side, map);
    sidecloseup = find_side_closeup(side, unit->side);
    if (!sidecloseup)
      sidecloseup = create_side_closeup(side, map, unit->side);
    draw_side_closeup(side, sidecloseup);
}

void
do_standing_orders(Side *side)
{
    int rslt;

    if (cmdargstr) {
	rslt = parse_standing_order(side, cmdargstr);
	if (rslt < 0)
	  cmd_error(side, "Parse error");

	/* destroy orders window and structures */
	side->ui->orderlist = NULL;
	if (side->ui->orders_shell) {
	    XtPopdown(side->ui->orders_shell);
	    XtDestroyWidget(side->ui->orders_shell);
	    side->ui->orders_shell = NULL;
	}

    } else
      cmd_error(side, "No arguments given.");
}

void
do_survey(Side *side)
{
    Map *map = side->ui->curmap;

    if (map->curtool == looktool) {
	map->curtool = movetool;
    } else if (map->curtool == movetool) {
	map->curtool = looktool;
    } else {
	beep(side);
	return;
    }
    set_tool_cursor(side, map);
    update_controls(side, map);
}

void
do_unit_closeup(Side *side)
{
    Map *map = side->ui->curmap;
    Unit *unit = map->curunit;
    UnitCloseup *unitcloseup;

    REQUIRE_UNIT(side, map);
    unitcloseup = find_unit_closeup(side, unit);
    if (!unitcloseup)
      unitcloseup = create_unit_closeup(side, map, unit);
    draw_unit_closeup(side, unitcloseup);
}

void
do_up(Side *side)
{
    beep(side);
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
    popup_help(side);
    /* Go directly to the warranty help node. */
    side->ui->curhelpnode = warranty_help_node;
    update_help(side);
}

/* Create a new world map (a regular map zoomed to display the whole
   world at once). */

void
do_world_map(Side *side)
{
    Map *wmap;

    wmap = create_map(side, -1, NULL);
}

void
do_zoom_in(Side *side)
{
    Map *map = side->ui->curmap;

    zoom_in_out(side, map, ZOOM_IN);
}

void
do_zoom_out(Side *side)
{
    Map *map = side->ui->curmap;

    zoom_in_out(side, map, ZOOM_OUT);
}

#ifdef DESIGNERS

static void aux_design(Side *side, Map *map, int cancel);

int designed_on = FALSE;

void enable_in_unit_type_list(Side *side, Map *map, int u, int flag);

static void really_do_design(Side *side);

void
do_design(Side *side)
{
    Map *map = side->ui->curmap;

    int u;
    Map *map2;

    if (!side->designer) {
	if (!designed_on) {
	    ask_bool(side, map, "Do you really want to start designing?",
		     FALSE, aux_design);
	} else {
	    really_do_design(side);
	}
    } else {
	net_become_nondesigner(side);
	for_all_maps(side, map2) {
	    map2->seeall = side->see_all;
	    update_controls(side, map2);
	    for_all_unit_types(u) {
		enable_in_unit_type_list(side, map2, u, FALSE);
	    }
	}
	popdown_design(side);
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

static void
really_do_design(Side *side)
{
    int u;
    Map *map2;

    net_become_designer(side);
    for_all_maps(side, map2) {
	map2->seeall = (side->see_all || side->may_set_show_all);
	update_controls(side, map2);
	for_all_unit_types(u) {
	    enable_in_unit_type_list(side, map2, u, TRUE);
	}
    }
    popup_design(side);
}

#endif /* DESIGNERS */

#ifdef DEBUGGING

void
do_profile(Side *side)
{
}

void
do_trace(Side *side)
{
}

#endif /* DEBUGGING */

/* Generic command error feedback. */

void
cmd_error(Side *side, char *fmt, ...)
{
    if (!empty_string(fmt)) {
	va_list ap;

	va_start(ap, fmt);
	vnotify(side, fmt, ap);
	va_end(ap);
    }
    /* Only beep once, even if a command generates multiple error messages. */
    if (side->ui->beepcount++ < 1)
      beep(side);
}

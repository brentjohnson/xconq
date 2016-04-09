/* Commands for the curses interface to Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cconq.h"
#include "aiunit.h"
#include "aiunit2.h"

extern int drawlinear;
extern char linear_char;
extern char bord_char;
extern char conn_char;

static void resize_map(int n);

#if 0 /* (should preserve in help somewhere) */
    tbcat(buf, "To move a unit, use [hjklyubn]\n");
    tbcat(buf, "[HJKLYUBN] moves unit repeatedly in that direction\n");
    tbcat(buf, "To look at another unit, use survey mode ('z')\n");
    tbcat(buf, "and use [hjklyubnHJKLYUBN] to move the cursor\n");
#endif

/* Use this macro in any command if it requires a current unit. */

#define REQUIRE_UNIT()  \
  if (!in_play(curunit)) {  \
    curunit = NULL;  \
    cmd_error(side, "No current unit to command!");  \
    return;  \
  }

Unit *lastactor = NULL;

Unit *
find_next_and_look()
{
    Unit *nextunit;

    nextunit = find_next_actor(dside, curunit);
    if (nextunit != NULL) {
	make_current(nextunit);
	show_cursor();
    }
    return nextunit;
}

static UnitVector *selvec;

UnitVector *
get_selected_units(Side *side)
{
    if (selvec == NULL)
      selvec = make_unit_vector(2);
    clear_unit_vector(selvec);
    if (in_play(curunit))
      selvec = add_unit_to_vector(selvec, curunit, 0);
    return selvec;
}

void
do_add_terrain(Side *side)
{
    int u, t, dir;

    REQUIRE_UNIT();
    u = curunit->type;
    if (ask_direction("Add terrain to where?", &dir)) {
	for_all_terrain_types(t) {
	    if (ut_acp_to_add_terrain(u, t) > 0
		&& curunit->act
		&& curunit->act->acp >= ut_acp_to_add_terrain(u, t)) {
		if (0 <= ut_alter_range(curunit->type, t)) {
		    if (net_prep_add_terrain_action(curunit, curunit,
						curunit->x, curunit->y,
						dir, t))
		      ;
		    else
		      xbeep();
		}
	    }
	}
    }
}

void
do_attack(Side *side)
{
    int x, y;
    Unit *other;

    REQUIRE_UNIT();
    if (ask_position("Attack where?", &x, &y)) {
	for_all_stack(x, y, other) {
	    if (!unit_trusts_unit(curunit, other)) {
		if (valid(check_attack_action(curunit, curunit, other, 100))) {
		    net_prep_attack_action(curunit, curunit, other, 100);
		    return;
		}
		/* (should try other types of actions?) */
	    }
	}
	cmd_error(side, "Nothing for %s to attack at %d,%d!",
		  unit_handle(dside, curunit), x, y);
    }
}

void
do_build(Side *side)
{
    int u, u2, numtypes, ufirst;

    REQUIRE_UNIT();
    u = curunit->type;
    if (!can_build(curunit)) {
	cmd_error(side, "%s cannot build anything!", unit_handle(dside, curunit));
	return;
    }
    numtypes = 0;
    for_all_unit_types(u2) {
	if (unit_can_build_type(curunit, u2)) {
	    tmp_u_array[u2] = TRUE;
	    ++numtypes;
	    ufirst = u2;
	} else {
	    tmp_u_array[u2] = FALSE;
	}
    }
    if (curunit->transport != NULL
	&& !uu_occ_can_build(curunit->transport->type, u)
	&& !(!completed(curunit->transport)
	     && uu_acp_to_build(u, curunit->transport->type) > 0)) {
	cmd_error(side, "%s cannot build while inside another unit!",
		  unit_handle(side, curunit));
	return;
    }
    switch (numtypes) {
      case 0:
	cmd_error(side, "%s cannot build anything right now!", 
			unit_handle(side, curunit));
	break;
      case 1:
	/* Only one type to build - just do it. */
	if (valid(can_create_in(curunit, curunit, ufirst, curunit)))
	    impl_build(side, curunit, ufirst, curunit, -1, -1, prefixarg);
	else
	    impl_build(
		side, curunit, ufirst, NULL, curunit->x, curunit->y, prefixarg);
	break;
      default:
	/* Player has to choose a type to build. */
	u2 = ask_unit_type("Type to build:", tmp_u_array);
	if (u2 != NONUTYPE) {
	    if (valid(can_create_in(curunit, curunit, ufirst, curunit)))
		impl_build(side, curunit, u2, curunit, -1, -1, prefixarg);
	    else
		impl_build(
		    side, curunit, ufirst, NULL, curunit->x, curunit->y, 
		    prefixarg);
	} else {
	    /* should clear toplines */
	}
	break;
    }
}

void
do_change_type(Side *side)
{
    int u = NONUTYPE, u2 = NONUTYPE, numtypes = 0, ufirst = NONUTYPE;

    REQUIRE_UNIT();
    u = curunit->type;
    if (curunit->side != side) {
        cmd_error(side, "%s is not your unit!", unit_handle(side, curunit));
        return;
    }
    for_all_unit_types(u2) {
        if (uu_acp_to_change_type(curunit->type, u2)) {
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
        impl_change_type(side, curunit, ufirst);
        break;
      default:
        u2 = ask_unit_type("Type to change into:", tmp_u_array);
        if (u2 != NONUTYPE) {
            impl_change_type(side, curunit, ufirst);
        }
        else {
            /* should clear toplines */
        }
        break;
    }
}

void
do_collect(Side *side)
{
    int mtocollect;
    char *arg;
    const char *rest;

    REQUIRE_UNIT();
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
    if (curunit->plan)
      net_set_collect_task(curunit, mtocollect, curunit->x, curunit->y);
}

void
do_copying(Side *side)
{
    cur_help_node = copying_help_node;
    do_help(side);
}

/* Supposedly you could only get to these by typing the full command names. */

void
do_dir(Side *side)
{
    int ndirs, dir1, dir2, modif;

    ndirs = char_to_dir(tmpkey, &dir1, &dir2, &modif);
    if (ndirs >= 1) {
	do_dir_2(dir1, prefixarg);
    } else {
	xbeep();
	return;
    }
}

void
do_dir_multiple(Side *side)
{
    cmd_error(side, "use the single-character commands instead");
}

/* Determine how far away another point is.  */

void
do_distance(Side *side)
{
    int x, y;

    if (ask_position("Distance to where?", &x, &y)) {
	notify(dside, "Distance is %d cells.", distance(curx, cury, x, y));
    }
}

void
do_escape(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

/* Command to fire at a specified unit or location. */

void
do_fire(Side *side)
{
    int x, y;
    Unit *unit2;
    UnitView *uview;

    REQUIRE_UNIT();
    sprintf(spbuf, "Fire %s at where?", unit_handle(dside, curunit));
    /* (should have some sort of range feedback) */
    if (ask_position(spbuf, &x, &y)) {
	for_all_view_stack(dside, x, y, uview) {
	    unit2 = view_unit(uview);
	    /* (should avoid shooting at allies too) */
	    if (unit2 != NULL
		&& unit2->side != curunit->side
		&& uview->date == g_turn()) {
		net_prep_fire_at_action(curunit, curunit, unit2, -1);
		return;
	    }
	}
	notify(dside, "No target unit seen, firing blindly");
	net_prep_fire_into_action(curunit, curunit, x, y, 0, -1);
    }
}

void
do_fire_into(Side *side)
{
    int x, y;

    REQUIRE_UNIT();
    sprintf(spbuf, "Fire %s at where?", unit_handle(dside, curunit));
    /* (should have some sort of range feedback) */
    if (ask_position(spbuf, &x, &y)) {
	net_prep_fire_into_action(curunit, curunit, x, y, 0, -1);
    }
}

void
do_flash(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

/* Toggle the action following flag. */

void
do_follow_action(Side *side)
{
    follow_action = !follow_action;
    if (follow_action) {
	notify(dside, "Following the action.");
    } else {
	notify(dside, "Not following the action.");
    }
}

/* Give a unit to another side or "to" independence. */

void
do_give_unit(Side *side)
{
    REQUIRE_UNIT();
#ifdef DESIGNERS
    if (dside->designer) {
	net_designer_change_side(curunit, side_n(prefixarg));
	return;
    }
#endif /* DESIGNERS */
    if (1) { /* (should test both temporary and permanent invalidity) */
	net_prep_change_side_action(curunit, curunit, side_n(prefixarg));
    } else {
	cmd_error(side, "You can't just give away the %s!", unit_handle(dside, curunit));
    }
}

/* Bring up help info. */

void
do_help(Side *side)
{
    /* Switch to help mode, saving current mode first. */
    prevmode = mode;
    mode = HELP;
    show_help();
    refresh();
}

/* Set the display of various kinds of data. */

void
do_map(Side *side)
{
    int value;
    char *str, *str2, tmpbuf[BUFSIZE];

    if (cmdargstr) {
	str = cmdargstr;
	while (*str != '\0') {
	    /* Collect the next whitespace-separated token from the
               arg string. */
	    while (*str != '\0' && *str == ' ')
	      ++str;
	    str2 = tmpbuf;
	    while (*str != '\0' && *str != ' ')
	      *str2++ = *str++;
	    *str2 = '\0';
	    str2 = tmpbuf;
	    value = TRUE;
	    /* See if it is prefixed with a "-" or "no". */
	    if (*str2 == '-') {
		value = FALSE;
		++str2;
	    } else if (*str2 == 'n' && *(str2+1) == 'o') {
		value = FALSE;
		str2 += 2;
	    }
	    if (strcmp(str2, "terrain") == 0 || strcmp(str2, "t") == 0) {
		drawterrain = value;
	    } else if (strcmp(str2, "unit") == 0 || strcmp(str2, "u") == 0) {
		drawunits = value;
	    } else if (strcmp(str2, "name") == 0 || strcmp(str2, "n") == 0) {
		drawnames = value;
	    } else if (strcmp(str2, "people") == 0 || strcmp(str2, "p") == 0) {
		drawpeople = value;
	    } else if (strcmp(str2, "cover") == 0 || strcmp(str2, "c") == 0) {
		draw_cover = value;
	    } else if (strcmp(str2, "one") == 0 || strcmp(str2, "1") == 0) {
		use_both_chars = !value;
	    } else if (strcmp(str2, "two") == 0 || strcmp(str2, "2") == 0) {
		use_both_chars = value;
	    } else if (strncmp(str2, "lin", 3) == 0) {
		drawlinear = value;
		if (value && str2[3] == '=' && str2[4] != '\0') {
		    linear_char = str2[4];
		}
	    } else if (strcmp(str2, ">") == 0) {
		if (prefixarg < 0)
		  prefixarg = 5;
		if (lw <= 5 && prefixarg > 0) {
		    cmd_error(side, "list side must be at least 5");
		    return;
		}
		if (lw - prefixarg < 5)
		  prefixarg = lw - 5;
		resize_map(prefixarg);
		return;
	    } else if (strcmp(str2, "<") == 0) {
		if (prefixarg < 0)
		  prefixarg = 5;
		if (mw <= 10 && prefixarg > 0) {
		    cmd_error(side, "map side must be at least 10");
		    return;
		}
		if (mw - prefixarg < 10)
		  prefixarg = mw - 10;
		resize_map(0 - prefixarg);
		return;
	    } else if (strcmp(str2, "_") == 0) {
		if (prefixarg < 0)
		  prefixarg = 5;
		if (prefixarg < 1)
		  prefixarg = 1;
		if (prefixarg > 10)
		  prefixarg = 10;
		infoh = prefixarg;
		mh = LINES - 2 - infoh - 1;
		closeupwin->h = infoh;
		mapwin->y = 2 + infoh;
		mapwin->h = mh + 1;
		/* Update the screen to reflect the changes. */
		set_scroll();
		redraw();
	    } else if (strcmp(str2, "v") == 0) {
		if (prefixarg < 0) {
		    cycle_list_type();
		} else if (prefixarg == 0) {
		    cycle_list_filter();
		} else if (prefixarg == 1) {
		    cycle_list_order();
		}
		show_list();
		refresh();
	    } else {
		cmd_error(side, "\"%s\" not recognized", tmpbuf);
	    }
	}
    } else {
	notify(dside, "Nothing to do.");
    }
    show_map();
    refresh();
}

static void
resize_map(int n)
{
    /* Resize the left-hand-side windows. */
    mw += n;
    closeupwin->w += n;
    mapwin->w += n;
    /* Move and resize the right-hand-side windows. */
    lw -= n;
    sideswin->x += n;
    sideswin->w -= n;
    listwin->x += n;
    listwin->w -= n;
    /* Update the screen to reflect the changes. */
    set_scroll();
    redraw();
}

/* Send a short message to another side. */

void
do_message(Side *side)
{
    char *msg;
    Side *side2;
    SideMask sidemask;

    if (prefixarg == 0) {
	/* (should ask who to send to) */
    }
    side2 = side_n(prefixarg);
    if (ask_string("Message:", "", &msg)) {
	if (empty_string(msg) || (prefixarg >= 0 && side2 == NULL)) {
	    notify(dside, "You keep your mouth shut.");
	    sidemask = NOSIDES;
	} else if (prefixarg < 0) {
	    notify(dside, "You made the announcement \"%s\"", msg);
	    sidemask = ALLSIDES;
	} else if (side2 != NULL) {
	    notify(dside, "Your message was sent.");
	    sidemask = add_side_to_set(side2, NOSIDES);
	}
	if (!empty_string(msg) && sidemask != NOSIDES)
	  net_send_message(dside, sidemask, msg);
    }
}

/* Set unit to move to a given location.  Designers do a teleport. */

void
do_move_to(Side *side)
{
    int x, y;

    REQUIRE_UNIT();
    if (!is_designer(side)) {
	if (!mobile(curunit->type)) {
	    cmd_error(side, "%s cannot move at all!", unit_handle(side, curunit));
	    return;
	}
    }
    sprintf(spbuf, "Move %s to where?", unit_handle(dside, curunit));
    if (ask_position(spbuf, &x, &y)) {
	if (impl_move_to(side, curunit, x, y, 0)) {
	    return;
	} else {

	}
    }
}

/* Command to name or rename the current unit or a given side. */

void
do_name(Side *side)
{
    char *newname;

    REQUIRE_UNIT();
    if (ask_string("New name for unit:", curunit->name, &newname)) {
	if (empty_string(newname))
	  newname = NULL;
	net_set_unit_name(dside, curunit, newname);
    }
}

void
do_new_map(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_occupant(Side *side)
{
    Unit *nextocc;

    if (curunit == NULL) {
	make_current_at(curx, cury);
    }
    REQUIRE_UNIT();
    nextocc = find_next_occupant(curunit);
    if (nextocc != curunit)
      make_current(nextocc);
}

void
do_orders_popup(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_other(Side *side)
{
    char *cmd;

    if (ask_string("Command:", NULL, &cmd)) {
	if (empty_string(cmd)) {
	    cmd_error(side, "No command");
	} else if (strcmp(cmd, "?") == 0) {
	    cur_help_node = long_commands_help_node;
	    do_help(side);
	} else {
	    execute_long_command(side, cmd);
	}
    }
}

void
do_print_view(Side *side)
{
    dump_text_view(dside, use_both_chars);
}

void
do_produce(Side *side)
{
    int m, n;
    Unit *unit = curunit;

    REQUIRE_UNIT();
    if (!can_produce(unit)) {
	cmd_error(side, "cannot do active production");
    }
    n = 9999;
    if (prefixarg > 0)
      n = prefixarg;
    /* Find the first produceable type and set up to produce it. */
    for_all_material_types(m) {
	if (um_acp_to_produce(unit->type, m) > 0) {
	    net_push_produce_task(unit, m, n);
	    return;
	}
    }
}

/* Command to get out of a game, one way or another. */

void
do_quit(Side *side)
{
    if (endofgame || beforestart || !dside->ingame) {
	exit_cconq();
	return;
    }
    /* Confirm the saving of any state. */
    if (!gamestatesafe) {
	if (ask_bool("Do you want to save the game?", TRUE)) {
	    if (all_others_willing_to_save(dside)) {
		do_save(side);
		exit_cconq();
		return;
	    } else {
		net_set_willing_to_save(dside, TRUE);
		notify(dside, "Other sides not willing to save.");
	    }
	}
    }
    if (all_others_willing_to_quit(dside)) {
	if (ask_bool("Do you really want to declare a draw?", FALSE)) {
	    net_set_willing_to_draw(dside, TRUE);
	} else {
	    notify(dside, "Not willing to draw.");
	}
	return;
    } else {
	if (ask_bool("You must resign to get out; do you want to resign?", FALSE)) {
	    do_resign(side);
	} else {
	    notify(dside, "Not resigning.");
	}
    }
}

/* Move the current location as close to the center of the display as
   possible, and redraw everything. */

void
do_recenter(Side *side)
{
    set_view_focus(mvp, curx, cury);
    center_on_focus(mvp);
    set_map_viewport();
    show_map();
    refresh();
}

/* Redraw everything using the same code as when windows need a redraw. */

void
do_refresh(Side *side)
{
    redraw();
}

void
do_remove_terrain(Side *side)
{
    int t, dir;

    REQUIRE_UNIT();
    if (ask_direction("Remove terrain from where?", &dir)) {
      for_all_terrain_types(t) {
	if (ut_acp_to_remove_terrain(curunit->type, t) > 0
	    && curunit->act
	    && curunit->act->acp >= ut_acp_to_remove_terrain(curunit->type, t)) {
	    if (0 <= ut_alter_range(curunit->type, t)) {
		if (net_prep_remove_terrain_action(curunit, curunit, curunit->x, curunit->y, dir, t))
		  ;
		else
		  xbeep();
	    }
	}
      }
    }
}

void
do_repair(Side *side)
{
    cmd_error(side, "repair command not implemented");
}

void
do_resign(Side *side)
{
    Side *side2;

    if (endofgame) {
	cmd_error(side, "Game is already over.");
    } else if (!dside->ingame) {
	cmd_error(side, "You are already out of the game.");
    } else if (ask_bool("Do you really want to resign?", FALSE)) {
	side2 = NULL;
	if (numsides > 2) {
	    side2 = ask_side("Who do you want to inherit?", NULL);
	    if (side2 == dside) {
		cmd_error(side, "You can't inherit your own units! (not giving to anybody)");
		side2 = NULL;
	    }
	}
	net_resign_game(dside, side2);
    }
}

/* Stuff game state into a file.  By default, it goes into the current
   directory.  If building a scenario, we can specify just which parts
   of the game state are to be written. */

void
do_save(Side *side)
{
    char *rawcontents;
    Module *module;
    Obj *contents;

#ifdef DESIGNERS
    if (dside->designer) {
	if (ask_string("Data to write?", "everything", &rawcontents)) {
	    /* (should be in a designer_create_module?) */
	    /* need to be able to get this name from somewhere */
	    module = create_game_module("random.scn");
	    /* need something better to turn contents into a Lisp object */
	    contents = intern_symbol(rawcontents);
	    /*	interpret_content_spec(module, contents);  */
	    notify(dside, "Module will be written to \"%s\" ...", module->filename);
	    /* This seems broken anyway, but we add module->filename to
	    make the code compile ... */
	    if (write_game_module(module, module->filename)) {
		notify(dside, "Done writing to \"%s\".", module->filename);
	    } else {
		cmd_error(side, "Can't open file \"%s\"!", module->filename);
	    }
	    return;
	} else {
	    return;
	}
    }
#endif /* DESIGNERS */
    if (0 /* checkpointing not allowed */) {
	if (ask_bool("You really want to save and exit?", FALSE)) {
	    notify(dside, "Game will be saved to \"%s\" ...", saved_game_filename());
	    if (write_entire_game_state(saved_game_filename())) {
		close_displays();
		/* this should be conditional? */
		exit(0);
	    } else {
		cmd_error(side, "Can't open file \"%s\"!", saved_game_filename());
	    }
	}
    } else {
	notify(dside, "Saving...");
	if (write_entire_game_state(saved_game_filename())) {
	    notify(dside, "Game saved.");
	} else {
	    cmd_error(side, "Couldn't save to \"%s\"!", saved_game_filename());
	}	    
    }
}

void
do_set_formation(Side *side)
{
    Unit *leader;

    REQUIRE_UNIT();
    sprintf(spbuf, "Which unit to follow?");
    if (ask_unit(spbuf, &leader)) {
	if (!in_play(leader)) {
	    cmd_error(side, "No unit to follow!");
	} else if (leader == curunit) {
	    cmd_error(side, "Unit can't follow itself!");
	} else if (leader->side != dside /* or "trusted side"? */) {
	    cmd_error(side, "Can't follow somebody else's unit!");
	} else {
	    net_set_formation(curunit, leader, curunit->x - leader->x, curunit->y - leader->y, 1, 1);
	}
    }
}

void
do_set_view_angle(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_side_closeup(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_standing_orders(Side *side)
{
    int rslt;

    if (cmdargstr) {
	rslt = parse_standing_order(dside, cmdargstr);
	if (rslt < 0)
	  xbeep();
    } else
      xbeep();
}

/* Command to toggle between interaction modes. */

void
do_survey(Side *side)
{
    if (mode == MOVE) {
	lastactor = curunit;
	mode = SURVEY;
    } else {
	mode = MOVE;
	/* If we weren't looking at a unit when we switched modes,
	   go back to the last unit that was being moved. */
	if (curunit == NULL && in_play(lastactor)) {
	    make_current(lastactor);
	}
    }
    show_map();
    refresh();
}

void
do_unit_closeup(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_up(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

/* Display the program version. */

void
do_version(Side *side)
{
    notify(dside, "Curses Xconq version %s", version_string());
    notify(dside, "(c) %s", copyright_string());
}

void
do_warranty(Side *side)
{
    cur_help_node = warranty_help_node;
    do_help(side);
}

void
do_world_map(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_zoom_in(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_zoom_out(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

#ifdef DESIGNERS

void
do_design(Side *side)
{
    if (!dside->designer) {
	net_become_designer(dside);
    } else {
	net_become_nondesigner(dside);
    }
}

#endif /* DESIGNERS */

#ifdef DEBUGGING

void
do_profile(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

void
do_trace(Side *side)
{
    cmd_error(side, "No curses version of this command.");
}

#endif /* DEBUGGING */

/* Generic command error routine just does a notify. */

void
cmd_error(Side *side, const char *fmt, ...)
{
    char tmpnbuf[BUFSIZE];
    va_list ap;

    if (!empty_string(fmt)) {

	va_start(ap, fmt);
	vsnprintf(tmpnbuf, sizeof tmpnbuf, fmt, ap);
	va_end(ap);

	low_notify(dside, tmpnbuf);
    }
    xbeep();
}

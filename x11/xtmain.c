/* Toplevel code for the X11 Xt/Xaw interface to Xconq.
   Copyright (C) 1987-1989, 1991-1998, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "xtconq.h"
extern void init_emblem(Side *side, Side *side2);

extern String fallback_resources[];

/* Local function declarations. */

static void quit_program(Widget wdgt, XEvent *event, String *params,
			 Cardinal *num_params);
static void run_game_proc(XtPointer client_data, XtIntervalId *id);
static Boolean run_game_idle(XtPointer clientdata);
static void animate_current_proc(XtPointer client_data, XtIntervalId *id);
static void animate_fire_proc(XtPointer client_data, XtIntervalId *id);

/* X-specific options, plus some NULL definitions so that they'll be
   ignored by X parsing and get passed through to the generic Xconq
   command-line parser. */

XrmOptionDescRec xoptions[] = {
    { "-background",	"*background",	XrmoptionSepArg,	NULL },
    { "-bg",		"*background",	XrmoptionSepArg,	NULL },
    { "-display",	".display",	XrmoptionSepArg,	NULL },
    { "-f",		NULL,		XrmoptionSkipArg,	NULL },
    { "-fg",		"*foreground",	XrmoptionSepArg,	NULL },
    { "-fn",		"*font",	XrmoptionSepArg,	NULL },
    { "-font",		"*font",	XrmoptionSepArg,	NULL },
    { "-foreground",	"*foreground",	XrmoptionSepArg,	NULL },
    { "-g",		NULL,		XrmoptionSkipArg,	NULL },
    { "-geometry",	"*geometry",	XrmoptionSepArg,	NULL },
    { "-n",		NULL,		XrmoptionNoArg,		NULL },
    { "-name",		".name",	XrmoptionSepArg,	NULL },
    { "-xrm",		NULL,		XrmoptionResArg,	NULL }
};

int xoptions_count;

int nargs;
Arg tmpargs[100];

XtAppContext thisapp;

Widget thistoplevel;

Widget choiceshell = NULL;

Widget scenarioshell = NULL;

XtActionsRec quit_actions_table[] = {
    { "wm-quit", quit_program },
};

static void
quit_program(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    Side *side;
    Map *map;

    /* The side with toplevel matching widget just quit. */
    if (!find_side_and_map_via_a_toplevel(w, &side, &map))
      return;

    if (w == side->ui->shell) {
	/* Go into the standard quit command. */
	do_quit(side);
    } else if (w == side->ui->help_shell) {
	popdown_help(side);
    }
}

void
initial_ui_init(int argc, char *argv[])
{
    /* Do the usual Xt application setup. */
    /* Note that this opens one display by default, which means that
       later code should take note and not try to open this a second time.
       Also, this will absorb all X-related arguments - anything remaining
       is either a generic Xconq option or a mistake. */
    xoptions_count = XtNumber(xoptions);
    thistoplevel =
      XtAppInitialize(&thisapp, PROGRAMCLASSNAME,
		      xoptions, XtNumber(xoptions), &argc, argv,
		      fallback_resources, (ArgList) NULL, 0);
}

void
ui_init(void)
{
    /* And, perform some once per application context initialization... */
    XtAppAddActions(thisapp, quit_actions_table, XtNumber(quit_actions_table));
    add_map_actions();
    XtAppAddTimeOut(thisapp, 10, run_game_proc, NULL);
    XtAppAddTimeOut(thisapp, 200, animate_current_proc, NULL);
    XtAppAddWorkProc(thisapp, run_game_idle, NULL);
}

void
ui_mainloop(void)
{
    XtAppMainLoop(thisapp);
}

/* This is a timed call; it gets called periodically. */

static void
run_game_proc(XtPointer client_data, XtIntervalId *id)
{
    int rslt = 0;
    unsigned long interval;

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    run_local_ai(1, 20);
    /* Run the kernel itself. */
    rslt += net_run_game(1);

#if 0
    run_local_ai(2, 20);
#endif

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    /* Set up to call it again in a little while. */
    /* If things are happening, call 40 times/sec, for responsiveness. */
    interval = 25;
    /* If nothing is happening right now, do at 4 times/sec. */
    if (rslt == 0)
      interval = 250;
    XtAppAddTimeOut(thisapp, interval, run_game_proc, NULL);
}

static Boolean
run_game_idle(XtPointer clientdata)
{
    Side *side;
    Map *map;
    Unit *unit;

    /* See if we should jump to another unit and make it current. */
    for_all_sides(side) {
	if (active_display(side)) {
	    for_all_maps(side, map) {
		if (map->curtool == movetool) {
		    unit = autonext_unit_inbox(side, map->curunit, map->vp);
		    if (unit != NULL) {
		      set_current_unit(side, map, unit);
		    /* Clear the current unit if it is no longer valid. */
		    } else {
		      clear_current(side, map);
		    }
		/* Also clear it in survey mode if invalid. */
		} else if (!in_play(map->curunit)
			|| !side_controls_unit(side, map->curunit)) {
		    clear_current(side, map);
		}
	    }
	}
    }
    /* Call us again! */
    return False;
}

/* This is a timed call; it gets called periodically. */

static void
animate_current_proc(XtPointer client_data, XtIntervalId *id)
{
    Side *side;
    Map *map;

    for_all_sides(side) {
	if (active_display(side)) {
	    for_all_maps(side, map) {
		map->anim_state = (map->anim_state + 1) % 8;
		if (map->curtool == movetool && map->curunit != NULL) {
		    draw_current(side, map);
		}
	    }
	}
    }
    XtAppAddTimeOut(thisapp, 500, animate_current_proc, NULL);
}

#if 0
void
init_x_signal_handlers()
{
    XSetErrorHandler(handle_x_error);
    XSetIOErrorHandler(handle_xio_error);
    XtAppSetErrorHandler(thisapp, handle_xt_error);
}

/* Handlers for X catastrophes attempt to do a save first. */

static int
handle_x_error (Display *dpy, XErrorEvent *evt)
{
    static int num_errors = 0;
    char buf[BUFSIZE];

    XGetErrorText(dpy, evt->error_code, buf, BUFSIZE);
    fprintf(stderr, "\nX error on display %s: %s\n", DisplayString(dpy), buf);
    if (++num_errors >= 10) {
        printf("\nX error: trying emergency save!\n");
	/* Note that if the save fails too, we're totally hosed. */
	/* (should use configurable name here) */
        write_entire_game_state("ack!.xconq");
#ifdef __cplusplus
	throw "snafu";
#else
	abort();
#endif
    }
    return 0;
}

static int
handle_xio_error (Display *dpy)
{
    fprintf(stderr, "\nX IO error on display %s: trying emergency save!\n",
	   DisplayString(dpy));
    write_entire_game_state("ack!.xconq");
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
    return 0;
}

static void
handle_xt_error(String msg)
{
    fprintf(stderr, "Xt error: %s\n", msg);
    /* Get a core dump to debug with. */
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
}
#endif

/* All update_xxx_display callbacks are here. */

void
update_area_display(Side *side)
{
  if (active_display(side))
    redraw(side);
}

extern int suppress_update_cell_display;

/* Draw an individual detailed hex, as a row of one, on all maps. */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    Map *map;

    if (active_display(side) && !suppress_update_cell_display) {
	for_all_maps(side, map) {
	    /* If update was only to report changes in undrawn info,
	       maybe don't bother. */
	    if (!((flags & UPDATE_ALWAYS)
		  || ((flags & UPDATE_COVER) && map->vp->draw_cover)
		  || ((flags & UPDATE_TEMP) && map->vp->draw_temperature)
		  || ((flags & UPDATE_CLOUDS) && map->vp->draw_clouds)
		  || ((flags & UPDATE_WINDS) && map->vp->draw_winds)
		  ))
	      continue;
	    draw_row(side, map, x, y, 1, TRUE);
	    if (map->curunit && map->curunit->x == x && map->curunit->y == y) {
		draw_current(side, map);
	    }
	}
	flush_output(side);
    }
}

/* The kernel calls this to update info about the given side. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    Map *map;

    if (active_display(side)) {
	/* If the side never had its emblem image loaded (such as when
	   it's added during the game), do it now. */
	if (!side->ui->eimages_loaded[side_number(side2)])
	  init_emblem(side, side2);
	for_all_maps(side, map) {
	    draw_side_info(side, map, side2);
	}
	if (rightnow)
	  flush_output(side);
    }
}

void
update_research_display(Side *side)
{
}

/* The kernel calls this to update info about the given unit. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    Map *map;

    if (active_display(side) && unit != NULL) {
	if (inside_area(unit->x, unit->y)) {
	    update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
	}
	/* Redraw any/all info about unit and its side. */
	for_all_maps(side, map) {
	    draw_map_info(side, map);
	    draw_side_info(side, map, unit->side);
	    update_unit_type_list(side, map, unit->type);
	}
	if (rightnow)
	  flush_output(side);
    }
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side)) {
    }
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    Action *action;
    Unit *unit2;
    char *unit2handle = NULL;

    if (active_display(side)) {
	action = &(unit->act->nextaction);
	switch (rslt) {
	  case A_ANY_DONE:
	    /* (anything worthwhile to show?) */
	    break;
	  case A_ANY_TOO_FAR:
	    if (action->type == ACTION_FIRE_AT) {
	      unit2 = find_unit(action->args[0]);
	      if (unit2) {
		unit2handle = unit_handle(side, unit2);
		if (unit2handle)
		  unit2handle = copy_string(unit2handle);
	        notify(side, "Distance to %s (%d) is out of range (%d) of %s.",
			unit2handle ? unit2handle : "target",
			distance(unit->x, unit->y, unit2->x, unit2->y),
			u_range(unit->type),
			unit_handle(side, unit));
		if (unit2handle)
			free(unit2handle);
	      } else {
		notify(side, "That is out of range of your %s.",
			unit_handle(side, unit));
	      }
	    }
	    break;
	  default:
	    break;
	}
	if (Debug) {
	    notify(side, "%s %s %s!", unit_desig(unit),
		   action_desig(action), hevtdefns[rslt].name);
	}
    }
}

/* The kernel calls this to update the global game state. */

void
update_turn_display(Side *side, int rightnow)
{
    int u;
    Map *map;
    Side *side2;

    if (active_display(side)) {
	for_all_maps(side, map) {
	    draw_game_state(side, map);
	    draw_view_in_panner(side, map);
	    for_all_unit_types(u) {
		update_unit_type_list(side, map, u);
	    }
	}
	if (endofgame && !side->ui->told_outcome) {
	    if (!side->see_all) {
		for_all_maps(side, map) {
		    update_controls(side, map);
		    draw_map(side, map);
		}
	    }
	    for_all_sides(side2) {
		update_side_display(side, side2, TRUE);
	    }
	    /* Set a flag so we do all this only once. */
	    side->ui->told_outcome = TRUE;
	}
	if (rightnow)
	  flush_output(side);
    }
}

void
update_action_display(Side *side, int rightnow)
{
    if (active_display(side)) {
	/* show state of actions */
	if (rightnow)
	  flush_output(side);
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
    Map *map;

    if (active_display(side)) {
	switch (hevt->type) {
	  case H_GAME_ENDED:
	    /* Reflect just-gained ability to toggle seeall. */
	    for_all_maps(side, map) {
		update_controls(side, map);
	    }
	    break;
	  default:
	    break;
	}
	if (rightnow)
	  flush_output(side);
    }
}

struct fire_state {
    Side *side;
    Map *map;
    int sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2;
    int step;
} firestates[10];

static void
animate_fire_proc(XtPointer client_data, XtIntervalId *id)
{
    int which = (int) client_data;
    int sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2, dx, dy, xx, yy;
    Side *side;
    Map *map;

    side = firestates[which].side;
    map = firestates[which].map;
    sx1 = firestates[which].sx1;  sy1 = firestates[which].sy1;
    sw1 = firestates[which].sw1;  sh1 = firestates[which].sh1;
    sx2 = firestates[which].sx2;  sy2 = firestates[which].sy2;
    sw2 = firestates[which].sw2;  sh2 = firestates[which].sh2;
    XSetFunction(side->ui->dpy, side->ui->gc, GXinvert);
    XSetLineAttributes(side->ui->dpy, side->ui->gc,
		       2, LineSolid, CapButt, JoinMiter); 
    compute_fire_line_segment(sx1 + sw1 / 2, sy1 + sh1 / 2,
			      sx2 + sw2 / 2, sy2 + sh2 / 2,
			      firestates[which].step, 4, &xx, &yy, &dx, &dy);
    /* Draw one segment of a line between the units. */
    if (dx > 0 || dy > 0) {
	XDrawLine(side->ui->dpy, map->viewwin, side->ui->gc,
		  xx, yy, xx + dx, yy + dy);
	flush_output(side);
    }
    XSetFunction(side->ui->dpy, side->ui->gc, GXcopy);
    XSetLineAttributes(side->ui->dpy, side->ui->gc,
		       1, LineSolid, CapButt, JoinMiter); 
    ++(firestates[which].step);
    if (firestates[which].step < 24) {
	XtAppAddTimeOut(thisapp, 150, animate_fire_proc, NULL);
    }
}

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int rightnow)
{
    int sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2;
    char *xxx, extrabuf[BUFSIZE];
    Map *map;
    
    if (active_display(side)) {
	for_all_maps(side, map) {
	    x_xform_unit(side, map, unit, &sx1, &sy1, &sw1, &sh1);
	    x_xform_unit(side, map, unit2, &sx2, &sy2, &sw2, &sh2);
	    firestates[0].side = side;
	    firestates[0].map = map;
	    firestates[0].sx1 = sx1;  firestates[0].sy1 = sy1;
	    firestates[0].sw1 = sw1;  firestates[0].sh1 = sh1;
	    firestates[0].sx2 = sx2;  firestates[0].sy2 = sy2;
	    firestates[0].sw2 = sw2;  firestates[0].sh2 = sh2;
	    firestates[0].step = 0;
	    XtAppAddTimeOut(thisapp, 10, animate_fire_proc, NULL);
	}
	xxx = unit_handle(side, unit);
	strcpy(extrabuf, xxx);
	notify(side, "%s fired at %s!", extrabuf, unit_handle(side, unit2));
    }
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m, int rightnow)
{
    int i, sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2, dx, dy, xx, yy;
    Map *map;
    
    if (active_display(side)) {
	XSetFunction(side->ui->dpy, side->ui->gc, GXinvert);
	XSetLineAttributes(side->ui->dpy, side->ui->gc, 2, LineSolid, CapButt, JoinMiter); 
	i = 0;
	while (i < 12) {  /* should be a timed loop */
	    for_all_maps(side, map) {
		x_xform_unit(side, map, unit, &sx1, &sy1, &sw1, &sh1);
		xform(side, map, x, y, &sx2, &sy2);
		sw2 = map->vp->hw;  sh2 = map->vp->hh;
		compute_fire_line_segment(sx1 + sw1 / 2, sy1 + sh1 / 2,
					  sx2 + sw2 / 2, sy2 + sh2 / 2,
					  i, 4, &xx, &yy, &dx, &dy);
		if (dx > 0 || dy > 0) {
		    XDrawLine(side->ui->dpy, map->viewwin, side->ui->gc,
			      xx, yy, xx + dx, yy + dy);
		    flush_output(side);
		}
	    }
	    ++i;
	}
	/* (should clean up after drawing) */
	notify(side, "%s fired into %d,%d!",
	       unit_handle(side, unit), x, y);
	XSetFunction(side->ui->dpy, side->ui->gc, GXcopy);
	XSetLineAttributes(side->ui->dpy, side->ui->gc, 1, LineSolid, CapButt, JoinMiter); 
    }
}

/* Updates to clock need to be sure that display changes immediately. */

void
update_clock_display(Side *side, int rightnow)
{
    Map *map;

    if (active_display(side)) {
	for_all_maps(side, map) {
	    draw_game_clocks(side, map);
	}
	if (rightnow)
	  flush_output(side);
    }
}

void
update_message_display(Side *side, Side *sender, char *str, int rightnow)
{
    if (active_display(side)) {
	notify(side, "From %s: \"%s\"",
	       (sender != NULL ? short_side_title(sender) : "<anon>"), str);
	if (rightnow)
	  flush_output(side);
    }
}

void
update_all_progress_displays(char *str, int s)
{
}

void
action_point(Side *side, int x, int y)
{
    Map *map;

    if (!active_display(side))
      return;
    if (!inside_area(x, y))
      return;

    for_all_maps(side, map) {
	if (map->follow_action && !in_middle(side, map, x, y)) {
	    put_on_screen(side, map, x, y);
	}
    }    
}

void
flush_display_buffers(Side *side)
{
    if (active_display(side)) {
	flush_output(side);
    }
}

void
update_everything()
{
    Side *side;

    for_all_sides(side) {
	if (active_display(side)) {
	  /* Possibly revent batching-up of server actions, so that if
	     you stop in the middle of drawing operations, the screen
	     reflects what has been sent to the server so far. */
	  XSynchronize(side->ui->dpy, DebugG);
	    /* (update all maps?) */
	}
    }
}

/* Close all displays that are still open. */

void
close_displays()
{
    Side *side;

    for_all_sides(side) {
	if (active_display(side))
	  close_display(side);
    }
}

/* Get rid of extra input events, generally because the situation has
   changed drastically and they are no longer of interest.  This routine
   has relatively few valid uses. */

void
flush_input(Side *side)
{
    DGprintf("doing an input flush\n");
    if (active_display(side)) {
	XSync(side->ui->dpy, TRUE);  
    }
}

/* Build a X-compatible widget name, by replacing non-alphanumerics with
   underscores. */

void
build_name(char *name, char *first, char *second)
{
    char *ch;

    strcpy(name, first);
    strcat(name, second);
    for (ch = name; *ch != '\0'; ++ch) {
	if (!isalnum(*ch))
	  *ch = '_';
    }
}

#ifdef DEBUGGING

/* Set all the window backgrounds, borders, GCs, etc, usually to reflect
   inversion of foreground and background. */

void
reset_color_state(Side *side)
{
    XGCValues values;
    unsigned int gcmask = GCForeground | GCBackground;

    /* Set the GCs. */
    values.foreground = side->ui->fgcolor;
    values.background = side->ui->bgcolor;
    XChangeGC(side->ui->dpy, side->ui->gc, gcmask, &values);
    XChangeGC(side->ui->dpy, side->ui->textgc, gcmask, &values);
    XChangeGC(side->ui->dpy, side->ui->ltextgc, gcmask, &values);
    XChangeGC(side->ui->dpy, side->ui->unitgc, gcmask, &values);
}

void
reset_window_colors(Side *side, Window win)
{
    XSetWindowBackground(side->ui->dpy, win, side->ui->bgcolor);
    XSetWindowBorder(side->ui->dpy, win, side->ui->fgcolor);
}

#endif /* DEBUGGING */

/* Completely redo all windows, making no assumptions about appearance.
   This is a last-gasp measure, most redrawing should be restricted
   to only the directly affected windows.  Also this shouldn't be
   done without the user's permission, since it will blow away impending
   input. */

void
redraw(Side *side)
{
    draw_all_maps(side);
    /* (should do popups also?) */
    flush_output(side);
    flush_input(side);
}

/* Trivial abstraction - sometimes other routines want to ensure all output
   so far is actually on the screen and not being buffered up somewhere. */

void
flush_output(Side *side) 
{  
    XFlush(side->ui->dpy);  
}

/* Beep the beeper! */

void
beep(Side *side)
{
    XBell(side->ui->dpy, side->ui->screen);
}

void
low_notify(Side *side, char *str)
{
    Map *map;

    if (!active_display(side))
      return;
    DGprintf("To %s: %s\n", side_desig(side), str);
    /* Paste into a subwindow of each map window. */
    for_all_maps(side, map) {
	textw_printf(map->history, "%s\n", str);
    }
}

int
schedule_movie(Side *side, char *movie, ...)
{
    int i;
    enum movie_type itype;

    if (!active_display(side))
      return FALSE;
    if (side->ui->numscheduled >= 10)
      return FALSE;
    memset(&(side->ui->movies[side->ui->numscheduled]), 0, sizeof(struct a_movie));
    side->ui->movies[side->ui->numscheduled].type = movie;
    itype = movie_null;
    if (strcmp(movie, "miss") == 0)
      itype = movie_miss;
    else if (strcmp(movie, "hit") == 0)
      itype = movie_hit;
    else if (strcmp(movie, "death") == 0)
      itype = movie_death;
    else if (strcmp(movie, "nuke") == 0)
      itype = movie_nuke;
    else if (strcmp(movie, "sound") == 0)
      itype = movie_sound;
    else if (strcmp(movie, "flash") == 0)
      itype = movie_flash;
    side->ui->movies[side->ui->numscheduled].itype = itype;
    {
	va_list ap;

	va_start(ap, movie);
	for (i = 0; i < 5; ++i)
	  side->ui->movies[side->ui->numscheduled].args[i] = va_arg(ap, int);
	va_end(ap);
    }
    ++side->ui->numscheduled;
    return TRUE;
}

void
play_movies(SideMask sidemask)
{
    int j, unitid, sx, sy, sw, sh;
    Map *map;
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	if (side_in_set(side, sidemask) && active_display(side)) {
	    for (j = 0; j < side->ui->numscheduled; ++j) {
		switch (side->ui->movies[j].itype) {
		  case movie_null:
		    break;
		  case movie_miss:
		    unitid = side->ui->movies[j].args[0];
		    unit = find_unit(unitid);
		    if (unit == NULL || !in_area(unit->x, unit->y))
		      continue;
		    for_all_maps(side, map) {
			x_xform_unit(side, map, unit, &sx, &sy, &sw, &sh);
			draw_blast_image(side, map, sx, sy, sw, sh, 0);
		    }
		    break;
		  case movie_hit:
		    unitid = side->ui->movies[j].args[0];
		    unit = find_unit(unitid);
		    if (unit == NULL || !in_area(unit->x, unit->y))
		      continue;
		    for_all_maps(side, map) {
			x_xform_unit(side, map, unit, &sx, &sy, &sw, &sh);
			draw_blast_image(side, map, sx, sy, sw, sh, 1);
		    }
		    break;
		  case movie_death:
		    unitid = side->ui->movies[j].args[0];
		    unit = find_unit(unitid);
		    if (unit == NULL || !in_area(unit->x, unit->y))
		      continue;
		    for_all_maps(side, map) {
			x_xform_unit(side, map, unit, &sx, &sy, &sw, &sh);
			draw_blast_image(side, map, sx, sy, sw, sh, 2);
		    }
		    break;
		  case movie_nuke:
		    break;
		  case movie_flash:
		    break;
		  default:
		    break;
		}
	    }
	    side->ui->numscheduled = 0;
	}
    }
}

/* General text drawer. */

void
draw_text(Side *side, Window win, int x, int y, char *str, int color)
{
    XFontStruct *font = side->ui->textfont;
    GC gc = side->ui->textgc;
    Display *dpy = side->ui->dpy;

    y += font->max_bounds.ascent;
    XSetForeground(dpy, gc, color);
    /* If "color" is bg, assume we want reverse video. */
    XSetBackground(dpy, gc, (color == side->ui->bgcolor ? side->ui->fgcolor : side->ui->bgcolor));
    XDrawImageString(dpy, win, gc, x, y, str, strlen(str));
#ifdef STUPIDFLUSH
    XFlush(dpy);
#endif /* STUPIDFLUSH */
}

/* Frequently-called routine to draw text in the foreground color. */

void
draw_fg_text(Side *side, Window win, int x, int y, char *str)
{
    draw_text(side, win, x, y, str, side->ui->fgcolor);
}

XawTextPosition
widget_text_length(Widget w)
{
    return XawTextSourceScan (XawTextGetSource (w),
			      (XawTextPosition) 0,
 			      XawstAll, XawsdRight, 1, TRUE);
}

/* Deiconify and raise the window corresponding to the given widget;
   to be used only with shell windows. */

void 
raise_widget(Widget w)
{
    XMapRaised(XtDisplay(w), XtWindow(w));
}

#if 0
void
move_caret_to_start(Widget w)
{
    XawTextSetInsertionPoint(w, (XawTextPosition) 0);
}
#endif

void
move_caret_to_end(Widget w)
{
    XawTextPosition pos;

    pos = widget_text_length(w);
    XawTextSetInsertionPoint(w, pos);
}

static int count_lines(Widget w);

static int
count_lines(Widget w)
{
    int	nlines;
    XawTextPosition	pos, len;

    len = widget_text_length(w);

    nlines = 0;
    pos = 0;
    while ((pos = XawTextSourceScan(XawTextGetSource(w),
				    pos, XawstEOL, XawsdRight, 1, TRUE))
	   != XawTextSearchError) {
	nlines++;
	if (pos >= len)
	  break;
    }
    return nlines;
}

static void drop_lines_from_top(Widget w, int num);

static void
drop_lines_from_top(Widget w, int num)
{
    int	nlines;
    XawTextPosition pos, len;
    XawTextBlock tb;
    XawTextEditType et;

    len = widget_text_length(w);

    nlines = num;
    pos = 0;
    while (nlines &&
	   (pos = XawTextSourceScan(XawTextGetSource(w),
				    pos, XawstEOL, XawsdRight, 1, TRUE))
	   != XawTextSearchError) {
	nlines--;
	if (pos >= len)
	  break;
    }
    
    if (pos == XawTextSearchError)
      return;
    
    tb.firstPos = 0;
    tb.length = 0;
    tb.ptr = NULL;
    tb.format = FMT8BIT;

    /* Can we write to it? */
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNeditType, &et);  nargs++;
    XtGetValues(w, tmpargs, nargs);

    if (et == XawtextRead) {
	/* Make it writable. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, XawtextEdit);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }
    
    XawTextReplace(w, (XawTextPosition) 0, pos, &tb);

    if (et == XawtextRead) {
	/* Set it back. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, et);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }

    move_caret_to_end(w);
}

void
textw_printf(const Widget w, const char *fmt, ...)
{
    XawTextBlock tb;
    XawTextPosition ins_point;
    XawTextEditType et;
    int nlines;
    char buf[BUFSIZ];

    {
	va_list vl;

	va_start(vl, fmt);
	(void) vsprintf(buf, fmt, vl);
	va_end(vl);
    }
    if (w == (Widget) NULL) {
	printf("%s", buf);
	return;
    }

    tb.ptr = buf;
    tb.length = strlen(buf);
    tb.format = FMT8BIT;
    tb.firstPos = 0;

    /* Can we write to it? */
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNeditType, &et);  nargs++;
    XtGetValues(w, tmpargs, nargs);

    if (et == XawtextRead) {
	/* Make it writable. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, XawtextEdit);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }
    move_caret_to_end(w);
    ins_point = widget_text_length(w);
    XawTextReplace(w, ins_point, ins_point, &tb);
    move_caret_to_end(w);
    if (et == XawtextRead) {
	/* Set it back. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, et);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }
    nlines = count_lines(w);
    /* Need to make this a resource. */
#define maxlines 256
    if (nlines > maxlines)
      drop_lines_from_top(w, nlines - maxlines);
#undef maxlines
}

#if 0
void textw_iprintf(const Widget w, const char *fmt, ...);

void
textw_iprintf(const Widget w, const char *fmt, ...)
{
    XawTextBlock tb;
    XawTextPosition	ins_point;
    XawTextEditType	et;
    char buf[BUFSIZ];

    {
	va_list		vl;

	va_start(vl, fmt);
	(void) vsprintf(buf, fmt, vl);
	va_end(vl);
    }
    tb.ptr = buf;
    tb.length = strlen(buf);
    tb.format = FMT8BIT;
    tb.firstPos = 0;

    /* Can we write to it? */
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNeditType, &et);  nargs++;
    XtGetValues(w, tmpargs, nargs);

    if (et == XawtextRead) {
	/* Make it writable. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, XawtextEdit);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }
    ins_point = XawTextGetInsertionPoint(w);
    XawTextReplace(w, ins_point, ins_point, &tb);
    if (et == XawtextRead) {
	/* Set it back. */
	nargs = 0;
	XtSetArg(tmpargs[nargs], XtNeditType, et);  nargs++;
	XtSetValues(w, tmpargs, nargs);
    }
}
#endif

#if 0
void
EraseTextWidget(Widget w)
{
    XawTextBlock block;

    block.ptr = "";
    block.length = 0;
    block.firstPos = 0;
    block.format = FMT8BIT;

    XawTextReplace(w, 0, widget_text_length(w), &block);
    /* If this fails, too bad. */
    move_caret_to_end(w);
}
#endif

int
find_side_via_widget(Widget w, Side **sidep)
{
    Side *side;

    for_all_sides(side) {
	if (active_display(side)) {
	    if (XtDisplay(w) == side->ui->dpy) {
		*sidep = side;
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* A minimal interface to Xconq.
   Copyright (C) 1991-1997, 1999-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This interface uses only what is required by ANSI, so it should run
   just about everywhere without any changes.  This is most useful for
   testing the kernel and game design libraries.

   This file can also serve as a starting point for writing a new
   interface, since it has simple or default implementations of the
   routines that any interface has to support. */

#include "conq.h"
#include "kpublic.h"
#include "print.h"
#include "cmdline.h"

extern Side* dside;

#ifdef MAC
int use_mac_charcodes = TRUE;
#endif

extern int autotest(void);

static void init_displays(void);
static void get_input(void);
static void interpret_command(Obj *cmd);
static void list_one_unit(Unit *unit);
static void interpret_help(Side *side, char *str);
static int do_cmd(Side *side, Obj *cmd, Obj *parms);
#ifdef DEBUGGING
static void toggle_debug(Side *side, Obj *cmd, Obj *parms);
static void toggle_debugg(Side *side, Obj *cmd, Obj *parms);
static void toggle_debugm(Side *side, Obj *cmd, Obj *parms);
#endif /* DEBUGGING */
static void list_sides(Side *side, Obj *cmd, Obj *parms);
static void list_units(Side *side, Obj *cmd, Obj *parms);
static void list_actors(Side *side, Obj *cmd, Obj *parms);
static void list_cells(Side *side, Obj *cmd, Obj *parms);
static void do_task_cmd(Side *side, Obj *cmd, Obj *parms);
static void do_finish_turn(Side *side, Obj *cmd, Obj *parms);
static void do_finish_all(Side *side, Obj *cmd, Obj *parms);
static void do_repeat(Side *side, Obj *cmd, Obj *parms);
static void do_multiple(Side *side, Obj *cmd, Obj *parms);
static void do_free_run(Side *side, Obj *cmd, Obj *parms);
static void do_save(Side *side, Obj *cmd, Obj *parms);
static void do_eval(Side *side, Obj *cmd, Obj *parms);
static void do_help(Side *side, Obj *cmd, Obj *parms);
static void do_print(Side *side, Obj *cmd, Obj *parms);
static void do_memory(Side *side, Obj *cmd, Obj *parms);
static void do_quit(Side *side, Obj *cmd, Obj *parms);
static int do_action(Side *side, Unit *unit, Obj *cmd, Obj *args);
static void show_help(Side *side, HelpNode *node);
static void describe_commands(int arg, char *key, TextBuffer *buf);

/* This structure maintains state that is local to a side's display.
   At the very least, it must track when the display is open and closed. */

typedef struct a_ui {
    int active;
} UI;

Side *defaultside = NULL;

HelpNode *curhelpnode;

int freerunturns = 0;

int repetition = 0;

Obj *multicmd = NULL;

time_t skelturnstart;

int numcellupdatesperturn = 0;

int numusefulcellupdatesperturn = 0;

/* The main program just calls the setup routines, then enters an infinite
   loop interpreting input and running the simulation. */

int
main(int argc, char *argv[])
{
    extern long initrandstate, randstate;
    long currandstate;
    int option_autotest = FALSE;

    printf("Skeleton Xconq version %s\n", version_string());
    printf("(C) %s\n", copyright_string());
    init_library_path(NULL);
    clear_game_modules();
#ifdef DEBUGGING
    init_debug_to_stdout();
#endif /* DEBUGGING */
    init_data_structures();
    /* Dump the random state so we can reproduce the run if necessary. */
    printf("Random state is %ld", randstate);
    if (initrandstate != randstate) {
	printf(" (seed was %ld)", initrandstate);
    }
    printf("\n");
    currandstate = randstate;
    if ((argc > 1) && strcmp ("--auto", argv[1]) == 0) {
	option_autotest = TRUE;
	++argv;
	--argc;
    }
    parse_command_line(argc, argv, general_options);
    if (currandstate != randstate) {
	printf("Random state is now %ld.\n", randstate);
    }
    load_all_modules();
    /* See if we have something resembling a valid game.  A synth method might
       still change some numbers, but it's up to the method to do it right. */
    check_game_validity();
    parse_command_line(argc, argv, variant_options);
    set_variants_from_options();
    parse_command_line(argc, argv, player_options);
    set_players_from_options();
    parse_command_line(argc, argv, leftover_options);
    make_trial_assignments();
    calculate_globals();
    run_synth_methods();
    final_init();
    assign_players_to_sides();
    init_signal_handlers();
    time(&skelturnstart);

    if (option_autotest) {
	exit(autotest());
    }

    run_game(0);
    init_displays();
    multicmd = lispnil;
    while (1) {
	if (freerunturns > 0) {
	    if (probability(10)) {
	    	printf("No apparent progress, forcing the turn to finish.\n");
	    	do_finish_all(NULL, lispnil, lispnil);
	    }
	    if (endofgame)
	      freerunturns = 0;
	} else {
	    get_input();
	}
	/* We stay in here until nothing more to do. */
	run_local_ai(1, 20);
	while (run_game(99) > 0) {
	    run_local_ai(2, 20);
	}
    }
}

void
run_ui_idler ()
{}

Player *
add_default_player(void)
{
    Player *dflt = add_player();

    dflt->displayname = "stdio";
    Dprintf("Added the default player %s\n", player_desig(dflt));
    return dflt;
}

void
make_default_player_spec(void)
{
    default_player_spec = "stdio";
}

/* This routine handles all the displays that might need to be opened. */
/* (Note that skelconq may have multiple "displays" open.) */

static void
init_displays(void)
{
    Side *side;

    for_all_sides(side) {
	if (side_has_display(side)) {
	    side->ui->active = TRUE;
	    printf("%s now has an open display.\n", side_desig(side));
	}
    }
}

/* Create a user interface, but leave it turned off. */

void
init_ui(Side *side)
{
    if (side_wants_display(side)) {
        side->ui = (UI *) xmalloc(sizeof(UI));
	/* Display should not become active yet. */
	side->ui->active = FALSE;
	defaultside = side;
	/* Do this so game doesn't run out of control. */
	net_set_autofinish(side, FALSE);
	DGprintf("Created a UI for %s\n", side_desig(side));
    } else {
	side->ui = NULL;
    }
}

/* This tests whether the side has a display and if it is in use. */

int
active_display(Side *side)
{
    return (side && side_has_display(side) && side->ui->active);
}

/* Input reading waits for a number of sides, possibly times out. */

static void
get_input(void)
{
    int cmdlineno = 1, endlineno = 1;
    Obj *cmd;
    Side *side = NULL;

    if (realtime_game()) {
	for_all_sides(side) {
	    update_clock_display(side, TRUE);
	}
    }
    /* (should say which sides we're waiting for) */
    printf("> ");
    fflush(stdout);
    if (repetition-- > 0) {
	cmd = multicmd;
    } else {
	cmd = read_form(stdin, &cmdlineno, &endlineno);
    }
    if (cmd != lispeof) {
	Dprintlisp(cmd);
	interpret_command(cmd);
    } else {
	printf("EOF reached\n");
	/* should just close one display, leave others running */
	exit(0);
    }
}

Unit *thisunit;

/* Do some simple command parsing, just enough to exercise the program. */

static void
interpret_command(Obj *origcmd)
{
    char *str;
    Obj *cmd, *verb = lispnil, *parms = lispnil;
    Side *side = NULL;
    Unit *unit = NULL;

    thisunit = NULL;
    cmd = origcmd;
    if (consp(cmd) && numberp(car(cmd))) {
	side = side_n(c_number(car(cmd)));
	cmd = cdr(cmd);
    }
    if (consp(cmd) && numberp(car(cmd))) {
	unit = find_unit(c_number(car(cmd)));
	thisunit = unit;
	cmd = cdr(cmd);
    }
    if (consp(cmd)) {
	verb = car(cmd);
	parms = cdr(cmd);
    } else if (symbolp(cmd)) {
	verb = cmd;
    }
    if (verb == lispnil) {
    } else if (do_cmd(side, verb, parms)) {
    } else if (do_action(side, unit, verb, parms)) {
    } else if (symbolp(verb) && *(str = c_string(verb)) == '?') {
	interpret_help(side, str+1);
    } else {
	printf("Command ");
	fprintlisp(stdout, origcmd);
	printf(" not understood, ignoring it\n");
    }
}

/* Random commands. */

static void
toggle_debug(Side *side, Obj *cmd, Obj *parms)
{
    if (parms == lispnil) {
	Debug = !Debug;
    } else if (symbolp(car(parms)) && equal(car(parms), intern_symbol("on"))) {
	Debug = TRUE;
    } else {
	Debug = FALSE;
    }
}

static void
toggle_debugm(Side *side, Obj *cmd, Obj *parms)
{
    if (parms == lispnil) {
	DebugM = !DebugM;
    } else if (symbolp(car(parms)) && equal(car(parms), intern_symbol("on"))) {
	DebugM = TRUE;
    } else {
	DebugM = FALSE;
    }
}

static void
toggle_debugg(Side *side, Obj *cmd, Obj *parms)
{
    if (parms == lispnil) {
	DebugG = !DebugG;
    } else if (symbolp(car(parms)) && equal(car(parms), intern_symbol("on"))) {
	DebugG = TRUE;
    } else {
	DebugG = FALSE;
    }
}

static void
list_sides(Side *side, Obj *cmd, Obj *parms)
{
    int u;
    Side *side2;
#if 0
    Agreement *ag;
    extern int num_agreements;
#endif

    printf("Sides: (%s)\n",
	   (g_use_side_priority() ? "sequential" : "simultaneous"));
    for_all_sides(side2) {
	printf("%s played by %s",
	       side_desig(side2), player_desig(side2->player));
	printf(", autofinish %d", side2->autofinish);
	if (g_use_side_priority())
	  printf(", priority %d", side2->priority);
	printf(", %s", (side2->finishedturn ? "finished" : "moving"));
	printf("\n");
	if (using_tech_levels()) {
	    printf("Tech:");
	    for_all_unit_types(u) {
		if (u_tech_max(u) > 0) {
		    printf("  %s %d/%d",
			   u_type_name(u), side2->tech[u], u_tech_max(u));
		}
	    }
	    printf("\n");
	}
	/* (should say something about mplayer goals here) */
    }
#if 0
    if (num_agreements > 0) {
	printf("Agreements:\n");
	for_all_agreements(ag) {
	    printf("%s\n", agreement_desig(ag));
	}
    }
#endif
}

static void
list_units(Side *side, Obj *cmd, Obj *parms)
{
    Unit *unit;

    printf("Units:\n");
    for_all_units(unit) {
	list_one_unit(unit);
    }
}

static void
list_actors(Side *side, Obj *cmd, Obj *parms)
{
    int i;
    Side *side2;
    Unit *unit;

    printf("Actors (listed by side):\n");
    for_all_sides(side2) {
	printf("%s: %s\n",
	       side_desig(side2),
	       (side2->finishedturn ? "(finished)" : ""));
    }
    for_all_sides(side2) {
	for (i = 0; i < side2->actionvector->numunits; ++i) {
	    unit = unit_in_vector(side2->actionvector, i);
	    list_one_unit(unit);
	}
    }
}

static void
list_one_unit(Unit *unit)
{
    char status[BUFSIZE];

    if (unit == NULL) {
	printf("  -\n");
	return;
    }
    if (!completed(unit)) {
	sprintf(status, " cp %d ", unit->cp);
    } else if (unit->hp < u_hp(unit->type)) {
	sprintf(status, " hp %d ", unit->hp);
    } else {
	sprintf(status, " ");
    }
    printf("  %s%s%s %s\n",
	   unit_desig(unit), status,
	   actorstate_desig(unit->act), plan_desig(unit->plan));
}

static void
list_cells(Side *side, Obj *cmd, Obj *parms)
{
    int x = c_number(car(parms)), y = c_number(cadr(parms));

    printf("At %d,%d", x, y);
    if (in_area(x, y)) {
	printf(", terrain %s", t_type_name(terrain_at(x, y)));
	/* (dump borders?) */
	printf(", elev %d", elev_at(x, y));
	printf(", temp %d", temperature_at(x, y));
	/* (etc) */
    } else {
	printf(" - outside area!");
    }
    printf("\n");
}

static void
do_task_cmd(Side *side, Obj *cmd, Obj *parms)
{
    int i, j, numargs;
    char *taskname;
    Obj *tasksym = car(parms), *taskparms = cdr(parms);
    Task *task;

    if (symbolp(tasksym)) {
	taskname = c_string(tasksym);
	/* Iterate through task names looking for a match. */
	for (i = 0; taskdefns[i].name != NULL; ++i) {
	    if (strcmp(taskname, taskdefns[i].name) == 0) {
		if (thisunit != NULL && thisunit->plan != NULL) {
		    task = create_task((TaskType) i);
		    numargs = strlen(taskdefns[i].argtypes);
		    for (j = 0; j < numargs; ++j) {
			if (taskparms != lispnil) {
			    task->args[j] = c_number(car(taskparms));
			    taskparms = cdr(taskparms);
			} else {
			    task->args[j] = 0;
			}
		    }
		    net_add_task(thisunit, 0, task);
		}
		return;
	    }
	}
	fprintf(stderr, "Task type \"%s\" not recognized\n", taskname);
    }
}

static void
do_finish_turn(Side *side, Obj *cmd, Obj *parms)
{
    net_finish_turn(side ? side : defaultside);
}

static void
do_finish_all(Side *side, Obj *cmd, Obj *parms)
{
    Side *side2;

    for_all_sides(side2) {
	net_finish_turn(side2);
    }
}

static void
do_free_run(Side *side, Obj *cmd, Obj *parms)
{
    Side *side2;

    if (consp(parms) && numberp(car(parms))) {
	freerunturns = c_number(car(parms));
    } else {
	freerunturns = 1;
    }
    for_all_sides(side2) {
	net_set_autofinish(side2, TRUE);
    }
}

static void
do_repeat(Side *side, Obj *cmd, Obj *parms)
{
}

static void
do_multiple(Side *side, Obj *cmd, Obj *parms)
{
    repetition = c_number(car(parms));
    multicmd = cdr(parms);
}

static void
do_save(Side *side, Obj *cmd, Obj *parms)
{
    if (!write_entire_game_state(saved_game_filename())) {
	fprintf(stderr, "Save failed.\n");
    }
}

static void
do_eval(Side *side, Obj *cmd, Obj *parms)
{
    interp_form(NULL, car(parms));
}

static void
do_help(Side *side, Obj *cmd, Obj *parms)
{
    printf("To look at help topics, type \"?<letter>\",\n");
    printf("where 'n' and 'p' go to next and previous nodes\n");
}

static void
do_print(Side *side, Obj *cmd, Obj *parms)
{
    char *fname;
    FILE *fp;

    if (parms != lispnil) {
	if (stringp(car(parms))) {
	    fname = c_string(car(parms));
	    if ((fp = fopen(fname, "w")) != NULL) {
		print_game_description_to_file(fp);
		fclose(fp);
	    } else {
		fprintf(stderr, "couldn't open \"%s\"\n", fname);
	    }
	} else {
	    /* error, not a string */
	}
    } else {
	print_game_description_to_file(stdout);
    }
}

static void
do_memory(Side *side, Obj *cmd, Obj *parms)
{
    extern int grandtotmalloc;

    printf("%d bytes allocated.\n", grandtotmalloc);
}

/* Exit immediately, no questions asked. */

static void
do_quit(Side *side, Obj *cmd, Obj *parms)
{
    /* A quit command of the form "(quit if-end)" only actually quits if the
       game is over. */
    if (symbolp(car(parms)) && equal(car(parms), intern_symbol("if-end"))) {
	if (!endofgame)
	  return;
    }
    printf("Quitting.\n");
    exit(0);
}

struct a_cmd {
    char *cmd;
    void (*fn)(Side *side, Obj *cmd, Obj *parms);
} cmdtable[] = {
    { "debug", toggle_debug },
    { "debugm", toggle_debugm },
    { "debugg", toggle_debugg },
    { "sides", list_sides },
    { "units", list_units },
    { "actors", list_actors },
    { "cells", list_cells },
    { "task", do_task_cmd },
    { "fin", do_finish_turn },
    { "finall", do_finish_all },
    { "again", do_repeat },
    { "*", do_multiple },
    { "run", do_free_run },
    { "save", do_save },
    { "eval", do_eval },
    { "help", do_help },
    { "print", do_print },
    { "memory", do_memory },
    { "quit", do_quit },
    { NULL, NULL }
};

/* Try to find and execute an arbitrary command. */

static int
do_cmd(Side *side, Obj *cmd, Obj *parms)
{
    struct a_cmd *cmdentry = cmdtable;
    char *cmdstr;

    if (!symbolp(cmd))
      return FALSE;
    cmdstr = c_string(cmd);
    while (cmdentry->cmd != NULL) {
	if (strcmp(cmdstr, cmdentry->cmd) == 0) {
	    (*(cmdentry->fn))(side, cmd, parms);
	    return TRUE;
	}
	++cmdentry;
    }
    return FALSE;
}

static int
do_action(Side *side, Unit *unit, Obj *cmd, Obj *args)
{
    int randomact = FALSE;
    ActionDefn *actdefn = actiondefns;
    char *cmdstr, *argstr;
    char localbuf[BUFSIZE];
    int i = 0, rslt;
    Obj *rest;
    Action action;

    if (!symbolp(cmd))
      return FALSE;
    cmdstr = c_string(cmd);
    if (side == NULL) {
	side = defaultside;
	if (side == NULL) {
	    fprintf(stderr, "Using first side since no defaults avail\n");
	    side = sidelist->next;
	}
    }
    if (unit == NULL) {
	/* (think of something to do here) */
    }
    while (actdefn->name != NULL) {
	if (strcmp(cmdstr, actdefn->name) == 0) {
	    memset(&action, 0, sizeof(Action));
	    action.type = actdefn->typecode;
	    /* Special option to generate random args to action. */
	    if (symbolp(car(args))
		&& strcmp("randomly", c_string(car(args))) == 0) {
		args = cdr(args);
		randomact = TRUE;
	    }
	    if (unit == NULL) {
		if (randomact) {
		    while (!((unit = find_unit(xrandom(numunits))) != NULL
			     && (unit->act != NULL || flip_coin()))
			   && probability(99));
		    if (unit == NULL) {
			fprintf(stderr, "Can't find a unit to act!\n");
			/* We're out of luck, just give up. */
			return TRUE;
		    }
		} else {
		    fprintf(stderr, "No unit to %s!\n", cmdstr);
		    /* *Command* *was* valid, just the args were bad. */
		    return TRUE;
		}
	    }
	    /* Move args from list into action. */
	    argstr = actdefn->argtypes;
	    for (rest = args; rest != lispnil; rest = cdr(rest)) {
		if (argstr[i] != '\0') {
		    action.args[i] = c_number(car(rest));
		} else {
		    break;
		}
		++i;
	    }
	    if (i != strlen(argstr)) {
		if (randomact) {
		    make_plausible_random_args(argstr, i, &(action.args[0]),
					       unit);
		} else {
		    printf("Mismatched args!\n");
		    return TRUE;
		}
	    }
	    sprintf(localbuf, "%s tries %s",
		    unit_desig(unit), action_desig(&action));
	    rslt = execute_action(unit, &action);
	    printf("%s - %s\n", localbuf, hevtdefns[rslt].name);
	    return TRUE;
	}
	++actdefn;
    }
    return FALSE;
}

/* Shut down displays - should be done before any sort of exit. */

void
close_displays(void)
{
    Side *side;

    for_all_sides(side) {
	if (active_display(side)) {
	    side->ui->active = FALSE;
	    printf("Display \"%s\" closed.\n", side->player->displayname);
	}
    }
}

void
cmd_error(Side *side, char *fmt, ...)
{
    char tmpnbuf[BUFSIZE];
    va_list ap;

    if (!empty_string(fmt)) {

	va_start(ap, fmt);
	vsnprintf(tmpnbuf, sizeof tmpnbuf, fmt, ap);
	va_end(ap);

	low_notify(dside, tmpnbuf);
    }
}

void
low_notify(Side *side, char *str)
{
    printf("To %s: %s\n", side_desig(side), str);
}

void
update_area_display(Side *side)
{
}

extern int suppress_update_cell_display;

/* (should count # of calls to inactive and non-displayed side...) */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    ++numcellupdatesperturn;
    if (active_display(side) && !suppress_update_cell_display) {
	++numusefulcellupdatesperturn;
	DGprintf("Update %s: view of %d,%d 0x%x\n",
		 side_desig(side), x, y, flags);
    }
}

/* This hook updates any display of the current turn/date, and any
   other global info, as needed. */

void
update_turn_display(Side *side, int rightnow)
{
    long secs;
    time_t xxx;

    if (active_display(side)) {
	time(&xxx);
	secs = idifftime(xxx, skelturnstart);
	printf("Update %s: turn %d (%ld seconds since last)",
	       side_desig(side), g_turn(), secs);
	DGprintf("%s", (rightnow ? " (now)" : ""));
	printf("\n");
	printf("%d cell updates, %d useful\n",
	       numcellupdatesperturn, numusefulcellupdatesperturn);
	numcellupdatesperturn = numusefulcellupdatesperturn = 0;
	skelturnstart = xxx;
	--freerunturns;
	if (freerunturns == 0) {
	    Side *side2;

	    for_all_sides(side2) {
	        if (active_display(side2))
		  net_set_autofinish(side2, FALSE);
	    }
	}
	if (rightnow)
	  fflush(stdout);
    }
}

void
update_action_display(Side *side, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: ready to act\n", side_desig(side));
    }
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s action result is %s\n",
	       side_desig(side), unit_desig(unit), hevtdefns[rslt].name);
    }
}

/* This is for animation of fire-at actions. */

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s fire at %s\n",
	       side_desig(side), unit_desig(unit), unit_desig(unit2));
    }
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m,
			 int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s fire into %d,%d\n",
	       side_desig(side), unit_desig(unit), x, y);
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
    if (active_display(side)) {
	switch (hevt->type) {
	  case H_SIDE_LOST:
	    printf("%s lost!\n", side_desig(side_n(hevt->data[0])));
	    break;
	  case H_SIDE_WON:
	    printf("%s won!\n",  side_desig(side_n(hevt->data[0])));
	    break;
	  default:
	    DGprintf("Update %s: event %s %d\n",
		     side_desig(side), hevtdefns[hevt->type].name,
		     hevt->data[0]);
	}
    }
}

void
update_all_progress_displays(char *str, int s)
{
    if (DebugG) {
	printf("Update all progress displays\n");
    }
}

/* This hook should update the side's view of the given side, no matter
   who it belongs to. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    char *side2desc = copy_string(side_desig(side2));

    if (active_display(side) && DebugG) {
	printf("Update %s: side %s%s\n",
	       side_desig(side), side2desc, (rightnow ? " (now)" : ""));
    }
}

void
update_research_display(Side *side)
{
    if (numatypes > 0
	&& g_side_can_research()
	&& dside->research_topic == NOADVANCE) {
	/* Should eventually pop up a side research dialog instead. */
	auto_pick_side_research(dside);
    }
}

/* This hook should update the side's view of the given unit, no matter
   who it belongs to. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: unit %s%s\n",
	       side_desig(side), unit_desig(unit), (rightnow ? " (now)" : ""));
    }
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: unit %s acp%s\n",
	       side_desig(side), unit_desig(unit), (rightnow ? " (now)" : ""));
    }
}

/* This hook updates any realtime clock displays.  If the game does not
   have any realtime constraints, this will never be called. */

void
update_clock_display(Side *side, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %d secs this turn, %d total\n",
	       side_desig(side), side->turntimeused, side->totaltimeused);
	/* also display total game clock */
    }
}

void
update_message_display(Side *side, Side *sender, char *str, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: side %d sends \"%s\"%s\n",
	       side_desig(side), side_number(sender), str, (rightnow ? " (now)" : ""));
    }
}

void
update_everything(void)
{
    printf("Update everything!\n");
}

void
action_point(Side *side, int x, int y)
{
}


/* Generate a description of all the user input that is possible. */

static void
describe_commands(int arg, char *key, TextBuffer *buf)
{
    struct a_cmd *cmdentry;

    for (cmdentry = cmdtable; cmdentry->cmd != NULL; ++cmdentry) {
	tbcat(buf, cmdentry->cmd);
	tbcat(buf, "\n");
    }
}

static void
interpret_help(Side *side, char *topic)
{
    HelpNode *node;

    if (side == NULL)
      side = defaultside;
    if (side == NULL) {
	fprintf(stderr, "no side to help?\n");
	return;
    }
    if (curhelpnode == NULL) {
	add_help_node("commands", describe_commands, 0, first_help_node);
	curhelpnode = first_help_node;
    }
    switch (topic[0]) {
      case 'n':
	curhelpnode = curhelpnode->next;
	break;
      case 'p':
	curhelpnode = curhelpnode->prev;
	break;
      case 'a':
	for (node = first_help_node->next; node != first_help_node; node = node->next) {
	    show_help(side, node);
	}
	return; /* don't show cur help node too */
      case '\0':
	/* Note that no topic char equals '\0', so will come here. */
      default:
	curhelpnode = first_help_node;
	break;
    }
    show_help(side, curhelpnode);
}

/* Spew out the entire text of the current help node. */

static void
show_help(Side *side, HelpNode *helpnode)
{
    int linelen, skipchar;
    char *linebegin = get_help_text(helpnode), *lineend;

    printf("Topic: %s\n", helpnode->key);
    while (*linebegin != '\0') {
	skipchar = 0;
	lineend = (char *) strchr(linebegin, '\n');
	if (lineend)
	  skipchar = 1;
	linelen = (lineend ? lineend - linebegin : strlen(linebegin));
	if (linelen > 75)
	  linelen = 75;
	strncpy(spbuf, linebegin, linelen);
	spbuf[linelen] = '\0';
	printf("%s\n", spbuf);
	linebegin += linelen + skipchar;
    }
}

/* This reports progress in reading GDL files. */

void
announce_read_progress(void)
{
}

/* This is used for initialization steps that take a long time. */

int linemiddle = FALSE;

void
announce_lengthy_process(char *msg)
{
    printf("%s; ", msg);
    fflush(stdout);
    linemiddle = TRUE;
}

/* Update the progress, expressing it as a percentage done. */

void
announce_progress(int percentdone)
{
    printf(" %d%%", percentdone);
    fflush(stdout);
    linemiddle = TRUE;
}

/* Announce the end of the lengthy process. */

void
finish_lengthy_process(void)
{
    printf(" done.\n");
    linemiddle = FALSE;
}

int
schedule_movie(Side *side, char *movie, ...)
{
    return FALSE;
}

void
play_movies(SideMask sidemask)
{
}

void
flush_display_buffers(Side *side)
{
    if (active_display(side) && DebugG) {
	printf("To %s: flush display buffers\n", side_desig(side));
    }
}

/* An init error needs to have the command re-run. */

void
low_init_error(char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

static void
report_warning(void)
{
    /* Ideally this would get passed back to the test framework which could
       record it as FAIL or ERROR, but for now just make sure it doesn't
       score as PASS.  */
    exit(EXIT_FAILURE);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);

    report_warning();
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    if (linemiddle)
      fprintf(stderr, "\n");
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
    report_warning();
}

void
print_form(Obj *form)
{
    print_form_and_value(stdout, form);
}

void
end_printing_forms(void)
{
}

/* Note that the fake declaration below will cause problems if imf.h
   is included. */

#if (0)
void make_generic_image_data(char *imf);
void
make_generic_image_data(char *imf)
{}
#else
#include "imf.h"
void make_generic_image_data(ImageFamily *imf);
void
make_generic_image_data(ImageFamily *imf)
{}
#endif


#ifdef MAC

int current_cursor;
int receivecursor;
int sendcursor;

#endif /* MAC */

void
unit_research_dialog(Unit *unit)
{
    auto_pick_unit_research(unit);
}

void
add_remote_locally(int rid, char *str)
{
}

void
send_chat(int rid, char *str)
{
    printf("From %d: received chat \"%s\"\n", rid, str);
}

/* Dummy function needed in run_turn_start. */

void
place_legends(Side *side)
{
}

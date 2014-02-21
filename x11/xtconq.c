/* The main program of the X11/Xt interface to Xconq.
   Copyright (C) 1987-1989, 1991-1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
extern void auto_pick_new_research(Unit *unit);
extern int auto_pick_new_build_task(Unit *unit);
extern void auto_pick_new_plan(Unit *unit);
#include "kpublic.h"
#include "cmdline.h"
#include "xtconq.h"
extern void init_emblem(Side *side, Side *side2);

/* Local function declarations. */

extern void init_all_displays(void);

static int handle_x_error(Display *dpy, XErrorEvent *evt);
static int handle_xio_error(Display *dpy);
#if 0
static void handle_xt_error(String msg);
#endif

void unit_research_dialog(Unit *unit);
void unit_plan_dialog(Unit *unit);
int launch_game(void);

/* Fallback resources come from an include file that is auto-generated
   from Xtconq.ad.  Note that this will only supply a basic b/w layout;
   to do color, the Xconq-co.ad resources have to be loaded already. */

String fallback_resources[] = {

#include "xtconqad.h"

  NULL
};

int announced = FALSE;

char *announcemsg = NULL;

/*
int nargs;
Arg tmpargs[100];
*/

/* The main program. */

int
main(int argc, char *argv[])
{
    extern uid_t games_uid;

    /* Dummy reference to get libraries pulled in */
    if (argc == -1)
      cmd_error(NULL, NULL);
    /* Shift to being the user that started the game. */
    games_uid = geteuid();
    setuid(getuid());
    init_library_path(NULL);
    printf("\n              Welcome to X11 Xconq version %s\n\n",
	   version_string());
    printf("%s", license_string());
    print_any_news();
    /* Fiddle with game module structures. */
    clear_game_modules();
#ifdef DEBUGGING
    init_debug_to_stdout();
#endif /* DEBUGGING */
    /* Set up empty data structures. */
    init_data_structures();

    initial_ui_init(argc, argv);

    parse_command_line(argc, argv, general_options);

    if (option_game_to_join != NULL) {
	int rslt;

	parse_command_line(argc, argv, variant_options);
	set_variants_from_options();
	parse_command_line(argc, argv, player_options);
	set_players_from_options();
	/* Go through once more and complain about anything not used. */
	parse_command_line(argc, argv, leftover_options);
	hosting = FALSE;
	rslt = open_remote_connection(option_game_to_join, hosting);
	if (rslt > 0) {
	    sendnow = TRUE;
	    rslt = send_join(default_player_spec);
	    sendnow = FALSE;
	    if (rslt) {
		master_rid = 1;
		while (TRUE) {
		    receive_data(0, MAXPACKETS);
		    if (!beforestart) {
			break;
		    }
		}
	    } else {
		close_remote_connection(0);
		fprintf(stderr, "Unable to join game \"%s\", exiting\n",
			option_game_to_join);
		exit(1);
	    }
	}
    } else {
	if (option_popup_new_game_dialog) {
	    popup_game_dialog();
	    /* Options have been supplied, fall through to do normal
	       game setup. */
	}
	if (option_popup_new_game_dialog) {
	    check_player_displays();
	} else {
	    load_all_modules();
	    check_game_validity();
	    parse_command_line(argc, argv, variant_options);
	    set_variants_from_options();
	    parse_command_line(argc, argv, player_options);
	    set_players_from_options();
	    make_trial_assignments();
	    check_player_displays();
	    /* Complain about anything that's left. */
	    parse_command_line(argc, argv, leftover_options);
	    /* (still need to merge some databases derived from display) */
	}
    }
    if (option_game_to_host != NULL) {
	hosting = TRUE;
	(assignments[0].player)->rid = 1;
	(assignments[1].player)->remotewanted = "x";
	open_remote_connection(option_game_to_host, hosting);
	numremotewaiting = 1;
	master_rid = 1;
	while (numremotewaiting > 0) {
	    int i;
	    Player *player;

	    receive_data(0, MAXPACKETS);
	    numremotewaiting = 0;
	    for (i = 0; i < numsides; ++i) {
		if (assignments[i].side && (assignments[i].side)->ingame) {
		    player = assignments[i].player;
		    if (player->remotewanted != NULL && player->rid == 0) {
			player->displayname = copy_string(":0.0");
			player->rid = 2; /* for now */
			sendnow = TRUE;
			send_version(player->rid);
			sendnow = FALSE;
			sleep(1);
			download_to_player(player);
		    }
		}
	    }
	}
    }
    if (option_game_to_join == NULL) {
	launch_game();
    }
    ui_init();
    /* Get the displays set up, but don't draw anything yet. */
    init_all_displays();
    /* Now bring up the init data on each display. */
    init_redraws();
    /* Set up the signal handlers. */
    init_signal_handlers();
    init_x_signal_handlers();
    notify_all("Command was \"%s %s\"", argv[0], args_used);
    notify_instructions();
    /* Go into the main play loop. */
    ui_mainloop();

    /* Humor the compiler. */
    return 0;
}

/* The default (human) player is the current user on the current display. */

Player *
add_default_player()
{
    Player *player = add_player();
    
    player->name = getenv("USER");
    player->configname = getenv("XCONQCONFIG");
    player->displayname = getenv("DISPLAY");
    return player;
}

void
make_default_player_spec(void)
{
    if (default_player_spec == NULL)
      default_player_spec = (char *)xmalloc(BUFSIZE);
    default_player_spec[0] = '\0';
    if (!empty_string(getenv("USER"))) {
	strncpy(default_player_spec, getenv("USER"), 
						   BUFSIZE - 2);
	strcat(default_player_spec, "@");
    }
    if (!empty_string(raw_default_player_spec)
	&& raw_default_player_spec[0] == '@') {
	strncat(default_player_spec, raw_default_player_spec, 
						   BUFSIZE - strlen(default_player_spec) - 1);
    } else if (!empty_string(getenv("DISPLAY"))) {
	strncat(default_player_spec, getenv("DISPLAY"), 
						   BUFSIZE - strlen(default_player_spec) - 1);
    }
}

/* An init error needs to have the command re-run. */

void
low_init_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stderr, "Warning: %s.\n", str);
    fflush(stderr);
}

void
low_run_error(char *str)
{
    close_displays();
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    fprintf(stderr, "Saving the game...");
    write_entire_game_state(saved_game_filename());
    fprintf(stderr, " done.\n");
    exit(1);
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    notify_all("Warning: %s; continuing...", str);
}

void
print_form(Obj *form)
{
    print_form_and_value(stdout, form);
}

void
end_printing_forms()
{
}

void
init_x_signal_handlers()
{
    XSetErrorHandler(handle_x_error);
    XSetIOErrorHandler(handle_xio_error);
#if 0 /* (why?) */
    XtAppSetErrorHandler(thisapp, handle_xt_error);
#endif
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

#if 0
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

/* Reading is usually pretty fast, so don't do anything special here. */

void
announce_read_progress()
{
}

/* Announce the start of a time-consuming computation. */

void
announce_lengthy_process(char *msg)
{
    n_seconds_elapsed(0);
    announcemsg = copy_string(msg);
    if (announcemsg) {
	printf("%s;", announcemsg);
	free(announcemsg);
	announcemsg = NULL;
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the making of progress on the computation. */

void
announce_progress(int percentdone)
{
    if (n_seconds_elapsed(2)) {
	printf(" %d%%,", percentdone);
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the end of the time-consuming computation. */

void
finish_lengthy_process()
{
    if (announced) {
	printf(" done.\n");
	announced = FALSE;
    }
}

/* This should be called before any sort of normal exit. */

void
exit_xconq(Side *side)
{
    close_displays();
    printf("\n%s\n", get_scores(side));
    exit(0);
}

int
launch_game()
{
    Player *player;

    /* Give all unassigned players to the master program. This code, taken
       from macinit.c, finally solved the ai problem in unix network games. */
    if (numremotes > 0) {
	for_all_players(player) {
	    if (player->rid == 0)
		player->rid = master_rid;
	}
    }

    /* Do the time-consuming part of setup calculations. */
    calculate_globals();
    run_synth_methods();
    final_init();
    assign_players_to_sides();
    print_instructions();
    /* Do first set of turn calcs, but let the host lead the way. */
    if (my_rid == 0 || (my_rid > 0 && my_rid == master_rid)) {
	net_run_game(0);
    }
    return TRUE;
}

/* Set up all sides' displays all at once. */

void
init_all_displays()
{
    int numdisplays;
    Side *side;

    numdisplays = 0;
    for_all_sides(side) {
	if (side_has_display(side)) {
	    if (side_has_local_display(side))
	      init_display(side);
	    ++numdisplays;
	}
    }
    if (numdisplays == 0) {
	fprintf(stderr, "Must have at least one display to start.\n");
	exit(0);
    }
}

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
}

/* The main program of the X11 interface to Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"

#include "tcltk/tkconq.h"

extern int use_stdio;

int blinking_curunit = FALSE;

#include <unistd.h>

/* Local function declarations. */

static void accept_all_remotes(void);
static int handle_x_error(Display *dpy, XErrorEvent *evt);
static int handle_xio_error(Display *dpy);

/* The main program. */

int
main(int argc, char *argv[])
{
	extern uid_t games_uid;

	use_stdio = TRUE;
	/* Dummy reference to get libraries pulled in */
	if (argc == -1)
	    cmd_error(NULL, NULL);
	/* Shift to being the user that started the game. */
	games_uid = geteuid();
	setuid(getuid());
	init_library_path(NULL);
	/* Fiddle with game module structures. */
	clear_game_modules();
#ifdef DEBUGGING
	init_debug_to_stdout();
#endif /* DEBUGGING */
	/* Set up empty data structures. */
	init_data_structures();

	parse_command_line(argc, argv, general_options);
	parse_command_line(argc, argv, variant_options);
	parse_command_line(argc, argv, player_options);
	/* Complain about anything that's left. */
	parse_command_line(argc, argv, leftover_options);

	if (argc > 1) {
		/* We're probably typing in a command line, use stdout. */
		printf("\n              Welcome to X11 Xconq version %s\n\n", version_string());
		printf("%s", license_string());
		print_any_news();
	} else {
		/* Total GUI-ness this way. */
		option_popup_new_game_dialog = TRUE;
	}

	initial_ui_init();

	if (option_popup_new_game_dialog) {
		/* Do all game setup via the GUI. */
		popup_game_dialog();
	} else if (option_game_to_join != NULL) {
		/* Joining a game, using the command line. */
		/* (should detect attempts to ask for options that will be ignored) */
		/* (should be able to ask for position among sides) */
		int rslt = try_join_game(option_game_to_join);

		if (rslt == FALSE) {
			fprintf(stderr, "No response from host at \"%s\"\n", option_game_to_join);
		    	exit(1);
		} else if (rslt == DONE) {
			fprintf(stderr, "Cannot join ongoing game at \"%s\"\n", option_game_to_join);
		    	exit(1);
		}
		printf("Connected to game host\n");
		/* Keep talking to the host until the game is all set up. */
		while (current_stage != game_ready_stage) {
		    	receive_data(0, MAXPACKETS);
		}
		launch_game();
	} else {
		/* Hosting a game from the command line, or else playing solo. */
		if (option_game_to_host != NULL) {
			/* Collect the programs that will participate in game
			setup (more may come later, after the game has
			started, if the game module allows it). */
		    	host_the_game(option_game_to_host);
		    	accept_all_remotes();
		}
		start_game_load_stage();
		load_all_modules();
		check_game_validity();
		if (my_rid > 0 && my_rid == master_rid) {
		    	broadcast_game_module();
		}
		interpret_variants();
		set_variants_from_options();
		start_player_pre_setup_stage();
		set_players_from_options();
		start_player_setup_stage();
		launch_game();
	}
	if (!option_popup_new_game_dialog)
	    print_instructions();
	/* At this point we know we can use popups instead of stdio for
	warnings and messages and such. */
	use_stdio = FALSE;
	/* Go into the main play loop. */
	ui_mainloop();

	/* Humor the compiler. */
	return 0;
}

/* The default (human) player is the current user on the current display. */

Player *
add_default_player(void)
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

/* Wait for all the players to join, set up each one as it comes in. */

static void
accept_all_remotes(void)
{
    int lastcount;
    extern int option_num_to_wait_for;

    /* (should error?) */
    if (option_num_to_wait_for <= 0)
      return;
    numremotewaiting = option_num_to_wait_for;
    printf("Now waiting for %d to join\n", numremotewaiting);
    while (numremotes < (option_num_to_wait_for + 1)) {
	lastcount = numremotes;
	receive_data(0, MAXPACKETS);
	if (numremotes != lastcount) {
	    numremotewaiting = (option_num_to_wait_for - (numremotes - 1));
	    printf("Now waiting for %d to join\n", numremotewaiting);
	}
    }
}

void
init_x_signal_handlers(void)
{
    XSetErrorHandler(handle_x_error);
    XSetIOErrorHandler(handle_xio_error);
}

/* Handlers for X catastrophes attempt to do a save first. */

static int
handle_x_error (Display *dpy, XErrorEvent *evt)
{
    static int num_errors = 0;
    char buf[BUFSIZE];

    XGetErrorText(dpy, evt->error_code, buf, BUFSIZE);
    fprintf(stderr, "\nX error on display %s: %s\n", DisplayString(dpy), buf);

#if 0 	 /* This code caused the program to crash every time a Tcl dialog
	    or window had been dismissed more than 5 times. The reason is 
	    that two harmless x errors, BadWindow and BadDrawable,are 
	    generated every time Tcl uses the "wm withdraw" command. */

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

#endif	/* End of auto-crashing code. */

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

/* Main program for running Xconq under SDL on Win32 computers.
   Copyright (C) 2002 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "sdlconq.h"

int using_sdl;

/* Local function declarations. */

static void accept_all_remotes(void);

/* The main program. */

int
main(int argc, char *argv[])
{
	/* Put the game in a defined state. */
	clear_game_modules();
	init_data_structures();
	init_library_path(NULL);

	/* Parse the command line if used. */
	parse_command_line(argc, argv, general_options);
	parse_command_line(argc, argv, variant_options);
	parse_command_line(argc, argv, player_options);
	parse_command_line(argc, argv, leftover_options);

	/* Will be set to TRUE if calling initial_ui_init in sdlmain.c. */
	using_sdl = FALSE;

	initial_ui_init();

	/* The new game dialog is not supported in sdlconq yet. */
	if (using_sdl) {
		option_popup_new_game_dialog = FALSE;
	}
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
		/* initial_ui_init(); */
		launch_game();
	}

	if (!option_popup_new_game_dialog)
	    print_instructions();

	/* At this point we know we can use popups instead of stdio for
	warnings and messages and such. */
	/* use_stdio = FALSE; */

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
    player->displayname = "WinSDL";
    return player;
}

void
make_default_player_spec(void)
{
  default_player_spec = "WinSDL";
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


/* Command-line parsing definitions for Xconq.
   Copyright (C) 1993, 1994, 1995, 1999, 2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#define CLIBUFSIZE  1024

/*! \file kernel/cmdline.h
 * \brief Command Line Options.
 *
 * This include file includes information for command line processing.
 */

enum parsestage {
    general_options,
    variant_options,
    player_options,
    interface_options,
    leftover_options
};

/* Global variables that command-line options tweak. */

extern int checkpointinterval;
extern int option_popup_new_game_dialog;
extern char *option_game_to_host;
extern char *option_game_to_join;
extern int option_num_to_wait_for;
extern char *default_player_spec;
extern char *raw_default_player_spec;
extern char *args_used;

extern void init_options(void);
extern void parse_command_line(int argc, char **argv, int spec);
extern void add_a_module(char *name, char *filename);
extern void load_all_modules(void);
extern void set_variants_from_options(void);
extern void set_players_from_options(void);
extern void print_instructions(void);

/* Command line parsing for Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is a command-line parser that may be used in implementations
   that get a command line from somewhere. */

/* Command lines get parsed in several stages, since for instance the
   choice of game module will decide which variants are available. */

#include "conq.h"
extern short initially_no_ai;
#include "kpublic.h"
#include "cmdline.h"

static void add_a_raw_player_spec(char *specstr);
static void version_info(void);
static void general_usage_info(void);
static void game_usage_info(void);
static void player_usage_info(void);
static void parse_world_option(char *str);
static void parse_realtime_option(char *subopt, char *arg);
static void parse_variant(char *str);
static int find_variant_from_name(Module *module, char *name);

/* The startup-settable options. */

static int option_width;
static int option_height;
static int option_circumference;
static int option_total_game_time;
static int option_per_side_time;
static int option_per_turn_time;
static int option_add_default_player;

char *option_game_to_host = NULL;

char *option_game_to_join = NULL;

int option_num_to_wait_for = 0;

static char *default_ai_type = ",mplayer";

/* Use this to record the options used to set up a game, so it can be
   reported to users. */
/* (should flush? better to display in intelligible form) */

char *args_used;

/* The list of asked-for players. */

struct raw_spec {
  char *spec;
  struct raw_spec *next;
} *raw_player_specs, *last_raw_player_spec;

char *raw_default_player_spec;

/* The list of accumulated variant choices. */

static Obj *variant_settings;

static char *program_name = "";

static int help_wanted = FALSE;

static int variant_help_wanted = FALSE;

static int version_wanted = FALSE;

static int had_error = FALSE;

/* Set the most basic of defaults on the dynamically-settable
   options. */

/* (need flags to indicate which options were actually used, so
   variant handling can warn about improper use) */

void
init_options(void)
{
    option_add_default_player = TRUE;
    variant_settings = lispnil;
}

/* Generic command line parsing.  This is used by several different
   programs, so it just collects info, doesn't process it much.  This
   is called several times, because the validity of some args depends
   on which game modules are loaded and which players are to be in the
   game, and interfaces may need to do some of their own processing in
   between. */

void
parse_command_line(int argc, char *argv[], int spec)
{
    char *arg, *aispec, tmpspec[100], tmpargbuf[CLIBUFSIZE], blurb[BLURBSIZE];
    int i, n, numused, total_arg_space, tmpargbuflen = 0;

/* This macro just checks that a required next argument is actually
   there. */

#define REQUIRE_ONE_ARG  \
  if (i + 1 >= argc) {  \
    fprintf(stderr, "Error: `%s' requires an argument, exiting now\n", argv[i]);  \
    had_error = TRUE;  \
    continue;  \
  }  \
  numused = 2;

/* Each of these causes argument parsing to skip over the option if
   it's not the right time to look at it. */

#define GENERAL_OPTION if (spec != general_options) continue;
#define VARIANT_OPTION if (spec != variant_options) continue;
#define PLAYER_OPTION  if (spec != player_options)  continue;

    /* (should peel off any path stuff) */
    program_name = argv[0];

    if (spec == general_options)
      init_options();

    total_arg_space = 0;
    for (i = 0; i < argc; ++i) {
      if (!empty_string(argv[i])) {
        strncpy(tmpargbuf, argv[i], CLIBUFSIZE);
        tmpargbuf[CLIBUFSIZE - 1] = 0;
        tmpargbuflen = strlen(tmpargbuf);
	total_arg_space += tmpargbuflen + 2;
        (argv[i])[tmpargbuflen] = 0; 
      }
    }
    if (args_used == NULL)
      args_used = (char *)xmalloc (total_arg_space);

    for (i = 1; i < argc; ++i) {
	if (argv[i] == NULL || (argv[i])[0] == '\0') {
	    /* Empty or already munched, nothing to do. */
	} else if ((argv[i])[0] == '-') {
	    arg = argv[i];
	    Dprintf("%s\n", arg);
	    numused = 1;
	    if (strcmp(arg, "-c") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		checkpoint_interval = atoi(argv[i+1]);
	    } else if (strcmp(arg, "-design") == 0) {
		GENERAL_OPTION;
#ifdef DESIGNERS
		allbedesigners = TRUE;
#else
		fprintf(stderr,
			"No designing available, ignoring option `%s'\n", arg);
#endif /* DESIGNERS */
	    } else if (strncmp(arg, "-D", 2) == 0) {
		GENERAL_OPTION;
#ifdef DEBUGGING
		Debug = TRUE;
		if (strchr(arg+2, '-'))
		  Debug = FALSE;
		if (strchr(arg+2, 'M'))
		  DebugM = TRUE;
		if (strchr(arg+2, 'G'))
		  DebugG = TRUE;
#else
		fprintf(stderr,
			"No debugging available, ignoring option `%s'\n", arg);
#endif /* DEBUGGING */
	    } else if (strncmp(arg, "-e", 2) == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		n = atoi(argv[i+1]);
		/* A comma indicates that the name of a particular
		   desired AI type follows. */
		if (strlen(arg) > 2) {
		    aispec = arg + 2;
		    if (*aispec != ',') {
			sprintf(tmpspec, "%s%s", default_ai_type, aispec);
			aispec = tmpspec;
		    }
		} else {
		    aispec = default_ai_type;
		}
		while (n-- > 0)
		  add_a_raw_player_spec(aispec);
	    } else if (strcmp(arg, "-f") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		add_a_module(NULL, argv[i+1]); 
	    } else if (strcmp(arg, "-g") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		add_a_module(copy_string(argv[i+1]), NULL);
	    } else if (strcmp(arg, "-h") == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		n = atoi(argv[i+1]);
		option_num_to_wait_for += n;
		while (n-- > 0)
		  add_a_raw_player_spec("?@");
	    } else if (strcmp(arg, "-help") == 0) {
		GENERAL_OPTION;
		help_wanted = TRUE;
		/* Will display help info later. */
	    } else if (strcmp(arg, "-host") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		option_game_to_host = copy_string(argv[i+1]);
	    } else if (strcmp(arg, "-join") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		option_game_to_join = copy_string(argv[i+1]);
	    } else if (strcmp(arg, "-L") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		if (strcmp(argv[i+1], "-") == 0)
		  add_library_path(NULL);
		else
		  add_library_path(argv[i+1]);
	    } else if (strcmp(arg, "-M") == 0) {
		REQUIRE_ONE_ARG;
		VARIANT_OPTION;
		parse_world_option(argv[i+1]);
	    } else if (strcmp(arg, "-noai") == 0) {
		PLAYER_OPTION;
		initially_no_ai = TRUE;
	    } else if (strcmp(arg, "-r") == 0) {
		PLAYER_OPTION;
		option_add_default_player = FALSE;
	    } else if (strcmp(arg, "-R") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
#ifdef DEBUGGING
		init_xrandom(atoi(argv[i+1]));
#else
		fprintf(stderr,
			"No debugging available, ignoring option `%s'\n", arg);
#endif /* DEBUGGING */
	    } else if (strcmp(arg, "-seq") == 0) {
		VARIANT_OPTION;
		push_key_int_binding(&variant_settings, K_SEQUENTIAL, 1);
	    } else if (strcmp(arg, "-sim") == 0) {
		VARIANT_OPTION;
		push_key_int_binding(&variant_settings, K_SEQUENTIAL, 0);
	    } else if (strncmp(arg, "-t", 2) == 0) {
		REQUIRE_ONE_ARG;
		VARIANT_OPTION;
		parse_realtime_option(arg, argv[i+1]);
	    } else if (strncmp(arg, "-v", 2) == 0) {
		VARIANT_OPTION;
		parse_variant(arg + 2);
	    } else if (strcmp(arg, "-V") == 0) {
		VARIANT_OPTION;
		push_key_int_binding(&variant_settings, K_SEE_ALL, 1);
	    } else if (strcmp(arg, "-V0") == 0) {
		VARIANT_OPTION;
		push_key_int_binding(&variant_settings, K_SEE_ALL, 0);
	    } else if (strcmp(arg, "-Vfalse") == 0) {
		VARIANT_OPTION;
		push_key_int_binding(&variant_settings, K_SEE_ALL, 0);
	    } else if (strcmp(arg, "-w") == 0) {
		GENERAL_OPTION;
		warnings_suppressed = TRUE;
	    } else if (strcmp(arg, "-x") == 0) {
		GENERAL_OPTION;
		option_popup_new_game_dialog = TRUE;
	    } else if (strcmp(arg, "--help") == 0) {
		GENERAL_OPTION;
		help_wanted = TRUE;
		/* Will display help info later. */
	    } else if (strcmp(arg, "--version") == 0) {
		GENERAL_OPTION;
		version_wanted = TRUE;
	    } else {
		numused = 0;
		/* Anything unrecognized during the last parse is an error. */
		if (spec >= leftover_options) {
		    fprintf(stderr, "Unrecognized option `%s'\n", arg);
		    had_error = TRUE;
		}
	    }
	    if (numused >= 1) {
		strcat(args_used, " ");
		strcat(args_used, argv[i]);
		argv[i] = "";
	    }
	    if (numused >= 2) {
		strcat(args_used, " ");
		strcat(args_used, argv[i+1]);
		argv[i+1] = "";
	    }
	    if (numused >= 1)
	      i += (numused - 1);
	} else {
	    if (spec == player_options) {
		if (*(argv[i]) == '+' || *(argv[i]) == '@') {
		    raw_default_player_spec = argv[i];
		} else {
		    add_a_raw_player_spec(argv[i]);
		}
		strcat(args_used, " ");
		strcat(args_used, argv[i]);
		argv[i] = NULL;
	    }
	}
    }
    if (version_wanted) {
	version_info();
    }
    if (had_error || help_wanted || variant_help_wanted) {
	/* If we want to get help about a particular game, have to
           load it first. */
	if (help_wanted || variant_help_wanted)
	  load_all_modules();
	if (had_error || help_wanted)
	  general_usage_info();
	if (had_error || help_wanted)
	  player_usage_info();
	if (help_wanted && mainmodule != NULL) {
	    printf("\nGame info:\n\n");
	    if (!empty_string(mainmodule->title))
	      printf("%s (%s)\n", mainmodule->title, mainmodule->name);
	    else
	      printf("%s\n", mainmodule->name);
	    printf("    %s\n");
	    if (mainmodule->blurb != lispnil) {
	    	append_blurb_strings(blurb, mainmodule->blurb);
	    	printf(blurb);
	    } else {
	    	printf("(no description)");
	    }
	}
	/* Display variant info here so it comes after basic info
	   about the game module. */
	if (had_error || help_wanted || variant_help_wanted)
	  game_usage_info();
    }
    if (had_error || help_wanted || variant_help_wanted || version_wanted) {
	exit(had_error);
    }
}

/* Given a module name and/or filename, add it to the list of modules
   to load. */

void
add_a_module(char *name, char *filename)
{
    Module *module;

    module = get_game_module(name);
    if (module == NULL)
      exit(1);  /* bad error if happens */
    if (filename)
      module->filename = copy_string(filename);
    if (mainmodule != NULL)
      printf("Warning: game module already specified, will replace");
    mainmodule = module;
}

/* Glue an unprocessed player spec onto the list of other specs found
   on the command line. */

static void
add_a_raw_player_spec(char *specstr)
{
    struct raw_spec *spec =
      (struct raw_spec *) xmalloc (sizeof(struct raw_spec));

    spec->spec = copy_string(specstr);
    if (raw_player_specs == NULL)
      raw_player_specs = spec;
    else
      last_raw_player_spec->next = spec;
    last_raw_player_spec = spec;
}

static void
version_info(void)
{
    printf("Xconq (%s) version %s\n", program_name, version_string());
}

/* This routine prints out help info about all the possible arguments. */

static void
general_usage_info(void)
{
    printf("Usage:\n\t%s [ -options ... ]\n\n", program_name);
    printf("General options:\n\n");
    printf("    -c n\t\tcheckpoint every <n> turns\n");
    printf("    -f filename\t\trun <filename> as a game\n");
    printf("    -g gamename\t\tfind <gamename> in library and run it\n");
    printf("    -help\t\tdisplay this help info\n");
    printf("    -host <game@host>\thost the given game\n");
    printf("    -join <game@host>\tconnect to the given game\n");
    printf("    -L pathname\t\tset <pathname> to be library location\n");
    printf("    -w\t\t\tsuppress warnings\n");
    printf("    -x\t\t\tuse game setup dialogs to choose game and options\n");
    printf("    -design\t\tmake every player a designer");
#ifndef DESIGNERS
    printf(" (not available)");
#endif
    printf("\n");
    /* We don't display the debugging options -D* and -R, on the
       theory that only hackers should try to use them. */
    /* Group the long options together, but don't identify them
       specially. */
    printf("    --help\t\tdisplay this help info\n");
    printf("    --version\t\tdisplay version info\n");
}

/* Describe the available variants for a game. */

static void
game_usage_info(void)
{
    int i, wid, hgt, circumf, lat, lon, pergame, perside, perturn;
    char *varid;
    char buf[BUFSIZE];
    Variant *var;
    Obj *vardflt;

    printf("\nGame variant options");
    if (mainmodule == NULL) {
	printf(":\n\n    No game loaded, no information available.\n\n");
	return;
    }
    printf(" for \"%s\":\n\n", mainmodule->name);
    if (mainmodule->variants == NULL) {
	printf("    No variants available.\n\n");
	return;
    }
    for (i = 0; mainmodule->variants[i].id != lispnil; ++i) {
	var = &(mainmodule->variants[i]);
	varid = c_string(var->id);
	vardflt = var->dflt;
	switch (keyword_code(varid)) {
	  case K_SEE_ALL:
	    printf("    -V\t\t\t%s (default %s)\n",
		   var->help,
		   (vardflt == lispnil ? "true" :
		    (c_number(eval(vardflt)) ? "true" : "false")));
	    break;
	  case K_SEQUENTIAL:
	    printf("    -seq\t\t%s (default %s)\n",
		   var->help,
		   (vardflt == lispnil ? "false" :
		    (c_number(eval(vardflt)) ? "false" : "true")));
	    printf("    -sim\t\tSides move simultaneously (opposite of -seq)\n");
	    break;
	  case K_WORLD_SEEN:
	    printf("    -v\t\t\t%s (default %s)\n",
		   var->help,
		   (vardflt == lispnil ? "true" :
		    (c_number(eval(vardflt)) ? "true" : "false")));
	    break;
	  case K_WORLD_SIZE:
	    printf("    -M wid[xhgt][Wcircumf][+lat][+long]\tset world size (default ");
	    /* Note that if the game definition sets these values
	       directly using world or area forms, this is misleading;
	       but that's the fault of the game designer for including
	       both preset values and a variant whose defaults don't
	       match those presets. */
	    circumf = DEFAULTCIRCUMFERENCE;
	    wid = DEFAULTWIDTH;  hgt = DEFAULTHEIGHT;
	    lat = lon = 0;
	    /* Pick the width and height out of the list. */
	    if (vardflt != lispnil) {
		wid = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    }
	    if (vardflt != lispnil) {
		hgt = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    } else {
		hgt = wid;
	    }
	    /* Pick up a circumference etc if given. */
	    if (vardflt != lispnil) {
		circumf = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    }
	    if (vardflt != lispnil) {
		lat = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    }
	    if (vardflt != lispnil) {
		lon = c_number(eval(car(vardflt)));
	    }
	    printf("%dx%dW%d", wid, hgt, circumf);
	    if (lat != 0 || lon != 0)
	      printf("+%d+%d", lat, lon);
	    printf(")\n");
	    break;
	  case K_REAL_TIME:
	    pergame = perside = perturn = 0;
	    if (vardflt != lispnil) {
		pergame = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    }
	    if (vardflt != lispnil) {
		perside = c_number(eval(car(vardflt)));
		vardflt = cdr(vardflt);
	    }
	    if (vardflt != lispnil) {
		perturn = c_number(eval(car(vardflt)));
	    }
	    printf("    -tgame mins\t\tlimit game time to <mins> minutes (default %d)\n",
		   pergame);
	    printf("    -tside mins\t\tlimit each player <mins> minutes in all (default %d)\n",
		   perside);
	    printf("    -tturn mins\t\tlimit each turn to <mins> minutes (default %d)\n",
		   perturn);
	    break;
	  default:
	    printf("    -v%s[=value]\t%s (default ", varid, var->help);
	    if (vardflt == lispnil
	        || (numberp(vardflt) && c_number(vardflt) == 0)) {
		printf("false");
	    } else if (numberp(vardflt) && c_number(vardflt) == 1) {
		printf("true");
	    } else {
		sprintlisp(buf, vardflt, BUFSIZE);
		printf("%s", buf);
	    }
	    printf(")\n");
	    break;
	}
    }
}

/* Dump info about player setup options.  These can only get used when
   the game has been loaded and variants chosen. */

static void
player_usage_info(void)
{
    printf("\nPlayer setup options:\n\n");
    printf("    [[name][,ai][/config]@]host[+advantage]\tadd player\n");
    printf("        ai\t\t= name of AI type or \"ai\" for default type\n");
    printf("        config\t\t= name of config file\n");
    printf("        host\t\t= name of player's host machine or display\n");
    printf("        advantage\t= numerical initial advantage (default 1)\n");
    printf("    -e number[,ai]\tadd <number> computer players\n");
    printf("    -h number[,ai]\tadd <number> human players\n");
    printf("    -noai\t\tsuppress all AIs\n");
    printf("    -r\t\t\tno default player on local display\n");
    /* (should available AI types here) */
}

/* Given a string representing world dimensions, extract various
   components. */

static void
parse_world_option(char *str)
{
    char *str2;

    option_width = atoi(str);
    option_height = option_width;
    str2 = strchr(str, 'x');
    if (str2 != NULL)
      option_height = atoi(str2 + 1);
    /* Add a world circumference setting if present. */
    str2 = strchr(str, 'W');
    if (str2 != NULL)
      option_circumference = atoi(str2 + 1);
}

static void
parse_realtime_option(char *subopt, char *arg)
{
    if (strcmp(subopt, "-tgame") == 0) {
	option_total_game_time = 60 * atoi(arg);
    } else if (strcmp(subopt, "-tside") == 0) {
	option_per_side_time = 60 * atoi(arg);
    } else if (strcmp(subopt, "-tturn") == 0) {
	option_per_turn_time = 60 * atoi(arg);
    } else {
	fprintf(stderr, "Realtime option not one of -tgame, -tside, -tturn, ignoring\n");
    }
}

/* Given a variant, turn it into a list "(name val)". */

static void
parse_variant(char *str)
{
    char *varname = NULL, *str2;
    Obj *varval = lispnil;

    if (strcmp(str, "") == 0) {
	push_key_int_binding(&variant_settings, K_WORLD_SEEN, 1);
    } else if (strcmp(str, "help") == 0) {
	variant_help_wanted = TRUE;
    } else {
	str2 = strchr(str, '=');
	if (str2 != NULL && str2 != str) {
	    /* (should interp val as string or number) */
	    varval = new_number(atoi(str2 + 1));
	    varname = copy_string(str);
	    varname[str2 - str] = '\0';
	} else {
	    /* Assume varname by itself means "enable" (set to value of 1). */
	    varname = str;
	    varval = new_number(1);
	}
	if (varname)
	  push_binding(&variant_settings, intern_symbol(varname), varval);
    }
}

/* Load the game module asked for on the cmd line, or else the
   default. */

void
load_all_modules(void)
{
    if (mainmodule != NULL) {
	load_game_module(mainmodule, TRUE);
    } else {
	/* If we've got absolutely no files to load, the standard game
	   is the one to go for.  It will direct the remainder of
	   random generation. */
	load_default_game();
    }
}

/* Set module variants from command line options. */

void
set_variants_from_options(void)
{
    int which;
    char *varname;
    Obj *varrest, *varset;

    /* Only the host of a networked game can set variants. */
    if (option_game_to_join != NULL) {
	if (variant_settings != lispnil
	    || option_width > 0)
	  fprintf(stderr, "Not the host, ignoring variant settings\n");
	return;
    }
    if (option_width > 0) {
	which = find_variant_from_name(mainmodule, keyword_name(K_WORLD_SIZE));
	if (which >= 0)
	  net_set_variant_value(which, option_width, option_height,
				option_circumference);
	else
	  fprintf(stderr, "World size variant not available, -M ignored\n");
    }
    /* Set the real-time variant if any times were supplied. */
    if (option_total_game_time != 0
	|| option_per_side_time != 0
	|| option_per_turn_time != 0) {
	which = find_variant_from_name(mainmodule, keyword_name(K_REAL_TIME));
	if (which >= 0)
	  net_set_variant_value(which, option_total_game_time,
				option_per_side_time, option_per_turn_time);
	else
	  fprintf(stderr, "Real time variants not available, ignored\n");
    }
    for_all_list(variant_settings, varrest) {
	varset = car(varrest);
	varname = c_string(car(varset));
	which = find_variant_from_name(mainmodule, varname);
	if (which >= 0)
	  net_set_variant_value(which, c_number(cadr(varset)), 0, 0);
	else
	  fprintf(stderr, "No variant `%s' known, ignored\n", varname);
    }
    do_module_variants(mainmodule, lispnil);
}

/* Given a string, return the index of the variant for the module, or
   -1 to indicate that it is not the name of a valid variant. */

static int
find_variant_from_name(Module *module, char *name)
{
    int i;
    Variant *var;

    if (module->variants == NULL)
      return -1;
    for (i = 0; module->variants[i].id != lispnil; ++i) {
	var = &(module->variants[i]);
	if (strcmp(name, c_string(var->id)) == 0)
	  return i;
    }
    return -1;
}

/* Set player characteristics from command-line options. */

void
set_players_from_options(void)
{
    int mergespecs;
    Player *player;
    struct raw_spec *spec;

    /* Only the host of a networked game can set up players. */
    if (option_game_to_join != NULL) {
	make_default_player_spec();
	return;
    }
    player = NULL;
    /* Assume that if any players exist already, then this is a restored
       game of some sort, and merge instead of adding new players. */
    mergespecs = (numplayers > 0);
    /* Add a player for the indepside if none found. */
    if (indepside_needed() && find_player(0) == NULL)
      player = add_player();
    /* If we're merging specs, skip over the indepside player. */
    if (mergespecs)
      player = playerlist->next;
    /* Add the default player. */
    if (option_game_to_host == NULL
	&& (raw_player_specs == NULL || option_add_default_player)) {
	if (!mergespecs)
	  player = add_default_player();
	parse_player_spec(player, raw_default_player_spec);
	canonicalize_player(player);
	if (player)
	  player = player->next;
    }
    /* Add the explicitly listed players. */
    for (spec = raw_player_specs; spec != NULL; spec = spec->next) {
	if (mergespecs) {
	    if (player == NULL) {
		fprintf(stderr, "Excess player spec `%s', ignoring\n",
			spec->spec);
		continue;
	    }
	} else {
	    player = add_player();
	}
	parse_player_spec(player, spec->spec);
	canonicalize_player(player);
	player = player->next;
    }
    /* If we made a special request to suppress AIs, go through all
       players existing at this time. */
    if (initially_no_ai) {
	for_all_players(player) {
	    player->aitypename = NULL;
	}
    }
    /* Transmit the official list of players to everybody else. */
    if (my_rid > 0 && my_rid == master_rid) {
	for_all_players(player) {
	    net_update_player(player);
	}
    }
}

/* This is not, strictly speaking, part of command line processing,
   but command-line programs are also the ones for which stdio is
   useful. */

void
print_instructions(void)
{
    Obj *instructions, *rest;

    printf("\n");
    if (mainmodule != NULL
	&& (instructions = mainmodule->instructions) != lispnil) {
	if (stringp(instructions)) {
	    printf("%s\n", c_string(instructions));
	} else if (consp(instructions)) {
	    for_all_list(instructions, rest) {
		if (stringp(car(rest))) {
		    printf("%s\n", c_string(car(rest)));
		} else {
		    run_warning("Instructions are of wrong type");
		}
	    }
	} else {
	    run_warning("Instructions are of wrong type");
	}
    } else {
	printf("(no instructions supplied)\n");
    }
}

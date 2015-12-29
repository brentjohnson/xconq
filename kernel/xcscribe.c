/* Xconq documentation writer.
   Copyright (C) 2004 Eric A. McDonald

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This writes out documentation of game modules to various file formats. */

#include "conq.h"
#include <wait.h>
#include "kpublic.h"
#include "print.h"
#include "cmdline.h"
#include "imf.h"
#include "ui.h"

extern Side* dside;
Side *defaultside = NULL;

/* This structure maintains state that is local to a side's display.
   At the very least, it must track when the display is open and closed. */

typedef struct a_ui {
    int active;
} UI;

/* Box for storing the results of command-line parsing. */

typedef struct param_box_cli {
    int flags;
    HelpOutputMode houtmode;
    char **modnamelist;
} CLIParamBox;

#define CLI_FLAG_DOCUMENT_GAMES	    1
#define CLI_FLAG_DOCUMENT_COMMANDS  2
#define CLI_FLAG_DOCUMENT_SYMBOLS   4
#define CLI_FLAG_MULTIPLE_GAMES	    8
#define CLI_FLAG_ALL_GAMES	    16
#define CLI_FLAG_INDEX_GAMES	    32

/* Tree structure to emulate game hierarchy. */

typedef struct game_tree_node {
    Module *module;
    struct game_tree_node *sibling;
    struct game_tree_node *child;
} GameTreeNode;

/* Prototypes of Xcscribe functions. */

static void parse_xcscribe_command_line (CLIParamBox *pboxcli, int argc, 
					 char *argv []);
static void show_xscribe_help_usage(char *argv0);
static void write_game_index(CLIParamBox *pboxcli);
static void add_to_game_tree(GameTreeNode **gametree, Module *module);
static GameTreeNode *create_game_tree_node(Module *module);
static GameTreeNode *find_game_in_tree(GameTreeNode *gametree, Module *module);
static void write_game_index_aux(GameTreeNode *gametree, CLIParamBox *pboxcli, 
				 int indentlvl);
static void write_help_files(HelpOutputMode houtmode, char *docdir, 
			     char *curmodulename);

#define MNAME_BUF_SIZE	255
#define ARGLIST_SIZE	6

/* Master-worker process model. Parent forks out a child process for each 
   game module to be documented in sequential order. */

int
main(int argc, char *argv[])
{
    int i = 0;
    CLIParamBox pboxcli;
    char curmodulename [MNAME_BUF_SIZE];
    Module *curmodule = NULL;
    int idxcurmodule = 0;
    pid_t pid = 0;
    int childstatus = 0, waitresult = 0;
    char *homedir = NULL, *docdir = NULL;

    /* Prep some data structures. */
    memset(curmodulename, 0, MNAME_BUF_SIZE);
    /* Setup the library path. */
    init_library_path(NULL);
    /* Initialize Xconq. */
    clear_game_modules();
#ifdef DEBUGGING
    init_debug_to_stdout();
#endif /* DEBUGGING */
    init_data_structures();
    /* Collect complete list of games. */
    collect_possible_games();
    if (!possible_games)
      init_error("No games were found in your game library!");
    /* Parse the command line. */
    parse_xcscribe_command_line(&pboxcli, argc, argv);
    /* Prepare documentation directory. */
    homedir = game_homedir();
    docdir = (char *)xmalloc(strlen(homedir) + 20);
    make_pathname(homedir, "doc", NULL, docdir);
    if (access(docdir, F_OK)) {
	mkdir(docdir, 0755);
	/* TODO: Should handle any errors that result from this. */
    }
    /* Set some help output options. */
    set_help_output_cc(HELP_OUTPUT_CC_FILES);
    set_help_output_mode(pboxcli.houtmode);
    set_help_output_dir(docdir);
    /* Write master game index, if requested. */
    if (pboxcli.flags & CLI_FLAG_INDEX_GAMES)
      write_game_index(&pboxcli);
    /* Fork/exec loop. */
    while (1) {
	/* Load the name of the module to process. */
	if (pboxcli.flags & CLI_FLAG_MULTIPLE_GAMES) {
	    if (pboxcli.flags & CLI_FLAG_ALL_GAMES) {
		if (idxcurmodule >= numgames)
		  break;
		else
		  strncpy(curmodulename, (possible_games[idxcurmodule])->name, 
			  MNAME_BUF_SIZE);
	    }
	    else {
		if (pboxcli.modnamelist[idxcurmodule])
		  strncpy(curmodulename, pboxcli.modnamelist[idxcurmodule], 
			  MNAME_BUF_SIZE);
		else
		  break;
	    }
	}
	else {
	    if (pboxcli.flags & CLI_FLAG_DOCUMENT_GAMES) {
		if (idxcurmodule)
		  break;
		else
		  strncpy(curmodulename, pboxcli.modnamelist[0], 
			  MNAME_BUF_SIZE);
	    }
	}
	/* Fork, if multiple games to process. */
	if (pboxcli.flags & CLI_FLAG_MULTIPLE_GAMES) {
	    pid = fork();
	    /* If error... */
	    if (0 > pid) {
		perror("xcscribe");
		run_error("Failed to start next doc writer process");
		exit(1);
	    }
	}
	/* If parent and multiple games to be written... */
	if ((pboxcli.flags & CLI_FLAG_MULTIPLE_GAMES) && pid) {
	    /* Wait for child to complete. */
	    waitresult = waitpid(pid, &childstatus, 0);
	    if (0 > waitresult) {
		perror("xcscribe");
		run_error("Waiting for a doc writer process failed");
		exit(1);
	    }
	    if (waitresult == pid) {
		if (WEXITSTATUS(childstatus))
		  run_error("Doc writer process terminated abnormally");
	    }
	    /* Increment module index. */
	    ++idxcurmodule;
	    /* Loop again. */
	    continue;
	} /* parent */
	/* If child or single game to be written... */
	else {
	    /* TODO: Conditionally write games only if CLI_FLAG_DOCUMENT_GAMES 
		     is set. */
	    /* Find the specified module. */
	    for (i = 0; i < numgames; ++i) {
		if (!strcmp(curmodulename, (possible_games[i])->name)) {
		    mainmodule = curmodule = possible_games[i];
		    break;
		}
	    }
	    /* Make sure that we found the specified module. */
	    if (NULL == curmodule) {
		init_error("Could not find the specified game module.");
		exit(1);
	    }
	    /* Load the designated module and any modules that it relies 
	       upon. */
	    load_all_modules();
	    /* See if we have something resembling a valid game. */
	    check_game_validity();
	    /* Write out the help files. */
	    write_help_files(pboxcli.houtmode, docdir, curmodulename);
	    /* Return */
	    return 0;
	}
    } /* Fork/exec loop. */
    return 0;
}

/* Parse command line. */

static void
parse_xcscribe_command_line (CLIParamBox *pboxcli, int argc, char *argv [])
{
    int i = 0, modnamelistidx = 0;

    /* Sanity checks. */
    assert_error(pboxcli, 
		 "Attempted to use a NULL command line argument container");
    assert_error((1 <= argc), "Invalid number of command line arguments");
    assert_error(argv, "Invalid command line arguments list");
    /* Initialize parambox. */
    pboxcli->flags = 0;
    pboxcli->houtmode = HELP_OUTPUT_HTML;
    pboxcli->modnamelist = (char **)xmalloc((numgames + 1) * sizeof(char *));
    memset(pboxcli->modnamelist, 0, (numgames + 1) * sizeof(char *));
    /* Parse the args, if any. */
    if (argc > 1) {
	for (i = 1; i < argc; ++i) {
	    if (!strncmp(argv[i], "--text", 6)
		|| !strncmp(argv[i], "-T", 2))
	      pboxcli->houtmode = HELP_OUTPUT_PLAIN_TEXT;
	    else if (!strncmp(argv[i], "--html", 6)
		|| !strncmp(argv[i], "-H", 2))
	      pboxcli->houtmode = HELP_OUTPUT_HTML;
	    else if (!strncmp(argv[i], "--all-games", 11)
		     || !strncmp(argv[i], "-AG", 3)) {
		pboxcli->flags |= CLI_FLAG_DOCUMENT_GAMES;
		pboxcli->flags |= CLI_FLAG_MULTIPLE_GAMES;
		pboxcli->flags |= CLI_FLAG_ALL_GAMES;
	    }
	    else if (!strncmp(argv[i], "--game-index", 12)
		     || !strncmp(argv[i], "-IG", 3)) {
		pboxcli->flags |= CLI_FLAG_INDEX_GAMES;
	    }
	    else if (!strncmp(argv[i], "--all-commands", 14)
		     || !strncmp(argv[i], "-AC", 3)) {
		pboxcli->flags |= CLI_FLAG_DOCUMENT_COMMANDS;
		printf("\n\"%s\" is not yet implemented.", argv[i]);
	    }
	    else if (!strncmp(argv[i], "--all-symbols", 13)
		     || !strncmp(argv[i], "-AS", 3)) {
		pboxcli->flags |= CLI_FLAG_DOCUMENT_SYMBOLS;
		printf("\n\"%s\" is not yet implemented.", argv[i]);
	    }
	    /* NOTE: Parsing of the "all" argument must go after the 
	       "all-*" arguments. */
	    else if (!strncmp(argv[i], "--all", 5)
		     || !strncmp(argv[i], "-A", 2)) {
		pboxcli->flags |= CLI_FLAG_DOCUMENT_GAMES;
		pboxcli->flags |= CLI_FLAG_DOCUMENT_COMMANDS;
		pboxcli->flags |= CLI_FLAG_DOCUMENT_SYMBOLS;
		pboxcli->flags |= CLI_FLAG_MULTIPLE_GAMES;
		pboxcli->flags |= CLI_FLAG_ALL_GAMES;
		pboxcli->flags |= CLI_FLAG_INDEX_GAMES;
	    }
	    else if (!strncmp(argv[i], "--help", 6)
		     || !strncmp(argv[i], "-help", 5)
		     || !strncmp(argv[i], "-?", 2)
		     || !strncmp(argv[i], "/HELP", 5)
		     || !strncmp(argv[i], "/help", 5)
		     || !strncmp(argv[i], "/?", 2)) {
		show_xscribe_help_usage(argv[0]);
		exit(0);
	    }
	    else {
		pboxcli->flags |= CLI_FLAG_DOCUMENT_GAMES;
		if (modnamelistidx > 0)
		  pboxcli->flags |= CLI_FLAG_MULTIPLE_GAMES;
		pboxcli->modnamelist[modnamelistidx++] = argv[i];
	    }
	}
    } /* argc > 1 */
    /* Set reasonable defaults if nothing specified. */
    else {
	pboxcli->flags |= CLI_FLAG_DOCUMENT_GAMES;
	pboxcli->flags |= CLI_FLAG_MULTIPLE_GAMES;
	pboxcli->flags |= CLI_FLAG_ALL_GAMES;
	pboxcli->flags |= CLI_FLAG_INDEX_GAMES;
    } /* args == 1 */
    /* Some final touchups. */
    if (!(pboxcli->modnamelist[0]) 
	&& !(pboxcli->flags & CLI_FLAG_MULTIPLE_GAMES)) {
	pboxcli->flags |= CLI_FLAG_MULTIPLE_GAMES;
	pboxcli->flags |= CLI_FLAG_ALL_GAMES;
	if (!(pboxcli->flags & CLI_FLAG_INDEX_GAMES)) {
	    pboxcli->flags |= CLI_FLAG_INDEX_GAMES;
	    pboxcli->flags |= CLI_FLAG_DOCUMENT_GAMES;
	}
    }
    fflush(NULL);
}

/* Describe the command-line arguments. */

static void
show_xscribe_help_usage(char *argv0)
{
    printf("\n\n");
    if (!argv0)
      printf("xcscribe");
    else
      printf("%s", argv0);
    printf(" [ --all ] [ -A ] [ --all-games ] [ -AG ]");
    printf(" [ --game-index ] [ -IG ]");
    printf(" [ --all-commands ] [ -AC ]");
    printf(" [ --all-symbols ] [ -AS ]");
    printf(" [ --html ] [ -H ]");
    printf(" [ --text ] [ -T ]");
    printf(" [ [ <game module name> ] ... ]\n");
    printf("\t--all | -A\t\tDocument all games, commands, and symbols.\n");
    printf("\t--all-games | -AG\tDocument all games.\n");
    printf("\t--game-index | -IG\tCreate a master index of all the games.\n");
    printf("\t--all-commands | -AC\tDocument all commands.\n");
    printf("\t--all-symbols | -AS\tDocument all symbols.\n");
    printf("\t--html | -H\t\tProduce output in HTML.\n");
    printf("\t--text | -T\t\tProduce output in plain text.\n");
    printf("\n");
    fflush(NULL);
}

/* Write out a file containing the index of the games. */

static void
write_game_index(CLIParamBox *pboxcli)
{
    int i = 0, j = 0;
    char curmodname [BUFSIZE];
    Module *curmod = NULL;
    GameTreeNode *gametree = NULL;
    char idxfilename [BUFSIZE];
    FILE *hidxfilep = NULL;

    make_pathname("", "index", get_help_file_extension(), idxfilename);
    hidxfilep = prep_help_file(idxfilename);
    write_help_file_header(hidxfilep, "Game Index");
    for (i = 0; i < numgames; ++i) {
	curmod = NULL;
	if (pboxcli->flags & CLI_FLAG_ALL_GAMES) 
	  curmod = possible_games[i];
	else {
	    memset(curmodname, 0, BUFSIZE);
	    if (!((pboxcli->modnamelist)[i]))
	      break;
	    else {
		strncpy(curmodname, (pboxcli->modnamelist)[i], BUFSIZE);
		for (j = 0; j < numgames; ++j) {
		    if (!strcmp(curmodname, (possible_games[j])->name)) {
			curmod = possible_games[j];
			break;
		    }
		}
		if (!curmod)
		  init_error("Could not find game module to index");
	    }
	}
	add_to_game_tree(&gametree, curmod);
    }
    set_help_toc_filep(hidxfilep);
    write_game_index_aux(gametree, pboxcli, 0);
    set_help_toc_filep(NULL);
    write_help_file_footer(hidxfilep, "Game Index");
    finish_help_file(hidxfilep);
}

/* Add a game module to a game tree. */

static void
add_to_game_tree(GameTreeNode **gametree, Module *module)
{
    Module *basemodule = NULL;
    GameTreeNode *curnode = NULL;

    assert_error(module, "Attempted to add a NULL game module to game tree");
    /* Insert parent modules first. */
    if (module->basemodulename) {
	basemodule = get_game_module(module->basemodulename);
	add_to_game_tree(gametree, basemodule);
    }
    /* If tree is empty, then create a first node for it. */
    if (!(*gametree)) {
	gametree[0] = create_game_tree_node(module);
	return;
    }
    /* If the game is not already in the tree, then add it. */
    if (!find_game_in_tree(gametree[0], module)) {
	/* If parent module is in the tree, then add relative to the parent. */
	if (module->basemodulename) {
	    curnode = find_game_in_tree(gametree[0], basemodule);
	    assert_error(curnode, 
			 "Unexpected failure to find a game in game tree");
	    /* Add directly off of parent node, if first child. */
	    if (!(curnode->child)) {
		curnode->child = create_game_tree_node(module);
		return;
	    }
	    else
	      curnode = curnode->child;
	}
	/* Add independent game relative to the base of the tree. */
	else {
	    curnode = gametree[0];
	}
	/* Find a free sibling slot. */
	while (curnode->sibling)
	  curnode = curnode->sibling;
	/* Add to free sibling slot. */
	curnode->sibling = create_game_tree_node(module);
	return;
    }
    else
      return;
}

/* Create a new game tree node. */

static GameTreeNode *
create_game_tree_node(Module *module)
{
    GameTreeNode *node = NULL;

    assert_error(module, "Attempted to add a NULL game module to game node");
    node = (GameTreeNode *)xmalloc(sizeof(GameTreeNode));
    node->module = module;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

/* Find a game in a game tree. Depth-first search. */

static GameTreeNode *
find_game_in_tree(GameTreeNode *gametree, Module *module)
{
    GameTreeNode *gamebranch = NULL; 

    assert_error(gametree, "Attempted to search an empty game tree");
    assert_error(module, "Attempted to find NULL game module in game tree");
    if (!strcmp(gametree->module->name, module->name))
      return gametree;
    if (gametree->child) {
	if ((gamebranch = find_game_in_tree(gametree->child, module)))
	  return gamebranch;
    }
    if (gametree->sibling)
      return find_game_in_tree(gametree->sibling, module);
    return gamebranch;
}

/* Write out game hierarchy into a game index file. */

static void
write_game_index_aux(GameTreeNode *gametree, CLIParamBox *pboxcli, 
		     int indentlvl)
{
    char gameidxname [BUFSIZE];

    assert_error(gametree, "Attempted to search an empty game tree");
    switch (pboxcli->houtmode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	make_pathname(gametree->module->name, "index", NULL, gameidxname);
	break;
      case HELP_OUTPUT_HTML:
	strncpy(gameidxname, gametree->module->name, BUFSIZE);
	strncat(gameidxname, "/index", 
		BUFSIZE - strlen(gametree->module->name));
	break;
      default: break;
    }
    write_help_toc_entry(gameidxname, gametree->module->title, indentlvl);
    if (gametree->child)
	write_game_index_aux(gametree->child, pboxcli, indentlvl+1);
    if (gametree->sibling)
      write_game_index_aux(gametree->sibling, pboxcli, indentlvl);
}

/* Set up the help system to write out files in the specified format and 
   in the specified place, and then do it. */

static void
write_help_files(HelpOutputMode houtmode, char *docdir, char *curmodulename)
{
    char *gamedocdir = NULL;

    /* Sanity checks. */
    assert_error(docdir, "Tried to write help files to a NULL doc directory.");
    assert_error(curmodulename, "Tried to write help for a NULL module name.");
    assert_error(curmodulename[0], 
		 "Tried to write help for an empty module name.");
    /* Determine the game documentation directory and create it, 
       if necessary. */
    gamedocdir = (char *)xmalloc(strlen(docdir) + 10 + strlen(curmodulename));
    strcpy(gamedocdir, docdir);
    strcat(gamedocdir, "/");
    strcat(gamedocdir, curmodulename);
    if (access(gamedocdir, F_OK)) {
	mkdir(gamedocdir, 0755);
	/* \TODO: Should handle any errors that result from this.\ */
    }
    /* Set help output directory. */
    set_help_output_dir(gamedocdir);
    /* Report the documentation's location. */
    printf("Writing game docs to '%s'.\n", gamedocdir);
    /* Initialize help system. */
    init_help();
    /* Write out all the help files for the current module. */
    create_game_help_nodes();
}

void
run_ui_idler (void)
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

void
update_cell_display(Side *side, int x, int y, int flags)
{
}

void
update_turn_display(Side *side, int rightnow)
{
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
update_all_progress_displays(const char *str, int s)
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


/* This reports progress in reading GDL files. */

void
announce_read_progress(void)
{
}

/* This is used for initialization steps that take a long time. */

int linemiddle = FALSE;

void
announce_lengthy_process(const char *msg)
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
schedule_movie(Side *side, const char *movie, ...)
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
low_init_error(const char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stderr, "%s Error: %s.\n", (mainmodule ? mainmodule->name : ""), 
	    str);
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
    fprintf(stdout, "%s Warning: %s.\n", (mainmodule ? mainmodule->name : ""), 
	    str);
    fflush(stdout);

    report_warning();
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    if (linemiddle)
      fprintf(stderr, "\n");
    fprintf(stderr, "%s Error: %s.\n", (mainmodule ? mainmodule->name : ""), 
	    str);
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
    fprintf(stdout, "%s Warning: %s.\n", (mainmodule ? mainmodule->name : ""), 
	    str);
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

void
make_generic_image_data(ImageFamily *imf)
{
}

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
add_remote_locally(int rid, const char *str)
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

/* The main program of the curses interface to Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "cconq.h"

#ifdef __MWERKS__

#include <sioux.h>

_MSL_BEGIN_EXTERN_C
_MSL_IMP_EXP_C extern int ccommand(char ***);
_MSL_END_EXTERN_C

#endif /* __MWERKS__ */

    /* If we are using the pdcurses library we have to declare this stuff 
       here, which is otherwise declared in curses.c of libcurses. */
#if (defined (WIN32) && !defined (__CYGWIN32__))

#ifndef EXPORT
#define EXPORT
#endif
/* The complete PDCurses lib already defines the following. */
#ifndef PDCURSES
EXPORT int	COLS = 0;
EXPORT int	LINES = 0;
EXPORT WINDOW* stdscr;
#else
extern int	COLS;
extern int	LINES;
#endif

#endif

extern int autofinish_start;
extern int autofinish_count;

static void describe_help(int arg, const char *key, TextBuffer *buf);

#ifdef MAC /* temporary */
int use_mac_charcodes = TRUE;
char **current_cursor;
char **sendcursor;
char **receivecursor;
#endif

int announced = FALSE;

char *announcemsg = NULL;

/* move to where a change has occured */

int follow_action;

/* The location currently being examined. */

int curx, cury;

int tmpcurx, tmpcury;

/* The unit currently being examined, if any. */

Unit *curunit;

Unit *tmpcurunit;

/* The current interaction mode. */

enum mode mode;

/* The pushed interaction mode. */

enum mode prevmode;

/* The current input character. */

char inpch;

/* The command's numeric argument. */

int prefixarg;

/* These are used when asking for a unit type. */

char *ustr;
int *uvec;
int *bvec;

/* length and number of notice lines */

int nw, nh;

/* last current x and y (-1,-1 initially) */

int lastx, lasty;

/* current prompt on display */

char *text1;
char *text2;

/* data about string under construction */

int reqstrbeg;
int reqstrend;

/* The help window. */

struct ccwin *helpwin;

/* Cached help info. */

HelpNode *cur_help_node;

HelpNode *help_help_node;

HelpNode *topics_help_node;

struct ccwin *datewin;

struct ccwin *sideswin;

struct ccwin *toplineswin;

struct ccwin *clockwin;

/* Display of a piece of the world */

struct ccwin *mapwin;

/* Unit listing */

struct ccwin *listwin;

struct ccwin *closeupwin;

struct ccwin *sidecloseupwin;

/* Map dimensions. */

int mw, mh;

VP *mvp;

/* lower left of viewport in cell coords */

int vx, vy;

/* Boundaries of viewport in cell coords */

int vw, vh;

/* 1/2 (rounded down) of above values */

int vw2, vh2;

int lastvcx, lastvcy;

/* Num text lines in unit info window. */

int infoh;

/* List dimensions. */

int lw, lh;

/* Side list dimensions. */

int sh;

/* How to draw the map. */

int drawterrain;

/* Display units on the map? */

int drawunits;

/* Display unit names/numbers on the map? */

int drawnames;

int drawpeople;

int draw_cover;

enum listsides listsides;
int test;
int value;
int sorton;
int sortorder;

/* True when the display is up and running. */

int active;

enum movie_type {
  movie_null,
  movie_miss,
  movie_hit,
  movie_death,
  movie_nuke
};

struct a_movie {
  const char *type;
  enum movie_type itype;
  int args[5];
};

/* Number of movies waiting to be played. */

int numscheduled;

/* Movies to be played. */

struct a_movie *movies;

/* The main program just invokes everything else. */

int
main(int argc, char *argv[])
{

#ifdef MAC

#ifdef __MWERKS__

    /* This is how Metrowerks C picks up a command line. */
    argc = ccommand(&argv);
    /* Configure the SIOUX window. */
    SIOUXSettings.initializeTB = TRUE;
    SIOUXSettings.standalone = FALSE;
    SIOUXSettings.setupmenus = FALSE;
    SIOUXSettings.autocloseonquit = FALSE;
    SIOUXSettings.asktosaveonclose = TRUE;
    SIOUXSettings.showstatusline = TRUE; 

#endif /* __MWERKS__ */

    _maccur_pgm_name = "Cconq";

#endif /* MAC */

    printf("\n              Welcome to Curses Xconq version %s\n\n",
	   version_string());
    init_library_path(NULL);
    print_any_news();
    printf("%s", license_string());
    clear_game_modules();
    init_data_structures();
    parse_command_line(argc, argv, general_options);
#ifdef DEBUGGING
    update_debugging();
#endif /* DEBUGGING */
    if (option_game_to_join != NULL) {
	int rslt;

	/* (should parse variant specs, then comment that they must be ignored) */
	/* (should accept player specs) */
	/* Go through once more and complain about anything not used. */
	parse_command_line(argc, argv, leftover_options);
	hosting = FALSE;
	rslt = open_remote_connection(option_game_to_join, hosting);
	if (rslt > 0) {
	    rslt = send_join("xxx");
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
	    }
	}
    } else {
	/* (should volunteer to restore saved game if one found?) */
	load_all_modules();
	check_game_validity();
	parse_command_line(argc, argv, variant_options);
	set_variants_from_options();
	parse_command_line(argc, argv, player_options);
	set_players_from_options();
	/* Go through once more and complain about anything not used. */
	parse_command_line(argc, argv, leftover_options);
	/* (should call launch_game() here) */
	make_trial_assignments();
	if (option_game_to_host != NULL) {
	    hosting = TRUE;
	    open_remote_connection(option_game_to_host, hosting);
	}

	/* Give all unassigned players to the master program. This
	   code, taken from macinit.c, finally solved the ai problem
	   in unix network games. */
	if (numremotes > 0) {
	    Player *player;

	    for_all_players(player) {
		if (player->rid == 0)
		    player->rid = master_rid;
	    }
	}

	calculate_globals();
	run_synth_methods();
	final_init();
	assign_players_to_sides();
#ifndef HAVE_SELECT
	/* If no way to check for input periodically, then we must force
	   sequentiality.  This must happen before the first run_game. */
	set_g_use_side_priority(TRUE);
#endif
	init_display();
	/* Do first set of turn calcs, but let the host lead the way. */
	if (my_rid == 0 || (my_rid > 0 && my_rid == master_rid)) {
		net_run_game(0);
	}
    }
    init_interaction();
    redraw();
    init_signal_handlers();
    while (TRUE) {
#ifdef HAVE_SELECT
please somebody write this code
	net_run_game(1);
#else
	run_local_ai(1, 20);
	/* assumes no real-timeness */
	while (probability(99)) {  /* a hack */
	    net_run_game(9);
	    run_local_ai(2, 20);
	}
	maybe_handle_input(0);
#endif
    }
    /* Humor the compiler. */
    return 0;
}

/* Nonempty display name not actually used, but needed to keep things
   straight. */

Player *
add_default_player(void)
{
    Player *player = add_player();

    player->name = getenv("USER");
    player->displayname = "term";
    return player;
}

void
make_default_player_spec(void)
{
    default_player_spec = "term";
}

/* Set up the basic user interface for a side. */

void
init_ui(Side *side)
{
    if (side_wants_display(side)) {
	if (dside == NULL) {
	    side->ui = (struct a_ui *) xmalloc(sizeof(int));
	    dside = side;
	    active = FALSE;
	    follow_action = FALSE;
	    cur_help_node = NULL;
	    mode = prevmode = SURVEY;
	    prefixarg = -1;
	} else {
	    fprintf(stderr, "More than one side wanted a display!\n");
	    exit(1);
	}
    }
}

/* Open display, create all the windows we'll need, do misc setup
   things, and initialize some globals to out-of-range values for
   recognition later. */

void
init_display(void)
{
    int x, y, i;

    if (dside == NULL) {
	fprintf(stderr, "No side wanted a display!\n");
	exit(1);
    }
    Dprintf("Will try to open screen as display...\n");
    /* Go through the standard curses setup sequence. */
    initscr();

    /* If we are using the pdcurses library we have to do this stuff 
       ourselves, which is otherwise done by initscr in libcurses. */
#if (defined (WIN32) && !defined (__CYGWIN32__))
    if (LINES == 0) {
		LINES = 24;
    }
    if (COLS == 0) {
		COLS = 80;
    }
    stdscr = newwin(0,0,0,0);
    if (stdscr == NULL) {
	fputs("curses: cannot allocate stdscr window.\n", stderr);
	abort();
    }
#endif

    nonl();
    noecho();
    cbreak();
#if 0 /* def MAC */ /* can't do this yet */
    nodelay(stdscr, 1);
#endif
    clear();
    /* Set up random globals. */
    nw = min(BUFSIZE, 60);
    nh = 1;
    text1 = (char *)xmalloc (BUFSIZE);
    text2 = (char *)xmalloc (BUFSIZE);
    text1[0] = '\0';
    text2[0] = '\0';
    ustr = (char *)xmalloc (numutypes);
    uvec = (int *) xmalloc (numutypes * sizeof(int));
    bvec = (int *) xmalloc (numutypes * sizeof(int));
    /* Compute the division of the screen. */
    mw = (2 * COLS) / 3;
    infoh = 4;
    mh = LINES - 2 - infoh - 1;

    mvp = new_vp();
    /* Each cell is actually 2 chars or "pixels" wide.  This is a
       difference from the standard power==0 parameters, so we do a
       cheesy thing and tweak the array directly. */
    hws[0] = 2;
    set_view_power(mvp, 0);
    set_view_size(mvp, mw, mh);

    lw = COLS - mw - 1;

    sh = numtotsides + 1;
    /* The height of the list window is what's left over after making
       space for the side list and status lines. */
    lh = LINES - sh - 1 - 1;

    pick_a_focus(dside, &x, &y);
    set_view_focus(mvp, x, y);
    lastvcx = lastvcy = -1;
    /* Set default values for the display controls. */
    drawterrain = TRUE;
    drawunits = TRUE;
    drawnames = FALSE;  /* they clutter up the screen */
    listsides = allsides;
    curunit = NULL;
    /* Create all the windows. */
    toplineswin = create_window(0, 0, nw, 2);
    datewin = create_window(nw, 0, COLS - nw, 1);
    clockwin = create_window(nw, 1, COLS - nw, 1); 
    closeupwin = create_window(0, 2, mw, infoh);
    mapwin = create_window(0, 2 + infoh, mw, mh + 1);
    sideswin = create_window(mw + 1, 2, lw, sh);
    listwin = create_window(mw + 1, 2 + sh, lw, lh);
    /* Help window covers the screen. */
    helpwin = create_window(0, 0, COLS, LINES);
    /* Set up the initial scroll position of the map. */
    set_scroll();
    /* Make a buffer full of dashes, for use on mode lines. */
    if (dashbuffer == NULL) {
	dashbuffer = (char *)xmalloc(COLS + 1);
	for (i = 0; i < COLS; ++i)
	  dashbuffer[i] = '-';
	dashbuffer[COLS] = '\0';
    }
    init_ui_chars();
    active = TRUE;
    Dprintf("Successfully opened display!\n");
}

void
init_interaction(void)
{
    curunit = NULL;
    find_next_and_look();
    if (curunit) {
	mode = MOVE;
    } else {
	mode = SURVEY;
	move_survey(mvp->vcx, mvp->vcy);
    }
    show_cursor();
    /* Set up some cconq-specific help nodes. */
    if (cur_help_node == NULL) {
	help_help_node =
	  add_help_node("help", describe_help, 0, first_help_node);
	key_commands_help_node =
	  add_help_node("commands", describe_key_commands, 0, first_help_node);
	long_commands_help_node =
	  add_help_node("long commands", describe_long_commands, 0,
			key_commands_help_node);
	topics_help_node =
	  add_help_node("topics", describe_topics, 0, first_help_node);
	cur_help_node = topics_help_node;
    }
    /* By default, require manual `return' to finish each turn. */
    net_set_autofinish(dside, FALSE);
}

/* Windows in curses are just simple bounding boxes. */

struct ccwin *
create_window(int x, int y, int w, int h)
{
    struct ccwin *newwin;

    if (x + w > COLS)
      w = COLS - x;
    if (y + h > LINES)
      h = LINES - y;
    DGprintf("Creating %dx%d window at %d,%d\n", w, h, x, y);

    newwin = (struct ccwin *) xmalloc(sizeof(struct ccwin));
    newwin->x = x;  newwin->y = y;
    newwin->w = w;  newwin->h = h;
    return newwin;
}

/* Generic input character acquisition. */

/* (should have a global that indicates whether this is in a modal
   place, then run_game not called on timeout, but waits until out
   of modal thing (perhaps aborts modal dialog automatically, then runs?) */

int
wait_for_char(void)
{
    show_cursor();
    /* Only take 7-bit chars. */
    inpch = getch() & 0177;
    return inpch;
}

void
maybe_handle_input(int tmout)
{
    Unit *unit;

    if (active_display(dside)) {
	if (mode == MOVE) {
	    if (curunit != NULL
	    	&& in_play(curunit)
		&& has_acp_left(curunit)
	    	&& (curunit->plan
		    && !curunit->plan->asleep
		    && !curunit->plan->reserve
		    && !curunit->plan->delayed)
		) {
		/* should scroll over to unit */
	    } else {
		unit = autonext_unit_inbox(dside, curunit, mvp);
		if (unit
		    && unit->plan
		    && !unit->plan->asleep
		    && !unit->plan->reserve
		    && !unit->plan->delayed
		    && unit->plan->waitingfortasks
		    ) {
		    make_current(unit);
		} else {
		    make_current(autonext_unit(dside, curunit));
		}
	    }
	}
#ifdef HAVE_SELECT
please somebody write this code
#else
	wait_for_char();
#endif
	/* Program will not work without following due to compiler bug. (?) */
	Dprintf("key is %c", inpch); 
	/* Interpret the input. */
	interpret_input();
    }
}

/* Mandatory UI Definition: 'ui_run_idler'. Handle UI events while kernel 
   or AI processing is taking place. */

void
run_ui_idler(void)
{
#if (0)
    maybe_handle_input(0);
#endif
}

void
interpret_input(void)
{
    int dir;

    if (inpch == REDRAW) {
	/* Redraw everything but leave the interaction state unchanged. */
	redraw();
    } else if (mode == HELP) {
	interpret_help();
    } else if (isdigit(inpch)) {
	/* Build up a numeric prefix argument. */
	if (prefixarg < 0) {
	    prefixarg = 0;
	} else {
	    prefixarg *= 10;
	}
	prefixarg += (inpch - '0');
	sprintf(text1, "Arg: %d", prefixarg);
	show_toplines();
    } else {
	clear_toplines();
	if ((dir = iindex(inpch, dirchars)) >= 0) {
	    if (prefixarg < 0)
	      prefixarg = 1;
	    do_dir_2(dir, prefixarg);
	} else if ((dir = iindex(lowercase(inpch), dirchars)) >= 0) {
	    if (prefixarg < 0)
	      prefixarg = 9999;
	    if (mode == SURVEY)
	      prefixarg = 10;
	    do_dir_2(dir, prefixarg);
	} else {
	    dside->prefixarg = prefixarg;
	    execute_command(dside, inpch);
	}
	/* Reset the arg so we don't get confused next time around */
	prefixarg = -1;
    }
}

/* Interpret a direction character. */

void
do_dir_2(int dir, int n)
{
    int nx, ny, rslt;
    HistEventType reason;
    char failbuf[BUFSIZE];

    if (mode == SURVEY) {
	if (point_in_dir_n(curx, cury, dir, n, &nx, &ny)) {
	    move_survey(nx, ny);
	} else if (n >= 2
		   && point_in_dir_n(curx, cury, dir, n / 2, &nx, &ny)) {
	    move_survey(nx, ny);
	} else {
	    xbeep();
	}
    } else if (curunit == NULL) {
	return;  /* no beep? */
    } else if (n > 1) {
	net_set_move_dir_task(curunit, dir, n);
    } else {
	if (point_in_dir(curx, cury, dir, &nx, &ny)) {
	    rslt = advance_into_cell(dside, curunit, nx, ny, unit_at(nx, ny),
				     &reason);
	    if (!rslt) {
		advance_failure_desc(failbuf, curunit, reason);
		notify(dside, "%s", failbuf);
		xbeep();
	    }
	} else {
	    xbeep();
	}
    }
}

int
auto_attack_on_move(Unit *unit, Unit *unit2)
{
    return TRUE;
}

void
move_survey(int nx, int ny)
{
    if (inside_area(nx, ny)) {
	make_current_at(nx, ny);
    } else {
	/* Complain if we're trying to move the cursor offworld. */
	xbeep();
    }
}

/* Ensure that given location is visible. */

void
put_on_screen(int x, int y)
{
    if (!in_middle(x, y)) {
	set_view_focus(mvp, x, y);
	center_on_focus(mvp);
	set_map_viewport();
	show_map();
	show_cursor();
    }
    show_closeup();
}

/* Prompt for a yes/no answer with a settable default. */

int
ask_bool(const char *question, int dflt)
{
    char ch;

    prevmode = mode;
    mode = PROMPT;
    sprintf(text1, "%s [%s]", question, (dflt ? "yn" : "ny"));
    show_toplines();
    ch = wait_for_char();
    if (dflt ? (lowercase(ch) == 'n') : (lowercase(ch) == 'y'))
      dflt = !dflt;
    mode = prevmode;
    clear_toplines();
    return dflt;
}

/* Prompt for a type of a unit from player, maybe only allowing some
   types to be accepted.  Also allow specification of no unit type.
   We do this by scanning the vector, building a string of chars and a
   vector of unit types, so as to be able to map back when done. */

int
ask_unit_type(const char *prompt, int *possibles)
{
    char ch;
    int numtypes = 0, u;

    for_all_unit_types(u) {
	bvec[u] = 0;
	if (possibles == NULL || possibles[u]) {
	    bvec[u] = 1;
	    uvec[numtypes] = u;
	    ustr[numtypes] = unitchars[u];
	    numtypes++;
	}
    }
    if (numtypes == 0) {
	return NONUTYPE;
    } else {
	prevmode = mode;
	mode = PROMPT;
	ustr[numtypes] = '\0';
	sprintf(text1, "%s [%s] ", prompt, ustr);
	show_toplines();
    }
    while (1) {
	ch = wait_for_char();
	if (ch == ESCAPE) {
	    mode = prevmode;
	    clear_toplines();
	    return NONUTYPE;
	}
	if (iindex(ch, ustr) >= 0) {
	    mode = prevmode;
	    clear_toplines();
	    return uvec[iindex(ch, ustr)];
	}
	xbeep();
    }
}

/* Prompt for a type of a terrain from player, maybe only allowing
   some types to be accepted.  Also allow specification of no terrain
   type.  We do this by scanning the vector, building a string of
   chars and a vector of terrain types, so as to be able to map back
   when done. */

int
ask_terrain_type(const char *prompt, int *possibles)
{
    char ch;
    int numtypes = 0, t, type;

    for_all_terrain_types(t) {
	bvec[t] = 0;
	if (possibles == NULL || possibles[t]) {
	    bvec[t] = 1;
	    uvec[numtypes] = t;
	    ustr[numtypes] = terrchars[t];
	    numtypes++;
	}
    }
    if (numtypes == 0) {
	type = NONTTYPE;
    } else if (numtypes == 1) {
	type = uvec[0];
	bvec[type] = 0;
    } else {
	ustr[numtypes] = '\0';
	sprintf(text1, "%s [%s] ", prompt, ustr);
	show_toplines();
    }
    while (1) {
	ch = wait_for_char();
	if (ch == ESCAPE) {
	    clear_toplines();
	    return NONTTYPE;
	}
	if (iindex(ch, ustr) >= 0) {
	    clear_toplines();
	    return uvec[iindex(ch, ustr)];
	}
	xbeep();
    }
}

/* Ask for a direction. */

int
ask_direction(const char *prompt, int *dirp)
{
    char ch;

    prevmode = mode;
    mode = PROMPTXY;
    sprintf(text1, "%s", prompt);
    sprintf(text2, " [direction keys]");
    show_toplines();
    save_cur();
    while (1) {
	ch = wait_for_char();
	if (ch == ESCAPE) {
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    return FALSE;
	}
	*dirp = iindex(ch, dirchars);
	if (*dirp >= 0) {
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    return TRUE;
	} else {
	    xbeep();
	}
    }
}

/* User is asked to pick a position on map.  This will iterate until
   '.' designates the final position. */

int
ask_position(const char *prompt, int *xp, int *yp)
{
    char ch;
    int dir, nx, ny;

    prevmode = mode;
    mode = PROMPTXY;
    sprintf(text1, "%s", prompt);
    sprintf(text2, " [direction keys to move, '.' to set]");
    show_toplines();
    save_cur();
    while (1) {
	ch = wait_for_char();
	if (ch == '.') {
	    *xp = curx;  *yp = cury;
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    return TRUE;
	}
	if (ch == ESCAPE) {
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    return FALSE;
	}
	if ((dir = iindex(ch, dirchars)) >= 0) {
	    point_in_dir(curx, cury, dir, &nx, &ny);
	    if (inside_area(nx, ny)) {
		curx = nx;  cury = ny;
		put_on_screen(curx, cury);
		show_cursor();
	    } else {
		xbeep();
	    }
	} else {
	    xbeep();
	}
    }
}

/* Save away the currently selected position. */

void
save_cur(void)
{
    tmpcurx = curx;  tmpcury = cury;
    tmpcurunit = curunit;
}

/* Restore the saved "cur" slots. */

void
restore_cur(void)
{
    curx = tmpcurx;  cury = tmpcury;
    curunit = tmpcurunit;
}

/* Read a string from the prompt window.  Deletion is allowed, and a
   cursor is displayed. */

int
ask_string(const char *prompt, const char *dflt, char **strp)
{
    char ch;
    int done = FALSE, rslt = FALSE;

    sprintf(text1, "%s ", prompt);
    reqstrbeg = strlen(text1);
    /* If a default was supplied, add it. */
    if (dflt != NULL)
      strcat(text1, dflt);
    reqstrend = strlen(text1);
    prevmode = mode;
    mode = PROMPT;
    show_toplines();
    while (!done) {
	ch = wait_for_char();
	switch (ch) {
	  case '\r':
	  case '\n':
	    /* Return a copy of the part of the buffer that was typed
               into. */
	    if (strp != NULL)
	      *strp = copy_string(text1 + reqstrbeg);
	    done = TRUE;
	    rslt = TRUE;
	    break;
	  case ESCAPE:
	    xbeep();
	    done = TRUE;
	    break;
	  case BACKSPACE:
	  case 0x7f:
	    if (reqstrend > reqstrbeg) {
		--reqstrend;
	    } else {
		xbeep();
	    }
	    break;
	  default:
	    if (reqstrend < BUFSIZE-2) {
		/* Insert the character. */
		(text1)[reqstrend++] = ch;
	    } else {
		xbeep();
	    }
	}
	/* Make sure we're always properly terminated. */
	(text1)[reqstrend] = '\0';
	show_toplines();
    }
    /* We're done, put everything back the way it was. */
    mode = prevmode;
    clear_toplines();
    show_cursor();
    return rslt;
}

/* Ask for a side. */

Side *
ask_side(const char *prompt, Side *dflt)
{
    char ch;
    Side *rslt = dflt;

    prevmode = mode;
    mode = PROMPT;
    sprintf(text1, "%s ", prompt);
    show_toplines();
    while (1) {
	show_cursor();
	ch = wait_for_char();
	if (ch == ESCAPE) {
	    rslt = dflt;
	    break;
	}
	if (between('0', ch, '9')) {
	    rslt = side_n(ch - '0');
	    break;
	}
	if (ch == '\r' || ch == '\n')
	  break;
	xbeep();
    }
    mode = prevmode;
    clear_toplines();
    return rslt;
}

int
ask_unit(char *prompt, Unit **unitp)
{
    char ch;
    int ndirs, dir, nx, ny, rslt = FALSE;

    *unitp = NULL;
    prevmode = mode;
    mode = PROMPTXY;
    sprintf(text1, "%s", prompt);
    sprintf(text2, " [direction keys to move, '.' to set]");
    show_toplines();
    save_cur();
    while (1) {
	ch = wait_for_char();
	if (ch == '.') {
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    rslt = TRUE;
	    break;
	}
	if (ch == ESCAPE) {
	    restore_cur();
	    mode = prevmode;
	    clear_toplines();
	    show_cursor();
	    break;
	}
	/* Also recognize the "in" ("occupant") command. */
	if (ch == 'i') {
	    if (*unitp != NULL)
	      *unitp = find_next_occupant(*unitp);
	    make_current(*unitp);
	    continue;
	}
	ndirs = char_to_dir(ch, &dir, NULL, NULL);
	if (ndirs > 0) {
	    point_in_dir(curx, cury, dir, &nx, &ny);
	    if (inside_area(nx, ny)) {
		make_current_at(nx, ny);
		if (curunit != NULL)
		  *unitp = curunit;
	    } else {
		xbeep();
	    }
	} else {
	    xbeep();
	}
    }
    return rslt;
}

/* Given a unit, make it be the one that we are "looking at". */

void
make_current(Unit *unit)
{
    curunit = unit;
    if (in_play(unit)) {
	curx = unit->x;  cury = unit->y;
	/* Make sure the screen shows this unit's char. */
	draw_row(curx, cury, 1);
    }
    show_closeup();
    show_cursor();
}

/* Give a location, make it be current, and pick a unit there to be
   the current unit. */

void
make_current_at(int x, int y)
{
    Unit *unit;

    curx = x;  cury = y;
    curunit = NULL;
    /* Look for a top-level unit to become the new current one. */
    for_all_stack(x, y, unit) {
	if (unit->side == dside) {
	    make_current(unit);
	    break;
	}
    }
    put_on_screen(curx, cury);
}

/* (should add individual screen scrolling also) */

void
interpret_help(void)
{
    HelpNode *prevhelpnode = cur_help_node;
    extern int first_visible_help_pos, last_visible_help_pos;

    switch (inpch) {
      case ' ':
        first_visible_help_pos = last_visible_help_pos;
        show_help();
	break;
      case 'n':
	cur_help_node = cur_help_node->next;
	break;
      case 'p':
	cur_help_node = cur_help_node->prev;
	break;
      case 'b':
	xbeep();
	break;
      case 't':
	cur_help_node = topics_help_node;
	break;
      case '?':
	cur_help_node = help_help_node;
	break;
      case ESCAPE:
      case 'q':
	mode = prevmode;
	redraw();
	break;
      default:
	xbeep();
	break;
    }
    if (prevhelpnode != cur_help_node) {
        first_visible_help_pos = last_visible_help_pos = 0;
	show_help();
    }
}

static void
describe_help(int arg, const char *key, TextBuffer *buf)
{
    tbcat(buf, "' ' to page down through a node\n");
    tbcat(buf, "'n' to go to next help node\n");
    tbcat(buf, "'p' to go to previous node\n");
    tbcat(buf, "'b' to go back to last node\n");
    tbcat(buf, "'t' to go to topics list\n");
    tbcat(buf, "'?' to go to this help info\n");
    tbcat(buf, "'q' or ESC to go back to game\n");
}

/* Reading is usually pretty fast, so don't do anything here. */

void
announce_read_progress(void)
{
}

/* Announce the start of a time-consuming computation. */

void
announce_lengthy_process(const char *msg)
{
    n_seconds_elapsed(0);
    announcemsg = copy_string(msg);
    if (announcemsg) {
	printf("%s;", announcemsg);
	fflush(stdout);
	announcemsg = NULL;
	announced = TRUE;
    }
}

void
announce_progress(int percentdone)
{
    if (n_seconds_elapsed(2)) {
	printf(" %d%%,", percentdone);
	fflush(stdout);
	announced = TRUE;
    }
}

void
finish_lengthy_process(void)
{
    if (announced) {
	printf(" done.\n");
	announced = FALSE;
    }
}

/* This should be called before any sort of normal exit. */

void
exit_cconq(void)
{
    close_displays();
    printf("\n%s\n", get_scores(dside));
    exit(0);
}

void
flush_display_buffers(Side *side)
{
    if (active_display(side)) {
	refresh();
    }
}

/* A predicate that tests whether our display can validly be written to. */

int
active_display(Side *side)
{
    return (side == dside
	    && dside != NULL
	    && side_has_display(dside)
	    && active);
}

void
update_area_display(Side *side)
{
  if (active_display(side))
    refresh();
}

extern int suppress_update_cell_display;

/* Draw an individual detailed cell, as a row of one, on all maps. */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    if (active_display(side) && !suppress_update_cell_display) {
	draw_row(x, y, 1);
	if (flags & UPDATE_ALWAYS)
	  refresh();
    }
}

/* Curses updates efficiently, so don't need special routine. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    if (active_display(side)) {
	show_side_list();
	if (rightnow)
	  refresh();
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

/* Just change the part of the unit list relevant to the given unit. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side) && unit != NULL) {
	if (curunit == unit) {
	    show_closeup();
	    show_map();
	}
	show_side_list();
	show_list();
	if (rightnow)
	  refresh();
    }
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    update_unit_display(side, unit, rightnow);
}

void
update_turn_display(Side *side, int rightnow)
{
    if (active_display(side)) {
	show_game_date();
	if (rightnow)
	  refresh();
	if (g_turn() >= autofinish_start + autofinish_count) {
	    net_set_autofinish(dside, FALSE);
	}
    }
}

void
update_action_display(Side *side, int rightnow)
{
    if (active_display(side)) {
	show_side_list();
	if (rightnow)
	  refresh();
    }
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    Action *action;

    if (active_display(side)) {
    	action = (unit->act ? &(unit->act->nextaction) : NULL);
    	if (action == NULL)
    	  return;
    	DGprintf("%s %s result is %s\n",
		 unit_desig(unit), action_desig(action), hevtdefns[rslt].name);
	if (rightnow)
	  refresh();
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
    /* (should add to some sort of history display) */
}

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m,
		       int rightnow)
{
    /* (should flash at target) */
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m,
			 int rightnow)
{
    /* (should flash at target) */
}

/* Updates to clock need to be sure that display changes immediately. */

void
update_clock_display(Side *side, int rightnow)
{
    if (active_display(side)) {
	show_clock();
	show_cursor();
	if (rightnow)
	  refresh();
    }
}

void
update_message_display(Side *side, Side *sender, const char *str, int rightnow)
{
    if (active_display(side)) {
	notify(side, "%s", str);
	if (rightnow)
	  refresh();
    }
}

void
update_all_progress_displays(const char *str, int s)
{
}

void
update_everything(void)
{
    redraw();
}

void
action_point(Side *side, int x, int y)
{
    if (!active_display(dside))
      return;
    if (!inside_area(x, y))
      return;
    /* (should scroll over to make action point visible, but
        probably not change current point?) */
}

void
close_displays(void)
{
    if (!active_display(dside))
      return;
    clear();
    refresh();
    endwin();
    active = FALSE;
    DGprintf("Display \"%s\" closed.\n", dside->player->displayname);
}

/* An init error needs to have the command re-run. */

void
low_init_error(const char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    close_displays();
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    fprintf(stderr, "Saving the game...");
    write_entire_game_state(error_save_filename());
    fprintf(stderr, " done.\n");
    fflush(stderr);
    exit(1);
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    if (active_display(dside)) {
	notify(dside, "Warning: %s; continue? ", str);
	wait_for_char();
    } else {
	low_init_warning(str);
    }
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

/* Even a curses interface can do simple "movies". */

int
schedule_movie(Side *side, const char *movie, ...)
{
    int i, itype;
    va_list ap;

    if (numscheduled >= 10)
      return FALSE;
    if (side != dside)
      return FALSE;
    if (empty_string(movie))
      return FALSE;
    if (movies == NULL)
      movies = (struct a_movie *) xmalloc(10 * sizeof(struct a_movie));
    memset(&(movies[numscheduled]), 0, sizeof(struct a_movie));
    movies[numscheduled].type = movie;
    itype = movie_null;
    if (strcmp(movie, "miss") == 0)
      itype = movie_miss;
    else if (strcmp(movie, "hit") == 0)
      itype = movie_hit;
    else if (strcmp(movie, "death") == 0)
	  itype = movie_death;
    else if (strcmp(movie, "nuke") == 0)
      itype = movie_nuke;
    movies[numscheduled].itype = (enum movie_type)itype;

    va_start(ap, movie);
    for (i = 0; i < 5; ++i)
      movies[numscheduled].args[i] = va_arg(ap, int);
    va_end(ap);

    ++numscheduled;
    return TRUE;
}

void
play_movies(SideMask sidemask)
{
    int j, unitid, blasttype;
    Unit *unit;

    if (!side_in_set(dside, sidemask))
      return;
    for (j = 0; j < numscheduled; ++j) {
	switch (movies[j].itype) {
	  case movie_null:
	    break;
	  case movie_miss:
	  case movie_hit:
	  case movie_death:
	    unitid = movies[j].args[0];
	    unit = find_unit(unitid);
	    if (unit == NULL || !in_area(unit->x, unit->y))
	      continue;
	    blasttype = (movies[j].itype == movie_miss ? 0 :
			 (movies[j].itype == movie_hit ? 1 : 2));
	    draw_blast(unit->x, unit->y, 0);
	    break;
	  case movie_nuke:
	    draw_blast(unit->x, unit->y, 1);
	    break;
	  default:
	    break;
	}
    }
    numscheduled = 0;
}

void
make_generic_image_data(ImageFamily *imf)
{
}

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
}

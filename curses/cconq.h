/* Definitions for the curses interface to Xconq.
   Copyright (C) 1986-1989, 1991-1996, 1999, 2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifdef MAC
#include "curses.h"
#else
#include <curses.h>
#endif

#include "imf.h"
#include "ui.h"

/* Escape is the standard abort character. */

#define ESCAPE '\033'

/* Backspace is for fixing strings being entered. */

#define BACKSPACE '\010'

/* ^@ does a "transparent redraw" - can be done in middle of another
   command for instance. */

#define REDRAW '\000'

/* A "window" is merely a non-overlapping rectangle of character
   positions. */

struct ccwin {
  int x, y;
  int w, h;
};

/* The program can be in a number of different "modes", which affect
   both the appearance of the screen and the interpretation of input. */

enum mode {
    SURVEY,
    MOVE,
    HELP,
    MORE,
    PROMPT,
    PROMPTXY
  };

enum listsides {
    ourside,
    ourallies,
    allsides,
    numlistsides
  };

extern int use_both_chars;
extern int follow_action;
extern int curx, cury;
extern int tmpcurx, tmpcury;
extern Unit *curunit;
extern Unit *tmpcurunit;
extern enum mode mode;
extern enum mode prevmode;
extern int itertime;
extern char inpch;
extern int prefixarg;
extern char *ustr;
extern int *uvec;
extern int *bvec;
extern int nw, nh;
extern int lastx, lasty;
extern char *text1;
extern char *text2;
extern int reqstrbeg;
extern int reqstrend;
extern struct ccwin *helpwin;
extern HelpNode *cur_help_node;
extern HelpNode *help_help_node;
extern HelpNode *topics_help_node;
extern struct ccwin *datewin;
extern struct ccwin *sideswin;
extern struct ccwin *toplineswin;
extern struct ccwin *clockwin;
extern struct ccwin *mapwin;
extern struct ccwin *listwin;
extern struct ccwin *closeupwin;
extern struct ccwin *sidecloseupwin;
extern int mw, mh;
extern int infoh;
extern int vx, vy;
extern int vw, vh;
extern int vw2, vh2;
extern VP *mvp;
extern int lastvcx, lastvcy;
extern int lw, lh;
extern int sh;
extern int drawterrain;
extern int drawunits;
extern int drawnames;
extern int drawpeople;
extern int draw_cover;
extern enum listsides listsides;
extern int test;
extern int value;
extern int sorton;
extern int sortorder;
extern int active;
extern char *dashbuffer;

extern void init_display(void);
extern void init_interaction(void);
extern int wait_for_char(void);
extern void maybe_handle_input(int);
extern void interpret_input(void);
extern void do_dir_2(int dir, int n);
extern void move_survey(int x, int y);
extern void put_on_screen(int x, int y);

extern int ask_bool(const char *question, int dflt);
extern int ask_unit_type(const char *question, int *possibles);
extern int ask_terrain_type(const char *question, int *possibles);
extern int ask_position(const char *prompt, int *xp, int *yp);
extern void save_cur(void);
extern void restore_cur(void);
extern int ask_string(const char *prompt, const char *dflt, char **strp);
extern Side *ask_side(const char *prompt, Side *dflt);
extern int ask_unit(char *prompt, Unit **unitp);
extern int ask_direction(const char *prompt, int *dirp);
extern void make_current(Unit *unit);
extern void make_current_at(int x, int y);
extern void interpret_help(void);

extern void exit_cconq(void);

extern struct ccwin *create_window(int x, int y, int w, int h);

extern int in_middle(int x, int y);
extern void set_map_viewport(void);
extern void set_scroll(void);

extern void redraw(void);
extern void show_toplines(void);
extern void clear_toplines(void);
extern void show_closeup(void);
extern void show_map(void);
extern void draw_row(int x, int y, int len);
extern void show_game_date(void);
extern void show_clock(void);
extern void show_side_list(void);
extern void show_list(void);
extern void show_cursor(void);
extern void show_help(void);

extern void clear_window(struct ccwin *win);

extern void draw_blast(int x, int y, int r);
extern int draw_text(struct ccwin *win, int x, int y, const char *str);
extern int cur_at(struct ccwin *win, int x, int y);

extern void cycle_list_type(void);
extern void cycle_list_filter(void);
extern void cycle_list_order(void);

extern int auto_attack_on_move(Unit *, Unit *);

extern Unit *find_next_and_look(void);

extern void xbeep(void);

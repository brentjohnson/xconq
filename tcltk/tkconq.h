/* Definitions for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifdef __cplusplus
extern "C" {
#endif

/* For tcltk 8.4 source compatibility. */
#define USE_NON_CONST

#ifdef MAC
#include <tclMacCommonPch.h>
#include <tclMac.h>			/* Includes tcl.h. */
#include <tkMac.h>			/* Includes tk.h. */
#else
#include <tcl.h>
#include <tk.h>
#endif /* MAC */

#ifdef __cplusplus
}
#endif

#include <math.h>

#ifndef MAC
#include "xlibstuff.h"
#endif

#include "imf.h"		/* Has to come before ui.h under C++ */
#include "ui.h"
#include "tkimf.h"

/* Every map has a mode that governs the interpretation of input (mainly
   mouse input) activity for that map. */

enum mapmode {
  survey_mode,
  move_mode,
  no_tmp_mode,
  attack_mode,
  fire_mode,
  build_mode,
  no_scroll_mode,
  scroll_up_mode,
  scroll_right_mode,
  scroll_down_mode,
  scroll_left_mode,
#ifdef DESIGNERS
  cell_paint_mode,
  bord_paint_mode,
  conn_paint_mode,
  coat_paint_mode,
  unit_paint_mode,
  people_paint_mode,
  control_paint_mode,
  feature_paint_mode,
  material_paint_mode,
  elevation_paint_mode,
  temperature_paint_mode,
  clouds_paint_mode,
  winds_paint_mode,
  view_paint_mode,
#endif /* DESIGNERS */
  nummodes
};

/* List of gray stipples available. */

enum grayshade {
  black,
  darkgray,
  gray,
  lightgray,
  verylightgray,
  numgrays
};

enum movie_type {
  movie_null,
  movie_miss,
  movie_hit,
  movie_hit_short,
  movie_death,
  movie_nuke,
  movie_sound,
  movie_flash
};

struct a_movie {
  char *type;
  enum movie_type itype;
  int args[5];
};

/* The user interface substructure.  This is only allocated for sides with
   attached displays. */

typedef struct a_ui {
    int active;			/* True when the display is usable */

    Display *dpy;		/* The X display used by this side */
    int screen;			/* The X screen in use */

    short bonw;			/* true if display is black-on-white */
    int sxdown, sydown;
    int	cellx, celly;
    int cellxy_ok;		/* cellx & celly valid? */
    int beepcount;		/* number of times we've been beeped */
    /* Constructed during display init. */
    short fw, fh;		/* dimensions of text font (in pixels) */
    /* Working variables for the display. */
    XColor *bgcolor;		/* background color */
    XColor *fgcolor;		/* foreground (text) color */
    XColor *whitecolor;		/* actual white for this display */
    XColor *blackcolor;		/* actual black for this display */
    XColor *graycolor;		/* color for graying out (usually gray) */
    XColor *diffcolor;		/* unusual/distinct color (usually maroon) */
    XColor *badcolor;		/* color for non-OKness (usually red) */
    XColor *warncolor;		/* color for warnings (usually yellow) */
    XColor *goodcolor;
    XColor *window_color;
    XColor *mask_color;
    XColor *text_color;
    XColor *grid_color;
    XColor *unseen_color;
    XColor *contour_color;
    XColor *country_border_color;
    XColor *feature_color;
    XColor *frontline_color;
    XColor *meridian_color;
    XColor *shoreline_color;
    XColor *unit_name_color;
    XColor **material_color;
    XColor **cell_color;	/* the color of each terrain type */
    XColor **cell_shades[5];
    XColor *default_colors[MAXSIDES + 1];	/* side->default_color as XColor. */
    XColor *colors[MAXSIDES + 1][3];		/* side->colorscheme as XColors. */
    int numcolors[MAXSIDES + 1];
    XColor **unitcolors;
    XColor **numunitcolors;
    short dflt_color_embl_images;
    short pref_solid_color_terrain;
    short default_meridian_interval;

    /* Map-related slots. */
    struct a_map *maps;		/* Chain of maps that are up */

    /* Help-related slots. */
    short helpw, helph;
    struct a_helpnode *curhelpnode;
    struct a_helpnode **nodestack;
    int nodenumber, nodestackpos;

    int i_metric;
#ifdef DESIGNERS
    /* Design-related slots. */
    short curdesigntool;
    short curbrushradius;
    short curttype;
    short curbgttype;
    short curutype;
    short curusidenumber;
    short curpeoplenumber;
    short curcontrolnumber;
    short curfid;
    short curmtype;
    short curmamount;
    short curelevation;
    short curelevationcode;
    short curelevationvary;
    short curtemperature;
    short curcloudtype;
    short curcloudbottom;
    short curcloudheight;
    short curwinddir;
    short curwindforce;
    short curtview;
    short curuview;
#endif /* DESIGNERS */
    /* Arrays of image families for materials, terrain, and side emblems. 
    uimages is no longer part of the ui, but a global declared in ui.h. */
    ImageFamily **mimages;
    ImageFamily **timages;
    ImageFamily **eimages;
    short *eimages_loaded;
    /* Terrain drawing machinery */
    Image **best_timages[NUMPOWERS];
    Image *best_unseen_images[NUMPOWERS];
    Pixmap *terrpics[NUMPOWERS];
    Pixmap hexpics[NUMPOWERS], bhexpics[NUMPOWERS];
    Pixmap hexisopics[NUMPOWERS];
    /* Emblem drawing machinery */
    Pixmap emblempics[MAXSIDES + 1];
    Pixmap emblemmasks[MAXSIDES + 1];
    int embw[MAXSIDES + 1], embh[MAXSIDES + 1];

    ImageFamily *blastimages[7];
    Legend *legends;

    int *grok_p1;
    int *grok_p2;
    int grok_size;
    /* Random stuff */
    Pixmap grays[numgrays];
    Pixmap dots;
    Tk_Cursor cursors[nummodes];

    int numscheduled;
    struct a_movie movies[10];
    struct a_map *curmap;
} UI;

typedef struct a_map {
    int number;
    enum mapmode mode;		/* User's main interaction mode */
    enum mapmode prevmode;		/* User's previous interaction mode */
    enum mapmode tmp_mode;	/* Action possible at mouse position */
    int autoselect;
    int move_on_click;
    int show_all;
    int terrain_images;
    int terrain_patterns;
    int colorize_units;

    short follow_action;	/* scroll to where something has occured */

    short scrolled_to_unit;

    struct a_unit *curunit;	/* Unit under cursor */
    int curunit_id;
    void (*modalhandler)(Side *side, struct a_map *map, int cancelled);
    char inpch;			/* Keyboard char */
    short inpsx, inpsy;
    short inpx, inpy;
    short inptype;		/* Unit type clicked in unit type list */
    Unit *inpunit;
    short prefixarg;		/* numerical prefix argument */
    int argunitid;		/* is unit id, not ptr, for safety */
    short tmpt;
    struct a_side *argside;
    short *uvec;		/* vector of allowed unit types to input */
    char *ustr;			/* used in composing unit type hints */
    short *tvec;		/* vector of allowed terrain types to input */
    char *tstr;			/* used in composing terrain type hints */
    char prompt[BUFSIZE];	/* prompt for input */
    char answer[BUFSIZE];	/* string being typed in */
    int tmpint;

    int last_rawx[2], last_rawy[2];
    enum mapmode scroll_mode[2];
    int autoscroll_delaying[2];

    short anim_state;

    char *widget;
    char *worldw;
    struct a_map *next;
} Map;

/* Iteration over all of a side's map windows. */

#define for_all_maps(m)  \
  for ((m) = dside->ui->maps; (m) != NULL; (m) = (m)->next)

/* Declarations of globals. */

extern Tcl_Interp *interp;

extern int any_resources;

extern short *utype_indexes;
extern short *mtype_indexes;

/* Declarations of functions. */

#if (!defined(UNIX) && !defined(__CYGWIN32__) && !defined(__MINGW32__))
extern void XSetTile(Display *dpy, GC gc, Pixmap pm);
#endif

extern XColor *request_color(char *name);

extern void recenter(Map *map, int x, int y);
extern void put_on_screen(Map *map, int x, int y);
extern int in_middle(Map *map, int x, int y);

extern void redraw_map(Map *map);
extern VP *widget_vp(Map *map);
extern VP *worldw_vp(Map *map);

extern char *c_to_tcl_string(char **cstr);
extern void eval_tcl_cmd(char *fmt, ...);

extern void set_current_unit(Map *map, Unit *unit);
extern void update_cell(Map *map, int x, int y);
extern void handle_mouse_down(Map *map, int sx, int sy, int button);
extern void handle_mouse_up(Map *map, int sx, int sy, int button);
extern void handle_world_mouse_down(Map *map, int sx, int sy, int button);
extern void handle_world_mouse_up(Map *map, int sx, int sy, int button);
extern void paint_on_drag(Map *map, int h0, int v0, int mods);
extern void update_action_controls_info(Map *map);
extern void update_view_controls_info(void);
extern void draw_fire_line(Map *map, Unit *unit, Unit *unit2, int x2, int y2);
extern void draw_unit_blast(Map *map, Unit *unit, int btype, int duration);
extern void draw_cell_blast(Map *map, int x, int y, int btype, int duration);
extern void set_tool_cursor(Map *map, int which);

extern int launch_game(void);
extern void interpret_variants(void);

extern void init_x_signal_handlers(void);	/* Lives in xconq.c. */

extern void initial_ui_init(void);
extern void init_display(void);
extern void init_redraws(void);
extern void ui_mainloop(void);
extern void popup_game_dialog(void);
extern void beep(Side *side);
extern void get_preferences(void);
extern void create_map(void);
extern void popup_help(Side *side, HelpNode *node);

extern int ask_unit_type(Side *side, Map *map, char *prompt, int *possibles,
			 void (*handler)(Side *side, Map *map, int cancelled));
extern int grok_unit_type(Side *side, Map *map, int *typep);
extern void cancel_unit_type(Side *side, Map *map);
extern int ask_terrain_type(Side *side, Map *map, char *prompt, int *poss,
			    void (*handler)(Side *side, Map *map,
					    int cancelled));
extern int grok_terrain_type(Side *side, Map *map, int *typep);
extern void ask_position(Side *side, Map *map, char *prompt,
			 void (*handler)(Side *side, Map *map, int cancelled));
extern int grok_position(Side *side, Map *map, int *xp, int *yp, Unit **unitp);
extern void ask_bool(Side *side, Map *map, char *question, int dflt,
		     void (*handler)(Side *side, Map *map, int cancelled));
extern int grok_bool(Side *side, Map *map);
extern void ask_string(Side *side, Map *map, char *prompt, char *dflt,
		       void (*handler)(Side *side, Map *map, int cancelled));
extern int grok_string(Side *side, Map *map, char **strp);
extern void ask_side(Side *side, Map *map, char *prompt, Side *dfltside,
		     void (*handler)(Side *side, Map *map, int cancelled));
extern int grok_side(Side *side, Map *map, Side **side2p);

extern void exit_xconq(Side *side);

extern void set_show_all(Map *map, int value);

extern void update_contour_intervals(void);

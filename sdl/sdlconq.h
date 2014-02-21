/* Definitions for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "SDL.h"

#if (0)
#include "paragui.h"
#include "pgapplication.h"
#include "pgbutton.h"
#include "pgwidgetlist.h"
#include "pglabel.h"
#include "pgwindow.h"
#include "pgmaskedit.h"
#include "pgscrollbar.h"
#include "pgprogressbar.h"
#include "pgradiobutton.h"
#include "pgthemewidget.h"
#include "pgcheckbutton.h"
#include "pgslider.h"
#include "pglistbox.h"
#include "pgcolumnitem.h"
#include "pgdropdown.h"
#include "pgeventobject.h"
#include "pgpopupmenu.h"
#include "pgspinnerbox.h"
#include "pglog.h"
#include "pgmenubar.h"
#endif

#include "imf.h"
#include "ui.h"

/* Spatial characteristics of UI elements. */

#define BOTTOM_PANEL_X_MARGIN		    5
#define BOTTOM_PANEL_Y_MARGIN		    5
#define MINIMAP_WIDTH_DEFAULT		    200
#define MINIMAP_BORDER_SIZE		    8
#define UNIT_ACTION_PANEL_BORDER_SIZE	    8
#define SIDE_RESEARCH_PANEL_BORDER_SIZE	    8

#ifdef MAC

#define exit mac_exit

#endif /* MAC */

/* All the types of possible panels. */

enum paneltype {
  empty_panel,
  mouseover_panel,
  unit_info_panel,
  bottom_panel,
  last_panel_type
};

/* Every map has a mode that governs the interpretation of input (mainly
   mouse input) activity for that map. */

enum mapmode {
  no_mode,
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

/* Named color lookup table entry. */

struct RGBColorTableEntry {
    char *name;
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

/* Lookup table of standard color names. */

extern RGBColorTableEntry stdcolors [];

/* List of gray stipples available. */

enum grayshade {
  black,
  darkgray,
  gray,
  lightgray,
  numgrays
};

/* Available movies. */

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

} UI;

#define sdl_image(img) ((SDL_Surface *) (img)->hook)

typedef struct a_cursor {
  struct SDL_Surface *surf;
  int hotx, hoty;
} SDLCursor;

/* The UI structure includes all information that is global to the
   interface.  Since the SDL interface only supports one player
   interface per program instance, these could all be global
   variables, but this approach accommodates possible future
   changes. */

struct a_real_ui {

    struct a_screen *screens;
    struct a_screen *curscreen;

    HelpNode *curhelpnode;

    int beepcount;

    /* Arrays of image families for materials, terrain, and side emblems. 
    uimages is no longer part of the ui, but a global declared in ui.h. */
    ImageFamily **mimages;
    ImageFamily **timages;
    ImageFamily **eimages;
    short *eimages_loaded;
    /* Terrain drawing machinery */
    Image **best_timages[NUMPOWERS];
    Image **tcolors;
    int embw[MAXSIDES + 1], embh[MAXSIDES + 1];

    ImageFamily *blastimages[7];
    Legend *legends;

    int curfid;

    SDLCursor *cursors[nummodes];

    SDL_Surface *grids[NUMPOWERS];
    int no_grid_available[NUMPOWERS];

    SDL_Surface *country_boundaries[NUMPOWERS];
    int no_country_boundary_available[NUMPOWERS];
};

/* A screen more-or-less corresponds to a physical display.  We assume
   a rectangular area that has a single SDL surface for it.  Maps,
   control panels, etc, carve up the space on each screen.  Note that
   things like popup dialogs must be associated with particular
   screens, and cannot be independent windows. */

typedef struct a_screen {

    struct a_map *map;
    struct a_panel *panels;

    struct a_map *minimap;

    int cursorx, cursory;
    int num_updates;
    SDL_Rect updates[1000];
    SDL_Surface *surf;

    SDLCursor *cursor;

    enum mapmode mode;		/* User's main interaction mode */
    enum mapmode tmp_mode;	/* Action possible at mouse position */
    int autoselect;
    int move_on_click;
    int show_all;
    int terrain_images;
    int colorize_units;

    int active;

    short follow_action;	/* scroll to where something has occured */

    short scrolled_to_unit;

    Unit *curunit;		/* Unit under cursor */
    int curunit_id;		/* Id of unit under cursor */
    Unit **selunits;		/* Vector of all selected units */
    void (*modalhandler)(struct a_screen *screen, int cancelled);
    char inpch;			/* Keyboard char */
    short inpsx, inpsy;
    short inpx, inpy;
    short inptype;		/* Unit, terrain, etc... type argument. */
    Unit *inpunit;
    short prefixarg;		/* numerical prefix argument */
    int argunitid;		/* is unit id, not ptr, for safety */
    short tmpt;
    Side *argside;
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

    int mouseover_panel_locked;
    int unit_action_panel_visible;
    int side_research_panel_visible;

    /* At present we can only have one screen per UI. */
    struct a_screen *next;

} Screen;

/* A map is a visual representation of an area.  It may be overhead
   or isometric.  The usual screen will have two maps, a main one
   for playing on, and a mini-map that shows the whole area. */

typedef struct a_map {
    VP *vp;			/* working viewport used for incr updates */
    VP *main_vp;		/* real viewport for the map */
    struct a_screen *screen;	/* backpointer to the screen object */
    int sx, sy;			/* map position on the screen */
    int is_world;
} Map;

/* A panel is a marginal area that includes various kinds of
   information, depending on its type. */

typedef struct a_panel {
    int type;
    int x, y;
    int w, h;
    int full_w, full_h;
    int sd;
    int overlay;
    char *label;
    int numbuttons;
    struct a_button *buttons[100];
    struct a_panel *in;
    struct a_panel *out;
    struct a_screen *screen;
    int redraw_pending;
} Panel;

/* SDL UI Protobutton and Button */

struct a_protobutton {
    char *label;
    char help[BUFSIZE];
    ImageFamily *picture;
    void (*click_fn)(struct a_button *);
    int data;
};

typedef struct a_button {
    int x, y;
    int w, h;
    int visible;
    int draw_bg;
    Uint32 bg;
    char *label;
    ImageFamily *picture;
    SDL_Surface *pic_surface;
    char *help;
    void (*click_fn)(struct a_button *);
    int data;
    Panel *panel;
} SDLButton;

/* Things related to the unit action buttons. */

extern int G_uact_button_offset;
extern struct a_protobutton G_protobuttons [];
extern int G_num_protobuttons;

/* Iteration over all of a side's screens. */
/* (At the moment there is only one.) */

#define for_all_screens(s)  \
  for ((s) = ui->screens; (s) != NULL; (s) = (s)->next)

extern SDL_Surface *mscreen;

extern struct a_real_ui *ui;

extern SDL_Surface *small_font;

extern Screen *sscreen;

extern int use_cursors;

extern char *default_cursor_name;

extern SDLCursor *generic_cursor;

extern int fullscreen;
extern int mainw;
extern int mainh;

#ifdef MAC

/* Mac-specific stuff. */

extern WindowPtr sdl_console;
extern WindowPtr sdl_window;

extern void mac_exit(int status);
extern int handle_macos_cursor(void);

#endif

/* Declarations of globally visible functions. */

/* sdlcmd.cc */

extern void maybe_ask_construct_location(Screen *screen, Unit *unit, int u2);

/* sdlmain.cc */

extern void initial_ui_init(void);

extern void ui_mainloop(void);

extern void exit_xconq(void);
extern void get_preferences(void);

extern int point_in_map(Map *map, int rawx, int rawy);
extern void autoscroll(Screen *screen, int rawx, int rawy);
extern void set_current_unit(Screen *screen, Unit *unit);

extern int ask_unit_type(Screen *screen, char *prompt, int *possibles,
			 void (*handler)(Screen *screen, int cancelled));
extern int grok_unit_type(Screen *screen, int *typep);
extern void cancel_unit_type(Screen *screen);
extern int ask_terrain_type(Screen *screen, char *prompt, int *poss,
			    void (*handler)(Screen *screen, int cancelled));
extern int grok_terrain_type(Screen *screen, int *typep);
extern void ask_position(Screen *screen, char *prompt,
			 void (*handler)(Screen *screen, int cancelled));
extern int grok_position(Screen *screen, int *xp, int *yp, Unit **unitp);
extern void ask_bool(Screen *screen, char *question, int dflt,
		     void (*handler)(Screen *screen, int cancelled));
extern int grok_bool(Screen *screen);
extern void ask_string(Screen *screen, char *prompt, char *dflt,
		       void (*handler)(Screen *screen, int cancelled));
extern int grok_string(Screen *screen, char **strp);
extern void ask_side(Screen *screen, char *prompt, Side *dfltside,
		     void (*handler)(Screen *screen, int cancelled));
extern int grok_side(Screen *screen, Side **side2p);

extern void set_tool_cursor(Screen *screen);

extern void update_cursor(Screen *screen);

extern void popup_help(Side *side, HelpNode *node);

extern void beep(void);

extern void popup_game_dialog(void);

extern int launch_game(void);

extern void interpret_variants(void);

extern void set_show_all(Screen *screen, int flag);

extern int screen_oneliner(Screen *screen);

/* sdlinit.cc */

extern void init_display(void);
extern SDLCursor *get_cursor(char *name, int hotx, int hoty);

/* sdlimf.cc */

extern ImageFamily *sdl_interp_imf(ImageFamily *imf, Image *img, int force);

/* sdlscreen.cc */

extern int alpha_blend_fill_rect(SDL_Surface *surf, SDL_Rect *rect,
				 Uint8 r, Uint8 g, Uint8 b, Uint8 a);

extern void reset_screen_input(Screen *screen);
extern void open_screen(void);

extern void add_update(Screen *screen, int sx, int sy, int sw, int sh);
extern void update_screen(Screen *screen, int rightnow);

extern void reset_panel_buttons(Panel *panel);
extern void draw_panel(Screen *screen, Panel *panel);
extern void redraw_screen(Screen *screen);

extern void update_cell(Screen *screen, int x, int y);
extern void update_unit_info(Screen *screen);

extern Uint32 random_color(SDL_Surface *surf);
extern void draw_rect(SDL_Surface *surf,
		      int sx, int sy, int sw, int sh, Uint32 col);
extern void draw_line(SDL_Surface *surf, int sx1, int sy1, int sx2, int sy2,
		      Uint32 col);
extern void draw_string(SDL_Surface *surf, int x0, int y0, char *str);
extern int SDLIntersectRect(SDL_Rect *A, SDL_Rect *B, SDL_Rect *intersection);

extern void draw_button(Screen *screen, Panel *panel, SDLButton *button);

/* sdlmap.cc */

extern Map *create_map(int is_world);

extern void draw_map(Map *map);

extern void recenter(Map *map, int x, int y);
extern void put_on_screen(Map *map, int x, int y);
extern int in_middle(Map *map, int x, int y);

/* sdluact.cc */

extern void add_delay_button(Screen *screen, Unit **units, int uvecsz);
extern void add_skip_button(Screen *screen, Unit **units, int uvecsz);
extern void add_sleep_button(Screen *screen, Unit **units, int uvecsz);
extern void add_wake_button(Screen *screen, Unit **units, int uvecsz);
extern void add_research_buttons(Screen *screen, Unit **units, int uvecsz);
extern void add_construct_buttons(Screen *screen, Unit **units, int uvecsz);
extern void add_develop_buttons(Screen *screen, Unit **units, int uvecsz);
extern void add_change_type_buttons(Screen *screen, Unit **units, int uvecsz);

/* sdlcltbl.cc */

extern int find_sdl_color_by_name(char *name, RGBColorTableEntry *table, 
				  SDL_Color *color);
extern int find_rgb_triplet_by_name(char *name, RGBColorTableEntry *table,
				    Uint8 *r, Uint8 *g, Uint8 *b);

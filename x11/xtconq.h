/* Definitions for the X11 Xt interface to Xconq.
   Copyright (C) 1987-1989, 1991-1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Default color of text and icons - 0 is for white on black, 1 is for
   black on white.  Should be set appropriately for the most common
   monochrome display (color displays always do white on black).  This
   is also settable by the player, so the default is just for
   convenience of the majority. */

#define BLACKONWHITE 1

/* The default fonts can be altered by users, so these are just hints. */

#define TEXTFONT "fixed"

/* Some X11 servers die if too much is written between output flushes. */

/* #define STUPIDFLUSH */

#ifdef __cplusplus
extern "C" {
#endif
#if (!defined (__CYGWIN32__) && !defined (__MINGW32__))
#include <X11/Xos.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#ifdef __cplusplus
}
#endif

#include <math.h>

#include "imf.h"
#include "ximf.h"
#include "ui.h"
#include "print.h"

enum grayshade {
  black,
  gray,
  darkgray,
  numgrays
};

/* This is the name of a family of programs, so argv[0] inadequate. */

#define PROGRAMNAME "xtconq"
#define PROGRAMCLASSNAME "Xtconq"

#ifdef __cplusplus
extern "C" {
#endif
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/SmeBSB.h>
#ifdef __cplusplus
}
#endif

#include <math.h>

/* Types of controls. */

enum controltypeenum {
    LOOK = 0,
    MOVE,
    UNIT_MOVE,
    UNIT_SHOOT,
    UNIT_BUILD,
    SHOW_TERRAIN,
    SHOW_GRID,
    SHOW_UNITS,
    SHOW_NAMES,
    SHOW_FEATURE_NAMES,
    SHOW_FEATURE_BOUNDARIES,
    SHOW_NUMBERS,
    SHOW_PEOPLE,
    SHOW_CONTROL,
    SHOW_ELEV,
    SHOW_TEMP,
    SHOW_CLOUDS,
    SHOW_WINDS,
    SHOW_COVER,
    SHOW_ALL,
    SHOW_MERIDIANS,
    SHOW_MORE,
    COLR_UNITS,
    COLR_TERR,
    COLR_EMBL,
    MONO_REVERSE,
    ZOOM_OUT,
    ZOOM_IN,
    POPUP_HELP,
    numcontrols
};

/* Tools govern the interpretation of mouse clicks in a map view. */

enum tooltypes {
  looktool,
  movetool,
  unitmovetool,
  unitshoottool,
  unitbuildtool,
#ifdef DESIGNERS
  cellpainttool,
  bordpainttool,
  connpainttool,
  unitaddtool,
  peoplepainttool,
  featurepainttool,
#endif /* DESIGNERS */
  numtools
};

enum movie_type {
  movie_null,
  movie_miss,
  movie_hit,
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

#define N_COMMAND  3
#define N_CHOICE   1
#define N_BUTTON   (N_COMMAND+N_CHOICE)
#define N_TOGGLE   6
#define N_DIALOG  12
#define N_DIMEN_D 10 
#define N_WIDGET (N_BUTTON+N_TOGGLE+N_DIALOG)

/* The user interface substructure.  This is only allocated for sides with
   attached displays. */

typedef struct a_ui {
    Display *dpy;		/* The X display used by this side */
    int active;			/* True when the display is usable */
    int screen;			/* The X screen in use */
    Widget shell;		/* Main shell for this side's displays */
    Window rootwin;		/* The root window */
    Pixel foreground;
    Pixel background;
    String geospec;
    int mpTime;			/* # of milliseconds to pause between mplayer moves */
    Atom kill_atom;
    short bonw;			/* true if display is black-on-white */
    short follow_action;	/* scroll to where something has occured */
    int sxdown, sydown;
    int	cellx, celly;
    Boolean cellxy_ok;		/* cellx & celly valid? */
    struct a_map *mapdown;
    int beepcount;		/* number of times we've been beeped */
    /* Constructed during display init. */
    short monochrome;		/* obvious */
    short fw, fh;		/* dimensions of text font (in pixels) */
    /* Working variables for the display. */
    long bgcolor;		/* background color */
    long fgcolor;		/* foreground (text) color */
    long whitecolor;		/* actual white for this display */
    long blackcolor;		/* actual black for this display */
    long graycolor;		/* color for graying out (usually gray) */
    long diffcolor;		/* unusual/distinct color (usually maroon) */
    long goodcolor;		/* color for OKness (usually green) */
    long badcolor;		/* color for non-OKness (usually red) */
    long gridcolor;
    long unseencolor;
    long contour_color;
    long country_border_color;
    long feature_color;
    long frontline_color;
    long meridian_color;
    long shoreline_color;
    long unit_name_color;
    long *cellcolor;	/* the color of each terrain type */
#if 0
    long *cellfgcolor; /* the "fg" color of each terrain type */
#endif
    long colors[MAXSIDES][3];
    int numcolors[MAXSIDES];
    long *unitcolors;
    long *numunitcolors;
    short dflt_color_unit_images;
    short dflt_color_terr_images;
    short dflt_color_embl_images;
    short pref_solid_color_terrain;
    short default_meridian_interval;
    GC gc;			/* a tmp graphics context for this display */
    GC textgc;			/* foreground on background text */
    GC ltextgc;			/* foreground on background text */
    GC terrgc;			/* terrain display gc */
    GC unitgc;			/* unit display gc */
    GC emblgc;			/* emblem display gc */
    GC bdrygc;			/* country/feature boundary gc */
    XFontStruct *textfont;	/* Font for text display */
    /* Map-related slots. */
    struct a_map *maps;		/* Chain of maps that are up */
    /* Help-related slots. */
    Widget help_shell;
    Widget help_form;
    Widget help_topicPort;
    Widget help_topicList;
    Widget help_title;
    Widget help_text;
    Widget help_button_box;
    Widget help_next;
    Widget help_prev;
    Widget help_back;
    Widget help_close;
    short helpw, helph;
    struct a_helpnode *curhelpnode;
    struct a_helpnode **nodestack;
    int nodenumber, nodestackpos;
    /* Printing-related slots. */
    Widget print_shell;
    Widget print_help_shell;
    Widget print_cmds[N_WIDGET];
    PrintParameters *ps_pp;
    int choi[N_CHOICE];
    int flag[N_TOGGLE];
    double parm[N_DIALOG];
    int i_metric;
#ifdef DESIGNERS
    /* Design-related slots. */
    Widget design_shell;
    Widget design;
    Widget normal_button;
    Widget normal_label;
    Widget terrain_button;
    Widget terrain_label;
    Widget unit_button;
    Widget unit_label;
    Widget people_button;
    Widget people_label;
    Widget feature_button;
    Widget feature_label;
    Widget brush_radius_label;
    short curdesigntool;
    short curbrushradius;
    short curttype;
    short curbgttype;
    short curutype;
    short curusidenumber;
    short cursidenumber;
    short curfid;
#endif /* DESIGNERS */
    /* Arrays of image families for terrain and side emblems. 
    uimages is no longer part of the ui, but a global declared in ui.h. */
    ImageFamily **timages;
    ImageFamily **eimages;
    short *eimages_loaded;
    /* Terrain drawing machinery */
    enum whattouse *usewhat[NUMPOWERS]; /* for each terrain */
    /* (should use as cache of image in image family - calc as needed) */
    Pixmap *terrpics[NUMPOWERS]; /* for each terrain */
    char *terrchars[NUMPOWERS]; /* for each terrain */
    XFontStruct **terrfonts[NUMPOWERS]; /* for each terrain */
    Pixmap hexpics[NUMPOWERS], bhexpics[NUMPOWERS];
    Pixmap hexchars[NUMPOWERS], bhexchars[NUMPOWERS];
    XFontStruct *hexfonts[NUMPOWERS], *bhexfonts[NUMPOWERS];
    /* Unit drawing machinery */
    Pixmap *unitpics[NUMPOWERS]; /* for each utype */
    Pixmap *unitmasks[NUMPOWERS]; /* for each utype */
    char *unitchars[NUMPOWERS]; /* for each utype */
    XFontStruct **unitfonts[NUMPOWERS]; /* for each utype */
    XFontStruct *unitfont;	/* font for unit characters */
    /* Emblem drawing machinery */
    Pixmap emblempics[MAXSIDES];
    Pixmap emblemmasks[MAXSIDES];
    int embw[MAXSIDES], embh[MAXSIDES];
    /* Map legend drawing machinery */
    XFontStruct **ulegendfonts[NUMPOWERS]; /* for each utype */
    XFontStruct *flegendfonts[6];
    Font flegendfids[6];
    Legend *legends;
    /* Side closeup list */
    struct a_side_closeup *sidecloseuplist;
    /* Unit list */
    struct a_unit_list *unitlistlist;
    /* Unit closeup list */
    struct a_unit_closeup *unitcloseuplist;
    /* Unit closeup summary */
    struct a_closeup_summary *closeupsummary;
    /* Orders interface */
    Widget orders_shell;
    Widget orders_label;
    Widget orders_form;
    Widget orders_radio;
    Widget orders_help_shell;
    StandingOrder *sorder_edit;
    char *sorder_types_edit;
    Task *sorder_task_edit;
    struct a_order_interface *ordi_edit;
    struct a_order_interface *orderlist;
    int *grok_p1;
    int *grok_p2;
    int grok_size;
    /* Random stuff */
    Pixmap bombpics[4];
    Pixmap hitpics[3];
    Pixmap controlpics[numcontrols];
    Pixmap toolcursors[numtools];
    Pixmap *unitcursors_foo; /* for each utype */
    Pixmap grays[numgrays];
    Pixmap dots;
    int numscheduled;
    struct a_movie movies[10];
    short told_outcome;
    struct a_map *curmap;
} UI;

/* A closer look at a side. */

typedef struct a_side_closeup {
    struct a_map *map;
    struct a_side *side;
    Widget shell, info, *filter, list_units;
    struct a_side_closeup *next;
} SideCloseup;

/* A list of units. */

typedef struct a_unit_list {
    struct a_map *map;
    int number;
    Unit **units;
    char **labels;
    Widget shell, label, list, close;
    struct a_unit_list *next;
} UnitList;

/* A closer look at a unit. */

typedef struct a_unit_closeup {
    struct a_map *map;
    struct a_unit *unit;
    Widget shell, info;
    struct a_unit_closeup *next;
} UnitCloseup;

/* List of unit closeups */

typedef struct a_closeup_summary {
    int number;
    UnitCloseup **unitcloseups;
    char **labels;
    Widget shell, label, list;
} CloseupSummary;
 
/* Orders interface. */

typedef struct a_order_interface {
    StandingOrder *sorder;
    Widget form, toggle, types, etype, eparms, task, tparms;
    Pixel form_bg, form_fg;
    struct a_order_interface *next;
} OrderInterface;

/* Iteration over all of a side's unit closeups. */

#define for_all_unit_closeups(c,s)  \
  for ((c) = (s)->ui->unitcloseuplist; (c) != NULL; (c) = (c)->next)

/* Iteration over all of a side's unit lists. */

#define for_all_unit_lists(l,s)  \
  for ((l) = (s)->ui->unitlistlist; (l) != NULL; (l) = (l)->next)

/* Iteration over all of a side's side closeups. */

#define for_all_side_closeups(c,s)  \
  for ((c) = (s)->ui->sidecloseuplist; (c) != NULL; (c) = (c)->next)

/* Each side can open up any number and shape of maps. */

/* All the maps will share colors and bitmaps though. */

typedef struct a_map {
    /* Widgets that make up the map. */
    Widget mainwidget;
    Widget leftpane;		/* lefthand area of window */
    Widget leftform;		/*  */
    Widget rightpane;		/* righthand area of window */
    Widget infoform;		/* parts of leftform */
    Widget controlform;		/* parts of leftform */
    Widget *controls;		/* Array of controls */
    Widget mapform;		/* parts of leftform */
    Widget sideform;		/* parts of rightform */
    Widget info;		/* Info about current pos/unit */
    Widget porthole;
    Widget portlabel;
    Widget gamedate;		/* Overall game info */
    Widget gameclock;		/* overall game info */
    Widget msgarea;		/* miscellaneous info related to UI */
    Widget sides;		/* List of sides */
    Widget history;		/* Place for notices/warnings */
    Widget promptlabel;
    Widget listview;
    Widget listform;
    Widget *list_buttons;	/* numutypes+1 of these */
    Widget pannerbox;
    Widget panner;
    Widget help_pop;
    Widget ctrlpanel_pop;	/* View control popup */
    Widget ctrlpanel_shell;	/* View control popup's shell */
    Widget ctrlpanel_form;
    Widget *ctrlpanel_buttons;	/* Array of buttons */
    /* Raw windows that we draw into directly. */
    Window infowin;		/* info about a unit and/or cell */
    Pixmap viewwin;		/* actual graphical display of an area */
    Window sideswin;		/* list of sides */
    /* panner pixmap */
    Pixmap panner_pix;
    /* How to draw the map. */
    short seeall;		/* True if viewing world data directly */
    short use_color_terr_images;
    short use_color_embl_images;
    short fullpanel;
    short follow_action;
    /* Slots used for internal display calculations. */
    VP *vp;			/* This map's generic view parameters */
    short totalw, totalh;	/* Total size of window in pixels */
    short leftfrac;
    short leftw;
    short pxw, pxh;		/* Size of map subwindow in pixels */
    short toph;
    short infoh;
    short list1frac;
    short list1w, list1h;
    short list2w, list2h;
    short panw, panh;
    short sidespacing;		/* Vertical space for each side in side list */
    short *last_num_in_play; /* for each utype */
    short *last_num_incomplete; /* for each utype */
    /* Interaction controls. */
    short curtool;		/* Interpretation of left-mouse clicks */
    short curx, cury;		/* Current spot being looked at */
    struct a_unit *curunit;	/* Unit under cursor */
    short savedcurx, savedcury;	/* Current spot being looked at (saved) */
    struct a_unit *savedcurunit;/* Unit under cursor (saved) */
    void (*modalhandler)(Side *side, struct a_map *map, int cancelled);
    char inpch;			/* Keyboard char */
    short inptype;		/* Unit type clicked in unit type list */
    short prefixarg;		/* numerical prefix argument */
    int argunitid;		/* is unit id, not ptr, for safety */
    short tmpt;
    struct a_side *argside;
    short *uvec; /* vector of allowed unit types to input, size numutypes */
    char *ustr;	/* used in composing unit type hints, size numutypes+1 */
    short *tvec; /* vector of allowed terrain types to input, size numttypes */
    char *tstr;	/* used in composing terrain type hints, size numttypes+1 */
    char prompt[BUFSIZE];	/* prompt for input */
    char answer[BUFSIZE];	/* string being typed in */
    int tmpint;
    short frombutton;		/* true if command was issued from button */
    short anim_state;
    /* Link to the next map. */
    struct a_map *next;
} Map;

/* Iteration over all of a side's map windows. */

#define for_all_maps(s,m)  \
  for ((m) = (s)->ui->maps; (m) != NULL; (m) = (m)->next)

#define num_unit_colors(s,s2,u)  \
  (((s)->ui->numunitcolors)[numutypes * (s2) + (u)])

#define unit_color(s,s2,u,n)  \
  (((s)->ui->numunitcolors)[3 * (numutypes * (s2) + (u)) + (n)])

/* Values shared by all displays and all sides. */

extern Widget thistoplevel;
extern XtAppContext thisapp;
extern int nargs;
extern Arg tmpargs[];

/* Declarations of globally visible functions. */

extern int ask_unit_type(Side *side, Map *map, char *prompt, int *poss,
			 void (*handler)(Side *side, Map *map, int cancel));
extern int ask_terrain_type(Side *side, Map *map, char *prompt, int *poss,
			    void (*handler)(Side *side, Map *map, int cancel));
extern void ask_side(Side *side, Map *map, char *prompt, Side *dflt,
		     void (*handler)(Side *side, Map *map, int cancel));
extern void ask_position(Side *side, Map *map, char *prompt,
			 void (*handler)(Side *side, Map *map, int cancel));
extern void ask_bool(Side *side, Map *map, char *prompt, int dflt,
		     void (*handler)(Side *side, Map *map, int cancel));
extern void ask_string(Side *side, Map *map, char *prompt, char *dflt,
		       void (*handler)(Side *side, Map *map, int cancel));

extern int grok_unit_type(Side *side, Map *map, int *typep);
extern int grok_terrain_type(Side *side, Map *map, int *typep);
extern int grok_side(Side *side, Map *map, Side **side2p);
extern int grok_position(Side *side, Map *map, int *xp, int *yp, Unit **unitp);
extern int grok_bool(Side *side, Map *map);
extern int grok_string(Side *side, Map *map, char **strp);

extern void xform(Side *side, Map *map, int x, int y, int *sxp, int *syp);
extern void x_xform_unit(Side *side, Map *map, Unit *unit,
			 int *sxp, int *syp, int *swp, int *shp);
extern void x_xform_unit_self(Side *side, Map *map, Unit *unit,
			      int *sxp, int *syp, int *swp, int *shp);
extern void x_xform_occupant(Side *side, Map *map, Unit *transport, Unit *unit,
			     int sx, int sy, int sw, int sh,
			     int *sxp, int *syp, int *swp, int *shp);
extern int x_nearest_cell(Side *side, Map *map, int sx, int sy,
			  int *xp, int *yp);
extern int x_nearest_boundary(Side *side, Map *map, int sx, int sy,
			      int *xp, int *yp, int *dirp);
extern int x_nearest_unit(Side *side, Map *map, int sx, int sy, Unit **unitp);

extern void init_x_signal_handlers(void);

extern void popup_game_dialog(void);

extern void check_player_displays(void);
extern void init_redraws(void);
extern void init_display(Side *side);
extern void set_colors(Side *side);
extern long request_color(Side *side, char *name);

extern XFontStruct *open_font(Side *side, char *name, char *xdefault,
			      XFontStruct *altfont, char *alttype, Font *fid);
extern Cursor make_cursor(Display *dpy, Window win,
			  char *cursbits, char *maskbits,
			  unsigned long fg, unsigned long bg,
			  unsigned int x, unsigned int y);

extern void reset_color_state(Side *side);
extern void reset_window_colors(Side *side, Window win);

extern void set_current_unit(Side *side, Map *map, Unit *unit);
extern void set_current_xy(Side *side, Map *map, int x, int y);
extern void clear_current(Side *side, Map *map);
extern void save_cur(Side *side, Map *map);
extern void restore_cur(Side *side, Map *map);

extern Map *create_map(Side *side, int power, char *geospec);

extern void add_map_actions(void);

extern void set_tool_cursor(Side *side, Map *map);

extern void set_map_power(Side *side, Map *map, int power);
extern void x_center_on_focus(Side *side, Map *map);

extern void handle_key_event(Side *side, Map *map, XEvent *evt);

extern int find_side_and_map(Widget w, Side **sidep, Map **mapp);
extern int find_side_and_map_via_control(Widget w, Side **sidep, Map **mapp);
extern int find_side_and_map_via_listform(Widget w, Side **sidep, Map **mapp);
extern int find_side_and_map_via_mapform(Widget w, Side **sidep, Map **mapp);
extern int find_side_and_map_via_rightform(Widget w, Side **sidep, Map **mapp);
extern int find_side_and_map_via_a_toplevel(Widget w, Side **sidep,
					    Map **mapp);
extern int find_side_and_map_via_porthole(Widget w, Side **sidep, Map **mapp);
extern int find_side_via_widget(Widget w, Side **sidep);

extern void update_controls(Side *side, Map *map);

extern void draw_all_maps(Side *side);

extern void draw_map(Side *side, Map *map);
extern void draw_map_view(Side *side, Map *map);
extern void destroy_map(Side *side, Map *map);


extern void draw_row(Side *side, Map *map, int x0, int y0, int len,
		     int clearit);
extern void draw_unit_image(ImageFamily *imf, Side *side, Window win,
			    int sx, int sy, int sw, int sh,
			    int s2, int fg, int bg, int mod);
extern void draw_side_emblem(Side *side, Window win, int ex, int ey,
			     int ew, int eh, int s2, int style);
extern void draw_current(Side *side, Map *map);
extern void erase_current(Side *side, Map *map, int x, int y, Unit *unit);

extern void draw_prompt(Side *side, Map *map);
extern void clear_prompt(Side *side, Map *map);
extern void draw_map_info(Side *side, Map *map);
extern void draw_game_state(Side *side, Map *map);
extern void draw_game_clocks(Side *side, Map *map);
extern void draw_map_sides(Side *side, Map *map);
extern void draw_side_info(Side *side, Map *map, Side *side2);
extern void draw_side_progress(Side *side, Map *map, Side *side2);

extern void put_on_screen(Side *side, Map *map, int x, int y);
extern int in_middle(Side *side, Map *map, int x, int y);

extern void move_the_selected_unit(Side *side, Map *map, Unit *unit,
				   int sx, int sy);

extern void create_help(Side *side);
extern void popup_help(Side *side);
extern void popdown_help(Side *side);

extern void recenter(Side *side, Map *map, int x, int y);

extern void draw_text(Side *side, Window win, int x, int y, char *str,
		      int color);
extern void draw_fg_text(Side *side, Window win, int x, int y, char *str);

extern void textw_printf(const Widget w, const char *fmt, ...);

extern int font_width(XFontStruct *font);
extern int font_height(XFontStruct *font);

extern void close_display(Side *side);

extern void exit_xconq(Side *side);

extern void notify_all(char *fmt, ...);
extern void low_notify(Side *side, char *str);
extern void redraw(Side *side);
extern void flush_output(Side *side);
extern void beep(Side *side);

extern void flush_input(Side *side);

extern void build_name(char *name, char *first, char *second);

extern void draw_view_in_panner(Side *side, Map *map);

extern void set_message_area(Map *map, char *msg);

extern void add_map_actions(void);

extern void update_unit_type_list(Side *side, Map *map, int u);

extern XawTextPosition widget_text_length(Widget w);

extern void move_caret_to_end(Widget w);

extern int popup_print_setup_dialog(Side *side);

void place_legends(Side *side);


void enable_in_unit_type_list(Side *side, Map *map, int u, int flag);

extern void zoom_in_out(Side *side, Map *map, int which);

extern int find_side_and_map_via_ctrlpanel_form(Widget w, Side **sidep,
						Map **mapp);

extern time_t game_start_in_real_time;
extern time_t turn_play_start_in_real_time;

extern void handle_map_sides_events(Widget w, XtPointer clientdata,
				    XEvent *evt, Boolean *contdispatch);
extern void handle_map_info_events(Widget w, XtPointer clientdata,
				   XEvent *evt, Boolean *contdispatch);

extern void place_legends(Side *side);

extern void scroll_map_absolute(Side *side, Map *map, int sx, int sy);
extern void scroll_map_relative(Side *side, Map *map, int sx, int sy);

extern void draw_blast_image(Side *side, Map *map, int sx, int sy,
			     int sw, int sh, int blasttype);
extern void invert_unit_subarea(Side *side, Map *map, int x, int y);

extern void update_help(Side *side);

#ifdef DESIGNERS
extern void create_design(Side *side);
extern void popup_design(Side *side);
extern void update_curttype(Side *side);
extern void update_curutype(Side *side);
extern void update_cursidenumber(Side *side);
extern void update_curfeature(Side *side);
extern void update_curbrushradius(Side *side);
extern void set_designer_cur_from_map(Side *side, Map *map, int sx, int sy);
extern void handle_designer_map_click(Side *side, Map *map, int sx, int sy);
extern void popdown_design(Side *side);
#endif /* DESIGNERS */

extern UnitCloseup *find_unit_closeup(Side *side, Unit *unit);
extern UnitCloseup *find_unit_closeup_via_button(Widget w, Side *side);
extern UnitCloseup *create_unit_closeup(Side *side, Map *map, Unit *unit);
extern void draw_unit_closeup(Side *side, UnitCloseup *unitcloseup);
extern void destroy_unit_closeup(Side *side, UnitCloseup *unitcloseup);
extern SideCloseup *find_side_closeup(Side *side, Side *side2);
extern SideCloseup *find_side_closeup_via_button(Widget w, Side *side);
extern SideCloseup *create_side_closeup(Side *side, Map *map, Side *side2);
extern void draw_side_closeup(Side *side, SideCloseup *sidecloseup);
extern void destroy_side_closeup(Side *side, SideCloseup *sidecloseup);
extern Pixmap get_unit_picture(int utype, Side *side);
extern Pixmap get_side_picture(Side *side, Side *side1);

void raise_widget(Widget w);

void initial_ui_init(int argc, char *argv[]);
void ui_init(void);
void ui_mainloop(void);



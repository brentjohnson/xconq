/* Definitions for graphics support not specific to any interface.
   Copyright (C) 1993-1995, 1997-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#define BMAP_BYTE unsigned char
#define NUMPOWERS 8

/* Xform flags for unit views. */
#define XFORM_UVIEW_LEGACY	    0   /*! Xform uview the old way. */
#define XFORM_UVIEW_PILES	    1   /*! Xform uview with piles. */
#define XFORM_UVIEW_ELLIPSIS	    2   /*! Xform uview with ellipsis. */
#define XFORM_UVIEW_AS_TSPT	    4   /*! Xform uview as transport. */
#define XFORM_UVIEW_AS_OCC	    8   /*! Xform uview as occupant. */
#define XFORM_UVIEW_DONT_PILE_ACP   16	/*! Don't pile if dissimilar ACP. */
#define XFORM_UVIEW_DONT_PILE_HP    32	/*! Don't pile if dissimilar HP. */
#define XFORM_UVIEW_DONT_PILE_CP    64	/*! Don't pile if incomplete. */
#define XFORM_UVIEW_DONT_PILE_OCC   128	/*! Don't pile if has occ. */
#define XFORM_UVIEW_DONT_DRAW	    (1 << 31)	/*! Don't draw the uview. */

/* Drawing techniques that may be used. */

enum whattouse {
  dontdraw,
  useblocks,
  usepolygons,
  usepictures
};

typedef struct a_vp {
    int sx, sy;			/* LL corner of the viewport, in pixels */
    int totsw, totsh;		/* total size of the map in pixels */
    int sxmin, symin;		/* lower bound on the viewport position */
    int sxmax, symax;		/* upper bound on the viewport position */
    short pxw, pxh;		/* size of window in pixels */
    short power;		/* index to this map's magnification */
    short mag;			/* magnification of each cell (a power of 2) */
    short vcx, vcy;		/* center of the view, in cells */
    short hw, hh;		/* pixel dims of a cell */
    short hch;			/* vertical center-to-center distance */
    short uw, uh;		/* pixel dims of unit subcell */
    short angle;		/* Angle of perspective view */
    int cellwidth;
    short vertscale;		/* vertical exaggeration of angle/iso views */
    short isometric;		/* true if display is isometric */
    short isodir;		/* direction in which iso viewer is facing */
    short scroll_beyond_dateline;   /* Permit scrolling beyond the dateline in 
				       wrapped worlds. Currently supported 
				       only in the mac ppc interface. */
    short show_all; 	/* True if the display should bypass view structures 
			   and display the state of everything accurately. This
    			   can be true if the side always sees all, or the 
			   side is designing, or the game is over. */
    short draw_terrain;		/* Display solid color terrain on the map? */
    short *draw_aux_terrain;	/* draw aux terrain, each type? */
    short draw_grid;		/* Draw outlines around cells? */
    short draw_cell_pats;	/* Draw terrain patterns? */
    short draw_other_maps;	/* Draw outlines of other maps? */
    short draw_units;		/* Draw units on the map? */
    short draw_names;		/* Draw unit names/numbers on the map? */
    short oldest_to_show;	/* the relative time of the oldest data */
    short age_to_fade;		/* point at which to gray out the display */
    short draw_people;		/* Draw people sides on the map? */
    short draw_control;		/* Draw controlling sides on the map? */
    short draw_elevations;	/* Draw elevations on the map? */
    short num_contours;		/* Number of contour lines to draw */
    short contour_interval;		/* Contour interval */
    short contour_interval_fixed;  	/* True if player set the interval */
    struct a_line_segment *linebuf;	/* Buffer of contour line segments */
    short *draw_materials;
    short num_materials_to_draw;
    short draw_lighting;		/* Draw day/night on the map? */
    short draw_temperature;		/* Draw temperatures on the map? */
    short draw_winds;			/* Draw wind vectors on the map? */
    short draw_clouds;			/* Draw clouds on the map? */
    short draw_storms;
    short draw_plans;
    short draw_cover;
    short draw_feature_boundaries;
    short draw_shorelines;
    short draw_feature_names;
    short draw_meridians;	/* Draw latitude/longitude in view */
    short meridian_interval;	/* Spacing of meridians in arc minutes */
    short lat_label_lon_interval;
    short lon_label_lat_interval;
    short draw_ai;		/* Draw AI info in view */
    short draw_occupants;	/* Draw unit occupants using grouping boxes */
} VP;

typedef struct a_legend {
    int ox, oy;			/* starting point */
    int dx, dy;			/* displacement */
    float angle, dist;		/* polar displacement (redundant, but useful) */
} Legend;

typedef struct a_line_segment {
    int sx1, sy1, sx2, sy2;
} LineSegment;

#define REDRAW_CHAR '\001'
#define ESCAPE_CHAR '\033'

#ifdef MAC_TCL
#define BACKSPACE_CHAR 0x7B
#define DELETE_CHAR 0x7B
#else
#define BACKSPACE_CHAR '\010'
#define DELETE_CHAR 0x7f
#endif

#define m_terrain_visible(vp, x, y) \
  ((vp)->show_all || terrain_visible(dside, (x), (y)))

#define m_seen_border(vp, x, y, b) \
  ((vp)->show_all || seen_border(dside, (x), (y), (b)))

/* Always redraw cells outside the area, that is the window background. */

#define cell_style(vp, x, y)  \
  ((m_terrain_visible(vp, x, y) || unseen_image != NULL || !in_area(x, y))  \
   ? (vp->power >= 3 ? usepolygons : useblocks)  \
   : dontdraw);

#define cell_overlay(vp, x, y)  \
  ((dside->designer && !(vp)->show_all) \
   ? ((terrain_view(dside, x, y) == UNSEEN) ? -2 : 0)  \
   : (!dside->see_all && (vp)->draw_cover  \
      ? (cover(dside, x, y) == 0  \
         ? -1  \
         : ((vp)->draw_lighting ? (night_at(x, y) ? -2 : 0) : 0))  \
      : ((vp)->draw_lighting  \
          ? (night_at(x, y) ? -2 : 0)  \
          : 0)))

#define cell_terrain(vp, x, y)  \
   ((!in_area(x, y)) ? BACKTTYPE :  \
   (((vp)->show_all || dside->designer) \
   ? terrain_at(x, y)  \
   : (m_terrain_visible(vp, x, y)  \
	  ? vterrain(terrain_view(dside, x, y))  \
	  : NONTTYPE)))

extern Side *dside;

extern short mags[], hws[], hhs[], hcs[], halfsides[], uws[], uhs[];

extern short iws[], ihs[], ics[], ix1[], ix2[];

extern short bwid[], bwid2[], cwid[];

extern short bsx[NUMPOWERS][7], bsy[NUMPOWERS][7];
extern short lsx[NUMPOWERS][6], lsy[NUMPOWERS][6];

extern short ibsx[NUMPOWERS][7], ibsy[NUMPOWERS][7];
extern short ilsx[NUMPOWERS][6], ilsy[NUMPOWERS][6];

extern short qx[NUMPOWERS][7], qy[NUMPOWERS][7];

extern const char *dirchars;

/* How much space to leave for a unit image, if all images should get
   the same amount (such as for a list of unit types). Currently used
   only by the X11 interface. */

extern int min_w_for_unit_image;
extern int min_h_for_unit_image;

extern int *numuimages;

extern ImageFamily **uimages;
extern ImageFamily *unseen_image;
extern ImageFamily *generic_transition;
extern ImageFamily *generic_fuzz;

extern char *terrchars;
extern char *unitchars;
extern char unseen_char_1, unseen_char_2;

extern Module **possible_games;

extern int numgames;

extern void (*imf_describe_hook)(Side *side, Image *img);

extern int big_unit_images;

/* Function declarations. */

extern void collect_possible_games(void);
extern void add_to_possible_games(Module *module);

extern VP *new_vp(void);

extern void xform_cell(VP *vp, int x, int y, int *sxp, int *syp);
extern void xform_cell_top(VP *vp, int x, int y, int *sxp, int *syp);
extern void xform_cell_fractional(VP *vp, int x, int y, int xf, int yf,
				  int *sxp, int *syp);
extern void xform_unit(VP *vp, Unit *unit,
		       int *sxp, int *syp, int *swp, int *shp);
extern void xform_unit_self(VP *vp, Unit *unit,
			    int *sxp, int *syp, int *swp, int *shp);
extern int xform_unit_self_view(Side *side, VP *vp, UnitView *uview,
				int *sxp, int *syp, int *swp, int *shp);
extern void xform_occupant(VP *vp, Unit *transport, Unit *unit,
			   int sx, int sy, int sw, int sh,
			   int *sxp, int *syp, int *swp, int *shp);
extern void xform_occupant_view(VP *vp, UnitView *traview, UnitView *uview,
			   int sx, int sy, int sw, int sh,
			   int *sxp, int *syp, int *swp, int *shp);
#if (0)
extern void xform_unit_view(Side *side, VP *vp, UnitView *uview,
			    int *sxp, int *syp, int *swp, int *shp);
#endif
extern int xform_unit_view(Side *side, VP *vp, UnitView *uview,
			    int *sxp, int *syp, int *swp, int *shp, 
			    int flags = XFORM_UVIEW_LEGACY, 
			    int *piles = NULL, int sxt = -1, int syt = -1, 
			    int swt = -1, int sht = -1);
extern void xform_cell_flat(VP *vp, int x, int y, int *sxp, int *syp);
extern void xform_cell_fractional_flat(VP *vp, int x, int y, int xf, int yf,
				       int *sxp, int *syp);
extern void scale_vp(VP *vp, VP *vp2, int *sxp, int *syp, int *swp, int *shp);
extern void scale_point(VP *vp, VP *vp2, int sx, int sy, int *sx2p, int *sy2p);

extern int nearest_cell(VP *vp, int sx, int sy, int *xp, int *yp,
			int *xfp, int *yfp);
extern int nearest_boundary(VP *vp, int sx, int sy, int *xp, int *yp,
			    int *dirp);
extern int nearest_unit(Side *side, VP *vp, int sx, int sy, Unit **unitp);
extern int nearest_unit_view(Side *side, VP *vp, int sx, int sy,
			     UnitView **uviewp);

extern int cell_is_visible(VP *vp, int x, int y);
extern int cell_is_in_middle(VP *vp, int x, int y);

extern int set_view_size(VP *vp, int w, int h);
extern int set_view_position(VP *vp, int sx, int sy);
extern int set_view_power(VP *vp, int power);
extern int set_view_angle(VP *vp, int angle);
extern int set_view_isometric(VP *vp, int flag, int scale);
extern int set_view_direction(VP *vp, int dir);
extern int set_view_focus(VP *vp, int x, int y);
extern void center_on_focus(VP *vp);
extern void focus_on_center(VP *vp);

extern void free_vp(VP *vp);

extern int compute_x1_len(VP *vp, int vx, int vy, int y, int *x1p, int *lenp);
extern int any_borders_in_dir(int x, int y, int dir);
extern int any_coating_at(int x, int y);
extern int compute_transition(Side *side, VP *vp, int x, int y, int dir,
			      int *sxp, int *syp, int *swp, int *shp,
			      int *offsetp);

extern void compute_fire_line_segment(int sx1, int sy1, int sx2, int sy2,
				      int i, int n, int *xx, int *yy,
				      int *dx, int *dy);
extern void place_feature_legends(Legend *leg, int nf, Side *side,
				  int orient, int block);
extern void set_meridian_interval(VP *vp, int interval);
extern void plot_meridians
 (VP *vp,
  void (*line_callback)(int x1, int y1, int x1f, int y1f,
			int x2, int y2, int x2f, int y2f),
  void (*text_callback)(int x1, int y1, int x1f, int y1f, char *str));
extern void set_contour_interval(VP *vp, int n);
extern void contour_lines_at(VP *vp, int x, int y, int sx, int sy,
			     LineSegment **lines, int *numlinesp);
extern int unit_visible(Side *side, VP *vp, Unit *unit);
extern int draw_temperature_here(Side *side, int x, int y);
extern int draw_winds_here(Side *side, int x, int y);

extern void oneliner(Side *side, VP *vp, int sx, int sy);

extern Unit *find_unit_or_occ(Side *side, VP *vp, Unit *unit,
			      int usx, int usy, int usw, int ush,
			      int sx, int sy);
extern Unit *find_unit_at(Side *side, VP *vp, int x, int y, int sx, int sy);
//! Return unit view at a certain pixel, if there is one.
extern UnitView *find_uview_at_pixel(
    Side *side, VP *vp, UnitView *uview, int usx, int usy, int usw, int ush,
    int sx, int sy);
//! Return unit view at a certain pixel, if there is one.
extern UnitView *find_uview_at_pixel(
    Side *side, VP *vp, int x, int y, int sx, int sy);

extern void pick_a_focus(Side *side, int *xp, int *yp);

extern Unit *autonext_unit(Side *side, Unit *unit);
extern Unit *autonext_unit_inbox(Side *side, Unit *unit, VP *vp);
extern int could_be_next_unit(Unit *unit);

extern Unit *find_next_occupant(Unit *unit);
extern Unit *embarkation_unit(Unit *unit);
extern int find_units_matching(Side *side, char *name, Unit **unitp);

extern int num_active_displays(void);

extern int char_to_dir(int ch, int *dir1p, int *dir2p, int *modp);
extern int advance_into_cell(Side *side, Unit *unit, int x, int y,
			     Unit *other, HistEventType *reason);

extern int unit_could_attack(Unit *unit, int u2, Side *side2, int x, int y);

extern Unit *give_supplies(Unit *unit, short *amts, short *rslts);
extern int take_supplies(Unit *unit, short *amts, short *rslts);

extern int impl_move_to(Side *side, Unit *unit, int x, int y, int dist);
extern int impl_build(
    Side *side, Unit *unit, int u2, Unit *transport, int x, int y, int n);
extern int impl_change_type(Side *side, Unit *unit, int u2);
extern int favored_type(Unit *unit);

extern void set_unit_image(Unit *unit);
extern void set_unit_view_image(UnitView *uview);

extern ImageFamily *get_unit_type_images(Side *side, int u);
extern ImageFamily *get_material_type_images(Side *side, int m);
extern ImageFamily *get_terrain_type_images(Side *side, int t);
extern ImageFamily *get_unseen_images(Side *side);
extern ImageFamily *get_emblem_images(Side *side, Side *side2);

extern void record_imf_get(ImageFamily *imf);

extern void describe_imf(Side *side, const char *classname, const char *imftype,
			 ImageFamily *imf);

extern void init_ui_chars(void);
extern void init_unit_images(Side *side);
extern void dump_text_view(Side *side, int use_both);

extern int terrain_seen_at(Side *side, int x, int y);
extern Unit *unit_seen_at(Side *side, int x, int y);
extern int utype_seen_at(Side *side, int x, int y);

/* cmd.c */

extern char *cmdargstr;
extern char tmpkey;

extern HelpNode *key_commands_help_node;
extern HelpNode *long_commands_help_node;
extern HelpNode *map_help_node;

/* Declarations of all the command functions. */

#undef DEF_CMD
#define DEF_CMD(letter,name,FN,help) \
  void FN(Side *side);

#include "cmd.def"

extern void execute_command(Side *side, int ch);
extern void execute_long_command(Side *side, char *cmdstr);
extern void parse_long_name_command(char *cmdstr, int *prefixp,
				    char **namep, char **argp, char *buf);
extern void describe_key_commands(int arg, const char *key, TextBuffer *buf);
extern void describe_long_commands(int arg, const char *key, TextBuffer *buf);

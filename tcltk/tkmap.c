/* Map widget for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2001 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#define USE_INTERP_RESULT

#include "conq.h"
#include "kpublic.h"
#include "tkconq.h"

extern int error_popped_up;
extern int grid_matches_unseen;

/* Tells us to use a blinking current unit instead of the crawling ants 
animation. Used by MacTcl, should be user settable preference in the
other interfaces. */

extern int blinking_curunit;

void handle_designer_mouse_down(Side *side, Map *map, int sx, int sy);
void move_look(Map *map, int sx, int sy);
void move_the_selected_unit(Map *map, Unit *unit, int sx, int sy);

extern VP default_vp;
extern int default_draw_lines;
extern int default_draw_polygons;
extern int default_draw_terrain_images;
extern int default_draw_terrain_patterns;
extern int default_draw_transitions;

extern Pixmap fuzzpics[NUMPOWERS];
extern Pixmap windpics[5][NUMDIRS];
extern Pixmap antpic;

typedef struct {
    Tk_Window tkwin;		/* Window that embodies the MapW.  NULL
				 * means window has been deleted but
				 * widget record hasn't been cleaned up yet. */
    Display *display;		/* X's token for the window's display. */
    Tcl_Interp *interp;		/* Interpreter associated with widget. */
    Tcl_Command widgetCmd;	/* Token for MapW's widget command. */
    Tk_3DBorder bg_border;	/* Used for drawing background. */

    GC gc;
    GC copygc;			/* Graphics context for copying from
				 * off-screen pixmap onto screen. */
    int double_buffer;		/* Non-zero means double-buffer redisplay
				 * with pixmap;  zero means draw straight
				 * onto the display. */
    Drawable d;
    int update_pending;		/* Non-zero means a call to mapw_display
				 * has already been scheduled. */
    int rsx, rsy, rsw, rsh;     /* rect to update */

    int width, height;
    int maxheight;
    VP *vp;
    int power;
    int world;
    Map *map;

    Tk_Cursor cursor;

    /* Copies of the generic viewport options. */
    int isometric;
    int isodir;
    int vertscale;
    int see_all;
    int *draw_aux_terrain;
    int draw_grid;		/* Draw outlines around cells? */
    int draw_names;		/* Draw unit names/numbers on the map? */
    int draw_people;		/* Draw people sides on the map? */
    int draw_control;		/* Draw controlling sides on the map? */
    int draw_elevations;	/* Draw elevations on the map? */
    int *draw_materials;
    int num_materials_to_draw;
    int draw_lighting;		/* Draw day/night on the map? */
    int draw_temperature;	/* Draw temperatures on the map? */
    int draw_winds;		/* Draw wind vectors on the map? */
    int draw_clouds;		/* Draw clouds on the map? */
    int draw_cover;
    int draw_feature_boundaries;
    int draw_feature_names;
    int draw_meridians;		/* Draw latitude/longitude in view */
    int meridian_interval;	/* Spacing of meridians in arc minutes */
    int draw_ai;		/* Draw AI info in view */

    /* View options specific to this interface. */
    int draw_lines;
    int draw_terrain_images;
    int draw_terrain_patterns;
    int draw_transitions;

    Tk_Font main_font;
    Tk_Font feature_font;

    int last_ws_valid;
    int last_wsx[4], last_wsy[4];  /* position of outline of map in world */

    int blastsx, blastsy, blastsw, blastsh, blasttype;
} MapW;

static Tk_ConfigSpec config_specs[] = {
    {TK_CONFIG_INT, "-ai", "ai", "AI",
	"0", Tk_Offset(MapW, draw_ai), 0},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	"gray", Tk_Offset(MapW, bg_border), TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	"gray50", Tk_Offset(MapW, bg_border), TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
#if 0 /* Although borderwidth is "standard", it's hard to make it work
	 right with the cell drawing, and the effect can be achieved
	 just by embedding the map in a frame, so omit it. */
    {TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
	"0", Tk_Offset(MapW, border_width), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
	"sunken", Tk_Offset(MapW, relief), 0},
#endif
    {TK_CONFIG_INT, "-clouds", "clouds", "Clouds",
	"0", Tk_Offset(MapW, draw_clouds), 0},
    {TK_CONFIG_INT, "-control", "control", "Control",
	"0", Tk_Offset(MapW, draw_control), 0},
    {TK_CONFIG_INT, "-coverage", "coverage", "Coverage",
     "0", Tk_Offset(MapW, draw_cover), 0},
    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
	"", Tk_Offset(MapW, cursor), TK_CONFIG_NULL_OK},
    {TK_CONFIG_INT, "-dbl", "doubleBuffer", "DoubleBuffer",
	"1", Tk_Offset(MapW, double_buffer), 0},
    {TK_CONFIG_INT, "-direction", "direction", "Direction",
	"0", Tk_Offset(MapW, isodir), 0},
    {TK_CONFIG_INT, "-elevations", "elevations", "Elevations",
	"0", Tk_Offset(MapW, draw_elevations), 0},
    {TK_CONFIG_INT, "-featureboundaries", "featureboundaries", "FeatureBoundaries",
	"0", Tk_Offset(MapW, draw_feature_boundaries), 0},
    {TK_CONFIG_INT, "-featurenames", "featurenames", "FeatureNames",
	"1", Tk_Offset(MapW, draw_feature_names), 0},
    {TK_CONFIG_FONT, "-mainfont", "mainfont", "MainFont",
	"Courier -12", Tk_Offset(MapW, main_font), 0},
    {TK_CONFIG_FONT, "-featurefont", "featurefont", "FeatureFont",
	"Courier -12", Tk_Offset(MapW, feature_font), 0},
    {TK_CONFIG_INT, "-grid", "grid", "Grid",
	"0", Tk_Offset(MapW, draw_grid), 0},
    {TK_CONFIG_PIXELS, "-height", "height", "Height",
	"0", Tk_Offset(MapW, height), 0},
    {TK_CONFIG_INT, "-isometric", "isometric", "Isometric",
	"0", Tk_Offset(MapW, isometric), 0},
    {TK_CONFIG_INT, "-lighting", "lighting", "Lighting",
	"0", Tk_Offset(MapW, draw_lighting), 0},
    {TK_CONFIG_INT, "-lines", "lines", "Lines",
	"0", Tk_Offset(MapW, draw_lines), 0},
    {TK_CONFIG_PIXELS, "-maxheight", "maxheight", "MaxHeight",
	"0", Tk_Offset(MapW, maxheight), 0},
    {TK_CONFIG_INT, "-meridians", "meridians", "Meridians",
	"0", Tk_Offset(MapW, draw_meridians), 0},
    {TK_CONFIG_INT, "-meridianinterval", "meridianinterval", "MeridianInterval",
	"0", Tk_Offset(MapW, meridian_interval), 0},
    {TK_CONFIG_INT, "-people", "people", "People",
	"0", Tk_Offset(MapW, draw_people), 0},
    {TK_CONFIG_INT, "-power", "power", "Power",
	"5", Tk_Offset(MapW, power), 0},
    {TK_CONFIG_INT, "-temperature", "temperature", "Temperature",
	"0", Tk_Offset(MapW, draw_temperature), 0},
    {TK_CONFIG_INT, "-terrainimages", "terrain_images", "TerrainImages",
	"1", Tk_Offset(MapW, draw_terrain_images), 0},
    {TK_CONFIG_INT, "-terrainpatterns", "terrain_patterns", "TerrainPatterns",
	"1", Tk_Offset(MapW, draw_terrain_patterns), 0},
    {TK_CONFIG_INT, "-transitions", "transitions", "Transitions",
	"1", Tk_Offset(MapW, draw_transitions), 0},
    {TK_CONFIG_INT, "-unitnames", "unitnames", "UnitNames",
	"1", Tk_Offset(MapW, draw_names), 0},
    {TK_CONFIG_INT, "-verticalscale", "verticalscale", "VerticalScale",
	"1", Tk_Offset(MapW, vertscale), 0},
    {TK_CONFIG_PIXELS, "-width", "width", "Width",
	"0", Tk_Offset(MapW, width), 0},
    {TK_CONFIG_INT, "-winds", "winds", "Winds",
	"0", Tk_Offset(MapW, draw_winds), 0},
    {TK_CONFIG_INT, "-world", "world", "World",
	"0", Tk_Offset(MapW, world), 0},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0}
};

static void mapw_cmd_deleted_proc(ClientData cldata);
static int mapw_configure(Tcl_Interp *interp, MapW *MapW,
			  int argc, char **argv, int flags);
#if 0
static void set_scrollbars(MapW *mapw);
#endif
static void mapw_destroy(char *memPtr);
static void mapw_event_proc(ClientData cldata, XEvent *eventPtr);
static int mapw_widget_cmd(ClientData cldata, Tcl_Interp *interp,
			   int argc, char **argv);
static void mapw_display(ClientData cldata);
static void draw_map_widget(MapW *mapw);
static void draw_map_overhead(MapW *mapw);
static void draw_map_isometric(MapW *mapw);
static void draw_terrain_iso(MapW *mapw, int x, int y);
static int shading(int dir);
static void draw_cliff(MapW *mapw, int sx, int sy, int dir, int drop,
		       int face);

static void draw_unseen_fuzz(MapW *mapw, int vx, int vyhi, int vylo);
static void draw_row(MapW *mapw, int x0, int y0, int len);
static void draw_current(MapW *mapw);
static void draw_blast_image(MapW *mapw, int sx, int sy, int sw, int sh,
			     int blasttype);

static void draw_unit_image(ImageFamily *imf, MapW *mapw, int sx, int sy, 
			    int sw, int sh, int s2, int mod);
static void draw_side_emblem(MapW *mapw, int ex, int ey, int ew, int eh,
			     int s2);

static void xform(MapW *mapw, int x, int y, int *sxp, int *syp);
static void xform_fractional(MapW *mapw, int x, int y, int xf, int yf,
			     int *sxp, int *syp);
static void xform_top(MapW *mapw, int x, int y, int *sxp, int *syp);
static void x_xform_unit(MapW *mapw, Unit *unit, int *sxp, int *syp,
			 int *swp, int *shp);
static void x_xform_unit_self(MapW *mapw, Unit *unit, int *sxp, int *syp,
			      int *swp, int *shp);
static int x_nearest_cell(MapW *mapw, int sx, int sy, int *xp, int *yp);
static Unit *x_find_unit_or_occ(MapW *mapw, Unit *unit, int usx, int usy,
				int usw, int ush, int sx, int sy);
static Unit *x_find_unit_at(MapW *mapw, int x, int y, int sx, int sy);
static int x_nearest_unit(MapW *mapw, int sx, int sy, Unit **unitp);

static void draw_rectangle(MapW *mapw, int sx, int sy, int sw, int sh);
static void draw_feature_name(MapW *mapw, int f);
static void draw_ai_region(MapW *mapw, int x, int y);
static void draw_resource_usage(MapW *mapw, int x, int y);
static void draw_hex_polygon(MapW *mapw, GC gc, int sx, int sy,
			int power, int over, int coat, XColor *color2, int dogrid);
static void draw_iso_polygon(MapW *mapw, GC gc, int sx, int sy, 
			int power, int terrain, int over, int coat, XColor *color2);
static void draw_area_background(MapW *mapw);
static void meridian_line_callback(int x1, int y1, int x1f, int y1f,
				   int x2, int y2, int x2f, int y2f);
static void meridian_text_callback(int x1, int y1, int x1f, int y1f,
				   char *str);
static void cell_drawing_colors(int t, int coat, XColor **colorp, XColor **color2p);
static void set_terrain_gc_for_image(MapW *mapw, GC gc, Image *timg);
static void draw_terrain_row(MapW *mapw, int x0, int y0, int len, int force);
static void draw_terrain_transitions(MapW *mapw, int x0, int y0, int len,
				     int force);
static void draw_terrain_grid(MapW *mapw, int x0, int y0, int len);
static void draw_contours(MapW *mapw, int x0, int y0, int len);
static void draw_clouds_row(MapW *mapw, int x0, int y0, int len);
static void draw_temperature_row(MapW *mapw, int x0, int y0, int len);
static void draw_winds_row(MapW *mapw, int x0, int y0, int len);
static void draw_uviews_in_cell(MapW *mapw, int x, int y);
static void draw_unit_view(MapW *mapw, UnitView *uview, int sx, int sy, 
			   int sw, int sh);
static void draw_unit_view_and_occs(MapW *mapw, UnitView *unitview, 
				    int sx, int sy, int sw, int sh);
static void draw_dots(MapW *mapw, int sx, int sy, int sw, int sh);
static void draw_unit_name(MapW *mapw, char *name, int sx, int sy,
			   int sw, int sh);
static void draw_unit_size(MapW *mapw, int size, int sx, int sy,
			   int sw, int sh);
static void draw_people(MapW *mapw, int x, int y);
static void draw_borders(MapW *mapw, int vx, int vyhi, int vylo, int b);
static void draw_borders_iso(MapW *mapw, int x, int y, int b);
static void draw_connections(MapW *mapw, int vx, int vyhi, int vylo, int c);
static void draw_connections_iso(MapW *mapw, int x, int y, int c);
static void draw_country_border_line(MapW *mapw, int sx, int sy, int dir, int con);
static void draw_feature_boundary(MapW *mapw, int x, int y);
static void draw_meridians(MapW *mapw);
static void draw_map_outline(MapW *worldw, MapW *mapw, int draw, int direct);

static void eventually_redraw(MapW *mapw, int sx, int sy, int sw, int sh);

#define GRAY(over) ((over) == -1 ? lightgray : \
		    ((over) == -2 ? gray : darkgray))

int mapw_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[]);

/* Draw a rectangle in a platform-independent way. */

void
draw_rectangle(MapW* mapw, int sx, int sy, int sw, int sh)
{
    Display *dpy = NULL;
    GC gc = NULL;

    assert_error(mapw, "Attempted to draw in a NULL map window");
    gc = mapw->gc;
    dpy = mapw->display;
#ifndef WIN32
    XDrawRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
#else
    XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, 2);
    XFillRectangle(dpy, mapw->d, gc, sx + sw - 2, sy, 2, sh);
    XFillRectangle(dpy, mapw->d, gc, sx, sy + sh - 2, sw, 2);
    XFillRectangle(dpy, mapw->d, gc, sx, sy, 2, sh);
#endif
}

int
mapw_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int x, y;
    Tk_Window mainw = (Tk_Window) cldata;
    MapW *mapw;
    Tk_Window tkwin;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " pathName ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }

    tkwin = Tk_CreateWindowFromPath(interp, mainw, argv[1], (char *) NULL);
    if (tkwin == NULL)
      return TCL_ERROR;

    Tk_SetClass(tkwin, "MapW");

    /* Allocate and initialize the widget record.  */

    mapw = (MapW *) ckalloc(sizeof(MapW));
    mapw->tkwin = tkwin;
    mapw->display = Tk_Display(tkwin);
    mapw->interp = interp;
    mapw->widgetCmd =
      Tcl_CreateCommand(interp,
			Tk_PathName(mapw->tkwin), mapw_widget_cmd,
			(ClientData) mapw, mapw_cmd_deleted_proc);
    mapw->bg_border = NULL;
    mapw->gc = None;
    mapw->copygc = None;
    mapw->main_font = NULL;
    mapw->feature_font = NULL;
    mapw->double_buffer = 1;
    mapw->update_pending = FALSE;
    
    mapw->last_ws_valid = FALSE;

    mapw->cursor = None;

    mapw->draw_lines = FALSE;
    mapw->draw_terrain_images = default_draw_terrain_images;
    mapw->draw_terrain_patterns = default_draw_terrain_patterns;
    mapw->draw_transitions = default_draw_transitions;

    mapw->map = dside->ui->maps;

    mapw->rsx = mapw->rsy = mapw->rsw = mapw->rsh = -1;

    mapw->width = 0;
    mapw->height = 0;
    mapw->maxheight = 0;

    /* Set up the map widget's generic viewport. */
    mapw->vp = new_vp();
    mapw->vp->draw_terrain = TRUE;
    mapw->vp->draw_units = TRUE;
    /* Set a default power that will get modified by widget configure. */
    set_view_power(mapw->vp, 2);

    Tk_CreateEventHandler(mapw->tkwin, ExposureMask|StructureNotifyMask,
			  mapw_event_proc, (ClientData) mapw);
    if (mapw_configure(interp, mapw, argc-2, argv+2, 0) != TCL_OK) {
	Tk_DestroyWindow(mapw->tkwin);
	return TCL_ERROR;
    }

    if (mapw->world)
      mapw->map->worldw = (char *) mapw;
    else
      mapw->map->widget = (char *) mapw;

    pick_a_focus(dside, &x, &y);
    set_view_focus(mapw->vp, x, y);

    mapw->blasttype = -1;

    Tcl_SetResult(interp, Tk_PathName(mapw->tkwin), TCL_VOLATILE);
    return TCL_OK;
}

static int
mapw_widget_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char **argv)
{
    MapW *mapw = (MapW *) cldata;
    int result = TCL_OK;
    int update, sx, sy, nsx, nsy;
    int nrsx, nrsy, nrsw, nrsh;
    size_t cmdlength;
    char c;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tcl_Preserve((ClientData) mapw);
    update = FALSE;
    sx = nsx = mapw->vp->sx;  sy = nsy = mapw->vp->sy;
    c = argv[1][0];
    cmdlength = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "cget", cmdlength) == 0)
	&& (cmdlength >= 2)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cget option\"",
		    (char *) NULL);
	    goto error;
	}
	result = Tk_ConfigureValue(interp, mapw->tkwin, config_specs,
				   (char *) mapw, argv[2], 0);
	update = TRUE;
    } else if ((c == 'c') && (strncmp(argv[1], "configure", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (argc == 2) {
	    result = Tk_ConfigureInfo(interp, mapw->tkwin, config_specs,
		    (char *) mapw, (char *) NULL, 0);
	} else if (argc == 3) {
	    result = Tk_ConfigureInfo(interp, mapw->tkwin, config_specs,
		    (char *) mapw, argv[2], 0);
	} else {
	    result = mapw_configure(interp, mapw, argc-2, argv+2,
		    TK_CONFIG_ARGV_ONLY);
	}
	update = TRUE;
    } else if ((c == 'x') && (strncmp(argv[1], "xview", cmdlength) == 0)) {
	int count, type;
	double fraction, fraction2;

	if (argc == 2) {
	    fraction = 0;
	    fraction2 = 1;
	    printf("map xview %g %g\n", fraction, fraction2);
	    sprintf(interp->result, "%g %g", fraction, fraction2);
	} else {
	    type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	    switch (type) {
	      case TK_SCROLL_ERROR:
		goto error;
	      case TK_SCROLL_MOVETO:
		nsx = (int)(((mapw->vp->sxmax - mapw->vp->sxmin) * fraction));
		break;
	      case TK_SCROLL_PAGES:
		nsx += (count * mapw->vp->pxw * 4) / 5;
		break;
	      case TK_SCROLL_UNITS:
		nsx += (count * 12);
		break;
	    }
	    /* Correct any out-of-bounds values. */
	    if (nsx < mapw->vp->sxmin)
	      nsx = mapw->vp->sxmin;
	    if (nsx > mapw->vp->sxmax)
	      nsx = mapw->vp->sxmax;
	}
    } else if ((c == 'y') && (strncmp(argv[1], "yview", cmdlength) == 0)) {
	int count, type;
	double fraction, fraction2;

	if (argc == 2) {
	    fraction = 0;
	    fraction2 = 1;
	    printf("map yview %g %g\n", fraction, fraction2);
	    sprintf(interp->result, "%g %g", fraction, fraction2);
	} else {
	    type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	    switch (type) {
	      case TK_SCROLL_ERROR:
		goto error;
	      case TK_SCROLL_MOVETO:
		nsy = (int)(((mapw->vp->symax - mapw->vp->symin) * fraction));
		break;
	      case TK_SCROLL_PAGES:
		nsy += (count * mapw->vp->pxh * 4) / 5;
		break;
	      case TK_SCROLL_UNITS:
		nsy += (count * 12);
		break;
	    }
	    /* Correct any out-of-bounds values. */
	    if (nsy < mapw->vp->symin)
	      nsy = mapw->vp->symin;
	    if (nsy > mapw->vp->symax)
	      nsy = mapw->vp->symax;
	}
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": must be cget, configure, xview, or yview",
		(char *) NULL);
	goto error;
    }
    nrsx = nrsy = nrsw = nrsh = -1;
    /* See if we're scrolling. */
    if (nsx != sx || nsy != sy) {
	int wid = Tk_Width(mapw->tkwin), hgt = Tk_Height(mapw->tkwin);
	Window win = Tk_WindowId(mapw->tkwin);
	GC gc = mapw->gc;
	Display *dpy = mapw->display;

	/* Clear the map outline rect if we are scrolling the worldmap. */
	if (mapw->world) {
	    draw_map_outline(mapw, (MapW *) (mapw->map->widget), FALSE, TRUE);
	}
	XSetClipMask(mapw->display, gc, None);
	if (nsx != sx) {
	    if (nsx > sx) {
		XCopyArea(dpy, win, win, gc, (nsx - sx), 0, wid, hgt, 0, 0);
		nrsx = mapw->vp->pxw - (nsx - sx);  nrsy = 0;
		nrsw = nsx - sx;  nrsh = mapw->vp->pxh;
	    } else {
		XCopyArea(dpy, win, win, gc, 0, 0, wid, hgt, (sx - nsx), 0);
		nrsx = 0;  nrsy = 0;
		nrsw = sx - nsx;  nrsh = mapw->vp->pxh;
	    }
	} else { /* nsy != sy */
	    if (nsy > sy) {
		XCopyArea(dpy, win, win, gc, 0, (nsy - sy), wid, hgt, 0, 0);
		nrsx = 0;  nrsy = mapw->vp->pxh - (nsy - sy);
		nrsw = mapw->vp->pxw;  nrsh = nsy - sy;
	    } else {
		XCopyArea(dpy, win, win, gc, 0, 0, wid, hgt, 0, (sy - nsy));
		nrsx = 0;  nrsy = 0;
		nrsw = mapw->vp->pxw;  nrsh = sy - nsy;
	    }
	}
	set_view_position(mapw->vp, nsx, nsy);
	/* After scrolling around a bit, you want to zoom in on the
	   current center of the display, rather than bouncing back to
	   the last focus point (a very annoying behavior!). */
	focus_on_center(mapw->vp);
	update = TRUE;
	
	/* Redraw the map outline rect if we are scrolling the worldmap. */
	if (mapw->world) {
	    draw_map_outline(mapw, (MapW *) (mapw->map->widget), TRUE, TRUE);
    	}
    }
    if (update) {
	eventually_redraw(mapw, nrsx, nrsy, nrsw, nrsh);
    }
    Tcl_Release((ClientData) mapw);
    return result;

error:
    Tcl_Release((ClientData) mapw);
    return TCL_ERROR;
}

static int
mapw_configure(Tcl_Interp *interp, MapW *mapw, int argc, char **argv,
	       int flags)
{
    int wid, hgt, viewpow;
    XGCValues gc_values;

    if (Tk_ConfigureWidget(interp, mapw->tkwin, config_specs,
			   argc, argv, (char *) mapw, flags) != TCL_OK)
      return TCL_ERROR;

    /* Set the background for the window and create graphics contexts
       for use during redisplay.  */
    Tk_SetWindowBackground(mapw->tkwin,
			   Tk_3DBorderColor(mapw->bg_border)->pixel);
    if (mapw->copygc == None) {
	gc_values.function = GXcopy;
	gc_values.graphics_exposures = False;
	mapw->copygc = XCreateGC(mapw->display,
				 DefaultRootWindow(mapw->display),
				 GCFunction|GCGraphicsExposures, &gc_values);
    }
    if (mapw->gc == None) {
	mapw->gc = XCreateGC(mapw->display, DefaultRootWindow(mapw->display),
			     None, NULL);
	XSetTSOrigin(mapw->display, mapw->gc, 0, 0);
	XSetLineAttributes(mapw->display, mapw->gc, 0, LineSolid, CapButt, JoinMiter);
    }
#if 0
    if (mapw->main_font == NULL)
      mapw->main_font = Tk_GetFont(interp, mapw->tkwin, "-weight bold");
    if (mapw->main_font == NULL)
      mapw->main_font = Tk_GetFont(interp, mapw->tkwin, "fixed");
#endif
    XSetFont(mapw->display, mapw->gc, Tk_FontId(mapw->main_font));

    /* Register the desired geometry for the window. */
    wid = mapw->width;  hgt = mapw->height;
    /* First "pass" at the size. */
    set_view_power(mapw->vp, mapw->power);
    set_view_isometric(mapw->vp, mapw->isometric, mapw->vertscale);
    set_view_direction(mapw->vp, mapw->isodir);
    /* Don't recenter the map if we are just changing the cursor. */
    if (strcmp(argv[0], "-cursor") != 0) {
	center_on_focus(mapw->vp);
    }    
    /* a hack */
    if (mapw->world && area.xwrap)
      mapw->vp->sx = 0;
    viewpow = mapw->power;
    /* If no explicit width/height passed in, use the full size of the
       map. */
    if (wid <= 0)
      wid = mapw->vp->totsw;
    if (hgt <= 0)
      hgt = mapw->vp->totsh;
    /* If we defined an explicit upper bound on the height of the map,
       take it into account.  (This is basically a supplement to the
       packing algorithm, which tries to honor geometry requests
       exactly and can thus let the map take the whole right-side area
       if the map is large; so we need to constrain the request *before*
       it is made.)  */
    if (mapw->maxheight > 0)
      hgt = min(hgt, mapw->maxheight);
    Tk_GeometryRequest(mapw->tkwin, wid, hgt);

    /* Copy view settings into the viewport. */
    mapw->vp->draw_ai = mapw->draw_ai;
    mapw->vp->draw_clouds = mapw->draw_clouds;
    mapw->vp->draw_control = mapw->draw_control;
    mapw->vp->draw_cover = mapw->draw_cover;
    mapw->vp->draw_elevations = mapw->draw_elevations;
    /* The test for TK_CONFIG_ARGV_ONLY is to make it possible to toggle drawing
    of feature boundaries manually from the menu (the override only applies when 
    the map is created). */
    if (flags != TK_CONFIG_ARGV_ONLY && g_feature_boundaries() != -1) {
	    mapw->vp->draw_feature_boundaries = g_feature_boundaries();
    	    mapw->draw_feature_boundaries = mapw->vp->draw_feature_boundaries;
	    eval_tcl_cmd("set map_options(.m%d,feature_boundaries) %d",
	    			mapw->map->number, 
	    			mapw->vp->draw_feature_boundaries);
    } else {
    	    mapw->vp->draw_feature_boundaries = mapw->draw_feature_boundaries;
    }
    mapw->vp->draw_feature_names = mapw->draw_feature_names;
    mapw->vp->draw_grid = mapw->draw_grid;
    mapw->vp->draw_lighting = mapw->draw_lighting;
    /* The test for TK_CONFIG_ARGV_ONLY is to make it possible to toggle drawing
    of meridians manually from the menu (the override only applies when the map 
    is created). */
    if (flags != TK_CONFIG_ARGV_ONLY && g_meridians() != -1) {
	    mapw->vp->draw_meridians = g_meridians();
	    mapw->draw_meridians = mapw->vp->draw_meridians;
	    eval_tcl_cmd("set map_options(.m%d,meridians) %d",
	    			mapw->map->number,  
	    			mapw->vp->draw_meridians);
    } else {
    	    mapw->vp->draw_meridians = mapw->draw_meridians;
    }
    /* The test for TK_CONFIG_ARGV_ONLY is to make it possible to toggle drawing
    of terrain transitions manually from the menu (the override only applies when 
    the map is created). */
    if (flags != TK_CONFIG_ARGV_ONLY && g_terrain_transitions() != -1) {
	    mapw->draw_transitions = g_terrain_transitions();
	    eval_tcl_cmd("set default_map_options(transitions) %d", 
	    			mapw->draw_transitions);
    }
    mapw->vp->draw_people = mapw->draw_people;
    mapw->vp->draw_temperature = mapw->draw_temperature;
    mapw->vp->draw_names = mapw->draw_names;
    mapw->vp->draw_winds = mapw->draw_winds;
    set_meridian_interval(mapw->vp, mapw->meridian_interval);

    Tk_SetInternalBorder(mapw->tkwin, 0);

    /* Set the map widget to be redisplayed when convenient. */
    eventually_redraw(mapw, -1, -1, -1, -1);

    return TCL_OK;
}

static void
mapw_event_proc(ClientData cldata, XEvent *event_ptr)
{
    MapW *mapw = (MapW *) cldata;

    if (event_ptr->type == Expose) {
	eventually_redraw(mapw, -1, -1, -1, -1);
    } else if (event_ptr->type == ConfigureNotify) {
	eventually_redraw(mapw, -1, -1, -1, -1);
    } else if (event_ptr->type == DestroyNotify) {
	if (mapw->tkwin != NULL) {
	    mapw->tkwin = NULL;
	    Tcl_DeleteCommand(mapw->interp,
			      Tcl_GetCommandName(mapw->interp,
						 mapw->widgetCmd));
	}
	if (mapw->update_pending) {
	    Tcl_CancelIdleCall(mapw_display, cldata);
	}
	Tcl_EventuallyFree(cldata, mapw_destroy);
    }
}

static void
mapw_cmd_deleted_proc(ClientData cldata)
{
    MapW *mapw = (MapW *) cldata;
    Tk_Window tkwin = mapw->tkwin;

    if (tkwin != NULL) {
	mapw->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

static void
mapw_display(ClientData cldata)
{
    MapW *mapw = (MapW *) cldata;
    Tk_Window tkwin = mapw->tkwin;
    int winwidth = Tk_Width(tkwin), winheight = Tk_Height(tkwin);
    int wid, hgt;
    
   mapw->update_pending = FALSE;
    if (!Tk_IsMapped(tkwin))
      return;

    /* Compute the desired geometry. */
    wid = mapw->width;  
    hgt = mapw->height;
    if (wid <= 0)
      wid = mapw->vp->totsw;
    if (hgt <= 0)
      hgt = mapw->vp->totsh;
    if (wid > mapw->vp->totsw)
      wid = mapw->vp->totsw;
    if (hgt > mapw->vp->totsh)
      hgt = mapw->vp->totsh;
    if (mapw->maxheight > 0)
      hgt = min(hgt, mapw->maxheight);
    /* We are zooming in or out. */
    if (wid < winwidth 
         || hgt < winheight
         || (wid > winwidth && winwidth < Tk_Width(Tk_Parent(tkwin)) - 2)
         || (hgt > winheight && winheight < Tk_Height(Tk_Parent(tkwin)) - 2)) {
	Tk_GeometryRequest(tkwin, wid, hgt);
	/* Redraw the widget's background in order to erase remnants 
	of the old map. */
	mapw->d = Tk_WindowId(tkwin);
	Tk_Fill3DRectangle(tkwin, mapw->d, mapw->bg_border, 
			0, 0, winwidth, winheight, 0, TK_RELIEF_FLAT);
	/* Then make sure we wait until the geometry request kicks in. */
	while (Tk_Width(tkwin) == winwidth 
	     && Tk_Height(tkwin) == winheight) {
		Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
	}
	winwidth = Tk_Width(tkwin);
	winheight = Tk_Height(tkwin);
    }

    /* If bad things are happening, don't try to continue - could
       cause an infinite loop, if drawing run_errors are causing
       popups to appear. */
    if (error_popped_up)
      return;

    set_view_size(mapw->vp, winwidth, winheight);

#if 0
    if (mapw == (MapW *) map->widget)
      set_scrollbars(mapw);
#endif

    /* Clear the map outline rect if we are drawing a worldmap. */
    if (mapw->world) {
    	draw_map_outline(mapw, (MapW *) (mapw->map->widget), FALSE, TRUE);
    }
    if (mapw->rsw >= 0 && mapw->rsh >= 0) {
	VP *saved_vp = mapw->vp, tmpvp;

	/* Use this to see each rect update. */
	if (DebugG) {
	    XSetClipMask(mapw->display, mapw->gc, None);
	    XSetForeground(mapw->display, mapw->gc,
			   dside->ui->badcolor->pixel);
	    XFillRectangle(mapw->display, Tk_WindowId(tkwin), mapw->gc,
			   mapw->rsx - 1, mapw->rsy - 1,
			   mapw->rsw + 2, mapw->rsh + 2);
	}
	/* Redraw a restricted rect of the widget. */
	tmpvp = *(mapw->vp);
	if (mapw->rsw == 0)
	  mapw->rsw = 1;
	if (mapw->rsh == 0)
	  mapw->rsh = 1;
	set_view_size(&tmpvp, mapw->rsw, mapw->rsh);
	set_view_position(&tmpvp, tmpvp.sx + mapw->rsx, tmpvp.sy + mapw->rsy);
	mapw->vp = &tmpvp;
	mapw->d = Tk_GetPixmap(mapw->display, Tk_WindowId(tkwin),
			       mapw->rsw, mapw->rsh,
			       DefaultDepthOfScreen(Tk_Screen(tkwin)));
	Tk_Fill3DRectangle(tkwin, mapw->d, mapw->bg_border, 0, 0, mapw->rsw,
			   mapw->rsh, 0, TK_RELIEF_FLAT);
	XSetTSOrigin(mapw->display, mapw->gc, - mapw->rsx, - mapw->rsy);
	draw_map_widget(mapw);
	XSetTSOrigin(mapw->display, mapw->gc, 0, 0);
	/* Copy to the screen and release the pixmap. */
	XCopyArea(mapw->display, mapw->d, Tk_WindowId(tkwin), mapw->copygc,
		  0, 0,
		  (unsigned) mapw->rsw, (unsigned) mapw->rsh,
		  mapw->rsx, mapw->rsy);
	Tk_FreePixmap(mapw->display, mapw->d);
	mapw->vp = saved_vp;
    } else {
	/* Do a full redraw of the widget. */
	mapw->d = Tk_WindowId(tkwin);
	/* Create a pixmap for double-buffering if preferred. */
	if (mapw->double_buffer) {
	    mapw->d = Tk_GetPixmap(mapw->display, Tk_WindowId(tkwin),
				   winwidth, winheight,
				   DefaultDepthOfScreen(Tk_Screen(tkwin)));
	}
	/* Redraw the widget's background and border. */
	Tk_Fill3DRectangle(tkwin, mapw->d, mapw->bg_border, 0, 0, winwidth,
			   winheight, 0, TK_RELIEF_FLAT);
	draw_map_widget(mapw);
	/* If double-buffered, copy to the screen and release the pixmap. */
	if (mapw->double_buffer) {
	    XCopyArea(mapw->display, mapw->d, Tk_WindowId(tkwin), mapw->copygc,
		      0, 0,
		      (unsigned) winwidth, (unsigned) winheight,
		      0, 0);
	    Tk_FreePixmap(mapw->display, mapw->d);
	}
    }
    /* Redraw the map outline rect if we are drawing a worldmap. */
    if (mapw->world) {
	draw_map_outline(mapw, (MapW *) (mapw->map->widget), TRUE, TRUE);
    }
    mapw->rsx = mapw->rsy = mapw->rsw = mapw->rsh = -2;

}

#if 0
static void
set_scrollbars(MapW *mapw)
{
    float fsx, fsy, first, last, tot;

    fsx = (float) mapw->vp->sx;
    tot = (mapw->vp->sxmax - mapw->vp->sxmin + mapw->vp->pxw);
    first = (fsx - mapw->vp->sxmin) / tot;
    last = (fsx - mapw->vp->sxmin + mapw->vp->pxw) / tot;
    eval_tcl_cmd("map_xscroll_set %f %f", first, last);
    fsy = (float) mapw->vp->sy;
    tot = (mapw->vp->symax - mapw->vp->symin + mapw->vp->pxh);
    first = (fsy - mapw->vp->symin) / tot;
    last = (fsy - mapw->vp->symin + mapw->vp->pxh) / tot;
    eval_tcl_cmd("map_yscroll_set %f %f", first, last);
}
#endif

static void
mapw_destroy(char *ptr)
{
    MapW *mapw = (MapW *) ptr;

    Tk_FreeOptions(config_specs, (char *) mapw, mapw->display, 0);
    if (mapw->gc != None)
      XFreeGC(mapw->display, mapw->gc);
    if (mapw->copygc != None)
      XFreeGC(mapw->display, mapw->copygc);
    if (mapw->main_font != NULL)
      Tk_FreeFont(mapw->main_font);
    if (mapw->feature_font != NULL)
      Tk_FreeFont(mapw->feature_font);
    ckfree((char *) mapw);
}


/* Transform map coordinates into screen coordinates. */

static void
xform(MapW *mapw, int x, int y, int *sxp, int *syp)
{
    xform_cell(mapw->vp, x, y, sxp, syp);
}

/* Transform, but also use position within cell. */

static void
xform_fractional(MapW *mapw, int x, int y, int xf, int yf, int *sxp, int *syp)
{
    xform_cell_fractional(mapw->vp, x, y, xf, yf, sxp, syp);
}

static void
xform_top(MapW *mapw, int x, int y, int *sxp, int *syp)
{
    xform_cell_top(mapw->vp, x, y, sxp, syp);
}

static void
x_xform_unit(MapW *mapw, Unit *unit, int *sxp, int *syp, int *swp, int *shp)
{
    xform_unit(mapw->vp, unit, sxp, syp, swp, shp);
}

static void
x_xform_unit_self(MapW *mapw, Unit *unit, int *sxp, int *syp,
		  int *swp, int *shp)
{
    xform_unit_self(mapw->vp, unit, sxp, syp, swp, shp);
}

static int
x_nearest_cell(MapW *mapw, int sx, int sy, int *xp, int *yp)
{
    return nearest_cell(mapw->vp, sx, sy, xp, yp, NULL, NULL);
}

static Unit *
x_find_unit_or_occ(MapW *mapw, Unit *unit, int usx, int usy, int usw, int ush,
		   int sx, int sy)
{
    int usx1, usy1, usw1, ush1;
    Unit *occ, *rslt;

    /* See if the point might be over an occupant. */
    if (unit->occupant != NULL
	&& (side_controls_unit(dside, unit)
	    || mapw->vp->show_all
	    || u_see_occupants(unit->type)
	    || side_owns_occupant_of_unit(dside, unit))) {
	for_all_occupants(unit, occ) {
	    x_xform_unit(mapw, occ, &usx1, &usy1, &usw1, &ush1);
	    rslt =
	      x_find_unit_or_occ(mapw, occ, usx1, usy1, usw1, ush1, sx, sy);
	    if (rslt) {
		return rslt;
	    }
	}
    }
    /* Otherwise see if it could be the unit itself.  This has the effect of
       "giving" the transport everything in its box that is not in an occ. */
    x_xform_unit(mapw, unit, &usx1, &usy1, &usw1, &ush1);
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1)) {
	return unit;
    }
    return NULL;
}

static Unit *
x_find_unit_at(MapW *mapw, int x, int y, int sx, int sy)
{
    int usx, usy, usw, ush;
    Unit *unit, *rslt;
	
    for_all_stack(x, y, unit) {
	x_xform_unit(mapw, unit, &usx, &usy, &usw, &ush);
	rslt = x_find_unit_or_occ(mapw, unit, usx, usy, usw, ush, sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

static int
x_nearest_unit(MapW *mapw, int sx, int sy, Unit **unitp)
{
    int x, y;
    UnitView *uview = NULL;

#if (0)
    if (!x_nearest_cell(mapw, sx, sy, &x, &y)) {
	*unitp = NULL;
    } else if (mapw->vp->uw >= 32) {
	*unitp = x_find_unit_at(mapw, x, y, sx, sy);
    } else {
	*unitp = unit_at(x, y);
    }
    DGprintf("Pixel %d,%d -> unit %s\n", sx, sy, unit_desig(*unitp));
#endif
    nearest_unit_view(dside, mapw->vp, sx, sy, &uview);
    if (uview)
      *unitp = view_unit(uview);
    return TRUE;
}

/* Draw the background area for the map. */

static void
draw_area_background(MapW *mapw)
{
    int aw, ah, adjx, adjy, i;
    int llx, lly, lrx, lry, rx, ry, urx, ury, ulx, uly, lx, ly, numpoints;
    XPoint points[6];
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    XColor *color;

    XSetClipMask(dpy, gc, None);
    color = dside->ui->unseen_color;
    if (!grid_matches_unseen && 0 /* no unseen areas */)
      color = dside->ui->grid_color;
    XSetForeground(dpy, gc, color->pixel);

    /* Compute the shape to draw, which depends on the shape of the area. */
    aw = area.width;  ah = area.height;
    if (area.xwrap) {
	/* Area is cylinder; draw a parallelogram. */
	xform(mapw, 0, 0, &llx, &lly);
	points[0].x = llx;  points[0].y = lly;
	xform(mapw, aw - 1, 0, &lrx, &lry);
	points[1].x = lrx;  points[1].y = lry;
	xform(mapw, aw - 1, ah - 1, &urx, &ury);
	points[2].x = urx;  points[2].y = ury;
	xform(mapw, 0, ah - 1, &ulx, &uly);
	points[3].x = ulx;  points[3].y = uly;
	numpoints = 4;
    } else {
	/* Area is hexagon; draw a hexagon. */
	xform(mapw, 0 + ah / 2, 0, &llx, &lly);
	points[0].x = llx;  points[0].y = lly;
	xform(mapw, aw - 1, 0, &lrx, &lry);
	points[1].x = lrx;  points[1].y = lry;
	xform(mapw, aw - 1, ah / 2, &rx, &ry);
	points[2].x = rx;   points[2].y = ry;
	xform(mapw, aw - 1 - (ah - 1) / 2, ah - 1, &urx, &ury);
	points[3].x = urx;  points[3].y = ury;
	xform(mapw, 0, ah - 1, &ulx, &uly);
	points[4].x = ulx;  points[4].y = uly;
	xform(mapw, 0, ah / 2, &lx, &ly);
	points[5].x = lx;   points[5].y = ly;
	numpoints = 6;
    }
    /* Offset so polygon edges run through middles of cells. */
    if (mapw->vp->isometric) {
	adjx = mapw->vp->hh / 2;  adjy = mapw->vp->hw / 4;
    } else {
	adjx = mapw->vp->hw / 2;  adjy = mapw->vp->hh / 2;
    }
    for (i = 0; i < numpoints; ++i) {
	points[i].x += adjx;  points[i].y += adjy;
    }
    XFillPolygon(dpy, mapw->d, gc, points, numpoints, Convex, CoordModeOrigin);
    if (0 /* map bg matches widget bg */) {
	XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	XDrawLines(dpy, mapw->d, gc, points, numpoints + 1, CoordModeOrigin);
    }
}

/* Draw the view proper. */

static void
draw_map_widget(MapW *mapw)
{

    if (mapw->vp->isometric) {
	draw_map_isometric(mapw);
    } else {
	if (use_clip_mask)
	  draw_area_background(mapw);
	draw_map_overhead(mapw);
    }
    if (mapw->map->curunit)
      draw_current(mapw);
    if (mapw->blasttype >= 0)
      draw_blast_image(mapw,
		       mapw->blastsx - mapw->rsx, mapw->blastsy - mapw->rsy,
		       mapw->blastsw, mapw->blastsh, mapw->blasttype);
    /* In theory this shouldn't be necessary, but in practice the
       interface widgets (esp. the progress bar fill color) are
       affected by the last value of the foreground left over from map
       drawing, so set to a consistent value. */
    XSetForeground(mapw->display, mapw->gc, dside->ui->blackcolor->pixel);
    XSetBackground(mapw->display, mapw->gc, dside->ui->whitecolor->pixel);
}

static void
draw_map_overhead(MapW *mapw)
{
    int vylo, vyhi, y, x1, vx, len, t;

    if (mapw->vp->vcx < 0 || mapw->vp->vcy < 0) {
    	run_warning("doing a nasty hack");
	mapw->vp->vcx = mapw->vp->vcy = 2;
    }
    /* This is equivalent to the y calc in nearest_cell. */
    vylo = ((mapw->vp->totsh - (mapw->vp->sy + mapw->vp->pxh)) / mapw->vp->hch);
    /* Adjust downwards by a row, to cover overlap. */
    vylo -= 1;
    /* Now adjust the bottom row so it doesn't go outside the area. */
    vylo = limitn(0, vylo, area.height - 1);
    vyhi = ((mapw->vp->totsh - (mapw->vp->sy + 0)) / mapw->vp->hch);
    vyhi = limitn(0, vyhi, area.height - 1);
    /* Compute the leftmost "column" (but add back 1 to vylo first to fix the
       half-drawn cell scrolling bug). */
    vx = (mapw->vp->sx - ((vylo + 1) * mapw->vp->hw) / 2) / mapw->vp->hw;
    if (mapw->vp->draw_terrain) {
	for (y = vyhi; y >= vylo; --y) {
	    if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	      continue;
	    if (DebugG) {
		int tmpsx1, tmpsy1, tmpsx2, tmpsy2;

		xform(mapw, x1, y, &tmpsx1, &tmpsy1);
		xform(mapw, x1 + len - 1, y, &tmpsx2, &tmpsy2);
		tmpsx1 += 0;             tmpsy1 += mapw->vp->hh / 2;
		tmpsx2 += mapw->vp->hw;  tmpsy2 += mapw->vp->hh / 2;
		if (mapw->rsw >= 0) {
		    tmpsx1 += mapw->rsx;  tmpsy1 += mapw->rsy;
		    tmpsx2 += mapw->rsx;  tmpsy2 += mapw->rsy;
		}
		XSetClipMask(mapw->display, mapw->gc, None);
		XSetForeground(mapw->display, mapw->gc,
			       dside->ui->badcolor->pixel);
		XDrawLine(mapw->display, Tk_WindowId(mapw->tkwin), mapw->gc,
			  tmpsx1, tmpsy1, tmpsx2, tmpsy2);
	    }
	    draw_terrain_row(mapw, x1, y, len, FALSE);
	    if (between(4, mapw->vp->power, 6) 
	    	&& mapw->draw_transitions
	    	&& !poor_memory) {
		draw_terrain_transitions(mapw, x1, y, len, FALSE);
	    }
	}
	/* Maybe draw the grid as an overlay, if the usual "draw the
	   hex slightly smaller" algorithm won't work.  Note that we
	   want to draw before linear terrains, so they don't get
	   obscured by the grid. */
	if (mapw->draw_grid
	    && !grid_matches_unseen
	    && between(4, mapw->vp->power, 6)) {
	    for (y = vyhi; y >= vylo; --y) {
		if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
		  continue;
		draw_terrain_grid(mapw, x1, y, len);
	    }
	}
	/* Restore the fill style after tinkering with it for terrain. */
	XSetFillStyle(mapw->display, mapw->gc, FillSolid);
	if (any_aux_terrain_defined()) {
	    for_all_terrain_types(t) {
		if (t_is_border(t)
		    && aux_terrain_defined(t)
		    && bwid[mapw->vp->power] > 0) {
		    draw_borders(mapw, vx, vyhi, vylo, t);
		} else if (t_is_connection(t)
			   && aux_terrain_defined(t)
			   && cwid[mapw->vp->power] > 0) {
		    draw_connections(mapw, vx, vyhi, vylo, t);
		}
	    }
	}
	/* The relative ordering of these is quite important.  Note that
	   each should be prepared to run independently also, since the
	   other displays might have been turned off. */
	if (elevations_defined()
	    && mapw->vp->draw_elevations
	    && mapw->vp->angle == 90) {
	    for (y = vyhi; y >= vylo; --y) {
		if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
		  continue;
		draw_contours(mapw, x1, y, len);
	    }
	}
	/* Fuzz out the edges of the known area. */
	if (!mapw->vp->show_all
	    && !g_terrain_seen()  /* (should look at count of unseen cells) */
	    && between(4, mapw->vp->power, 6)
	    && !poor_memory) {
		draw_unseen_fuzz(mapw, vx, vyhi, vylo);
    	}
    }
    /* Now draw the lat-long grid if asked to do so. */
    if (mapw->vp->draw_meridians && mapw->vp->meridian_interval > 0)
      draw_meridians(mapw);
    for (y = vyhi; y >= vylo; --y) {
	if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	  continue;
	draw_row(mapw, x1, y, len);
    }
}

static void
draw_map_isometric(MapW *mapw)
{
    int x0, y0, x, y, d1, d2, dx, dy, left, ndir;
    int xbegin, ybegin, xrow, yrow, xrow2, yrow2;
    int j, k, numrows, rowlen;
    GC gc = mapw->gc;
    Display *dpy = mapw->display;

    /* Compute the horizontal offsets of each row.  For hexagons, the
       displacement is equivalent up and right, followed by down and
       right, which is the same as right_dir + right_dir(right_dir). */
    d1 = right_dir(mapw->vp->isodir);
    d2 = right_dir(d1);
    dx = (dirx[d1] + dirx[d2]) / abs(dirx[d1] + dirx[d2]);  
    dy = (diry[d1] + diry[d2]) / abs(diry[d1] + diry[d2]);
    /* Derive the number of rows and row length from the size of the
       map window, adding a little padding.  Each row consists of
       spaced hexes and is half of a hex width, so we need 4 times as
       many to cover the viewport. */
    /* The additional rows and length were heuristically determined by
       increasing until scrolling around never left any black holes.
       The numbers ought to be better explained though. */
    numrows = ((mapw->vp->pxh / mapw->vp->hw) * 4) + 8;
    rowlen = (mapw->vp->pxw / mapw->vp->hch) + 2;
    /* Get the cell at the top left corner (note that it may be
       outside the area). */
    nearest_cell(mapw->vp, 0, 0, &x0, &y0, NULL, NULL);
    /* Go up by one row. */
    xbegin = x0 + dirx[mapw->vp->isodir];
    ybegin = y0 + diry[mapw->vp->isodir];
    switch (mapw->vp->isodir) {
    	case	SOUTHEAST:
    	case	NORTHWEST:
		/* Shift the row to the left and increase its length
		by 1 cell to avoid black holes when scrolling. */ 
    		xbegin += dirx[left_dir(mapw->vp->isodir)];
    		ybegin += diry[left_dir(mapw->vp->isodir)];
    		rowlen += 1;
    		numrows += 5;
    		break;
    	case	WEST:
    	case	EAST:
		/* Adjust number of rows. */
    		numrows += area.width;
    		break;
    	case	SOUTHWEST:
    		/* Adjust start position and number of rows. */
    		ybegin -= area.width / 2;
    		numrows += area.width;
    		break;
    	case	NORTHEAST:
    		/* Adjust start position and number of rows. */
     		ybegin += area.width / 2;
    		numrows += area.width;
    		break;
    }
    xrow = xbegin;
    yrow = ybegin;
    left = TRUE;
    for (j = 0; j < numrows; ++j) {
	/* Draw a row of hexes. */
	x = xrow;  
	y = yrow;
	for (k = 0; k < rowlen; ++k) {
	    if (in_area(x, y)) {
		draw_terrain_iso(mapw, x, y);
		draw_row(mapw, x, y, 1);
	    }
	    x += dx;  
	    y += dy;
	}
	/* Alternately go either down and right, or down and left. */
	if (left) {
	    ndir = right_dir(opposite_dir(mapw->vp->isodir));
	} else {
	    ndir = left_dir(opposite_dir(mapw->vp->isodir));
	}
	left = !left;
	xrow2 = xrow + dirx[ndir];
	yrow2 = yrow + diry[ndir];
	xrow = xrow2;  
	yrow = yrow2;
    }
    XSetFillStyle(dpy, gc, FillSolid);
}

static void
draw_terrain_iso(MapW *mapw, int x, int y)
{
    int t, sx, sy, elev, elev1, drop, x1, y1, t1, dir, dir2;
    int sx1, sy1, sw1, sh1;
    int viewpow = mapw->vp->power;
    int viewdir = mapw->vp->isodir;
    int hw = mapw->vp->hw, hh = mapw->vp->hh;
    int style, over, coat;
    int drawsolid = FALSE;
    Image *timg, *subimg;
    TkImage *tkimg;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    Display *dpy = mapw->display;
    enum grayshade shade;
    XColor *color, *color2;

    style = cell_style(vp, x, y);
    if (style == dontdraw)
      return;
    t = cell_terrain(vp, x, y);
    over = cell_overlay(vp, x, y);
    coat = any_coating_at(x, y);
    cell_drawing_colors(t, coat, &color, &color2);
    if (t == NONTTYPE) {
	timg = best_image(unseen_image, hh, hw - hw / 4);
    } else {
    	timg = best_image(dside->ui->timages[t], hh, hw - hw / 4);
    }
    set_terrain_gc_for_image(mapw, gc, timg);
    xform(mapw, x, y, &sx, &sy);
    XSetClipOrigin(dpy, gc, sx, sy);
    if (mapw->draw_terrain_images) {
	int quasirand = abs((x * x + y * y ) % 101);

	subimg = timg;
	if (timg->numsubimages > 0 && timg->subimages) {
	    subimg = timg->subimages[quasirand % timg->numsubimages];
	}
	tkimg = (TkImage *) subimg->hook;
	if (subimg
	    && tkimg
	    && tkimg->colr != None
	    && !timg->istile) {
	    if (tkimg->mask != None) {
		XSetClipOrigin(dpy, gc, sx, sy - hw / 4);
		XSetClipMask(dpy, gc, tkimg->mask);
	    }
	    sx1 = sx;  
	    sy1 = sy - hw / 4;
	    sw1 = hh;  
	    sh1 = hw - hw / 4;
	    if (!use_clip_mask) {
		/* Draw a black mask underneath on Windows. */
		if (tkimg->mask != None) {
		    XSetFunction(dpy, gc, GXand);
		    XCopyArea(dpy, tkimg->mask, mapw->d, gc, 0, 0, 
			      sw1, sh1, sx1, sy1);			
		}
		XSetFunction(dpy, gc, GXor);
	    }
	    XCopyArea(dpy, tkimg->colr, mapw->d, gc, 0, 0, sw1, sh1, sx1, sy1);
	    if (!use_clip_mask) {
		    XSetFunction(dpy, gc, GXcopy);
	    }
	} else {
		drawsolid = TRUE;
	}
    } else {
	drawsolid = TRUE;
    }
    /* Restore clip origin and offset. */
    XSetClipOrigin(dpy, gc, sx, sy);
    sx1 = sx;  
    sy1 = sy;
    sw1 = hh;  
    sh1 = hw / 2;
    /* Handle the case of a solid-color terrain. */
    if (drawsolid) {
	XSetForeground(dpy, gc, color->pixel);
	if (use_clip_mask) {
	    /* Draw the cell proper. */
	    XSetClipMask(dpy, gc, dside->ui->hexisopics[viewpow]);
	    XFillRectangle(dpy, mapw->d, gc, sx1, sy1, sw1, sh1);
    	} else {
	    draw_iso_polygon(mapw, gc, sx1, sy1, viewpow, TRUE, over, 
			     coat, color2);
    	}
    } 
    /* Draw cliffs indicating elevation differences. */
    if (elevations_defined()) {
	elev = (elevations_defined() ? elev_at(x, y) : 0);
	XSetForeground(dpy, gc, color->pixel);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	for_all_directions(dir) {
	    dir2 = dir_subtract(dir, viewdir);
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		t1 = cell_terrain(vp, x1, y1);
		elev1 = (elevations_defined() ? elev_at(x1, y1) : 0);
	    } else {
		elev1 = area.minelev;
	    }
	    /* Compute the number of pixels between cell elevs. */
	    drop = (elev - elev1);
	    if (drop > 0) {
	    	drop *= mapw->vp->vertscale;
	    	drop = (drop * hh) / mapw->vp->cellwidth;
    		if (t == NONTTYPE) {
		    XSetForeground(dpy, gc, dside->ui->unseen_color->pixel);
		} else {
		    /* Note that we're shading independent of view dir. */
		    XSetForeground(dpy, gc,
			   (dside->ui->cell_shades[shading(dir)])[t]->pixel);
		}
		draw_cliff(mapw, sx, sy, dir2, drop,
			   dir2 == WEST
			   || dir2 == SOUTHWEST
			   || dir2 == SOUTHEAST);
	    }
	}
    } 
    /* We don't want coatings, shading, borders or connections on top of 
       unseen terrain. */
    if (t == NONTTYPE) {
    	XSetFillStyle(mapw->display, mapw->gc, FillSolid);
	return;
    }
    /* Apply any coating */
    if (coat != NONTTYPE) {
	if (use_clip_mask) {
	    XSetFillStyle(dpy, gc, FillStippled);
	    XSetStipple(dpy, gc, dside->ui->grays[gray]);
	    XSetForeground(dpy, gc, color2->pixel);
	    XSetClipMask(dpy, gc, dside->ui->hexisopics[viewpow]);
	    XFillRectangle(dpy, mapw->d, gc, sx1, sy1, sw1, sh1);
    	} else {
	    draw_iso_polygon(mapw, gc, sx1, sy1, viewpow, FALSE, over, 
			     coat, color2);
    	}
    }
    /* Apply any overlay shading. */
    if (over < 0) {
	if (use_clip_mask) {
	    shade = GRAY(over);
	    XSetFillStyle(dpy, gc, FillStippled);
	    XSetStipple(dpy, gc, dside->ui->grays[shade]);
	    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	    XSetClipMask(dpy, gc, dside->ui->hexisopics[viewpow]);
	    XFillRectangle(dpy, mapw->d, gc, sx1, sy1, sw1, sh1);
    	} else {
	    draw_iso_polygon(mapw, gc, sx1, sy1, viewpow, FALSE, over, 
			     coat, color2);
    	}
    }
    /* The draw the grid if asked for and we lack cliffs. */
    if (!elevations_defined()
        && mapw->vp->draw_grid 
        && mapw->vp->hh > 10) {
	xform_top(mapw, x, y, &sx, &sy);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	XSetForeground(dpy, gc, dside->ui->grid_color->pixel);
	for_all_directions(dir) {
	    dir2 = dir_subtract(dir, viewdir);
	    XDrawLine(dpy, mapw->d, gc,
		      sx + ibsx[viewpow][dir2], sy + ibsy[viewpow][dir2],
		      sx + ibsx[viewpow][dir2+1], sy + ibsy[viewpow][dir2+1]);
	}
    }
    /* Restore the fill style after tinkering with it for terrain. */
    XSetFillStyle(mapw->display, mapw->gc, FillSolid);
    if (any_aux_terrain_defined()) {
	for_all_terrain_types(t) {
	    if (t_is_border(t)
		&& aux_terrain_defined(t)
		&& bwid[mapw->vp->power] > 0) {
		draw_borders_iso(mapw, x, y, t);
	    } else if (t_is_connection(t)
		       && aux_terrain_defined(t)
		       && cwid[mapw->vp->power] > 0) {
		draw_connections_iso(mapw, x, y, t);
	    }
	}
    }
}

static void
draw_cliff(MapW *mapw, int sx, int sy, int dir, int drop, int face)
{
    int viewpow = mapw->vp->power;
    GC gc = mapw->gc;
    Display *dpy = mapw->display;
    XPoint points[4];

    /* Add a little slop. */
    ++drop;
    points[0].x = sx + ibsx[viewpow][dir];  
    points[0].y = sy + ibsy[viewpow][dir];
    points[1].x = sx + ibsx[viewpow][dir+1];  
    points[1].y = sy + ibsy[viewpow][dir+1];
    points[2].x = points[1].x;  points[2].y = points[1].y + drop;
    points[3].x = points[0].x;  points[3].y = points[0].y + drop;
    if (face) {
	XFillPolygon(dpy, mapw->d, gc, points, 4, Convex, CoordModeOrigin);
    } else {
	XDrawLine(dpy, mapw->d, gc,
		points[0].x, points[0].y, points[1].x, points[1].y);
    }
}

static int
shading(int dir)
{
    switch (dir) {
	case NORTHEAST:
	  return 3;
	case EAST:
	  return 4;
	case SOUTHEAST:
	  return 3;
	case SOUTHWEST:
	  return 1;
	case WEST:
	  return 0;
	case NORTHWEST:
	  return 1;
    }
    return 1;
}

/* Temporary stashes for the meridian drawing callbacks. */

static MapW *tmpmapw;

/* Draw latitude and longitude lines & labels. */

static void
draw_meridians(MapW *mapw)
{
    Display *dpy = mapw->display;

    XSetClipMask(dpy, mapw->gc, None);
    XSetForeground(dpy, mapw->gc, dside->ui->meridian_color->pixel);
    XSetBackground(dpy, mapw->gc, dside->ui->bgcolor->pixel);
    tmpmapw = mapw;
    plot_meridians(mapw->vp, meridian_line_callback, meridian_text_callback);
}

static void
meridian_line_callback(int x1, int y1, int x1f, int y1f,
		       int x2, int y2, int x2f, int y2f)
{
    int sx1, sy1, sx2, sy2;

    xform_fractional(tmpmapw, x1, y1, x1f, y1f, &sx1, &sy1);
    xform_fractional(tmpmapw, x2, y2, x2f, y2f, &sx2, &sy2);
    XDrawLine(tmpmapw->display, tmpmapw->d, tmpmapw->gc, sx1, sy1, sx2, sy2);
}

static void
meridian_text_callback(int x1, int y1, int x1f, int y1f, char *str)
{
    int sx1, sy1;

    xform_fractional(tmpmapw, x1, y1, x1f, y1f, &sx1, &sy1);
    XSetClipMask(tmpmapw->display, tmpmapw->gc, None);
    XSetForeground(tmpmapw->display, tmpmapw->gc, dside->ui->fgcolor->pixel);
    Tk_DrawChars(tmpmapw->display, tmpmapw->d, tmpmapw->gc, tmpmapw->main_font,
		 str, strlen(str), sx1 + 1, sy1 + 1);
}

/* The basic map drawing routine does an entire row at a time, which yields
   order-of-magnitude speedups. */

static void
draw_row(MapW *mapw, int x0, int y0, int len)
{
    int x, i, namelength = 0;
    Unit *unit;

    if (clouds_defined() && mapw->vp->draw_clouds) {
	draw_clouds_row(mapw, x0, y0, len);
    }
    if (temperatures_defined() && mapw->vp->draw_temperature 
	&& mapw->vp->hw > 10) {
	draw_temperature_row(mapw, x0, y0, len);
    }
    if (winds_defined() && mapw->vp->draw_winds && mapw->vp->hw > 10) {
	draw_winds_row(mapw, x0, y0, len);
    }
#if 0
    /* Skip the top and bottom rows if they are edge rows. */
    if (!between(1, y0, area.height - 2))
      return;
    /* Skip the rightmost and leftmost cells if on the edge. */
    if (!inside_area(x0 + len - 1, y0))
      --len;
    if (!inside_area(x0, y0)) {
	++x0;
	--len;
    }
    if (len <= 0)
      return;
#endif
    /* Draw things that only appear on interior cells. */
    if (features_defined() 
        && mapw->vp->draw_feature_boundaries) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_feature_boundary(mapw, x, y0);
	}
    }
    if (features_defined() 
        && mapw->vp->draw_feature_names 
        && dside->ui->legends
        && mapw->vp->hw > 4) {
	for (i = 0; i < numfeatures; ++i) {
	    if (dside->ui->legends[i].oy == y0) {
		draw_feature_name(mapw, i);
	    }
	}
    }
    /* Draw sparse things on top of the basic row. */
    if (((people_sides_defined() && mapw->vp->draw_people)
	 || (control_sides_defined() && mapw->vp->draw_control))
	&& mapw->vp->hw >= 8) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_people(mapw, x, y0);
	}
    }
    /* Draw units. */
    /* (should do names separately, to prevent overlap problems.) */
    if (mapw->vp->draw_units) {
	/* First redraw named units W of (x0, y0) to ensure that the
	names are not erased during scrolling. */
	if (mapw->vp->draw_names && mapw->vp->power > 2) {
	    if (mapw->vp->power == 3)
		namelength = 11;
	    if (mapw->vp->power == 4)
		namelength = 6;
	    if (mapw->vp->power == 5)
		namelength = 4;
	    if (mapw->vp->power == 6)
		namelength = 3;
	    for (i = x0 - namelength; i < x0; i++) {
		if (!inside_area(i, y0)) {
		    continue;
		}
		for_all_stack(i, y0, unit) {
		    if (unit->name) {
			draw_uviews_in_cell(mapw, i, y0);
			/* Only do this once for each cell. */
			break;
		    }
		}
	    }
	}
	/* Then draw any units in the row. */
	for (x = x0; x < x0 + len; ++x) {
	    draw_uviews_in_cell(mapw, x, y0);
	}
    }
    /* Draw resources, such as usage by a city in Civ-type games. */
    /* (should this be controlled by vp->draw_units or not?) */
    if (any_resources) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_resource_usage(mapw, x, y0);
	}
    }
    if (mapw->vp->draw_ai) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_ai_region(mapw, x, y0);
	}
    }
}

char buffer[BUFSIZE];

static void
draw_feature_name(MapW *mapw, int f)
{
    Legend *legend = &dside->ui->legends[f];
    int x = legend->ox, y = legend->oy;
    int dist = ((legend->dx + 1) * mapw->vp->hw * 9) / 10;
    int sx0, sy0, sxc, syc, namelength = 0;
    char *name;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    Tk_Font tkfont;

    name = feature_desc(find_feature(f + 1), buffer);
    if (empty_string(name))
      return;

    /* xform returns coordinates of the upper-left corner of the cell */
    xform(mapw, x, y, &sx0, &sy0);

    /* Start east of feature if it is a single cell, 
       else center on top of it. */
    if (find_feature(f +1)->size == 1) {
    	sxc = sx0 + mapw->vp->hw;
    } else {
    	sxc = sx0 + legend->dx  * mapw->vp->hw / 2;
    }
    /* Tweak y position to minimize overlap with unit names. */
    syc  = sy0 + mapw->vp->hh * 3 / 8;					

    /* Find max length of a feature's name. */
    if (mapw->vp->draw_names && mapw->vp->power > 2) {
	if (mapw->vp->power == 3)
	    namelength = 11;
	if (mapw->vp->power == 4)
	    namelength = 6;
	if (mapw->vp->power == 5)
	    namelength = 4;
	if (mapw->vp->power == 6)
	    namelength = 3;
    }
    /* A hack - should do real centering like in the MacPPC interface. */
    if (find_feature(f +1)->size > 1) {
    	sxc -= namelength * mapw->vp->hw / 4;
    }
    /* Add namelength to cutoff margin to ensure that feature names that 
    are scrolled into view are drawn. */
    if (sxc + dist + namelength * mapw->vp->hw < 0)
      return;
    XSetClipMask(dpy, gc, None);
    /* Draw single cell features with the same font sizes as unit names. */
    if (find_feature(f +1)->size == 1) {
    	tkfont = mapw->main_font;
    /* Draw other features using the larger feature font. */
    } else {
    	tkfont = mapw->feature_font;
    }
    XSetFont(dpy, gc, Tk_FontId(tkfont));
    /* Draw a black shadow, unless the color of the text is to be
       black. */
    if (dside->ui->unit_name_color != dside->ui->blackcolor) {
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	/* This may seem excessive but it's the only way to make shadows look
	really good for all characters and numbers. */
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 1, syc);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 2, syc);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc, syc + 1);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc, syc + 2);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 1, syc + 1);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 1, syc + 2);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 2, syc + 1);
	Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc + 2, syc + 2);
    }
    XSetForeground(dpy, gc, dside->ui->feature_color->pixel);
    Tk_DrawChars(dpy, mapw->d, gc, tkfont, name, strlen(name),
		 sxc, syc);
}

static void
cell_drawing_colors(int t, int coat, XColor **colorp, XColor **color2p)
{
	*colorp = NULL;
	*colorp = dside->ui->cell_color[t];
	if (*colorp == NULL)
	  *colorp = dside->ui->blackcolor;
	if (t == NONTTYPE)
	  *colorp = dside->ui->unseen_color;
	if (t == BACKTTYPE)
	  *colorp = dside->ui->window_color;
	*color2p = NULL;
	if (coat != NONTTYPE) {
	    *color2p = dside->ui->cell_color[coat];
	    if (*color2p == NULL)
	      *color2p = dside->ui->whitecolor;
	}
}

/* Given a terrain image, set up the GC for tiling, stippling, or solid
   color, as determined by the image and desired display style. */

static void
set_terrain_gc_for_image(MapW *mapw, GC gc, Image *timg)
{
    TkImage *tkimg;
    Display *dpy = mapw->display;

    if (timg != NULL && timg->istile && mapw->draw_terrain_patterns) {
	tkimg = (TkImage *) timg->hook;
	if (tkimg != NULL) {
	    if (tkimg->colr != None) {
		XSetFillStyle(dpy, gc, FillTiled);
		XSetTile(dpy, gc, tkimg->colr);
		return;
	    } else if (tkimg->mono != None) {
		XSetFillStyle(dpy, gc, FillOpaqueStippled);
		XSetStipple(dpy, gc, tkimg->mono);
		return;
	    }
	}
    }
    /* The fallback case. */
    XSetFillStyle(dpy, gc, FillSolid);
}

/* This interfaces higher-level drawing decisions to the rendition of
   individual pieces of display.  The rendering technique chosen
   depends on what the init code has decided is appropriate given what
   it found during init and what magnification the display is at.

   This routine is performance-critical; any improvements will
   probably have a noticeable effect on the display.  But also note
   that X's main bottleneck is the network connection, so it's more
   useful to eliminate roundtrips to the server than anything else. */

/* (should draw overlay in a separate pass?) */

static void
draw_terrain_row(MapW *mapw, int x0, int y0, int len, int force)
{
    VP *vp = mapw->vp;
    int x, x1, t, sx, sy, i = 0, j;
    int w = vp->hw, h = vp->hh, p = vp->power;
    int dogrid = (vp->draw_grid && grid_matches_unseen);
    int style, segstyle, terr, segterr, over, segover, coat, segcoat;
    int drawsolid;
    XColor *color, *segcolor, *color2, *segcolor2;
    Image *timg, *subimg;
    GC gc = mapw->gc;
    Display *dpy = mapw->display;
    enum grayshade shade;

    x1 = x0;
    segstyle = cell_style(vp, x0, y0);
    segterr = cell_terrain(vp, x0, y0);
    segover = cell_overlay(vp, x0, y0);
    segcoat = any_coating_at(x0, y0);
    cell_drawing_colors(segterr, segcoat, &segcolor, &segcolor2);
    for (x = x0; x < x0 + len + 1; ++x) {
	style = cell_style(vp, x, y0);
	terr = cell_terrain(vp, x, y0);
	over = cell_overlay(vp, x, y0);
	coat = any_coating_at(x, y0);
	cell_drawing_colors(terr, coat, &color, &color2);
	/* Decide if the run is over and we need to dump some output. */
	if (x == x0 + len
	    || x == area.width
	    || style != segstyle
	    || terr != segterr
	    || over != segover
	    || coat != segcoat
	    || color != segcolor
	    || color2 != segcolor2
	    || segstyle == usepolygons
	    || force) {
	    /* Note: we might end up drawing something that matches
	       the background color, which wastes time, but apparently
	       the test "(segdrawmethod != dontdraw && segcolor !=
	       dside->ui->bgcolor)" is not completely sufficient.
	       (should figure this one out sometime) */
	    t = cell_terrain(vp, x1, y0);
	    if (t == NONTTYPE) {
		timg = dside->ui->best_unseen_images[p];
	    } else {
		timg = (dside->ui->best_timages[p])[t];
	    }
	    xform(mapw, x1, y0, &sx, &sy);
	    XSetForeground(dpy, gc, segcolor->pixel);
	    switch (segstyle) {
	      case dontdraw:
		/* Don't do anything. */
		break;
	      case useblocks:
		XSetClipMask(dpy, gc, None);
		set_terrain_gc_for_image(mapw, gc, timg);
		XFillRectangle(dpy, mapw->d, gc, sx, sy, i * w, h);
		/* We don't want coatings or shading on top of unseen 
		   terrain. */
		if (t == NONTTYPE) {
		    break;
		}
		if (segcoat != NONTTYPE) {
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, dside->ui->grays[gray]);
		    XSetForeground(dpy, gc, segcolor2->pixel);
		    XFillRectangle(dpy, mapw->d, gc, sx, sy, i * w, h);
		}
		if (segover < 0) {
		    shade = GRAY(segover);
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, dside->ui->grays[shade]);
		    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		    XFillRectangle(dpy, mapw->d, gc, sx, sy, i * w, h);
		}
		break;
	      case usepolygons:
		if (use_clip_mask) {
		    if (dogrid) {
			XSetClipMask(dpy, gc, dside->ui->bhexpics[p]);
		    } else {
			XSetClipMask(dpy, gc, dside->ui->hexpics[p]);
		    }
		}
		set_terrain_gc_for_image(mapw, gc, timg);
		drawsolid = FALSE;
		for (j = 0; j < i; ++j) {
			int quasirand = 
			    abs(((x1 + j) * (x1 + j) + y0 * y0 ) % 101);
			TkImage *tkimg;

			xform(mapw, x1 + j, y0, &sx, &sy);
			if (use_clip_mask) {
			    XSetClipOrigin(dpy, gc, sx, sy);
			}
			if (mapw->draw_terrain_images) {
			    subimg = timg;
			    if (timg->numsubimages > 0 && timg->subimages) {
				subimg = timg->subimages[quasirand % 
							 timg->numsubimages];
			    }
			    tkimg = (TkImage *) subimg->hook;
			    if (subimg
				&& tkimg
				&& tkimg->colr != None
				&& !timg->istile) {
				if (!use_clip_mask) {
				    /* If this is an edge cell we need to draw 
				       a black mask underneath it on Windows 
				       since the area background is partly 
				       missing. */
				    /*! \bug Does not work for wraparound map. 
					     Believe it or not, it would seem 
					     that the 'inside_area' macro may 
					     be suspect. In the meantime, 
					     we simply have to live with 
					     twice as many blits on Windows. */
#if (0)
				    if (!inside_area(x1 + j, y0)
					&& tkimg->mask != None) {
#endif
					XSetFunction(dpy, gc, GXand);
					XCopyArea(dpy, tkimg->mask, mapw->d, 
						  gc, 0, 0, w, h, sx, sy);
#if (0)
				    }
#endif
				    XSetFunction(dpy, gc, GXor);
				}
				XCopyArea(dpy, tkimg->colr, mapw->d, gc,
					  0, 0, w, h, sx, sy);
				if (!use_clip_mask) {
				    XSetFunction(dpy, gc, GXcopy);
				}
			    } else {
				drawsolid = TRUE;
			    }
			} else {
			    drawsolid = TRUE;
			}
			/* Handle the case of a solid-color terrain. */
			if (drawsolid) {
			    if (use_clip_mask) {
				XFillRectangle(dpy, mapw->d, gc, sx, sy, w, h);
			    } else {
				draw_hex_polygon(mapw, gc, sx, sy, p, segover, 
						 segcoat, segcolor2, dogrid);
			    }
			}
		}
		/* We don't want coatings or shading on top of 
		   unseen terrain. */
		if (t == NONTTYPE) {
		    XSetFillStyle(mapw->display, mapw->gc, FillSolid);
		    break;
		}
		if (segcoat != NONTTYPE) {
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, dside->ui->grays[gray]);
		    XSetForeground(dpy, gc, segcolor2->pixel);
		    for (j = 0; j < i; ++j) {
			xform(mapw, x1 + j, y0, &sx, &sy);
			if (use_clip_mask) {
			    XSetClipOrigin(dpy, gc, sx, sy);
			    XFillRectangle(dpy, mapw->d, gc, sx, sy, w, h);
			} else {
			    draw_hex_polygon(mapw, gc, sx, sy, p, segover, 
					     segcoat, segcolor2, dogrid);
			}
		    }
		}
		if (segover < 0) {
		    shade = GRAY(segover);
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, dside->ui->grays[shade]);
		    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		    for (j = 0; j < i; ++j) {
			xform(mapw, x1 + j, y0, &sx, &sy);
			if (use_clip_mask) {
			    XSetClipOrigin(dpy, gc, sx, sy);
			    XFillRectangle(dpy, mapw->d, gc, sx, sy, w, h);
			} else {
			    draw_hex_polygon(mapw, gc, sx, sy, p, segover, 
					     segcoat, segcolor2, dogrid);
			}
		    }
		}
		break;
	    }
	    /* Reset everything for the next run. */
	    i = 0;
	    x1 = x;
	    segstyle = style;
	    segterr = terr;
	    segover = over;
	    segcoat = coat;
	    segcolor = color;
	    segcolor2 = color2;
	}
	++i;
    }
}

/* Scrath pads for pixmap gymnastics on Windows. */

static Pixmap maskmap = None;
static Pixmap colormap = None;

static void
draw_terrain_transitions(MapW *mapw, int x0, int y0, int len, int force)
{
    int x1, y1, x, t, dir, sx, sy, fillsolid;
    int sx2, sy2, sw, sh, offset, t1, rslt, quasirand;
    int mainstyle, mainover, adjover, maincoat, adjcoat;
    XColor *maincolor, *maincolor2, *adjcolor, *adjcolor2;
    Image *timg, *subimg;
    Image *trimg, *subtrimg;
    TkImage *tktrimg, *tkimg;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    Display *dpy = mapw->display;
    enum grayshade shade;

    trimg = best_image(generic_transition, mapw->vp->hw, mapw->vp->hh);
    if (trimg == NULL)
      return;

    /* Init the scratch pads for Windows. */
    if (!use_clip_mask
        && maskmap == None) {
	maskmap = Tk_GetPixmap(dpy, Tk_WindowId(mapw->tkwin), 88, 96, 
			       DefaultDepthOfScreen(Tk_Screen(mapw->tkwin)));
	colormap = Tk_GetPixmap(dpy, Tk_WindowId(mapw->tkwin), 88, 96, 
				DefaultDepthOfScreen(Tk_Screen(mapw->tkwin)));
    }
    for (x = x0; x < x0 + len + 1; ++x) {
	mainstyle = cell_style(vp, x, y0);
	t = cell_terrain(vp, x, y0);
	mainover = cell_overlay(vp, x, y0);
	maincoat = any_coating_at(x, y0);
	cell_drawing_colors(t, maincoat, &maincolor, &maincolor2);
	if (x == x0 + len
	    || x == area.width
	    || mainstyle == dontdraw)
	  continue;
	for_all_directions(dir) {
	    rslt = compute_transition(dside, mapw->vp, x, y0, dir,
				      &sx2, &sy2, &sw, &sh, &offset);
	    if (!rslt)
	      continue;
#if 0	   /* for debugging */
	    XSetClipMask(dpy, gc, None);
	    XSetFillStyle(dpy, gc, FillSolid);
	    XSetForeground(dpy, gc, dside->ui->badcolor->pixel);
	    XDrawRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
#endif
	    quasirand = abs((x * x + y0 * y0 ) % 101);
	    subtrimg = trimg->subimages[(quasirand % 4) * 4 + offset];
	    tktrimg = (TkImage *) subtrimg->hook;
	    xform(mapw, x, y0, &sx, &sy);
	    /* We already know the result, just need x1,y1 */
	    point_in_dir(x, y0, dir, &x1, &y1);
	    t1 = cell_terrain(vp, x1, y1);
	    adjover = cell_overlay(vp, x1, y1);
	    adjcoat = any_coating_at(x1, y1);
	    cell_drawing_colors(t1, adjcoat, &adjcolor, &adjcolor2);
	    /* Draw a bit of shadow along coastlines. */
	    /* (should just be a default, 
		for when no coastline imagery available) */
	    if (use_clip_mask
		&& t_liquid(t) != t_liquid(t1)
		&& between(SOUTHWEST, dir, EAST)) {
		XSetClipOrigin(dpy, gc,
			       sx - 4, sy - (dir == EAST ? 0 : 1));
		XSetFillStyle(dpy, gc, FillSolid);
		XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		XFillRectangle(dpy, mapw->d, gc,
			       sx2 - 4, sy2 - (dir == EAST ? 0 : 1),
			       sw, sh);
	    }
	    if (use_clip_mask) {
		XSetClipOrigin(dpy, gc, sx, sy);
		XSetClipMask(dpy, gc, tktrimg->mask);
	    } else {
		XSetFunction(dpy, gc, GXand);
		XCopyArea(dpy, tktrimg->mask, mapw->d, gc,
				  sx2 - sx, sy2 - sy, sw, sh, sx2, sy2);
		XSetFunction(dpy, gc, GXcopy);
	    }  
	    fillsolid = FALSE;
	    if (mapw->draw_terrain_images) {
		timg = best_image(dside->ui->timages[t1], 
				  mapw->vp->hw, mapw->vp->hh);
		set_terrain_gc_for_image(mapw, gc, timg);
		subimg = timg;
		if (timg->numsubimages > 0 && timg->subimages) {
		    subimg = timg->subimages[quasirand % timg->numsubimages];
		}
		tkimg = (TkImage *) subimg->hook;
		if (subimg
		    && tkimg
		    && tkimg->colr != None
		    && !timg->istile) {
		    if (use_clip_mask) {
			XCopyArea(dpy, tkimg->colr, mapw->d, gc,
				  sx2 - sx, sy2 - sy, sw, sh, sx2, sy2);
		    } else {
			/* Copy the INVERTED mask into maskmap. */
			XSetFunction(dpy, gc, GXcopyInverted);
			XCopyArea(dpy, tktrimg->mask, maskmap, gc,
				  sx2 - sx, sy2 - sy, sw, sh, 0, 0);
			/* AND in the terrain image. */
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->colr, maskmap, gc,
				  sx2 - sx, sy2 - sy, sw, sh, 0, 0);
			/* OR the result into the map. */
			XSetFunction(dpy, gc, GXor);
			XCopyArea(dpy, maskmap, mapw->d, gc,
				  0, 0, sw, sh, sx2, sy2);
			XSetFunction(dpy, gc, GXcopy);
		    }
		} else if (use_clip_mask
		    && timg->istile
		    && mapw->draw_terrain_patterns) {
		    XSetForeground(dpy, gc, adjcolor->pixel);
		    XFillRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
		} else {
		    fillsolid = TRUE;
		}
	    } else {
		if (use_clip_mask
		    && timg
		    && timg->istile
		    && mapw->draw_terrain_patterns) {
			XSetForeground(dpy, gc, adjcolor->pixel);
			XFillRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
		} else {
			fillsolid = TRUE;
		}
	    }
	    if (fillsolid) {
		if (use_clip_mask) {
		    XSetFillStyle(dpy, gc, FillSolid);
		    XSetForeground(dpy, gc, adjcolor->pixel);
		    XFillRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
	    	} else {
		    /* Fill colormap with color. */
		    XSetFillStyle(dpy, gc, FillSolid);
		    XSetForeground(dpy, gc, adjcolor->pixel);
		    XFillRectangle(dpy, colormap, gc, 0, 0, sw, sh);
		    /* Copy the INVERTED mask into maskmap. */
		    XSetFunction(dpy, gc, GXcopyInverted);
		    XCopyArea(dpy, tktrimg->mask, maskmap, gc,
			      sx2 - sx, sy2 - sy, sw, sh, 0, 0);
		    /* AND in colormap. */
		    XSetFunction(dpy, gc, GXand);
		    XCopyArea(dpy, colormap, maskmap, gc,
			      0, 0, sw, sh, 0, 0);			
		    /* OR the result into the map. */
		    XSetFunction(dpy, gc, GXor);
		    XCopyArea(dpy, maskmap, mapw->d, gc,
			      0, 0, sw, sh, sx2, sy2);
		    /* Offset 1 pixel and repeat to fix display glitch 
		    (should figure out why the pixels don't line up). */
		    XCopyArea(dpy, maskmap, mapw->d, gc,
			      0, 0, sw, sh, sx2, sy2 + 1);
		    XSetFunction(dpy, gc, GXcopy);
	    	}
	    }
	    if (adjcoat != NONTTYPE) {
		/* (assumes use_clip_mask) */
		XSetFillStyle(dpy, gc, FillStippled);
		XSetStipple(dpy, gc, dside->ui->grays[gray]);
		XSetForeground(dpy, gc, adjcolor2->pixel);
		XFillRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
	    }
	    if (adjover < 0) {
		/* (assumes use_clip_mask) */
		shade = GRAY(adjover);
		XSetFillStyle(dpy, gc, FillStippled);
		XSetStipple(dpy, gc, dside->ui->grays[shade]);
		XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		XFillRectangle(dpy, mapw->d, gc, sx2, sy2, sw, sh);
	    }
	}
    }
}

/* Draw a single row of explicit grid lines.  Explicit lines are
   necessary when the grid and unseen colors are different. */
/* (and possibly other times - tests for "leaky_grid" in the Mac
   interface solve more-or-less the same problem - should merge?) */

static void
draw_terrain_grid(MapW *mapw, int x0, int y0, int len)
{
    int x1, y1, x, dir, sx, sy;
    int dx1, dy1, dx2, dy2;
    int viewpow = mapw->vp->power;
    GC gc = mapw->gc;
    Display *dpy = mapw->display;

    /* Set pen size explicitly in case it was not restored by some
    other line drawing code. */
    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter); 
    XSetClipMask(dpy, gc, None);
    XSetFillStyle(dpy, gc, FillSolid);
    XSetForeground(dpy, gc, dside->ui->grid_color->pixel);

    for (x = x0; x < x0 + len + 1; ++x) {
	if (x == x0 + len || x == area.width)
	  continue;
	if (!terrain_visible(dside, x, y0))
	  continue;
	xform(mapw, x, y0, &sx, &sy);
	for_all_directions(dir) {
	    if (dir == NORTHEAST || dir == NORTHWEST || dir == WEST)
	      continue;
	    if (!point_in_dir(x, y0, dir, &x1, &y1))
	      continue;
	    if (!terrain_visible(dside, x1, y1))
	      continue;
	    /* Calculate start point. */
	    dx1 = bsx[viewpow][dir];  dy1 = bsy[viewpow][dir];
	    if (dir == WEST || dir == EAST)
	      dx1 -= 1;
	    /* Calculate end point. */
	    dx2 = bsx[viewpow][dir+1];  dy2 = bsy[viewpow][dir+1];
	    if (dir == WEST || dir == EAST)
	      dx2 -= 1;

	    XDrawLine(dpy, mapw->d, gc,
		      sx + dx1, sy + dy1 - 1, sx + dx2, sy + dy2 - 1);
	}
    }
}

/* Draw the fuzzed-out edges of the seen terrain all at once, using the
   same general algorithm as for borders, but testing seen-ness. */

static void
draw_unseen_fuzz(MapW *mapw, int vx, int vyhi, int vylo)
{
    int x, y, x1, len, bitmask, sx, sy, halfside, subi;
    int xw, yw, xne, yne, xnw, ynw;
    int hw = mapw->vp->hw, hch = mapw->vp->hch;
    int power = mapw->vp->power;
    Display *dpy = mapw->display;
    Image *timg, *subimg;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    TkImage *tkimg;
    
    if (use_clip_mask) {
	XSetFillStyle(dpy, gc, FillSolid);
	XSetForeground(dpy, gc, dside->ui->unseen_color->pixel);
	XSetClipMask(dpy, gc, fuzzpics[power]);	    
    } else {
	timg = best_image(generic_fuzz, hws[power], hhs[power]);
	if (timg == NULL) {
	    return;
    	}
    }
    halfside = halfsides[power];
    for (y = vyhi; y >= vylo; --y) {
	if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	  continue;
	for (x = x1; x < x1 + len; ++x) {
	    /* Compute coordinates of adjacent cells. */
	    if (!point_in_dir(x, y, NORTHEAST, &xne, &yne))
	      continue;
	    if (!point_in_dir(x, y, NORTHWEST, &xnw, &ynw))
	      continue;
	    if (!point_in_dir(x, y, WEST, &xw, &yw))
	      continue;
	    xform(mapw, x, y, &sx, &sy);
	    /* Draw the junction at the top of the x,y hex. */
	    bitmask = 0;
	    if (m_terrain_visible(vp, xne, yne)
		!= m_terrain_visible(vp, xnw, ynw))
	      bitmask |= 1;
	    if (m_terrain_visible(vp, x, y)
		!= m_terrain_visible(vp, xne, yne))
	      bitmask |= 2;
	    if (m_terrain_visible(vp, x, y)
		!= m_terrain_visible(vp, xnw, ynw))
	      bitmask |= 4;
	    if (bitmask != 0) {
	    	if (use_clip_mask) {
			XSetClipOrigin(dpy, gc,
				       sx - (bitmask & 3) * hw,
				       sy - halfside - 
					((bitmask & 4) ? 3 : 1) * hch);
			XFillRectangle(dpy, mapw->d, gc, sx, sy - halfside, 
				       hw, hch);
		} else {
			subi = (bitmask & 3) + ((bitmask & 4) ? 12 : 4);
			if (timg->numsubimages > 0 && timg->subimages)
			    subimg = timg->subimages[subi];
			if (subimg && subimg->hook)
			    tkimg = (TkImage *) subimg->hook;
			if (tkimg && tkimg->mask != None) {
				XSetFunction(dpy, gc, GXand);
				XCopyArea(dpy, tkimg->mask, mapw->d, gc,
					  0, 0, hws[power], hcs[power], 
					  sx, sy - halfside);
				XSetFunction(dpy, gc, GXcopy);
			}
		}
	    }
	    /* Draw the junction at the top left corner of the x,y hex. */
	    bitmask = 0;
	    if (m_terrain_visible(vp, x, y)
		!= m_terrain_visible(vp, xw, yw))
	      bitmask |= 1;
	    if (m_terrain_visible(vp, x, y)
		!= m_terrain_visible(vp, xnw, ynw))
	      bitmask |= 2;
	    if (m_terrain_visible(vp, xw, yw)
		!= m_terrain_visible(vp, xnw, ynw))
	      bitmask |= 4;
	    if (bitmask != 0) {
	    	if (use_clip_mask) {
			XSetClipOrigin(dpy, gc,
					sx - (bitmask & 3) * hw - hw / 2,
					sy - halfside - ((bitmask & 4) ? 2 : 0) * hch);
			XFillRectangle(dpy, mapw->d, gc,
					sx - hw / 2, sy - halfside, hw, hch);
		} else {
			subi = (bitmask & 3) + ((bitmask & 4) ? 8 : 0);
			if (timg->numsubimages > 0 && timg->subimages)
			    subimg = timg->subimages[subi];
			if (subimg && subimg->hook)
			    tkimg = (TkImage *) subimg->hook;
			if (tkimg && tkimg->mask != None) {
				XSetFunction(dpy, gc, GXand);
				XCopyArea(dpy, tkimg->mask, mapw->d, gc,
					  0, 0, hws[power], hcs[power], 
					  sx - hws[power] / 2, sy - halfside);
				XSetFunction(dpy, gc, GXcopy);
			}
		}
	    }
	}
    }
}

static void
draw_contours(MapW *mapw, int x0, int y0, int len)
{
    int x, y;
    int i, sx, sy, numlines;
    LineSegment *lines;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;

    if (mapw->vp->contour_interval < 1)
      return;
    XSetClipMask(dpy, gc, None);
    XSetFillStyle(dpy, gc, FillSolid);
    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter); 
    XSetForeground(dpy, gc, dside->ui->contour_color->pixel);
    y = y0;
    for (x = x0; x < x0 + len; ++x) {
	if (m_terrain_visible(vp, x, y0)) {
	    xform(mapw, x, y, &sx, &sy);
	    contour_lines_at(mapw->vp, x, y, sx, sy, &lines, &numlines);
	    for (i = 0; i < numlines; ++i) {
		XDrawLine(dpy, mapw->d, gc,
			  lines[i].sx1, lines[i].sy1,
			  lines[i].sx2, lines[i].sy2);
	    }
	}
    }
}

static void
draw_clouds_row(MapW *mapw, int x0, int y0, int len)
{
    int x, sx, sy, cloudtype;
    Display *dpy = mapw->display;
    VP *vp = mapw->vp;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (m_terrain_visible(vp, x, y0)) {
	    cloudtype = cloud_view(dside, x, y0);
	    if (cloudtype == 0)
	        continue;
	    xform(mapw, x, y0, &sx, &sy);
	    XSetClipMask(dpy, mapw->gc, None);
             XSetFillStyle(dpy, mapw->gc, FillStippled);
	    XSetForeground(dpy, mapw->gc, dside->ui->whitecolor->pixel);
	    switch (cloudtype) {
	    	case	1: 
		    XSetStipple(dpy, mapw->gc, dside->ui->grays[verylightgray]);
		    break;
	    	case	2: 
		    XSetStipple(dpy, mapw->gc, dside->ui->grays[lightgray]);
		    break;
	    	case	3: 
		    XSetStipple(dpy, mapw->gc, dside->ui->grays[gray]);
		    break;
	    	case	4: 
		    XSetStipple(dpy, mapw->gc, dside->ui->grays[darkgray]);
		    break;
	    }
	    XFillArc(dpy, mapw->d, mapw->gc, sx, sy, mapw->vp->hw, mapw->vp->hh, 0, 360 * 64); 
	}
    }
     XSetForeground(dpy, mapw->gc, dside->ui->fgcolor->pixel);
     XSetFillStyle(dpy, mapw->gc, FillSolid);
}

static void
draw_temperature_row(MapW *mapw, int x0, int y0, int len)
{
    int x, sx, sy;
    Display *dpy = mapw->display;
    VP *vp = mapw->vp;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (m_terrain_visible(vp, x, y0)
	    && draw_temperature_here(dside, x, y0)) {
	    sprintf(spbuf, "%d", temperature_view(dside, x, y0));
	    xform(mapw, x, y0, &sx, &sy);
	    XSetClipMask(dpy, mapw->gc, None);
	    XSetForeground(dpy, mapw->gc, dside->ui->fgcolor->pixel);
	    Tk_DrawChars(dpy, mapw->d, mapw->gc, mapw->main_font,
			 spbuf, strlen(spbuf), sx + 5, sy + mapw->vp->uh / 2);
	}
    }
}

static void
draw_winds_row(MapW *mapw, int x0, int y0, int len)
{
    int x, sx, sy, rawwind, wdir, wforce, swforce;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (draw_winds_here(dside, x, y0)) {
	    rawwind = wind_view(dside, x, y0);
	    /* Show designers an accurate view. (should test in kernel?) */
	    if (is_designer(dside))
	      rawwind = (winds_defined() ? raw_wind_at(x, y0) : CALM);
	    wdir = wind_dir(rawwind);  wforce = wind_force(rawwind);
	    xform(mapw, x, y0, &sx, &sy);
	    sx += (mapw->vp->hw - 16) / 2;  
	    sy += (mapw->vp->hh - 16) / 2;
	    if (wforce < 0) {
		DGprintf("negative wind force %d, substituting 0", wforce);
		wforce = 0;
	    }
	    swforce =
	      ((wforce - minwindforce) * 5) / (maxwindforce - minwindforce);
	    if (swforce > 4)
	      swforce = 4;
	    if (swforce == 0)
	      wdir = 0;
	    if (use_clip_mask) {
	      XSetClipMask(dpy, gc, windpics[wforce][wdir]);
	      XSetClipOrigin(dpy, gc, sx + 1, sy + 1);
	      /* Draw a white arrow, offset slightly, for contrast on dark
		 backgrounds. */
	      XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	      XFillRectangle(dpy, mapw->d, gc, sx + 1, sy + 1, 16, 16);
	      /* Draw the main black arrow. */
	      XSetClipOrigin(dpy, gc, sx, sy);
	      XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	      XFillRectangle(dpy, mapw->d, gc, sx, sy, 16, 16);
	    } else {
	    }
	}
    }
}

/* Draw all the visible units in a given cell. */

static void
draw_uviews_in_cell(MapW *mapw, int x, int y)
{
    VP *vp = NULL;
    Display *dpy = NULL;
    GC gc = NULL;
    UnitView *uview = NULL, *uvstack = NULL;
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int i = 0;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    assert_error(mapw, "Attempted to draw on a NULL map widget");
    vp = mapw->vp;
    assert_error(vp, "Attempted to access a NULL viewport");
    dpy = mapw->display;
    assert_error(dpy, "Attempted to draw on a NULL display");
    gc = mapw->gc;
    assert_error(gc, "Attempted to access a NULL graphics context");
    /* Find out if there is anything to see at the location. */
    if (vp->show_all) {
	uvstack = query_uvstack_at(x, y);
	side = NULL;
    }
    else {
	uvstack = unit_view_at(dside, x, y);
	side = dside;
    }
    if (!uvstack)
      return;
    /* Iterate through uvstack and dispatch the appropriate drawing routines. */
    for_all_uvstack(uvstack, uview) {
	/* Increment uview counter. */
	++i;
	/* Get screen coords and dims of unit. */
	flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, flags);
	/* If we were told not to draw, then honor the request. */
	if (flags & XFORM_UVIEW_DONT_DRAW)
	  continue;
#if (0)
	/* If maximum number of displayable uviews has been reached, 
	   then draw ellipsis and don't consider any more uviews. */
	if ((i >= maxviews) && (maxviews > 1)) {
	    draw_dots(mapw, sx, sy, sw, sh);
	    break;
	}
#endif
	/* Draw the unit and any occs (if allowed and reasonable). */
	draw_unit_view_and_occs(mapw, uview, sx, sy, sw, sh);
    }
}

/* Draw grouping box to indicate a transport with occs. */

static int
draw_uimg_gbox(MapW *mapw, UnitView *uview, int sx, int sy, int sw, int sh)
{
    Display *dpy = NULL;
    GC gc = NULL;
    XColor *color = NULL;

    assert_error(mapw, "Attempted to draw in a NULL map window");
    assert_error(uview, "Attempted to access a NULL unit view");
    dpy = mapw->display;
    gc = mapw->gc;
    /* Draw a filled box to indicate the grouping. */
    color = request_color(g_unit_gbox_fill_color());
    XSetClipMask(dpy, gc, None);
    XSetForeground(dpy, gc, color->pixel);
    XFillRectangle(dpy, mapw->d, gc, sx, sy, sw + 1, sh - 1);
    /* Put a border around it, for better contrast. */
    color = request_color(g_unit_gbox_border_color());
    XSetForeground(dpy, gc, color->pixel);
    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter); 
    XDrawRectangle(dpy, mapw->d, gc, sx, sy, sw + 1, sh - 1);
    return TRUE;
}

/* Draw an unit view in the map window. */

static void
draw_unit_view(MapW *mapw, UnitView *uview, int sx, int sy, int sw, int sh)
{
    Display *dpy = NULL;
    GC gc = NULL;
    int sidenum = -1;

    assert_error(mapw, "Attempted to draw on a NULL map window");
    assert_error(uview, "Attempted to access a NULL unit view");
    dpy = mapw->display;
    gc = mapw->gc;
    /* If an occupant's side is the same as its transport's, then
       there's really no need to draw its side emblem, since the
       transport's emblem will also be visible. */
    if (uview->transport && (uview->siden == uview->transport->siden))
      sidenum = -1;
    else
      sidenum = uview->siden;
    /* Draw the unit image proper. */
    draw_unit_image(uview->imf, mapw, sx, sy, sw, sh, sidenum, 
		    !uview->complete);
    /* If the unit image is sufficiently large and the unit is advanced, 
       then draw its size. */
    if (sw >= 8 && u_advanced(uview->type)) {
	draw_unit_size(mapw, uview->size, sx + sw, sy - sh/4, 
		       sw, mapw->vp->uh);
    }
    /* If the unit has a name, the viewport allows name display, and the 
       unit image is sufficiently large, then display its name. */
    if (sw >= 16 && mapw->vp->draw_names && uview->name) {
	draw_unit_name(mapw, uview->name, sx, sy, sw, sh); 
    }
}

/* Draw the unit view, taking into account any occs and the zoom factor. */

static void
draw_unit_view_and_occs(MapW *mapw, UnitView *uview, int sx, int sy, 
			int sw, int sh)
{
    int sx2, sy2, sw2, sh2;
    UnitView *occview;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    /* Sanity checks. */
    assert_error(mapw, "Attempted to draw on a NULL map window");
    assert_error(uview, "Attempted to access a NULL unit view");
    /* Setup the side to xform for, depending on 'show_all'. */
    if (mapw->vp->show_all)
      side = NULL;
    else
      side = dside;
    /* If the unit is wide enough to support the drawing of occs, 
       then draw them. */
    if ((sw >= 16) && uview->occupant && mapw->vp->draw_occupants
	&& !(mapw->vp->isometric)) {
	/* Draw the grouping box. */
	draw_uimg_gbox(mapw, uview, sx, sy, sw, sh);
	/* Maybe draw the transport's shrunken image. */
	if (sw >= 32) {
	    flags = xform_unit_view(side, mapw->vp, uview, 
				    &sx2, &sy2, &sw2, &sh2, 
				    XFORM_UVIEW_AS_TSPT, NULL, sx, sy, sw, sh);
	    if (flags & XFORM_UVIEW_DONT_DRAW)
	      return;
	}
	else {
	    sx2 = sx;  sy2 = sy;  sw2 = sw; sh2 = sh;
	}
	draw_unit_view(mapw, uview, sx2, sy2, sw2, sh2);
	/* Maybe draw the occupants' images. */
	if (sw >= 32) {
	    for_all_occupant_views(uview, occview) {
		flags = xform_unit_view(side, mapw->vp, occview, 
					&sx2, &sy2, &sw2, &sh2, 
					XFORM_UVIEW_AS_OCC, NULL, 
					sx, sy, sw, sh);
		if (flags & XFORM_UVIEW_DONT_DRAW)
		  continue;
		draw_unit_view_and_occs(mapw, occview, sx2, sy2, sw2, sh2);
	    }
	}
    }
    /* Else just draw the unit. */
    else {
	draw_unit_view(mapw, uview, sx, sy, sw, sh);
    }
}

/* Draw an ellipsis to indicate more units are present than are shown. */

static void
draw_dots(MapW *mapw, int sx, int sy, int sw, int sh)
{
    int osx, osy;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    osx = sx + sw / 2 - 6;  
    osy = sy + sh - 2;
    /* (should be able to do with one fill?) */
    if (use_clip_mask) {
	XSetClipMask(dpy, gc, None);
	XSetClipOrigin(dpy, gc, osx, osy - 1);
	XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	XFillRectangle(dpy, mapw->d, gc, osx, osy - 1, 12, 4);
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XSetClipMask(dpy, gc, dside->ui->dots);
	XFillRectangle(dpy, mapw->d, gc, osx, osy - 1, 12, 4);
    } else {
	XSetFillStyle(dpy, gc, FillOpaqueStippled);
	XSetTSOrigin(dpy, gc, osx, osy - 1);
	XSetStipple(dpy, gc, dside->ui->dots);
	XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XFillRectangle(dpy, mapw->d, gc, osx, osy - 1, 12, 4);
	XSetTSOrigin(dpy, gc, 0, 0);
	XSetFillStyle(dpy, gc, FillSolid);
    }
}

static void
draw_unit_name(MapW *mapw, char *name, int sx, int sy, int sw, int sh)
{
    char namebuf[BUFSIZE];
    Display *dpy = mapw->display;
    Drawable d = mapw->d;
    GC gc = mapw->gc;
    Tk_Font tkfont;

    /* Don't draw numbers in this routine. */
    if (empty_string(name))
      return;
    strcpy(namebuf, name);
    sx += mapw->vp->hw + 1;
    sy += mapw->vp->hh / 2;  

    XSetClipMask(dpy, gc, None);

    tkfont = mapw->main_font;
    XSetFont(dpy, gc, Tk_FontId(tkfont));
    /* Draw a black shadow, unless the color of the text is to be
       black. */
    if (dside->ui->unit_name_color != dside->ui->blackcolor) {
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	/* This may seem excessive but it's the only way to make shadows look
	really good for all characters and numbers. */
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 1, sy);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 2, sy);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx, sy + 2);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 1, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 1, sy + 2);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 2, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		     sx + 2, sy + 2);
    }
    XSetForeground(dpy, gc, dside->ui->unit_name_color->pixel);
    Tk_DrawChars(dpy, d, gc, tkfont, namebuf, strlen(namebuf),
		 sx, sy);
}

static void
draw_unit_size(MapW *mapw, int size, int sx, int sy, int sw, int sh)
{
    char numbuf[12];
    Display *dpy = mapw->display;
    Drawable d = mapw->d;
    GC gc = mapw->gc;
    Tk_Font tkfont;

    /* Filter out very small images. */
    if (sw < 16)
	return; 
    /* Check for size. */
    if (size < 1)
	return;

    if (sh < 32) {
    	sx += 2;
    	sy += 11;
    } else {
    	sy += 3 * sh / 5;
    }
    sprintf(numbuf, "%d", size);
    XSetClipMask(dpy, gc, None);

    tkfont = mapw->main_font;
    XSetFont(dpy, gc, Tk_FontId(tkfont));
    if (dside->ui->unit_name_color != dside->ui->blackcolor) {
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	/* This may seem excessive but it's the only way to make shadows look
	really good for all characters and numbers. */
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx, sy + 2);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx + 1, sy);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx + 2, sy);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx  + 1, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx  + 1, sy + 2);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx  + 2, sy + 1);
	Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		     sx  + 2, sy + 2);
    }
    XSetForeground(dpy, gc, dside->ui->unit_name_color->pixel);
    Tk_DrawChars(dpy, d, gc, tkfont, numbuf, strlen(numbuf),
		 sx, sy);
}

/* Indicate what kind of people are living in the given cell. */

static void
draw_people(MapW *mapw, int x, int y)
{
    int pop, sx, sy, sw, sh, ex, ey, ew, eh, dir, x1, y1, pop1;
    int	con, con1, cbitmask, pbitmask, drawemblemhere;
    VP *vp = mapw->vp;

    if (!m_terrain_visible(vp, x, y))
      return;
    pop = (people_sides_defined() ? people_side_at(x, y) : NOBODY);
    con = (control_sides_defined() ? control_side_at(x, y) : NOCONTROL);
    cbitmask = pbitmask = 0;
    drawemblemhere = FALSE;
    /* Decide which edges are borders of the country. */
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    if (inside_area(x1, y1) && m_terrain_visible(vp, x1, y1)) {
		pop1 = (people_sides_defined() ? people_side_at(x1, y1) 
					       : NOBODY);
		con1 = (control_sides_defined() ? control_side_at(x1, y1) 
						: NOCONTROL);
		/* Just draw emblems against liquid terrain. */
		if (t_liquid(terrain_at(x, y))
		    || t_liquid(terrain_at(x1, y1))) {
		    if (con != con1 || pop != pop1) {
			drawemblemhere = TRUE;
		    }
		} else if (mapw->vp->draw_control && con != con1) {
		    cbitmask |= 1 << dir;
		    drawemblemhere = TRUE;
		} else if (mapw->vp->draw_people && pop != pop1) {
		    pbitmask |= 1 << dir;
		    drawemblemhere = TRUE;
		}
	    }
	}
    }
    if (drawemblemhere) {
	xform(mapw, x, y, &sx, &sy);
	for_all_directions(dir) {
	    int mask = 1 << dir;

	    if (cbitmask & mask) {
		draw_country_border_line(mapw, sx, sy, dir, TRUE);
	    } else if (pbitmask & mask) {
		draw_country_border_line(mapw, sx, sy, dir, FALSE);
	    }
	}
	/* Don't draw emblems at small magnifications. */
	if (mapw->vp->power > 3) {
	    /* Draw an emblem for the people in the cell. */
	    if (mapw->vp->draw_people && pop != NOBODY) {
		/* (should make generic) */
		sw = (mapw->vp->isometric ? mapw->vp->hh : mapw->vp->hw);
		sh = (mapw->vp->isometric ? mapw->vp->hw / 2 : mapw->vp->hh);
		ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
		ex = sx + sw / 2 - ew / 2;  ey = sy + sh / 2 - eh / 2;
		/* Maybe offset emblem so it will be visible underneath
		   control emblem. */
		if (mapw->vp->draw_control && con != pop) {
		    ex += ew / 4;  ey += eh / 4;
		}
		draw_side_emblem(mapw, ex, ey, ew, eh, pop);
	    }
	    if (mapw->vp->draw_control && con != NOCONTROL) {
		/* (should make generic) */
		sw = (mapw->vp->isometric ? mapw->vp->hh : mapw->vp->hw);
		sh = (mapw->vp->isometric ? mapw->vp->hw / 2 : mapw->vp->hh);
		ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
		ex = sx + sw / 2 - ew / 2;  ey = sy + sh / 2 - eh / 2;
		if (mapw->vp->draw_people && con != pop) {
		    ex -= ew / 4;  ey -= eh / 4;
		}
		draw_side_emblem(mapw, ex, ey, ew, eh, con);
	    }
	}
    }
}

/* There are two ways to draw borders; as images taken from a special
   border type image that includes 16 subimages for different combinations,
   or as lines colored according to the border's terrain type. */

static void
draw_borders(MapW *mapw, int vx, int vyhi, int vylo, int b)
{
    int x, y, x1, len, dir, bitmask, sx, sy, x3, y3, subi;
    int halfside;
    Image *timg, *subimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    XColor *color;
    int power = mapw->vp->power;
    int wid = bwid[power], wid2, sx1, sy1, sx2, sy2;

    timg = (dside->ui->best_timages[power])[b];
    if (!mapw->draw_lines
	&& between(4, power, 6)
	&& timg->isborder
	&& timg->subimages != NULL) {
	/* We want to draw individual images for border segments. */
	halfside = halfsides[power];
	for (y = vyhi; y >= vylo; --y) {
	    if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	      continue;
	    for (x = x1; x < x1 + len; ++x) {
		/* Draw the junction at the top of the x,y hex. */
		bitmask = 0;
		if (point_in_dir(x, y, NORTHEAST, &x3, &y3)
		    && border_at(x3, y3, WEST, b)
		    && m_seen_border(vp, x3, y3, WEST))
		  bitmask |= 1;
		if (border_at(x, y, NORTHEAST, b)
		    && m_seen_border(vp, x, y, NORTHEAST))
		  bitmask |= 2;
		if (border_at(x, y, NORTHWEST, b)
		    && m_seen_border(vp, x, y, NORTHWEST))
		  bitmask |= 4;
		if (bitmask != 0) {
		    xform(mapw, x, y, &sx, &sy);
		    subi = (bitmask & 3) + ((bitmask & 4) ? 12 : 4);
		    subimg = timg->subimages[subi];
		    tkimg = (TkImage *) subimg->hook;
		    if (tkimg != NULL && tkimg->colr != None) {
			if (use_clip_mask) {
			    if (tkimg->mask != None) {
				XSetClipOrigin(dpy, gc, sx, sy - halfside);
				XSetClipMask(dpy, gc, tkimg->mask);
			    }
			} else {
			    if (tkimg->mask != None) {
				XSetFunction(dpy, gc, GXand);
				/* (should draw smaller part of image) */
				XCopyArea(dpy, tkimg->mask, mapw->d, gc,
					  0, 0, mapw->vp->hw, mapw->vp->hch,
					  sx, sy - halfside);
			    }
			    XSetFunction(dpy, gc, GXor);
			}
			/* (should draw smaller part of image) */
			XCopyArea(dpy, tkimg->colr, mapw->d, gc,
				  0, 0, mapw->vp->hw, mapw->vp->hch,
				  sx, sy - halfside);
			if (!use_clip_mask)
			  XSetFunction(dpy, gc, GXcopy);
		    }
		}
		/* Draw the junction at the top left corner of the x,y hex. */
		bitmask = 0;
		if (border_at(x, y, WEST, b)
		    && m_seen_border(vp, x, y, WEST))
		  bitmask |= 1;
		if (border_at(x, y, NORTHWEST, b)
		    && m_seen_border(vp, x, y, NORTHWEST))
		  bitmask |= 2;
		if (point_in_dir(x, y, WEST, &x3, &y3)
		    && border_at(x3, y3, NORTHEAST, b)
		    && m_seen_border(vp, x3, y3, NORTHEAST))
		  bitmask |= 4;
		if (bitmask != 0) {
		    xform(mapw, x, y, &sx, &sy);
		    subi = (bitmask & 3) + ((bitmask & 4) ? 8 : 0);
		    subimg = timg->subimages[subi];
		    tkimg = (TkImage *) subimg->hook;
		    if (tkimg != NULL && tkimg->colr != None) {
			if (use_clip_mask) {
			    if (tkimg->mask != None) {
				XSetClipOrigin(dpy, gc, sx - mapw->vp->hw / 2, sy - halfside);
				XSetClipMask(dpy, gc, tkimg->mask);
			    }
			} else {
			    if (tkimg->mask != None) {
				XSetFunction(dpy, gc, GXand);
				/* (should draw smaller part of image) */
				XCopyArea(dpy, tkimg->mask, mapw->d, gc,
					  0, 0, mapw->vp->hw, mapw->vp->hch,
					  sx - mapw->vp->hw / 2, sy - halfside);
			    }
			    XSetFunction(dpy, gc, GXor);
			}
			/* (should draw smaller part of image) */
			XCopyArea(dpy, tkimg->colr, mapw->d, gc,
				  0, 0, mapw->vp->hw, mapw->vp->hch,
				  sx - mapw->vp->hw / 2, sy - halfside);
			if (!use_clip_mask)
			  XSetFunction(dpy, gc, GXcopy);
		    }
		}
	    }
	}
	if (use_clip_mask)
	  XSetClipMask(dpy, gc, None);
    } else {
	/* We want to draw borders as solid-color line segments. */
	wid2 = wid / 2;
	XSetLineAttributes(dpy, gc, bwid[power],
			   LineSolid, CapButt, JoinMiter); 
	XSetFillStyle(dpy, gc, FillSolid);
	color = dside->ui->cell_color[b];
	if (color == NULL)
	  color = dside->ui->blackcolor;
	XSetForeground(dpy, gc, color->pixel);
	XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	XSetClipMask(dpy, gc, None);
	for (y = vyhi; y >= vylo; --y) {
	    if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	      continue;
	    for (x = x1; x < x1 + len; ++x) {
		if (!m_terrain_visible(vp, x, y)
		    || !any_borders_at(x, y, b))
		  continue;
		bitmask = 0;
		for_all_directions(dir) {
		    if (border_at(x, y, dir, b)
			&& m_seen_border(vp, x, y, dir)) {
			bitmask |= 1 << dir;
		    }
		}
		if (bitmask != 0) {
		    xform(mapw, x, y, &sx, &sy);
		    for_all_directions(dir) {
			if (bitmask & (1 << dir)) {
			    sx1 = bsx[power][dir];  sy1 = bsy[power][dir];
			    sx2 = bsx[power][dir+1];  sy2 = bsy[power][dir+1];
			    XDrawLine(dpy, mapw->d, gc,
				      sx + sx1 - wid2, sy + sy1 - wid2,
				      sx + sx2 - wid2, sy + sy2 - wid2);
			}
		    }
		}
	    }
	}
	XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter); 
    }
}

static void
draw_borders_iso(MapW *mapw, int x, int y, int b)
{
    int bitmask, dir, dir2, sx, sy;
    int viewpow = mapw->vp->power;
    int wid = bwid[viewpow], wid2;
    Image *timg, *subimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    XColor *color;

    if (!m_terrain_visible(vp, x, y)
	|| !tt_drawable(b, terrain_at(x, y)))
      return;
    /* Make a bitmask of all the dirs that have a connection. */
    bitmask = 0;
    for_all_directions(dir) {
	if (border_at(x, y, dir, b)) {
	    /* Draw only borders "in front". */
	    dir2 = dir_subtract(dir, mapw->vp->isodir);
	    if (dir2 == WEST || dir2 == SOUTHWEST || dir2 == SOUTHEAST)
	      bitmask |= (1 << dir);
	}
    }
    if (bitmask != 0) {
	timg = (dside->ui->best_timages[viewpow])[b];
	if (1 /*mapw->draw_lines
	    || !between(4, viewpow, 6)
	    || !timg->isborder
	    || timg->subimages == NULL*/) {
	    wid2 = wid / 2;
	    XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	    color = dside->ui->cell_color[b];
	    if (!color)
	      color = dside->ui->blackcolor;
	    XSetForeground(dpy, gc, color->pixel);
	    XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	    set_terrain_gc_for_image(mapw, gc, timg);
	}
	xform(mapw, x, y, &sx, &sy);
	/* Choose whether to draw images or lines.  Unlike in border
	   drawing, the two algorithms are so similar it's not worth
	   writing a different loop for each case. */
	if (0 /*!mapw->draw_lines
	    && between(4, viewpow, 6)
	    && timg->isconnection
	    && timg->subimages != NULL*/) {
	    subimg = timg->subimages[bitmask];
	    tkimg = (TkImage *) subimg->hook;
	    if (tkimg != NULL && tkimg->colr != None) {
		if (use_clip_mask) {
		    if (tkimg->mask != None) {
			XSetClipOrigin(dpy, gc, sx, sy);
			XSetClipMask(dpy, gc, tkimg->mask);
		    }
		} else {
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, mapw->d, gc,
				  0, 0, mapw->vp->hw, mapw->vp->hh,
				  sx, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		XCopyArea(dpy, tkimg->colr, mapw->d, gc,
			  0, 0, mapw->vp->hw, mapw->vp->hh, sx, sy);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    }
	} else {
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    int adj;

		    dir2 = dir_subtract(dir, mapw->vp->isodir);
		    if (dir2 == WEST || dir2 == SOUTHWEST || dir2 == SOUTHEAST)
		      adj = -2;
		    else
		      adj = +2;
		    XDrawLine(dpy, mapw->d, gc,
			      sx + ibsx[viewpow][dir2] + adj, 
			      sy + ibsy[viewpow][dir2] + adj,
			      sx + ibsx[viewpow][dir2+1] + adj, 
			      sy + ibsy[viewpow][dir2+1] + adj);
		}
	    }
	}
	/* Clean up after ourselves. */
	if (1 /*mapw->draw_lines || !between(4, viewpow, 6) 
	    || timg->subimages == NULL*/) {
	  XSetFillStyle(dpy, gc, FillSolid);
	  XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
	} else {
	  XSetClipMask(dpy, gc, None);
	}
    }
}

/* Draw all the connections of the given type that are visible. */

static void
draw_connections(MapW *mapw, int vx, int vyhi, int vylo, int c)
{
    int x, y, x1, len, dir, bitmask, sx, sy;
    int power = mapw->vp->power;
    int wid = cwid[power], wid2, cx = hws[power] / 2, cy = hhs[power] / 2;
    Image *timg, *subimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    XColor *color;

    timg = (dside->ui->best_timages[power])[c];
    if (mapw->draw_lines
	|| !between(4, power, NUMPOWERS - 1)
	|| !timg->isconnection
	|| timg->subimages == NULL) {
	wid2 = wid / 2;
	XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	color = dside->ui->cell_color[c];
	if (!color)
	  color = dside->ui->blackcolor;
	XSetForeground(dpy, gc, color->pixel);
	XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	set_terrain_gc_for_image(mapw, gc, timg);
    }
    for (y = vyhi; y >= vylo; --y) {
	if (!compute_x1_len(mapw->vp, vx, vylo, y, &x1, &len))
	  continue;
	for (x = x1; x < x1 + len; ++x) {
	    if (!m_terrain_visible(vp, x, y)
		|| !tt_drawable(c, terrain_at(x, y)))
	      continue;
	    /* Make a bitmask of all the dirs that have a connection. */
	    bitmask = 0;
	    for_all_directions(dir) {
		if (connection_at(x, y, dir, c))
		  bitmask |= (1 << dir);
	    }
	    if (bitmask != 0) {
		xform(mapw, x, y, &sx, &sy);
		/* Choose whether to draw images or lines.  Unlike in
		   border drawing, the two algorithms are so similar it's
		   not worth writing a different loop for each case. */
		if (!mapw->draw_lines
		    && between(4, power, NUMPOWERS - 1)
		    && timg->isconnection
		    && timg->subimages != NULL) {
		    subimg = timg->subimages[bitmask];
		    tkimg = (TkImage *) subimg->hook;
		    if (tkimg != NULL && tkimg->colr != None) {
			if (use_clip_mask) {
			    if (tkimg->mask != None) {
				XSetClipOrigin(dpy, gc, sx, sy);
				XSetClipMask(dpy, gc, tkimg->mask);
			    }
			} else {
			    if (tkimg->mask != None) {
				XSetFunction(dpy, gc, GXand);
				XCopyArea(dpy, tkimg->mask, mapw->d, gc,
					  0, 0, mapw->vp->hw, mapw->vp->hh,
					  sx, sy);
			    }
			    XSetFunction(dpy, gc, GXor);
			}
			XCopyArea(dpy, tkimg->colr, mapw->d, gc,
				  0, 0, mapw->vp->hw, mapw->vp->hh, sx, sy);
			if (!use_clip_mask)
			  XSetFunction(dpy, gc, GXcopy);
		    }
		} else {
		    for_all_directions(dir) {
			if (bitmask & (1 << dir)) {
			    XDrawLine(dpy, mapw->d, gc,
				      sx + cx - wid2, sy + cy - wid2,
				      sx + cx + lsx[power][dir] - wid2,
				      sy + cy + lsy[power][dir] - wid2);
			}
		    }
		}
	    }
	}
    }
    /* Clean up after ourselves. */
    if (mapw->draw_lines 
        || !between(4, power, NUMPOWERS - 1) 
        || !timg->isconnection
        || timg->subimages == NULL) {
	XSetFillStyle(dpy, gc, FillSolid);
	XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
    } else {
	XSetClipMask(dpy, gc, None);
    }
}

static void
draw_connections_iso(MapW *mapw, int x, int y, int c)
{
    int bitmask, bitmask2, dir, dir2, sx, sy;
    int power = mapw->vp->power;
    int hw = mapw->vp->hw, hh = mapw->vp->hh;
    int wid = cwid[power], wid2, cx = hhs[power] / 2, cy = hws[power] / 4;
    Image *timg, *subimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;
    XColor *color;

    if (!m_terrain_visible(vp, x, y)
	|| !tt_drawable(c, terrain_at(x, y)))
      return;
    /* Make a bitmask of all the dirs that have a connection. */
    bitmask = 0;
    for_all_directions(dir) {
	if (connection_at(x, y, dir, c))
	  bitmask |= (1 << dir);
    }
    if (bitmask != 0) {
	timg = best_image(dside->ui->timages[c], hh, hw - hw / 4);
	if (mapw->draw_lines
	    || !between(4, power, NUMPOWERS - 1)
	    || !timg->isconnection
	    || timg->subimages == NULL) {
	    wid2 = wid / 2;
	    XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	    color = dside->ui->cell_color[c];
	    if (!color)
	      color = dside->ui->blackcolor;
	    XSetForeground(dpy, gc, color->pixel);
	    XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	    set_terrain_gc_for_image(mapw, gc, timg);
	}
	xform(mapw, x, y, &sx, &sy);
	/* Choose whether to draw images or lines.  Unlike in border
	   drawing, the two algorithms are so similar it's not worth
	   writing a different loop for each case. */
	if (!mapw->draw_lines
	    && between(4, power, NUMPOWERS - 1)
	    && timg->isconnection
	    && timg->subimages != NULL) {
	    bitmask2 = 0;
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    dir2 = dir - mapw->vp->isodir;
		    if (dir2 < 0)
		      dir2 += NUMDIRS;
		    bitmask2 |= (1 << dir2);
		}
	    }
	    subimg = timg->subimages[bitmask2];
	    tkimg = (TkImage *) subimg->hook;
	    if (tkimg != NULL && tkimg->colr != None) {
		if (use_clip_mask) {
		    if (tkimg->mask != None) {
			XSetClipOrigin(dpy, gc, sx, sy - hw / 4);
			XSetClipMask(dpy, gc, tkimg->mask);
		    }
		} else {
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, mapw->d, gc,
				  0, 0, mapw->vp->hw, mapw->vp->hh,
				  sx, sy - hw / 4);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		XCopyArea(dpy, tkimg->colr, mapw->d, gc,
			  0, 0, mapw->vp->hw, mapw->vp->hh, sx, sy - hw / 4);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    }
	} else {
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    dir2 = dir - mapw->vp->isodir;
		    if (dir2 < 0)
		      dir2 += NUMDIRS;
		    XDrawLine(dpy, mapw->d, gc,
			      sx + cx - wid2, sy + cy - wid2,
			      sx + cx + ilsx[power][dir2] - wid2,
			      sy + cy + ilsy[power][dir2] - wid2);
		}
	    }
	}
	/* Clean up after ourselves. */
	if (mapw->draw_lines
	    || !between(4, power, NUMPOWERS - 1)
	    || !timg->isconnection
	    || timg->subimages == NULL) {
	    XSetFillStyle(dpy, gc, FillSolid);
	    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
	} else {
	    XSetClipMask(dpy, gc, None);
	}
    }
}

/* Draw any resource usage inidcators in the surrounding cells used by 
   the current unit. */

int
draw_uview_rsrc_usage(MapW *mapw, UnitView *uview)
{
    int x = -1, y = -1;
    Unit *unit = NULL;

    assert_error(mapw, "Attempted to draw on a NULL map window");
    assert_error(uview, "Attempted to access a NULL unit view");
    if (!any_resources || !u_advanced(uview->type))
      return FALSE;
    unit = view_unit(uview);
    for_all_cells_within_reach(unit, x, y) {
	if (user_at(x, y) == unit->id) {
	    draw_resource_usage(mapw, x, y);
	}
    }
    return TRUE;
}

/* Draw selection rectangle around an unit view. */
/*! \todo Break this function up into a part that handles the solid colored 
	  rectangle, and one that handles the crawling ants or blinking 
	  rectangle. This would become useful for handling multiple unit 
	  selection. Multiple units could be selected, but only the current 
	  unit would have the animated rectangle. */

static int
draw_uview_selection_rectangle(MapW *mapw, UnitView *uview, 
			       int sx, int sy, int sw, int sh)
{
    VP *vp = NULL;
    Display *dpy = NULL;
    GC gc = NULL;
    Unit *unit = NULL;
    enum grayshade shade = black;

    assert_error(mapw, "Attempted to draw in a NULL map window");
    assert_error(uview, "Attempted to access a NULL unit view");
    vp = mapw->vp;
    dpy = mapw->display;
    gc = mapw->gc;
    unit = view_unit(uview);
    /* "Outdent" the curunit indicator slightly, so that it doesn't cover
       up as much of the unit image, if there is room to do so. */
    if ((sw < vp->hw) && !uview->transport) {
	sx -= 1;  sy -= 1;
	sw += 2;  sh += 2;
    }
    /* Black is for units that can still act, dark gray for actors
       that used all acp for this turn, gray if the unit can't ever do
       anything. */
    shade = gray;
    if (unit->act && (unit->act->initacp > 0))
      shade = (has_acp_left(unit) ? black : darkgray);
    /* Draw the selected unit indicator proper. */
    XSetClipMask(dpy, gc, None);
    if ((sw >= 4) && (sh >= 4)) {
	XSetFillStyle(dpy, gc, FillSolid);
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	if (shade != black) {
	    XSetFillStyle(dpy, gc, FillOpaqueStippled);
	    XSetStipple(dpy, gc, dside->ui->grays[shade]);
	    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	    XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	}
	draw_rectangle(mapw, sx + 1, sy + 1, sw - 3, sh - 2);
    }
    /* Draw an outer box, either a crawling ants line if we're in
       autoselect mode, or solid white if the main selection box is
       solid black, for contrast. */
    if (!dside->finishedturn) {
	if (mapw->map->autoselect) {
	     XSetBackground(dpy, gc, dside->ui->blackcolor->pixel);
	     XSetFillStyle(dpy, gc, FillOpaqueStippled);
	     XSetStipple(dpy, gc, antpic);
	     XSetTSOrigin(dpy, gc, mapw->map->anim_state % 8, 0);
	}
	if (mapw->map->autoselect || shade == black) {
	    /* Use blinking instead for Mac and Windows. */
	    if (blinking_curunit && mapw->map->anim_state % 2) {
		XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	    } else {
		XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	    }
	    /* Draw active unit rect. */
	    if (sw >= 2 && sh >= 2) {
		draw_rectangle(mapw, sx, sy, sw - 1, sh);
	    /* Just fill in the whole unit area, at small mags. */
	    } else {
		XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
	    }
	}
	/* Reset the tile/stipple origin if necessary. */
	if (mapw->map->autoselect)
	  XSetTSOrigin(dpy, gc, 0, 0);
    }
    XSetFillStyle(dpy, gc, FillSolid);
    return TRUE;
}

/* Draw health bar over the unit view. */

static int
draw_uview_health_bar(MapW *mapw, UnitView *uview, 
		      int sx, int sy, int sw, int sh)
{
    Display *dpy = NULL;
    GC gc = NULL;
    int barwid, tsw, hsw;
    XColor *color;
    Unit *unit = NULL;
    VP *vp = NULL;

    assert_error(mapw, "Attempted to draw on a NULL window");
    assert_error(uview, "Attempted to access a NULL unit view");
    dpy = mapw->display;
    gc = mapw->gc;
    vp = mapw->vp;
    unit = view_unit(uview);
    /* If image width is too small, then don't bother. */
    if (sw < 16)
      return FALSE;
    /* If image is not yet completed, then don't bother. */
    /*! \todo This behavior may change once we allow units-under-construction 
	      to have more than 1 HP. */
    if (!completed(unit))
      return FALSE;
    barwid = 1;
    tsw = sw - 4;
    /* Adjust the position downwards if we are dealing with an oversized
       image. */
    if (sh >= (vp->hw - (2 + barwid) - 1))
      sy += (5 + barwid);
    hsw = (unit->hp * (tsw - 2)) / u_hp(unit->type);
    if (hsw == 0)
      hsw = 1;
    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
    XFillRectangle(dpy, mapw->d, gc,
		   sx + 2, sy - (2 + barwid) - 1, tsw, 2 + barwid);
    /* (should be able to control color changes) */
    color = dside->ui->goodcolor;
    if (unit->hp * 4 <= u_hp(unit->type))
      color = dside->ui->badcolor;
    else if (unit->hp * 2 <= u_hp(unit->type))
      color = dside->ui->warncolor;
    XSetForeground(dpy, gc, color->pixel);
    XFillRectangle(dpy, mapw->d, gc,
		   sx + 3, sy - (2 + barwid), hsw, barwid);
    return TRUE;
}

/* Draw the currently selected unit and its associated decorations. */

static void
draw_current(MapW *mapw)
{
    int sx, sy, sw, sh;
    Unit *unit = NULL;
    UnitView *uview = NULL, *origuview = NULL;
    VP *vp = mapw->vp;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    unit = mapw->map->curunit;
    if (!in_play(unit) || unit->id != mapw->map->curunit_id)
      return;
    /* Set the side according to whether we are in show_all mode or not. */
    if (vp->show_all)
      side = NULL;
    else
      side = dside;
    /* Get uview associated with unit. */
    origuview = uview = find_unit_view(side, unit);
    /* If there is no view for the current unit, then get out. */
    /* This may be called while there is no view object for
       the current unit, such as while updating the display
       when the unit moves and is "between" cells. */
    if (!uview)
      return;
    /* Sanity check. */
    if (unit->transport)
      assert(uview->transport);
    /* While isometric view, and inside transport, try to find a transport 
       to display instead. */
    while (vp->isometric && uview->transport) 
      uview = uview->transport;
    /* Get screen coords from uview coords. */
    flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, flags);
    /* Return immediately if the uview is not be drawn. */
    if (flags & XFORM_UVIEW_DONT_DRAW)
      return;
    /* While uview width less than 16 or the unit width for the current zoom, 
       and while uview is in a transport, try to find a transport view that is 
       wide enough. */
    while ((sw < min(16, vp->uw)) && uview->transport) {
	/* Get transport view instead. */
	uview = uview->transport;
	/* Get screen coords from uview coords. */
	flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, 
				flags & ~XFORM_UVIEW_AS_OCC);
	/* Return immediately if the uview is not be drawn. */
	if (flags & XFORM_UVIEW_DONT_DRAW)
	  return;
    }
    /* Draw the unit view and associated trappings. */
    draw_unit_view_and_occs(mapw, origuview, sx, sy, sw, sh);
    /* Draw unit's resource usage, if any. */
    if (sw >= 8)
      draw_uview_rsrc_usage(mapw, origuview);
    /* If a grouping box is suspected, then tighten up around to just the 
       unit and not everything in the box. */
    if (uview->occupant && (uview == origuview) && (sw >= 32)) {
	flags = XFORM_UVIEW_AS_TSPT;
	flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, 
				flags, NULL, sx, sy, sw, sh); 
	if (flags & XFORM_UVIEW_DONT_DRAW)
	  return;
    }
    /* Draw unit selection rectangle and current unit indicator. */
    draw_uview_selection_rectangle(mapw, origuview, sx, sy, sw, sh);
    /* Draw a health bar. */
    draw_uview_health_bar(mapw, origuview, sx, sy, sw, sh);
    /* \todo Draw a completion bar once the fadeout is being used to 
	     indicate unit view age. */
}

void
draw_unit_blast(Map *map, Unit *unit, int blasttype, int duration)
{
    int sx, sy, sw, sh;
    MapW *mapw = (MapW *) map->widget;
    VP *vp = mapw->vp;

    if (between(0, blasttype, 2)) {
	if (vp->hw > 10 && !vp->isometric) {
	    x_xform_unit(mapw, unit, &sx, &sy, &sw, &sh);
	} else {
	    xform(mapw, unit->x, unit->y, &sx, &sy);
	    if (vp->isometric) {
		sy += (vp->hw / 2 - vp->uh);
	    }
	    sw = vp->hw;  sh = vp->hh;
	}
	mapw->blastsx = sx;  mapw->blastsy = sy;
	mapw->blastsw = sw;  mapw->blastsh = sh;
	mapw->blasttype = blasttype;
	update_cell(map, unit->x, unit->y);
	Tcl_Sleep(duration);
	mapw->blasttype = -1;
	update_cell(map, unit->x, unit->y);
    }
}

void
draw_cell_blast(Map *map, int x, int y, int blasttype, int duration)
{
    int sx, sy, sw, sh;
    MapW *mapw = (MapW *) map->widget;

    if (blasttype == 3) {
	xform(mapw, x, y, &sx, &sy);
	sw = mapw->vp->hw;  sh = mapw->vp->hh;
	mapw->blastsx = sx;  mapw->blastsy = sy;
	mapw->blastsw = sw;  mapw->blastsh = sh;
	mapw->blasttype = blasttype;
	update_cell(map, x, y);
	Tcl_Sleep(duration);
	++mapw->blasttype;
	update_cell(map, x, y);
	Tcl_Sleep(duration);
	++mapw->blasttype;
	update_cell(map, x, y);
	Tcl_Sleep(duration);
	mapw->blasttype = -1;
	update_cell(map, x, y);
    } else if (blasttype == 10) {
	mapw->blasttype = blasttype;
	eventually_redraw(mapw, 0, 0, mapw->vp->pxw, mapw->vp->pxh);
	eval_tcl_cmd("update idletasks");
	Tcl_Sleep(duration);
	mapw->blasttype = -1;
	eventually_redraw(mapw, 0, 0, mapw->vp->pxw, mapw->vp->pxh);
	eval_tcl_cmd("update idletasks");
    }
}

/* Given a pixel location on the map, draw a blast of the given type. */

static void
draw_blast_image(MapW *mapw, int sx, int sy, int sw, int sh, int blasttype)
{
    int sx2, sy2;
    ImageFamily *bimf;
    Image *bimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    Drawable d = mapw->d;
    GC gc = mapw->gc;

    XSetClipMask(dpy, gc, None);
    if (blasttype == 10) {
	/* Flash the whole screen white. */
	XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	XSetFunction(dpy, gc, GXcopy);
	XFillRectangle(dpy, d, gc, 0, 0, mapw->vp->pxw, mapw->vp->pxh);
    } else if (sw >= 16 && sh >= 16
	       && ((bimf = dside->ui->blastimages[blasttype]) != NULL)) {
	/* Normally draw a colorized image of the blast type. */
	bimg = best_image(bimf, sw, sh);
	if (bimg != NULL
	    && ((tkimg = (TkImage *) bimg->hook) != NULL)) {
	    /* (should abstract into a separate routine) */
	    /* Offset the image to draw in the middle of its area,
	       whether larger or smaller than the given area. */
	    sx2 = sx + (sw - bimg->w) / 2;  sy2 = sy + (sh - bimg->h) / 2;
	    /* Only change the size of the rectangle being drawn if it's
	       smaller than what was passed in. */
	    if (bimg->w < sw) {
		sx = sx2;
		sw = bimg->w;
	    }
	    if (bimg->h < sh) {
		sy = sy2;
		sh = bimg->h;
	    }
	    if (tkimg->colr != None) {
		if (use_clip_mask) {
		    if (tkimg->mask != None) {
			/* set the clip mask */
			XSetClipOrigin(dpy, gc, sx2, sy2);
			XSetClipMask(dpy, gc, tkimg->mask);
		    }
		} else {
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, mapw->d, gc,
				  0, 0, sw, sh, sx, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		/* Draw the color image. */
		XCopyArea(dpy, tkimg->colr, mapw->d, gc, 0, 0, sw, sh, sx, sy);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    } else if (tkimg->mono != None) {
		if (use_clip_mask) {
		    /* Draw the shadow. */
		    XSetClipOrigin(dpy, gc, sx2 + 1, sy2 + 1);
		    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		    XSetClipMask(dpy, gc, tkimg->mono);
		    XFillRectangle(dpy, d, gc, sx + 1, sy + 1, sw, sh);
		    /* Draw the blast image proper. */
		    XSetClipOrigin(dpy, gc, sx2, sy2);
		    XSetForeground(dpy, gc, dside->ui->badcolor->pixel);
		    XSetClipMask(dpy, gc, tkimg->mono);
		    XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
		} else {
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, d, gc,
				  0, 0, sw, sh, sx, sy);
			/* Make a shadow. */
			XCopyArea(dpy, tkimg->mask, d, gc,
				  0, 0, sw, sh, sx + 1, sy + 1);
		    }
		    XSetFunction(dpy, gc, GXor);
		    XCopyArea(dpy, tkimg->mono, d, gc,
			      0, 0, sw, sh, sx, sy);
		    XSetFunction(dpy, gc, GXcopy);
		}
	    } else {
		printf("no tkimg mono\n");
	    }
	} else {
	    printf("no bimg\n");
	}
    } else if (sw >= 4 && sh >= 4) {
	/* At smaller scales, draw just boxes. */
	/* Draw misses as 1/2 the size of hits and kills. */
	if (blasttype == 0) {
	    sx += sw / 4;  sy += sh / 4;
	    sw /= 2;  sh /= 2;
	}
	/* Draw a red box with a black shadow. */
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XFillRectangle(dpy, d, gc, sx + 1, sy + 1, sw - 1, sh - 1);
	XSetForeground(dpy, gc, dside->ui->badcolor->pixel);
	XFillRectangle(dpy, d, gc, sx, sy, sw - 1, sh - 1);
    } else {
	/* At the smallest sizes, draw a red box only. */
	XSetForeground(dpy, gc, dside->ui->badcolor->pixel);
	XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
    }
}

static void
draw_feature_boundary(MapW *mapw, int x, int y)
{
    int wid, p, dir, fid0, fid1, x1, y1, sx, sy, bitmask1, bitmask2;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    VP *vp = mapw->vp;

    if (!m_terrain_visible(vp, x, y))
      return;
    if (t_liquid(terrain_at(x, y)))
      return;
    fid0 = raw_feature_at(x, y);
    if (fid0 == 0)
      return;
    p = mapw->vp->power;
    wid = bwid[p];
    if (wid == 0)
      return;
    /* Compute which sides should have border lines drawn. */
    bitmask1 = bitmask2 = 0;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    /* Don't draw boundaries between sea and land. */
	    if (t_liquid(terrain_at(x1, y1)))
		continue;
	    fid1 = raw_feature_at(x1, y1);
	    if (fid1 != fid0 && m_terrain_visible(vp, x1, y1)) {
		if (is_designer(dside)
		    && (fid0 == dside->ui->curfid
			|| fid1 == dside->ui->curfid))
		  bitmask2 |= (1 << dir);
		else
		  bitmask1 |= (1 << dir);
	    }
	}
    }
    if (bitmask1 != 0 || bitmask2 != 0) {
	xform(mapw, x, y, &sx, &sy);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	/* Line width of 0 draws as width 1, but possibly more efficiently. */
	wid = (p >= 4 ? 2 : 0);
	if (wid > 0)
	  XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	if (bitmask1 != 0) {
	    XSetForeground(dpy, gc, dside->ui->feature_color->pixel);
	    for_all_directions(dir) {
		if (bitmask1 & (1 << dir)) {
		    XDrawLine(dpy, mapw->d, gc,
			      sx + bsx[p][dir], sy + bsy[p][dir],
			      sx + bsx[p][dir+1], sy + bsy[p][dir+1]);
		}
	    }
	}
	if (bitmask2 != 0) {
	    XSetForeground(dpy, gc, dside->ui->badcolor->pixel);
	    for_all_directions(dir) {
		if (bitmask2 & (1 << dir)) {
		    XDrawLine(dpy, mapw->d, gc,
			      sx + bsx[p][dir], sy + bsy[p][dir],
			      sx + bsx[p][dir+1], sy + bsy[p][dir+1]);
		}
	    }
	}
	if (wid > 0)
	  XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
    }
}

/* Draw resources in a style similar to that used in Civ, as a row of
   images (or solid-color squares), as many as the amount of
   production of each type of resource. */

static void
draw_resource_usage(MapW *mapw, int x, int y)
{
    int m, totprod, incr, prod, i, xoff, sx, sy, sx2, sy2, sx3, sy3, sw, sh;
    int useimage;
    Image *mimg;
    TkImage *tkimg;
    XColor *color;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    if (!user_defined())
      return;
    if (user_at(x, y) == NOUSER)
      return;
    /* Don't draw if cells too small. */
    if (mapw->vp->hw <= 12)
      return;
    /* Draw only for the selected unit. */
    if (mapw->map->curunit == NULL)
      return;
    if (user_at(x, y) != mapw->map->curunit->id)
      return;
    xform(mapw, x, y, &sx, &sy);
    sy += mapw->vp->hh / 2;
    XSetClipMask(dpy, gc, None);
    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
    totprod = 0;
    for_all_material_types(m) {
	if (m_resource_icon(m) <= 0)
	  continue;
	totprod += production_at(x, y, m);
    }
    /* Nothing to draw, get out of here. */
    if (totprod == 0)
      return;
    /* Compute the spacing between each image. */
    incr = 16;
    if (totprod > 0)
      incr = (mapw->vp->hw - 6 - 2) / totprod;
    if (incr > 16)
      incr = 16;
    if (incr < 2)
      incr = 2;
    xoff = 2;
    /* (should center if few icons to display?) */
    for_all_material_types(m) {
	/* Skip over materials that are not "city resources". */
	if (m_resource_icon(m) <= 0)
	  continue;
	prod = production_at(x, y, m);
	if (prod <= 0)
	  continue;
	sw = sh = 16;
	sx3 = sx;  sy3 = sy;
	useimage = FALSE;
	mimg = best_image(dside->ui->mimages[m], sw, sh);
	if (mimg != NULL) {
	    sx2 = sx + (sw - mimg->w) / 2;  sy2 = sy + (sh - mimg->h) / 2;
	    /* Only change the size of the rect being drawn if
	       it's smaller than what was passed in. */
	    if (mimg->w < sw) {
		sx3 = sx2;
		sw = mimg->w;
	    }
	    if (mimg->h < sh) {
		sy3 = sy2;
		sh = mimg->h;
	    }
	    tkimg = (TkImage *) mimg->hook;
	    if (tkimg != NULL && tkimg->colr != None)
	      useimage = TRUE;
	}
	/* Draw as many images/boxes as there is production. */
	for (i = 0; i < prod; ++i) {
	    if (useimage) {
		if (use_clip_mask) {
		    if (tkimg->mask != None) {
			/* Set the clip mask. */
			XSetClipOrigin(dpy, gc, sx2 + xoff, sy2);
			XSetClipMask(dpy, gc, tkimg->mask);
		    }
		} else {
		    /* Cut a mask-shaped hole. */
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, mapw->d, gc,
				  0, 0, sw, sh, sx + xoff, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		/* Draw the color image. */
		XCopyArea(dpy, tkimg->colr, mapw->d, gc, 0, 0,
			  sw, sh, sx3 + xoff, sy3);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    } else {
		/* If no usable image, draw a solid-color box. */
		if (dside->ui->material_color[m]) {
		    color = dside->ui->material_color[m];
		} else {
		    color = dside->ui->whitecolor;
		}
		XSetForeground(dpy, gc, color->pixel);
		XFillRectangle(dpy, mapw->d, gc, sx3 + xoff, sy3, 6, 6);
		XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
		XDrawRectangle(dpy, mapw->d, gc, sx3 + xoff, sy3, 6, 6);
	    }
	    xoff += incr;
	}
    }
}

static void
draw_ai_region(MapW *mapw, int x, int y)
{
    int thid, sx, sy, dir, x1, y1, thid1, bitmask = 0, wid;
    int p = mapw->vp->power;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    thid = ai_region_at(dside, x, y);
    /* Decide which edges are borders of the theater. */
    for_all_directions(dir) {
	/* Don't do anything about edge cells. */
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    thid1 = ai_region_at(dside, x1, y1);
	    if (thid != thid1) {
		bitmask |= (1 << dir);
	    }
	}
    }
    if (bitmask != 0) {
	xform(mapw, x, y, &sx, &sy);
	XSetClipMask(dpy, gc, None);
	XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	wid = bwid[p];
	if (wid > 0) {
	    if (wid == 1)
	      wid = 0;
	    if (wid > 0)
	      XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    XDrawLine(dpy, mapw->d, gc,
			      sx + bsx[p][dir], sy + bsy[p][dir],
			      sx + bsx[p][dir+1], sy + bsy[p][dir+1]);
		}
	    }
	    if (wid > 0)
	      XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter); 
	} else {
	    XFillRectangle(dpy, mapw->d, gc, sx, sy, hws[p], hhs[p]);
	}
    }
}

/* Do the grody work of drawing very large polygons accurately. */

static void
draw_hex_polygon(MapW *mapw, GC gc, int sx, int sy, int power, int over,
		 int coat, XColor *color2, int dogrid)
{
    XPoint points[6];
    int hw = hws[power], hh = hhs[power], delt = (hhs[power] - hcs[power]);
    int ew = (dogrid ? 1 : 0);
    enum grayshade shade;
    Display *dpy = mapw->display;

    points[0].x = sx + hw / 2;         points[0].y = sy;
    points[1].x = sx + hw - ew;        points[1].y = sy + delt /*- ew*/;
    points[2].x = sx + hw - ew;        points[2].y = sy + (hh - delt - ew);
    points[3].x = sx + hw / 2;         points[3].y = sy + (hh - ew);
    points[4].x = sx;                  points[4].y = sy + (hh - delt - ew);
    points[5].x = sx;                  points[5].y = sy + delt;
    XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    if (coat != NONTTYPE) {
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, dside->ui->grays[gray]);
	XSetForeground(dpy, gc, color2->pixel);
	XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    }
    if (over < 0) {
	shade = GRAY(over);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, dside->ui->grays[shade]);
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    }
}

static void
draw_iso_polygon(MapW *mapw, GC gc, int sx, int sy, int power, int terrain, 
		 int over, int coat, XColor *color2)
{
    Display *dpy = mapw->display;
    XPoint points[6];
    enum grayshade shade;

    points[0].x = sx + ix1[power];      points[0].y = sy;
    points[1].x = sx + ix2[power];	points[1].y = sy;
    points[2].x = sx + iws[power]; 	points[2].y = sy + ics[power];
    points[3].x = sx + ix2[power];	points[3].y = sy + ihs[power];
    points[4].x = sx + ix1[power];	points[4].y = sy + ihs[power];
    points[5].x = sx;                  	points[5].y = sy + ics[power];

    XSetClipMask(dpy, gc, None);
    if (terrain) {    
	XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    }
    if (coat != NONTTYPE) {
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, dside->ui->grays[gray]);
	XSetForeground(dpy, gc, color2->pixel);
	XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    }
    if (over < 0) {
	shade = GRAY(over);
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, dside->ui->grays[shade]);
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XFillPolygon(dpy, mapw->d, gc, points, 6, Convex, CoordModeOrigin);
    }
}

/* If the requested unit image is less than 4x4, draw a rectangle 
   containing the side colors in its stead. */

int
draw_uimg_as_side_color(Image *img, MapW *mapw, int sx, int sy, int sidenum)
{
    XColor *imagecolor = dside->ui->blackcolor;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    int sw = -1, sh = -1;

    assert_error(img, "Attempted to draw a NULL unit image");
    assert_error(mapw, "Attempted to draw in a NULL map widget");
    sw = img->w;  sh = img->h;
    /* Ignore images that are too small. */
    if (!sw || !sh)
      return FALSE;
    /* If image is too large, then tell caller to draw it some other way. */
    if ((sw > 4) && (sh > 4))
      return FALSE;
    /* Else, if image has at least one dimension <= 4 px. */
    XSetClipMask(dpy, gc, None);
    imagecolor = dside->ui->default_colors[sidenum];
    /* First draw a black shadow offset one pixel. */
    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
    XFillRectangle(dpy, mapw->d, gc, sx, sy, sw + 1, sh + 1);
    /* Then draw a box in the side's default color. */
    XSetForeground(dpy, gc, imagecolor->pixel);
    XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
    return TRUE;
}

/* Draw any requested fadeout over an image. */

int
draw_uimg_fadeout(TkImage *tkimg, MapW *mapw, int sx, int sy, int fadelvl, 
		  int ignoremask)
{
    Display *dpy = mapw->display;
    GC gc = mapw->gc;
    Image *img = NULL;
    int sw = -1, sh = -1;
    Pixmap stipple;

    assert_error(tkimg, "Attempted to draw a NULL unit image");
    assert_error(mapw, "Attempted to draw in a NULL map widget");
    img = tkimg->generic;
    assert_error(img, "Attempted to draw a NULL unit image");
    sw = img->w;  sh = img->h;
    /* Choose appropriate stipple. */
    if (fadelvl <= 0)
      return FALSE;
    else if (between(1, fadelvl, 25))
      stipple = dside->ui->grays[verylightgray];
    else if (between(26, fadelvl, 50))
      stipple = dside->ui->grays[lightgray];
    else if (between(51, fadelvl, 75))
      stipple = dside->ui->grays[gray];
    else if (between(76, fadelvl, 100))
      stipple = dside->ui->grays[darkgray];
    else
      stipple = dside->ui->grays[black];
    /* Use the mask or the mono image if available for clipping. */
    if (!ignoremask && use_clip_mask) {
	if (tkimg->mask != None) {
	    XSetClipMask(dpy, gc, tkimg->mask);
	} else if (tkimg->mono != None) {
	    XSetClipMask(dpy, gc, tkimg->mono);
	} else {
	    XSetClipMask(dpy, gc, None);
	}
    /* Else use no mask. */
    } else {
	XSetClipMask(dpy, gc, None);
    }
    /* Setup stipple. */
    XSetFillStyle(dpy, gc, FillStippled);
    XSetStipple(dpy, gc, dside->ui->grays[gray]);
    XSetForeground(dpy, gc, dside->ui->mask_color->pixel);
    /* Fill with stipple. */
    XFillRectangle(dpy, mapw->d, gc, sx + 2, sy + 2, sw - 4, sh - 4);
    /* Reset drawing params. */
    XSetFillStyle(dpy, gc, FillSolid);
    return TRUE;
}

/* Draw side emblem on unit image, if requested. */

int
draw_uimg_emblem(Image *img, MapW *mapw, int sx, int sy, int sidenum)
{
    int sw = -1, sh = -1;
    char *ename = NULL;
    ImageFamily *eimf = NULL;
    int ex = -1, ey = -1, ew = -1, eh = -1;

    assert_error(img, "Attempted to draw a NULL unit image");
    assert_error(mapw, "Attempted to draw in a NULL map widget");
    /* If side number is invalid (such as NULL indepside), then forget 
       about it. */
    if (!between(0, sidenum, numsides))
      return FALSE;
    sw = img->w;  sh = img->h;
    /* If image is too small, then forget about it. */
    if ((sw <= 4) || (sh <= 4))
      return FALSE;
    /* Get emblem name. */
    ename = (side_n(sidenum) ? side_n(sidenum)->emblemname : NULL);
    /* Get emblem IMF. */
    eimf = dside->ui->eimages[sidenum];
    /* Calculate emblem position, and proceed if the result is valid. */
    if (emblem_position(img, ename, eimf, sw, sh, mapw->vp->uh, mapw->vp->hh, 
			&ex, &ey, &ew, &eh)) {
	/* Do the drawing proper. */
	draw_side_emblem(mapw, sx + ex, sy + ey, ew, eh, sidenum);
    }
    return TRUE;
}

/* Put a unit image in the widget's window. */

void
draw_unit_image(ImageFamily *imf, MapW *mapw, int sx, int sy, int sw, int sh, 
		int s2, int mod)
{
    int sx2, sy2, desperate = FALSE;
    XColor *imagecolor = dside->ui->blackcolor;
    XColor *maskcolor = dside->ui->whitecolor;
    Image *uimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    /* Filter out very small images. */
    if (sw <= 0)
      return;
    /* Reset the clip mask. */
    XSetClipMask(dpy, gc, None);
    /* Find best image in range of widths and heights. */
    uimg = best_image_in_range(imf, sw, sh, sw, sh, sw, sh);
    /* If image is small enough, then substitute a rectangle in the side 
       color instead. */
    if (uimg && draw_uimg_as_side_color(uimg, mapw, sx, sy, s2))
      return;
    if (uimg != NULL) {
	/* Offset the image to draw in the middle of its area,
	   whether larger or smaller than the given area. */
	sx2 = sx + (sw - uimg->w) / 2;  
	sy2 = sy + (sh - uimg->h) / 2;
	/* Only change the size of the rectangle being drawn if it's
	   smaller than what was passed in. */
	if (uimg->w < sw) {
	    sx = sx2;
	    sw = uimg->w;
	}
	if (uimg->h < sh) {
	    sy = sy2;
	    sh = uimg->h;
	}
	/* Figure out what colors to use. */
	if (mapw->map->colorize_units && dside->ui->numcolors[s2] > 0)
	  imagecolor = dside->ui->colors[s2][0];
	if (mapw->map->colorize_units && dside->ui->numcolors[s2] > 1)
	  maskcolor = dside->ui->colors[s2][1];
	tkimg = (TkImage *) uimg->hook;
	if (tkimg != NULL) {
	    if (tkimg->colr != None) {
		if (use_clip_mask) {
		    if (tkimg->mask != None) {
			/* Set the clip mask. */
			XSetClipOrigin(dpy, gc, sx2, sy2);
			XSetClipMask(dpy, gc, tkimg->mask);
		    }
		} else {
		    if (tkimg->mask != None) {
			XSetFunction(dpy, gc, GXand);
			XCopyArea(dpy, tkimg->mask, mapw->d, gc,
				  0, 0, sw, sh, sx, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		/* Draw the color image. */
		XCopyArea(dpy, tkimg->colr, mapw->d, gc, 0, 0, sw, sh, sx, sy);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    } else if (tkimg->mono != None || tkimg->mask != None) {
	      if (use_clip_mask) {
		/* Set the origin for any subsequent clipping. */
		XSetClipOrigin(dpy, gc, sx2, sy2);
		/* Set the color we're going to use for the mask; use
		   the imagecolor if we'll be using the mask as the
		   only image. */
		XSetForeground(dpy, gc,
			       (tkimg->mono == None ? imagecolor 
						    : maskcolor)->pixel);
		/* Set the clip mask to be explicit mask or unit's image. */
		if (tkimg->mask)
		  XSetClipMask(dpy, gc, tkimg->mask);
		else
		  XSetClipMask(dpy, gc, tkimg->mono);
		/* Draw the mask. */
		XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
		/* Draw the image proper. */
		if (tkimg->mono != None) {
		    XSetForeground(dpy, gc, imagecolor->pixel);
		    XSetClipMask(dpy, gc, tkimg->mono);
		    XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
		}
	      } else {
		/* Use the mask to make a black hole. */
		if (tkimg->mask != None) {
		  XSetFunction(dpy, gc, GXand);
		  XCopyArea(dpy, tkimg->mask, mapw->d, gc,
			    0, 0, sw, sh, sx, sy);
		}
		/* OR the mono image into the hole. */
		if (tkimg->mono != None) {
		  XSetFunction(dpy, gc, GXor);
		  XCopyArea(dpy, tkimg->mono, mapw->d, gc,
			    0, 0, sw, sh, sx, sy);
		}
		XSetFunction(dpy, gc, GXcopy);
	      }
	    }
	} else {
	    desperate = TRUE;
	}
    } else {
	desperate = TRUE;
    }
    /* If all else fails, draw a black box and maybe a white border. */
    if (desperate) {
	XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	XFillRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
	if (sw >= 16) {
	    XSetForeground(dpy, gc, dside->ui->whitecolor->pixel);
	    XDrawRectangle(dpy, mapw->d, gc, sx, sy, sw, sh);
	}
    }
    /* Take care of effects and decorations. */
    if (!desperate) {
	/* Draw a fadeout over the image. */
	/*! \todo Change fadeout to be according to unit view age rather than 
		  completeness. */
	draw_uimg_fadeout(tkimg, mapw, sx, sy, mod, desperate);
	/* Draw unit emblem. */
	draw_uimg_emblem(uimg, mapw, sx, sy, s2);
    }
}

/* Draw an emblem identifying the given side.  If a side does not have a
   distinguishing emblem, fall back on some defaults. */

void
draw_side_emblem(MapW *mapw, int ex, int ey, int ew, int eh, int s2)
{
    int ex2, ey2, ew2, eh2;
    XColor *imagecolor, *maskcolor;
    Image *eimg;
    TkImage *tkimg;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    /* Draw the emblem's mask, or else an enclosing box. */
    eimg = best_image(dside->ui->eimages[s2], ew, eh);
    if (eimg != NULL) {
	ew2 = eimg->w;  eh2 = eimg->h;
	/* Arbitrarily make an 8x6 flag for solid-color emblems. */
	if (ew2 == 1 && eh2 == 1) {
	    ew2 = 8;  eh2 = 6;
	}
	/* Offset the image to draw in the middle of its area,
	   whether larger or smaller than the given area. */
	ex2 = ex + (ew - ew2) / 2;  ey2 = ey + (eh - eh2) / 2;
	/* Only change the size of the rectangle being drawn if it's
	   smaller than what was passed in. */
	if (ew2 < ew) {
	    ex = ex2;
	    ew = ew2;
	}
	if (eh2 < eh) {
	    ey = ey2;
	    eh = ew2;
	}
	tkimg = (TkImage *) eimg->hook;
	/* Decide on the colors to use with the emblem. */
	if (dside->ui->numcolors[s2] > 0) {
	    imagecolor = dside->ui->colors[s2][0];
	} else {
	    imagecolor = dside->ui->blackcolor;
	}
	if (dside->ui->numcolors[s2] > 1) {
	    maskcolor = dside->ui->colors[s2][1];
	} else {
	    maskcolor = dside->ui->whitecolor;
	}
	/* If the emblem is a solid color, fill the entire emblem area
	   with that color. */
	if (eimg->w == 1 && eimg->h == 1 && tkimg->solid != NULL) {
	    XSetClipMask(dpy, gc, None);
	    XSetForeground(dpy, gc, tkimg->solid->pixel);
	    XFillRectangle(dpy, mapw->d, gc, ex, ey, ew, eh);
	    XSetForeground(dpy, gc, dside->ui->blackcolor->pixel);
	    XDrawRectangle(dpy, mapw->d, gc, ex, ey, ew, eh);
	    return;
	}
	/* Draw the mask. */
	XSetForeground(dpy, gc, maskcolor->pixel);
	if (use_clip_mask)
	  XSetClipOrigin(dpy, gc, ex, ey);
	if (tkimg != NULL && tkimg->mask != None) {
	  if (use_clip_mask) {
	    XSetClipMask(dpy, gc, tkimg->mask);
	    XFillRectangle(dpy, mapw->d, gc, ex, ey, ew, eh);
	  } else {
	    XSetFunction(dpy, gc, GXand);
	    XCopyArea(dpy, tkimg->mask, mapw->d, gc, 0, 0, ew, eh, ex, ey);
	    XSetFunction(dpy, gc, GXor);
	  }
	} else {
	    XSetClipMask(dpy, gc, None);
	    XFillRectangle(dpy, mapw->d, gc, ex - 1, ey, ew + 1, eh + 1);
	}
	/* Now draw the emblem proper. */
	if (dside->ui->dflt_color_embl_images
	    && tkimg != NULL
	    && tkimg->colr != None) {
	    /* Draw the color image. */
	    XCopyArea(dpy, tkimg->colr, mapw->d, gc, 0, 0, ew, eh, ex, ey);
	} else {
	    XSetForeground(dpy, gc, imagecolor->pixel);
	    if (use_clip_mask) {
	      XSetClipMask(dpy, gc, (tkimg != NULL ? tkimg->mono : None));
	      XFillRectangle(dpy, mapw->d, gc, ex, ey, ew, eh);
	    }
	}
	if (!use_clip_mask)
	  XSetFunction(dpy, gc, GXcopy);
    }
}

static void
draw_country_border_line(MapW *mapw, int sx, int sy, int dir, int con)
{
    int viewpow = mapw->vp->power;
    int wid = bwid[viewpow], dir2;
    Display *dpy = mapw->display;
    GC gc = mapw->gc;

    if (wid == 0)
      return;
    wid = max(1, wid / 2);
    XSetClipMask(dpy, gc, None);
    XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter);
    if (con)
      XSetForeground(dpy, gc, dside->ui->frontline_color->pixel);
    else
      XSetForeground(dpy, gc, dside->ui->country_border_color->pixel);
    if (mapw->vp->isometric) {
	dir2 = dir_subtract(dir, mapw->vp->isodir);
	XDrawLine(dpy, mapw->d, gc,
		  sx + ibsx[viewpow][dir2], sy + ibsy[viewpow][dir2],
		  sx + ibsx[viewpow][dir2+1], sy + ibsy[viewpow][dir2+1]);
    } else {
	XDrawLine(dpy, mapw->d, gc,
		  sx + bsx[viewpow][dir], sy + bsy[viewpow][dir],
		  sx + bsx[viewpow][dir+1], sy + bsy[viewpow][dir+1]);
    }
    XSetFillStyle(dpy, gc, FillSolid);
    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
}

/* Draw an outline of the main map in the world map.  We draw directly on
   the screen and use inversion to avoid having to recalc anything
   underneath. */

static void
draw_map_outline(MapW *worldw, MapW *mapw, int draw, int direct)
{
    int i, sx, sy, sw, sh, x, y, moved = FALSE;
    int wsx[4], wsy[4];
    Display *dpy = worldw->display;
    Drawable d;
    GC gc = worldw->gc;
    XPoint points[5];
    VP *vp = mapw->vp;

    /* Choose whether to draw directly on the screen or to the
       widget's buffer. */
    if (direct)
      d = Tk_WindowId(worldw->tkwin);
    else
      d = worldw->d;
    /* (should make generic) */
    if (mapw->vp->isometric) {
#if 0
	/* (should use this instead of doing nearest_cell/xform cell) */
	scale_point(worldw->vp, vp, 0, 0, &(wsx[0]), &(wsy[0]));
	scale_point(worldw->vp, vp, vp->pxw, 0, &(wsx[1]), &(wsy[1]));
	scale_point(worldw->vp, vp, vp->pxw, vp->pxh, &(wsx[2]), &(wsy[2]));
	scale_point(worldw->vp, vp, 0, vp->pxh, &(wsx[3]), &(wsy[3]));
#else
	/* Note that the following nearest cells and scaled points will
	   be offworld, but be laid back about it, let the world map's
	   window do any necessary clipping. */
	nearest_cell(vp, 0, 0, &x, &y, NULL, NULL);
	xform_cell(worldw->vp, x, y, &(wsx[0]), &(wsy[0]));
	nearest_cell(vp, vp->pxw, 0, &x, &y, NULL, NULL);
	xform_cell(worldw->vp, x, y, &(wsx[1]), &(wsy[1]));
	nearest_cell(vp, vp->pxw, vp->pxh, &x, &y, NULL, NULL);
	xform_cell(worldw->vp, x, y, &(wsx[2]), &(wsy[2]));
	nearest_cell(vp, 0, vp->pxh, &x, &y, NULL, NULL);
	xform_cell(worldw->vp, x, y, &(wsx[3]), &(wsy[3]));
#endif
    } else {
#if 0
	scale_point(worldw->vp, vp, 0, 0, &(wsx[0]), &(wsy[0]));
	scale_point(worldw->vp, vp, vp->pxw, 0, &(wsx[1]), &(wsy[1]));
	scale_point(worldw->vp, vp, vp->pxw, vp->pxh, &(wsx[2]), &(wsy[2]));
	scale_point(worldw->vp, vp, 0, vp->pxh, &(wsx[3]), &(wsy[3]));
#else
	scale_vp(worldw->vp, mapw->vp, &sx, &sy, &sw, &sh);
	wsx[0] = sx;       
	wsy[0] = sy;
	wsx[1] = sx + sw;  
	wsy[1] = sy;
	wsx[2] = sx + sw;  
	wsy[2] = sy + sh;
	wsx[3] = sx;       
	wsy[3] = sy + sh;
#endif
    }
    XSetClipMask(dpy, gc, None);
    XSetFunction(dpy, gc, GXinvert);
    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
    /* Undo the last draw (if it exists). */
    if (!draw && worldw->last_ws_valid) {
	for (i = 0; i < 4; ++i) {
	    points[i].x = worldw->last_wsx[i];
	    points[i].y = worldw->last_wsy[i];
	}
    /* Or draw the new set of lines. */
    } else if (draw) {
	for (i = 0; i < 4; ++i) {
	    points[i].x = wsx[i];
	    points[i].y = wsy[i];
	}
    }
    points[4].x = points[0].x;
    points[4].y = points[0].y;
    /* Check if the map was moved or scrolled. */
    if (worldw->last_ws_valid) {
	for (i = 0; i < 4; ++i) {
	    if (worldw->last_wsx[i] != wsx[i]  
		|| worldw->last_wsy[i] != wsy[i]) {
		moved = TRUE;
	    }
	}
    }
    if ((!draw && worldw->last_ws_valid) || draw) {
#if 0
	/* This code causes an interface freeze after 34-35 turns 
	on the mac. Should figure out why. It seems that many calls 
	to XDrawLine after each other lose, even if it is the same line 
	that is redrawn again and again. If Xdrawline is called directly
	instead of XDrawlines, the freeze comes after 22-23 turns.
	If XDrawRectangle is used (in the non-isometric case) the 
	freeze comes after 34-35 turns. It does not matter which line 
	is drawn or whether the view is isometric or not. Nor does it 
	matter if we are erasing or re-drawing the line. When the 
	freeze happens, the interface hangs for several minutes, after 
	which the game proceeds as normal. However, if the game is 
	running under ai control, it goes through 10-15 turns or so 
	while the interface is frozen. So the kernel is not affected by 
	the interface freeze. */
	XDrawLines(dpy, d, gc, points, 5, CoordModeOrigin);
#else
	/* This code draws only those line segments that intersect the
	update rect. It fixes the above problem. The min/max stuff is
	there to make things work also during isometric drawing. */
	if (worldw->rsh < 0 || worldw->rsw < 0 || moved
	    || (min(points[0].x, points[1].x) <=  worldw->rsx + worldw->rsw 
	    && max(points[0].x, points[1].x) >= worldw->rsx
	    && min(points[0].y, points[1].y) <= worldw->rsy + worldw->rsh
	    && max(points[0].y, points[1].y) >= worldw->rsy)) {
	    XDrawLine(dpy, d, gc, points[0].x, points[0].y, 
		      points[1].x - 1, points[1].y);	
    	}	
	if (worldw->rsh < 0 || worldw->rsw < 0 || moved
	    || (min(points[1].y, points[2].y) <= worldw->rsy + worldw->rsh 
	    && max(points[1].y, points[2].y) >= worldw->rsy
	    && max(points[1].x, points[2].x) >= worldw->rsx
	    && min(points[1].x, points[2].x) <= worldw->rsx + worldw->rsw)) {
	    XDrawLine(dpy, d, gc, points[1].x, points[1].y, 
		      points[2].x, points[2].y - 1);	
    	}	
	if (worldw->rsh < 0 || worldw->rsw < 0 || moved
	    || (min(points[2].x, points[3].x) <= worldw->rsx + worldw->rsw
	    && max(points[2].x, points[3].x) >= worldw->rsx
	    && min(points[2].y, points[3].y) <= worldw->rsy + worldw->rsh 
	    && max(points[2].y, points[3].y) >= worldw->rsy)) {
	    XDrawLine(dpy, d, gc, points[2].x, points[2].y, 
		      points[3].x + 1, points[3].y);	
    	}	
	if (worldw->rsh < 0 || worldw->rsw < 0 || moved
	    || (min(points[3].y, points[0].y) <= worldw->rsy + worldw->rsh 
	    && max(points[3].y, points[0].y) >= worldw->rsy
	    && max(points[3].x, points[0].x) >= worldw->rsx
	    && min(points[3].x, points[0].x) <= worldw->rsx + worldw->rsw)) {
	    XDrawLine(dpy, d, gc, points[3].x, points[3].y, 
		      points[0].x, points[0].y + 1);	
    	}	
#endif
   }
    XFlush(dpy);
    XSetFunction(dpy, gc, GXcopy);
    /* Record the location of the set we just drew (but not after erasing). */
    if (draw) {
	for (i = 0; i < 4; ++i) {
	    worldw->last_wsx[i] = wsx[i];  
	    worldw->last_wsy[i] = wsy[i];
	}
	worldw->last_ws_valid = TRUE;
    }
}

/* Put the point x, y in the center of the mapw, or at least as close
   as possible. */

void
recenter(Map *map, int x, int y)
{
    int oldsx, oldsy;
    VP *vp;

    if (map->widget == NULL)
      return;
    vp = widget_vp(map);
    oldsx = vp->sx;  oldsy = vp->sy;
    set_view_focus(vp, x, y);
    center_on_focus(vp);
    if (vp->sx != oldsx || vp->sy != oldsy) {
	redraw_map(map);
    }
}

/* Ensure that given location is visible on the front map.  We
   (should) also flush the input because any input relating to a
   different screen is probably worthless. */

void
put_on_screen(Map *map, int x, int y)
{
    /* Ugly hack to prevent extra boxes being drawn during init - don't ask!*/
    if (x == 0 && y == 0)
      return;
    if (!in_middle(map, x, y))
      recenter(map, x, y);
}

/* Decide whether given location is not too close to edge of screen.
   We do this because it's a pain to move units when half the adjacent
   places aren't even visible.  This routine effectively places a
   lower limit of 5x5 for the map window. (I think) */

int
in_middle(Map *map, int x, int y)
{
    int sx, sy, insetx1, insety1, insetx2, insety2;
    MapW *mapw = (MapW *) map->widget;

    if (mapw == NULL)
      return FALSE;

    xform(mapw, x, y, &sx, &sy);
    /* Adjust to be the center of the cell, more reasonable if large. */
    sx += mapw->vp->hw / 2;  sy += mapw->vp->hh / 2;
    insetx1 = min(mapw->vp->pxw / 4, 1 * mapw->vp->hw);
    insety1 = min(mapw->vp->pxh / 4, 1 * mapw->vp->hch);
    insetx2 = min(mapw->vp->pxw / 4, 2 * mapw->vp->hw);
    insety2 = min(mapw->vp->pxh / 4, 2 * mapw->vp->hch);
    if (sx < insetx2)
      return FALSE;
    if (sx > mapw->vp->pxw - insetx2)
      return FALSE;
    if (sy < (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    if (sy > mapw->vp->pxh - (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    return TRUE;
}

int mouse_is_down;

void
handle_mouse_down(Map *map, int sx, int sy, int button)
{
    int ax, ay;
    void (*fn)(Side *sidex, Map *mapx, int cancelledx);
    VP *vp = widget_vp(map);

    mouse_is_down = TRUE;
    if (map == NULL) {
	beep(dside);
	return;
    }
    if (!x_nearest_cell((MapW *) map->widget, sx, sy, &ax, &ay)) {
	beep(dside);
	return;
    }
    map->inpx = ax;  map->inpy = ay;
    x_nearest_unit((MapW *) map->widget, sx, sy, &(map->inpunit));
    /* Assume that last place clicked is a reasonable focus. */
    if (inside_area(ax, ay)) {
	set_view_focus(vp, ax, ay);
    }
#ifdef DESIGNERS
    if (is_designer(dside) && dside->ui->curdesigntool != survey_mode) {
	handle_designer_mouse_down(dside, map, sx, sy);
	return;
    }
#endif /* DESIGNERS */
    if (map->modalhandler) {
	fn = map->modalhandler;
	map->modalhandler = NULL;
	(*fn)(dside, map, 0);
	return;
    }

    switch (map->mode) {
      case survey_mode:
	if (button == 1) {
	    move_look(map, sx, sy);
	} else if (button == 3) {
	    if (map->curunit && side_controls_unit(dside, map->curunit)) {
		move_the_selected_unit(map, map->curunit, sx, sy);
	    } else {
		beep(dside);
	    }
	}
	break;
      case move_mode:
	/* Both left and right buttons do the same thing in this mode. */
	if (map->curunit && side_controls_unit(dside, map->curunit)) {
	    move_the_selected_unit(map, map->curunit, sx, sy);
	} else {
	    beep(dside);
	}
	break;
     default:
	/* error eventually */
	break;
    }
}

void
move_look(Map *map, int sx, int sy)
{
    int nx, ny;
    Unit *unit = NULL;
    MapW *mapw = (MapW *) map->widget;

    if (x_nearest_cell(mapw, sx, sy, &nx, &ny)) {
	if (inside_area(nx, ny)) {
	    x_nearest_unit(mapw, sx, sy, &unit);
	    if (unit != NULL
		&& !(side_controls_unit(dside, unit) || endofgame))
	      unit = NULL;
	    set_current_unit(map, unit);
	} else {
	    beep(dside);
	}
    }
}

void
move_the_selected_unit(Map *map, Unit *unit, int sx, int sy)
{
    int x, y, rslt;
    HistEventType reason;
    Unit *other = NULL;
    MapW *mapw = (MapW *) map->widget;
    char failbuf[BUFSIZE];

    if (!unit->act || !unit->plan || !mobile(unit->type)) {
	notify(dside, "Cannot move this unit!");		
	notify(dside, "Please pick another unit.");		
	beep(dside);
	return;
    }
    if (!x_nearest_cell(mapw, sx, sy, &x, &y)) {
	notify(dside, "Cannot move unit outside the map!");		
	notify(dside, "Please pick a new destination.");		
	beep(dside);
	return;
    }
    x_nearest_unit(mapw, sx, sy, &other);
    rslt = advance_into_cell(dside, unit, x, y, other, &reason);
    if (!rslt) {
	advance_failure_desc(failbuf, unit, reason);
	if (!empty_string(failbuf)) {
		notify(dside, "%s", failbuf);
	}
	beep(dside);
    }
}

void
handle_mouse_up(Map *map, int sx, int sy, int button)
{
    mouse_is_down = FALSE;
}

void
handle_world_mouse_down(Map *map, int sx, int sy, int button)
{
    int x, y;

    if (map == NULL) {
	beep(dside);
	return;
    }
    if (!x_nearest_cell((MapW *) map->worldw, sx, sy, &x, &y)) {
	beep(dside);
	return;
    }
    recenter(map, x, y);
}

void
handle_world_mouse_up(Map *map, int sx, int sy, int button)
{
}

VP *
widget_vp(Map *map)
{
    MapW *mapw = (MapW *) map->widget;

    return (mapw ? mapw->vp : NULL);
}

VP *
worldw_vp(Map *map)
{
    MapW *mapw = (MapW *) map->worldw;

    return (mapw ? mapw->vp : NULL);
}

/* Force a refresh of the map widgets in a map. */

void
redraw_map(Map *map)
{
    MapW *mapw, *worldw;

    mapw = (MapW *) map->widget;
    if (mapw == NULL)
      return;
    eventually_redraw(mapw, -1, -1, -1, -1);
    worldw = (MapW *) map->worldw;
    if (worldw == NULL)
      return;
    eventually_redraw(worldw, -1, -1, -1, -1);
    eval_tcl_cmd("update idletasks");
}

void
set_show_all(Map *map, int value)
{
    MapW *mapw, *worldw;

    map->show_all = value;

    mapw = (MapW *) map->widget;
    if (mapw == NULL)
      return;
    /* Make the viewport see_all match the map's value. */
    mapw->vp->show_all = map->show_all;
    worldw = (MapW *) map->worldw;
    if (worldw == NULL)
      return;
    /* Make the viewport see_all match the map's value. */
    worldw->vp->show_all = map->show_all;
    eval_tcl_cmd("update_show_all %d %d", map->number, map->show_all);

}

void
set_tool_cursor(Map *map, int which)
{
    MapW *mapw;
    Tk_Cursor cursor;

    if (which == 0) {
	mapw = (MapW *) map->widget;
	if (mapw == NULL)
	  return;
	cursor = dside->ui->cursors[map->mode];
	if (map->tmp_mode != no_tmp_mode)
	  cursor = dside->ui->cursors[map->tmp_mode];
	if (cursor == None)
	  cursor = dside->ui->cursors[survey_mode];
    } else {
	mapw = (MapW *) map->worldw;
	if (mapw == NULL)
	  return;
	cursor = None;
    }
    if (map->scroll_mode[which] != no_scroll_mode)
      cursor = dside->ui->cursors[map->scroll_mode[which]];
    Tk_DefineCursor(mapw->tkwin, cursor);
}

int painttype;
int paintdepth;
int paintctrl;
int paintpeop;
int paintfid;
int paint_elev;
int paint_elev_code;
int paint_elev_vary;
int painttview;
int paintuview = FALSE;  /* (should have a checkbox to set) */

void
handle_designer_mouse_down(Side *side, Map *map, int sx, int sy)
{
    int x, y, dir, oldt, olddepth, oldctrl, oldpeop, oldfid;
    int oldtview /*, olduview*/;
    int brush;
    Unit *unit;

    nearest_boundary(widget_vp(map), sx, sy, &x, &y, &dir);
    brush = dside->ui->curbrushradius;
    switch (dside->ui->curdesigntool) {
      case cell_paint_mode:
	/* Choose to paint fg or bg type, depending on what's already there. */
	oldt = terrain_at(x, y);
	painttype = (dside->ui->curttype == oldt ? dside->ui->curbgttype : dside->ui->curttype);
	net_paint_cell(dside, x, y, brush, painttype);
	break;
      case bord_paint_mode:
	net_paint_border(dside, x, y, dir, dside->ui->curttype, -1);
	break;
      case conn_paint_mode:
	net_paint_connection(dside, x, y, dir, dside->ui->curttype, -1);
	break;
      case coat_paint_mode:
	allocate_area_aux_terrain(dside->ui->curttype);
	olddepth = aux_terrain_at(x, y, dside->ui->curttype);
	paintdepth = (olddepth == 1 ? 0 : 1);
	net_paint_coating(dside, x, y, brush, dside->ui->curttype, paintdepth);
	break;
      case unit_paint_mode:
	unit = net_designer_create_unit(dside, dside->ui->curutype,
					dside->ui->curusidenumber, x, y);
	if (unit != NULL) {
	    set_current_unit(map, unit);
	}
	break;
      case people_paint_mode:
	/* Paint people or clear, inverting from what is already here. */
	oldpeop = (people_sides_defined() ? people_side_at(x, y) : NOBODY);
	paintpeop = (dside->ui->curpeoplenumber == oldpeop ? NOBODY : dside->ui->curpeoplenumber);
	net_paint_people(dside, x, y, brush, paintpeop);
	break;
      case control_paint_mode:
	/* Paint control or clear it, inverting from what is already here. */
	oldctrl = (control_sides_defined() ? control_side_at(x, y) : NOCONTROL);
	paintctrl = (dside->ui->curcontrolnumber == oldctrl ? NOCONTROL : dside->ui->curcontrolnumber);
	net_paint_control(dside, x, y, brush, paintctrl);
	break;
      case feature_paint_mode:
	oldfid = (features_defined() ? raw_feature_at(x, y) : 0);
	paintfid = (dside->ui->curfid == oldfid ? 0 : dside->ui->curfid);
	net_paint_feature(dside, x, y, brush, paintfid);
	break;
      case elevation_paint_mode:
	paint_elev = dside->ui->curelevation;
	paint_elev_code = dside->ui->curelevationcode;
	paint_elev_vary = dside->ui->curelevationvary;
	net_paint_elevation(dside, x, y, brush, paint_elev_code, paint_elev,
			    paint_elev_vary);
	break;
      case temperature_paint_mode:
	net_paint_temperature(dside, x, y, brush, dside->ui->curtemperature);
	break;
      case clouds_paint_mode:
	net_paint_clouds(dside, x, y, brush, dside->ui->curcloudtype,
			dside->ui->curcloudbottom, dside->ui->curcloudheight);
	break;
      case winds_paint_mode:
	net_paint_winds(dside, x, y, brush,
			dside->ui->curwinddir, dside->ui->curwindforce);
	break;
      case view_paint_mode:
	if (dside->terrview == NULL || dside->unit_views == NULL)
	  break;
	oldtview = terrain_view(dside, x, y);
	painttview = (oldtview == UNSEEN ? 1234567 : UNSEEN);
	net_paint_view(dside, x, y, brush, painttview, paintuview);
	/* (should paint other view layers also) */
	break;
      default:
	break;
    }
    update_contour_intervals();
    update_view_controls_info();
}

void
update_contour_intervals(void)
{
    int oldinterval;
    Map *map;
    MapW *mapw;

    for_all_maps(map) {
	mapw = (MapW *) map->widget;
	if (mapw->vp->contour_interval_fixed) {
	  set_contour_interval(mapw->vp, mapw->vp->contour_interval);
	} else {
	    oldinterval = mapw->vp->contour_interval;
	    set_contour_interval(mapw->vp, 0);
	    if (mapw->vp->contour_interval != oldinterval)
	      notify(dside, "Contour interval on map %d is now %d, was %d.",
		     map->number, mapw->vp->contour_interval, oldinterval);
	}
    }
}

/* Call this repeatedly when dragging around the mouse while button
   is down, to drag-paint the current type of layer data. */

void
paint_on_drag(Map *map, int sx, int sy, int mods)
{
    int x, y, brush;

    if (!mouse_is_down)
      return;
    if (x_nearest_cell((MapW *) map->widget, sx, sy, &x, &y)) {
	brush = dside->ui->curbrushradius;
	switch (dside->ui->curdesigntool) {
	  case cell_paint_mode:
	    net_paint_cell(dside, x, y, brush, painttype);
	    break;
	  case coat_paint_mode:
	    net_paint_coating(dside, x, y, brush, dside->ui->curttype, paintdepth);
	    break;
	  case unit_paint_mode:
	    /* Drag-painting is almost never useful for unit placement,
	       so don't do anything. */
	    break;
	  case people_paint_mode:
	    net_paint_people(dside, x, y, brush, paintpeop);
	    break;
	  case control_paint_mode:
	    net_paint_control(dside, x, y, brush, paintctrl);
	    break;
	  case feature_paint_mode:
	    net_paint_feature(dside, x, y, brush, paintfid);
	    break;
	  case elevation_paint_mode:
	    net_paint_elevation(dside, x, y, brush, paint_elev_code,
				paint_elev, paint_elev_vary);
	    break;
	  case temperature_paint_mode:
	    net_paint_temperature(dside, x, y, brush, 
	    		     dside->ui->curtemperature);
	    break;
	  case clouds_paint_mode:
	    net_paint_clouds(dside, x, y, brush, dside->ui->curcloudtype, 
			    dside->ui->curcloudbottom, dside->ui->curcloudheight);
	    break;
	  case winds_paint_mode:
	    net_paint_winds(dside, x, y, brush,
			    dside->ui->curwinddir, dside->ui->curwindforce);
	    break;
	  case view_paint_mode:
	    net_paint_view(dside, x, y, brush, painttview, paintuview);
	    break;
	  default:
	    break;
	}
    }
}

/* Add the given rectangle to the area that we plan to redraw at the
   next update. */

static void
eventually_redraw(MapW *mapw, int sx, int sy, int sw, int sh)
{
    int sx1, sy1, sx2, sy2;

    if (sw >= 0 && sh >= 0) {
	if (mapw->rsw == -2) {
		mapw->rsx = sx;  
		mapw->rsy = sy;
		mapw->rsw = sw;
		mapw->rsh = sh;

/* The code below just clips the update rect to the map. It was an attempt to handle
the offset error when scrolling to the end of the map border while using expanded 
update rects because of big icons. This fixed the offset bug, but for some mysterious 
reason scrolling became extremely slow as soon as the current unit was no longer 
visible on the map. As an alternative solution, the big icons code has been moved to
update_cell, so that it is never executed when scrolling. This fixes both problems,
but it would be better to figure out how to clip the update rect without scrolling
problems, since there are other reasons (e.g. isometric drawing) why it may get 
expanded beyond the map border. */

#if 0
		/* Clip the redraw rect to the map boundaries. */
		if (mapw->rsx < 0) {
			mapw->rsw -= (0 -mapw->rsx);	//Problem when scrolling after current unit.
			mapw->rsx = 0;
		}
		if (mapw->rsy < 0) {
			mapw->rsh -= (0 -mapw->rsy);		//Problem when scrolling after current unit.
			mapw->rsy = 0;
		}
		if (mapw->rsx + mapw->rsw >= mapw->vp->pxw) {
			mapw->rsw = mapw->vp->pxw - mapw->rsx;	//Problem when scrolling before current unit.
		}
		if (mapw->rsy + mapw->rsh > mapw->vp->pxh) {
			mapw->rsh = mapw->vp->pxh - mapw->rsy;	//Problem when scrollling before current unit.
		}
#endif

	} else if (mapw->rsw == -1) {
	    /* do nothing, already drawing whole map */
	} else {
	    sx1 = mapw->rsx;  sy1 = mapw->rsy;
	    sx2 = mapw->rsx + mapw->rsw;  sy2 = mapw->rsy + mapw->rsh;
	    if (sx < sx1)
	      sx1 = sx;
	    if (sy < sy1)
	      sy1 = sy;
	    if (sx + sw > sx2)
	      sx2 = sx + sw;
	    if (sy + sh > sy2)
	      sy2 = sy + sh;
	    mapw->rsx = sx1;  mapw->rsy = sy1;
	    mapw->rsw = sx2 - sx1;  mapw->rsh = sy2 - sy1;
	}
    } else {
	mapw->rsx = mapw->rsy = mapw->rsw = mapw->rsh = -1;
    }
    if (!mapw->update_pending) {
	Tcl_DoWhenIdle(mapw_display, (ClientData) mapw);
	mapw->update_pending = TRUE;
    }
}

void
update_cell(Map *map, int x, int y)
{
    int namelength = 1, cells = 1, sx, sy, sw, sh, i;
    MapW *mapw, *worldw;
    Unit *unit;

    if (!in_area(x, y))
      return;
    mapw = (MapW *) map->widget;
    if (mapw == NULL)
      return;

    /* Find max length of a unit name (skip at 4 x 4 or below). */
    if (mapw->vp->draw_names && mapw->vp->power > 2) {
	if (mapw->vp->power == 3)
	    namelength = 11;
	if (mapw->vp->power == 4)
	    namelength = 6;
	if (mapw->vp->power == 5)
	    namelength = 4;
	if (mapw->vp->power == 6)
	    namelength = 3;
    }
    /* Skip cells that are not inside the area. */
    if (!area.xwrap) {
	for (i = 1; i <= namelength;  i++) {
	    if (!in_area(x + i, y)) {
		namelength = i;
		break;
	    }
	}
    }	
    /* Then check if an erasing row really is needed. */
    if (namelength > 1) {
	if (mapw->vp->show_all) {
		for_all_units(unit) {
		    /* Only check named units. */
		    if (!unit->name)
			    continue;
		    /* First check if the unit is located at (x, y).*/
		    if (is_located_at(unit, x, y) 
			/* Also check if it just left (x, y). 
			   Catches unit that died!*/
			|| was_located_at(unit, x, y)) {
			    /* Make an erasing row if either is true. */
			    cells = namelength;
			    break;
		    }
		}
	} else {
	    /* We can't figure out if a unit view got erased in this update, so 
	       we always do the erasing row. */ 
	    cells = namelength;
	}
    }
    xform(mapw, x, y, &sx, &sy);
    sh = mapw->vp->hh;
    sw = mapw->vp->hw * cells;
#if (0)
    /* Even though big icons fit within one hex, the selection box
       and health bar does not. */
    if (big_unit_images) {
	sx -= 3;
	sy -= 4;
	sw += 6;
	sh += 7; 
    }
#endif
    /* Unit images may extend up into the hex row above. */
    if (mapw->vp->isometric) {
	sy -= mapw->vp->hh / 2;
	sh += mapw->vp->hh / 2;
    }
    /* Clip the result to avoid misaligned updates at the edges. */
    sx = limitn(0, sx, mapw->vp->totsw);
    sy = limitn(0, sy, mapw->vp->totsh);
    sw = limitn(0, sw, mapw->vp->totsw - sx);
    sh = limitn(0, sh, mapw->vp->totsh - sy);

    eventually_redraw(mapw, sx, sy, sw, sh);
    worldw = (MapW *) map->worldw;
    if (worldw != NULL) {
	xform(worldw, x, y, &sx, &sy);
	eventually_redraw(worldw, sx, sy, worldw->vp->hw, worldw->vp->hh);
    }
    eval_tcl_cmd("update idletasks");
}

void
draw_fire_line(Map *map, Unit *unit, Unit *unit2, int x2, int y2)
{
    int i, sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2, dx, dy, xx, yy;
    MapW *mapw;

    mapw = (MapW *) map->widget;
    if (mapw == NULL)
      return;
    {
	Display *dpy = mapw->display;
	Drawable d = Tk_WindowId(mapw->tkwin);
	GC gc = mapw->gc;

	XSetClipMask(dpy, gc, None);
	XSetFunction(dpy, gc, GXinvert);
	for (i = 0; i < 32; ++i) {
	    x_xform_unit_self(mapw, unit, &sx1, &sy1, &sw1, &sh1);
	    if (unit2 != NULL) {
		x_xform_unit_self(mapw, unit2, &sx2, &sy2, &sw2, &sh2);
	    } else {
		xform(mapw, x2, y2, &sx2, &sy2);
		sw2 = mapw->vp->hw;  sh2 = mapw->vp->hh;
	    }
	    compute_fire_line_segment(sx1 + sw1 / 2, sy1 + sh1 / 2,
				      sx2 + sw2 / 2, sy2 + sh2 / 2,
				      i, 4, &xx, &yy, &dx, &dy);
	    XDrawLine(dpy, d, gc, xx, yy, xx + dx, yy + dy);
	    XFlush(dpy);
	    /* (should do a busy wait if necessary to ensure timing here) */
	    Tcl_Sleep(10);
	}
	XSetFunction(dpy, gc, GXcopy);
    }
}

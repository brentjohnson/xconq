/* Initialization for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "aiunit.h"
#include "tkconq.h"

void init_other_images(void);
void really_do_design(Side *side);

void set_colors(void);
void init_emblem(Side *side2);

int can_give_take(Unit *unit);

Pixmap fuzzpics[NUMPOWERS];
Pixmap windpics[5][NUMDIRS];
Pixmap antpic;

int any_resources;

int grid_matches_unseen;

short *utype_indexes;
short *mtype_indexes;

static void set_optional_colors(void);
static void init_cursors(void);
static Tk_Cursor make_cursor(Tk_Window tkwin, char *cursbits, char *maskbits, int x, int y);
static void init_bitmaps(void);

static void init_material_images(void);
static void init_terrain_images(void);
static void init_emblem_images(void);
static void report_missing_images(void);
static void tk_describe_image(Side *side, Image *img);

/* Display and window globals used in callbacks from generic imf
   handling code. */

extern int tmp_valid;

extern Tk_Window tmp_root_window;

/* various bitmap definitions. */
#include "bitmaps/lookglass.b"
#include "bitmaps/lookmask.b"
#if 0
#include "bitmaps/movecurs.b"
#include "bitmaps/movemask.b"
#endif
#include "bitmaps/shootcurs.b"
#include "bitmaps/shootmask.b"
#include "bitmaps/attackcurs.b"
#include "bitmaps/attackmask.b"
#include "bitmaps/buildcurs.b"
#include "bitmaps/buildmask.b"

#include "bitmaps/scursup.b"
#include "bitmaps/scursupm.b"
#include "bitmaps/scursright.b"
#include "bitmaps/scursrightm.b"
#include "bitmaps/scursdown.b"
#include "bitmaps/scursdownm.b"
#include "bitmaps/scursleft.b"
#include "bitmaps/scursleftm.b"

#include "bitmaps/return.b"
#include "bitmaps/sleep.b"
#include "bitmaps/reserve.b"
#include "bitmaps/delay.b"

#include "bitmaps/closer.b"
#include "bitmaps/farther.b"
#include "bitmaps/iso.b"
#include "bitmaps/rotl.b"
#include "bitmaps/rotr.b"

#include "bitmaps/hex8.b"
#include "bitmaps/hex8b.b"
#include "bitmaps/hex16.b"
#include "bitmaps/hex16b.b"
#include "bitmaps/hex32.b"
#include "bitmaps/hex32b.b"
#include "bitmaps/hex64.b"
#include "bitmaps/hex64b.b"

#include "bitmaps/fuzz16.b"
#include "bitmaps/fuzz32.b"
#include "bitmaps/fuzz64.b"

#include "bitmaps/hex8iso.b"
#include "bitmaps/hex16iso.b"
#include "bitmaps/hex32iso.b"
#include "bitmaps/hex64iso.b"

#include "bitmaps/wind0.b"
#include "bitmaps/wind1ne.b"
#include "bitmaps/wind1e.b"
#include "bitmaps/wind1se.b"
#include "bitmaps/wind1sw.b"
#include "bitmaps/wind1w.b"
#include "bitmaps/wind1nw.b"
#include "bitmaps/wind2ne.b"
#include "bitmaps/wind2e.b"
#include "bitmaps/wind2se.b"
#include "bitmaps/wind2sw.b"
#include "bitmaps/wind2w.b"
#include "bitmaps/wind2nw.b"
#include "bitmaps/wind3ne.b"
#include "bitmaps/wind3e.b"
#include "bitmaps/wind3se.b"
#include "bitmaps/wind3sw.b"
#include "bitmaps/wind3w.b"
#include "bitmaps/wind3nw.b"
#include "bitmaps/wind4ne.b"
#include "bitmaps/wind4e.b"
#include "bitmaps/wind4se.b"
#include "bitmaps/wind4sw.b"
#include "bitmaps/wind4w.b"
#include "bitmaps/wind4nw.b"

#include "bitmaps/dots.b"
#include "bitmaps/ants.b"

#include "bitmaps/laurels.b"

#ifdef DESIGNERS
#include "bitmaps/curscell.b"
#include "bitmaps/curscellm.b"
#include "bitmaps/cursbord.b"
#include "bitmaps/cursbordm.b"
#include "bitmaps/cursconn.b"
#include "bitmaps/cursconnm.b"
#include "bitmaps/curscoat.b"
#include "bitmaps/curscoatm.b"
#include "bitmaps/cursunit.b"
#include "bitmaps/cursunitm.b"
#include "bitmaps/curspeop.b"
#include "bitmaps/curspeopm.b"
#include "bitmaps/cursfeat.b"
#include "bitmaps/cursfeatm.b"
#include "bitmaps/cursmaterial.b"
#include "bitmaps/cursmaterialm.b"
#include "bitmaps/curselev.b"
#include "bitmaps/curselevm.b"
#include "bitmaps/curstemp.b"
#include "bitmaps/curstempm.b"
#include "bitmaps/cursclouds.b"
#include "bitmaps/curscloudsm.b"
#include "bitmaps/curswinds.b"
#include "bitmaps/curswindsm.b"
#include "bitmaps/cursview.b"
#include "bitmaps/cursviewm.b"
#endif /* DESIGNERS */

void
init_ui(Side *side)
{
    /* Might be called a second time, deal with it. */
    if (dside != NULL)
      return;
    if (side_wants_display(side)) {
	if (dside == NULL) {
	    dside = side;
	    dside->ui = (UI *) xmalloc(sizeof(UI));
	    Dprintf("One UI is %d bytes.\n", sizeof(UI));
	    return;
	} else {
	    init_warning("Only one display allowed, cannot open for side %d",
			 side_number(side));
	}
    }
    side->ui = NULL;
}

/* Open display, create all the windows we'll need, do misc setup
   things, and initialize some globals to out-of-range values for
   recognition later. */

void
init_display(void)
{
    int u, s, i;
    char *dpyname;

    dpyname = dside->player->displayname;

    Dprintf("Will try to open %s display `%s'...\n",
	    side_desig(dside), dpyname);

    /* Detect the placeholder for the default display. */
    if (strcmp("_", dpyname) == 0)
      dpyname = getenv("DISPLAY");

    dside->ui->cell_color = (XColor **) xmalloc(numttypes * sizeof(XColor *));
    for (i = 0; i < 5; ++i)
      dside->ui->cell_shades[i] =
	(XColor **) xmalloc(numttypes * sizeof(XColor *));
    dside->ui->material_color =
      (XColor **) xmalloc(nummtypes * sizeof(XColor *));

    /* Set up things shared by all the windows. */
    dside->ui->dflt_color_embl_images = TRUE;
    dside->ui->default_meridian_interval = 600;
    for (s = 0; s <= MAXSIDES; ++s) {
	dside->ui->emblempics[s] = None;
	dside->ui->emblemmasks[s] = None;
    }
    set_colors();
    tmp_root_window = Tk_MainWindow(interp);
    tmp_valid = TRUE;

    if (DebugG == 2)
      XSynchronize(Tk_Display(tmp_root_window), True);

    init_ui_chars();
    init_unit_images(dside);
    init_material_images();
    init_terrain_images();
    init_emblem_images();
    init_other_images();
    tmp_valid = FALSE;
    report_missing_images();
    imf_describe_hook = tk_describe_image;
    set_optional_colors();
    init_bitmaps();
    init_cursors();
    update_view_controls_info();
    eval_tcl_cmd("update_show_all_info %d", dside->may_set_show_all);
    eval_tcl_cmd("set designer %d", is_designer(dside));
    eval_tcl_cmd("set any_elev_variation %d", any_elev_variation);
    eval_tcl_cmd("set any_temp_variation %d", any_temp_variation);
    eval_tcl_cmd("set any_wind_variation %d", any_wind_variation);
    eval_tcl_cmd("set any_clouds %d", any_clouds);

    /* Compute the position of each unit type in the list of unit types. */
    {
	int i = 0;

	if (utype_indexes == NULL)
	  utype_indexes = (short *) xmalloc(numutypes * sizeof(short));

	for_all_unit_types(u) {
	    if (type_ever_available(u, dside))
	      utype_indexes[u] = i++;
	    else
	      utype_indexes[u] = -1;
	}
    }
    {
	int m, i = 0;

	if (mtype_indexes == NULL)
	  mtype_indexes = (short *) xmalloc(nummtypes * sizeof(short));

	for_all_material_types(m) {
	    /* Test for the presence of "resources" to draw on maps. */
	    if (m_resource_icon(m) > 0)
	      any_resources = TRUE;
	    if (m_treasury(m))
	      mtype_indexes[m] = i++;
	    else
	      mtype_indexes[m] = -1;
	}
    }
    /* Remove the startup dialog right before creating the map. */
    eval_tcl_cmd(" withdraw_window \".newgame\"");

    /* Create the generic windows. */
    dside->ui->maps = NULL;

    create_map();

#ifdef DESIGNERS
    /* If this side is already a designer (perhaps via command-line option)
       popup the design controls now. */
    if (is_designer(dside))
      really_do_design(dside);
#endif /* DESIGNERS */

    Dprintf("Successfully initialized `%s'!\n", dpyname);
}

/* This will set up the correct set of colors at any point in the game.
   Colors are all specified by name; if any are not available, it is up to
   the graphics interface to supply a substitute. */

void
set_colors(void)
{
    Side	*side;
    int		t, i;
	
    dside->ui->fgcolor = request_color("black");
    dside->ui->bgcolor = request_color("white");
    dside->ui->whitecolor = request_color("white");
    dside->ui->blackcolor = request_color("black");
    dside->ui->diffcolor = dside->ui->blackcolor;
    dside->ui->graycolor = dside->ui->whitecolor;
    dside->ui->badcolor = dside->ui->whitecolor;
    dside->ui->warncolor = dside->ui->whitecolor;
    dside->ui->goodcolor = dside->ui->whitecolor;
    dside->ui->unit_name_color = dside->ui->whitecolor;
    dside->ui->grid_color = dside->ui->fgcolor;
    dside->ui->unseen_color = dside->ui->fgcolor;
    dside->ui->contour_color = dside->ui->fgcolor;
    dside->ui->country_border_color = dside->ui->fgcolor;
    dside->ui->feature_color = dside->ui->fgcolor;
    dside->ui->frontline_color = dside->ui->fgcolor;
    dside->ui->meridian_color = dside->ui->fgcolor;
    dside->ui->shoreline_color = dside->ui->fgcolor;
    /* Set colors to distinctive default; will set usefully later. */
    for_all_terrain_types(t) {
	dside->ui->cell_color[t] = NULL;
	for (i = 0; i < 5; ++i)
	  (dside->ui->cell_shades[i])[t] = dside->ui->graycolor;
    }
    /* Convert the default side colors to XColors. */
    for_all_sides(side) {
	dside->ui->default_colors[side->id] = 
	  request_color(side->default_color);
    }
    if (strcmp(g_grid_color(), g_unseen_color()) == 0)
	grid_matches_unseen = TRUE;
}

/* This will set up colors that are not crucial to the game.  By doing
   after basic image color allocation, these won't suck up colormap
   space that should go to images. */

static void
set_optional_colors(void)
{
    XColor *color;

    if ((color = request_color(g_fore_color())) != NULL)
      dside->ui->fgcolor = color;
    if ((color = request_color(g_window_color())) != NULL)
      dside->ui->window_color = color;
    if ((color = request_color(g_mask_color())) != NULL)
      dside->ui->mask_color = color;
    if ((color = request_color(g_text_color())) != NULL)
      dside->ui->text_color = color;
    if ((color = request_color(g_grid_color())) != NULL)
      dside->ui->grid_color = color;
    if ((color = request_color(g_unseen_color())) != NULL)
      dside->ui->unseen_color = color;
    if ((color = request_color(g_contour_color())) != NULL)
      dside->ui->contour_color = color;
    if ((color = request_color(g_country_border_color())) != NULL)
      dside->ui->country_border_color = color;
    if ((color = request_color(g_frontline_color())) != NULL)
      dside->ui->frontline_color = color;
    if ((color = request_color(g_meridian_color())) != NULL)
      dside->ui->meridian_color = color;
    if ((color = request_color(g_shoreline_color())) != NULL)
      dside->ui->shoreline_color = color;
    if ((color = request_color(g_feature_color())) != NULL)
      dside->ui->feature_color = color;
    if ((color = request_color(g_unit_name_color())) != NULL)
      dside->ui->unit_name_color = color;
    if ((color = request_color("maroon")) != NULL)
      dside->ui->diffcolor = color;
    if ((color = request_color("light gray")) != NULL)
      dside->ui->graycolor = color;
    if ((color = request_color("red")) != NULL)
      dside->ui->badcolor = color;
    if ((color = request_color("yellow")) != NULL)
      dside->ui->warncolor = color;
    if ((color = request_color("green")) != NULL)
      dside->ui->goodcolor = color;
}

/* Collect all the names of types etc for which no images could be found,
   report them all at once. */

static void
report_missing_images()
{
    int u, t, e;
    Side *side2;

    for_all_terrain_types(t) {
	if (dside->ui->timages[t] == NULL
	    || dside->ui->timages[t]->ersatz)
	  record_missing_image(TTYP, t_type_name(t));
    }
    for_all_unit_types(u) {
	if (uimages[u] == NULL
	    || uimages[u]->ersatz)
	  record_missing_image(UTYP, u_type_name(u));
    }
    for_all_sides(side2) {
	e = side_number(side2);
	if (dside->ui->eimages[e] == NULL
	    || dside->ui->eimages[e]->ersatz)
	  record_missing_image(3, side_desig(side2));
    }
    if (missing_images(tmpbuf))
      init_warning("Could not find %s", tmpbuf);
}

/* A predicate that tests whether our display can safely be written to. */

int
active_display(Side *side)
{
    return (side && side->ui && side->ui->active);
}

/* Make all the different kinds of cursors we intend to use.  Should be
   one for each kind of mode or tool.  Cursors should always be 16x16. */

static void
init_cursors(void)
{
    int i;
    Tk_Window rootwin = Tk_MainWindow(interp);

    Tk_MakeWindowExist(rootwin);
    for (i = 0; i < nummodes; ++i)
      dside->ui->cursors[i] = None;
#ifdef MAC
    /* Tk_GetCursorFromData is not supported for MacOS and Windows.
    We use native cursors on the Mac instead. A solution for Windows is 
    yet to be implemented. */
    dside->ui->cursors[survey_mode] = 
    	Tk_GetCursor(interp, rootwin, "Lookglass");
    dside->ui->cursors[move_mode] = 
    	Tk_GetCursor(interp, rootwin, "Shoot");
    dside->ui->cursors[attack_mode] = 
    	Tk_GetCursor(interp, rootwin, "Attack");
    dside->ui->cursors[fire_mode] = 
    	Tk_GetCursor(interp, rootwin, "Shoot");
    dside->ui->cursors[build_mode] = 
    	Tk_GetCursor(interp, rootwin, "OpenCross");
    dside->ui->cursors[scroll_up_mode] = 
    	Tk_GetCursor(interp, rootwin, "Up");
    dside->ui->cursors[scroll_right_mode] = 
    	Tk_GetCursor(interp, rootwin, "Right");
    dside->ui->cursors[scroll_down_mode] = 
    	Tk_GetCursor(interp, rootwin, "Down");
    dside->ui->cursors[scroll_left_mode] = 
    	Tk_GetCursor(interp, rootwin, "Left");
#ifdef DESIGNERS
    dside->ui->cursors[cell_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Cell");
    dside->ui->cursors[bord_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Bord");
    dside->ui->cursors[conn_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Conn");
    dside->ui->cursors[coat_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Coat");
    dside->ui->cursors[unit_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Unit");
    dside->ui->cursors[people_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "People");
     /* We use the People cursor also for Control. */
    dside->ui->cursors[control_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "People");
    dside->ui->cursors[feature_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Feature");
    dside->ui->cursors[material_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Material");
    dside->ui->cursors[elevation_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Elevation");
    dside->ui->cursors[temperature_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Temperature");
    dside->ui->cursors[clouds_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Clouds");
    dside->ui->cursors[winds_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "Winds");
    dside->ui->cursors[view_paint_mode] = 
    	Tk_GetCursor(interp, rootwin, "View");
#endif /* DESIGNERS */
#else /* MAC */
    dside->ui->cursors[survey_mode] =
      make_cursor(rootwin, (char *)lookglass_bits, (char *)lookmask_bits,
		  lookglass_x_hot, lookglass_y_hot);
    dside->ui->cursors[move_mode] =
      make_cursor(rootwin, (char *)shootcursor_bits, (char *)shootmask_bits,
		  shootcursor_x_hot, shootcursor_y_hot);
    dside->ui->cursors[attack_mode] =
      make_cursor(rootwin, (char *)attackcurs_bits, (char *)attackmask_bits,
		  attackcurs_x_hot, attackcurs_y_hot);
    dside->ui->cursors[fire_mode] =
      make_cursor(rootwin, (char *)shootcursor_bits, (char *)shootmask_bits,
		  shootcursor_x_hot, shootcursor_y_hot);
#if 0
    dside->ui->cursors[build_mode] =
      make_cursor(rootwin, buildcursor_bits, buildmask_bits,
		  buildcursor_x_hot, buildcursor_y_hot);
#else
    dside->ui->cursors[build_mode] = 
    	Tk_GetCursor(interp, rootwin, "plus");
#endif
    dside->ui->cursors[scroll_up_mode] =
      make_cursor(rootwin, (char *)scursup_bits, (char *)scursupm_bits,
		  scursup_x_hot, scursup_y_hot);
    dside->ui->cursors[scroll_right_mode] =
      make_cursor(rootwin, (char *)scursright_bits, (char *)scursrightm_bits,
		  scursright_x_hot, scursright_y_hot);
    dside->ui->cursors[scroll_down_mode] =
      make_cursor(rootwin, (char *)scursdown_bits, (char *)scursdownm_bits,
		  scursdown_x_hot, scursdown_y_hot);
    dside->ui->cursors[scroll_left_mode] =
      make_cursor(rootwin, (char *)scursleft_bits, (char *)scursleftm_bits,
		  scursleft_x_hot, scursleft_y_hot);
#ifdef DESIGNERS
    dside->ui->cursors[cell_paint_mode] =
      make_cursor(rootwin, (char *)curscell_bits, (char *)curscellm_bits,
		  curscell_x_hot, curscell_y_hot);
    dside->ui->cursors[bord_paint_mode] =
      make_cursor(rootwin, (char *)cursbord_bits, (char *)cursbordm_bits,
		  cursbord_x_hot, cursbord_y_hot);
    dside->ui->cursors[conn_paint_mode] =
      make_cursor(rootwin, (char *)cursconn_bits, (char *)cursconnm_bits,
		  cursconn_x_hot, cursconn_y_hot);
    dside->ui->cursors[coat_paint_mode] =
      make_cursor(rootwin, (char *)curscoat_bits, (char *)curscoatm_bits,
		  curscoat_x_hot, curscoat_y_hot);
    dside->ui->cursors[unit_paint_mode] =
      make_cursor(rootwin, (char *)cursunit_bits, (char *)cursunitm_bits,
		  cursunit_x_hot, cursunit_y_hot);
    dside->ui->cursors[people_paint_mode] =
      make_cursor(rootwin, (char *)curspeop_bits, (char *)curspeopm_bits,
		  curspeop_x_hot, curspeop_y_hot);
    dside->ui->cursors[control_paint_mode] =
      make_cursor(rootwin, (char *)curspeop_bits, (char *)curspeopm_bits,
		  curspeop_x_hot, curspeop_y_hot);
    dside->ui->cursors[feature_paint_mode] =
      make_cursor(rootwin, (char *)cursfeat_bits, (char *)cursfeatm_bits,
		  cursfeat_x_hot, cursfeat_y_hot);
    dside->ui->cursors[material_paint_mode] =
      make_cursor(rootwin, (char *)cursmaterial_bits, 
		  (char *)cursmaterialm_bits,
		  cursmaterial_x_hot, cursmaterial_y_hot);
    dside->ui->cursors[elevation_paint_mode] =
      make_cursor(rootwin, (char *)curselev_bits, (char *)curselevm_bits,
		  curselev_x_hot, curselev_y_hot);
    dside->ui->cursors[temperature_paint_mode] =
      make_cursor(rootwin, (char *)curstemp_bits, (char *)curstempm_bits,
		  curstemp_x_hot, curstemp_y_hot);
    dside->ui->cursors[clouds_paint_mode] =
      make_cursor(rootwin, (char *)cursclouds_bits, (char *)curscloudsm_bits,
		  cursclouds_x_hot, cursclouds_y_hot);
    dside->ui->cursors[winds_paint_mode] =
      make_cursor(rootwin, (char *)curswinds_bits, (char *)curswindsm_bits,
		  curswinds_x_hot, curswinds_y_hot);
    dside->ui->cursors[view_paint_mode] =
      make_cursor(rootwin, (char *)cursview_bits, (char *)cursviewm_bits,
		  cursview_x_hot, cursview_y_hot);
#endif /* DESIGNERS */
#endif /* MAC */
    DGprintf("Cursors stored ...\n");
}

static Tk_Cursor
make_cursor(Tk_Window tkwin, char *cursbits, char *maskbits, int x, int y)
{
    return Tk_GetCursorFromData(interp, tkwin, cursbits, maskbits, 16, 16,
				x, y, Tk_GetUid("black"), Tk_GetUid("white"));
}

/* Get a pointer to the color of the given name. */

XColor *
request_color(char *name)
{
    XColor *rslt;
    Tk_Window tkwin = Tk_MainWindow(interp);

    DGprintf("Requesting color %s\n", (name ? name : "<null>"));
    /* This might be called to get user-specified colors, even on a mono
       display, so deal with it. */
    if (empty_string(name)) {
	init_warning(
          "Requesting anonymous color on display \"%s\", substituting white",
		     dside->player->displayname);
	rslt = Tk_GetColor(interp, tkwin, Tk_GetUid("white"));
    } else {
	/* Try the generic Tk mechanism first. */
	rslt = Tk_GetColor(interp, tkwin, Tk_GetUid(name));
	/* Now try the imf library. */
	if (rslt == NULL) {
	    int dummy, red, grn, blu;
	    ImageFamily *imf = get_generic_images(name);
	    Image *img;
	    XColor col;

	    if (imf != NULL
		&& imf->numsizes > 0
		&& imf->images != NULL
		&& imf->images->w == 1
		&& imf->images->h == 1
		&& imf->images->palette != lispnil) {
		img = imf->images;
		parse_lisp_palette_entry(car(img->palette),
					 &dummy, &red, &grn, &blu);
		col.red = red;  col.green = grn;  col.blue = blu;
		rslt = Tk_GetColorByValue(tkwin, &col);
	    }
	}
    }
    if (rslt == NULL)
      init_warning("Request for \"%s\" failed", name);
    return rslt;
}

static void
init_bitmaps(void)
{
    int dir;
    Tk_Window rootwin = Tk_MainWindow(interp);

    Tk_MakeWindowExist(rootwin);

    /* Get the solid hex outlines. */
    Tk_DefineBitmap(interp, Tk_GetUid("hex8"),
		    (char *)hex8_bits, hex8_width, hex8_height);
    dside->ui->hexpics[3] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex8"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex8b"),
		    (char *)hex8b_bits, hex8b_width, hex8b_height);
    dside->ui->bhexpics[3] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex8b"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex16"),
		    (char *)hex16_bits, hex16_width, hex16_height);
    dside->ui->hexpics[4] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex16"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex16b"),
		    (char *)hex16b_bits, hex16b_width, hex16b_height);
    dside->ui->bhexpics[4] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex16b"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex32"),
		    (char *)hex32_bits, hex32_width, hex32_height);
    dside->ui->hexpics[5] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex32"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex32b"),
		    (char *)hex32b_bits, hex32b_width, hex32b_height);
    dside->ui->bhexpics[5] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex32b"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex64"),
		    (char *)hex64_bits, hex64_width, hex64_height);
    dside->ui->hexpics[6] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex64"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex64b"),
		    (char *)hex64b_bits, hex64b_width, hex64b_height);
    dside->ui->bhexpics[6] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex64b"));

    Tk_DefineBitmap(interp, Tk_GetUid("fuzz16"),
		    (char *)fuzz16_bits, fuzz16_width, fuzz16_height);
    fuzzpics[4] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("fuzz16"));
    Tk_DefineBitmap(interp, Tk_GetUid("fuzz32"),
		    (char *)fuzz32_bits, fuzz32_width, fuzz32_height);
    fuzzpics[5] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("fuzz32"));
    Tk_DefineBitmap(interp, Tk_GetUid("fuzz64"),
		    (char *)fuzz64_bits, fuzz64_width, fuzz64_height);
    fuzzpics[6] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("fuzz64"));

    Tk_DefineBitmap(interp, Tk_GetUid("hex8iso"),
		    (char *)hex8iso_bits, hex8iso_width, hex8iso_height);
    dside->ui->hexisopics[3] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex8iso"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex16iso"),
		    (char *)hex16iso_bits, hex16iso_width, hex16iso_height);
    dside->ui->hexisopics[4] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex16iso"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex32iso"),
		    (char *)hex32iso_bits, hex32iso_width, hex32iso_height);
    dside->ui->hexisopics[5] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex32iso"));
    Tk_DefineBitmap(interp, Tk_GetUid("hex64iso"),
		    (char *)hex64iso_bits, hex64iso_width, hex64iso_height);
    dside->ui->hexisopics[6] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("hex64iso"));

    Tk_DefineBitmap(interp, Tk_GetUid("wind0"),
		    (char *)wind0_bits, wind0_width, wind0_height);
    for_all_directions(dir) {
	windpics[0][dir] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind0"));
    }
    Tk_DefineBitmap(interp, Tk_GetUid("wind1ne"),
		    (char *)wind1ne_bits, wind1ne_width, wind1ne_height);
    windpics[1][NORTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1ne"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind1e"),
		    (char *)wind1e_bits, wind1e_width, wind1e_height);
    windpics[1][EAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1e"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind1se"),
		    (char *)wind1se_bits, wind1se_width, wind1se_height);
    windpics[1][SOUTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1se"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind1sw"),
		    (char *)wind1sw_bits, wind1sw_width, wind1sw_height);
    windpics[1][SOUTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1sw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind1w"),
		    (char *)wind1w_bits, wind1w_width, wind1w_height);
    windpics[1][WEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1w"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind1nw"),
		    (char *)wind1nw_bits, wind1nw_width, wind1nw_height);
    windpics[1][NORTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind1nw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2ne"),
		    (char *)wind2ne_bits, wind2ne_width, wind2ne_height);
    windpics[2][NORTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2ne"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2e"),
		    (char *)wind2e_bits, wind2e_width, wind2e_height);
    windpics[2][EAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2e"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2se"),
		    (char *)wind2se_bits, wind2se_width, wind2se_height);
    windpics[2][SOUTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2se"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2sw"),
		    (char *)wind2sw_bits, wind2sw_width, wind2sw_height);
    windpics[2][SOUTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2sw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2w"),
		    (char *)wind2w_bits, wind2w_width, wind2w_height);
    windpics[2][WEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2w"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind2nw"),
		    (char *)wind2nw_bits, wind2nw_width, wind2nw_height);
    windpics[2][NORTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind2nw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3ne"),
		    (char *)wind3ne_bits, wind3ne_width, wind3ne_height);
    windpics[3][NORTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3ne"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3e"),
		    (char *)wind3e_bits, wind3e_width, wind3e_height);
    windpics[3][EAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3e"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3se"),
		    (char *)wind3se_bits, wind3se_width, wind3se_height);
    windpics[3][SOUTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3se"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3sw"),
		    (char *)wind3sw_bits, wind3sw_width, wind3sw_height);
    windpics[3][SOUTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3sw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3w"),
		    (char *)wind3w_bits, wind3w_width, wind3w_height);
    windpics[3][WEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3w"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind3nw"),
		    (char *)wind3nw_bits, wind3nw_width, wind3nw_height);
    windpics[3][NORTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind3nw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4ne"),
		    (char *)wind4ne_bits, wind4ne_width, wind4ne_height);
    windpics[4][NORTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4ne"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4e"),
		    (char *)wind4e_bits, wind4e_width, wind4e_height);
    windpics[4][EAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4e"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4se"),
		    (char *)wind4se_bits, wind4se_width, wind4se_height);
    windpics[4][SOUTHEAST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4se"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4sw"),
		    (char *)wind4sw_bits, wind4sw_width, wind4sw_height);
    windpics[4][SOUTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4sw"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4w"),
		    (char *)wind4w_bits, wind4w_width, wind4w_height);
    windpics[4][WEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4w"));
    Tk_DefineBitmap(interp, Tk_GetUid("wind4nw"),
		    (char *)wind4nw_bits, wind4nw_width, wind4nw_height);
    windpics[4][NORTHWEST] = Tk_GetBitmap(interp, rootwin, Tk_GetUid("wind4nw"));


    /* Set up three shades of gray stipple. */
    dside->ui->grays[darkgray] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("gray75"));
    dside->ui->grays[gray] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("gray50"));
    dside->ui->grays[lightgray] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("gray25"));
    dside->ui->grays[verylightgray] =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("gray12"));
    Tk_DefineBitmap(interp, Tk_GetUid("dotdotdot"),
		    (char *)dots_bits, dots_width, dots_height);
    dside->ui->dots =
      Tk_GetBitmap(interp, rootwin, Tk_GetUid("dotdotdot"));
    Tk_DefineBitmap(interp, Tk_GetUid("ants"),
		    (char *)ants_bits, ants_width, ants_height);
    antpic = Tk_GetBitmap(interp, rootwin, Tk_GetUid("ants"));
    /* These bitmaps are used only by tcl code, don't need to be assigned
       to a C variable. */
    Tk_DefineBitmap(interp, Tk_GetUid("shoot_cursor"),
		    (char *)shootcursor_bits, 
		    shootcursor_width, shootcursor_height);
    Tk_DefineBitmap(interp, Tk_GetUid("build"),
		    (char *)buildcursor_bits, 
		    buildcursor_width, buildcursor_height);
    Tk_DefineBitmap(interp, Tk_GetUid("looking_glass"),
		    (char *)lookglass_bits, lookglass_width, lookglass_height);
    Tk_DefineBitmap(interp, Tk_GetUid("return"),
		    (char *)return_bits, return_width, return_height);
    Tk_DefineBitmap(interp, Tk_GetUid("sleep"),
		    (char *)sleep_bits, sleep_width, sleep_height);
    Tk_DefineBitmap(interp, Tk_GetUid("reserve"),
		    (char *)reserve_bits, reserve_width, reserve_height);
    Tk_DefineBitmap(interp, Tk_GetUid("delay"),
		    (char *)delay_bits, delay_width, delay_height);
    Tk_DefineBitmap(interp, Tk_GetUid("closer"),
		    (char *)closer_bits, closer_width, closer_height);
    Tk_DefineBitmap(interp, Tk_GetUid("farther"),
		    (char *)farther_bits, farther_width, farther_height);
    Tk_DefineBitmap(interp, Tk_GetUid("iso"),
		    (char *)iso_bits, iso_width, iso_height);
    Tk_DefineBitmap(interp, Tk_GetUid("rotl"),
		    (char *)rotl_bits, rotl_width, rotl_height);
    Tk_DefineBitmap(interp, Tk_GetUid("rotr"),
		    (char *)rotr_bits, rotr_width, rotr_height);
    Tk_DefineBitmap(interp, Tk_GetUid("laurels"),
		    (char *)laurels_bits, laurels_width, laurels_height);
#ifdef DESIGNERS
    Tk_DefineBitmap(interp, Tk_GetUid("paint_cell"),
		    (char *)curscell_bits, curscell_width, curscell_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_unit"),
		    (char *)cursunit_bits, cursunit_width, cursunit_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_people"),
		    (char *)curspeop_bits, curspeop_width, curspeop_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_control"),
		    (char *)curspeop_bits, curspeop_width, curspeop_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_feature"),
		    (char *)cursfeat_bits, cursfeat_width, cursfeat_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_material"),
		    (char *)cursmaterial_bits, 
		    cursmaterial_width, cursmaterial_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_elev"),
		    (char *)curselev_bits, curselev_width, curselev_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_temp"),
		    (char *)curstemp_bits, curstemp_width, curstemp_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_clouds"),
		    (char *)cursclouds_bits, 
		    cursclouds_width, cursclouds_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_winds"),
		    (char *)curswinds_bits, curswinds_width, curswinds_height);
    Tk_DefineBitmap(interp, Tk_GetUid("paint_view"),
		    (char *)cursview_bits, cursview_width, cursview_height);
#endif
}

static void
init_material_images(void)
{
    int m;
    ImageFamily *imf;
    Image *img, *mimg;
    TkImage *tkimg;

    dside->ui->mimages =
      (ImageFamily **) xmalloc(nummtypes * sizeof(ImageFamily *));
    for_all_material_types(m) {
	imf = get_material_type_images(dside, m);
	if (DebugG)
	  describe_imf(dside, "material type", m_type_name(m), imf);
	dside->ui->mimages[m] = imf;
	/* Look for a solid color. */
	mimg = NULL;
	for_all_images(dside->ui->mimages[m], img) {
	    if (img->w == 1 && img->h == 1) {
		mimg = img;
		break;
	    }
	}
	if (mimg != NULL && mimg->w == 1 && mimg->h == 1) {
	    if (mimg->hook) {
		tkimg = (TkImage *) mimg->hook;
		/* Save the color if found. */
		if (tkimg->solid)
		  dside->ui->material_color[m] = tkimg->solid;
	    }
	}
    }
}

/* Collect images for all the terrain types. */

static void init_shades(int t);

static void
init_terrain_images(void)
{
    int t, p;
    ImageFamily *imf;
    Image *img, *timg, *subimg;
    TkImage *tkimg;

    dside->ui->timages =
      (ImageFamily **) xmalloc(numttypes * sizeof(ImageFamily *));
    for (p = 0; p < NUMPOWERS; ++p) {
	dside->ui->best_timages[p] =
	  (Image **) xmalloc(numttypes * sizeof(Image *));
	dside->ui->terrpics[p] =
	  (Pixmap *) xmalloc(numttypes * sizeof(Pixmap));
    }
    for_all_terrain_types(t) {
	imf = get_terrain_type_images(dside, t);
	if (DebugG)
	  describe_imf(dside, "terrain type", t_type_name(t), imf);
	dside->ui->timages[t] = imf;
	/* Precalculate which images to use at which magnifications. */
	for (p = 0; p < NUMPOWERS; ++p) {
	    (dside->ui->terrpics[p])[t] = None;
	    timg = best_image(dside->ui->timages[t], hws[p], hhs[p]);
	    (dside->ui->best_timages[p])[t] = timg;
	    if (timg != NULL) {
		subimg = timg;
		/* If we have multiple subimages, pick the first as
                   representative. */
		if (timg->numsubimages > 0 && timg->subimages)
		  subimg = timg->subimages[0];
		tkimg = (TkImage *) subimg->hook;
		if (tkimg != NULL) {
		    if (tkimg->colr != None)
		      (dside->ui->terrpics[p])[t] = tkimg->colr;
		    else
		      (dside->ui->terrpics[p])[t] = tkimg->mono;
		}
	    }
	}
	/* Look for a solid color. */
	timg = NULL;
	for_all_images(dside->ui->timages[t], img) {
	    if (img->w == 1 && img->h == 1) {
		timg = img;
		break;
	    }
	}
	if (timg != NULL && timg->w == 1 && timg->h == 1) {
	    if (timg->hook) {
		tkimg = (TkImage *) timg->hook;
		/* Save the color if found. */
		if (tkimg->solid)
		  dside->ui->cell_color[t] = tkimg->solid;
	    }
	}
	if (elevations_defined())
	  init_shades(t);
    }
    /* Finally get the unseen terrain images. */
    get_unseen_images(dside);
    for (p = 0; p < NUMPOWERS; ++p) {
	dside->ui->best_unseen_images[p] = 
	  (Image *) xmalloc(sizeof(Image *));
	timg = best_image(unseen_image, hws[p], hhs[p]);
	dside->ui->best_unseen_images[p] = timg;    
    }
}

static void
init_shades(int t)
{
    XColor col, *newcolor, *basecolor = dside->ui->cell_color[t];
    Tk_Window rootwin = Tk_MainWindow(interp);

    if (basecolor == NULL)
      basecolor = dside->ui->graycolor;
    col.red   = min((12 * basecolor->red) / 10, 65535);
    col.green = min((12 * basecolor->green) / 10, 65535);
    col.blue  = min((12 * basecolor->blue) / 10, 65535);
    if ((newcolor = Tk_GetColorByValue(rootwin, &col)) != NULL)
      (dside->ui->cell_shades[0])[t] = newcolor;
    col.red   = min((11 * basecolor->red) / 10, 65535);
    col.green = min((11 * basecolor->green) / 10, 65535);
    col.blue  = min((11 * basecolor->blue) / 10, 65535);
    if ((newcolor = Tk_GetColorByValue(rootwin, &col)) != NULL)
      (dside->ui->cell_shades[1])[t] = newcolor;
    (dside->ui->cell_shades[2])[t] = basecolor;
    col.red   = (9 * basecolor->red) / 10;
    col.green = (9 * basecolor->green) / 10;
    col.blue  = (9 * basecolor->blue) / 10;
    if ((newcolor = Tk_GetColorByValue(rootwin, &col)) != NULL)
      (dside->ui->cell_shades[3])[t] = newcolor;
    col.red   = (8 * basecolor->red) / 10;
    col.green = (8 * basecolor->green) / 10;
    col.blue  = (8 * basecolor->blue) / 10;
    if ((newcolor = Tk_GetColorByValue(rootwin, &col)) != NULL)
      (dside->ui->cell_shades[4])[t] = newcolor;
}

/* Set up a side's view of everybody else's colors and emblems. */

static void
init_emblem_images(void)
{
    Side *side2;
    
    dside->ui->eimages =
      (ImageFamily **) xmalloc((g_sides_max() + 1) * sizeof(ImageFamily *));
    dside->ui->eimages_loaded =
      (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    /* Independent units may have a distinguishing emblem, so the
       indepside is included here. */
    for_all_sides(side2) {
	init_emblem(side2);
    }
}

/* Compute the distinctive emblem by which one side will recognize units
   of another side.  This does both our view of ourselves and others
   orthogonally.  Note that sides without displays can still have emblems
   and colors that the sides with displays will see, but that sides
   without displays don't need to do any emblem init. */

void
init_emblem(Side *side2)
{
    char cbuf[BUFSIZ], *s, *c;
    int s2 = side_number(side2), i;
    ImageFamily *imf;
    Tk_Window tkwin = Tk_MainWindow(interp);
    XColor *color;

    /* Collect the color names of the other side and try to request
       them for our own display. */
    if (!empty_string(side2->colorscheme)) {
	/* take spec apart and iterate for multiple colors */
        /* (should be generic code) */
	for (s = side2->colorscheme, c = cbuf, i = 0; i < 3; ++s) {
	    if (*s == ',' || *s == '\0') {
		*c = '\0';
		c = cbuf;
		color = request_color(cbuf);
		if (color != NULL)
		  dside->ui->colors[s2][i++] = color;
	    } else {
		*c++ = *s;
	    }
	    if (*s == '\0')
	      break;
	}
	dside->ui->numcolors[s2] = i;
    } else {
	dside->ui->numcolors[s2] = 0;
    }
    tmp_root_window = tkwin;
    tmp_valid = TRUE;
    imf = get_emblem_images(dside, side2);
    tmp_valid = FALSE;
    if (DebugG)
      describe_imf(dside, "emblem", side_desig(side2), imf);
    dside->ui->eimages[s2] = imf;
    dside->ui->eimages_loaded[s2] = TRUE;
}

/* Collect random other images via the image family system, such as
   blasts, terrain transitions, etc. */

/* Wired-in names that we assume are available. */

char *blastnames[] = { "miss", "hit", "kill", "nuke1", "nuke2", "nuke3" };

void
init_other_images(void)
{
    int i;
    ImageFamily *imf;
    Tk_Window tkwin = Tk_MainWindow(interp);

    tmp_root_window = tkwin;
    tmp_valid = TRUE;

    /* Normal combat blasts include images for misses, hits, and
       kills, while nuclear detonations get a sequence of three
       images. */
    for (i = 0; i < 6; ++i) {
	imf = get_generic_images(blastnames[i]);
	record_imf_get(imf);
	if (DebugG)
	  describe_imf(dside, "blast", blastnames[i], imf);
	dside->ui->blastimages[i] = imf;
    }
    /* Get terrain transition bitmaps unless we are low on memory. */
    if (!poor_memory) {
	    imf = get_generic_images("transition");
	    record_imf_get(imf);
	    generic_transition = imf;
	    if (DebugG)
		  describe_imf(dside, "transition", "", imf);
    }
    /* Get generic fuzz images on Windows where we cannot use fuzzpics. */
    if (!use_clip_mask
    	/* but not if we are low on memory ... */ 
    	&& !poor_memory
    	/* ... or if the terrain is visible. */
    	&& !g_see_all()
    	&& !g_terrain_seen()) {
	imf = get_generic_images("fuzz");
	record_imf_get(imf);
	generic_fuzz = imf; 
    }
    tmp_valid = FALSE;
}

/* Describe (for debugging) Tk-specific parts of an image. */

static void
tk_describe_image(Side *side, Image *img)
{
    TkImage *tkimg;

    tkimg = (TkImage *) img->hook;
    if (tkimg)
      DGprintf(" (tk mono %d color %d mask %d solid %d)",
	       tkimg->mono, tkimg->colr, tkimg->mask, tkimg->solid);
}

void
update_view_controls_info(void)
{
    eval_tcl_cmd("update_view_controls_info %d %d %d %d %d %d %d %d",
		 dside->see_all,
		 people_sides_defined(),
		 control_sides_defined(),
		 elevations_defined(),
		 (world.daylength != 1),
		 temperatures_defined(),
		 winds_defined(),
		 clouds_defined());
}

void
update_action_controls_info(Map *map)
{
    int canact = FALSE, canplan = FALSE, canmove = FALSE;
    int canbuild = FALSE, canattack = FALSE;
    int rslt;
    Unit *unit;
    char flagsbuf[BUFSIZE];

    unit = map->curunit;
    strcpy(flagsbuf, "{");
    if (!beforestart && !endofgame && in_play(unit)) {
	if (u_acp(unit->type) > 0)
	  canact = TRUE;
	if (unit->plan)
	  canplan = TRUE;
	if (can_move_at_all(unit)) {
	    canmove = TRUE;
	    if (nummtypes > 0) /* needs to be more precise */
	      strcat(flagsbuf, " can_return");
	}
	if (is_active(unit) 
	    && (valid(can_construct_any(unit, unit)) || can_develop(unit))) {
	    canbuild = TRUE;
	}
	if (is_active(unit) && valid(can_repair_any(unit, unit))) {
	    strcat(flagsbuf, " can_repair");
	}
        if (can_change_type(unit)) {
            strcat(flagsbuf, " can_change_type");
        }
	if (can_attack_any(unit, unit)) {
	    canattack = TRUE;
	}
	if (can_fire(unit, unit)) {
	    strcat(flagsbuf, " can_fire");
	}
	if (can_detonate(unit, unit)) {
	    strcat(flagsbuf, " can_detonate");
	}
	if (unit->transport != NULL) {
	    strcat(flagsbuf, " can_disembark");
	}
	if (can_disband(unit)) {
	    strcat(flagsbuf, " can_disband");
	}
	if (can_add_terrain(unit)) {
	    strcat(flagsbuf, " can_add_terrain");
	}
	if (can_add_terrain(unit)) {
	    strcat(flagsbuf, " can_remove_terrain");
	}
	if (can_give_take(unit)) {
	    strcat(flagsbuf, " can_give_take");
	}
	if (embarkation_unit(unit) != NULL) {
	    strcat(flagsbuf, " can_embark");
	}
    }
    strcat(flagsbuf, "}");
    eval_tcl_cmd("update_action_controls_info %d %d %d %d %d %s",
		 canact, canplan, canmove, canbuild, canattack, flagsbuf);
}

int
can_give_take(Unit *unit)
{
    int u = unit->type, m;

    for_all_material_types(m) {
	if (um_storage_x(u, m) > 0)
	  return TRUE;
    }
    return FALSE;
}

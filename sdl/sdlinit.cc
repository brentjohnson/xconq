/* Initialization for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "conq.h"
#include "sdlconq.h"

ImageFamily *hexagon_w1;
ImageFamily *hexagon_w2;

static void init_material_images(void);
static void init_terrain_images(void);
static void init_emblem_images(void);
static void init_emblem(Side *side2);
static void init_other_images(void);

static void sdl_describe_image(Side *side, Image *img);
static void report_missing_images(void);

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
	    get_preferences();
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
    int i;
    SDLCursor *newgeneric = NULL;

    imf_describe_hook = sdl_describe_image;

    /* Update the generic cursor if the global variable was changed by
       the loaded game. */
    if (strcmp(default_cursor_name, g_generic_cursor()) != 0)
      newgeneric = get_cursor(g_generic_cursor(), 0, 0);
    if (newgeneric != NULL)
      generic_cursor = newgeneric;

    for (i = 0; i < nummodes; ++i)
      ui->cursors[i] = NULL;
    ui->cursors[scroll_down_mode] = get_cursor("cursor-scroll-down", 16, 32);
    ui->cursors[scroll_left_mode] = get_cursor("cursor-scroll-left", 0, 16);
    ui->cursors[scroll_right_mode] = get_cursor("cursor-scroll-right", 32, 16);
    ui->cursors[scroll_up_mode] = get_cursor("cursor-scroll-up", 16, 0);

    ui->cursors[survey_mode] = get_cursor(g_survey_cursor(), 0, 0);
    ui->cursors[attack_mode] = get_cursor(g_attack_cursor(), 0, 0);

    init_ui_chars();
    init_unit_images(dside);
    init_material_images();
    init_terrain_images();
    init_emblem_images();
    init_other_images();

    report_missing_images();
}

/* A predicate that tests whether our display can safely be written to. */

int
active_display(Side *side)
{
    return (side && side->ui && side->ui->active);
}

SDLCursor *
get_cursor(char *name, int hotx, int hoty)
{
    ImageFamily *imf;
    Image *img;
    SDL_Surface *surf = NULL;
    SDLCursor *rslt = NULL;

    imf = get_generic_images(name);
    if (imf != NULL) {
	record_imf_get(imf);
	if (DebugG)
	  describe_imf(dside, "cursor", name, imf);
	img = best_image(imf, 32, 32);
	if (img != NULL)
	  surf = sdl_image(img);
    }
    if (surf != NULL) {
	rslt = (SDLCursor *) xmalloc(sizeof(SDLCursor));
	rslt->surf = surf;
	rslt->hotx = hotx;  rslt->hoty = hoty;
    } else {
	init_warning("Could not load cursor \"%s\"", name);
    }
    return rslt;
}

static void
init_material_images(void)
{
    int m;
    ImageFamily *imf;
    Image *img, *mimg;

    ui->mimages =
      (ImageFamily **) xmalloc(nummtypes * sizeof(ImageFamily *));
    for_all_material_types(m) {
	imf = get_material_type_images(dside, m);
	if (DebugG)
	  describe_imf(dside, "material type", m_type_name(m), imf);
	ui->mimages[m] = imf;
	/* Look for a solid color. */
	mimg = NULL;
	for_all_images(ui->mimages[m], img) {
	    if (img->w == 1 && img->h == 1) {
		mimg = img;
		break;
	    }
	}
	if (mimg != NULL && mimg->w == 1 && mimg->h == 1) {
	    if (mimg->hook) {
#if 0
		/* Save the color if found. */
		if (tkimg->solid)
		  ui->material_color[m] = tkimg->solid;
#endif
	    }
	}
    }
}

/* Collect images for all the terrain types. */

#if 0
static void init_shades(int t);
#endif

static void
init_terrain_images(void)
{
    int t, p;
    ImageFamily *imf;
    Image *img, *timg, *subimg;

    ui->timages = (ImageFamily **) xmalloc(numttypes * sizeof(ImageFamily *));
    ui->tcolors = (Image **) xmalloc(numttypes * sizeof(Image *));
    for (p = 0; p < NUMPOWERS; ++p) {
	ui->best_timages[p] =
	  (Image **) xmalloc(numttypes * sizeof(Image *));
    }
    for_all_terrain_types(t) {
	imf = get_terrain_type_images(dside, t);
	if (DebugG)
	  describe_imf(dside, "terrain type", t_type_name(t), imf);
	ui->timages[t] = imf;
	/* Precalculate which images to use at which magnifications. */
	for (p = 0; p < NUMPOWERS; ++p) {
	    timg = best_image(ui->timages[t], hws[p], hhs[p]);
	    (ui->best_timages[p])[t] = timg;
	    if (timg != NULL) {
		subimg = timg;
		/* If we have multiple subimages, pick the first as
                   representative. */
		if (timg->numsubimages > 0 && timg->subimages)
		  subimg = timg->subimages[0];
	    }
	}
	/* Look for a solid color. */
	timg = NULL;
	for_all_images(ui->timages[t], img) {
	    if (img->w == 1 && img->h == 1) {
		ui->tcolors[t] = img;
		break;
	    }
	}
#if 0
	if (elevations_defined())
	  init_shades(t);
#endif
    }
}

/* Set up a side's view of everybody else's colors and emblems. */

static void
init_emblem_images(void)
{
    Side *side2;
    
    ui->eimages =
      (ImageFamily **) xmalloc((g_sides_max() + 1) * sizeof(ImageFamily *));
    ui->eimages_loaded =
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

static void
init_emblem(Side *side2)
{
#if 0
    char cbuf[BUFSIZ], *s, *c;
#endif
    int s2 = side_number(side2) /*, i*/;
    ImageFamily *imf;

    /* Collect the color names of the other side and try to request
       them for our own display. */
#if 0
    if (!empty_string(side2->colorscheme)) {
	/* take spec apart and iterate for multiple colors */
        /* (should be generic code) */
	for (s = side2->colorscheme, c = cbuf, i = 0; i < 3; ++s) {
	    if (*s == ',' || *s == '\0') {
		*c = '\0';
		c = cbuf;
		color = request_color(cbuf);
		if (color != NULL)
		  ui->colors[s2][i++] = color;
	    } else {
		*c++ = *s;
	    }
	    if (*s == '\0')
	      break;
	}
	ui->numcolors[s2] = i;
    } else {
	ui->numcolors[s2] = 0;
    }
#endif
    imf = get_emblem_images(dside, side2);
    if (DebugG)
      describe_imf(dside, "emblem", side_desig(side2), imf);
    ui->eimages[s2] = imf;
    ui->eimages_loaded[s2] = TRUE;
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

    /* Normal combat blasts include images for misses, hits, and
       kills, while nuclear detonations get a sequence of three
       images. */
    for (i = 0; i < 6; ++i) {
	imf = get_generic_images(blastnames[i]);
	record_imf_get(imf);
	if (DebugG)
	  describe_imf(dside, "blast", blastnames[i], imf);
	ui->blastimages[i] = imf;
    }
    /* Get terrain transition bitmaps. */
    imf = get_generic_images("transition");
    record_imf_get(imf);
    if (DebugG)
      describe_imf(dside, "transition", "", imf);
    generic_transition = imf;
    imf = get_generic_images("hexagon-w1");
    record_imf_get(imf);
    if (DebugG)
      describe_imf(dside, "hexagon-w1", "", imf);
    hexagon_w1 = imf;
    imf = get_generic_images("hexagon-w2");
    record_imf_get(imf);
    if (DebugG)
      describe_imf(dside, "hexagon-w2", "", imf);
    hexagon_w2 = imf;
}

/* Describe (for debugging) SDL-specific parts of an image. */

static void
sdl_describe_image(Side *side, Image *img)
{
    if (img->hook != NULL)
      DGprintf(" (SDL surface 0x%x)", img->hook);
}

/* Collect all the names of types etc for which no images could be found,
   report them all at once. */

static void
report_missing_images(void)
{
    int u, t, e;
    Side *side2;

    for_all_terrain_types(t) {
	if (ui->timages[t] == NULL || ui->timages[t]->ersatz)
	  record_missing_image(TTYP, t_type_name(t));
    }
    for_all_unit_types(u) {
	if (uimages[u] == NULL || uimages[u]->ersatz)
	  record_missing_image(UTYP, u_type_name(u));
    }
    for_all_sides(side2) {
	e = side_number(side2);
	if (ui->eimages[e] == NULL || ui->eimages[e]->ersatz)
	  record_missing_image(3, side_desig(side2));
    }
    if (missing_images(tmpbuf))
      init_warning("Could not find %s", tmpbuf);
}

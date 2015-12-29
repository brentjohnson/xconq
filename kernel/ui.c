/* Graphics support not specific to any Xconq interface.
   Copyright (C) 1992-2000 Stanley T. Shebs.
   Copyright (C) 2003-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file includes some very general graphics-related functionality
   that interfaces can (but are not required to) use.  For instance,
   the size and shapes of hex cells have been precalculated to provide
   a reasonable appearance at several magnifications.  Note that some
   of the algorithms in this file are abstracted from code that has been
   tuned and tweaked over many years, so it is strongly recommended that
   all new graphical interfaces use these. */

#include "conq.h"
#include "kpublic.h"
#include "aiutil.h"
#include "imf.h"
#include "ui.h"
#include "ai.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"
#include "aioprt.h"

using namespace Xconq;
using namespace Xconq::AI;

static void compute_q(void);
static int module_name_compare(CONST void *a1, CONST void *a2);
static int special_strcmp(const char *str1, const char *str2);
static void calc_view_misc(VP *vp);
static int blocking_utype(int u, int block);
static ImageFamily *add_default_terrain_image(ImageFamily *imf, int t);
static ImageFamily *add_default_material_image(ImageFamily *imf, int m);
static ImageFamily *add_default_unit_image(ImageFamily *imf, int u);
static ImageFamily *get_generic_utype_images(int u, const char *name);
static ImageFamily *add_default_emblem_image(ImageFamily *imf, int s2);
static UnitView *find_unit_or_occ_view(Side *side, VP *vp, UnitView *uview,
				       int usx, int usy, int usw, int ush,
				       int sx, int sy);
static UnitView *find_unit_view_at(Side *side, VP *vp, int x, int y,
				   int sx, int sy);
static int can_use_oversized_img(UnitView *uview, VP *vp);

/* The two games that should be always be available. */

const char *first_game_name = INTRO_GAME;

const char *second_game_name = STANDARD_GAME;

const char *second_game_title;

/* The side with a display open. */

Side *dside;

/* The following magical arrays set all the sizes at each magnification. */

/* This is the basic cell size. */

short mags[NUMPOWERS] = { 1, 2, 4, 8, 16, 32, 64, 128 };

/* These give the total dimensions of each hex cell, plus the vertical
   distance center-to-center.  This is all carefully calculated to make
   the cells overlap perfectly at each different magnification, assuming
   that the icons have the right shape and size. */

/* Note that mathematically, 12x14, 24x28, etc, is a better
   approximation to a hexagonal shape.  But in practice, the slope of
   the diagonal sides can't be quite as steep as it should be for a
   true regular hexagon, and so it actually looks better to shrink the
   height by a couple pixels. */

short hws[NUMPOWERS] = { 1, 2, 4, 12, 24, 44, 88, 174 };
short hhs[NUMPOWERS] = { 1, 2, 4, 13, 26, 48, 96, 192 };
short hcs[NUMPOWERS] = { 1, 2, 4, 10, 20, 37, 74, 148 };

/* Dimensions of isometric cells. */

short iws[NUMPOWERS] = { 1, 2, 4, 13, 26, 48, 96, 192};
short ihs[NUMPOWERS] = { 1, 1, 2, 6, 12, 22, 44, 88 };
short ics[NUMPOWERS] = { 1, 1, 1, 3, 6, 11, 22, 44 };

/* x offset of corners in isometric cells. */ 

short ix1[NUMPOWERS] = { 0, 0, 1, 3, 6, 11, 22, 44 };
short ix2[NUMPOWERS] = { 1, 2, 3, 10, 20, 37, 74, 148 };

/* These numbers are half the length of a vertical side. */

short halfsides[NUMPOWERS] = { 1, 1, 2, 5, 7, 13, 26, 52 };

/* The sizes of the unit subcells.  This is available drawing area, exact
   unit icon sizes depends on what's available. */

short uhs[NUMPOWERS] = { 1, 1, 3, 8, 16, 32, 64, 128 };
short uws[NUMPOWERS] = { 1, 1, 3, 8, 16, 32, 64, 128 };

/* Widths of borders and connections (0 implies don't draw at all). */

/* Full border width. */

short bwid[NUMPOWERS] = { 0, 0, 1, 2, 3, 5, 7, 9 };

/* Half-width, for narrower inset borders. */

short bwid2[NUMPOWERS] = { 0, 0, 1, 1, 2, 3, 4, 5 };

/* Full connection width. */

short cwid[NUMPOWERS] = { 0, 0, 1, 2, 3, 5, 7, 9 };

/* Coordinates of the hex borders. */
/* Note that array has extra column so don't need to wrap index. */

short bsx[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  2,   4,   4,  2,  0,  0,  2 },
    {  6,  12,  12,  6,  0,  0,  6 },
    { 12,  24,  24, 12,  0,  0, 12 },
    { 22,  44,  44, 22,  0,  0, 22 },
    { 44,  88,  88, 44,  0,  0, 44 },
    { 87, 174, 174, 87,  0,  0, 87 }
};
short bsy[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  0,   0,   4,   4,   4,  0,  0 },
    {  0,   3,  10,  13,  10,  3,  0 },
    {  0,   6,  20,  26,  20,  6,  0 },
    {  0,  11,  37,  48,  37, 11,  0 },
    {  0,  21,  75,  96,  75, 21,  0 },
    {  0,  44, 148, 192, 148, 44,  0 }
};

short ibsx[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  1,   3,   4,  3,  1,  0,  1 },
    {  3,  10,  13, 10,  3,  0,  3 },
    {  6,  20,  26, 20,  6,  0,  6 },
    { 11,  37,  48, 37, 11,  0, 11 },
    { 22,  74,  96, 74, 22,  0, 22 },
    { 0 }
};
short ibsy[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  0,   0,   1,   2,   2,  1,  0 },
    {  0,   0,   3,   6,   6,  3,  0 },
    {  0,   0,   6,  12,  12,  6,  0 },
    {  0,   0,  11,  22,  22, 11,  0 },
    {  0,   0,  22,  44,  44, 22,  0 },
    { 0 }
};

/* Coords of middles of each hex border (half a connection, basically). */
 
short lsx[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  1,  2,   1,  -1,  -2,  -1 },
    {  3,  6,   3,  -3,  -6,  -3 },
    {  6, 12,   6,  -6, -12,  -6 },
    { 11, 22,  11, -11, -22, -11 },
    { 22, 44,  22, -22, -44, -22 },
    { 44, 87,  44, -44, -87, -44 }
};
short lsy[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  -2,  0,   2,   2,   0,  -2 },
    {  -5,  0,   5,   5,   0,  -5 },
    {  -9,  0,   9,   9,   0,  -9 },
    { -18,  0,  18,  18,   0, -18 },
    { -36,  0,  36,  36,   0, -36 },
    { -74,  0,  74,  74,   0, -74 }
};

/* Isometric versions. */

short ilsx[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  0,  2,  2,  0,  -2,  -2 },
    {  0,  5,  5,  0,  -5,  -5 },
    {  0, 10, 10,  0, -10, -10 },
    {  0, 19, 19,  0, -19, -19 },
    {  0, 38, 38,  0, -38, -38 },
    { 0 }
};
short ilsy[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  -1,  -1,  1,  1,  1,  -1 },
    {  -3,  -2,  1,  3,  1,  -2 },
    {  -6,  -3,  3,  6,  3,  -3 },
    { -11,  -5,  5, 11,  5,  -5 },
    { -22, -10, 10, 22, 10, -10 },
    { 0 }
};

short qx[NUMPOWERS][7], qy[NUMPOWERS][7];

static int q_computed;

int extracells = 3;

/* Table of traditional unit widths. */
/*! \note This table can be dynamically calculated. */

short uw_trad[NUMPOWERS][NUMPOWERS] = {
    {   1,  1,  1,  1,  1,  1,  1,  1 },
    {   1,  1,  1,  1,  1,  1,  1,  1 },
    {   3,  1,  1,  1,  1,  1,  1,  1 },
    {   8,  4,  2,  1,  1,  1,  1,  1 },
    {  16,  8,  4,  2,  1,  1,  1,  1 },
    {  32, 16,  8,  4,  2,  1,  1,  1 },
    {  64, 32, 16,  8,  4,  2,  1,  1 },
    { 128, 64, 32, 16,  8,  4,  2,  1 }
};

/* Table of unit widths with ellipsis present. */
/*! \note This table can be dynamically calculated. */

short uw_elli[NUMPOWERS][NUMPOWERS] = {
    {   1,  1,  1,  1,  1,  1,  1,  1 },
    {   1,  1,  1,  1,  1,  1,  1,  1 },
    {   3,  1,  1,  1,  1,  1,  1,  1 },
    {   8,  4,  4,  4,  4,  4,  4,  4 },
    {  16,  8,  4,  4,  4,  4,  4,  4 },
    {  32, 16, 16, 16, 16, 16, 16, 16 },
    {  64, 32, 16, 16, 16, 16, 16, 16 },
    { 128, 64, 32, 16, 16, 16, 16, 16 }
};

/* The traditional direction characters. */

const char *dirchars = "ulnbhy";

/* The unit images. */

ImageFamily **uimages = NULL;

/* The number of images for each unit type. */

int *numuimages = NULL;

/* How much space to leave for a unit image, if all images should get
   the same amount (such as for a list of unit types). Currently used
   only by the X11 interface. */

int min_w_for_unit_image = 16;
int min_h_for_unit_image = 16;

/* The image family for regions that are not yet discovered. */

ImageFamily *unseen_image;

ImageFamily *generic_transition;

ImageFamily *generic_fuzz;

char *unitchars = NULL;
char *terrchars = NULL;
char unseen_char_1;
char unseen_char_2;

ImageFamily **recorded_imfs;

int num_recorded_imfs;

int max_recorded_imfs;

void (*imf_describe_hook)(Side *side, Image *img);

/* This flag is set if unit icons wider than 32 pixels exist in the game. */

int big_unit_images = FALSE;

/* Machinery to find and list all the games that should be listed as
   choices for the user. We don't actually scan library folders
   looking for all possible game designs therein, that would pick up
   experimental games and modules that are only for other modules'
   use. */

Module **possible_games = NULL;

int numgames = 0;

/* The comparison function for the game list puts un-formally-named
   modules at the end, plus the default sorting puts initial-lowercased
   names after uppercased ones. */

static int
module_name_compare(CONST void *a1, CONST void *a2)
{
    const char *title1, *title2, *basetitle1, *basetitle2;
    Module *mp1, *mp2, *base1, *base2;
    int rslt;

    mp1 = *((Module **) a1);
    mp2 = *((Module **) a2);
    title1 = (!empty_string(mp1->title) ? mp1->title : mp1->name);
    title2 = (!empty_string(mp2->title) ? mp2->title : mp2->name);
    basetitle1 = title1;
    if ((base1 = find_game_module(mp1->basemodulename)) != NULL)
      basetitle1 = (!empty_string(base1->title) ? base1->title : base1->name);
    basetitle2 = title2;
    if ((base2 = find_game_module(mp2->basemodulename)) != NULL)
      basetitle2 = (!empty_string(base2->title) ? base2->title : base2->name);
    rslt = special_strcmp(basetitle1, basetitle2);
    if (rslt != 0)
      return rslt;
    if (mp1 == base2)
      return (-1);
    if (mp2 == base1)
      return 1;
    return special_strcmp(title1, title2);
}

static int
special_strcmp(const char *str1, const char *str2)
{
    if (strcmp(str1, second_game_title) == 0) {
	if (strcmp(str2, second_game_title) == 0)
	  return 0;
	else
	  return (-1);
    } else {
	if (strcmp(str2, second_game_title) == 0)
	  return 1;
	else
	  return strcmp(str1, str2);
    }
}

static int max_possible_games;

void
collect_possible_games(void)
{
    int len;
    const char *modulename = NULL;
    char *modulecontents = NULL;
    Obj *lis;
    Module *module, *basemodule;
    FILE *fp;
    int startline = 0, endline = 0;

    if (numgames == 0 && numutypes == 0 /* !game_already_loaded() */) {
	len = 0;
	lis = lispnil;
	fp = open_library_file("game.dir");
	if (fp != NULL) {
	    lis = read_form(fp, &startline, &endline);
	    if (consp(lis)) {
		len = length(lis);
	    } else {
		init_warning("Game directory has bad format, no games found");
	    }
	    fclose(fp);
	}
	max_possible_games = 2 + len * 2;
	/* Make enough room to record all the possible games. */
	possible_games =
	  (Module **) xmalloc(max_possible_games * sizeof(Module *));
	/* Collect the intro and standard game modules and put at head
	   of list. */
	module = get_game_module(first_game_name);
	add_to_possible_games(module);
	module = get_game_module(second_game_name);
	add_to_possible_games(module);
	second_game_title =
	  (!empty_string(module->title) ? module->title : module->name);
	for (; lis != lispnil; lis = cdr(lis)) {
	    if (!(symbolp(car(lis)) || stringp(car(lis)))) {
		init_warning("Bad name in game dir list, ignoring");
		continue;
	    }
	    modulename = c_string(car(lis));
	    if (modulename != NULL) {
		module = get_game_module(modulename);
		module->contents = modulecontents;
		add_to_possible_games(module);
		if (module->basemodulename != NULL) {
		    basemodule = get_game_module(module->basemodulename);
		    add_to_possible_games(basemodule);
		}
	    }
	}
	if (numgames > 1) {
	    /* Sort all but the first game into alphabetical order
	       by displayed name. */
	    qsort(&(possible_games[1]), numgames - 1, sizeof(Module *),
		  module_name_compare);
	}
    }
}

/* Load a game's description and add it to the list of games. */

void
add_to_possible_games(Module *module)
{
    int i;

    if (module != NULL) {
	if (load_game_description(module)) {
	    /* It might be that the module description supplies the real name,
	       and that the module already exists. (work on this) */
	    /* Don't add duplicate modules. */
	    for (i = 0; i < numgames; ++i) {
		if (possible_games[i] == module)
		  return;
	    }
	    if (numgames < max_possible_games) {
		possible_games[numgames++] = module;
	    }
	}
    }
}

/* Choose and return a reasonable location for map displays to start out
   centered on. */

void
pick_a_focus(Side *side, int *xp, int *yp)
{
    int tmpx, tmpy, dist, closest = area.maxdim;
    Unit *unit, *closestunit = NULL;

    /* Explicit setting overrides any guesses. */
    if (in_area(side->init_center_x, side->init_center_y)) {
	*xp = side->init_center_x;  *yp = side->init_center_y;
	return;
    }
    if (side->startx < 0 || side->starty < 0)
      calc_start_xy(side);
    if (side->startx < 0 || side->starty < 0) {
	*xp = area.width / 2 - area.height / 4;  *yp = area.halfheight;
    } else {
	tmpx = side->startx;  tmpy = side->starty;
	/* Rescan the units to find a closest one. */
	for_all_side_units(side, unit) {
	    if (in_play(unit)) {
		/* If already got one right there, just return. */
		if (unit->x == tmpx && unit->y == tmpy) {
		    *xp = tmpx;  *yp = tmpy;
		    return;
		} else {
		    dist = distance(unit->x, unit->y, tmpx, tmpy);
		    if (dist < closest) {
			closest = dist;
			closestunit = unit;
		    }
		}
	    }
	}
	if (closestunit != NULL) {
	    /* Return the position of the unit closest to the avg position. */
	    *xp = closestunit->x;  *yp = closestunit->y;
	} else {
	    *xp = tmpx;  *yp = tmpy;
	}
    }
}

int
num_active_displays(void)
{
    int n = 0;
    Side *side;

    for_all_sides(side) {
	if (active_display(side))
	  ++n;
    }
    return n;
}

/* Compute positions at each hex corner, slightly inset. */ 

static void
compute_q(void)
{
    int d, p, w;

    for (p = 0; p < NUMPOWERS; ++p) {
	if (p < 2)
	  continue;
	w = bwid[p] + 1;
	for_all_directions(d) {
	    qx[p][d] = bsx[p][d] + ((hws[p] - 2 * bsx[p][d]) * w) / (2 * mags[p]);
	    qy[p][d] = bsy[p][d] + ((hhs[p] - 2 * bsy[p][d]) * w) / (2 * mags[p]);
	}
	qx[p][NUMDIRS] = qx[p][0];
	qy[p][NUMDIRS] = qy[p][0];
    }
}

/* Viewport handling. */

VP *
new_vp(void)
{
    int t, thickest;
    VP *vp;

    if (!q_computed) {
	compute_q();
	q_computed = TRUE;
    }
    vp = (VP *) xmalloc(sizeof(VP));
    vp->draw_aux_terrain = (short *) xmalloc(numttypes * sizeof(short));
    /* View at a 90 degree angle by default. */
    vp->angle = 90;
    /* No vertical exaggeration by default. */
    vp->vertscale = 1;
    /* For isometric views, start out looking northeast. */
    vp->isodir = NORTHEAST;
    vp->cellwidth = area.cellwidth;
    /* If the cellwidth is not reasonable for drawing elevations, use
       an approximation based on the range of elevations and terrain
       thicknesses. */
    if (vp->cellwidth <= 1) {
	thickest = 0;
	for_all_terrain_types(t) {
	    if (t_thickness(t) > thickest)
	      thickest = t_thickness(t);
	}
	vp->cellwidth = ((area.maxelev + thickest) - area.avgelev) / 2;
    }
    if (vp->cellwidth < 1)
      vp->cellwidth = 1;
    set_contour_interval(vp, 0);
    vp->draw_materials = (short *) xmalloc(nummtypes * sizeof(short));
    vp->draw_occupants = TRUE;
    return vp;
}

/* xform_cell has been separated into two functions, a core that does
   the xform calculations and a shell that adds on the vertical
   offset. This makes it possible to bypass the vertical offset by
   calling xform_cell_flat directly. */

/* Given a viewport and a cell, figure out where its UL corner will be. */

void
xform_cell(VP *vp, int x, int y, int *sxp, int *syp)
{
    int		elev, offset;

    /* First call the core function. */
    xform_cell_flat(vp, x, y, sxp, syp);

    /* Then add vertical offset if drawing at an angle. */
    if (vp->angle != 90 || vp->isometric) {
	elev = (elevations_defined() ? elev_at(x, y) : 0) - area.avgelev;
	/* Exaggerate the vertical scale if requested. */
	elev *= vp->vertscale;
	offset = (elev * vp->hh) / vp->cellwidth;
	*syp -= offset;
    }
}

void
xform_cell_top(VP *vp, int x, int y, int *sxp, int *syp)
{
    int 	elev, offset;

    /* First call the core function. */
    xform_cell_flat(vp, x, y, sxp, syp);

    /* Then add vertical offset if drawing at an angle. */
    if (vp->angle != 90 || vp->isometric) {
	elev = (elevations_defined() ? elev_at(x, y) : 0) - area.avgelev;
	/* We see the top of the terrain in the cell, not the bottom. */
	elev += t_thickness(terrain_at(x, y));
	/* Exaggerate the vertical scale if requested. */
	elev *= vp->vertscale;
	offset = (elev * vp->hh) / vp->cellwidth;
	*syp -= offset;
    }
}

/* Core calculation for xform_cell without vertical offset. */ 

void
xform_cell_flat(VP *vp, int x, int y, int *sxp, int *syp)
{
    if (vp->isometric) {
	/* The isometric case is funky in that each view direction
	   seems to need a different formula.  There may be some
	   unifying form that works for all directions, but I don't
	   know what it might look like. */
	switch (vp->isodir) {
	  case NORTHEAST:
	    *sxp = x * vp->hch + (vp->hh - vp->hch);
	    *syp = (((area.height - 1 - y) * vp->hw) / 2
		    + ((area.width - 1 - x) * vp->hw) / 4
		    - (area.halfheight * vp->hw) / 4);
	    break;
	  case EAST:
	    *sxp = (area.height - 1 - y) * vp->hch;
	    *syp = (((area.halfheight + area.width) * vp->hw) / 2
		    - ((x * vp->hw) / 2 + ((y + area.halfheight) * vp->hw) / 4)
		    - vp->hw / 2);
	    break;
	  case SOUTHEAST:
	    *sxp = ((area.width - 1 - x + area.height - 1 - y) * vp->hch
		    + (vp->hh - vp->hch));
	    *syp = ((area.width - 1 - x + y) * vp->hw) / 4;
	    break;
	  case SOUTHWEST:
	    *sxp = (area.width - 1 - x) * vp->hch + (vp->hh - vp->hch);
	    *syp = (y * vp->hw) / 2 + (x * vp->hw) / 4;
	    break;
	  case WEST:
	    *sxp = y * vp->hch;
	    *syp = (x * vp->hw) / 2 + ((y - area.halfheight) * vp->hw) / 4;
	    if (area.xwrap)
	      *syp += (area.halfheight * vp->hw) / 4;
	    break;
	  case NORTHWEST:
	    *sxp = (x + y) * vp->hch + (vp->hh - vp->hch);
	    *syp = ((area.height - 1 - y + x) * vp->hw) / 4;
	    break;
	  default:
	    case_panic("view direction", vp->isodir);
	    break;
	}
	/* Make viewport-relative, works the same for every view direction. */
	*sxp -= vp->sx;  *syp -= vp->sy;
	return;
    }
    /* Traditional overhead transformation. */

    /* Compute the scaled x. */
    *sxp = x * vp->hw + (y * vp->hw) / 2 - vp->sx;
    /* If the world is cylindrical, then we want to come up with a
    	transformed point that is in the viewport if possible;
	either add or subtract the scaled width of the world if
	that will help. */
    if (area.xwrap) {
	    if (*sxp < - vp->hw
		&& between(0, *sxp + vp->totsw, vp->pxw))
	      *sxp += vp->totsw;
	    if (*sxp > vp->pxw
		&& between(0, *sxp - vp->totsw, vp->pxw))
	      *sxp -= vp->totsw;
    }
    /* Compute the scaled y.  Screen coords for y run downwards, so we
       subtract. */
    *syp = (vp->totsh - (vp->hh + y * vp->hch)) - vp->sy;
}

/* Similarly, but allowing 1/1000ths of cells as input.  Note that
   since .001 is at the "bottom" of the cell as it appears on the
   screen, we must subtract the fraction from 1000, similarly to how
   we do the main y value. */

void
xform_cell_fractional(VP *vp, int x, int y, int xf, int yf, int *sxp, int *syp)
{
    xform_cell(vp, x, y, sxp, syp);
    *sxp += (xf * vp->hw) / 1000;  *syp += ((1000 - yf) * vp->hch) / 1000;
}

/* Same as xform_cell_fractional but bypasses vertical offset. */

void
xform_cell_fractional_flat(VP *vp, int x, int y, int xf, int yf,
			   int *sxp, int *syp)
{
    xform_cell_flat(vp, x, y, sxp, syp);
    *sxp += (xf * vp->hw) / 1000;  *syp += ((1000 - yf) * vp->hch) / 1000;
}

void
xform_unit(VP *vp, Unit *unit, int *sxp, int *syp, int *swp, int *shp)
{
    int num = 0, n = -1, sq, sx, sy, sx1, sy1, sw1, sh1;
    int x = unit->x, y = unit->y;
    Unit *unit2;

    if (!in_play(unit)) {
        run_warning(
            "Tried xform_unit on an out of play unit, %s!", unit_desig(unit));
        return;
    }
    if (unit->transport == NULL) {
	xform_cell(vp, x, y, &sx, &sy);
	/* Adjust to the unit box within the cell. */
	sx += (vp->hw - vp->uw) / 2;
	if (vp->isometric)
	  sy += (vp->hw / 2 - vp->uh);
	else
	  sy += (vp->hh - vp->uh) / 2;
	/* Figure out our position in this cell's stack. */
	for_all_stack(x, y, unit2) {
	    /* (should only count units visible to a given side) */
	    if (unit == unit2)
	      n = num;
	    ++num;
	}
	if (n < 0) {
	    run_warning("xform_unit weirdness with %s", unit_desig(unit));
	    *sxp = *syp = 0;
	    *swp = *shp = 1;
	    return;
	}
	if (num <= 1) {
	    sq = 1;
	} else if (num <= 4) {
	    sq = 2;
	} else if (num <= 16) {
	    sq = 4;
	} else if (num <= 256) {
	    sq = 8;
	} else {
	    /* This is room for 65,536 units in a stack. */
	    sq = 16;
	}
	/* Tighten up positions if using big icons. */
	if (big_unit_images) {
		*swp = vp->hw / sq;  
		*shp = vp->hw / sq;
		sx -= (vp->hw - vp->uw) / 2;
		sy -= (vp->hw - vp->uw) / (2 * sq);
		*sxp = sx + *swp * (n / sq) * 3 / 4;
		*syp = sy + *shp * (n % sq) * 2 / 3;
	} else {
		*swp = vp->uw / sq;  
		*shp = vp->uh / sq;
		*sxp = sx + *swp * (n / sq);  
		*syp = sy + *shp * (n % sq);
	}
    } else {
	/* Go up the transport chain to get the bounds for this unit. */
	xform_unit(vp, unit->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant(vp, unit->transport, unit, sx1, sy1, sw1, sh1, 
		       sxp, syp, swp, shp);
    }
}

/*! Return the maximum number of unit views in a given rectangle depending on 
   the uview display flags. */
/*! \todo Account for height? */

int
max_uviews_in_rect(int w, int h, int flags)
{
    int maxviews = 0;
    int i = 0;

    /* If requesting to draw a transport, then just return 1. */
    if (flags & XFORM_UVIEW_AS_TSPT)
      return 1;
    /* If the UI is planning on drawing an ellipsis, then we can plan on 
       fewer unit views. */
    if (flags & XFORM_UVIEW_ELLIPSIS) {
	if (w >= uws[7])
	  maxviews = (uws[7]/16)*(uws[7]/16);	/* smallest == 16x16 */
	else if (between(uws[6], w, uws[7] - 1))
	  maxviews = (uws[6]/16)*(uws[6]/16);	/* smallest == 16x16 */
	else if (between(uws[5], w, uws[6] - 1))
	  maxviews = (uws[5]/16)*(uws[5]/16);	/* smallest == 16x16 */
	else if (between(uws[4], w, uws[5] - 1))
	  maxviews = (uws[4]/4)*(uws[4]/4);	/* smallest == 4x4 */
	else if (between(uws[3], w, uws[4] - 1))
	  maxviews = (uws[3]/4)*(uws[3]/4);	/* smallest == 4x4 */
	else
	  maxviews = 1;
    }
    /* Else, traditional scaling. smallest == 1x1 */
    else {
	if (w >= uws[NUMPOWERS - 1])
	  maxviews = uws[NUMPOWERS - 1]*uws[NUMPOWERS - 1];
	else {
	    for (i = NUMPOWERS - 1; i > 0; --i) {
		if (between(uws[i-1], w, uws[i] - 1)) {
		    maxviews = uws[i-1]*uws[i-1];
		    break;
		}
	    }
	}
    }
    /* If we are requesting to draw an occupant, then only the lower half of 
       the box is available. */
    if (flags & XFORM_UVIEW_AS_OCC) 
      maxviews /= 2;
    return maxviews;
}

/* Return the maximum width of an unit view in a given rectangle with given 
   display flags and a given number of views that will be present in the 
   rectangle. */

int
uview_width_in_rect(int w, int h, int flags, int numviews, int usestdsz)
{
#if (0)
    int maxwidth = 0;
    short *uwtab = NULL;
    int ipower = 0;
    int wdiv = 0;
    int i = 0;
#endif
    int vpower = 0, vpower2 = 0, vbracket = 1;

    /* The number of uviews must be artifically doubled for occupants. */
    if (flags & XFORM_UVIEW_AS_OCC)
      numviews *= 2;
    /* Just return the width if only 1 uview. */
    if (1 == numviews)
      return w;
    /* Determine equivalent view power given the number of units. */
    vbracket = 1;
    if (1 >= numviews)
      vpower = 0;
    else {
	for (vpower = 1; vpower < NUMPOWERS; ++vpower) {
	    if (between(vbracket + 1, numviews, vbracket * 4)) 
	      break;
	    else
	      vbracket *= 4;
	}
	if (vpower >= NUMPOWERS)
	  vpower = NUMPOWERS - 1;
    }
    /* Determine equivalent view power given the width of the rectangle. */
    if (w >= uws[NUMPOWERS-1])
      vpower2 = NUMPOWERS - 1;
    else {
	for (vpower2 = NUMPOWERS - 1; vpower2 > 0; --vpower2) {
	    if (between(uws[vpower2-1], w, uws[vpower2] - 1)) {
		--vpower2;
		break;
	    }
	}
    }
    /* Lookup the unit width in the appropriate table. */
    if (flags & XFORM_UVIEW_ELLIPSIS)
      return uw_elli[vpower2][vpower];
    else
      return uw_trad[vpower2][vpower];
    return w;
#if (0)
    /* Choose correct lookup table. */
    if (usestdsz)
      uwtab = uws;
    else
      uwtab = hws;
    /* The number of uviews must be artifically doubled for occupants. */
    if (flags & XFORM_UVIEW_AS_OCC)
      numviews *= 2;
    /* Round numviews to nearest even power of 2. */
    while (numviews > 1) {
	numviews /= 2;
	++ipower;
    }
    if (ipower % 2)
      ++ipower;
    /* Determine width divisor. */
    ipower /= 2;
    wdiv = 1;
    for (; ipower > 0; --ipower)
      wdiv *= 2;
    /* Divide the width. */
    if (w >= uwtab[NUMPOWERS - 1])
      maxwidth = uwtab[NUMPOWERS - 1] / wdiv;
    else {
	for (i = NUMPOWERS - 1; i > 0; --i) {
	    if (between(uwtab[i-1], w, uwtab[i] - 1)) {
		maxwidth = uwtab[i-1] / wdiv;
		break;
	    }
	}
    }
    /* Modify, if ellipsis is turned on. */
    if (flags & XFORM_UVIEW_ELLIPSIS) {
	if (between(uwtab[3], w, uwtab[5] - 1) && (maxwidth < uwtab[2]))
	  maxwidth = uwtab[2];
	if ((w >= uwtab[5]) && (maxwidth < uwtab[4]))
	  maxwidth = uwtab[4];
    }
    /* Return the maxwidth. */
    return max(1, maxwidth);
#endif
}

/* Test if a given uview can use oversized image at current magnification. */

int
can_use_oversized_img(UnitView *uview, VP *vp)
{
    Image *img = NULL;

    assert_error(vp, "Attempted to access a NULL viewport");
    assert_error(uview, "Attempted to access a NULL unit view");
    if (!uview->imf)
	return FALSE;
    for_all_images(uview->imf, img) {
	if ((vp->power >= (NUMPOWERS - 1)) && (img->w > uws[vp->power]))
	    return TRUE;
	if ((vp->power < (NUMPOWERS - 1)) 
	    && between(uws[vp->power] + 1, img->w, uws[vp->power + 1] - 1))
	    return TRUE;
    }
    return FALSE;
}

/* Transform the map coords of a given unit view into pixel coords relative 
   to its current cell. */

int
xform_unit_view(Side *side, VP *vp, UnitView *uview, 
		int *sxp, int *syp, int *swp, int *shp, 
		int flags, int *piles, int sxt, int syt, int swt, int sht)
{
    int x = -1, y = -1;
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int sx1 = -1, sy1 = -1, sw1 = -1, sh1 = -1;
    UnitView *uview2 = NULL;
    Unit *unit2 = NULL;
    int numviews = 0, uvpos = 0, inpile = FALSE, maxviews = -1;
    int numrows = 0;
    int usestdsz = FALSE, useoversz = FALSE;

    /* Sanity checks. */
    assert_error(vp, "Attempted to access a NULL view port");
    assert_error(side || (!side && vp->show_all), 
"Attempted to access the views of a NULL side while non-designer");
    assert_error(uview, "Attempted to access a NULL unit view");
    /* More sanity checks. */
    assert_warning(!piles || (piles && (flags & XFORM_UVIEW_PILES)), 
		   "Attempted to pile unit views into NULL piles");
    if (!piles && (flags & XFORM_UVIEW_PILES))
      flags &= ~XFORM_UVIEW_PILES;
    assert_warning(!(flags & XFORM_UVIEW_AS_OCC)
		   || ((flags & XFORM_UVIEW_AS_OCC) && uview->transport),
"Attempted to view an unit as an occupant of non-existent transport");
    if (!(uview->transport) && (flags & XFORM_UVIEW_AS_OCC))
      flags &= ~XFORM_UVIEW_AS_OCC;
    /* If called with "don't draw" set, then just get out. */
    if (flags & XFORM_UVIEW_DONT_DRAW)
      return flags;
    /* Don't attempt to draw occupants in isometric mode. */
    if (vp->isometric && (flags & (XFORM_UVIEW_AS_OCC | XFORM_UVIEW_AS_TSPT)))
      return (flags | XFORM_UVIEW_DONT_DRAW);
    /* Alias some values for brevity. */
    x = uview->x;  
    y = uview->y;
    /* Reset the piles. */
    if (piles)
      memset(piles, 0, numsides * numutypes * sizeof(int));
    *sxp = *syp = *swp = *shp = 100;
    /* Check if the uview is in a seen transport. If so, then walk up the 
       transport chain, and then transform as occupants. */
    if (uview->transport 
	&& !(flags & (XFORM_UVIEW_AS_OCC | XFORM_UVIEW_AS_TSPT))) {
	/* First find transport's rect. */
	xform_unit_view(side, vp, uview->transport, &sx1, &sy1, &sw1, &sh1, 
			flags & ~XFORM_UVIEW_AS_TSPT, piles);
	/* Now find occ in context of transport. */
	flags = xform_unit_view(side, vp, uview, sxp, syp, swp, shp, 
				flags | XFORM_UVIEW_AS_OCC, piles, 
				sx1, sy1, sw1, sh1);
	return flags;
    }
    /* If unit directly in cell, then adjust unit box within cell. */
    if (!(flags & (XFORM_UVIEW_AS_TSPT | XFORM_UVIEW_AS_OCC))) {
	/* Get screen coordinates of cell. */
	xform_cell(vp, x, y, &sx, &sy);
    }
    /* Else, adjust the unit box within the transport. */
    else {
	sx = sxt;  sy = syt;
    }
    /* Prepare to iterate (or skip iteration) through unit views. */
    if (flags & XFORM_UVIEW_AS_TSPT) {
	uview2 = NULL;
	numviews = 1;
	uvpos = 0;
    }
    else if (flags & XFORM_UVIEW_AS_OCC) 
      uview2 = uview->transport->occupant;
    else {
	if (side)
	  uview2 = unit_view_at(side, x, y);
	else {
	    unit2 = unit_at(x, y);
	    assert_error(unit2, 
			 "Could not find unit in supposedly occupied cell");
	    uview2 = find_unit_view(side, unit2);
	}
    }
    /* Count up the current uview's position in the view stack. */
    for (; uview2;
	 uview2 = ((flags & (XFORM_UVIEW_AS_OCC | XFORM_UVIEW_AS_TSPT)) 
		   ? uview2->nexthere
		   : unit_view_next(side, x, y, uview2))) {
	/* If uview belongs to given side, then get an unit rep of it. */
	if (!side || (uview->siden == side->id))
	  unit2 = view_unit(uview2);
	else
	  unit2 = NULL;
	/* Assume that it is not in pile to start with. */
	inpile = FALSE;
	/* If we are using piles, then increment the appropriate pile. */
	if (flags & XFORM_UVIEW_PILES) {
	    /* If units which have less than full ACP are to be segregated... */
	    if ((flags & XFORM_UVIEW_DONT_PILE_ACP) && unit2
		&& (unit2->act 
		    || (unit2->act->acp != unit2->act->initacp))) 
	      ++numviews;
	    /* If units which have been damaged are to be segregated... */
	    else if ((flags & XFORM_UVIEW_DONT_PILE_HP) && unit2
		     && (unit2->hp != u_hp_max(unit2->type))) 
	      ++numviews;
	    /* If units which are incomplete are to be segregated... */
	    else if ((flags & XFORM_UVIEW_DONT_PILE_CP)
		     && !uview2->complete)
	      ++numviews;
	    /* If units which have >= 1 occs are to be segregated... */
	    else if ((flags & XFORM_UVIEW_DONT_PILE_OCC) 
		     && uview2->occupant)
	      ++numviews;
	    /* Else, this unit view is not distinct... */
	    else {
		/* If pile is empty, increment number of distinct views 
		   as well. */
		if (!piles[uview->siden * numutypes + uview->type]) 
		  ++numviews;
		/* Else, set the marker indicating that uview is being 
		   added to an existing pile. */
		else
		  inpile = TRUE;
		/* Increment the pile. */
		++piles[uview2->siden * numutypes + uview2->type];
	    }
	}
	/* Else no piling, so always increment. */
	else
	  ++numviews;
	/* If our view is found, then record its position. */
	if (uview == uview2)
	  uvpos = numviews - 1;
    }
    /* Unset any flags that should not be returned to the UI. */
    if (inpile)
      flags &= ~(XFORM_UVIEW_DONT_PILE_ACP | XFORM_UVIEW_DONT_PILE_HP |
		 XFORM_UVIEW_DONT_PILE_CP | XFORM_UVIEW_DONT_PILE_OCC);
    if (!inpile)
      flags &= ~XFORM_UVIEW_PILES;
    /* Get maximum number of views given rectangle and flags. */
    if (!(flags & (XFORM_UVIEW_AS_TSPT | XFORM_UVIEW_AS_OCC)))
      maxviews = max_uviews_in_rect(vp->hw, vp->hw, flags);
    else
      maxviews = max_uviews_in_rect(swt, sht, flags);
    /* Unset ellipsis flag if no dots needed. */
    if ((maxviews <= numviews) || (uvpos < maxviews))
      flags &= ~XFORM_UVIEW_ELLIPSIS;
    /* If unit is not to be drawn, then indicate this to the UI. */
    if (!maxviews)
      return (flags | XFORM_UVIEW_DONT_DRAW);
    /* Truncate numviews, if excessive. */
    numviews = min(numviews, maxviews);
    /* Truncate position, if excessive. */
    uvpos = min(uvpos, maxviews - 1);
    /* Check if unit sizes that go according to uws rather hws. */
    if ((uview->occupant && !(flags & XFORM_UVIEW_AS_TSPT))
	|| ((numviews == 1) && !can_use_oversized_img(uview, vp))
	|| (numviews > 1))
      usestdsz = TRUE;
    if (!usestdsz && (numviews == 1) && !uview->occupant
	&& can_use_oversized_img(uview, vp))
      useoversz = TRUE;
    /* Get unit view display width (and height). */
    if (flags & XFORM_UVIEW_AS_TSPT)
      sw = swt / 2;
    else if (flags & XFORM_UVIEW_AS_OCC)
      sw = uview_width_in_rect(swt, sht, flags, numviews, usestdsz);
    else {
	if (usestdsz)
	  sw = uview_width_in_rect(vp->uw, vp->uw, flags, numviews, usestdsz);
	else
	  sw = vp->hw;
    }
    sh = sw;
    /* Calculate number of rows per col. */
    if (flags & XFORM_UVIEW_AS_OCC)
      numrows = (sht / 2) / sh;
    else if (flags & XFORM_UVIEW_AS_TSPT)
      numrows = 1;
    else {
	if (usestdsz)
	  numrows = vp->uw / sh;
	else
	  numrows = 1;
    }
    numrows = max(1, numrows);
    /* Calculate vertical offset. */
    sy += (uvpos % numrows) * sh;
    if (flags & XFORM_UVIEW_AS_OCC)
      sy += (sht / 2);
    if (!(flags & (XFORM_UVIEW_AS_OCC | XFORM_UVIEW_AS_TSPT))
	&& (usestdsz || useoversz)) {
	if (vp->isometric)
	  sy += (vp->hw / 2 - (useoversz ? sh : vp->uw));
	else
	  sy += (vp->hh - (useoversz ? sh : vp->uw)) / 2;
    }
    /* Calculate horizontal offset. */
    sx += (uvpos / numrows) * sw;
    if (!(flags & (XFORM_UVIEW_AS_OCC | XFORM_UVIEW_AS_TSPT))
	&& (usestdsz || useoversz)) {
	sx += (vp->hw - (useoversz ? sw : vp->uw)) / 2;
	if (vp->isometric)
	  sx += ix1[vp->power] / 3;
    }
    /* Set results. */
    *sxp = sx;  *syp = sy;
    *swp = sw;  *shp = sh;
    /* Return all used flags. */
    return flags;
}

#if (0)
void
xform_unit_view(Side *side, VP *vp, UnitView *uview,
		int *sxp, int *syp, int *swp, int *shp)
{
    int num = 0, n = -1, sq, sx, sy, x, y, sx1, sy1, sw1, sh1;
    UnitView *uview2 = NULL;

    x = uview->x;  
    y = uview->y;
    /* If unit is in transport, then go up transport chain first, 
       and then xform as an occupant. */
    if (uview->transport) {
	/* Go up the transport chain to get the bounds for this unit. */
	xform_unit_view(side, vp, uview->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant_view(vp, uview->transport, uview, sx1, sy1, sw1, sh1, 
			    sxp, syp, swp, shp);
    }
    /* If unit is directly in cell... */
    else {
	/* Get screen coordinates of cell. */
	xform_cell(vp, x, y, &sx, &sy);
	/* Adjust to the unit box within the cell. */
	sx += (vp->hw - vp->uw) / 2;
	if (vp->isometric)
	  sy += (vp->hw / 2 - vp->uh);
	else
	  sy += (vp->hh - vp->uh) / 2;
	/* Figure out our position in this cell's stack. */
	for_all_view_stack(side, x, y, uview2) {
	    if (uview2->transport == NULL) {
		if (uview2 == uview) 
		  n = num;
		++num;
	    }
	}
	/* If uview not found, then something strange is going on. */
	if (n < 0) {
	    run_warning("xform_unit_view weirdness with");
	    *sxp = *syp = 0;
	    *swp = *shp = 1;
	    return;
	}
	/* How to dice up the uview square in the cell. */
	/*! \todo Replace this stuff with better limits for the 
		  given view size. */
	if (num <= 1) {
	    sq = 1;
	} else if (num <= 4) {
	    sq = 2;
	} else if (num <= 16) {
	    sq = 4;
	} else if (num <= 256) {
	    sq = 8;
	} else {
	    /* This is room for 65,536 unit views in a stack. */
	    sq = 16;
	}
	/* Make room for a single, unoccupied oversized icon. */
	if ((num == 1) && can_use_oversized_img(uview, vp) 
	    && (NULL == uview->occupant)) {
	    *swp = vp->hw / sq;  
	    *shp = vp->hw / sq;
	    sx -= (vp->hw - vp->uw) / 2;
	    sy -= (vp->hw - vp->uw) / (2 * sq);
	    *sxp = sx + *swp * (n / sq) * 3 / 4;
	    *syp = sy + *shp * (n % sq) * 2 / 3;
	/* Else, slice the cell normally. */
	} else {
	    *swp = vp->uw / sq;  
	    *shp = vp->uh / sq;
	    *sxp = sx + *swp * (n / sq);  
	    *syp = sy + *shp * (n % sq);
	}
    }
}
#endif

void
xform_unit_self(VP *vp, Unit *unit, int *sxp, int *syp, int *swp, int *shp)
{
    int sx1, sy1, sw1, sh1;

    if (unit->transport == NULL) {
	if (unit->occupant == NULL) {
	    xform_unit(vp, unit, sxp, syp, swp, shp);
	} else {
	    xform_unit(vp, unit, &sx1, &sy1, &sw1, &sh1);
	    xform_occupant(vp, unit, unit, sx1, sy1, sw1, sh1, sxp, syp, 
			   swp, shp);
	}
    } else {
	xform_unit(vp, unit->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant(vp, unit->transport, unit, sx1, sy1, sw1, sh1, 
		       sxp, syp, swp, shp);
    }
}

/* Given a unit, compute its actual bounding box (as opposed to the
   box that includes both its and its occupants). */

int
xform_unit_self_view(Side *side, VP *vp, UnitView *uview,
		     int *sxp, int *syp, int *swp, int *shp)
{
    int sx1, sy1, sw1, sh1;

    if (uview->transport == NULL) {
	if (uview->occupant == NULL) {
	    xform_unit_view(side, vp, uview, sxp, syp, swp, shp);
	} else {
	    xform_unit_view(side, vp, uview, &sx1, &sy1, &sw1, &sh1);
	    xform_occupant_view(vp, uview, uview, sx1, sy1, sw1, sh1,
				sxp, syp, swp, shp);
	}
    } else {
	xform_unit_view(side, vp, uview->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant_view(vp, uview->transport, uview, sx1, sy1, sw1, sh1,
			    sxp, syp, swp, shp);
    }
    return TRUE;
}

void
xform_occupant(VP *vp, Unit *transport, Unit *unit,
	       int sx, int sy, int sw, int sh,
	       int *sxp, int *syp, int *swp, int *shp)
{
    int num = 0, n = -1, nmx, nmy;
    Unit *unit2;

    /* Skip transformation if we are not drawing occupants. */
    if (!vp->draw_occupants) {
    	*sxp = sx;
    	*syp = sy;
    	*swp = sw;
    	*shp = sh;
	return;
    }
    /* Figure out the position of this unit amongst all the occupants. */
    for_all_occupants(transport, unit2) {
	if (unit2 == unit)
	  n = num;
	++num;
    }
    if (unit == transport) {
	if (num > 0) {
	    /* Transport image shrinks by half in each dimension. */
	    *swp = sw / 2;  *shp = sh / 2;
	}
	/* Transport is always in the UL corner. */
	*sxp = sx;  *syp = sy;
    } else {
	if (n < 0)
	  run_error("xform_occupant weirdness");
	/* Compute how the half-box will be subdivided.  Only use
	   powers of two, so image scaling works better. */
	if (num <= 2) {
	    nmx = 2;
	} else if (num <= 8) {
	    nmx = 4;
	} else if (num <= 128) {
	    nmx = 8;
	} else {
	    /* This is room for 32,768 units in a stack. */
	    nmx = 16;
	}
	nmy = nmx / 2;
	*swp = sw / nmx;  
	*shp = (sh / 2) / nmy;
	*sxp = sx + *swp * (n / nmy) + 1;  
	*syp = sy + sh / 2 + *shp * (n % nmy);
    }
}

void
xform_occupant_view(VP *vp, UnitView *traview, UnitView *uview,
		    int sx, int sy, int sw, int sh,
		    int *sxp, int *syp, int *swp, int *shp)
{
    int num = 0, n = -1, nmx, nmy;
    UnitView *uview2;

    /* Skip transformation if we are not drawing occupants. */
    if (!vp->draw_occupants) {
    	*sxp = sx;
    	*syp = sy;
    	*swp = sw;
    	*shp = sh;
	return;
    }
    /* Figure out the position of this unit amongst all the occupants. */
    for_all_occupant_views(traview, uview2) {
	if (uview2 == uview)
	  n = num;
	++num;
    }
    if (uview == traview) {
	if (num > 0) {
	    /* Transport image shrinks by half in each dimension. */
	    *swp = sw / 2;  
	    *shp = sh / 2;
	}
	/* Transport is always in the UL corner. */
	*sxp = sx;  
	*syp = sy;
    } else {
	if (n < 0)
	  run_error("xform_occupant_view weirdness");
	/* Compute how the half-box will be subdivided.  Only use
	   powers of two, so image scaling works better. */
	if (num <= 2) {
	    nmx = 2;
	} else if (num <= 8) {
	    nmx = 4;
	} else if (num <= 128) {
	    nmx = 8;
	} else {
	    /* This is room for 32,768 units in a stack. */
	    nmx = 16;
	}
	nmy = nmx / 2;
	*swp = sw / nmx;  
	*shp = (sh / 2) / nmy;
	*sxp = sx + *swp * (n / nmy) + 1;  
	*syp = sy + sh / 2 + *shp * (n % nmy);
    }
}

/* Scale one viewport box to its position in another. */

void
scale_vp(VP *vp, VP *vp2, int *sxp, int *syp, int *swp, int *shp)
{
    *sxp = (vp2->sx * vp->hw) / vp2->hw - vp->sx;
    *syp = (vp2->sy * vp->hch) / vp2->hch - vp->sy;
    *swp = (vp2->pxw * vp->hw) / vp2->hw;
    *shp = (vp2->pxh * vp->hch) / vp2->hch;
}

/* This works correctly for non-isometric maps only. */

void
scale_point(VP *vp, VP *vp2, int sx, int sy, int *sx2p, int *sy2p)
{
    *sx2p = ((sx + vp2->sx) * vp->hw) / vp2->hw - vp->sx;
    *sy2p = ((sy + vp2->sy) * vp->hch) / vp2->hch - vp->sy;
}

int
nearest_cell(VP *vp, int sx, int sy, int *xp, int *yp, int *xfp, int *yfp)
{
    int sxadj, syadj, sxfrac, syflipped, syfrac, tmp1;

    if (vp->isometric) {
	/* Convert to absolute scaled coordinates. */
	sx += vp->sx;  
	sy += vp->sy;
	switch (vp->isodir) {
	  case NORTHEAST:
	    sy += (area.halfheight * vp->hw) / 4;
	    *xp = (sx - (vp->hh - vp->hch)) / vp->hch;
	    syadj = sy - ((area.width - 1 - *xp) * vp->hw) / 4;
	    *yp = (area.height - 1) - (2 * syadj) / vp->hw;
	    break;
	  case EAST:
	    sy += vp->hw / 2;
	    *yp = (area.height - 1) - sx / vp->hch;
	    tmp1 = ((area.halfheight + area.width) * vp->hw) / 2;
	    *xp = ((tmp1 - sy - ((*yp + area.halfheight) * vp->hw) / 4) * 2) 
		  / vp->hw;
	    break;
	  case SOUTHEAST:
	    *xp = area.width - 1 - (((sx - (vp->hh - vp->hch)) / vp->hch)
				    + ((sy * 4) / vp->hw)
				    - (area.height - 1)) / 2;
	    *yp = ((sy * 4) / vp->hw) - (area.width - 1 - *xp);
	    break;
	  case SOUTHWEST:
	    *xp = (area.width - 1) - (sx - (vp->hh - vp->hch)) / vp->hch;
	    *yp = ((sy - (*xp * vp->hw) / 4) * 2) / vp->hw;
	    break;
	  case WEST:
	    if (area.xwrap)
	      sy -= (area.halfheight * vp->hw) / 4;
	    *yp = sx / vp->hch;
	    *xp = ((sy - ((*yp - area.halfheight) * vp->hw) / 4) * 2) / vp->hw;
	    break;
	  case NORTHWEST:
	    *xp = ((sx + (vp->hh - vp->hch)) / vp->hch
		   - (area.height - 1)
		   + ((sy * 4) / vp->hw)) / 2;
	    *yp = *xp - ((sy * 4) / vp->hw) + (area.height - 1);
	    break;
	  default:
	    case_panic("view direction", vp->isodir);
	    break;
	}
	return (in_area(*xp, *yp));
    }
    /* We don't want points below the map to be rounded off to zero and
       then interpreted as being within the map. */
    if (sy > vp->totsh) {
	*yp = -1;
	if (yfp) {
		*yfp = 0;
	}
    } else {
	/* Flip the raw y and then scale to hex coords. */
	syflipped = vp->totsh - (vp->sy + sy);
	*yp = syflipped / vp->hch;
	if (yfp) {
	    syfrac = syflipped - (*yp * vp->hch) - (vp->hh - vp->hch);
	    *yfp = (syfrac * 1000) / vp->hch;
	}
    }
    /* Adjust scaled x. */
    sxadj = (sx + vp->sx - (*yp * vp->hw) / 2);
    /* The division by hw below might round towards 0, so wrap
       negative numbers around to positive values.  This should only
       ever happens for cylinder areas, but doesn't hurt to just
       adjust all negative values. */

    /* Unfortunately, it does hurt. The mac-specific meridian drawing
       code must be allowed to use negative values for non-cylindrical
       worlds! */
    if (area.xwrap &&  sxadj < 0) {
	sxadj += (2 * vp->totsw);
    }
    /* We don't want points to the left of the map to be rounded off to zero
    and then interpreted as being within the map. */
    if (sxadj < 0) {
    	*xp = -1;
    	if (xfp) {
	    *xfp = 0;
    	}
    } else {
	*xp = sxadj / vp->hw;
	if (xfp) {
	    sxfrac = sxadj - (*xp * vp->hw);
	    *xfp = (sxfrac * 1000) / vp->hw;
	}
    }
    /* If the magnification of the map is large enough that the top
       and bottom edges of a hex are visibly sloping, then we have to
       take those edges into account, and accurately. */
    if ((vp->hh - vp->hch) / 2 > 1) {
	/* (should adjust according to hex boundaries correctly here) */
    }
    /* Wrap coords as usual. */
    if (area.xwrap)
      *xp = wrapx(*xp);
    DGprintf("Pixel %d,%d -> hex %d.%03d,%d.%03d\n",
	     sx, sy, *xp, (xfp ? *xfp : 0), *yp, (yfp ? *yfp : 0));
    return (in_area(*xp, *yp));
}

int
nearest_boundary(VP *vp, int sx, int sy, int *xp, int *yp, int *dirp)
{
    int sx2, sy2, ydelta, hexslope;

    /* Get the nearest cell... */
    if (nearest_cell(vp, sx, sy, xp, yp, NULL, NULL)) {
	/* ... and xform it back to get the pixel coords. */ 
	xform_cell(vp, *xp, *yp, &sx2, &sy2);
	ydelta = sy - sy2;
	hexslope = (vp->hh - vp->hch) / 2;
	if (sx - sx2 > vp->hw / 2) {
	    *dirp = ((ydelta < hexslope) ? NORTHEAST : (ydelta > vp->hch ? SOUTHEAST : EAST));
	} else {
	    *dirp = ((ydelta < hexslope) ? NORTHWEST : (ydelta > vp->hch ? SOUTHWEST : WEST));
	}
	DGprintf("Pixel %d,%d -> hex %d,%d dir %d\n", sx, sy, *xp, *yp, *dirp);
	return TRUE;
    } else {
	return FALSE;
    }
}

//! Return unit view at a certain pixel, if there is one.

UnitView *
find_uview_at_pixel(
    Side *side, VP *vp, UnitView *uview, int usx, int usy, int usw, int ush,
    int sx, int sy)
{
    int usx1, usy1, usw1, ush1;
    int flags = 0;
    UnitView *uvocc = NULL, *rslt = NULL;

    /* See if the point might be over an occupant. */
    for_all_occupant_views(uview, uvocc) {
	flags = 0;
	flags = xform_unit_view(side, vp, uvocc, &usx1, &usy1, &usw1, &ush1, 
				XFORM_UVIEW_AS_OCC, NULL, usx, usy, usw, ush);
	if (flags & XFORM_UVIEW_DONT_DRAW)
	  continue;
	rslt = 
	    find_uview_at_pixel(side, vp, uvocc, usx1, usy1, usw1, ush1, 
				sx, sy);
	if (rslt)
	  return rslt;
    }
    /* Now try the uview itself. */
    xform_unit_view(side, vp, uview, &usx1, &usy1, &usw1, &ush1, 0, NULL, 
		    usx, usy, usw, ush);
    if (flags & XFORM_UVIEW_DONT_DRAW)
      return NULL;
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1))
      return uview;
    return NULL;
}

Unit *
find_unit_or_occ(Side *side, VP *vp, Unit *unit, int usx, int usy,
		 int usw, int ush, int sx, int sy)
{
    int usx1, usy1, usw1, ush1;
    Unit *occ, *rslt;

    /* See if the point might be over an occupant. */
    if (unit->occupant != NULL
    /* See if we are drawing occs. */
	&& vp->draw_occupants
	&& (side_controls_unit(side, unit)
	    /* If side can examine unit in detail it can also see its occs. */
	    || side_sees_unit(side, unit)
	    || vp->show_all
	    || u_see_occupants(unit->type)
	    || side_owns_occupant_of_unit(side, unit))) {
	for_all_occupants(unit, occ) {
	    xform_unit(vp, occ, &usx1, &usy1, &usw1, &ush1);
	    rslt = find_unit_or_occ(side, vp, occ, usx1, usy1, usw1, ush1,
				    sx, sy);
	    if (rslt)
	      return rslt;
	}
    }
    /* Otherwise see if it could be the unit itself.  This has the effect of
       "giving" the transport everything in its box that is not in an occ. */
    xform_unit(vp, unit, &usx1, &usy1, &usw1, &ush1);
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1))
      return unit;
    return NULL;
}

//! Return unit view at a certain pixel, if there is one.

UnitView *
find_uview_at_pixel(Side *side, VP *vp, int x, int y, int sx, int sy)
{
    int usx, usy, usw, ush;
    int flags = 0;
    UnitView *uvstack = NULL, *uview = NULL, *rslt = NULL;

    /* Pick up the appropriate uvstack. */
    if (vp->show_all) {
        uvstack = query_uvstack_at(x, y);
        side = NULL;
    }
    else
      uvstack = unit_view_at(dside, x, y);
    /* Iterate through uvstack attempting to find uview. */
    for_all_uvstack(uvstack, uview) {
	flags = 0;
	flags = xform_unit_view(side, vp, uview, &usx, &usy, &usw, &ush, flags);
	if (flags & XFORM_UVIEW_DONT_DRAW)
	  continue;
	rslt = find_uview_at_pixel(side, vp, uview, usx, usy, usw, ush, sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

Unit *
find_unit_at(Side *side, VP *vp, int x, int y, int sx, int sy)
{
    int usx, usy, usw, ush;
    Unit *unit, *rslt;
    
    for_all_stack(x, y, unit) {
	xform_unit(vp, unit, &usx, &usy, &usw, &ush);
	rslt = find_unit_or_occ(side, vp, unit, usx, usy, usw, ush, sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

int
nearest_unit(Side *side, VP *vp, int sx, int sy, Unit **unitp)
{
    int x, y;
    
    if (!nearest_cell(vp, sx, sy, &x, &y, NULL, NULL)) {
	*unitp = NULL;
	DGprintf("Pixel %d,%d -> outside area\n", sx, sy);
	return FALSE;
    }
    if (vp->power > 4) {
	*unitp = find_unit_at(side, vp, x, y, sx, sy);
    } else {
	*unitp = unit_at(x, y);
    }
    DGprintf("Pixel %d,%d -> unit %s\n", sx, sy, unit_desig(*unitp));
    return TRUE;
}

UnitView *
find_unit_or_occ_view(Side *side, VP *vp, UnitView *uview, int usx, int usy,
		      int usw, int ush, int sx, int sy)
{
    int usx1, usy1, usw1, ush1;

#if 0 /* until we figure this out */
    Unit *unit, *occ, *rslt;
    /* See if the point might be over an occupant. */
    if (unit->occupant != NULL
    /* See if we are drawing occs. */
	&& vp->draw_occupants
	&& (side_controls_unit(side, unit)
	    /* If side can examine unit in detail it can also see its occs. */
	    || side_sees_unit(side, unit)
	    || vp->show_all
	    || u_see_occupants(unit->type)
	    || side_owns_occupant_of_unit(side, unit))) {
	for_all_occupants(unit, occ) {
	    xform_unit(vp, occ, &usx1, &usy1, &usw1, &ush1);
	    rslt = find_unit_or_occ(side, vp, occ, usx1, usy1, usw1, ush1,
				    sx, sy);
	    if (rslt)
	      return rslt;
	}
    }
#endif
    /* Set side to null, if it is the dside and 'show_all' is in effect. */
    if ((side == dside) && vp->show_all) {
	side = NULL;
	uview = query_uvstack_from_unit(view_unit(uview));
    }
    /* Otherwise see if it could be the unit itself.  This has the effect of
       "giving" the transport everything in its box that is not in an occ. */
    xform_unit_view(side, vp, uview, &usx1, &usy1, &usw1, &ush1);
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1))
      return uview;
    return NULL;
}

UnitView *
find_unit_view_at(Side *side, VP *vp, int x, int y, int sx, int sy)
{
    int usx, usy, usw, ush;
    UnitView *uview, *rslt;

    for_all_view_stack_with_occs(side, x, y, uview) {
	xform_unit_view(side, vp, uview, &usx, &usy, &usw, &ush);
	rslt = find_unit_or_occ_view(side, vp, uview, usx, usy, usw, ush,
				     sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

int
nearest_unit_view(Side *side, VP *vp, int sx, int sy, UnitView **uviewp)
{
    int x, y;
    
    if (!nearest_cell(vp, sx, sy, &x, &y, NULL, NULL)) {
	*uviewp = NULL;
	DGprintf("Pixel %d,%d -> outside area\n", sx, sy);
	return FALSE;
    }
    if (vp->power > 4) {
#if (0)
	*uviewp = find_unit_view_at(side, vp, x, y, sx, sy);
#else
	*uviewp = find_uview_at_pixel(side, vp, x, y, sx, sy);
#endif
    } else {
	*uviewp = unit_view_at(side, x, y);
    }
    DGprintf("Pixel %d,%d -> ", sx, sy);
    if (DebugG) {
	if (*uviewp != NULL) {
	    DGprintf("uview %d,%d,%d\n",
		     (*uviewp)->id, (*uviewp)->type, (*uviewp)->siden);
	} else {
	    DGprintf("no uview\n");
	}
    }
    return TRUE;
}

/* Return true if the cell is visible in the viewport. */

int
cell_is_visible(VP *vp, int x, int y)
{
    int sx, sy;
 
    if (!in_area(x, y))
      return FALSE;   
    xform_cell(vp, x, y, &sx, &sy);
    if (area.xwrap && sx > vp->totsw)
      sx -= vp->totsw;
    if (sx + vp->hw < 0)
      return FALSE;
    if (sx > vp->pxw) 
      return FALSE;
    if (sy + vp->hh < 0)
      return FALSE;
    if (sy > vp->pxh)
      return FALSE;
    return TRUE;
}

/* Decide whether given location is away from the edge of the map's
   window. */

int
cell_is_in_middle(VP *vp, int x, int y)
{
    int sx, sy, insetx1, insety1, insetx2, insety2;
    
    if (!in_area(x, y))
      return FALSE;   
    xform_cell(vp, x, y, &sx, &sy);
    /* Adjust to be the center of the cell, more reasonable if large. */
    sx += vp->hw / 2;  sy += vp->hh / 2;
    insetx1 = min(vp->pxw / 4, 1 * vp->hw);
    insety1 = min(vp->pxh / 4, 1 * vp->hch);
    insetx2 = min(vp->pxw / 4, 2 * vp->hw);
    insety2 = min(vp->pxh / 4, 2 * vp->hch);
    if (sx < insetx2)
      return FALSE;
    if (sx > vp->pxw - insetx2)
      return FALSE;
    if (sy < (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    if (sy > vp->pxh - (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    return TRUE;
}

/* Set vcx/vcy to point to the center of the view. */

void
focus_on_center(VP *vp)
{
    vp->vcy = (vp->totsh - (vp->sy + vp->pxh / 2)) / vp->hch;
    vp->vcx = vp->sx / vp->hw - (vp->vcy / 2) + (vp->pxw / vp->hch) / 2;
    /* Restrict the focus to be *inside* the area. */
    vp->vcy = limitn(1, vp->vcy, area.height - 2);
    if (area.xwrap) {
	vp->vcx = wrapx(vp->vcx);
    } else {
	vp->vcx = limitn(1, vp->vcx, area.width - 2);
	if (vp->vcx + vp->vcy < area.halfheight + 1)
	  vp->vcx = area.halfheight + 1;
	if (vp->vcx + vp->vcy > area.width + area.halfheight - 1)
	  vp->vcx = area.width + area.halfheight - 1;
    }
}

void
center_on_focus(VP *vp)
{
    int sx, sy, xnw = vp->vcx;

    if (vp->isometric) {
	xform_cell(vp, vp->vcx, vp->vcy, &sx, &sy);
	sx += vp->sx - vp->pxw / 2;
	sy += vp->sy - vp->pxh / 2;
    } else {
	if (area.xwrap && xnw >= (area.width - vp->vcy / 2))
	  xnw -= area.width;
	/* Scale, add hex offset adjustment, translate to get left edge. */
	sx = xnw * vp->hw + (vp->vcy * vp->hw) / 2 - vp->pxw / 2 + vp->hw / 2;
	/* Scale, translate to top edge, flip. */
	sy = vp->totsh - (vp->vcy * vp->hch + vp->pxh / 2 + vp->hh / 2);
    }
    set_view_position(vp, sx, sy);
    DGprintf("Viewport 0x%x at %d,%d, focused at %d,%d\n",
	     vp, vp->sx, vp->sy, vp->vcx, vp->vcy);
}

int
set_view_size(VP *vp, int w, int h)
{
    if (w < 1 || h < 1)
      run_error("Bad viewport size %dx%d", w, h);
    vp->pxw = w;  vp->pxh = h;
    calc_view_misc(vp);
    return TRUE;
}

int
set_view_position(VP *vp, int sx, int sy)
{
    vp->sx = sx;  vp->sy = sy;
    /* Clip to rational limits. */
    vp->sx = limitn(vp->sxmin, vp->sx, vp->sxmax);
    vp->sy = limitn(vp->symin, vp->sy, vp->symax);
    return TRUE;
}

/* Given a magnification power, calculate and cache the sizes within a cell,
   and the scaled size in pixels of the entire world. */

int
set_view_power(VP *vp, int power)
{
    vp->power = power;
    vp->mag = mags[power]; /* is this used?? */
    vp->hw = hws[power];  vp->hh = hhs[power];
    vp->hch = hcs[power];
    vp->uw = uws[power];  vp->uh = uhs[power];
    if (vp->angle == 30) {
	vp->hh /= 2;
	vp->hch /= 2;
    } else if (vp->angle == 15) {
	vp->hh /= 4;
	vp->hch /= 4;
    }
    calc_view_misc(vp);
    DGprintf("Viewport 0x%x power is now %d, total scaled area is %d x %d\n",
	     vp, vp->power, vp->totsw, vp->totsh);
    return TRUE;
}

int
set_view_focus(VP *vp, int x, int y)
{
    if (!in_area(x, y))
      run_error("View focus of %d,%d not in area", x, y);
    vp->vcx = x;  vp->vcy = y;
    return TRUE;
}

int
set_view_angle(VP *vp, int angle)
{
    if (!(angle == 90 || angle == 30 || angle == 15)) {
	run_warning("Bad angle %d, setting to 90", angle);
	angle = 90;
    }
    vp->angle = angle;
    vp->hh = hhs[vp->power];
    vp->hch = hcs[vp->power];
    vp->uh = uhs[vp->power];
    if (vp->angle == 30) {
	vp->hh /= 2;
	vp->hch /= 2;
	vp->uh /= 2;
    } else if (vp->angle == 15) {
	vp->hh /= 4;
	vp->hch /= 4;
	vp->uh /= 4;
    }
    calc_view_misc(vp);
    DGprintf("Angle is now %d, total scaled area is %d x %d\n",
	     vp->angle, vp->totsw, vp->totsh);
    return TRUE;
}

int
set_view_isometric(VP *vp, int flag, int scale)
{
    vp->isometric = flag;
    vp->vertscale = scale;
    calc_view_misc(vp);
    return TRUE;
}

int
set_view_direction(VP *vp, int dir)
{
    vp->isodir = dir;
    calc_view_misc(vp);
    return TRUE;
}

static void
calc_view_misc(VP *vp)
{
    if (vp->isometric) {
	/* Common stuff for NE, SW, SE & NW. */
	vp->totsw = area.width * vp->hch + (vp->hh - vp->hch);
	vp->totsh = area.width * vp->hw / 4 + area.height * vp->hw / 4;
	vp->sxmin = vp->hw / 4;
	vp->symin = 0;
	if (area.xwrap) {
		vp->totsh += area.height * vp->hw / 4 ;
		vp->symin -= area.halfheight * vp->hw / 4;
	}
 	switch (vp->isodir) {
	  case NORTHEAST:
	  	vp->symin += (area.height - 2 * area.halfheight - 1) * vp->hw / 4;
		break;
	  case SOUTHWEST:
		vp->symin += area.halfheight * vp->hw / 4;
		break;
	  case SOUTHEAST:
		vp->sxmin += (area.height - area.halfheight - 1) * vp->hch;
		break;
	  case NORTHWEST:
		vp->sxmin += area.halfheight * vp->hch;
		break;
	  case EAST:
	  case WEST:
		vp->totsw = area.height * vp->hch + (vp->hh - vp->hch);
		vp->totsh = (area.width * vp->hw) / 2;
		vp->sxmin = 0;
		vp->symin = 0;
		break;
	  default:
		case_panic("view direction", vp->isodir);
		break;
	}
	/* (should add any adjust due to elevated cells) */
   } else {
	/* Calculate and cache the width in pixels of the whole area. */
	vp->totsw = area.width * vp->hw;
    
	/* This has an effect in the tcltk interface which is similar to setting
	   scroll_beyond_dateline in that it becomes possible to scroll one full
	   screen beyond the dateline. However, the new terrain is not drawn
	   correctly (scrolling bug). */
#if 0    
	if (area.xwrap && (vp->totsw > (vp->pxw - vp->hw)))
	  vp->totsw += (vp->pxw - vp->hw);
#endif
	/* Total scaled height is based on center-to-center height, plus
	   an adjustment to include the bottom parts of the bottom row. */
	vp->totsh = area.height * vp->hch + (vp->hh - vp->hch);
	vp->sxmin = (area.xwrap ? 0 : (area.halfheight * vp->hw) / 2);
	vp->symin = 0;
    }
    /* Make it posssible to scroll beyond the dateline, thus scrolling around 
    the globe. Currently supported only in the mac ppx interface. Note: if this 
    is enabled in the tcltk interface, scrolling continues beyond the dateline, 
    but the new terrain is not drawn (scrolling bug). In the SDL interface, 
    scrolling continues and the terrain is drawn also beyond the dateline, but 
    subsequent screen updates fail (black garbage appears under the cursor 
    when moved). */
    if (area.xwrap && vp->scroll_beyond_dateline) {
	vp->sxmax = vp->totsw;
    } else {
         vp->sxmax = vp->sxmin + vp->totsw - vp->pxw;
	vp->sxmax = max(vp->sxmin, vp->sxmax);  
    }
    vp->symax = vp->symin + vp->totsh - vp->pxh;
    vp->symax = max(vp->symin, vp->symax);
    vp->sx = limitn(vp->sxmin, vp->sx, vp->sxmax);
    vp->sy = limitn(vp->symin, vp->sy, vp->symax);
}

void
free_vp(VP *vp)
{
    free(vp);
}

/* Given a row in the viewport, compute the starting cell and length
   of the row of cells to draw. */

int
compute_x1_len(VP *vp, int vx, int vy, int y, int *x1p, int *lenp)
{
    int x1, x2, vw, halfheight = area.height / 2;

    /* Compute the number of cells visible in this row. */
    vw = (vp->pxw + vp->hw - 1) / vp->hw;
    vw = min(vw, area.width);
    /* Adjust the right and left bounds to fill the viewport as much
       as possible, without going too far (the drawing code will clip,
       but clipped drawing is still expensive). */
    x1 = vx - (y - vy) / 2;
    /* We add 1 cell to x2 since the leftmost column bug fix in 
    draw_map_overhead has decreased vx by 1/2 cell width, thus
    causing len to be too short in some cases. Note: this is probably 
    overkill. Should try to nail down exactly when len needs to be
    increased by 1 cell. */
    x2 = x1 + vw + 1;
    if (1 /* should be more precise */)
      --x1;
    if (1 /* should be more precise */)
      ++x2;
    if (area.xwrap) {
    } else {
	/* Truncate x's to stay within the area. */
	x1 = max(0, min(x1, area.width-1));
	x2 = max(0, min(x2, area.width));
	/* If this row is entirely in the NE corner, don't draw
	   anything. */
	if (x1 + y > area.width + halfheight)
	  return FALSE;
	/* If this row is entirely in the SW corner, don't draw
	   anything. */
	if (x2 + y < halfheight)
	  return FALSE;
	/* If the row ends up in the NE corner, shorten it. */
	if (x2 + y > area.width + halfheight)
	  x2 = area.width + halfheight - y;
	/* If the row starts out in the SW corner, shorten it. */
	if (x1 + y < halfheight)
	  x1 = halfheight - y;
    }
    *x1p = x1;
    *lenp = x2 - x1;
    return (*lenp > 0);
}

int
any_borders_in_dir(int x, int y, int dir)
{
    int b;

    if (!any_aux_terrain_defined())
      return FALSE;
    for_all_border_types(b) {
	/* Note that border_at tests for layer existence. */
	if (border_at(x, y, dir, b))
	  return TRUE;
    }
    return FALSE;
}

int
any_coating_at(int x, int y)
{
  int t;

  if (numcoattypes == 0)
    return NONTTYPE;
  for_all_terrain_types(t) {
    if (t_is_coating(t)
	&& aux_terrain_defined(t)
	&& aux_terrain_at(x, y, t) > 0)
      return t;
  }
  return NONTTYPE;
}

/* Compute the transition between two cells with different terrain,
   returning the positioning of the transition and which of four
   subimages to use. */

int
compute_transition(Side *side, VP *vp, int x, int y, int dir,
		   int *sxp, int *syp, int *swp, int *shp, int *offsetp)
{
    int t, x1, y1, t1, x2, y2, sx, sy, sw, sh, sx2, sy2;
    int trite, tleft, overrite, overleft, offset;
    int w = vp->hw, h = vp->hh, hch = vp->hch;

    if (!point_in_dir(x, y, dir, &x1, &y1))
      return FALSE;
    if (!(vp->show_all || terrain_visible(side, x, y)))
      return FALSE;
    if (!(vp->show_all || terrain_visible(side, x1, y1)))
      return FALSE;
    t = terrain_at(x, y);
    t1 = terrain_at(x1, y1);
    /* We want to overlap the adjacent terrain into this cell if the
       terrain type is higher-numbered (meaning that land usually
       overlaps sea for instance), but not if there is a border,
       because the border itself provides the transition. */
    if (t1 < t || any_borders_in_dir(x, y, dir))
      return FALSE;
    /* If coatings differ we still want a transition even if the terrain
    is the same. The test for <= ensures that only one transition is drawn
    between cells with different coatings. */
    if (t1 == t 
    	&& any_coating_at(x, y) <= any_coating_at(x1, y1)
    	&& cell_overlay(vp, x, y) <= cell_overlay(vp, x1, y1))
      return FALSE;
    /* We don't want to do overlap if there is a beach and 
    shorelines are drawn. */
    if ((t_liquid(t) != t_liquid(t1))
        && vp->draw_shorelines)
      return FALSE;
    if (point_in_dir(x, y, right_dir(dir), &x2, &y2))
      trite = terrain_at(x2, y2);
    else
      trite = t;
    /* Overlap on the right-hand corner if the third cell's terrain is
       the same as the adjacent cell's terrain. */
    overrite = (trite == t1);
    /* Border on one side suppresses overlap. */
    if (any_borders_in_dir(x, y, right_dir(dir)))
      overrite = FALSE;
    if (point_in_dir(x, y, left_dir(dir), &x2, &y2))
      tleft = terrain_at(x2, y2);
    else
      tleft = t;
    /* Overlap on the left-hand corner if the third cell's terrain is
       the same as the adjacent cell's terrain. */
    overleft = (tleft == t1);
    /* Border on one side suppresses overlap. */
    if (any_borders_in_dir(x, y, left_dir(dir)))
      overleft = FALSE;
    xform_cell(vp, x, y, &sx, &sy);
    /* Now, given the overlaps on each side, choose the right piece of
       transition bitmap to use. */
    switch (dir) {
      case NORTHEAST:
	sx2 = sx + w / 2;  sy2 = sy;
	sw = w / 2;  sh = h - hch + 1;
	if (overrite)
	  offset = 1;
	else if (overleft)
	  offset = 2;
	break;
      case EAST:
	sx2 = sx + w / 2;  sy2 = sy + (h - hch) + 1;
	sw = w / 2;  sh = h - 2 * (h - hch) - 2;
	if (overrite)
	  offset = 2;
	else if (overleft)
	  offset = 1;
	break;
      case SOUTHEAST:
	sx2 = sx + w / 2;  sy2 = sy + hch - 1;
	sw = w / 2;  sh = h - hch + 1;
	if (overrite)
	  offset = 1;
	else if (overleft)
	  offset = 2;
	break;
      case SOUTHWEST:
	sx2 = sx;  sy2 = sy + hch - 1;
	sw = w / 2;  sh = h - hch + 1;
	if (overrite)
	  offset = 2;
	else if (overleft)
	  offset = 1;
	break;
      case WEST:
	sx2 = sx;  sy2 = sy + (h - hch) + 1;
	sw = w / 2;  sh = h - 2 * (h - hch) - 2;
	if (overrite)
	  offset = 1;
	else if (overleft)
	  offset = 2;
	break;
      case NORTHWEST:
	sx2 = sx;  sy2 = sy;
	sw = w / 2;  sh = h - hch + 1;
	if (overrite)
	  offset = 2;
	else if (overleft)
	  offset = 1;
	break;
    }
    if (overrite && overleft)
      offset = 0;
    else if (!overrite && !overleft)
      offset = 3;
    *sxp = sx2;  *syp = sy2;
    *swp = sw;  *shp = sh;
    *offsetp = offset;
    return TRUE;
}

void
compute_fire_line_segment(int sx1, int sy1, int sx2, int sy2, int i, int n,
			  int *xx, int *yy, int *dx, int *dy)
{
    /* Position one segment of a line between the locations. */
    *dx = (sx2 - sx1) / n;  *dy = (sy2 - sy1) / n;
    *xx = sx1 + ((i / 2) % n) * *dx;  *yy = sy1 + ((i / 2) % n) * *dy;
}

/* This routine can be used by the interface to place legends */

/* orient==0 :  E (horizontal) only;
   orient==1 :  E, SE, NE;
   orient==2 :  E, SE, NE, ESE, ENE, N; */

/* block==0  :  write over any unit;
   block==1  :  don't write over "city-like" units;
   block==2  :  don't write over visible units. */

#if 0

void
place_feature_legends(Legend *legend, int nf, Side *side, int orient,
		      int block)
{
    int x, y, x1, y1, dx, dy, f, i, i3, j, d, ndirstotry, d1, dc;
    double dist;
    static int numdirstotry[] = { 1, 3, 6 };
    static int dirstotry[] =
    { EAST, SOUTHEAST, NORTHEAST, NORTHEAST, SOUTHEAST, EAST };
    static int da[] = { 0, -60, 60, 90, -30, 30 };
    unsigned char *auxf_layer, dmask;
    
    if (!features_defined())
      return;
    
    orient = min(orient, 2);
    ndirstotry = numdirstotry[orient];
    
    for (f = 1; f <= nf; f++) {
	legend[f-1].ox = 0;  legend[f-1].oy = 0;
	legend[f-1].dx = 0;  legend[f-1].dy = 0;
	legend[f-1].angle = 0;
	legend[f-1].dist  = -1;
    }
    
    /* Speedup: in auxf_layer we keep this information:
       the cell is unseen or hosts a blocking unit (bit 7);
       the cell has already been reached from direction id (bit id)
       [this avoids repeating the same path over and over;
	note that directions 3,4,5 zig-zag with step 3,
	so this bit is set/checked only every 3 steps.] */
    
    auxf_layer = (unsigned char *)
      malloc(area.height * area.width * sizeof(unsigned char));
    
    if (auxf_layer == NULL)
      return;
    
    for_all_cells(x, y) {
	if (terrain_seen_at(side, x, y) == NONTTYPE ||
	    blocking_utype(utype_seen_at(side, x, y), block)) {
	    aset(auxf_layer, x, y, '\200');
	} else {
	    aset(auxf_layer, x, y, '\0');
	}
    }
    
    for_all_cells(x, y) {
	f = raw_feature_at(x, y);
	if (f < 1 || f > nf)
	  continue;

	for (j = 0; j < ndirstotry; j++) {
	    dmask = '\001' << j;
	    d = dirstotry[j];
	    d1 = ((j < 3) ? d : left_dir(left_dir(d)));
	    x1 = x;  y1 = y;
	    dx = dy = 0;
	    i3 = i = 0;
	    dist = 0;
	    while (raw_feature_at(x1, y1) == f &&
		   !(aref(auxf_layer, x1, y1) &
		     ((j < 3 || !i3) ? ('\200' | dmask) : '\200'))) {
		if (dist > legend[f-1].dist && (j < 3 || !i3)) {
		    legend[f-1].ox = x;  legend[f-1].oy = y;
		    legend[f-1].dx = dx;  legend[f-1].dy = dy;
		    legend[f-1].angle = da[j];
		    legend[f-1].dist  = dist;
		}
		if (j < 3 || !i3) {
		    auxf_layer[area.width * y1 + x1] |= dmask;
		}
		dc = ((i3 == 1) ? d1 : d);
		dx += dirx[dc];
		x1 += dirx[dc];
		dy += diry[dc];
		y1 += diry[dc];
		dist += ((j < 3) ? 1.0 : (i3 ? 0.5 * 1.73205080756888 : 0.0));
		++i;
		i3 = i % 3;
	    }
	}
    }
    
    free(auxf_layer);
}

#else

void
place_feature_legends(Legend *legend, int nf, Side *side, int orient,
		      int block)
{
    int x, y, x1, dx, f;
    double dist;
    unsigned char *auxf_layer, dmask;
    
    if (!features_defined())
      return;
    
    for (f = 1; f <= nf; f++) {
	legend[f-1].ox = 0;  legend[f-1].oy = 0;
	legend[f-1].dx = 0;  legend[f-1].dy = 0;
	legend[f-1].angle = 0;
	legend[f-1].dist  = -1;
    }
    
    /* Speedup: in auxf_layer we keep this information:
       the cell is unseen or hosts a blocking unit (bit 7);
       the cell has already been reached from direction id (bit id)
       [this avoids repeating the same path over and over;
	note that directions 3,4,5 zig-zag with step 3,
	so this bit is set/checked only every 3 steps.] */
    
    auxf_layer = (unsigned char *)
      malloc(area.height * area.width * sizeof(unsigned char));
    
    if (auxf_layer == NULL)
      return;
    
    for_all_cells(x, y) {
	if (terrain_seen_at(side, x, y) == NONTTYPE ||
	    blocking_utype(utype_seen_at(side, x, y), block)) {
	    aset(auxf_layer, x, y, '\200');
	} else {
	    aset(auxf_layer, x, y, '\0');
	}
    }
    
    for_all_cells(x, y) {
	f = raw_feature_at(x, y);
	if (f < 1 || f > nf)
	  continue;

	    dmask = '\001';
	    x1 = x;  
	    dx = 0;
	    dist = 0;
	    while (raw_feature_at(x1, y) == f &&
		   !(aref(auxf_layer, x1, y) & ('\200' | dmask))) {
		if (dist > legend[f-1].dist) {
		    legend[f-1].ox = x;  
		    legend[f-1].oy = y;
		    legend[f-1].dx = dx;  
		    legend[f-1].dy = 0;
		    legend[f-1].angle = 0;
		    legend[f-1].dist  = dist;
		}
		auxf_layer[area.width * y + x1] |= dmask;
		dx += 1;
		x1+= 1;
		dist += 1;
	    }
    }
    
    free(auxf_layer);
}

#endif

static int
blocking_utype(int u, int block)
{
    if (u == NONUTYPE || block == 0)
      return 0;
    if (block > 1)
      return 1;
    /* block==1:  only visible see-always unmovable units */
    return ((u_already_seen(u) > 99 || u_see_always(u)) && !mobile(u));
}

/* Set the spacing of the meridians of latitude and longitude. */

void
set_meridian_interval(VP *vp, int interval)
{
    vp->meridian_interval = interval;
    /* Don't want to compute the lat/lon label spacing yet, we don't
       necessarily have the window's size recorded in the viewport. */
    vp->lat_label_lon_interval = vp->lon_label_lat_interval = 0;
}

/* Compute the location of meridians desired for the given viewport
   and render the lines and labels using the two callback
   functions. */

/* (should attempt to draw the lines all the way to the edge of
   the area, but formulas more complicated) */

void vp_latlong(VP *vp, int *lat1p, int *lon1p, int *lat2p, int *lon2p);

void
plot_meridians(VP *vp,
	       void (*line_callback)(int x1, int y1, int x1f, int y1f,
				     int x2, int y2, int x2f, int y2f),
	       void (*text_callback)(int x1, int y1, int x1f, int y1f,
				     char *str))
{
    int lat1, lon1, lat2, lon2, latmin, latmax, lonmin, lonmax, incr, lat, lon;
    int latmid, lonmid, xmid, ymid, xmidf, ymidf;
    int x1, y1, x2, y2, x1f, y1f, x2f, y2f;
    int sx1, sy1, sx2, sy2;
    int latdeg, latminu, londeg, lonminu;
    char minbuf[10];

    incr = vp->meridian_interval;
    /* Draw only if the interval is not too small. */
    xy_to_latlong(area.halfwidth, area.height - 2, 0, 0, &lat, &lon);
    latlong_to_xy(lat, lon, &x1, &y1, &x1f, &y1f);
    latlong_to_xy(lat - incr, lon, &x2, &y2, &x2f, &y2f);
    /* If a single interval down from the top middle of the area is
       off the map, then there won't be any lines to plot anyway, so
       OK to escape. */
    if (!in_area(x2, y2))
      return;
    xform_cell_fractional(vp, x1, y1, x1f, y1f, &sx1, &sy1);
    xform_cell_fractional(vp, x2, y2, x2f, y2f, &sx2, &sy2);
    /* Don't draw if lines would be really closely spaced. */
    if (sy2 - sy1 < 20)
      return;
    /* Find the closest meridians that will be visible in the
       viewport. */
    vp_latlong(vp, &lat1, &lon1, &lat2, &lon2);
    /* Round to even multiples of the meridian interval. */
    latmin = (lat1 / incr) * incr;
    latmax = (lat2 / incr) * incr + incr;
    lonmin = (lon1 / incr) * incr;
    lonmax = (lon2 / incr) * incr + incr;
    if (lonmax <= lonmin)
      lonmax += 21600;
    vp_latlong(vp, &lat1, &lon1, &lat2, &lon2);
    /* Determine the spacing of the text labels.  They should be far
       enough apart so that only one set of each is visible at any
       time, although it's more OK for two sets to appear than for
       none to be visible. */
    /* This algorithm assumes that we first do a draw of the whole
       viewport before using smaller viewports for incremental
       updates. */
    if (vp->lon_label_lat_interval == 0) {
	vp->lon_label_lat_interval = (abs(lat2 - lat1) / incr) * incr;
	if (vp->lon_label_lat_interval == 0)
	  vp->lon_label_lat_interval = 1;
	vp->lat_label_lon_interval = (abs(lon2 - lon1) / incr) * incr;
	if (vp->lat_label_lon_interval == 0)
	  vp->lat_label_lon_interval = 1;
    }
    /* Draw each line as an individual segment, because meridians may
       not be straight lines. */
    for (lat = latmax; lat >= latmin; lat -= incr) {
	for (lon = lonmin; lon <= lonmax; lon += incr) {
	    latlong_to_xy(lat, lon, &x1, &y1, &x1f, &y1f);
	    if (in_area(x1, y1)) {
		latlong_to_xy(lat - incr, lon, &x2, &y2, &x2f, &y2f);
		if (line_callback) {
		    if (in_area(x2, y2)) {
			(*line_callback)(x1, y1, x1f, y1f, x2, y2, x2f, y2f);
		    }
		    latlong_to_xy(lat, lon + incr, &x2, &y2, &x2f, &y2f);
		    if (in_area(x2, y2)) {
			(*line_callback)(x1, y1, x1f, y1f, x2, y2, x2f, y2f);
		    }
		}
		if (text_callback) {
		    if (lon % vp->lat_label_lon_interval == 0) {
			lonmid = lon + incr / 2;
			latlong_to_xy(lat, lonmid,
				      &xmid, &ymid, &xmidf, &ymidf);
			/* If adding the incr goes off the world,
			   subtract instead. */
			if (!in_area(xmid, ymid)) {
			    lonmid = lon - incr / 2;
			    latlong_to_xy(lat, lonmid,
					  &xmid, &ymid, &xmidf, &ymidf);
			}
			/* Give up if this did not work. */
			if (in_area(xmid, ymid)) {
			    latdeg = abs(lat) / 60;
			    latminu = abs(lat) % 60;
			    minbuf[0] = '\0';
			    if (latminu != 0)
			      sprintf(minbuf, "%dm", latminu);
			    sprintf(tmpbuf, "%dd%s%c",
				    latdeg, minbuf, (lat >= 0 ? 'N' : 'S'));
			    (*text_callback)(xmid, ymid, xmidf, ymidf, tmpbuf);
			}
		    }
		    if (lat % vp->lon_label_lat_interval == 0) {
			latmid = lat + incr / 2;
			latlong_to_xy(latmid, lon,
				      &xmid, &ymid, &xmidf, &ymidf);
			/* If adding the incr goes off the world,
			   subtract instead. */
			if (!in_area(xmid, ymid)) {
			    latmid = lat - incr / 2;
			    latlong_to_xy(latmid, lon,
					  &xmid, &ymid, &xmidf, &ymidf);
			}
			/* If this position is valid, draw the
                           longitude. */
			if (in_area(xmid, ymid)) {
			    londeg = abs(lon) / 60;
			    lonminu = abs(lon) % 60;
			    minbuf[0] = '\0';
			    if (lonminu != 0)
			      sprintf(minbuf, "%dm", lonminu);
			    sprintf(tmpbuf, "%dd%s%c",
				    londeg, minbuf, (lon >= 0 ? 'E' : 'W'));
			    (*text_callback)(xmid, ymid, xmidf, ymidf, tmpbuf);
			}
		    }
		}
#if 0 /* use to debug meridian plotting */
		sprintf(tmpbuf, "%d %d", lat, lon);
		(*text_callback)(x1, y1, x1f, y1f, tmpbuf);
		latlong_desc(tmpbuf, x1, y1, x1f, y1f, 3);
		(*text_callback)(x1, y1, x1f, y1f, tmpbuf);
#endif
	    }
	}
    }
}

void
vp_latlong(VP *vp, int *lat1p, int *lon1p, int *lat2p, int *lon2p)
{
    int xmin, ymin, xmax, ymax, xminf, yminf, xmaxf, ymaxf;

    /* The implementation of nearest_cell still does the correct
       calculation, even if the pixel position is not in the area. */
    nearest_cell(vp, 0, vp->pxh, &xmin, &ymin, &xminf, &yminf);
    nearest_cell(vp, vp->pxw, 0, &xmax, &ymax, &xmaxf, &ymaxf);
    xy_to_latlong(xmin, ymin, xminf, yminf, lat1p, lon1p);
    xy_to_latlong(xmax, ymax, xmaxf, ymaxf, lat2p, lon2p);
}

/* Use this to set the contour interval, or, by setting to 0, let it
   float to a plausible value based on the actual range of
   elevations. */

void
set_contour_interval(VP *vp, int n)
{
    int ncontours;

    if (n > 0) {
	/* Set contour interval, compute num contours from it. */
	vp->contour_interval = n;
	ncontours = (area.maxelev - area.minelev) / vp->contour_interval;
	vp->contour_interval_fixed = TRUE;
    } else {
	/* Set num contours, compute contour interval. */
	ncontours = max(1, min(15, area.maxelev - area.minelev));
	vp->contour_interval = (area.maxelev - area.minelev) / ncontours;
	vp->contour_interval_fixed = FALSE;
    }
    if (ncontours != vp->num_contours) {
	vp->num_contours = ncontours;
	if (vp->linebuf != NULL) {
	    free(vp->linebuf);
	    vp->linebuf = NULL;
	}
    }
}

/* The theory of contour lines is that each hex can be considered as
   six triangles, each of which has a vertex at the center and two
   on adjacent corners of the hex.  The elevation of the center vertex
   is the overall elevation of the cell, the elevations of the corners
   are averages with the adjacent cells.  If a particular contour
   elevation is between any pair of vertex elevations, then the contour
   line must cross that side of the triangle - and one of the other two
   sides.  We decide which of the two it is, interpolate to get the
   actual positions of each endpoint of the line segment, then draw it. */

void
contour_lines_at(VP *vp, int x, int y, int sx, int sy, LineSegment **lines,
		 int *numlinesp)
{
    int el, el2, dir, x1, y1, sum, n, lowest, liq0, liq, ecor[NUMDIRS], ec;
    int maxel;
    int sxcor[NUMDIRS], sycor[NUMDIRS], sxc, syc;
    int power = vp->power;
    int ecorr, ecorl, sxcorr, sycorr, sxcorl, sycorl;
    int sx1, sy1, sx2, sy2;

    *numlinesp = 0;
    if (vp->contour_interval < 1)
      return;
    if (vp->linebuf == NULL)
      vp->linebuf = (LineSegment *) xmalloc ((vp->num_contours + 2) * 2 * NUMDIRS * sizeof(LineSegment));
    *lines = vp->linebuf;
    /* It's possible that the contour intervals have not yet been
       adjusted to account for all elevations in the world, so be
       sure to clip. */ 
    maxel = area.minelev + vp->num_contours * vp->contour_interval;
    el = elev_at(x, y);
    if (el > maxel)
      el = maxel;
    sxc = sx + vp->hw / 2;  syc = sy + vp->hh / 2;
    /* Compute the elevation at each corner of the cell. */
    liq0 = t_liquid(terrain_at(x, y));
    for_all_directions(dir) {
	sum = el;
	n = 1;
	lowest = el;
	liq = liq0;
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    el2 = elev_at(x1, y1);
	    if (el2 > maxel)
	      el2 = maxel;
	    sum += el2;
	    ++n;
	    lowest = min(lowest, el2);
	    if (t_liquid(terrain_at(x1, y1)))
	      liq = TRUE;
	}
	if (point_in_dir(x, y, left_dir(dir), &x1, &y1)) {
	    el2 = elev_at(x1, y1);
	    if (el2 > maxel)
	      el2 = maxel;
	    sum += el2;
	    ++n;
	    lowest = min(lowest, el2);
	    if (t_liquid(terrain_at(x1, y1)))
	      liq = TRUE;
	}
	/* Pick lowest, in the case of liquids, or average. */
	if (liq)
	  ecor[dir] = lowest;
	else
	  ecor[dir] = sum / n;
	sxcor[dir] = sx + bsx[power][dir];  sycor[dir] = sy + bsy[power][dir];
    }
    /* Iterate over all the possible elevations for contour lines. */
    for (ec = area.minelev + vp->contour_interval; ec < area.maxelev; ec += vp->contour_interval) {
	for_all_directions(dir) {
	    ecorr = ecor[dir];
	    ecorl = ecor[left_dir(dir)];
	    sxcorr = sxcor[dir];  sycorr = sycor[dir];
	    sxcorl = sxcor[left_dir(dir)];  sycorl = sycor[left_dir(dir)];
	    if (el != ecorr && between(min(el, ecorr), ec, max(el, ecorr))) {
		if (el != ecorl
		    && between(min(el, ecorl), ec, max(el, ecorl))) {
		    sx1 = sxc + ((sxcorr - sxc) * (ec - el)) / (ecorr - el);
		    sy1 = syc + ((sycorr - syc) * (ec - el)) / (ecorr - el);
		    sx2 = sxc + ((sxcorl - sxc) * (ec - el)) / (ecorl - el);
		    sy2 = syc + ((sycorl - syc) * (ec - el)) / (ecorl - el);
		    if (sx1 != sx2 || sy1 != sy2) {
			vp->linebuf[*numlinesp].sx1 = sx1;
			vp->linebuf[*numlinesp].sy1 = sy1;
			vp->linebuf[*numlinesp].sx2 = sx2;
			vp->linebuf[*numlinesp].sy2 = sy2;
			++(*numlinesp);
		    }
		} else if (ecorl != ecorr) {
		    sx1 = sxc + ((sxcorr - sxc) * (ec - el)) / (ecorr - el);
		    sy1 = syc + ((sycorr - syc) * (ec - el)) / (ecorr - el);
		    /* By inverting odd directions before the
		       calculation we ensure that the endpoints are
		       calculated in exactly the same way for the two
		       line segments from adjacent cells that should
		       end on the same point. This eliminates the
		       small jumps in contour lines as they cross from
		       one cell to another. */
		    /* Line ends on outer edge. */
		    if (dir == NORTHEAST || dir == SOUTHEAST || dir == WEST) {
			sx2 = sxcorr + ((sxcorl - sxcorr) * (ec - ecorr)) / (ecorl - ecorr);
			sy2 = sycorr + ((sycorl - sycorr) * (ec - ecorr)) / (ecorl - ecorr);
		    } else { 
			sx2 = sxcorl + ((sxcorl - sxcorr) * (ec - ecorl)) / (ecorl - ecorr);
			sy2 = sycorl + ((sycorl - sycorr) * (ec - ecorl)) / (ecorl - ecorr);
	            }
		    if (sx1 != sx2 || sy1 != sy2) {
			vp->linebuf[*numlinesp].sx1 = sx1;
			vp->linebuf[*numlinesp].sy1 = sy1;
			vp->linebuf[*numlinesp].sx2 = sx2;
			vp->linebuf[*numlinesp].sy2 = sy2;
			++(*numlinesp);
		    }
		}
	    }
	    if (el != ecorl && between(min(el, ecorl), ec, max(el, ecorl))) {
		if (ecorl != ecorr
		    && between(min(ecorr, ecorl), ec, max(ecorr, ecorl))) {
		    sx1 = sxc + ((sxcorl - sxc) * (ec - el)) / (ecorl - el);
		    sy1 = syc + ((sycorl - syc) * (ec - el)) / (ecorl - el);
		    /* Line ends on outer edge */
		    if (dir == NORTHEAST || dir == SOUTHEAST || dir == WEST) {
			sx2 = sxcorr + ((sxcorl - sxcorr) * (ec - ecorr)) / (ecorl - ecorr);
			sy2 = sycorr + ((sycorl - sycorr) * (ec - ecorr)) / (ecorl - ecorr);
		    } else { 
			sx2 = sxcorl + ((sxcorl - sxcorr) * (ec - ecorl)) / (ecorl - ecorr);
			sy2 = sycorl + ((sycorl - sycorr) * (ec - ecorl)) / (ecorl - ecorr);
		    }
		    if (sx1 != sx2 || sy1 != sy2) {
			vp->linebuf[*numlinesp].sx1 = sx1;
			vp->linebuf[*numlinesp].sy1 = sy1;
			vp->linebuf[*numlinesp].sx2 = sx2;
			vp->linebuf[*numlinesp].sy2 = sy2;
			++(*numlinesp);
		    }
		}
	    }
	}
    }
}

int
unit_visible(Side *side, VP *vp, Unit *unit)
{
    if (vp->show_all)
      return TRUE;
    /* Designer needs to see all units, even when show_all has been
       turned off - otherwise cells with units look empty. */
    if (is_designer(side))
      return TRUE;
    return side_sees_unit(side, unit);
}

/* Don't draw the temperature in every cell, only do ones with even
   coords or ones where the temperature in any adjacent cell is
   different. */

int
draw_temperature_here(Side *side, int x, int y)
{
    int dir, x1, y1, temphere = temperature_view(side, x, y);

    /* Designers should see temperature in every cell. */
    if (is_designer(side))
      return TRUE;
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    if (temphere != temperature_view(side, x1, y1))
	      return TRUE;
	    /* Always show temperature around edge of known area. */
	    if (terrain_view(side, x1, y1) == UNSEEN)
	      return TRUE;
	}
    }
    return (x % 2 == 0 && y % 2 == 0);
}

/* Don't draw the winds in every cell, only do ones with odd coords or
   ones where the wind in any adjacent cell is different. */

int
draw_winds_here(Side *side, int x, int y)
{
    int dir, x1, y1, windhere = wind_view(side, x, y);

    /* Designers should see wind in every cell. */
    if (is_designer(side))
      return TRUE;
    /* Don't draw wind in unseen areas. */
    if (terrain_view(side, x, y) == UNSEEN)
      return FALSE;
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    if (windhere != wind_view(side, x1, y1))
	      return TRUE;
	    /* Always show wind around edge of known area. */
	    if (terrain_view(side, x1, y1) == UNSEEN)
	      return TRUE;
	}
    }
    return (x % 2 == 1 && y % 2 == 1);
}

/* Return a textual description of what is at the given pixel in the
   given viewport. */

void
oneliner(Side *side, VP *vp, int sx, int sy)
{
    int x, y, xf, yf;
    Unit *unit, *unit2, *user;
    int t2, u, ps = NOBODY, cs = NOBODY, dep, sayin = FALSE, userid;
    const char *peopdesc = NULL;
    const char *str;
    char descbuf[80], ctrlbuf[80], buf[BUFSIZE];
    Side *side2, *side3;
    Feature *feature;
    UnitView *uview;

    /* Return an empty result if the point is outside the viewport
       altogether. */
    if (!(between(0, sx, vp->pxw) && between(0, sy, vp->pxh))) {
	tmpbuf[0] = '\0';
	return;
    }
    nearest_cell(vp, sx, sy, &x, &y, &xf, &yf);
    nearest_unit(side, vp, sx, sy, &unit);
    nearest_unit_view(side, vp, sx, sy, &uview);
    if (!in_area(x, y)) {
	strcpy(tmpbuf, "(nothing)");
	return;
    } else if (vp->show_all || terrain_visible(side, x, y)) {
	strcpy(tmpbuf, " ");
	/* Describe the side of the people here. */
	if (people_sides_defined()) {
	    ps = people_side_at(x, y);
	    if (ps != NOBODY) {
		side2 = side_n(ps);
		if (side2 == NULL) {
		    peopdesc = "indep";
		} else if (side2 == side) {
		    peopdesc = "your";
		} else {
		    peopdesc = side_adjective(side2);
		    if (peopdesc[0] == '\0') {
			sprintf(descbuf, "s%d", side2->id);
			peopdesc = descbuf;
		    }
		}
	    }
	}
	if (control_sides_defined()) {
	    cs = control_side_at(x, y);
	    if (cs != ps) {
		side3 = side_n(cs);
		if (side3 == NULL) {
		    strcpy(ctrlbuf, "uncontrolled");
		} else if (side3 == side) {
		    strcpy(ctrlbuf, "your");
		} else {
		    strcpy(ctrlbuf, side_adjective(side3));
		    if (ctrlbuf[0] == '\0') {
			sprintf(ctrlbuf, "s%d", side3->id);
		    }
		    strcat(ctrlbuf, "-controlled");
		}
		if (peopdesc != NULL) {
		    strcat(ctrlbuf, " ");
		    strcat(ctrlbuf, peopdesc);
		}
		peopdesc = ctrlbuf;
	    }
	}
	if (vp->show_all || (uview != NULL && uview->siden == side->id)) {
	    if (unit != NULL) {
		if (unit->side != side) {
		    if (ps != NOBODY && ps == side_number(unit->side)) {
			peopdesc = "own";
		    }
		}
		strcat(tmpbuf, unit_handle(side, unit));
		sayin = TRUE;
	    }
	} else {
	    if (uview != NULL) {
		u = uview->type;
		side2 = side_n(uview->siden);
		if (ps != NOBODY && ps == side2->id) {
		    peopdesc = "own";
		}
		/* Display the real name of a non-mobile unit,
		   even if it's not in view coverage. */
		/* (should fix this to use view stack) */
		unit = NULL;
		for_all_stack(x, y, unit2) {
		    if (unit2->type == u
			&& (!mobile(u) || type_max_acp(u) == 0)) {
			unit = unit2;
			break;
		    }
		}
		if (unit != NULL) {
		    strcat(tmpbuf,
			   apparent_unit_handle(side, unit, side2));
		} else {
		    strcat(tmpbuf, side_adjective(side2));
		    strcat(tmpbuf, " ");
		    strcat(tmpbuf, u_type_name(u));
		}
		sayin = TRUE;
	    }
	}
	if (sayin) {
	    strcat(tmpbuf, " (in ");
	}
	if (peopdesc != NULL) {
	    strcat(tmpbuf, peopdesc);
	    strcat(tmpbuf, " ");
	}
	if (vp->show_all)
	  t2 = terrain_at(x, y);
	else
	  t2 = vterrain(terrain_view(side, x, y));
	strcat(tmpbuf, t_type_name(t2));
	if (sayin) {
	    strcat(tmpbuf, ")");
	}
	/* (should be able to display views of these) */
	if (elevations_defined()) {
	    tprintf(tmpbuf, " Elev %d", elev_at(x, y));
	}
	if (temperatures_defined()) {
	    tprintf(tmpbuf, " T %ddeg", temperature_at(x, y));
	}
	if (numcoattypes > 0) {
	    for_all_terrain_types(t2) {
		if (t_is_coating(t2)
		    && aux_terrain_defined(t2)
		    && ((dep = aux_terrain_view(side, x, y, t2)) > 0)) {
		    tprintf(tmpbuf, " %s %d", t_type_name(t2), dep);
		}
	    }
	}
    } else {
	sprintf(tmpbuf, "(unknown)");
    }
    strcat(tmpbuf, " @");
    if (1 /* drawxy */) {
	tprintf(tmpbuf, "%d,%d", x, y);
    } else if (vp->draw_meridians) {
	latlong_desc(descbuf, x, y, xf, yf, 3);
	strcat(tmpbuf, descbuf);
    }
    if (vp->show_all || terrain_visible(side, x, y)) {
	feature = feature_at(x, y);
	if (feature != NULL) {
	    if (feature->size > 0) {
		str = feature_desc(feature, buf);
		if (str != NULL) {
		    strcat(tmpbuf, " (");
		    strcat(tmpbuf, str);
		    strcat(tmpbuf, ")");
		}
	    }
	}
    }
    if (1 /* drawxy */ && vp->draw_meridians) {
	latlong_desc(descbuf, x, y, xf, yf, 3);
	strcat(tmpbuf, " (");
	strcat(tmpbuf, descbuf);
	strcat(tmpbuf, ")");
    }
    if (user_defined()
	&& ((userid = user_at(x, y)) != NOUSER)) {
	user = find_unit(userid);
	if (in_play(user)) {
	    strcat(tmpbuf, " (used by ");
	    if (user->name != NULL)
	      strcat(tmpbuf, user->name);
	    else
	      sprintf(tmpbuf+strlen(tmpbuf), "%s u#%d",
		      u_type_name(user->type), user->id);
	    strcat(tmpbuf, ")");
	}
    }
    if (vp->draw_ai && side_has_ai(side)) {
	str = ai_at_desig(side, x, y);
	if (str) {
	    strcat(tmpbuf, " ");
	    strcat(tmpbuf, str);
	}
    }
}

/* (needs a better home?) */

/* Given a side and a unit, calculate the correct "next unit".  Typically
   used by autonext options, thus the name. */

Unit *
autonext_unit(Side *side, Unit *unit)
{
    int i, uniti = -1, n, numdelayed = 0;
    Unit *nextunit;

    if (!side->ingame
	|| side->finishedturn
	|| side->actionvector == NULL)
      return NULL;
    if (could_be_next_unit(unit) && side_controls_unit(side, unit)) 
      return unit;
    for (i = 0; i < side->actionvector->numunits; ++i) {
	nextunit = unit_in_vector(side->actionvector, i);
    	if (in_play(nextunit) && side_controls_unit(side, nextunit)) {
	    if (!is_active(unit) || unit == nextunit) {
		uniti = i;
		break;
	    }
    	}
    }
    if (uniti < 0)
      return NULL;
  try_again:
    /* (should scan for both a preferred and an alternate - preferred
       could be within a supplied bbox so as to avoid scrolling) */
    for (i = uniti; i < uniti + side->actionvector->numunits; ++i) {
    	n = i % side->actionvector->numunits;
    	nextunit = unit_in_vector(side->actionvector, n);
    	if (could_be_next_unit(nextunit) && side_controls_unit(side, nextunit))
    	  return nextunit;
	if (nextunit->plan && nextunit->plan->delayed && has_acp_left(nextunit))
	  ++numdelayed;
    }
    /* If any units were delayed and another unit was not selected, 
       then it is perhaps time to undelay everyone on the side and look 
       again. */
    if (numdelayed) {
	for (i = 0; i < side->actionvector->numunits; ++i) {
	    nextunit = unit_in_vector(side->actionvector, i);
	    if (nextunit->plan)
	      net_delay_unit(nextunit, FALSE);
	}
	numdelayed = 0;
	goto try_again;
    }
    return NULL;
}

/*
 * This should really be called autonext_unit and the decision
 * whether to check inbox or not should depend on the bbox being
 * valid. i.e. could be called with -1,-1,-1,-1 to disable the bbox.
 */
Unit *
autonext_unit_inbox(Side *side, Unit *unit, VP *vp)
{
    int i, u, mx, my, val, prefval = -999, v = 10;
    Unit *nextunit = NULL, *prefunit = NULL;

    if (!side->ingame || side->finishedturn || side->actionvector == NULL)
      return NULL;

    /* degenerate case... this unit still has stuff to do. */
    if (could_be_next_unit(unit) && side_controls_unit(side, unit))
	return unit;
    
    if (unit == NULL) {
	u = 0;
	if (!nearest_cell(vp, vp->sx + vp->pxw / 2, vp->sy + vp->pxh / 2, 
			  &mx, &my, NULL, NULL)) {
	    mx = area.width / 2;  my = area.halfheight;
	}
    } else {
	u = unit->type;
	mx = unit->x;  my = unit->y;
    }
    for (i = 0; i < side->actionvector->numunits; ++i) {
    	nextunit = unit_in_vector(side->actionvector, i);
	if (side_controls_unit(side, nextunit) 
	    && could_be_next_unit(nextunit)) {
	    val = v - distance(nextunit->x, nextunit->y, mx, my);
	    if (cell_is_in_middle(vp, nextunit->x, nextunit->y))
	      val += v;
	    if (nextunit->type == u)
	      val += 2;

	    if (val > prefval) {
		prefval = val;
		prefunit = nextunit;
	    }
	}
    }
    return prefunit;
}

int
could_be_next_unit(Unit *unit)
{
    return (unit != NULL
	    && is_active(unit)
	    && has_acp_left(unit)
	    && (unit->plan
	    	&& !unit->busy
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& !unit->plan->delayed
		&& unit->plan->waitingfortasks));
}

/* Do a depth-first traversal of all the occupants of a unit. */

Unit *
find_next_occupant(Unit *unit)
{
    Unit *nextup;

    if (unit->occupant != NULL) {
	return unit->occupant;
    } else if (unit->nexthere != NULL) {
	return unit->nexthere;
    } else {
	nextup = unit->transport;
	if (nextup != NULL) {
	    while (nextup->transport != NULL && nextup->nexthere == NULL) {
		nextup = nextup->transport;
	    }
	    if (nextup->nexthere != NULL)
	      return nextup->nexthere;
	    if (nextup->transport == NULL)
	      return nextup;
	} else {
	    /* This is a no-op if there is no stacking within a hex. */
	    return unit_at(unit->x, unit->y);
	}
    }
    return unit;
}

int
find_units_matching(Side *side, char *name, Unit **unitp)
{
    int num = 0;
    Unit *unit;

    if (empty_string(name))
      return 0;
    for_all_units(unit) {
	if (!empty_string(unit->name)
	    && strstr(unit->name, name)
	    && side_sees_image(side, unit)) {
	    *unitp = unit;
	    ++num;
	}
    }
    return num;
}

Unit *
embarkation_unit(Unit *unit)
{
    Unit *transport, *occ;

    /* look for the first possible transport */
    for_all_stack(unit->x, unit->y, transport) {
	/* make sure its not the transport we're in and we can enter it */
	if (transport != unit->transport &&
	    valid(check_enter_action(unit, unit, transport))) {
	    return transport;
	}
	/* check the occupants too */
	for_all_occupants(transport, occ) {
	    if (occ != unit->transport &&
		valid(check_enter_action(unit, unit, occ))) {
		return occ;
	    }
	}
    }
    return NULL;
}

/* Given a character, compute the direction(s) that it represents.
   Return the number of directions. */

int
char_to_dir(int ch, int *dir1p, int *dir2p, int *modp)
{
    char basech, *rawdir;
    int ndirs = 0;

    if (isupper(ch)) {
	basech = tolower(ch);
	if (modp)
	 *modp = 1;
    } else if (ch < ' ') {
	basech = ch + 0x60;
	if (modp)
	  *modp = 2;
    } else {
	basech = ch;
	if (modp)
	  *modp = 0;
    }
    rawdir = strchr(dirchars, basech);
    if (rawdir) {
	*dir1p = rawdir - dirchars;
	ndirs = 1;
    } else if (basech == 'k') {
	if (flip_coin()) {
	    *dir1p = NORTHEAST;
	    if (dir2p)
	      *dir2p = NORTHWEST;
	} else {
	    *dir1p = NORTHWEST;
	    if (dir2p)
	      *dir2p = NORTHEAST;
	}
	ndirs = 2;
    } else if (basech == 'j') {
	if (flip_coin()) {
	    *dir1p = SOUTHEAST;
	    if (dir2p)
	      *dir2p = SOUTHWEST;
	} else {
	    *dir1p = SOUTHWEST;
	    if (dir2p)
	      *dir2p = SOUTHEAST;
	}
	ndirs = 2;
    }
    return ndirs;
}

/* Given that the player desires to move the given unit into the given
   cell/other unit, prepare a "most appropriate" action. */
/* (should share diff cell and same cell interaction code) */

int
advance_into_cell(Side *side, Unit *unit, int x, int y, Unit *other,
		  HistEventType *reason)
{
    int z, m, rslt = H_UNDEFINED;

    /* Make sure we have some default value. */
    if (reason)
      *reason = (HistEventType)rslt;
#ifdef DESIGNERS
    /* Designers use this function to push units around, bound only by the
       limits on occupancy. */
    if (is_designer(side))
      return net_designer_teleport(unit, x, y, other);
#endif /* DESIGNERS */
    z = unit->z;
    if (x != unit->x || y != unit->y) {
	/* Units that can't act/plan can't do anything to adjacent units. */
	if (unit->act == NULL || unit->plan == NULL) {
	    notify(side, "%s cannot act right now.", unit_handle(side, unit));
	    return FALSE;
	}
	if (!mobile(unit->type)
	    && u_advanced(unit->type)
	    && distance(unit->x, unit->y, x, y) <= unit->reach) {
	    net_toggle_user_at(unit->id, x, y);
	    return TRUE;
	}
	if (distance(unit->x, unit->y, x, y) == 1) {
	    /* Destination is adjacent to us. */
	    /* First, handle the case where no unit was clicked on. */
	    if (other == NULL) {
		if (can_extract_at(unit, x, y, &m)
		    || can_load_at(unit, x, y, &m)) {
			net_set_collect_task(unit, m, x, y);
			return TRUE;
		} else if (mobile(unit->type)) {
		    rslt = check_move_action(unit, unit, x, y, z);
		    if (valid(rslt)) {
			net_prep_move_action(unit, unit, x, y, z);
			return TRUE;
		    }
		}
		if (reason)
	      	    *reason = (HistEventType)rslt;
		return FALSE;
	    }
	    /* If a unit was clicked on, decide how to interact. */
	    if (unit_trusts_unit(unit, other)
		|| (other->side == indepside
		    && sides_allow_entry(unit, other))) {
		/* A friend, maybe get on it. */
		if (can_occupy(unit, other)) {
		    if (valid(check_enter_action(unit, unit, other))) {
			net_prep_enter_action(unit, unit, other);
		    } else {
			/* (should schedule for next turn?) */
		    }
		} else if (can_occupy(other, unit)) {
		    if (type_max_acp(other->type) > 0) {
			/* Have other unit do an enter action, then
			   move. */
			/* Not quite right, move should happen after
			   other unit is actually inside, in case it
			   fills dest. */
			net_prep_enter_action(other, other, unit);
			net_set_move_to_task(unit, x, y, 0);
		    } else {
			net_prep_enter_action(unit, other, unit);
			net_set_move_to_task(unit, x, y, 0);
		    }
		} else if (other->transport != NULL
			   && can_occupy(unit, other->transport)) {
		    if (valid(check_enter_action(unit, unit,
						 other->transport))) {
			net_prep_enter_action(unit, unit,
					      other->transport);
		    } else {
			/* (should schedule for next turn?) */
		    }
		} else if (other->transport != NULL
			   && other->transport->transport != NULL
			   && can_occupy(unit, other->transport->transport)) {
		    /* two levels up should be sufficient */
		    if (valid(
			    check_enter_action(unit, unit, 
					       other->transport->transport))) {
			net_prep_enter_action(unit, unit, 
					      other->transport->transport);
		    } else {
			/* (should schedule for next turn?) */
		    }
		} else if (can_extract_at(unit, x, y, &m)
			   || can_load_at(unit, x, y, &m)) {
		    /* (should test extraction from specific unit) */
		    net_set_collect_task(unit, m, x, y);
		} else {
		    rslt = check_move_action(unit, unit, x, y, z);
		    if (valid(rslt)) {
			net_prep_move_action(unit, unit, x, y, z);
			return TRUE;
		    }
		    if (reason)
		      *reason = (HistEventType)rslt;
		    return FALSE;
		}
	    } else {
		/* Somebody else's unit, try to victimize it in
		   various ways, trying coexistence only as a last
		   resort. */
		rslt = check_capture_action(unit, unit, other);
		if (valid(rslt)) {
		    net_prep_capture_action(unit, unit, other);
		    return TRUE;
		}
		rslt = check_overrun_action(unit, unit, x, y, z, 100);
		if (valid(rslt)) {
		    net_prep_overrun_action(unit, unit, x, y, z, 100);
		    return TRUE;
		}
		if (reason && rslt == A_ANY_NO_AMMO)
		  *reason = (HistEventType)rslt;
		rslt = check_attack_action(unit, unit, other, 100);
		if (valid(rslt)) {
		    net_prep_attack_action(unit, unit, other, 100);
		    return TRUE;
		}
		if (reason && rslt == A_ANY_NO_AMMO)
		  *reason = (HistEventType)rslt;
		rslt = check_fire_at_action(unit, unit, other, -1);
		if (valid(rslt)) {
		    net_prep_fire_at_action(unit, unit, other, -1);
		    return TRUE;
		}
		if (reason && rslt == A_ANY_NO_AMMO)
		  *reason = (HistEventType)rslt;
		rslt = check_detonate_action(unit, unit, x, y, z);
		if (valid(rslt)) {
		    net_prep_detonate_action(unit, unit, x, y, z);
		    return TRUE;
		}
		if (can_extract_at(unit, x, y, &m)
		    || can_load_at(unit, x, y, &m)) {
		    /* (should test extraction from specific unit) */
		    net_set_collect_task(unit, m, x, y);
		    return TRUE;
		}
		rslt = check_move_action(unit, unit, x, y, z);
		if (valid(rslt)) {
		    net_prep_move_action(unit, unit, x, y, z);
		    return TRUE;
		}
		/* None of the possible actions worked, so fail. */
		return FALSE;
	    }
	} else {
	    /* We're not adjacent to the destination; actions alone
	       won't suffice. */
	    if (can_extract_at(unit, x, y, &m)
		|| can_load_at(unit, x, y, &m)) {
		net_set_collect_task(unit, m, x, y);
	    } else if (mobile(unit->type)) {
		/* Although it's theoretically possible for a
		   non-mobile unit to fulfill a move task by being
		   carried all the way, in practice players are going
		   to direct the transport to make the move, not its
		   immobile occupants; so only allow mobile units and
		   beep the player for trying to move a non-mobile
		   unit. */
		if (impl_move_to(side, unit, x, y, 0)) {
		    return TRUE;
		} else {
			if (reason) {
			    /* Not always true, but it suppresses the redundant
			    error message. */
			    *reason = (HistEventType) A_MOVE_NO_PATH;
		    	}
		    return FALSE;
		}
	    } else {
		return FALSE;
	    }
	}
    /* Destination is in the unit's own cell. */
    } else {
	/* First handle advanced units that are trying to toggle their own cell. */
	if (!mobile(unit->type)
	    && u_advanced(unit->type)) {
		net_toggle_user_at(unit->id, x, y);
		return TRUE;
	} else if (other != NULL) {
	    if (unit_trusts_unit(unit, other)) {
		if (valid(check_enter_action(unit, unit, other))) {
		    net_prep_enter_action(unit, unit, other);
		} else {
		    return FALSE;
		}
	    } else {
		/* Somebody else's unit, try to victimize it in various ways,
		   trying coexistence only as a last resort. */
		if (valid(check_capture_action(unit, unit, other))) {
		    net_prep_capture_action(unit, unit, other);
		} else if (valid(check_attack_action(unit, unit, other, 100))) {
		    net_prep_attack_action(unit, unit, other, 100);
		} else if (valid(check_fire_at_action(unit, unit, other, -1))) {
		    net_prep_fire_at_action(unit, unit, other, -1);
		} else if (valid(check_detonate_action(unit, unit, x, y, z))) {
		    net_prep_detonate_action(unit, unit, x, y, z);
		} else {
		    return FALSE;
		}
	    }
	} else if (unit->transport != NULL) {
	    /* Unit is an occupant wanting to leave, but yet remain in
	       the same cell as the transport. */
	    rslt = check_move_action(unit, unit, x, y, z);
	    if (valid(rslt)) {
		net_prep_move_action(unit, unit, x, y, z);
		return TRUE;
	    }
	    if (reason)
	      *reason = (HistEventType)rslt;
	    return FALSE;
	/* Unit is trying to extract materials in the same cell. */
	} else if (can_extract_at(unit, x, y, &m)
	    || can_load_at(unit, x, y, &m)) {
		net_set_collect_task(unit, m, x, y);
	} else {
	    /* This is a no-op, don't do anything. */
	}
    }
    /* All the failures got filtered out by early returns, so anything
       that got here is a success. */
    return TRUE;
}

/* Return if it's possible for the given unit to attack a unit of the
   given type, side, and location. */
/* (should return immed/later, type of attack possible) */

int
unit_could_attack(Unit *unit, int u2, Side *side2, int x, int y)
{
    int u = unit->type;

    if (trusted_side(unit->side, side2))
      return FALSE;
    switch (g_combat_model()) {
      case 0:
	/* See if a direct attack is possible. */
	if (uu_acp_to_attack(u, u2) > 0
	    && uu_hit(u, u2) > 0
#if 0 /* assume we can get there eventually */
	    && between(uu_attack_range_min(u, u2),
		       distance(unit->x, unit->y, x, y)
		       uu_attack_range(u, u2))
#endif
	    )
	  return TRUE;
	/* See if attack by firing is possible. */
	/* (should use this to test blocking elev) */
        if (fire_hit_chance(u, u2))
	  return TRUE;
      case 1:
      default:
           return TRUE;
    }
    return FALSE;
}

/* Given a unit and amounts of supplies desired to transfer, move as
   much of them as possible into the unit's transport, or to a nearby
   units if there is no transport. */

static int give_supplies_to_one(Unit *unit, Unit *unit2, short *amts,
				short *rslts);

Unit *
give_supplies(Unit *unit, short *amts, short *rslts)
{
    int dir, x1, y1, didsome;
    Unit *unit2;

    unit2 = unit->transport;
    if (unit2 != NULL) {
	didsome = give_supplies_to_one(unit, unit2, amts, rslts);
	if (didsome)
	  return unit2;
    }
    for_all_occs_with_occs(unit, unit2) {
	/* Note that we might have enemy spies as occupants, so test
	   relationship even here. */
	if (unit_trusts_unit(unit, unit2)) {
	    didsome = give_supplies_to_one(unit, unit2, amts, rslts);
	    if (didsome)
	      return unit2;
	}
    }
    for_all_stack(unit->x, unit->y, unit2) {
	if (unit2 != unit && unit_trusts_unit(unit, unit2)) {
	    didsome = give_supplies_to_one(unit, unit2, amts, rslts);
	    if (didsome)
	      return unit2;
	}
    }
    /* Look around for adjacent units to give to. */
    for_all_directions(dir) {
	if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
	    for_all_stack(x1, y1, unit2) {
		if (unit_trusts_unit(unit, unit2)) {
		    didsome = give_supplies_to_one(unit, unit2, amts, rslts);
		    if (didsome)
		      return unit2;
		}
	    }
	}
    }
    /* Now try again on adjacent occupants.  We do it this way so
       top-level adjacent units get tried before occupants, since
       for_all_stack_with_occs iterates depth-first. */
    for_all_directions(dir) {
	if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
	    for_all_stack_with_occs(x1, y1, unit2) {
		if (unit_trusts_unit(unit, unit2)) {
		    didsome = give_supplies_to_one(unit, unit2, amts, rslts);
		    if (didsome)
		      return unit2;
		}
	    }
	}
    }
    return NULL;
}

/* Given a pair of units, have one give the desired amounts to the
   other. */

static int
give_supplies_to_one(Unit *unit, Unit *unit2, short *amts, short *rslts)
{
    int m, gift, maxgift, actual, didsome = FALSE;

    if (!(in_play(unit2) && completed(unit2)))
      return FALSE;
    for_all_material_types(m) {
	if (rslts)
	  rslts[m] = 0;
	maxgift = min(unit->supply[m],
		      um_storage_x(unit2->type, m) - unit2->supply[m]);
	gift = ((amts == NULL || amts[m] == -1) ? (maxgift / 2) : amts[m]);
	if (gift > 0) {
	    if (1 /* can do immed transfer */) {
		/* Be stingy if giver is low */
		if (2 * unit->supply[m] < um_storage_x(unit->type, m))
		  gift = max(1, gift / 2);
		actual = transfer_supply(unit, unit2, m, gift);
		if (rslts)
		  rslts[m] = actual;
		if (actual > 0)
		  didsome = TRUE;
	    }
	}
    }

    if (didsome) {
	/* Recompute the supply_is_low flags for both unit and unit2.  */
	if (unit->plan != NULL
	    && !unit->plan->supply_is_low
	    && past_halfway_point(unit)
	    ) {
	    unit->plan->supply_is_low = TRUE;
	    update_unit_display(unit->side, unit, TRUE);
	}
	if (unit2->plan != NULL
	    && unit2->plan->supply_is_low
	    && !past_halfway_point(unit2)
	    ) {
	    unit2->plan->supply_is_low = FALSE;
	    update_unit_display(unit2->side, unit2, TRUE);
	}
    }

    return didsome;
}

/* Attempt to transfer the given amounts of material from the unit's
   transport into the unit. */

int
take_supplies(Unit *unit, short *amts, short *rslts)
{
    int m, want, actual, neededsome;
    Unit *unit2;

    neededsome = FALSE;
    for_all_material_types(m) {
	if (rslts)
	  rslts[m] = 0;
	want = ((amts == NULL || amts[m] == -1)
	        ? (um_storage_x(unit->type, m) - unit->supply[m])
	        : amts[m]);
	if (want > 0) {
	    neededsome = TRUE;
	    unit2 = unit->transport;
	    if (in_play(unit2) && completed(unit2)) {
	    	/* Bug fix. Units inside a transport inside a base or
	    	   city were previously unable to resupply directly
	    	   from the base. Now the occ checks if the transport
	    	   is inside a base and then resupplies directly from
	    	   it. Moreover, the 'stingy' 50% left principle is
	    	   applied only to units that need m themselves. Other
	    	   units are free to give away all their supply. */
	    
		Unit *base = unit2->transport;
		int request = want;				

		/* First take as much supplies as possible from the
		   transport's transport (the base) if it exists. */
		actual = 0;
		if (in_play(base) && completed(base)) {
		    /* Be stingy if base is low (but only if it also
                       needs m). */
		    if (2 * base->supply[m] < um_storage_x(base->type, m)
			&& (um_base_consumption(base->type, m) > 0 
			    || um_consumption_per_move(base->type, m) > 0)) {
			request = max(1, want/2);
		    }
		    actual += transfer_supply(base, unit, m, request);
		    want -= actual;
		}
		/* If still unsatisfied, also take supplies from
                   transport. */
		if (want) {
		    request = want;
		    /* Be stingy if transport is low (but only if it
		       also needs m). */
		    if (2 * unit2->supply[m] < um_storage_x(unit2->type, m)
			&& (um_base_consumption(unit2->type, m) > 0 
			    || um_consumption_per_move(unit2->type, m) > 0)) {
			request = max(1, want/2);
		    }
		    actual += transfer_supply(unit2, unit, m, want);
		}
		if (rslts)
		  rslts[m] = actual;
	    }
	}
    }

    if (neededsome) {
	/* Recompute the supply_is_low flags for both unit and unit2.  */
	if (unit2 != NULL
	    && unit2->plan != NULL
	    && !unit2->plan->supply_is_low
	    && past_halfway_point(unit2)
	    ) {
	    unit2->plan->supply_is_low = TRUE;
	    update_unit_display(unit2->side, unit2, TRUE);
	}
	if (unit->plan != NULL
	    && unit->plan->supply_is_low
	    && !past_halfway_point(unit)
	    ) {
	    unit->plan->supply_is_low = FALSE;
	    update_unit_display(unit->side, unit, TRUE);
	}
    }

    return neededsome;
}

int
impl_move_to(Side *side, Unit *unit, int x, int y, int dist)
{
#ifdef DESIGNERS
	if (is_designer(side)) {
		notify(side, "%s will teleport to (%d,%d).", 
					unit_handle(side, unit), x, y);
		net_designer_teleport(unit, x, y, NULL);
		return TRUE;
	}
#endif /* DESIGNERS */
	/* Check that we can put the unit at the destination. */
	if (!side_thinks_it_can_put_type_at(side, unit->type, x, y)) {
		notify(side, "%s cannot move to (%d,%d). No room.", 
					unit_handle(side, unit), x, y);
		notify(side, 
		       "Please pick a new destination (or Escape to cancel).");
		return FALSE;
	}
	/* Disable pre-flight command checks for now, since we don't
	have any path-finding. */
	if (1 /*choose_move_direction(unit, x, y, 0) >= 0*/) {
		notify(side, "%s will move to (%d,%d).", 
					unit_handle(side, unit), x, y);
		net_set_move_to_task(unit, x, y, 0);
		return TRUE;
    	} else {
		notify(side, "%s cannot move to (%d,%d). No way there.", 
					unit_handle(side, unit), x, y);
		notify(side, 
		       "Please pick a new destination (or Escape to cancel).");
		return FALSE;		    	
    	}
}

/* Common user interface implementation of the 'build' command. */

int
impl_build(
    Side *side, Unit *unit, int u2, Unit *transport, int x, int y, int n)
{
	int range, u = unit->type;
	Unit *resume = NULL, *occ = NULL;
	UnitView *uview = NULL;

	/* This is to support the curses interface, which cannot 
           pick places to build and therefore passes 0 as x and y. */
	if (x == -1 && y == -1) {
	    x = unit->x;
	    y = unit->y;
	}
	/* Check that build is possible before proceeding. */
	if (!unit_can_build_type_at(unit, u2, x, y)) {
	    return FALSE;
	}
	// Check if we are trying to build inside an enemy.
	if (transport && !allied_side(unit->side, transport->side))
	    return FALSE;
	/* If mobile builder, we overrule doctrines and default run
	   length since we want to ask where to build each new unit. */ 
	if (mobile(u) && n <= 0) {
	    n = 1;
	}
	/* Check the doctrine. */
	if (n < 0) {
	    n = construction_run_doctrine(unit, u2);
	}
	/* If no doctrine, we default to 1. */
	if (n <= 0) {
	    n = 1;
	}
	notify(side, "%s will build %d %s at (%d,%d).", 
	       unit_handle(side, unit), n, u_type_name(u2), x, y);
	if (transport) {
	    for_all_occupants(transport, occ) {
		if (completed(occ))
		    continue;
		if (!allied_side(unit->side, occ->side))
		    continue;
		if (u2 != occ->type)
		    continue;
		resume = occ;
		break;
	    }
	    if (resume)
		net_set_build_task(unit, resume->id, u_cp(u2) - resume->cp);
	    // BUG: Need to have check that does not consider distance.
	    else if (valid(can_create_in(unit, unit, u2, transport)))
		net_set_construct_task(unit, u2, n, transport->id, -1, -1);
	    // HACK: Maybe a non-transport was clicked on.
	    //	    Try building in cell instead of unit.
	    else
		net_set_construct_task(unit, u2, n, -1, x, y);
	} // if transport
	else {
	    for_all_view_stack(unit->side, x, y, uview) {
		if (!allied_side(unit->side, side_n(uview->siden)))
		    continue;
		if (u2 != uview->type)
		    continue;
		resume = view_unit(uview);
		break;
	    }
	    if (resume) {
		net_set_build_task(unit, resume->id, u_cp(u2) - resume->cp);
	    }
	    else
		net_set_construct_task(unit, u2, n, -1, x, y);
	}
	/* We may need to move to get to where we're supposed to build.
	    Just push a task to get within range. */
	range = min(uu_create_range(u, u2), uu_build_range(u, u2));
	if (distance(x, y, unit->x, unit->y) > range) {
	    net_push_move_to_task(unit, x, y, range);
	}
	return TRUE;
}

/* Common user interface implementation of the 'change-type' command. */

int
impl_change_type(Side *side, Unit *unit, int u2)
{
    int rslt = -1;

    if (!type_can_occupy_cell_without(u2, unit->x, unit->y, unit)) {
        cmd_error(side, "%s cannot change into a %s: not enough room.", 
                  unit_handle(side, unit), u_type_name(u2));
        return FALSE;
    }
    rslt = check_change_type_action(unit, unit, u2);
    if (valid(rslt)) {
	notify(side, "%s changes into a %s.", 
		unit_handle(side, unit), u_type_name(u2));
        net_prep_change_type_action(unit, unit, u2);
    }
    else {
        cmd_error(side, "%s cannot change into a %s: %s.",
                  unit_handle(side, unit), u_type_name(u2), 
                  action_result_desc(rslt));
        return FALSE;
    }
    return TRUE;
}

/* Return the type to build that dialogs should highlight initially. */

int
favored_type(Unit *unit)
{
    int u;

    if (unit == NULL)
      return NONUTYPE;
    /* If a type is already in the works, return it. */
    if (unit->plan
	&& unit->plan->tasks
	&& unit->plan->tasks->type == TASK_BUILD)
      return unit->plan->tasks->args[0];
    /* Return the first buildable type found. */
    for_all_unit_types(u) {
	if (unit_can_build_type(unit, u))
	  return u;
    }
    return NONUTYPE;
}

void
set_unit_image(Unit *unit)
{
    int u = unit->type, choice, i = 0;
    ImageFamily *imf;
    
    /* Some interfaces don't have images. */
    if (!uimages) {
    	return;
    }
    /* Use the preassigned image if available. */
    if (!empty_string(unit->image_name)) {
    	imf = find_imf(unit->image_name);
    	if (imf) {
	    unit->imf = imf;
	    return;
    	}
    }
    /* Else pick a random image from the list for this unit type. */
    choice = xrandom(numuimages[u]);
    for (imf = uimages[u]; imf != NULL; imf = imf->next) {
	if (i == choice) {    	
	    unit->imf = imf;
	    unit->image_name = imf->name;
	    break;
    	}
	++i;
    }
}

void
set_unit_view_image(UnitView *uview)
{
    int u = uview->type, choice, i = 0;
    ImageFamily *imf;
    
    /* Some interfaces don't have images. */
    if (!uimages) {
    	return;
    }
    /* Use the preassigned image if available. */
    if (!empty_string(uview->image_name)) {
    	imf = find_imf(uview->image_name);
    	if (imf) {
	    uview->imf = imf;
	    uview->image_name = imf->name;
	    return;
    	}
    }
    /* Else pick a random image from the list for this unit type. */
    choice = xrandom(numuimages[u]);
    for (imf = uimages[u]; imf != NULL; imf = imf->next) {
	if (i == choice) {    	
	    uview->imf = imf;
	    uview->image_name = imf->name;
	    break;
    	}
	++i;
    }
}

ImageFamily *
get_unit_type_images(Side *side, int u)
{
    Obj *image_name, *rest;
    const char *name;
    ImageFamily *imf = NULL, *nextimf;

    if (!numuimages) {
    	numuimages = (int *) xmalloc(numutypes * sizeof(int));
    }
    image_name = u_image_name(u);
    if (image_name != lispnil) {
	if (stringp(image_name)) {
	    name = c_string(image_name);
	    imf = get_generic_utype_images(u, name);
	    ++numuimages[u];
	} else if (consp(image_name)) {
	    for_all_list(image_name, rest) {
		if (stringp(car(rest))) {
		    name = c_string(car(rest));
		    if (imf) {
			nextimf->next = get_generic_utype_images(u, name);
			nextimf = nextimf->next;
		    } else {
			imf = get_generic_utype_images(u, name);
			nextimf = imf;
		    }				
		    ++numuimages[u];
		}
	    }	
	}
    } else {
      name = u_internal_name(u);
	imf = get_generic_utype_images(u, name);
	++numuimages[u];
    }
    return imf;
}

ImageFamily *
get_generic_utype_images(int u, const char *name)
{
    ImageFamily *imf = NULL;

    /* Get IMF corresponding to name. */
    imf = get_generic_images(name);
    /* If an image is void, then substitute a default one. */
    if (imf != NULL && imf->numsizes == 0) {
	imf->ersatz = TRUE;
	imf = add_default_unit_image(imf, u);
    }
    /* Register image for posterity. */
    record_imf_get(imf);
#if (0)
    /* Check if the largest image is wider than 32 pixels. */
    if (imf->images->w > 32) {
    	big_unit_images = TRUE;
    }
#endif
    return imf;
}

static ImageFamily *
add_default_unit_image(ImageFamily *imf, int u)
{
    int i, hi, lo;
    Image *img;

    img = get_img(imf, 16, 16);
    if (img == NULL)
      return imf;
    img->rawmonodata = (char *)xmalloc(32);
    img->rawmaskdata = (char *)xmalloc(32);
    hi = u >> 4;
    lo = (u & 0xf) << 4;
    for (i = 4; i < 28; i += 2) {
	(img->rawmonodata)[i] = hi;
	(img->rawmonodata)[i + 1] = lo;
	(img->rawmaskdata)[i] = 0x7f;
	(img->rawmaskdata)[i + 1] = 0xfe;
    }
    (img->rawmonodata)[14] = 0x1f;
    (img->rawmonodata)[15] = 0xf8;
    (img->rawmonodata)[16] = 0x1f;
    (img->rawmonodata)[17] = 0xf8;
    (img->rawmaskdata)[2] = 0x7f;
    (img->rawmaskdata)[3] = 0xfe;
    (img->rawmaskdata)[28] = 0x7f;
    (img->rawmaskdata)[29] = 0xfe;
    if (imf_interp_hook)
      imf = (*imf_interp_hook)(imf, NULL, FALSE);
    return imf;
}

/* Acquire all imagery relating to the given material type. */

ImageFamily *
get_material_type_images(Side *side, int m)
{
    const char *name;
    ImageFamily *imf;

    if (!empty_string(m_image_name(m)))
      name = m_image_name(m);
    else
      name = m_type_name(m);
    imf = get_generic_images(name);
    if (imf != NULL && imf->numsizes == 0) {
	imf->ersatz = TRUE;
	imf = add_default_material_image(imf, m);
    }
    record_imf_get(imf);
    return imf;
}

/* The default material image is ugly but functional; basically a
   binary encoding of the material type number.  Also add a shade of
   gray, for use when solid colors are wanted. */

static ImageFamily *
add_default_material_image(ImageFamily *imf, int m)
{
    int gray;
    Image *img;

    img = get_img(imf, 8, 8);
    img->istile = TRUE;
    img->rawmonodata = (char *)xmalloc(8);
    (img->rawmonodata)[1] = (m << 2);
    (img->rawmonodata)[2] = (m << 2);
    (img->rawmonodata)[3] = 0x7e;
    (img->rawmonodata)[4] = (m << 2);
    (img->rawmonodata)[5] = (m << 2);
    /* Also add a shade of gray. */
    img = get_img(imf, 1, 1);
    img->istile = TRUE;
    /* Range from dark to light gray. */
    gray = 15000 + ((m * 45000) / nummtypes);
    img->palette = cons(cons(new_number(0),
			     cons(new_number(gray),
				  cons(new_number(gray),
				       cons(new_number(gray), lispnil)))),
			lispnil);
    if (imf_interp_hook)
      imf = (*imf_interp_hook)(imf, NULL, FALSE);
    return imf;
}

/* Acquire all imagery relating to the given terrain type. */

ImageFamily *
get_terrain_type_images(Side *side, int t)
{
    const char *name;
    ImageFamily *imf;

    if (!empty_string(t_image_name(t)))
      name = t_image_name(t);
    else
      name = t_type_name(t);
    imf = get_generic_images(name);
    if (imf != NULL && imf->numsizes == 0) {
	imf->ersatz = TRUE;
	imf = add_default_terrain_image(imf, t);
    }
    record_imf_get(imf);
    return imf;
}

/* The default terrain image is ugly but functional; basically a binary
   encoding of the terrain type number. */

static ImageFamily *
add_default_terrain_image(ImageFamily *imf, int t)
{
    Image *img;

    img = get_img(imf, 8, 8);
    img->istile = TRUE;
    img->rawmonodata = (char *)xmalloc(8);
    (img->rawmonodata)[1] = (t << 2);
    (img->rawmonodata)[2] = (t << 2);
    (img->rawmonodata)[3] = 0x7e;
    (img->rawmonodata)[4] = (t << 2);
    (img->rawmonodata)[5] = (t << 2);
    if (imf_interp_hook)
      imf = (*imf_interp_hook)(imf, NULL, FALSE);
    return imf;
}

ImageFamily *
get_unseen_images(Side *side)
{
    if (!empty_string(g_unseen_color())) {
	unseen_image = get_generic_images(g_unseen_color());
	if (unseen_image != NULL && unseen_image->numsizes == 0) {
	    /* Appears to have failed - clear the unseen image then. */
	    unseen_image = NULL;
	    /* Note that we shouldn't try to free the imf, because it
	       may be in use elsewhere. */
	}
    }
    record_imf_get(unseen_image);
    return unseen_image;
}

ImageFamily *
get_emblem_images(Side *side, Side *side2)
{
    char *c, tmpembuf[BUFSIZE];
    const char *s, *name;
    int s2 = side_number(side2);
    ImageFamily *imf;

    if (side2 == NULL) {
	name = "s0";
    } else if (!empty_string(side2->emblemname)) {
	name = side2->emblemname;
    } else if (!empty_string(side2->colorscheme)) {
	/* Take the first (main) color if there are multiple colors. */
	for (s = side2->colorscheme, c = tmpembuf;; ++s) {
		/* Test for end of name or end of scheme. */
		if (*s == ',' || *s == '\0') {
			/* Terminate the name string. */
			*c = '\0';
			c = tmpembuf;
			break;
		/* Add one more character. */
		} else *c++ = *s;
	}
	name = copy_string(tmpembuf);
    	/* Make sure the color emblemname is set so that it is saved
    	   correctly. */
    	net_set_side_emblemname(side, side2, name);
    } else {
	/* There is a set of default emblems named "s1", etc. */
	sprintf(tmpembuf, "s%d", s2);
	name = copy_string(tmpembuf);
    	/* Make sure the default emblemname is set so that it is 
    	   saved correctly. */
    	net_set_side_emblemname(side, side2, name);
    }
    imf = get_generic_images(name);
    /* If we must have an image, and none were acquired, invoke the default
       image getter. */
    if (imf != NULL && imf->numsizes == 0 && strcmp(name, "none") != 0) {
	imf->ersatz = TRUE;
	imf = add_default_emblem_image(imf, s2);
    }
    record_imf_get(imf);
    return imf;
}

/* For a substitute emblem, make a solid white square with stripes at
   top and bottom, with the encoding of the side number in the
   middle.  (In practice, the number emblems usually get used.) */

static ImageFamily *
add_default_emblem_image(ImageFamily *imf, int s2)
{
    int i;
    Image *img;

    img = get_img(imf, 8, 8);
    if (img == NULL)
      return imf;
    img->rawmonodata = (char *)xmalloc(8);
    img->rawmaskdata = (char *)xmalloc(8);
    for (i = 0; i < 8; ++i) {
	(img->rawmonodata)[i] = s2;
	(img->rawmaskdata)[i] = 0xff;
    }
    (img->rawmonodata)[0] = 0xff;
    (img->rawmonodata)[7] = 0xff;
    if (imf_interp_hook)
      imf = (*imf_interp_hook)(imf, NULL, FALSE);
    return imf;
}

/* Record that the given image family was used.  This is used when
   saving a game, for instance. */

void
record_imf_get(ImageFamily *imf)
{
    int i;
    ImageFamily **new_record;

    if (imf == NULL)
      return;
    /* Estimate and allocate the usual amount of space needed. */
    if (max_recorded_imfs == 0)
      max_recorded_imfs = numutypes + numttypes + (MAXSIDES + 1) + 1;
    if (recorded_imfs == NULL) {
	recorded_imfs =
	  (ImageFamily **) xmalloc(max_recorded_imfs * sizeof(ImageFamily *));
    }
    /* Allocate more space if needed. */
    if (num_recorded_imfs >= max_recorded_imfs) {
	max_recorded_imfs += max_recorded_imfs / 2;
	new_record =
	  (ImageFamily **) xmalloc(max_recorded_imfs * sizeof(ImageFamily *));
	for (i = 0; i < num_recorded_imfs; ++i) {
	    new_record[i] = recorded_imfs[i];
	}
	recorded_imfs = new_record;
    }
    for (i = 0; i < num_recorded_imfs; ++i) {
	if (strcmp(imf->name, recorded_imfs[i]->name) == 0)
	  return;
    }
    recorded_imfs[num_recorded_imfs++] = imf;
    /* Expand any interface-specific data into its all-interface form,
       so that saved games will include it.  This needs to be done
       now, because game saving may occur in a low-memory situation
       and there may not be enough memory available then. */
    make_generic_image_data(imf);
}

/* Output a general description of an image family. */

void
describe_imf(Side *side, const char *classname, const char *imftype, ImageFamily *imf)
{
    Image *img;

    if (imf == NULL) {
	DGprintf("No image family for %s %s for %s",
		 classname, imftype, side_desig(side));
	return;
    }
    DGprintf("%s %s family for %s has %d images",
	     classname, imftype, side_desig(side), imf->numsizes);
    if (imf->location)
      DGprintf(" and is in %s", imf->location->name);
    DGprintf("\n");
    for_all_images(imf, img) {
	DGprintf("    %dx%d", img->w, img->h);
	if (img->istile)
	  DGprintf(" tile");
	if (img->isterrain)
	  DGprintf(" terrain");
	if (img->isconnection)
	  DGprintf(" connection");
	if (img->isborder)
	  DGprintf(" border");
	if (img->istransition)
	  DGprintf(" transition");
	if (img->numsubimages > 0)
	  DGprintf(" %d subimages", img->numsubimages);
	if (imf_describe_hook)
	  (*imf_describe_hook)(side, img);
	DGprintf("\n");
    }
}

/* Compute and cache single-char representations for things. */

void
init_ui_chars(void)
{
    int u, t;
    const char *str;

    if (unitchars == NULL) {
	unitchars = (char *)xmalloc(numutypes);
	for_all_unit_types(u) {
	    str = u_uchar(u);
	    unitchars[u] =
	      (!empty_string(str) ? str[0] : utype_name_n(u, 1)[0]);
	}
    }
    if (terrchars == NULL) {
	terrchars = (char *)xmalloc(numttypes);
	for_all_terrain_types(t) {
	    str = t_char(t);
	    terrchars[t] = (!empty_string(str) ? str[0] : t_type_name(t)[0]);
	}
    }
    unseen_char_1 = unseen_char_2 = ' ';
    str = g_unseen_char();
    if (strlen(str) >= 1) {
	unseen_char_1 = unseen_char_2 = str[0];
	if (strlen(str) >= 2) {
	    unseen_char_2 = str[1];
	}
    }
}

void
init_unit_images(Side *side)
{
    ImageFamily *imf;
    Image *smallest;
    UnitView *uview;
    Unit *unit;
    int u;

    if (!uimages)
      uimages = (ImageFamily **) xmalloc(numutypes * sizeof(ImageFamily *));
    for_all_unit_types(u) {
	imf = get_unit_type_images(side, u);
	if (DebugG)
	  describe_imf(side, "unit type", u_type_name(u), imf);
	uimages[u] = imf;
	/* This is used by the X11 interface only, to calculate 
	   image sizes for the unit type list. */
	smallest = smallest_image(imf);
	if (smallest) {
	    if (smallest->w > min_w_for_unit_image)
	      min_w_for_unit_image = smallest->w;
	    if (smallest->h > min_h_for_unit_image)
	      min_h_for_unit_image = smallest->h;
	}
    }
    for_all_units(unit) {
	if (unit->imf)
	  continue;
	/* Find and load any unit-specific images. */
	if (unit->image_name) {
	    imf = get_generic_utype_images(u, unit->image_name);
	    unit->imf = imf;
	}
	/* Fill in any missing unit images. */
	if (!unit->imf) {
	    set_unit_image(unit);
	}
    }
    for_all_unit_views(uview) {
	if (uview->imf)
	  continue;
	/* Get the unit. It will provide the authoritative image name in 
	   the case that uview's image name is empty. */
	unit = view_unit(uview);
	if (!unit)
	  continue;
	if (empty_string(uview->image_name) && (uview->type == unit->type))
	  uview->image_name = unit->image_name;
	/* Find and load any unit-specific images. */
	if (uview->image_name) {
	    imf = get_generic_utype_images(u, uview->image_name);
	    uview->imf = imf;
	}
	/* Fill in any missing unit view images. */
	if (!uview->imf) {
	    set_unit_view_image(uview);
	}
    }
}

/* Write the side's view of the world, as ASCII. */

/* (should be intelligent enough to cut into pages, or else document
   how to do it) */
/* (maybe display names too somehow, perhaps as second layer?) */

#define VIEWFILE "view.ccq"

void
dump_text_view(Side *side, int use_both_chars)
{
    char ch1, ch2;
    int x, y, t, u, s, draw, i;
    Side *side2;
    Unit *unit;
    UnitView *uview;
    FILE *fp;

    fp = open_file(VIEWFILE, "w");
    if (fp != NULL) {
	for (y = area.height-1; y >= 0; --y) {
	    for (i = 0; i < y; ++i)
	      fputc(' ', fp);
	    for (x = 0; x < area.width; ++x) {
		ch1 = ch2 = ' ';
		if (in_area(x, y) && terrain_visible(side, x, y)) {
		    t = terrain_at(x, y);
		    ch1 = terrchars[t];
		    ch2 = (use_both_chars ? ch1 : ' ');
		    draw = FALSE;
		    if (side->see_all) {
			unit = unit_at(x, y);
			if (unit != NULL) {
			    u = unit->type;
			    s = side_number(unit->side);
			    draw = TRUE;
			}
		    } else {
			uview = unit_view_at(side, x, y);
			if (uview != NULL) {
			    u = uview->type;
			    s = uview->siden;
			    draw = TRUE;
			}
		    }
		    if (draw) {
			ch1 = unitchars[u];
			ch2 = ' ';
			if (between(1, s, 9))
			  ch2 = s + '0';
			else if (s >= 10)
			  /* This could get weird if s > 36, but not much
			     chance of that because MAXSIDES < 31 always. */
			  ch2 = s - 10 + 'A';
		    }
		}
		fputc(ch1, fp);
		fputc(ch2, fp);
	    }
	    fprintf(fp, "\n");
	}
	fprintf(fp, "\n\nTerrain Types:\n");
	for_all_terrain_types(t) {
	    fprintf(fp, "  %c%c  %s\n",
		    terrchars[t], terrchars[t], t_type_name(t));
	}
	fprintf(fp, "\n\nUnit Types:\n");
	for_all_unit_types(u) {
	    fprintf(fp, "  %c   %s\n", unitchars[u], u_type_name(u));
	}
	fprintf(fp, "\n\nSides:\n");
	for_all_sides(side2) {
	    fprintf(fp, "  %d   %s\n", side_number(side2), side_name(side2));
	}
	fclose(fp);
	notify(side, "Dumped area view to \"%s\".", VIEWFILE);
    } else {
	notify(side, "Can't open \"%s\"!!", VIEWFILE);
    }
}

/* Return the type of cell terrain that the given side sees at the given
   location. */

int
terrain_seen_at(Side *side, int x, int y)
{
    /* We are outside the map. */
    if (!in_area(x, y)) {
    	return BACKTTYPE;
    }
    /* The see all terrain cases. */
    if (side->see_all 
    	|| side->show_all
#ifdef DESIGNERS
	|| side->designer
#endif /* DESIGNERS */
        ) {
    	return terrain_at(x, y);
    }
    /* The normal case. */
    if (terrain_view(side, x, y) != UNSEEN) {
	return terrain_at(x, y);
    } else {
	return NONTTYPE;
    }
}

/* (should remove both of these, only used in ps.c currently) */
/* Return the unit seen by the given side at the given location.  Note
   that this should not be used casually by interfaces, since the result
   is a pointer to a real unit, not a view of one. */
/* (should result depend on contents of stack?) */

Unit *
unit_seen_at(Side *side, int x, int y)
{
    if (!in_area(x, y))
      return NULL;
    if (side->see_all
    	|| side->show_all
#ifdef DESIGNERS
	|| side->designer
#endif /* DESIGNERS */
	|| cover(side, x, y) > 0)
      return unit_at(x, y);
    return NULL;
}

int
utype_seen_at(Side *side, int x, int y)
{
    Unit *unit;
#if 0
    UnitView *uview;
#endif

    if (!in_area(x, y))
      return NONUTYPE;
    unit = unit_seen_at(side, x, y);
    if (unit)
      return unit->type;

#if 0 /* for now */
    uview = unit_view_at(side, x, y);
    if (uview != NULL)
      return uview->type;
#endif

    return NONUTYPE;
}

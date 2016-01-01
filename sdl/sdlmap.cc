/* Map display code for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file is all about drawing a single map.  Most player displays
   will have two maps to a screen, one full size map and one mini-map
   displaying the whole world.  The code here can render both of these
   types. */

#include "sdlpreconq.h"
#include "conq.h"
#include "sdlconq.h"
extern void hex_face_rect(int dir, int pow, int *sxp, int *syp, int *swp, int *shp);
extern void get_grid_surface(int pow);
extern void get_country_boundary_surface(int pow);

extern ImageFamily *hexagon_w1;
extern ImageFamily *hexagon_w2;

static void draw_map_overhead(Map *map);
static void draw_map_isometric(Map *map);
static void draw_terrain_iso(Map *map, int x, int y);
#if 0
static int shading(int dir);
static void draw_cliff(Map *map, int sx, int sy, int dir, int drop,
		       int face);

static void draw_unseen_fuzz(Map *map, int vx, int vyhi, int vylo);
#endif
static void draw_row(Map *map, int x0, int y0, int len);
static void draw_current(Map *map);
#if 0
static void draw_blast_image(Map *map, int sx, int sy, int sw, int sh,
			     int blasttype);
#endif
static void draw_unit_image(ImageFamily *imf, Map *map, int sx, int sy, int sw, int sh,
			    int s2, int mod);
static void draw_side_emblem(Map *map, int ex, int ey, int ew, int eh,
			     int s2);

extern void xform(Map *map, int x, int y, int *sxp, int *syp);
static void xform_fractional(Map *map, int x, int y, int xf, int yf,
			     int *sxp, int *syp);
static void xform_top(Map *map, int x, int y, int *sxp, int *syp);
static void x_xform_unit(Map *map, Unit *unit, int *sxp, int *syp,
			 int *swp, int *shp);
static int x_nearest_cell(Map *map, int sx, int sy, int *xp, int *yp);
static Unit *x_find_unit_or_occ(Map *map, Unit *unit, int usx, int usy,
				int usw, int ush, int sx, int sy);
static Unit *x_find_unit_at(Map *map, int x, int y, int sx, int sy);
static int x_nearest_unit(Map *map, int sx, int sy, Unit **unitp);

static void draw_feature_name(Map *map, int f);
#if 0
static void draw_ai_region(Map *map, int x, int y);
static void draw_resource_usage(Map *map, int x, int y);
static void draw_hex_polygon(Map *map, GC gc, int sx, int sy,
			     int power, int over, int dogrid);
static void draw_area_background(Map *map);
static void meridian_line_callback(int x1, int y1, int x1f, int y1f,
				   int x2, int y2, int x2f, int y2f);
static void meridian_text_callback(int x1, int y1, int x1f, int y1f,
				   char *str);
#endif
static int cell_drawing_info(Map *map, int xw, int y, int *colorp, int *overp);
static void draw_terrain_row(Map *map, int x0, int y0, int len);
static void draw_terrain_transitions(Map *map, int x0, int y0, int len);
static void draw_terrain_grid(Map *map, int x0, int y0, int len);
#if 0
static void draw_contours(Map *map, int x0, int y0, int len);
#endif
static void draw_clouds_row(Map *map, int x0, int y0, int len);
static void draw_temperature_row(Map *map, int x0, int y0, int len);
static void draw_winds_row(Map *map, int x0, int y0, int len);
static void draw_uviews_in_cell(Map *map, int x, int y);
static void draw_unit_view_and_occs(Map *map, UnitView *uview, int sx, int sy, int sw, int sh);
static void draw_dots(Map *map, int sx, int sy, int sw, int sh);
static void draw_unit_name(Map *map, const char *name, int sx, int sy,
			   int sw, int sh);
static void draw_unit_size(Map *map, int size, int sx, int sy,
			   int sw, int sh);
static void draw_people(Map *map, int x, int y);
static void draw_borders(Map *map, int vx, int vyhi, int vylo, int b);
static void draw_borders_iso(Map *map, int x, int y, int b);
static void draw_connections(Map *map, int vx, int vyhi, int vylo, int c);
static void draw_connections_iso(Map *map, int x, int y, int c);
static void draw_country_boundary_line(Map *map, int sx, int sy, int dir,
				     int con, int heavy);
static void draw_feature_boundary(Map *map, int x, int y);
#if 0
static void draw_meridians(Map *map);
#endif
static void draw_map_outline(Map *worldmap, Map *map);

Map *
create_map(int is_world)
{
    Map *map;

    map = (Map *) xmalloc(sizeof(Map));
    /* Set up the main viewport. */
    map->main_vp = new_vp();
    map->main_vp->draw_terrain = TRUE;
    map->main_vp->draw_grid = TRUE;
    map->main_vp->draw_names = TRUE;
    map->main_vp->draw_feature_names = TRUE;
    map->main_vp->draw_people = TRUE;
    map->main_vp->draw_control = TRUE;
    /* Allocate the working viewport.  This one is a clone of the
       main viewport used when redrawing subareas. */
    map->vp = new_vp();
    map->is_world = is_world;
    return map;
}

void
draw_map(Map *map)
{
  if (map->vp->isometric)
    draw_map_isometric(map);
  else
    draw_map_overhead(map);
  if (map->is_world)
    draw_map_outline(map, map->screen->map);
}

/* Transform map coordinates into screen coordinates. */

void
xform(Map *map, int x, int y, int *sxp, int *syp)
{
    xform_cell(map->main_vp, x, y, sxp, syp);
    *sxp += map->sx;  
    *syp += map->sy;
}

/* Transform, but also use position within cell. */

static void
xform_fractional(Map *map, int x, int y, int xf, int yf, int *sxp, int *syp)
{
    xform_cell_fractional(map->main_vp, x, y, xf, yf, sxp, syp);
    *sxp += map->sx;  
    *syp += map->sy;
}

static void
xform_top(Map *map, int x, int y, int *sxp, int *syp)
{
    xform_cell_top(map->main_vp, x, y, sxp, syp);
    *sxp += map->sx;  
    *syp += map->sy;
}

static void
x_xform_unit(Map *map, Unit *unit, int *sxp, int *syp, int *swp, int *shp)
{
    xform_unit(map->main_vp, unit, sxp, syp, swp, shp);
    *sxp += map->sx;  
    *syp += map->sy;
}

static int
x_nearest_cell(Map *map, int sx, int sy, int *xp, int *yp)
{
    return nearest_cell(map->main_vp, sx, sy, xp, yp, NULL, NULL);
}

static Unit *
x_find_unit_or_occ(Map *map, Unit *unit, int usx, int usy, int usw, int ush,
		   int sx, int sy)
{
    int usx1, usy1, usw1, ush1;
    Unit *occ, *rslt;

    /* See if the point might be over an occupant. */
    if (unit->occupant != NULL
	&& (side_controls_unit(dside, unit)
	    || map->main_vp->show_all
	    || u_see_occupants(unit->type)
	    || side_owns_occupant_of_unit(dside, unit))) {
	for_all_occupants(unit, occ) {
	    x_xform_unit(map, occ, &usx1, &usy1, &usw1, &ush1);
	    rslt =
	      x_find_unit_or_occ(map, occ, usx1, usy1, usw1, ush1, sx, sy);
	    if (rslt) {
		return rslt;
	    }
	}
    }
    /* Otherwise see if it could be the unit itself.  This has the effect of
       "giving" the transport everything in its box that is not in an occ. */
    x_xform_unit(map, unit, &usx1, &usy1, &usw1, &ush1);
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1)) {
	return unit;
    }
    return NULL;
}

static Unit *
x_find_unit_at(Map *map, int x, int y, int sx, int sy)
{
    int usx, usy, usw, ush;
    Unit *unit, *rslt;
	
    for_all_stack(x, y, unit) {
	x_xform_unit(map, unit, &usx, &usy, &usw, &ush);
	rslt = x_find_unit_or_occ(map, unit, usx, usy, usw, ush, sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

static int
x_nearest_unit(Map *map, int sx, int sy, Unit **unitp)
{
    int x, y;
    UnitView *uview = NULL;

#if (0)
    if (!x_nearest_cell(map, sx, sy, &x, &y)) {
	*unitp = NULL;
    } else if (map->main_vp->uw >= 32) {
	*unitp = x_find_unit_at(map, x, y, sx, sy);
    } else {
	*unitp = unit_at(x, y);
    }
    DGprintf("Pixel %d,%d -> unit %s\n", sx, sy, unit_desig(*unitp));
#endif
    nearest_unit_view(dside, map->vp, sx, sy, &uview);
    if (uview)
      *unitp = view_unit(uview);
    return TRUE;
}

static void
draw_map_overhead(Map *map)
{
    int vylo, vyhi, y, x1, vx, len, t;

    if (map->vp->vcx < 0 || map->vp->vcy < 0) {
    	run_warning("doing a nasty hack");
	map->vp->vcx = map->vp->vcy = 2;
    }

    /* This is equivalent to the y calc in nearest_cell. */
    vylo = ((map->vp->totsh - (map->vp->sy + map->vp->pxh)) / map->vp->hch);
    /* Adjust downwards by a row, to cover overlap. */
    vylo -= 1;
    /* Now adjust the bottom row so it doesn't go outside the area. */
    vylo = limitn(0, vylo, area.height - 1);
    vyhi = ((map->vp->totsh - (map->vp->sy + 0)) / map->vp->hch);
    vyhi = limitn(0, vyhi, area.height - 1);
    /* Compute the leftmost "column" (but add back 1 to vylo first to fix the
    half-drawn cell scrolling bug). */
    vx = (map->vp->sx - ((vylo + 1) * map->vp->hw) / 2) / map->vp->hw;
    if (map->vp->draw_terrain) {
	for (y = vyhi; y >= vylo; --y) {
	    if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
	      continue;
#if 0
	    if (1 /*DebugG*/) {
		int tmpsx1, tmpsy1, tmpsx2, tmpsy2;
		SDL_Rect rect;

		xform(map, x1, y, &tmpsx1, &tmpsy1);
		xform(map, x1 + len - 1, y, &tmpsx2, &tmpsy2);
		tmpsx1 += 0;            tmpsy1 += map->vp->hh / 2;
		tmpsx2 += map->vp->hw;  tmpsy2 += map->vp->hh / 2;
#if 0
		if (mapw->rsw >= 0) {
		    tmpsx1 += mapw->rsx;  tmpsy1 += mapw->rsy;
		    tmpsx2 += mapw->rsx;  tmpsy2 += mapw->rsy;
		}
#endif
		rect.x = tmpsx1;  rect.y = tmpsy1;
		rect.w = tmpsx2 - tmpsx1;  rect.h = 2;
		SDL_FillRect(map->screen->surf, &rect,
			     SDL_MapRGB(map->screen->surf->format, 255, 0, 0));
	    }
#endif
	    draw_terrain_row(map, x1, y, len);
	    if (between(4, map->vp->power, 6) && 1/*mapw->draw_transitions*/) {
		draw_terrain_transitions(map, x1, y, len);
	    }
	}
	/* Draw the grid as an overlay.  Note that we want to draw
	   before linear terrains, so they don't get obscured by the
	   grid. */
	if (map->vp->draw_grid
	    && between(4, map->vp->power, 6)) {
	    for (y = vyhi; y >= vylo; --y) {
		if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
		  continue;
		draw_terrain_grid(map, x1, y, len);
	    }
	}
	if (any_aux_terrain_defined()) {
	    for_all_terrain_types(t) {
		if (t_is_border(t)
		    && aux_terrain_defined(t)
		    && bwid[map->vp->power] > 0) {
		    draw_borders(map, vx, vyhi, vylo, t);
		} else if (t_is_connection(t)
			   && aux_terrain_defined(t)
			   && cwid[map->vp->power] > 0) {
		    draw_connections(map, vx, vyhi, vylo, t);
		}
	    }
	}
#if 0
	/* The relative ordering of these is quite important.  Note that
	   each should be prepared to run independently also, since the
	   other displays might have been turned off. */
	if (elevations_defined()
	    && map->vp->draw_elevations
	    && map->vp->angle == 90) {
	    for (y = vyhi; y >= vylo; --y) {
		if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
		  continue;
		draw_contours(mapw, x1, y, len);
	    }
	}
	/* Fuzz out the edges of the known area. */
	if (!map->vp->show_all
	    && !g_terrain_seen()  /* (should look at count of unseen cells) */
	    && between(4, map->vp->power, 6))
	  draw_unseen_fuzz(mapw, vx, vyhi, vylo);
#endif
    }
#if 0
    /* Now draw the lat-long grid if asked to do so. */
    if (map->vp->draw_meridians && map->vp->meridian_interval > 0)
      draw_meridians(mapw);
#endif
    for (y = vyhi; y >= vylo; --y) {
	if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
	  continue;
	draw_row(map, x1, y, len);
    }
    if (map->screen->curunit && map->screen->curunit->id == map->screen->curunit_id)
      draw_current(map);
}

static void
draw_map_isometric(Map *map)
{
    int x0, y0, x, y, d1, d2, dx, dy, left, ndir;
    int xbegin, ybegin, xrow, yrow, xrow2, yrow2;
    int j, k, numrows, rowlen;

    /* Compute the horizontal offsets of each row.  For hexagons, the
       displacement is equivalent up and right, followed by down and
       right, which is the same as right_dir + right_dir(right_dir). */
    d1 = right_dir(map->vp->isodir);
    d2 = right_dir(d1);
    dx = dirx[d1] + dirx[d2];  dy = diry[d1] + diry[d2];
    /* Derive the number of rows and row length from the size of the
       map window, adding a little padding.  Each row consists of
       spaced hexes and is half of a hex width, so we need 4 times as
       many to cover the viewport. */
    /* The additional rows and length were heuristically determined by
       increasing until scrolling around never left any black holes.
       The numbers ought to be better explained though. */
    numrows = ((map->vp->pxh / map->vp->hw) * 4) + 8;
    rowlen = (map->vp->pxw / map->vp->hch) + 2;
    /* Get the cell at the top left corner (note that it may be
       outside the area). */
    nearest_cell(map->vp, 0, 0, &x0, &y0, NULL, NULL);
    /* Go up by one row. */
    xbegin = x0 + dirx[map->vp->isodir];
    ybegin = y0 + diry[map->vp->isodir];
    switch (map->vp->isodir) {
    	case	SOUTHEAST:
    	case	NORTHWEST:
		/* Shift the row to the left and increase its length
		by 1 cell to avoid black holes when scrolling. */ 
    		xbegin += dirx[left_dir(map->vp->isodir)];
    		ybegin += diry[left_dir(map->vp->isodir)];
    		rowlen += 1;
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
		draw_terrain_iso(map, x, y);
		draw_row(map, x, y, 1);
	    }
	    x += dx;  
	    y += dy;
	}
	/* Alternately go either down and right, or down and left. */
	if (left) {
	    ndir = right_dir(opposite_dir(map->vp->isodir));
	} else {
	    ndir = left_dir(opposite_dir(map->vp->isodir));
	}
	left = !left;
	xrow2 = xrow + dirx[ndir];
	yrow2 = yrow + diry[ndir];
	xrow = xrow2;  
	yrow = yrow2;
    }
}

static void
draw_terrain_iso(Map *map, int x, int y)
{
    int t, sx, sy /*, elev, elev1, drop, x1, x1w, y1, t1, dir, dir2*/;
#if 0
    int sx1, sy1, sw1, sh1;
    int pow = map->vp->power;
    int viewdir = map->vp->isodir;
#endif
    int hw = map->vp->hw, hh = map->vp->hh;
    int drawsolid, over;
    Image *timg, *subimg;
#if 0
    enum grayshade shade;
#endif
    int color/*, color2*/;
    enum whattouse drawmeth;
    SDL_Rect rect;

    drawmeth = (enum whattouse)cell_drawing_info(map, x, y, &color, &over);
    if (drawmeth == dontdraw)
      return;
    t = terrain_at(x, y);
    timg = best_image(ui->timages[t], hh, hw - hw / 4);
    xform(map, x, y, &sx, &sy);
    drawsolid = FALSE;
    if (1/*map->draw_terrain_images*/) {
	int quasirand = abs((x * x + y * y ) % 101);

	subimg = timg;
	if (timg->numsubimages > 0 && timg->subimages)
	  subimg = timg->subimages[quasirand % timg->numsubimages];
	rect.x = sx;  rect.y = sy - hw / 4;
	rect.w = hh;  rect.h = hw;
	if (sdl_image(subimg))
	  SDL_BlitSurface(sdl_image(subimg), NULL, map->screen->surf, &rect);
	else
	  drawsolid = TRUE;
    } else {
	drawsolid = TRUE;
    }
#if 0
    /* Handle the case of a solid-color terrain. */
    if (drawsolid) {
	XSetForeground(dpy, gc, color->pixel);
	XSetClipMask(dpy, gc, ui->hexisopics[pow]);
	/* Draw the cell proper. */
	sx1 = sx;  sy1 = sy;
	sw1 = hh;  sh1 = hw / 2;
	XFillRectangle(dpy, map->d, gc, sx1, sy1, sw1, sh1);
    }
#endif
#if 0
    /* Apply any overlay shading. */
    if (over < 0) {
	shade = GRAY(over);
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, ui->grays[shade]);
	if (color2 == NULL)
	  color2 = ui->blackcolor;
	XSetForeground(dpy, gc, color2->pixel);
	XFillRectangle(dpy, map->d, gc, sx1, sy1, sw1, sh1);
    }
#endif
#if 0
    /* Draw cliffs indicating elevation differences. */
    if (elevations_defined()) {
	elev = (elevations_defined() ? elev_at(x, y) : 0);
	XSetForeground(dpy, gc, color->pixel);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	for_all_directions(dir) {
	    dir2 = dir_subtract(dir, viewdir);
	    if (point_in_dir(xw, y, dir, &x1, &y1)) {
		t1 = terrain_at(x1, y1);
		elev1 = (elevations_defined() ? elev_at(x1, y1) : 0);
	    } else {
		elev1 = area.minelev;
	    }
	    /* Compute the number of pixels between cell elevs. */
	    drop = (elev - elev1);
	    drop *= map->vp->vertscale;
	    drop = (drop * hh) / map->vp->cellwidth;
	    if (drop > 0) {
		/* Note that we're shading independent of view dir. */
		XSetForeground(dpy, gc,
			       (ui->cell_shades[shading(dir)])[t]->pixel);
		draw_cliff(map, sx, sy, dir2, drop,
			   dir2 == WEST
			   || dir2 == SOUTHWEST
			   || dir2 == SOUTHEAST);
	    } else if (drop == 0 && map->vp->draw_grid && map->vp->hh > 10) {
		dir2 = dir_subtract(dir, viewdir);
		XSetForeground(dpy, gc, ui->blackcolor->pixel);
		XDrawLine(dpy, map->d, gc,
			  sx + ibsx[pow][dir2], sy + ibsy[pow][dir2],
			  sx + ibsx[pow][dir2+1], sy + ibsy[pow][dir2+1]);
	    }
	}
    } else if (map->vp->draw_grid && map->vp->hh > 10) {
	xform_top(map, x, y, &sx, &sy);
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	XSetForeground(dpy, gc, ui->blackcolor->pixel);
	for_all_directions(dir) {
	    dir2 = dir_subtract(dir, viewdir);
	    XDrawLine(dpy, map->d, gc,
		      sx + ibsx[pow][dir2], sy + ibsy[pow][dir2],
		      sx + ibsx[pow][dir2+1], sy + ibsy[pow][dir2+1]);
	}
    }
#endif
    /* Restore the fill style after tinkering with it for terrain. */
    if (any_aux_terrain_defined()) {
	for_all_terrain_types(t) {
	    if (t_is_border(t)
		&& aux_terrain_defined(t)
		&& bwid[map->vp->power] > 0) {
		draw_borders_iso(map, x, y, t);
	    } else if (t_is_connection(t)
		       && aux_terrain_defined(t)
		       && cwid[map->vp->power] > 0) {
		draw_connections_iso(map, x, y, t);
	    }
	}
    }
}

static int
cell_drawing_info(Map *map, int x, int y, int *colorp, int *overp)
{
    int t /*, t2*/;
    enum whattouse rslt;

    if (m_terrain_visible(map->vp, x, y) || is_designer(dside)) {
	t = terrain_at(x, y);
	*colorp = t;
	if (map->vp->draw_cover
	    && !map->vp->show_all
	    && cover(dside, x, y) == 0)
	  *overp = -2;
	else if (map->vp->draw_lighting && night_at(x, y))
	  *overp = -1;
	else
	  *overp = 0;
	/* Designing overrides any optional overlay choice. */
	if (is_designer(dside)) {
	    if (!m_terrain_visible(map->vp, x, y))
	      *overp = -2;
	    else
	      *overp = 0;
	}
#if 0
	/* Coatings override everything. */
	if ((t2 = any_coating_at(x, y)) != NONTTYPE) {
	    *overp = -2;
	    *color2p = ui->cell_color[t2];
	    if (*color2p == NULL)
	      *color2p = ui->whitecolor;
	}
#endif
	rslt = (map->vp->power >= 3 ? usepictures : useblocks);
    } else {
	*colorp = -1;
	*overp = 0;
	rslt = dontdraw;
    }
    return rslt;
}

static void
draw_terrain_row(Map *map, int x0, int y0, int len)
{
    int x1, x, t, sx, sy, i = 0, j;
    int w = map->vp->hw, h = map->vp->hh;
    int color, segcolor;
    int over, segover;
    enum whattouse drawmethod, segdrawmethod;
    Image *timg, *subimg;
#if 0
    enum grayshade shade;
#endif
    SDL_Rect rect;

    x1 = x0;
    segdrawmethod = (enum whattouse)cell_drawing_info(map, x0, y0, &segcolor, &segover);
    for (x = x0; x < x0 + len + 1; ++x) {
	t = terrain_at(x, y0);
	drawmethod = (enum whattouse)cell_drawing_info(map, x, y0, &color, &over);
	/* Decide if the run is over and we need to dump some output. */
	if (x == x0 + len
	    || x == area.width
	    || drawmethod != segdrawmethod
	    || color != segcolor
	    || over != segover) {
	    /* Note: we might end up drawing something that matches
	       the background color, which wastes time, but apparently
	       the test "(segdrawmethod != dontdraw && segcolor !=
	       ui->bgcolor)" is not completely sufficient.
	       (should figure this one out sometime) */
	    t = terrain_at(x1, y0);
	    timg = best_image(ui->timages[t], w, h);
	    xform(map, x1, y0, &sx, &sy);
	    switch (segdrawmethod) {
	      case dontdraw:
		/* Don't do anything. */
		break;
	      case useblocks:
		timg = ui->tcolors[t];
		if (timg) {
		  rect.x = sx;  rect.y = sy;
		  rect.w = i * w;  rect.h = h;
		  SDL_FillRect(map->screen->surf, &rect,
			       SDL_MapRGB(map->screen->surf->format,
					  (timg->r >> 8),
					  (timg->g >> 8),
					  (timg->b >> 8)));
		}
#if 0
		if (segover < 0) {
		    shade = GRAY(segover);
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, ui->grays[shade]);
		    if (segcolor2 != NULL)
		      XSetForeground(dpy, gc, segcolor2->pixel);
		    else
		      XSetForeground(dpy, gc, ui->blackcolor->pixel);
		    XFillRectangle(dpy, map->d, gc, sx, sy, i * w, h);
		}
#endif
		break;
	      case usepictures:
		for (j = 0; j < i; ++j) {
		    int quasirand = abs(((x1 + j) * (x1 + j) + y0 * y0 ) % 101);

		    xform(map, x1 + j, y0, &sx, &sy);
		    if (1 /*map->draw_terrain_images*/) {
		      subimg = timg;
		      if (timg->numsubimages > 0 && timg->subimages)
			subimg =
			  timg->subimages[quasirand % timg->numsubimages];
		      rect.x = sx;  rect.y = sy;
		      rect.w = w;  rect.h = h;
		      if (sdl_image(subimg))
			SDL_BlitSurface(sdl_image(subimg), NULL, map->screen->surf,
					&rect);
		    } else {
		      /* should draw a hexagon */
		      if (timg) {
			rect.x = sx;  rect.y = sy;
			rect.w = w;  rect.h = h;
			SDL_FillRect(map->screen->surf, &rect,
				     SDL_MapRGB(map->screen->surf->format,
						(timg->r >> 8),
						(timg->g >> 8),
						(timg->b >> 8)));
		      }
		    }
		}
#if 0
		if (segover < 0) {
		    shade = GRAY(segover);
		    XSetFillStyle(dpy, gc, FillStippled);
		    XSetStipple(dpy, gc, ui->grays[shade]);
		    if (segcolor2 != NULL)
		      XSetForeground(dpy, gc, segcolor2->pixel);
		    else
		      XSetForeground(dpy, gc, ui->blackcolor->pixel);
		    for (j = 0; j < i; ++j) {
			xform(map, x1 + j, y0, &sx, &sy);
			XSetClipOrigin(dpy, gc, sx, sy);
			XFillRectangle(dpy, map->d, gc, sx, sy, w, h);
		    }
		}
#endif
		break;
	    }
	    /* Reset everything for the next run. */
	    i = 0;
	    x1 = x;
	    segdrawmethod = drawmethod;
	    segcolor = color;
	    segover = over;
	}
	++i;
    }
}

/* There are two ways to draw borders; as images taken from a special
   border type image that includes 16 subimages for different combinations,
   or as lines colored according to the border's terrain type. */

static void
draw_borders(Map *map, int vx, int vyhi, int vylo, int b)
{
    int x, y, x1, len, bitmask, sx, sy, x3, y3, subi;
    int halfside;
    Image *timg, *subimg;
    int power = map->vp->power;
    SDL_Rect rect;

    timg = (ui->best_timages[power])[b];
    if (1 /*!map->draw_lines*/
	&& between(4, power, 6)
	&& timg->isborder
	&& timg->subimages != NULL) {
	/* We want to draw individual images for border segments. */
	halfside = halfsides[power];
	for (y = vyhi; y >= vylo; --y) {
	    if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
	      continue;
	    for (x = x1; x < x1 + len; ++x) {
		/* Draw the junction at the top of the x,y hex. */
		bitmask = 0;
		if (point_in_dir(x, y, NORTHEAST, &x3, &y3)
		    && border_at(x3, y3, WEST, b)
		    && m_seen_border(map->vp, x3, y3, WEST))
		  bitmask |= 1;
		if (border_at(x, y, NORTHEAST, b)
		    && m_seen_border(map->vp, x, y, NORTHEAST))
		  bitmask |= 2;
		if (border_at(x, y, NORTHWEST, b)
		    && m_seen_border(map->vp, x, y, NORTHWEST))
		  bitmask |= 4;
		if (bitmask != 0) {
		    xform(map, x, y, &sx, &sy);
		    subi = (bitmask & 3) + ((bitmask & 4) ? 12 : 4);
		    subimg = timg->subimages[subi];
		    rect.x = sx;  rect.y = sy - halfside;
		    rect.w = map->vp->hw;  rect.h = map->vp->hch;
		    if (sdl_image(subimg))
		      SDL_BlitSurface(sdl_image(subimg), NULL, map->screen->surf,
				      &rect);
#if 0
		    draw_rect(map->screen->surf, rect.x, rect.y, rect.w, rect.h,
			      random_color(map->screen->surf));
#endif
		}
		/* Draw the junction at the top left corner of the x,y hex. */
		bitmask = 0;
		if (border_at(x, y, WEST, b)
		    && m_seen_border(map->vp, x, y, WEST))
		  bitmask |= 1;
		if (border_at(x, y, NORTHWEST, b)
		    && m_seen_border(map->vp, x, y, NORTHWEST))
		  bitmask |= 2;
		if (point_in_dir(x, y, WEST, &x3, &y3)
		    && border_at(x3, y3, NORTHEAST, b)
		    && m_seen_border(map->vp, x3, y3, NORTHEAST))
		  bitmask |= 4;
		if (bitmask != 0) {
		    xform(map, x, y, &sx, &sy);
		    subi = (bitmask & 3) + ((bitmask & 4) ? 8 : 0);
		    subimg = timg->subimages[subi];
		    rect.x = sx - map->vp->hw / 2;  rect.y = sy - halfside;
		    rect.w = map->vp->hw;  rect.h = map->vp->hch;
		    if (sdl_image(subimg))
		      SDL_BlitSurface(sdl_image(subimg), NULL, map->screen->surf,
				      &rect);
#if 0
		    draw_rect(map->screen->surf, rect.x, rect.y, rect.w, rect.h,
			      random_color(map->screen->surf));
#endif
		}
	    }
	}
    }
}

static void
draw_borders_iso(Map *map, int x, int y, int b)
{
    int bitmask, dir, dir2, sx, sy;
    int pow = map->vp->power;
#if 0
    int wid = bwid[pow], wid2;
    SDL_Rect rect;
#endif
    Image *timg /*, *subimg*/;

    if (!m_terrain_visible(map->vp, x, y)
	|| !tt_drawable(b, terrain_at(x, y)))
      return;
    /* Make a bitmask of all the dirs that have a connection. */
    bitmask = 0;
    for_all_directions(dir) {
	if (border_at(x, y, dir, b)) {
	    /* Draw only borders "in front". */
	    dir2 = dir_subtract(dir, map->vp->isodir);
	    if (dir2 == WEST || dir2 == SOUTHWEST || dir2 == SOUTHEAST)
	      bitmask |= (1 << dir);
	}
    }
    if (bitmask != 0) {
	timg = (ui->best_timages[pow])[b];
	xform(map, x, y, &sx, &sy);
#if 0
	/* Choose whether to draw images or lines.  Unlike in border
	   drawing, the two algorithms are so similar it's not worth
	   writing a different loop for each case. */
	if (0 /*!map->draw_lines
	    && between(4, pow, 6)
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
			XCopyArea(dpy, tkimg->mask, map->d, gc,
				  0, 0, map->vp->hw, map->vp->hh,
				  sx, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		XCopyArea(dpy, tkimg->colr, map->d, gc,
			  0, 0, map->vp->hw, map->vp->hh, sx, sy);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    }
	} else {
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    int adj;

		    dir2 = dir_subtract(dir, map->vp->isodir);
		    if (dir2 == WEST || dir2 == SOUTHWEST || dir2 == SOUTHEAST)
		      adj = -2;
		    else
		      adj = +2;
		    XDrawLine(dpy, map->d, gc,
			      sx + ibsx[pow][dir2] + adj, sy + ibsy[pow][dir2] + adj,
			      sx + ibsx[pow][dir2+1] + adj, sy + ibsy[pow][dir2+1] + adj);
		}
	    }
	}
#endif
    }
}

/* Draw all the connections of the given type that are visible. */

static void
draw_connections(Map *map, int vx, int vyhi, int vylo, int c)
{
    int x, y, x1, len, dir, bitmask, sx, sy;
    int power = map->vp->power;
#if 0
    int wid = cwid[power], wid2, cx = hws[power] / 2, cy = hhs[power] / 2;
#endif
    Image *timg, *subimg;
    SDL_Rect rect;

    timg = (ui->best_timages[power])[c];
    for (y = vyhi; y >= vylo; --y) {
	if (!compute_x1_len(map->vp, vx, vylo, y, &x1, &len))
	  continue;
	for (x = x1; x < x1 + len; ++x) {
	    if (!m_terrain_visible(map->vp, x, y)
		|| !tt_drawable(c, terrain_at(x, y)))
	      continue;
	    /* Make a bitmask of all the dirs that have a connection. */
	    bitmask = 0;
	    for_all_directions(dir) {
		if (connection_at(x, y, dir, c))
		  bitmask |= (1 << dir);
	    }
	    if (bitmask != 0) {
		xform(map, x, y, &sx, &sy);
		/* Choose whether to draw images or lines.  Unlike in
		   border drawing, the two algorithms are so similar it's
		   not worth writing a different loop for each case. */
		if (1 /*!map->draw_lines*/
		    && between(4, power, 6)
		    && timg->isconnection
		    && timg->subimages != NULL) {
		    subimg = timg->subimages[bitmask];
		    rect.x = sx;  rect.y = sy;
		    rect.w = map->vp->hw;  rect.h = map->vp->hh;
		    if (sdl_image(subimg))
		      SDL_BlitSurface(sdl_image(subimg), NULL, map->screen->surf,
				      &rect);
		} else {
#if 0
		    for_all_directions(dir) {
			if (bitmask & (1 << dir)) {
			    XDrawLine(dpy, map->d, gc,
				      sx + cx - wid2, sy + cy - wid2,
				      sx + cx + lsx[power][dir] - wid2,
				      sy + cy + lsy[power][dir] - wid2);
			}
		    }
#endif
		}
	    }
	}
    }
}

static void
draw_connections_iso(Map *map, int x, int y, int c)
{
    int bitmask /*, bitmask2*/, dir /*, dir2, sx, sy*/;
    int hw = map->vp->hw, hh = map->vp->hh;
#if 0
    int power = map->vp->power;
    int wid = cwid[power], wid2, cx = hhs[power] / 2, cy = hws[power] / 4;
    SDL_Rect rect;
#endif
    Image *timg /*, *subimg*/;

    if (!m_terrain_visible(map->vp, x, y)
	|| !tt_drawable(c, terrain_at(x, y)))
      return;
    /* Make a bitmask of all the dirs that have a connection. */
    bitmask = 0;
    for_all_directions(dir) {
	if (connection_at(x, y, dir, c))
	  bitmask |= (1 << dir);
    }
    if (bitmask != 0) {
	timg = best_image(ui->timages[c], hh, hw - hw / 4);
#if 0
	if (map->draw_lines
	    || !between(4, power, 6)
	    || !timg->isconnection
	    || timg->subimages == NULL) {
	    wid2 = wid / 2;
	    XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	    color = ui->cell_color[c];
	    if (color < 0)
	      color = ui->blackcolor;
	    XSetForeground(dpy, gc, color->pixel);
	    XSetBackground(dpy, gc, ui->whitecolor->pixel);
	    set_terrain_gc_for_image(map, gc, timg);
	}
	xform(map, x, y, &sx, &sy);
	/* Choose whether to draw images or lines.  Unlike in border
	   drawing, the two algorithms are so similar it's not worth
	   writing a different loop for each case. */
	if (!map->draw_lines
	    && between(4, power, 6)
	    && timg->isconnection
	    && timg->subimages != NULL) {
	    bitmask2 = 0;
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    dir2 = dir - map->vp->isodir;
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
			XCopyArea(dpy, tkimg->mask, map->d, gc,
				  0, 0, map->vp->hw, map->vp->hh,
				  sx, sy);
		    }
		    XSetFunction(dpy, gc, GXor);
		}
		XCopyArea(dpy, tkimg->colr, map->d, gc,
			  0, 0, map->vp->hw, map->vp->hh, sx, sy - hw / 4);
		if (!use_clip_mask)
		  XSetFunction(dpy, gc, GXcopy);
	    }
	} else {
	    for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
		    dir2 = dir - map->vp->isodir;
		    if (dir2 < 0)
		      dir2 += NUMDIRS;
		    XDrawLine(dpy, map->d, gc,
			      sx + cx - wid2, sy + cy - wid2,
			      sx + cx + ilsx[power][dir2] - wid2,
			      sy + cy + ilsy[power][dir2] - wid2);
		}
	    }
	}
#endif
    }
}

static void
draw_terrain_transitions(Map *map, int x0, int y0, int len)
{
    int x1, y1, x, t, dir, sx, sy;
    int sx2, sy2, sw, sh, offset, t1, rslt, quasirand;
    int color;
    int mainover /*, adjover*/;
    enum whattouse drawmethod;
    Image *trimg, *subtrimg;
    Image *timg, *subimg;
#if 0
    enum grayshade shade;
#endif
    int fillsolid = FALSE;

    trimg = best_image(generic_transition, map->vp->hw, map->vp->hh);
    if (trimg == NULL)
      return;
    for (x = x0; x < x0 + len + 1; ++x) {
	t = terrain_at(x, y0);
	drawmethod = (enum whattouse)cell_drawing_info(map, x, y0, &color, &mainover);
	if (x == x0 + len
	    || x == area.width
	    || drawmethod == dontdraw)
	  continue;
	for_all_directions(dir) {
	    rslt = compute_transition(dside, map->vp, x, y0, dir,
				      &sx2, &sy2, &sw, &sh, &offset);
	    if (!rslt)
	      continue;
#if 0 /* for debugging */
	    draw_rect(map->screen->surf, sx2, sy2, sw, sh);
#endif
	    quasirand = abs((x * x + y0 * y0 ) % 101);
	    subtrimg = trimg->subimages[(quasirand % 4) * 4 + offset];

	    xform(map, x, y0, &sx, &sy);
	    /* We already know the result, just need x1,y1 */
	    point_in_dir(x, y0, dir, &x1, &y1);
	    t1 = terrain_at(x1, y1);
	    timg = best_image(ui->timages[t1], map->vp->hw, map->vp->hh);
	    if (1/*map->draw_terrain_images*/) {
		subimg = timg;
		if (timg->numsubimages > 0 && timg->subimages)
		  subimg = timg->subimages[quasirand % timg->numsubimages];
#if 0
		tkimg = (TkImage *) subimg->hook;
		if (subimg
		    && tkimg
		    && tkimg->colr != None
		    && !timg->istile) {
		    XCopyArea(dpy, tkimg->colr, map->d, gc,
			      sx2 - sx, sy2 - sy, sw, sh, sx2, sy2);
		} else {
		    fillsolid = TRUE;
		}
#endif
	    } else {
		fillsolid = TRUE;
	    }
	    if (fillsolid) {
#if 0
		XSetFillStyle(dpy, gc, FillSolid);
		cell_drawing_info(map, x1, y1, &pat, &color, &adjover,
				  &color2);
		XSetForeground(dpy, gc, color->pixel);
		XFillRectangle(dpy, map->d, gc, sx2, sy2, sw, sh);
#endif
	    }
	}
#if 0
	if (mainover < 0) {
	    /* (assumes use_clip_mask) */
	    shade = GRAY(mainover);
	    XSetFillStyle(dpy, gc, FillStippled);
	    XSetStipple(dpy, gc, ui->grays[shade]);
	    if (color2 == NULL)
	      color2 = ui->blackcolor;
	    XSetForeground(dpy, gc, color2->pixel);
	    XFillRectangle(dpy, map->d, gc, sx2, sy2, sw, sh);
	}
#endif
    }
}

/* Draw a single row of explicit grid lines. */

static void
draw_terrain_grid(Map *map, int x0, int y0, int len)
{
    int x1, y1, x, dir;
    int sx, sy, sw, sh, rsx, rsy;
    SDL_Rect rect, rect2;
    SDL_Surface *grid;

    for (x = x0; x < x0 + len + 1; ++x) {
	if (x == x0 + len || x == area.width)
	  continue;
	if (!terrain_visible(dside, x, y0))
	  continue;
	for_all_directions(dir) {
	    /* Skip 1/2 of the directions. */
	    if (dir == NORTHEAST || dir == EAST || dir == SOUTHEAST)
	      continue;
	    if (!point_in_dir(x, y0, dir, &x1, &y1))
	      continue;
	    if (!terrain_visible(dside, x1, y1))
	      continue;
	    if (ui->grids[map->vp->power] == NULL)
	      get_grid_surface(map->vp->power);
	    grid = ui->grids[map->vp->power];
	    if (grid != NULL) {
		xform(map, x, y0, &sx, &sy);
		hex_face_rect(dir, map->vp->power, &rsx, &rsy, &sw, &sh);
		rect.x = rsx;  rect.y = rsy;
		rect.w = sw;  rect.h = sh;
		rect2.x = sx + rsx;  rect2.y = sy + rsy;
		rect2.w = sw;  rect2.h = sh;
		SDL_BlitSurface(grid, &rect, map->screen->surf, &rect2);
	    }
	}
    }
}

/* Call on an as-needed basis to get a surface with the thin lines
   needed to draw the grid. */

void
get_grid_surface(int pow)
{
    Image *himg;
    SDL_Surface *sdlimg, *tmpsurf;
    SDL_Rect rect;

    /* If we've already looked before, don't try again. */
    if (ui->no_grid_available[pow])
      return;
    himg = best_image(hexagon_w1, hws[pow], hhs[pow]);
    if (himg == NULL || himg->w != hws[pow] || himg->h != hhs[pow]) {
	ui->no_grid_available[pow] = TRUE;
	return;
    }
    sdlimg = sdl_image(himg);
    if (sdlimg == NULL) {
	ui->no_grid_available[pow] = TRUE;
	return;
    }
    tmpsurf = SDL_AllocSurface(SDL_SWSURFACE, himg->w, himg->h, 32,
			       0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    SDL_FillRect(tmpsurf, NULL, SDL_MapRGB(tmpsurf->format, 255, 0, 255));
    rect.x = rect.y = 0;
    rect.w = himg->w;  rect.h = himg->h;
    SDL_BlitSurface(sdlimg, NULL, tmpsurf, &rect);
    SDL_SetColorKey(tmpsurf, SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(sdlimg->format, 255, 255, 255));
    ui->grids[pow] =
      SDL_AllocSurface(SDL_SWSURFACE, himg->w, himg->h, 32,
		       0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    SDL_FillRect(ui->grids[pow], NULL,
		 SDL_MapRGB(ui->grids[pow]->format, 0, 0, 0));
    SDL_BlitSurface(tmpsurf, NULL, ui->grids[pow], &rect);
    SDL_SetColorKey(ui->grids[pow], SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(ui->grids[pow]->format, 255, 0, 255));
    SDL_FreeSurface(tmpsurf);
}

static void
draw_row(Map *map, int x0, int y0, int len)
{
    int x, i;

    if (clouds_defined() && map->vp->draw_clouds) {
	draw_clouds_row(map, x0, y0, len);
    }
    if (temperatures_defined() && map->vp->draw_temperature
	&& map->vp->hw > 10) {
	draw_temperature_row(map, x0, y0, len);
    }
    if (winds_defined() && map->vp->draw_winds && map->vp->hw > 10) {
	draw_winds_row(map, x0, y0, len);
    }
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
    /* Draw things that only appear on interior cells. */
    if (features_defined() && map->vp->draw_feature_boundaries) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_feature_boundary(map, x, y0);
	}
    }
    if (features_defined() && map->vp->draw_feature_names && ui->legends) {
	for (i = 0; i < numfeatures; ++i) {
	    if (ui->legends[i].oy == y0) {
		draw_feature_name(map, i);
	    }
	}
    }
    /* Draw sparse things on top of the basic row. */
    if (((people_sides_defined() && map->vp->draw_people)
	 || (control_sides_defined() && map->vp->draw_control))
	&& map->vp->hw >= 8) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_people(map, x, y0);
	}
    }
    /* Draw units. */
    /* (should do names separately, to prevent overlap problems.) */
    if (1 /*map->vp->draw_units*/) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_uviews_in_cell(map, x, y0);
	}
    }
#if 0
    /* Draw resources, such as usage by a city in Civ-type games. */
    /* (should this be controlled by vp->draw_units or not?) */
    if (any_resources) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_resource_usage(map, x, y0);
	}
    }
    if (map->vp->draw_ai) {
	for (x = x0; x < x0 + len; ++x) {
	    draw_ai_region(map, x, y0);
	}
    }
#endif
}

static void
draw_clouds_row(Map *map, int x0, int y0, int len)
{
    int x, sx, sy;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (m_terrain_visible(map->vp, x, y0)) {
	    sprintf(spbuf, "%d", cloud_view(dside, x, y0));
	    xform(map, x, y0, &sx, &sy);
	    draw_string(map->screen->surf, sx, sy, spbuf);
	}
    }
}

static void
draw_temperature_row(Map *map, int x0, int y0, int len)
{
    int x, sx, sy;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (m_terrain_visible(map->vp, x, y0)
	    && draw_temperature_here(dside, x, y0)) {
	    sprintf(spbuf, "%d", temperature_view(dside, x, y0));
	    xform(map, x, y0, &sx, &sy);
	    draw_string(map->screen->surf, sx, sy, spbuf);
	}
    }
}

static void
draw_winds_row(Map *map, int x0, int y0, int len)
{
    int x, sx, sy, rawwind, wdir, wforce, swforce;

    for (x = x0; x < x0 + len - 1; ++x) {
	if (draw_winds_here(dside, x, y0)) {
	    rawwind = wind_view(dside, x, y0);
	    /* Show designers an accurate view. (should test in kernel?) */
	    if (is_designer(dside))
	      rawwind = (winds_defined() ? raw_wind_at(x, y0) : CALM);
	    wdir = wind_dir(rawwind);  
	    wforce = wind_force(rawwind);
	    xform(map, x, y0, &sx, &sy);
	    sx += (map->vp->hw - 16) / 2;  
	    sy += (map->vp->hh - 16) / 2;
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
#if 0
	    if (use_clip_mask) {
	      XSetClipMask(dpy, gc, windpics[wforce][wdir]);
	      XSetClipOrigin(dpy, gc, sx + 1, sy + 1);
	      /* Draw a white arrow, offset slightly, for contrast on dark
		 backgrounds. */
	      XSetForeground(dpy, gc, ui->whitecolor->pixel);
	      XFillRectangle(dpy, map->d, gc, sx + 1, sy + 1, 16, 16);
	      /* Draw the main black arrow. */
	      XSetClipOrigin(dpy, gc, sx, sy);
	      XSetForeground(dpy, gc, ui->blackcolor->pixel);
	      XFillRectangle(dpy, map->d, gc, sx, sy, 16, 16);
	    } else {
	    }
#endif
	}
    }
}

/* Draw unit completeness indicator. */

static int
draw_unit_completeness(Map *map, UnitView *uview, 
		       int sx, int sy, int sw, int sh)
{
    Unit *unit = NULL;
    Uint8 r = 0, g = 0, b = 0, a = 0;
    int u = NONUTYPE;
    SDL_Rect rect;

    /* Sanity checks. */
    assert_error(map, "Attempted to draw on a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    if (uview->complete)
      return FALSE;
    if (sw < 16)
      return FALSE;
    /* Get unit type. */
    u = uview->type;
    if (0 >= u_cp(u))
      return FALSE;
    /* Draw border. */
    if (!find_rgb_triplet_by_name(u_construction_border_color(u), 
				  stdcolors, &r, &g, &b))
      run_warning("Could find the color: %s", u_construction_border_color(u));
    draw_rect(map->screen->surf, sx, sy, sw, sh,
	      SDL_MapRGB(map->screen->surf->format, r, g, b));
    /* Fill interior only if we know much about the unit. */
    if (uview->siden == dside->id) {
	unit = view_unit(uview);
	assert_error(unit, "A NULL unit was produced by a non-NULL uview");
	/* Calculate the alpha channel. */
	a = SDL_ALPHA_OPAQUE - (unit->cp * SDL_ALPHA_OPAQUE) / u_cp(u);
	/* Pull the fill rectangle inside the border. */
	rect.x = sx + 1;  rect.y = sy + 1;
	rect.w = sw - 2;  rect.h = sh - 2;
	/* Find the fill color. */
	if (!find_rgb_triplet_by_name(u_construction_fill_color(u), 
				      stdcolors, &r, &g, &b))
	  run_warning("Could find the color: %s", u_construction_fill_color(u));
	/* Fill the rect. */
	alpha_blend_fill_rect(map->screen->surf, &rect, r, g, b, a);
    }
    return TRUE;
}

/* Draw a given unit view on the map. */

static void
draw_unit_view(Map *map, UnitView *uview, int sx, int sy, int sw, int sh)
{
    int sidenum = -1;

    assert_error(map, "Attempted to draw on a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    /* If an occupant's side is the same as its transport's, then
       there's really no need to draw its side emblem, since the
       transport's emblem will also be visible. */
    if (uview->transport && (uview->siden == uview->transport->siden))
      sidenum = -1;
    else
      sidenum = uview->siden;
    /* Draw completeness indicator, if needed. */
    if (!uview->complete)
      draw_unit_completeness(map, uview, sx, sy, sw, sh);
    /* Draw the unit image proper. */
    draw_unit_image(uview->imf, map, sx, sy, sw, sh, sidenum,
		    !uview->complete);
    /* If the unit image is sufficiently large and the unit is advanced,
       then draw its size. */
    if (sw >= 8 && u_advanced(uview->type)) {
        draw_unit_size(map, uview->size, sx + sw, sy - sh/4, sw, map->vp->uh);
    }
    /* If the unit has a name, the viewport allows name display, and the
       unit image is sufficiently large, then display its name. */
    if (sw >= 16 && map->vp->draw_names && uview->name) {
        draw_unit_name(map, uview->name, sx, sy, sw, sh);
    }
}

/* Draw a grouping box around a transport and its occs. */

static int
draw_uimg_gbox(Map *map, UnitView *uview, int sx, int sy, int sw, int sh)
{
    SDL_Rect rect;
    Uint8 r, g, b;

    assert_error(map, "Attempted to draw on a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    /* Spec the grouping box. */
    rect.x = sx;  rect.y = sy;
    rect.w = sw + 1;  rect.h = sh - 1;
    /* Draw border rect first. */
    if (!find_rgb_triplet_by_name(g_unit_gbox_border_color(), stdcolors, 
				  &r, &g, &b))
      run_warning("Could not find the requested color: %s", 
		  g_unit_gbox_border_color());
    SDL_FillRect(map->screen->surf, &rect,
		 SDL_MapRGB(map->screen->surf->format, r, g, b));
    /* Then draw fill rect. */
    ++rect.x;  ++rect.y;
    rect.w -= 2;  rect.h -= 2;
    if (!find_rgb_triplet_by_name(g_unit_gbox_fill_color(), stdcolors, 
				  &r, &g, &b))
      run_warning("Could not find the requested color: %s",
		  g_unit_gbox_fill_color());
    SDL_FillRect(map->screen->surf, &rect,
		 SDL_MapRGB(map->screen->surf->format, r, g, b));
    return TRUE;
}

/* Draw the unit view, taking into account any occs and the zoom factor. */

static void
draw_unit_view_and_occs(Map *map, UnitView *uview, int sx, int sy,
                        int sw, int sh)
{
    int sx2, sy2, sw2, sh2;
    UnitView *occview;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    /* Sanity checks. */
    assert_error(map, "Attempted to draw on a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    /* Setup the side to xform for, depending on 'show_all'. */
    if (map->main_vp->show_all)
      side = NULL;
    else
      side = dside;
    /* If the unit rect is wide enough to support the drawing of occs,
       then draw them. */
    if ((sw >= 16) && uview->occupant && map->vp->draw_occupants
        && !(map->vp->isometric)) {
        /* Draw the grouping box. */
        draw_uimg_gbox(map, uview, sx, sy, sw, sh);
        /* Maybe draw the transport's image. */
	if (sw >= 32) {
	    flags = xform_unit_view(side, map->main_vp, uview, 
				    &sx2, &sy2, &sw2, &sh2,
				    XFORM_UVIEW_AS_TSPT, NULL, sx, sy, sw, sh);
	    if (flags & XFORM_UVIEW_DONT_DRAW)
	      return;
	}
	else {
            sx2 = sx;  sy2 = sy;  sw2 = sw; sh2 = sh;
        }
        draw_unit_view(map, uview, sx2, sy2, sw2, sh2);
        /* Maybe draw the occupants' images. */
	if (sw >= 32) {
	    for_all_occupant_views(uview, occview) {
		flags = xform_unit_view(side, map->main_vp, occview,
					&sx2, &sy2, &sw2, &sh2,
					XFORM_UVIEW_AS_OCC, NULL, 
					sx, sy, sw, sh);
		if (flags & XFORM_UVIEW_DONT_DRAW)
		  continue;
		draw_unit_view_and_occs(map, occview, sx2, sy2, sw2, sh2);
	    }
	}
    }
    /* Else just draw the unit. */
    else {
        draw_unit_view(map, uview, sx, sy, sw, sh);
    }
}

/* Draw all visible units in a given cell. */

static void
draw_uviews_in_cell(Map *map, int x, int y)
{
    VP *vp = NULL;
    UnitView *uview = NULL, *uvstack = NULL;
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    assert_error(map, "Attempted to draw on a NULL map");
    vp = map->main_vp;
    assert_error(vp, "Attempted to access a NULL viewport");
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
    for_all_uvstack(uvstack, uview) {
        /* Get screen coords and dims of unit. */
        flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, flags);
        /* If we were told not to draw, then honor the request. */
        if (flags & XFORM_UVIEW_DONT_DRAW)
          continue;
	/* Draw the unit and any occs (if allowed and reasonable). */
        draw_unit_view_and_occs(map, uview, sx, sy, sw, sh);
    }
}

static void
draw_dots(Map *map, int sx, int sy, int sw, int sh)
{
#if 0
    int osx, osy;

    osx = sx + sw / 2 - 6;  osy = sy + sh - 2;
    /* (should be able to do with one fill?) */
    if (use_clip_mask) {
	XSetClipMask(dpy, gc, None);
	XSetClipOrigin(dpy, gc, osx, osy - 1);
	XSetForeground(dpy, gc, ui->whitecolor->pixel);
	XFillRectangle(dpy, map->d, gc, osx, osy - 1, 12, 4);
	XSetForeground(dpy, gc, ui->blackcolor->pixel);
	XSetClipMask(dpy, gc, ui->dots);
	XFillRectangle(dpy, map->d, gc, osx, osy - 1, 12, 4);
    } else {
	XSetFillStyle(dpy, gc, FillOpaqueStippled);
	XSetTSOrigin(dpy, gc, osx, osy - 1);
	XSetStipple(dpy, gc, ui->dots);
	XSetBackground(dpy, gc, ui->whitecolor->pixel);
	XSetForeground(dpy, gc, ui->blackcolor->pixel);
	XFillRectangle(dpy, map->d, gc, osx, osy - 1, 12, 4);
	XSetTSOrigin(dpy, gc, 0, 0);
	XSetFillStyle(dpy, gc, FillSolid);
    }
#endif
}

static void
draw_unit_name(Map *map, const char *name, int sx, int sy, int sw, int sh)
{
    char namebuf[BUFSIZE];

    /* Don't draw numbers in this routine. */
    if (empty_string(name))
      return;
    strcpy(namebuf, name);
    sx += map->vp->hw + 2;
    sy += map->vp->uh / 2;  
    draw_string(map->screen->surf, sx, sy, namebuf);
}

static void
draw_unit_size(Map *map, int size, int sx, int sy, int sw, int sh)
{
    char numbuf[12];

    /* Filter out very small images. */
    if (sw < 16)
	return; 
    /* Check for size. */
    if (size < 1)
	return;
    if (sh < 32) {
    	sx += 2;
    } else {
    	sy += sh / 4;
    }
    sprintf(numbuf, "%d", size);
    draw_string(map->screen->surf, sx, sy, numbuf);
}

static void
draw_feature_boundary(Map *map, int x, int y)
{
    int wid, p, dir, fid0, fid1, x1, y1, sx, sy, bitmask1, bitmask2;

    if (!m_terrain_visible(map->vp, x, y))
      return;
    fid0 = raw_feature_at(x, y);
    if (fid0 == 0)
      return;
    p = map->vp->power;
    wid = bwid[p];
    if (wid == 0)
      return;
    /* Compute which sides should have border lines drawn. */
    bitmask1 = bitmask2 = 0;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    fid1 = raw_feature_at(x1, y1);
	    if (fid1 != fid0 && m_terrain_visible(map->vp, x1, y1)) {
		if (is_designer(dside)
		    && (fid0 == ui->curfid
			|| fid1 == ui->curfid))
		  bitmask2 |= (1 << dir);
		else
		  bitmask1 |= (1 << dir);
	    }
	}
    }
    if (bitmask1 != 0 || bitmask2 != 0) {
	xform(map, x, y, &sx, &sy);
#if 0
	XSetClipMask(dpy, gc, None);
	XSetFillStyle(dpy, gc, FillSolid);
	/* Line width of 0 draws as width 1, but possibly more efficiently. */
	wid = (p >= 4 ? 2 : 0);
	if (wid > 0)
	  XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, JoinMiter); 
	if (bitmask1 != 0) {
	    XSetForeground(dpy, gc, ui->graycolor->pixel);
	    for_all_directions(dir) {
		if (bitmask1 & (1 << dir)) {
		    XDrawLine(dpy, map->d, gc,
			      sx + bsx[p][dir], sy + bsy[p][dir],
			      sx + bsx[p][dir+1], sy + bsy[p][dir+1]);
		}
	    }
	}
	if (bitmask2 != 0) {
	    XSetForeground(dpy, gc, ui->badcolor->pixel);
	    for_all_directions(dir) {
		if (bitmask2 & (1 << dir)) {
		    XDrawLine(dpy, map->d, gc,
			      sx + bsx[p][dir], sy + bsy[p][dir],
			      sx + bsx[p][dir+1], sy + bsy[p][dir+1]);
		}
	    }
	}
	if (wid > 0)
	  XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
#endif
    }
}

char buffer[BUFSIZE];

static void
draw_feature_name(Map *map, int f)
{
    Legend *legend = &ui->legends[f];
    int x = legend->ox, y = legend->oy;
    int dist = ((legend->dx + 1) * map->vp->hw * 9) / 10;
    int sx0, sy0, sxc2, syc;
    const char *name;

    name = feature_desc(find_feature(f + 1), buffer);
    if (empty_string(name))
      return;

    xform(map, x, y, &sx0, &sy0);
    /* xform returns coordinates of the upper-left corner of the cell */
    sxc2 = 2 * sx0 + (legend->dx + 1) * map->vp->hw; /* twice center x */
    syc  = sy0 + map->vp->hh / 2;		  /* center y */
    if (sxc2 + 2 * dist < 0)
      return;

    draw_string(map->screen->surf, sxc2 / 2, syc, name);
}

/* Indicate what kind of people are living in the given cell. */

static void
draw_people(Map *map, int x, int y)
{
    int pop, sx, sy, sw, sh, ex, ey, ew, eh, dir, x1, y1, pop1;
    int	con, con1, cbitmask1, cbitmask2, pbitmask1, pbitmask2, drawemblemhere;

    if (!m_terrain_visible(map->vp, x, y))
      return;
    pop = (people_sides_defined() ? people_side_at(x, y) : NOBODY);
    con = (control_sides_defined() ? control_side_at(x, y) : NOCONTROL);
    cbitmask1 = cbitmask2 = pbitmask1 = pbitmask2 = 0;
    drawemblemhere = FALSE;
    /* Decide which edges are borders of the country. */
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    if (inside_area(x1, y1) && m_terrain_visible(map->vp, x1, y1)) {
		pop1 = (people_sides_defined() ? people_side_at(x1, y1) : NOBODY);
		con1 = (control_sides_defined() ? control_side_at(x1, y1) : NOCONTROL);
		if (map->vp->draw_control && con != con1) {
		    cbitmask1 |= 1 << dir;
		    if (con != NOCONTROL && con1 != NOCONTROL) {
			cbitmask2 |= 1 << dir;
		    }
		    drawemblemhere = TRUE;
		} else if (map->vp->draw_people && pop != pop1) {
		    pbitmask1 |= 1 << dir;
		    if (pop != NOBODY && pop1 != NOBODY) {
			pbitmask2 |= 1 << dir;
		    }
		    drawemblemhere = TRUE;
		}
	    } else {
		/* Draw just people in the cells right at the edge of
                   the known world. */
		drawemblemhere = TRUE;
	    }
	}
    }
    if (drawemblemhere) {
	xform(map, x, y, &sx, &sy);
	for_all_directions(dir) {
	    int mask = 1 << dir;

	    if (cbitmask1 & mask) {
		draw_country_boundary_line(map, x, y, dir, TRUE,
					 (cbitmask2 & mask));
	    } else if (pbitmask1 & mask) {
		draw_country_boundary_line(map, x, y, dir, FALSE,
					 (pbitmask2 & mask));
	    }
	}
	/* Draw an emblem for the people in the cell. */
	if (map->vp->draw_people && pop != NOBODY) {
	    /* (should make generic) */
	    sw = (map->vp->isometric ? map->vp->hh : map->vp->hw);
	    sh = (map->vp->isometric ? map->vp->hw / 2 : map->vp->hh);
	    ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
	    ex = sx + sw / 2 - ew / 2;  ey = sy + sh / 2 - eh / 2;
	    /* Maybe offset emblem so it will be visible underneath
	       control emblem. */
	    if (map->vp->draw_control && con != pop) {
		ex += ew / 4;  ey += eh / 4;
	    }
	    draw_side_emblem(map, ex, ey, ew, eh, pop);
	}
	if (map->vp->draw_control && con != NOCONTROL) {
	    /* (should make generic) */
	    sw = (map->vp->isometric ? map->vp->hh : map->vp->hw);
	    sh = (map->vp->isometric ? map->vp->hw / 2 : map->vp->hh);
	    ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
	    ex = sx + sw / 2 - ew / 2;  ey = sy + sh / 2 - eh / 2;
	    if (map->vp->draw_people && con != pop) {
		ex -= ew / 4;  ey -= eh / 4;
	    }
	    draw_side_emblem(map, ex, ey, ew, eh, con);
	}
    }
}

/* Draw a rect containing the side color instead of an unit image. */

int
draw_uimg_as_side_color(Image *img, Map *map, int sx, int sy, int sidenum)
{
    SDL_Rect rect;
    int sw = -1, sh = -1;
    Uint8 r = 0, g = 0, b = 0;
    Side *side = NULL;

    assert_error(img, "Attempted to draw a NULL unit image");
    assert_error(map, "Attempted to draw in a NULL map");
    sw = img->w;  sh = img->h;
    /* Ignore images that are too small. */
    if (!sw || !sh)
      return FALSE;
    /* If image is too large, then tell caller to draw it some other way. */
    if ((sw > 4) && (sh > 4))
      return FALSE;
    /* Get rectangle color. */
    if (sidenum < 0)
      sidenum = dside->id;
    side = side_n(sidenum);
    assert_error(side, "Invalid side number encountered");
    find_rgb_triplet_by_name(side->default_color, stdcolors, &r, &g, &b);
    /* Draw the rectangle. */
    rect.x = sx;  
    rect.y = sy;
    rect.w = sw;  
    rect.h = sh;
    SDL_FillRect(map->screen->surf, &rect,
		 SDL_MapRGB(map->screen->surf->format, r, g, b));
    return TRUE;
}

/* Draw a side emblem for an unit image, if requested. */

int
draw_uimg_emblem(Image *img, Map *map, int sx, int sy, int sidenum)
{
    int sw = -1, sh = -1;
    const char *ename = NULL;
    ImageFamily *eimf = NULL;
    int ex = -1, ey = -1, ew = -1, eh = -1;

    assert_error(img, "Attempted to draw a NULL unit image");
    assert_error(map, "Attempted to draw in a NULL map widget");
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
    eimf = ui->eimages[sidenum];
    /* Calculate emblem position, and proceed if the result is valid. */
    if (emblem_position(img, ename, eimf, sw, sh, map->vp->uh, map->vp->hh,
                        &ex, &ey, &ew, &eh)) {
        /* Do the drawing proper. */
        draw_side_emblem(map, sx + ex, sy + ey, ew, eh, sidenum);
    }
    return TRUE;
}

/* Put an unit image in the map window. */

void
draw_unit_image(ImageFamily *imf, Map *map, int sx, int sy, int sw, int sh, 
		int s2, int mod)
{
    int sx2, sy2, desperate = FALSE;
    Image *uimg;
    SDL_Rect rect;
    SDL_Surface *sdluimg = NULL;

    /* Filter out very small images. */
    if (sw <= 0)
      return;
    /* Find best image in range of widths and heights. */
    uimg = best_image_in_range(imf, sw, sh, sw, sh, sw, sh);
    /* If image is small enough, then substitute a rectangle in the side
       color instead. */
    if (uimg && draw_uimg_as_side_color(uimg, map, sx, sy, s2))
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
	rect.x = sx;  rect.y = sy;
	rect.w = sw;  rect.h = sh;
	sdluimg = sdl_image(uimg);
	if (sdluimg)
	  SDL_BlitSurface(sdluimg, NULL, map->screen->surf, &rect);
	else
	  desperate = TRUE;
    } else {
	desperate = TRUE;
    }
    /* If all else fails, draw a black box and maybe a white border. */
    if (desperate) {
	if (sw >= 16) {
	    rect.x = sx - 1;  rect.y = sy - 1;
	    rect.w = sw + 2;  rect.h = sh + 2;
	    SDL_FillRect(map->screen->surf, &rect,
			 SDL_MapRGB(map->screen->surf->format, 255, 255, 255));
	}
	rect.x = sx;  rect.y = sy;
	rect.w = sw;  rect.h = sh;
	SDL_FillRect(map->screen->surf, &rect,
		     SDL_MapRGB(map->screen->surf->format, 0, 0, 0));
    }
    /* Take care of effects and decorations. */
    if (!desperate) {
	/*! \todo Draw any requested fadeout. (Instead of using a stipple like 
		  with Tkconq, we could probably use an alpha channel.) */
	/* Draw a side emblem, if one was requested. */
	draw_uimg_emblem(uimg, map, sx, sy, s2);
    }
}

/* Draw an emblem identifying the given side.  If a side does not have a
   distinguishing emblem, fall back on some defaults. */

void
draw_side_emblem(Map *map, int ex, int ey, int ew, int eh, int s2)
{
    int ex2, ey2, ew2, eh2;
    Uint32 col;
    Image *eimg;
    SDL_Rect rect;
    SDL_Surface *esurf = NULL;

    /* Draw the emblem's mask, or else an enclosing box. */
    eimg = best_image(ui->eimages[s2], ew, eh);
    if (eimg == NULL)
      return;
    ew2 = eimg->w;  eh2 = eimg->h;
    /* Arbitrarily make an 8x6 flag for solid-color emblems. */
    if (ew2 == 1 && eh2 == 1) {
	ew2 = 8;  eh2 = 6;
    }
    /* Offset the image to draw in the middle of its area, whether
       larger or smaller than the given area. */
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
    rect.x = ex;  rect.y = ey;
    rect.w = ew;  rect.h = eh;
    /* If the emblem is a solid color, fill the entire emblem area
       with that color, plus a black border. */
    if (eimg->w == 1 && eimg->h == 1) {
	col = SDL_MapRGB(map->screen->surf->format, 0, 0, 0);
	SDL_FillRect(map->screen->surf, &rect, col);
	col = SDL_MapRGB(map->screen->surf->format,
			 eimg->r >> 8, eimg->g >> 8, eimg->b >> 8);
	rect.x = ex + 1;  rect.y = ey + 1;
	rect.w = ew - 2;  rect.h = eh - 2;
	SDL_FillRect(map->screen->surf, &rect, col);
    } else {
	esurf = sdl_image(eimg);
	if (esurf) {
	    SDL_BlitSurface(esurf, NULL, map->screen->surf, &rect);
	}
    }
}

static void
draw_country_boundary_line(Map *map, int x, int y, int dir, int con, int heavy)
{
    int pow = map->vp->power;
    int wid = bwid[pow], dir2;

    if (wid == 0)
      return;
    wid = max(1, wid / 2);
    if (!heavy)
      wid = max(1, wid / 2);
#if 0
    if (con)
      XSetForeground(dpy, gc, dside->ui->frontline_color->pixel);
    else
      XSetForeground(dpy, gc, dside->ui->country_border_color->pixel);
    if (!heavy) {
	/* Lighten the color by stippling with white. */
	XSetBackground(dpy, gc, dside->ui->whitecolor->pixel);
	XSetFillStyle(dpy, gc, FillOpaqueStippled);
	XSetStipple(dpy, gc, dside->ui->grays[gray]);
    }
#endif
    if (map->vp->isometric) {
	dir2 = dir_subtract(dir, map->main_vp->isodir);
#if 0
	XDrawLine(dpy, map->d, gc,
		  sx + ibsx[pow][dir2], sy + ibsy[pow][dir2],
		  sx + ibsx[pow][dir2+1], sy + ibsy[pow][dir2+1]);
#endif
    } else {
	int sx, sy, sw, sh, rsx, rsy;
	SDL_Surface *country_boundary;
	SDL_Rect rect, rect2;

	if (ui->country_boundaries[pow] == NULL)
	  get_country_boundary_surface(pow);
	country_boundary = ui->country_boundaries[pow];
	if (country_boundary != NULL) {
	    xform(map, x, y, &sx, &sy);
	    hex_face_rect(dir, pow, &rsx, &rsy, &sw, &sh);
	    rect.x = rsx;  rect.y = rsy;
	    rect.w = sw;  rect.h = sh;
	    rect2.x = sx + rsx;  rect2.y = sy + rsy;
	    rect2.w = sw;  rect2.h = sh;
	    SDL_BlitSurface(country_boundary, &rect, map->screen->surf, &rect2);
	}
    }
}

/* Call on an as-needed basis to get a surface with the country boundary. */

void
get_country_boundary_surface(int pow)
{
    Image *himg;
    SDL_Surface *sdlimg, *tmpsurf;
    SDL_Rect rect;

    /* If we've already looked before, don't try again. */
    if (ui->no_country_boundary_available[pow])
      return;
    himg = best_image(hexagon_w1, hws[pow], hhs[pow]);
    if (himg == NULL || himg->w != hws[pow] || himg->h != hhs[pow]) {
	ui->no_country_boundary_available[pow] = TRUE;
	return;
    }
    sdlimg = sdl_image(himg);
    if (sdlimg == NULL) {
	ui->no_country_boundary_available[pow] = TRUE;
	return;
    }
    tmpsurf = SDL_AllocSurface(SDL_SWSURFACE, himg->w, himg->h, 32,
			       0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    SDL_FillRect(tmpsurf, NULL, SDL_MapRGB(tmpsurf->format, 255, 0, 255));
    rect.x = rect.y = 0;
    rect.w = himg->w;  rect.h = himg->h;
    SDL_BlitSurface(sdlimg, NULL, tmpsurf, &rect);
    SDL_SetColorKey(tmpsurf, SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(sdlimg->format, 255, 255, 255));
    ui->country_boundaries[pow] =
      SDL_AllocSurface(SDL_SWSURFACE, himg->w, himg->h, 32,
		       0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    SDL_FillRect(ui->country_boundaries[pow], NULL,
		 SDL_MapRGB(ui->country_boundaries[pow]->format, 255, 0, 0));
    SDL_BlitSurface(tmpsurf, NULL, ui->country_boundaries[pow], &rect);
    SDL_SetColorKey(ui->country_boundaries[pow], SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(ui->country_boundaries[pow]->format, 255, 0, 255));
    SDL_FreeSurface(tmpsurf);
}

/* Given a direction and magnification, return a rectangle enclosing
   that border of a hexagon. */

void
hex_face_rect(int dir, int pow, int *sxp, int *syp, int *swp, int *shp)
{
    int hw = hws[pow], hh = hhs[pow], hch = hcs[pow];

    switch (dir) {
      case NORTHEAST:
	*sxp = hw / 2;  *syp = 0;
	*swp = hw / 2;  *shp = hh - hch;
	break;
      case EAST:
	*sxp = hw / 2;  *syp = hh - hch;
	*swp = hw / 2;  *shp = hch - (hh - hch);
	break;
      case SOUTHEAST:
	*sxp = hw / 2;  *syp = hch;
	*swp = hw / 2;  *shp = hh - hch;
	break;
      case SOUTHWEST:
	*sxp = 0;       *syp = hch;
	*swp = hw / 2;  *shp = hh - hch;
	break;
      case WEST:
	*sxp = 0;       *syp = hh - hch;
	*swp = hw / 2;  *shp = hch - (hh - hch);
	break;
      case NORTHWEST:
	*sxp = 0;       *syp = 0;
	*swp = hw / 2;  *shp = hh - hch;
	break;
    }
}

/* Put the point x, y in the center of the map, or at least as close
   as possible. */

void
recenter(Map *map, int x, int y)
{
    int oldsx, oldsy;

    oldsx = map->main_vp->sx;  oldsy = map->main_vp->sy;
    set_view_focus(map->main_vp, x, y);
    center_on_focus(map->main_vp);
    if (map->main_vp->sx != oldsx || map->main_vp->sy != oldsy)
      redraw_screen(map->screen);
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
    if (map == NULL)
      map = sscreen->map;
    if (map == NULL)
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

    xform(map, x, y, &sx, &sy);
    /* Adjust to be the center of the cell, more reasonable if large. */
    sx += map->main_vp->hw / 2;  sy += map->main_vp->hh / 2;
    insetx1 = min(map->main_vp->pxw / 4, 1 * map->main_vp->hw);
    insety1 = min(map->main_vp->pxh / 4, 1 * map->main_vp->hch);
    insetx2 = min(map->main_vp->pxw / 4, 2 * map->main_vp->hw);
    insety2 = min(map->main_vp->pxh / 4, 2 * map->main_vp->hch);
    if (sx < insetx2)
      return FALSE;
    if (sx > map->main_vp->pxw - insetx2)
      return FALSE;
    if (sy < (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    if (sy > map->main_vp->pxh - (between(2, y, area.height-3) ? insety2 
							       : insety1))
      return FALSE;
    return TRUE;
}

/* Draw selection rectangle around an unit view. */
/*! \todo Break this function up into a part that handles the solid colored
          rectangle, and one that handles the crawling ants or blinking
          rectangle. This would become useful for handling multiple unit
          selection. Multiple units could be selected, but only the current
          unit would have the animated rectangle. */

static int
draw_uview_selection_rectangle(Map *map, UnitView *uview,
                               int sx, int sy, int sw, int sh)
{
    VP *vp = NULL;
    Unit *unit = NULL;
    enum grayshade shade = black;
    Uint32 color;

    assert_error(map, "Attempted to draw in a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    vp = map->vp;
    unit = view_unit(uview);
    /* "Outdent" the curunit indicator slightly, so that it doesn't cover
       up as much of the unit image. */
    if ((sw < vp->hw) && !uview->transport) {
        sx -= 1;  sy -= 1;
        sw += 2;  sh += 2;
    }
#if (0)
    /* Black is for units that can still act, dark gray for actors
       that used all acp for this turn, gray if the unit can't ever do
       anything. */
    shade = gray;
    if (unit->act && (unit->act->initacp > 0))
      shade = (has_acp_left(unit) ? black : darkgray);
#endif
    /* Draw the selection indicator proper. */
    if (sw >= 4 && sh >= 4) {
	color = SDL_MapRGB(map->screen->surf->format, 0, 0, 0);
	draw_rect(map->screen->surf, sx + 1, sy + 1, sw - 3, sh - 2, color);
    }
    /*! \todo Draw curunit indicator. */
    return TRUE;
}

/* Draw health bar over the unit view. */

static int
draw_uview_health_bar(Map *map, UnitView *uview,
                      int sx, int sy, int sw, int sh)
{
    int barwid, tsw, hsw;
    Uint32 color;
    SDL_Rect rect;
    Unit *unit = NULL;
    VP *vp = NULL;

    assert_error(map, "Attempted to draw on a NULL map");
    assert_error(uview, "Attempted to access a NULL unit view");
    vp = map->vp;
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
    color = SDL_MapRGB(map->screen->surf->format, 0, 0, 0);
    rect.x = sx + 2;  rect.y = sy - (2 + barwid) - 1;
    rect.w = tsw;  rect.h = 2 + barwid;
    SDL_FillRect(map->screen->surf, &rect, color);
    /* (should be able to control color changes) */
    color = SDL_MapRGB(map->screen->surf->format, 0, 255, 0);
    if (unit->hp * 4 <= u_hp(unit->type))
      color = SDL_MapRGB(map->screen->surf->format, 255, 0, 0);
    else if (unit->hp * 2 <= u_hp(unit->type))
      color = SDL_MapRGB(map->screen->surf->format, 255, 255, 0);
    rect.x = sx + 3;  rect.y = sy - (2 + barwid);
    rect.w = hsw;  rect.h = barwid;
    SDL_FillRect(map->screen->surf, &rect, color);
    return TRUE;
}

/* Draw the currently-selected unit and its associated decorations. */

static void
draw_current(Map *map)
{
    int sx, sy, sw, sh;
    Unit *unit;
    UnitView *uview = NULL, *origuview = NULL;
    VP *vp = map->main_vp;
    int flags = XFORM_UVIEW_LEGACY;
    Side *side = NULL;

    unit = map->screen->curunit;
    if (!in_play(unit) || unit->id != map->screen->curunit_id)
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
    /* While isometric view, and inside transport, try to find a transport
       to display instead. */
    while (vp->isometric && uview->transport)
      uview = uview->transport;
    /* Get screen position of unit image to be drawn. */
    flags = xform_unit_view(side, vp, uview, &sx, &sy, &sw, &sh, flags);
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
    draw_unit_view_and_occs(map, origuview, sx, sy, sw, sh);
    /*! \todo Draw unit's resource usage. */
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
    draw_uview_selection_rectangle(map, origuview, sx, sy, sw, sh);
#if 0
    if (sw > 4 && vp->draw_names && unit->name)
      draw_unit_name(map, unit->name, sx, sy, sw, sh);
    /* Make sure resource usage appears over the selected unit. */
    if (any_resources
	&& u_advanced(unit->type)) {
	int x, y;
	
	for_all_cells_within_reach(unit, x, y) {
		if (user_at(x, y) == unit->id) {
			draw_resource_usage(map, x, y);
		}
	}
    }
#endif
    draw_uview_health_bar(map, origuview, sx, sy, sw, sh);
    /*! \todo Draw any other curunit decorations. */
}

/* Draw an outline of the main map in the world map. */

static void
draw_map_outline(Map *worldmap, Map *map)
{
    int i, j, sx, sy, sw, sh;
    int wsx[4], wsy[4];
    VP *wvp = worldmap->main_vp;
    Uint32 color;

    if (map->vp->isometric) {
	int x, y;
	VP *vp = map->main_vp;

#if 0
	/* (should use this instead of doing nearest_cell/xform cell) */
	scale_point(vp, wvp,       0,       0, &(wsx[0]), &(wsy[0]));
	scale_point(vp, wvp, vp->pxw,       0, &(wsx[1]), &(wsy[1]));
	scale_point(vp, wvp, vp->pxw, vp->pxh, &(wsx[2]), &(wsy[2]));
	scale_point(vp, wvp,       0, vp->pxh, &(wsx[3]), &(wsy[3]));
#endif
	/* Note that the following nearest cells and scaled points may
	   be offworld, but be laid back about it, do the clipping in
	   a moment. */
	nearest_cell(vp, 0, 0, &x, &y, NULL, NULL);
	xform_cell(wvp, x, y, &(wsx[0]), &(wsy[0]));
	nearest_cell(vp, vp->pxw, 0, &x, &y, NULL, NULL);
	xform_cell(wvp, x, y, &(wsx[1]), &(wsy[1]));
	nearest_cell(vp, vp->pxw, vp->pxh, &x, &y, NULL, NULL);
	xform_cell(wvp, x, y, &(wsx[2]), &(wsy[2]));
	nearest_cell(vp, 0, vp->pxh, &x, &y, NULL, NULL);
	xform_cell(wvp, x, y, &(wsx[3]), &(wsy[3]));
    } else {
	scale_vp(wvp, map->main_vp, &sx, &sy, &sw, &sh);
	wsx[0] = sx;       wsy[0] = sy;
	wsx[1] = sx + sw;  wsy[1] = sy;
	wsx[2] = sx + sw;  wsy[2] = sy + sh;
	wsx[3] = sx;       wsy[3] = sy + sh;
    }
    /* Translate to the location of the world map. */
    for (i = 0; i < 4; ++i) {
	wsx[i] = max(0, wsx[i]);  wsy[i] = max(0, wsy[i]);
	wsx[i] = min(wsx[i], wvp->pxw);  wsy[i] = min(wsy[i], wvp->pxh);
	wsx[i] += worldmap->sx;  wsy[i] += worldmap->sy;
    }
    /* Use black for the outline. */
    color = SDL_MapRGB(worldmap->screen->surf->format, 0, 0, 0);
    for (i = 0; i < 4; ++i) {
	j = (i + 1) % 4;
	draw_line(worldmap->screen->surf, wsx[i], wsy[i], wsx[j], wsy[j],
		  color);
    }
}

// xConq
// Lifecycle management and ser/deser of game areas.

// $Id: area.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Lifecycle management and ser/deser of game areas.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/dir.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Uniques

Area area;

// Global Variables: Game Setup

int any_temp_variation = FALSE;
int any_wind_variation = FALSE;
int any_clouds = FALSE;

// Local Variables: Search and Apply Algorithms

static int stop_apply;

// Queries

int
distance(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1, dy = y2 - y1;

    if (area.xwrap) {
    	/* Choose the shortest way around a cylinder. */
    	dx = (dx < 0 ? (dx < 0 - area.width / 2 ? area.width + dx : dx)
	        	 : (dx > area.width / 2 ? dx - area.width : dx));
    }
    if (dx >= 0) {
	if (dy >= 0) {
	    return (dx + dy);
	} else if ((0 - dy) <= dx) {
	    return dx;
	} else {
	    return (0 - dy);
	}
    } else {
	if (dy <= 0) {
	    return (0 - (dx + dy));
	} else if (dy <= (0 - dx)) {
	    return (0 - dx);
	} else {
	    return dy;
	}
    }
}

int
point_in_dir(int x, int y, int dir, int *xp, int *yp)
{
    *xp = wrapx(x + dirx[dir]);  *yp = y + diry[dir];
    return (in_area(*xp, *yp));
}

// Search and Apply Algorithms

int
limited_search_around(
    int x0, int y0,
    int range,
    int (*pred)(int, int, int *, ParamBox *),
    int incr,
    int *counter,
    int rsltlimit,
    ParamBox *parambox)
{
    int clockwise, dist, dd, d, dir, x1, y1, i, dir2, x, y, xw;

    range = max(min(range, area.width), min(range, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    for (dist = 1; dist <= range; dist += incr) {
	dd = random_dir();
	for_all_directions(d) {
	    dir = (d + dd) % NUMDIRS;
	    x1 = x0 + dist * dirx[dir];
	    y1 = y0 + dist * diry[dir];
	    for (i = 0; i < dist; ++i) {
		dir2 = opposite_dir(dir + clockwise);
		x = x1 + i * dirx[dir2] * incr;
		y = y1 + i * diry[dir2] * incr;
		xw = wrapx(x);
		if (inside_area(x, y) && (*pred)(xw, y, counter, parambox))
		  return TRUE;
                if (counter && (*counter >= rsltlimit))
                  return FALSE;
	    }
	}
    }
    return FALSE;
}

void
apply_to_area(int x0, int y0, int dist, void (*fn)(int, int))
{
    int x, y, x1, y1, x2, y2;

    dist = min(dist, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    stop_apply = FALSE;
    for (y = y1; y <= y2; ++y) {
	if (between(1, y, area.height-2)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(x, y)) {
		    ((*fn)(wrapx(x), y));
		    /* Backdoor for early termination. */
		    if (stop_apply)
		      return;
		}
	    }
	}
    }
}

void
apply_to_ring(int x0, int y0, int distmin, int distmax, void (*fn)(int, int))
{
    int dist, x, y, x1, y1, x2, y2;

    dist = min(distmax, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    for (y = y1; y <= y2; ++y) {
	if (between(1, y, area.height-2)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(x, y) && distance(x, y, x0, y0) >= distmin) {
		    ((*fn)(wrapx(x), y));
		}
	    }
	}
    }
}

// Game Setup

int
set_area_shape(int width, int height, int warn)
{
    int changed = FALSE;

    if (!valid_area_shape(width, height, warn))
      return FALSE;
    if (area.width != width || area.height != height) {
      changed = TRUE;
    }
    area.width = width;  area.height = height;
    area.halfwidth = area.width / 2;  area.halfheight = area.height / 2;
    area.maxdim = max(area.width, area.height);
    area.xwrap = (world.circumference == area.width);
    area.numcells = area.width * area.height;
    if (!area.xwrap)
      area.numcells = (area.numcells * 3) / 4;
    /* We don't have code to resize the units table once allocated. */
    if (changed && area.units != NULL) {
      run_error("attempt to resize world");
    }
    return TRUE;
}

int
valid_area_shape(int width, int height, int warn)
{
    if (width < MINWIDTH || height < MINHEIGHT) {
	if (warn)
	  init_warning("area is %dx%d, too small", width, height);
    	return FALSE;
    }
    if (width != world.circumference && width * 2 < height) {
	if (warn)
	  init_warning("hexagon area is %dx%d, impossible dimensions",
		       width, height);
    	return FALSE;
    }
    return TRUE;
}

void
check_area_shape(void)
{
    if (area.width == 0 || area.height == 0)
      run_error("0x0 area");
    if (!valid_area_shape(area.width, area.height, TRUE))
      run_error("sorry, this is a fatal error here");
}

void
patch_linear_terrain(int t)
{
    int x, y, dir, x1, y1;

    if (t_is_border(t)) {
	for_all_cells(x, y) {
	    /* This test is a hack to save some time.  If a cell has
	       no border flags in any direction, then either it has no
	       borders or else it will be fixed up later on, when an
	       adjacent cell is patched. */
	    if (aux_terrain_at(x, y, t) != 0) {
		for_all_directions(dir) {
		    if (border_at(x, y, dir, t)
			&& point_in_dir(x, y, dir, &x1, &y1)
			&& !border_at(x1, y1, opposite_dir(dir), t))
		      set_border_at(x, y, dir, t, TRUE);
		}
	    }
	}
    } else if (t_is_connection(t)) {
	for_all_cells(x, y) {
	    if (aux_terrain_at(x, y, t) != 0) {
		for_all_directions(dir) {
		    if (connection_at(x, y, dir, t)
			&& point_in_dir(x, y, dir, &x1, &y1)
			&& !connection_at(x1, y1, opposite_dir(dir), t))
		      set_connection_at(x, y, dir, t, TRUE);
		}
	    }
	}
    }
}

void
compute_elevation_bounds(void)
{
    int x, y, el, count, sum, rowcount, rowsum, rowavg;

    area.minelev = area.maxelev = area.avgelev = 0;
    if (elevations_defined()) {
	area.maxelev = area.minelev = elev_at(area.width / 2, area.height / 2);
	/* In order to avoid overflowing by summing all elevations at
	   once, we average along a row, then average the row averages. */
	count = sum = 0;
	for (y = 0; y < area.height; y++) {
	    rowcount = rowsum = 0;
	    for (x = 0; x < area.width; x++) {
		if (in_area(x, y)) {
		    el = elev_at(x, y);
		    if (el < area.minelev)
		      area.minelev = el;
		    if (el > area.maxelev)
		      area.maxelev = el;
		    ++rowcount;
		    rowsum += el;
		}
	    }
	    rowavg = rowsum / rowcount;
	    ++count;
	    sum += rowavg;
	}
	area.avgelev = sum / count;
	Dprintf("Area elevations range %d to %d, averaging %d\n",
		area.minelev, area.maxelev, area.avgelev);
    }
}

// GDL I/O

void
add_to_area(Obj *spec)
{
    int x0, y0, w, h, t, x, y;
    char *propname;
    Obj *val, *typeval;

    /* Collect the dimensions of the area to modify. */
    if (numberp(car(spec))) {
	x0 = c_number(car(spec));
	spec = cdr(spec);
    }
    if (numberp(car(spec))) {
	y0 = c_number(car(spec));
	spec = cdr(spec);
    }
    if (numberp(car(spec))) {
	w = c_number(car(spec));
	spec = cdr(spec);
	if (numberp(car(spec))) {
	    h = c_number(car(spec));
	    spec = cdr(spec);
	} else {
	    h = w;
	}
    } else {
	w = h = 1;
    }
    PARSE_PROPERTY(spec, propname, val);
    switch (keyword_code(propname)) {
      case K_TERRAIN:
	if (symbolp(val) && ttypep(typeval = eval(val))) {
	    t = c_number(typeval);
	    for (x = x0; x < x0 + w; ++x) {
		for (y = y0; y < y0 + h; ++y) {
		    if (in_area(x, y)) {
			set_terrain_at(x, y, t);
		    }
		}
	    }
	}
	break;
      case K_AUX_TERRAIN:
	break;
      case K_PEOPLE_SIDES:
	break;
      case K_CONTROL_SIDES:
	break;
      case K_FEATURES:
	break;
      case K_ELEVATIONS:
	break;
      case K_MATERIAL:
	break;
      case K_TEMPERATURES:
	break;
      case K_WINDS:
	break;
      case K_CLOUDS:
	break;
      case K_CLOUD_BOTTOMS:
	break;
      case K_CLOUD_HEIGHTS:
	break;
      default:
	unknown_property("area", "", propname);
    }
}

void
interp_area(Obj *form)
{
    int newarea = FALSE, newwidth = 0, newheight = 0, numval;
    Obj *props, *subprop, *bdg, *propval, *rest, *val;
    char *propname, *strval;

    props = cdr(form);
    if (match_keyword(car(props), K_ADD)) {
	add_to_area(cdr(props));
	return;
    }
    /* (eventually this will be an id or name) */
    if (symbolp(car(props))) {
	props = cdr(props);
	newarea = TRUE;
    }
    /* Collect the width of the area. */
    if (numberp(car(props))) {
	newwidth = newheight = c_number(car(props));
    	if (area.fullwidth == 0)
	  newarea = TRUE;
	if (area.fullwidth > 0 && area.fullwidth != newwidth)
	  read_warning("weird areas - %d vs %d", area.fullwidth, newwidth);
	props = cdr(props);
    }
    /* Collect the height of the area. */
    if (numberp(car(props))) {
	newheight = c_number(car(props));
    	if (area.fullheight == 0)
	  newarea = TRUE;
	if (area.fullheight > 0 && area.fullheight != newheight)
	  read_warning("weird areas - %d vs %d", area.fullheight, newheight);
	props = cdr(props);
    }
    /* See if we're restricting ourselves to a piece of a larger area. */
    if (consp(car(props))
        && match_keyword(car(car(props)), K_RESTRICT)) {
        subprop = cdr(car(props));
	val = car(subprop);
        if (numberp(val)) {
	    area.fullwidth = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fullheight = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fullx = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fully = c_number(val);
        } else if (match_keyword(car(subprop), K_RESET)) {
	    area.fullwidth = area.fullheight = 0;
	    area.fullx = area.fully = 0;
        } else {
	    syntax_error(car(props), "not 4 numbers or 'reset'");
	    return;
        }
	props = cdr(props);
    }
    /* If this is setting the area's shape for the first time,
       actually do it. */
    if (newarea)
      set_area_shape(newwidth, newheight, TRUE);
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (numberp(propval))
	  numval = c_number(propval);
	rest = cdr(bdg);
	switch (keyword_code(propname)) {
	  case K_WIDTH:
	    /* Note that this may fail if the height has to change at
	       at the same time. */
	    set_area_shape(numval, area.height, TRUE);
	    break;
	  case K_HEIGHT:
	    /* Note that this may fail if the width has to change at
	       at the same time. */
	    set_area_shape(area.width, numval, TRUE);
	    break;
	  case K_LATITUDE:
	    area.latitude = numval;
	    break;
	  case K_LONGITUDE:
	    area.longitude = numval;
	    break;
	  case K_PROJECTION:
	    area.projection = numval;
	    break;
	  case K_CELL_WIDTH:
	    area.cellwidth = numval;
	    break;
	  case K_SUN:
	    area.sunx = numval;
	    area.suny = c_number(cadr(rest));
	    break;
	  case K_TEMPERATURE_YEAR_CYCLE:
	    area.temp_year = propval;
	    break;
	  case K_IMAGE_NAME:
	    strval = NULL;
	    if (stringp(propval))
	      strval = c_string(propval);
	    area.image_name = strval;
	    break;
	  case K_TERRAIN:
	    fill_in_terrain(rest);
	    break;
	  case K_AUX_TERRAIN:
	    fill_in_aux_terrain(rest);
	    break;
	  case K_PEOPLE_SIDES:
	    fill_in_people_sides(rest);
	    break;
	  case K_CONTROL_SIDES:
	    fill_in_control_sides(rest);
	    break;
	  case K_FEATURES:
	    fill_in_features(rest);
	    break;
	  case K_ELEVATIONS:
	    fill_in_elevations(rest);
	    break;
	  case K_MATERIAL:
	    fill_in_cell_material(rest);
	    break;
	  case K_TEMPERATURES:
	    fill_in_temperatures(rest);
	    break;
	  case K_WINDS:
	    fill_in_winds(rest);
	    break;
	  case K_CLOUDS:
	    fill_in_clouds(rest);
	    break;
	  case K_CLOUD_BOTTOMS:
	    fill_in_cloud_bottoms(rest);
	    break;
	  case K_CLOUD_HEIGHTS:
	    fill_in_cloud_heights(rest);
	    break;
	  case K_USER:
	    fill_in_users(rest);
	    break;
	  default:
	    unknown_property("area", "", propname);
	}
    }
}

void
write_areas(Module *module)
{
    int all = module->def_all, compress = module->compress_layers;

    newline_form();
    /* Write the basic dimensions. */
    start_form(key(K_AREA));
    /* K_WIDTH, K_HEIGHT written if nonzero. */
    if (area.width > 0 || area.height > 0) {
	add_num_to_form((doreshape ? reshaper->final_width : area.width));
	add_num_to_form((doreshape ? reshaper->final_height : area.height));
    }
    /* Write all the scalar properties. */
    write_num_prop(key(K_LATITUDE), area.latitude, 0, 0, FALSE);
    write_num_prop(key(K_LONGITUDE), area.longitude, 0, 0, FALSE);
    write_num_prop(key(K_PROJECTION), area.projection, 0, 0, FALSE);
    write_num_prop(key(K_CELL_WIDTH), area.cellwidth, 0, 0, FALSE);
    if (area.sunx != area.width / 2 || area.suny != area.halfheight) {
	start_form(key(K_SUN));
	add_num_to_form(area.sunx);
	add_num_to_form(area.suny);
	end_form();
	newline_form();
    }
    write_lisp_prop(key(K_TEMPERATURE_YEAR_CYCLE), area.temp_year,
		    lispnil, FALSE, FALSE, TRUE);
    write_str_prop(key(K_IMAGE_NAME), area.image_name,
		   "", FALSE, TRUE);
    end_form();
    newline_form();
    /* Write the area's layers, each as a separate form. */
    if (all || module->def_area_terrain)
      write_area_terrain(compress);
    if (all || module->def_area_terrain)
      write_area_aux_terrain(compress);
    if (all || module->def_area_misc)
      write_area_features(compress);
    if (all || module->def_area_misc)
      write_area_elevations(compress);
    if (all || module->def_area_misc)
      write_area_people_sides(compress);
    if (all || module->def_area_misc)
      write_area_control_sides(compress);
    if (all || module->def_area_material)
      write_area_materials(compress);
    if (all || module->def_area_weather)
      write_area_temperatures(compress);
    if (all || module->def_area_weather)
      write_area_clouds(compress);
    if (all || module->def_area_weather)
      write_area_winds(compress);
   /* It is alloc_area_terrain that handles the user layer. */
   if (all || module->def_area_terrain)
      write_area_users(compress);
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

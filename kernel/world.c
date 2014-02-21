/* Worlds and areas in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

#include <math.h>

/* The main world structure. */

World world;

/* The current area of the world. */

Area area;

/* This is the head and tail of the list of features. */

Feature *featurelist = NULL;

Feature *last_feature = NULL;

/* Feature id 0 means no geographical feature defined. */

int nextfid = 1;

int numfeatures;

int minelev;
int maxelev;

/* This is true if the elevation may ever vary in a game. */

int any_elev_variation = FALSE;

int mintemp;
int maxtemp;

/* This is true if the temperature ever changes during a game. */

int any_temp_variation = FALSE;

/* This is true if the temperatures in a layer can have different values in
   different cells. */

int any_temp_variation_in_layer = FALSE;

int minwindforce;
int maxwindforce;

int any_wind_variation = FALSE;

/* This is true if the winds in a layer can have different values in
   different cells. */

int any_wind_variation_in_layer = FALSE;

/* This is true if clouds can ever exist. */

int any_clouds = FALSE;

int minclouds;
int maxclouds;

/* This is true if it is ever possible to have quantities of material
   in cells. */

int any_materials_in_terrain = FALSE;

static Feature *tmpfeature;

static int tmpint, tmpint2, tmpint3;

TRegion *terrain_region_list;
TRegion *landsea_region_list;

int num_terrain_regions;
int num_landsea_regions;

/* Clean out all the world and area data. */

void
init_world(void)
{
    memset(&world, 0, sizeof(World));
    /* These default values effectively disable day and year effects. */
    world.daylength = 1;
    world.yearlength = 1;
    world.daylight_fraction = 50;
    world.twilight_fraction = 60;
    /* Init the current (default) area. */
    memset(&area, 0, sizeof(Area));
    /* Note especially that area width and height dimensions are now zero. */
    area.sunx = area.suny = -1;
    area.temp_year = lispnil;
}

int
set_world_circumference(int circum, int warn)
{

    /* All world circumferences are valid, no checks necessary. */
    /* (circumference < area width means that the "world" is not part
       of a sphere, such as for space games; meridian display should be
       disabled, etc) */
    world.circumference = circum;
    world.daylight_width =
      ((world.daylight_fraction * world.circumference) / 100) / 2;
    world.twilight_width =
      ((world.twilight_fraction * world.circumference) / 100) / 2;
    area.xwrap = (world.circumference == area.width);
    return TRUE;
}

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

/* Calculate globals that we can use later to decide if particular
   classes of calculations need to be done, such as weather changes. */

void
calculate_world_globals(void)
{
    int t, m;

    /* This is a last chance to set a default world size.
       Will usually be set already, by players or by module. */
    if (area.width <= 0 && area.height <= 0)
      set_area_shape(DEFAULTWIDTH, DEFAULTHEIGHT, 0);

    /* Also assign a reasonable circumference if still undefined. Use default circumference 
    for all those real-earth games in which it is undefined (zero). However, don't allow the 
    circumference to be less than the area width.  To force a flat world (space games etc.) the 
    circumference should be set to -1. */
    if (world.circumference == 0)
	world.circumference = max(area.width, DEFAULTCIRCUMFERENCE);

    /* Set uninitialized sun position to something reasonable. */
    if (area.sunx < 0)
      area.sunx = area.width / 2;
    if (area.suny < 0)
      area.suny = area.halfheight;
    minelev = t_elev_min(0);
    maxelev = t_elev_max(0);
    mintemp = t_temp_min(0);
    maxtemp = t_temp_max(0);
    minwindforce = t_wind_force_min(0);
    maxwindforce = t_wind_force_max(0);
    minclouds = t_clouds_min(0);
    maxclouds = t_clouds_max(0);
    for_all_terrain_types(t) {
	if (t_elev_min(t) < minelev)
	  minelev = t_elev_min(t);
	if (t_elev_max(t) > maxelev)
	  maxelev = t_elev_max(t);
	if (t_temp_min(t) < mintemp)
	  mintemp = t_temp_min(t);
	if (t_temp_max(t) > maxtemp)
	  maxtemp = t_temp_max(t);
	if (t_wind_force_min(t) < maxwindforce)
	  minwindforce = t_wind_force_min(t);
	if (t_wind_force_max(t) > maxwindforce)
	  maxwindforce = t_wind_force_max(t);
	if (t_clouds_min(t) < minclouds)
	  minclouds = t_clouds_min(t);
	if (t_clouds_max(t) > maxclouds)
	  maxclouds = t_clouds_max(t);	
	/* Decide if materials can ever be accumulated in cells. */
	for_all_material_types(m) {
	    if (tm_storage_x(t, m) > 0)
	      any_materials_in_terrain = TRUE;
	}
    }
    if (minelev != maxelev)
      any_elev_variation = TRUE;
    if (mintemp != maxtemp)
      any_temp_variation = TRUE;
    if (minwindforce != maxwindforce)
      any_wind_variation = TRUE;
    if (minwindforce != maxwindforce)
      any_wind_variation_in_layer = TRUE;
    if (minclouds != maxclouds)
      any_clouds = TRUE;
}

/* Do the final setup that the world needs. */

void
final_init_world(void)
{
    int u;

    /* Calculate extent and position of geographical features. */
    compute_all_feature_centroids();
    /* These area layers must be allocated already here since the tcltk
    Weather menu needs to know if temps etc. exist when init_display
    is run. */
    if (any_temp_variation && !temperatures_defined()) {
        allocate_area_temperatures();
        allocate_area_scratch(3);
    }
    if (any_clouds && !clouds_defined()) {
        allocate_area_clouds();
        allocate_area_scratch(3);
    }
    if (any_wind_variation_in_layer && !winds_defined()) {
        allocate_area_winds();
        allocate_area_scratch(2);
    }
    if (any_elev_variation && !elevations_defined()) {
    	allocate_area_elevations();
    }
    /* Allocate the user layer if there are any advanced units. */
    for_all_unit_types(u) {
	if (u_advanced(u)){
		allocate_area_users();
		break;
	} 
    }
    /* Calculate the landsea layer if not already done. */
    if (area.landsea_regions == NULL) {
	area.landsea_regions = malloc_area_layer(TRegion *);
	divide_into_regions(area.terrain, area.landsea_regions, TRUE);
    }
    compute_elevation_bounds();
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

/* Make space for an area's terrain, and for the unit cache. */

/* This can be postponed until it is actually needed. */

void
allocate_area_terrain(void)
{
    check_area_shape();
    /* Get rid of old stuff maybe. (is this desirable?) */
    if (area.terrain != NULL) {
	free((char *) area.terrain);
	free((char *) area.units);
    }
    /* Allocate the basic terrain layer. */
    /* It doesn't matter what ttype 0 is, we're guaranteed that it
       will be defined eventually. */
    area.terrain = malloc_area_layer(char);
    /* Allocate and null out the unit cache. */
    area.units = malloc_area_layer(Unit *);
}

/* Set up the auxiliary terrain layer of the area. */

void
allocate_area_aux_terrain(int t)
{
    if (!any_aux_terrain_defined()) {
	area.auxterrain = (char **) xmalloc(numttypes * sizeof(char *));
    }
    if (!aux_terrain_defined(t)) {
	area.auxterrain[t] = malloc_area_layer(char);
    }
}

/* Allocate some number of scratch layers.  These are used as temporaries
   in calculations, etc. */

void
allocate_area_scratch(int n)
{
    check_area_shape();
    if (n >= 1 && !area.tmp1) {
	area.tmp1 = malloc_area_layer(short);
    }
    if (n >= 2 && !area.tmp2) {
	area.tmp2 = malloc_area_layer(short);
    }
    if (n >= 3 && !area.tmp3) {
	area.tmp3 = malloc_area_layer(short);
    }
    if (n >= 4) {
	run_error("can't allocate more than 3 scratch layers");
    }
}

/* Should have something to free scratch layers up also maybe. */

/* Allocate and init the elevation layer. */

void
allocate_area_elevations(void)
{
    int x, y;

    if (!elevations_defined()) {
	check_area_shape();
	area.elevations = malloc_area_layer(short);
	/* If we have terrain, make the initial elevations fall into
	   the correct ranges for the types of terrain present. */
	if (terrain_defined()) {
	    for_all_cells(x, y) {
		set_elev_at(x, y, t_elev_min(terrain_at(x, y)));
	    }
	}
    }
}

/* Allocate and init the temperature layer. */

void
allocate_area_temperatures(void)
{
    if (!temperatures_defined()) {
	check_area_shape();
	area.temperature = malloc_area_layer(short);
    }
    /* We'll need one scratch layer too. */
    allocate_area_scratch(1);
}

/* Allocate a layer indicating the side of the people living in each cell. */

void
allocate_area_people_sides(void)
{
    if (!people_sides_defined()) {
	check_area_shape();
	area.peopleside = malloc_area_layer(char);
	/* NOBODY != 0, so need to blast it over the layer. */
	memset(area.peopleside, NOBODY, area.width * area.height);
	memset(area.peopleside, NOBODY, area.width * area.height);
    }
}

/* Allocate a layer indicating the side in control of each cell. */

void
allocate_area_control_sides(void)
{
    if (!control_sides_defined()) {
	check_area_shape();
	area.controlside = malloc_area_layer(char);
	/* NOCONTROL != 0, so need to blast it over the layer. */
	memset(area.controlside, NOCONTROL, area.width * area.height);
    }
}

/* Set up a cell material layer of the area. */

void
allocate_area_material(int m)
{
    check_area_shape();
    if (!any_cell_materials_defined()) {
	area.materials = (long **) xmalloc(nummtypes * sizeof(long *));
    }
    if (!cell_material_defined(m)) {
	area.materials[m] = malloc_area_layer(long);
    }
}

void
allocate_area_clouds(void)
{
    if (!clouds_defined()) {
	check_area_shape();
	area.clouds = malloc_area_layer(short);
    }
}

void
allocate_area_cloud_altitudes(void)
{
    allocate_area_cloud_bottoms();
    allocate_area_cloud_heights();
}

void
allocate_area_cloud_bottoms(void)
{
    if (!cloud_bottoms_defined()) {
	check_area_shape();
	area.cloudbottoms = malloc_area_layer(short);
    }
}

void
allocate_area_cloud_heights(void)
{
    if (!cloud_heights_defined()) {
	check_area_shape();
	area.cloudheights = malloc_area_layer(short);
    }
}

void
allocate_area_winds(void)
{
    if (!winds_defined()) {
	check_area_shape();
	area.winds = malloc_area_layer(short);
    }
}

void
allocate_area_users(void)
{
    if (!user_defined()) {
	check_area_shape();
	area.user = malloc_area_layer(short);
    }
}

int
fn_terrain_at(int x, int y)
{
    return terrain_at(x, y);
}

int
fn_aux_terrain_at(int x, int y)
{
    return aux_terrain_at(x, y, tmpttype);
}

int
fn_feature_at(int x, int y)
{
    return raw_feature_at(x, y);
}

int
fn_elevation_at(int x, int y)
{
    return elev_at(x, y);
}

int
fn_people_side_at(int x, int y)
{
    return people_side_at(x, y);
}

int
fn_control_side_at(int x, int y)
{
    return control_side_at(x, y);
}

int
fn_user_at(int x, int y)
{
    return user_at(x, y);
}

int
fn_material_at(int x, int y)
{
    return material_at(x, y, tmpmtype);
}

int
fn_temperature_at(int x, int y)
{
    return temperature_at(x, y);
}

int
fn_raw_cloud_at(int x, int y)
{
    return raw_cloud_at(x, y);
}

int
fn_raw_cloud_bottom_at(int x, int y)
{
    return raw_cloud_bottom_at(x, y);
}

int
fn_raw_cloud_height_at(int x, int y)
{
    return raw_cloud_height_at(x, y);
}

int
fn_raw_wind_at(int x, int y)
{
    return raw_wind_at(x, y);
}

void
fn_set_terrain_at(int x, int y, int val)
{
    extern int warnbadterrain, numbadterrain;

    /* It's important not to put bad values into the terrain layer. */
    if (!is_terrain_type(val)) {
    	/* Only warn the first few times, just count thereafter. */
    	if (warnbadterrain && numbadterrain < 10) {
	    read_warning("Unknown terrain type (%d) at %d,%d; substituting %s",
			 val, x, y, t_type_name(0));
	}
	val = 0;
	++numbadterrain;
    }
    set_terrain_at(x, y, val);
}

void
fn_set_aux_terrain_at(int x, int y, int val)
{
    /* Filter anything but the basic six bits. */
    val &= 0x3f;
    set_aux_terrain_at(x, y, tmpttype, val);
}

void
fn_set_people_side_at(int x, int y, int val)
{
    set_people_side_at(x, y, val);
}

void
fn_set_control_side_at(int x, int y, int val)
{
    set_control_side_at(x, y, val);
}

void
fn_set_user_at(int x, int y, int val)
{
    set_user_at(x, y, val);
}

void
fn_set_raw_feature_at(int x, int y, int val)
{
    set_raw_feature_at(x, y, val);
}

void
fn_set_elevation_at(int x, int y, int val)
{
    set_elev_at(x, y, val);
}

void
fn_set_material_at(int x, int y, int val)
{
    set_material_at(x, y, tmpmtype, val);
}

void
fn_set_temperature_at(int x, int y, int val)
{
    set_temperature_at(x, y, val);
}

void
fn_set_raw_wind_at(int x, int y, int val)
{
    set_raw_wind_at(x, y, val);
}

void
fn_set_raw_cloud_at(int x, int y, int val)
{
    set_raw_cloud_at(x, y, val);
}

void
fn_set_raw_cloud_bottom_at(int x, int y, int val)
{
    set_raw_cloud_bottom_at(x, y, val);
}

void
fn_set_raw_cloud_height_at(int x, int y, int val)
{
    set_raw_cloud_height_at(x, y, val);
}

/* Change the terrain of the cell at the given location to the given
   type of terrain, and update everything affected by the change. */

void
change_terrain_type(int x, int y, int t2)
{
    int t, m, curview, newview, update, u2;
    HistEventType hevttype;
    Unit *unit2;
    Side *side;

    t = terrain_at(x, y);
    set_terrain_at(x, y, t2);
    for_all_material_types(m) {
	if (tm_initial(t, m) > 0) {
	    if (!(any_cell_materials_defined() && cell_material_defined(m)))
	      allocate_area_material(m);
	    set_material_at(x, y, m, min(tm_storage_x(t, m), tm_initial(t, m)));
	} else {
	    if (any_cell_materials_defined() && cell_material_defined(m))
	      set_material_at(x, y, m, 0);
	}
    }
    /* Let everybody see this change. */
    for_all_sides(side) {
	update = FALSE;
	if (side->see_all) {
	    update = TRUE;
	} else if (cover(side, x, y) > 0) {
	    /* Always update our knowledge of the terrain. */
	    curview = terrain_view(side, x, y);
	    newview = buildtview(t2);
	    set_terrain_view(side, x, y, newview);
	    if (!g_see_terrain_always())
	      set_terrain_view_date(side, x, y, g_turn());
	    if (newview != curview)
	      update = TRUE;
	}
	if (update) {
	    update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
	}
    }
    /* There might be catastrophic consequences for any units here. */
    for_all_stack(x, y, unit2) {
	u2 = unit2->type;
	if (ut_vanishes_on(u2, t2)
	    /* The unit may be rescued by a connection. */
	    && !type_survives_in_cell(unit2->type, unit2->x, unit2->y)) {
	    hevttype = H_UNIT_VANISHED;
	    kill_unit(unit2, hevttype);
	} else if (ut_wrecks_on(u2, t2)
	    	   /* The unit may be rescued by a connection. */
		   && !type_survives_in_cell(unit2->type, unit2->x, unit2->y)) {
	    if (u_wrecked_type(u2) == NONUTYPE) {
		/* Occupants always die if the wrecked unit disappears. */
		hevttype = H_UNIT_WRECKED;
		kill_unit(unit2, hevttype);
	    } else {
		/* Change the unit's type. */
		hevttype = H_UNIT_WRECKED;
		wreck_unit(unit2, hevttype, WRECK_TYPE_TERRAIN, t2, NULL);
		/* Maybe make it go away, taking remaining unlucky
                   occupants with. */
		if (ut_vanishes_on(unit2->type, t2)) {
		    hevttype = H_UNIT_VANISHED;
		    kill_unit(unit2, hevttype);
		}
	    }
	}
	if (active_display(unit2->side)) {
	    if (hevttype == H_UNIT_VANISHED) {
		notify(unit2->side, "Terrain change from %s to %s causes %s to vanish",
		       t_type_name(t), t_type_name(t2),
		       unit_handle(unit2->side, unit2));
	    } else if (hevttype == H_UNIT_WRECKED) {
		notify(unit2->side, "Terrain change from %s to %s wrecks %s",
		       t_type_name(t), t_type_name(t2),
		       unit_handle(unit2->side, unit2));
	    }
	}
    }
}

/* Generalized area search routine.  It starts in the immediately adjacent
   cells and expands outwards.  The basic structure is to examine successive
   "rings" out to the max distance; within each ring, we must scan each of
   six faces (picking a random one to start with) by iterating along that
   face, in a direction 120 degrees from the direction out to one corner of
   the face.  Draw a picture if you want to understand it... */

/* Incr is normally one.  It is set to area_size to search on areas
   instead of cells. */

/* Note that points far outside the map may be generated, but the predicate
   will not be called on them.  It may be applied to the same point several
   times, however, if the distance is enough to wrap around the area. */

/* This needs to be changed to understand different world shapes. */

/* Predicate can manipulate a counter and a parameter box for returning 
   compound results. A result limit can terminate the search if counter 
   exceeds it. */

int
limited_search_around(int x0, int y0, int range, 
		      int (*pred)(int, int, int *, ParamBox *),
		      int incr, int *counter, int rsltlimit, 
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

/* Search around without any result limits. */

int
search_around(int x0, int y0, int range, 
		      int (*pred)(int, int, int *, ParamBox *),
		      int incr, ParamBox *parambox)
{
    return limited_search_around(x0, y0, range, pred, incr,  
				 NULL, INT_MAX, parambox);
}

/* Original version of 'search_around'. */
/* (NOTE: This is part of the gradual evolution of 'search_around' to 
    resemble 'search_under_arc' in terms of predicate function parameters. 
    This should eventually be phased out to eliminate code duplication.) */

int
search_around(int x0, int y0, int maxdist, int (*pred)(int, int),
	      int *rxp, int *ryp, int incr)
{
    int clockwise, dist, dd, d, dir, x1, y1, i, dir2, x, y, xw;

    maxdist = max(min(maxdist, area.width), min(maxdist, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    for (dist = 1; dist <= maxdist; dist += incr) {
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
		if (inside_area(x, y) && (*pred)(xw, y)) {
		    *rxp = xw;  *ryp = y;
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/* Limited version of search_around for optimizing performance by 
   restricting the number of query results. The address of the result 
   counter must be given to the predicate function so that it can 
   increment it as deemed fit. */
/* (NOTE: This is part of the gradual evolution of 'search_around' to 
    resemble 'search_under_arc' in terms of predicate function parameters. 
    This should eventually be phased out to eliminate code duplication.) */

int
limited_search_around(int x0, int y0, int maxdist, int (*pred)(int, int, int *),
	      int *rxp, int *ryp, int incr, int rsltlimit)
{
    int clockwise, dist, dd, d, dir, x1, y1, i, dir2, x, y, xw, rsltcount = 0;

    maxdist = max(min(maxdist, area.width), min(maxdist, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    for (dist = 1; dist <= maxdist; dist += incr) {
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
		if (inside_area(x, y) && (*pred)(xw, y, &rsltcount)) {
		    *rxp = xw;  *ryp = y;
		    return TRUE;
		}
                if (rsltcount >= rsltlimit)
                  return FALSE;
	    }
	}
    }
    return FALSE;
}

int
search_and_apply(int x0, int y0, int maxdist, int (*pred)(int, int),
		 int *rxp, int *ryp, int incr, void (*fn)(int, int), int num)
{
    int clockwise, dist, x0w, dd, d, dir, x1, y1, i, dir2, x, y, xw;

    maxdist = max(min(maxdist, area.width), min(maxdist, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    if (maxdist >= 0) {
	x0w = wrapx(x0);
	if (inside_area(x0w, y0)) {
	    if ((*pred)(x0w, y0)) {
		*rxp = x0w;  *ryp = y0;
		(*fn)(x0w, y0);
	    } else {
		return 0;
	    }
	}
    }
    for (dist = 1; dist <= maxdist; dist += incr) {
	dd = random_dir();
	for_all_directions(d) {
	    dir = (d + dd) % NUMDIRS;
	    x1 = x0 + dist * dirx[dir];
	    y1 = y0 + dist * diry[dir];
	    for (i = 0; i < dist; ++i) {
		dir2 = opposite_dir(dir + clockwise);
		x = x1 + i * dirx[dir2] * incr;
		y = y1 + i * diry[dir2] * incr;
		if (between(0, y, area.height-1)) {
		    xw = wrapx(x);
		    if (inside_area(x, y)) {
			if ((*pred)(xw, y)) {
			    *rxp = xw;  *ryp = y;
			    (*fn)(xw, y);
			} else {
			    return 0;
			}
		    }
		}
	    }
	}
    }
    return 0;
}

/* Apply a function to every cell within the given radius, being
   careful (for both safety and efficiency reasons) not to go past
   edges.  Note that the distance is inclusive, and that distance of 0
   means x0,y0 only.  Also, if the distance is greater than either map
   dimension, this routine still operates on a correct intersection
   with the area.  */

int stop_apply;

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

/* Similarly, but operating on edge cells as well. */

void
apply_to_area_plus_edge(int x0, int y0, int dist, void (*fn)(int, int))
{
    int x, y, x1, y1, x2, y2;

    dist = min(dist, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, area.height-1)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(x, y)) {
		    ((*fn)(wrapx(x), y));
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

/* Apply the function to the hexagon bounded by w,h. */

void
apply_to_hexagon(int x0, int y0, int w2, int h2, void (*fn)(int, int))
{
    int x, y, x1, y1, x2, y2;

    y1 = limity(y0 - h2);
    y2 = limity(y0 + h2);
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, area.height-1)) {  /* always true? */
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - w2 + (y < y0 ? (y0 - y) : 0);
	    x2 = x0 + w2 - (y > y0 ? (y - y0) : 0);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(x, y)) {
		    ((*fn)(wrapx(x), y));
		}
	    }
	}
    }
}

/* Search cells under an arc between one directional ray and a neighboring 
   directional ray until rlstlimit results have been accumulated. */
/* Note that larger arcs may searched by decomposing them into the size of 
   arcs used in this search. It is harder to efficiently search wider arcs 
   without overlaps. */
/* If dirbias is set to 0, then only a radial line segment will be searched. */
/* (May be useful for implementing some LOS stuff.) */
int
limited_search_under_arc(int x, int y, int dir, int range, 
                         int dirbias,
                         int (*pred)(int x, int y, int *counter, 
                                     ParamBox *parambox), 
                         int *counter, int rsltlimit, ParamBox *parambox)
{
    int nextdir = NODIR;
    int nx = -1, ny = -1;

    /* Sanity checks. */
    if ((dir < 0) || (dir >= NUMDIRS))
      return FALSE;
    if (range < 0)
      return FALSE;
    /* Apply predicate function to this cell. */
    if (pred(x, y, counter, parambox))
      return TRUE;
    if (counter && (*counter >= rsltlimit))
      return FALSE;
    /* Decrement the remaining range to search. */
    --range;
    /* Recurse into next cells, if they exist and we still have range left. */
    if (range >= 0) {
        nextdir = dir;
        if (!point_in_dir(x, y, nextdir, &nx, &ny))
          nextdir = NODIR;
        if ((nextdir != NODIR)
            && limited_search_under_arc(nx, ny, dir, range, 0, pred, 
                                        counter, rsltlimit, parambox))
          return TRUE;
        if (counter && (*counter >= rsltlimit))
          return FALSE;
        if (dirbias < 0)
          nextdir = left_dir(dir);
        else if (dirbias > 0)
          nextdir = right_dir(dir);
        else
          nextdir = NODIR;
        if (nextdir != NODIR) {
            if (!point_in_dir(x, y, nextdir, &nx, &ny))
              nextdir = NODIR;
            if ((nextdir != NODIR)
                && limited_search_under_arc(nx, ny, dir, range, dirbias, pred, 
                                            counter, rsltlimit, parambox))
              return TRUE;
            if (counter && (*counter >= rsltlimit))
              return FALSE;
        }
    }
    return FALSE;
}

/* Search under an arc between one directional ray and a neighboring 
   directional ray. */
int
search_under_arc(int x, int y, int dir, int range, int dirbias,
                 int (*pred)(int x, int y, int *counter, ParamBox *parambox), 
                 ParamBox *parambox)
{
    return limited_search_under_arc(x, y, dir, range, dirbias, pred, NULL, 
                                    INT_MAX, parambox);
}

/* Apply a function all along a path. */

void
apply_to_path(int fx, int fy, int tx, int ty,
	      int (*dirtest)(int x, int y, int dir),
	      int (*dirsort)(int x, int y, int *dirchoices, int numchoices),
	      int (*fn)(int x, int y, int dir, int j, int numchoices),
	      int shortest)
{
    int i = 500, j, x = fx, y = fy, moved;
    int dx, dxa, dy, dirchoices[NUMDIRS], axis, hextant, sig;
    int d1, d2, d3, d4;
    int numchoices, shortestnumchoices;

    while (!(x == tx && y == ty) && i-- > 0 /* safety */) {
	dx = tx - x;  dy = ty - y;
	/* If in a wrapping world, choose the shortest of directions. */
	if (area.xwrap) {
	    dxa = (tx + area.width) - fx;
	    if (ABS(dx) > ABS(dxa))
	      dx = dxa;
	    dxa = (tx - area.width) - fx;
	    if (ABS(dx) > ABS(dxa))
	      dx = dxa;
	}
	/* Figure out the axis or hextant of this delta. */
	axis = hextant = -1;
	/* Decode the delta values. */
	if (dx == 0) {
	    axis = (dy > 0 ? NORTHEAST : SOUTHWEST);
	} else if (dy == 0) {
	    axis = (dx > 0 ? EAST : WEST);
	} else if (dx == (0 - dy)) {
	    axis = (dy > 0 ? NORTHWEST : SOUTHEAST);
	} else if (dx > 0) {
	    hextant = (dy > 0 ? EAST :
		       (ABS(dx) > ABS(dy) ? SOUTHEAST : SOUTHWEST));
	} else {
	    hextant = (dy < 0 ? WEST :
		       (ABS(dx) > ABS(dy) ? NORTHWEST : NORTHEAST));
	}
	numchoices = 0;
	/* On an axis, there's no choice. */
	if (axis >= 0) {
	    d1 = d2 = axis;
	    if (dirtest == NULL || (*dirtest)(x, y, d1))
	      dirchoices[numchoices++] = axis;
	}
	/* Two choices in the middle of a hextant. */
	if (hextant >= 0) {
	    d1 = left_dir(hextant);
	    d2 = hextant;
	    if (dirtest == NULL || (*dirtest)(x, y, d1))
	      dirchoices[numchoices++] = d1;
	    if (dirtest == NULL || (*dirtest)(x, y, d2))
	      dirchoices[numchoices++] = d2;
	}
	/* Sort the choices if requested. */
	if (numchoices > 1 && dirsort != NULL)
	  numchoices = (*dirsort)(x, y, dirchoices, numchoices);
	/* Try each of the directions. */
	if (!inside_area(x, y))
	  return;
	moved = FALSE;
	for (j = 0; j < numchoices; ++j) {
	    sig = (*fn)(x, y, dirchoices[j], j, numchoices);
	    if (sig > 0) {
		/* It's cool - go with this dir. */
		/* Jump along to the new spot on the path. */
		x += dirx[dirchoices[j]];  y += diry[dirchoices[j]];
		moved = TRUE;
		/* Out of the loop. */
		break;
	    } else if (sig < 0) {
		/* Stop applying to the path right now. */
		return;
	    } else {
		/* Try another. */
	    }
	}
	/* If we don't have to pick a shortest path, we have two more
	   directions to try if we need to. */
	if (!moved && !shortest) {
	    shortestnumchoices = numchoices;
	    d3 = left_dir(d1);
	    d4 = right_dir(d2);
	    if (dirtest == NULL || (*dirtest)(x, y, d3))
	      dirchoices[numchoices++] = d3;
	    if (dirtest == NULL || (*dirtest)(x, y, d4))
	      dirchoices[numchoices++] = d4;
	    if (numchoices > shortestnumchoices + 1 && dirsort != NULL)
	      (*dirsort)(x, y, dirchoices + shortestnumchoices, numchoices - shortestnumchoices);
	    /* Try each of the directions. */
	    for (j = shortestnumchoices; j < numchoices; ++j) {
		sig = (*fn)(x, y, dirchoices[j], j, numchoices - shortestnumchoices);
		if (sig > 0) {
		    /* It's cool - go with this dir. */
		    /* Jump along to the new spot on the path. */
		    x += dirx[dirchoices[j]];  y += diry[dirchoices[j]];
		    /* Out of the loop. */
		    break;
		} else if (sig < 0) {
		    /* Stop applying to the path right now. */
		    return;
		} else {
		    /* Try another. */
		}
	    }
	}
    }
}

#if 0 /* currently unused */
/* Find a path between the two given points. */

/* The chooser function gets passed an small array for directions;
   it is expected to fill it with directions sorted in order of
   preference, and to return the number of directions it found. */

/* (the chooser also needs to respect already-marked cells) */
/* (marking should account for all directions in?) */
/* (should return a "figure of merit" sometimes) */
/* (main prog should test vicinity of dest, might not be reachable
   anyway, but maybe should have "reach within n cells") */

int
find_path(fx, fy, tx, ty, chooser, maxwps, waypoints, numwpsp)
int fx, fy, tx, ty, maxwps, *numwpsp;
int (*chooser)(int, int, int, int, int *);
Waypoint *waypoints;
{
    int ndirs, trythese[NUMDIRS], i;
    int x1, y1, x2, y2;

    if (fx == fy && tx == ty) {
	return TRUE;
    }
    ndirs = (*chooser)(x1, y1, x2, y2, trythese);
    if (ndirs == 0) {
	/* We're totally blocked. */
	return FALSE;
    } else {
	for (i = 0; i < ndirs; ++i) {
	    /* try this direction with find_path_aux */
	}
    }
    return FALSE;
}
#endif

int
search_straight_line(int x0, int y0, int x1, int y1, int (*pred)(int, int),
		     int *rxp, int *ryp)
{
    int i, dist, x, y, xw;

    dist = distance(x0, y0, x1, y1);
    if (dist <= 1)
      return FALSE;
    for (i = 1; i < dist; ++i) {
	/* Interpolate to get an x,y */
	if (x0 == x1)
	  x = x0;
	else
	  x = x0 + (i * (x1 - x0)) / dist;
	if (y0 == y1)
	  y = y0;
	else
	  y = y0 + (i * (y1 - y0)) / dist;
	xw = wrapx(x);
	if (inside_area(x, y) && (*pred)(xw, y)) {
	    *rxp = xw;  *ryp = y;
	    return TRUE;
	}
    }
    return FALSE;
}

/* For now, set a bit on both sides of a border. */

void
set_border_at(int x, int y, int dir, int t, int onoff)
{
    int ox, oy, bord, obord;
    int odir = opposite_dir(dir);

    if (!t_is_border(t))
      return;
    if (!point_in_dir(x, y, dir, &ox, &oy)) {
	run_warning("border on outside of world at %d,%d, can't set", x, y);
	return;
    }
    allocate_area_aux_terrain(t);
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    bord = aux_terrain_at(x, y, t);
    bord = ((onoff << dir) | (bord & ~(1 << dir)));
    set_aux_terrain_at(x, y, t, bord);
    /* Go to the other cell and tweak its border bits. */
    obord = aux_terrain_at(ox, oy, t);
    obord = ((onoff << odir) | (obord & ~(1 << odir)));
    set_aux_terrain_at(ox, oy, t, obord);
}

/* For now, set a bit on both sides of a connection. */

void
set_connection_at(int x, int y, int dir, int t, int onoff)
{
    int ox, oy, conn, oconn;
    int odir = opposite_dir(dir);

    if (!t_is_connection(t))
      return;
    if (!point_in_dir(x, y, dir, &ox, &oy)) {
	run_warning("connection to outside of world at %d,%d, can't set",
		    x, y);
	return;
    }
    allocate_area_aux_terrain(t);
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    conn = aux_terrain_at(x, y, t);
    conn = ((onoff << dir) | (conn & ~(1 << dir)));
    set_aux_terrain_at(x, y, t, conn);
    /* Go to the other cell and tweak its connection bits. */
    oconn = aux_terrain_at(ox, oy, t);
    oconn = ((onoff << odir) | (oconn & ~(1 << odir)));
    set_aux_terrain_at(ox, oy, t, oconn);
}

/* If there might be any inconsistencies in borders or connections,
   this fixes them.  Basically this just detects if a bit is set on
   either side, and sets the bits on both sides if so. */

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

/* Make space to record named features. */

void
init_features(void)
{
    if (features_defined())
      return;
    featurelist = last_feature = NULL;
    numfeatures = 0;
    area.features = malloc_area_layer(short);
    /* No need to fill layer with default value, as long as feature 0
       means "no feature". */
}

Feature *
create_feature(char *feattype, char *name)
{
    Feature *newfeature = (Feature *) xmalloc(sizeof(Feature));

    /* If we're going to have a feature, we need to have the layer
       too.  init_features clears other data, which is why we do it at
       the beginning of this function. */
    if (!features_defined())
      init_features();
    newfeature->id = nextfid++;
    newfeature->feattype = feattype;
    newfeature->name = name;
    /* Add to the end of the feature list. */
    if (last_feature != NULL) {
    	last_feature->next = newfeature;
    } else {
    	featurelist = newfeature;
    }
    last_feature = newfeature;
    ++numfeatures;
    return newfeature;
}

/* Given a feature id, find the corresponding object. */

Feature *
find_feature(int fid)
{
    Feature *feature;

    /* Feature id of 0 is always a non-feature. */
    if (fid == 0)
      return NULL;
    for_all_features(feature) {
	if (feature->id == fid)
	  return feature;
    }
    return NULL;
}

/* Return the feature object at the given location. */

Feature *
feature_at(int x, int y)
{
    int fid;

    if (!features_defined())
      return NULL;

    fid = raw_feature_at(x, y);
    return find_feature(fid);
}

/* Set the type name of the feature. */

void
set_feature_type_name(Feature *feature, char *feattype)
{
    if (feature == NULL)
      return;
    feature->feattype = copy_string(feattype);
    /* (should ping all displays) */
}

/* Set the name of the feature. */

void
set_feature_name(Feature *feature, char *name)
{
    if (feature == NULL)
      return;
    feature->name = copy_string(name);
    /* (should ping all displays) */
}

/* Given a feature object, make it go away. */

void
destroy_feature(Feature *feature)
{
    int x, y, oldfid, fid;
    Feature *tmp, *prev;

    if (feature == NULL)
      return; /* (should be error?) */
    oldfid = feature->id;
    if (feature == featurelist) {
	featurelist = feature->next;
	if (feature == last_feature)
	  last_feature = NULL;
    } else {
	prev = NULL;
	for_all_features(tmp) {
    	    if (tmp == feature) {
		if (prev != NULL)
		  prev->next = tmp->next;
		break;
	    }
	    prev = tmp;
	}
	if (feature == last_feature)
	  last_feature = prev;
    }
    --numfeatures;
    /* Clear out any uses in the features layer. */
    for_all_cells(x, y) {
	fid = raw_feature_at(x, y);
	if (fid == oldfid)
	  set_raw_feature_at(x, y, 0);
    }
    /* (should dealloc also?) */
}

/* Renumber everything in the layer according to the order in which
   it appears in the list of features. */
/* (should be able to call this from designer tools) */

void
renumber_features(void)
{
    int newfid = 1, maxoldfid = 0, x, y;
    short *newlabels;
    Feature *feature;

    if (!features_defined())
      return;
    for_all_features(feature) {
    	maxoldfid = max(maxoldfid, feature->id);
    	feature->relabel = newfid++;
    }
    if (maxoldfid > 1000)
      return; /* don't risk it */
    newlabels = (short *) xmalloc((maxoldfid + 1) * sizeof(short));
    for_all_features(feature) {
    	newlabels[feature->id] = feature->relabel;
    }
    /* Apply the new labels to everything in the layer. */
    for_all_cells(x, y) {
    	set_raw_feature_at(x, y, newlabels[raw_feature_at(x, y)]);
    }
    for_all_features(feature) {
    	feature->id = feature->relabel;
    }
    free(newlabels);
}

/* A feature's centroid is the closest it has to an actual center;
   this function computes for all features. */

void
compute_all_feature_centroids(void)
{
    compute_feature_centroid(NULL);
}

/* The same, but for a single feature.  This would be very inefficient
   to call repeatedly for each feature in the world. */

void
compute_feature_centroid(Feature *feature)
{
    int x, y;
    Feature *tmpfeature;

    /* Only do this if features and a feature layer to work with. */
    if (featurelist == NULL || !features_defined())
      return;
    /* Either clear all features or just the one passed in. */
    for_all_features(tmpfeature) {
	if (feature == NULL || tmpfeature == feature) {
	    tmpfeature->size = 0;
	    tmpfeature->x = tmpfeature->y = 0;
	    tmpfeature->xmin = tmpfeature->ymin = area.maxdim + 1;
	    tmpfeature->xmax = tmpfeature->ymax = -1;
	}
    }
    /* Iterate over all cells. */
    for_all_cells(x, y) {
	if (feature) {
	    if (feature->id == raw_feature_at(x, y))
	      tmpfeature = feature;
	    else
	      tmpfeature = NULL;
	} else {
	    tmpfeature = feature_at(x, y);
	}
	if (tmpfeature != NULL) {
	    ++(tmpfeature->size);
	    tmpfeature->x += x;
	    tmpfeature->y += y;
	    tmpfeature->xmin = min(x, tmpfeature->xmin); 
	    tmpfeature->ymin = min(y, tmpfeature->ymin);
	    tmpfeature->xmax = max(x, tmpfeature->xmax); 
	    tmpfeature->ymax = max(y, tmpfeature->ymax);
	}
    }
    for_all_features(tmpfeature) {
	if (feature == NULL || tmpfeature == feature) {
	    if (tmpfeature->size > 0) {
		tmpfeature->x = tmpfeature->x / tmpfeature->size;
		tmpfeature->y = tmpfeature->y / tmpfeature->size;
	    }
	}
    }
}

int
num_people_at(int x, int y)
{
    int m, num;

    num = 0;
    for_all_material_types(m) {
	if (cell_material_defined(m)) {
	    num += material_at(x, y, m) * m_people(m);
	}
    }
    return num;
}

/* Compute the coords of a point in the given direction. */

int
point_in_dir(int x, int y, int dir, int *xp, int *yp)
{
    *xp = wrapx(x + dirx[dir]);  *yp = y + diry[dir];
    return (in_area(*xp, *yp));
}

/* Compute the coords of a point in the given direction, but flag
   points not inside the area. */

int
interior_point_in_dir(int x, int y, int dir, int *xp, int *yp)
{
    *xp = wrapx(x + dirx[dir]);  *yp = y + diry[dir];
    return (inside_area(*xp, *yp));
}

/* Compute the point in a direction, n cells away. */

int
point_in_dir_n(int x, int y, int dir, int n, int *xp, int *yp)
{
    *xp = wrapx(x + n * dirx[dir]);  *yp = y + n * diry[dir];
    return (in_area(*xp, *yp));
}

int
interior_point_in_dir_n(int x, int y, int dir, int n, int *xp, int *yp)
{
    *xp = wrapx(x + n * dirx[dir]);  *yp = y + n * diry[dir];
    return (inside_area(*xp, *yp));
}

/* Return a random point guaranteed inside the area. */

int
random_point(int *xp, int *yp)
{
    int tries = 500;

    while (tries-- > 0) {
	*xp = xrandom(area.width);  *yp = xrandom(area.height - 2) + 1;
	if (inside_area(*xp, *yp))
	  return TRUE;
    }
    return FALSE;
}

/* Return a random point guaranteed on the edge of the area. */

int
random_edge_point(int *xp, int *yp)
{
    int tries = 500, adjustedwidth, ratio, val;

    while (tries-- > 0) {
	if (area.xwrap) {
	    *xp = xrandom(area.width);
	    *yp = (flip_coin() ? 0 : area.height - 1);
	} else {
	    adjustedwidth = area.width - area.halfheight;
	    ratio = (adjustedwidth * 100) / (area.width + area.halfheight);
	    if (probability(ratio)) {
		/* Pick along top or bottom edge. */
		if (flip_coin()) {
		    /* Pick along bottom edge. */
		    *xp = xrandom(adjustedwidth) + area.halfheight;
		    *yp = 0;
		} else {
		    /* Pick along top edge. */
		    *xp = xrandom(adjustedwidth);
		    *yp = area.height - 1;
		}
	    } else {
		/* Pick along right or left side. */
		if (flip_coin()) {
		    /* Pick along right side. */
		    if (flip_coin()) {
			/* Pick along upper right side. */
			val = xrandom(area.halfheight);
			*xp = area.width - val;
			*yp = area.halfheight + val;
		    } else {
			/* Pick along lower right side. */
			*xp = area.width;
			*yp = xrandom(area.halfheight);
		    }
		} else {
		    /* Pick along left side. */
		    if (flip_coin()) {
			/* Pick along upper left side. */
			*xp = 0;
			*yp = area.halfheight + xrandom(area.halfheight);
		    } else {
			/* Pick along lower left side. */
			val = xrandom(area.halfheight);
			*xp = val;
			*yp = area.halfheight - val;
		    }
		}
	    }
	}
	if (in_area(*xp, *yp) && !inside_area(*xp, *yp))
	  return TRUE;
    }
    return FALSE;
}

/* Return a random point guaranteed to be within a given radius of
   a given point. */

int
random_point_near(int cx, int cy, int radius, int *xp, int *yp)
{
    int tries = 500;

    if (radius <= 0)
      return FALSE;
    while (tries-- > 0) {
	*xp = cx + xrandom(2 * radius + 1) - radius;
	*yp = cy + xrandom(2 * radius + 1) - radius;
	if (inside_area(*xp, *yp)
	    && distance(cx, cy, *xp, *yp) <= radius)
	      return TRUE;
    }
    return FALSE;
}

/* Return a random point guaranteed to be within a given radius of
   a given point. */

int
random_point_in_area(int cx, int cy, int rx, int ry, int *xp, int *yp)
{
    int tries = 500;

    if (rx < 0)
      rx = 0;
    if (ry < 0)
      ry = 0;
    /* If no variation allowed, just return the center directly. */
    if (rx == 0 && ry == 0) {
	*xp = cx;
	*yp = cy;
	return inside_area(*xp, *yp);
    }
    while (tries-- > 0) {
	*xp = cx + xrandom(2 * rx + 1) - rx;
	*yp = cy + xrandom(2 * ry + 1) - ry;
	if (inside_area(*xp, *yp)
	    && distance(cx, cy, *xp, *yp) <= max(rx - ry, ry - rx))
	  return TRUE;  /* (should fix test?) */
    }
    return FALSE;
}

/* Generic warning that a terrain subtype is incorrect. */

void
terrain_subtype_warning(char *context, int t)
{
    run_warning("In %s: Garbage t%d (%s) subtype %d",
		context, t, t_type_name(t), t_subtype(t));
}

/* Given a vector, return the direction that best approximates it. */

int
approx_dir(int dx, int dy)
{
    if (dx == 0) {
	if (dy == 0)
	  return -1; /* should flag so can use special cursor */
	if (dy > 0)
	  return NORTHEAST;
	return SOUTHWEST;
    } else if (dx > 0) {
    	/* Check for the axes first. */
	if (dy == 0)
	  return EAST;
	if (dy == (-dx))
	  return SOUTHEAST;
	if (dy > dx)
	  return NORTHEAST;
	if ((-dy) <= dx / 2)
	  return EAST;
	if ((-dy) < dx * 2)
	  return SOUTHEAST;
	return SOUTHWEST;
    } else {
    	/* Check for the axes first. */
	if (dy == 0)
	  return WEST;
    	if (dy == (-dx))
    	  return NORTHWEST;
	if (dy > (-dx) * 2)
	  return NORTHEAST;
	if (dy >= (-dx) / 2)
	  return NORTHWEST;
	if (dy > dx)
	  return WEST;
	return SOUTHWEST;
    }
}

/* Computing distance in a hexagonal system is a little peculiar,
   since it's sometimes just delta x or y, and other times is the sum.
   Basically there are six formulas to compute distance, depending on
   the direction between the two points.  If the area wraps, this
   routine reports the shortest distance. */

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

/* Compute distance in the world, irrespective of area size.  The
   result of this function is undefined if the world circumference
   is 0. (why?) */

int
world_distance(int x1, int y1, int x2, int y2)
{
    int circumf, dx = x2 - x1, dy = y2 - y1;

    circumf = world.circumference;
    /* Choose the shortest way around the world. */
    dx = (dx < 0 ? (dx < 0 - circumf / 2 ? circumf + dx : dx)
	         : (dx > 0 + circumf / 2 ? dx - circumf : dx));
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

/* Find the direction matching the given x and y, return -1 if no
   match.  Callers should be careful to test for this! */

int
closest_dir(int x, int y)
{
    int dir;

    for_all_directions(dir) {
	if (dirx[dir] == x && diry[dir] == y)
	  return dir;
    }
    return -1;
}

/* 360 degrees * 60 minutes */

#define MINUTES (21600)

/* Given an x,y and position within a cell, compute the latitude and
   longitude in arc minutes. */

void
xy_to_latlong(int x, int y, int xf, int yf, int *latp, int *lonp)
{
    int lat, lon, center;
    float fx1, fy1, ratio;

    /* The world must be considered to be spherical. */
    if (world.circumference <= 0) {
	*latp = *lonp = 0;
	return;
    }
    fy1 = y + area.latitude - area.halfheight + ((float) yf) / 1000;
    /* Convert to arc minutes. */
    lat = (int)(fy1 * MINUTES) / world.circumference;
    center = (1000 * (area.width + area.halfheight - y)) / 2 - 500;
    fx1 = 1000 * x + xf;
    fx1 -= center;
    if (area.projection == 1) {
	/* Compute compression ratio due to latitude. */
	/* The magic value is 2 pi / MINUTES. */
	ratio = cos(abs(lat) * 0.000290888);
	fx1 = fx1 / ratio;
    }
    /* Offset by the longitude. */
    fx1 += 1000 * area.longitude;
    /* Convert to arc minutes. */
    lon = (int)(fx1 * MINUTES) / world.circumference;
    lon /= 1000;
    /* Push the longitude into the range -180 to 180 degrees. */
    if (lon > MINUTES / 2)
      lon -= MINUTES;
    if (lon <= (- MINUTES / 2))
      lon += MINUTES;
    *latp = lat;  *lonp = lon;
}

/* Given latitude/longitude in arc minutes, compute the cell and position
   within the cell, in 1/1000ths of the cell. */

int
latlong_to_xy(int lat, int lon, int *xp, int *yp, int *xfp, int *yfp)
{
    int x1, y1, center;
    float fx1, fy1, ratio;

    /* The world must be considered to be spherical. */
    if (world.circumference <= 0) {
	return FALSE;
    }
    /* Convert latitude to 1000ths of a cell. */
    fy1 = (((float) lat) * world.circumference) / MINUTES;
    y1 = (int)(fy1 * 1000);
    /* Latitude of 0 passes through the center of the area. */
    *yp = y1 / 1000 - area.latitude + area.halfheight;
    *yfp = y1 % 1000;

    /* Convert longitude to cell. */
    /* Only do this for wrapped areas, or it will prevent drawing of 
    the Western hemisphere meridians on the Mac. */
    if (lon <= 0 && area.xwrap)
      lon += MINUTES;
    fx1 = (((float) lon) * world.circumference) / MINUTES;
    fx1 -= area.longitude;
    if (area.projection == 1) {
	/* Compute compression ratio due to latitude. */
	/* The magic value is 2 pi / MINUTES. */
	ratio = cos(abs(lat) * 0.000290888);
	/* Compute x by compressing longitude towards center of the area. */
	fx1 = fx1 * ratio;
    }
    x1 = (int)(fx1 * 1000);
    /* Compute the effect of obliqueness on x. */
    center = (1000 * (area.width + area.halfheight - *yp)) / 2 - 500;
    x1 += center;
    /* Convert to cell and cell fraction. */
    *xp = x1 / 1000;
    *xfp = x1 % 1000;
    if (*xp < 0)
      *xp += world.circumference;
    return TRUE;
}

void add_neighbor(TRegion *reg1, TRegion *reg2);

int region_value(int x, int y, int landsea);

/* The landsea flag tells us that we only want to consider if terrain
is liquid or not when making regions. */

int
region_value(int x, int y, int landsea)
{
 	if (landsea) {
		return (t_liquid(terrain_at(x, y)) ? 0 : 1);
	} else {
		return (terrain_at(x, y));
	}
}

void
divide_into_regions(char *tlayer, TRegion **rlayer, int landsea)
{
    int x, y, dir, x1, y1, i;
    TRegion *region, *reg1, *reg2;

    if (landsea) {
	    landsea_region_list = NULL;
	    num_landsea_regions = 0;
    } else {
	    terrain_region_list = NULL;
	    num_terrain_regions = 0;
    }
    /* We need to know the exact order of scanning, so can't use standard
       iteration macro. */
    for (y = 1; y < area.height - 1; ++y) {
	for (x = 0; x < area.width; ++x) {
	    if (inside_area(x, y)) {
		if (interior_point_in_dir(x, y, WEST, &x1, &y1)
		    && region_value(x, y, landsea) == region_value(x1, y1, landsea)) {
		    region = aref(rlayer, x1, y1);
		} else if (interior_point_in_dir(x, y, SOUTHWEST, &x1, &y1)
			   && region_value(x, y, landsea) == region_value(x1, y1, landsea)) {
		    region = aref(rlayer, x1, y1);
		} else if (interior_point_in_dir(x, y, SOUTHEAST, &x1, &y1)
			   && region_value(x, y, landsea) == region_value(x1, y1, landsea)) {
		    region = aref(rlayer, x1, y1);
		} else {
		    region = NULL;
		}
		/* Create a new region if no matches found. */
		if (region == NULL) {
		    region = (TRegion *) xmalloc(sizeof(TRegion));
		    region->ttype = region_value(x, y, landsea);
		    region->xmin = region->ymin = 10000;
		    region->xmax = region->ymax = -1;
		    if (landsea) {
			    region->next = landsea_region_list;
			    landsea_region_list = region;
			    ++num_landsea_regions;
		    } else {
			    region->next = terrain_region_list;
			    terrain_region_list = region;
			    ++num_terrain_regions;
		    }
		}
		/* Add the point to the region. */
		aset(rlayer, x, y, region);
		++(region->size);
		region->xmin = min(x, region->xmin);
		region->ymin = min(y, region->ymin);
		region->xmax = max(x, region->xmax);
		region->ymax = max(y, region->ymax);
	    }
	}
    }
    /* We need to iterate the process several times in order to make sure
    that all regions that should be merged really are merged. A region of 
    type 1 may border on second region of type 1 which borders on a third
    region of type 1. Only the first two regions will then get merged in the 
    first pass. */
    for (i = 0; i < 10; i++) {
	    for_all_interior_cells(x, y) {
		for_all_directions(dir) {
		    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			if (region_value(x, y, landsea) == region_value(x1, y1, landsea)) {
			    reg1 = aref(rlayer, x, y);
			    reg2 = aref(rlayer, x1, y1);
			    if (reg1 != reg2) {
				if (reg1->size > reg2->size)
				  reg2->merge = reg1;
				else if (reg1->size > reg2->size)
				  reg1->merge = reg2;
			    }
			}
		    }
		}
	    }
	    for_all_interior_cells(x, y) {
		reg1 = aref(rlayer, x, y);
		reg2 = reg1->merge;
		if (reg2 != NULL) {
		    aset(rlayer, x, y, reg2);
		    ++(reg2->size);
		    reg2->xmin = min(x, reg2->xmin);  reg2->ymin = min(y, reg2->ymin);
		    reg2->xmax = max(x, reg2->xmax);  reg2->ymax = max(y, reg2->ymax);
		    --(reg1->size);
		}
	    }
    }
    for_all_interior_cells(x, y) {
	reg1 = aref(rlayer, x, y);
	for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		reg2 = aref(rlayer, x1, y1);
		if (reg1 != reg2) {
		    add_neighbor(reg1, reg2);
		    add_neighbor(reg2, reg1);
		}
	    }
	}
    }
}

void
add_neighbor(TRegion *reg1, TRegion *reg2)
{
    TRegionLink *link;

    for (link = reg1->neighbors; link != NULL; link = link->next) {
	/* If already listed as neighbor, we're done. */
	if (reg2 == link->neighbor)
	  return;
    }
    link = (TRegionLink *) xmalloc(sizeof(TRegionLink));
    link->neighbor = reg2;
    link->next = reg1->neighbors;
    reg1->neighbors = link;
}

/* (should put internal date-handling code here) */

#ifdef DESIGNERS

static void paint_cell_1(int x, int y);
static void paint_coating_1(int x, int y);
static void paint_people_1(int x, int y);
static void paint_control_1(int x, int y);
static void paint_feature_1(int x, int y);
static void paint_elevation_1(int x, int y);
static void paint_temperature_1(int x, int y);
static void paint_material_1(int x, int y);
static void paint_clouds_1(int x, int y);
static void paint_winds_1(int x, int y);

extern void fix_elevations(void);

/* Cell painting. */

void
paint_cell(Side *side, int x, int y, int r, int t)
{
    tmpside = side;
    tmpttype = t;
    apply_to_area_plus_edge(x, y, r, paint_cell_1);
}

static void
paint_cell_1(int x, int y)
{
    /* Only do anything if we're actually changing to a different type. */
    if (terrain_at(x, y) != tmpttype) {
	set_terrain_at(x, y, tmpttype);
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

/* Border/connection painting. */

void
paint_border(Side *side, int x, int y, int dir, int t, int mode)
{
    int oldbord, x1, y1;

    if (!inside_area(x, y))
      return;
    allocate_area_aux_terrain(t);
    oldbord = border_at(x, y, dir, t);
    set_border_at(x, y, dir, t, (mode < 0 ? !oldbord : mode));
    if (oldbord != border_at(x, y, dir, t)) {
	update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
	/* We need to do this because UPDATE_ADJ is a hint, not a
	   requirement to redraw adjacent cells. */
	if (point_in_dir(x, y, dir, &x1, &y1))
	  update_cell_display(side, x1, y1, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

void
paint_connection(Side *side, int x, int y, int dir, int t, int mode)
{
    int oldconn, x1, y1;

    if (!inside_area(x, y))
      return;
    allocate_area_aux_terrain(t);
    oldconn = connection_at(x, y, dir, t);
    set_connection_at(x, y, dir, t, (mode < 0 ? !oldconn : mode));
    if (oldconn != connection_at(x, y, dir, t)) {
	update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
	/* We need to do this because UPDATE_ADJ is a hint, not a
	   requirement to redraw adjacent cells. */
	if (point_in_dir(x, y, dir, &x1, &y1))
	  update_cell_display(side, x1, y1, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

/* Coating painting. */

void
paint_coating(Side *side, int x, int y, int r, int t, int depth)
{
    allocate_area_aux_terrain(t);
    tmpside = side;
    tmpttype = t;
    tmpint = depth;
    apply_to_area_plus_edge(x, y, r, paint_coating_1);
}

static void
paint_coating_1(int x, int y)
{
    int olddepth = aux_terrain_at(x, y, tmpttype);

    if (olddepth != tmpint) {
	set_aux_terrain_at(x, y, tmpttype, tmpint);
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

/* Painting of people sides. */

void
paint_people(Side *side, int x, int y, int r, int s)
{
    allocate_area_people_sides();
    tmpside = side;
    tmpint = s;
    apply_to_area(x, y, r, paint_people_1);
}

static void
paint_people_1(int x, int y)
{
    int oldpeop = people_side_at(x, y);

    if (oldpeop != tmpint) {
	set_people_side_at(x, y, tmpint);
	/* Appearance of adjacent cells may change also. */
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);

    }
}

/* Painting of controlling side. */

void
paint_control(Side *side, int x, int y, int r, int s)
{
    allocate_area_control_sides();
    tmpside = side;
    tmpint = s;
    apply_to_area(x, y, r, paint_control_1);
}

static void
paint_control_1(int x, int y)
{
    int oldpeop = control_side_at(x, y);

    if (oldpeop != tmpint) {
	set_control_side_at(x, y, tmpint);
	/* Appearance of adjacent cells may change also. */
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

/* Painting of geographical features.  A feature id of 0 directs feature
   removal from the given area. */

void
paint_feature(Side *side, int x, int y, int r, int f)
{
    init_features();
    tmpside = side;
    tmpint = f;
    tmpfeature = find_feature(f);
    apply_to_area(x, y, r, paint_feature_1);
}

static void
paint_feature_1(int x, int y)
{
    int oldfid = raw_feature_at(x, y);
    Feature *oldfeature;

    if (oldfid != tmpint) {
	set_raw_feature_at(x, y, tmpint);
	if (tmpfeature != NULL) {
	    ++(tmpfeature->size);
	    /* too expensive while painting */
	    /* compute_feature_centroid(tmpfeature); */
	}
    	if (oldfid != 0) {
	    oldfeature = find_feature(oldfid);
	    if (oldfeature != NULL) {
		--(oldfeature->size);
		/* compute_feature_centroid(oldfeature); */
	    }
	}
	/* Might need to redraw feature borders in adjacent cells. */
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

/* Painting of terrain elevations. */

void
paint_elevation(Side *side, int x, int y, int r, int code, int elev, int vary)
{
    allocate_area_elevations();
    tmpside = side;
    tmpint = code;
    tmpint2 = elev;
    tmpint3 = vary;
    apply_to_area_plus_edge(x, y, r, paint_elevation_1);
}

static void
paint_elevation_1(int x, int y)
{
    int oldelev = elev_at(x, y), newelev;

    /* Decide whether we're setting or adjusting the elevation. */
    switch (tmpint) {
      case 0:
	newelev = tmpint2;
	break;
      case 1:
	newelev = oldelev + tmpint2;
	break;
      default:
	run_warning("bogus elevation paint code");
	newelev = 0;
	break;
    }
    /* Add an optional random variation if desired. */
    if (tmpint3 > 0)
      newelev += xrandom(tmpint3 * 2 + 1) - tmpint3;
    if (newelev != oldelev) {
	set_elev_at(x, y, newelev);
	if (newelev < area.minelev)
	  area.minelev = newelev;
	if (newelev > area.maxelev)
	  area.maxelev = newelev;
	/* Note that the contour intervals may now no longer be
	   reasonable, but since redoing them will probably require a
	   full map update, don't adjust them automatically here. */
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

void
paint_temperature(Side *side, int x, int y, int r, int temp)
{
    allocate_area_temperatures();
    tmpside = side;
    tmpint = temp;
    apply_to_area_plus_edge(x, y, r, paint_temperature_1);
}

static void
paint_temperature_1(int x, int y)
{
    int n, t = terrain_at(x, y), oldtemp = temperature_at(x, y);
    
    n = max(t_temp_min(t), min(tmpint, t_temp_max(t)));
    if (n != oldtemp) {
	set_temperature_at(x, y, n);
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS);
    }
}

/* Material layer painting. */

void
paint_material(Side *side, int x, int y, int r, int m, int amt)
{
    allocate_area_material(m);
    tmpside = side;
    tmpmtype = m;
    tmpint = amt;
    apply_to_area_plus_edge(x, y, r, paint_material_1);
}

static void
paint_material_1(int x, int y)
{
    int oldm = material_at(x, y, tmpmtype);

    if (oldm != tmpint) {
	set_material_at(x, y, tmpmtype, tmpint);
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS);
    }
}

/* Cloud painting is more complicated because up to three separate
   layers are involved. */

void
paint_clouds(Side *side, int x, int y, int r, int cloudtype, int bot, int hgt)
{
    allocate_area_clouds();
    /* (should not always do altitudes) */
    allocate_area_cloud_altitudes();
    tmpside = side;
    tmpint = cloudtype;
    tmpint2 = bot;
    tmpint3 = hgt;
    apply_to_area_plus_edge(x, y, r, paint_clouds_1);
}

static void
paint_clouds_1(int x, int y)
{
    int oldcl = raw_cloud_at(x, y);
    int oldbot = raw_cloud_bottom_at(x, y);
    int oldhgt = raw_cloud_height_at(x, y);
    int changed = FALSE;

    if (oldcl != tmpint) {
	set_raw_cloud_at(x, y, tmpint);
	changed = TRUE;
    }
    if (oldbot != tmpint2) {
	set_raw_cloud_bottom_at(x, y, tmpint2);
	changed = TRUE;
    }
    if (oldhgt != tmpint3) {
	set_raw_cloud_height_at(x, y, tmpint3);
	changed = TRUE;
    }
    if (changed)
      update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
}

/* Winds painting. */

void
paint_winds(Side *side, int x, int y, int r, int dir, int force)
{
    allocate_area_winds();
    tmpside = side;
    tmpint = force << 3 | dir;
    apply_to_area_plus_edge(x, y, r, paint_winds_1);
}

static void
paint_winds_1(int x, int y)
{
    int oldw = raw_wind_at(x, y);

    if (oldw != tmpint) {
	set_raw_wind_at(x, y, tmpint);
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS);
    }
}

/* Scan the elevation layer, putting all values into the ranges
   required by terrain types. */

void
fix_elevations(void)
{
    int x, y, t, oldelev, newelev, numfixed;
    Side *side;

    if (!elevations_defined())
      return;
    numfixed = 0;
    area.maxelev = area.minelev = elev_at(area.width / 2, area.height / 2);
    for_all_interior_cells(x, y) {
	t = terrain_at(x, y);
	oldelev = elev_at(x, y);
	/* Clip desired elevation to what's allowed for the terrain here. */
	newelev = max(t_elev_min(t), min(oldelev, t_elev_max(t)));
	set_elev_at(x, y, newelev);
	/* (should try to share with final_init_world?) */
	if (newelev < area.minelev)
	  area.minelev = newelev;
	if (newelev > area.maxelev)
	  area.maxelev = newelev;
	if (newelev != oldelev)
	  ++numfixed;
    }
    /* Set edge elevations to be averages of adjacent interior
       cells. */
    set_edge_terrain(FALSE);
    /* Let everybody know what we did. */
    notify_all("Fixed %d elevations, range from %d to %d.",
	       numfixed, area.minelev, area.maxelev);
    if (numfixed > 0)
      for_all_sides(side)
	update_area_display(side);
}

#endif /* DESIGNERS */

/* For an advanced unit, toggle its use of the given cell. */

void
toggle_user_at(Unit *unit, int x, int y)
{
    Unit *unit2;
    Side *side2;

    /* Return if landuse is undefined. */
    if (!user_defined())
      return;
    /* Return if the cell is not visible to this side. */
    if (!terrain_visible(unit->side, x, y))
      return;
    /* Return if the cell is used by another unit. */
    if (user_at(x, y) != NOUSER && user_at(x, y) != unit->id) {
	/* (could be more helpful) */
	notify(unit->side, "Cell at %d,%d is already in use", x, y);
	return;
    }
    /* Return if using maxcells and we are trying to add one more. */
    if (unit->usedcells >= unit->maxcells && user_at(x, y) != unit->id) {
	notify(unit->side, "Cannot use any more cells");
	return;
    }
    /* Return if independents or untrusted side has a unit in the cell. */
    if (unit_at(x, y) != NULL) {
	for_all_stack(x, y, unit2) {
	    if (!trusted_side(unit->side, unit2->side))
	      return;
	}
    }
    /* Toggle landuse by unit either on or off for the cell. */
    if (user_at(x, y) == NOUSER) {
	set_user_at(x, y, unit->id);
	unit->usedcells += 1;
    } else if (user_at(x, y) == unit->id) {
	set_user_at(x, y, NOUSER);
	unit->usedcells -= 1;
    }
    for_all_sides(side2) {
	if (side2->see_all
	    || side_sees_unit(side2, unit) 
	    || side_tracking_unit(side2, unit)) {
	    update_cell_display(side2, x, y, UPDATE_ALWAYS);
	}
    }
}

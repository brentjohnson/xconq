/* Terrain generation for Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is the collection of terrain generation methods. */

#include "conq.h"
#include "kernel.h"

extern TRegion *landsea_region_list;
extern TRegion *terrain_region_list;
extern int any_borders_in_dir(int x, int y, int dir);

static void make_blobs(short *layer, int numblobs, int blobradius,
		       int blobalt);
static void limit_layer(short *layer, int hi, int lo);
static void smooth_layer(short *layer, int times);
static void percentile(short *layer, int *percentiles);
static void compose_area(void);
static int terrain_from_percentiles(int x, int y);
static void make_earthlike_fractal_terrain(int calls, int runs);
static void compose_earthlike_area(void);
static int earthlike_terrain_from_percentiles(int x, int y);
static void dig_maze_path(int x1, int y1, int dir1);
static void dig_maze_path_between(int x1, int y1, int x2, int y2);
static int test_maze_segment(int x, int y, int dir);
static int sort_maze_segments(int x, int y, int *dirchoices, int numchoices);
static int lay_maze_segment(int x, int y, int dir, int choice, int numchoices);
static int num_open_adj(int x, int y);
static int random_solid_terrain(void);
static int random_room_terrain(void);
static int random_passage_terrain(void);
static void set_room_interior(int x, int y);
static void name_highest_peaks(char *specificname, Obj *parms);
static int high_point(int x, int y);
static void name_islands(char *specificname, Obj *parms);
static void name_continents(char *specificname, Obj *parms);
static void name_lakes(char *specificname, Obj *parms);
static void name_seas(char *specificname, Obj *parms);
static void name_bays(char *specificname, Obj *parms);
static int bay_point(int x, int y);
static void name_terrain(int t, char *name, Obj *parms);
static char *name_feature_at(int x, int y, char *feattype);
static char *name_feature_using(Obj *namerlist, char *feattype);
static void fix_adjacent_terrain(void);
static void flatten_liquid_terrain(void);
static void set_edge_values(int x, int y, int t);
static void set_edge_elevation(int x, int y);

/* Fractal terrain generation. */

/* The process is actually done for elevation and water separately, then
   the terrain type is derived from looking at both together. */

/* This bounds the range of high and low spots. */

#define MAXALT 4000

/* The following dynamically allocated arrays must be ints, since a area
   may have >32K cells. */

int *histo;             /* histogram array */
int *alts;              /* percentile for each elevation */
int *wets;              /* percentile for level of moisture */

/* Area scratch layers are used as: relief = tmp1, moisture = tmp2,
   aux = tmp3 */

int stepsize = 20;

int partdone;

/* This variable records the number of cells that didn't match any of the
   terrain type percentiles. */

static int numholes;

/* The main function goes through a heuristically-determined process */
/* (read: I dinked until I liked the results) to make a area. */

/* Should add a cheap erosion simulator. */

static void add_random_changes(void);

int
make_fractal_terrain(int calls, int runs)
{
    int actualcells, altnumblobs, altblobradius, altblobalt;
    int wetnumblobs, wetblobradius, wetblobalt;

    /* Don't run if terrain is already present. */
    if (terrain_defined())
      return FALSE;
    /* Note that we may still want this even if only one ttype defined,
       since elevs may still vary usefully. */
    /* Heuristic limit - this algorithm would get weird on small areas */
    if (area.width < 9 || area.height < 9) {
	init_warning("cannot generate fractal terrain for a %d x %d area, must be at least 9x9",
		     area.width, area.height);
	return FALSE;
    }
    Dprintf("Going to make fractal terrain ...\n");
    allocate_area_scratch(3);
    histo  = (int *) xmalloc(MAXALT * sizeof(int));
    alts   = (int *) xmalloc(MAXALT * sizeof(int));
    wets   = (int *) xmalloc(MAXALT * sizeof(int));
    announce_lengthy_process("Making fractal terrain");
    /* Need a rough estimate of how much work involved, so can do progress. */
    if (g_alt_blob_density() > 0) {
	actualcells = (g_alt_blob_size() * area.numcells) / 10000;
	altblobradius = isqrt((actualcells * 4) / 3) / 2;
	altnumblobs = (g_alt_blob_density() * area.numcells) / 10000;
	altblobalt = g_alt_blob_height();
    }
    if (g_wet_blob_density() > 0) {
	actualcells = (g_wet_blob_size() * area.numcells) / 10000;
	wetblobradius = isqrt((actualcells * 4) / 3) / 2;
	wetnumblobs = (g_wet_blob_density() * area.numcells) / 10000;
	wetblobalt = g_wet_blob_height();
    }
    if (g_alt_blob_density() > 0) {
	/* Build a full relief area. */
	partdone = 0;
	make_blobs(area.tmp1, altnumblobs, altblobradius, altblobalt);
	/* Run the requested number of smoothing steps. */
	partdone += stepsize;
	smooth_layer(area.tmp1, g_alt_smoothing());
	percentile(area.tmp1, alts);
    }
    if (g_wet_blob_density() > 0) {
	/* Build a "moisture relief" area. */
	partdone += stepsize;
	make_blobs(area.tmp2, wetnumblobs, wetblobradius, wetblobalt);
	partdone += stepsize;
	smooth_layer(area.tmp2, g_wet_smoothing());
	percentile(area.tmp2, wets);
    }
    /* Put it all together. */
    partdone += stepsize;
    compose_area();
    add_random_changes();
    fix_adjacent_terrain();
    add_edge_terrain();
    flatten_liquid_terrain();
    /* Free up what we don't need anymore. */
    free(histo);
    free(alts);
    free(wets);
    finish_lengthy_process();
    /* Report on the substitutions made. */
    if (numholes > 0) {
	init_warning("no possible terrain for %d cells, made them into %s",
		     numholes, t_type_name(0));
    }
    return TRUE;
}

static void
make_blobs(short *layer, int numblobs, int blobradius, int blobalt)
{
    int x0, y0, x1, y1, x2, y2, updown, x, y;
    int maxdz, i, dz, oz;

    /* Init everything to the middle of the raw altitude range. */
    for_all_cells(x, y)
      aset(layer, x, y, MAXALT/2);
    numblobs = max(1, numblobs);
    maxdz = min(max(1, blobalt), MAXALT/2);
    Dprintf("Making %d blobs of radius %d max-dz %d...\n",
	    numblobs, blobradius, maxdz);
    /* Now lay down blobs. */
    for (i = 0; i < numblobs; ++i) {
	if (i % 100 == 0) {
	    announce_progress(partdone + (stepsize * i) / numblobs);
	}
	/* Decide whether we're making a hole or a hill. */
	updown = (flip_coin() ? 1 : -1);
	/* Pick a center for the bump. */
	random_point(&x0, &y0);
	if (blobradius <= 0) {
	    /* Special case for one-cell blobs. */
	    aadd(layer, x0, y0, updown * maxdz);
	} else {
	    /* Compute the LL corner. */
	    x1 = x0 - blobradius;  y1 = y0 - blobradius;
	    /* Compute the UR corner. */
	    x2 = x0 + blobradius;  y2 = y0 + blobradius;
	    /* Raise/lower all the cells within this bump. */
	    for (y = y1; y <= y2; ++y) {
		for (x = x1; x <= x2; ++x) {
		    if ((x - x1 + y - y1 > blobradius)
			&& (x2 - x + y2 - y > blobradius)) {
			/* skip points outside the area */
			if (in_area(x, y)) {
			    oz = aref(layer, x, y);
			    /* Add some variation within the bump. */ 
			    dz = updown * (maxdz + xrandom(maxdz/2));
			    /* If dz is really extreme, cut it down. */
			    if (!between(0, oz + dz, MAXALT))
			      dz /= 2;
			    if (!between(0, oz + dz, MAXALT))
			      dz /= 2;
			    aset(layer, x, y, oz + dz);
			}
		    }
		}
	    }
	}
    }
    /* Adding and subtracting might have got out of hand. */
    limit_layer(layer, MAXALT-1, 0); 
}

/* Ensure that area values stay within given range. */

static void
limit_layer(short *layer, int hi, int lo)
{
    int x, y, m;
    
    for_all_cells(x, y) {
	m = aref(layer, x, y);
	aset(layer, x, y, max(lo, min(m, hi)));
    }
}

/* Average each cell with its neighbors, using tmp3 as scratch layer. */

static void
smooth_layer(short *layer, int times)
{
    int i, x, y, dir, x1, y1, ndirs, sum;

    for (i = 0; i < times; ++i) {
	Dprintf("Smoothing...\n");
	for (x = 0; x < area.width; ++x) {
	    for (y = 0; y < area.height; ++y) {
		if (in_area(x, y)) {
		    sum = aref(layer, x, y);
		    if (inside_area(x, y) /* and hex geometry */) {
			sum += aref(layer,  x, y + 1);
			sum += aref(layer, x + 1, y);
			sum += aref(layer, x + 1, y - 1);
			sum += aref(layer,  x, y - 1);
			sum += aref(layer, x - 1, y);
			sum += aref(layer, x - 1, y + 1);
			sum /= (NUMDIRS + 1);
		    } else {
			/* Otherwise, use a slower but more general algorithm. */
			ndirs = 0;
			for_all_directions(dir) {
			    if (point_in_dir(x, y, dir, &x1, &y1)) {
				sum += aref(layer, x1, y1);
				++ndirs;
			    }
			}
			if (ndirs > 0)
			  sum /= (ndirs + 1);
		    }
		    set_tmp3_at(x, y, sum);
		}
	    }
	}
	for (x = 0; x < area.width; ++x) {
	    for (y = 0; y < area.height; ++y) {
		aset(layer, x, y, tmp3_at(x, y));
	    }
	}
	announce_progress(partdone + (stepsize * i) / times);
    }
}

/* Terrain types are specified in terms of percentage cover on a area, so
   for instance the Earth is 70% sea.  Since each of several types will have
   its own percentages (both for elevation and moisture), the simplest thing
   to do is to calculate the percentile for each raw elevation and moisture
   value, and save them all away.  */

static void
percentile(short *layer, int *percentiles)
{
    int i, x, y, total;
    
    Dprintf("Computing percentiles...\n");
    limit_layer(layer, MAXALT-1, 0);
    for (i = 0; i < MAXALT; ++i) {
	histo[i] = 0;
	percentiles[i] = 0;
    }
    /* Make the basic histogram, counting only the inside. */
    for_all_interior_cells(x, y) {
	++histo[aref(layer, x, y)];
    }
    /* Integrate over the histogram. */
    for (i = 1; i < MAXALT; ++i)
	histo[i] += histo[i-1];
    /* Total here should actually be same as number of cells in the area */
    total = histo[MAXALT-1];
    /* Compute the percentile position */
    for (i = 0; i < MAXALT; ++i) {
	percentiles[i] = (100 * histo[i]) / total;
    }
}

/* Final creation and output of the area. */

static int *elev_range;
static int *elev_lo;
static int *elev_hi;
static int *raw_range;
static int *raw_lo;
static int *raw_hi;

static void
compose_area(void)
{
    int x, y, t, t2, t3, elev, raw_elev;

    Dprintf("Assigning terrain types to cells...\n");
    if (elev_range == NULL)
      elev_range = (int *) xmalloc(numttypes * sizeof(int));
    if (elev_lo == NULL)
      elev_lo = (int *) xmalloc(numttypes * sizeof(int));
    if (elev_hi == NULL)
      elev_hi = (int *) xmalloc(numttypes * sizeof(int));
    if (raw_range == NULL)
      raw_range = (int *) xmalloc(numttypes * sizeof(int));
    if (raw_lo == NULL)
      raw_lo = (int *) xmalloc(numttypes * sizeof(int));
    if (raw_hi == NULL)
      raw_hi = (int *) xmalloc(numttypes * sizeof(int));
    /* Make the terrain layer itself. */
    allocate_area_terrain();
    numholes = 0;
    for_all_interior_cells(x, y) {
	t = terrain_from_percentiles(x, y);
	set_terrain_at(x, y, t);
    }
    if (any_elev_variation) {
    	/* Compute elevation variations of terrain.  This works on
    	   interior cells only; the edge gets handled later, by
    	   edge-specific code. */
    	if (!elevations_defined()) {
	    allocate_area_elevations();
   	}
    	for_all_terrain_types(t) {
	    raw_lo[t] = MAXALT + 1;
	    raw_hi[t] = -1;
    	}
	for_all_interior_cells(x, y) {
	    t = terrain_at(x, y);
	    raw_lo[t] = min(tmp1_at(x, y), raw_lo[t]);
	    raw_hi[t] = max(tmp1_at(x, y), raw_hi[t]);
	}
	/* If the final elevation ranges for several terrain types
	   overlap, then they should all be considered as a group;
	   this is because the low raw values ought to become low real
	   elevations, and use the same scale as other terrain types
	   whose elevations are in the same or similar ranges.  The
	   calculation below basically iterates over terrain types,
	   and since it is possible for two apparently disjoint ranges
	   to be "joined" later by another range that overlaps both,
	   we must iterate up to as many times as there are terrain
	   types. */
	for_all_terrain_types(t) {
	    elev_lo[t] = t_elev_min(t);
	    elev_hi[t] = t_elev_max(t);
    	    Dprintf("%10.10s: raw1 %6d - %6d, elev1 %6d - %6d\n",
		    t_type_name(t),
		    raw_lo[t], raw_hi[t], elev_lo[t], elev_hi[t]);
	}
	for_all_terrain_types(t) {
	    for_all_terrain_types(t3) {
		for_all_terrain_types(t2) {
		    if (between(elev_lo[t], elev_lo[t2], elev_hi[t])
			|| between(elev_lo[t], elev_hi[t2], elev_hi[t])) {
			elev_lo[t] = min(elev_lo[t], elev_lo[t2]);
			elev_hi[t] = max(elev_hi[t], elev_hi[t2]);
			raw_lo[t] = min(raw_lo[t], raw_lo[t2]);
			raw_hi[t] = max(raw_hi[t], raw_hi[t2]);
		    }
		}
	    }
	}
    	for_all_terrain_types(t) {
    	    raw_range[t] = raw_hi[t] - raw_lo[t];
    	    elev_range[t] = t_elev_max(t) - t_elev_min(t);
    	    Dprintf("%10.10s: raw2 %6d - %6d, elev2 %6d - %6d\n",
		    t_type_name(t),
		    raw_lo[t], raw_hi[t], elev_lo[t], elev_hi[t]);
    	}
 	for_all_interior_cells(x, y) {
	    t = terrain_at(x, y);
	    elev = 0;
	    if (elev_range[t] > 0) {
		if (raw_range[t] > 0) {
		    raw_elev = tmp1_at(x, y);
		    elev = (((raw_elev - raw_lo[t]) * elev_range[t])
			    / raw_range[t]);
		} else {
		    elev = elev_range[t] / 2;
		}
	    }
	    elev += t_elev_min(t);
	    /* Clip elevation to required bounds. */
	    if (elev < t_elev_min(t))
	      elev = t_elev_min(t);
	    if (elev > t_elev_max(t))
	      elev = t_elev_max(t);
	    set_elev_at(x, y, elev);
	}
    }
}

/* Compute the actual terrain types.  This is basically a process of
   checking the percentile limits on each type against what is actually
   there. */

static int
terrain_from_percentiles(int x, int y)
{
    int t, rawalt = tmp1_at(x, y), rawwet = tmp2_at(x, y);

    if (numttypes == 1)
      return 0;
    for_all_terrain_types(t) {
	if (t_is_cell(t)
	    && between(t_alt_min(t), alts[rawalt], t_alt_max(t))
	    && between(t_wet_min(t), wets[rawwet], t_wet_max(t))) {
	    return t;
	}
    }
    /* No terrain maybe not an error, so just count and summarize later. */
    ++numholes;
    return 0;
}

/* Totally random (with weighting) terrain generation, as well as
   random elevations. */

int
make_random_terrain(int calls, int runs)
{
    int t, x, y, cellsum;
    int i, numcells, n, elevrange, elev, occur, dir;

    if (terrain_defined())
      return FALSE;
    announce_lengthy_process("Making random terrain");
    Dprintf("Assigning terrain types...\n");
    cellsum = 0;
    for_all_terrain_types(t) {
	if (t_is_cell(t)) {
	    cellsum += t_occurrence(t);
	}
	tmp_t_array[t] = cellsum;
    }
    allocate_area_terrain();
    /* Overwrite already-defined elevs? */
    if (!elevations_defined() && any_elev_variation)
      allocate_area_elevations();
    i = 0;
    numcells = area.width * area.height;
    for_all_interior_cells(x, y) {
	if (i++ % 100 == 0)
	  announce_progress((i * 100) / numcells);
 	n = xrandom(cellsum + 1);
 	for_all_terrain_types(t) {
 	    if (n <= tmp_t_array[t] && t_is_cell(t))
 	      break;
 	}
	set_terrain_at(x, y, t);
	if (elevations_defined() && any_elev_variation) {
	    elevrange = t_elev_max(t) - t_elev_min(t) + 1;
	    elev = xrandom(elevrange) + t_elev_min(t);
	    set_elev_at(x, y, elev);
	}
    }
    /* Make sure the edge of the area has something in it. */
    add_edge_terrain();
    /* Now maybe add borders and connections. */
    for_all_terrain_types(t) {
	switch (t_subtype(t)) {
	  case cellsubtype:
	    /* do nothing */
	    break;
	  case bordersubtype:
	    occur = t_occurrence(t);
	    if (occur > 0) {
		for_all_interior_cells(x, y) {
		    for_all_directions(dir) {
			if (xrandom(10000) < occur)
			  set_border_at(x, y, dir, t, 1);
		    }
		}
	    }
	    break;
	  case connectionsubtype:
	    occur = t_occurrence(t);
	    if (occur > 0) {
		for_all_interior_cells(x, y) {
		    for_all_directions(dir) {
			if (xrandom(10000) < occur)
			  set_connection_at(x, y, dir, t, 1);
		    }
		}
	    }
	    break;
	  case coatingsubtype:
	    /* (should do something here) */
	    break;
	}
    }
    finish_lengthy_process();
    return TRUE;
}

/* Method that is wired to be as close to earth as possible. */

/* (need to compute scale, get avg elevation and rainfall) */

enum el_ttypes {
    el_sea,
    el_desert,
    el_plains,
    el_forest,
    el_ice,
    numeltypes
};

int seatype = NONTTYPE;
int landtype = NONTTYPE;
int deserttype = NONTTYPE;
int foresttype = NONTTYPE;
int icetype = NONTTYPE;

int coast_distance = 0;

int el_alt_blob_size = 400;
int el_alt_blob_density = 10000;
int el_alt_blob_height = 500;
int el_alt_smoothing = 2;
int el_wet_blob_size = 1000;
int el_wet_blob_density = 200;
int el_wet_blob_height = 100;
int el_wet_smoothing = 10;

int *el_alt_min;
int *el_alt_max;
int *el_wet_min;
int *el_wet_max;

int
make_earthlike_terrain(int calls, int runs)
{
    int t, elevrange;
    int seafrac, desertfrac, forestfrac;

    if (terrain_defined())
      return FALSE;
    if (world.circumference < (area.maxdim * 100)) {
	init_warning("Scale wrong for random earthlike terrain, use a real map");
	return FALSE;
    }
    Dprintf("Categorizing terrain types...\n");
    for_all_terrain_types(t) {
    	if (strcmp("sea", t_type_name(t)) == 0)
    	  seatype = t;
    	if (strcmp("plains", t_type_name(t)) == 0)
    	  landtype = t;
    	if (strcmp("desert", t_type_name(t)) == 0)
    	  deserttype = t;
    	if (strcmp("forest", t_type_name(t)) == 0)
    	  foresttype = t;
    	if (strcmp("ice", t_type_name(t)) == 0)
    	  icetype = t;
    	/* etc */
    }
    if (seatype == NONTTYPE
	|| deserttype == NONTTYPE
	|| landtype == NONTTYPE
	|| foresttype == NONTTYPE
	|| icetype == NONTTYPE) {
    	init_warning("can't find earthlike terrain types");
    	return FALSE;
    }
    announce_lengthy_process("Making Earth-like terrain");
    el_alt_min = (int *) xmalloc(numeltypes * sizeof(int));
    el_alt_max = (int *) xmalloc(numeltypes * sizeof(int));
    el_wet_min = (int *) xmalloc(numeltypes * sizeof(int));
    el_wet_max = (int *) xmalloc(numeltypes * sizeof(int));
    if (coast_distance <= - area.maxdim)
      seafrac = 98;
    else if (coast_distance >= area.maxdim)
      seafrac = 2;
    else
      seafrac = 98 - ((coast_distance + (area.maxdim / 2)) * 100) / area.maxdim;
    seafrac = min(seafrac, 98);
    seafrac = max(seafrac, 2);
    desertfrac = 5;
    forestfrac = 95;
    if (probability(30)) {
	desertfrac = 80;
	forestfrac = 98;
    } else if (probability(30)) {
	desertfrac = 2;
	forestfrac = 60;
    }
    el_alt_min[el_sea] = 0;
    el_alt_max[el_sea] = seafrac;
    el_alt_min[el_desert] = seafrac;
    el_alt_max[el_desert] = 100;
    el_alt_min[el_plains] = seafrac;
    el_alt_max[el_plains] = 100;
    el_alt_min[el_forest] = seafrac;
    el_alt_max[el_forest] = 100;
    el_alt_min[el_ice] = 0;
    el_alt_max[el_ice] = 100;
    el_wet_min[el_sea] = 0;
    el_wet_max[el_sea] = 100;
    el_wet_min[el_desert] = 0;
    el_wet_max[el_desert] = desertfrac;
    el_wet_min[el_plains] = desertfrac;
    el_wet_max[el_plains] = forestfrac;
    el_wet_min[el_forest] = forestfrac;
    el_wet_max[el_forest] = 100;
    el_wet_min[el_ice] = 0;
    el_wet_max[el_ice] = 100;
    if (area.latitude > ((world.circumference / 4) * 80) / 100) {
	el_alt_min[el_ice] = el_alt_min[el_desert] + 1;
	el_alt_max[el_desert] = el_alt_min[el_desert] + 1;
	el_alt_max[el_plains] = el_alt_min[el_plains] + 1;
	el_alt_max[el_forest] = el_alt_min[el_forest] + 1;
    }
    allocate_area_terrain();
    /* Overwrite already-defined elevs? */
    if (!elevations_defined() && any_elev_variation)
      allocate_area_elevations();
    elevrange = maxelev - minelev;
    make_earthlike_fractal_terrain(0, 0);
    /* Make sure the border of the area has something in it. */
    add_edge_terrain();
    finish_lengthy_process();
    return TRUE;
}

static void
make_earthlike_fractal_terrain(int calls, int runs)
{
    int actualcells, altnumblobs, altblobradius, altblobalt;
    int wetnumblobs, wetblobradius, wetblobalt;

    /* Heuristic limit - this algorithm would get weird on small areas */
    if (area.width < 9 || area.height < 9) {
	init_warning("cannot generate fractal terrain for a %d x %d area, must be at least 9x9",
		     area.width, area.height);
	return;
    }
    Dprintf("Going to make earthlike fractal terrain ...\n");
    allocate_area_scratch(3);
    histo  = (int *) xmalloc(MAXALT * sizeof(int));
    alts   = (int *) xmalloc(MAXALT * sizeof(int));
    wets   = (int *) xmalloc(MAXALT * sizeof(int));
    announce_lengthy_process("Making earthlike fractal terrain");
    /* Need a rough estimate of how much work involved, so can do progress. */
    if (el_alt_blob_density > 0) {
	actualcells = (el_alt_blob_size * area.numcells) / 10000;
	altblobradius = isqrt((actualcells * 4) / 3) / 2;
	altnumblobs = (el_alt_blob_density * area.numcells) / 10000;
	altblobalt = el_alt_blob_height;
    }
    if (el_wet_blob_density > 0) {
	actualcells = (el_wet_blob_size * area.numcells) / 10000;
	wetblobradius = isqrt((actualcells * 4) / 3) / 2;
	wetnumblobs = (el_wet_blob_density * area.numcells) / 10000;
	wetblobalt = el_wet_blob_height;
    }
    if (el_alt_blob_density > 0) {
	/* Build a full relief area. */
	partdone = 0;
	make_blobs(area.tmp1, altnumblobs, altblobradius, altblobalt);
	/* Run the requested number of smoothing steps. */
	partdone += stepsize;
	smooth_layer(area.tmp1, el_alt_smoothing);
	percentile(area.tmp1, alts);
    }
    if (el_wet_blob_density > 0) {
	/* Build a "moisture relief" area. */
	partdone += stepsize;
	make_blobs(area.tmp2, wetnumblobs, wetblobradius, wetblobalt);
	partdone += stepsize;
	smooth_layer(area.tmp2, el_wet_smoothing);
	percentile(area.tmp2, wets);
    }
    /* Put it all together. */
    partdone += stepsize;
    compose_earthlike_area();
    fix_adjacent_terrain();
    add_edge_terrain();
    flatten_liquid_terrain();
    /* Free up what we don't need anymore. */
    free(histo);
    free(alts);
    free(wets);
    finish_lengthy_process();
    /* Report on the substitutions made. */
    if (numholes > 0) {
	init_warning("no possible terrain for %d cells, made them into %s",
		     numholes, t_type_name(0));
    }
    return;
}

/* Final creation and output of the area. */

static void
compose_earthlike_area()
{
    int x, y, t, elev;

    Dprintf("Assigning terrain types to cells...\n");
    if (elev_range == NULL)
      elev_range = (int *) xmalloc(numttypes * sizeof(int));
    /* Make the terrain layer itself. */
    allocate_area_terrain();
    numholes = 0;
    for_all_interior_cells(x, y) {
	t = earthlike_terrain_from_percentiles(x, y);
	set_terrain_at(x, y, t);
    }
    if (any_elev_variation) {
    	/* Compute elevation variations of terrain.  This works on interior
    	   cells only; the edge gets handled later, by edge-specific code. */
    	for_all_terrain_types(t) {
    	    elev_range[t] = t_elev_max(t) - t_elev_min(t);
    	}
    	if (!elevations_defined()) {
	    allocate_area_elevations();
   	}
 	for_all_interior_cells(x, y) {
	    t = terrain_at(x, y);
	    elev = 0;
	    if (elev_range[t] > 0)
	      elev = elev_range[t] / 2;
	    elev += t_elev_min(t);
	    /* Clip elevation to required bounds. */
	    if (elev < t_elev_min(t))
	      elev = t_elev_min(t);
	    if (elev > t_elev_max(t))
	      elev = t_elev_max(t);
	    set_elev_at(x, y, elev);
	}
    }
}

/* Compute the actual terrain types.  This is basically a process of
   checking the percentile limits on each type against what is actually
   there. */

static int
earthlike_terrain_from_percentiles(int x, int y)
{
    int j, rawalt = tmp1_at(x, y), rawwet = tmp2_at(x, y);

    if (numttypes == 1)
      return 0;
    for (j = 0; j < numeltypes; ++j) {
	if (between(el_alt_min[j], alts[rawalt], el_alt_max[j])
	    && between(el_wet_min[j], wets[rawwet], el_wet_max[j])) {
	    switch (j) {
	    case el_sea:
	      return seatype;
	    case el_desert:
	      return deserttype;
	    case el_forest:
	      return foresttype;
	    case el_plains:
	      return landtype;
	    case el_ice:
	      return icetype;
	    }
	}
    }
    /* No terrain maybe not an error, so just count and summarize later. */
    ++numholes;
    return 0;
}

/* Maze terrain generation.  */

int numsolidtypes = 0;
int numroomtypes = 0;
int numpassagetypes = 0;

int sumsolidoccur = 0;
int sumroomoccur = 0;
int sumpassageoccur = 0;

int solidtype = NONTTYPE;
int roomtype = NONTTYPE;
int passagetype = NONTTYPE;

int numpassagecells = 0;

static void
set_room_interior(int x, int y)
{
    set_terrain_at(x, y, random_room_terrain());
}

int
make_maze_terrain(int calls, int runs)
{
    int t, x, y, x1, y1, i, n;
    int numcells, tries, numpassagecellsneeded;
    int roomcells, roomradius, numrooms, *roomx, *roomy;

    if (terrain_defined())
      return FALSE;
    for_all_terrain_types(t) {
    	if ((n = t_occurrence(t)) > 0) {
	    sumsolidoccur += n;
	    ++numsolidtypes;
	    solidtype = t;
    	}
    	if ((n = t_maze_room_occurrence(t)) > 0) {
	    sumroomoccur += n;
	    ++numroomtypes;
	    roomtype = t;
    	}
    	if ((n = t_maze_passage_occurrence(t)) > 0) {
	    sumpassageoccur += n;
	    ++numpassagetypes;
	    passagetype = t;
    	}
    }
    if (numsolidtypes + numroomtypes + numpassagetypes < 2) {
    	init_warning("No types to make maze with");
    	return FALSE;
    }
    announce_lengthy_process("Making maze terrain");
    allocate_area_terrain();
    /* Fill in the area with solid terrain. */
    for_all_cells(x, y) {
	set_terrain_at(x, y, random_solid_terrain());
    }
    /* Set the edges properly. */
    add_edge_terrain();
    numcells = area.numcells;
    if (g_maze_room() > 0) {
	roomcells = 7;
	roomradius = 1;
	numrooms = ((numcells * g_maze_room()) / 10000) / roomcells;
	roomx = (int *) xmalloc(numrooms * sizeof(int));
	roomy = (int *) xmalloc(numrooms * sizeof(int));
	for (i = 0; i < numrooms; ++i) {
	    random_point(&x1, &y1);
	    apply_to_area(x1, y1, roomradius, set_room_interior);
	    /* Record room position so passage-making can use. */
	    roomx[i] = x1;  roomy[i] = y1;
	}
    }
    if (g_maze_passage() > 0) {
	numpassagecellsneeded = (numcells * g_maze_passage()) / 10000;
	/* Connect all the rooms together, sometimes directly, sometimes
	   via a randomly-chosen spot. */
	for (i = 0; i < numrooms - 1; ++i) {
	    if (flip_coin()) {
		dig_maze_path_between(roomx[i], roomy[i], roomx[i+1], roomy[i+1]);
	    } else {
		random_point(&x1, &y1);
		dig_maze_path_between(roomx[i], roomy[i], x1, y1);
		dig_maze_path_between(x1, y1, roomx[i+1], roomy[i+1]);
	    }
	}
	/* If we need more passageway, start from random open cells and dig out
	   into the surrounding rock. */
	for (tries = 0; tries < numcells / 2; ++tries) {
	    if (numpassagecells >= numpassagecellsneeded)
	      break;
	    random_point(&x1, &y1);
	    if (t_maze_room_occurrence(terrain_at(x1, y1)) > 0
		|| t_maze_passage_occurrence(terrain_at(x1, y1)) > 0) {
		dig_maze_path(x1, y1, random_dir());
	    }
	}
	/* Find areas of all-passage terrain and reduce them down a little. */
	for_all_interior_cells(x, y) {
	    if (t_maze_passage_occurrence(terrain_at(x, y)) > 0
		&& num_open_adj(x, y) >= NUMDIRS - 1) {
		set_terrain_at(x, y, random_solid_terrain());
	    }
	}
    }
    /* Make sure the border of the area is fixed up. */
    add_edge_terrain();
    finish_lengthy_process();
    return TRUE;
}

/* Given a starting position and direction, dig a passageway that
   generally heads in that direction, but avoids other passages
   and occasionally turns or branches. */

static void
dig_maze_path(int x1, int y1, int dir1)
{
    int found;
    int x, y, iter = 0, dir, dir2, nx, ny;
    
    while (!interior_point_in_dir(x1, y1, dir1, &nx, &ny))
      dir1 = random_dir();
    x = x1;  y = y1;
    dir = dir1;
    for (iter = 0; iter < area.maxdim; ++iter) {
	if (!interior_point_in_dir(x, y, dir, &nx, &ny))
	  break;
	found = FALSE;
	if (t_occurrence(terrain_at(nx, ny)) > 0
	    && num_open_adj(nx, ny) == 1) {
	    found = TRUE;
	} else {
	    /* Try different directions. */
	    for_all_directions(dir2) {
		point_in_dir(x, y, dir2, &nx, &ny);
		if (inside_area(nx, ny)
		    && t_occurrence(terrain_at(nx, ny)) > 0
		    && num_open_adj(nx, ny) == 1) {
		    found = TRUE;
		    dir = dir2;
		    break;
		}
	    }
	    if (!found) {
		/* Try again, allowing two open cells to be adjacent. */
		for_all_directions(dir2) {
		    point_in_dir(x, y, dir2, &nx, &ny);
		    if (inside_area(nx, ny)
			&& t_occurrence(terrain_at(nx, ny)) > 0
			&& num_open_adj(nx, ny) == 2) {
			found = TRUE;
			dir = dir2;
			break;
		    }
		}
	    }
	}
	if (found) {
	    set_terrain_at(nx, ny, random_passage_terrain());
	    ++numpassagecells;
	} else {
	    return;
	}
	if (probability(20)) {
	    dig_maze_path(nx, ny, left_dir(dir));
	    dig_maze_path(nx, ny, right_dir(dir));
	} else {
	    x = nx;  y = ny;
	    dir = (probability(50) ? dir : random_dir());
	}
    }
}

static void
dig_maze_path_between(int x1, int y1, int x2, int y2)
{
    apply_to_path(x1, y1, x2, y2, test_maze_segment, sort_maze_segments,
		  lay_maze_segment, FALSE);
}

static int
test_maze_segment(int x, int y, int dir)
{
    int x1, y1;

    return (interior_point_in_dir(x, y, dir, &x1, &y1));
}

static int
sort_maze_segments(int x, int y, int *dirchoices, int numchoices)
{
    int nx, ny, w0, w1, tmp;

    if (numchoices == 2) {
    	point_in_dir(x, y, dirchoices[0], &nx, &ny);
	if (t_maze_room_occurrence(terrain_at(nx, ny)) > 0)
	  w0 = 2;
	else if (t_maze_passage_occurrence(terrain_at(nx, ny)) > 0)
	  w0 = 1;
	else
	  w0 = 0;
    	point_in_dir(x, y, dirchoices[1], &nx, &ny);
	if (t_maze_room_occurrence(terrain_at(nx, ny)) > 0)
	  w1 = 2;
	else if (t_maze_passage_occurrence(terrain_at(nx, ny)) > 0)
	  w1 = 1;
	else
	  w1 = 0;
	if (w1 > w0 || (w1 == w0 && flip_coin())) {
	    tmp = dirchoices[0];
	    dirchoices[0] = dirchoices[1];
	    dirchoices[1] = tmp;
	}
    }
    return numchoices;
}

static int
lay_maze_segment(int x, int y, int d, int choice, int numchoices)
{
    int x1, y1;

    if (interior_point_in_dir(x, y, d, &x1, &y1)) {
	if (t_occurrence(terrain_at(x, y)) > 0) {
	    set_terrain_at(x, y, random_passage_terrain());
	    ++numpassagecells;
	}
	return 1;
    }
    return 0;
}

static int
num_open_adj(int x, int y)
{
    int dir, rslt = 0, nx, ny;

    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (t_maze_room_occurrence(terrain_at(nx, ny)) > 0
	    || t_maze_passage_occurrence(terrain_at(nx, ny)) > 0)
	  ++rslt;
    }
    return rslt;
}

static int
random_solid_terrain()
{
    if (numsolidtypes == 1)
      return solidtype;
    return (xrandom(numttypes));
}

static int
random_room_terrain()
{
    if (numroomtypes == 1)
      return roomtype;
    return (xrandom(numttypes));
}

static int
random_passage_terrain()
{
    if (numpassagetypes == 1)
      return passagetype;
    return (xrandom(numttypes));
}

static void
add_random_changes(void)
{
    int t1, t2, anychanges = FALSE, x, y, dens;

    /* Might not be anything to do, scan the table. */
    for_all_terrain_types(t1) {
	for_all_terrain_types(t2) {
	    if (tt_density(t1, t2) > 0) {
		anychanges = TRUE;
		break;
	    }
	}
    }
    if (!anychanges)
      return;
    for_all_cells(x, y) {
	for_all_terrain_types(t2) {
	    dens = tt_density(terrain_at(x, y), t2);
	    if (dens > 0 && dens > xrandom(10000)) {
		set_terrain_at(x, y, t2);
		break;
	    }
	}
    }
}

/* This method adds some randomly named geographical features. */

/* (This needs to interact properly with convex region finder eventually) */

int
name_geographical_features(int calls, int runs)
{
    char *classname, *specificname;
    Obj *rest, *head, *parms;
    short t, found;
    
    /* If we got features from file or somewhere, don't overwrite them. */
    if (features_defined())
      return FALSE;
    /* We need to have some terrain to work from. */
    if (!terrain_defined())
      return FALSE;
    /* If no feature types requested, don't make any. */
    if (g_feature_types() == lispnil)
      return FALSE;
    announce_lengthy_process("Adding geographical features");
    Dprintf("Adding geographical features...\n");
    /* Set up the basic layer of data. */
    init_features();
    /* Scan through list to see what's being requested. */
    for_all_list(g_feature_types(), rest) {
	found = FALSE;
	head = car(rest);
	if (symbolp(head)) {
	    classname = c_string(head);
	    parms = lispnil;
	} else if (consp(head) && symbolp(car(head))) {
	    classname = c_string(car(head));
	    parms = cdr(head);
	} else {
	    run_warning("Feature type clause not recognized");
	    continue;
	}
	specificname = classname;
	if (parms != lispnil && symbolp(car(parms))) {
	    specificname = c_string(car(parms));
	    parms = cdr(parms);
	}
	if (strcmp(classname, "peaks") == 0) {
	    name_highest_peaks(specificname, parms);
	} else if (strcmp(classname, "islands") == 0) {
	    name_islands(specificname, parms);
	} else if (strcmp(classname, "continents") == 0) {
	    name_continents(specificname, parms);
	} else if (strcmp(classname, "lakes") == 0) {
	    name_lakes(specificname, parms);
	} else if (strcmp(classname, "seas") == 0) {
	    name_seas(specificname, parms);
	} else if (strcmp(classname, "bays") == 0) {
	    name_bays(specificname, parms);
	} else {
		/* If the feature name is identical to a terrain type name, it means that
		we want to name each patch of that terrain type whose size is at least
		parms. */
		for_all_terrain_types(t) {
			if (t_type_name(t)
			    && strcmp(classname, t_type_name(t)) == 0) {
				name_terrain(t, specificname, parms);
				found = TRUE;
				break;
			}
		}
		if (!found) {
			run_warning("Don't know to identify \"%s\" features", classname);
		}
	}
    }
    finish_lengthy_process();
    return TRUE;
}

/* Identify the highest high points as "peaks". */

static void
name_highest_peaks(char *name, Obj *parms)
{
    int density, x, y, maxpeaks, numpeaks, *peakx, *peaky, i, lo;
    char *fname;
    Feature *mountain;

    /* If no possibility of peaks, return quietly.  (A warning here seemed like
       a good idea, but many games happen to be on flat worlds, and there
       wasn't actually anything wrong with those.) */
    if (!elevations_defined() || !any_elev_variation)
      return;
    /* Default to a peak for about every 20x10 group of cells, or about 10 for a
       default-sized game. */
    density = 50;
    if (consp(parms) && numberp(car(parms))) {
	density = c_number(car(parms));
    }
    maxpeaks = max(1, (area.numcells * density) / 10000);
    peakx = (int *) xmalloc(maxpeaks * sizeof(int));
    peaky = (int *) xmalloc(maxpeaks * sizeof(int));
    numpeaks = 0;

    for_all_interior_cells(x, y) {
	if (high_point(x, y)) {
	    if (numpeaks < maxpeaks) {
		peakx[numpeaks] = x;  peaky[numpeaks] = y;
		++numpeaks;
	    } else {
		/* Find the lowest of existing peaks. */
		lo = 0;
		for (i = 0; i < numpeaks; ++i) {
		    if (elev_at(peakx[i], peaky[i]) <
			elev_at(peakx[lo], peaky[lo])) {
			lo = i;
		    }
		}
		/* If less than our new candidate, replace. */
		if (elev_at(x, y) > elev_at(peakx[lo], peaky[lo])) {
		    peakx[lo] = x;  peaky[lo] = y;
		}
	    }
	}
    }
    for (i = 0; i < numpeaks; ++i) {
	fname = name_feature_at(x, y, "peak");
	if (fname == NULL) {
	    sprintf(tmpbuf, "Pk %d", elev_at(peakx[i], peaky[i]));
	    fname = copy_string(tmpbuf);
	}
	mountain = create_feature("peak", fname);
	mountain->size = 1;
	set_raw_feature_at(peakx[i], peaky[i], mountain->id);
    }
}

/* True if xy is a local high point. */

static int
high_point(int x, int y)
{
    int dir, nx, ny;

    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (elev_at(nx, ny) >= elev_at(x, y)) {
	    return FALSE;
	}
    }
    return TRUE;
}

static void
name_islands(char *name, Obj *parms)
{
	int sizemin, sizemax, x, y;
	TRegion *region;
	char *fname;
	Feature *island;

	if (area.landsea_regions == NULL) {
		area.landsea_regions = malloc_area_layer(TRegion *);
		divide_into_regions(area.terrain, area.landsea_regions, TRUE);
	}
	/* Get the min size for fetures. */
	sizemin = 1;
	if (consp(parms) && numberp(car(parms))) {
		sizemin = c_number(car(parms));
	}
	/* And their max size. */
	sizemax = 30;
	if (consp(parms) && numberp(cadr(parms))) {
		sizemax = c_number(cadr(parms));
		/* Warn if sizemax was set to less than 1. */ 
		if (sizemax < 1) {
			init_warning("Invalid max size %d for named islands.", sizemax);
		}
	}
	for (region = landsea_region_list; region != NULL; region = region->next) {
		if (region->size > 0
		&& region->size <= sizemax
		&& region->size >= sizemin
		&& region->ttype == 1) {
			fname = name_feature_at(area.width / 2, area.height / 2, name);
			if (fname != NULL) {
				island = create_feature("island", fname);
				island->size = 0;
				for (x = region->xmin; x <= region->xmax; ++x) {
					for (y = region->ymin; y <= region->ymax; ++y) {
						if (aref(area.landsea_regions, x, y) == region) {
							set_raw_feature_at(x, y, island->id);
							++(island->size);
						}
					}
				}
			}
		}
	}
}

static void
name_continents(char *name, Obj *parms)
{
	int sizemin, sizemax, x, y;
	TRegion *region;
	char *fname;
	Feature *continent;
	extern TRegion *landsea_region_list;

	if (area.landsea_regions == NULL) {
		area.landsea_regions = malloc_area_layer(TRegion *);
		divide_into_regions(area.terrain, area.landsea_regions, TRUE);
	}
	/* Get the min size for fetures. */
	sizemin = 31;
	if (consp(parms) && numberp(car(parms))) {
		sizemin = c_number(car(parms));
	}
	/* And their max size. */
	sizemax = area.width * area.height;
	if (consp(parms) && numberp(cadr(parms))) {
		sizemax = c_number(cadr(parms));
		/* Warn if sizemax was set to less than 1. */ 
		if (sizemax < 1) {
			init_warning("Invalid max size %d for named continents.", sizemax);
		}
	}
	for (region = landsea_region_list; region != NULL; region = region->next) {
		if (region->size > 0
		&& region->size <= sizemax
		&& region->size >= sizemin
		&& region->ttype == 1) {
			fname = name_feature_at(area.width / 2, area.height / 2, name);
			if (fname != NULL) {
				continent = create_feature("continent", fname);
				continent->size = 0;
				for (x = region->xmin; x <= region->xmax; ++x) {
					for (y = region->ymin; y <= region->ymax; ++y) {
						if (aref(area.landsea_regions, x, y) == region) {
							set_raw_feature_at(x, y, continent->id);
							++(continent->size);
						}
					}
				}
			}
		}
	}
}

/* Identify lakes by looking for small areas of liquid terrain surrounded by
   non-liquid terrain. */

static void
name_lakes(char *name, Obj *parms)
{
    int sizemin, sizemax, x, y;
    TRegion *region;
    char *fname;
    Feature *lake;

	if (area.landsea_regions == NULL) {
		area.landsea_regions = malloc_area_layer(TRegion *);
		divide_into_regions(area.terrain, area.landsea_regions, TRUE);
	}
	/* Get the min size for fetures. */
	sizemin = 1;
	if (consp(parms) && numberp(car(parms))) {
		sizemin = c_number(car(parms));
	}
	/* And their max size. */
	sizemax = 30;
	if (consp(parms) && numberp(cadr(parms))) {
		sizemax = c_number(cadr(parms));
		/* Warn if sizemax was set to less than 1. */ 
		if (sizemax < 1) {
			init_warning("Invalid max size %d for named lakes.", sizemax);
		}
	}
	for (region = landsea_region_list; region != NULL; region = region->next) {
		if (region->size > 0
		&& region->size <= sizemax
		&& region->size >= sizemin
		&& region->ttype == 0) {
			fname = name_feature_at(area.width / 2, area.height / 2, name);
			if (fname != NULL) {
				lake = create_feature("lake", fname);
				lake->size = 0;
				for (x = region->xmin; x <= region->xmax; ++x) {
					for (y = region->ymin; y <= region->ymax; ++y) {
						if (aref(area.landsea_regions, x, y) == region) {
							set_raw_feature_at(x, y, lake->id);
							++(lake->size);
						}
					}
				}
			}
		}
	}
}

static void
name_seas(char *name, Obj *parms)
{
    int sizemin, sizemax, x, y;
    TRegion *region;
    char *fname;
    Feature *sea;

	if (area.landsea_regions == NULL) {
		area.landsea_regions = malloc_area_layer(TRegion *);
		divide_into_regions(area.terrain, area.landsea_regions, TRUE);
	}
	/* Get the min size for fetures. */
	sizemin = 31;
	if (consp(parms) && numberp(car(parms))) {
		sizemin = c_number(car(parms));
	}
	/* And their max size. */
	sizemax = area.width * area.height;
	if (consp(parms) && numberp(cadr(parms))) {
		sizemax = c_number(cadr(parms));
		/* Warn if sizemax was set to less than 1. */ 
		if (sizemax < 1) {
			init_warning("Invalid max size %d for named seas.", sizemax);
		}
	}
	for (region = landsea_region_list; region != NULL; region = region->next) {
		if (region->size > 0
		&& region->size <= sizemax
		&& region->size >= sizemin
		&& region->ttype == 0) {
			fname = name_feature_at(area.width / 2, area.height / 2, name);
			if (fname != NULL) {
				sea = create_feature("sea", fname);
				sea->size = 0;
				for (x = region->xmin; x <= region->xmax; ++x) {
					for (y = region->ymin; y <= region->ymax; ++y) {
						if (aref(area.landsea_regions, x, y) == region) {
							set_raw_feature_at(x, y, sea->id);
							++(sea->size);
						}
					}
				}
			}
		}
	}
}

/* Identify bays in the world.  Bays are single points where liquid terrain is
   mostly surrounded by non-liquid terrain. */

static void
name_bays(char *name, Obj *parms)
{
    int x, y;
    char *fname;
    Feature *bay;

    for_all_interior_cells(x, y) {
	if (bay_point(x, y)) {
	    fname = name_feature_at(x, y, name);
	    if (fname != NULL) {
		bay = create_feature("bay", fname);
		bay->size = 1;
		set_raw_feature_at(x, y, bay->id);
	    }
	}
    }
}

static int
bay_point(int x, int y)
{
    int dir, nx, ny, seacount, dir1, dir2;

    if (!t_liquid(terrain_at(x, y)))
      return FALSE;
    /* Don't call every cell in a small lake a 'bay'. */
    if (aref(area.landsea_regions, x, y)->size < 6)
      return FALSE;
    seacount = 0;
    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	/* Don't count 'bays' on the area edge. */
	if (!inside_area(nx, ny))
	    return FALSE;
	if (t_liquid(terrain_at(nx, ny))) {
	    ++seacount;
	    if (seacount == 1)
	      dir1 = dir;
	    if (seacount == 2)
	      dir2 = dir;
	}
    }
    return (seacount == 1
	    || (seacount == 2
		&& (dir2 == left_dir(dir1) || dir2 == right_dir(dir1))));
}

/* Name patches of the given terrain type. */

static void
name_terrain(int t, char *name, Obj *parms)
{
	int sizemin, sizemax, x, y;
	TRegion *region;
	char *fname;
	Feature *terrain;

	/* Get the min size for regions. */
	sizemin = 1;
	if (consp(parms) && numberp(car(parms))) {
		sizemin = c_number(car(parms));
	}
	/* And their max size. */
	sizemax = area.width * area.height;
	if (consp(parms) && numberp(cadr(parms))) {
		sizemax = c_number(cadr(parms));
		/* Warn if sizemax was set to less than 1. */ 
		if (sizemax < 1) {
			init_warning("Invalid max size %d for named %s terrain.", sizemax, name);
		}
	}
	/* In the non-trivial case, we use the region code. */ 
	if (sizemax > 1) {
		if (area.terrain_regions == NULL) {
			area.terrain_regions = malloc_area_layer(TRegion *);
			divide_into_regions(area.terrain, area.terrain_regions, FALSE);
		}
		for (region = terrain_region_list; region != NULL; region = region->next) {
			if (region->size > 0
			    && region->size >= sizemin
			    && region->size <= sizemax
			    && region->ttype == t) {
				x = region->xmax - region->xmin;
				y = region->ymax - region->ymin;
				fname = name_feature_at(x, y, name);
				if (fname != NULL) {
					terrain = create_feature(name, fname);
					terrain->size = 0;
					for (x = region->xmin; x <= region->xmax; ++x) {
						for (y = region->ymin; y <= region->ymax; ++y) {
							if (aref(area.terrain_regions, x, y) == region) {
								set_raw_feature_at(x, y, terrain->id);
								++(terrain->size);
							}
						}
					}
				}
			}
		}
	/* If each single cell of the correct type is supposed to be
	a feature, we don't need to use the region code. */
	} else {
		for_all_interior_cells(x, y) {
			if (terrain_at(x, y) == t) {	
				fname = name_feature_at(x, y, name);
				if (fname != NULL) {
					terrain = create_feature(name, fname);
					set_raw_feature_at(x, y, terrain->id);
					terrain->size = 1;
				}
			}
		}
	}
}

static char *
name_feature_at(int x, int y, char *feattype)
{
    char *rslt;
    Obj *namerlist;
    Side *side;

    /* Look for any side-specific namers. */
    if (people_sides_defined()) {
	for_all_sides(side) {
	    if (side->featurenamers != NULL
		&& side->featurenamers != lispnil
		&& people_side_at(x, y) == side->id) {
		rslt = name_feature_using(side->featurenamers, feattype);
		if (rslt != NULL)
		  return rslt;
	    }
	}
    }
    /* Now try generic namer list. */
    namerlist = g_feature_namers();
    /* If no generic namers, get out of here. */
    if (namerlist == lispnil)
      return NULL;
    return name_feature_using(namerlist, feattype);
}

static char *
name_feature_using(Obj *namerlist, char *feattype)
{
    Obj *rest, *head, *namerspec;

    /* If no namers found, get out of here. */
    if (namerlist == lispnil)
      return NULL;
    for_all_list(namerlist, rest) {
	head = car(rest);
	if (consp(head)
	    && (stringp(car(head)) || symbolp(car(head)))
	    && strcmp(c_string(car(head)), feattype) == 0) {
	    namerspec = cadr(head);
	    /* This might be a string naming a namer, try making it into
	       a symbol. */
	    if (stringp(namerspec))
	      namerspec = intern_symbol(c_string(namerspec));
    	    if (boundp(namerspec))
	      return run_namer(symbol_value(namerspec));
	    /* If the namer doesn't exist, then this will continue looking for
	       one that does, which is maybe good, because this might fall back
	       from a side-specific to a generic namer, but can be confusing to
	       game designers, because then a feature might be created with no
	       name, with no warning of a problem with namers. */
	}
    }
    return NULL;
}

/* Resolve any incompatibilities of synthesized terrain in adjacent cells.
   Also, take care of any borders that should be set up between the cells. */

static void
fix_adjacent_terrain()
{
    int t = NONTTYPE, t2 = NONTTYPE, t3 = NONTTYPE, tb = NONTTYPE;
    int anyadjeffects, anyadjborders, anyadjconnections;
    int changechance = 0, changeagain = 0;
    int maxpasses, anychanges, x, y, dir, x1, y1, passnum = 0;

    anyadjeffects = FALSE;
    anyadjborders = FALSE;
    anyadjconnections = FALSE;
    for_all_terrain_types(t) {
	tmp_t_array[t] = FALSE;
	for_all_terrain_types(t2) {
	    t3 = tt_adj_terr_border(t, t2);
	    if ((t3 >= 0) && tt_adj_terr_border_chance(t, t2)) {
		if (!is_terrain_type(t3) || !t_is_border(t3))
		  run_warning(
"Bad value %d in 'adjacent-terrain-border' table; ignoring.", t3);
		else
		  anyadjborders = TRUE;
	    }
	    t3 = tt_adj_terr_connection(t, t2);
	    if ((t3 >= 0) && tt_adj_terr_connection_chance(t, t2)) {
		if (!is_terrain_type(t3) || !t_is_connection(t3))
		    run_warning(
"Bad value %d in 'adjacent-terrain-connection' table; ignoring.", t3);
		else
		    anyadjconnections = TRUE;
	    }
	    t3 = tt_adj_terr_effect(t, t2);
	    if ((t3 >= 0) && tt_adj_terr_effect_chance(t, t2)) {
		/* Table definitions have no provision for
		   dynamically-determined bounds, so we have to check
		   it here. */
		if (!is_terrain_type(t3) || !t_is_cell(t3)) {
		    run_warning(
"Bad value %d in 'adjacent-terrain-effect' table; ignoring", t3);
		    break;
		}
		anyadjeffects = TRUE;
		tmp_t_array[t] = TRUE;
		break;
	    }
	}
    }
    if (!anyadjeffects && !anyadjborders && !anyadjconnections)
      return;
    announce_lengthy_process(
"Fixing adjacent terrain cells, borders, and connections");
    maxpasses = area.height;
    /* Iterate until we converge on a stable solution [i.e., no more 
       terrain changes can occur]. */
    while (anyadjeffects && (maxpasses-- > 0)) {
	anychanges = FALSE;
	for_all_cells(x, y) {
	    t = terrain_at(x, y);
	    set_tmp1_at(x, y, t);
	    if (tmp_t_array[t]) {
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			t2 = terrain_at(x1, y1);
			t3 = tt_adj_terr_effect(t, t2);
			changechance = tt_adj_terr_effect_chance(t, t2);
			changeagain = tt_adj_terr_effect_passes(t, t2);
			if (changeagain > -1)
			  changeagain = (changeagain > passnum);
			else
			  changeagain = TRUE;
			if (is_terrain_type(t3) && t_is_cell(t3) && (t3 != t)
			    && probability(changechance)
			    && changeagain) {
			    set_tmp1_at(x, y, t3);
			    anychanges = TRUE;
			    break;
			} 
		    }
		}
	    }
	} /* for_all_cells */
	/* Increment pass number. */
	++passnum;
	if (anychanges) {
	    /* Copy from the tmp layer back to the area. */
	    for_all_cells(x, y) {
		set_terrain_at(x, y, tmp1_at(x, y));
	    }
	} else {
	    /* No changes, things have stabilized; so break. */
	    break;
	}
    } /* while (maxpasses) */
    /* Now determine which cells require borders between them. */
    /* [ Only requires 1 pass. ] */
    if (anyadjborders || anyadjconnections)
    for_all_cells(x, y) {
	t = terrain_at(x, y);
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		t2 = terrain_at(x1, y1);

	        t3 = tt_adj_terr_border(t, t2);
		changechance = tt_adj_terr_border_chance(t, t2);
		if (is_terrain_type(t3) && t_is_border(t3) 
		    && probability(changechance)) {
		    /* Override old border type, if necessary. */
		    if (any_borders_in_dir(x, y, dir)) {
			for_all_border_types(tb) {
			    if (aux_terrain_defined(tb))
			        set_border_at(x, y, dir, tb, FALSE);
			}
		    }
		    /* Set the new border type. */
		    set_border_at(x, y, dir, t3, TRUE);
		}

	        t3 = tt_adj_terr_connection(t, t2);
		changechance = tt_adj_terr_connection_chance(t, t2);
		if (is_terrain_type(t3) && t_is_connection(t3) 
		    && probability(changechance)) {
		    /* Override old connection type, if necessary. */
		    for_all_connection_types(tb) {
		        if (aux_terrain_defined(tb))
		            set_connection_at(x, y, dir, tb, FALSE);
		    }
		    /* Set the new connection type. */
		    set_connection_at(x, y, dir, t3, TRUE);
		}
	    }
	} /* for_all_directions */
    } /* for_all_cells */
    finish_lengthy_process();
}

/* For efficiency and semantics reasons, the methods might not assign values
   to the cells around the edge of the area (if there *are* edges; neither
   a torus nor sphere will have any).  Note that there is no way to
   disable this from the game module; if having nonconstant edges is important
   enough to be worth the user confusion, don't call this from your
   area generation method. */

void
add_edge_terrain(void)
{
    set_edge_terrain(TRUE);
}

void
set_edge_terrain(int doterr)
{
    int x, y, t = g_edge_terrain(), halfheight = area.halfheight;

    /* Use ttype 0 if edge terrain is nonsensical. */
    if (!between(0, t, numttypes-1))
      t = 0;
    if (!doterr)
      t = NONUTYPE;
    /* Right/left upper/lower sides of a hexagon. */
    if (!area.xwrap) {
	for (y = 0; y < halfheight; ++y) {
	    /* SW edge */
	    set_edge_values(halfheight - y, y, t);
	    /* NW edge */
	    set_edge_values(0, halfheight + y, t);
	    /* SE edge */
	    set_edge_values(area.width-1, y, t);
	    /* NE edge */
	    set_edge_values(area.width-1 - y, halfheight + y, t);
	}
    }
    /* Top and bottom edges of the area. */
    for (x = 0; x < area.width; ++x) {
	set_edge_values(x, 0, t);
	set_edge_values(x, area.height-1, t);
    }
}

static void
set_edge_values(int x, int y, int t)
{
    if (t != NONUTYPE)
      set_terrain_at(x, y, t);
    /* Give it an average elevation. */
    if (elevations_defined())
      set_edge_elevation(x, y);
}

static void
set_edge_elevation(int x, int y)
{
    int sumelev, numadj, dir, x1, y1, elev;

    sumelev = 0;
    numadj = 0;
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    sumelev += elev_at(x1, y1);
	    ++numadj;
	}
    }
    /* It's possible that the caller is being sloppy and setting edge
       cells not actually in the area, so be cool about it. */
    if (numadj > 0) {
	elev = sumelev / numadj;
	/* Note that we clip only to overall bounds for the layer, not
	   to the specific terrain type's bounds.  This is so that the
	   edge terrain can be, say, ice, whose normal elevation range
	   is high altitude, which would make a perspective view look
	   weird (like peering into a sink). */
	if (elev < minelev)
	  elev = minelev;
	if (elev > maxelev)
	  elev = maxelev;
	set_elev_at(x, y, elev);
    }
}

/* This makes area of liquid terrain types all have the same
   elevation, and an elevation less than that of any adjacent
   non-liquid terrain.  In simple terms, this makes ponds and such
   look right on contour maps (more importantly, LOS visibility will
   work as expected around water features). */

static void
flatten_liquid_terrain(void)
{
    int t1, t2, anyliquideffects;
    int maxpasses, anychanges, x, y, dir, x1, y1, highest, lowest;

    if (!elevations_defined() || !any_elev_variation)
      return;
    anyliquideffects = FALSE;
    for_all_terrain_types(t1) {
	tmp_t_array[t1] = t_liquid(t1);
	if (tmp_t_array[t1])
	  anyliquideffects = TRUE;
    }
    if (!anyliquideffects)
      return;
    /* First raise up low places in all-liquid areas. */
    maxpasses = area.height;
    while (maxpasses-- > 0) {
	anychanges = FALSE;
	for_all_cells(x, y) {
	    t1 = terrain_at(x, y);
	    if (tmp_t_array[t1]) {
		highest = elev_at(x, y);
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			t2 = terrain_at(x1, y1);
			if (tmp_t_array[t2]) {
			    highest = max(highest, elev_at(x1, y1));
			} else {
			    goto nextcell;
			}
		    }
		}
		/* Now we have the highest of liquid elevations. */
		if (highest != elev_at(x, y)) {
		    set_elev_at(x, y, highest);
		    anychanges = TRUE;
		}
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			t2 = terrain_at(x1, y1);
			if (tmp_t_array[t2] && highest != elev_at(x1, y1)) {
			    set_elev_at(x1, y1, highest);
			    anychanges = TRUE;
			} 
		    }
		}
	    }
	  nextcell:
	    ;
	}
	if (!anychanges)
	  break;
    }
    /* Now bring up all too-high places that are adjacent to non-liquid
       terrain. */
    maxpasses = area.height;
    while (maxpasses-- > 0) {
	anychanges = FALSE;
	for_all_cells(x, y) {
	    t1 = terrain_at(x, y);
	    if (tmp_t_array[t1]) {
		lowest = elev_at(x, y);
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			t2 = terrain_at(x1, y1);
			if (tmp_t_array[t2]) {
			    lowest = min(lowest, elev_at(x1, y1));
			} 
		    }
		}
		/* Now we have the lowest of liquid elevations. */
		if (lowest != elev_at(x, y)) {
		    set_elev_at(x, y, lowest);
		    anychanges = TRUE;
		}
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			t2 = terrain_at(x1, y1);
			if (tmp_t_array[t2] && lowest != elev_at(x1, y1)) {
			    set_elev_at(x1, y1, lowest);
			    anychanges = TRUE;
			} 
		    }
		}
	    }
	}
	if (!anychanges)
	  break;
    }
}


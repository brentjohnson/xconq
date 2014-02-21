// xConq
// Lifecycle management and ser/deser of per-side cell views.

// $Id: cellview.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of per-side cell views.
    \ingroup grp_gdl
*/

#include "gdl/gvars.h"
#include "gdl/tables.h"
#include "gdl/side/side.h"
#include "gdl/gamearea/area.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Game Setup

short any_los = -1;
int any_material_views = -1;
char *any_material_views_by_m = NULL;

// Game Area View Layer Accessors

int
fn_terrain_view(int x, int y)
{
    return terrain_view(tmpside, x, y);
}

int
fn_terrain_view_date(int x, int y)
{
    return terrain_view_date(tmpside, x, y);
}

int
fn_aux_terrain_view(int x, int y)
{
    return aux_terrain_view(tmpside, x, y, tmpttype);
}

int
fn_aux_terrain_view_date(int x, int y)
{
    return aux_terrain_view_date(tmpside, x, y, tmpttype);
}

int
fn_material_view(int x, int y)
{
    return material_view(tmpside, x, y, tmpmtype);
}

int
fn_material_view_date(int x, int y)
{
    return material_view_date(tmpside, x, y, tmpmtype);
}

int
fn_temp_view(int x, int y)
{
    return temperature_view(tmpside, x, y);
}

int
fn_temp_view_date(int x, int y)
{
    return temperature_view_date(tmpside, x, y);
}

int
fn_cloud_view(int x, int y)
{
    return cloud_view(tmpside, x, y);
}

int
fn_cloud_bottom_view(int x, int y)
{
    return cloud_bottom_view(tmpside, x, y);
}

int
fn_cloud_height_view(int x, int y)
{
    return cloud_height_view(tmpside, x, y);
}

int
fn_cloud_view_date(int x, int y)
{
    return cloud_view_date(tmpside, x, y);
}

int
fn_wind_view(int x, int y)
{
    return wind_view(tmpside, x, y);
}

int
fn_wind_view_date(int x, int y)
{
    return wind_view_date(tmpside, x, y);
}

void
fn_set_terrain_view(int x, int y, int val)
{
    int rawval;

    if (1)
      rawval = val;
    else
      /* This is a more efficient encoding, but only usable if can guarantee
	 see-terrain-always upon rereading. */
      rawval = (val ? buildtview(terrain_at(x, y)) : 0);
    set_terrain_view(tmpside, x, y, rawval);
}

void
fn_set_terrain_view_date(int x, int y, int val)
{
    set_terrain_view_date(tmpside, x, y, val);
}

void
fn_set_aux_terrain_view(int x, int y, int val)
{
    /* Filter anything but the basic six bits. */
    val &= 0x3f;
    set_aux_terrain_view(tmpside, x, y, tmpttype, val);
}

void
fn_set_aux_terrain_view_date(int x, int y, int val)
{
    set_aux_terrain_view_date(tmpside, x, y, tmpttype, val);
}

void
fn_set_material_view(int x, int y, int val)
{
    set_material_view(tmpside, x, y, tmpmtype, val);
}

void
fn_set_material_view_date(int x, int y, int val)
{
    set_material_view_date(tmpside, x, y, tmpmtype, val);
}

void
fn_set_temp_view(int x, int y, int val)
{
    set_temperature_view(tmpside, x, y, val);
}

void
fn_set_temp_view_date(int x, int y, int val)
{
    set_temperature_view_date(tmpside, x, y, val);
}

void
fn_set_cloud_view(int x, int y, int val)
{
    set_cloud_view(tmpside, x, y, val);
}

void
fn_set_cloud_bottom_view(int x, int y, int val)
{
    set_cloud_bottom_view(tmpside, x, y, val);
}

void
fn_set_cloud_height_view(int x, int y, int val)
{
    set_cloud_height_view(tmpside, x, y, val);
}

void
fn_set_cloud_view_date(int x, int y, int val)
{
    set_cloud_view_date(tmpside, x, y, val);
}

void
fn_set_wind_view(int x, int y, int val)
{
    set_wind_view(tmpside, x, y, val);
}

void
fn_set_wind_view_date(int x, int y, int val)
{
    set_wind_view_date(tmpside, x, y, val);
}

// Lifecycle Management

int
init_view(Side *side)
{
    int terrainset = FALSE, t, u, m;

    /* Be sure that we're not trying to set this up too soon. */
    check_area_shape();
    /* Allocate terrain view layers. */
    if ((!g_terrain_seen() || !g_see_terrain_always())
	&& side->terrview == NULL) {
	side->terrview = malloc_area_layer(char);
	if (!g_see_terrain_always()) {
	    side->terrviewdate = malloc_area_layer(short);
	    /* The terrview also holds the "have we seen it" flag, so
	       it must always be allocated, but the aux terrain view
	       need not be. */
	    if (numcelltypes < numttypes) {
		side->auxterrview =
		  (char **) xmalloc(numttypes * sizeof(short *));
		for_all_terrain_types(t) {
		    if (!t_is_cell(t)) {
			side->auxterrview[t] = malloc_area_layer(char);
		    }
		}
		if (0 /* aux terrain seen separately from main terrain */) {
		    side->auxterrviewdate =
		      (short **) xmalloc(numttypes * sizeof(short *));
		    for_all_terrain_types(t) {
			if (!t_is_cell(t)) {
			    side->auxterrviewdate[t] =
			      malloc_area_layer(short);
			}
		    }
		}
	    }
	}
    } else {
	terrainset = TRUE;
    }
    /* Allocate material view layers. */
    if (any_material_views < 0) {
	any_material_views = FALSE;
	for_all_material_types(m) {
	    for_all_terrain_types(t) {
		if (tm_storage_x(t, m) > 0 && tm_see_always(t, m) == 0) {
		    any_material_views = TRUE;
		    if (any_material_views_by_m == NULL)
		      any_material_views_by_m = (char *)xmalloc(nummtypes);
		    any_material_views_by_m[m] = TRUE;
		    break;
		}
	    }
	}
    }
    if (any_material_views) {
	if (side->materialview == NULL)
	  side->materialview = (short **) xmalloc(nummtypes * sizeof(short *));
	if (side->materialviewdate == NULL)
	  side->materialviewdate =
	    (short **) xmalloc(nummtypes * sizeof(short *));
	for_all_material_types(m) {
	    if (any_material_views_by_m[m]) {
		if (side->materialview[m] == NULL) {
		    side->materialview[m] = malloc_area_layer(short);
		}
		if (side->materialviewdate[m] == NULL) {
		    side->materialviewdate[m] = malloc_area_layer(short);
		}
	    }
	}
    }
    /* Allocate weather view layers. */
    if (any_temp_variation && !g_see_weather_always()) {
	if (side->tempview == NULL) {
	    side->tempview = malloc_area_layer(short);
	}
	if (side->tempviewdate == NULL) {
	    side->tempviewdate = malloc_area_layer(short);
	}
    }
    if (any_clouds && !g_see_weather_always()) {
	if (side->cloudview == NULL) {
	    side->cloudview = malloc_area_layer(short);
	}
	if (side->cloudbottomview == NULL) {
	    side->cloudbottomview = malloc_area_layer(short);
	}
	if (side->cloudheightview == NULL) {
	    side->cloudheightview = malloc_area_layer(short);
	}
	if (side->cloudviewdate == NULL) {
	    side->cloudviewdate = malloc_area_layer(short);
	}
    }
    if (any_wind_variation && !g_see_weather_always()) {
	if (side->windview == NULL) {
	    side->windview = malloc_area_layer(short);
	}
	if (side->windviewdate == NULL) {
	    side->windviewdate = malloc_area_layer(short);
	}
    }
    /* Allocate the vision coverage cache. */
    if (side->coverage == NULL) {
	side->coverage = malloc_area_layer(short);
    }
    /* Allocate vision altitude coverage if needed */
    if (any_los < 0) {
	any_los = FALSE;
	for_all_unit_types(u) {
	    if (!u_can_see_behind(u)) {
		any_los = TRUE;
		break;
	    }
	}
    }
    if (side->alt_coverage == NULL && any_los) {
	side->alt_coverage = malloc_area_layer(short);
    }
    return terrainset;
}

// GDL I/O

void
read_view_layer(
    Side *side, Obj *contents, void (*setter)(int x, int y, int val))
{
    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    read_layer(contents, setter);
}

void
read_aux_terrain_view_layer(
    Side *side, Obj *contents, void (*setter)(int x, int y, int val))
{
    int t;
    Obj *typesym = car(contents), *typeval;

    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    if (!symbolp(typesym)) {
	return;
    }
    typeval = eval(typesym);
    if (!ttypep(typeval)) {
	return;
    }
    t = c_number(typeval);
    if (!t_is_cell(t)) {
	return;
    }
    contents = cdr(contents);
    tmpttype = t;
    read_layer(contents, setter);
}

void
read_material_view_layer(
    Side *side, Obj *contents, void (*setter)(int x, int y, int val))
{
    int m;
    Obj *typesym = car(contents), *typeval;

    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    if (!symbolp(typesym)) {
	return;
    }
    typeval = eval(typesym);
    if (!mtypep(typeval)) {
	return;
    }
    m = c_number(typeval);
    contents = cdr(contents);
    tmpmtype = m;
    read_layer(contents, setter);
}

void
write_one_side_view_layer(int propkey, int (*fn)(int x, int y))
{
    newline_form();
    start_form(key(K_SIDE));
    add_num_to_form(tmpside->id);
    space_form();
    start_form(key((enum keywords)propkey));
    newline_form();
    write_rle(fn, -32767, 32767, NULL, tmpcompress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

// xConq
// Lifecycle management and ser/deser of per-side cell views.

// $Id: cellview.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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

#ifndef XCONQ_GDL_SIDE_CELLVIEW_H
#define XCONQ_GDL_SIDE_CELLVIEW_H

#include "gdl/lisp.h"
#include "gdl/side/side_FWD.h"

// Global Constant Macros

//! Cell terrain which has not been seen by side.
#define UNSEEN (0)

// Function Macros: Type Conversion

//! Convert terrain type to terrain view type.
/*!
    Take the terrain type, and convert it to a terrain view type
    by adding 1 to it.
*/
#define buildtview(t) ((t) + 1)

//! Convert terrain view type to terrain type.
/*!
    Take the terrain view type and convert it to a terrain type by
    subtracting 1 from it.
*/
#define vterrain(v) ((v) - 1)

// Function Macros: Game Area View Layer Accessors
/* Macros for accessing and setting a side's view data.  In general,
   interfaces should go through these macros instead of accessing the
   world's state directly. */

//! Get cell value from given side's terrain view layer.
/*! Synthesize terrain view if layer does not exist. */
#define terrain_view(s,x,y)  \
  ((s)->terrview ? aref((s)->terrview, x, y) : buildtview(terrain_at(x, y)))

//! Get cell value from given side's terrain view dates layer.
/*!
    If the terrain view dates layer exists for a side,
    return the terrain view date. If it doesn't, return UNSEEN.
*/
#define terrain_view_date(s,x,y)  \
  ((s)->terrviewdate ? aref((s)->terrviewdate, x, y) : 0)

//! Get cell value from particular aux terrain view layer of given side.
/*!
    If the auxillary terrain view type layer exists for a side,
    and exists for this terrain type, return the auxillary terrain view.
    If a global auxillary view for a terrain type exits, return the global
    auxillary terrain view.
    If it doesn't, return UNSEEN.
*/
#define aux_terrain_view(s,x,y,t)  \
  (((s)->auxterrview && (s)->auxterrview[t]) ? \
        aref((s)->auxterrview[t], x, y) : \
        (aux_terrain_defined(t) ? aux_terrain_at(x, y, t) : 0))

//! Get cell value from particular aux terrain view dates layer of given side.
/*!
    If the auxillary terrain view dates layer exists for a side,
    and exists for this terrain type, then return the aux terrain view date.
    If a global auxillary view for a terrain type exits, return the global
    auxillary terrain view date.
    If it doesn't, return UNSEEN.
*/
#define aux_terrain_view_date(s,x,y,t)  \
  (((s)->auxterrviewdate && (s)->auxterrviewdate[t]) ? \
    aref((s)->auxterrviewdate[t], x, y) : 0)

//! Get cell value from specific material views layer of given side.
#define material_view(s,x,y,m)  \
  (((s)->materialview && (s)->materialview[m]) ? \
    aref((s)->materialview[m], x, y) : \
    (cell_material_defined(m) ? material_at(x, y, m) : 0))

//! Get cell value from specific material view dates layer of given side.
#define material_view_date(s,x,y,m)  \
  (((s)->materialviewdate && (s)->materialviewdate[m]) ? \
    aref((s)->materialviewdate[m], x, y) : 0)

//! Get cell value from given side's temperature views layer.
#define temperature_view(s,x,y)  \
  ((s)->tempview ? aref((s)->tempview, x, y) : \
    (temperatures_defined() ? temperature_at(x, y) : 0))

//! Get cell value of given side's temperature view dates layer.
#define temperature_view_date(s,x,y)  \
  ((s)->tempviewdate ? aref((s)->tempviewdate, x, y) : 0)

//! Get cell value from given side's cloud views layer.
#define cloud_view(s,x,y)  \
  ((s)->cloudview ? aref((s)->cloudview, x, y) : \
    (clouds_defined() ? raw_cloud_at(x, y) : 0))

//! Get cell value of given side's cloud view dates layer.
#define cloud_view_date(s,x,y)  \
  ((s)->cloudviewdate ? aref((s)->cloudviewdate, x, y) : 0)

//! Get cell value from given side's cloud bottom views layer.
#define cloud_bottom_view(s,x,y)  \
  ((s)->cloudbottomview ? \
    aref((s)->cloudbottomview, x, y) : \
    (cloud_bottoms_defined() ? raw_cloud_bottom_at(x, y) : 0))

//! Get cell value from given side's cloud bottom view dates layer.
#define cloud_bottom_view_date(s,x,y)  \
  ((s)->cloudbottomviewdate ? aref((s)->cloudbottomviewdate, x, y) : 0)

//! Get cell value for given side's cloud height views layer.
#define cloud_height_view(s,x,y)  \
  ((s)->cloudheightview ? \
    aref((s)->cloudheightview, x, y) : \
    (cloud_heights_defined() ? raw_cloud_height_at(x, y) : 0))

//! Get cell value of given side's cloud height/top view dates layer.
#define cloud_height_view_date(s,x,y)  \
  ((s)->cloudheightviewdate ? aref((s)->cloudheightviewdate, x, y) : 0)

//! Get cell value from given side's wind views layer.
#define wind_view(s,x,y)  \
  ((s)->windview ? aref((s)->windview, x, y) : (winds_defined() ? raw_wind_at(x, y) : CALM))

//! Get cell value of given side's wind view dates layer.
#define wind_view_date(s,x,y)  \
  ((s)->windviewdate ? aref((s)->windviewdate, x, y) : 0)

//! Set cell value in given side's terrain view layer.
/*!
    Sets the value of terrain view at a cell, if a side has
    terrain view layer defined.  Otherwise does nothing.
*/
#define set_terrain_view(s,x,y,v)  \
  ((s)->terrview ? aset((s)->terrview, x, y, v) : 0)

//! Set cell value in given side's terrain view dates layer.
/*!
    Sets the value of terrain view date at a cell, if a
    side has terrain view layer defined.  Otherwise does nothing.
*/
#define set_terrain_view_date(s,x,y,v)  \
  ((s)->terrviewdate ? aset((s)->terrviewdate, x, y, v) : 0)

//! Set cell value in particular aux terrain view layer of given side.
/*!
    Sets the value of the auxillary terrain view type at a cell, if a
    side has auxillary view defined.  Otherwise does nothing.
*/
#define set_aux_terrain_view(s,x,y,t,v)  \
  (((s)->auxterrview && (s)->auxterrview[t]) ? \
    aset((s)->auxterrview[t], x, y, v) : 0)

//! Set cell value in particular aux terrain view dates layer of given side.
/*!
    Sets the value of the auxillary terrain view type date at a cell,
    if a side has auxillary view defined.  Otherwise does nothing.
*/
#define set_aux_terrain_view_date(s,x,y,t,v)  \
  (((s)->auxterrviewdate && (s)->auxterrviewdate[t]) ? \
    aset((s)->auxterrviewdate[t], x, y, v) : 0)

//! Set cell value in specific material views layer of given side.
#define set_material_view(s,x,y,m,v)  \
  (((s)->materialview && (s)->materialview[m]) ? \
    aset((s)->materialview[m], x, y, v) : 0)

//! Set cell value in given side's material view dates layer.
#define set_material_view_date(s,x,y,m,v)  \
  (((s)->materialviewdate && (s)->materialviewdate[m]) ? \
    aset((s)->materialviewdate[m], x, y, v) : 0)

//! Set cell value in given side's temperature views layer.
#define set_temperature_view(s,x,y,v)  \
  ((s)->tempview ? aset((s)->tempview, x, y, v) : 0)

//! Set cell value in given side's temperture view dates layer.
#define set_temperature_view_date(s,x,y,v)  \
  ((s)->tempviewdate ? aset((s)->tempviewdate, x, y, v) : 0)

//! Set cell value in given side's cloud views layer.
#define set_cloud_view(s,x,y,v)  \
  ((s)->cloudview ? aset((s)->cloudview, x, y, v) : 0)

//! Set cell value for given side's cloud view dates layer.
#define set_cloud_view_date(s,x,y,v)  \
  ((s)->cloudviewdate ? aset((s)->cloudviewdate, x, y, v) : 0)

//! Set cell value for given side's cloud bottom views layer.
#define set_cloud_bottom_view(s,x,y,v)  \
  ((s)->cloudbottomview ? aset((s)->cloudbottomview, x, y, v) : 0)

//! Set cell vlaue for given side's cloud bottom view dates layer.
#define set_cloud_bottom_view_date(s,x,y,v)  \
  ((s)->cloudbottomviewdate ? aset((s)->cloudbottomviewdate, x, y, v) : 0)

//! Set cell value for given side's cloud height/top views layer.
#define set_cloud_height_view(s,x,y,v)  \
  ((s)->cloudheightview ? aset((s)->cloudheightview, x, y, v) : 0)

//! Set cell value in given side's cloud height/top view dates layer.
#define set_cloud_height_view_date(s,x,y,v)  \
  ((s)->cloudheightviewdate ? aset((s)->cloudheightviewdate, x, y, v) : 0)

//! Set cell value for given side's wind views layer.
#define set_wind_view(s,x,y,v)  \
  ((s)->windview ? aset((s)->windview, x, y, v) : 0)

//! Set cell value for given side's wind view dates layer.
#define set_wind_view_date(s,x,y,v)  \
  ((s)->windviewdate ? aset((s)->windviewdate, x, y, v) : 0)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Game Setup

//! Is line-of-sight in effect?
extern short any_los;
//! Any material views in game?
extern int any_material_views;
//! Array of possible material views by type.
extern char *any_material_views_by_m;

// Game Area View Layer Accessors

//! Get cell value from terrain views layer.
extern int fn_terrain_view(int x, int y);
//! Get cell value from terrain view dates layer.
extern int fn_terrain_view_date(int x, int y);
//! Get cell value from aux terrain views layer.
extern int fn_aux_terrain_view(int x, int y);
//! Get cell value from aux terrain view dates layer.
extern int fn_aux_terrain_view_date(int x, int y);
//! Get cell value from material views layer.
extern int fn_material_view(int x, int y);
//! Get cell value from material view dates layer.
extern int fn_material_view_date(int x, int y);
//! Get cell value from temp views layer.
extern int fn_temp_view(int x, int y);
//! Get cell value from temp view dates layer.
extern int fn_temp_view_date(int x, int y);
//! Get cell value from cloud views layer.
extern int fn_cloud_view(int x, int y);
//! Get cell value from cloud bottom views layer.
extern int fn_cloud_bottom_view(int x, int y);
//! Get cell value from cloud top/height views layer.
extern int fn_cloud_height_view(int x, int y);
//! Get cell value from cloud view dates layer.
extern int fn_cloud_view_date(int x, int y);
//! Get cell value from wind views layer.
extern int fn_wind_view(int x, int y);
//! Get cell value from wind view dates layer.
extern int fn_wind_view_date(int x, int y);

//! Set cell value in terrain views layer.
extern void fn_set_terrain_view(int x, int y, int val);
//! Set cell value in terrain view dates layer.
extern void fn_set_terrain_view_date(int x, int y, int val);
//! Set cell value in aux terrain views layer.
extern void fn_set_aux_terrain_view(int x, int y, int val);
//! Set cell value in aux terrain view dates layer.
extern void fn_set_aux_terrain_view_date(int x, int y, int val);
//! Set cell value in material views layer.
extern void fn_set_material_view(int x, int y, int val);
//! Set cell value in terrain view dates layer.
extern void fn_set_material_view_date(int x, int y, int val);
//! Set cell value in temp views layer.
extern void fn_set_temp_view(int x, int y, int val);
//! Set cell value in temp view dates layer.
extern void fn_set_temp_view_date(int x, int y, int val);
//! Set cell value in cloud views layer.
extern void fn_set_cloud_view(int x, int y, int val);
//! Set cell value in cloud bottom views layer.
extern void fn_set_cloud_bottom_view(int x, int y, int val);
//! Set cell value in cloud height/top views layer.
extern void fn_set_cloud_height_view(int x, int y, int val);
//! Set cell value in cloud view dates layer.
extern void fn_set_cloud_view_date(int x, int y, int val);
//! Set cell value in wind views layer.
extern void fn_set_wind_view(int x, int y, int val);
//! Set cell value in wind view dates layer.
extern void fn_set_wind_view_date(int x, int y, int val);

// Lifecycle Management

//! Initialize given side's cell views layers.
/*! Initialize basic viewing structures for a side.  This happens when
    the side is created (during/after reading but before synthesis).
    At this time we're mostly allocating space; filling in the views
    happens once all the units are in position.  We don't know the
    final values of parameters like side->see_all, so we have to
    allocate some data that might never be used.
*/
extern int init_view(Side *side);

// GDL I/O

//! Read side view layer from GDL.
extern void read_view_layer(
    Side *side, Obj *contents, void (*setter)(int, int, int));
//! Read side aux terrain view layer from GDL.
extern void read_aux_terrain_view_layer(
    Side *side, Obj *contents, void (*setter)(int, int, int));
//! Read side material view layer from GDL.
extern void read_material_view_layer(
    Side *side, Obj *contents, void (*setter)(int, int, int));

//! Serialize side view layer to GDL.
extern void write_one_side_view_layer(int propkey, int (*fn)(int x, int y));

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SIDE_CELLVIEW_H

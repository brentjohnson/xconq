// xConq
// Lifecycle management and ser/deser of game area layers.

// $Id: layers.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of game area layers.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_GAMEAREA_LAYERS_H
#define XCONQ_GDL_GAMEAREA_LAYERS_H

#include "gdl/lisp.h"

// Global Constant Macros

//! No people in cell.
/*! A cell might be entirely uninhabited, so need an extra value to indicate.
    This value is chosen to be well above any possible MAXSIDES, and encodes
    in layers as 'X', which is convenient.

    \note It might be better to define this in terms of MAXSIDES, to
    make sure there is no conflict.
*/
#define NOBODY (60)

//! No side controls cell.
/*! A cell might be entirely uncontrolled, so need an extra value to indicate.
    This value is chosen to be well above any possible MAXSIDES, and encodes
    in layers as 'X', which is convenient.

    \note It might be better to define this in terms of MAXSIDES, to
    make sure there is no conflict.
*/
#define NOCONTROL (60)

//! Calm wind force.
#define CALM (0)

//! Cell has no user.
#define NOUSER (0)

// Function Macros: Queries

//! Test if terrain layer has been allocated yet.
#define terrain_defined() (area.terrain != NULL)

//! Test if auxillary terrain layers have been allocated yet.
#define any_aux_terrain_defined() (area.auxterrain != NULL)

//! Test if particular auxillary terrain layer has been allocated yet.
#define aux_terrain_defined(at)  \
  (any_aux_terrain_defined() && area.auxterrain[at] != NULL)

//! Test if elevations layer has been allocated yet.
#define elevations_defined() (area.elevations != NULL)

//! Test if temperatures layer has been allocated yet.
#define temperatures_defined() (area.temperature != NULL)

//! Test if geographical features layer has been allocated yet.
#define features_defined() (area.features != NULL)

//! Test if people sides layer has been allocated yet.
#define people_sides_defined() (area.peopleside != NULL)

//! Test if side control layer has been allocated yet.
#define control_sides_defined() (area.controlside != NULL)

//! Test if materials layers have been allocated yet.
#define any_cell_materials_defined() (area.materials != NULL)

//! Test if particular materials layer has been allocated yet.
#define cell_material_defined(m) (area.materials[m] != NULL)

//! Test if clouds layer has been allocated yet.
#define clouds_defined() (area.clouds != NULL)

//! Test if cloud bottoms layer has been allocated yet.
#define cloud_bottoms_defined() (area.cloudbottoms != NULL)

//! Test if cloud heights layer has been allocated yet.
#define cloud_heights_defined() (area.cloudheights != NULL)

//! Test if winds layer has been allocated yet.
#define winds_defined() (area.winds != NULL)

//! Test if unit user ID layer has been allocated yet.
#define user_defined() (area.user != NULL)

//! Get cell value from layer.
#define aref(m,x,y) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] : 0)

// Function Macros: Game Area Layer Accessors

//! Get unit list at given cell in units layer.
/*!
    The unit is a raw pointer - this macro is used a *lot*.  This could
    be space-optimized by using a 16-bit unit id.  It uses the area.units global
    layer.  Multiple units can be stacked by building a list using the
    'nexthere' pointer.

*/
#define unit_at(x,y) aref(area.units, x, y)

//! Get cell value from terrain layer.
#define terrain_at(x,y) aref(area.terrain, x, y)

//! Get cell value from particular aux terrain layer.
#define aux_terrain_at(x,y,at) aref(area.auxterrain[at], x, y)

//! Is given border type in given direction present in given location?
#define border_at(x,y,dir,b)  \
  (aux_terrain_defined(b) ? (aux_terrain_at(x, y, b) & (1 << (dir))) : FALSE)

//! Are any connection types at given location?
#define any_connections_at(x,y,c) (aux_terrain_at(x, y, c) != 0)

//! Is given connection type in given direction at given location?
#define connection_at(x,y,dir,c)  \
  (aux_terrain_defined(c) ? (aux_terrain_at(x, y, c) & (1 << (dir))) : FALSE)

//! Get cell value from features layer.
#define raw_feature_at(x,y) aref(area.features, x, y)

//! Get cell value from elevations layer.
#define elev_at(x,y) aref(area.elevations, x, y)

//! Get cell value from specific material type layer.
#define material_at(x,y,m) aref(area.materials[m], x, y)

//! Get cell value from temperature layer.
#define temperature_at(x,y) aref(area.temperature, x, y)

//! Get cell value from clouds layer.
#define raw_cloud_at(x,y) aref(area.clouds, x, y)

//! Get cell value from cloud bottoms layer.
#define raw_cloud_bottom_at(x,y) aref(area.cloudbottoms, x, y)

//! Get cell value from cloud heights/tops layer.
#define raw_cloud_height_at(x,y) aref(area.cloudheights, x, y)

//! Get cell value from winds layer.
#define raw_wind_at(x,y) aref(area.winds, x, y)

//! Get cell value from people sides layer.
#define people_side_at(x,y) aref(area.peopleside, x, y)

//! Get cell value from side control layer.
#define control_side_at(x,y) aref(area.controlside, x, y)

//! Get cell value from cell users layer.
#define user_at(x, y) aref(area.user, x, y)

//! Get cell value from scratch layer 1.
#define tmp1_at(x,y) aref(area.tmp1, x, y)

//! Get cell value from scratch layer 2.
#define tmp2_at(x,y) aref(area.tmp2, x, y)

//! Get cell value from scratch layer 3.
#define tmp3_at(x,y) aref(area.tmp3, x, y)

//! Set the value of a cell in a layer.
#define aset(m,x,y,v) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] = (v) : 0)

//! Add value to cell in a layer.
#define aadd(m,x,y,v) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] += (v) : 0)

//! Set unit at cell.
/*!
    Set the head of the unit list at cell(x,y).
    \note This macro DOES NOT link the Unit to any previous Units at
    the location, so it has to be done manually.
*/
#define set_unit_at(x,y,u) aset(area.units, x, y, u)

//! Set cell in terrain layer.
#define set_terrain_at(x,y,t) aset(area.terrain, x, y, t)

//! Set cell in particular aux terrain layer.
#define set_aux_terrain_at(x,y,at,v) aset(area.auxterrain[at], x, y, v)

//! Set cell in elevations layer.
#define set_elev_at(x,y,v) aset(area.elevations, x, y, v)

//! Set cell to particular side in people sides layer.
#define set_people_side_at(x,y,s) aset(area.peopleside, x, y, s)

//! Set cell to given side in side control layer.
#define set_control_side_at(x,y,s) aset(area.controlside, x, y, s)

//! Set cell to given feature ID in features layer.
#define set_raw_feature_at(x,y,f) aset(area.features, x, y, f)

//! Set cell to given amount of material in particular materials layer.
#define set_material_at(x,y,m,v) aset(area.materials[m], x, y, v)

//! Set cell to given temperature in temperatures layer.
#define set_temperature_at(x,y,v) aset(area.temperature, x, y, v)

//! Set cell to given value in clouds layer.
#define set_raw_cloud_at(x,y,v) aset(area.clouds, x, y, v)

//! Set cell to given value in cloud bottoms layer.
#define set_raw_cloud_bottom_at(x,y,v) aset(area.cloudbottoms, x, y, v)

//! Set cell to given value in cloud heights layer.
#define set_raw_cloud_height_at(x,y,v) aset(area.cloudheights, x, y, v)

//! Set cell to given raw wind value in winds layer.
/*! The raw wind value is a combination of direction and force.
    The direction is in the bottom 3 bits of the cell value.
*/
#define set_raw_wind_at(x,y,v) aset(area.winds, x, y, v)

//! Set cell to given unit ID in user unit IDs layer.
#define set_user_at(x, y, u) aset(area.user, x, y, u)

//! Set cell to given value in scratch layer 1.
#define set_tmp1_at(x,y,v) aset(area.tmp1, x, y, v)

//! Set cell to given value in scratch layer 2.
#define set_tmp2_at(x,y,v) aset(area.tmp2, x, y, v)

//! Set cell to given value in scratch layer 3.
#define set_tmp3_at(x,y,v) aset(area.tmp3, x, y, v)

// Function Macros: Lifecycle Management

//! Allocate layer.
/*!
    Use this macro to get a area-spanning layer of the given type.
    This allocates enough memory to hold a value of TYPE for every
    cell in the world.
*/
#define malloc_area_layer(TYPE)  \
  ((TYPE *) xmalloc(area.width * area.height * sizeof(TYPE)))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Game Area Layers Accessors

//! Get cell value from terrain layer.
extern int fn_terrain_at(int x, int y);
//! Get cell value from aux terrains layers.
extern int fn_aux_terrain_at(int x, int y);
//! Get cell value from features layer.
extern int fn_feature_at(int x, int y);
//! Get cell value from elevations layer.
extern int fn_elevation_at(int x, int y);
//! Get cell value from people sides layer.
extern int fn_people_side_at(int x, int y);
//! Get cell value from side control layer.
extern int fn_control_side_at(int x, int y);
//! Get cell value from materials layers.
extern int fn_material_at(int x, int y);
//! Get cell value from temperatures layer.
extern int fn_temperature_at(int x, int y);
//! Get cell value from clouds layer.
extern int fn_raw_cloud_at(int x, int y);
//! Get cell value from cloud bottoms layer.
extern int fn_raw_cloud_bottom_at(int x, int y);
//! Get cell value from cloud tops/heights layer.
extern int fn_raw_cloud_height_at(int x, int y);
//! Get cell value from winds layer.
extern int fn_raw_wind_at(int x, int y);
//! Get cell value from cell users layer.
extern int fn_user_at(int x, int y);

//! Set cell in terrain layer.
extern void fn_set_terrain_at(int x, int y, int val);
//! Set cell in aux terrain layer.
extern void fn_set_aux_terrain_at(int x, int y, int val);
//! Set cell in aux terrain layer to given type and direction of border.
extern void set_border_at(int x, int y, int dir, int t, int onoff);
//! Set cell in aux terrain layer to given type and direction of connection.
extern void set_connection_at(int x, int y, int dir, int t, int onoff);
//! Set cell in side people layer.
extern void fn_set_people_side_at(int x, int y, int val);
//! Set cell in side control layer.
extern void fn_set_control_side_at(int x, int y, int val);
//! Set cell in features layer.
extern void fn_set_raw_feature_at(int x, int y, int val);
//! Set cell in elevations layer.
extern void fn_set_elevation_at(int x, int y, int val);
//! Set cell in materials layer.
extern void fn_set_material_at(int x, int y, int val);
//! Set cell in temperatures layer.
extern void fn_set_temperature_at(int x, int y, int val);
//! Set cell in clouds layer.
extern void fn_set_raw_cloud_at(int x, int y, int val);
//! Set cell in cloud bottoms layer.
extern void fn_set_raw_cloud_bottom_at(int x, int y, int val);
//! Set cell in cloud heights layer.
extern void fn_set_raw_cloud_height_at(int x, int y, int val);
//! Set cell in winds layer.
extern void fn_set_raw_wind_at(int x, int y, int val);
//! Set cell in unit user IDs layer.
extern void fn_set_user_at(int x, int y, int val);

// Lifecycle Management

//! Allocate memory for terrain layer.
/*! Also make space for units layer. */
extern void allocate_area_terrain(void);
//! Allocate memory for aux terrain layer of given type.
extern void allocate_area_aux_terrain(int t);
//! Allocate memory for elevations layer.
extern void allocate_area_elevations(void);
//! Allocate memory for temperatures layer.
extern void allocate_area_temperatures(void);
//! Allocate memory for people sides layer.
extern void allocate_area_people_sides(void);
//! Allocate memory for side control layer.
extern void allocate_area_control_sides(void);
//! Allocate memory for material layer of given type.
extern void allocate_area_material(int m);
//! Allocate memory for clouds layer.
extern void allocate_area_clouds(void);
//! Allocate memory for cloud altitudes layer.
extern void allocate_area_cloud_altitudes(void);
//! Allocate memory for cloud bottoms layer.
extern void allocate_area_cloud_bottoms(void);
//! Allocate memory for cloud heights layer.
extern void allocate_area_cloud_heights(void);
//! Allocate memory for winds layer.
extern void allocate_area_winds(void);
//! Allocate memory for user unit IDs layer.
extern void allocate_area_users(void);
//! Allocate memory for scratch layer with given ID.
/*! \note Currently, no more than 3 allowed.

    \todo Maybe make something to free scratch layers.
*/
extern void allocate_area_scratch(int n);

// GDL I/O
/* The general plan of reading is similar for all layers - create a blank
   layer if none allocated, then call read_layer and pass a function that will
   actually put a value into a cell of the layer.  We need to define those
   functions because most of the setters are macros, and because we can do
   some extra error checking.
*/

//! Read in layer contents and set layer with appropriate setter function.
extern void read_layer(Obj *contents, void (*setter)(int, int, int));

//! Read in terrain layer from GDL form.
extern void fill_in_terrain(Obj *contents);
//! Read in aux terrain layers from GDL form.
extern void fill_in_aux_terrain(Obj *contents);
//! Read in people sides layer from GDL form.
extern void fill_in_people_sides(Obj *contents);
//! Read in side control layer from GDL form.
extern void fill_in_control_sides(Obj *contents);
//! Read in features layer from GDL form.
/*! \todo This should recompute size etc of all these features too. */
extern void fill_in_features(Obj *contents);
//! Read in elevations layer from GDL form.
extern void fill_in_elevations(Obj *contents);
//! Read in materials layers from GDL form.
extern void fill_in_cell_material(Obj *contents);
//! Read in temperatures layer from GDL form.
extern void fill_in_temperatures(Obj *contents);
//! Read in winds layer from GDL form.
extern void fill_in_winds(Obj *contents);
//! Read in clouds layer from GDL form.
extern void fill_in_clouds(Obj *contents);
//! Read in cloud bottoms layer from GDL form.
extern void fill_in_cloud_bottoms(Obj *contents);
//! Read in cloud heights layer from GDL form.
extern void fill_in_cloud_heights(Obj *contents);
//! Read in layer of unit IDs of resources users.
extern void fill_in_users(Obj *contents);

//! Encode and write textual run-length encodings.
/*!
    This is a generalized routine to do run-length-encoding of area
    layers.  It uses hook fns to acquire data at a point and an
    optional translator to do any last-minute fixing.  It can use
    either a char or numeric encoding, depending on the expected range
    of values.
*/
extern void write_rle(
    int (*datafn)(int, int),
    int lo, int hi,
    int (*translator)(int),
    int compress);

//! Serialize terrain layer to GDL.
extern void write_area_terrain(int compress);
//! Serialize aux terrain layers to GDL.
extern void write_area_aux_terrain(int compress);
//! Serialize features layer to GDL.
extern void write_area_features(int compress);
//! Serialize elevations layer to GDL.
extern void write_area_elevations(int compress);
//! Serialize people sides layer to GDL.
extern void write_area_people_sides(int compress);
//! Serialize control sides layer to GDL.
extern void write_area_control_sides(int compress);
//! Serialize materials layers to GDL.
extern void write_area_materials(int compress);
//! Serialize temperatures layer to GDL.
extern void write_area_temperatures(int compress);
//! Serialize clouds layers to GDL.
extern void write_area_clouds(int compress);
//! Serialize winds layer to GDL.
extern void write_area_winds(int compress);
//! Serialize cell users layer to GDL.
extern void write_area_users(int compress);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_GAMEAREA_LAYERS_H

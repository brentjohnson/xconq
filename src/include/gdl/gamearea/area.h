// xConq
// Lifecycle management and ser/deser of game areas.

// $Id: area.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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

#ifndef XCONQ_GDL_GAMEAREA_AREA_H
#define XCONQ_GDL_GAMEAREA_AREA_H

#include "gdl/gamearea/feature.h"
#include "gdl/gamearea/layers.h"

// Global Constant Macros

//! Minimum area width.
/*!
    This is the smallest allowable width for a map.

    \note Theoretically, there is no maximum size to Xconq areas, but the minimum
    size is set by mystical properties, and is not negotiable.
*/
#define MINWIDTH 3

//! Minimum area height.
/*! This is the smallest allowable height for a map. */
#define MINHEIGHT 3

// Function Macros: Queries

//! Is in area?
/*!
    Test whether x,y is a valid position anywhere in the current area.

    \note in_area and inside_area are very heavily used; any
    optimization will likely speed up Xconq overall.
*/
#define in_area(x, y)  \
  (between(0, (y), area.height-1) && x_in_area(x, y))

//! Wrap x coordinate module area width, if area width > world circumference.
/*!
    This little macro implements wraparound in the x direction.

    \note The stupid add of shifted width is for the benefit of brain-damaged
    mod operators that don't handle negative numbers properly.
*/
#define wrapx(x) (area.xwrap ? (((x) + (area.width << 8)) % area.width) : (x))

//! Is x in area?
/*!
    Test whether x is a inside the current area.

    \note in_area and inside_area are very heavily used; any
    optimization will likely speed up Xconq overall.
*/
#define x_in_area(x, y)  \
  (area.xwrap ? TRUE : (between(0, (x), area.width-1) &&  \
			between(area.halfheight,  \
				(x)+(y),  \
				area.width+area.halfheight-1)))

//! Is x a valid unit postion?
/*!
    Test whether x is a inside the current area, accounting for
    wrapping.

    \note in_area and inside_area are very heavily used; any
	  optimization will likely speed up Xconq overall.

    \bug Does x testing work right for even/odd heights?
*/
#define x_inside_area(x, y)  \
  (area.xwrap ? TRUE : (between(1, (x), area.width-2) &&  \
			between(area.halfheight+1,  \
				(x)+(y),  \
				area.width+area.halfheight-2)))

//! Is valid position?
/*!
    Test whether x,y is a valid position anywhere in the current area.

    \note in_area and inside_area are very heavily used; any
	  optimization will likely speed up Xconq overall.

    \bug Does x testing work right for even/odd heights?
*/
#define inside_area(x, y)  \
  (between(1, (y), area.height-2) && x_inside_area(x, y))

// Iterator Macros

//! Iterate over all cells.
/*!
    Iteration over all valid cell positions in a area.  These should be
    used carefully, since they don't (can't) have any grouping braces
    embedded.
*/
#define for_all_cells(x,y)  \
  for (x = 0; x < area.width; ++x)  \
    for (y = 0; y < area.height; ++y)  \
      if (x_in_area(x, y))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

//! Playing area in world.
/*! An "area" is always basically a rectangular array of positions.  The
    hex effect can be achieved by interpreting neighborliness and direction
    differently, but that's all that's needed.  All of the "layers" are
    dynamically allocated as needed, to save (considerable!) space.

    See the discussion of layers for a more complete discussion of the
    various types of layers.
*/
typedef struct a_area {
    //! Width (in cells) of area.
    short width;
    //! Height (in cells) of area.
    short height;
    //! Half-width of area. (width / 2)
    short halfwidth;
    //! Half-height of area. (height / 2)
    short halfheight;
    //! The greater of width or height.
    short maxdim;
    //! True, if x coordinates wrap around. (Because width == circumference).
    short xwrap;
    //! Total number of cells in playing area.
    int numcells;
    //! Latitude of playing area within larger world.
    int latitude;
    //! Longitude of playing area within larger world.
    int longitude;
    //! Projection of coordinate system.
    short projection;
    //! Distance (in altitude units) across one cell.
    int cellwidth;
    //! Width of area being used for data.
    short fullwidth;
    //! Height of area being used for data.
    short fullheight;
    //! X-offset within full area to get data from.
    short fullx;
    //! Y-offset within full area to get data from.
    short fully;
    //! Current x-position of sun.
    short sunx;
    //! Current y-position of sun.
    short suny;
    //! Temperature cycle.
    Obj *temp_year;
    //! Name of image to displayy for map.
    char *image_name;
    //! Units layer.
    struct a_unit **units;
    //! Terrain type layer.
    char *terrain;
    //! Aux terrain types layers.
    char **auxterrain;
    //! People side alignment layer.
    char *peopleside;
    //! Side control layer.
    char *controlside;
    //! Layer of feature IDs.
    short *features;
    //! Layer of elevations.
    short *elevations;
    //! Average elevation across area.
    short avgelev;
    //! Minimum elevation across area.
    short minelev;
    //! Maximum elevation across area.
    short maxelev;
    //! Materials (per type) layers.
    long **materials;
    //! Temperatures layer.
    short *temperature;
    //! Clouds layer.
    short *clouds;
    //! Cloud bottoms layer.
    short *cloudbottoms;
    //! Cloud heights layer.
    short *cloudheights;
    //! Winds layer.
    short *winds;
    /* These layers are for temporary use in calculations. */
    //! Temporary layer 1.
    short *tmp1;
    //! Temporary layer 2.
    short *tmp2;
    //! Temporary layer 3.
    short *tmp3;
    //! Land regions (per terrain type).
    struct a_region **terrain_regions;
    //! Land-sea regions (per land terrain or sea terrain).
    struct a_region **landsea_regions;
    //! Layer of unit IDs using cells for resources.
    short *user;
} Area;

// Global Variables: Uniques

//! The game playing area of the world.
extern Area area;

// Global Variables: Game Setup

//! Any temperture variation during game? */
extern int any_temp_variation;
/*! Any wind variation during game? */
extern int any_wind_variation;
/*! Any clouds are possible? */
extern int any_clouds;

// Queries

//! Distance between two points in playing area.
/*!
    Computing distance in a hexagonal system is a little peculiar,
    since it's sometimes just delta x or y, and other times is the sum.
    Basically there are six formulas to compute distance, depending on
    the direction between the two points.  If the area wraps, this
    routine reports the shortest distance.
*/
extern int distance(int x1, int y1, int x2, int y2);

//! What is nearest point in given direction from given point?
extern int point_in_dir(int x, int y, int dir, int *xp, int *yp);

// Search and Apply Algorithms

//! Search with function around given position to given radius.
/*!
    Generalized area search routine.  It starts in the immediately adjacent
    cells and expands outwards.  The basic structure is to examine successive
    "rings" out to the max distance; within each ring, we must scan each of
    six faces (picking a random one to start with) by iterating along that
    face, in a direction 120 degrees from the direction out to one corner of
    the face.  Draw a picture if you want to understand it...\n
    Incr is normally one.  It is set to area_size to search on areas
    instead of cells.\n
    Predicate can manipulate a counter and a parameter box for returning
    compound results. A result limit can terminate the search if counter
    exceeds it.

    \note Points far outside the map may be generated, but the predicate
    will not be called on them.  It may be applied to the same point several
    times, however, if the distance is enough to wrap around the area.\n

    \todo This needs to be changed to understand different world shapes.
*/
extern int limited_search_around(
    int x0, int y0,
    int range,
    int (*pred)(int, int, int *, ParamBox *),
    int incr,
    int *counter,
    int rsltlimit,
    ParamBox *parambox);

//! Apply function from given position to given radius.
/*!
    Apply a function to every cell within the given radius, being
    careful (for both safety and efficiency reasons) not to go past
    edges.  Note that the distance is inclusive, and that distance of 0
    means x0,y0 only.  Also, if the distance is greater than either map
    dimension, this routine still operates on a correct intersection
    with the area.
*/
extern void apply_to_area(int x0, int y0, int dist, void (*fn)(int, int));

//! Apply function from given position from inner radius to outer radius.
extern void apply_to_ring(
    int x0, int y0, int distmin, int distmax, void (*fn)(int, int));

// Game Setup

//! Set dimensions of playing area.
extern int set_area_shape(int width, int height, int warn);
//! Is the supplied area shape valid?
extern int valid_area_shape(int width, int height, int warn);
//! Check shape of playing area.
extern void check_area_shape(void);

//! Make borders and connections match between adjacent cells.
/*! For a given terrain type. */
extern void patch_linear_terrain(int t);

//! Compute bounds on elevations.
extern void compute_elevation_bounds(void);

// GDL I/O

//! Read in playing area addition from GDL form.
extern void add_to_area(Obj *form);

//! Read GDL playing area form.
/*!
    Only one area, of fixed size.
    Created anew if shape/size is supplied, else just modified.
*/
extern void interp_area(Obj *form);

//! Serialize game areas to GDL.
/*!
    Write info about the area in the world.  This code uses run-length
    encoding to reduce the size of each written layer as much as
    possible.  Note also that each layer is written as a separate form,
    so that the Lisp reader doesn't have to read really large forms
    back in.
*/
extern void write_areas(struct a_module *module);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_GAMEAREA_AREA_H

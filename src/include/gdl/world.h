// xConq
// Lifecycle management and ser/deser of worlds.

// $Id: world.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2005-2006   Eric A. McDonald

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
    \brief Lifecycle management and ser/deser of worlds.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_WORLD_H
#define XCONQ_GDL_WORLD_H

#include "gdl/lisp.h"

// Global Constant Macros

#ifndef DEFAULTWIDTH
//! Default world width.
/*! Default random world size.  Adjust these to taste - for the standard
    game, 60x60 is a moderate length game, 30x30 is short, 360x120 is
    L-O-N-G !  (Remember to multiply the two numbers to get an idea of the
    number of cells that will be in the world!)  Defaults can also be set
    by modules explicitly, so this is really a "default default" :-) .
*/
#define DEFAULTWIDTH 60
#endif

#ifndef DEFAULTHEIGHT
//! Default world height.
#define DEFAULTHEIGHT 30
#endif

#ifndef DEFAULTCIRCUMFERENCE
//! Default world circumference.
/*!
    If this matches the default width, then the game area
    wraps around in x; otherwise it will be a polygon.
*/
#define DEFAULTCIRCUMFERENCE 360
#endif

// Function Macros: Queries

//! Lighting at cell.
/*! - two if in sunlight;
    - one if in twilight; or
    - zero if in darkness.
*/
#define lighting(x,y,snx,sny)  \
  ((world_distance(x, y, snx, sny) < world.daylight_width) ? 2 : \
   ((world_distance(x, y, snx, sny) < world.twilight_width) ? 1 : 0))

//! Night at cell?
#define night_at(x,y)  \
    (daynight && lighting((x), (y), (int) sunx, (int) suny) == 0)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

//! World.
typedef struct a_world {
    //! Circumference (in cells) of world.
    int circumference;
    //! Number of turns in day.
    int daylength;
    //! Number of turns in year.
    int yearlength;
    //! Affects seasons and lighting.
    int axial_tilt;
    //! Percentage of circumference that is lit.
    int daylight_fraction;
    //! Percentage of circumference that is in twilight.
    int twilight_fraction;
    //! Daylight width.
    int daylight_width;
    //! Twilight width.
    int twilight_width;
} World;

// Global Variables: Uniques

//! The World
extern World world;

//! Sun directly over x co-ordinate.
/*!
    The x-location of the sun, as a position relative to the area.  The
    actual values may be far outside the area.
*/
extern int sunx;
//! Sun directly over y co-ordinate.
/*!
    The x-location of the sun, as a position relative to the area.  The
    actual values may be far outside the area.
*/
extern int suny;

// Global Variables: Behavior Options

//! Are there days and nights in game?
extern int daynight;

//! True if the area is to be saved to a different size than it is now.
extern int doreshape;

// Queries

//! Distance between two points on world.
/*! Note that this is not the same as distance in playing area.
    Compute distance in the world, irrespective of area size.  The
    result of this function is undefined if the world circumference
    is 0.
*/
extern int world_distance(int x1, int y1, int x2, int y2);

// Game Setup

//! Set circumference of world.
extern int set_world_circumference(int circum, int warn);

// GDL I/O

//! Read world from GDL form.
extern void interp_world(Obj *form);

//! Compute and return the corresponding point in an area being reshaped.
extern int reshaped_point(int x1, int y1, int *x2p, int *y2p);

//! Serialize world to GDL.
extern void write_world(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_WORLD_H

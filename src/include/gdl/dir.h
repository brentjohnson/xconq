// xConq
// Compass directions.

// $Id: dir.h,v 1.1 2006/05/23 03:17:38 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1994   Stanley T. Shebs
  Copyright (C) 1996	    Stanley T. Shebs
  Copyright (C) 2001	    Stanley T. Shebs
  Copyright (C) 2006	    Eric A. McDonald

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
    \brief Compass directions.
    \ingroup grp_gdl

    The terrain model is based on hexes arranged in horizontal rows. This
    means that although east and west remain intact, the concepts of north
    and south have vanished.
    Unfortunately, not all hex-dependent definitions are here. Pathfinding
    code has some knowledge of hexes also, as does map generation and
    probably parts of the machine player code.
*/

#ifndef XCONQ_GDL_DIR_H
#define XCONQ_GDL_DIR_H

#include "gdl/base.h"

// Global Constant Macros: Limits

//! Number of directions.
#define NUMDIRS 6

// Global Constant Macros: Lookup Tables

//! String names for the directions.
#define DIRNAMES { "NE", "E", "SE", "SW", "W", "NW" }

//! X-delta from direction.
#define DIRX { 0, 1,  1,  0, -1, -1 }
//! Y-delta from direction.
#define DIRY { 1, 0, -1, -1,  0,  1 }

// Function Macros: Queries

//! Oposite direction of given one.
#define opposite_dir(d) (((d) + 3) % NUMDIRS)

// Function Macros: Randomness

//! Generate random direction.
#define random_dir() (xrandom(NUMDIRS))

// Iterator Macros

//! Iteration over the different directions.
/*!
    Iterate through all six directions, going from NORTHEAST to
    NORTHWEST.
*/
#define for_all_directions(dir) \
    for ((dir) = 0; (dir) < NUMDIRS; ++(dir))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Lookup Tables

//! Short names of directions.
extern char *dirnames[];

//! X delta array.
extern int dirx[];
//! Y delta array.
extern int diry[];

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_DIR_H

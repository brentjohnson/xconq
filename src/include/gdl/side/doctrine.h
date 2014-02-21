// xConq
// Lifecycle management and ser/deser of side doctrines.

// $Id: doctrine.h,v 1.3 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of side doctrines.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_DOCTRINE_H
#define XCONQ_GDL_DOCTRINE_H

#include "gdl/lisp.h"
#include "gdl/side/side_FWD.h"

// Iterator Macros

//! Iterate over all doctrines.
#define for_all_doctrines(d)  \
    for ((d) = doctrine_list; (d) != NULL; (d) = (d)->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

//! Doctine.
/*! Doctrine is policy info that units and players use to help decide behavior. */
typedef struct a_doctrine {
    //! ID of doctrine.
    short id;
    //! Name of doctrine.
    char *name;
    //! Do resupply task, if below this.
    short resupply_percent;
    //! Do resupply for ammo, if below this.
    short rearm_percent;
    //! Do repairs, if HP at or below this.
    short repair_percent;
    //! Cease resupplying, if at or above this.
    short resupply_complete;
    //! Cease resupplying ammo, if at or above this.
    short rearm_complete;
    //! Cease repairing, if HP at or above this.
    short repair_complete;
    //! Minimum number of turns for which unit should have starvable supplies.
    short min_turns_food;
    //! Minimum distance for which unit should have fuel.
    short min_distance_fuel;
    //! Array of quantities of unit types to build.
    short *construction_run;
    //! True, if doctrine is immutable by player.
    short locked;
    //! Next doctrine.
    struct a_doctrine *next;
} Doctrine;

// Global Variables

//! List of doctrine objects.
extern Doctrine *doctrine_list;
//! Last doctrine object in list.
extern Doctrine *last_doctrine;

// Queries

//! Get doctrine from doctrine ID.
extern Doctrine *find_doctrine(int id);
//! Get doctrine from doctrine name.
extern Doctrine *find_doctrine_by_name(char *name);

// Lifecycle Management

//! Create new doctrine with given ID.
extern Doctrine *new_doctrine(int id);

// Game Setup

//! Initialize doctrines for given side.
extern void init_doctrine(Side *side);

// GDL I/O

//! Load default doctrine for given side.
/*!
    Get the default doctrine for the side, looking for it by either
    name or number.
*/
extern void read_default_doctrine(Side *side, Obj *props);
//! Get per-unit-type doctrines for the given side.
extern void read_utype_doctrine(Side *side, Obj *list);

//! Read doctrine properties from GDL form.
extern void fill_in_doctrine(struct a_doctrine *doctrine, Obj *props);
//! Read doctrine from GDL form.
extern void interp_doctrine(Obj *form);

//! Serialize side doctrines to GDL.
extern void write_doctrines(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_DOCTRINE_H

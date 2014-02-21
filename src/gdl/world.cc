// xConq
// Lifecycle management and ser/deser of worlds.

// $Id: world.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1986-1989   Stanley T. Shebs
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

#include "gdl/types.h"
#include "gdl/dir.h"
#include "gdl/unit/unit.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Uniques

World world;

int sunx;
int suny;

int daynight = FALSE;

// Global Variables: Behavior Options

int doreshape;

// Queries

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

// Game Setup

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

// GDL I/O

void
interp_world(Obj *form)
{
    int numval;
    Obj *props, *bdg, *propval;
    char *propname;

    props = cdr(form);
    if (symbolp(car(props))) {
	/* This is the id of the world (eventually). */
	props = cdr(props);
    }
    if (numberp(car(props))) {
    	set_world_circumference(c_number(car(props)), TRUE);
	props = cdr(props);
    }
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (numberp(propval))
	  numval = c_number(propval);
	switch (keyword_code(propname)) {
	  case K_CIRCUMFERENCE:
	    set_world_circumference(numval, TRUE);
	    break;
	  case K_DAY_LENGTH:
	    world.daylength = numval;
	    break;
	  case K_YEAR_LENGTH:
	    world.yearlength = numval;
	    break;
	  case K_AXIAL_TILT:
	    world.axial_tilt = numval;
	    break;
	  case K_DAYLIGHT_FRACTION:
	    world.daylight_fraction = numval;
	    break;
	  case K_TWILIGHT_FRACTION:
	    world.twilight_fraction = numval;
	    break;
	  default:
	    unknown_property("world", "", propname);
	}
    }
}

int
reshaped_point(int x1, int y1, int *x2p, int *y2p)
{
    *x2p = (((x1 - reshaper->subarea_x) * reshaper->final_subarea_width )
	    / reshaper->subarea_width ) + reshaper->final_subarea_x;
    *y2p = (((y1 - reshaper->subarea_y) * reshaper->final_subarea_height)
	    / reshaper->subarea_height) + reshaper->final_subarea_y;
    return TRUE;
}

void
write_world(void)
{
    newline_form();
    start_form(key(K_WORLD));
    /* K_CIRCUMFERENCE always written. */
    add_num_to_form((doreshape ? reshaper->final_circumference : world.circumference));
    write_num_prop(key(K_DAY_LENGTH), world.daylength, 1, FALSE, FALSE);
    write_num_prop(key(K_YEAR_LENGTH), world.yearlength, 1, FALSE, FALSE);
    write_num_prop(key(K_AXIAL_TILT), world.axial_tilt, 0, FALSE, FALSE);
    write_num_prop(key(K_DAYLIGHT_FRACTION), world.daylight_fraction, 0, FALSE, FALSE);
    write_num_prop(key(K_TWILIGHT_FRACTION), world.twilight_fraction, 0, FALSE, FALSE);
    end_form();
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END


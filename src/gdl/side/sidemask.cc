// xConq
// Lifecycle management and ser/deser of side masks.

// $Id: sidemask.cc,v 1.3 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of side masks.
    \ingroup grp_gdl
*/

#include "gdl/gvars.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

void
interp_side_mask_list(SideMask *maskp, Obj *lis)
{
    Obj *rest, *head;

    if (maskp == NULL)
      run_error("null mask for side mask list?");
    *maskp = NOSIDES;
    for_all_list(lis, rest) {
    	head = car(rest);
	if (numberp(head)) {
	    *maskp = c_number(head);
	    return;
	} else if (symbolp(head)) {
	    int s2 = c_number(eval(head));

	    if (between(1, s2, g_sides_max()))
	      add_side_to_set(side_n(s2), *maskp);
	    else
	      read_warning("bad side spec");
	} else if (consp(head)) {
	    Obj *sidespec = car(head);
	    int s2, val2 = c_number(cadr(head));

	    if (numberp(sidespec) || symbolp(sidespec)) {
		s2 = c_number(eval(sidespec));
		if (between(1, s2, g_sides_max())) {
		    if (val2)
		      add_side_to_set(side_n(s2), *maskp);
		    /* (should have option to remove a side from mask) */
		} else
		  read_warning("bad side spec");
	    } else if (consp(sidespec)) {
	    	read_warning("not implemented");
	    } else {
	    	read_warning("not implemented");
	    }
	} else {
	    read_warning("not implemented");
	}
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

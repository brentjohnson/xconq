// xConq
// Lifecycle management and ser/deser of namers.

// $Id: namer.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of namers.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/namer.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Variables

//! List of namers.
static Obj *namerlist;
//! Last namer in list.
static Obj *lastnamer;

// Operations

void
merge_unit_namers(Side *side, Obj *lis)
{
    int u, u2 = 0;
    Obj *rest, *element, *types, *typesrest, *type1, *namer;

    for_all_list(lis, rest) {
	element = car(rest);
	if (consp(element)) {
	    types = eval(car(element));
	    namer = cadr(element);
	    if (stringp(namer) || symbolp(namer)) {
		if (utypep(types)) {
		    u = c_number(types);
		    side->unitnamers[u] = c_string(namer);
		} else if (listp(types)) {
		    /* If a list of unit types, iterate. */
		    for_all_list(types, typesrest) {
			type1 = car(typesrest);
			if (utypep(type1)) {
			    u = c_number(type1);
			    side->unitnamers[u] = c_string(namer);
			} else {
			    syntax_error(type1, "not a unit type");
			}
		    }
		} else {
		    syntax_error(types, "garbled unit types for namer");
		}
	    } else {
		syntax_error(element, "garbled unit namer");
	    }
	} else if (stringp(element) || symbolp(element)) {
	    if (u2 < numutypes) {
		side->unitnamers[u2++] = c_string(element);
	    } else {
		read_warning("more unit namers than unit types, ignoring extra");
	    }
	} else {
	    syntax_error(element, "not a valid unit namer");
	}
    }
}

// Lifecycle Management

Obj *
make_namer(Obj *sym, Obj *meth)
{
    Obj *namer = new_pointer(sym, (char *) meth), *tmpobj;

    /* Append the new namer to the end of the list of namers. */
    tmpobj = cons(namer, lispnil);
    if (lastnamer != lispnil) {
	set_cdr(lastnamer, tmpobj);
	lastnamer = tmpobj;
    } else {
	namerlist = lastnamer = tmpobj;
    }
    return namer;
}

// GDL I/O

void
interp_namer(Obj *form)
{
    Obj *id = cadr(form), *meth = caddr(form);

    if (symbolp(id)) {
	setq(id, make_namer(id, meth));
    }
}

void
write_namers(void)
{
    extern Obj *namerlist;
    Obj *rest, *namer;

    for_all_list(namerlist, rest) {
	namer = car(rest);
	start_form(key(K_NAMER));
	space_form();
	add_form_to_form(namer->v.ptr.sym);
	newline_form();
	space_form();
	space_form();
	add_form_to_form((Obj *) namer->v.ptr.data);
	newline_form();
	space_form();
	space_form();
	end_form();
	newline_form();
	newline_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

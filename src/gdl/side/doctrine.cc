// xConq
// Lifecycle management and ser/deser of side doctrines.

// $Id: doctrine.cc,v 1.3 2006/06/02 16:57:43 eric_mcdonald Exp $

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

#include "gdl/types.h"
#include "gdl/side/doctrine.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Doctrine *doctrine_list;
Doctrine *last_doctrine;

// Queries

Doctrine *
find_doctrine_by_name(char *name)
{
    Doctrine *doctrine;

    if (name == NULL)
      return NULL;
    for_all_doctrines(doctrine) {
	if (doctrine->name != NULL && strcmp(name, doctrine->name) == 0)
	  return doctrine;
    }
    return NULL;
}

Doctrine *
find_doctrine(int id)
{
    Doctrine *doctrine;

    for_all_doctrines(doctrine) {
	if (doctrine->id == id)
	  return doctrine;
    }
    return NULL;
}

// Game Setup

void
init_doctrine(Side *side)
{
    int u;

    /* Every side must have a default doctrine to fall back on. */
    if (side->default_doctrine == NULL) {
	side->default_doctrine = new_doctrine(0);
    }
    /* Make each individual unit doctrine point to the generic
       doctrine by default. */
    if (side->udoctrine == NULL) {
	side->udoctrine =
	  (Doctrine **) xmalloc(numutypes * sizeof(Doctrine *));
    }
    for_all_unit_types(u) {
	if (side->udoctrine[u] == NULL)
	  side->udoctrine[u] = side->default_doctrine;
    }
}

// GDL I/O

void
read_default_doctrine(Side *side, Obj *props)
{
    int id = 0;
    char *name = NULL;
    Obj *ident = lispnil;
    Doctrine *doctrine = NULL;

    if (props == lispnil) {
	read_warning("no doctrine information supplied");
	return;
    }
    ident = car(props);
    if (numberp(ident)) {
	id = c_number(ident);
	doctrine = find_doctrine(id);
    } else if (symbolp(ident) || stringp(ident)) {
	name = c_string(ident);
	doctrine = find_doctrine_by_name(name);
    } else {
	type_error(ident, "invalid doctrine reference");
	return;
    }
    /* Make sure we have a doctrine to fill in the side's field. */
    if (doctrine == NULL) {
	doctrine = new_doctrine(id);
	doctrine->name = name;
    }
    side->default_doctrine = doctrine;
}

void
read_utype_doctrine(Side *side, Obj *lis)
{
    int u, id;
    char *name;
    Obj *item, *rest, *ulist, *props, *ident;
    Doctrine *doctrine;

    if (rd_u_arr == NULL)
      rd_u_arr = (short *) xmalloc(numutypes * sizeof(short));

    for_all_list(lis, rest) {
	item = car(rest);
	ulist = car(item);
	props = cdr(item);
	for_all_unit_types(u)
	  rd_u_arr[u] = FALSE;
	interp_utype_list(rd_u_arr, ulist);
	ident = car(props);
	id = 0;
	name = NULL;
	if (numberp(ident)) {
	    id = c_number(ident);
	    doctrine = find_doctrine(id);
	} else if (symbolp(ident) || stringp(ident)) {
	    name = c_string(ident);
	    doctrine = find_doctrine_by_name(name);
	} else {
	    type_error(ident, "invalid doctrine reference");
	    return;
	}
	if (doctrine == NULL) {
	    doctrine = new_doctrine(id);
	    doctrine->name = name;
	}
	for_all_unit_types(u) {
	    if (rd_u_arr[u]) {
		if (side->udoctrine == NULL)
		  init_doctrine(side);
		side->udoctrine[u] = doctrine;
	    }
	}
    }
}

void
fill_in_doctrine(Doctrine *doctrine, Obj *props)
{
    int numval;
    char *propname;
    Obj *bdg, *val;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	if (numberp(val))
	  numval = c_number(val);
	switch (keyword_code(propname)) {
	  case K_RESUPPLY_PERCENT:
	    doctrine->resupply_percent = numval;
	    break;
	  case K_REARM_PERCENT:
	    doctrine->rearm_percent = numval;
	    break;
	  case K_REPAIR_PERCENT:
	    doctrine->repair_percent = numval;
	    break;
	  case K_CONSTRUCTION_RUN:
	    interp_utype_value_list(doctrine->construction_run, cdr(bdg));
	    break;
	  case K_LOCKED:
	    doctrine->locked = numval;
	    break;
	  default:
	    unknown_property("doctrine", "", propname);
	}
    }
}

void
interp_doctrine(Obj *form)
{
    int id = -1;
    char *name = NULL;
    Obj *ident = lispnil, *props = cdr(form);
    Doctrine *doctrine = NULL;

    /* Pick up an optional numeric or symbolic id. */
    if (props != lispnil) {
	if (!consp(car(props))) {
	    ident = car(props);
	    props = cdr(props);
	}
    }
    /* Use the id to find an existing doctrine. */
    if (numberp(ident)) {
	id = c_number(ident);
	doctrine = find_doctrine(id);
    }
    if (symbolp(ident)) {
	name = c_string(ident);
	doctrine = find_doctrine_by_name(name);
    }
    if (doctrine == NULL) {
	doctrine = new_doctrine(id);
	doctrine->name = name;
    }
    /* Give the symbol a binding so eval'ing works. */
    if (symbolp(ident))
      setq(ident, new_string(name));
    fill_in_doctrine(doctrine, props);
    Dprintf("  Got doctrine\n");
}

void
write_doctrines(void)
{
    Doctrine *doc;

    for_all_doctrines(doc) {
	/* (should filter out doctrines with no non-default values) */
	start_form(key(K_DOCTRINE));
	if (doc->name)
	  add_to_form(escaped_symbol(doc->name));
	else
	  add_num_to_form(doc->id);
	newline_form();
	space_form();
	write_num_prop(key(K_RESUPPLY_PERCENT), doc->resupply_percent,
		       /* (should get these defaults from a common place) */
		       50, FALSE, TRUE);
	write_num_prop(key(K_REARM_PERCENT), doc->rearm_percent,
		       20, FALSE, TRUE);
	write_num_prop(key(K_REPAIR_PERCENT), doc->repair_percent,
		       35, FALSE, TRUE);
	write_utype_value_list(key(K_CONSTRUCTION_RUN), doc->construction_run,
			       0, TRUE);
	write_num_prop(key(K_LOCKED), doc->locked, 0, FALSE, TRUE);
	space_form();
	end_form();
	newline_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END


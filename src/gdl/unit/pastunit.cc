// xConq
// Lifecycle management and ser/deser of past units.

// $Id: pastunit.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of past units.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/desc.h"
#include "gdl/unit/pastunit.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

PastUnit *past_unit_list;

// Local Variables

//! End of past unit list.
static PastUnit *last_past_unit;

//! The ID of the next past unit.
/*! -1 is reserved, so start counting down from -2. */
static int next_past_unit_id = -2;

// Local Variables: Buffers

static char *past_unitbuf = NULL;

// Queries

PastUnit *
find_past_unit(int n)
{
    PastUnit *pastunit;

    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next) {
	if (pastunit->id == n)
	  return pastunit;
    }
    return NULL;
}

char *
past_unit_handle(Side *side, PastUnit *past_unit)
{
    char *utypename;
    Side *side2;

    if (past_unitbuf == NULL)
      past_unitbuf = (char *)xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (past_unit == NULL)
      return "???";
    /* Decide how to identify the side. */
    side2 = past_unit->side;
    if (side2 == NULL) {
	sprintf(past_unitbuf, "the ");
    } else if (side2 == side) {
	sprintf(past_unitbuf, "your ");
    } else {
	/* If the side adjective is a genitive (ends in 's, s' or z')
	   we should skip the definite article. */
	int len = strlen(side_adjective(side2));
	char *end = side_adjective(side2) + len - 2;

	if (strcmp(end, "'s") != 0
	    && strcmp(end, "s'") != 0
	    && strcmp(end, "z'") != 0)
	  sprintf(past_unitbuf, "the ");
	sprintf(past_unitbuf, side_adjective(side2));
	strcat(past_unitbuf, " ");
    }
    /* Now add the past_unit's unique description. */
    utypename = u_type_name(past_unit->type);
    if (past_unit->name) {
	tprintf(past_unitbuf, "%s %s", utypename, past_unit->name);
    } else if (past_unit->number > 0) {
	tprintf(past_unitbuf, "%d%s %s",
		past_unit->number, ordinal_suffix(past_unit->number),
		utypename);
    } else {
	strcat(past_unitbuf, utypename);
    }
    return past_unitbuf;
}

// Lifecycle Management

PastUnit *
create_past_unit(int type, int id)
{
    PastUnit *pastunit = (PastUnit *) xmalloc(sizeof(PastUnit));

    pastunit->type = type;
    if (id == 0)
      id = next_past_unit_id--;
    else
      next_past_unit_id = min(next_past_unit_id, id - 1);
    pastunit->id = id;
    /* Glue at the end of the list, so all stays sorted. */
    if (past_unit_list == NULL) {
	past_unit_list = last_past_unit = pastunit;
    } else {
	last_past_unit->next = pastunit;
	last_past_unit = pastunit;
    }
    return pastunit;
}

// GDL I/O

void
interp_past_unit(Obj *form)
{
    int u = NONUTYPE, nid;
    char *propname;
    Obj *props, *bdg, *propval, *idval;
    PastUnit *pastunit;

    Dprintf("Reading a past unit from ");
    Dprintlisp(form);
    Dprintf("\n");
    props = cdr(form);
    idval = car(props);
    TYPECHECK(numberp, idval, "id not a number")
    nid = c_number(idval);
    props = cdr(props);
    if (symbolp(car(props))) {
	u = utype_from_symbol(car(props));
	props = cdr(props);
    }
    if (u == NONUTYPE) {
	read_warning("bad exu");
	return;
    }
    pastunit = create_past_unit(u, nid);
    /* Peel off fixed-position properties, if they're supplied. */
    if (numberp(car(props))) {
	pastunit->x = c_number(car(props));
	props = cdr(props);
    }
    if (numberp(car(props))) {
	pastunit->y = c_number(car(props));
	props = cdr(props);
    }
    if (!consp(car(props))) {
	pastunit->side = side_n(c_number(eval(car(props))));
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	switch (keyword_code(propname)) {
	  case K_Z:
	    pastunit->z = c_number(propval);
	    break;
	  case K_N:
	    pastunit->name = c_string(propval);
	    break;
	  case K_NB:
	    pastunit->number = c_number(propval);
	    break;
	  default:
	    unknown_property("exu", "??", propname);
	}
    }
}

void
write_past_unit(PastUnit *pastunit)
{
    start_form(key(K_EXU));
    add_num_to_form(pastunit->id);
    add_to_form(shortest_escaped_name(pastunit->type));
    add_num_to_form(pastunit->x);
    add_num_to_form(pastunit->y);
    add_num_to_form(side_number(pastunit->side));
    write_num_prop(key(K_Z), pastunit->z, 0, FALSE, FALSE);
    write_str_prop(key(K_N), pastunit->name, NULL, FALSE, FALSE);
    write_num_prop(key(K_NB), pastunit->number, 0, FALSE, FALSE);
    end_form();
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

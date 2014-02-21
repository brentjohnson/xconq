// xConq
// Lifecycle management and ser/deser of pacts among sides.

// $Id: pact.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of pacts among sides.
    \ingroup grp_gdl
*/

#include "gdl/pact.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

#if (0)

void
interp_agreement(Obj *form)
{
    int id = 0;
    char *propname;
    Obj *props = cdr(form), *agid, *bdg, *val;
    Agreement *ag;

    agid = car(props);
    if (numberp(agid)) {
    	id = c_number(agid);
	/* should use the number eventually */
	props = cdr(props);
    }
    if (1 /* must create a new agreement object */) {
	ag = create_agreement(id);
	/* Fill in defaults for the slots. */
	ag->state = draft;  /* default for now */
	ag->drafters = NOSIDES;
	ag->proposers = NOSIDES;
	ag->signers = NOSIDES;
	ag->willing = NOSIDES;
	ag->knownto = NOSIDES;
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_TYPE_NAME:
	    ag->agtype = c_string(val);
	    break;
	  case K_NAME:
	    ag->name = c_string(val);
	    break;
	  case K_STATE:
	    ag->state = c_number(val);
	    break;
	  case K_TERMS:
	    ag->terms = val;
	    break;
	  case K_DRAFTERS:
	    break;
	  case K_PROPOSERS:
	    break;
	  case K_SIGNERS:
	    break;
	  case K_WILLING_TO_SIGN:
	    break;
	  case K_KNOWN_TO:
	    break;
	  case K_ENFORCEMENT:
	    ag->enforcement = c_number(val);
	    break;
	    break;
	  default:
	    unknown_property("agreement", "", propname);
	}
    }
}

void
write_agreement(Agreement *agreement)
{
    start_form(key(K_AGREEMENT));
    add_num_to_form(agreement->id);
    newline_form();
    space_form();
    write_str_prop(key(K_TYPE_NAME), agreement->agtype, "", FALSE, TRUE);
    write_str_prop(key(K_NAME), agreement->name, "", FALSE, TRUE);
    write_num_prop(key(K_STATE), agreement->state, 0, FALSE, TRUE);
    write_lisp_prop(key(K_TERMS), agreement->terms, lispnil, FALSE, FALSE, TRUE);
    write_num_prop(key(K_DRAFTERS), agreement->drafters, 0, FALSE, TRUE);
    write_num_prop(key(K_PROPOSERS), agreement->proposers, 0, FALSE, TRUE);
    write_num_prop(key(K_SIGNERS), agreement->signers, 0, FALSE, TRUE);
    write_num_prop(key(K_WILLING_TO_SIGN), agreement->willing, 0, FALSE, TRUE);
    write_num_prop(key(K_KNOWN_TO), agreement->knownto, 0, FALSE, TRUE);
    write_num_prop(key(K_ENFORCEMENT), agreement->enforcement, 0, FALSE, TRUE);
    space_form();
    end_form();
    newline_form();
}

void
write_agreements(void)
{
    Agreement *ag;

    for_all_agreements(ag) {
	write_agreement(ag);
    }
}

#endif

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

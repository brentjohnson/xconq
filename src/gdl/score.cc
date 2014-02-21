// xConq
// Lifecycle management and ser/deser of scorekeepers and scores.

// $Id: score.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of scorekeepers and scores.
    \ingroup grp_gdl

    Scoring in Xconq does not happen by default; instead a game design
    may include one or more "scorekeepers", which are objects with
    properties that define how winning and losing will be decided.

    There are two kinds of functionality in scorekeepers.  One kind is
    the ability to end a game based on a specified condition, such as
    "the Star of Satyria enters Noblum".  The other is the accumulation
    of a numeric score.  A scorekeeper may include both, so it can (for
    instance) end the game the instant that a player's score reaches
    100, or 100 more than another player, or whatever.

    The indepside should not participate in scoring, so the iterations
    in this file generally iterate only over the "real" sides.
*/

#include "gdl/score.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Scorekeeper *scorekeepers;
int numscorekeepers;
int numscores;

int any_post_action_scores;
int any_post_event_scores;

// Local Variables

//! The end of the list of scorekeepers.
static Scorekeeper *last_scorekeeper;

// Local Variables: Counters

//! ID of next scorekeeper created.
static int nextskid;

// Queries

Scorekeeper *
find_scorekeeper(int id)
{
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	if (sk->id == id)
	  return sk;
    }
    return NULL;
}

// Lifecycle Management

Scorekeeper *
create_scorekeeper(void)
{
    Scorekeeper *sk = (Scorekeeper *) xmalloc(sizeof(Scorekeeper));

    /* Initialize any nonzero fields. */
    sk->id = nextskid++;
    sk->when = lispnil;
    sk->who = lispnil;
    sk->whomask = ALLSIDES;
    sk->knownto = lispnil;
    sk->knowntomask = ALLSIDES;
    sk->trigger = lispnil;
    sk->body = lispnil;
    sk->record = lispnil;
    sk->notes = lispnil;
    sk->scorenum = -1;
    sk->keepscore = TRUE;
    sk->initial = 0;
    sk->triggered = FALSE;
    /* Add the new scorekeeper to the end of the list. */
    if (last_scorekeeper != NULL) {
	last_scorekeeper->next = sk;
	last_scorekeeper = sk;
    } else {
	scorekeepers = last_scorekeeper = sk;
    }
    ++numscorekeepers;
    return sk;
}

// GDL I/O

void
interp_scorekeeper(Obj *form)
{
    int id = 0;
    char *propname;
    Obj *props = cdr(form), *bdg, *propval;
    Scorekeeper *sk = NULL;

    if (numberp(car(props))) {
	id = c_number(car(props));
	props = cdr(props);
    }
    if (id > 0) {
	sk = find_scorekeeper(id);
    }
    /* Create a new scorekeeper object if necessary. */
    if (sk == NULL) {
	sk = create_scorekeeper();
	if (id > 0) {
	    sk->id = id;
	}
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	switch (keyword_code(propname)) {
	  case K_TITLE:
	    sk->title = c_string(propval);
	    break;
	  case K_WHEN:
	    sk->when = propval;
	    break;
	  case K_APPLIES_TO:
	    sk->who = propval;
	    break;
	  case K_KNOWN_TO:
	    sk->knownto = propval;
	    break;
	  case K_TRIGGER:
	    sk->trigger = propval;
	    break;
	  case K_DO:
	    sk->body = propval;
	    break;
	  case K_TRIGGERED:
	    sk->triggered = c_number(propval);
	    break;
	  case K_KEEP_SCORE:
	    if (symbolp(propval) && boundp(propval))
	      propval = eval_symbol(propval);
	    sk->keepscore = c_number(propval);
	    break;
	  case K_INITIAL_SCORE:
	    sk->initial = c_number(propval);
	    break;
	  case K_NOTES:
	    sk->notes = propval;
	    break;
	  default:
	    unknown_property("scorekeeper", "??", propname);
	}
    }
}

void
write_scorekeepers(void)
{
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	start_form(key(K_SCOREKEEPER));
	add_num_to_form(sk->id);
	newline_form();
	space_form();
	write_str_prop(key(K_TITLE), sk->title, "", FALSE, TRUE);
	write_lisp_prop(key(K_WHEN), sk->when, lispnil, FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_APPLIES_TO), sk->who, lispnil,
			FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_KNOWN_TO), sk->who, lispnil, FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_TRIGGER), sk->trigger, lispnil,
			FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_DO), sk->body, lispnil, FALSE, FALSE, TRUE);
	write_num_prop(key(K_TRIGGERED), sk->triggered, 0, FALSE, TRUE);
	write_num_prop(key(K_KEEP_SCORE), sk->keepscore, 0, FALSE, TRUE);
	write_num_prop(key(K_INITIAL_SCORE), sk->initial, -10001, FALSE, TRUE);
	write_lisp_prop(key(K_NOTES), sk->notes, lispnil, FALSE, FALSE, TRUE);
	space_form();
	end_form();
	newline_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

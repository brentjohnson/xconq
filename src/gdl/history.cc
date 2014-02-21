// xConq
// Lifecycle management and ser/deser of game history.

// $Id: history.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of game history.
    \ingroup grp_gdl
*/

#include "gdl/base.h"
#include <cstdarg>

#include "gdl/kernel.h"
#include "gdl/ui.h"
#include "gdl/types.h"
#include "gdl/gvars.h"
#include "gdl/unit/pastunit.h"
#include "gdl/history.h"
#include "gdl/score.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

HevtDefn hevtdefns[] = {

#ifdef  DEF_HEVT
#undef  DEF_HEVT
#endif
#define DEF_HEVT(NAME, code, DATADESCS) { NAME, DATADESCS },

#include "gdl/history.def"

    { NULL, NULL }

};

// Global Variables

HistEvent *history;

// Local Variables: Behavior Options

//! True, if events are being recorded into the history.
static int recording_events;

// Local Function Declarations: History Management

//! Notify side of event.
static void notify_event(Side *side, HistEvent *hevt);

// Queries

int
find_event_type(Obj *sym)
{
    int i;

    for (i = 0; hevtdefns[i].name != NULL; ++i) {
	if (strcmp(c_string(sym), hevtdefns[i].name) == 0)
	  return i;
    }
    return -1;
}

int
pattern_matches_event(Obj *pattern, HistEvent *hevt)
{
    int data0, u;
    Obj *rest, *subpat;
    PastUnit *pastunit;

    if (find_event_type(car(pattern)) != hevt->type)
      return FALSE;
    data0 = hevt->data[0];
    for_all_list(cdr(pattern), rest) {
	subpat = car(rest);
	if (symbolp(subpat)) {
	    switch (hevt->type) {
	      case H_UNIT_STARVED:
	      case H_UNIT_VANISHED:
		u = utype_from_name(c_string(subpat));
		pastunit = find_past_unit(data0);
		if (pastunit != NULL && pastunit->type == u)
		  return TRUE;
		else
		  return FALSE;
		break;
	      default:
		return FALSE;
	    }
	} else {
	    /* (should warn of bad pattern syntax?) */
	    return FALSE;
	}
    }
    return TRUE;
}

void
event_desc_from_list(Side *side, Obj *lis, HistEvent *hevt, char *buf)
{
    int n;
    Obj *rest, *item;
    PastUnit *pastunit;

    buf[0] = '\0';
    for_all_list(lis, rest) {
	item = car(rest);
	if (stringp(item)) {
	    strcat(buf, c_string(item));
	} else if (numberp(item)) {
	    n = c_number(item);
	    if (between(0, n, 3)) {
		switch (hevt->type) {
		  case H_UNIT_STARVED:
		  case H_UNIT_VANISHED:
		    pastunit = find_past_unit(hevt->data[0]);
		    if (pastunit != NULL) {
			strcat(buf, past_unit_handle(side, pastunit));
		    } else {
			tprintf(buf, "%d?", hevt->data[0]);
		    }
		    break;
		  /* (should add other event types) */
		  default:
		    break;
		}
	    } else {
		tprintf(buf, " ??%d?? ", n);
	    }
	} else {
	    strcat(buf, " ????? ");
	}
    }
}

// History Management

static
void
notify_event(Side *side, HistEvent *hevt)
{
    char abuf[BUFSIZE];
    Obj *rest, *head, *pattern, *text;
    Side *side2;
    Unit *unit;

    for_all_list(g_event_notices(), rest) {
	head = car(rest);
	if (consp(head)) {
	    pattern = car(head);
	    if (symbolp(pattern)
		&& find_event_type(pattern) == hevt->type) {
		text = cadr(head);
		if (stringp(text)) {
		    sprintf(abuf, c_string(text));
		} else {
		    sprintlisp(abuf, text, 50);
		}
		notify(side, "%s", abuf);
		return;
	    } else if (consp(pattern)
		       && symbolp(car(pattern))
		       && pattern_matches_event(pattern, hevt)
		       ) {
		text = cadr(head);
		if (stringp(text)) {
		    strcpy(abuf, c_string(text));
		} else {
		    event_desc_from_list(side, text, hevt, abuf);
		}
		notify(side, "%s", abuf);
		return;
	    }
	}
    }
    /* If we didn't find anything special, still put out some generic
       messages for important cases. */
    switch (hevt->type) {
      case H_SIDE_LOST:
	if (hevt->data[0] == side_number(side)) {
	    notify(side, "You lost!");
	} else {
	    notify(side, "%s lost!", side_desig(side_n(hevt->data[0])));
	}
	break;
      case H_SIDE_WON:
	if (hevt->data[0] == side_number(side)) {
	    notify(side, "You won!");
	} else {
	    notify(side, "%s won!", side_desig(side_n(hevt->data[0])));
	}
	break;
#if 0 /* this already has a special message */
      case H_GAME_ENDED:
	notify(side, "The game is over!");
	break;
#endif
      case H_UNIT_COMPLETED:
	side2 = side_n(hevt->data[0]);
	unit = find_unit(hevt->data[1]);
	if (unit != NULL) {
	    if (side2 == side) {
		notify(side, "You completed %s.",
		       unit_handle(side, unit));
	    } else {
		notify(side, "%s completed %s.",
		       side_desig(side2), unit_handle(side2, unit));
	    }
	}
	break;
      case H_UNIT_CREATED:
	side2 = side_n(hevt->data[0]);
	unit = find_unit(hevt->data[1]);
	if (unit != NULL) {
	    if (side2 == side) {
		notify(side, "You created %s.",
		       unit_handle(side, unit));
	    } else {
		notify(side, "%s created %s.",
		       side_desig(side2), unit_handle(side2, unit));
	    }
	}
	break;
      default:
	break;
    }
}

HistEvent *
record_event(HistEventType type, SideMask observers, ...)
{
    int i, val;
    char *descs;
    HistEvent *hevt;
    Side *side;
    va_list ap;

    if (!recording_events)
      return NULL;
    hevt = create_historical_event(type);
    hevt->startdate = g_turn();
    hevt->enddate = g_turn();
    hevt->observers = observers;
    descs = hevtdefns[type].datadescs;

    va_start(ap, observers);
    for (i = 0; descs[i] != '\0'; ++i) {
	if (i >= 4)
	  run_error("hevt type %d has too many parameters", type);
	val = va_arg(ap, int);
	hevt->data[i] = val;
    }
    va_end(ap);

    /* Check on plausibility of event data. */
    for (i = 0; descs[i] != '\0'; ++i) {
	val = hevt->data[i];
	switch (descs[i]) {
	  case 'S':
	    if (!between(0, val, numsides))
	      run_warning("invalid side number %d in hist event", val);
	    break;
	  case 'U':
	    /* Note that when validating unit id, the unit may be in
	       the middle of the the process of becoming a past unit,
	       so allowing finding a dead unit. */
	    if (val != 0
		&& find_unit_dead_or_alive(val) == NULL
		&& find_past_unit(val) == NULL)
	      run_warning("invalid unit/pastunit id %d in hist event", val);
	    break;
	  default:
	    break;
	}
    }
    /* Insert the newly created event. */
    hevt->next = history;
    hevt->prev = history->prev;
    history->prev->next = hevt;
    history->prev = hevt;
    Dprintf("Recorded event %s (observed by %d)\n",
	    hevtdefns[hevt->type].name, hevt->observers);
    if (observers != NOSIDES) {
	/* Let all the observers' interfaces look at this event. */
	for_all_sides(side) {
	    if (side_in_set(side, observers) && active_display(side)) {
		update_event_display(side, hevt, TRUE);
		notify_event(side, hevt);
		if (g_event_movies() != lispnil) {
		    play_event_movies(side, hevt);
		}
	    }
	}
    }
    /* Flag that we should look at post-event scorekeepers soon. */
    if (any_post_event_scores && !beforestart && !endofgame)
      need_post_event_scores = TRUE;
    return hevt;
}

// Lifecycle Management

HistEvent *
create_historical_event(HistEventType type)
{
    HistEvent *hevt = (HistEvent *) xmalloc(sizeof(HistEvent));

    if (type >= NUMHEVTTYPES)
      run_warning("unknown hist event type %d", type);
    hevt->type = type;
    hevt->observers = ALLSIDES;
    hevt->next = hevt->prev = NULL;
    return hevt;
}

// GDL I/O

void
interp_history(Obj *form)
{
    int startdate, type, i;
    char *evttype;
    SideMask observers;
    Obj *props, *val;
    HistEvent *hevt;

    Dprintf("Reading a hist event from ");
    Dprintlisp(form);
    Dprintf("\n");
    props = cdr(form);
    /* Get the event's date. */
    val = car(props);
    TYPECHECK(numberp, val, "date not a number")
    startdate = c_number(val);
    props = cdr(props);
    /* Get the event's type. */
    val = car(props);
    TYPECHECK(symbolp, val, "type not a symbol")
    evttype = c_string(val);
    /* (should be in separate routine) */
    type = -1;
    for (i = 0; hevtdefns[i].name != NULL; ++i) {
	if (strcmp(evttype, hevtdefns[i].name) == 0) {
	    type = i;
	    break;
	}
    }
    if (type < 0) {
	read_warning("Historical event type `%s' not recognized", evttype);
	return;
    }
    props = cdr(props);
    /* Get the bit vector of observers. */
    if (numberp(car(props))) {
	observers = c_number(car(props));
	props = cdr(props);
    } else if (symbolp(car(props))
	       && keyword_code(c_string(car(props))) == K_ALL) {
	observers = ALLSIDES;
	props = cdr(props);
    } else {
	syntax_error(form, "bad hevt observers");
	return;
    }
    hevt = create_historical_event((HistEventType)type);
    hevt->startdate = startdate;
    hevt->observers = observers;
    /* Read up to 4 remaining numbers. */
    i = 0;
    for (; props != lispnil && i < 4; props = cdr(props)) {
	hevt->data[i++] = c_number(car(props));
    }
    /* Insert the newly created event. */
    /* (linking code should be in its own routine) */
    hevt->next = history;
    hevt->prev = history->prev;
    history->prev->next = hevt;
    history->prev = hevt;
}

void
write_historical_event(HistEvent *hevt)
{
    int i;
    char *descs;

    /* Might be reasons not to write this event. */
    if (hevt->startdate < 0)
      return;
    start_form(key(K_EVT));
    add_num_to_form(hevt->startdate);
    add_to_form(hevtdefns[hevt->type].name);
    if (hevt->observers == ALLSIDES)
      add_to_form(key(K_ALL));
    else
      add_num_to_form(hevt->observers);
    descs = hevtdefns[hevt->type].datadescs;
    for (i = 0; descs[i] != '\0'; ++i) {
	switch (descs[i]) {
	  case 'm':
	  case 'n':
	  case 's':
	  case 'S':
	  case 'u':
	  case 'U':
	  case 'x':
	  case 'y':
	    add_num_to_form(hevt->data[i]);
	    break;
	  default:
	    run_warning("'%c' is not a recognized history data desc char",
			descs[i]);
	    break;
	}
    }
    end_form();
    newline_form();
}

void
write_history(void)
{
    PastUnit *pastunit;
    HistEvent *hevt;

    /* Write all the past units that might be mentioned in events.  These
       should already be sorted by id. */
    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next)
      write_past_unit(pastunit);
    newline_form();
    /* Now write all the events, doing the first one separately so as to
       simplify testing for the end of the history list (which is circular). */
    write_historical_event(history);
    for (hevt = history->next; hevt != history; hevt = hevt->next)
      write_historical_event(hevt);
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END


/* Management of historical information about an Xconq game.
   Copyright (C) 1992-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

static void notify_event(Side *side, HistEvent *hevt);
static void play_event_movies(Side *side, HistEvent *hevt);
static void rewrite_unit_references(int oldid, int newid);
static void rewrite_unit_references_in_event(HistEvent *hevt, int oldid,
					     int newid);

extern int any_post_event_scores;
extern int need_post_event_scores;

HevtDefn hevtdefns[] = {

#undef  DEF_HEVT
#define DEF_HEVT(NAME, code, DATADESCS) { NAME, DATADESCS },

#include "history.def"

    { NULL, NULL }
};

/* Head of the list of events. */

HistEvent *history;

int tmphevtdata1;

/* The list of all past unit records. */

PastUnit *past_unit_list;

PastUnit *last_past_unit;

/* The id of the next past unit to be synthesized.  -1 is reserved, so
   start counting down from -2. */

int next_past_unit_id = -2;

/* Buffers for descriptions of past units. */

#define NUMPASTBUFS 3

int curpastbuf = 0;

char *pastbufs[NUMPASTBUFS] = { NULL, NULL, NULL };

/* True if events are being recorded into the history. */

static int recording_events;

/* True if statistics dump is wanted. */

int statistics_wanted = FALSE;

/* True after the statistics file has been written. */

int statistics_dumped;

void
init_history(void)
{
    /* The first "event" is just a marker. */
    history = create_historical_event(H_LOG_HEAD);
    /* Give it an impossible date. */
    history->startdate = -1;
    history->next = history->prev = history;
    /* Initialize the past unit record. */
    past_unit_list = last_past_unit = NULL;
    next_past_unit_id = -2;
}

void
start_history(void)
{
    /* Ignore multiple starts. */
    if (recording_events)
      return;
    recording_events = TRUE;
    record_event(H_LOG_STARTED, ALLSIDES);
}

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

/* Record a historical event. */

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

static void
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

static void
play_event_movies(Side *side, HistEvent *hevt)
{
    int found = FALSE;
    char *soundname;
    Obj *rest, *head, *parms, *msgdesc;

    if (!should_play_movies())
      return;
    for_all_list(g_event_movies(), rest) {
	head = car(rest);
	if (consp(head)
	    && symbolp(car(head))
	    && hevt->type == find_event_type(car(head))) {
	    found = TRUE;
	    break;
	}
	if (consp(head)
	    && consp(car(head))
	    && symbolp(car(car(head)))
	    && hevt->type == find_event_type(car(car(head)))) {
	    parms = cdr(car(head));
	    if (parms == lispnil) {
		found = TRUE;
		break;
	    }
#if 0
	    if (((symbolp(car(parms))
		   && strcmp(c_string(car(parms)),
			     u_type_name(unit->type)) == 0)
		  || match_keyword(car(parms), K_USTAR)
		  || (symbolp(car(parms))
		      && boundp(car(parms))
		      && ((symbolp(symbol_value(car(parms)))
		          && strcmp(c_string(symbol_value(car(parms))),
				    u_type_name(unit->type)) == 0)
		          || (numberp(symbol_value(car(parms)))
		              && c_number(symbol_value(car(parms)))
			      == unit->type)))
		  )) {
		found = TRUE;
		break;
	    }
	    /* (should be able to match on particular data also) */
#endif
	}
    }
    /* If we have a match, do something with it. */
    if (found) {
	msgdesc = cadr(head);
	if (stringp(msgdesc)) {
	    notify(side, "%s", c_string(msgdesc));
	} else if (consp(msgdesc)
		   && symbolp(car(msgdesc))
		   && strcmp(c_string(car(msgdesc)), "sound") == 0
		   && stringp(cadr(msgdesc))) {
	    soundname = c_string(cadr(msgdesc));
	    /* (should not be passing ptrs to schedule_movie) */
	    schedule_movie(side, "sound", soundname);
	    play_movies(add_side_to_set(side, NOSIDES));
	} else {
	}
    }
}

/* A unit's death requires some bookkeeping - we need to update the
   history to use a past unit, plus record the loss for statistics
   purposes. */

void
record_unit_death(Unit *unit, HistEventType reason)
{
    enum loss_reasons lossreason;
    PastUnit *pastunit;

    pastunit = create_past_unit(unit->type, 0);
    pastunit->name = unit->name;
    pastunit->number = unit->number;
    pastunit->x = unit->x;  pastunit->y = unit->y;  pastunit->z = unit->z;
    pastunit->side = unit->side;
    rewrite_unit_references(unit->id, pastunit->id);
    if (reason >= 0) {
	switch (reason) {
	  case H_UNIT_GARRISONED:
	    /* Garrison events mention the unit being garrisoned. */
	    record_event(reason, add_side_to_set(unit->side, NOSIDES),
			 pastunit->id, tmphevtdata1);
	    break;
	  default:
	    record_event(reason, add_side_to_set(unit->side, NOSIDES),
			 pastunit->id, -1);
	    break;
	}
    }
    /* Reduce specific events to general categories of unit loss, used
       for statistics display. */
    /* Note that we track independent unit losses as well. */
    switch (reason) {
      case H_UNIT_KILLED:
      case H_UNIT_WRECKED:
	lossreason = combat_loss;
	break;
      case H_UNIT_STARVED:
	lossreason = starvation_loss;
	break;
      case H_UNIT_DIED_IN_ACCIDENT:
      case H_UNIT_WRECKED_IN_ACCIDENT:
      case H_UNIT_VANISHED:
	lossreason = accident_loss;
	break;
      case H_UNIT_DIED_FROM_ATTRITION:
      case H_UNIT_WRECKED_FROM_ATTRITION:
	lossreason = attrition_loss;
	break;
      case H_UNIT_DISBANDED:
	lossreason = disband_loss;
	break;
      default:
	lossreason = other_loss;
	break;
    }
    count_loss(unit->side, unit->type, lossreason);
}

/* Create a past unit corresponding to the given unit, replace unit
   references in the history. */

PastUnit *
change_unit_to_past_unit(Unit *unit)
{
    PastUnit *pastunit;

    pastunit = create_past_unit(unit->type, 0);
    pastunit->name = unit->name;
    pastunit->number = unit->number;
    pastunit->x = unit->x;  pastunit->y = unit->y;  pastunit->z = unit->z;
    pastunit->side = unit->side;
    rewrite_unit_references(unit->id, pastunit->id);
    return pastunit;
}

void
record_unit_side_change(Unit *unit, Side *newside, HistEventType reason,
			Unit *agent)
{
    int capture;
    enum loss_reasons lossreason;
    enum gain_reasons gainreason;
    SideMask observers;
    PastUnit *pastunit;

    /* Side loss hevt has no space for individual units, so change to a type
       of event that does. */
    if (reason == H_SIDE_LOST)
      reason = H_UNIT_ACQUIRED;
    pastunit = change_unit_to_past_unit(unit);
    observers = NOSIDES;
    observers = add_side_to_set(unit->side, observers);
    observers = add_side_to_set(newside, observers);
    if (agent != NULL)
      observers = add_side_to_set(agent->side, observers);
    /* Check the event type to decide how many parameters to pass to
       the generic recorder. */
    capture = (reason == H_UNIT_CAPTURED || reason == H_UNIT_SURRENDERED);
    if (capture)
      record_event(reason, observers, pastunit->id, (agent ? agent->id : 0),
		   side_number(newside));
    else
      record_event(reason, observers, pastunit->id,
		   side_number(newside));
    lossreason = (capture ? capture_loss : other_loss);
    count_loss(unit->side, unit->type, lossreason);
    gainreason = (capture ? capture_gain : other_gain);
    count_gain(newside, unit->type, gainreason);
}

/* Record a unit's name change by creating a past unit with the
   previous name. */

void
record_unit_name_change(Unit *unit, char *newname)
{
    PastUnit *pastunit;

    pastunit = change_unit_to_past_unit(unit);
    record_event(H_UNIT_NAME_CHANGED, ALLSIDES, pastunit->id, unit->id);
}

void
count_gain(Side *side, int u, enum gain_reasons reason)
{
    assert_error((reason < num_gain_reasons), 
		 "Invalid gain reason given for historical record.");
    assert_error(is_unit_type(u), 
		 "Invalid unit type given for historical record.");
    if (side)
      ++(side->gaincounts[num_gain_reasons * u + reason]);
}

void
count_loss(Side *side, int u, enum loss_reasons reason)
{
    assert_error((reason < num_loss_reasons), 
		 "Invalid loss reason given for historical record.");
    assert_error(is_unit_type(u), 
		 "Invalid unit type given for historical record.");
    if (side)
      ++(side->losscounts[num_loss_reasons * u + reason]);
}

/* Create a past unit with the given type and for the unit with
   the given id. */

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

/* Scan through the history, changing matching unit/pastunit
   references into past unit references. */

static void
rewrite_unit_references(int oldid, int newid)
{
    HistEvent *hevt;

    rewrite_unit_references_in_event(history, oldid, newid);
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	rewrite_unit_references_in_event(hevt, oldid, newid);
    }
}

static void
rewrite_unit_references_in_event(HistEvent *hevt, int oldid, int newid)
{
    int i;
    char *descs;

    descs = hevtdefns[hevt->type].datadescs;
    /* Scan through the data description, looking for values that are
       references to actual units. */
    for (i = 0; descs[i] != '\0'; ++i) {
	if (descs[i] == 'U' && hevt->data[i] == oldid)
	  hevt->data[i] = newid;
    }
}

/* Indicate that history is no longer being recorded. */

void
end_history(void)
{
    record_event(H_LOG_ENDED, ALLSIDES);
    recording_events = FALSE;
}

/* Find the historical event that would be at the given index, in
   a list where each event gets one line, and dates appear on
   separate lines whenever the date is different. */

HistEvent *
get_nth_history_line(Side *side, int n, HistEvent **nextevt)
{
    int i = 0;
    HistEvent *hevt, *hevtprev = NULL;
    char buf[500];

    Dprintf("Getting line %d\n", n);	
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	historical_event_desc(side, hevt, buf);
	Dprintf("  event is %s, date is %d, i is %d\n", buf, hevt->startdate, i);
	if (side_in_set(side, hevt->observers)) {
	    Dprintf("    observed\n");
	    if (hevtprev == NULL || hevt->startdate != hevtprev->startdate) {
		if (i == n) {
		    Dprintf("Returning NULL\n");
		    return NULL;
		}
		++i;
	    }
	    if (i == n) {
		historical_event_desc(side, hevt, buf);
		    Dprintf("Returning %s\n", hevt);
	      return hevt;
	    }
	    ++i;
	    hevtprev = hevt;
	}
    }
    /* Return the last event. */
    return history->prev;
}

/* Count the total number of lines that can be in the history. */

int
update_total_hist_lines(Side *side)
{
    int nlines;
    HistEvent *hevt, *hevtprev = NULL;
	
    nlines = 0;
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	if (side_in_set(side, hevt->observers)) {
	    /* Count an extra line for date changes. */
	    if (hevtprev == NULL || hevt->startdate != hevtprev->startdate)
	      ++nlines;
	    ++nlines;
	    hevtprev = hevt;
	}
    }
    return nlines;
}

/* This routine builds up the array of events and dates to draw, with
   a combination of pointers to hevts and NULLs where dates should
   appear. */

int
build_hist_contents(Side *side, int n, HistEvent **histcontents,
		    int numvishistlines)
{
    int numcontents;
    HistEvent *hevt, *nexthevt;

    numcontents = 0;
    hevt = get_nth_history_line(side, n, &nexthevt);
    histcontents[numcontents++] = hevt;
    /* Since we want to iterate over links (get_nth_history_line may not
       be very efficient), ensure that we have an actual hevt. */
    if (hevt == NULL) {
	hevt = get_nth_history_line(side, n + 1, &nexthevt);
	histcontents[numcontents++] = hevt;
    }
    for (hevt = hevt->next; hevt != history; hevt = hevt->next) {
	/* Stop if we've filled the contents buffer. */
	if (numcontents >= numvishistlines)
	  break;
	if (side_in_set(side, hevt->observers)) {
	    /* Leave a NULL if the date of this event is different
               from the last. */
	    if (numcontents > 0
		&& histcontents[numcontents - 1] != NULL
		&& hevt->startdate != histcontents[numcontents - 1]->startdate) {
		histcontents[numcontents++] = NULL;
	    }
	    histcontents[numcontents++] = hevt;
	}
    }
    return numcontents;
}

/* Summarize various aspects of performance in the game, writing it
   all to a file. */

void
dump_statistics(void)
{
    char *fname;
    Side *side;
    FILE *fp;

    if (!statistics_wanted)
      return;
    fname = statistics_filename();
    /* If no name returned, assume that there is a good reason
       (for instance, the user might have cancelled). */
    if (empty_string(fname))
      return;
    fp = open_file(fname, "w");
    if (fp != NULL) {
	if (1 /* records exist */) {
	    write_side_results(fp, NULL);
	    write_unit_record(fp, NULL);
	    write_combat_results(fp, NULL);
	    fprintf(fp, "\f\n");
	    for_all_sides(side) {
		write_side_results(fp, side);
		write_unit_record(fp, side);
		write_combat_results(fp, side);
		if (side->next != NULL)
		  fprintf(fp, "\f\n");
	    }
	} else {
	    fprintf(fp, "No statistics were kept.\n");
	}
	statistics_dumped = TRUE;
	fclose(fp);
    } else {
	run_warning("Can't open statistics file \"%s\"", fname);
    }
}

/* Short, unreadable, but greppable listing of past unit.  Primarily
   useful for debugging and warnings.  We use several buffers and
   rotate between them so we can call this more than once in a single
   printf. */

char *
past_unit_desig(PastUnit *pastunit)
{
    char *shortbuf;

    if (pastunit == NULL)
      return "no pastunit";
    /* Allocate if not yet done so. */
    if (pastbufs[curpastbuf] == NULL)
      pastbufs[curpastbuf] = (char *)xmalloc(BUFSIZE);
    shortbuf = pastbufs[curpastbuf];
    curpastbuf = (curpastbuf + 1) % NUMPASTBUFS;
    if (pastunit->id == -1) {
	sprintf(shortbuf, "s%d head", side_number(pastunit->side));
	return shortbuf;
    } else if (is_unit_type(pastunit->type)) {
	sprintf(shortbuf, "s%d %-3.3s %d (%d,%d",
		side_number(pastunit->side),
		shortest_unique_name(pastunit->type),
		pastunit->id, pastunit->x, pastunit->y);
	if (pastunit->z != 0)
	  tprintf(shortbuf, ",%d", pastunit->z);
	strcat(shortbuf, ")");  /* close out the pastunit location */
	return shortbuf;
    } else {
	return "!garbage pastunit!";
    }
}

/* Would be faster to stash these, but enough difference to care? */

int
total_gain(Side *side, int u)
{
    int i, total = 0;

    for (i = 0; i < num_gain_reasons; ++i)
      total += side_gain_count(side, u, i);
    return total;
}

int
total_loss(Side *side, int u)
{
    int i, total = 0;

    for (i = 0; i < num_loss_reasons; ++i)
      total += side_loss_count(side, u, i);
    return total;
}

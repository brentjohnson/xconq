/* Scores and scoring in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Scoring in Xconq does not happen by default; instead a game design
   may include one or more "scorekeepers", which are objects with
   properties that define how winning and losing will be decided.

   There are two kinds of functionality in scorekeepers.  One kind is
   the ability to end a game based on a specified condition, such as
   "the Star of Satyria enters Noblum".  The other is the accumulation
   of a numeric score.  A scorekeeper may include both, so it can (for
   instance) end the game the instant that a player's score reaches
   100, or 100 more than another player, or whatever.

   The indepside should not participate in scoring, so the iterations
   in this file generally iterate only over the "real" sides. */

#include "conq.h"
#include "kernel.h"

typedef struct a_score_record {
    const char *gamename;
    Obj *sides;
    Obj *raw;
    int numturns;
    Obj *varsets;
    struct a_score_record *next;
} ScoreRecord;

/* This is true when the given side should be tested against the given
   scorekeeper. */

#define scorekeeper_applicable(side,sk)  \
  ((side)->ingame && (side_in_set((side), (sk)->whomask)))

/* Iteration over all recorded scores. */

#define for_all_score_records(sr)  \
  for ((sr) = records; (sr) != NULL; (sr) = (sr)->next)

static int read_scorefile(void);
static int interp_score_record(Obj *form);
static const char *basic_player_name(Player *player);
static void eval_sk_last_side_wins(Scorekeeper *sk);
static void eval_sk_last_alliance_wins(Scorekeeper *sk);
static void score_variant_desc(ScoreRecord *sr, char *varbuf);

/* The head of the list of scorekeepers. */

Scorekeeper *scorekeepers;

/* The end of the list of scorekeepers. */

Scorekeeper *last_scorekeeper;

/* The total number of scorekeepers defined. */

int numscorekeepers;

int nextskid;

/* The number of scorekeepers maintaining numeric scores. */

int numscores;

/* True if any pre-turn scorekeepers are defined. */

int any_pre_turn_scores;

/* True if any post-turn scorekeepers are defined. */

int any_post_turn_scores;

/* True if any post-action scorekeepers are defined. */

int any_post_action_scores;

/* True if any post-event scorekeepers are defined. */

int any_post_event_scores;

/* True if any turn-specific scorekeepers are defined. */

int any_turn_specific_scores;

/* The count of sides in the game when the last-side-wins scorekeeper
   first tested. */

static int num_sides_originally;

static int only_checking;

static int we_have_a_winner;

ScoreRecord *records;

/* Clear out any possible scorekeepers. */

void
init_scorekeepers(void)
{
    scorekeepers = last_scorekeeper = NULL;
    numscorekeepers = 0;
    nextskid = 1;
    any_pre_turn_scores = FALSE;
    any_post_turn_scores = FALSE;
    any_post_action_scores = FALSE;
    any_post_event_scores = FALSE;
    any_turn_specific_scores = FALSE;
}

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

/* Allocate and fill in the initial score records for each side.  This
   must happen after all scorekeepers have been defined. */

void
init_scores(void)
{
    int score;
    Side *side;
    Scorekeeper *sk;
    Obj *savedscores, *when;

    /* First count and index all the scorekeepers that maintain
       a numeric score. */
    numscores = 0;
    for_all_scorekeepers(sk) {
    	if (sk->keepscore) {
    	    sk->scorenum = numscores++;
    	}
    }
    /* Allocate an appropriately-sized scorecard for each side.  Note that a
       particular position in the scorecard might not apply to all sides. */
    for_all_sides(side) {
	/* Collect any Lisp object that might have been stashed here
	   while reading a game module. */
	savedscores = side->rawscores;
	side->scores = NULL;
	if (numscores > 0) {
	    side->scores = (short *) xmalloc(numscores * sizeof(short));
	    for_all_scorekeepers(sk) {
		if (!sk->keepscore)
		  continue;
		score = sk->initial;
		/* Collect a saved score if there is one to collect. */
		if (savedscores != NULL
		    && savedscores != lispnil
		    && numberp(car(savedscores))) {
		    score = c_number(car(savedscores));
		    savedscores = cdr(savedscores);
		}
		side->scores[sk->scorenum] = score;
	    }
	}
    }
    /* Some kinds of scorekeepers are expensive to run, so we set flags to
       indicate directly that we need to make the check. */
    for_all_scorekeepers(sk) {
    	when = sk->when;
    	if (consp(when)) {
	    if (cdr(when) != lispnil) {
		any_turn_specific_scores = TRUE;
	    }
	    when = car(when);
	}
	if (match_keyword(when, K_BEFORE_TURN)) {
	    any_pre_turn_scores = TRUE;
	}
	if (when == lispnil || match_keyword(when, K_AFTER_TURN)) {
	    any_post_turn_scores = TRUE;
	}
	if (match_keyword(when, K_AFTER_ACTION)) {
	    any_post_action_scores = TRUE;
	}
	if (match_keyword(when, K_AFTER_EVENT)) {
	    any_post_event_scores = TRUE;
	}
    }
    /* Compute the number of sides in the game initially.  This is so
       we don't force the game to end because (for whatever reason)
       only one side was active from the beginning. */
    num_sides_originally = 0;
    for_all_sides(side) {
	if (side->ingame)
	  ++num_sides_originally;
    }
}

/* Generate a sidemask for sides that the scorekeeper should run for.
   Note that this can be a subset of the 'whomask' for the scorekeeper, 
    since the 'when' condition may limit the sk from being applied to some 
    sides.
*/

SideMask
generate_effective_sk_whomask(Scorekeeper *sk)
{
    SideMask ewhomask = NOSIDES;
    Side *side = NULL;
    Obj *whenexpr = NULL;

    assert_error(sk, "Attempted to access a NULL scorekeeper");
    assert_error(sk->when, 
		 "Attempted to access a NULL scorekeeper \"when expression\"");
    /* If the when expression is NIL or not a full expression, 
	then just return the whomask. */
    if ((sk->when == lispnil) || !consp(sk->when))
      return sk->whomask;
    /* Get the actual when expression. 
       We do not care about the when keyword here. */
    whenexpr = cdr(sk->when);
    /* If the when expression is a number, 
	then return NOSIDES if it is 0 (false), else return the sidemask. */
    if (numberp(whenexpr)) {
	if (c_number(whenexpr))
	  ewhomask = sk->whomask;
	else
	  ewhomask = NOSIDES;
    }
    /* Else if the expression is NIL, then return NOSIDES. */
    else if (lispnil == whenexpr)
      ewhomask = NOSIDES;
    /* Else if the expression is a cons, 
	then evaluate it for each side and build the sidemask accordingly. */
    else if (consp(whenexpr)) {
	/* If the test is a cons, then evaluate it. */
	if (consp(car(whenexpr))) {
	    for_all_sides(side) {
		if (!scorekeeper_applicable(side, sk))
		  continue;
		if (lispnil != eval_sk_test(side, sk, car(whenexpr)))
		  ewhomask = add_side_to_set(side, ewhomask);
	    }
	}
	/* Else if the test is just a number, then assume <= g_turn(). */
	/*! \todo Decide based on the when type once we support event and 
		    action scorekeepers. */
	else if (numberp(car(whenexpr))) {
	    if (c_number(car(whenexpr)) <= g_turn()) 
	      ewhomask = sk->whomask;
	}
	/* Else we don't know how to handle the test... */
	else {
	    run_warning(
			"Garbled \"when expression\" for scorekeeper %s",
			(sk->title ? sk->title : ""));
	    ewhomask = NOSIDES;
	}
    }
    /* Else if the expression cannot be understood, 
	then warn and return NOSIDES. */
    else {
	run_warning(
		    "Garbled \"when expression\" for scorekeeper %s",
		    (sk->title ? sk->title : ""));
	ewhomask = NOSIDES;
    }
    return ewhomask;
}

/* Test all the scorekeepers that should be run immediately before any
   side moves anything. */

void
check_pre_turn_scores(void)
{
    Side *side;
    Scorekeeper *sk;

    if (any_pre_turn_scores) {
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_BEFORE_TURN)) {
		for_all_sides(side) {
		    if (scorekeeper_applicable(side, sk)) {
			run_scorekeeper(side, sk); 
		    } else {
		    	Dprintf("sk %d not applicable to %s\n",
		    		sk->id, side_desig(side));
		    }
		}
	    }
	}
    }
}

/* Test all the scorekeepers that should be run only at the end of a turn. */

void
check_post_turn_scores(void)
{
    Side *side;
    Scorekeeper *sk;
    Obj *when = lispnil;
    SideMask ewhomask = NOSIDES;

    if (any_post_turn_scores) {
	for_all_scorekeepers(sk) {
	    Dprintf("Checking post-turn scorekeeper %d\n", sk->id);
	    ewhomask = NOSIDES;
	    /* Parse the when expression. */
	    when = sk->when;
	    if ((when != lispnil) && consp(when)) 
	      when = car(when);
	    /* See if when expression is applicable here, 
	       and act accordingly. */
	    if ((when == lispnil) 
		|| match_keyword(when, K_AFTER_TURN)) {
		ewhomask = generate_effective_sk_whomask(sk);
		if (NOSIDES == ewhomask)
		  continue;
	    	if (symbolp(sk->body) 
		    && match_keyword(sk->body, K_LAST_SIDE_WINS)) {
		    eval_sk_last_side_wins(sk);
	    	} else if (symbolp(sk->body)
			   && match_keyword(sk->body, K_LAST_ALLIANCE_WINS)) {
		    eval_sk_last_alliance_wins(sk);
	    	} else {
		    for_all_sides(side) {
			if (side_in_set(side, ewhomask)) {
			    run_scorekeeper(side, sk); 
			} else {
			    Dprintf("sk %d not applicable to %s\n",
				    sk->id, side_desig(side));
			}
		    }
		}
	    }
	}
    }
}

void
check_post_action_scores(void)
{
    Side *side;
    Scorekeeper *sk;

    if (any_post_action_scores) {
	Dprintf("Checking post-action scorekeepers\n");
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_AFTER_ACTION)) {
		if (sk->trigger == lispnil || sk->triggered) {
		    for_all_sides(side) {
			if (scorekeeper_applicable(side, sk)) {
			    run_scorekeeper(side, sk);
		        } else {
		    	    Dprintf("sk %d not applicable to %s\n",
		    		    sk->id, side_desig(side));
			}
		    }
		}
	    }
	}
    }
}

void
check_post_event_scores(void)
{
    Side *side;
    Scorekeeper *sk;

    if (any_post_event_scores) {
	Dprintf("Checking post-event scorekeepers\n");
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_AFTER_EVENT)) {
		if (sk->trigger == lispnil || sk->triggered) {
		    for_all_sides(side) {
			if (scorekeeper_applicable(side, sk)) {
			    run_scorekeeper(side, sk); 
		        } else {
		    	    Dprintf("sk %d not applicable to %s\n",
		    		    sk->id, side_desig(side));
			}
		    }
		}
	    }
	}
    }
}

/* This is what actually does the test and effect of the scorekeeper
   on the given side.  This can be expensive to run. */

void
run_scorekeeper(Side *side, Scorekeeper *sk)
{
    eval_sk_form(side, sk, sk->body);
}

/* Sum an integer uprop across a given list of utypes. */
/* If side is NULL, then all sides should be considered. */

Obj *
sum_uprop(Side *side, Obj *form)
{
    int u = NONUTYPE;
    Obj *uprop = lispnil;
    Side *side2 = NULL;
    Unit *unit = NULL;
    int i = 0, usepointval = FALSE, sum = 0;
    const char *upropname = NULL;
    int (*intgetter)(int) = NULL;

    /* Sanity Checks. */
    assert_error(form, "Attempted to access a NULL GDL form");
    if (lispnil == form) {
	run_warning("Bad 'sum-uprop' form encountered.");
	return lispnil;
    }
    /* Prep data structures. */
    for_all_unit_types(u) 
      tmp_u_array[u] = FALSE;
    /* Parse utype(s). */
    fill_utype_array_from_lisp(tmp_u_array, car(form));
    /* Retrieve uprop. */
    if (lispnil == cdr(form)) {
	run_warning("Bad 'sum-uprop' form encountered.");
	return lispnil;
    }
    form = cdr(form);
    uprop = car(form);
    if (lispnil == uprop) {
	run_warning("Bad 'sum-uprop' form encountered.");
	return lispnil;
    }
    /*! \todo Generate optional sidemask. */
    /*! \todo Handle other filters. */
    /* Parse uprop. */
    while (symbolp(uprop) && boundp(uprop)) 
      uprop = eval_symbol(uprop);
    if (symbolp(uprop)) 
      /*! \todo Add a GC to GDL to handle memory leaks like the following. */
      uprop = new_string(c_string(uprop));
    if (stringp(uprop)) {
	upropname = c_string(uprop);
	/* Handle special cases. */
	if (!strcmp(upropname, "point-value"))
	  usepointval = TRUE;
	/* Handle general cases. */
	else {
	    for (i = 0; utypedefns[i].name; ++i) {
		if (!strcmp(upropname, utypedefns[i].name)) {
		    intgetter = utypedefns[i].intgetter;
		    if (!intgetter) {
			run_warning(
"'sum-uprop' encountered non-integer uprop, \"%s\"", upropname);
			return lispnil;
		    }
		    break;
		}
	    }
	    if (!utypedefns[i].name) {
		run_warning("No uprop matching name, \"%s\", found",
			    upropname);
		return lispnil;
	    }
	}
    }
    /* Sum it across relevant units. */
    for_all_sides(side2) {
	/*! \note For now, we only sum across the given side. */
	if (side != side2)
	  continue;
	for_all_side_units(side2, unit) {
	    /* Skip any unit type not in the utype array. */
	    if (!tmp_u_array[unit->type])
	      continue;
	    /* Handle special cases. */
	    if (usepointval)
	      sum += point_value(unit);
	    /* Handle general cases. */
	    else
	      sum += intgetter(unit->type);
	}
    }
    /*! \todo Add a GC to GDL to handle memory leaks like the following. */
    return new_number(sum);
}

/* Return the point value of a particular unit. */

int
point_value(Unit *unit)
{
    /* Incomplete units are always worthless. */
    if (unit->cp > 0 && !completed(unit))
      return 0;
    if (unit_point_value(unit) >= 0)
      return unit_point_value(unit);
    return u_point_value(unit->type);
}

/* Return the sum of a side's units' point values. */

int
side_point_value(Side *side)
{
    Unit *unit;

    if (!side->point_value_valid) {
	side->point_value_cache = 0;
	for_all_side_units(side, unit) {
	    /* Note that we want to count units that may appear in
	       the future. */
	    if ((in_play(unit) && completed(unit))
		|| (alive(unit) && unit->cp < 0)) {
		side->point_value_cache += point_value(unit);
	    }
	}
	side->point_value_valid = TRUE;
    }
    return side->point_value_cache;
}

int
alliance_point_value(Side *side)
{
    int score = 0;
    Side *side2;

    for_all_sides(side2) {
	if (side2->ingame 
	    && (side2 == side || trusted_side(side2, side))) {
		score += side_point_value(side2);
	}
    }    
    return score;
}

int
has_allies(Side *side)
{
	Side *side2;
	
	for_all_sides(side2) {
		if (side2->ingame
		    && side2 != side
		    && trusted_side(side2, side)) {
			return TRUE;
		}	
	}
	return FALSE;
}

/* Evaluate a given scorekeeper test form wrt to a given side and 
    scorekeeper. */
/* This function walks through the expression tree, and only expands things 
    related to scorekeeper tests. */

Obj *
eval_sk_test(Side *side, Scorekeeper *sk, Obj *form)
{
    Obj *curobj = lispnil, *nextobj = lispnil, *newform = lispnil;
    Obj *rest = lispnil;
    enum keywords code;

    /* Sanity checks. */
    assert_error(sk, "No scorekeeper provided for scorekeeper test");
    assert_error(form, 
"No scorekeeper test expression provided for scorekeeper test");
    /* If test form is nil, then return nil. */
    if (lispnil == form)
      return lispnil;
    /* If test form is list... */
    if (consp(form)) {
	/* If action form has a test or action form keyword. */
	curobj = car(form);
	if (symbolp(curobj) && !boundp(curobj)) {
	    code = (enum keywords)keyword_code(c_string(curobj));
	    switch (code) {
	      /* Sum an uprop over a list of utypes. */
	      case K_SUM_UPROP:
		return sum_uprop(side, cdr(form));
	      /*! \todo Add other interesting cases. */
	      default: break;
	    }
	}
	/* Iterate through test form in order. */
	curobj = lispnil;
	for_all_list(form, rest) {
	    nextobj = eval_sk_test(side, sk, car(rest));
	    nextobj = cons(nextobj, lispnil);
	    if (lispnil != curobj) 
	      set_cdr(curobj, nextobj);
	    else
	      newform = nextobj;
	    curobj = nextobj;
	}
    }
    /* If test form is symbol... */
    else if (symbolp(form)) {
	/* If symbol is bound, then substitute and re-test. */
	if (boundp(form))
	  newform = eval_sk_test(side, sk, eval_symbol(form));
	/* Else, try to substitute a scorekeeper keyword. */
	code = (enum keywords)keyword_code(c_string(form));
	switch (code) {
	  /* Current turn number. */
	  case K_TURN:
	    return new_number(g_turn());
	  /* Current score for given side and scorekeeper. */
	  case K_SCORE:
	    if (!sk->keepscore) {
		run_warning(
"Attempted to use 'score' with non-numeric scorekeeper");
		return lispnil;
	    }
	    /*! \todo Need GC so these memleaks don't accumulate. */
	    return new_number(side->scores[sk->scorenum]);
	  /*! \todo Add other interesting cases here. */
	  /* No matching keyword, so leave it be. */
	  default:
	    return form;
	}
    }
    else
      newform = form;
    /* Evaluate new form and return. */
    return eval(newform);
}

/* Evaluate a given scorekeeper action form wrt to a given side and 
    scorekeeper. */
/* This function walks through the expression tree, and only expands things 
    related to scorekeeper actions. */

Obj *
eval_sk_form(Side *side, Scorekeeper *sk, Obj *form)
{
    Obj *curobj = lispnil, *nextobj = lispnil, *newform = lispnil;
    Obj *clauses = lispnil, *clause = lispnil, *rest = lispnil;
    enum keywords code;

    /* Sanity checks. */
    assert_error(sk, "No scorekeeper provided for scorekeeper body");
    assert_error(form, 
"No scorekeeper body expression provided for scorekeeper body");
    /* If action form is nil, then return nil. */
    if (lispnil == form)
      return lispnil;
    /* If action form is list... */
    if (consp(form)) {
	/* If action form has a test or action form keyword. */
	curobj = car(form);
	if (symbolp(curobj) && !boundp(curobj)) {
	    code = (enum keywords)keyword_code(c_string(curobj));
	    switch (code) {
	      /* Interpret Common Lisp 'if' form. */
	      case K_IF:
		if (lispnil != eval_sk_test(side, sk, cadr(form))) 
		  return eval_sk_form(side, sk, caddr(form));
		else {
		    if (lispnil != cdr(cddr(form)))
		      return eval_sk_form(side, sk, cadr(cddr(form)));
		}
		return lispnil;
	      /* Interpret the traditional cond form. */
	      case K_COND:
		for_all_list(cdr(form), clauses) {
		    clause = car(clauses);
		    if (lispnil != 
			(newform = eval_sk_test(side, sk, car(clause)))) {
			for_all_list(cdr(clause), rest) 
			  newform = eval_sk_form(side, sk, car(rest));
			return newform;
		    }
		} 
		return lispnil;
	      case K_SET_SCORE:
		if (!sk->keepscore) {
		    run_warning(
"Attempted to use 'set-score' with non-numeric scorekeeper");
		    return lispnil;
		}
		curobj = eval_sk_test(side, sk, cadr(form));
		if (!numberp(curobj))
		  run_warning("Invalid 'set-score' form encountered");
		else
		  side->scores[sk->scorenum] = c_number(curobj);
		return curobj;
	      case K_ADD_SCORE:
		if (!sk->keepscore) {
		    run_warning(
"Attempted to use 'add-score' with non-numeric scorekeeper");
		    return lispnil;
		}
		curobj = eval_sk_test(side, sk, cadr(form));
		if (!numberp(curobj))
		  run_warning("Invalid 'add-score' form encountered");
		else
		  side->scores[sk->scorenum] += c_number(curobj);
		return curobj;
	      /*! \todo Add other interesting cases. */
	      default: break;
	    }
	}
	/* Iterate through action form in order. */
	curobj = lispnil;
	for_all_list(form, rest) {
	    nextobj = eval_sk_form(side, sk, car(rest));
	    nextobj = cons(nextobj, lispnil);
	    if (lispnil != curobj) 
	      set_cdr(curobj, nextobj);
	    else
	      newform = nextobj;
	    curobj = nextobj;
	}
    }
    /* If test form is symbol... */
    else if (symbolp(form)) {
	/* If symbol is bound, then substitute and re-eval. */
	if (boundp(form))
	  newform = eval_sk_form(side, sk, eval_symbol(form));
	/* Else, try to substitute a scorekeeper keyword. */
	code = (enum keywords)keyword_code(c_string(form));
	switch (code) {
	  case K_LAST_SIDE_WINS:
	    eval_sk_last_side_wins(sk);
	    break;
	  case K_LAST_ALLIANCE_WINS:
	    eval_sk_last_alliance_wins(sk);
	    break;
	  case K_WIN:
	    if (side->ingame && !only_checking)
	      side_wins(side, sk->id);
	    break;
	  case K_LOSE:
	    if (side->ingame && !only_checking)
	      side_loses(side, NULL, sk->id);
	    break;
	  case K_END:
	    if (!only_checking)
	      all_sides_draw();
	    break;
	  /*! \todo Add other interesting cases here. */
	  /* No matching keyword, so leave it be. */
	  default:
	    return form;
	}
    }
    else
      newform = form;
    return eval(newform);
}

/* Test whether a single side is still alive in the game. */

static void
eval_sk_last_side_wins(Scorekeeper *sk)
{
    Side *side2, *winner = NULL;
    int numleft = 0, points;

    /* This is only meaningful in games with at least two sides. */
    if (num_sides_originally < 2)
      return;
    for_all_sides(side2) {
	/* The independent side should not be counted among the sides
	   left (unless it has somebody running it), otherwise the
	   "but I already won!" player has to scour the map looking for
	   remaining inert independents. Even if someone is running the 
	   indepside, it may be reasonable to exclude it from scorekeeping. */
	if (inactive_indepside(side2))
	  continue;
	/* Exclude any side that is not in the scorekeeper's sidemask. */
	if (!side_in_set(side2, sk->whomask))
	  continue;
	if (side2->ingame) {
	    points = side_point_value(side2);
	    Dprintf("%s has %d points worth of units\n",
		    side_desig(side2), points);
	    if (points == 0) {
		if (!only_checking)
		  side_loses(side2, NULL, sk->id);
	    } else {
		++numleft;
		/* Take note of the possible winner. */
		winner = side2;
	    }
	}
    }
    if (numleft == 1) {
	we_have_a_winner = TRUE;
	if (!only_checking)
	  side_wins(winner, sk->id);
    }
}

/* Test whether a single alliance (a group of trusting sides) is all
   that remains in the game. */

static void
eval_sk_last_alliance_wins(Scorekeeper *sk)
{
    Side *side2, *side3, *winner = NULL;
    int numleft = 0, sidepoints[MAXSIDES+1], alliancepoints[MAXSIDES+1];
    int alliancewins;

    /* This is only meaningful in games with at least two sides. */
    if (num_sides_originally < 2)
      return;
    for_all_sides(side2) {
	sidepoints[side2->id] = alliancepoints[side2->id] = 0;
	if (side2->ingame) {
	    sidepoints[side2->id] = side_point_value(side2);
	    Dprintf("%s has %d points worth of units\n",
		    side_desig(side2), sidepoints[side2->id]);
	}
    }
    /* Sum up to get points for each alliance. */
    for_all_sides(side2) {
	if (side2->ingame) {
	    for_all_sides(side3) {
		if (side2 == side3 || trusted_side(side2, side3)) {
		    alliancepoints[side2->id] += sidepoints[side3->id];
		}
	    }
	}
    }
    /* Make all the sides belonging to point-less alliances lose now,
       and look for a non-losing side. */
    for_all_sides(side2) {
	if (inactive_indepside(side2))
	  continue;
	/* Exclude any side that is not in the scorekeeper's sidemask. */
	if (!side_in_set(side2, sk->whomask))
	  continue;
	if (side2->ingame) {
	    if (alliancepoints[side2->id] == 0) {
		if (!only_checking)
		  side_loses(side2, NULL, sk->id);
	    } else {
		++numleft;
		/* We still need to pick a single side, we'll take
		   care of all its buddies shortly. */
		winner = side2;
	    }
	}
    }
    /* It may happen that all sides lose (for instance if the last two
       units die in a duel). */
    if (numleft == 0)
      return;
    /* See if the non-losers all belong to a single alliance. */
    alliancewins = TRUE;
    for_all_sides(side2) {
	if (inactive_indepside(side2))
	  continue;
	/* Exclude any side that is not in the scorekeeper's sidemask. */
	if (!side_in_set(side2, sk->whomask))
	  continue;
	if (side2->ingame
	    && side2 != indepside
	    && side2 != winner
	    && !trusted_side(winner, side2)) {
	    alliancewins = FALSE;
	    break;
	}
    }
    /* If so, everybody in the alliance wins equally. */
    if (alliancewins) {
	we_have_a_winner = TRUE;
	for_all_sides(side2) {
	    if (inactive_indepside(side2))
	      continue;
	    /* Exclude any side that is not in the scorekeeper's sidemask. */
	    if (!side_in_set(side2, sk->whomask))
	      continue;
	    if (side2->ingame
		&& (side2 == winner || trusted_side(winner, side2))) {
		if (!only_checking)
		  side_wins(side2, sk->id);
	    }
	}
    }
}

/* Implement the effects of a side winning. */

void
side_wins(Side *side, int why)
{
    /* Nothing happens to the side's units or people. */
    side->status = 1;
    remove_side_from_game(side);
    /* Record the event after the side is removed, so we don't get infinite
       recursion if there is an event-triggered scorekeeper. */
    record_event(H_SIDE_WON, ALLSIDES, side_number(side), why);
}

/* Implement the effects of a side losing. */

void
side_loses(Side *side, Side *side2, int why)
{
    int x, y, s, s2, ux, uy, changed;
    Unit *unit;
    Side *side3;

    /* We can't lose twice. */
    if (!side->ingame)
        return;
    /* The independents cannot lose by definition even if they
    are down to zero points (unless played by a human). */
    if (side == indepside && !side_has_display(side))
      return;
    /* These should not happen, but a stupid AI might try, so just
       and clear the mistaken side. */
    if (side == side2) {
	run_warning("losing to self, ignoring side");
	side2 = NULL;
    }
    if (side2 != NULL && !side2->ingame) {
	run_warning("losing to side not in game, ignoring side");
	side2 = NULL;
    }
    /* If there's a controlling side, it gets everything. */
    if (side->controlled_by != NULL && side->controlled_by->ingame) {
	side2 = side->controlled_by;
    }
    if (side2 != NULL) {
	/* Dispose of all of a side's units. */
	for_all_units(unit) {
	    if (unit->side == side) {
		if (in_play(unit)) {
		    ux = unit->x;  uy = unit->y;
		    change_unit_side(unit, side2, H_SIDE_LOST, NULL);
		    /* Everybody gets to see this change. */
		    all_see_cell(ux, uy);
		} else {
		    /* Even out-of-play units need to have their side set. */
		    set_unit_side(unit, side2);
		}
	    }
	}
	/* The people also change sides. */
	if (people_sides_defined() || control_sides_defined()) {
	    s = side_number(side);
	    s2 = side_number(side2);
	    for_all_cells(x, y) {
		changed = FALSE;
		if (people_sides_defined() && people_side_at(x, y) == s) {
		    set_people_side_at(x, y, s2);
		    changed = TRUE;
		}
		if (control_sides_defined() && control_side_at(x, y) == s) {
		    set_control_side_at(x, y, s2);
		    changed = TRUE;
		}
		if (changed)
		  all_see_cell(x, y);
	    }
	}
	/* Reset view coverage everywhere. */
	if (!side->see_all) {
	    for_all_cells(x, y) {
		set_cover(side, x, y, 0);
	    }
	}
    }
    
    /* Advanced units (cities) should not disappear - make them independent. */
    else if (indepside->ingame) {
	for_all_units(unit) {
	    if (unit->side == side 
	        && u_advanced(unit->type)) {
		if (in_play(unit)) {
		    ux = unit->x;  
		    uy = unit->y;
		    change_unit_side(unit, indepside, H_SIDE_LOST, NULL);
		    /* Everybody gets to see this change. */
		    all_see_cell(ux, uy);
		} else {
		    /* Even out-of-play units need to have their side set. */
		    set_unit_side(unit, indepside);
		}
	    }
	}
    }
    
    /* Add the mark of shame itself. */
    side->status = -1;
    remove_side_from_game(side);
    /* Record the event after the side is removed, so we don't get infinite
       recursion if there is an event-triggered scorekeeper. */
    record_event(H_SIDE_LOST, ALLSIDES, side_number(side), why);
    /* As a special case, look at post-turn scorekeepers to see if
       sides should end their turns early because the game is over. */
    only_checking = TRUE;
    check_post_turn_scores();
    only_checking = FALSE;
    if (we_have_a_winner) {
	for_all_sides(side3) {
	    if (side3->ingame) {
		finish_turn(side3);
	    }
	}
    }
    /* When the remaining sides' turns are finished, the end-of-turn
       processing will commence, and it includes the real run of
       post-turn scorekeeper checking. */
}

/* Implement a draw.  Note that unlike winning or losing, the draw
   applies to all sides currently in the game. */

void
all_sides_draw(void)
{
    SideMask drew;
    Side *side;

    drew = NOSIDES;
    for_all_sides(side) {
    	if (side->ingame) {
	    side->status = 0;
	    remove_side_from_game(side);
	    drew = add_side_to_set(side, drew);
	}
    }
    /* Now that all sides are out, safe to record events to that effect
       (no possibility of triggering post-event scorekeepers). */
    for_all_sides(side) {
    	if (side_in_set(side, drew)) {
	    record_event(H_SIDE_WITHDREW, ALLSIDES, side_number(side));
	}
    }
}

/* (should move this to nlang.c?) */
static const char *
basic_player_name(Player *player)
{
    const char *playername;

    playername = "";
    if (player) {
	if (!empty_string(player->name))
	  playername = player->name;
	else if (!empty_string(player->displayname))
	  playername = player->displayname;
	else if (!empty_string(player->aitypename))
	  playername = player->aitypename;
    }
    return playername;
}

/* Record the outcome of the game into the scorefile. */

void
record_into_scorefile(void)
{
    int i;
    int any_advantage_variation = FALSE, adv, adv2, advantage;
    const char *filename, *playername, *mversion;
    const char *varname;
    Variant *variants, *var;
    FILE *fp;
    Side *side;

    /* (should make following code into a separate routine) */
    adv = -1;
    for_all_sides(side) {
	adv2 = actual_advantage(side);
	if (adv < 1)
	  adv = adv2;
	if (adv != adv2) {
	    any_advantage_variation = TRUE;
	    break;
	}
    }
    filename = SCOREFILE;
    if (!empty_string(g_scorefile_name()))
      filename = g_scorefile_name();
    fp = open_scorefile_for_writing(filename);
    if (fp == NULL) {
	run_warning("%s cannot be opened for writing, will not record score",
		    filename);
	/* (should provide some sort of retry here) */
    } else {
	fprintf(fp, "(g %s",
		/* (should record this for comparison when displaying scores) */
		escaped_symbol((mainmodule->origmodulename
				? mainmodule->origmodulename
				: mainmodule->name)));
	/* Record the module's version if defined. */
	mversion = (mainmodule->origversion
		    ? mainmodule->origversion
		    : mainmodule->version);
	if (!empty_string(mversion))
	  fprintf(fp, " (ve \"%s\")", mversion);
	/* Record all the choices of variant. */
	variants = (mainmodule->origvariants
		    ? mainmodule->origvariants
		    : mainmodule->variants);
	if (variants) {
	    fprintf(fp, " (v");
	    for (i = 0; variants[i].id != lispnil; ++i) {
		var = &(variants[i]);
		varname = c_string(var->id);
		fprintf(fp, " (");
		/* Encode the common variants by number, for
		   compactness.  Note that once assigned, these
		   numbers can never change. */
		switch (keyword_code(varname)) {
		  case K_WORLD_SEEN:
		    fprintf(fp, "1 %d", var->intvalue);
		    break;
		  case K_SEE_ALL:
		    fprintf(fp, "2 %d", var->intvalue);
		    break;
		  case K_SEQUENTIAL:
		    fprintf(fp, "3 %d", var->intvalue);
		    break;
		  case K_PEOPLE:
		    fprintf(fp, "4 %d", var->intvalue);
		    break;
		  case K_ECONOMY:
		    fprintf(fp, "5 %d", var->intvalue);
		    break;
		  case K_SUPPLY:
		    fprintf(fp, "6 %d", var->intvalue);
		    break;
		  case K_WORLD_SIZE:
		    fprintf(fp, "11 %d %d %d",
			    area.width, area.height, world.circumference);
		    break;
		  case K_REAL_TIME:
		    fprintf(fp, "12 %d %d %d",
			    g_rt_for_game(),
			    g_rt_per_side(),
			    g_rt_per_turn());
		    break;
		  default:
		    fprintf(fp, "%s", escaped_symbol(varname));
		    if (var->hasintvalue) {
			fprintf(fp, " %d", var->intvalue);
		    }
		    /* Variants with unknown types of data end up getting
		       recorded as just "(<var-name>)", which is OK. */
		}
		fprintf(fp, ")");
	    }
	    fprintf(fp, ")");
	}
	fprintf(fp, " (t %d)", g_turn());
	/* End of the first line of a score record. */
	fprintf(fp, "\n");
	/* Record all the participants and how they fared. */
	fprintf(fp, " (s");
	for_all_sides(side) {
	    /* Only record info about sides that actually participated. */
	    if (side->everingame) {
		playername = basic_player_name(side->player);
		/* If a side has no player, don't record it. */
		if (empty_string(playername))
		  continue;
		fprintf(fp, " (");
		fprintf(fp, "%s", escaped_symbol(playername));
		fprintf(fp, " %s",
			(side_won(side)
			 ? "won"
			 : (side_lost(side)
			    ? "lost"
			    : "drew")));
		/* (should write info about ai helping human players) */
		if (any_advantage_variation) {
		    advantage = actual_advantage(side);
		    if (advantage > 1)
		      fprintf(fp, " (a %d)", advantage); 
		}
		if (numscores > 0) {
		    fprintf(fp, " (sc");
		    for (i = 0; i < numscores; ++i) {
			fprintf(fp, " %d", side->scores[i]);
		    }
		    fprintf(fp, ")");
		}
		fprintf(fp, ")");
	    }
	}
	fprintf(fp, ")");
	/* (should record other useful info about game, such as
           date(s) played) */
	fprintf(fp, ")\n");
	close_scorefile_for_writing(fp);
    }
}

static int
read_scorefile(void)
{
    int startlineno = 1, endlineno = 1;
    int numrecs;
    const char *filename;
    Obj *form;
    FILE *fp;
    ScoreRecord *sr;

    filename = SCOREFILE;
    if (!empty_string(g_scorefile_name()))
      filename = g_scorefile_name();
    fp = open_scorefile_for_reading(filename);
    if ((NULL == fp) && !strcmp(filename, SCOREFILE))
      fp = open_scorefile_for_reading(OLD_SCOREFILE);
    if (fp != NULL) {
        /* Note that we clear all existing score records.  This is because
	   the score file may be shared and thus getting multiple updates. */
	/* (should free any existing records) */
	records = NULL;
	numrecs = 0;
	/* Read everything in the file. */
	while ((form = read_form(fp, &startlineno, &endlineno)) != lispeof) {
	    if (interp_score_record(form)) {
		++numrecs;
	    }
	}
	fclose(fp);
	Dprintf("%d score records read.\n", numrecs);
	for_all_score_records(sr) {
	    Dprintf("%s\n", sr->gamename);
	}
	return TRUE;
    }
    return FALSE;
}

static int
interp_score_record(Obj *form)
{
    const char *propname;
    Obj *props, *prop;
    ScoreRecord *sr;

    if (consp(form)
	&& symbolp(car(form))
	&& strcmp(c_string(car(form)), "g") == 0
	&& (stringp(cadr(form)) || symbolp(cadr(form)))) {
	sr = (ScoreRecord *) xmalloc(sizeof(ScoreRecord));
	sr->gamename = c_string(cadr(form));
	sr->sides = lispnil;
	sr->varsets = lispnil;
	for_all_list(cddr(form), props) {
	    prop = car(props);
	    if (symbolp(car(prop))) {
		propname = c_string(car(prop));
		if (strcmp(propname, "s") == 0) {
		    sr->sides = cdr(prop);
		} else if (strcmp(propname, "t") == 0) {
		    sr->numturns = c_number(cadr(prop));
		} else if (strcmp(propname, "v") == 0) {
		    sr->varsets = cdr(prop);
		} else if (strcmp(propname, "ve") == 0) {
		    /* (should do something with module version) */
		} else {
		    run_warning("Score record prop name `%s' not recognized, ignoring",
				propname);
		}
	    }
	}
	sr->raw = form;
	/* Add the record to the beginning of the list.  This is so displays list
	   the most recent game first. */
	sr->next = records;
	records = sr;
	return TRUE;
    } else {
	run_warning("Garbage in scorefile, ignoring");
    }
    return FALSE;
}

/* Collect scorefile contents and format them.  This routine starts from
   a fresh set of records each time, and allocates a new formatting
   buffer each time, so should be called sparingly. */

const char *
get_scores(Side *side)
{
    int wins, losses, draws, plays, allplays;
    char *buf, sdadvbuf[20], varbuf[BUFSIZE];
    const char *playername, *sdnamestr, *sdfatestr, *thisgame, *thisgametitle;
    Obj *sds, *sd, *sdname, *sdfate, *more, *more1;
    ScoreRecord *sr;

    thisgame = (mainmodule->origmodulename
		? mainmodule->origmodulename
		: mainmodule->name);
    if (thisgame == NULL)
      return "???";
    read_scorefile();
    if (records == NULL)
      return "No scores available.\n";
    buf = (char *)xmalloc(5000);
    thisgametitle = thisgame;
    if (!empty_string(mainmodule->title))
      thisgametitle = mainmodule->title;
    sprintf(buf, "Scores for %s", thisgametitle);
    strcat(buf, ":\n");
    playername = NULL;
    if (side != NULL) {
	playername = basic_player_name(side->player);
    }
    if (1 /* summarize */) {
	wins = losses = draws = plays = 0;
	allplays = 0;
	for_all_score_records(sr) {
	    if (!empty_string(sr->gamename)
		&& strcmp(thisgame, sr->gamename) == 0) {
		if (playername != NULL) {
		    /* Scan all the sides listed as having played in
                       the game. */
		    for_all_list(sr->sides, sds) {
			sd = car(sds);
			if (numberp(car(sd)))
			  sd = cdr(sd);
			sdname = car(sd);
			sdnamestr = ((symbolp(sdname) || stringp(sdname))
				     ? c_string(sdname) : (char *)"");
			sdfate = cadr(sd);
			sdfatestr = (symbolp(sdfate) ? c_string(sdfate) : (char *)"");
			if (sdnamestr != NULL
			    && strcmp(playername, sdnamestr) == 0) {
			    if (strcmp("won", sdfatestr) == 0) {
				++wins;
			    } else if (strcmp("lost", sdfatestr) == 0) {
				++losses;
			    } else if (strcmp("drew", sdfatestr) == 0) {
				++draws;
			    }
			    ++plays;
			    /* Only count the first appearance of the
			       player in the list; multiple
			       appearances can occur, but are likely
			       to be test games (two players with same
			       username open on the same X screen, for
			       instance) */
			    break;
			}
			/* (should study scores also?) */
		    }
		}
		++allplays;
	    }
	}
	/* (should go to nlang.c) */
	tprintf(buf, "You (%s) won %d, lost %d, and drew %d of %d game%s played.\n",
		playername, wins, losses, draws, plays, (plays == 1 ? "" : "s"));
	if (allplays != plays)
	  tprintf(buf, "Altogether, this game has been played %d time%s.\n",
		  allplays, (allplays == 1 ? "" : "s"));
	tprintf(buf, "\n\n");
    }
    /* List all the games explicitly. */
    if (1 /* complete listing */) {
	tprintf(buf, "Listing of games played:\n");
	for_all_score_records(sr) {
	    if (!empty_string(sr->gamename)
		&& strcmp(thisgame, sr->gamename) == 0) {
		if (playername == NULL
		    || 1 /* matching playername */) {
		    for_all_list(sr->sides, sds) {
			sd = car(sds);
			if (numberp(car(sd)))
			  sd = cdr(sd);
			sdname = car(sd);
			sdnamestr = ((symbolp(sdname) || stringp(sdname))
				     ? c_string(sdname) : (char *)"?");
			sdfate = cadr(sd);
			sdfatestr = (symbolp(sdfate) ? c_string(sdfate) : (char *)"?");
			strcpy(sdadvbuf, "");
			for_all_list(cddr(sd), more) {
			    more1 = car(more);
			    Dprintf("entry is");
			    Dprintlisp(more1);
			    Dprintf("\n");
			    if (consp(more1)
				&& symbolp(car(more1))
				&& strcmp(c_string(car(more1)), "a") == 0
				&& numberp(cadr(more1))) {
				sprintf(sdadvbuf, " +%d",
					c_number(cadr(more1)));
				break;
			    }
			}
			if (sds != sr->sides)
			  tprintf(buf, ", ");
			tprintf(buf, "%s%s %s",
				(!empty_string(sdnamestr)
				 ? sdnamestr : "(no player)"),
				sdadvbuf, sdfatestr);
		    }
		    if (sr->numturns > 0)
		      tprintf(buf, " (in %d turn%s)",
			      sr->numturns, (sr->numturns != 1 ? "s" : ""));
		    score_variant_desc(sr, varbuf);
		    if (!empty_string(varbuf)) {
			strcat(buf, " (variants");
			strcat(buf, varbuf);
			strcat(buf, ")");
		    }
		    tprintf(buf, "\n");
		    if (strlen(buf) > 4500)
		      break;
		}
	    }
	}
    }
    return buf;
}

/* Given a score record, report any variants that were specified and
   different from the current module's defaults. */

static void
score_variant_desc(ScoreRecord *sr, char *varbuf)
{
    int i, hasdflt, dfltval, val;
    Obj *restvarset, *varset, *varsetname, *rslt;
    Variant *variants, *tmpvar, *var;

    varbuf[0] = '\0';
    if (sr->varsets != lispnil) {
	variants = (mainmodule->origvariants
		    ? mainmodule->origvariants
		    : mainmodule->variants);
	for_all_list(sr->varsets, restvarset) {
	    varset = car(restvarset);
	    if (consp(varset)) {
		if (numberp(car(varset))) {
		    /* Get the names of the standard variants from
                       their numbers. */
		    switch (c_number(car(varset))) {
		      case 1:
			varsetname = intern_symbol(keyword_name(K_WORLD_SEEN));
			break;
		      case 2:
			varsetname = intern_symbol(keyword_name(K_SEE_ALL));
			break;
		      case 3:
			varsetname = intern_symbol(keyword_name(K_SEQUENTIAL));
			break;
		      case 4:
			varsetname = intern_symbol(keyword_name(K_PEOPLE));
			break;
		      case 5:
			varsetname = intern_symbol(keyword_name(K_ECONOMY));
			break;
		      case 6:
			varsetname = intern_symbol(keyword_name(K_SUPPLY));
			break;
		      case 11:
			varsetname = intern_symbol(keyword_name(K_WORLD_SIZE));
			break;
		      case 12:
			varsetname = intern_symbol(keyword_name(K_REAL_TIME));
			break;
		      default:
			break;
		    }
		} else {
		    varsetname = car(varset);
		}
		var = NULL;
		if (variants != NULL) {
		    for (i = 0; variants[i].id != lispnil; ++i) {
			tmpvar = &(variants[i]);
			if (tmpvar->id == varsetname) {
			    var = tmpvar;
			    break;
			}
		    }
		}
		hasdflt = dfltval = FALSE;
		/* Don't complain if variant not found, might have been
		   removed from current module. */
		if (var != NULL) {
		    rslt = eval(var->dflt);
		    if (numberp(rslt))
		      dfltval = c_number(rslt);
		    hasdflt = TRUE;
		}
		val = 54321;
		/* The numeric value is the second element in the variant
		   setting. */
		if (numberp(cadr(varset)) && cddr(varset) == lispnil)
		  val = c_number(cadr(varset));
		/* Handle variants with several values. */
		if (keyword_code(c_string(varsetname)) == K_WORLD_SIZE) {
		    if (!hasdflt || !equal(rslt, cdr(varset)))
		      tprintf(varbuf, " %s=%dx%dW%d", c_string(varsetname),
			      c_number(cadr(varset)), c_number(caddr(varset)),
			      c_number(cadr(cddr(varset))));
		} else if (keyword_code(c_string(varsetname)) == K_REAL_TIME) {
		    if (!hasdflt || !equal(rslt, cdr(varset)))
		      tprintf(varbuf, " %s=%d,%d,%d", c_string(varsetname),
			      c_number(cadr(varset)), c_number(caddr(varset)),
			      c_number(cadr(cddr(varset))));
		} else {
		    if (!hasdflt || val != dfltval)
		      tprintf(varbuf, " %s=%d", c_string(varsetname), val);
		}
	    }
	}
    }
}


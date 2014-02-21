/* Implementation of the "mplayer" AI in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "ai.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aioprt.h"

extern void register_mplayer(AI_ops *ops);

static Theater *tmptheater;

static Side *anewside;

/* Local function declarations. */

static void mplayer_init(Side *side);
static void mplayer_init_turn(Side *side);
static void mplayer_create_strategy(Side *side);
static void reset_strategy(Side *side);
static void analyze_the_game(Side *side);
static void determine_subgoals(Side *side);
static void review_theaters(Side *side);
static void create_initial_theaters(Side *side);
static void mplayer_calc_start_xy(Side *side);
static Theater *create_theater(Side *side);
static void remove_theater(Side *side, Theater *theater);
static void move_theater_cell(int x, int y);
static void remove_small_theaters(Side *side);
static void compute_theater_bounds(Side *side);
static void review_goals(Side *side);
static void mplayer_review_units(Side *side);
static void update_side_strategy(Side *side);
static void decide_theater_needs(Side *side, Theater *theater);
static void estimate_strengths(Side *side);
static void decide_resignation(Side *side);
static void mplayer_decide_plan(Side *side, Unit *unit);
static int mplayer_adjust_plan(Side *side, Unit *unit);
static void mplayer_react_to_task_result(Side *side, Unit *unit, Task *task, 
					 TaskOutcome rslt);
static void change_to_adjacent_theater(Side *side, Unit *unit);
static void mplayer_react_to_new_side(Side *side, Side *side2);
static void mplayer_finish_movement(Side *side);
static Unit *search_for_available_transport(Unit *unit, int purpose);
static char *mplayer_at_desig(Side *side, int x, int y);
static int mplayer_theater_at(Side *side, int x, int y);
static int mplayer_read_strengths(Side *side);
static Obj *mplayer_save_state(Side *side);

void
register_mplayer(AI_ops *ops)
{
    ops->name = "mplayer";
    ops->help = "general AI that can (supposedly) play any game";
    ops->to_init = mplayer_init;
    ops->to_init_turn = mplayer_init_turn;
    ops->to_decide_plan = mplayer_decide_plan;
    ops->to_react_to_task_result = mplayer_react_to_task_result;
    ops->to_react_to_new_side = mplayer_react_to_new_side;
    ops->to_adjust_plan = mplayer_adjust_plan;
    ops->to_finish_movement = mplayer_finish_movement;
    ops->to_save_state = mplayer_save_state;
    ops->region_at = mplayer_theater_at;
    ops->at_desig = mplayer_at_desig;
}

static void
mplayer_init(Side *side)
{
    /* Initialize master AI struct for side. */
    Xconq::AI::create_side_ai(side);
    /* Discover game class. */
    if (game_class == gc_none) {
	game_class = find_game_class();
    }
    /* Create AI strategy object. */
    mplayer_create_strategy(side);
    /* If the side has no units at the moment, it doesn't really need to
       plan. */
    if (!side_has_units(side))
      return;
    /* Compute an initial estimation of units on each side. */
    /* (Needed for save/restore consistency, otherwise not
       critical to do here.) */
    estimate_strengths(side);
    /* Study the scorekeepers and such, decide how to play the game. */
    analyze_the_game(side);
}

/* At the beginning of each turn, make plans and review the situation. */

static void
mplayer_init_turn(Side *side)
{
    using namespace Xconq::AI;

    int u, u2;
    Xconq::AI::AI_Side *ai = NULL;
    OpRole *oprole = NULL;

    /* Cases where we no longer need to run. */
    if (!side->ingame)
      return;
    /* A side without units hasn't got anything to do but wait. */
    /* (should account for possible units on controlled sides) */
    if (!side_has_units(side))
      return;
    /* Mplayers in a hacked game will not play,
       unless they're being debugged. */
    if (compromised && !DebugM)
      return;
    update_all_progress_displays("ai turn init start", side->id);
    DMprintf("%s mplayer init turn\n", side_desig(side));
    /* Make sure a strategy object exists. */
    if (ai(side) == NULL)
      mplayer_create_strategy(side);
    // Retrieve side AI.
    ai = get_side_ai(side);
    // Reset oprole exec counters.
    for_all_oproles(side, oprole) {
	oprole->execs_this_turn = 0;
	oprole->fails_this_turn = 0;
    }
    /* Look over the game design we're playing with. */
    analyze_the_game(side);
    if (ai(side)->report_not_understood) {
	notify_all("%s AI doesn't understand scoring in this game!", 
		   short_side_title(side));
	ai(side)->report_not_understood = FALSE;
    }
    /* code specific to the "time" game */
    if (game_class == gc_time) {
	for_all_unit_types(u) {
	    if (ai(side)->develop_status[u] == RS_DEVELOP_ASSIGNED) {
		u2 = ai(side)->develop_on[u];
		if (!needs_develop(side, u2)) {
		    /* develop done, start upgrading */
		    DMprintf("%s has completed develop on %s\n",
			     side_desig(side), u_type_name(u2));
		    ai(side)->develop_status[u] = RS_UPGRADE_NEEDED;
		}
	    }
	}
    }
    /* Check out the current goal tree first. */
    review_goals(side);
    /* Goal analysis might have triggered resignation. */
    if (!side->ingame)
      goto done;
    /* Check out all the theaters. */
    review_theaters(side);
    /* Check out all of our units. */
    mplayer_review_units(side);
    /* Decide on the new current plan. */
    update_side_strategy(side);
    /* Propagate this to individual unit plans. */
    update_unit_plans(side);
  done:
    update_all_progress_displays("", side->id);
    DMprintf("%s mplayer init turn done\n", side_desig(side));
}

/* Create and install an entirely new strategy object for the side. */

void
mplayer_create_strategy(Side *side)
{
    int s;
    Strategy *strategy = (Strategy *) xmalloc(sizeof(Strategy));

    /* Put the specific structure into a generic slot. */
    side->ai = (struct a_ai *) strategy;
    /* Allocate a table of pointers to theaters, for access via small numbers
       rather than full pointers. */
    strategy->theatertable = (Theater **) xmalloc(127 * sizeof(Theater *));
    /* Allocate a layer of indexes into the theater table. */
    strategy->areatheaters = malloc_area_layer(char);
    /* Allocate various things. */
    for (s = 0; s <= MAXSIDES; ++s) {
	strategy->strengths[s] = (short *) xmalloc(numutypes * sizeof(short));
	strategy->alstrengths[s] = (short *) xmalloc(numutypes * sizeof(short));
	strategy->strengths0[s] = (short *) xmalloc(numutypes * sizeof(short));
	strategy->alstrengths0[s] = 
	    (short *) xmalloc(numutypes * sizeof(short));
    }
    /* Arrays for unit types. */
    strategy->actualmix = (short *) xmalloc(numutypes * sizeof(short));
    strategy->expectedmix = (short *) xmalloc(numutypes * sizeof(short));
    strategy->idealmix = (short *) xmalloc(numutypes * sizeof(short));
    strategy->develop_status = (short *) xmalloc(numutypes * sizeof(short));
    strategy->develop_on     = (short *) xmalloc(numutypes * sizeof(short));
    /* Arrays for terrain types. */
    strategy->terrainguess = (short *) xmalloc(numttypes * sizeof(short));
    strategy->writable_state = lispnil;
    /* Set everything to correct initial values. */
    reset_strategy(side);
}

/* Put all the right initial values into the strategy, but don't
   allocate anything. */

static void
reset_strategy(Side *side)
{
    int u, u2, t, dir;
    Strategy *strategy = (Strategy *) side->ai;

    /* Remember when we did this. */
    strategy->creationdate = g_turn();
    /* Null out various stuff. */
    strategy->numgoals = 0;
    strategy->theaters = NULL;
    /* Actually we start with no theaters, but it's convenient to leave entry 0
       in the theater table pointing to NULL. */
    strategy->numtheaters = 1;
    /* Clear pointers to special-purpose theaters. */
    strategy->homefront = NULL;
    for_all_directions(dir) {
    	strategy->perimeters[dir] = NULL;
    	strategy->midranges[dir] = NULL;
    	strategy->remotes[dir] = NULL;
    }
    for_all_unit_types(u) {
	strategy->actualmix[u] = 0;
	strategy->expectedmix[u] = 0;
	strategy->idealmix[u] = 0;
	strategy->develop_status[u] = 0;
	strategy->develop_on[u] = 0;
	/* code specific to the "time" game */
	if (game_class == gc_time) {
	    for_all_unit_types(u2) {
		if (needs_develop(side, u2) && can_develop_on(u, u2)) {
		    strategy->develop_status[u] = RS_DEVELOP_NEEDED;
		    strategy->develop_on[u] = u2;
		    DMprintf("%s can develop on %s (to level %d)\n",
			     u_type_name(u), u_type_name(u2),
			     u_tech_to_build(u2));
		}
	    }
	}
    }
    /* Reset the summation of our exploration needs. */
    strategy->explorersneeded = 0;
    for_all_terrain_types(t) {
	strategy->terrainguess[t] = 0;
    }
    strategy->analyzegame = TRUE;
    /* Analyze the game and decide our basic goals. */
    analyze_the_game(side);
}

/* Look over the game design and decide what we're supposed to be doing,
   if anything at all.  This just sets up toplevel goals based on the
   game design, does not evaluate goals or any such. */

static void
analyze_the_game(Side *side)
{
    int maybedraw, i;
    Goal *goal;

    if (ai(side)->analyzegame) {
	ai(side)->trytowin = TRUE;
	/* Figure what exactly we have to do in order to win. */
	determine_subgoals(side);
	/* Machine will want to keep playing as long as it thinks
	   it has a chance to win. */
	maybedraw = FALSE;
	/* Be trusting about game saves, at least for now. (The problem
	   is that a human player could escape fate by saving the game
	   and then either editing the saved game or just throwing it
	   away.) */
	if (TRUE != side->willingtosave 
	  /* (should) and decision delegated to AI */)
	  net_set_willing_to_save(side, TRUE);
	if (maybedraw != side->willingtodraw 
	  /* (should) and decision delegated to AI */)
	  try_to_draw(side, maybedraw, "mplayer");
	ai(side)->analyzegame = FALSE;
	/* Summarize our analysis of this game. */
	DMprintf("%s will try to %s this game\n",
		 side_desig(side),
		 ai(side)->trytowin ? "win" : "have fun in");
	for (i = 0; i < ai(side)->numgoals; ++i) {
	    goal = ai(side)->goals[i];
	    DMprintf("%s has %s\n", side_desig(side), goal_desig(goal));
	}
    }
}

static void
determine_subgoals(Side *side)
{
    int understood;
    Side *side2;
    Goal *goal;
    AI_Side *ai = NULL;

    understood = TRUE;
    ai = Xconq::AI::get_side_ai(side);
    /* Analyze the scorekeepers to figure out any victory conditions. */
    ai->sk_analyses = analyze_scorekeepers(side);
    /* We might develop a sudden interest in exploration. */
    /* (but should only be if information is really important to winning) */
    if (!side->see_all) {
	/* It will be important to keep track of other sides' units
	   as much as possible. */
	for_all_sides(side2) {
	    if (side != side2) {
		goal = create_goal(GOAL_POSITIONS_KNOWN, side, TRUE);
		goal->args[0] = (long) side2;
		add_goal(side, goal);
	    }
	}
	/* Also add the general goal of knowing where indeps are. */
	goal = create_goal(GOAL_POSITIONS_KNOWN, side, TRUE);
	goal->args[0] = (long) indepside;
	add_goal(side, goal);
    }
}

/* Do a combination of analyzing existing theaters and creating new ones. */

static void
review_theaters(Side *side)
{
    int x, y, u, s, pop, totnumunits;
    int firstcontact = FALSE;
    int homefound = FALSE;
    Unit *unit;
    UnitView *uview;
    Side *firstcontactside, *homefoundside, *otherside, *side2;
    Theater *theater;

    /* Create some theaters if none exist. */
    if (ai(side)->theaters == NULL) {
	create_initial_theaters(side);
	compute_theater_bounds(side);
    }
    for_all_theaters(side, theater) {
	theater->allied_units = 0;
	theater->makers = 0;
	theater->unexplored = 0;
	theater->border = FALSE;
	theater->allied_bases = 0;
	for_all_unit_types(u) {
	    theater->numassigned[u] = 0;
	    theater->numneeded[u] = 0;
	    theater->numenemies[u] = 0;
	    theater->numsuspected[u] = theater->numsuspectedmax[u] = 0;
	    theater->numtotransport[u] = 0;
	}
	if (people_sides_defined()) {
	    for (s = 0; s <= numsides; ++s)
	      theater->people[s] = 0;
	}
	theater->units_lost /= 2;
    }
    /* Now look at all the units that we can. */
    for_all_side_units(side, unit) {
    	if (in_play(unit)) {
	    theater = unit_theater(unit);
	    if (theater != NULL) {
		++(theater->allied_units);
		++(theater->numassigned[unit->type]);
		if (isbase(unit))
		  ++(theater->allied_bases);
		if (unit->plan
		    && unit->plan->waitingfortransport)
		  ++(theater->numtotransport[unit->type]);
	    }
	}
    }
    /* (should also analyze allies etc) */
    /* Now look at the whole world. */
    for_all_interior_cells(x, y) {
	theater = theater_at(side, x, y);
	if (theater != NULL) {
	    if (side->see_all) {
		/* We get to look at the real units. */
	    	for_all_stack(x, y, unit) {
	    	    /* what about occupants? */
	    	    if (in_play(unit)
	    	    	&& !trusted_side(side, unit->side)
	    	    	&& (!indep(unit)
	    	    	    || u_point_value(unit->type) > 0)) {
			if (enemy_side(side, unit->side))
			  ++(theater->numenemies[unit->type]);
	    	    	if (ai(side)->contacted[unit->side->id] == 0) {
			    ai(side)->contacted[unit->side->id] = 1;
			    if (!indep(unit)) {
				firstcontact = TRUE;
				firstcontactside = unit->side;
			    }
	    	    	}
	    	    	if (ai(side)->homefound[unit->side->id] == 0
	    	    	    && !mobile(unit->type)) {
			    ai(side)->homefound[unit->side->id] = 1;
			    if (!indep(unit)) {
				homefound = TRUE;
				homefoundside = unit->side;
			    }
	    	    	}
	    	    }
	    	}
		if (people_sides_defined()) {
		    pop = people_side_at(x, y);
		    if (pop != NOBODY) {
			++(theater->people[pop]);
	    	    	if (ai(side)->homefound[pop] == 0) {
			    ai(side)->homefound[pop] = 1;
			    if (pop != 0) {
				homefound = TRUE;
				homefoundside = side_n(pop);
			    }
	    	    	}
		    }
		}
	    } else {
		/* We must satisfy ourselves with unit views. */
		if (terrain_view(side, x, y) == UNSEEN) {
		    ++(theater->unexplored);
		} else {
		    for_all_view_stack_with_occs(side, x, y, uview) {
			side2 = side_n(uview->siden);
			if (enemy_side(side, side2)) {
			    /* Note that we assume indeps are enemies
			       here. */
			    u = uview->type;
			    if (u_point_value(u) > 0) {
				++(theater->numsuspected[u]);
				++(theater->numsuspectedmax[u]);
			    }
			}
		    }
		    if (people_sides_defined()) {
			pop = people_side_at(x, y);
			if (pop != NOBODY) {
			    ++(theater->people[pop]);
			}
		    }
		}
	    }
	}
    }
    for_all_theaters(side, theater) {
    	theater->x = (theater->xmin + theater->xmax) / 2;
    	theater->y = (theater->ymin + theater->ymax) / 2;
    	theater->enemystrengthmin = theater->enemystrengthmax = 0;
    	for_all_unit_types(u) {
	    theater->enemystrengthmin +=
	      theater->numenemies[u] + theater->numsuspected[u];
	}
	theater->enemystrengthmax = theater->enemystrengthmin;
    }
    
    if ((firstcontact || homefound)
        /* We dont want to replan every unit at turn one in see_all games. */
        && !side->see_all) {
	DMprintf("Forced replan of all %s ai-controlled units because of enemy contact.\n", side_desig(side));
    	for_all_side_units(side, unit) {
	    if (ai_controlled(unit)) {
		net_force_replan(unit);
		set_unit_theater(unit, NULL);
		update_unit_display(side, unit, TRUE);
	    }
	}
    }
    for_all_theaters(side, theater) {
	DMprintf("%s theater \"%s\" at %d,%d from %d,%d to %d,%d (size %d)\n",
		 side_desig(side), theater->name, theater->x, theater->y,
		 theater->xmin, theater->ymin, theater->xmax, theater->ymax,
		 theater->size);
	/* Summarize what we know about the theater. */
	DMprintf("%s theater \"%s\"", side_desig(side), theater->name);
	if (!side->see_all && theater->unexplored > 0) {
	    DMprintf(" unexplored %d", theater->unexplored);
	}
	DMprintf(" enemy %d", theater->enemystrengthmin);
	if (theater->enemystrengthmin != theater->enemystrengthmax) {
	    DMprintf("-%d", theater->enemystrengthmax);
	}
	for_all_unit_types(u) {
	    if (theater->numenemies[u] + theater->numsuspected[u] > 0) {
	    	DMprintf(" %3s %d", u_type_name(u), theater->numenemies[u]);
	    	if (theater->numsuspected[u] > 0) {
		    DMprintf("+%d", theater->numsuspected[u]);
		}
	    }
	}
	if (people_sides_defined()) {
	    DMprintf(" people");
	    for (s = 0; s <= numsides; ++s) {
		if (theater->people[s] > 0) {
		    DMprintf(" s%d %d", s, theater->people[s]);
		}
	    }
	}
	DMprintf("\n");
	totnumunits = 0;
	for_all_unit_types(u) {
	    totnumunits +=
	      (theater->numassigned[u] + theater->numneeded[u] + theater->numtotransport[u]);
	}
	if (totnumunits > 0) {
	    /* Summarize the status of our own units in this theater. */
	    DMprintf("%s theater \"%s\" has ", side_desig(side), theater->name);
	    for_all_unit_types(u) {
		if (theater->numassigned[u] + theater->numneeded[u] + theater->numtotransport[u] > 0) {
		    DMprintf(" %d %3s", theater->numassigned[u], u_type_name(u));
			if (theater->numneeded[u] > 0) {
			    DMprintf(" (of %d needed)", theater->numneeded[u]);
			}
			if (theater->numtotransport[u] > 0) {
			    DMprintf(" (%d awaiting transport)", theater->numtotransport[u]);
			}
		}
	    }
	    DMprintf("\n");
	}
    }
    /* Also summarize contacts. */
    for_all_sides(otherside) {
    	if (otherside != side) {
	    if (ai(side)->contacted[otherside->id]) {
		DMprintf("%s contacted s%d", side_desig(side), otherside->id);
		if (ai(side)->homefound[otherside->id]) {
		    DMprintf(", home found");
		}
		DMprintf("\n");
	    }
    	}
    }
}

/* Set up the initial set of theaters. */

static void
create_initial_theaters(Side *side)
{
    int x, y, dir, dist, i, j;
    int xmin, ymin, xmax, ymax;
    int homeradius, perimradius, midradius, xxx;
    int numthx, numthy, thwid, thhgt;
    Unit *unit;
    Theater *homefront, *enemyarea, *theater;
    Theater *gridtheaters[8][8];
    Strategy *strategy = ai(side);
    
    for (i = 0; i < 8; ++i) {
	for (j = 0; j < 8; ++j) {
	    gridtheaters[i][j] = NULL;
	}
    }
    /* Compute bbox of initial (should also do enemy?) units. */
    xmin = area.width;  ymin = area.height;  xmax = ymax = 0;
    for_all_side_units(side, unit) {
	if (alive(unit) /* and other preconditions? */) {
	    if (unit->x < xmin)
	      xmin = unit->x;
	    if (unit->y < ymin)
	      ymin = unit->y;
	    if (unit->x > xmax)
	      xmax = unit->x;
	    if (unit->y > ymax)
	      ymax = unit->y;
	}
    }
    /* Most games start with each side's units grouped closely together.
       If this is not the case, do something else. */
    if (xmax - xmin > area.width / 4 && ymax - ymin > area.height / 4) {
	/* (should do some sort of clustering of units) */
	if (0 /*people_sides_defined()*/) {
	    homefront = create_theater(side);
	    homefront->name = "Home Front";
	    enemyarea = create_theater(side);
	    enemyarea->name = "Enemy Area";
	    for_all_interior_cells(x, y) {
	        if (people_side_at(x, y) == side->id) {
		    set_theater_at(side, x, y, homefront);
	        } else {
		    set_theater_at(side, x, y, enemyarea);
	        }
	    }
	} else {
	    /* Divide the world up along a grid. */
	    numthx = (area.width  > 60 ? (area.width  > 120 ? 7 : 5) : 3);
	    numthy = (area.height > 60 ? (area.height > 120 ? 7 : 5) : 3);
	    thwid = max(8, area.width / numthx);
	    thhgt = max(8, area.height / numthy);
	    for_all_interior_cells(x, y) {
		i = x / thwid;  j = y / thhgt;
		if (gridtheaters[i][j] == NULL) {
		    theater = create_theater(side);
		    sprintf(spbuf, "Grid %d,%d", i, j);
		    theater->name = copy_string(spbuf);
		    theater->x = x;  theater->y = y;
		    gridtheaters[i][j] = theater;
		} else {
		    theater = gridtheaters[i][j];
		}
		set_theater_at(side, x, y, theater);
	    }
	}
	return;
    } else {
	/* Always create a first theater that covers the starting area. */
	homefront = create_theater(side);
	homefront->name = "Home Front";
	/* Calculate startxy if not already available. */
	if (side->startx < 0 && side->starty < 0) {
		/* Broadcasting needed to avoid sync errors! */
		mplayer_calc_start_xy(side);
	}
	homefront->x = side->startx;  homefront->y = side->starty;
	strategy->homefront = homefront;
	homeradius = max(5, g_radius_min());
	perimradius = max(homeradius + 5, g_separation_min() - homeradius);
	midradius = max(perimradius + 10, g_separation_min() * 2);
	xxx = max((side->startx - perimradius), (area.width - side->startx - perimradius));
	xxx /= 2;
	midradius = min(midradius, perimradius + xxx);
	for_all_interior_cells(x, y) {
	    dist = distance(x, y, side->startx, side->starty);
	    if (people_sides_defined()
		&& people_side_at(x, y) == side->id
		&& dist < (perimradius - 3)) {
		set_theater_at(side, x, y, homefront);
	    } else {
		if (dist < homeradius) {
		    set_theater_at(side, x, y, homefront);
		} else {
		    dir = approx_dir(x - side->startx, y - side->starty);
		    if (dist < perimradius) {
			if (strategy->perimeters[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Perimeter %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->perimeters[dir] = theater;
			} else {
			    theater = strategy->perimeters[dir];
			}
		    } else if (dist < midradius) {
			if (strategy->midranges[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Midrange %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->midranges[dir] = theater;
			} else {
			    theater = strategy->midranges[dir];
			}
		    } else {
			if (strategy->remotes[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Remote %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->remotes[dir] = theater;
			} else {
			    theater = strategy->remotes[dir];
			}
		    }
		    set_theater_at(side, x, y, theater);
	    	}
	    }
	}  
    }
    remove_small_theaters(side);
    /* Assign all units to the theater they're currently in. */
    /* (how do reinforcements get handled? mplayer should get hold of perhaps) */
    for_all_side_units(side, unit) {
	if (in_play(unit) /* and other preconditions? */) {
	    set_unit_theater(unit, theater_at(side, unit->x, unit->y));
	}
    }
}

/* Calculate the centroid of all the starting units. Broadcasting clone of function
in side.c. This is needed to avoid sync errors. */

void
mplayer_calc_start_xy(Side *side)
{
    int num = 0, sumx = 0, sumy = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (in_play(unit)) {
	    sumx += unit->x;  
	    sumy += unit->y;
	    ++num;
	}
    }
    if (num > 0) {
	net_set_side_startx(side, wrapx(sumx / num));  
	net_set_side_starty(side, sumy / num);  
    }
}

/* Create a single theater object and link it into the list of
   theaters. */

/* (should be able to re-use theaters in already in theater table) */

static Theater *
create_theater(Side *side)
{
    Theater *theater = (Theater *) xmalloc(sizeof(Theater));
    
    if (ai(side)->numtheaters > MAXTHEATERS)
      return NULL;
    theater->id = (ai(side)->numtheaters)++;
    theater->name = "?";
    theater->maingoal = NULL;
    theater->numassigned = (short *) xmalloc(numutypes * sizeof(short));
    theater->numneeded = (short *) xmalloc(numutypes * sizeof(short));
    theater->numtotransport = (short *) xmalloc(numutypes * sizeof(short));
    theater->numenemies = (short *) xmalloc(numutypes * sizeof(short));
    theater->numsuspected = (short *) xmalloc(numutypes * sizeof(short));
    theater->numsuspectedmax = (short *) xmalloc(numutypes * sizeof(short));
    theater->people = (int *) xmalloc((numsides + 1) * sizeof(int));
    /* Connect theater into a linked list. */
    theater->next = ai(side)->theaters;
    ai(side)->theaters = theater;
    /* Install it into the theater table also. */
    ai(side)->theatertable[theater->id] = theater;
    return theater;
}

/* Clear all references to the theater and remove it from the list.
   Note that the theater size must already be zero. */

static void
remove_theater(Side *side, Theater *theater)
{
    int dir;
    Theater *prev;

    if (ai(side)->homefront == theater)
      ai(side)->homefront = NULL;
    for_all_directions(dir) {
	if (ai(side)->perimeters[dir] == theater)
	  ai(side)->perimeters[dir] = NULL;
	if (ai(side)->midranges[dir] == theater)
	  ai(side)->midranges[dir] = NULL;
	if (ai(side)->remotes[dir] == theater)
	  ai(side)->remotes[dir] = NULL;
    }
    if (ai(side)->theaters == theater)
      ai(side)->theaters = theater->next;
    else {
	prev = NULL;
	for_all_theaters(side, prev) {
	    if (prev->next == theater) {
		prev->next = theater->next;
		break;
	    }
	}
	/* If prev still null, badness */
    }
    --(ai(side)->numtheaters);
}

static void
move_theater_cell(int x, int y)
{
    int dir, x1, y1;
    Theater *theater2;

    if (theater_at(tmpside, x, y) == tmptheater) {
	for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		theater2 = theater_at(tmpside, x1, y1);
		if (theater2 != NULL && theater2 != tmptheater) {
		    set_theater_at(tmpside, x, y, theater2);
		    ++(theater2->size);
		    /* (should recompute bbox too) */
		    --(tmptheater->size);
		}
	    }
	}
    }
}

static void
remove_small_theaters(Side *side)
{
    int domore;
    Theater *theater;

    compute_theater_bounds(side);
    domore = TRUE;
    while (domore) {
	domore = FALSE;
	for_all_theaters(side, theater) {
	    if (between(1, theater->size, 5)) {
		tmpside = side;
		tmptheater = theater;
		apply_to_area(theater->x, theater->y, 6, move_theater_cell);
		if (theater->size == 0) {
		    remove_theater(side, theater);
		    /* Have to start over now. */
		    domore = TRUE;
		    break;
		}
	    }
	}
    }
    /* Redo, many random changes to bounds. */
    compute_theater_bounds(side);
}

/* Compute the size and bounding box of each theater.  This should be run
   each time theaters change in size or shape. */

static void
compute_theater_bounds(Side *side)
{
    int x, y;
    Theater *theater;

    for_all_theaters(side, theater) {
	theater->size = 0;
	theater->xmin = theater->ymin = -1;
	theater->xmax = theater->ymax = -1;
    }
    for_all_interior_cells(x, y) {
	theater = theater_at(side, x, y);
	if (theater != NULL) {
	    ++(theater->size);
	    /* Compute bounding box of theater if not already done. */
	    if (theater->xmin < 0 || x < theater->xmin)
	      theater->xmin = x;
	    if (theater->ymin < 0 || y < theater->ymin)
	      theater->ymin = y;
	    if (theater->xmax < 0 || x > theater->xmax)
	      theater->xmax = x;
	    if (theater->ymax < 0 || y > theater->ymax)
	      theater->ymax = y;
	}
    }
}

/* Examine the goals to see what has been accomplished and what still
   needs to be done. */

static void
review_goals(Side *side)
{
    int i;
    Scorekeeper *sk;
    Goal *goal;
    Side *side2;
    Strategy *strategy = ai(side);

    /* First check on our friends and enemies. */
    for_all_sides(side2) {
	/* If they're not trusting us, we don't want to trust them. */
	/* (should be able to update this immediately after other side
           changes trust) */
	if (!trusted_side(side2, side) && trusted_side(side, side2))
	  net_set_trust(side, side2, FALSE);
    }
    for (i = 0; i < strategy->numgoals; ++i) {
	goal = strategy->goals[i];
	DMprintf("%s has %s\n", side_desig(side), goal_desig(goal));
    }
    /* Should look at certainty of each goal and decide whether to keep or
       drop it, and mention in debug output also. */
    /* Also think about resigning. */
    if (g_ai_may_resign()
         && keeping_score()) {
	for_all_scorekeepers(sk) {
	    if (symbolp(sk->body)
		&& (match_keyword(sk->body, K_LAST_SIDE_WINS)
		    || match_keyword(sk->body, K_LAST_ALLIANCE_WINS))) {
		decide_resignation(side);
	    }
	}
    }
}

static void
estimate_strengths(Side *side)
{
    int u, sn1, x, y;
    Side *side1, *side2;
    Strategy *strategy = ai(side);
    Unit *unit;
    UnitView *uview;

    for_all_sides(side1) {
	sn1 = side1->id;
	for_all_unit_types(u) {
	    (strategy->strengths[sn1])[u] = 0;
	}
	/* this lets us count even semi-trusted allies' units accurately... */
	if (side1 == side || allied_side(side, side1)) {
	    for_all_side_units(side1, unit) {
		/* Note that we count off-area units, since they are
		   reinforcements usually. */
		if (alive(unit) && completed(unit)) {
		    ++((strategy->strengths[sn1])[unit->type]);
		}
	    }
	}
    }
    if (side->see_all
	|| (!strategy->initial_strengths_computed 
	    && !mplayer_read_strengths(side))) {
	/* If we can see everything, we can add up units accurately.
	   We also allow initial strengths to be known accurately;
	   this prevents the AI from resigning just because an
	   important unit appears for the first time, even though all
	   players know that it was one of the starting units. */
	for_all_cells(x, y) {
	    for_all_stack(x, y, unit) {
		side2 = unit->side;
		if (side2 != NULL
		    && !(side2 == side || allied_side(side, side2))) {
		    if (completed(unit)) {
		        ++((strategy->strengths[side2->id])[unit->type]);
		    }
		}
	    }
	}
    } else {
	/* Look at the current view to get enemy strength. */
	/* This is too easily faked, and doesn't know about hiding units... */
	/* Should also discount old data. */
	for_all_cells(x, y) {
	    for_all_view_stack_with_occs(side, x, y, uview) {
		side2 = side_n(uview->siden);
		/* Count only units on other sides. */
		if (!(side2 == side || allied_side(side, side2))) {
		  	++((strategy->strengths[side2->id])[uview->type]);
	    	}
	    }
	}
    }
    /* Estimate point values. */
    /* (should try to account for individual units with special point
       values) */
    for_all_sides(side1) {
	sn1 = side1->id;
	strategy->points[sn1] = 0;
	for_all_unit_types(u) {
	    strategy->points[sn1] +=
	      (strategy->strengths[sn1])[u] * u_point_value(u);
	}
    }
    /* Estimate how many of each type in allied group. */
    for_all_sides(side1) {
	sn1 = side1->id;
	for_all_unit_types(u) {
	    strategy->alstrengths[sn1][u] = (strategy->strengths[sn1])[u];
	    for_all_sides(side2) {
		if (side1 != side2 && allied_side(side1, side2)) {
		    strategy->alstrengths[sn1][u] +=
		      (strategy->strengths[side2->id])[u];
		}
	    }
	}
	strategy->alpoints[sn1] = strategy->points[sn1];
	for_all_sides(side2) {
	    if (side1 != side2 && allied_side(side1, side2)) {
		strategy->alpoints[sn1] += strategy->points[side2->id];
	    }
	}
    }
    /* The first time we estimate strength, record it specially.
       Later we will use this to compute which sides are getting
       stronger/weaker as time goes on. */
    if (!strategy->initial_strengths_computed) {
	if (!mplayer_read_strengths(side)) {
	    for_all_sides(side1) {
		sn1 = side1->id;
		strategy->points0[sn1] = strategy->points[sn1];
		strategy->alpoints0[sn1] = strategy->alpoints[sn1];
		for_all_unit_types(u) {
		    (strategy->strengths0[sn1])[u] =
		      (strategy->strengths[sn1])[u];
		    (strategy->alstrengths0[sn1])[u] =
		      (strategy->alstrengths[sn1])[u];
		}
	    }
	}
	ai_save_state(side);
	strategy->initial_strengths_computed = TRUE;
    }
    /* If we're calling strength estimation because a new side has
       come into existence, use the current strengths as the new
       side's initial strengths. */
    if (anewside != NULL) {
	sn1 = anewside->id;
	strategy->points0[sn1] = strategy->points[sn1];
	strategy->alpoints0[sn1] = strategy->alpoints[sn1];
	for_all_unit_types(u) {
	    (strategy->strengths0[sn1])[u] =
	      (strategy->strengths[sn1])[u];
	    (strategy->alstrengths0[sn1])[u] =
	      (strategy->alstrengths[sn1])[u];
	}
	/* Have to redo the saveable state also; force this by
	   blasting any existing recordable state (it should all be
	   re-creatable from mplayer's internal state). */
	strategy->writable_state = lispnil;
	ai_save_state(side);
    }
    /* Dump out a detailed listing of our estimates. */
    if (DebugM) {
	for_all_sides(side1) {
	    sn1 = side1->id;
	    DMprintf("%s ", side_desig(side));
	    DMprintf("est init streng of %s: ", side_desig(side1));
	    for_all_unit_types(u) {
		DMprintf(" %d", (strategy->strengths0[sn1])[u]);
	    }
	    DMprintf(" (%d points)\n", strategy->points0[sn1]);
	    DMprintf("%s ", side_desig(side));
	    DMprintf("est curr streng of %s: ", side_desig(side1));
	    for_all_unit_types(u) {
		DMprintf(" %d", (strategy->strengths[sn1])[u]);
	    }
	    DMprintf(" (%d points)\n", strategy->points[sn1]);
	    DMprintf("%s ", side_desig(side));
	    DMprintf("est init allied of %s: ", side_desig(side1));
	    for_all_unit_types(u) {
		DMprintf(" %d", (strategy->alstrengths0[sn1])[u]);
	    }
	    DMprintf(" (%d points)\n", strategy->alpoints0[sn1]);
	    DMprintf("%s ", side_desig(side));
	    DMprintf("est curr allied of %s: ", side_desig(side1));
	    for_all_unit_types(u) {
		DMprintf(" %d", (strategy->alstrengths[sn1])[u]);
	    }
	    DMprintf(" (%d points)\n", strategy->alpoints[sn1]);
	}
    }
}

/* Sometimes there is no point in going on, but be careful not to be
   too pessimistic.  Right now we only give up if no hope at all.
   Currently this is only used if there is a last-side-wins
   scorekeeper; it would need to be modified considerably to be useful
   with scorekeepers in general. */

static void
decide_resignation(Side *side)
{
    int sn, sn1, ratio, ratio0, chance = 0, chance1;
    Side *side1;
    Strategy *strategy = ai(side);

    /* Disable resignation during the first 10 turns. This fixes problem 
    with mplayer resigning at the start of the game if it has one settler
    (1 point) and some other side already built a city (10 points). */
    if (g_turn() < 10)
        return;
    sn = side->id;
    estimate_strengths(side);
    /* If our estimate of our own points is zero, then we're about to
       lose the game anyway, so just return and avoid screwing up
       ratio calcs below. */
    if (strategy->alpoints[sn] <= 0)
      return;
    for_all_sides(side1) {
	if (side != side1 && side1->ingame && !allied_side(side, side1)) {
	    sn1 = side1->id;
	    /* Note that ratio calculations always scale up by 100, so
	       that we can do finer comparisons without needing
	       floating point. */
	    ratio = (strategy->alpoints[sn1] * 100) / strategy->alpoints[sn];
	    /* If the AI can see all enemy units everywhere, it resigns much 
	    too easily. (Iceland should not quit just because there are lots of
	    Chineese troops in China) (should ultimately test for proximity 
	    to enemy)*/
	    if (side->see_all) {
		ratio /= 5;
	    /* code specific to the "time" game */
	    /* the mplayer can severely underestimates its own strength */
	    } else if (game_class == gc_time) {
		ratio /= 3;
	    }
	    chance1 = 0;
	    if (strategy->alpoints0[sn] > 0) {
		ratio0 =
		  (strategy->alpoints0[sn1] * 100) / strategy->alpoints0[sn];
		/* If we estimated 0 points for some side's initial
		   strength, then our estimate is bad; assume
		   parity. */
		if (ratio0 <= 0)
		  ratio0 = 100;
		/* This formula basically calls for no resignation if
		   ratio is no more than twice what it was initially,
		   50% chance if ratio is four times what it was (if
		   we started out even, then we're outnumbered 4 to
		   1), and interpolates for ratios in between. */
		chance1 = (((ratio * 100) / ratio0) - 200) / 5;
		chance1 = max(chance1, 0);
	    } else {
		/* work by absolute ratios */
		if (ratio > 400) {
		    chance1 = ratio / 10;
		}
	    }
	    /* The overall chance is determined by the most threatening
	       side or alliance. */
	    chance = max(chance, chance1);
	}
    }
    /* Never go all the way to 100%; perhaps the lopsided ratio is
       just a temporary setback. */
    chance = min(chance, 90);
    /* Whether or not we actually resign, we may be willing to go for
       a draw if other players want to. */
    /* (Note that toggling this flag is not exactly poker-faced
       behavior, but I doubt human players will be able to derive much
       advantage, since they'll already have a pretty good idea if the
       AI is in trouble or not.) */
    try_to_draw(side, (chance > 0), "mplayer");
    /* Maybe resign. */
    if (chance > 0) {
	if (probability(chance)) {
	    give_up(side, "mplayer");
	}
    }
}

/* Go through all our units (and allied ones?). */

static int *mru_numoffensive;
static int *mru_numdefensive;

static void
mplayer_review_units(Side *side)
{
    using namespace Xconq::AI;

    int u, u2, cp, cpmin, any;
    Unit *unit, *occ, *unit2, *transport = NULL;
    UnitView *uview = NULL, *uvstack = NULL;
    Plan *plan;
    Theater *oldtheater, *theater;

    if (mru_numoffensive == NULL)
      mru_numoffensive = (int *) xmalloc(numutypes * sizeof(int));
    if (mru_numdefensive == NULL)
      mru_numdefensive = (int *) xmalloc(numutypes * sizeof(int));
    /* This code is specific to the "time" game. */
    if (game_class == gc_time) {
	for_all_unit_types(u) {
	    u2 = ai(side)->develop_on[u];
	    if (ai(side)->develop_status[u] == RS_DEVELOP_ASSIGNED) {
		/* is anyone developing? */
		unit2 = NULL;
		for_all_side_units(side, unit) {
		    if (unit->type==u && in_play(unit) &&
			unit->plan && unit->plan->aicontrol) {
			if (unit->plan->tasks &&
			    unit->plan->tasks->type == TASK_DEVELOP &&
			    unit->plan->tasks->args[0] == u2) 
			  unit2 = unit;
		    }
		}
		if (unit2 != NULL) {
		    DMprintf("%s is developing for %s on %s (level %d/%d)\n",
			     unit_desig(unit2), side_desig(side),
			     u_type_name(u2),
			     side->tech[u2], u_tech_to_build(u2));
		} else {
		    DMprintf("no %s is developing for %s on %s!\n",
			     u_type_name(u), side_desig(side),
			     u_type_name(u2));
		    ai(side)->develop_status[u] = RS_DEVELOP_NEEDED;
		}
	    }
	    if (ai(side)->develop_status[u] == RS_DEVELOP_NEEDED
		&& needs_develop(side, u2)) {
		/* pick for develop a unit not building; 
		   if all are building, choose the one which started last */
		unit2 = NULL;
		cpmin = 9999;
		any = 0;
		for_all_side_units(side, unit) {
		    if (unit->type == u
			&& in_play(unit)
			&& unit->plan
			&& unit->plan->aicontrol) {
			any = 1;
			cp = 0;
			occ = NULL;
			if ((unit->plan->tasks != NULL
			     && unit->plan->tasks->type == TASK_BUILD))
			  occ = find_unit_to_complete(unit, unit->plan->tasks);
			if (occ != NULL) {
			    cp = occ->cp - uu_creation_cp(u,occ->type);
			    if (uu_cp_per_build(u,u2) > 0)
			      cp /= uu_cp_per_build(u,u2);
			}
			if (cp < cpmin) {
			    unit2 = unit;
			    cpmin = cp;
			}
		    }
		}
		if (unit2 == NULL) {
		    if (any)
		      DMprintf("no %s is available to develop for %s on %s!\n",
			       u_type_name(u), side_desig(side),
			       u_type_name(u2));
		} else {
		    if (assign_to_develop_on(side, unit2, u2)) {
			ai(side)->develop_status[u] =
			  RS_DEVELOP_ASSIGNED;
		    }
		}
	    }
	}
    }
    for_all_unit_types(u) {
	mru_numoffensive[u] = mru_numdefensive[u] = 0;
    }
    for_all_side_units(side, unit) {
	if (in_play(unit) && ai_controlled(unit)) {
	    /* Count plan types. */
	    switch (unit->plan->type) {
	      case PLAN_OFFENSIVE:
	      case PLAN_EXPLORATORY:
		++mru_numoffensive[unit->type];
		break;
	      case PLAN_DEFENSIVE:
		++mru_numdefensive[unit->type];
		break;
	      default:
	        break;
	    }
	}
    }
    for_all_side_units(side, unit) {
	if (in_play(unit) && ai_controlled(unit)) {
	    /* code specific to the "time" game */
	    if (game_class == gc_time) {
		u = unit->type;
		u2 = ai(side)->develop_on[u];
		/* should we upgrade? */
		if (ai(side)->develop_status[u] == RS_UPGRADE_NEEDED) {
		    cp = 0;
		    occ = NULL;
		    if ((unit->plan->tasks != NULL &&
			 unit->plan->tasks->type == TASK_BUILD))
		      occ = find_unit_to_complete(unit, unit->plan->tasks);
		    if (occ != NULL) {
			cp = occ->cp - uu_creation_cp(u,occ->type);
			if (uu_cp_per_build(u,u2)>0)
			  cp /= uu_cp_per_build(u,u2);
		    }
		    if (occ != NULL && occ->type==u2) {
			/* already upgrading */
			DMprintf("%s is upgrading to %s (%d/%d cp)\n",
				 unit_desig(unit), u_type_name(u2),
				 occ->cp, u_cp(occ->type));
		    } else if (cp >= u_cp(u2)/4) { /* rule-of-thumb... */
			/* complete unit under construction */
			DMprintf("%s will complete %s (now %d/%d cp) before upgrading to %s\n",
				 unit_desig(unit), u_type_name(occ->type),
				 occ->cp, u_cp(occ->type), u_type_name(u2));
		    } else {
			/* start upgrading */
			if (occ != NULL && !completed(occ)) {
			    DMprintf("%s will drop work on %s (%d/%d cp) and immediately start upgrading to %s\n",
				 unit_desig(unit), u_type_name(occ->type),
				 occ->cp, u_cp(occ->type), u_type_name(u2));
			} else {
			    DMprintf("%s will start upgrading to %s\n",
				     unit_desig(unit), u_type_name(u2));
			}
			net_set_unit_plan_type(side, unit, PLAN_IMPROVING);
			// Rummage through potential transports in cell.
			uvstack = query_uvstack_at(unit->x, unit->y);
			for_all_uvstack(uvstack, uview) {
			    transport =
				choose_transport_to_construct_in(
				    u2, unit->side, uview);
			    if (transport)
				break;
			}
			if (transport)
			    net_set_construct_task(
				unit, u2, 1, transport->id, unit->x, unit->y);
			else
			    net_set_construct_task(
				unit, u2, 1, -1, unit->x, unit->y);
		    }
		}
	    }

	    plan = unit->plan;
	    oldtheater = unit_theater(unit);
	    /* Goal might have become satisfied. */
	    if (plan->maingoal) {
		if (goal_truth(side, plan->maingoal) == 100) {
		    DMprintf("Forced replan: %s %s satisfied, removing\n",
			     unit_desig(unit), goal_desig(plan->maingoal));
		    net_force_replan(unit);
		    set_unit_theater(unit, NULL);
		}
	    }
	    /* Theater might have become explored enough (90% known). */
	    if (plan->type == PLAN_EXPLORATORY
	        && (theater = unit_theater(unit)) != NULL
	        && theater->unexplored < theater->size / 10) {
		    DMprintf("Forced replan: %s theater %s is mostly known\n",
			     unit_desig(unit), theater->name);
		    net_force_replan(unit);
		    set_unit_theater(unit, NULL);
	    }
	    /* Don't let defense-only units pile up. */
	    if (plan->type == PLAN_DEFENSIVE
		&& mobile(unit->type)
		&& (mru_numoffensive[unit->type] / 3) < mru_numdefensive[unit->type]
		/* However, don't mess with units that have specific defensive goals. */
		&& (plan->maingoal == NULL
			|| (plan->maingoal->type != GOAL_UNIT_OCCUPIED
			 && plan->maingoal->type != GOAL_CELL_OCCUPIED))
		&& flip_coin()) {
		DMprintf("Forced replan: %s one of too many on defense (%d off, %d def), replanning\n",
			 unit_desig(unit), mru_numoffensive[unit->type], mru_numdefensive[unit->type]);
		net_force_replan(unit);
	    }
	    theater = unit_theater(unit);
	    DMprintf("%s currently assigned to %s",
	    	     unit_desig(unit),
		     (theater ? theater->name : "no theater"));
	    if (oldtheater != theater) {
	    	DMprintf(" (was %s)",
			 (oldtheater ? oldtheater->name : "no theater"));
	    }
	    DMprintf("\n");
	}
    }
}

/* Look at our current overall strategy and hack it as needed. */

static void
update_side_strategy(Side *side)
{
    Theater *theater;

    DMprintf("%s updating strategy\n", side_desig(side));
    /* Add something to add/update theaters as things open up. (?) */
    for_all_theaters(side, theater) {
	decide_theater_needs(side, theater);
    }
}

/* Figure out how many units to request for each area. */

static void
decide_theater_needs(Side *side, Theater *theater)
{
    if (theater->unexplored > 0) {
    	/* Exploration is less important when 90% of a theater is known. */
    	if (theater->unexplored > (theater->size / 10)) {
	    ++(ai(side)->explorersneeded);
    	}
	/* Should look for good exploration units. */
	theater->importance = 50;  /* should depend on context */
/*	theater->reinforce = EXPLORE_AREA;  */
#if 0
    } else if (0 /* few enemies? */) {
	if (theater->allied_makers == 0
	    && theater->makers > 0
	    && theater->nearby) {
	    theater->reinforce = GUARD_BORDER_TOWN + 2 * theater->makers;
	} else if (theater->makers > 0) {
	    theater->reinforce = (theater->border ? GUARD_BORDER_TOWN :
				  GUARD_TOWN) + 2 * theater->allied_makers;
	} else if (theater->allied_bases > 0) {
	    theater->reinforce = (theater->border ? GUARD_BORDER: GUARD_BASE);
	} else if (theater->border) {
	    theater->reinforce = NO_UNITS;
	} else {
	    theater->reinforce = NO_UNITS;
	}
    } else {
	if (theater->allied_makers > 0) {
	    theater->reinforce = DEFEND_TOWN + 5 * theater->makers;
	} else if (theater->allied_bases > 0) {
	    theater->reinforce = DEFEND_BASE + theater->allied_bases;
	} else {
	    theater->reinforce = 0 /* DEFEND_AREA */;
	}
#endif
    }
}

/* Different slots for plan assignment lottery. */

enum AI_PL_ChanceSlots {
    AI_PLCS_RESERVE = 0,
    AI_PLCS_CONSTRUCTION,
    AI_PLCS_EXPLORATION,
    AI_PLCS_OFFENSE,
    AI_PLCS_DEFENSE,
    AI_PLCS_IMPROVEMENT,
    AI_PLCS_GATHERING,
    AI_PLCS_TOTAL
};

/* This is for when a unit needs a plan and asks its side for one. */
/*! 
    \todo Handle 'A_ANY_NO_MATERIAL' results.
    \note This function will eventually be supplanted by strategic evaluators 
	    which assign operational roles to units.
*/

static void
mplayer_decide_plan(Side *side, Unit *unit)
{
    using namespace Xconq;
    using namespace Xconq::AI;

    int u = unit->type;
    int u2 = NONUTYPE;
    Plan *plan = unit->plan;
    Goal *goal;
    int totchances = 0, chances[AI_PLCS_TOTAL], luckynum = 0;
    int unexploredpct = 0, enemiestot = 0, friendliestot = 0, cworth = 0;
    int rslt = A_ANY_OK;
    Theater *theater = NULL;
    Unit *occ = NULL;
    OpRole *oprole = NULL;

    if (!is_active(unit)) {
	net_set_unit_reserve(side, unit, TRUE, FALSE);
	return;
    }
    /* Initialize plan lottery slots. */
    memset(chances, 0, AI_PLCS_TOTAL * sizeof(int));
    /* If the plan still has tasks, let the tasks run out first. */
    if (plan->tasks)
      return;
    /* Honor any operational role that is in effect. */
    oprole = find_oprole(side, unit->id);
    if (oprole && (OR_NONE != oprole->type))
      return;
    /* If unit is mobile and has occs, examine them for move-to tasks. */
    if (unit->occupant && mobile(unit->type)) {
	for_all_occupants(unit, occ) {
	    if (!occ->plan)
	      continue;
	    if (!occ->plan->tasks)
	      continue;
	    if (occ->plan->tasks->type != TASK_MOVE_TO)
	      continue;
	    net_set_move_to_task(unit, occ->plan->tasks->args[0],
				 occ->plan->tasks->args[1], 
				 occ->plan->tasks->args[3]);
	    return;
	}
    }
    /*! \todo Should take care of theater reassignment here, 
		before setting up a new plan. */
    /* Decide what to do based on the existing plan. */
    switch (plan->type) {
      case PLAN_PASSIVE: case PLAN_NONE:
	/* Find out which theater we are in, so we can look at its stats. */
	theater = theater_at(unit->side, unit->x, unit->y);
	if (theater) {
	    /* How many enemies in our theater? */
	    for_all_unit_types(u2)
	      enemiestot += theater->numenemies[u2];
	    /* How many friendlies in out theater? */
	    friendliestot = theater->allied_units;
	    /* Explored percentage. */
	    unexploredpct = (100 * theater->unexplored) / theater->size;
	}
	else {
	    /* (TODO: Search around for enemies and count them.) */
	    enemiestot = 0;
	    /* (TODO: Search around for friendlies and count them.) */
	    friendliestot = 0;
	    /* (TODO: Search around for unexplored cells and count them.) */
	    unexploredpct = 0;
	}
	// Can the unit construct anything?
	cworth = 
	    u_colonizer_worth(u) + u_colonization_support_worth(u)
	    + u_exploration_support_worth(u) + u_defensive_support_worth(u)
	    + u_offensive_support_worth(u) + u_base_construction_worth(u);
	if (valid(rslt = can_construct_any(unit, unit))) 
	    totchances += (chances[AI_PLCS_CONSTRUCTION] += cworth);
	else if (A_ANY_NO_ACP == rslt) 
	    totchances += (chances[AI_PLCS_RESERVE] += cworth);
	/* If unit can explore, and explorers are needed. */
	if (need_explorers(side) && (0 < u_ai_explorer_worth(u))
	    /* && (enemiestot == 0) */) {
	    if (valid(rslt = can_explore(unit, unit)))
	      totchances += 
		(chances[AI_PLCS_EXPLORATION] += u_ai_explorer_worth(u));
	    else if (A_ANY_NO_ACP == rslt)
	      totchances +=
		(chances[AI_PLCS_RESERVE] += u_ai_explorer_worth(u));
	}
	/* If unit can damage another unit. */
	if (0 < u_offensive_worth(u)) { 
	    if (valid(rslt = can_destroy_any(unit, unit)) 
		&& valid(rslt = can_move(unit, unit)))
	      totchances += (chances[AI_PLCS_OFFENSE] += u_offensive_worth(u));
	    else if (A_ANY_NO_ACP == rslt)
	      totchances += (chances[AI_PLCS_RESERVE] += u_offensive_worth(u));
	}
	/* If unit can capture another unit. */
	if (0 < u_siege_worth(u)) {
	    if (valid(rslt = can_capture_any(unit, unit)))
	      totchances += (chances[AI_PLCS_OFFENSE] += u_siege_worth(u));
	    else if (A_ANY_NO_ACP == rslt)
	      totchances += (chances[AI_PLCS_RESERVE] += u_siege_worth(u));
	}
	/* If unit can defend against another unit.*/
	if ((0 < u_defensive_worth(u))
	    && !(unit->transport && mobile(unit->transport->type))) {
	    if (valid(rslt = can_defend_against_any(unit, unit)))
	      totchances += (chances[AI_PLCS_DEFENSE] += u_defensive_worth(u));
	    else if (A_ANY_NO_ACP == rslt)
	      totchances += (chances[AI_PLCS_RESERVE] += u_defensive_worth(u));
	}
	/* If unit could change its utype. */
	/* (TODO: Figure out some change-type worth.) */
	if (valid(rslt = can_change_type(unit)))
	  totchances += (chances[AI_PLCS_IMPROVEMENT] += 5000);
	else if (A_ANY_NO_ACP == rslt)
	  totchances += (chances[AI_PLCS_RESERVE] += 5000);
	/* If unit could build units to enhance itself. */
	for_all_unit_types(u2) {
	    if (!could_be_occupant_of(u2, u))
	      continue;
	    if (0 < u_ai_prod_enhancer_worth(u2)) {
		if (valid(rslt = can_construct(unit, unit, u2)))
		  totchances += 
		    (chances[AI_PLCS_IMPROVEMENT] += 
			u_ai_prod_enhancer_worth(u2));
		else if (A_ANY_NO_ACP == rslt)
		  totchances +=
		    (chances[AI_PLCS_RESERVE] += 
			u_ai_prod_enhancer_worth(u2));
	    }
	    /* (TODO: Consider other kinds of enhancers.) */
	}
	/* Maybe assign to collecting materials. */
	if ((goal = has_unsatisfied_goal(side, GOAL_HAS_MATERIAL_TYPE))
	    && need_this_type_to_collect(side, u, goal->args[0])) 
	  totchances += (chances[AI_PLCS_GATHERING] += 5000);
	/* Run the lottery to choose a plan. */
	luckynum = xrandom(totchances);
	totchances = 0;
	if (luckynum < (totchances += chances[AI_PLCS_RESERVE]))
	  net_set_unit_reserve(side, unit, TRUE, FALSE);
	else if (luckynum < (totchances += chances[AI_PLCS_CONSTRUCTION]))
	  acquire_oprole(side, unit->id, OR_CONSTRUCTOR);
	else if (luckynum < (totchances += chances[AI_PLCS_EXPLORATION]))
	  assign_to_exploration(side, unit);
	else if (luckynum < (totchances += chances[AI_PLCS_OFFENSE]))
	  assign_to_offense(side, unit);
	else if (luckynum < (totchances += chances[AI_PLCS_DEFENSE]))
	  assign_to_defense(side, unit);
	else if (luckynum <
		    (totchances += chances[AI_PLCS_IMPROVEMENT]))
	  assign_to_improve(side, unit);
	else if (luckynum <
		    (totchances += chances[AI_PLCS_GATHERING])) {
	    goal = has_unsatisfied_goal(side, GOAL_HAS_MATERIAL_TYPE);
	    assign_to_collection(side, unit, goal->args[0]);
	}
	else {
	    net_set_unit_reserve(side, unit, TRUE, FALSE);
	    DMprintf("mplayer_decide_plan failed for %s.\n", unit_desig(unit));
	}
	break;
      /* Leave other plans alone. */
      default:
	break;
    }
}

static int
mplayer_adjust_plan(Side *side, Unit *unit)
{
    using namespace Xconq;
    using namespace Xconq::AI;

    OpRole *oprole = NULL;

    /* Honor any operational role that is in effect. */
    oprole = find_oprole(side, unit->id);
    if (oprole && (OR_NONE != oprole->type))
      return TRUE;
    /* If a collecting unit achieves its goal, cancel its plan. */
    if (unit->plan->type == PLAN_IMPROVING
	&& mobile(unit->type)
	&& unit->plan->aicontrol
	&& !unit->plan->asleep
	&& unit->plan->tasks == NULL) {
	DMprintf("Forced replan: %s done collecting.\n", unit_desig(unit));
	net_force_replan(unit);
	return TRUE;
    }
    if (unit->plan->waitingfortasks && unit->plan->aicontrol) {
	DMprintf("Forced replan: %s under ai-control & waiting for tasks.\n", 
		 unit_desig(unit));
	net_force_replan(unit);
	return TRUE;
    }
    if (!unit->plan->reserve
	&& unit->plan->execs_this_turn > 
	    (10 * max(1, type_max_acp(unit->type)))) {
	DMprintf("Forced reserve: %s did more than 10 plan execs this turn.\n", 
		 unit_desig(unit));
	net_set_unit_reserve(side, unit, TRUE, FALSE);
	return TRUE;
    }
    /* Look at more units. */
    return TRUE;
}

/* This is a hook that runs after each task is executed. */

static void
mplayer_react_to_task_result(
    Side *side, Unit *unit, Task *task, TaskOutcome rslt)
{
    int dx, dy, x1, y1, fact;
    Unit *occ;
    Theater *theater;

    /* React to an apparent blockage. */
    if (rslt == TASK_FAILED
	&& task != NULL
	&& task->type == TASK_MOVE_TO
	&& task->retrynum > 2) {
	if (desired_direction_impassable(unit, task->args[0], task->args[1])) {
	    if (could_be_ferried(unit, task->args[0], task->args[1])) {
		if (unit->plan->type == PLAN_EXPLORATORY) {
		    DMprintf("%s blocked while exploring, ", unit_desig(unit));
	      	    if (flip_coin()) {
			DMprintf("changing theaters\n");
			change_to_adjacent_theater(side, unit);
		    }
		    else if (probability(75)) {
			DMprintf("%s blocked, will wait for transport\n",
				 unit_desig(unit));
			theater = theater_at(side, unit->x, unit->y);
			if (theater != NULL) {
			    ++(theater->numtotransport[unit->type]);
			}
			net_set_unit_reserve(side, unit, TRUE, FALSE);
			net_set_unit_waiting_for_transport(side, unit, TRUE);
		    }
		    else {
			DMprintf("changing goal within theater\n");
			/* Clear the existing goal and create a new one. */
			assign_explorer_to_theater(side, unit,
						   unit_theater(unit));
		    }
		    return;
		} else if (probability(75)) {
		    DMprintf("%s blocked, will wait for transport\n",
			     unit_desig(unit));
		    theater = theater_at(side, unit->x, unit->y);
		    if (theater != NULL) {
			++(theater->numtotransport[unit->type]);
		    }
		    net_set_unit_reserve(side, unit, TRUE, FALSE);
		    net_set_unit_waiting_for_transport(side, unit, TRUE);
		    return;
		}
	    } else {
	    	if (unit->occupant) {
		    DMprintf(
"%s blocked while transporting, will sit briefly\n",
			     unit_desig(unit));
		    net_set_unit_reserve(side, unit, TRUE, FALSE);
		    for_all_occupants(unit, occ) {
		    	net_wake_unit(side, occ, FALSE);
		    }
		    return;
	    	}
		/* Another option is to transfer to another theater.
		   This is especially useful when exploring. */
		if (unit->plan->type == PLAN_EXPLORATORY && flip_coin()) {
		    DMprintf("%s blocked while exploring, changing theaters\n",
			     unit_desig(unit));
		    change_to_adjacent_theater(side, unit);
		    return;
		}
	    }
	    /* Try moving sideways. */
	    if (probability(80)) {
		dx = task->args[0] - unit->x;  dy = task->args[1] - unit->y;
		fact = (flip_coin() ? 50 : -50);
		x1 = unit->x - ((fact * dy) / 100);
		y1 = unit->y + ((fact * dx) / 100);
		if (inside_area(x1, y1)) {
		    DMprintf("%s blocked, moving sideways to (%d, %d).", 
			     unit_desig(unit), x1, y1);
		    net_push_move_to_task(unit, x1, y1, 1);
	    	}
	    }
	    return;
	} else if (blocked_by_enemy(unit, task->args[0], task->args[1], TRUE)) {
	    /* (should decide if allowable risk to passengers) */
	    DMprintf("%s blocked by enemy, unable to move to (%d, %d).",
		     unit_desig(unit), task->args[0], task->args[1]);
	    if (!attack_blockage(side, unit, task->args[0], task->args[1], 
				 TRUE)) {
		if (blocked_by_enemy(unit, task->args[0], task->args[1], 
				     FALSE)) {
		    attack_blockage(side, unit, task->args[0], task->args[1], 
				    FALSE);
		} else {
		    /* (should move sideways?) */
		}
	    }
	} else {
	    /* what to do about other failures? */
	}
	return;
    }
    /* React to inability to resupply by trying to build a base. */
    if (rslt == TASK_FAILED
	&& task != NULL
	&& task->type == TASK_RESUPPLY
	&& task->retrynum > 2) {
    	net_set_unit_reserve(side, unit, FALSE, FALSE);
    	build_depot_for_self(unit);
    }
}

/* Reassign the given unit to another theater, usually because it is
   unable to fulfill its goal in its current theater. */

static void
change_to_adjacent_theater(Side *side, Unit *unit)
{
    int dir;
    Theater *theater, *newtheater = NULL;

    theater = unit_theater(unit);
    if (theater != NULL) {
   	for_all_directions(dir) {
	    /* If we have a radial pattern of theaters, randomly
	       choose between inward/outward or right/left. */
   	    if (theater == ai(side)->perimeters[dir]) {
		if (probability(20) && ai(side)->midranges[dir] != NULL)
		  newtheater = ai(side)->midranges[dir];
		else
   	    	  newtheater = ai(side)->perimeters[flip_coin() ? left_dir(dir) : right_dir(dir)];
   	    	break;
   	    }
   	    if (theater == ai(side)->midranges[dir]) {
		if (probability(20) && ai(side)->perimeters[dir] != NULL)
		  newtheater = ai(side)->perimeters[dir];
		else if (probability(20) && ai(side)->remotes[dir] != NULL)
		  newtheater = ai(side)->remotes[dir];
		else
   	    	  newtheater = ai(side)->midranges[flip_coin() ? left_dir(dir) : right_dir(dir)];
   	    	break;
   	    }
   	    if (theater == ai(side)->remotes[dir]) {
 		if (probability(20) && ai(side)->midranges[dir] != NULL)
		  newtheater = ai(side)->midranges[dir];
		else
   	    	  newtheater = ai(side)->remotes[flip_coin() ? left_dir(dir) : right_dir(dir)];
   	    	break;
   	    }
   	}
   	/* (should add grid case also?) */
   	if (newtheater != NULL) {
	    assign_explorer_to_theater(side, unit, newtheater);
   	}
    }
}

/* This function is called whenever a new side appears in the game.  It
   mainly needs to make that any allocated data is resized appropriately. */

static void
mplayer_react_to_new_side(Side *side, Side *side2)
{
    /* (Assumes we call this right after adding each new side) */
    int oldnumsides = numsides - 1;
    int *newpeople, s;
    Theater *theater;

    for_all_theaters(side, theater) {
	/* Grow any people count arrays if present. */
	if (theater->people != NULL) {
	    newpeople = (int *) xmalloc ((numsides + 1) * sizeof(int));
	    for (s = 0; s <= oldnumsides; ++s)
	      newpeople[s] = theater->people[s];
	    free(theater->people);
	    theater->people = newpeople;
	}
    }
    anewside = side2;
    estimate_strengths(side);
    anewside = NULL;
}

/* At the end of a turn, re-evaluate the plans of some units in case
   the situation changed. */

static void
mplayer_finish_movement(Side *side)
{
    int u, scan;
    Unit *unit;
    Theater *theater;

    scan = FALSE;
    for_all_theaters(side, theater) {
	for_all_unit_types(u) {
	    if (theater->numtotransport[u] > 0) {
		scan = TRUE;
		break;
	    }
	}
	if (scan)
	  break;
    }
    if (scan) {
	/* Find a unit needing transport. */
	for_all_side_units(side, unit) {
	    if (is_active(unit)
		&& ai_controlled(unit)
		&& unit->plan
		&& unit->plan->waitingfortransport) {
		search_for_available_transport(unit, 0);
		/* Whether or not the search succeeded, stop waiting
		   to see if we get transportation. */
		net_set_unit_waiting_for_transport(side, unit, FALSE);
	    }
	}
    }
    for_all_side_units(side, unit) {
	if (is_active(unit) && ai_controlled(unit)) {
	    /* Content of old mplayer_rethink_plan. */
	    Task *toptask = unit->plan->tasks, *nexttask = NULL;
	    Plan *plan = unit->plan;
	    Unit *transport;
	    int dist;

	    if (toptask)
	      nexttask = toptask->next;
	    /* If we have a long ways to go, 
		see if there is a transport available 
		that can get us there faster.  */
	    if (toptask != NULL
		&& (toptask->type == TASK_HIT_UNIT
		    || (toptask->type == TASK_MOVE_TO
#if (0)
			&& nexttask != NULL
			&& nexttask->type == TASK_HIT_UNIT))
#else
		       ))
#endif
	        && !plan->reserve
	        && !plan->asleep
	        && !plan->waitingfortransport
	        && (unit->transport == NULL || !mobile(unit->transport->type))
	        && ((dist = distance(unit->x, unit->y,
				     toptask->args[0], toptask->args[1]))
	            >= 4 * type_max_acp(unit->type))
	        && accelerable(unit->type)
	        ) {
	        DMprintf("%s looking for transport to accelerate with;\n", 
			 unit_desig(unit));
	        transport = search_for_available_transport(unit, 1);
	        if (transport != NULL) {
		    net_push_sentry_task(unit, max(1, dist / max(1, 
		      type_max_acp(transport->type))));
		    net_set_unit_waiting_for_transport(unit->side, unit, FALSE);
	        } else {
		    DMprintf("  found nothing\n");
	        }
	    }
	}
    }
}

/* Given a unit and a reason to be needing transport, look around for
   something suitable.  We need a transport that has available space
   and is not doing something else more important. */
/* (should also choose transports that can reach the unit's
   destination or at least close by) */

static Unit *
search_for_available_transport(Unit *unit, int purpose)
{
    int dist, closestdist = area.maxdim, tdestx = -1, tdesty = -1;
    Unit *transport, *closesttransport = NULL;
    Theater *theater = unit_theater(unit);

    /* If we are already in a mobile transport, then it is closest. */
    if (unit->transport && mobile(unit->transport->type))
      return unit->transport;
    /* (more efficient to search adjacent cells first?) */
    for_all_side_units(unit->side, transport) {
	if (is_active(transport)
	    && mobile(transport->type)
	    && could_carry(transport->type, unit->type)
	    && can_occupy(unit, transport)
	    && transport->act != NULL /* not quite correct, but to fix bug */
	    && (purpose == 1 ? accelerator(transport->type, unit->type) : TRUE)
	    ) {
	    /* Don't grab at units being moved manually. */
	    if (!ai_controlled(transport))
	      continue;
	    /* Don't mess with units that are doing resupply or repair ... */
	    if (transport->plan
		&& transport->plan->tasks
		&& (transport->plan->tasks->type == TASK_REPAIR
		     || transport->plan->tasks->type == TASK_RESUPPLY
		     /* ... or are on their way to resupply or repair. */
		     || (transport->plan->tasks->type == TASK_MOVE_TO
			&& transport->plan->tasks->next
			&& (transport->plan->tasks->next->type == TASK_REPAIR
			     || transport->plan->tasks->next->type == 
				TASK_RESUPPLY)))) {
		continue;
	    }
	    /* Maybe this one is already coming to get somebody. */
	    if (transport->plan
		&& transport->plan->tasks != NULL
		&& transport->plan->tasks->type == TASK_PICKUP) {
		if (transport->plan->tasks->args[0] == unit->id)
		  return transport;
		/* Picking up somebody else - don't hijack. */
		continue;
	    }
	    if (transport->plan
		&& transport->plan->tasks != NULL
		&& transport->plan->tasks->type == TASK_MOVE_TO
		&& transport->plan->tasks->next != NULL
		&& transport->plan->tasks->next->type == TASK_PICKUP) {
		if (transport->plan->tasks->next->args[0] == unit->id)
		  return transport;
		/* Picking up somebody else - don't hijack. */
		continue;
	    }
	    /* If transport appears to be on a delivery run, 
		and is redshifted, then leave it alone. */
	    if (transport->plan && transport->plan->tasks
		&& transport->occupant
		&& (transport->plan->tasks->type == TASK_MOVE_TO)) {
		tdestx = transport->plan->tasks->args[0];
		tdesty = transport->plan->tasks->args[1];
		if (distance(transport->x, transport->y, tdestx, tdesty) <=
		    distance(unit->x, unit->y, tdestx, tdesty))
		  continue;
	    }
	    /* If transport is full, then leave it alone. */
	    if (transport->occupant 
		&& !type_can_occupy(unit->type, transport))
	      continue;
	    dist = distance(unit->x, unit->y, transport->x, transport->y);
	    if (dist < closestdist || (dist == closestdist && flip_coin())) {
		closesttransport = transport;
		closestdist = dist;
	    }
	    /* If transport already adjacent, no need to keep looking. */
	    if (closestdist <= 1)
	      break;
	}
    }
    if (closesttransport != NULL && closesttransport->plan != NULL) {
	if (unit->plan)
	  net_set_unit_plan_type(unit->side, closesttransport,
				 unit->plan->type);
	net_set_unit_main_goal(unit->side, closesttransport, NULL);
	net_push_pickup_task(closesttransport, unit);
	net_push_move_to_task(closesttransport, unit->x, unit->y, 1);
	net_push_occupy_task(unit, closesttransport);
	/* No longer count this unit as needing transport. */
	if (theater != NULL) {
	    --(theater->numtotransport[unit->type]);
	    set_unit_theater(closesttransport, theater);
	}
	DMprintf("%s will be picked up by closest transport %s\n",
	         unit_desig(unit), unit_desig(closesttransport));
	return closesttransport;
    }
    return NULL;
}

/* This is used by interfaces to display the name of the theater in
   use at a given point. */

static char *
mplayer_at_desig(Side *side, int x, int y)
{
    Theater *theater;

    if (ai(side) == NULL)
      return "";
    theater = theater_at(side, x, y);
    return (theater ? theater->name : (char *)"<no theater>");
}

/* This is used by interfaces to display boundaries between theaters,
   by comparing the numbers returned. */

static int
mplayer_theater_at(Side *side, int x, int y)
{
    Theater *theater;

    if (ai(side) == NULL)
      return 0;
    theater = theater_at(side, x, y);
    return (theater ? theater->id : 0);
}

/* Collect initial strength information stored in the mplayer's
   private saved data. */

static int
mplayer_read_strengths(Side *side)
{
    int sn1, u, found = FALSE;
    char *propname;
    Obj *props, *bdg, *rest, *sidebdg, *urest;
    Side *side1;
    Strategy *strategy = ai(side);

    props = find_at_key(side->aidata, "mplayer");
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	propname = c_string(car(bdg));
	if (strcmp(propname, "strengths0") == 0) {
	    found = TRUE;
	    rest = cdr(bdg);
	    for_all_sides(side1) {
		sn1 = side1->id;
		sidebdg = car(rest);
		urest = cadr(sidebdg);
		for_all_unit_types(u) {
		    (strategy->strengths0[sn1])[u] = c_number(car(urest));
		    urest = cdr(urest);
		}
		rest = cdr(rest);
	    }
	} else if (strcmp(propname, "alstrengths0") == 0) {
	    found = TRUE;
	    rest = cdr(bdg);
	    for_all_sides(side1) {
		sn1 = side1->id;
		sidebdg = car(rest);
		urest = cadr(sidebdg);
		for_all_unit_types(u) {
		    (strategy->alstrengths0[sn1])[u] = c_number(car(urest));
		    urest = cdr(urest);
		}
		rest = cdr(rest);
	    }
	} else if (strcmp(propname, "points0") == 0) {
	    found = TRUE;
	    rest = cdr(bdg);
	    for_all_sides(side1) {
		sn1 = side1->id;
		strategy->points0[sn1] = c_number(car(rest));
		rest = cdr(rest);
	    }
	} else if (strcmp(propname, "alpoints0") == 0) {
	    found = TRUE;
	    rest = cdr(bdg);
	    for_all_sides(side1) {
		sn1 = side1->id;
		strategy->alpoints0[sn1] = c_number(car(rest));
		rest = cdr(rest);
	    }
	} else {
	}
    }
    return found;
}

/* Write out any state that the mplayer must preserve.  We don't
   actually write; instead we build a Lisp object and pass that back
   to the writing routines. */

static Obj *
mplayer_save_state(Side *side)
{
    int sn1, u;
    Obj *rslt, *vallist, *uvallist;
    Side *side1;
    Strategy *strategy = ai(side);

    rslt = lispnil;
    /* Just return last result if it's already been computed. */
    if (strategy->writable_state != lispnil || xmalloc_warnings)
      return strategy->writable_state;
    /* We're pushing bindings onto a list, so do in reverse of desired
       order. */
    vallist = lispnil;
    for_all_sides(side1) {
	sn1 = side1->id;
	uvallist = lispnil;
	for_all_unit_types(u) {
	    uvallist = cons(new_number((strategy->alstrengths0[sn1])[u]),
			    uvallist);
	}
	uvallist = reverse(uvallist);
	push_binding(&vallist, new_number(sn1), uvallist);
    }
    vallist = reverse(vallist);
    push_cdr_binding(&rslt, intern_symbol("alstrengths0"), vallist);
    vallist = lispnil;
    for_all_sides(side1) {
	sn1 = side1->id;
	uvallist = lispnil;
	for_all_unit_types(u) {
	    uvallist = cons(new_number((strategy->strengths0[sn1])[u]),
			    uvallist);
	}
	uvallist = reverse(uvallist);
	push_binding(&vallist, new_number(sn1), uvallist);
    }
    vallist = reverse(vallist);
    push_cdr_binding(&rslt, intern_symbol("strengths0"), vallist);
    vallist = lispnil;
    for_all_sides(side1) {
	sn1 = side1->id;
	vallist = cons(new_number(strategy->alpoints0[sn1]), vallist);
    }
    vallist = reverse(vallist);
    push_cdr_binding(&rslt, intern_symbol("alpoints0"), vallist);
    vallist = lispnil;
    for_all_sides(side1) {
	sn1 = side1->id;
	vallist = cons(new_number(strategy->points0[sn1]), vallist);
    }
    vallist = reverse(vallist);
    push_cdr_binding(&rslt, intern_symbol("points0"), vallist);
    strategy->writable_state = rslt;
    return rslt;
}

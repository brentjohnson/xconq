/* Definitions common to all AIs.
   Copyright (C) 1992-1997, 1999-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file ai.h 
 * Declaration of AI structs, functions, and enums.
 */

/*! Game class */
/*! This enum defines the type of game for the AI
 */
typedef enum a_game_class {
    gc_none,                	/*!< No game class defined */
    gc_standard,            	/*!< Standard game class. */
    gc_time,                	/*!< Time-like game class. */
    gc_advanced		/*!< Advanced (game with advances?) game class */
} GameClass;

/*! AI operation? */
typedef struct a_ai_op {
    char *name;
    char *help;
    int (*to_test_compat)(void);
    void (*to_init)(Side *side);
    void (*to_init_turn)(Side *side);
    void (*to_decide_plan)(Side *side, Unit *unit);
    void (*to_react_to_task_result)(Side *side, Unit *unit,
				    Task *task, TaskOutcome rslt);
    void (*to_react_to_new_side)(Side *side, Side *side2);
    int (*to_adjust_plan)(Side *side, Unit *unit);
    void (*to_finish_movement)(Side *side);
    Obj *(*to_save_state)(Side *side);
    int (*region_at)(Side *side, int x, int y);
    char *(*at_desig)(Side *side, int x, int y);
} AI_ops;

/* Definition common to all ai types. (?) */

typedef struct a_ai {
  int dummy;
} AI;

#define side_ai_type(s) ((s)->aitype)

/* Limit on the number of goals that a side may have. */

#define MAXGOALS 30

/* Limit on the number of theaters a single side may have. */

#define MAXTHEATERS 98

/* Strategy is what a side uses to make decisions. */

typedef struct a_strategy {
    int trytowin;
    int report_not_understood;
    int creationdate;
    short *strengths[MAXSIDES + 1];	/* estimated numbers of units */
    short points[MAXSIDES + 1];		/* estimated point value */
    short *alstrengths[MAXSIDES + 1];  	/* numbers in alliances */
    short alpoints[MAXSIDES + 1];	/* points in alliances */
    short initial_strengths_computed;
    short *strengths0[MAXSIDES + 1];  	/* initial estimated numbers of units */
    short points0[MAXSIDES + 1];	/* initial estimated point value */
    short *alstrengths0[MAXSIDES + 1]; 	/* initial numbers in alliances */
    short alpoints0[MAXSIDES + 1];	/* initial points in alliances */
    short contacted[MAXSIDES+1];
    short homefound[MAXSIDES+1];
    int analyzegame;
    struct a_theater *theaters;
    struct a_theater **theatertable;
    short numtheaters;
    char *areatheaters;
    struct a_theater *homefront;
    struct a_theater *perimeters[NUMDIRS];
    struct a_theater *midranges[NUMDIRS];
    struct a_theater *remotes[NUMDIRS];
    int numgoals;
    struct a_goal *goals[MAXGOALS];
    /* Exploration and search slots. */
    int zonewidth, zoneheight;
    int numzonex, numzoney;    	/* dimensions of search zone array */
    int numzones;
    struct a_searchzone *searchzones;
    short *explorertypes;
    short explorersneeded;
    short *terrainguess;
    short cx, cy;              	/* "centroid" of all our units */
    short *demand;             	/* worth of each utype w.r.t. strategy */
    int explore_priority;
    int defend_priority;
    int attack_priority;
    struct a_unit **unitlist;  	/* lists to help mplay efficiency */
    short *unitlistcount;  	/* counts of above lists */
    short *actualmix;
    short *expectedmix;
    short *idealmix;
    short *develop_status;  	/* specific to the "time" game */
    short *develop_on;      	/* specific to the "time" game */
    Obj *writable_state;
} Strategy;

/* A Theater is a sub-area that can be planned for all at once. */

/* To save space in theater layer, no more than 127 theaters may exist
   at once.  This should be sufficient, even a Napoleon would have
   trouble keeping track of that much activity. */

typedef struct a_theater {
    short id;
    char *name;			/* an informative name for this theater */
    short x, y;			/* center of the theater */
    short xmin, ymin;
    short xmax, ymax;
    int size;			/* number of cells in the theater */
    short importance;		/* 0 = shrug, 100 = critical */
    Goal *maingoal;
    short allied_units;		/* How many units on our side here. */
    short makers;		/* Total number of makers */
    short unexplored;		/* number of unseen cells in theater */
    short allied_bases;		/* total number of our bases, includes towns */
    short border;		/* true if this is a border theater. */
    short reinforce;		/* priority on request for units. */
    short *numassigned;		/* num of each type assigned to theater */
    short *numneeded;		/* units we should move to theater. */
    short *numtotransport;	/* types needing transportation. */
    short *numenemies;
    short *numsuspected;
    short *numsuspectedmax;
    int *people;		/* number of populated cells seen */
    int enemystrengthmin;	/* estimate of enemy unit strength */
    int enemystrengthmax;	/* estimate of enemy unit strength */
    short units_lost;		/* How many units have we lost here. */
    struct a_theater *next;	/* pointer to the next theater */
} Theater;

#define ai(s) ((Strategy *) (s)->ai)

#define strength_est(side,side2,u) ((ai(side)->strengths[side2->id])[u])
#define allied_strength_est(side,side2,u) ((ai(side)->alstrengths[side2->id])[u])
#define strength0_est(side,side2,u) ((ai(side)->strengths0[side2->id])[u])
#define allied_strength0_est(side,side2,u) ((ai(side)->alstrengths0[side2->id])[u])
  
#define for_all_theaters(s,th) \
  	for ((th) = ai(s)->theaters; (th) != NULL; (th) = (th)->next) \

#define theater_at(s,x,y)  \
  (ai(s)->theaters ? ai(s)->theatertable[((int) ai(s)->areatheaters[(x)+area.width*(y)])] : NULL)

#define set_theater_at(s,x,y,th)  \
  ((ai(s)->areatheaters[(x)+area.width*(y)]) = (th)->id)

#define for_all_cells_in_theater(s,x,y,th) { \
  for ((x) = theater->xmin; (x) < theater->xmax; ++(x))  \
    for ((y) = theater->ymin; (y) < theater->ymax; ++(y))  \
      if (theater_at((s), (x), (y)) == (th))  }

#define unit_theater(unit) ((Theater *) (unit)->aihook)

#define set_unit_theater(unit,theater) ((unit)->aihook = (char *) (theater))

/* utype-specific develop status codes for the "time" game */

#define RS_DEVELOP_NEEDED 4
#define RS_DEVELOP_ASSIGNED 3
#define RS_UPGRADE_NEEDED 1

struct weightelt {
    int weight;
    long data;
};

extern GameClass game_class;
extern int bhw_max;

/* Common functions shared between ai.c and specific AIs. */

extern void try_to_draw(Side *side, int flag, char *ainame);
extern void give_up(Side *side, char *ainame);
extern int goal_truth(Side *side, Goal *goal);
extern int accelerable(int u);
extern int accelerator(int u1, int u2);
extern GameClass find_game_class(void);
extern void update_unit_plans(Side *side);
extern void update_unit_plans_randomly(Side *side);
extern int need_this_type_to_collect(Side *side, int u, int m);
extern void assign_to_collection(Side *side, Unit *unit, int m);
extern void assign_to_improve(Side *side, Unit *unit);
extern int assign_to_colonize(Side *side, Unit *unit);
extern void assign_to_defend_unit(Unit *unit, Unit *unit2);
extern void assign_to_exploration(Side *side, Unit *unit);
extern void assign_explorer_to_theater(
    Side *side, Unit *unit, Theater *theater);
extern void assign_to_explorer_construction(Side *side, Unit *unit);
extern void assign_to_offense(Side *side, Unit *unit);
extern void assign_to_offense_support(Side *side, Unit *unit);
extern void assign_to_colonization_support(Side *side, Unit *unit);
extern void assign_to_defense(Side *side, Unit *unit);
extern void assign_to_defense_support(Side *side, Unit *unit);
extern int preferred_build_type(Side *side, Unit *unit, int plantype);
extern int can_develop_on(int u, int u2);
extern int needs_develop(Side *side, int u);
extern int assign_to_develop_on(Side *side, Unit *unit, int u2);
extern int build_depot_for_self(Unit *unit);
extern int desired_direction_impassable(Unit *unit, int x, int y);
extern int could_be_ferried(Unit *unit, int x, int y);
extern int blocked_by_enemy(Unit *unit, int x, int y, int shortest);
extern int attack_blockage(Side *side, Unit *unit, int x, int y, int shortest);
extern int enemy_close_by(Side *side, Unit *unit, int dist, int *xp, int *yp);
extern int suitable_port(Unit *unit);
extern int need_more_transportation(Side *side);
extern int need_explorers(Side *side);
extern void add_goal(Side *side, Goal *goal);
extern Goal *has_goal(Side *side, GoalType goaltype);
extern Goal *has_unsatisfied_goal(Side *side, GoalType goaltype);

#if 0		/* Unused functions. */

extern void assign_to_defend_cell(Unit *unit, int x, int y);
extern void assign_to_defend_vicinity(Unit *unit, int x, int y, int w, int h);
extern void assign_to_defend_theater(Unit *unit, Theater *theater);
extern int build_base_for_self(Side *side, Unit *unit);
extern int build_base_for_others(Side *side, Unit *unit);

#endif

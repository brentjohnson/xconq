/* Definitions relating to units in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/unit.h
 * \brief Definitions relating to units in Xconq.
 *
 * Prototypes, constants, and macros relating to units.
 */

#include "parambox.h"

/* Advanced unit support. */

/*! \brief Advanced \Unit: default size. */
extern short default_size; 
/*! \brief Advanced \Unit: default used cells. */
extern short default_usedcells; 
/*! \brief Advanced \Unit: default max cells. */
extern short default_maxcells; 
/*! \brief Advanced \Unit: default population. */
extern long default_population; 

/* Forward declarations of various structs. */
struct a_unit;
struct a_unit_extras;
struct a_unit_view;

/*! \brief Unit.
 *
 * The Unit structure should be small, because there may be many of them.
 * Unit semantics go in this structure, while unit brains go into the
 * act/plan.  Test: a unit that is like a rock and can't do anything at all
 * just needs basic slots, plan needn't be allocated.  Another test:
 * unit should still function correctly after its current plan has been
 * destroyed and replae`d with another.
 * \note unit->side and unit->origside must always point to
 * valid side objects; dereferences are not checked!
 */
typedef struct a_unit {
    short type;                		/*!< type */
    int id;            			/*!< truly unique id number */
    const char *name;          		/*!< the name, if given */
    int number;                		/*!< semi-unique number */
    struct a_unit_view *uview;		/*!< master view of unit */
    struct a_image_family *imf;       	/*!< the image family */
    const char *image_name;		/*!< the name of the image, if given */
    short x;   				/*!< x position of unit in world */
    short y;   				/*!< y position of unit in world */
    short z;   				/*!< z position of unit in world */
    struct a_side *side;       	     	/*!< whose side this unit is on */
    struct a_side *origside;   	     	/*!< the first side this unit was on */
    short hp;          		/*!< how much more damage each part can take */
    short hp2;              	/*!< buffer for next value of hp */
    short cp;	               	/*!< state of construction */
    short cxp;                 	/*!< combat experience */
    short morale;              	/*!< morale */
    struct a_unit *transport;  	/*!< pointer to transporting unit if any */
    SideMask tracking;         	/*!< which sides always see us (bit vector) */
    long *supply;             	/*!< how much supply we're carrying */
    short s_flow;              	/*!< how much supply we received this turn */
    short s_conn;       /*!< how well connected we are to supply zones */
    short *tooling;     /*!< level of preparation for construction */
    short *opinions;    /*!< opinion of each side, both own and others */
    struct a_actorstate *act;  		/*!< the unit's current actor state */
    struct a_plan *plan;       	 	/*!< the unit's current plan */
    struct a_unit_extras *extras;	/*!< pointer to optional stuff */
    char *aihook;          	/*!< used by AI to keep info about this unit */
    /* Following slots are never saved. */
    struct a_unit *occupant; 	/*!< pointer to first unit being carried */
    struct a_unit *nexthere;   	/*!< pointer to fellow occupant */
    struct a_unit *prev;       	/*!< previous unit in list of side's units */
    struct a_unit *next;        /*!< next unit in list of side's units */
    struct a_unit *unext;      	/*!< next unit in list of all units */
    short prevx;               	/*!< x co-odinate of where were we last */
    short prevy;               	/*!< y co-ordinate of where were we last */
    Obj *transport_id;         	/*!< read-in id of transport */
                                /* (transport_id should be tmp array instead) */
    short flags;            	/*!< assorted flags */
    /* (should make optional in UnitExtras?) */
    short size;                	/*!< Abstract size of the unit */
    short reach;               	/*!< Size of the unit's zone of influence */
    short usedcells;           	/*!< Number of cells actually used by unit */
    short maxcells;            	/*!< Max number of cells that unit can use */
    short curadvance;          	/*!< Advance currently being researched. */ 
    long population;           	/*!< Size of units population */
    short *production;         	/*!< Cache of units last production. */
    short cp_stash;        /*!< CP's recovered from incomplete build tasks. */
    short researchdone;    /*!< We are done with research for this turn. */
    short busy;     /*!< Broadcasted action that has not yet executed. 
                     * \note this is NOT the same as has_pending_action(unit),
                     * which tells us that an action has been broadcasted and
                     * then confirmed by the host. The busy flag is set locally
                     * client-side long before that, when the action is first
                     * broadcasted. Its purpose is to filter out double-clicks
             	     * which may cause commands to be issued to a dead unit. */ 
    int creation_id; 		/*!< Id of unit's unbuilt creation. */
} Unit;

/*! \brief Unit exta options.
 *
 * The unit extras structure stores properties that are (usually)
 * uncommon or special-purpose.  It is allocated as needed, access is
 * through macros, and this does not appear as a separate structure
 * when saving/restoring.  If a value is likely to be filled in for
 * most or all units in several different game designs, then it should
 * be in the main structure (even at the cost of extra space), rather
 * than here. */
typedef struct a_unit_extras {
    short point_value;      	/*!< individual point value for this unit */
    short appear;           	/*!< turn of appearance in game */
    short appear_var_x;     	/*!< variation around appearance location */
    short appear_var_y;     	/*!< variation around appearance location */
    short disappear;        	/*!< turn of disappearance from game */
    short priority;         	/*!< individual priority */
    Obj *sym;               	/*!< symbol for xrefs */
    Obj *sides;             	/*!< list of possible sides */
} UnitExtras;

/*! \brief Unit View.
 *
 * The \UnitView is a partial mirror of an actual \Unit, including only
 * the data that might be known to another side seeing the \Unit from a
 * distance. */
typedef struct a_unit_view {
    short observer;		/*!< number of observing side */
    short type;        		/*!< type */
    short siden;       		/*!< side number of unit */
    const char *name;      	/*!< the name, if given */
    struct a_image_family *imf;	/*!< the image family */
    const char *image_name;	/*!< the name of the image, if given */
    short size;                	/*!< abstract size */
    short x;                   	/*!< x location of view */
    short y;                   	/*!< y location of view */
    short complete;	/*!< true if the underlying \Unit was completed */
    short date;        	/*!< turn on which last updated */
    int id;            	/*!< id of actual \Unit if known */
    struct a_unit *unit;	    /*!< pointer to actual \Unit, if id valid */
    struct a_unit_view *transport;  /*!< pointer to transport's \UnitView */
    struct a_unit_view *occupant;   /*!< pointer to 1st occupant's \UnitView */
    struct a_unit_view *nexthere;   /*!< pointer to next \UnitView 
					 at this level if any */
    struct a_unit_view *nextinhash; /*!< pointer to next \UnitView in the hash 
					 table */
    struct a_unit_view *vnext;	    /*!< pointer to next \UnitView in the 
					 global list */
} UnitView;

/* Unit parameter box. */
struct ParamBoxUnit : public ParamBox {
    Unit *unit;
    ParamBoxUnit() { pboxtype = PBOX_TYPE_UNIT; unit = NULL; }
};

/* "Unit at" parameter box. */
struct ParamBoxUnitAt : public ParamBoxUnit {
    int x, y;
    ParamBoxUnitAt() : ParamBoxUnit() {
	pboxtype = PBOX_TYPE_UNIT_AT;
	x = -1;  y = -1;
    }
};

/* Unit-unit parameter box. */
struct ParamBoxUnitUnit : public ParamBox {
    Unit *unit1, *unit2;
    ParamBoxUnitUnit() { 
	pboxtype = PBOX_TYPE_UNIT_UNIT;
	unit1 = NULL;
	unit2 = NULL;
    }
};

/* Unit-side parameter box. */
struct ParamBoxUnitSide : public ParamBoxUnit {
    Side *side;
    ParamBoxUnitSide() : ParamBoxUnit() {
	pboxtype = PBOX_TYPE_UNIT_SIDE;
	side = NULL;
    }
};

/* SeerNode is an useful struct for tracking who sees clearly,
   and who doesn't. */

typedef struct a_seer_node {
    Unit *seer;
    int mistakes;
} SeerNode;

/* Parameter boxen related to seers. */

struct ParamBoxUnitSideSeers : public ParamBoxUnitSide {
    SeerNode *seers;
    ParamBoxUnitSideSeers() : ParamBoxUnitSide () {
	pboxtype = PBOX_TYPE_UNIT_SIDE_SEERS;
	seers = NULL;
    }
    virtual ~ParamBoxUnitSideSeers() { seers = NULL; }
};

struct ParamBoxUnitUnitSeers : public ParamBoxUnitUnit {
    SeerNode *seers;
    ParamBoxUnitUnitSeers() : ParamBoxUnitUnit () {
	pboxtype = PBOX_TYPE_UNIT_UNIT_SEERS;
	seers = NULL;
    }
    virtual ~ParamBoxUnitUnitSeers() { seers = NULL; }
};

/* The global linked list of all unit views. */

extern UnitView *viewlist;

/* Some convenient macros. */


/*! \brief Iteration over all units.
 *
 * For header to iterat through all \Units.
 * \note Since it is possible for a unit to change sides and therefore
 * prev/next pointers while iterating using the macros below, one
 * must be very careful either that unit sides can't change during
 * the loop, or else to maintain a temp var that can be used to
 * repair the iteration.  This also applies to units dying.
 *\param v is the Unit vector list iteration variable.
 */
#define for_all_units(v)  \
    for (v = unitlist; v != NULL; v = v->unext)

/*! \brief Iterate over all units.
 *
 * Iteration over all \Units on a given side.
 * \see for_all_units
 * \param s is a pointer to a \Side.
 * \param v is the view iteration parameter.
 */
#define for_all_side_units(s,v) \
    for (v = (s)->unithead->next; v != (s)->unithead; v = v->next)

/*! \brief Iterate over direct components.
 *
 * Iteration over all occupants of a \Unit (but not sub-occupants).
 * \see for_all_units
 * \param u1 is pointer to unit list.
 * \param v is the iteration parameter.
 */
#define for_all_occupants(u1,v) \
  for (v = (u1)->occupant; v != NULL; v = v->nexthere)

/*! \brief Iterate over ALL components.
 *
 * Iterate through all occupants including occupents within occupents within
 * occupents within \Unit.
 * This nifty little macro will climb the occupant : nexthere tree and
 * follow all branches three levels below (u) to find all the occs
 * within occs within occs within (u).  The test for (var) !=
 * (u)->nexthere is to stop the macro from climbing up above (u).
 * \see for_all_units
 * \param u is a pointer to the \Unit.
 * \param var is the iteration varable.
 */
#define for_all_occs_with_occs(u,var)  \
  for ((var) = (u)->occupant; \
       (var) != NULL && (var) != (u)->nexthere; \
  	 (var) = ((var)->occupant != NULL ? \
  	 	        (var)->occupant : \
  	 	      ((var)->nexthere != NULL ? \
  	 	        (var)->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->nexthere != NULL ? \
  		        (var)->transport->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->transport != NULL && \
  		        (var)->transport->transport->nexthere != NULL ? \
  		        (var)->transport->transport->nexthere : NULL)))))

/*! \brief Is valid Unit type?
 *
 * Check to see if \Unit is a valid Unit type.
 * \param unit is a pointer to a \Unit.
 * \return TRUE if the type of a \Unit is a valid type.
 */
#define is_unit(unit) ((unit) != NULL && is_unit_type((unit)->type))

/*! \brief Is alive?
 *
 * Is this unit alive?
 * \param unit is a pointer to a \Unit.
 * \return TRUE if the Unit has at least 1 HP.
 */
#define alive(unit) ((unit)->hp > 0)

/*! \brief Is independant?
 *
 * Is this \Unit independant?
 * \param unit is a pointer to a \Unit.
 * \return TRUE if the Unit belongs to an independant.
 */
#define indep(unit) ((unit)->side == indepside)

/*! \brief Is complete?
 *
 * Has this \Unit been completed?
 * \param unit is a pointer to a \Unit.
 * \return TRUE if the Unit has been completed.
 */
#define completed(unit) \
  ((unit)->cp >= u_cp((unit)->type))

/*! \brief Unit altitude.
 *
 * Extractor for the actual altitude of an airborne unit.
 * \param unit is a pointer to a \Unit.
 * \return altitude of unit or zero.
 */
#define unit_alt(unit) (((unit)->z & 1) == 0 ? ((unit)->z >> 1) : 0)

/*! \brief Unit connection.
 *
 * Extractor for the connection a unit is on. */
#define unit_conn(unit) (((unit)->z & 1) == 1 ? ((unit)->z >> 1) : NONTTYPE)

/*! Unit in play.
 * This is true if the unit is on the board somewhere.
 * \see is_unit, alive, inside_area.
 * \param unit is the pointer to the \Unit.
 * \return TRUE if it is a unit AND it's alive AND it's in a cell on the board.
 */
#define in_play(unit) \
  (is_unit(unit) && alive(unit) && inside_area((unit)->x, (unit)->y))

/*! \brief Is Unit Active?
 *
 * Is the unit currently available?
 * \see in_play, completed.
 * \param unit is the pointer to the \Unit.
 * \return TRUE if it is in play and completed.
 */
#define is_active(unit) (in_play(unit) && completed(unit))

/*! \brief Is unit located at (x,y)?
 *
 * Is unit in play and located at (x,y)?
 * \see in_play, wrapx.
 * \param unit is the pointer to the \Unit.
 * \param x is a possibly unwrapped x co-ordinate.
 * \param y is a y co-ordinate.
 * \return TRUE if unit is in play and located at (wrap(x), y).
 */
#define is_located_at(unit,x,y) (in_play(unit) && (unit)->x == wrapx(x) && (unit)->y == (y))

/*! \brief Was unit located at (x,y)?
 *
 * Was unit (maybe now dead) located at (x,y)?
 * \see is_unit, wrapx.
 * \param unit is the pointer to the \Unit.
 * \param x is a possibly unwrapped x co-ordinate.
 * \param y is a y co-ordinate.
 * \return TRUE if unit previously was located at (wrap(x), y).
 * \since unit may be dead now, we just test for is_unit instead of in_play.
 */
#define was_located_at(unit,x,y) (is_unit(unit) && (unit)->prevx == wrapx(x) && (unit)->prevy == (y))

/*! \brief Is unit being tracked?
 *
 * Is a side watching a \Unit?
 * \see side_in_set.
 * \param side is a pointer to a \Side.
 * \param unit is the pointer to the \Unit.
 * \return TRUE if the side bit for the side is set in the tracking mask.
 */
#define side_tracking_unit(side,unit) (side_in_set((side), (unit)->tracking))

/*! \brief Unit opinion.
 *
 * \Unit opinion, if defined, is an array of 2-byte values, one per side,
 * indexed by side number.  Each value consists of a low byte, which is the
 * for/against strength (positive is "for", negative is "against", neutral is 0),
 * and a high byte, which is the level of courage/fear (positive is courage,
 * negative is fear).
 * \see unit_courage, x_opinion, side_number.
 * \param unit is a pointer to the \Unit.
 * \param side is a pointer to the \Side.
 * \return 
 *    - < 0 if against,
 *    - = 0 if neutral or not defined, or
 *    - > 0 if for.
 * */
#define unit_opinion(unit,side)  \
  ((unit)->opinions != NULL ? x_opinion((unit)->opinions[side_number(side)]) : 0)

/*! \brief Unit courage.
 *
 * \Unit courage is the upper byte of the unit opinon array.  
 * \see unit_opinion, x_courage, side_number.
 * \param unit is a pointer to the \Unit.
 * \param side is a pointer to the \Side.
 * \return 
 *    - < 0 if fearful,
 *    - = 0 if neutral or not defined, or
 *    - > 0 if couragous.
 * */
#define unit_courage(unit,side)  \
  ((unit)->opinions != NULL ? x_courage((unit)->opinions[side_number(side)]) : 0)

/*! \brief Extract opinion.
 *
 * Extract the low order byte, and convert to +- range.
 * \param val is the value to extract
 * \return low byte value in range -128/127.
 */
#define x_opinion(val) (((val) & 0xff) - 128)

/*! \brief Extract courage.
 *
 * Extract the high order byte, and convert to +- range.
 * \param val is the value to extract
 * \return high byte value in range -128/127.
 */
#define x_courage(val) (((val) >> 8) & 0xff)

/*! \brief Set opinion value.
 *
 * Set the opinion value.  Mask old value and 'or' in new
 * value.
 * \param val is the opinion word.
 * \param val2 is the new opinion part of the opinion word.
 * \return new 'opion' word.
 */
#define s_opinion(val,val2) (((val) & ~0xff) | (val2 + 128))

/*! \brief Set courage value.
 *
 * Set the courage value.  Mask old value and 'or' in new
 * value.
 * \param val is the opinion word.
 * \param val2 is the new courage part of the opinion word.
 * \return new 'opinion' word.
 */
#define s_courage(val,val2) (((val) & ~0xff00) | (val2 << 8))

/* These return a percentage value. */

#define supply_inflow(unit) ((unit)->s_flow / 163)

#define supply_connectedness(unit) ((unit)->s_conn / 254)

/* (could use bit fields in struct I suppose...) */

#define DETONATE_FLAG_BIT 0

#define was_detonated(unit) ((unit)->flags & (1 << DETONATE_FLAG_BIT))

#define set_was_detonated(unit, v) \
  ((v) ? ((unit)->flags |= 1 << DETONATE_FLAG_BIT) \
   : ((unit)->flags &= ~(1 << DETONATE_FLAG_BIT)))

#define unit_point_value(unit) ((unit)->extras ? (unit)->extras->point_value : -1)

#define unit_appear_turn(unit) ((unit)->extras ? (unit)->extras->appear : -1)

#define unit_appear_var_x(unit) ((unit)->extras ? (unit)->extras->appear_var_x : -1)

#define unit_appear_var_y(unit) ((unit)->extras ? (unit)->extras->appear_var_y : -1)

#define unit_disappear_turn(unit) ((unit)->extras ? (unit)->extras->disappear : -1)

#define unit_extra_priority(unit) ((unit)->extras ? (unit)->extras->priority : -1)

#define unit_symbol(unit) ((unit)->extras ? (unit)->extras->sym : lispnil)

#define unit_sides(unit) ((unit)->extras ? (unit)->extras->sides : lispnil)

#define unit_doctrine(unit)  \
  ((unit)->side->udoctrine[(unit)->type])

#define construction_run_doctrine(unit,u2) \
  (unit_doctrine(unit)->construction_run[u2])

#define unit_side_treasury(unit) \
  ((unit)->side->treasury)

/* A sortable vector of units, generally useful. */

/*! \brief Sort types.
 *
 * The kinds of sort keys available for list windows. */
enum sortkeys {
    bynothing,      		/*!< No sort. */
    bytype,         		/*!< Sort by type. */
    byname,         		/*!< Sort by name. */
    byactorder,     	/*!< Sort by actions. */
    bylocation,     		/*!< Sort by location. */
    byside,         		/*!< Sord by side. */
    numsortkeytypes 	/*!< Number sort key types. */
};

/*! Can sort on as many as five keys. */
#define MAXSORTKEYS 5

/*! Unit vector entry. */
typedef struct a_unitvectorentry {
    Unit *unit;     	/*!< Pointer to Unit. */
    int flag;       	/*!< Flag */
} UnitVectorEntry;

/*! Sortable unit vector */
typedef struct a_unitvector {
    int size;                               /*!< Size */ 
    int numunits;                           /*!< Number of entries. */       
    enum sortkeys sortkeys[MAXSORTKEYS];    /*!< Sort by keys. */
    UnitVectorEntry units[1];               /*!< Head of vector. */
} UnitVector;

/*! \brief Unit in vector?
 *
 * Given an index into a unit vector, return the unit. */
/* \note (should check args?) */
#define unit_in_vector(uvec,ix) ((uvec)->units[ix].unit)

#undef  DEF_ACTION
#define DEF_ACTION(name,CODE,args,prepfn,netprepfn,dofn,checkfn,argdecl,doc) CODE,

/*! \brief Action types.
 *
 * Types of primitive unit actions. 
 */
typedef enum actiontype {

#include "action.def"

    NUMACTIONTYPES

} ActionType;

/*! \brief Action definition. */
typedef struct a_actiondefn {
    ActionType typecode;    	/*!< Action type. */
    const char *name;           /*!< Name. */
    const char *argtypes;       /*!< argument type string. */
} ActionDefn;

/*! \brief Maximum number of arguments to action. */
#define MAXACTIONARGS 4

/*! \brief Action. */
typedef struct a_action {
    ActionType type;		/*!< the type of the action */
    int args[MAXACTIONARGS];	/*!< assorted parameters */
    int actee;			/*!< the unit being affected by action */
    struct a_action *next;	/*!< chain to next action */
} Action;

/*! Actor state. */
typedef struct a_actorstate {
    short initacp;		/*!< how much we can still do */
    short acp;			/*!< how much we can still do */
    short actualmoves;		/*!< cells actually covered this turn */
    Action nextaction;  	/*!< Next action. */
} ActorState;

#define valid(x) ((x) == A_ANY_OK)

#define has_pending_action(unit)  \
  ((unit)->act && (unit)->act->nextaction.type != ACTION_NONE)

#define acp_indep(unit) u_acp_independent((unit)->type)

/*! \brief Does unit have ACP left?
 *
 * This is true if the unit still has acp left to act with.  This used
 * to test acp > acp-min, but that allows units to move around with
 * negative acp, and doesn't work as well in game designs.
 * Now returns TRUE if an acp-independent unit that can build is 
 * either waiting for tasks or not yet done with construction for
 * this turn.
 * \param unit is pointer to unit.
 */

#define has_acp_left(unit) \
  ((unit)->act && ((0 < (unit)->act->acp) || acp_indep(unit)))

/* What is the maximum amount of ACP an unit type can have? */
#define type_acp_max(u) ((u_acp_max(u) >= 0) ? u_acp_max(u) : u_acp(u)) 

/* All the definitions that govern planning. */

#undef  DEF_GOAL
#define DEF_GOAL(name,dname,GOALTYPE,args) GOALTYPE,

/*!
 * \brief Goal Type.
 *
 * The different types of goals.
 * A goal is a predicate object that can be tested to see whether it has
 * been achieved.  As such, it is a relatively static object and may be
 * shared.
 */
enum goaltype {

#include "goal.def"

    g_t_dummy
};
typedef enum goaltype GoalType;

/*! \brief Goal definition. */
typedef struct a_goaldefn {
    const char *name;             /*!< Name. */
    const char *display_name;     /*!< Display name. */
    const char *argtypes;         /*!< Type string of arguments. */
} GoalDefn;

/* The goal structure proper. */

/*! \brief Maximum number of goal arguments. */
#define MAXGOALARGS 5

/*! \brief Goal. */
typedef struct a_goal {
    GoalType type;              /*!< Goal type. */
    short tf;                   /*!< Terrain flag. ??? */
    Side *side;                 /*!< Pointer to side. */
    short args[MAXGOALARGS];    /*!< Argument array. */
} Goal;

#undef  DEF_TASK
#define DEF_TASK(name,dname,CODE,argtypes,dofn,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl) CODE,

/*! \brief Task type.
 *
 * A task is a single executable element of a unit's plan.  Each task
 * type is something that has been found useful or convenient to
 * encapsulate as a step in a plan.  Tasks are also useful for human
 * interfaces to use instead of actions, since they have retry
 * capabilities that are not part of action execution.
 * Note that a task's arguments are not the same as the arguments
 * passed to the functions that create and push tasks.  For instance,
 * the 'c' task argument is actually a state variable recording a
 * direction choice; this is important for coherent routefinding, but
 * is not needed in order to create a movement task.
 */
typedef enum a_tasktype {

#include "task.def"

    NUMTASKTYPES
} TaskType;

/*! \brief Task outcome. */
typedef enum a_taskoutcome {
  TASK_UNKNOWN,         /*!< Unknown task. */
  TASK_FAILED,          /*!< Task failed. */
  TASK_IS_INCOMPLETE,   /*!< Task incomplete. */
  TASK_PREPPED_ACTION,  /*!< Task prepped action. */
  TASK_IS_COMPLETE      /*!< Task complete. */
} TaskOutcome;

/*! \brief Maximum arguments for a task. */
#define MAXTASKARGS 6

/*! \brief Task. */
typedef struct a_task {
    TaskType type;		/*!< the kind of task we want to do */
    int args[MAXTASKARGS];	/*!< arguments */
    short execnum;		/*!< how many times this has been done */
    short retrynum;		/*!< number of immed failures so far */
    struct a_task *next;	/*!< the next task to undertake */
} Task;

/*! \brief Task definition. */
typedef struct a_taskdefn {
    const char *name;		/*!< name of the task type */
    const char *display_name;	/*!< name to display in interfaces */
    const char *argtypes;	/*!< string giving types of task's args */
    TaskOutcome (*exec)(Unit *unit, Task *task); /*!< Pointer to task function. */
} TaskDefn;

#define is_task_type(x) (between(0, (x), NUMTASKTYPES - 1))

#undef  DEF_PLAN
#define DEF_PLAN(name,CODE) CODE,

/*! \brief Plan type.
 *
 * A plan is what a single unit uses to make decisions, both for itself and
 * for any other units it commands.  Any unit that can act at all has a
 * plan object.  A plan collects lots of unit behavior, but its most
 * important structure is the task queue, which contains a list of what
 * to do next, in order.
 * Plan types distinguish several kinds of usages. 
 */
typedef enum plantype {

#include "plan.def"

    NUMPLANTYPES
} PlanType;

/*! \brief Plan. */
typedef struct a_plan {
    PlanType type;				/*!< general type of plan that we've got here */
    short creation_turn;			/*!< turn at which this plan was created */
    short initial_turn;				/*!< turn at which this plan is to be done */
    short final_turn;				/*!< turn to deactivate this plan */
    short asleep;					/*!< true if the unit is doing nothing */
    short reserve;					/*!< true if unit waiting until next turn */
    short delayed;      				/*!< true if plan delayed. ??? */
    short waitingfortasks;			/*!< true if waiting to be given a task */
    short aicontrol;				/*!< true if an AI can operate on the unit */
    short supply_alarm;    		 	/*! true if supply alarm is needed. */
    short supply_is_low;   		 	/*! true is supply is low. */
    short waitingfortransport;		/*!< true if waiting for transport. */
    struct a_goal *maingoal;			/*! the main goal of this plan */
    struct a_goal *formation;			/*! goal to keep in a formation */
    struct a_task *tasks;			/*! pointer to chain of sequential tasks */
    /* Not saved/restored. (little value, some trouble to do) */
    struct a_unit *funit;			/*! pointer to unit keeping formation */
    Action lastaction;	 			/*! a copy of the last action attempted */
    short lastresult;				/*! that action's outcome */
    Task last_task;				/*! a copy of the last task executed */
    TaskOutcome last_task_outcome;	/*! that task's outcome */
    short execs_this_turn; 			/*!< Excutes this turn if true. */
} Plan;

#define for_all_tasks(plan,task)  \
  for (task = (plan)->tasks; task != NULL; task = task->next)

#define ai_controlled(unit)  \
  ((unit)->plan && (unit)->plan->aicontrol && side_has_ai(unit->side))

/* Global unit variables. */

/*! \brief Unit list. */
extern Unit *unitlist;
/*! \brief Unit temporary. */
extern Unit *tmpunit;

/*! \brief Number of Units. */
extern int numunits;
/*! \brief Number of live Units. */
extern int numliveunits;

/*! \brief Temporary sort keys. */
extern enum sortkeys tmpsortkeys[];


/* \brief Action definition array. */
extern ActionDefn actiondefns[];

/*! \brief Goal definition array. */
extern GoalDefn goaldefns[];

/*! \brief Task definition array. */
extern TaskDefn taskdefns[];

/*! \brief Plan type name array. */
extern const char *plantypenames[];

/* Declarations of unit-related functions that do not change the kernel state. 
Those that actually do something have been moved to kernel.h. */

/* Testable conditions etc. */

extern int side_owns_occupant_of_unit(Side *side, Unit *unit);
extern int side_owns_viewer_in_unit(Side *side, Unit *unit);
extern int type_can_occupy_cell(int u, int x, int y);
extern int side_thinks_type_can_occupy_cell(Side *side, int u, int x, int y);
extern int type_can_occupy_empty_cell(int u, int x, int y);
extern int type_can_occupy_terrain(int u, int t);
extern int can_occupy_cell_without(Unit *unit, int x, int y, Unit *unit3);
extern int type_can_occupy_cell_without(int u, int x, int y, Unit *unit3);
extern int side_can_put_type_at(Side *side, int u, int x, int y);
extern int side_can_put_type_at_without(Side *side, int u, int x, int y, Unit *unit);
extern int side_thinks_it_can_put_type_at(Side *side, int u, int x, int y);
extern int side_thinks_it_can_put_type_at_without(Side *side, int u, int x, int y, Unit *unit);
extern int can_occupy(Unit *unit, Unit *transport);
extern int type_can_occupy(int u, Unit *transport);
extern int side_thinks_type_can_occupy(Side *side, int u, UnitView *transport);
extern int type_can_occupy_without(int u, Unit *transport, Unit *unit);
extern int side_thinks_type_can_occupy_without(int u, UnitView *transport, Unit *unit);
extern int type_can_occupy_empty_type(int u, int u2);
extern int type_can_have_occs(int u);
extern int new_unit_allowed_on_side(int u, Side *side);
extern int unit_allowed_on_side(Unit *unit, Side *side);
extern int num_sides_allowed(int u);
extern int type_allowed_on_side(int u, Side *side);
extern int type_ever_available(int u, Side *side);
extern int unit_trusts_unit(Unit *unit1, Unit *unit2);
extern int type_survives_in_cell(int u, int nx, int ny);
extern int type_survives_in_terrain(int u, int t);
extern int can_build(Unit *unit);
extern int type_can_build(int u, Side *side);
extern int can_move(Unit *unit);
extern int can_extract_at(Unit *unit, int x, int y, int *mp);
extern int can_load_at(Unit *unit, int x, int y, int *mp);
extern int can_develop(Unit *unit);
extern int type_can_develop(int u);
extern int can_change_type(Unit *unit);
extern int can_change_type_to(Unit *unit, int u2);
extern int could_change_type(int u);
extern int could_change_type_to(int u, int u2);
#define could_auto_upgrade_to(u,u2) ((u2) == u_auto_upgrade_to(u))
extern int can_disband(Unit *unit);
extern int type_can_disband(int u);
extern int side_can_disband(Side *side, Unit *unit);
extern int can_add_terrain(Unit *unit);
extern int type_can_add_terrain(int u);
extern int can_remove_terrain(Unit *unit);
extern int type_can_remove_terrain(int u);
extern int can_build_attackers(Side *side, int u);
extern int can_build_defenders(Side *side, int u);
extern int can_build_explorers(Side *side, int u);
extern int can_build_colonizers(Side *side, int u);
extern int can_build_facilities(Side *side, int u);
extern int can_build_or_help(Unit *unit);
extern int can_research(Unit *unit);
extern int can_produce(Unit *unit);
extern int total_production(Unit *unit, int m);
extern int base_production(Unit *unit, int m);
extern int total_consumption(Unit *unit, int m);
extern int base_consumption(Unit *unit, int m);
extern int survival_time(Unit *unit);
extern int will_not_move(Unit *unit);
extern int needs_material_to_move(Unit *unit, int m);
extern int needs_material_to_survive(Unit *unit, int m);
extern int operating_range_best(int u);
extern int operating_range_worst(int u);
extern int real_operating_range_best(Unit *unit);
extern int real_operating_range_worst(Unit *unit);
extern int can_build_type(Unit *unit, int u2);
extern int can_develop_or_build_type(Unit *unit, int u2);
extern int unit_can_build_type_at(Unit *unit, int u2, int x, int y);
extern int unit_can_build_type(Unit *unit, int u2);
extern int type_can_build_type(int u, Side *side, int u2);

extern Unit *incomplete_build_target(Unit *unit);

#if 0
extern int moves_till_low_supplies(Unit *unit);
extern int num_occupants(Unit *unit);
extern int num_units_at(int x, int y);
#endif

extern int see_chance(int u, int u2, int dist);
extern int see_chance(Unit *seer, Unit *tosee);

/* Unit names and designations. */

extern const char *unit_desig(Unit *unit);
extern const char *unit_desig_no_loc(Unit *unit);
extern const char *utype_name_n(int u, int n);
extern char *shortest_unique_name(int u);
extern char *shortest_generic_name(int u);
extern const char *actorstate_desig(struct a_actorstate *as);

/* Unit locators. */

extern Unit *find_unit(int n);
extern Unit *find_unit_dead_or_alive(int n);
extern Unit *find_unit_by_name(const char *nm);
extern Unit *find_unit_by_number(int nb);
extern Unit *find_unit_by_symbol(Obj *sym);

/* Unit vector manipulating functions. Declared here rather than in kernel.h 
since they also are used in the interfaces. */

extern UnitVector *make_unit_vector(int initsize);
extern void clear_unit_vector(UnitVector *vec);
extern UnitVector *add_unit_to_vector(UnitVector *vec, Unit *unit, int flag);
extern void remove_unit_from_vector(UnitVector *vec, Unit *unit, int pos);
extern void sort_unit_vector(UnitVector *vec);

/* Used in mknames.c. */

extern void sort_units(int byidonly);

/* Turn init code - does not really belong here. */

extern void check_all_units(void);

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,prepfn,netprepfn,dofn,CHECKFN,ARGDECL,doc)  \
  extern int CHECKFN ARGDECL;

#include "action.def"

#undef  DEF_TASK
#define DEF_TASK(name,dname,code,argtypes,dofn,CREATEFN,setfn,netsetfn,pushfn,netpushfn,ARGDECL)  \
  extern Task *CREATEFN ARGDECL;

#include "task.def"

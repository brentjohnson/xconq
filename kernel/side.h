/* Definitions for sides in Xconq.
   Copyright (C) 1987-1989, 1991-1997, 1999-2000 Stanley T. Shebs.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/side.h
 * \brief Definitions for sides in Xconq.
 *
 * \note Many of the side macros check for a null side pointer.  This
 * is due to a null pointer being used to indicate that the unit was 
 * independant.  That has been changed, now the independant units are
 * assigned to side 0.  The check for a null side pointer should not 
 * be needed any longer, but has been left in for the time being.
 * In particular, the Side Mask equates a null side pointer and the
 * side 0 as being the same thing.
 */

#include "parambox.h"

#if MAXSIDES < 31
/*! \brief Side mask.
 *
 * A side mask is a bit vector, where the bit position corresponds to the
 * side number.  It currently is implemented on tha basis of a 32 bit
 * integer, so it cannot support more than 30 players.
 */
typedef int SideMask;
/*! \brief No Sides are all zero bits. */
#define NOSIDES (0)
/*! \brief All Sides is all ones. */
#define ALLSIDES (-1)
/*! \brief Add Side to vector.
 *
 * Sets the bit corresponding to the Side number, or the
 * zero'th bit if the \Side pointer is nil.
 * \param side is the pointer to the \Side.
 * \param mask is the side mask.
 * \return the modified side mask.
 */
#define add_side_to_set(side,mask) ((mask) | (1 << ((side) ? (side)->id : 0)))

/*! \brief Remove side from mask.
 *
 * Resets the bit corresponding to the Side number, or the
 * zero'th bit if the \Side pointer is nil.
 * \param side is the pointer to the \Side.
 * \param mask is the side mask.
 * \return the side mask.
 */
#define remove_side_from_set(side,mask) \
  ((mask) & ~(1 << ((side) ? (side)->id : 0)))

/*! \brief Side in set.
 *
 * This returns a non-zero value if the Side is set.
 * \param side is a pointer to the \Side.
 * \param mask is the side mask.
 * \return non-zero if the side is in the set.
 */
#define side_in_set(side,mask) ((mask) & (1 << ((side) ? (side)->id : 0)))
#else
not implemented yet
#endif /* MAXSIDES */

/*! \brief Doctine.
 *
 * Doctrine is policy info that Units and Players use to help decide
 * behavior.
 */
typedef struct a_doctrine {
    short id;  			/*!< a unique id */
    const char *name;  		/*!< a distinguishing name for the doctrine */
    short resupply_percent;	/*!< do resupply task if below this */
    short rearm_percent;     	/*!< do resupply of ammo if below this */
    short repair_percent;     	/*!< do repairs if hp at or below this */
    short resupply_complete;   	/*!< cease resupply if % at or above this */
    short rearm_complete;     	/*!< cease rearming if % at or above this */
    short repair_complete;     	/*!< cease repairs if hp at or above this */
    short min_turns_food; 	/*!< minimal number of turns for which we 
				     should have food-type supplies */
    short min_distance_fuel; 	/*!< minimal distance in cells for which we 
				     should have fuel-type supplies */
    short *construction_run;	/*!< number of each type to build usually */
    short locked;               /*!< true if values can't be changed */
    struct a_doctrine *next;	/*!< pointer to next doctrine defined */
} Doctrine;

/*! \brief Standing order conditions.
 * This modifies a standing order to be limited to a 
 * particular location/unit.
 */
enum sordercond {
    sorder_always,	/*!< always */
    sorder_at,          /*!< when at */
    sorder_in,          /*!< when in */
    sorder_near         /*!< when near */
};

/*! \brief Standing order.
 *
 * A standing order is a conditional order that applies to any matching
 * unit not already doing a task.
 */
typedef struct a_standing_order {
    char *types;	       	/*!< unit types to which order applies */
    enum sordercond condtype;	/*!< type of condition to trigger on */
    int a1;             	/*!< first parameter to test (x, y, etc). */
    int a2;                    	/*!< second parameter to test (x, y, etc). */
    int a3;		       	/*!< third parameters to test (x, y, etc). */
    struct a_task *task;	/*!< \ref a_task "Task" to perform */
    struct a_standing_order *next;	/*!< link to next standing order for 
					     side */
} StandingOrder;

namespace Xconq {
    namespace AI {
	struct AI_Side;
    }
}
typedef Xconq::AI::AI_Side AI_Side;

/*! \brief Side.
 *
 * Each Xconq player is a "side" - more or less one country.  A side
 * may or may not be played by a person, and may or may not have a
 * display attached to it.  Each side has a different view of the
 * world.  Side 0 is for independant units.
 */
typedef struct a_side {
    int id;			/*!< a unique id */
    Obj *symbol;		/*!< a symbol bound to side's id */
    const char *name;		/*!< This is the proper name of a side, 
				 *   as a country or alliance name. Examples 
				 *   include "Axis" and "Hyperborea". */
    const char *longname;      	/*!< This is the long form of a side's name, 
				 *   as in "People's Republic of Hyperborea". 
				 *   Defaults to be the same as the side's 
				 *   name. */
    const char *shortname;	/*!< This is an short name or acronym for the 
				 *   side, often just the letters of the long 
				 *   name, as in "PRH". */
    const char *noun;		/*!< This is the name of an individual unit or 
				 *   person belonging to the side. Defaults 
				 *   to "", which suppresses any mention of 
				 *   the side when (textually) describing the 
				 *   individual. */
    const char *pluralnoun;	/*!< This is what you would call a group of 
				 *   individuals. Defaults to the most common 
				 *   plural form of the noun (in English, 
				 *   the default pluralizer adds an "s"), so 
				 *   any alternative plural noun, such as 
				 *   "Chinese", will need an explicit 
				 *   plural-noun value. */
    const char *adjective;	/*!< This is an adjective that can be used of 
				 *   individuals on the side, as in "Spanish".  
				 *   Defaults to "", which suppresses use of 
				 *   the adjective. As a complete example, a 
				 *   side named "Poland" would have a long 
				 *   name "Kingdom of Poland", short name 
				 *   "Po", noun "Pole", plural noun "Poles", 
				 *   and adjective "Polish". */
    const char *colorscheme;	/*!< This is a comma-separated list of colors 
				 *   that represents the side. Defaults to 
				 *   "black". */
    const char *default_color;	/*!< default color used in the absence of 
				 *   emblems and at low magnifications 
				 *   <I>name</I> */
    const char *emblemname;	/*!< This property is the name of a graphical 
				 *   icon that represents the side. An emblem 
				 *   name of "none" suppresses any emblem 
				 *   display for the side. Defaults to "", 
				 *   which gives the side a randomly-selected 
				 *   emblem. */
    const char **unitnamers;	/*!< This specifies which namers will be used 
				 *   with which types that the side starts out 
				 *   with or creates new units. These will not 
				 *   be run automatically on captured units or 
				 *   gifts. */
    Obj *featurenamers;		/*!< This specifies which namers to use with 
				 *   which geographical features in the side's 
				 *   initial country (if if has one). */
    short nameslocked;		/*!< True if names may not be changed by 
				 *   player. */
    const char *sideclass;	/*!< This is a side's class, which is a 
				 *   keyword that characterizes the side. Any 
				 *   number of sides may be in the same 
				 *   class. */
    struct a_unit *self_unit;	/*!< This identifies a unit that represents 
				 *   the side itself. The value may be a unit 
				 *   id, number, string, or symbol. Defaults 
				 *   to 0, which means that no unit represents 
				 *   the side. See \ref selfUnit for more 
				 *   information. */
    short self_unit_id;		/*!< Id of the self_unit. */
    struct a_side *controlled_by;	/*!< side controlling this one 
					 *   <I>relationship</I> */
    short controlled_by_id;	/*!< id of controlling side 
				 *   <I>relationship</I> */
    SideMask knows_about;	/*!< true if side knows about another side 
				 *   <I>relationship</I> */
    short *trusts;		/*!< true if side trusts another side 
				 *   <I>relationship</I> */
    short *trades;		/*!< true if side trades with another side. 
				 *   <I>relationship</I> */
    short *startwith;		/*!< how many units of each type at start of 
				 *   game <I>relationship</I> */
    short *counts;		/*!< array of numbers for identifying units 
				 *   <I>relationship</I> */
    short *tech;		/*!< tech level for each unit type 
				 *   <I>relationship</I> */
    short *inittech;		/*!< tech level at beginning of turn 
				 *   <I>relationship</I> */
    short *action_priorities;	/*!< action priority for each unit type 
				 *   <I>relationship</I> */
    short *already_seen;	/*!< chance that other units already seen 
				 *   <I>relationship</I> */
    short *already_seen_indep;	/*!< chance that independent units already 
				 *   seen <I>relationship</I> */
    Doctrine *default_doctrine;	/*!< fallback \ref a_doctrine "Doctrine" 
				 *   <I>relationship</I> */
    Doctrine **udoctrine;	/*!< array of per-unit-type \ref a_doctrine 
				 *   "Doctrines" <I>relationship</I> */
    short doctrines_locked;	/*!< true if all doctrines locked 
				 *   <I>relationship</I> */
    StandingOrder *orders;	/*!< list of \ref a_standing_order 
				 *   "Standing Orders" for the side 
				 *   <I>relationship</I> */
    StandingOrder *last_order;	/*!< Pointer to last \ref a_standing_order 
				 *   "Standing Order" in list 
				 *   <I>relationship</I> */
    char *uorders;		/*!< bit vector of types that have orders 
				 *   <I>relationship</I> */
    Obj *possible_units;	/*!< list of \ref a_unit "Units" to give to 
				 *   side <I>relationship</I> */
    short ai_may_resign;	/*!< true if AI may draw or resign on its own 
				 *   <I>relationship</I> */
    short *advance;		/*!< State of research (accum pts) for each 
				 *   Advance. Set to -1 when completed. 
				 *   <I>relationship</I> */
    short research_topic;	/*!< Advance that the side is working on 
				 *   <I>relationship</I> */
    short autoresearch;		/*!< TRUE if next research topic autoselected 
				 *   <I>relationship</I> */
    short research_goal;	/*!< Research goal for the side. */
    long *treasury;		/*!< globally available supply of each material 
				 *   type <I>relationship</I> */
    short *c_rates;		/*!< material conversion rates 
				 *   <I>relationship</I> */
    char *terrview;		/*!< ptr to view of terrain <I>view</I>*/
    char **auxterrview;		/*!< ptr to view of aux terrain <I>view</I>*/
    short *terrviewdate;	/*!< ptr to dates of view of terrain 
				 *   <I>view</I> */
    short **auxterrviewdate;	/*!< ptr to dates of view of aux terrain 
				 *   <I>view</I>*/
    struct a_unit_view **unit_views;	/*!< ptr to table of views of units 
				 *   <I>view</I>*/
    short **materialview;	/*!< ptr to view of cell materials <I>view</I>*/
    short **materialviewdate;	/*!< ptr to dates of view of cell materials 
				 *   <I>view</I>*/
    short *tempview;		/*!< ptr to view of temperature <I>view</I>*/
    short *tempviewdate;	/*!< ptr to dates of view of temperature 
				 *   <I>view</I>*/
    short *cloudview;		/*!< ptr to view of clouds <I>view</I>*/
    short *cloudbottomview;	/*!< ptr to view of cloud bottoms <I>view</I>*/
    short *cloudheightview;	/*!< ptr to view of cloud heights <I>view</I>*/
    short *cloudviewdate;	/*!< ptr to dates of view of clouds 
				 *   <I>view</I>*/
    short *windview;		/*!< ptr to view of winds <I>view</I>*/
    short *windviewdate;	/*!< ptr to dates of view of winds <I>view</I>*/
    short ingame;		/*!< true if side participating in game 
				 *   <I>status</I>*/
    short everingame;		/*!< true if side ever participated in a turn 
				 *   <I>status</I>*/
    short priority;		/*!< overall action priority of this side 
				 *   <I>status</I>*/
    short status;		/*!< -1/0/1 for lost/draw/won <I>status</I>*/
    short *scores;		/*!< array of scores managed by scorekeepers 
				 *   <I>status</I>*/
    Obj *rawscores;		/*!< score data as read from file 
				 *   <I>status</I>*/
    short willingtodraw;	/*!< will this side go along with quitters? 
				 *   <I>status</I>*/
    short autofinish;		/*!< turn done when all units acted 
				 *   <I>status</I>*/
    short finishedturn;		/*!< true when side wants to go to next turn 
				 *   <I>status</I>*/
    short turntimeused;		/*!< seconds used this turn <I>status</I>*/
    short totaltimeused;	/*!< total seconds used <I>status</I>*/
    short timeouts;		/*!< total timeouts used <I>status</I>*/
    short timeoutsused;		/*!< total timeouts used <I>status</I>*/
    short curactor_pos;		/*!< index in actionvector to current unit 
				 *   <I>status</I>*/
    struct a_unit *curactor;	/*!< pointer to current unit acting. 
				 *   <I>status</I>*/
    short curactor_id;		/*!< Id of current unit acting. <I>status</I>*/
    short advantage;		/*!< actual advantage <I>setup</I>*/
    short minadvantage;		/*!< min advantage requestable during init 
				 *   <I>setup</I>*/
    short maxadvantage;		/*!< max of same <I>setup</I>*/
    short init_center_x;	/*!< center of preferred initial view, 
				 *   x<I>setup</I>*/
    short init_center_y;	/*!< center of preferred initial view, 
				 *   y<I>setup</I>*/
    Obj *instructions;		/*!< notes to player about the game 
				 *   <I>setup</I>*/
    short last_notice_date;	/*!< last turn during which notice was posted */
    short realtimeout;		/*!< how long to wait before just going ahead */
    long startbeeptime;		/*!< after this time, beep to signal next 
				 *   turn */
    short *gaincounts;		/*!< array of counts of unit gains by the 
				 *   side */
    short *losscounts;		/*!< array of counts of losses by the side */
    long **atkstats;		/*!< array of counts of attacks by units */
    long **hitstats;		/*!< array of damage done by unit attacks */
    struct a_player *player;	/*!< pointer to data about the player */
    short playerid;		/*!< numeric id of the player */
    struct a_ui *ui;		/*!< pointer to all the user interface data */
    Obj *uidata;		/*!< read/write form of user interface state */
    short aitype;		/*!< type of AI making decisions */
    struct a_ai *ai;		/*!< pointer to the AI making decisions */
    Obj *aidata;		/*!< readable/writable form of AI state */
    struct a_rui *rui;		/*!< pointer to interface data if remote */
    struct a_rai *rai;		/*!< pointer to AI data if remote */
    //! Pointer to side's master AI struct.
    AI_Side *master_ai;
    short startx, starty;	/*!< approx center of side's "country" */
    short busy;			/*!< true when side state not saveable */
    short finalradius;		/*!< ??? */
    short willingtosave;	/*!< will this side go along with saving 
				 *   game? */
    short see_all;		/*!< True if the side sees everything. Right 
				 *   now this is just a cache for g_see_all().  
				 *   Note in particular that becoming a 
				 *   designer should not change this value. */
    short show_all;		/*!< True if the side's interface should 
				 *   display everything. This is seeded from 
				 *   side->see_all, and also becomes true when 
				 *   designing or when the game is over. */
    short may_set_show_all;	/*!< True if the value of show_all can be 
				 *   changed by the player.  This is normally 
				 *   off, but can be toggled while designing or 
				 *   after the game is over. */
    short unit_view_restored;	/*!< true if unit view was read in */
#ifdef DESIGNERS
    short designer;		/*!< true if side is doing scenario design */
#endif /* DESIGNERS */
    short *uavail;		/*!< vector of types allowed on this side 
				 *   (as determined by u_possible_sides)*/
    short *canbuild;		/*!< vector of unit types that side has 
				 *   advances to build */
    short *candevelop;		/*!< vector of unit types that side has 
				 *   advances to develop */
    short *cancarry;		/*!< vector of unit types that side has 
				 *   transports to carry */
    short *canresearch;		/*!< vector of advances that side has advances 
				 *   to research */
    short *research_precluded;	/*!< Vector of advances that can no longer 
				 *   be researched by the side because they 
				 *   have been precluded by other researched 
				 *   advances. */
    //! List of utypes that side can act with.
    short *could_act_with;
    //! List of utypes that side is restricted to constructing.
    short *could_construct;
    //! List of utypes that side is restricted to developing.
    short *could_develop;
    //! List of atypes that side is restricted to researching.
    short *could_research;
    struct a_unit *unithead;	/*!< points to list of all units on this side */
    struct a_unitvector *actionvector;	/*!< vector of acting units */
    short numwaiting;		/*!< number of units waiting to get orders */
    short *coverage;		/*!< indicates how many looking at this cell */
    short *alt_coverage;	/*!< indicates minimum altitude visible */
    short *numunits;		/*!< number of units the side has */
    short numacting;		/*!< number of units that can do things */
    short numfinished;		/*!< number of units that have finished 
				 *   acting */
    long turnstarttime;		/*!< real time at start of turn */
    long lasttime;		/*!< when clock started counting down again */
    long turnfinishtime;	/*!< real time when we've finished 
				 *   interacting */
    long laststarttime;		/*!< ??? */
    int point_value_cache;	/*!< current score. ??? */
    int point_value_valid;	/*!< true if point_value_cache is valid. ??? */
    int prefixarg;		/*!< cache of UI prefixarg, for generic cmds */
    struct a_side *next;	/*!< pointer to next in list */
} Side;

/* Side parameter box. */
struct ParamBoxSide : public ParamBox {
    Side *side;
    ParamBoxSide() { pboxtype = PBOX_TYPE_SIDE; side = NULL; }
};

/* Some convenient macros. */

/*! \brief Is side playing?
 *
 * Check to see if \Side is in game and playing.
 * \param side is a pointer to the \Side to check.
 */
#define side_in_play(side) (side->ingame)

/* Iteration over all sides. */

/*! \brief Iterate real \Sides..
 *
 * This is the old for_all_sides which does not include indepside.
 *
 * \param v is the side iteration variable. It is a pointer to a \Side.
 */
#define for_all_real_sides(v)  \
  for (v = (sidelist ? sidelist->next : NULL); v != NULL; v = v->next)

/*! \brief Iterate all sides.
 *
 * This is the new indepside-including version of for_all_sides. 
 * It has replaced for_all_side_plus_indep everywhere.
 * \param v is the side iteration variable. It is a pointer to a \Side.
 */
#define for_all_sides(v)  \
  for (v = sidelist; v != NULL; v = v->next)

/* Macros for accessing and setting a side's view data.  In general,
   interfaces should go through these macros instead of accessing the
   world's state directly. */

/*! \brief Terrain view.
 *
 * If the \ref viewTerrainType exists for a \Side, return the View Terrain Type.
 * If it doesn't, return the the View Terrain Type from the terrain.
 * \see aref, buildtview, terrain_at.
 * \param s is the pointer to the \Side.
 * \param x is the x-coordinate of the cell to view.
 * \param y is the y-coordinate of the cell to view.
 * \return the value for the \ref viewTerrainType at the co-ordinates.
 */
#define terrain_view(s,x,y)  \
  ((s)->terrview ? aref((s)->terrview, x, y) : buildtview(terrain_at(x, y)))

/*! \brief Set View Terrain view.
 *
 * Sets the value of the \ref viewTerrainType at a cell, if a \Side has a view
 * defined.  Otherwise does nothing.
 * \see aset.
 * \param s is a pointer to the \Side.
 * \param x is the cell's x co-ordinate to set.
 * \param y is the cell's y co-ordinate to set.
 * \param v is the view terain type to which to set the cell.
 * \return the view type of the terrain, or 0.
 */
#define set_terrain_view(s,x,y,v)  \
  ((s)->terrview ? aset((s)->terrview, x, y, v) : 0)

/*! \brief Convert Terrain to View Terrain type.
 *
 * Take the Terrain type, and convert it to a \ref viewTerrainType 
 * by adding 1 to it.
 * \param t is the Terrain type.
 * \return the View Terrain type.
 */
#define buildtview(t) ((t) + 1)

/*! \brief Convert View Terrain type to Terrain type.
 *
 * Take a \ref viewTerrainType and convert it to a Terrain type by
 * subtracting 1 from it.
 * \param v is the View Terrain type.
 * \return the Terrain type.
 */
#define vterrain(v) ((v) - 1)

/*! \brief a cell which has not been seen by a side. */
#define UNSEEN (0)

/*! \brief Terrain view on date.
 *
 * If the \ref viewTerrainType on date exists for a side, return the \ref viewTerrainType.
 * If it doesn't, return UNSEEN.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x-coordinate of the cell to view.
 * \param y is the y-coordinaof of the cell to view.
 * \return 
 *   - the value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if side->terrviewdate is NULL
 */
#define terrain_view_date(s,x,y)  \
  ((s)->terrviewdate ? aref((s)->terrviewdate, x, y) : 0)

/*! \brief Set View Terrain type on date.
 *
 * Sets the value of the \ref viewTerrainType on date at a cell, if a 
 * \Side has a view on date defined.  Otherwise does nothing.
 * \see aset.
 * \param s is a pointer to the \Side.
 * \param x is the cell's x co-ordinate to set.
 * \param y is the cell's y co-ordinate to set.
 * \param v is the view terain type to which to set the cell.
 * \return 
 *   - the new value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if side->terrviewdate is NULL
 */
#define set_terrain_view_date(s,x,y,v)  \
  ((s)->terrviewdate ? aset((s)->terrviewdate, x, y, v) : 0)

/*! \brief Auxillary Terrain view.
 *
 * If the auxillary \ref viewTerrainType exists for a \Side,
 * and exits for this Terrain type, return the auxillary \ref viewTerrainType.
 * If a global auxillary view for a terrain type exits, return the global
 * auxillary \ref viewTerrainType.
 * If it doesn't, return UNSEEN.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x-coordinate of the cell to view.
 * \param y is the y-coordinaof of the cell to view.
 * \param t is the terrain of the cell.
 * \return 
 *   - the value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if side->terrviewdate is NULL
 */
#define aux_terrain_view(s,x,y,t)  \
  (((s)->auxterrview && (s)->auxterrview[t]) ? \
        aref((s)->auxterrview[t], x, y) : \
        (aux_terrain_defined(t) ? aux_terrain_at(x, y, t) : 0))

/*! \brief Set auxillary Terrain view.
 *
 * Sets the value of the auxillary \ref viewTerrainType at a cell, if a 
 * \Side has auxillary view defined.  Otherwise does nothing.
 * \see aset, aux_terrain_view.
 * \param s is a pointer to the \Side.
 * \param x is the cell's x co-ordinate to set.
 * \param y is the cell's y co-ordinate to set.
 * \param t is the terrain of the cell.
 * \param v is the \ref viewTerrainType to which to set the cell.
 * \return 
 *   - the new value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if side->terrviewdate is NULL
 */
#define set_aux_terrain_view(s,x,y,t,v)  \
  (((s)->auxterrview && (s)->auxterrview[t]) ? aset((s)->auxterrview[t], x, y, v) : 0)

/*! \brief Auxillary Terrain view on date.
 *
 * If the auxillary \ref viewTerrainType exists for a \Side, and exits for this
 * Terrain type at the current date, return the auxillary View Terrain Type.
 * If a global auxillary view for a terrain type exits, return the global
 * auxillary View Terrain Type.
 * If it doesn't, return UNSEEN.
 * \see aref, aux_terrain_view.
 * \param s is the pointer to the \Side.
 * \param x is the x-coordinate of the cell to view.
 * \param y is the y-coordinaof of the cell to view.
 * \param t is the Terrain of the cell.
 * \return 
 *   - the value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if side->terrviewdate is NULL
 */
#define aux_terrain_view_date(s,x,y,t)  \
  (((s)->auxterrviewdate && (s)->auxterrviewdate[t]) ? aref((s)->auxterrviewdate[t], x, y) : 0)

/*! \brief Set auxillary Terrain view.
 *
 * Sets the value of the auxillary Terrain view type at a cell, if a side has
 * auxillary view defined on this date.  Otherwise does nothing.
 * \see aset, aux_terrain_view.
 * \param s is a pointer to the \Side.
 * \param x is the cell's x co-ordinate to set.
 * \param y is the cell's y co-ordinate to set.
 * \param t is the terrain of the cell.
 * \param v is the view terain type to which to set the cell.
 * \return 
 *   - the new value for the \ref viewTerrainType at the co-ordinates,
 *   - zero if \Side->terrviewdate is NULL
 */
#define set_aux_terrain_view_date(s,x,y,t,v)  \
  (((s)->auxterrviewdate && (s)->auxterrviewdate[t]) ? aset((s)->auxterrviewdate[t], x, y, v) : 0)

/*! \brief Iteration over all Unit views.
 *
 * For header to iterat through all \UnitViews.
 *\param v is the \UnitView vector list iteration variable.
 */
#define for_all_unit_views(v)  \
    for (v = viewlist; v != NULL; v = v->vnext)

/*! \brief Find next unit view.
 *
 * Find unit_view_next for top units, (uv)->nexthere for occs.
 * \param s is a pointer to the \Side.
 * \param x is the x location.
 * \param y is the y location.
 * \param uv is the Unit view iteration variable.
 */
#define nextview(s,x,y,uv) \
	((uv)->transport == NULL ? unit_view_next((s), (x), (y), (uv)) : (uv)->nexthere)

/*! \brief Iterate Unit views at cell.
 *
 * Iterate over all unit views at a given location.
 * \see unit_view_at, unit_view_next.
 * \param s is a pointer to the \Side.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \param uv is the Unit view iteration variable.
 */
#define for_all_view_stack(s,x,y,uv) \
  for ((uv) = unit_view_at(s,x,y); (uv) != NULL; (uv) = unit_view_next(s,x,y,uv))

/* Iterate over all uviews in a given uvstack. */
#define for_all_uvstack(uvs,uv) \
    for ((uv) = (uvs); (uv); \
	 (uv) = unit_view_next((uvs)->observer == -1 \
				? NULL : side_n((uvs)->observer), \
			       (uvs)->x, (uvs)->y, (uv)))

/*! \brief Iterate Unit stack and occupants.
 *
 * Iterate through all unit views in this cell including occs within occs
 * within occs.   
 * This nifty little macro will climb the occupant : nexthere tree and
 * follow all branches four levels down to find all the occs within
 * occs within occs within units in the stack.
 * \param x is the x location.
 * \param y is the y location.
 * \param var is the Unit view pointer.
 */
#define for_all_view_stack_with_occs(s, x,y,uv)  \
  for ((uv) = unit_view_at((s), (x), (y)); \
          (uv) != NULL; \
  	 (uv) = ((uv)->occupant != NULL ? \
  	 	        (uv)->occupant : \
  		      (nextview((s), (x), (y), (uv)) != NULL ? \
  		        nextview((s), (x), (y), (uv)) : \
  		      ((uv)->transport != NULL && \
  		        nextview((s), (x), (y), (uv)->transport) != NULL ? \
  		        nextview((s), (x), (y), (uv)->transport) : \
  		      ((uv)->transport != NULL && \
  		        (uv)->transport->transport != NULL && \
  		        nextview((s), (x), (y), (uv)->transport->transport) != NULL ? \
  		        nextview((s), (x), (y), (uv)->transport->transport) : \
  		      ((uv)->transport != NULL && \
  		        (uv)->transport->transport != NULL && \
  		        (uv)->transport->transport->transport != NULL && \
  		        nextview((s), (x), (y), (uv)->transport->transport->transport) != NULL ? \
  		        nextview((s), (x), (y), (uv)->transport->transport->transport) : NULL))))))

/*! \brief Iterate views of occupants.
 *
 * Iterate over all views of occupants.
 * \param uv is the unit view of the transport.
 * \param v is the occupant view iteration variable.
 */
#define for_all_occupant_views(uv,v) \
  for (v = (uv)->occupant; v != NULL; v = v->nexthere)

/*! \brief Iterate over ALL components.
 *
 * Iterate through all occupants including occupents within occupents within
 * occupents within a unit view.
 * This nifty little macro will climb the occupant : nexthere tree and
 * follow all branches three levels below (uv) to find all the occs
 * within occs within occs within (uv).  The test for (var) !=
 * (uv)->nexthere is to stop the macro from climbing up above (uv).
 * \param uv is a pointer to the unit view.
 * \param var is the iteration varable.
 */
#define for_all_occupant_views_with_occs(uv,var)  \
  for ((var) = (uv)->occupant; \
       (var) != NULL && (var) != (uv)->nexthere; \
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

/* Get a pointer to the actual unit being viewed if possible. */

/*! \brief View Unit Unit pointer.
 *
 * Given a unit view pointer, return the actual Unit.
 * \param uv is the pointer to the unit view.
 * \return 
 *    - NULL if 
 *      - pointer to unit is NULL,
 *      - view unit id is not the same as the actual Unit id.
 *    - pointer to actual Unit.
 */
#define view_unit(uv) \
  (((uv)->unit != NULL && (uv)->id == (uv)->unit->id) ? (uv)->unit : NULL)

/* Manipulation of view of cell materials. */

/*! \brief Material view.
 *
 * Show viewable Material at cell.
 * \see aref, cell_material_defined, material_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param m is the Material type.
 * \return the Material viewable at cell.
 */
#define material_view(s,x,y,m)  \
  (((s)->materialview && (s)->materialview[m]) ? aref((s)->materialview[m], x, y) : (cell_material_defined(m) ? material_at(x, y, m) : 0))

/*! \brief Set Material view.
 *
 * Set viewable Material at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param m is the Material type.
 * \param v is the viewable Material at cell.
 * \return the Material viewable at cell or zero if not setable.
 */
#define set_material_view(s,x,y,m,v)  \
  (((s)->materialview && (s)->materialview[m]) ? aset((s)->materialview[m], x, y, v) : 0)

/*! \brief Material view on date.
 *
 * Show viewable Material at cell at date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param m is the Material type.
 * \return the Material viewable at cell at date.
 */
#define material_view_date(s,x,y,m)  \
  (((s)->materialviewdate && (s)->materialviewdate[m]) ? aref((s)->materialviewdate[m], x, y) : 0)

/*! \brief Set Material view at date.
 *
 * Set viewable Material at cell on date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param m is the Material type.
 * \param v is the viewable Material at cell.
 * \return the Material viewable at cell on date or zero if not setable.
 */
#define set_material_view_date(s,x,y,m,v)  \
  (((s)->materialviewdate && (s)->materialviewdate[m]) ? aset((s)->materialviewdate[m], x, y, v) : 0)

/* Manipulation of view of weather. */

/*! \brief Show temperture view.
 *
 * Show viewable temperature at cell.
 * \see aref, temperature_defined, temperature_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the temperature viewable at cell.
 */
#define temperature_view(s,x,y)  \
  ((s)->tempview ? aref((s)->tempview, x, y) : (temperatures_defined() ? temperature_at(x, y) : 0))

/*! \brief Set temperture view.
 *
 * Set the viewable temperature at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the temperature.
 * \return the temperature viewable at cell.
 */
#define set_temperature_view(s,x,y,v)  \
  ((s)->tempview ? aset((s)->tempview, x, y, v) : 0)

/*! \brief Show temperture view on date.
 *
 * Show viewable temperature at cell on this date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the temperature viewable at cell.
 */
#define temperature_view_date(s,x,y)  \
  ((s)->tempviewdate ? aref((s)->tempviewdate, x, y) : 0)

/*! \brief Set temperture view on date.
 *
 * Set the viewable temperature at cell on this date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the temperature.
 * \return the temperature viewable at cell.
 */
#define set_temperature_view_date(s,x,y,v)  \
  ((s)->tempviewdate ? aset((s)->tempviewdate, x, y, v) : 0)

/*! \brief Show cloud view.
 *
 * Show viewable cloud at cell.
 * \see aref, clouds_defined, raw_clouds_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_view(s,x,y)  \
  ((s)->cloudview ? aref((s)->cloudview, x, y) : (clouds_defined() ? raw_cloud_at(x, y) : 0))

/*! \brief Set cloud view.
 *
 * Set the viewable cloud at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud.
 * \return the cloud viewable at cell.
 */
#define set_cloud_view(s,x,y,v)  \
  ((s)->cloudview ? aset((s)->cloudview, x, y, v) : 0)

/*! \brief Show cloud view on date.
 *
 * Show viewable cloud at cell on this date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_view_date(s,x,y)  \
  ((s)->cloudviewdate ? aref((s)->cloudviewdate, x, y) : 0)

/*! \brief Set cloud view on date.
 *
 * Set the viewable cloud at cell on this date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud.
 * \return the cloud viewable at cell.
 */
#define set_cloud_view_date(s,x,y,v)  \
  ((s)->cloudviewdate ? aset((s)->cloudviewdate, x, y, v) : 0)

/*! \brief Get cloud bottom.
 *
 * Get cloud bottom value.
 * \see aref, cloud_bottoms_defined, raw_cloud_bottom_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_bottom_view(s,x,y)  \
  ((s)->cloudbottomview ? aref((s)->cloudbottomview, x, y) : (cloud_bottoms_defined() ? raw_cloud_bottom_at(x, y) : 0))

/*! \brief Set cloud bottom.
 *
 * Set the cloud bottom at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud bottom.
 * \return the cloud bottom at cell.
 */
#define set_cloud_bottom_view(s,x,y,v)  \
  ((s)->cloudbottomview ? aset((s)->cloudbottomview, x, y, v) : 0)

/*! \brief Get cloud bottom on date.
 *
 * Get cloud bottom value on this date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_bottom_view_date(s,x,y)  \
  ((s)->cloudbottomviewdate ? aref((s)->cloudbottomviewdate, x, y) : 0)

/*! \brief Set cloud bottom on date.
 *
 * Set the cloud bottom at cell on this date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud bottom.
 * \return the cloud bottom at cell.
 */
#define set_cloud_bottom_view_date(s,x,y,v)  \
  ((s)->cloudbottomviewdate ? aset((s)->cloudbottomviewdate, x, y, v) : 0)

/*! \brief Get cloud height.
 *
 * Get cloud height value.
 * \see aref, cloud_heights_defined, raw_cloud_height_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_height_view(s,x,y)  \
  ((s)->cloudheightview ? aref((s)->cloudheightview, x, y) : (cloud_heights_defined() ? raw_cloud_height_at(x, y) : 0))

/*! \brief Set cloud height.
 *
 * Set the cloud height at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud height.
 * \return the cloud height at cell.
 */
#define set_cloud_height_view(s,x,y,v)  \
  ((s)->cloudheightview ? aset((s)->cloudheightview, x, y, v) : 0)

/*! \brief Get cloud height on date.
 *
 * Get cloud height value on this date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the clouds viewable at cell.
 */
#define cloud_height_view_date(s,x,y)  \
  ((s)->cloudheightviewdate ? aref((s)->cloudheightviewdate, x, y) : 0)

/*! \brief Set cloud height on date.
 *
 * Set the cloud height at cell on this date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the cloud height.
 * \return the cloud height at cell.
 */
#define set_cloud_height_view_date(s,x,y,v)  \
  ((s)->cloudheightviewdate ? aset((s)->cloudheightviewdate, x, y, v) : 0)

/*! \brief Get wind view.
 *
 * Get wind view.
 * \see aref, winds_defined, raw_wind_at.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the wind viewable at cell.
 */
#define wind_view(s,x,y)  \
  ((s)->windview ? aref((s)->windview, x, y) : (winds_defined() ? raw_wind_at(x, y) : CALM))

/*! \brief Set wind view.
 *
 * Set the wind at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the wind.
 * \return the wind at cell.
 */
#define set_wind_view(s,x,y,v)  \
  ((s)->windview ? aset((s)->windview, x, y, v) : 0)

/*! \brief Get wind on date.
 *
 * Get wind value on this date.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the wind viewable at cell.
 */
#define wind_view_date(s,x,y)  \
  ((s)->windviewdate ? aref((s)->windviewdate, x, y) : 0)

/*! \brief Set wind on date.
 *
 * Set the wind at cell on this date.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the wind.
 * \return the wind at cell.
 */
#define set_wind_view_date(s,x,y,v)  \
  ((s)->windviewdate ? aset((s)->windviewdate, x, y, v) : 0)

/* Basic manipulation of vision coverage cache layer. */

/*! \brief Get vision coverage.
 *
 * Get vision coverate at cell.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the vision coverage at cell.
 */
#define cover(s,x,y)  \
  ((s)->coverage ? aref((s)->coverage, x, y) : 0)

/*! \brief Set vision coverage.
 *
 * Set the vision coverage at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the vision coverage.
 * \return the vision coverage at cell.
 */
#define set_cover(s,x,y,v)  \
  ((s)->coverage ? aset((s)->coverage, x, y, v) : 0)

/*! \brief Add vision coverage.
 *
 * Add vision coverate at cell.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the vision coverage.
 * \return the vision coverage at cell.
 */
#define add_cover(s,x,y,v)  \
  ((s)->coverage ? aadd((s)->coverage, (x), (y), (v)) : 0)

/*! \brief Get alternate vision coverage.
 *
 * Get alternate vision coverate at cell.
 * \see aref.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \return the alternate vision coverage at cell.
 */
#define alt_cover(s,x,y)  \
  ((s)->alt_coverage ? aref((s)->alt_coverage, x, y) : 0)

/*! \brief Set alternate vision coverage.
 *
 * Set the alternate vision coverage at cell.
 * \see aset.
 * \param s is the pointer to the \Side.
 * \param x is the x co-ordiate of the cell.
 * \param y is the y co-ordiate of the cell.
 * \param v is the value of the alternate vision coverage.
 * \return the alternate vision coverage at cell.
 */
#define set_alt_cover(s,x,y,v)  \
  ((s)->alt_coverage ? aset((s)->alt_coverage, x, y, v) : 0)

/*! \brief Has treasury?
 *
 * Tests both that s can have a treasury and that m can be stored in it.
 * \see m_treasury, g_indepside_has_treasury.
 * \param s is the pointer to the \Side.
 * \param m is the Material.
 */
#define side_has_treasury(s,m) (s != indepside ? m_treasury(m) : g_indepside_has_treasury() ? m_treasury(m) : FALSE) 

/* Tests of who/what runs the side. */

/*! \brief Side wants display?
 *
 * Determine if \Side needs a display.
 * \param s is the pointer to the \Side.
 * \return TRUE is side's player pointer and side's player display name are not NULL.
 */
#define side_wants_display(s) ((s)->player && (s)->player->displayname)

/*! \brief Side wants AI?
 *
 * Determin if a \Side needs AI.
 * \param s is the pointer to the \Side.
 * \return TRUE if the side's player pointer and the side's player ai type 
 *         name are not NULL.
 */
#define side_wants_ai(s) ((s)->player && (s)->player->aitypename)

/*! \brief Side has display?
 *
 * See if \Side has a display.
 * \param s is the pointer to the \Side.
 * \return TRUE if the side's user interface pointer and the side's remote
 *         user interface pointer are not NULL.
 */
#define side_has_display(s) (((s)->ui) != NULL || ((s)->rui) != NULL)

/*! \brief Side has local display?
 *
 * See if \Side has a local display.
 * \param s is the pointer to the \Side.
 * \return TRUE if the side's user interface pointer is not NULL.
 */
#define side_has_local_display(s) ((s)->ui != NULL)

/*! \brief Side has AI?
 *
 * See if the \Side has an AI running.
 * \param s is the pointer to the \Side.
 * \return TRUE if the side's AI pointer and the remote AI pointer
 *         are not NULL.
 */
#define side_has_ai(s) (((s)->ai) != NULL || ((s)->rai) != NULL)

/*! \brief Side has local AI?
 *
 * See if the \Side has an AI running locally.
 * \param s is the pointer to the \Side.
 * \return TRUE if the side's AI pointer is not NULL.
 */
#define side_has_local_ai(s) ((s)->ai != NULL)

/* Tests of side state. */

/*! \brief Has side lost?
 *
 * \param s is the pointer to the \Side.
 * \return TRUE if the side pointer is not NULL, the ingame status is
 *         FALSE, and the status is less than 0.
 */
#define side_lost(s) ((s) != NULL && !(s)->ingame && (s)->status < 0)

/*! \brief Has side drawn?
 *
 * \param s is the pointer to the \Side.
 * \return TRUE if the side pointer is not NULL, the ingame status is
 *         FALSE, and the status is 0.
 */
#define side_drew(s) ((s) != NULL && !(s)->ingame && (s)->status == 0)

/*! \brief Has side won?
 *
 * \param s is the pointer to the \Side.
 * \return TRUE if the side pointer is not NULL, the ingame status is
 *         FALSE, and the status is greater than 0.
 */
#define side_won(s) ((s) != NULL && !(s)->ingame && (s)->status > 0)

#define side_gain_count(s,u,r) (((s)->gaincounts)[num_gain_reasons*(u)+(r)])

#define side_loss_count(s,u,r) (((s)->losscounts)[num_loss_reasons*(u)+(r)])

#define side_atkstats(s,a,d) ((s)->atkstats[a] ? ((s)->atkstats[a])[d] : 0)

#define side_hitstats(s,a,d) ((s)->hitstats[a] ? ((s)->hitstats[a])[d] : 0)

/*! \brief Is Terrain visible?
 *
 * Can terain be seen by side at this location?
 * \see terrain_view.
 * \param side is the pointer to the \Side.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \return TRUE if side can see something at cell(x,y).
 */
#define terrain_visible(side, x, y)  \
  ((side)->see_all || (terrain_view((side), (x), (y)) != UNSEEN))

/*! \brief is border visable?
 *
 * Can border be seen?
 * \see seen_border.
 * \param side is the pointer t the \Side.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordiante of the cell.
 * \param d is the direction for which to check for the border.
 * \return TRUE is a border is visible.
 */
#define borders_visible(side, x, y, d)  \
  ((side)->see_all || seen_border((side), (x), (y), (d)))

/*! \brief Is unit visible?
 * Can one or more Units be seen in cell?
 * \see cover.
 * \param side is the pointer t the \Side.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordiante of the cell.
 * \return TRUE if at least one Unit is visible.
 */
#define units_visible(side, x, y)  \
  ((side)->see_all || (cover((side), (x), (y)) >= 1))

/*! \brief Does side have advance?
 *
 * Does the \Side have this advance?
 * \param side is the pointer t the \Side.
 * \param a is the Advance for which to check.
 * \return TRUE if side has the advance.
 */
#define has_advance(side,a) ((side)->advance[(a)] == DONE)

/* Return true if a side's units can build the given type. */

/*! \brief Can side build unit type?
 *
 * Can the \Side build a Unit type?
 * \param side is the pointer t the \Side.
 * \param u is the Unit type for which to check.
 * \return TRUE if the unit type can be built.
 */
#define side_can_build(side,u)  \
  ((side)->canbuild != NULL ? (side)->canbuild[u] : TRUE)

/* Return true if a side's units can carry the given type. */

/*! \brief Can side carry unit type?
 *
 * Can the \Side carry a Unit type?
 * \param side is the pointer t the \Side.
 * \param u is the Unit type for which to check.
 * \return TRUE if the unit type can be carried.
 */
#define side_can_carry(side,u)  \
  ((side)->cancarry != NULL ? (side)->cancarry[u] : TRUE)

/* Return true if a side's units can develop the given type. */

/*! \brief Can side develop Unit?
 *
 * Can this side develop this Unit type?
 * \param side is the pointer t the \Side.
 * \param u is the Unit type for which to check.
 * \return TRUE if the Side can develp the Unit.
 */
#define side_can_develop(side,u)  \
  ((side)->candevelop != NULL ? (side)->candevelop[u] : TRUE)

/* Return true if a side can research the given type. */

/*! \brief Can reasearch Advance?
 *
 * Can the Side research this Advance?
 * \param side is the pointer t the \Side.
 * \param a is the Advance to be reshearched.
 * \return TRUE if the Advance my be researched.
 */
#define side_can_research(side,a)  \
  ((side)->canresearch != NULL ? (side)->canresearch[a] : TRUE)

/*! Is an independant side inactive?
 *
 * Check to see if a side is both independant and inactive.
 * \see side_has_display.
 * \param side is the pointer t the \Side.
 * \return TRUE if the side is independant, run by AI, and has
 *         a display.
 */
#define inactive_indepside(side)  \
  ((side) == indepside && !side_has_ai(side) && !side_has_display(side))

#ifdef DESIGNERS
#define is_designer(side) (side->designer)
#else /* DESIGNERS */
#define is_designer(side) (FALSE)
#endif /* DESIGNERS */

#define for_all_doctrines(d)  \
  for ((d) = doctrine_list; (d) != NULL; (d) = (d)->next)

/* Side-related variables. */

/*! \brief List of \Sides. */
extern Side *sidelist;
/*! \brief Last \Side in list. */
extern Side *lastside;
/*! \brief List of independant \Sides. */
extern Side *indepside;
/*! \brief Current \Side. */
extern Side *curside;
/*! \brief Temporary \Side pointer. */
extern Side *tmpside;

/*! \brief Number of \Sides. */
extern int numsides;
/*! \brief Total number of \Sides. */
extern int numtotsides;
/*! \brief Number of \Players. */
extern int numplayers;
#ifdef DESIGNERS
/*! \brief Number of designers. */
extern int numdesigners;
#endif /* DESIGNERS */

#if 0

/*! \brief Agreement.
 *
 * Definition of an agreement between sides. */
typedef struct a_agreement {
    short id;                 			/*!< a unique id */
    char *agtype;             		/*!< a descriptive general name */
    char *name;               		/*!< the specific name of this agreement */
    int state;                			/*!< is this agreement in effect */
    SideMask drafters;        		/*!< sides drafting the agreement */
    SideMask proposers;       		/*!< sides ready to propose the draft agreement */
    SideMask signers;         		/*!< proposed list of signers */
    SideMask willing;         		/*!< sides that have indicated agreement so far */
    SideMask knownto;         		/*!< sides that are aware of signed agreement */
    struct a_obj *terms;      		/*!< list of specific terms */
    short enforcement;        		/*!< true if program should try to enforce terms */
    struct a_agreement *next;	/*!< link to next agreement. */
} Agreement;

/*! \brief Agreement state. 
 *
 * Possible states that agreements may be in.
 */
enum {
    draft,           	/*!< agreement is circulating among drafters */
    proposed,     	/*!< agreement proposed to prospective signers */
    in_force,     	/*!< agreement is active */
    moribund   	/*!< agreement has expired */
};

/* Iteration over all agreements in the game. */

/*! \brief Iterate over agreements.
 *
 * for header to interate through all \ref a_agreement "Agreements".
 * \param v is the iteration variable (pointer to Agreement).
 */
#define for_all_agreements(v) for (v = agreement_list; v != NULL; v = v->next)

/*! \brief Is there any Agreement?
 *
 * Check for non NULL \ref a_agreement "Agreement" list.
 * \return TRUE if Agreement list is not NULL.
 */
#define any_agreements() (agreement_list != NULL)

/*! \brief Has side signed Agreement?
 *
 * \param side is the pointer to the \Side.
 * \param ag is the pointer to the \ref a_agreement "Agreement".
 * \return TRUE if side signed Agreement.
 */
#define side_signed_agreement(side,ag) ((ag)->signers[side_n(side)])

#define side_willing_agreement(side,ag) ((ag)->willing[side_n(side)])

/*! \brief Side knows of Agreement?
 *
 * Does the side know about the \ref a_agreement "Agreement"?
 * \param side is the pointer to the \ref a_sice "Side".
 * \param ag is the pointer to the \ref a_agreement "Agreement".
 * \return TRUE if side knows about the Agreement.
 */
#define side_knows_about_agreement(side,ag) ((ag)->knownto[side_n(side)])

/* Agreement-related variables. */

/*! \brief Number of agreements. */
extern int num_agreements;

/*! \brief List of agreements. */
extern Agreement *agreement_list;
/*! \brief Last agreement in list. */
extern Agreement *last_agreement;

#endif

/* Note: Can't use the "Unit" typedef below, must use "struct a_unit". */

extern int n_units_on_side(Side *side);

extern void init_sides(void);
extern Side *create_side(void);
extern void init_side_unithead(Side *side);
extern int side_has_units(Side *side);
extern void init_doctrine(Side *side);
extern void init_self_unit(Side *side);
extern int init_view(Side *side);
extern void calc_start_xy(Side *side);
extern const char *side_name(Side *side);
extern const char *side_adjective(Side *side);
extern int side_number(Side *side);
extern Side *side_n(int n);
extern Side *find_side_by_name(const char *str);
extern int side_controls_side(Side *side, Side *side2);
extern int side_controls_unit(Side *side, struct a_unit *unit);
extern int side_sees_unit(Side *side, struct a_unit *unit);
extern int side_sees_image(Side *side, struct a_unit *unit);
extern int occupants_visible(Side *side, struct a_unit *unit);
extern int num_units_in_play(Side *side, int u);
extern int num_units_incomplete(Side *side, int u);
extern struct a_unit *find_next_unit(Side *side, struct a_unit *prevunit);
extern struct a_unit *find_prev_unit(Side *side, struct a_unit *nextunit);
extern struct a_unit *find_next_actor(Side *side, struct a_unit *prevunit);
extern struct a_unit *find_prev_actor(Side *side, struct a_unit *nextunit);
extern struct a_unit *find_next_mover(Side *side, struct a_unit *prevunit);
extern struct a_unit *find_prev_mover(Side *side, struct a_unit *nextunit);
extern struct a_unit *find_next_awake_mover(Side *side,
					    struct a_unit *prevunit);
extern struct a_unit *find_prev_awake_mover(Side *side,
					    struct a_unit *nextunit);
extern int side_initacp(Side *side);
extern int side_acp(Side *side);
extern int side_acp_reserved(Side *side);
extern int using_tech_levels(void);
extern void remove_side_from_game(Side *side);
extern int num_displayed_sides(void);
extern int trusted_side(Side *side1, Side *side2);
extern int enemy_side(Side *s1, Side *s2);
extern int allied_side(Side *s1, Side *s2);
extern int neutral_side(Side *s1, Side *s2);
extern void reveal_side(Side *sender, Side *recipient, int *types);
extern void receive_message(Side *side, Side *sender, const char *str);
extern struct a_unit_view *unit_view_at(Side *side, int x, int y);
extern struct a_unit_view *unit_view_next(Side *side, int x, int y,
					  struct a_unit_view *uview);
extern struct a_unit *query_unit_from_uview(struct a_unit_view *uview);
extern struct a_unit_view *query_uvstack_from_unit(struct a_unit *unit);
extern struct a_unit_view *query_uvstack_at(int x, int y);
extern void all_see_occupy(struct a_unit *unit, int x, int y, int inopen);
extern void all_see_leave(struct a_unit *unit, int x, int y, int inopen);
extern void cover_area(Side *side, struct a_unit *unit,
		       struct a_unit *oldtransport, int x0, int y0,
		       int x1, int y1);
extern void reset_coverage(void);
extern void really_reset_coverage(void);
extern void calc_coverage(Side *side);
extern void reset_all_views(void);
extern void reset_view(Side *side);
extern void react_to_seen_unit(Side *side, struct a_unit *unit, int x, int y);
extern void all_see_cell(int x, int y);
extern int see_cell(Side *side, int x, int y);
extern void see_exact(Side *side, int x, int y);
extern int seen_border(Side *side, int x, int y, int dir);
extern void maybe_track(struct a_unit *unit);
extern void maybe_lose_track(struct a_unit *unit, int nx, int ny);
extern char *side_desig(Side *side);

extern Side *parse_side_spec(char *str);

extern int actual_advantage(Side *side);

extern Doctrine *new_doctrine(int id);
extern Doctrine *find_doctrine(int id);
extern Doctrine *find_doctrine_by_name(const char *name);
extern Doctrine *clone_doctrine(Doctrine *doctrine);
extern void set_doctrine(Side *side, const char *spec);

extern StandingOrder *new_standing_order(void);
extern void add_standing_order(Side *side, StandingOrder *sorder, int pos);
extern int parse_standing_order(Side *side, const char *cmdstr);
extern const char *parse_unit_types(Side *side, const char *str, char *utypevec);
extern const char *parse_order_cond(Side *side, const char *str, StandingOrder *sorder);
extern const char *get_next_arg(const char *str, char *buf, char **rsltp);
extern char *standing_order_desc(StandingOrder *sorder, char *buf);
#if 0
extern void init_agreements(void);
extern Agreement *create_agreement(int id);
extern Agreement *find_agreement(int id);
extern char *agreement_desig(Agreement *ag);
#endif
extern void fn_set_terrain_view(int x, int y, int val);
extern void fn_set_aux_terrain_view(int x, int y, int val);
extern void fn_set_terrain_view_date(int x, int y, int val);
extern void fn_set_aux_terrain_view_date(int x, int y, int val);
extern void fn_set_material_view(int x, int y, int val);
extern void fn_set_material_view_date(int x, int y, int val);
extern void fn_set_temp_view(int x, int y, int val);
extern void fn_set_temp_view_date(int x, int y, int val);
extern void fn_set_cloud_view(int x, int y, int val);
extern void fn_set_cloud_bottom_view(int x, int y, int val);
extern void fn_set_cloud_height_view(int x, int y, int val);
extern void fn_set_cloud_view_date(int x, int y, int val);
extern void fn_set_wind_view(int x, int y, int val);
extern void fn_set_wind_view_date(int x, int y, int val);

extern int load_side_config(Side *side);

extern void update_side_display_all_sides(Side *side, int rightnow);

extern int side_material_supply(Side * side, int m);
extern int side_material_production(Side * side, int m);
extern int side_material_storage(Side * side, int m);

extern int fn_test_side_in_sideclass(Obj *osclass, ParamBox *pbox);


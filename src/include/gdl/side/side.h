// xConq
// Lifecycle management and ser/deser of sides.

// $Id: side.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Lifecycle management and ser/deser of sides.
    \ingroup grp_gdl

    \note Many of the side macros check for a null side pointer.  This
	  is due to a null pointer being used to indicate that the unit was
	  independent.  That has been changed, now the independent units are
	  assigned to side 0.  The check for a null side pointer should not
	  be needed any longer, but has been left in for the time being.
	  In particular, the Side Mask equates a null side pointer and the
	  side 0 as being the same thing.

    \todo Get rid of ugly hack involving AI_Side.
*/

#ifndef XCONQ_GDL_SIDE_SIDE_H
#define XCONQ_GDL_SIDE_SIDE_H

#include "gdl/unit/unit.h"
#include "gdl/side/sidemask.h"
#include "gdl/side/unitview.h"
#include "gdl/side/cellview.h"
#include "gdl/side/doctrine.h"
#include "gdl/side/sorder.h"

// Function Macros: Queries

//! Does given side have treasury?
/*! Tests both that s can have a treasury and that m can be stored in it. */
#define side_has_treasury(s,m) \
    (s != indepside ? m_treasury(m) : g_indepside_has_treasury() ? m_treasury(m) : FALSE)

//! Is side running an AI locally?
#define side_has_local_ai(s) ((s)->ai != NULL)

//! Is given unit being tracked by given side?
#define side_tracking_unit(side,unit) (side_in_set((side), (unit)->tracking))

//! How many attacks for given side with given attacker against given defender?
#define side_atkstats(s,a,d) ((s)->atkstats[a] ? ((s)->atkstats[a])[d] : 0)

//! How many hits for given side with given attacker against given defender?
#define side_hitstats(s,a,d) ((s)->hitstats[a] ? ((s)->hitstats[a])[d] : 0)

// Function Macros: Game Area Layer Accessors

//! Get vision coverage of given cell for given side.
#define cover(s,x,y)  \
  ((s)->coverage ? aref((s)->coverage, x, y) : 0)

//! Set vision coverage of given cell for given side.
#define set_cover(s,x,y,v)  \
  ((s)->coverage ? aset((s)->coverage, x, y, v) : 0)

//! Add vision coverage at given cell for given side.
#define add_cover(s,x,y,v)  \
  ((s)->coverage ? aadd((s)->coverage, (x), (y), (v)) : 0)

//! Get alternate vision coverage of given cell for given side.
#define alt_cover(s,x,y)  \
  ((s)->alt_coverage ? aref((s)->alt_coverage, x, y) : 0)

//! Set alternate vision coverage of given cell for given side.
#define set_alt_cover(s,x,y,v)  \
  ((s)->alt_coverage ? aset((s)->alt_coverage, x, y, v) : 0)

// Iterator Macros

//! Iterate all sides.
/*! This is the new indepside-including version of for_all_sides.
    It has replaced for_all_side_plus_indep everywhere.
*/
#define for_all_sides(v)  \
  for (v = sidelist; v != NULL; v = v->next)

//! Iterate over player sides.
/*! This is the old for_all_sides which does not include indepside. */
#define for_all_real_sides(v)  \
  for (v = (sidelist ? sidelist->next : NULL); v != NULL; v = v->next)

//! Iterate over all units on side.
#define for_all_side_units(s,v) \
    for (v = (s)->unithead->next; v != (s)->unithead; v = v->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

//! Side.
/*! Each Xconq player is a "side" - more or less one country.  A side
    may or may not be played by a person, and may or may not have a
    display attached to it.  Each side has a different view of the
    world.  Side 0 is for independant units.
*/
typedef struct a_side {
    //! Unique ID
    int id;
    //! GDL Symbol
    Obj *symbol;
    //! Proper Name
    /*! This is the proper name of a side, as a country or alliance name.
	Examples include "Axis" and "Hyperborea".
    */
    char *name;
    //! Long Name
    /*! This is the long form of a side's name,
	as in "People's Republic of Hyperborea".
	Defaults to be the same as the side's name.
    */
    char *longname;
    //! Abbreviated Name
    /*! This is an short name or acronym for the side,
	often just the letters of the long name, as in "PRH".
    */
    char *shortname;
    //! Noun
    /*! This is the name of an individual unit or person belonging to the side.
	Defaults to "", which suppresses any mention of the side when
	(textually) describing the individual.
    */
    char *noun;
    //! Plural Noun
    /*! This is what you would call a group of individuals.
	Defaults to the most common plural form of the noun
	(in English, the default pluralizer adds an "s"),
	so any alternative plural noun, such as "Chinese",
	will need an explicit plural-noun value.
    */
    char *pluralnoun;
    //! Adjective
    /*! This is an adjective that can be used of individuals on the side,
	as in "Spanish". Defaults to "", which suppresses use of the adjective.
	As a complete example, a side named "Poland" would have a long name
	"Kingdom of Poland", short name "Po", noun "Pole", plural noun "Poles",
	and adjective "Polish".
    */
    char *adjective;
    //! List of colors that represent side.
    /*! Defaults to "black". */
    char *colorscheme;
    //! Default color to represent side (if no emblems or low magnification).
    char *default_color;
    //! Name of emblem image family.
    /*! An emblem name of "none" suppresses any emblem display for the side.
	Defaults to "", which gives the side a randomly-selected emblem.
    */
    char *emblemname;
    //! Array of unit namers for side.
    /*! This specifies which namers will be used with which types
	that the side starts out with or creates new units.
	These will not be run automatically on captured units or gifts.
    */
    char **unitnamers;
    //! Array of namers for features in side's initial country (if it has one).
    Obj *featurenamers;
    //! True, if names cannot be changed by player.
    short nameslocked;
    //! Name of class to which side belongs.
    char *sideclass;
    //! Unit that represents the side itself. An avatar.
    /*! Defaults to 0, which means that no unit represents the side. */
    struct a_unit *self_unit;
    //! ID of self unit.
    short self_unit_id;
    //! Side that controls the side.
    struct a_side *controlled_by;
    //! ID of controlling side.
    short controlled_by_id;
    //! Mask of sides that the side knows about.
    SideMask knows_about;
    //! Array of trust relationships to another side.
    short *trusts;
    //! Array of trade relationships to another side.
    short *trades;
    //! Array of initial units per unit type.
    short *startwith;
    //! Array of counts of units per unit type.
    short *counts;
    //! Array of tech levels per unit type.
    short *tech;
    //! Array of initial tech levels per unit type.
    short *inittech;
    //! Array of action priorities per unit type.
    short *action_priorities;
    //! Array of chances that units per unit type are already seen.
    short *already_seen;
    //! Array of chances that independent units per unit type are already seen.
    short *already_seen_indep;
    //! Default doctrine for side.
    Doctrine *default_doctrine;
    //! Array of doctrines per unit type.
    Doctrine **udoctrine;
    //! True, if doctrines are locked.
    short doctrines_locked;
    //! List of standing orders for units on a side.
    StandingOrder *orders;
    //! Last order in list of standing orders.
    StandingOrder *last_order;
    //! Bit vector of unit types that have standing orders.
    /*! \bug What if more unit types than width of bit vector? */
    char *uorders;
    //! GDL expression or list for possible units on a side.
    Obj *possible_units;
    //! True, if side's AI may resign or draw a game on its own.
    short ai_may_resign;
    //! Array of research per advance type.
    short *advance;
    //! Current research topic.
    short research_topic;
    //! True, if the next research topic is autoselected.
    short autoresearch;
    //! What is the longer term research goal for the side?
    short research_goal;
    //! Side treasuries per material type.
    long *treasury;
    //! Side's material conversion rates.
    short *c_rates;
    //! Array of cell views.
    char *terrview;
    //! Array of cell aux terrain views per aux terrain type.
    char **auxterrview;
    //! Array of cell view ages.
    short *terrviewdate;
    //! Array of cell aux terrain view ages per aux terrain type.
    short **auxterrviewdate;
    //! Array of unit views.
    struct a_unit_view **unit_views;
    //! Array of cell material views per material type.
    short **materialview;
    //! Array of cell material view ages per material type.
    short **materialviewdate;
    //! Array of cell temperature views.
    short *tempview;
    //! Array of cell temperature view ages.
    short *tempviewdate;
    //! Array of cell cloud views.
    short *cloudview;
    //! Array of cell cloud bottom views.
    short *cloudbottomview;
    //! Array of cell cloud height views.
    short *cloudheightview;
    //! Array of cell cloud view ages.
    short *cloudviewdate;
    //! Array of cell wind views.
    short *windview;
    //! Array of cell wind view ages.
    short *windviewdate;
    //! Is side participating in game?
    short ingame;
    //! True, if side ever participated in a turn.
    short everingame;
    //! Overall action priority of side.
    short priority;
    //! Side's outcome in game: -1/0/1 for lost/draw/won.
    short status;
    //! Array of scores per scorekeeper.
    short *scores;
    //! GDL object containing score data from scores file.
    Obj *rawscores;
    //! Is side willing to accept draws?
    short willingtodraw;
    //! Turn done when all units have acted.
    short autofinish;
    //! True, if side is done with turn.
    short finishedturn;
    //! Seconds used this turn.
    /*! \bug More than 20 hours on a turn may overflow this. */
    short turntimeused;
    //! Total number of seconds used.
    /*! \bug Too many seconds will overflow this. */
    short totaltimeused;
    //! Number of timeouts.
    short timeouts;
    //! Number of timeouts used.
    short timeoutsused;
    //! Current position in action vector.
    short curactor_pos;
    //! Unit that is currently acting.
    struct a_unit *curactor;
    //! ID of unit that is currently acting,
    short curactor_id;
    //! Advantage in initial units.
    short advantage;
    //! Minimum advantage that can be requested during setup.
    short minadvantage;
    //! Maximum advantage that can be requested during setup.
    short maxadvantage;
    //! X-center of preferred initial view.
    short init_center_x;
    //! Y-center of preferred initial view.
    short init_center_y;
    //! GDL object containing instructions to player about game setup.
    Obj *instructions;
    //! Last turn during which notice was posted.
    short last_notice_date;
    //! How long to wait before going ahead.
    short realtimeout;
    //! After this time, beep to signal next turn.
    long startbeeptime;
    //! Array of unit gains per unit type.
    short *gaincounts;
    //! Array of unit losses per unit type.
    short *losscounts;
    //! Array of attacks per unit types.
    long **atkstats;
    //! Array of successful attacks per unit types.
    long **hitstats;
    //! Side's player.
    struct a_player *player;
    //! ID of side's player.
    short playerid;
    //! Pointer to UI data.
    struct a_ui *ui;
    //! GDL object containing UI state.
    Obj *uidata;
    //! Type of AI.
    short aitype;
    //! Pointer to AI.
    struct a_ai *ai;
    //! GDL object containing AI state.
    Obj *aidata;
    //! Pointer to remote UI.
    struct a_rui *rui;
    //! Pointer to remote AI.
    struct a_rai *rai;
#if (0)
    //! Pointer to side's master AI struct.
    AI_Side *master_ai;
#endif
    //! Approximate X-center of side's country.
    short startx;
    //! Approximate Y-center of side's country.
    short starty;
    //! True, when side is busy and cannot be saved.
    short busy;
    //! Final radius of side's country. (?)
    short finalradius;
    //! True, if side is willing to allow game to be saved.
    short willingtosave;
    //! True, if side sees everything.
    /*! Right now this is just a cache for g_see_all().
	Note in particular that becoming a designer should not change this
value.
    */
    short see_all;
    //! True, if side's UI should display everything.
    /*! This is seeded from side->see_all,
	and also becomes true when designing or when the game is over.
    */
    short show_all;
    //! True, if the value of 'show_all' may be changed by the player.
    /*! This is normally off,
	but can be toggled while designing or after the game is over.
    */
    short may_set_show_all;
    //! True, if previous unit views were restored.
    short unit_view_restored;
#ifdef DESIGNERS
    //! True, if the side is in designer mode.
    short designer;
#endif /* DESIGNERS */
    //! Array of unit types allowed on side.
    short *uavail;
    //! Array of unit types that can be built by side.
    short *canbuild;
    //! Array of unit types that can be developed by side.
    short *candevelop;
    //! Array of unit types that side has transports for.
    short *cancarry;
    //! Array of advance types that can be researched.
    short *canresearch;
    //! Array of advances that cannot be researched by side.
    short *research_precluded;
    //! Array of utypes that side can act with.
    short *could_act_with;
    //! Array of utypes that side is restricted to constructing.
    short *could_construct;
    //! Array of utypes that side is restricted to developing.
    short *could_develop;
    //! Array of atypes that side is restricted to researching.
    short *could_research;
    //! List of units on side.
    struct a_unit *unithead;
    //! Array of units available to act.
    struct a_unitvector *actionvector;
    //! Number of units awaiting orders or plans.
    short numwaiting;
    //! Array of cell vision coverages.
    short *coverage;
    //! Array of cell vision coverages, accounting for altitude.
    short *alt_coverage;
    //! Array of units per unit type that side has.
    short *numunits;
    //! Number of units that side has available to act with during the turn.
    short numacting;
    //! Number of units that have finished acting for the turn.
    short numfinished;
    //! Real clock start time of turn.
    long turnstarttime;
    //! Time of last clock reset.
    long lasttime;
    //! Time to complete turn.
    long turnfinishtime;
    //! Real clock start time of previous turn.
    long laststarttime;
    //! Current score. (?)
    int point_value_cache;
    //! Is point value cache valid? (?)
    int point_value_valid;
    //! Current integer prefix for UI command.
    int prefixarg;
    //! Next side in list.
    struct a_side *next;
} Side;

//! AI operation hooks.
typedef struct a_ai_op {
    //! Name of AI.
    char *name;
    //! Help text about AI.
    char *help;
    //! ??
    int (*to_test_compat)(void);
    //! Hook: Initialize AI for given side.
    void (*to_init)(Side *side);
    //! Hook: Perform start-of-turn analysis/planning.
    void (*to_init_turn)(Side *side);
    //! Hook: Choose a plan for given unit.
    void (*to_decide_plan)(Side *side, Unit *unit);
    //! Hook: React to outcome of executed unit task.
    void (*to_react_to_task_result)(
        Side *side, Unit *unit, Task *task, TaskOutcome rslt);
    //! Hook: React to new side joining the game.
    void (*to_react_to_new_side)(Side *side, Side *side2);
    //! Hook: Adjust an unit's plan during turn.
    int (*to_adjust_plan)(Side *side, Unit *unit);
    //! Hook: Finalize action for turn.
    void (*to_finish_movement)(Side *side);
    //! Hook: Save state.
    Obj *(*to_save_state)(Side *side);
    //! Hook: Get ID of AI region at given coordinates.
    int (*region_at)(Side *side, int x, int y);
    //! Hook: Get designator of AI at given coordinates.
    char *(*at_desig)(Side *side, int x, int y);
} AI_ops;

//! Parameter box containing unit and side.
struct ParamBoxUnitSide : public ParamBoxUnit {
    //! Side.
    Side *side;
    //! Default constructor.
    ParamBoxUnitSide() : ParamBoxUnit() {
	pboxtype = PBOX_TYPE_UNIT_SIDE;
	side = NULL;
    }
};

//! Parameter box containing list of seers of unit on side.
struct ParamBoxUnitSideSeers : public ParamBoxUnitSide {
    //! List of units that see unit.
    SeerNode *seers;
    //! Default constructor.
    ParamBoxUnitSideSeers() : ParamBoxUnitSide () {
	pboxtype = PBOX_TYPE_UNIT_SIDE_SEERS;
	seers = NULL;
    }
    //! Default destructor.
    virtual ~ParamBoxUnitSideSeers() { seers = NULL; }
};

// Global Variables

//! List of sides.
extern Side *sidelist;
//! Last side in sides list.
extern Side *lastside;
//! Number of sides.
/*!
    The actual number of sides in a game.  This number never decreases;
    sides no longer playing need to be around for recordskeeping purposes.
*/
extern int numsides;
//! Total number of sides, including independents.
/*!
    This is the number of sides including indepside. We need it for the
    AI assignment in init.c. We want to keep numsides referring to
    normal sides only, since it is used a lot in the interface code,
    where we typically don't want to include indepside.
*/
extern int numtotsides;

//! The array of all possible AI operation hooks.
extern AI_ops *all_ai_ops;

// Global Variables: Uniques

//! Independant side.
extern Side *indepside;

// Global Variables: Game Setup

//! List of side defaults applied to all sides read subsequently.
extern Obj *side_defaults;

//! Used to generate the id number of the side.
extern int nextsideid;

// Global Variables: Behavior Options

extern int tmpcompress;

// Global Variables: Buffers

//! Scratch side.
extern Side *tmpside;

// Queries

//! Get side from ID number.
/*! \todo Should cache values in a table, do direct lookup. */
extern Side *side_n(int n);
//! Get side from name.
extern Side *find_side_by_name(char *str);

//! Get side number.
extern int side_number(Side *side);
//! Get side adjective.
extern char *side_adjective(Side *side);
//! Get side name.
/*!
    Get a char string naming the side.  Doesn't have to be pretty.
    \todo Should synth complete name/adjective from other parts of speech.
*/
extern char *side_name(Side *side);
//! Get side designator string.
/*! Make a printable identification of the given side.  This should be
    used for debugging and designing, not regular play.
*/
extern char *side_desig(Side *side);

//! Is 2nd given side trusted by the 1st one?
/*!
    Return TRUE if side1 trusts side2.  Note that the nature of trust
    is such that we don't want to check that the trust is mutual
    (although in practice it would be a foolish player who would trust
    a side that doesn't reciprocate!)
*/
extern int trusted_side(Side *side1, Side *side2);
//! Is 2nd given side controlled by the 1st one?
extern int side_controls_side(Side *side, Side *side2);
//! Is 2nd given side allied to 1st one?
extern int allied_side(Side *s1, Side *s2);

//! Does side own any observer in given unit?
/*! Test if the side has a seeing occupant inside a transport. */
extern int side_owns_viewer_in_unit(Side *side, Unit *unit);

//! True, if given name is in use on given side.
extern int name_in_use(Side *side, char *str);

// Visibility and Vision

//! Does given side see given unit?
extern int side_sees_unit(Side *side, struct a_unit *unit);
//! Does given side see given unit's occupants?
extern int occupants_visible(Side *side, struct a_unit *unit);

//! Alter area coverage on given side because of given unit.
/*!
    Unit's beady eyes are shifting from one location to another.  Since
    new things may be coming into view, we have to check and maybe draw
    lots of cells (but only need the one output flush, fortunately).
    \note LOS comes in here, to make irregular coverage areas.
*/
extern void cover_area(
    Side *side,
    struct a_unit *unit,
    struct a_unit *oldtransport,
    int x0, int y0,
    int x1, int y1);
//! Make all observers see unit leave cell.
/*!
    Some highly visible unit types cannot leave a cell without everybody
    knowing about the event.  The visibility is attached to the unit, not
    the cell, so first the newly-empty cell is viewed, then view coverage
    is decremented.
*/
extern void all_see_leave(struct a_unit *unit, int x, int y, int inopen);

//! Can given side see anything in given cell?
/*!
    Look at the given position, possibly not seeing anything.  Return
    true if a unit was spotted.
*/
extern int see_cell(Side *side, int x, int y);

//! Wake given unit and occs on given side.
extern void wake_unit(Side *side, Unit *unit, int forcewakeoccs);
//! Conditioanlly wake given unit and occs on given side.
/*! Conditionally attempt to wake up a unit based on restrictions on recursive
    waking.
*/
extern void selectively_wake_unit(
    Side *side, Unit *unit, int wakeoccs, int forcewakeoccs);
//! Wake appropriate side units if hostile unit spotted.
extern void react_to_seen_unit(Side *side, struct a_unit *unit, int x, int y);

//! Flush all stale views.
extern void flush_stale_views(void);

// Game Setup

//! Create a new side data structure.
extern Side *create_side(void);
//! Initialize side's list of units.
/*! To make the double links work, we have to have one pseudo-unit to
    serve as a head. This unit should not be seen by any outside code.
*/
extern void init_side_unithead(Side *side);

// GDL I/O

//! Read side properties from GDL form.
extern void fill_in_side(Side *side, Obj *props, int userdata);
//! Read side from GDL form.
extern void interp_side(Obj *form, Side *side);

//! Read side-value list from GDL list.
extern void interp_side_value_list(short *arr, Obj *lis);

//! Serialize side's AI state to GDL.
extern void ai_save_state(Side *side);
//! Serialize side's views layers to GDL.
extern void write_side_view(Side *side, int compress);
//! Serialize side properties to GDL.
extern void write_side_properties(Side *side);
//! Serialize sides to GDL.
extern void write_sides(struct a_module *module);
//! Serialize list of side-value pairs to GDL.
extern void write_side_value_list(
    char *name, short *arr, int dflt, int addnewline);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SIDE_SIDE_H

// xConq
// Lifecycle management and ser/deser of units.

// $Id: unit.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of units.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_H
#define XCONQ_GDL_UNIT_H

#include "gdl/lisp.h"
#include "gdl/media/imf.h"
#include "gdl/unit/plan.h"
#include "gdl/unit/unitview.h"
#include "gdl/side/sidemask.h"

// Macro Functions: Queries

//! Is unit of valid type?
#define is_unit(unit) ((unit) != NULL && is_unit_type((unit)->type))

//! Is unit alive?
#define alive(unit) ((unit)->hp > 0)

//! Is unit in play?
/*! This is true if the unit is on the board somewhere. */
#define in_play(unit) \
  (is_unit(unit) && alive(unit) && inside_area((unit)->x, (unit)->y))

//! Is unit complete?
#define completed(unit) \
  ((unit)->cp >= u_cp((unit)->type))

//! Is unit an ACP-independent type?
#define acp_indep(unit) u_acp_independent((unit)->type)

//! Get symbol associated with unit, if there is one.
#define unit_symbol(unit) ((unit)->extras ? (unit)->extras->sym : lispnil)

//! Unit altitude.
#define unit_alt(unit) (((unit)->z & 1) == 0 ? ((unit)->z >> 1) : 0)

//! Unit's point value.
#define unit_point_value(unit) \
    ((unit)->extras ? (unit)->extras->point_value : -1)

//! Turn unit is to appear on.
#define unit_appear_turn(unit) \
    ((unit)->extras ? (unit)->extras->appear : -1)

//! Turn unit is to disappear on.
#define unit_disappear_turn(unit) \
    ((unit)->extras ? (unit)->extras->disappear : -1)

//! Sides unit can be on.
#define unit_sides(unit) ((unit)->extras ? (unit)->extras->sides : lispnil)

//! Can unit pass the terrain at the cell?
#define impassable(u, x, y) (!could_be_on((u)->type, terrain_at((x), (y))))

//! Is unit a base?
#define isbase(u) (u_is_base((u)->type))

//! Is unit a base builder?
#define base_builder(u) (u_is_base_builder((u)->type))

//! Is unit a transport?
#define istransport(u) (u_is_transport((u)->type))

// Iterator Macros

//! Iterate over all units.
/*! \note Since it is possible for a unit to change sides and therefore
	  prev/next pointers while iterating using the macros below, one
	  must be very careful either that unit sides can't change during
	  the loop, or else to maintain a temp var that can be used to
	  repair the iteration.  This also applies to units dying.
*/
#define for_all_units(v)  \
    for (v = unitlist; v != NULL; v = v->unext)

//! Iterate over units at given position.
/*! Does not recurse into occupants. */
#define for_all_stack(x,y,var)  \
  for ((var) = unit_at((x), (y)); (var) != NULL;  (var) = (var)->nexthere)

//! Iterate over direct occupants of an unit.
/*! Iteration over all occupants of an unit (but not sub-occupants). */
#define for_all_occupants(u1,v) \
    for (v = (u1)->occupant; v != NULL; v = v->nexthere)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Unit.
/*! The Unit structure should be small, because there may be many of them.
    Unit semantics go in this structure, while unit brains go into the
    act/plan.  Test: a unit that is like a rock and can't do anything at all
    just needs basic slots, plan needn't be allocated.  Another test:
    unit should still function correctly after its current plan has been
    destroyed and replae`d with another.

    \note unit->side and unit->origside must always point to
	  valid side objects; dereferences are not checked!
*/
struct a_unit {
    //! Type ID.
    short type;
    //! Unique ID.
    int id;
    //! Specific name, if given.
    char *name;
    //! Created order of unit type on side.
    int number;
    //! Master view of unit.
    struct a_unit_view *uview;
    //! Associated image family.
    struct a_image_family *imf;
    //! Name of unit image, if given.
    char *image_name;
    //! X-position in world.
    short x;
    //! Y-position in world,
    short y;
    //! Z-position in world.
    short z;
    //! Side to which unit belongs.
    struct a_side *side;
    //! Side which originally owned unit.
    struct a_side *origside;
    //! Cuurent hitpoints.
    short hp;
    //! Calculated new hitpoints.
    short hp2;
    //! Construction points finished.
    short cp;
    //! Combat experience accrued.
    short cxp;
    //! Current morale.
    short morale;
    //! Containing unit, if any.
    struct a_unit *transport;
    //! Which sides are tracking unit?
    SideMask tracking;
    //! Unit supplies per material type.
    long *supply;
    //! Supply lines: how much supply received this turn.
    short s_flow;
    //! Supply lines: how well-connected unit is to supply zones.
    short s_conn;
    //! Tooling levels per unit type.
    short *tooling;
    //! Opinions of other sides per side.
    short *opinions;
    //! Actor state.
    struct a_actorstate *act;
    //! Current plan.
    struct a_plan *plan;
    //! Possible additional information.
    struct a_unit_extras *extras;
    //! AI info about unit.
    char *aihook;
    /* Following slots are never saved. */
    //! First occupant unit.
    struct a_unit *occupant;
    //! Next sibling occupant.
    struct a_unit *nexthere;
    //! Previous unit in stack.
    struct a_unit *prev;
    //! Next unit in stack.
    struct a_unit *next;
    //! Next unit in list of all units.
    struct a_unit *unext;
    //! Previous x coordinate.
    short prevx;
    //! Previous y coordinate.
    short prevy;
    //! GDL object containing read-in ID of container unit.
    /*! \todo transport_id should be tmp array instead */
    Obj *transport_id;
    //! Assorted flags.
    short flags;
    //! Abstract size of unit.
    /*! \todo Should make optional in UnitExtras? */
    short size;
    //! Reach into surrounding areas for materials acquisition.
    short reach;
    //! Number of cells used by unit.
    short usedcells;
    //! Maximum cells that unit can use.
    short maxcells;
    //! Advance that unit is currently researching.
    short curadvance;
    //! Abstract population of unit.
    long population;
    //! Cache of previous production.
    short *production;
    //! Construction points recovered from incomplete builds.
    short cp_stash;
    //! Is unit done with research this turn?
    short researchdone;
    //! Is unit considered to be in a busy state?
    /*! Broadcasted action that has not yet executed.

	\note this is NOT the same as has_pending_action(unit),
	which tells us that an action has been broadcasted and
	then confirmed by the host. The busy flag is set locally
	client-side long before that, when the action is first
	broadcasted. Its purpose is to filter out double-clicks
	which may cause commands to be issued to a dead unit.
    */
    short busy;
    //! ID of unit's unbuilt creation.
    int creation_id;
};

//! Unit extra options.
/*! The unit extras structure stores properties that are (usually)
    uncommon or special-purpose.  It is allocated as needed, access is
    through macros, and this does not appear as a separate structure
    when saving/restoring.  If a value is likely to be filled in for
    most or all units in several different game designs, then it should
    be in the main structure (even at the cost of extra space), rather
    than here.
*/
typedef struct a_unit_extras {
    //! Point value of individual unit.
    short point_value;
    //! Turn that unit should appear in game.
    short appear;
    //! X-coordinate of appearance location.
    short appear_var_x;
    //! Y-coordinate of appearance location.
    short appear_var_y;
    //! Turn that unit should disappear from game.
    short disappear;
    //! Individual unit priority.
    short priority;
    //! Symbol for xrefs.
    Obj *sym;
    //! List of possible sides.
    Obj *sides;
} UnitExtras;

//! Seer node: unit vision clarity counter.
/*!
    SeerNode is an useful struct for tracking who sees clearly,
    and who doesn't. Useful for 'see_cell' kinds of business.
*/
typedef struct a_seer_node {
    //! Unit that is seeing others.
    Unit *seer;
    //! Mistakes made in seeing others.
    int mistakes;
} SeerNode;

//! Parameter box containing an unit.
struct ParamBoxUnit : public ParamBox {
    //! Unit to search against.
    Unit *unit;
    //! Default constructor.
    ParamBoxUnit() { pboxtype = PBOX_TYPE_UNIT; unit = NULL; }
};

//! Parameter box containing an unit and location.
struct ParamBoxUnitAt : public ParamBoxUnit {
    //! X-position to search against.
    int x;
    //! Y-position to search against.
    int y;
    //! Default constructor.
    ParamBoxUnitAt() : ParamBoxUnit() {
	pboxtype = PBOX_TYPE_UNIT_AT;
	x = -1;  y = -1;
    }
};

//! Parameter box containing 2 units as part of search.
struct ParamBoxUnitUnit : public ParamBox {
    //! 1st unit as part of search.
    Unit *unit1;
    //! 2nd unit as part of search.
    Unit *unit2;
    //! Default constructor.
    ParamBoxUnitUnit() {
	pboxtype = PBOX_TYPE_UNIT_UNIT;
	unit1 = NULL;
	unit2 = NULL;
    }
};

//! Parameter box containing 2 units and list of seers.
struct ParamBoxUnitUnitSeers : public ParamBoxUnitUnit {
    //! List of seer units.
    SeerNode *seers;
    //! Default constructor.
    ParamBoxUnitUnitSeers() : ParamBoxUnitUnit () {
	pboxtype = PBOX_TYPE_UNIT_UNIT_SEERS;
	seers = NULL;
    }
    //! Default destructor.
    virtual ~ParamBoxUnitUnitSeers() { seers = NULL; }
};

// Global Variables

//! List of all units.
extern Unit *unitlist;
//! Number of units in existence.
extern int numunits;
//! Number of live units.
extern int numliveunits;

// Global Variables: Images

//! Number of images per unit type.
extern int *numuimages;
//! Image families per unit type.
extern ImageFamily **uimages;

// Global Variables: Advanced Unit Defaults

//! Advanced unit: default abstract size.
extern short default_size;
//! Advanced unit: default number of cells used.
extern short default_usedcells;
//! Advanced unit: default maximum number of cells to use.
extern short default_maxcells;
//! Advanced unit: default size of population.
extern long default_population;

// Global Variables: GDL I/O

//! Default unit GDL specification.
extern Obj *default_unit_spec;
//! List of unit GDL specifications.
extern Obj *unit_specs;
//! Last unit GDL specification.
extern Obj *last_unit_spec;

// Queries

//! Get short name for unit.
/*! Shorter unit description omits side name, but uses same buffer.
    This is mainly useful for describing the transport of a unit and
    suchlike, where the player will likely already know the side of the
    unit.
*/
extern char *short_unit_handle(Unit *unit);
//! Get medium-to-long name for given unit.
/*! This version lists the side but skips original side etc. */
extern char *medium_long_unit_handle(Unit *unit);
//! Get unit designator string.
/*! Short, unreadable, but greppable listing of unit.  Primarily useful
    for debugging and warnings.  We use several buffers and rotate between
    them so we can call this more than once in a single printf.
*/
extern char *unit_desig(Unit *unit);

//! Handle briefly describing any unit.
/*!
    This version allows the caller to supply a side other than the
    unit's actual side, such as when describing an out-of-date image of
    a unit that may have been captured.
*/
extern char *apparent_unit_handle(Side *side, Unit *unit, Side *side2);
//! Handle briefly describing side's unit.
/*!
    Build a short phrase describing a given unit to a given side,
    basically consisting of indication of unit's side, then of unit
    itself.
*/
extern char *unit_handle(Side *side, Unit *unit);

//! Find alive unit by ID.
/*! \todo This is used a lot, it should be sped up. */
extern Unit *find_unit(int n);
//! Find unit by ID whether it is dead or alive.
extern Unit *find_unit_dead_or_alive(int n);
//! Find unit with the given name, either alive or dead.
extern Unit *find_unit_by_name(char *nm);
//! Find unit with the given number, either alive or dead.
extern Unit *find_unit_by_number(int nb);
//! Find unit with given symbol, either dead or alive.
extern Unit *find_unit_by_symbol(Obj *sym);

//! Does 1st unit trust 2nd unit?
extern int unit_trusts_unit(Unit *unit1, Unit *unit2);

//! Could unit type in given terrain type?
extern int type_survives_in_terrain(int u, int t);
//! Could unit type in given cell?
extern int type_survives_in_cell(int u, int nx, int ny);
//! Could unit type occupy given cell?
/*!
    Return true if the given unit type can fit onto the given cell.
    \todo Should eventually account for variable-size units
*/
extern int type_can_occupy_cell(int u, int x, int y);
//! Can given unit type occupy given container unit?
extern int type_can_occupy(int u, Unit *transport);
//! Can given unit occupy given container unit?
extern int can_occupy(Unit *unit, Unit *transport);

// Sorting

//! Sort units.
/*! Do a bubble sort.
    Data is generally coherent, so bubble sort not too bad if we allow
    early termination when everything is already in order.

    \note If slowness objectionable, replace with something clever, but be
          sure that average performance in real games is what's being improved.
*/
extern void sort_units(int byidonly);

// Vision and Visibility

//! Chance of one unit type seeing another at a given distance.
/*! \todo Should go 'types.h', not here. */
extern int see_chance(int u, int u2, int dist);
//! Chance of one unit seeing another.
extern int see_chance(Unit *seer, Unit *tosee);

// Position Manipulation

//! Enter cell.
extern int enter_cell(Unit *unit, int x, int y);
//! Place into container unit.
extern void enter_transport(Unit *unit, Unit *transport);
//! Leave cell into limbo.
/*!
    Unit departs from a cell by zeroing out pointer if in cell or by being
    removed from the list of transport occupants.
    Dead units (hp = 0) may be run through here, so don't error out.
*/
extern void leave_cell(Unit *unit);
//! Remove from container unit.
extern void leave_transport(Unit *unit);

// Lifecycle Management

//! Create unfilled-out unit of given type.
/*! The primitive unit creator, called by regular creator and also used to
    make the dummy units that sit at the heads of the per-side unit lists.
*/
extern Unit *create_bare_unit(int type);
//! Create unit of given type, maybe with AI control.
/*! The regular unit creation routine.  All the slots should have something
    reasonable in them, since individual units objects see a lot of reuse.
*/
extern Unit *create_unit(int type, int makebrains);
//! Initialize unit's extras, if necessary.
/*! Allocate and fill in the unit extras.  This is needed whenever any
    one of the extra properties is going to get a non-default value.
    (Normally, the accessors for this structure return a default value
    if one is not present in a unit.)
*/
extern void init_unit_extras(Unit *unit);
//! Give unit supplies that it would have at start of game.
extern void init_supply(Unit *unit);
//! Initialize unit's tooling array.
extern void init_unit_tooling(Unit *unit);
//! Initialize or resize unit's side opinions array.
extern void init_unit_opinions(Unit *unit, int nsides);

//! Get rid of all dead units at once.
/*!
   This routine is basically a garbage collector, and should not be
   called during a unit list traversal. The process starts by finding
   the first live unit, making it the head, then linking around all in
   the middle.  Dead units stay on the dead unit list for each side
   until that side has had a chance to move.  Then they are finally
   flushed in a permanent fashion.
*/
extern void flush_dead_units(void);

// Images

//! Bind unit type image to unit.
extern void set_unit_image(Unit *unit);

// Sides

//! Bind unit to side.
/*!
    Put the given unit on the given side, without all the fancy
    effects.  Important to handle independents, because this gets
    called during init.  This is the only way that a unit's side may be
    altered.
    \note This may be run on dead units, as part of clearing out a
    side's units, in which case we just want to relink, don't care
    about testing whether the type is allowed or not.
*/
extern void set_unit_side(Unit *unit, Side *side);

//! Record original side of unit.
/*!
    The origside is more of a historical record or cache, doesn't need
    the elaboration that unit side change does.
*/
extern void set_unit_origside(Unit *unit, Side *side);

//! GDL Unit I/O

//! Read unit defaults from GDL form.
extern void interp_unit_defaults(Obj *form);
//! Read unit from GDL form.
extern Unit *interp_unit(Obj *form);

//! Serialize unit properties to GDL.
/*! Write various properties, but only if they have non-default values. */
extern void write_unit_properties(Unit *unit);
//! Serialize units to GDL.
/*!
    \todo Should write out "unit groups" with dict prepended, then can use with
          multiple games
*/
extern void write_units(struct a_module *module);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_H

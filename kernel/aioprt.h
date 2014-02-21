/* Operational Roles and Analysis for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Operational Roles and Analysis for AIs

    Part of the AI API, Level 4. 

    Provides an operational level AI implementation.

    \note Nothing in this file should ever be required to implement an AI; 
	    everything here is optional.

*/

namespace Xconq {
namespace AI {

//! Types of Operational Roles

enum OpRole_Type {
    //! No operational role.
    OR_NONE = 0,
    //! Operate as a shuttle for other units.
    OR_SHUTTLE,
    //! Operate as a constructor for other units.
    OR_CONSTRUCTOR,
    /* (TODO: Insert new oproles here.) */
    OR_Total
};

//! Operational Role Outcomes

enum OpRole_Outcome {
    //! Oprole was invalid.
    ORO_INVALID = 0,
    //! Oprole executed with failures.
    ORO_FAILED,
    //! Oprole executed without failures.
    ORO_OK,
    //! Oprole executed a tactical handler.
    ORO_HANDLED_TACTICAL
    // TODO: Insert new oprole outcomes here.
};

//! Operational Role

struct OpRole {
    /* Basic Info */
    //! Unit ID
    int id;
    //! Type of Operational Role
    OpRole_Type type;
    /* Extended Info */
    //! Dedication to role.
    int dedication;
    //! Execs this turn.
    int execs_this_turn;
    //! Failures this turn.
    int fails_this_turn;
    /* (TODO: Graft in tasks queue here.) */
    /* Overhead */
    //! Next oprole in pool.
    OpRole *next;
    //! Next oprole in hash bucket.
    OpRole *next_by_type;
    //! Back-pointer to side.
    Side *side;
};


/* TEMPORARY: Side-related AI management.
    Should be moved to new files in the future. */

//! Per-side master AI structure.

struct AI_Side {
    /* Scorekeeper-related information. */
    //! List of scorekeeper analyses.
    AI_SKAnalysis *sk_analyses;
    /* Operational roles. */
    //! List of in-use opertional role nodes.
    OpRole *oproles;
    //! List of unused operational role nodes.
    OpRole *oproles_free;
    //! Hash buckets of operational role nodes.
    OpRole **oprole_buckets;
    /* Overhead. */
    //! Back-pointer to side.
    Side *side;
};

//! Create/init master AI structure associated with a side.
extern void create_side_ai(Side *side);
//! Return master AI structure associated with a side.
extern AI_Side * get_side_ai(Side *side);

/** Management of Operational Roles **/

//! Number of OpRole nodes per allocation block.
#define OR_BLK_SZ	200

//! Iterate through all oproles on a side.

#define for_all_oproles(side,oprole) \
    for ((oprole) = (get_side_ai(side) ? get_side_ai(side)->oproles : NULL); \
	 (oprole); (oprole) = (oprole)->next)

//! Iterate through all oproles on a side for a given type.

#define for_all_oproles_by_type(side,type,oprole) \
    for ((oprole) = (get_side_ai(side) ? \
			get_side_ai(side)->oprole_buckets[type] : NULL); \
	 (oprole); (oprole) = (oprole)->next_by_type)

//! Allocate a new block of oproles in pool.
extern void allocate_oproles(Side *side, int nodesnum = OR_BLK_SZ);
//! Acquire oprole.
extern OpRole *acquire_oprole(Side *side, int id, OpRole_Type type);
//! Release oprole.
extern void release_oprole(OpRole *oprole);
//! Find oprole associated with a particular unit ID.
extern OpRole *find_oprole(Side *side, int id);

/** Handling of Operational Roles **/

/* Construction */

//! Choose utype to construct, and optionally return its relative worth.
extern int choose_utype_to_construct(OpRole *oprole, int *uscore);
//! Choose transport to construct given utype in.
extern Unit *choose_transport_to_construct_in(
    int u2, Side *side, UnitView *uview, int upchain = FALSE);
//! Generate an appropriate construction task given constructor and utype.
extern Task *generate_construction_task(Unit *unit, int u2);
//! Choose a construction task to perform.
extern int choose_construction_or_repair(OpRole *oprole);
//! Handle constructor operational role.
extern OpRole_Outcome handle_constructor_oprole(OpRole *oprole);

/* General Operational Role Handling */

//! Evaluate an operational role.
extern void handle_oprole(OpRole *oprole);
//! Evaluate all operational roles on a side.
extern void handle_oproles(Side *side);

} // namespace Xconq::AI
} // namespace Xconq

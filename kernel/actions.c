/* Implementations of Xconq actions.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/actions.c
 * \brief Implementations of Xconq actions.
 *
 * The general theory of actions is that interface or AI code calls, for
 * an action foo, the routine prep_foo_action, which just records the action
 * for later execution.  The action loop in run_game eventually calls
 * do_foo_action, which first calls check_foo_action to confirm that the
 * action will succeed.  If check_foo_action does not find any errors, then
 * the action cannot fail.  The main body of do_foo_action then implements
 * the effects of the action.  Interfaces may call check_foo_action freely,
 * but should never call do_foo_action directly.
 * 
 * action.def is used to declare the actions as extern int, as well
 * as defining the action default function array, actiondefns.  A macro,
 * ACTION_DEF is redefined to use different portions of it's arguments
 * to define the different types of information to this module.
 *
 * Each action has an acting unit, which is the unit that schedules the action 
 * and supplies the ACP to perform the action. Each action also has an "actee",
 * a unit that actually performs the action (extractor, builder, creator, etc).
 * The two units are currently always identical, but the theoretical possibility 
 * of splitting the work in this way is supported by two separate unit arguments
 * in all action functions. It is the status, position, capabilities and available
 * supplies of the "actee" that determines if, how and to what extent the action 
 * can be performed, but the action is also limited by the ACPs that are available 
 * to the acting unit. Note that the "actee" is called so for historical reasons - a
 * better name would be "actor" since it in most cases is the active unit. One
 * exception is the materials transfer code, where the "actee" is the donor (not
 * necessarily active) of the transferred materials. Another example is the
 * disband action where the "actee" is the unit being disbanded. Other parameters 
 * may be defined.
 *
 * \note We can't declare all the action functions as static 
 * because some of them are in other files, but don't 
 * let them be visible to all files. 
 * 
 * \note <P>The DEF_ACTION macro and it's like can't be documented directly
 * with Doxygen, as it doesn't support overlaid macros, or macro substitiution
 * in comments.  So where a single definition file is used for several purposes,
 * the documentation has to be manually included each time, which kind of 
 * defeats the purpose of having the definions in a single file.  One
 * possibility would be to use a pre-processor like m4 to create a set
 * of documentation includes, like action_function.doc to be included.
 * It's probably not worthwile for external function declarations, since
 * they'll be documented where they are defined, but it might be nice for
 * things like enumerations or fixed data structures.
 */

#include "conq.h"
#include "kernel.h"

using namespace Xconq;

extern int at_turn_start;

static int creator_always_dies_on_creation(int u, int new_u);
static void set_created_unit_props(Unit *newunit, Unit *creator, Side *side);
//! Checks that are common to both explicit and auto-repair.
static int can_any_repair_common(Unit *actor, Unit *repairer, Unit *repairee);
static int give_away(Unit *unit, int m, int amt);
static void notify_action(Unit *unit, Action *action);
static void play_action_movies(Unit *unit, Action *action);
static void adjust_tech_crossover(Side *side, int u);
static void adjust_tooling_crossover(Unit *unit, int u2);
static void distribute_material(Unit *unit, int m, int amt);
static void notify_tech(Side *side, int u, int oldtech, int newtech);
static void notify_tp(Side *side, Unit *unit, int u2, int oldtp, int newtp);
static int extract_one_round(Unit *extractor, int x, int y, int m, int amount);

#undef  DEF_ACTION

#define DEF_ACTION(name,code,args,prepfn,netprepfn,DOFN,checkfn,ARGDECL,doc)  \
    extern int DOFN ARGDECL;

#include "action.def"

#undef  DEF_ACTION
#define DEF_ACTION(NAME,CODE,ARGS,prepfn,netprepfn,dofn,checkfn,argdecl,doc)  \
    { CODE, NAME, ARGS },

/*! \brief Action function table.
 *
 * The table of all the action functions declared above.  It uses 
 * CODE_DEF_ACTION and action.def above to generate an "extern int"
 * declaration for every action (prep, do, and * check).  Below it
 * generates a table with 3 entries for each function, a code number
 * (CODe), the name of the function (NAME), and the extended
 * declaration list for the function (ARGS).
 */
ActionDefn actiondefns[] = {

#include "action.def"

    { (ActionType)-1, NULL, NULL }
};

/*! \brief Retreat flag.
 *
 * This is used to indicate that a move is a retreat; for normal 
 * movement it will always be false.
 */
int retreating;

/*! \brief Type of Unit from which a Unit is retreating.
 *
 * This is a specific type of Unit that the retreater is running away from.
 */
int retreating_from = NONUTYPE;

/*! \brief Action description buffer.
 *
 * This is used to compose a description of what a Unit is doing.
 */
char *actiondesigbuf = NULL;

extern Action *latest_action;

/* Cache variables. */
int *cache__type_max_night_acp_from_any_terrains = NULL;
int *cache__type_max_acp_from_any_occs = NULL;
int *cache__type_max_acp = NULL;

int
could_take_from_treasury(int u, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return (um_takes_from_treasury(u, m) && m_treasury(m) 
	    && (0 < g_treasury_size()));
}

int
could_take_from_treasury(int u, Side *side, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(side, "Attempted to access a NULL side");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return ((side == indepside) && !g_indepside_has_treasury() ? FALSE
	    : could_take_from_treasury(u, m));
}

int
could_take_directly_from_treasury(int u, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return (!um_storage_x(u, m) && could_take_from_treasury(u, m));
}

int
could_take_directly_from_treasury(int u, Side *side, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(side, "Attempted to access a NULL side");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return ((side == indepside) && !g_indepside_has_treasury() ? FALSE
	    : could_take_directly_from_treasury(u, m));
}

int
could_give_to_treasury(int u, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return (um_gives_to_treasury(u, m) && m_treasury(m) 
	    && (0 < g_treasury_size()));
}

int
could_give_to_treasury(int u, Side *side, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(side, "Attempted to access a NULL side");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return ((side == indepside) && !g_indepside_has_treasury() ? FALSE
	    : could_give_to_treasury(u, m));
}

int
could_give_directly_to_treasury(int u, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return (!um_storage_x(u, m) && could_give_to_treasury(u, m));
}

int
could_give_directly_to_treasury(int u, Side *side, int m)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(side, "Attempted to access a NULL side");
    assert_error(is_material_type(m),
                 "Attempted to manipulate an invalid mtype");
    return ((side == indepside) && !g_indepside_has_treasury() ? FALSE
	    : could_give_directly_to_treasury(u, m));
}

void
get_materials_availability(
    Unit *actor, Unit *agent, long *p_mavail, long *p_mavailmax, int flags)
{
    int u2 = NONUTYPE;
    Side *side = NULL, *side2 = NULL, *tside = NULL;
    int m = NONMTYPE;

    assert_error(in_play(actor),
		 "Materials Check: Attempted to access an out-of-play actor");
    assert_error(in_play(agent),
		 "Materials Check: Attempted to access an out-of-play agent");
    assert_error(p_mavail,
		 "Materials Check: Attempted to access a NULL materials array");
    assert_error(p_mavailmax,
		 "Materials Check: Attempted to access a NULL materials array");
    // Useful info.
    u2 = agent->type;
    side = actor->side;
    side2 = agent->side;
    // Process flags.
    tside = (flags & MReq_AGENT_TREASURY ? side2 : side);
    // Fill out the materials arrays.
    for_all_material_types(m) {
	if (could_take_directly_from_treasury(u2, tside, m)) {
	    p_mavail[m] = tside->treasury[m];
	    p_mavailmax[m] = g_treasury_size();
	}
	else if (could_take_from_treasury(u2, tside, m)) {
	    p_mavail[m] = 
		min(agent->supply[m] + tside->treasury[m], um_storage_x(u2, m));
	    p_mavailmax[m] = um_storage_x(u2, m);
	}
	else {
	    p_mavail[m] = agent->supply[m];
	    p_mavailmax[m] = um_storage_x(u2, m);
	}
    }
}

int
can_meet_materials_requirement(
    long *p_mreq, long *p_mavail, long *p_mavailmax)
{
    int m = NONMTYPE;

    assert_error(p_mreq,
		 "Materials Check: Attempted to access a NULL materials array");
    assert_error(p_mavail,
		 "Materials Check: Attempted to access a NULL materials array");
    assert_error(p_mavailmax,
		 "Materials Check: Attempted to access a NULL materials array");
    // For each material, see if requirement can be met by availabilities.
    for_all_material_types(m) {
	// If requirement > what can ever be available...
	if (p_mreq[m] > p_mavailmax[m])
	    return A_ANY_CANNOT_DO;
	// If requirement > what is currently available...
	if (p_mreq[m] > p_mavail[m])
	    return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

void
consume_materials(
    Unit *actor, Unit *consumer, int (*getter)(int u, int m), int u3, int mult)
{
    static long *p_mreq;

    int u2 = NONUTYPE;
    int m = NONMTYPE;

    assert_error(in_play(actor), 
"Materials Consumer: Attempted to access an out-of-play actor");
    assert_error(in_play(consumer), 
"Materials Consumer: Attempted to access an out-of-play consumer");
    assert_error(getter, 
		 "Materials Consumer: Attempted to use a NULL getter function");
    u2 = consumer->type;
    // Initialize materials request array, if necessary.
    if (!p_mreq)
	p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    // Fill out materials vector before passing to other 'consume_materials'.
    for_all_material_types(m)
	p_mreq[m] = getter(is_unit_type(u3) ? u3 : u2, m) * mult;
    consume_materials(actor, consumer, p_mreq);
}

void
consume_materials(Unit *actor, Unit *consumer, long *p_mreq)
{
    int u2 = NONUTYPE;
    int m = NONMTYPE;
    Side *side = NULL;
    long amtreq = 0, camtavail = 0, tamtavail = 0, cstorcap = 0, tsize = 0;

    assert_error(in_play(actor), 
"Materials Consumer: Attempted to access an out-of-play actor");
    assert_error(in_play(consumer), 
"Materials Consumer: Attempted to access an out-of-play consumer");
    assert_error(p_mreq, 
		 "Materials Consumer: Encountered NULL consumption vector");
    u2 = consumer->type;
    side = actor->side;
    for_all_material_types(m) {
	/* Get amount requested. */
	amtreq = p_mreq[m];
	/* If none requested, then no further processing for material. */
	if (!amtreq)
	  continue;
	/* Get amount available in consumer. */
	camtavail = consumer->supply[m];
	/* Get consumer's storage capacity for material. */
	cstorcap = um_storage_x(u2, m);
	/* Get treasury amount, if any. */
	if (side_has_treasury(side, m) && um_takes_from_treasury(u2, m))
	  tamtavail = side->treasury[m];
	else
	  tamtavail = 0;
	/* Get treasury's size. */
	if (side_has_treasury(side, m) && um_gives_to_treasury(u2, m))
	  tsize = g_treasury_size();
	else
	  tsize = 0;
	/* If request is positive. */
	if (0 < amtreq) {
	    /* Consume consumer supply. */
	    if (amtreq <= camtavail) {
		consumer->supply[m] -= amtreq;
		continue;
	    }
	    else {
		amtreq -= camtavail;
		consumer->supply[m] = 0;
	    }
	    /* Consume treasury supply. */
	    side->treasury[m] -= amtreq;
	}
	/* Else request is negative. */
	else {
	    /* Produce consumer supply, if possible. */
	    if (-amtreq <= (cstorcap - camtavail)) {
		consumer->supply[m] -= amtreq;
		continue;
	    }
	    else {
		amtreq += (cstorcap - camtavail);
		consumer->supply[m] = cstorcap;
	    }
	    /* Produce treasury supply, if possible. */
	    if (0 < tsize) {
		side->treasury[m] = min(-amtreq + tamtavail, tsize);
		amtreq = tsize - (-amtreq + tamtavail);
		/* (TODO: Try something with any remaining excess?) */
	    }
	}
    }
}

//! Does unit has enough of each material needed to act?

int
has_supply_to_act(Unit *unit)
{
    int m = NONMTYPE;

    for_all_material_types(m) {
	if (unit->supply[m] < um_to_act(unit->type, m))
	  return FALSE;
    }
    return TRUE;
}

/* Unit/utype status/analysis functions. */

/*! \todo Eventually assert activeness instead of just returning an error. */

int
can_act(Unit *actor, Unit *agent)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;

    // Only active units can act.
    if (!is_active(actor))
	return A_ANY_ERROR;
    if (!is_active(agent))
	return A_ANY_ERROR;
    u = actor->type;
    u2 = agent->type;
    side = actor->side;
    // Is actor's side allowed to act with given agent utype?
    if (!side->could_act_with[u2])
	return A_ANY_CANNOT_DO;
    // Does agent have enough material to act?
    if (!has_supply_to_act(agent))
	return A_ANY_NO_MATERIAL;
    return rslt;
}

int
has_enough_tooling(Unit *constructor, int u2)
{
    assert_error(is_active(constructor),
		 "Toolup Check: Attempted to access an inactive constructor");
    assert_error(is_unit_type(u2),
		 "Toolup Check: Encountered invalid unit type");
    return ((constructor->tooling ? constructor->tooling[u2] : 0)
	    >= uu_tp_to_build(constructor->type, u2));
}

int
could_toolup_for(int u, int u2)
{
    assert_error(is_unit_type(u),
		 "Toolup Check: Encountered invalid unit type");
    assert_error(is_unit_type(u2),
		 "Toolup Check: Encountered invalid unit type");
    return (uu_toolup_for(u, u2) && (0 < uu_tp_per_toolup(u, u2))
	    && (uu_tp_max(u, u2) >= uu_tp_to_build(u, u2)));
}

int
could_toolup_for_any(int u)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u),
		 "Toolup Check: Encountered invalid unit type");
    for_all_unit_types(u2) {
	if (could_toolup_for(u, u2))
	    return TRUE;
    }
    return FALSE;
}

int
can_toolup_for(Unit *actor, Unit *constructor, int u3)
{
    static long *p_mavail, *p_mavailmax, *p_mreq;

    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int acpcost = 0, tpt = 0;
    int m = NONMTYPE;

    assert_error(is_active(actor),
                 "Toolup Check: Attempted to manipulate an inactive actor");
    assert_error(is_active(constructor),
"Toolup Check: Attempted to manipulate an inactive constructor");
    assert_error(is_unit_type(u3),
                 "Toolup Check: Encountered invalid unit type to toolup for");
    // Can the creator act?
    if (!valid(rslt = can_act(actor, constructor)))
      return rslt;
    // Some useful info.
    u = actor->type;
    u2 = constructor->type;
    side = actor->side;
    tpt = uu_tp_per_toolup(u2, u3);
    // Maybe we are already tooled up or don't have any tooling up to do?
    if (has_enough_tooling(constructor, u3))
	return A_ANY_OK;
    // Could the constructor's utype toolup for the requested utype?
    if (!could_toolup_for(u2, u3))
	return A_ANY_CANNOT_DO;
    // Initialize materials arrays, if necessary.
    if (!p_mavail)
        p_mavail = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mavailmax)
        p_mavailmax = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mreq)
        p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    // Possibly check against ACP.
    if (!u_acp_independent(u2) 
	&& (0 < (acpcost = uu_acp_to_toolup(u2, u3)))) {
        // Can the ACP source ever provide enough ACP?
        if (!can_have_enough_acp(actor, acpcost))
            return A_ANY_CANNOT_DO;
        // Does the ACP source have enough ACP to support toolup?
        if (!has_enough_acp(actor, acpcost))
            return A_ANY_NO_ACP;
    }
    // Check if utype is allowed, and if needed tech & advances exist.
    if (!side_can_build(side, u3))
        return A_ANY_CANNOT_DO;
    // Compare materials availability and requirements.
    get_materials_availability(actor, constructor, p_mavail, p_mavailmax);
    for_all_material_types(m) {
        p_mreq[m] =
            max(um_to_toolup(u2, m),
                um_consumption_per_tooledup(u3, m)
                + um_consumption_per_toolup(u2, m)
		+ tpt * um_consumption_per_tp(u2, m));
    }
    if (!valid(
            rslt =
                can_meet_materials_requirement(p_mreq, p_mavail, p_mavailmax)))
        return rslt;
    return A_ANY_OK;
}

//! Materials requirement for build or create-as-build action.

static int
materials_req_for_build(int u2, int u3, int m, int cp)
{
    assert_error(is_unit_type(u2),
		 "Construct Check: Encountered invalid constructor unit type");
    assert_error(is_unit_type(u3),
		 "Construct Check: Encountered invalid constructee unit type");
    assert_error(is_material_type(m),
"Construct Check: Encountered invalid construction material type");
    return max(um_to_build(u2, m),
	       (um_consumption_per_cp(u3, m) * cp)
		+ um_consumption_per_built(u3, m) 
		+ um_consumption_per_build(u2, m));
}

int
could_create(int u, int u2)
{
    assert_error(is_unit_type(u),
		 "Create Check: Encountered invalid creator unit type");
    assert_error(is_unit_type(u2),
		 "Create Check: Encountered invalid creation unit type");
    if (uu_create_as_build(u, u2)) {
	if (!uu_build(u, u2) || (0 > uu_cp_per_build(u, u2)))
	    return FALSE;
    }
    else {
	if (!uu_create(u, u2) || (0 > uu_creation_cp(u, u2)))
	    return FALSE;
    }
    if ((0 < uu_tp_to_build(u, u2)) && !could_toolup_for(u, u2))
	return FALSE;
    return TRUE;
}

int
could_create_any(int u)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u),
		 "Create Check: Encountered invalid creator unit type");
    for_all_unit_types(u2) {
	if (could_create(u, u2))
	    return TRUE;
    }
    return FALSE;
}

int
can_create_common(
    Unit *actor, Unit *creator, int u3, int x, int y, 
    int could, int acpcost_spcl, long *p_mreq_spcl)
{
    static long *p_mavail, *p_mavailmax, *p_mreq;

    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int acpcost = 0, cp = -1;
    int m = NONMTYPE;
    int cab = FALSE;

    assert_error(is_active(actor),
		 "Create Check: Attempted to manipulate an inactive actor");
    assert_error(is_active(creator),
		 "Create Check: Attempted to manipulate an inactive creator");
    assert_error(is_unit_type(u3), 
		 "Create Check: Encountered invalid unit type to create");
    if (!inside_area(x, y))
	return A_MOVE_CANNOT_LEAVE_WORLD;
    // Can the creator act?
    if (!valid(rslt = can_act(actor, creator)))
      return rslt;
    u = actor->type;
    u2 = creator->type;
    side = actor->side;
    cab = uu_create_as_build(u2, u3);
    // Can the creator's utype create the requested utype? 
    if (!could && !could_create(u2, u3))
	return A_ANY_CANNOT_DO;
    // Is actor's side allowed to construct requested utype?
    if (!side->could_construct[u3])
	return A_ANY_CANNOT_DO;
    // Initialize materials arrays, if necessary.
    if (!p_mavail)
	p_mavail = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mavailmax)
	p_mavailmax = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mreq)
	p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    // Is the distance within creation range?
    if (distance(creator->x, creator->y, x, y) 
	> (cab ? uu_build_range(u2, u3) : uu_create_range(u2, u3))) 
	return A_ANY_TOO_FAR;
    // Possibly check against ACP.
    if (0 > acpcost_spcl)
	acpcost = (cab ? uu_acp_to_build(u2, u3) : uu_acp_to_create(u2, u3));
    else
	acpcost = acpcost_spcl;
    if (!u_acp_independent(u2) && (0 < acpcost)) {
	// Can the ACP source ever provide enough ACP? 
	if (!can_have_enough_acp(actor, acpcost)) 
	    return A_ANY_CANNOT_DO;
	// Does the ACP source have enough ACP to support creation? 
	if (!has_enough_acp(actor, acpcost)) 
	    return A_ANY_NO_ACP;
    }
    // Check if utype is allowed, and if needed tech & advances exist. 
    if (!side_can_build(side, u3)) 
	return A_ANY_CANNOT_DO;
    // Check if we hit a limit on the number of units. 
    if (!new_unit_allowed_on_side(u3, side)) 
	return A_ANY_CANNOT_DO;
    // Check the tooling.
    if (!valid(can_toolup_for(actor, creator, u3)))
	return A_ANY_CANNOT_DO;
    if (!has_enough_tooling(creator, u3))
	return A_CONSTRUCT_NO_TOOLING;
    // If creator is in transport, can creator build from within the transport? 
    if (creator->transport
        && !uu_occ_can_build(creator->transport->type, u2))
	return A_ANY_CANNOT_DO;
    // Compare materials availability and requirements.
    cp = min((cab ? uu_cp_per_build(u2, u3) : uu_creation_cp(u2, u3)),
	     u_cp(u3));
    get_materials_availability(actor, creator, p_mavail, p_mavailmax);
    if (!p_mreq_spcl) {
	for_all_material_types(m) {
	    if (cab)
		p_mreq[m] = materials_req_for_build(u2, u3, m, cp);
	    else
		p_mreq[m] =
		    max(um_to_create(u2, m),
			um_consumption_per_cp(u3, m) * cp
			+ um_consumption_on_creation(u3, m)
			+ um_consumption_per_create(u2, m));
	}
    }
    else {
	for_all_material_types(m)
	    p_mreq[m] = p_mreq_spcl[m];
    }
    if (!valid(
	    rslt = 
		can_meet_materials_requirement(p_mreq, p_mavail, p_mavailmax)))
	return rslt;
    return A_ANY_OK;
}

int
could_build(int u, int u2)
{
    assert_error(is_unit_type(u),
		 "Build Check: Encountered invalid builder unit type");
    assert_error(is_unit_type(u2),
		 "Build Check: Encountered invalid buildee unit type");
    return (uu_build(u, u2) && (0 < uu_cp_per_build(u, u2))
	    && (!uu_tp_to_build(u, u2) || could_toolup_for(u, u2)));
}

int
could_build_any(int u)
{
    int u2 = NONUTYPE;

    assert_error(is_unit_type(u),
		 "Build Check: Encountered invalid builder unit type");
    for_all_unit_types(u2) {
	if (could_build(u, u2))
	    return TRUE;
    }
    return FALSE;
}

int
can_build(Unit *actor, Unit *builder, int u3)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;

    assert_error(is_active(actor),
		 "Build Check: Attempted to access an inactive actor");
    assert_error(is_active(builder),
		 "Build Check: Attempted to access an inactive builder");
    assert_error(is_unit_type(u3),
		 "Build Check: Encountered an invalid buildee unit type");
    u = actor->type;
    u2 = builder->type;
    side = actor->side;
    if (!valid(rslt = can_act(actor, builder)))
	return rslt;
    if (!could_build(u2, u3))
	return A_ANY_CANNOT_DO;
    if (!side->could_construct[u3])
	return A_ANY_CANNOT_DO;
    if (!valid(can_toolup_for(actor, builder, u3)))
	return A_ANY_CANNOT_DO;
    if (!has_enough_tooling(builder, u3))
	return A_CONSTRUCT_NO_TOOLING;
    return rslt;
}

int
can_build(
    Unit *actor, Unit *builder, Unit *buildee, 
    int could, int acpcost_spcl, long *p_mreq_spcl)
{
    static long *p_mavail, *p_mavailmax, *p_mreq;

    int rslt = A_ANY_OK;
    int acpcost = 0, cp = -1;
    int u = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    int m = NONMTYPE;
    Unit *transport = NULL;
    
    assert_error(is_active(actor),
		 "Build Check: Attempted to access an inactive actor");
    assert_error(is_active(builder),
		 "Build Check: Attempted to access an inactive builder");
    assert_error(in_play(buildee),
		 "Build Check: Attempted to access an out-of-play buildee");
    u = actor->type;
    u2 = builder->type;
    u3 = buildee->type;
    transport = builder->transport;
    if (!could && !valid(rslt = can_build(actor, builder, u3)))
      return rslt;
    // Can builder build while inside transport?
    // Note that an exception is made, if building on the transport itself.
    if (transport && (transport != buildee)
        && !uu_occ_can_build(transport->type, u2)) 
	return A_ANY_CANNOT_DO;
    // Initialize materials arrays, if necessary.
    if (!p_mavail)
        p_mavail = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mavailmax)
        p_mavailmax = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mreq)
        p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    // Is distance within build range?
    // TODO: If build range is -1, then check transport-occ chain.
    if (distance(builder->x, builder->y, buildee->x, buildee->y)
	> uu_build_range(u2, u3))
	return A_ANY_TOO_FAR;
    // Check if necessary ACP is available to build.
    if (0 > acpcost_spcl)
	acpcost = uu_acp_to_build(u2, u3);
    else
	acpcost = acpcost_spcl;
    if (!u_acp_independent(u2) && (0 < acpcost)) {
	if (!can_have_enough_acp(actor, acpcost)) 
	    return A_ANY_CANNOT_DO;
	if (!has_enough_acp(actor, acpcost))
	    return A_ANY_NO_ACP;
    }
    // Compare materials availability and requirements.
    cp = min(uu_cp_per_build(u2, u3), u_cp(u3) - buildee->cp);
    get_materials_availability(actor, builder, p_mavail, p_mavailmax);
    if (!p_mreq_spcl) {
	for_all_material_types(m) 
	    p_mreq[m] = materials_req_for_build(u2, u3, m, cp);
    }
    else {
	for_all_material_types(m)
	    p_mreq[m] = p_mreq_spcl[m];
    }
    if (!valid(
            rslt =
                can_meet_materials_requirement(p_mreq, p_mavail, p_mavailmax)))
        return rslt;
    return rslt;
}

static int
can_any_repair_common(Unit *actor, Unit *repairer, Unit *repairee)
{
    static long *p_mavail, *p_mavailmax, *p_mreq;

    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    int m = NONMTYPE;

    assert_error(is_active(actor),
		 "Repair Check: Attempted to manipulate inactive actor");
    assert_error(is_active(repairer),
		 "Repair Check: Attempted to manipulate inactive repairer");
    assert_error(in_play(repairer),
		 "Repair Check: Attempted to manipulate out-of-play repairee");
    // Check if we are attempting to repair an enemy?
    if (enemy_side(repairer->side, repairee->side))
	return A_ANY_CANNOT_DO;
    // Check if we meet basic action requirements.
    if (!valid(rslt = can_act(actor, repairer)))
	return rslt;
    // Useful info.
    u = actor->type;
    u2 = repairer->type;
    u3 = repairee->type;
    // Initialize materials arrays, if necessary.
    if (!p_mavail)
	p_mavail = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mavailmax)
	p_mavailmax = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mreq)
	p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    // Compare materials availability and requirements.
    get_materials_availability(actor, repairer, p_mavail, p_mavailmax);
    for_all_material_types(m) {
	p_mreq[m] =
	    max(um_to_repair(u2, m),
		um_consumption_per_repaired(u3, m)
		+ um_consumption_per_repair(u2, m));
    }
    if (!valid(
            rslt =
                can_meet_materials_requirement(p_mreq, p_mavail, p_mavailmax)))
        return rslt;
    return A_ANY_OK;
}

int
could_auto_repair(int u, int u2)
{
    assert_error(is_unit_type(u),
		 "Repair Check: Encountered an invalid repairer unit type");
    assert_error(is_unit_type(u2),
		 "Repair Check: Encountered an invalid repairee unit type");
    if (0 < uu_auto_repair(u, u2))
	return TRUE;
    return FALSE;
}

int
can_auto_repair(Unit *repairer, Unit *repairee)
{
    int rslt = A_ANY_OK;
    int u2 = NONUTYPE, u3 = NONUTYPE;
    int recovery = FALSE;
    int dist = -1;

    if (!is_active(repairer))
	return A_ANY_ERROR;
    assert_error(in_play(repairee),
		 "Repair Check: Attempted to manipulate out-of-play repairee");
    // Useful info.
    u2 = repairer->type;
    u3 = repairee->type;
    dist = distance(repairer->x, repairer->y, repairee->x, repairee->y);
    // Check if there is any repair to perform.
    // Note: We check against full HP rather than doctrine level here, 
    //	because, technically, anything less than full HP means that 
    //	repair could be performed.
    if (repairee->hp >= u_hp(u3))
	return A_ANY_OK;
    // Could we use hp-recovery mechanism?
    if ((repairer == repairee) && (0 < u_hp_recovery(u2))) 
	recovery = TRUE;
    // Could we auto-repair?
    if (!could_auto_repair(u2, u3) && !recovery)
	return A_ANY_CANNOT_DO;
    // Can we use hp-recovery mechanism?
    if (recovery) {
	if (repairee->hp < u_hp_to_recover(u2))
	    // TODO: Should return A_REPAIR_TOO_DAMAGED.
	    return A_ANY_NO_MATERIAL;
	return A_ANY_OK;
    }
    // Are we in range for auto-repair?
    if (dist > uu_auto_repair_range(u2, u3))
	return A_ANY_TOO_FAR;
    // Additional checks.
    if (!valid(rslt = can_any_repair_common(repairer, repairer, repairee)))
	return rslt;
    return rslt;
}

int
could_repair(int u, int u2)
{
    assert_error(is_unit_type(u),
		 "Repair Check: Encountered an invalid repairer unit type");
    assert_error(is_unit_type(u2),
		 "Repair Check: Encountered an invalid repairee unit type");
    if (uu_repair(u, u2) && (0 < uu_hp_per_repair(u, u2)))
	return TRUE;
    return FALSE;
}

int
can_repair(Unit *actor, Unit *repairer, int u3)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE;

    assert_error(is_active(actor),
		 "Repair Check: Attempted to manipulate inactive actor");
    assert_error(is_active(repairer),
		 "Repair Check: Attempted to manipulate inactive repairer");
    assert_error(is_unit_type(u3),
		 "Repair Check: Encountered an invalid repairee unit type");
    u = actor->type;
    u2 = repairer->type;
    // Check if we meet basic action requirements.
    if (!valid(rslt = can_act(actor, repairer)))
	return rslt;
    // Could we explicitly repair?
    if (!could_repair(u2, u3))
	return A_ANY_CANNOT_DO;
    return rslt;
}

int
can_repair(Unit *actor, Unit *repairer, Unit *repairee)
{
    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    int acp = -1, dist = -1;

    assert_error(is_active(actor),
		 "Repair Check: Attempted to manipulate inactive actor");
    assert_error(is_active(repairer),
		 "Repair Check: Attempted to manipulate inactive repairer");
    assert_error(in_play(repairer),
		 "Repair Check: Attempted to manipulate out-of-play repairee");
    // Additional checks.
    if (!valid(rslt = can_any_repair_common(repairer, repairer, repairee)))
	return rslt;
    // Useful info.
    u = actor->type;
    u2 = repairer->type;
    u3 = repairee->type;
    dist = distance(repairer->x, repairer->y, repairee->x, repairee->y);
    // Check if there is any repair to perform.
    // Note: We check against full HP rather than doctrine level here, 
    //	because, technically, anything less than full HP means that 
    //	repair could be performed.
    if (repairee->hp >= u_hp(u3))
	return A_ANY_OK;
    if (!valid(rslt = can_repair(actor, repairer, u3)))
	return rslt;
    if (dist > uu_repair_range(u2, u3))
	return A_ANY_TOO_FAR;
    // Check if necessary ACP is available for explicit repair.
    if (!u_acp_independent(u2) && (0 < uu_acp_to_repair(u2, u3))) {
	acp = uu_acp_to_repair(u2, u3);
	if (!can_have_enough_acp(actor, acp))
	    return A_ANY_CANNOT_DO;
	if (!has_enough_acp(actor, acp))
	    return A_ANY_NO_ACP;
    }
    return rslt;
}

int
can_change_type_to(int u, int u2, Side *side)
{
    int rslt = A_ANY_OK;
    int a = NONATYPE;

    assert_error(is_unit_type(u), "Attempted to reference an invalid utype");
    assert_error(is_unit_type(u2), 
		 "Attempted to change type to an invalid utype");
    assert_error(side, "Attempted to access a NULL side");
    /* Check if a manual change-type or auto-upgrade is ever possible. */
    if (!could_change_type_to(u, u2) && !could_auto_upgrade_to(u, u2))
      return A_ANY_CANNOT_DO;
    /* Check if the turn number is high enough. */
    if (g_turn() < uu_turn_to_change_type(u, u2))
      /* (Should return A_ANY_PREMATURE.) */
      return A_ANY_NO_MATERIAL; 
    /* Check if the unit's new type can be on the side. */
    if (!type_allowed_on_side(u2, side))
      return A_ANY_CANNOT_DO;
    /* Check if the unit could ever have anough CXP. */
    if (u_cxp_max(u) < uu_cxp_to_change_type(u, u2))
      return A_ANY_CANNOT_DO;
    /* Check if there could ever be ebough tech. */
    if (u_tech_max(u2) < u_tech_to_change_type_to(u2))
      return A_ANY_CANNOT_DO;
    /* Check if the side has enough tech. */
    if (side->tech[u2] < u_tech_to_change_type_to(u2))
      /* (Should return A_ANY_NO_TECH.) */
      return A_ANY_NO_MATERIAL;
    /* Check that the side has any advances necessary for the change. */
    for_all_advance_types(a) {
        if (!ua_to_change_type(u, a))
          continue;
        if (!has_advance(side, a))
          /* (Should return A_ANY_NO_ADVANCE.) */
          return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

//! Can given unit change into given utype?

int
can_change_type_to(Unit *actor, Unit *morpher, int u3)
{
    static long *p_mavail, *p_mavailmax, *p_mreq;

    int rslt = A_ANY_OK;
    int u = NONUTYPE, u2 = NONUTYPE, u4 = NONUTYPE;
    int m = NONMTYPE;
    int acp = 0;
    Unit *occ = NULL;

    assert_error(in_play(actor), "Attempted to manipulate an out-of-play unit");
    assert_error(in_play(morpher), 
		 "Attempted to manipulate an out-of-play unit");
    assert_error(is_unit_type(u3), 
		 "Attempted to change type into an invalid utype");
    if (!valid(rslt = can_act(actor, morpher)))
      return rslt;
    u = actor->type;
    u2 = morpher->type;
    /* Is manual change-type valid? */
    if (!at_turn_start && !uu_change_type_to(u2, u3))
      return A_ANY_CANNOT_DO;
    /* Is auto-upgrade valid? */
    if (at_turn_start && (u3 != u_auto_upgrade_to(u2)))
      return A_ANY_CANNOT_DO;
    /* Go through utype-based and side-based tests. */
    if (!valid(rslt = can_change_type_to(u2, u3, actor->side)))
      return rslt;
    // Initialize materials arrays, if necessary.
    if (!p_mavail)
        p_mavail = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mavailmax)
        p_mavailmax = (long *)xmalloc(nummtypes * sizeof(long));
    if (!p_mreq)
        p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    acp = uu_acp_to_change_type(u2, u3);
#if (0)
    /* Check if the unit can even perform the action. */
    if (!at_turn_start && !u_acp_independent(u) && (acp < 1))
      return A_ANY_CANNOT_DO;
#endif
    /* Check if the actor actually has enough ACP to make the 
       change-type happen. Give a free pass for automated happenings at the 
       the turn start. */
    if (!at_turn_start && !u_acp_independent(u) && !has_enough_acp(actor, acp))
      return A_ANY_NO_ACP;
    /* Check that the unit has grown enough. */
    if (morpher->size < uu_size_to_change_type(u2, u3))
      /* (Should return A_ANY_TOO_SMALL.) */
      return A_ANY_NO_MATERIAL;
    /* Check that the unit has the necessary CXP for the change. */
    if (morpher->cxp < uu_cxp_to_change_type(u2, u3))
      /* (Should return A_ANY_NO_CXP.) */
      return A_ANY_NO_MATERIAL; 
    // Compare materials availability and requirements.
    get_materials_availability(actor, morpher, p_mavail, p_mavailmax);
    for_all_material_types(m) {
	p_mreq[m] = 
	    max(um_to_change_type(u2, m), 
		um_consumption_per_change_type(u2, m));
    }
    if (!valid(
            rslt =
                can_meet_materials_requirement(p_mreq, p_mavail, p_mavailmax)))
        return rslt;
    /* Check that the unit has any occupants necessary for the change. */
    for_all_unit_types(u4) 
      tmp_u_array[u4] = uu_occs_to_change_type(u2, u4);
    for_all_occupants(morpher, occ) {
        u4 = occ->type;
        if (!uu_occs_to_change_type(u2, u4))
          continue;
        --(tmp_u_array[u4]);
    }
    for_all_unit_types(u4) {
        if (tmp_u_array[u4])
          /* (Should return A_ANY_NO_OCCS.) */
          return A_ANY_NO_MATERIAL;
    }
    return rslt;
}

/*! \brief Action Initialization.
 *
 * Do any action-related initialization.
 * Currently it doesn't do anything.
 */
void
init_actions(void)
{
}

/*! \brief Prepare for No Action.
 *
 * Just a placeholder action, so not much to do here.
 *
 * \see do_none_action, check_none_action
 *
 * \param unit is the pointer to the \Unit initiating action.
 * \param actee is the pointer to the \Unit which performs the action.
 * \return
 *   - TRUE if action is queed.
 *   - FALSE if 
 *     - \Unit pointer to unit is NULL, 
 *     - \Unit action pointer is NULL, or
 *     - \Unit pointer to actee is NULL
 */
int
prep_none_action(Unit *unit, Unit *actee)
{
    if (unit == NULL || unit->act == NULL || actee == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_NONE;
    unit->act->nextaction.actee = actee->id;
    return TRUE;
}

/*! \brief Do No Action.
 *
 * Perform no action action. :-)
 *
 * \see prep_none_action, check_none_action.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param actee is the pointer to the \Unit which performs the action.
 * \return A_ANY_DONE always.
 */
int
do_none_action(Unit *unit, Unit *actee)
{
    return A_ANY_DONE;
}

/*! \brief Check for No Action.
 *
 * Check to see if no action is possible.
 *
 * \see prep_none_action, do_none_action.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param actee is the pointer to the \Unit which performs the action
 * \return A_ANY_DONE always.
 */
int
check_none_action(Unit *unit, Unit *actee)
{
    return A_ANY_DONE;
}

/* Material actions. */

/* Explicit material production. */

/*! \brief Prepare for Material Production Action.
 *
 * Produce materials from unit.  The unit supplies the ACP
 * for the action, and stores the action arguments.  The
 * producer stores the available material.
 * - Sets the next action type to ACTION_PRODUCE.
 * - Sets the type and amount of production.
 * - Set producer to the actee.
 * Fails if either of the pointers to \Unit is NULL, or tha ction is NULL.
 *
 * \see do_produce_action, check_produce_action.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param producer is the pointer to the \Unit extracting the material.
 * \param m is material type.
 * \param amount is amount of material to produce.
 * \return 
 *    - TRUE if action is queed.
 *    - FALSE if
 *      - unit pointer is NULL,
 *      - unit action pointer is NULL, or
 *      - producer pointer is NULL.
 */
int
prep_produce_action(Unit *unit, Unit *producer, int m, int amount)
{
    if (unit == NULL || unit->act == NULL || producer == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_PRODUCE;
    unit->act->nextaction.args[0] = m;
    unit->act->nextaction.args[1] = amount;
    unit->act->nextaction.actee = producer->id;
    return TRUE;
}

/*! \brief Do Material Production Action.
 *
 * Produce materials from a unit.  The producer unit stores as much material 
 * internally as possible, then distributes it ot units around it.
 * - Sets the production amount to the quantity asked for, or the
 *    maximum that can be produced, whichever is less.
 * - Clips the amount stored to the storeage maximum.
 * - If there is an excess amount, passes it around, if possible.
 * - Uses up ACP for production.
 *
 * \see prep_produce_action, check_produce_action, um_material_per_production,
 *      um_storate_x, distribute_material, use_up_acp, 
 *      um_acp_to_produce.
 *
 * \GDL
 *    - <a href="xcdesign_246.html#GDL_ref_acp-to-produce">acp-to-produce</a>
 *    - <a href="xcdesign_246.html#GDL_ref_material-per-production">material-per-production</a>
 *    - <a href="xcdesign_246.html#GDL_ref_material-to-produce">material-to-produce</a>
 *    - <a href="xcdesign_204.html#GDL_ref_unit-storage-x">unit-storage-x</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param producer is the pointer to the \Unit which extracts the material.
 * \param m is the material type.
 * \param amount is the amount of material to produce.
 * \return A_ANY_DONE always.
 */
int
do_produce_action(Unit *unit, Unit *producer, int m, int amount)
{
    int amt;	/* amount of material produced */
    int excess; /* amount of material that can't be stored by producer */
    int space; /* max amount of material that can be added to the supply */

    /* Note: it is very important that the transactions below are made in the
    correct order to avoid both numeric overflow in the producer's supply and 
    the latter being depleted before all excess has been passed around. */
    amt = min(amount, um_material_per_production(producer->type, m));
    space = um_storage_x(producer->type, m) - producer->supply[m];

    /* First fill up the producer as far as there is room. */
    producer->supply[m] += min(amt, space);
    /* Compute the excess we couldn't store. */
    excess = amt - space;
    /* Try to give away the same amount to make room for the excess. */
    if (excess > 0) {
    	/* But first clip it to available supplies. */
    	excess = min(excess, producer->supply[m]);
    	producer->supply[m] -= excess;
	distribute_material(producer, m, excess);
    }
    /* Then try to fill up the producer again with the remaining excess. */
    space = um_storage_x(producer->type, m) - producer->supply[m];
    /* The check for available space is redundant now, but will be needed
    in the future when distribute_material has been improved. */
    producer->supply[m] += min(excess, space);
    use_up_acp(unit, um_acp_to_produce(producer->type, m));
    return A_ANY_DONE;
}

/*! \brief Check for Material Production Action.
 *
 * Produce materials from Unit.
 * Validate:
 *    - unit is in play
 *    - producer is in play.
 *    - material type is valid.
 *    - producer needs at least 1 acp to produce material
 *    - unit must be able to have enough acp to produce at some point
 *      in the future.
 *    - unit has enough acp to produce the material
 *    - all needed supplies to produce the material
 *      are available.
 *    .
 * \see prep_produce_action, do_produce_action, in_play,
 *      is_material_type, um_acp_to_produce, can_have_enough_acp,
 *      um_material_per_production, has_enough_acp, for_all_material_types,
 *      um_to_produce.
 *
 * \GDL
 *    - <a href="xcdesign_246.html#GDL_ref_acp-to-produce">acp-to-produce</a>
 *    - <a href="xcdesign_246.html#GDL_ref_material-per-production">material-per-production</a>
 *    - <a href="xcdesign_246.html#GDL_ref_material-to-produce">material-to-produce</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param producer is the pointer to the \Unit that extracts the material.
 * \param m is material type?
 * \param amount is amount of material produced?
 * \return 
 *   - A_ANY_ERROR if
 *     - unit is not in play
 *     - producer is not in play
 *     - the material type is unknown
 *     .
 *   - A_ANY_CANNOT_DO if
 *     - ACP of unit is less than 1
 *     - if the unit cannot ever have enough ACP for the extraction
 *     - if the material production per turn is less than 1
 *     .
 *   - A_ANY_NO_ACP if the unit doesn't have enough ACP
 *   - A_ANY_NO_MATERIAL if any required materials are not available.
 *   - A_ANY_OK if the unit can produce the material.
 */
int
check_produce_action(Unit *unit, Unit *producer, int m, int amount)
{
    int acp;			/* ACP used in production of material */
    int supply;		/* iteration - material needed as supplies to produce material */

    if (!in_play(unit)) {
	return A_ANY_ERROR;
    }
    if (!in_play(producer)) {
	return A_ANY_ERROR;
    }
    if (!is_material_type(m)) {
	return A_ANY_ERROR;
    }
    acp = um_acp_to_produce(producer->type, m);
    if (acp < 1) {
	return A_ANY_CANNOT_DO;
    }
    if (!can_have_enough_acp(unit, acp)) {
	return A_ANY_CANNOT_DO;
    }
    if (um_material_per_production(producer->type, m) < 1) {
	return A_ANY_CANNOT_DO;
    }
    if (!has_enough_acp(unit, acp)) {
	return A_ANY_NO_ACP;
    }
    /* Check that the unit has any required supplies. */
    for_all_material_types(supply) {
	if (producer->supply[supply] < um_to_produce(producer->type, supply)) {
	    return A_ANY_NO_MATERIAL;
	}
    }
    return A_ANY_OK;
}

/* Extraction of material from terrain. */

/*! \brief Prepare for Terrain Extraction Action.
 *
 * Prepare to extract material from a cell.  The unit supplies the ACP
 * for the extraction process and stores the parameters for the extaction
 * action.  The extractor is the unit which recieves the material.
 *
 * \see do_extract_action, check_extract_action.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param extractor is the pointer to the \Unit doing the extraction.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \param m is the material to extract from the cell.
 * \param amount is the quantify of material to extract from the cell.
 * \return
 *    - true action is queed.
 *    -false if 
 *      - unit pointer is NULL,
 *      - unit action pointer is NULL, or
 *      - extractor pointer is NULL
 */
int
prep_extract_action(Unit *unit, Unit *extractor, int x, int y, int m, int amount)
{
    if (unit == NULL || unit->act == NULL || extractor == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_EXTRACT;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = m;
    unit->act->nextaction.args[3] = amount;
    unit->act->nextaction.actee = extractor->id;
    return TRUE;
}

/*! \brief Do Terrain Material Extraction Action.
 *
 * Extract material from the cell or other \Units in the cell.
 * - Calculate material production.
 *   - if cell produces material and has at least amount units of material
 *      - reduce materials at cell by amount.
 *      - if remaining amount is zero and the probability of changing
 *           on exhaustion is met, and the terrain on exhaustion has
 *           a value, change the terrain.
 *   - else try to supply from \Units in cell.
 * - clip to amount requested.
 * - distribute excess to other \Units.
 * - reduce ACP of the unit \Unit.
 *
 * \see prep_extract_action, check_extract_action, any_cell_materials_defined,
 *      cell_material_defined, material_at, set_material_at, terrain_at,
 *      probabability, tm_change_on_exhaust, tm_exhaust_type,
 *      change_terrain_type, for_all_stack, um_storage_x, distribute_material,
 *      use_up_acp, um_acp_to_extract.
 *
 * \GDL
 *    - <a href="xcdesign_247.html#GDL_ref_acp-to-extract">acp-to-extract</a>
 *    - <a href="xcdesign_263.html#GDL_ref_change-on-exhaustion-chance">change-on-exhaustion-chance</a>
 *    - <a href="xcdesign_263.html#GDL_ref_terrain-exhaustion-type">terrain-exhaustion-type</a>
 *    - <a href="xcdesign_204.html#GDL_ref_unit-storage-x">unit-storage-x</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param extractor is the pointer to the \Unit extracting the material.
 * \param x is the cell x location.
 * \param y is the cell y location.
 * \param m is the material needed.
 * \param amount is the quanity to produce.
 * \return A_ANY_DONE.
 */
int
do_extract_action(Unit *unit, Unit *extractor, int x, int y, int m, int amount)
{
    int found = 0;		/* Amount of material actually extracted. */
    int excess;         	/* Amount of material that can't be stored in extractor. */
    int space; 		/* Free space in the supply. */

    /* Do one attempt at extracting the amount. */
    found = extract_one_round(extractor, x, y, m, amount);
    /* Compute the excess we couldn't extract. */
    excess = amount - found;
    /* Compute the remaining storage space. */
    space = um_storage_x(extractor->type, m) - extractor->supply[m];
    /* If we ran out of space, we try to make room for more. */
    if (space == 0
        && excess > 0) {
    	/* But first clip to availble storage space. */
    	excess = min(excess, extractor->supply[m]);
	extractor->supply[m] -= excess;
	distribute_material(extractor, m, excess);
    }
    /* Do one more attempt at extracting the remaining amount. */
    found += extract_one_round(extractor, x, y, m, excess);

    use_up_acp(unit, um_acp_to_extract(extractor->type, m));
    if (found < amount) {
    	/* Should never happen since materials were checked by check_extract_action. */
    	run_warning("%s unable to extract %d %s at (%d, %d).", 
		short_unit_handle(extractor), amount, m_type_name(m), x, y);
	return A_ANY_ERROR;
    } else {
	return A_ANY_DONE;
    }
}

int
extract_one_round(Unit *extractor, int x, int y, int m, int amount)
{
    int oldamt;		/* Amount of material in terrain before adjustment */
    int amt;			/* Amount of material removed. */
    int newamt;		/* New amount of material in terrain after extraction. */
    int space; 		/* Max amount of material that can be added to the supply */
    int found = 0;		/* Amount of material actually extracted. */
    int t;              		/* Rerrain of cell */
    int newt;		/* New terrain type of cell if all materials extracted */
    Unit *unit2;		/* Iteration - pointer to next unit in stack of units. */

    /* First compute the available storage space. */
    space = um_storage_x(extractor->type, m) - extractor->supply[m];

    /* Get what we can get from the terrain. */
    if (any_cell_materials_defined()
	  && cell_material_defined(m)
	  && material_at(x, y, m) > 0) {
	oldamt = material_at(x, y, m);
	amt = min(amount, oldamt);
	/* First fill up the extractor as far as there is room. */
	extractor->supply[m] += min(amt, space);
	newamt = oldamt - min(amt, space);
	found += min(amt, space);
	space -= min(amt, space);
	set_material_at(x, y, m, newamt);
	/* (should do with a common routine) */
	t = terrain_at(x, y);
	if (newamt == 0
	      && probability(tm_change_on_exhaust(t, m))
	      && tm_exhaust_type(t, m) != NONTTYPE) {
	    newt = tm_exhaust_type(t, m);
	    /* Change the terrain's type. */
	    change_terrain_type(x, y, newt);
	}
    }
    /* If there was not enough material in the terrain we proceed
    to plunder any owner-less independent units in the stack. */
    if (amount - found > 0
    	&& space > 0
    	&& g_no_indepside_ingame()) {
	for_all_stack(x, y, unit2) {
	    /* If we don't need more or ran out of space we are done. */
	    if (amount - found == 0 || space == 0) {
	    	break;
	    }
	    if (in_play(unit2) 
	        && indep(unit2) 
	        && unit2->supply[m] >= 0) {
		oldamt = unit2->supply[m];
		amt = min(amount - found, oldamt);
		/* First fill up the extractor as far as there is room. */
		extractor->supply[m] += min(amt, space);
		found += min(amt, space);
		newamt = oldamt - min(amt, space);
		unit2->supply[m] = newamt;
		space -= min(amt, space);
	    }
	}
    }
    return found;
}

/*! \brief Check for Terrain Material Estraction Action.
 *
 * This checks to see if an extract operation is possible.
 * It verifies:
 * - the unit is in play,
 * - the extractor is in play,
 * - the material type is valid,
 * - the unit is capable of extracting material (no negative ACP balance)
 * - the unit has enough available ACP,
 * - the extractor has any supplies needed for extraction.
 * It then checks to see if material can be produced from the cell,
 * else from any other independent \Units also in the cell.
 *
 * \see prep_extract_action, do_extract_action, in_play, is_material_type,
 *      um_acp_to_extract, can_have_enough_acp, has_enough_acp,
 *      for_all_material_types, um_to_extract, any_cell_materials_defined,
 *      cell_material_defined, material_at, for_all_stack, indep.
 *
 * \GDL
 *    - <a href="xcdesign_247.html#GDL_ref_acp-to-extract">u-acp-to-extract</a>
 *    - <a href="xcdesign_263.html#GDL_ref_change-on-exhaustion-chance">change-on-exhaustion-chance</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param extractor is the pointer to the \Unit on which the action is preformed,
 * \param x is the x co-ordinate of the  cell,
 * \param y is the y co-ordinate of the cell,
 * \param m is the material to be produced,
 * \param amount is the quanity to be produced.
 * \return
 *    - A_ANY_ERROR if
 *      - the unit is not in play,
 *      - the extractor is not in play,
 *      - the material type is not valid.
 *    - A_ANY_CANNOT_DO if
 *      - the ACP is negative,
 *      - the unit can never have enough ACP.
 *    - A_ANY_NO_ACP if the unit doesn't have enough ACP.
 *    - A_ANY_NO_MATERIAL if the cell doesn't have the material.
 *    - A_ANY_OK if the material is available.
 *    - A_EXTRACT_NO_SOURCE if no material could be extracted.
 */
int
check_extract_action(Unit *unit, Unit *extractor, int x, int y, int m, int amount)
{
    int acp;			/* ACP for extraction. */
    int supply;		/* supply needed for extraction. */
    int found;		/* materials found when looking around. */
    Unit *stack_unit;	/* interation - pointer to next stack unit. */

    if (!in_play(unit)) {
	return A_ANY_ERROR;
    }
    if (!in_play(extractor)) {
	return A_ANY_ERROR;
    }
    if (!is_material_type(m)) {
	return A_ANY_ERROR;
    }
    acp = um_acp_to_extract(extractor->type, m);
    if (acp < 1) {
	return A_ANY_CANNOT_DO;
    }
    if (!can_have_enough_acp(unit, acp)) {
	return A_ANY_CANNOT_DO;
    }
    if (!has_enough_acp(unit, acp)) {
	return A_ANY_NO_ACP;
    }
    /* Check that the unit has any required supplies. */
    for_all_material_types(supply) {
	if (extractor->supply[supply] < um_to_extract(extractor->type, supply)) {
	    return A_ANY_NO_MATERIAL;
	}
    }
    /* Look for case of extraction from terrain. */
    found = 0;
    if (any_cell_materials_defined()
	&& cell_material_defined(m)
	/* amount is currently always 1 (see do_collect_task) so this is in fact
	a check for material_at(x, y, m) > 0. However, we keep support
	for n in case the task code is modified in the future. */
	&& material_at(x, y, m) > 0) {
    	found += material_at(x, y, m);
    }
    /* Then look for extraction from independent unit provided that there
    is no independent side that owns these units. This breaks the one game
    that uses explicit extraction, ancient-days, but prevents the absurd 
    case where a unit is using units belonging to another side (indepside) 
    to resupply itself. Possible fix for ancient-days: let berry bushes etc.
    be independent, set no-indepside-ingame to true, and let live animals
    belong to a third side. */
    if (g_no_indepside_ingame()) {
	    for_all_stack(x, y, stack_unit) {
		if (in_play(stack_unit)
		    && indep(stack_unit)
		    && stack_unit->supply[m] > 0) {
    			found += stack_unit->supply[m];
		}
	    }
    }
    if (found >= amount) {
    	return A_ANY_OK;    
    }
    return A_EXTRACT_NO_SOURCE;
}

/* Transfer action. */

/* This action transfers material from one unit to another. */

/*! \brief Prepare for Material Transfer Action.
 *
 * This routine sets up a material transfer from one \Unit to another.
 * The unit \Unit supplies the ACP for the action, and stores the parameters
 * for the action.  The from Unit supplies the material to the to Unit.
 *
 * \see do_transfer_action, check_transfer_action.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param from is the \Unit the materisl comes from.
 * \param m is the material being transferred.
 * \param amount is the quanity of material being transferred.
 * \param to is the \Unit receiving the transfer.
 * \return 
 *   - TRUE if action is queed.
 *   - FALSE if 
 *     - unit pointer is NULL,
 *     - unit action pointer is NULL,
 *     - from pointer is NULL, or
 *     - to pointer is NULL.
 */
int
prep_transfer_action(Unit *unit, Unit *from, int m, int amount, Unit *to)
{
    if (unit == NULL || unit->act == NULL || from == NULL || to == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_TRANSFER;
    unit->act->nextaction.args[0] = m;
    unit->act->nextaction.args[1] = amount;
    unit->act->nextaction.args[2] = to->id;
    unit->act->nextaction.actee = from->id;
    return TRUE;
}

/*! \brief Do Material Transfer Action.
 *
 * This routine transfers material from one \Unit to another.
 * Transfer goes from the to Unit to the from Unit if the 
 * quanity transferred is negative.  The actual quantity transfered is
 * calculated by transfer_supply.  The ACP for the transer
 * is used buy the unit Unit, even if no materis is transfered.
 * Transfers always use 1 ACP.
 *
 * \todo Add return code specifying that only a partial amount of
 *       material was transferred.
 *
 * \see prep_transfer_action, check_transfer_action, transfer_supply.
 *
 * \GDL
 *    - <a href="xcdesign_248.html#GDL_ref_acp-to-unload">acp-to-unload</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param from is the pointer to the \Unit supplying the material
 *        (for positive transfers).
 * \param m is the material being transferred.
 * \param amount is the quanity of material being transferred.
 * \param to is the pointer to the \Unit receiving the transfer
 *        (for positive transfers).
 * \return 
 *   - A_ANY_ERROR if no material was transferred.
 *   - A_ANY_DONE if at least one unit of material was transferred.
 */
int
do_transfer_action(Unit *unit, Unit *from, int m, int amount, Unit *to)
{
    int actual; /* Actual amount of material transferred */

    if (amount > 0) {
    	actual = transfer_supply(from, to, m, amount);
    	use_up_acp(unit, um_acp_to_unload(from->type, m));
    } else {
    	actual = transfer_supply(to, from, m, -amount);
    	use_up_acp(unit, um_acp_to_unload(to->type, m));
    }
    if (actual == amount) {
	return A_ANY_DONE;
    } else if (actual == 0) {
	return A_ANY_ERROR;
    } else {
    	/* (should be able to say that action did not do all that was requested) */
	return A_ANY_DONE;
    }
}

/*! \brief Check for Material Transfer Action.
 *
 * This routine makes sure that a transfer is possible.
 * It verifies
 *  - that unit, <I>to</I>, and <I>from</I> \Units are in play.
 *  - that the material tor transfer is a valid material,
 *  - that the quantify of material to transfer is non-zero.
 *  - that the <I>from</I> \Unit can unload material (ACP 1 or more),
 *  - that the <I>to</I> \Unit may load material(ACP 1 or more),
 *  - that the unit can ever have enough ACP to transfer the material,
 *  - that the \Unit suppling the material has the supplies needed to 
 *    transfer the material,
 *  - that the \Unit receivng the material has the storage for the 
 *    transferred material. and
 *  - that the unit has enough ACP to transfer the material.
 *
 * \see prep_transfer_action, do_transfer_action, in_play, is_material_type, 
 *      um_acp_to_unload, um_acp_to_load, can_have_enough_acp,
 *      um_storage_x, has_enough_acp.
 *
 * \GDL
 *    - <a href="xcdesign_248.html#GDL_ref_acp-to-load">acp-to-load</a>
 *    - <a href="xcdesign_248.html#GDL_ref_acp-to-unload">acp-to-unload</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *    
 * \param unit is the pointer to the \Unit initiating the action.
 * \param from is the pointer to the \Unit supplying the material (if
 *        amount is positive).
 * \param m is the material being transferred.
 * \param amount is the quanity of material being transferred.
 * \param to is the pointer to the \Unit receiving the transfer (if
 *        amount is positive).
 * \return 
 *   - A_ANY_ERROR if
 *     - the unit is not in play,
 *     - the <I>to</I> \Unit is not in play,
 *     - the material type is invalid,
 *     - the quanity of material to produce is zero (negative is ok),
 *     - the <I>from</I> \Unit is not in play,
 *     - the appropriate \Unit (depending on quantity positive or negative) does
 *       not have enough supplies of other materials on hand, or
 *     - the appropriate \Unit does not have enough storage capacity.
 *   - A_ANY_CANNOT_DO if
 *     - the <I>from</I> \Unit cannot unload material (ACP to unload less than 1),
 *     - the <I>to</I> \Unit has a NULL action pointer or cannot load material
 *       (ACP to load less than 1), or
 *     - the unit can never have enought ACP to do the transfer.
 *   - A_ANY_NO_ACP if the unit doesn't have enought ACP.
 *   - A_ANY_DONE if at least one unit of material can be transferred.
 */
int
check_transfer_action(Unit *unit, Unit *from, int m, int amount, Unit *to)
{
    if (!in_play(unit)) {
	return A_ANY_ERROR;
    }
    if (!in_play(from)) {
	return A_ANY_ERROR;
    }
    if (!is_material_type(m)) {
	return A_ANY_ERROR;
    }
    if (amount == 0) {
	return A_ANY_ERROR;
    }
    if (!in_play(to)) {
	return A_ANY_ERROR;
    }
    if (amount > 0) {
	if (um_acp_to_unload(from->type, m) < 1) {
	    return A_ANY_CANNOT_DO;
	}
	if (to->act && um_acp_to_load(to->type, m) < 1) {
	    return A_ANY_CANNOT_DO;
	}
	/* We check only the acp needed for the unload action since um_acp_to_load 
	is used strictly as a permission flag (never causes acps to be consumed). */
	if (!can_have_enough_acp(unit, um_acp_to_unload(from->type, m))) {
	    return A_ANY_CANNOT_DO;	      
	}
	if (from->supply[m] <= 0) {
	    return A_ANY_ERROR;
	}
	if (um_storage_x(to->type, m) == 0) {
	    return A_ANY_ERROR;
	}
    } else {
	if (um_acp_to_load(from->type, m) < 1) {
	    return A_ANY_CANNOT_DO;
	}
	if (to->act && um_acp_to_unload(to->type, m) < 1) {
	    return A_ANY_CANNOT_DO;
	}
	/* We check only the acp needed for the unload action since um_acp_to_load 
	is used strictly as a permission flag (never causes acps to be consumed). */
	if (!can_have_enough_acp(unit, um_acp_to_unload(to->type, m))) {
	    return A_ANY_CANNOT_DO;	      
	}
	if (to->supply[m] <= 0) {
	    return A_ANY_ERROR;
	}
	if (um_storage_x(from->type, m) == 0) {
	    return A_ANY_ERROR;
	}
    }
    if (!has_enough_acp(unit, 1)) {
	return A_ANY_NO_ACP;
    }
    return A_ANY_OK;
}

/*! \brief Transfer Supply.
 *
 * Move supply from one \Unit to another.  Don't move more than is
 * possible; check both from and to amounts and capacities.  This
 * routine will also transfer to the side's treasury if necessary can
 * handle overflow. This is a utility routine.
 * - Calculate the min amount to transfer based on the available
 *   supply and the maximum that can be loaded in the to \Unit.
 * - if there is a treasury for the material, and the receiving
 *   \Unit can transfer material to the treasury, then transfer any
 *   excess production from the from \Unit to the treasure
 * - reduce the material supply of the from \Unit,
 * - increase the  material supply of the to \Unit.
 * - update to and from \Unit supply display for the sides involved.
 * - if the treasury was modified, update the treasury supply display.
 * - print debug message, if it's enabled.
 *
 * \see do_transfer_material, um_unload_max, um_load_max, UM_storage_x,
 *      side_has_treasury, um_gives_to_treasury, update_unit_display,
 *      update_side_display.
 *
 * \GDL
 *    - <a href="xcdesign_248.html#GDL_ref_load-max">load-max</a>
 *    - <a href="xcdesign_262.html#GDL_ref_gives-to-treasury">gives-to-treasury</a>
 *    - <a href="xcdesign_161.html#GDL_ref_treasury">treasury</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *    - <a href="xcdesign_248.html#GDL_ref_unload-max">unload-max</a>
 *
 * \param from is the pointer to the \Unit supplying the material.
 * \param to is the pointer to the \Unit receiving the material.
 * \param m is the material type to transfer.
 * \param amount is the amount of material to transfer.
 * \return the amount to transfer.
 */
int
transfer_supply(Unit *from, Unit *to, int m, int amount)
{
    int amt2;					/* material amount */
    int origfrom = from->supply[m];	/* Material originally in from unit */
    int origto = to->supply[m];		/* Material originally in to Unit */

    /* Constrain the amount of supply that may be unloaded. */
    amount = min(amount, origfrom);
    if (um_unload_max(from->type, m) >= 0) {
	amount = min(amount, um_unload_max(from->type, m));
    }
    /* Now constrain the amount of material that can be loaded */
    if (um_load_max(to->type, m) >= 0) {
	amount = min(amount, um_load_max(to->type, m));
    }
    /* amount is now the material that is possible to transfer, 
     * now figure out the amount that can actually be put into
     * the 'to' Unit. */
    amt2 = min(amount, um_storage_x(to->type, m) - origto);

#if 0 /* Using the treasury to dump overflow is rather dubious. 
	Presumably, we are doing the transfer action because we
	want 'to' to get the stuff, not because we want to fill up the
	treasury. Better to keep the material in 'from', and do a
	second transfer action if necessary. */ 

    /* Transfer overflow to side's treasury if it exists.  If it 
     * doesn't, reduce the amount transferred to the amount that
     * the 'to' unit can hold. */
    if (side_has_treasury(to->side, m)
        && um_gives_to_treasury(to->type, m)) {
	/* Make sure the treasury does not overflow. */
	to->side->treasury[m] += min(g_treasury_size() - to->side->treasury[m], amount - amt2);
    } else {
	amount = amt2;
    }
  /* Also dubious if from->supply should be decremented with 
  more than is actually moved to either 'to' or treasury. */
    from->supply[m] -= amount;

#else

    from->supply[m] -= amt2;

#endif

    to->supply[m] += amt2;

    /* Make sure any displays of supply levels see the transfer. */
    update_unit_display(from->side, from, TRUE);
    update_unit_display(to->side, to, TRUE);

#if 0		/* Not needed if we don't transfer overflow to treasury. */

    if (amount != amt2) {
	update_side_display(from->side, from->side, TRUE);
	update_side_display(to->side, to->side, TRUE);
    }

#endif

    Dprintf("%s (had %d) transfers %d %s to %s (had %d)\n",
	    unit_desig(from), origfrom, amt2, m_type_name(m),
	    unit_desig(to), origto);
    return amt2;
}

/* Develop action. */


/*! \brief Prepare for technical Development Action.
 *
 * Setup a Develop action.
 * In order to build a specific unit type, a side may first have to develop
 * the tech level needed for that unit. This is achieved by the develop action.
 * \note Technical development is different from advances and advanced
 * units.
 * \see do_devop_action, do_check_action.
 * \param unit is the pointer to the \Unit initiating the action.
 * \param developer is the pointer to the \Unit doing the development.
 * \param new_u is the new Unit type being developed.
 * \return 
 *   - TRUE action is queed.
 *   - FALSE if 
 *     - unit is null pointer,
 *     - unit action is a null pointer, or
 *     - developer is null pointer.
 */
int
prep_develop_action(Unit *unit, Unit *developer, int new_u)
{
    if (unit == NULL || unit->act == NULL || developer == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_DEVELOP;
    unit->act->nextaction.args[0] = new_u;
    unit->act->nextaction.actee = developer->id;
    return TRUE;
}

/*! \brief Do Technical Development Action.
 *
 * Develop the tech level needed for building \new_u, if not already available.
 * -# calculate contribution to tech by the developer \Unit.  Use the value
 *    of uu_tech_per_develop divided by 100, plus a random value of zero or
 *    one.
 * -# Caclulate limit on tech change for the side
 * -# If tech change exceeds limit, reset to limit
 * -# If tech level changes,
 *   - notify side of tech change.
 *   - update vector of buildable Units.
 * -# Adjust the tech levels of related Units
 * -# Uuse up the ACP of the \Unit.
 *
 * \note Technical development is different from advances and advanced
 * units.
 *
 * \see prep_develop_action, prob_fraction, uu_tech_per_develop,
 *      u_tech_per_turn_max, update_canbuild_vector, adjust_tech_crossover,
 *      use_up_acp, uu_acp_to_develop.
 *
 * \GDL
 *    - <a href="xcdesign_241.html#GDL_ref_acp-to-develop">acp-to-develop</a>
 *    - <a href="xcdesign_157.html#GDL_ref_init-tech">init-tech</a>
 *    - <a href="xcdesign_157.html#GDL_ref_tech">tech</a>
 *    - <a href="xcdesign_241.html#GDL_ref_tech-per-develop">tech-per-develop</a>
 *    - <a href="xcdesign_241.html#GDL_ref_tech-per-turn-max">tech-per-turn-max</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param developer is the pointer to the \Unit acted upon.
 * \param new_u is the Unit type of the increasing tech Unit.
 * \return A_ANY_DONE.
 */
int
do_develop_action(Unit *unit, Unit *developer, int new_u)
{
    int oldtech;	/* Side's starting tech level */
    int lim;		/* limit on tech development */
    Side *side;	/* Developer's side */

    side = developer->side;
    /* Save the initial tech level */
    oldtech = side->tech[new_u];
    side->tech[new_u] += prob_fraction(uu_tech_per_develop(developer->type, new_u));
    /* Silently apply the per-side-per-turn limit on tech gains. */
    lim =  side->inittech[new_u] + u_tech_per_turn_max(new_u);
    if (side->tech[new_u] > lim) {
	side->tech[new_u] = lim;
    }
    if (side->tech[new_u] != oldtech) {
	/* (should do generic side display update?) */
	notify_tech(side, new_u, oldtech, side->tech[new_u]);
	/* Update info for side_can_build. */
	update_canbuild_vector(side);
    }
    /* Adjust the tech levels of any related unit types. */
    adjust_tech_crossover(side, new_u);
    use_up_acp(unit, uu_acp_to_develop(developer->type, new_u));
    return A_ANY_DONE;
}

/*! \brief Check for Technical Development Action.
 *
 * Check to see is tech development action is possible.  Tech development is
 * on a unit basis, as opposed to advances, which may depend on each other.
 * \note Technical development is different from advances and advanced
 * units.
 * <P>
 * - Validate \Units acted on.
 *   - unit must be in play,
 *   - developer must be in play, and
 *   - newunt must be a Unit type.
 * - Check to see if the side is an independent side, and independents are 
 *    allowed to develop.
 * - the developer must be able to develop the new unit type
 * - unit must be able to have enough acp for the developer to develop the 
 *    new unit type
 * - the side's tech level must not be at the side's maximum value already
 * - the unit must have enough acp for the developer to develop the new unit
 *    type
 * - the developer must have all materials needed to develop the new unit type
 *
 * \see prep_develop_action, do_develop_action, in_play, is_unit_type,
 *      g_indepside_can_develop, could_develop, can_have_enough_acp,
 *      uu_acp_to_develop, u_tech_max, for_all_material_types.
 *
 * \GDL
 *    - <a href="xcdesign_241.html#GDL_ref_acp-to-develop">acp-to-develop</a>
 *    - <a href="xcdesign_157.html#GDL_ref_init-tech">init-tech</a>
 *    - <a href="xcdesign_157.html#GDL_ref_tech">tech</a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param developer is the pointer to the \Unit which does the development
 * \param new_u is the new Unit type to be developed.
 * \return
 *   - A_ANY_ERROR if
 *     - unit is not in play,
 *     - developer is not in play,
 *     - new_u in not a valid Unit type,
 *     - the side is independent, and not allowed technical development, or
 *     - the side has reached it's maximum tech level.
 *   - A_ANY_CANNOT_DO if
 *      - the developer cannot develop the new unit type, or
 *      - the unit can never have enough ACP to develop the new Unit type.
 *   - A_ANY_NO_ACP if the unit does not currently have enough acp to develop  
 *     the new unit type.
 *   - A_ANY_OK if the do_develop_action can be done.
 */
int
check_develop_action(Unit *unit, Unit *developer, int new_u)
{
    int m;		/* material type */
    Side *side;	/* unit's side */

    if (!in_play(unit)) {
      return A_ANY_ERROR;
    }
    if (!in_play(developer)) {
      return A_ANY_ERROR;
    }
    if (!is_unit_type(new_u)) {
      return A_ANY_ERROR;
    }
    side = unit->side;
    /* Independent units might not do develop. */
    if (side == indepside && g_indepside_can_develop() == FALSE) {
      return A_ANY_ERROR;
    }
    /* This unit must be of a type that can develop the given type. */
    if (!could_develop(developer->type, new_u)) {
      return A_ANY_CANNOT_DO;
    }
    if (!can_have_enough_acp(unit, uu_acp_to_develop(developer->type, new_u))) {
      return A_ANY_CANNOT_DO;
    }
    /* Max tech level means there's nothing more to learn. */
    if (side->tech[new_u] >= u_tech_max(new_u)) {
      return A_ANY_ERROR;
    }
    if (!has_enough_acp(unit, uu_acp_to_develop(developer->type, new_u))) {
      return A_ANY_NO_ACP;
    }
    /* Check that the unit has any required supplies. */
    for_all_material_types(m) {
	if (developer->supply[m] < um_to_develop(developer->type, m)) {
	  return A_ANY_NO_MATERIAL;
	}
    }
    return A_ANY_OK;
}

/*!
 * \brief Adjuxt for Tchnical Development Crossover.
 *
 * For all Unit types, bring their tech level up to match the crossovers
 * from the given Unit type.
 * - for all Unit types ( for all sides, presumably? )
 *   - if the Unit type isn't the Unit type being developed
 *     - calculate the crosstech value
 *     - if the crosstech value increaes the tech level of the other \Unit
 *       - change the tech level of the other \Unit.
 *     - if the other uunit's tech level increases
 *       - notify the \Side of a tech increase for the Unit.
 *       - update the can build vector for the side.
 *
 * \see do_develop_action, for_all_unit_types, uu_tech_crossover, u_tech_max,
 *      notify_tech, update_canbuild_vector.
 *
 *  \GDL
 *    - <a href="xcdesign_157.html#GDL_ref_tech">tech</a>
 *    - <a href="xcdesign_190.html#GDL_ref_tech-crossover">tech-crossover</a>
 *    - <a href="xcdesign_190.html#GDL_ref_tech-max">tech-max</a>
 *
 * \param side is the pointer to \Side with the technical development.
 * \param new_u is the new Unit type being developed.
 */
void
adjust_tech_crossover(Side *side, int new_u)
{
    int other_u;	/* other unit type (iteration variable) */
    int oldtech;        /* initial tech level of the other unit */
    int crosstech;	/* crossover technical development of unit */

    for_all_unit_types(other_u) {
	if (other_u != new_u) {
	    oldtech = side->tech[other_u];
	    /* Compute the crossover as a ratio of max tech levels for
               each type. */
	    crosstech =
	      ((side->tech[new_u] * uu_tech_crossover(new_u, other_u) * u_tech_max(other_u)) /
	       (u_tech_max(new_u) * 100));
	    if (crosstech > side->tech[other_u]) {
		side->tech[other_u] = crosstech;
	    }
	    if (side->tech[other_u] != oldtech) {
		/* (should do generic side display update?) */
		notify_tech(side, other_u, oldtech, side->tech[other_u]);
		/* Update info for side_can_build. */
		update_canbuild_vector(side);
	    }
	}
    }
}

/*! \brief Notify of Technical Advance.
 *
 * Notify the given side of any notable changes in technological
 * ability.
 * For the given Unit, chck and notify the \Side if technology development allows
 * the side to see a new Unit type, own a new Unit type, use a new Unit type, or
 * build a new Unit type.
 *
 * \todo move to nlang.c?
 *
 * \see do_develop_action, adjust_tech_crossover, u_tech_to_see, u_tech_to_own,
 *      u_tech_to_use, u_tech_to_build, notify.
 *
 * \GDL
 *    - <a href="xcdesign_190.html#GDL_ref_tech-to-build">tech-to-build</a>
 *    - <a href="xcdesign.html#GDL_ref_tech-to_own">tech-to_own</a>
 *    - <a href="xcdesign_190.html#GDL_ref_tech-to-see">tech-to-see</a>
 *    - <a href="xcdesign_190.html#GDL_ref_tech-to-use">tech-to-use</a>
 *
 * \param side is the pointer to the \Side owning the Unit.
 * \param u is the Unit type.
 * \param oldtech is the old technical level for the Unit.
 * \param newtech is the new technical level for the Unit.
 */
void
notify_tech(Side *side, int u, int oldtech, int newtech)
{
    if (oldtech < u_tech_to_see(u)
	&& newtech >= u_tech_to_see(u)) {
	notify(side, "You now have the technology to see %s units",
	       u_type_name(u));
    }
    if (oldtech < u_tech_to_own(u)
	&& newtech >= u_tech_to_own(u)) {
	notify(side, "You now have the technology to own %s units",
	       u_type_name(u));
    }
    if (oldtech < u_tech_to_use(u)
	&& newtech >= u_tech_to_use(u)) {
	notify(side, "You now have the technology to use %s units",
	       u_type_name(u));
    }
    if (oldtech < u_tech_to_build(u)
	&& newtech >= u_tech_to_build(u)) {
	notify(side, "You now have the technology to build new %s units",
	       u_type_name(u));
    }
    if ((oldtech < u_tech_to_change_type_to(u))
	&& (newtech >= u_tech_to_change_type_to(u))) {
	notify(side, "You now have the technology to change type to %s units",
	       u_type_name(u));
    }
}

/* Toolup action. */

/*! \brief Prepare for Toolup Action.
 *
 * Before a \Unit can build another, it may need to take some time to
 * prepare by "tooling up". This is to simulate the amount of time
 * it takes to build the first of a Unit.
 * \see do_toolup_action, check_toolup_action.
 * \param unit is the pointer to the \Unit initiating the action.
 * \param constructor is a pointer to the \Unit which builds the new Unit.
 * \param u3 is the type of Unit being built.
 * \return
 *   - TRUE if action is successfully queued.
 */

int
prep_toolup_action(Unit *actor, Unit *constructor, int u3)
{
    assert_error(is_active(actor),
		 "Toolup Prep: Attempted to access an inactive actor");
    assert_error(is_active(constructor),
		 "Toolup Prep: Attempted to access an inactive constructor");
    assert_error(is_unit_type(u3),
		 "Toolup Prep: Encountered an invalid unit type to toolup for");
    actor->act->nextaction.type = ACTION_TOOL_UP;
    actor->act->nextaction.args[0] = u3;
    actor->act->nextaction.actee = constructor->id;
    return TRUE;
}

/*! \brief Do Toolup Action.
 *
 * Do the tooup action.
 * - If builder tooling pointer is NULL, initialize tooling for it.
 * - Get current tooling points (tp), add in the builder \Unit tooling points,
 *    and clip to the max tooling points for creating the new Unit type.
 * - If the number of tooling points changes (can go negative?), 
 *    notify the side.
 * - Adjust the tooling on all other Units, via a crossover array.
 * - update Unit display for the side.
 * - use up ACP for the unit \Unit
 *
 * \see prep_toolup_action, check_tooup_action, init_unit_tooling,
 *      uu_tp_per_toolup, uu_tp_max, notify_tp, adjust_tooling_crossover,
 *      update_unit_display, use_up_acp, uu_acp_to_toolup.
 *
 * \param actor is the pointer to the \Unit initiating the action.
 * \param constructor is pointer to the \Unit which is building the New unit.
 * \param u3 is the Unit type for which to tool up.
 * \return A_ANY_DONE always.
 */

int
do_toolup_action(Unit *actor, Unit *constructor, int u3)
{
    int oldtp = -1, tp = -1;
    Side *side = NULL;
    int u2 = NONUTYPE;

    assert_error(is_active(actor),
		 "Create Action: Attempted to manipulate an inactive actor");
    assert_error(is_active(constructor),
"Create Action: Attempted to manipulate an inactive constructor");
    assert_error(is_unit_type(u3),
		 "Create Action: Encountered an invalid unit type");
    u2 = constructor->type;
    side = actor->side;
    // Initialize the constructor's tooling array, if necessary.
    if (!constructor->tooling) 
	init_unit_tooling(constructor);
    // Increase the tooling, clipping to its max. 
    oldtp = tp = constructor->tooling[u3];
    tp = min(tp + uu_tp_per_toolup(u2, u3), uu_tp_max(u2, u3));
    constructor->tooling[u3] = tp;
    if (constructor->tooling[u3] != oldtp) 
	notify_tp(side, constructor, u3, oldtp, constructor->tooling[u3]);
    // Adjust any related toolings.
    adjust_tooling_crossover(constructor, u3);
    update_unit_display(side, constructor, TRUE);
    // Consume resources for this toolup.
    consume_materials(actor, constructor, um_consumption_per_tooledup, u3);
    consume_materials(actor, constructor, um_consumption_per_toolup);
    use_up_acp(actor, uu_acp_to_toolup(u2, u3));
    return A_ANY_DONE;
}

/*! \brief Check Toolup Action.
 *
 * Check to see if a toolup action may be performed.
 *
 * \see prep_toolup_action, do_toolup_action. in_play, is_unit_type,
 *      uu_acp_to_toolup, can_have_enough_acp, uu_tp_max,
 *      has_enough_acp.
 *
 * \param actor is the pointer to the \Unit initiating the action.
 * \param constructor is pointer to \Unit which will do the tool up for the
 *        new unit.
 * \param u3 is the type of Unit for which to toolup.
 * \return
 *    - A_ANY_ERROR if
 *      - unit is not in play,
 *      - builder is not in play,
 *      - new_u is not a Unit type, or
 *      - tooling points equal or exceed the number needed to toolup for the
 *        new Unit type.
 *    - A_ANY_CANNOT_DO if
 *      - the builder \Unit cannot toolup for the required Unit type 
 *        (acp less than 1), or
 *      - the unit \Unit can never have enough acp to produce the Unit type.
 *    - A_ANY_NO_ACP if the unit \Unit doesn't have enough acp to toolup this turn.
 *    - A_ANY_OK if the toolup is possible.
 */

int
check_toolup_action(Unit *actor, Unit *constructor, int u3)
{
    int rslt = A_ANY_OK;

    if (!valid(rslt = can_toolup_for(actor, constructor, u3)))
	return rslt;
    return A_ANY_OK;
}

/*! \brief Adjust Tooling Startup Crossover.
 *
 * For all Unit types, adjust any toolup crossover from the Unit toolup.
 * - If the \Unit tooling pointer is NULL, return
 * - For all Unit types
 *   - for each Unit type not the same as the calling Unit's type,
 *     - calculate crossover toolup points.
 *     - if crossover value increases,
 *       - set new toolup point value.
 *       - notify side of Unit change.
 *
 * \see do_toolup_action, for_all_unit_types, uu_tp_crossover, uu_tp_max,
 *      notify_tp.
 *
 * \param unit is a pointer to the \Unit perfroming the toolup
 * \param u is the type of Unit that was tooled up.
 */

void
adjust_tooling_crossover(Unit *constructor, int u2)
{
    int u = NONUTYPE, u3 = NONUTYPE;
    int uucross = -1, oldtp = -1;
    short int *tooling = NULL;

    assert_error(is_active(constructor),
"Toolup Action: Attempted to manipulate an out-of-play constructor");
    assert_error(is_unit_type(u2),
		 "Toolup Action: Encountered an invalid unit type");
    u = constructor->type;
    // Perhaps nothing to cross over with.
    if (!constructor->tooling) 
	return;
    tooling = constructor->tooling;
    // Look through other utypes and find any to share tooling with.
    for_all_unit_types(u3) {
	if (u3 == u2)
	    continue;
	uucross = uu_tp_crossover(u2, u3);
	// Clobber any toolings which cannot be shared.
	if (0 >= uucross) {
	    tooling[u3] = 0;
	    continue;
	}
	oldtp = tooling[u3];
	// Calculate the crossover as a ratio of max levels.
	tooling[u3] = (tooling[u2] * uucross * uu_tp_max(u, u3)) 
		       / (uu_tp_max(u, u2) * 100);
	if (tooling[u3] != oldtp) 
	    notify_tp(constructor->side, constructor, u3, oldtp, tooling[u3]);
    }
}

/*! \brief Notify of Tooling Startup Change.
 *
 * Notify the given \Side of any notable changes in tooling.
 * If toolup points moved from not buildable to buildable, notify side.
 *
 * \todo move to nlang.c?
 *
 * \see do_toolup_action, adjust_toolup_crossover, notify.
 *
 *  \GDL
 *    - <a href="xcdesign_242.html#GDL_ref_tp-to-build"></a>
 *    - <a href="xcdesign_182.html#GDL_ref_unit-type">unit-type</a>
 *
 * \param side is a pointer to the \Side.
 * \param unit is a pointer to the \Unit.
 * \param u is the type of the Unit.
 * \param oldtp is the old toolup points.
 * \param newtp is the new toolup points.
 * 
 */
void
notify_tp(Side *side, Unit *unit, int u, int oldtp, int newtp)
{
    if (oldtp < uu_tp_to_build(unit->type, u)
	&& newtp >= uu_tp_to_build(unit->type, u)) {
	notify(side, "%s is now tooled to build %s units",
	       unit_handle(side, unit), u_type_name(u));
    }
}

//! Consume materials needed by a build or create-as-build action.

static void
consume_materials_for_build(Unit *actor, Unit *constructor, int u3, int cp)
{
    assert_error(is_active(actor),
		 "Construct Action: Attempted to access an inactive actor");
    assert_error(is_active(constructor),
"Construct Action: Attempted to access an inactive constructor");
    assert_error(is_unit_type(u3),
"Construct Action: Encountered an invalid unit type to construct");
    consume_materials(actor, constructor, um_consumption_per_cp, u3, cp);
    consume_materials(actor, constructor, um_consumption_per_built, u3);
    consume_materials(actor, constructor, um_consumption_per_build);
}

static void
do_create_action_common_1(
    Unit *actor, Unit *creator, Unit *creation,
    int cp_spcl = -1, int acpcost_spcl = -1, long *p_mreq_spcl = NULL)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;
    int cab = FALSE;
    int cp = -1;

    assert_error(is_active(actor),
		 "Create Action: Attempted to manipulate inactive actor");
    assert_error(is_active(creator),
		 "Create Action: Attempted to manipulate inactive creator");
    assert_error(creation,
		 "Create Action: Attempted to manipulate null creation");
    u2 = creator->type;
    u3 = creation->type;
    cab = uu_create_as_build(u2, u3);
    cp = min((cab ? uu_cp_per_build(u2, u3) : uu_creation_cp(u2, u3)),
	     u_cp(u3));
    // Consume resources for this create.
    if (0 > acpcost_spcl) {
	if (cab)
	    use_up_acp(actor, uu_acp_to_build(u2, u3));
	else
	    use_up_acp(actor, uu_acp_to_create(u2, u3));
    }
    else
	use_up_acp(actor, acpcost_spcl);
    if (!p_mreq_spcl) {
	if (cab)
	    consume_materials_for_build(actor, creator, u3, cp);
	else {
	    consume_materials(actor, creator, um_consumption_per_cp, u3, cp);
	    consume_materials(actor, creator, um_consumption_on_creation, u3);
	    consume_materials(actor, creator, um_consumption_per_create);
	}
    }
    else
	consume_materials(actor, creator, p_mreq_spcl);
    // Fill in various properties.
    set_created_unit_props(creation, creator, actor->side);
    // Give the creator a handle to the new unit. 
    creator->creation_id = creation->id;
    // Modify CP as necessary.
    if (0 > cp_spcl) {
	// Transfer the creator's stash of CPs to newunit if permitted. 
	if (uu_builder_can_reuse_cp(creator->type, u3)) {
	    creation->cp += min(creator->cp_stash, u_cp(u3) - creation->cp);
	    creator->cp_stash -= 
		min(creator->cp_stash, u_cp(u3) - creation->cp);
	}
    }
    else
	creation->cp = cp_spcl;
}

static void
do_create_action_common_2(Unit *actor, Unit *creator, Unit *creation)
{
    int u3 = NONUTYPE;

    assert_error(actor,
		 "Create Action: Attempted to manipulate NULL actor");
    assert_error(creator,
		 "Create Action: Attempted to manipulate NULL creator");
    assert_error(in_play(creation),
		 "Create Action: Attempted to manipulate out-of-play creation");
    u3 = creation->type;
    // Unit might have started out complete. 
    if (completed(creation)) {
	if (uu_constructor_absorbed_by(creator->type, u3))
	    garrison_unit(creator, creation);
	make_unit_complete(creation);
    }
    else 
	record_event(H_UNIT_CREATED, 
		     add_side_to_set(actor->side, NOSIDES),
		     side_number(actor->side), creation->id);
    // Add creation to actor's side's tally of units.
    if (alive(actor))
	count_gain(actor->side, u3, build_gain);
}

/* Create-in action. */

/*! \brief Prepare for Create In Unit Action.
 *
 * This action creates the (incomplete) unit.
 * \see do_create_in_action, check_create_in_action.
 * \param unit is the pointer to the \Unit initiating the action.
 * \param creator is the pointer to the \Unit which creates the new Unit.
 * \param new_u the the type of \Unit to create.
 * \param dest is the pointer to the \Unit in which to create the new Unit.
 * \return 
 *   - TRUE if action is queued.
 *   - FALSE if
 *     - the unit pointer is NULL,
 *     - the unit action pointer is NULL,
 *     - the creator pointer is NULL, or
 *     - the dest pointer is NULL.
 */
int
prep_create_in_action(Unit *unit, Unit *creator, int new_u, Unit *dest)
{
    if (unit == NULL || unit->act == NULL || creator == NULL || dest == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_CREATE_IN;
    unit->act->nextaction.args[0] = new_u;
    unit->act->nextaction.args[1] = dest->id;
    unit->act->nextaction.actee = creator->id;
    return TRUE;
}

/*! \brief Do Create In Unit Action.
 *
 * This action creates a \Unit "in" another Unit.
 * - If the Unit being created is advanced
 *   - Figure out if the destination unit is the creator's transport
 *   - If the destination unit is the creator's transport
 *     - increment destination unit's size by 1.
 *     - garrison (destroy) the creator
 *     - update the Unit display
 *     - use up ACPs
 *     - return
 * - (If we fell through above code) create the new \Unit.
 * - If the pointer to the new \Unit is not NULL
 *    - Fill in new \Unit properties.
 *    - if creation points can be passed to the new \Unit
 *      - stuff new \Unit with as much as it can hold or is avalible,
 *      - move balances to the correct places.
 *    - Put the new \Unit in the transport (dest)
 *    - If \Unit created complete
 *      - garrison \Unit in transport
 *      - make \Unit complete
 *    - Else
 *      - record the creation event.
 *    - If creator is alive
 *      - count the gain in \Units (scorekeeping?)
 *      - for all materials (supplies)
 *        - calculate the amount of material consumed in createing new \Unit
 *        - if consumption greater than supply
 *          - figure out amount lacking
 *          - reduce supply by amount consumed
 *          - if lacking supply from unit2
 *            - remove need amount from treasury.
 *          - if not in treasury
 *            - generate run time warning (but still return good status).
 *    - use ACP needed to create new \Unit.
 *    - return
 * - else return error (no memory for new \Unit?)
 *
 * \todo Add crosscheck to make sure that an advanced unit
 *       does not exceed it's maximum size.
 *
 * \see prep_create_in_action, check_create_in_action, u_advanced,
 *      update_unit_display, garrison_unit, use_up_acp, create_unit,
 *      set_created_unit_props, uu_builder_can_reuse_cp, enter_transport,
 *      make_unit_complete, completed, record_event, add_sid_to_set, alive,
 *      count_gain, for_all_material_types, um_consumption_on_creation
 *      side_has_treasury, um_takes_from_treasury, run_warning, 
 *      use_up_acp, uu_acp_to_create.
 *
 * \param unit is the pointer to the \Unit initiating the action.
 * \param creator is pointer to \Unit performing the creation.
 * \param new_u is the new Unit type.
 * \param dest is the \Unit to create the new Unit in.
 * \return
 *   - A_ANY_ERROR if the new \Unit could not be created.
 *   - A_ANY_DONE otherwise.
 */

int
do_create_in_action(Unit *actor, Unit *creator, int u3, Unit *transport)
{
    Unit *creation = NULL;
    int u2 = NONUTYPE;

    assert_error(is_active(actor),
		 "Create Action: Attempted to manipulate inactive actor");
    assert_error(is_active(creator),
		 "Create Action: Attempted to manipulate inactive creator");
    assert_error(in_play(transport),
"Create Action: Attempted to manipulate out-of-play transport");
    // Useful info.
    u2 = creator->type;
    // Special Case.
    // Merge "creator" into its transport to increase transport's size.
    // Transport must be advanced unit.
    if (u_advanced(u3)
    	&& creator->transport && (creator->transport == transport)
    	&& (transport->type == u3)) {
	// TODO: Should test whether this kidn of joining is allowed.
	// TODO: Be more flexible with how much size is added.
	transport->size += 1;
	update_unit_display(transport->side, transport, TRUE);
	garrison_unit(creator, transport);
	use_up_acp(actor, uu_acp_to_create(u2, u3));
	return A_ANY_DONE;
    }
    // Create the new unit.
    creation = create_unit(u3, FALSE);
    // If creation succeeded...
    if (creation) {
	do_create_action_common_1(actor, creator, creation);
	// Put the new unit inside the designated transport.
	enter_transport(creation, transport);
	do_create_action_common_2(actor, creator, creation);
	return A_ANY_DONE;
    }
    // Else, we failed to create the unit.
    // Perhaps we have reached the side limit on units?
    else 
	return A_ANY_ERROR;
}

/*! \brief Check for Create in Unit Action
 *
 * Check to see if it's possible to create a \Unit in another Unit.
 *
 * \see prep_create_in_action, do_create_in_action, in_play, 
 *      u_advanced, unit_at, type_can_occupy.
 *
 * \return
 *   - the return value from can_create_common, if it is not
 *     A_ANY_OK;
 *   - A_ANY_ERROR if
 *     - destination \Unit is not in play, or
 *     - the new \Unit cannot be created in the destination Unit;
 *   - A_ANY_OK otherwise.
 */
int
check_create_in_action(Unit *actor, Unit *creator, int uc, Unit *transport)
{
    int rslt = A_ANY_OK;

    // Is transport unit in play?
    if (!in_play(transport))
	return A_ANY_ERROR;
    // If common checks are not passed, then get out.
    if (!valid(
	    rslt = 
		can_create_common(
		    actor, creator, uc, transport->x, transport->y)))
	return rslt;
    // Special Feature: Can we add a colonizer to an existing city?
    if (u_advanced(uc)
    	&& creator->transport && (creator->transport == transport)
    	&& (transport->type == uc)) 
	return A_ANY_OK;
    // Can the new utype fit in the transport?
    if (!type_can_occupy(uc, transport)) 
	return A_ANY_ERROR;
    return A_ANY_OK;
}

/*! \brief Set Created Unit Properties.
 *
 * Set the properties of the newly created unit.
 * Set various values:
 *   - hp and hp2 to 1,
 *   - cp to creation cp table value,
 *   - the pointer to the original \Side,
 *   - assign a unique unit number
 *   - set all the initial supplies, and
 *   - if morale is used, set the morale.
 *   
 * \todo Possibly should set the unit's morale from a global
 *       side morale value if there isn't a specific value
 *       for the unit.
 * \see uu_creation_cp, set_unit_origside, assign_unit_number,
 *      for_all_material_types, max, um_created_supply, min, 
 *      UM_storage_x, u_morale_max, uu_createion_morale,
 * \param newunit is a pointer to the new \Unit.
 * \param creator_u is the unit type of the creating unit.
 * \param side is the pointer to the creating \Side.
 * \param creator_mor is the creating unit's morale.
 */
static void
set_created_unit_props(Unit *newunit, Unit *creator, Side *side)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;
    int m = NONMTYPE;
    int amt;
    int mfrac;
    int mor;

    u2 = creator->type;
    u3 = newunit->type;
    newunit->hp = newunit->hp2 = 1;
    newunit->cp = 
	(uu_create_as_build(u2, u3) ? uu_cp_per_build(u2, u3) 
				    : uu_creation_cp(u2, u3)); 
    set_unit_side(newunit, side);
    set_unit_origside(newunit, side);
    /* Always number the unit when first created. */
    assign_unit_number(newunit);
    /* Set all supplies to their just-created levels. */
    for_all_material_types(m) {
        amt = newunit->supply[m];
        amt = max(amt, um_created_supply(newunit->type, m));
        /* Clip to capacity. */
        amt = min(amt, um_storage_x(newunit->type, m));
        newunit->supply[m] = amt;
    }
    /* Set the created unit's morale as a ratio to the creator's morale. */
    if (u_morale_max(newunit->type) > 0) {
        if (u_morale_max(creator->type) > 0) {
            mfrac = (creator->morale * 100) / u_morale_max(creator->type);
            mor = (uu_creation_morale(creator->type, newunit->type) * mfrac) 
		  / 100;
            /* Scale to new unit's morale range. */
            newunit->morale = (mor * u_morale_max(newunit->type)) / 100;
        } else {
            /* (should get from global side morale or some such?) */
            newunit->morale = u_morale_max(newunit->type);
        }
    }
}

/* Create-at action. */

/* Create a new unit of a given type, out in the open at a given location. */

/*! \brief Prepare for Create Unit at Location.
 *
 * Prepare to create a unit in the open at the given location.
 * \param unit is the pointer to the \Unit initiating the action.
 * \param creator is a pointer to the \Unit creating the new Unit.
 * \param new_u is the Unit type of the unit being created.
 * \param x is the cell x co-ordinate.
 * \param y is the cell y co-ordinate.
 * \param z is the altitude of the unit.
 * \return
 *   - TRUE if action is queued.
 *   - FALSE if
 *     - Acting \Unit pointer is NULL, 
 *     - Acting \Unit action pointer is NULL, or
 *     - Creating \Unit is NULL;
 */
int
prep_create_at_action(Unit *unit, Unit *creator, int new_u, int x, int y, int z)
{
    if (unit == NULL || unit->act == NULL || creator == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_CREATE_AT;
    unit->act->nextaction.args[0] = new_u;
    unit->act->nextaction.args[1] = x;
    unit->act->nextaction.args[2] = y;
    unit->act->nextaction.args[3] = z;
    unit->act->nextaction.actee = creator->id;
    return TRUE;
}

/*! \brief Do Create Unit at Location.
 *
 * Create the unit at the given location.
 * - Try to allocate a new \Unit.
 * - If allocated, 
 *    - Set the default properties of the \Unit.
 *    - If the \Unit can reuse creation points, adjust the new unit's and creator's 
 *      creation points.
 *    - If the new unit can occupy the cell
 *      - enter the cell with the new \Unit.
 *      .
 *    - else if there is space in cell and the creating \Unit can transport
 *      the new unit
 *      - make the creating \Unit leave the cell
 *      - make the new \Unit enter the cell
 *      - make the creator \Unit eter the new unit as an occupant.
 *      .
 *    - else there was an error.
 *    - if the new \Unit is completed on creation
 *      - destroy creating \Unit, if neccessary.  This is for things
 *        like creatinn a city from a colonizer that is "used up" in
 *        the process of creating the new Unit.
 *      - complete the new \Unit.
 *      .
 *    - else record the creation event.
 *    - if the creating unit is still alive (not destroyed by creating the new unit)
 *      - record any score gain from creating unit.
 *      - use supplies for creation (local and/or treasury).
 *      .
 *    - use up creation ACP from the unit.
 *    .
 *  - else ran out of memory
 *  .
 * \see create_unit, set_created_unit_props, uu_builder_can_reuse_cp,
 *      mn, u_cp, can_occupy_cell, enter_cell, can_occupy_cell_without,
 *      can_occupy, leave_cell, enter_transport, run_error, completed,
 *      garrison_unit, make_unit_complete, recod_event, add_side_to_set,
 *      side_number, alive, count_gain, for_all_material_types,
 *      um_consumption_on_creation, side_has_treasury, 
 *      run_warning, use_up_acp.
 * \param actor is the pointer to the \Unit initiating the action.
 * \param creator is a pointer to the \Unit doing the creation.
 * \param u3 is the Unit type to be created.
 * \param x is the creation location's x co-ordiate.
 * \param y is the creation location's y co-ordinate.
 * \param z is the creation location's altitude.
 * \return 
 *   - A_ANY_ERROR if no memory for \Unit creation.
 *   - A_ANY_OK if unit created.
 */

int
do_create_at_action(Unit *actor, Unit *creator, int u3, int x, int y, int z)
{
    int u2 = NONUTYPE;
    int t = NONTTYPE;
    Unit *creation = NULL;

    assert_error(is_active(actor),
		 "Create Action: Attempted to manipulate inactive unit");
    assert_error(is_active(creator),
		 "Create Action: Attempted to manipulate inactive unit");
    assert_error(is_unit_type(u3),
		 "Create Action: Encountered invalid unit type for new unit");
    assert_error(inside_area(x, y),
		 "Create Action: Attempted to place new unit outside of world");
    u2 = creator->type;
    // TODO: Handle any ZOC violations as appropriate.
    // Create the new unit.
    creation = create_unit(u3, FALSE);
    if (creation) {
	do_create_action_common_1(actor, creator, creation);
	// Put the unit where there is room for it. 
	t = terrain_at(x, y);
    	// If there is room in the cell... 
	if (type_can_occupy_cell(u3, x, y)
    	    && type_survives_in_cell(u3, x, y)) {  
	    enter_cell(creation, x, y);
	}
	// Else if the creator occupies its construction work...
	else if (can_occupy_cell_without(creation, x, y, creator)
		 && type_survives_in_cell(u3, x, y) 
		 && can_occupy(creator, creation)) {  
		leave_cell(creator);
		enter_cell(creation, x, y);
		enter_transport(creator, creation);
	}
	// Else if the creator is going to die.
	else if (can_occupy_cell_without(creation, x, y, creator)
		 && type_survives_in_cell(u3, x, y) 
		 && creator_always_dies_on_creation(creator->type, u3)) {  
		leave_cell(creator);
		enter_cell(creation, x, y);
	}
	// This should never happen. 
	//  If it does, then 'check_create_at_action' has made a mistake.
	else
	    run_error(
		"Create Action: Tried to place creation where it cannot be");
	// TODO: Adjust altitude.
	do_create_action_common_2(actor, creator, creation);
	return A_ANY_DONE;
    }
    // Else, we failed to create the unit.
    // Perhaps we have reached the side limit on units?
    else {
        return A_ANY_ERROR;
    }
}

/*! \brief Check for Create Unit at Location.
 *
 * Check to see if a \Unit may be created at cell(x,y) at altitude z.
 * \see inside_area, type_can_occupy_cell,
 *      type_can_occupy_enpty_type, type_can_occupy_cell_without,
 *      terrain_at, ut_vanished_on, ut_wrecks_on,
 *      type_can_sit_on_conn.
 * \param actor is the pointer to the \Unit initiating the action.
 * \param creator is thepointer to the \Unit creating the Unit.
 * \param u3 is the type of Unit being created.
 * \param x is the location's x co-ordinate.
 * \param y is the location's y co-ordinate.
 * \param z is the altitude of the new \Unit.
 * \return
 *   - result of can_create_common, if not A_ANY_OK
 *   - A_ANY_ERROR if
 *     - the creation location is not in the world,
 *     - the new Unit can't occupy the desired cell, or the creator can't 
 *       occupy the new Unit.
 *     - the Unit vanishes or wrecks in the cell, or can't sit 
 *       on the connection.
 *   - A_ANY_OK if it's ok to create the unit.
 */

int
check_create_at_action(
    Unit *actor, Unit *creator, int u3, int x, int y, int z)
{
    int rslt = A_ANY_OK;
    int u2 = NONUTYPE;
    int t = NONTTYPE;

    // If not inside the playing area, then error.
    if (!inside_area(x, y)) 
        return A_ANY_ERROR;
    // TODO: Check altitude.
    // If common checks are not passed, then error.
    if (!valid(rslt = can_create_common(actor, creator, u3, x, y)))
        return rslt;
    // Useful info.
    u2 = creator->type;
    // Check that the desired cell has room for the unit.
    t = terrain_at(x, y);
    // If there is room in the cell.
    if (type_can_occupy_cell(u3, x, y)
    	&& type_survives_in_cell(u3, x, y)) 
    	return A_ANY_OK;
    // Else if in same cell, and creator dies or occupies its construction work.
    if ((x == creator->x) && (y == creator->y)
	&& type_can_occupy_cell_without(u3, x, y, creator)
    	&& type_survives_in_cell(u3, x, y)
    	&& (type_can_occupy_empty_type(u2, u3)
	    || creator_always_dies_on_creation(u2, u3)))
    	return A_ANY_OK;
    // Else, there is no room.
    return A_ANY_ERROR;
}

/* Tests if a creator always dies on creation. Note that the creator may die
even if this test returns false, if its actual hp is less than hp-max. */

int
creator_always_dies_on_creation(int u, int new_u)
{
	if (uu_hp_to_garrison(u, new_u) >= u_hp_max(u)
	    && uu_creation_cp(u, new_u) >= u_cp(new_u)) {
		return TRUE;
	}	
	return FALSE;
}

/* Build action. */

/*! \brief Prepare to Build Unit in Unit.
 *
 * Sets up build operation.
 * \bug Shouldn't newunit be checked for NULL as well?  If it is 
 *      NULL, then newunit->id will probably not work as intended.
 * \param unit is the \Unit initiating the action.
 * \param builder is the \Unit building the new Unit.
 * \param newunit is the new Unit to build.
 * \return 
 *   - FALSE if 
 *     - unit pointer to unit is NULL,
 *     - pointer to unit action is NULL, or
 *     - builder pointer to actee is NULL;
 *   - TRUE otherwise.
 */
int
prep_build_action(Unit *unit, Unit *builder, Unit *newunit)
{
    if (unit == NULL || unit->act == NULL || builder == NULL) {
	return FALSE;
    }
    unit->act->nextaction.type = ACTION_BUILD;
    unit->act->nextaction.args[0] = newunit->id;
    unit->act->nextaction.actee = builder->id;
    return TRUE;
}

//! Do the build action, accepting special modifiers.

static int
do_build_action_1(
    Unit *actor, Unit *builder, Unit *newunit,
    int cp_spcl = -1, int acpcost_spcl = -1, long *p_mreq_spcl = NULL)
{
    int rslt = A_ANY_DONE;
    int cp = -1;
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_active(actor), 
		 "Build Action: Attempted to manipulate inactive actor");
    assert_error(is_active(builder), 
		 "Build Action: Attempted to manipulate inactive builder");
    assert_error(in_play(newunit), 
"Build Action: Attempted to manipulate out-of-play unit-to-build");
    u2 = builder->type;
    u3 = newunit->type;
    // Calculate CP gained for this build.
    cp = min(uu_cp_per_build(u2, u3), u_cp(u3) - newunit->cp);
    // Consume resources for this build.
    if (0 > acpcost_spcl)
	use_up_acp(actor, uu_acp_to_build(u2, u3));
    else
	use_up_acp(actor, acpcost_spcl);
    if (!p_mreq_spcl)
	consume_materials_for_build(actor, builder, u3, cp);
    else
	consume_materials(actor, builder, p_mreq_spcl);
    // Increment the new unit's CP.
    if (0 > cp_spcl)
	newunit->cp += cp;
    else
	newunit->cp += cp_spcl;
    // Handle completeness.
    if (completed(newunit)) {
	if (uu_constructor_absorbed_by(u2, u3))
	    garrison_unit(builder, newunit);
        make_unit_complete(newunit);
	// TODO: Implement consumption-to-complete.
	rslt = A_BUILD_COMPLETED;
    }
    update_unit_display(newunit->side, newunit, TRUE);
    return rslt;
}

/*! \brief Do Build Unit in Unit Action.
 *
 * Build a unit.
 * - for all materials
 *   - reduce the supply of the building \Unit by the proportional amount based
 *     on construction points.
 * - increase constrution points for the unit for this build
 * - if \Unit completed,
 *   - get rid of building \Unit, if neccessary
 *   - make \Unit complete,
 *   - update Unit display,
 *   - use up ACP.
 * - else
 *   - update Unit display
 *   - use up ACP
 * \see for_all_material_types, side_has_treasury, um_takes_from_treasury
 *      run_warning, uu_cp_per_build, completed, garrison_unit,
 *      make_unit_complete, update_unit_display, use_up_acp.
 * \param actor is the pointer to the \Unit initiating the action.
 * \param builder is the pointer to \Unit building the new Unit.
 * \param newunit is the pointer to \Unit being built.
 * \return
 *   - A_BUILD_COMPLETED if new \Unit is completed.
 *   - A_ANY_DONE if build done.
 */

int
do_build_action(Unit *actor, Unit *builder, Unit *newunit)
{
    return do_build_action_1(actor, builder, newunit);
}

/*! \brief Garrison Unit.
 *
 * This name is somewhat confusing.  It uses the uu_hp_to_garrison table
 * to determine if the unit should be removed.  It is used to remove a
 * building Unit upon creating a new Unit.  An example would be a 
 * colonizer/settler/engineer disappearing when a villiage/town/city is
 * built.  It is also sometimes used when a unit captures a unit, such as
 * an army capturing a city, and being destroyed in the process of 
 * capturing the city.
 * - If the hit points to create/capture (hp_to_garrison) is greater than 
 * the remaining hit points of the building \Unit
 *   - remove the new \Unit from the cell,
 *   - if the new \Unit is being transported by the building/capturing Unit
 *     - force the new \Unit out of the building/capturing Unit.
 *     - if the there is a transport availble (not currently implemented)
 *       - put the new \Unit in the transport
 *     - else
 *       - put the new \Unit in the cell.
 *   - if the new \Unit is being transported, but not by the 
 *     building/capturing Unit
 *     - set the alternate transport local variable to the transporting \Unit.
 *   - for all occupants of the building/capturing \Unit
 *     - if the occupant of the building \Unit can occupy the new
 *       Unit
 *       - remove the occupant from the new \Unit.
 *       - update the Unit display.
 *       - add the occupant to the new \Unit.
 *     - else if the alternate transport can hold the occupant
 *       - remove the ocupant from the building/capuring \Unit.
 *       - update the Unit display.
 *       - add the occupant to the alternate transport.
 *     - else if the occupant can enter the cell
 *       - remove the occupant from the building/capturing \Unit.
 *       - update the Unit display.
 *       - add the occupant to the cell.
 *     - By default, leave the occupant in the building/capturing \Unit to be
 *       destroyed with the building/capturing Unit.
 *   - Set the temporary event data 1 to the id of the new \Unit.
 *   - Kill the building/capturing Unit.
 * - else reduce the building/capturing hp and hp2 by the hp to garrison
 *   amount. \note if this occurs before damage is calculated, hp and hp2
 *   may be different.
 * \todo Add code to handle the garrison \Unit being put in a transport?
 * \todo Add code to handle sub-occupants of doomed occupants?
 * \todo Add code to post event for garrisoning damage?
 * \see uu_hp_to_garrison, leave_cell, leave_trasnport, enter_transport,
 *      can_occupy, update_unit_display, kill_unit.
 *  \param builder is a pointer to the \Unit building/capturing the unit.
 *  \param newunit is a pointer to the \Unit being built/captured.
 */
void
garrison_unit(Unit *builder, Unit *newunit)
{
    int x = builder->x;		/* x co-ordinate of builder */
    int y = builder->y;		/* y co-ordinate of builder */
    Unit *transport = NULL;	/* pointer to transport unit */
    Unit *occ;			/* pointer to occupant unit */
    Unit *nextocc;		   	/* pointer to next occupant unit */

    /* Maybe get rid of the building unit if it is to be the garrison. */
    if (uu_hp_to_garrison(builder->type, newunit->type) >= builder->hp) {
	/* But first get the about-to-be-killed garrisoning unit
	   disconnected from everything. */
	leave_cell(builder);
	/* Put new unit in place of the garrisoning one, if it was an
           occupant. */
	if (newunit->transport == builder) {
	    leave_transport(newunit);
	    enter_cell(newunit, x, y);
	}
	if (newunit->transport != NULL && newunit->transport != builder)
	  transport = newunit->transport;
	/* for_all_occupants will not work here, 
	   since leave_transport changes occ->nexthere */
	for (occ = builder->occupant; occ != NULL; occ = nextocc) {
	    nextocc = occ->nexthere;
	    /* Move the other occupants anywhere we can find. */
	    if (can_occupy(occ, newunit)) {
		// leave_cell won't work here, 
		//  since "builder" already left cell */
		leave_transport(occ);
		update_unit_display(builder->side, builder, TRUE);
		enter_transport(occ, newunit);
	    } else if (transport != NULL && can_occupy(occ, transport)) {
		leave_transport(occ);
		update_unit_display(builder->side, builder, TRUE);
		enter_transport(occ, transport);
	    } else if (type_can_occupy_cell(occ->type, x, y)
		       && type_survives_in_cell(occ->type, x, y)) {
	    	// We test for type_survives_in_cell here,
		//   since we call enter_cell directly,
		//   and thus bypass the unit-wrecking code in do_move_action.
		leave_transport(occ);
		update_unit_display(builder->side, builder, TRUE);
		enter_cell(occ, x, y);
	    }
	    /* Otherwise the occupant has to die along with the garrison. */
	    /* (should also do something with sub-occs of doomed occs?) */
	}
	/* Now we can get rid of the garrisoning unit without scrambling
	   anything else. */
	tmphevtdata1 = newunit->id;
	kill_unit(builder, H_UNIT_GARRISONED);
    } else {
	/* Note that if this all happens before damage is reckoned,
	   hp and hp2 might be different. */
	builder->hp -= uu_hp_to_garrison(builder->type, newunit->type);
	builder->hp2 -= uu_hp_to_garrison(builder->type, newunit->type);
	/* (should record loss of hp as garrison event?) */
    }
}

/*! \brief Check for Build Unit in Unit Action.
 *
 * Check to see if a Unit can continue to be built.
 * \see prep_build_action, do_build_action, in_play, uu_acp_to_build,
 *      can_have_enough_acp, uu_cp_per_build, uu_tp_to_build,
 *	fillsized, dostamce. uu_build_range, completed, uu_occ_can_build,
 *	has_enough_acp, um_to_build.
 * \param actor is the pointer to the \Unit initiating the action.
 * \param builder is the pointer to \Unit building the new Unit.
 * \param buildee is the pointer to \Unit being built.
 */

int
check_build_action(Unit *actor, Unit *builder, Unit *buildee)
{
    int rslt = A_ANY_OK;
    int u3 = NONUTYPE;

    if (!in_play(buildee))
	return A_ANY_ERROR;
    u3 = buildee->type;
    if (!valid(rslt = can_build(actor, builder, buildee)))
      return rslt;
    if (completed(buildee)) 
	return A_ANY_ERROR;
    return A_ANY_OK;
}

void
grant_occs_on_completion(Unit *unit)
{
    static long *p_mreq;

    int u = NONUTYPE, u2 = NONUTYPE;
    Side *side = NULL;
    int i = -1, imax = -1;
    Unit *unit2 = NULL;

    assert_error(in_play(unit),
		 "Complete Action: Attempted to access an out-of-play unit");
    u = unit->type;
    side = unit->side;
    if (!p_mreq)
	p_mreq = (long *)xmalloc(nummtypes * sizeof(long));
    for_all_unit_types(u2) {
	if (!uu_complete_occs_on_completion(u, u2)
	    && !uu_alt_complete_occs_on_completion(u, u2)
	    && !uu_incomplete_occs_on_completion(u, u2)
	    && !uu_alt_incomplete_occs_on_completion(u, u2))
	    continue;
	if (!type_can_occupy(u2, unit)) {
	    run_warning(
"Cannot fit any implicitly created %s units into newly completed %s",
			u_type_name(u2), medium_long_unit_handle(unit));
	    continue;
	}
	// Determine number of complete occs-on-completion.
	imax = prob_fraction(uu_complete_occs_on_completion(u, u2));
	imax += max(0, roll_dice1(uu_alt_complete_occs_on_completion(u, u2)));
	// Try to add complete occs-on-completion.
	for (i = 0; i < imax; ++i) {
	    if (!type_can_occupy(u2, unit)) {
		run_warning(
"Cannot fit any more implictly created %s units into newly completed %s",
			    u_type_name(u2), medium_long_unit_handle(unit));
		break;
	    }
	    if (!valid(can_create_common(
			unit, unit, u2, unit->x, unit->y, TRUE, 0, p_mreq))
		|| !(unit2 = create_unit(u2, FALSE))) {
		notify(side, 
"Failed to implicitly create %s unit in newly completed %s",
		       u_type_name(u2), medium_long_unit_handle(unit));
		break;
	    }
	    do_create_action_common_1(unit, unit, unit2, u_cp(u2), 0, p_mreq);
	    enter_transport(unit2, unit);
	    do_create_action_common_2(unit, unit, unit2); 
	} // for all complete occs
	// Determine number of incomplete occs-on-completion.
	imax = prob_fraction(uu_incomplete_occs_on_completion(u, u2));
	imax += max(0, roll_dice1(uu_alt_incomplete_occs_on_completion(u, u2)));
	for (i = 0; i < imax; ++i) {
	    if (!type_can_occupy(u2, unit)) {
		run_warning(
"Cannot fit any more implictly created %s units into newly completed %s",
			    u_type_name(u2), medium_long_unit_handle(unit));
		break;
	    }
	    if (!valid(can_create_common(
			unit, unit, u2, unit->x, unit->y, TRUE, 0, p_mreq))
		|| !(unit2 = create_unit(u2, FALSE))) {
		notify(side, 
"Failed to implicitly create %s unit in newly completed %s",
		       u_type_name(u2), medium_long_unit_handle(unit));
		break;
	    }
	    do_create_action_common_1(unit, unit, unit2, 0, 0, p_mreq);
	    enter_transport(unit2, unit);
	    do_create_action_common_2(unit, unit, unit2); 
	} // for all incomplete occs
    } // for all unit types
}

/*!
 * \brief Complete Unit.
 *
 * Do all the little things to make a fully operational unit.
 *   - if a unit has multiple parts ( fleet or army, for example),
 *     it is not created at full size, but at a size of 1, so
 *     construction points and hit points are set appropriately.
 *   - Give the unit a name.
 *   - set the view coverage area of the unit.
 *   - if unit creation cuases a change in control of the cell,
 *     remove enemy units.
 *   - handle material supplied, created, and/or shared for created
 *     unit.
 *   - initialize action state.
 *  
 * \see max, u_cp, u_hp, make_up_unit_name, cover_area,
 * Kick_out_enemy_users, for_all_material_types, um_completed_supply,
 * um_storage_x, try_sharing, init_unit_actorstate, init_unit_plan,
 * set_unit_acp_for_turn, make_unit_vector, clear_unit_vector, 
 * add_unit_to_vector, for_all_sides, trusted_side, add_side_to_set, 
 * record_event, side_number, see_all_cell, printf
 * \param unit is the unit to be completed.
 */

void
make_unit_complete(Unit *unit)
{
    int u = NONUTYPE;
    int m = NONMTYPE;
    SideMask observers;
    Side *side = NULL, *side2 = NULL;

    assert_error(in_play(unit),
		 "Complete Action: Attempted to access an out-of-play unit");
    u = unit->type;
    side = unit->side;
    unit->cp = u_cp(u);
    unit->hp = unit->hp2 = u_hp(u);
    /* Christen our new unit. Its serial number (if it is a type that has
       one) was assigned just after its creation. */
    make_up_unit_name(unit);
    /* It also starts viewing its surroundings. */
    cover_area(side, unit, unit->transport, -1, -1, unit->x, unit->y);
    /* Kick out enemy users if we control this cell. */
    kick_out_enemy_users(side, unit->x, unit->y);
    /* Update the cancarry vector if this is a transport. */
    if (u_is_transport(u)) 
    	update_cancarry_vector(side);
    /* Set all the supplies up to their unit-just-completed levels. */
    for_all_material_types(m) {
	unit->supply[m] = max(unit->supply[m], um_completed_supply(u, m));
	unit->supply[m] = min(unit->supply[m], um_storage_x(u, m));
    }
    // Add any incomplete or complete occs that come as "part of the package".
    grant_occs_on_completion(unit);
#if (1)
    /* The new unit may not need any materials due to being assigned 
	a stationary goal. Let the economy code decide wheter or not to fill 
	it up with materials. */ 
    /* Also see if anybody here is willing to share to make up any
       deficiencies before the end of the turn. */
    for_all_material_types(m) {
	if (unit->transport)
	  try_sharing(unit->transport, unit, m);
    }
#endif
    init_unit_actorstate(unit, FALSE);
    init_unit_plan(unit);
    /* Put this unit into action immediately, at full acp. */
    if (unit->act) {
	set_unit_acp_for_turn(unit);
	if (unit->act->initacp > 0) {
	   /* Make_unit_complete may be called by advanced units at
	      turn 0 before compose_actionvectors has been called! */
	   if (unit->side->actionvector == NULL) {
		unit->side->actionvector = make_unit_vector(max(numunits, 100));
		clear_unit_vector(unit->side->actionvector);
	   }
	   unit->side->actionvector = 
	    add_unit_to_vector(unit->side->actionvector, unit, 0);
	}
    }
    /* Inform all sides that should know about the completion. */
    observers = NOSIDES;
    for_all_sides(side2) {
	if (side2 == unit->side
	    || trusted_side(unit->side, side2)
	    /* (or add all sides if g_see_all?) */
	    ) {
	    observers = add_side_to_set(unit->side, observers);
	}
    }
    record_event(H_UNIT_COMPLETED, observers, side_number(unit->side),
		 unit->id);
    /* Make sure the image of the newly completed unit 
	(and its name) is drawn. */
    all_see_cell(unit->x, unit->y);
    /* (should add to any per-side tallies) */
    Dprintf("%s is completed\n", unit_desig(unit));
}

/* Repair action. */

/*!
 * \brief Prepare for Unit Repair Action.
 */

int
prep_repair_action(Unit *actor, Unit *repairer, Unit *repairee)
{
    if (!is_active(actor) || !is_active(repairer) || !in_play(repairee))
	return FALSE;
    if (!repairer->act)
	return FALSE;
    repairer->act->nextaction.type = ACTION_REPAIR;
    repairer->act->nextaction.args[0] = repairee->id;
    repairer->act->nextaction.actee = repairer->id;
    return TRUE;
}

/*!
 * \brief Do Repair Unit Action.
 *
 * Use a unit to repair another unit.
 *   - Calculate amount of hitpoints that can be repaired.
 *   - Adjust the repaired unit's hitpoints.
 *   - Use up any materials required.
 *   - Use up any ACP needed.
 * \see uu_repair, add_to_unit_hp, prob_fraction,
 *      for_all_material_types, um_comsumption_per_repair,
 *      use_up_acp, uu_acp_to_repair.
 * \return A_ANY_DONE always.
 */

int
do_repair_action(Unit *acpsrc, Unit *repairer, Unit *repairee)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;

    assert_error(is_active(acpsrc),
		 "Repair Action: Attempted to manipulate an inactive unit");
    assert_error(is_active(repairer),
		 "Repair Action: Attempted to manipulate an inactive unit");
    assert_error(in_play(repairee),
		 "Repair Action: Attempted to manipulate an out-of-play unit");
    // Useful info.
    u2 = repairer->type;
    u3 = repairee->type;
    // If at turn start, then we consider auto-repair mechanisms.
    if (at_turn_start) {
	// Hp-recovery mechanism.
	if ((repairer == repairee)
	    && (0 < u_hp_recovery(u2)) 
	    && (repairee->hp >= u_hp_to_recover(u2))) {
	    add_to_unit_hp(repairee, prob_fraction(u_hp_recovery(u2)));
	}
	// Auto-repair mechanism.
	if (0 < uu_auto_repair(u2, u3)) {
	    add_to_unit_hp(repairee, prob_fraction(uu_auto_repair(u2, u3)));
	    consume_materials(acpsrc, repairer, um_consumption_per_repair);
	    consume_materials(acpsrc, repairer, um_consumption_per_repaired, 
			      u3);
	}
    }
    // Else during turn, then we consider explicit repair.
    else {
	add_to_unit_hp(repairee, prob_fraction(uu_hp_per_repair(u2, u3)));
	consume_materials(acpsrc, repairer, um_consumption_per_repair);
	consume_materials(acpsrc, repairer, um_consumption_per_repaired, u3);
	use_up_acp(acpsrc, uu_acp_to_repair(u2, u3));
    }
    return A_ANY_DONE;
}

/*!
 * \brief Check for Repair Unit Action.
 *
 * Check to see if the repair action can be done.
 * Validate:
 *   - All units are in play.
 *   - The repairng unit can repair the repaired unit.
 *   - Sufficient materials are on hand to do the repair.
 *   - the unit has enought ACP to do the repair this turn.
 * \see in_play, uu_acp_to_repair, can_have_enough_acp, uu_hp_to_repair,
 *      for_all_material_types, um_to_repair, um_consumption_per_repair,
 *      has_enough_acp.
 */

int
check_repair_action(Unit *actor, Unit *repairer, Unit *repairee)
{
    int rslt = A_ANY_OK;

    // Is the repairee in play?
    if (!in_play(repairee))
	return A_ANY_ERROR;
    // Can explicit repair occur during turn?
    if (!at_turn_start && !valid(rslt = can_repair(actor, repairer, repairee)))
	return rslt;
    // Can auto-repair occur at turn start?
    if (at_turn_start 
	&& !valid(rslt = can_auto_repair(repairer, repairee)))
	return rslt;
    return A_ANY_OK;
}

/* Disband action. */

/*!
 * \brief Prepare for Unit Disband Action.
 *
 * Set up to do a disband action.
 * The disband action destroys a unit in an "orderly" fashion, and can be
 * undertaken voluntarily.
 * \param unit is a pointer to the \Unit initiating the action.
 * \param disbanded is a pointer to the unit being disbanded.
 * \return
 *    - true if the action is queued;
 *    - false if
 *      - unit pointer is NULL,
 *      - the unit action pointer is NULL, or
 *      - the disbanded pointer is NULL.
 */
int
prep_disband_action(Unit *unit, Unit *disbanded)
{
    if (unit == NULL || unit->act == NULL || disbanded == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_DISBAND;
    unit->act->nextaction.actee = disbanded->id;
    return TRUE;
}

/*!
 * \brief Do Unit Disband Action.
 *
 * Disband the unit by:
 *   - distributing as much of it's supplies as possble,
 *   - if it has more hit points than needed to disband, recover
 *     as much of it's disbanding materials as possible, 
 *   - kill the unit, and
 *   - use it's ACP.
 * \see for_all_material_types, um_supply_per_disband, distribute_material.
 *      hp_per_disband, for_all_material_types, um_recycleable, 
 *      distribute_material, kill_unit, use_up_acp, u_acp_to_disband.
 *
 * \GDL
 *    - <A href="xcdesign.html#GDL_ref_acp-to-disband">acp-to-disband</A>
 *    .
 * \param unit is a pointer to the \Unit initiating the action.
 * \param disbanded is a pointer to the unit being disbanded.
 * \return A_ANY_OK always.
 */
int
do_disband_action(Unit *unit, Unit *disbanded)
{
    int m, amt, disb;

    /* Recover some percentage of the unit's supply. */
    for_all_material_types(m) {
    	if (um_supply_per_disband(disbanded->type, m) > 0 && disbanded->supply[m] > 0) {
    	    amt = (disbanded->supply[m] * um_supply_per_disband(disbanded->type, m)) / 100;
    	    /* Unit always loses the amount, whether or not distributed. */
    	    disbanded->supply[m] -= amt;
    	    distribute_material(disbanded, m, amt);
    	}
    }
    /* Remove hit points or kill the unit directly. */
    disb = u_hp_per_disband(disbanded->type);
    if (disb < disbanded->hp) {
	disbanded->hp -= disb;
	disbanded->hp2 = disbanded->hp;
    } else {
    	/* Pass around whatever we can get out of the unit itself. */
    	for_all_material_types(m) {
    	    if (um_recycleable(disbanded->type, m) > 0) {
    	    	distribute_material(disbanded, m, um_recycleable(disbanded->type, m));
    	    }
    	}
	kill_unit(disbanded, H_UNIT_DISBANDED);
    }
    use_up_acp(unit, u_acp_to_disband(disbanded->type));
    return A_ANY_DONE;
}

/*
 * \brief Distribute Material to Units.
 *
 * \todo The adjacent distribution code and the ranged distribution
 * code are not the same.  Question: why not?  FIXME in general: why is
 * the algorithm here so different from run_economy?  Should
 * perhaps merge some of the code or at least ideas.  But I'm not
 * sure run_economy is quite right either in terms of making sure
 * that it transfers supplies rather than losing them due to
 * being full. 
 * <P> Stanley M. Sutton comment. 
 * <P> To be really accurate, this should be iterative, and 
 * prioritorized.  If all units in range are full, the algorithym
 * should check to see if the full units can transfer supplies
 * to non-full units within their range, and so on.
 * Since this could use an appreciable amount of ACP if there are ACP
 * costs involved, it should really be solved by a global optimization
 * algorithm that can figure out the best way to disribute all supplies
 * to be distruted with the minimum outlay of ACP, perhaps limited by 
 * available ACP, a doctrine, a plan, or a player settable value as a
 * percentage of the turn's available ACP, or a player setable maximem
 * number.
 * <P>Should we distribute material to other cccupants on the
 * same transport before checking the stack?
 * <P>Should we check for units inside transports on the same
 * cell?
 * \parm unit is the \Unit distributing material.
 * \param m is the type of material being distributed.
 * \param amt is the amount of material being distributed.
 * \see give_away, for_all_stack, unit_trusts_unit, for_all_directions,
 *      interior_point_in_dir, 
 * Given a unit and a quantity of material, pass it out to nearby units.
 */

void
distribute_material(Unit *unit, int m, int amt)
{
    int dir, x1, y1, dist;
    Unit *unit2;

    /* Distribute to transport first. */
    if (unit->transport != NULL) {
	amt = give_away(unit->transport, m, amt);
	if (unit->transport->transport != NULL) {
		amt = give_away(unit->transport->transport, m, amt);
    	}
    }
    /* Then to any unit in the cell. */
    for_all_stack_with_occs(unit->x, unit->y, unit2) {
    	if (amt <= 0) {
    		break;
    	}
	if (unit2 != unit && unit_trusts_unit(unit, unit2)) {
		amt = give_away(unit2, m, amt);
    	}
    }
    /* Then to any unit in an adjacent cell. */
    for_all_directions(dir) {
	if (amt <= 0) {
		break;
	}
	if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
		for_all_stack(x1, y1, unit2) {
			if (amt <= 0) {
				break;
			}
			if (unit_trusts_unit(unit, unit2)) {
				amt = give_away(unit2, m, amt);
			}
		}
	}
    }
    /* Then to any unit within range.  This in addition to the above
     * code for adjacent cells because the latter does not check in-length
     * and out-length.
     */
	for_all_cells_within_range(unit->x, unit->y, 
				   um_outlength (unit->type, m), x1, y1) {
	    if (amt <= 0) {
		break;
	    }
	    if (!inside_area(x1, y1)) {
		continue;
	    }
	    if (!terrain_visible(unit->side, x1, y1)) {
		continue;
	    }
	    dist = distance(unit->x, unit->y, x1, y1);
	    /* Include all occupants like try_transfer_to_cell does for
	       run_economy.  */
	    for_all_stack_with_occs(x1, y1, unit2) {
		if (amt <= 0) {
			goto done;
		}
		if (is_active(unit2)
		    && unit_trusts_unit(unit, unit2)
		    && um_inlength (unit2->type, m) >= dist) {
		    amt = give_away (unit2, m, amt);
		}
	    }
	}
 done:;
}

/* Give as much as possible of the given material to the unit,
   return the amount left to give away. */

static int
give_away(Unit *unit, int m, int amt)
{
    int space, add;

    /* We can't give away a negative amount. */
    if (amt <= 0) {
    	return amt;
    }
    space = um_storage_x(unit->type, m) - unit->supply[m];
    add = (amt < space ? amt : space);
    unit->supply[m] += add;
    amt -= add;
    if (add > 0)
      update_unit_display(unit->side, unit, TRUE);
    return amt;
}

int
check_disband_action(Unit *unit, Unit *disbanded)
{
    int acp;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(disbanded))
      return A_ANY_ERROR;
    acp = u_acp_to_disband(disbanded->type);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (u_hp_per_disband(disbanded->type) <= 0)
      return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp))
      return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Change-type action. */

int
prep_change_type_action(Unit *unit, Unit *unit2, int u3)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_CHANGE_TYPE;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* Actually change the type of a unit. */

int
do_change_type_action(Unit *unit, Unit *unit2, int u3)
{
    int u, u2;
    int acpdebt = 0;

    u = unit->type;  u2 = unit2->type;
    acpdebt = uu_acp_to_change_type(u2, u3);
    change_unit_type(unit2, u3, H_UNIT_TYPE_CHANGED, NULL);
    update_unit_display(unit2->side, unit2, TRUE);
    if (!acp_indep(unit) && unit->act) {
	if (type_acp_max(u) > 0)
	  acpdebt = (acpdebt * type_acp_max(u3)) / type_acp_max(u);
	else
	  acpdebt = 0;
	use_up_acp(unit, min(acpdebt, unit->act->acp - u_acp_min(u3)));
    }
    return A_ANY_DONE;
}

//! Check the 'change-type' action.
/*!
    Arbiter Function.
*/

int
check_change_type_action(Unit *unit, Unit *unit2, int u3)
{
    int rslt = A_ANY_OK;

    assert_return(in_play(unit), A_ANY_ERROR);
    assert_return(in_play(unit2), A_ANY_ERROR);
    assert_return(is_unit_type(u3), A_ANY_ERROR);
    if (!valid(rslt = can_change_type_to(unit, unit2, u3)))
      return rslt;
    return A_ANY_OK;
}

/* Change-side action. */

/* Tell a unit to change to a given side. */

/* (what about occs, garrisons, plans?) */

int
prep_change_side_action(Unit *unit, Unit *unit2, Side *side)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_CHANGE_SIDE;
    unit->act->nextaction.args[0] = side_number(side);
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_change_side_action(Unit *unit, Unit *unit2, Side *side)
{
    int rslt;

    if (side_controls_unit(unit->side, unit2)) {
	/* If we own it, we can just change it. */
	change_unit_side(unit2, side, H_UNIT_ACQUIRED, NULL);
	rslt = A_ANY_DONE;
    } else {
	rslt = A_ANY_ERROR;
    }
    use_up_acp(unit, u_acp_to_change_side(unit2->type));
    return rslt;
}

int
check_change_side_action(Unit *unit, Unit *unit2, Side *side)
{
    int u, u2, acp;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!side_in_play(side))
      return A_ANY_ERROR;
    if (unit2->side == side)
      return A_ANY_ERROR;
    if (!unit_allowed_on_side(unit2, side))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = u_acp_to_change_side(u2);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (!can_have_enough_acp(unit, acp))
      return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp))
      return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Alter-terrain action. */

/* Change the terrain in the cell to something else. */

/* We don't need to ensure that the unit can exist on the new terrain
   type, because the designer is presumed to have set things up sensibly,
   because the unit might be in an appropriate transport, or because
   there is some actual use in such a bizarre shtick. */

/* What if engineers dig hole underneath enemy unit?  Should this be
   possible, or should there be a "can-dig-under-enemy" parm?  */

int
prep_alter_cell_action(Unit *unit, Unit *unit2, int x, int y, int t)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_ALTER_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_alter_cell_action(Unit *unit, Unit *unit2, int x, int y, int t)
{
    int u, u2, oldt, acpr, acpa, m, amt, excess, space;

    u = unit->type;  u2 = unit2->type;
    oldt = terrain_at(x, y);
    /* Change the terrain to the new type. */
    change_terrain_type(x, y, t);
    for_all_material_types(m) {
	amt = tm_material_per_remove_terrain(oldt, m) - tm_consumption_per_add_terrain(t, m);
	space = um_storage_x(unit2->type, m) - unit2->supply[m];
    	/* First fill up unit2 as far as there is room. */
	unit2->supply[m] += min(amt, space);
	/* Compute the excess we couldn't store. */
	excess = amt - space;
	/* Try to give away the same amount to make room for the excess. */
	if (excess > 0) {
	    /* But first clip it to available supplies. */
	    excess = min(excess, unit2->supply[m]);
	    unit2->supply[m] -= excess;
	    distribute_material(unit2, m, excess);
	}
    }
    /* Then try to fill up unit2 again with the remaining excess. */
    space = um_storage_x(unit2->type, m) - unit2->supply[m];
    /* The check for available space is redundant now, but will be needed
    in the future when distribute_material has been improved. */
    unit2->supply[m] += min(excess, space);

    /* Note that we still charge acp even if terrain type doesn't change. */
    acpr = ut_acp_to_remove_terrain(u2, oldt);
    acpa = ut_acp_to_add_terrain(u2, t);
    use_up_acp(unit, acpr + acpa);
    return A_ANY_DONE;
}

int
check_alter_cell_action(Unit *unit, Unit *unit2, int x, int y, int t)
{
    int u, u2, m, oldt, acpr, acpa;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!in_area(x, y))
      return A_ANY_ERROR;
    if (!is_terrain_type(t))
      return A_ANY_ERROR;
    if (!t_is_cell(t))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    oldt = terrain_at(x, y);
    acpr = ut_acp_to_remove_terrain(u2, oldt);
    acpa = ut_acp_to_add_terrain(u2, t);
    if (acpr < 1 || acpa < 1)
      return A_ANY_CANNOT_DO;
    if (!can_have_enough_acp(unit, acpr + acpa))
      return A_ANY_CANNOT_DO;
    if (distance(unit2->x, unit2->y, x, y) > ut_alter_range(u2, t))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, acpr + acpa))
      return A_ANY_NO_ACP;
    /* We have to have a minimum level of supply to be able to do the
       action. */
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_remove_terrain(u2, m))
	  return A_ANY_NO_MATERIAL;
	if (unit2->supply[m] < um_to_add_terrain(u2, m))
	  return A_ANY_NO_MATERIAL;
	if (unit2->supply[m] < (tm_consumption_per_add_terrain(t, m) - tm_material_per_remove_terrain(oldt, m)))
	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* Add-terrain action. */

/* Add terrain; border, connection, or coating. */

int
prep_add_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_ADD_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = dir;
    unit->act->nextaction.args[3] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_add_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    int u = unit->type, oldval, newval, m, amt;
    Side *side;

    switch (t_subtype(t)) {
      case cellsubtype:
      	/* Will never happen. */
      	break;
      case bordersubtype:
      	oldval = border_at(x, y, dir, t);
      	newval = TRUE;
	set_border_at(x, y, dir, t, newval);
      	break;
      case connectionsubtype:
      	oldval = connection_at(x, y, dir, t);
      	newval = TRUE;
	set_connection_at(x, y, dir, t, newval);
      	break;
      case coatingsubtype:
    	oldval = aux_terrain_at(x, y, t);
      	/* Interpret "dir" as depth of coating to add. */
    	newval = min(oldval + dir, tt_coat_max(terrain_at(x, y), t));
    	set_aux_terrain_at(x, y, t, newval);
      	break;
    }
    /* Consume any material necessary to the action. */
    for_all_material_types(m) {
	amt = tm_consumption_per_add_terrain(t, m);
        if (side_has_treasury(unit2->side, m)
	    && um_takes_from_treasury(unit2->type, m)
	    && (amt > unit2->supply[m])) {
	    amt -= unit2->supply[m];
	    unit2->supply[m] = 0;
	    unit2->side->treasury[m] -= amt;
	} else
	    unit2->supply[m] -= amt;
    }
    /* Let everybody see what has happened. */
    for_all_sides(side) {
	if (active_display(side)) {
	    if (terrain_visible(side, x, y))
	      update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    	}
    }
    use_up_acp(unit, (newval != oldval ? ut_acp_to_add_terrain(u, t) : 1));
    return A_ANY_DONE;
}

int
check_add_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    int u, u2, m, acp, treas;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!inside_area(x, y))
      return A_ANY_ERROR;
    if (!between(0, dir, NUMDIRS - 1))
      return A_ANY_ERROR;
    if (!is_terrain_type(t))
      return A_ANY_ERROR;
    if (t_is_cell(t))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = ut_acp_to_add_terrain(u2, t);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
     if (!can_have_enough_acp(unit, acp))
       return A_ANY_CANNOT_DO;
   if (distance(unit->x, unit->y, x, y) > ut_alter_range(u2, t))
      return A_ANY_TOO_FAR;
    if (!has_enough_acp(unit, acp))
      return A_ANY_NO_ACP;
    /* We have to have certain amounts supply to be able to do the action. */
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_add_terrain(u2, m))
	  return A_ANY_NO_MATERIAL;
        if (side_has_treasury(unit2->side, m)
	    && um_takes_from_treasury(u2, m))
	    treas = unit2->side->treasury[m];
        else
	    treas = 0;
	if ((unit2->supply[m] + treas) < tm_consumption_per_add_terrain(t, m))
	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* Remove-terrain action. */

/* Remove a border, connection, or coating. */

int
prep_remove_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_REMOVE_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = dir;
    unit->act->nextaction.args[3] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_remove_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    int u = unit->type, oldval, newval, m, amt, excess, space;
    Side *side;

    switch (t_subtype(t)) {
      case cellsubtype:
      	/* Will never happen. */
      	break;
      case bordersubtype:
	oldval = border_at(x, y, dir, t);
	newval = FALSE;
	set_border_at(x, y, dir, t, newval);
      	break;
      case connectionsubtype:
	oldval = connection_at(x, y, dir, t);
	newval = FALSE;
	set_connection_at(x, y, dir, t, newval);
      	break;
      case coatingsubtype:
    	oldval = aux_terrain_at(x, y, t);
       	/* Interpret "dir" as depth of coating to remove. */
   	newval = max(oldval - dir, 0);
   	/* If newval drops below the min coating depth, coating will vanish. */
    	if (newval < tt_coat_min(terrain_at(x, y), t))
    	  newval = 0;
    	set_aux_terrain_at(x, y, t, newval);
      	break;
    }
    for_all_material_types(m) {
	amt = tm_material_per_remove_terrain(t, m);
	space = um_storage_x(unit2->type, m) - unit2->supply[m];
    	/* First fill up unit2 as far as there is room. */
	unit2->supply[m] += min(amt, space);
	/* Compute the excess we couldn't store. */
	excess = amt - space;
	/* Try to give away the same amount to make room for the excess. */
	if (excess > 0) {
	    /* But first clip it to available supplies. */
	    excess = min(excess, unit2->supply[m]);
	    unit2->supply[m] -= excess;
	    distribute_material(unit2, m, excess);
	}
    }
    /* Then try to fill up unit2 again with the remaining excess. */
    space = um_storage_x(unit2->type, m) - unit2->supply[m];
    /* The check for available space is redundant now, but will be needed
    in the future when distribute_material has been improved. */
    unit2->supply[m] += min(excess, space);

    /* Let everybody see what has happened. */
    for_all_sides(side) {
	if (active_display(side)) {
	    if (terrain_visible(side, x, y))
	      update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    	}
    }
    use_up_acp(unit, (newval != oldval ? ut_acp_to_remove_terrain(u, t) : 1));
    return A_ANY_DONE;
}

int
check_remove_terrain_action(Unit *unit, Unit *unit2, int x, int y, int dir, int t)
{
    int u, u2, m, acp;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!inside_area(x, y))
      return A_ANY_ERROR;
    if (!between(0, dir, NUMDIRS - 1))
      return A_ANY_ERROR;
    if (!is_terrain_type(t))
      return A_ANY_ERROR;
    if (t_is_cell(t))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = ut_acp_to_remove_terrain(u2, t);
    if (acp < 1)
      return A_ANY_CANNOT_DO;
    if (!can_have_enough_acp(unit, acp))
      return A_ANY_CANNOT_DO;
    if (distance(unit->x, unit->y, x, y) > ut_alter_range(u2, t))
      return A_ANY_TOO_FAR;
    if (!has_enough_acp(unit, acp))
      return A_ANY_NO_ACP;
    /* We have to have a minimum level of supply to be able to do the
       action. */
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_remove_terrain(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

//! Should action movies be played?
/*! \note Stupid hack. */

int
should_play_movies(void)
{
    Side *side = NULL;

    for_all_sides(side) {
	if (side == indepside)
	  continue;
	if (!side_has_ai(side))
	  return TRUE;
    }
    return FALSE;
}

/* Execute a given action on a given unit. */

int
execute_action(Unit *unit, Action *action)
{
    const char *argtypestr;
    int u = unit->type, rslt = A_ANY_ERROR, n, i, a[4];
    Unit *unit2, *aunits[4];
    Side *asides[4];
    extern int numsoundplays;

    Dprintf("%s doing %s with %d acp left\n",
	    unit_desig(unit), action_desig(action), unit->act->acp);
    if (!alive(unit) || !unit->act || unit->act->acp < u_acp_min(u))
      return A_ANY_ERROR;
    if (!unit->side->could_act_with[u])
	return A_ANY_CANNOT_DO;
    argtypestr = actiondefns[(int) action->type].argtypes;
    n = strlen(argtypestr);
    for (i = 0; i < n; ++i) {
	switch (argtypestr[i]) {
	  case 'n':
	  case 'u':
	  case 'm':
	  case 't':
	  case 'x':
	  case 'y':
	  case 'z':
	  case 'd':
	    a[i] = action->args[i];
	    break;
	  case 'U':
	    aunits[i] = find_unit(action->args[i]);
	    /* It may be that the argunit cannot be found - perhaps the
	       unit died before the action could be applied to it.  In any
	       case, let the per-action code handle the case. */
	    break;
	  case 'S':
	    asides[i] = side_n(action->args[i]);
	    if (asides[i] == NULL)
	      asides[i] = indepside;
	    break;
	  default:
	    case_panic("action argument type", argtypestr[i]);
	    break;
	}
    }
    if (action->actee == 0) {
	unit2 = unit;
    } else {
	unit2 = find_unit(action->actee);
    }
    if (unit2 == NULL) {
	return A_ANY_ERROR;
    }
    switch (actiondefns[(int) action->type].typecode) {
      case ACTION_NONE:
	rslt = check_none_action(unit, unit2);
	break;
      case ACTION_MOVE:
	rslt = check_move_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_ENTER:
	rslt = check_enter_action(unit, unit2, aunits[0]);
	break;
      case ACTION_ATTACK:
	rslt = check_attack_action(unit, unit2, aunits[0], a[1]);
	break;
      case ACTION_OVERRUN:
	rslt = check_overrun_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_FIRE_AT:
	rslt = check_fire_at_action(unit, unit2, aunits[0], a[1]);
	break;
      case ACTION_FIRE_INTO:
	rslt = check_fire_into_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_CAPTURE:
	rslt = check_capture_action(unit, unit2, aunits[0]);
	break;
      case ACTION_DETONATE:
	rslt = check_detonate_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_PRODUCE:
	rslt = check_produce_action(unit, unit2, a[0], a[1]);
	break;
      case ACTION_EXTRACT:
	rslt = check_extract_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_TRANSFER:
	rslt = check_transfer_action(unit, unit2, a[0], a[1], aunits[2]);
	break;
      case ACTION_DEVELOP:
	rslt = check_develop_action(unit, unit2, a[0]);
	break;
      case ACTION_TOOL_UP:
	rslt = check_toolup_action(unit, unit2, a[0]);
	break;
      case ACTION_CREATE_IN:
	rslt = check_create_in_action(unit, unit2, a[0], aunits[1]);
	break;
      case ACTION_CREATE_AT:
	rslt = check_create_at_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_BUILD:
	rslt = check_build_action(unit, unit2, aunits[0]);
	break;
      case ACTION_REPAIR:
	rslt = check_repair_action(unit, unit2, aunits[0]);
	break;
      case ACTION_DISBAND:
	rslt = check_disband_action(unit, unit2);
	break;
      case ACTION_CHANGE_TYPE:
	rslt = check_change_type_action(unit, unit2, a[0]);
	break;
      case ACTION_CHANGE_SIDE:
	rslt = check_change_side_action(unit, unit2, asides[0]);
	break;
      case ACTION_ALTER_TERRAIN:
	rslt = check_alter_cell_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_ADD_TERRAIN:
	rslt = check_add_terrain_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_REMOVE_TERRAIN:
	rslt = check_remove_terrain_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case NUMACTIONTYPES:
	/* should be error */
	break;
    }
    numsoundplays = 0; /* kind of a hack */
    if (!valid(rslt)) {
	if (unit->plan) {
	    unit->plan->lastaction = *action;
	    unit->plan->lastresult = rslt;
	}
	Dprintf("%s action %s can't be done, result is %s\n",
		unit_desig(unit), action_desig(action), hevtdefns[rslt].name);
	return rslt;
    }
    if (g_action_notices() != lispnil)
      notify_action(unit, action);
    if (g_action_movies() != lispnil)
      play_action_movies(unit, action);
    switch (actiondefns[(int) action->type].typecode) {
      case ACTION_NONE:
	rslt = do_none_action(unit, unit2);
	break;
      case ACTION_MOVE:
	rslt = do_move_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_ENTER:
	rslt = do_enter_action(unit, unit2, aunits[0]);
	break;
      case ACTION_ATTACK:
	rslt = do_attack_action(unit, unit2, aunits[0], a[1]);
	break;
      case ACTION_OVERRUN:
	rslt = do_overrun_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_FIRE_AT:
	rslt = do_fire_at_action(unit, unit2, aunits[0], a[1]);
	break;
      case ACTION_FIRE_INTO:
	rslt = do_fire_into_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_CAPTURE:
	rslt = do_capture_action(unit, unit2, aunits[0]);
	break;
      case ACTION_DETONATE:
	rslt = do_detonate_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_PRODUCE:
	rslt = do_produce_action(unit, unit2, a[0], a[1]);
	break;
      case ACTION_EXTRACT:
	rslt = do_extract_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_TRANSFER:
	rslt = do_transfer_action(unit, unit2, a[0], a[1], aunits[2]);
	break;
      case ACTION_DEVELOP:
	rslt = do_develop_action(unit, unit2, a[0]);
	break;
      case ACTION_TOOL_UP:
	rslt = do_toolup_action(unit, unit2, a[0]);
	break;
      case ACTION_CREATE_IN:
	rslt = do_create_in_action(unit, unit2, a[0], aunits[1]);
	break;
      case ACTION_CREATE_AT:
	rslt = do_create_at_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_BUILD:
	rslt = do_build_action(unit, unit2, aunits[0]);
	break;
      case ACTION_REPAIR:
	rslt = do_repair_action(unit, unit2, aunits[0]);
	break;
      case ACTION_DISBAND:
	rslt = do_disband_action(unit, unit2);
	break;
      case ACTION_CHANGE_TYPE:
	rslt = do_change_type_action(unit, unit2, a[0]);
	break;
      case ACTION_CHANGE_SIDE:
	rslt = do_change_side_action(unit, unit2, asides[0]);
	break;
      case ACTION_ALTER_TERRAIN:
	rslt = do_alter_cell_action(unit, unit2, a[0], a[1], a[2]);
	break;
      case ACTION_ADD_TERRAIN:
	rslt = do_add_terrain_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case ACTION_REMOVE_TERRAIN:
	rslt = do_remove_terrain_action(unit, unit2, a[0], a[1], a[2], a[3]);
	break;
      case NUMACTIONTYPES:
	/* should be error */
	break;
    }
    /* The action may have been blasted if the unit was wrecked, 
    so we use latest_action instead. */
    Dprintf("%s action %s result is %s, %d acp left\n",
	    unit_desig(unit), action_desig(latest_action), hevtdefns[rslt].name,
	    (unit->act ? unit->act->acp : -9999));
    /* Not sure what to do if the action was blasted, so we also test 
    that it still exists. In reality act and plan are blasted together, 
    so this test may be redundant. Note: using latest_action will not 
    work here since it changes all the time. */
    if (unit->plan
        && unit->act) {
	unit->plan->lastaction = *action;
	unit->plan->lastresult = rslt;
    }
    /* Report the result to the unit's owner. */
    if (unit->side && side_has_display(unit->side)) {
	update_action_result_display(unit->side, unit, rslt, TRUE);
    }
    /* Show other sides that some action has occurred. */
    update_side_display_all_sides(unit->side, TRUE);
#if 0 /* doesn't do anything yet */
    /* If of a type that might be spotted if it does anything, check
       each side to see if they notice.  Note that the type is from
       *before* the action, not after (some actions may cause type
       changes). */
    if (u_spot_action(u)
	&& !g_see_all()
	&& !u_see_always(u)
	&& in_area(unit->x, unit->y)) {
	Side *side2;
	for_all_sides(side2) {
	    if (cover(side2, unit->x, unit->y) > 0) {
		/* (should call some sort of "glimpsing" routine) */
	    }
	}
    }
#endif
    /* Check on any scorekeepers that run after each action.  Note
       that one or more sides may be out of the game after this
       returns. */
    if (any_post_action_scores)
      check_post_action_scores();
    /* Return success/failure so caller can use. */
    return rslt;
}

static int pattern_matches_action(Obj *pattern, Unit *unit, Action *action);
static void action_desc_from_list(Side *side, Obj *lis, Unit *unit, Action *action, char *buf);

static void
notify_action(Unit *unit, Action *action)
{
    int found = FALSE;
    const char *atypename;
    char abuf[BUFSIZE];
    Obj *rest, *head, *pat, *msgdesc;

    atypename = actiondefns[(int) action->type].name;
    for_all_list(g_action_notices(), rest) {
	head = car(rest);
	if (!consp(head)) {
	    run_warning("Non-list in action-notices");
	    continue;
	}
	pat = car(head);
	if (symbolp(pat) && strcmp(c_string(pat), atypename) == 0) {
	    found = TRUE;
	    break;
	}
	if (consp(pat)
	    && symbolp(car(pat))
	    && strcmp(c_string(car(pat)), atypename) == 0
	    && pattern_matches_action(cdr(pat), unit, action)) {
	    found = TRUE;
	    break;
	}
    }
    /* If we have a match, do something with it. */
    if (found) {
	msgdesc = cadr(head);
	if (stringp(msgdesc)) {
	    strcpy(abuf, c_string(msgdesc));
	} else {
	    action_desc_from_list(unit->side, msgdesc, unit, action, abuf);
	}
	notify(unit->side, "%s", abuf);
    }
}

static int
pattern_matches_action(Obj *parms, Unit *unit, Action *action)
{
    Obj *head;
    Unit *actee;

    head = car(parms);
    if (!(symbolp(head) && strcmp(c_string(head), u_type_name(unit->type)) == 0)
	 || match_keyword(head, K_USTAR)
	 || (symbolp(head)
	     && boundp(head)
	     && ((symbolp(symbol_value(head))
	          && strcmp(c_string(symbol_value(head)), u_type_name(unit->type)) == 0)
	         || (numberp(symbol_value(head))
	             && c_number(symbol_value(head)) == unit->type))))
      return FALSE;
    parms = cdr(parms);
    head = car(parms);
    if (action->actee == unit->id)
      actee = unit;
    else
      actee = find_unit(action->actee);
    if (!(symbolp(head) && strcmp(c_string(head), u_type_name(actee->type)) == 0)
	 || match_keyword(head, K_USTAR)
	 || (symbolp(head)
	     && boundp(head)
	     && ((symbolp(symbol_value(head))
	          && strcmp(c_string(symbol_value(head)), u_type_name(actee->type)) == 0)
	         || (numberp(symbol_value(head))
	             && c_number(symbol_value(head)) == actee->type))))
      return FALSE;
    /* (should test result also) */
    return TRUE;
}

static void
action_desc_from_list(Side *side, Obj *lis, Unit *unit, Action *action,
		      char *buf)
{
    int n;
    const char *symname;
    Obj *rest, *item;
    Unit *actee;

    buf[0] = '\0';
    for_all_list(lis, rest) {
	item = car(rest);
	if (stringp(item)) {
	    strcat(buf, c_string(item));
	} else if (symbolp(item)) {
	    symname = c_string(item);
	    if (strcmp(symname, "unit") == 0) {
		sprintf(buf+strlen(buf), "%s", unit_handle(side, unit));
	    } else if (strcmp(symname, "actee") == 0) {
		if (action->actee == unit->id)
		  actee = unit;
		else
		  actee = find_unit(action->actee);
		sprintf(buf+strlen(buf), "%s", unit_handle(side, actee));
	    } else {
		sprintf(buf+strlen(buf), " ??%s?? ", symname);
	    }
	} else if (numberp(item)) {
	    n = c_number(item);
	    if (0 /* special processing */) {
	    } else {
		sprintf(buf+strlen(buf), "%d", n);
	    }
	} else {
	    strcat(buf, " ????? ");
	}
    }
}

static void
play_action_movies(Unit *unit, Action *action)
{
    int found = FALSE;
    const char *soundname;
    Obj *rest, *head, *parms, *msgdesc;

    if (!should_play_movies())
      return;
    for_all_list(g_action_movies(), rest) {
	head = car(rest);
	if (consp(head)
	    && symbolp(car(head))
	    && strcmp(c_string(car(head)),
		      actiondefns[(int) action->type].name) == 0) {
	    found = TRUE;
	    break;
	}
	if (consp(head)
	    && consp(car(head))
	    && symbolp(car(car(head)))
	    && strcmp(c_string(car(car(head))),
		      actiondefns[(int) action->type].name) == 0) {
	    parms = cdr(car(head));
	    if (parms == lispnil) {
		found = TRUE;
		break;
	    }
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
	    /* (should be able to match on particular action parms also) */
	}
    }
    /* If we have a match, do something with it. */
    if (found) {
	msgdesc = cadr(head);
	if (stringp(msgdesc)) {
	    notify(unit->side, "%s", c_string(msgdesc));
	} else if (consp(msgdesc)
		   && symbolp(car(msgdesc))
		   && strcmp(c_string(car(msgdesc)), "sound") == 0
		   && stringp(cadr(msgdesc))) {
	    soundname = c_string(cadr(msgdesc));
	    /* (should not be passing ptrs to schedule_movie) */
	    schedule_movie(unit->side, "sound", soundname);
	    play_movies(add_side_to_set(unit->side, NOSIDES));
	} else {
	}
    }
}

/* Basic check that unit has sufficient acp to do an action. */

int
can_have_enough_acp(Unit *unit, int acp)
{
    if (!type_can_have_enough_acp(unit->type, acp)) {
	if (acp < new_acp_for_turn(unit))
	  return TRUE;
    }
    else
      return TRUE;
    return FALSE;
}

int
type_can_have_enough_acp(int u, int acp)
{
    int maxacp, minacp;

    /* Acp-independent units always have enough acp. */
    if (u_acp_independent(u))
    	return TRUE;
    /* Else determine the basic ACP for the unit. */
    maxacp = u_acp(u);
    if (u_acp_turn_max(u) >= 0)
      maxacp = min(maxacp, u_acp_turn_max(u));
    maxacp = (u_acp_max(u) < 0 ? maxacp : u_acp_max(u));
    minacp = u_acp_min(u);
    return (maxacp + u_free_acp(u) - acp >= minacp);
}

int
has_enough_acp(Unit *unit, int acp)
{
    int u = unit->type;

    /* ACP-indep units always have enough ACP. */
    if (acp_indep(unit))
      return TRUE;
    /* If the ACP required is 0, then allow the action. */
    if (!acp)
      return TRUE;
    /* If an unit cannot act, then it cannot have enough ACP. */
    if (!unit->act)
      return FALSE;
    return ((unit->act->acp + u_free_acp(u) - acp) >= u_acp_min(unit->type));
}

/* What is the maximum ACP that can be gained from being on any terrain
   at night? */
int
type_max_night_acp_from_any_terrains(int u)
{
    int t = NONTTYPE, u3 = NONUTYPE;
    int acp = 0, maxacp = 0;

    if (!cache__type_max_night_acp_from_any_terrains) {
	cache__type_max_night_acp_from_any_terrains = 
	  (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u3) {
	    for_all_terrain_types(t) {
		if (t_is_cell(t) /* || connector || coating */) {
		    acp = ut_night_adds_acp(u3, t);
		    maxacp = max(acp, maxacp);
		}
	    }
	    cache__type_max_night_acp_from_any_terrains[u3] = maxacp;
	}
    }
    return cache__type_max_night_acp_from_any_terrains[u];
}

/* What is the maximum ACP that can be gained from having any occupant? */
/* (To do this calculation truthfully, we need to use a knapsack algorithm,
    because it may be that some combination of occ types yields the best
    results. Nonetheless, the function, in its current form, at least tells
    us whether or not any occ type will boost the ACP.) */
int
type_max_acp_from_any_occs(int u)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;
    int acp = 0, maxacp = 0;

    if (!cache__type_max_acp_from_any_occs) {
	cache__type_max_acp_from_any_occs = (int *)xmalloc(numutypes * 
							   sizeof(int));
	for_all_unit_types(u3) {
	    for_all_unit_types(u2) {
		acp = uu_occ_adds_acp(u3, u2);
		if (acp) {
		    if (type_can_occupy_empty_type(u2, u3))
			maxacp = max(acp, maxacp);
		}
	    }
	    cache__type_max_acp_from_any_occs[u3] = maxacp;
	}
    }
    return cache__type_max_acp_from_any_occs[u];
}

/* What is the maximum ACP that an unit type can have? */
int
type_max_acp(int u)
{
    int u3 = NONUTYPE;
    int acp = 0;

    if (!cache__type_max_acp) {
	cache__type_max_acp = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u3) {
	    if (u_acp_max(u3) >= 0) {
		cache__type_max_acp[u3] = u_acp_max(u3);
		continue;
	    }
	    acp = u_acp(u3);
	    acp += type_max_night_acp_from_any_terrains(u3);
	    acp += type_max_acp_from_any_occs(u3);
	    /* (Should consider multiplicative effects.) */
	    cache__type_max_acp[u3] = acp;
	}
    }
    return cache__type_max_acp[u];
}

//! Make the consumed acp disappear, but not go below the minimum possible.
/*! \note u_free_acp does not need to be explicitly considered here. */

void
use_up_acp(Unit *unit, int acp)
{
    int oldacp, newacp, acpmin;

    /* This can sometimes be called on dead or non-acting units,
       so check first. */
    if (alive(unit) && !u_acp_independent(unit->type) && unit->act && acp > 0) {
    	oldacp = unit->act->acp;
	newacp = oldacp - acp;
	acpmin = u_acp_min(unit->type);
	unit->act->acp = max(newacp, acpmin);
	/* Maybe modify the unit's display. */
	if (oldacp != unit->act->acp) {
	    update_unit_acp_display(unit->side, unit, TRUE);
	}
    }
}

/* The following is generic code. */

int
construction_possible(int u2)
{
    int u;

    for_all_unit_types(u) {
	if (could_create(u, u2)
	    && uu_tp_max(u, u2) >= uu_tp_to_build(u, u2))
	  return TRUE;
    }
    return FALSE;
}

int
any_construction_possible(void)
{
    int u, u2;
    static int any_construction = -1;

    if (any_construction < 0) {
	any_construction = FALSE;
	for_all_unit_types(u) {
	    for_all_unit_types(u2) {
		if (could_create(u, u2)) {
		    any_construction = TRUE;
		    return any_construction;
		}
	    }
	}
    }
    return any_construction;
}

/* Test if the given mtype ever is stored in individual units. */

int
storage_possible(int m)
{
	int	u;
	Side	*side;
	
	/* Test if any utype can store the mtype AND does not
	always give it to a treasury. */
	for_all_unit_types(u) {
		/* Skip utypes that cannot store m. */
		if (!um_storage_x(u, m))
		    continue;
		/* This utype will give m to a treasury if it exists,
		but might not give ALL in backdrop model 1. */
		if (um_gives_to_treasury(u, m)
		    && (g_backdrop_model() != 1)) {
			/* Test if all sides that can have this unit also
			have a treasury that will recieve m. */
			for_all_sides(side) {
				if (side_has_treasury(side, m))
				    continue;
				if (!type_allowed_on_side(u, side))
				    continue;
				/* We found a side that can have this utype
				and lacks a treasury for m. */
				return TRUE;
			}
		/* We found a utype that can store m and does not give it
		to a treasury even if it exists. */
		} else return TRUE;
	}
	return FALSE;
}

/* Test if any mtype is ever stored in individual units. */

int
any_storage_possible(void)
{
	int m;
	
	for_all_material_types(m) {
		if (storage_possible(m)) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Compose a legible description of a given action. */

const char *
action_desig(Action *act)
{
    int i, slen;
    char ch, *str;

    if (act == NULL)
      return "?null action?";
    if (act->type == ACTION_NONE)
      return "[]";
    if (actiondesigbuf == NULL)
      actiondesigbuf = (char *)xmalloc(BUFSIZE);
    str = actiondesigbuf;
    sprintf(str, "[%s", actiondefns[act->type].name);
    slen = strlen(actiondefns[act->type].argtypes);
    for (i = 0; i < slen; ++i) {
	ch = (actiondefns[act->type].argtypes)[i];
	switch (ch) {
	  case 'U':
	    tprintf(str, " \"%s\"",
		    unit_desig(find_unit(act->args[i])));
	    break;
	  default:
	    tprintf(str, " %d", act->args[i]);
	}
    }
    if (act->actee != 0) {
	tprintf(str, " (#%d)", act->actee);
    }
    strcat(str, "]");
    return actiondesigbuf;
}

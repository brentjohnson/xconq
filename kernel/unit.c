/* Units in Xconq.
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"
#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"
#include "aioprt.h"

/* This is not a limit, just sets initial allocation of unit objects
   and how many additional to get if more are needed.  This can be
   tweaked for more frequent but smaller allocation, or less frequent
   but larger and possibly space-wasting allocation. */

#ifndef INITMAXUNITS
#define INITMAXUNITS 200
#endif

extern void free_used_cells(Unit *unit); 	/* Used in move.c */
extern void set_unit_image(Unit *unit);	/* From ui.c */

extern int *numuimages;

static void allocate_unit_block(void);
static int compare_units(Unit *unit1, Unit *unit2);
static int compare_units_by_keys(const void *e1, const void *e2);
static void leave_cell_aux(Unit *unit, Unit *oldtransport);
static void change_cell_aux(Unit *unit, int x0, int y0, int x, int y);
static void kill_unit_aux(Unit *unit, int reason);
static void set_unit_position(Unit *unit, int x, int y, int z);
static void enter_cell_aux(Unit *unit, int x, int y);
static void eject_excess_occupants(Unit *unit);
static void eject_occupant(Unit *unit, Unit *occ);
static void insert_unit(Unit *unithead, Unit *unit);
static void delete_unit(Unit *unit);
static void check_unit(Unit *unit);
static int disband_unit_directly(Side *side, Unit *unit);
static void add_unit_to_ustack(Unit *unit, int x, int y);
static void add_unit_to_ustack(Unit *unit, Unit *transport);
static void remove_unit_from_ustack(Unit *unit);
static void glimpse_adjacent_terrain(Unit *unit);
static int budget_space_for_changed_type(Unit *unit, int newtype);
static int try_changing_side_of_changed_unit(Unit *unit, Side *newside, 
					     int newtype, int reason);
static int desperately_try_changing_side_of_changed_unit(Unit *unit, 
							 Side *newside, 
							 int newtype, 
							 int reason);
static void resurrect_self_unit(Unit *unit, Side *oldside, Side *newside);

/* The list of available units. */

Unit *freeunits;

/* The global linked list of all units. */

Unit *unitlist;

/* A scratch global. */

Unit *tmpunit;

/* Buffers for descriptions of units. */

/* We use several and rotate among them; this is so multiple calls in
   a single printf will work as expected.  Not elegant, but simple and
   sufficient. */

#define NUMSHORTBUFS 3

static int curshortbuf;

static char *shortbufs[NUMSHORTBUFS] = { NULL, NULL, NULL };

static char *utypenamen;

char **shortestnames = NULL;

int longest_shortest;

char **shortestgnames = NULL;

char *actorstatebuf = NULL;

/* Total number of units in existence. */

int numunits;

/* Total number of units that are alive. */

int numliveunits;

/* Total number of live units, by type. */

int *numlivebytype;

/* The next number to use for a unit id. */

int nextid = 1;

/* Advanced unit support. */

short default_size = 1;		/* Default abstract size of unit */
short default_usedcells = 0;	/* Number of cells being used by the unit */
short default_maxcells = 1;	/* Default max number of cells that unit can use */
short default_curadvance = -1;	/* Default advance being researched by new unit */
long default_population = 1;	/* Default size of unit's population */

extern void free_used_cells(Unit *unit);

/* Can the unit be an actor? */

int
can_be_actor(Unit *unit)
{
    int factor = FALSE;
    Unit *occ = NULL;
    int t = NONTTYPE;

    assert_error(unit, "Attempted to access a null unit.");
    if (unit->cp < 0)
      return FALSE;
    /* Could the unit get acp every turn? */
    factor = (u_acp(unit->type) > 0);
    /* Is the unit ACP-independent? ACP-indep units still need plan. */
    if (!factor)
      factor = acp_indep(unit);
    /* Does an occupant give the unit ACP? */
    if (!factor) {
	for_all_occupants(unit, occ) {
	    if (0 < uu_occ_adds_acp(unit->type, occ->type)) {
		factor = TRUE;
		break;
	    }
	}
    }
    /* Does the unit's terrain at night give the unit ACP? */
    if (!factor && in_play(unit) && night_at(unit->x, unit->y)) {
	t = terrain_at(unit->x, unit->y);
	factor = (0 < ut_night_adds_acp(unit->type, t));
	/* (Should also consider connectors and coatings.) */
    }
    return factor;
}

/* Grab a block of unit objects to work with. */

static void
allocate_unit_block(void)
{
    int i;
    Unit *unitblock = (Unit *) xmalloc(INITMAXUNITS * sizeof(Unit));

    for (i = 0; i < INITMAXUNITS; ++i) {
        unitblock[i].id = -1;
        unitblock[i].next = &unitblock[i+1];
    }
    unitblock[INITMAXUNITS-1].next = NULL;
    freeunits = unitblock;
    Dprintf("Allocated space for %d units.\n", INITMAXUNITS);
}

/* Init gets a first block of units, and sets up the "independent side" list,
   since newly created units will appear on it. */

void
init_units(void)
{
    unitlist = NULL;
    allocate_unit_block();
    init_side_unithead(indepside);
}

/* The primitive unit creator, called by regular creator and also used to
   make the dummy units that sit at the heads of the per-side unit lists. */

Unit *
create_bare_unit(int type)
{
    Unit *newunit;

    /* If our free list is empty, go and get some more units. */
    if (freeunits == NULL) {
	allocate_unit_block();
    }
    /* Take the first unit off the free list. */
    newunit = freeunits;
    freeunits = freeunits->next;
    /* Give it a valid type... */
    newunit->type = type;
    /* ...but an invalid id. */
    newunit->id = -1;
    return newunit;
}

/* The regular unit creation routine.  All the slots should have something
   reasonable in them, since individual units objects see a lot of reuse. */

Unit *
create_unit(int type, int makebrains)
{
    int i, m;
    Unit *newunit;

    if (numlivebytype == NULL)
      numlivebytype = (int *) xmalloc(numutypes * sizeof(int));
    /* Test whether we've hit any designer-specified limits. */
    if ((u_type_in_game_max(type) >= 0
	 && numlivebytype[type] >= u_type_in_game_max(type))
	|| (g_units_in_game_max() >= 0
	    && numliveunits >= g_units_in_game_max())) {
	return NULL;
    }
    /* Allocate the unit object.  Xconq will fail instead of returning null. */
    newunit = create_bare_unit(type);
    /* Set the master uview to NULL; it will be allocated and updated as 
       needed. */
    newunit->uview = NULL;
    /* Init all the slots to distinguishable values.  The unit is not
       necessarily newly allocated, so we have to hit all of its slots. */
    /* Note that we don't check for overflow of unit ids, since it's
       highly unlikely that a single game would ever create 2 billion
       units. */
    newunit->id = nextid++;
    newunit->name = NULL;
    /* Number == 0 means unit is unnumbered. */
    newunit->number = 0;
    /* If during launch, images may not be loaded yet. */
    if (numuimages) {
    	set_unit_image(newunit);
    }
    /* Outside the world. */
    newunit->x = newunit->y = -1;
    /* At ground level. */
    newunit->z = 0;
    /* Units default to being independent. */
    newunit->side = indepside;
    newunit->origside = indepside;
    /* Create at max hp, let others reduce if necessary. */
    newunit->hp = newunit->hp2 = u_hp(type);
    /* Create fully functional, let other routines set incompleteness. */
    newunit->cp = u_cp(type);
    /* Zero the combat experience. */
    newunit->cxp = 0;
    /* Zero the morale. */
    newunit->morale = 0;
    /* Not in a transport. */
    newunit->transport = NULL;
    /* Note that the space never needs to be freed. */
    if (newunit->supply == NULL && nummtypes > 0) {
        newunit->supply = (long *) xmalloc(nummtypes * sizeof(long));
    }
    /* Always zero out all the supply values. */
    for_all_material_types(m)
      newunit->supply[m] = 0;
    /* Allocate cache for units last production. */
    if (nummtypes > 0)
        newunit->production = (short *) xmalloc(nummtypes * sizeof(short));
    /* Always zero out all the production values. */
    for_all_material_types(m)
       newunit->production[m] = 0;
    /* Will allocate tooling state when actually needed. */
    newunit->tooling = NULL;
    /* Will allocate opinions when actually needed. */
    newunit->opinions = NULL;
    if (makebrains) {
	init_unit_actorstate(newunit, TRUE);
	init_unit_plan(newunit);
    } else {
	newunit->act = NULL;
	newunit->plan = NULL;
    }
    if (newunit->extras != NULL)
      init_unit_extras(newunit);
    newunit->occupant = NULL;
    newunit->nexthere = NULL;
    newunit->creation_id = 0;
    /* Glue this unit into the list of independent units. */
    newunit->next = newunit;
    newunit->prev = newunit;
    insert_unit(indepside->unithead, newunit);
    newunit->unext = unitlist;
    unitlist = newunit;
    /* Init various other slots. */
    newunit->prevx = newunit->prevy = -1;
    newunit->transport_id = lispnil;
    newunit->aihook = NULL;
    /* Advanced unit support. */
    newunit->size = default_size;
    newunit->reach = u_reach(type);
    newunit->usedcells = default_usedcells;
    newunit->maxcells = default_maxcells;
    newunit->population = default_population;
    newunit->curadvance = default_curadvance;
    newunit->cp_stash = 0;
    newunit->researchdone = FALSE;
    newunit->busy = FALSE;
    /* Add to the global unit counts. */
    ++numunits;
    ++numliveunits;
    ++(numlivebytype[type]);
    return newunit;
}

void
init_unit_tooling(Unit *unit)
{
    unit->tooling = (short *) xmalloc(numutypes * sizeof(short));
}

/* Set or resize the array of a unit's opinions about each side in a game. */

void
init_unit_opinions(Unit *unit, int nsides)
{
    int i;
    short *temp;

    temp = NULL;
    if (u_opinion_min(unit->type) != u_opinion_max(unit->type)) {
	if (unit->opinions != NULL && nsides > numsides) {
	    temp = unit->opinions;
	    unit->opinions = NULL;
	}
	if (unit->opinions == NULL)
	  unit->opinions = (short *) xmalloc(nsides * sizeof(short));
	/* Opinions are now all neutral. */
	if (temp != NULL) {
	    /* Copy over old opinions. */
	    for (i = 0; i < numsides; ++i)
	      unit->opinions[i] = temp[i];
	    free(temp);
	}
    } else {
	if (unit->opinions != NULL)
	  free(unit->opinions);
	unit->opinions = NULL;
    }
}

/* Alter the actorstate object to be correct for this unit. Allow for 
   artificially injecting ACP, since this is useful in cases when an unit 
   is "enabled" by an occupant, but the occupant has not yet been read in 
   during game restoration. */

void
init_unit_actorstate(Unit *unit, int flagacp, int acp)
{
    if (can_be_actor(unit) || (0 < acp)) {
	/* Might already have an actorstate, don't realloc if so;
	   but do clear an existing actorstate, since might be reusing. */
	if (unit->act == NULL)
	  unit->act = (ActorState *) xmalloc(sizeof(ActorState));
	else
	  memset((char *) unit->act, 0, sizeof(ActorState));
	/* Indicate that the action points have not been set. */
	if (flagacp)
	  unit->act->acp = unit->act->initacp = u_acp_min(unit->type) - 1;
	/* Flag the action as undefined. */
	unit->act->nextaction.type = ACTION_NONE;
    } else {
	if (unit->act != NULL)
	  free(unit->act);
	unit->act = NULL;
    }
}

/* Allocate and fill in the unit extras.  This is needed whenever any
   one of the extra properties is going to get a non-default value.
   (Normally, the accessors for this structure return a default value
   if one is not present in a unit.) */

void
init_unit_extras(Unit *unit)
{
    if (unit->extras == NULL)
      unit->extras = (UnitExtras *) xmalloc(sizeof(UnitExtras));
    /* Each slot must get the same value as the accessor would return
       if the structure had not been allocated. */
    unit->extras->point_value = -1;
    unit->extras->appear = -1;
    unit->extras->appear_var_x = -1;
    unit->extras->appear_var_y = -1;
    unit->extras->disappear = -1;
    unit->extras->priority = -1;
    unit->extras->sym = lispnil;
    unit->extras->sides = lispnil;
}

/* Changing a unit's type has many effects. */

void
change_unit_type(Unit *unit, int newtype, int reason, Side *newside)
{
    int oldtype = unit->type, oldhp = unit->hp;
    PastUnit *pastunit;
    Side *oldside;
    int oldacp = 0;
    int oldacpuser = TRUE;
    int fnewside = FALSE;
    Unit *transport = NULL;
    int m = NONMTYPE;

    assert_error(unit, "Tried to change the type of a null unit.");
    /* Don't do anything if we're "changing" to the same type. */
    if (oldtype == newtype)
      return;
    /* Shorthand for unit->transport. */
    transport = unit->transport;
    /* Record the old ACP. */
    if (unit->act) {
	oldacp = unit->act->acp;
    }
    /* Flag if the unit did not use or did not have ACP. */
    else {
	oldacpuser = FALSE;
    }
    /* Make sure that there is enough space for new type. */
    if (!budget_space_for_changed_type(unit, newtype))
      return;
    /* Update the transport as the budgeting process may have pulled the 
       unit out of the transport. */
    transport = unit->transport;
    /* Consume materials as necessary. */
    if (in_play(unit)) {
	consume_materials(unit, unit, um_consumption_per_change_type);
    }
    /* Remember unit and its side. */
    oldside = unit->side;
    pastunit = change_unit_to_past_unit(unit);
    if (reason >= 0)
      record_event((HistEventType)reason, ALLSIDES, pastunit->id, unit->id, 
		   newtype);
    /* Decrement viewing coverage of our old type. */
    cover_area(unit->side, unit, transport, unit->x, unit->y, -1, -1);
    all_see_leave(unit, unit->x, unit->y, (transport == NULL));
    /* Do the actual change. */
    unit->type = newtype;
    /* Make sure the new unit is complete. */
    unit->cp = u_cp(newtype);
    /* Set the new hp to the same ratio of max as the unit had before.
       Caller can tweak to something else if necessary. */
    unit->hp = (oldhp * u_hp_max(newtype)) / u_hp_max(oldtype);
    /* Need to guarantee a positive value though. */
    if (unit->hp < 1)
      unit->hp = 1;
    unit->hp2 = unit->hp;
    /* Alter the unit's action vector as needed. */
    /* Set the ACP to the same ratio of max as the unit had before. */
    /* If old unit was ACP-indep or actionless, then we give the new unit 
       a free booster in the world of ACP, if it is set up to use ACP. */
    if (!oldacpuser || (u_acp(newtype) <= 0)) {
        init_unit_actorstate(unit, FALSE);
	if (unit->act)
	  unit->act->initacp = unit->act->acp = total_acp_for_turn(unit);
    }
    else {
	if (oldacp > 0) {
	    unit->act->acp = (oldacp * type_acp_max(newtype)) / 
			     type_acp_max(oldtype);
	    unit->act->initacp = total_acp_for_turn(unit);
	}
	else {
	    unit->act->initacp = unit->act->acp = new_acp_for_turn(unit);
	}
    }
    /* Knock out the old nextaction. Might not be compatible. */
    if (unit->act)
      unit->act->nextaction.type = ACTION_NONE;
    /* Invalidate the side's point value cache. */
    if (unit->side)
      unit->side->point_value_valid = FALSE;
    /* Try changing to a new side if necessary. */
    if (!type_allowed_on_side(newtype, unit->side))
      fnewside = try_changing_side_of_changed_unit(unit, newside, newtype, 
						   reason);
    /* (should modify per-side counts) */
    /* Unit will always need a new number. */
    assign_unit_number(unit);
    /* Unit may also need a name. */
    /* (Old names should also be gotten rid of, if the new unit type is not 
	supposed to have names assigned to its units.) */
    make_up_unit_name(unit);
    /* If old type was self and new type can not be self, 
       then a new avatar must be found or else the side loses. */
    if (unit->side->self_unit == unit) {
	if (!u_can_be_self(newtype))
	  resurrect_self_unit(unit, unit->side, newside);
    }
    /* Update the cancarry vector if this is a transport. */
    if (u_is_transport(unit->type)) {
    	update_cancarry_vector(unit->side);
    }
    init_unit_opinions(unit, numsides);
    /* Redo the supply numbers. */
    /* init_supply(unit); */
    /* (TODO: Dump excess in the terrain or stack neighbors, or ....) */
    for_all_material_types(m) {
	unit->supply[m] = min(unit->supply[m], um_storage_x(newtype, m));
    }
    init_unit_plan(unit);
    unit->aihook = NULL;
    /* Advanced unit support. */
    /* (should share with unit creation) */
    if (u_advanced(newtype)) {
	/* If weren't previously and advanced unit. */
	if (!u_advanced(oldtype)) {
	    unit->size = default_size;
	    unit->reach = u_reach(newtype);
	    unit->usedcells = default_usedcells;
	    unit->maxcells = default_maxcells;
	    unit->population = default_population;
	    unit->curadvance = default_curadvance;
	    unit->cp_stash = 0;
	}
	/* Else we were already and advanced unit. */
	else {
	    /* Extend reach if old reach is too little. */
	    unit->reach = max(unit->reach, u_reach(newtype));
	    /* Contract reach if old reach is too large. */
	    unit->reach = min(unit->reach, u_reach(newtype));
	}
    }
    /* If the unit needs to change side but could not earlier, start trying 
       some desperate measures. */
    if (!fnewside && !type_allowed_on_side(newtype, unit->side)) {
	if (!desperately_try_changing_side_of_changed_unit(unit, newside, 
							   newtype, reason))
	  return;
    }
    /* Set the unit image by first clearing the old image name. 
       'set_unit_image' will set a new image name and image for us. */
    unit->image_name = "";
    set_unit_image(unit);
    /* Increment viewing coverage. */
    cover_area(unit->side, unit, transport, -1, -1, unit->x, unit->y);
    /* If vision range is 0, allow glimpses of adjacent cell terrain.
       This applies to terrain only, adjacent units cannot be seen. */
    if (u_vision_range(unit->type) == 0)
      glimpse_adjacent_terrain(unit);
    all_see_occupy(unit, unit->x, unit->y, (transport == NULL));
    count_loss(oldside, oldtype, (reason == H_UNIT_WRECKED ? combat_loss 
							   : other_loss));
    count_gain(unit->side, newtype, other_gain);
    update_side_display(unit->side, unit->side, TRUE);
    /* Update global counts. */
    --(numlivebytype[oldtype]);
    ++(numlivebytype[newtype]);
}

/* Try to change the side of an unit that changed type, if necesary. */

static int
try_changing_side_of_changed_unit(Unit *unit, Side *newside, int newtype, 
				  int reason)
{
    int fnewside = FALSE;
    Unit *transport = NULL;

    assert_error(unit, "Tried changing side of null unit.");
    transport = unit->transport;
    /* We might have to change sides. */
    if (!type_allowed_on_side(newtype, unit->side)) {
	/* Unit maybe joins another side. */
	if (!fnewside && newside && type_allowed_on_side(newtype, newside)) {
	    if (!transport 
		|| (transport && transport->side
		    && (trusted_side(transport->side, newside)
			|| ((indepside == transport->side)
			    && uu_can_enter_indep(newtype, 
						  transport->type))))) {
		change_unit_side(unit, newside, reason, NULL);
		fnewside = TRUE;
	    }
	}
	/* Unit maybe becomes independent. */
    	if (!fnewside && type_allowed_on_side(newtype, indepside)) {
	    if (!transport 
		|| (transport && transport->side
		    && (trusted_side(transport->side, indepside)
			|| (indepside == transport->side)))) {
		change_unit_side(unit, indepside, reason, NULL);
		fnewside = TRUE;
	    }
	}
	/* (If we fail here, we may get more desparate later.) */
    }
    return fnewside;
}

/* Desperately try to change the side of an unit that changed type, 
   if necesary. */

static int
desperately_try_changing_side_of_changed_unit(Unit *unit, Side *newside, 
					      int newtype, int reason)
{
    int fnewside = FALSE, fkillunit = FALSE;
    Unit *transport = NULL;

    assert_error(unit, "Tried changing side of null unit.");
    transport = unit->transport;
    if (!fnewside && !type_allowed_on_side(newtype, unit->side)) {
    /* (I have quite a few reservations about the following commented out 
	code. For one, I do not think that we should be initiating a 
	capture action from within a low-level function such as 
	'change_unit_type'. For another, we cannot easily forsee all the 
	circumstances under which 'change_unit_type' is invoked, and 
	therefore it is hard to judge whether automatic escape or capture 
	are always the correct things to try. And, this adds a lot of 
	complexity to the game, which if mistakenly abused may appear to 
	be a bug to a game designer. And finally, with the ability to 
	change unit type on capture in place, have captures invoked from 
	'change_unit_type' may lead to infinite recursion in some cases.) */
#if (0)
	if (transport) {
	    /* (TODO: Implement occupant escape chance for units which 
		changed type.) */
	    if (valid(check_capture_action(unit, unit, transport))) {
		if (A_CAPTURE_FAILED == 
		    do_capture_action(unit, unit, transport))
	          fkillunit = TRUE;
	    }
	}
	else
	  fkillunit = TRUE;
#else
	fkillunit = TRUE;
#endif
	/* Unitum delenda est. */
	if (fkillunit) {
	    /* Give any occupants a chance to escape. */
	    rescue_occupants(unit);
	    /* Say goodbye to the unit. */
	    kill_unit(unit, H_UNIT_VANISHED);
	    Dprintf(
"%s killed by type change since new type %s could not belong to old side or any possible new side",
		    unit_desig(unit), u_type_name(newtype));
	}
	else
	  fnewside = TRUE;
    }
    return fnewside;
}

/* Try to budget some space for an unit which about to change to a new type. */

static int
budget_space_for_changed_type(Unit *unit, int newtype)
{
    int fkillunit = FALSE;
    Unit *transport = NULL;
    int x = -1, y = -1;

    assert_error(unit, "Attempted to change type of a null unit.");
    transport = unit->transport;
    x = unit->x; y = unit->y;
    /* Kill the unit if the new type cannot sit in the old type's cell or 
       old type's transport. Note that we do not permit the unit to change 
       type one more time by being wrecked at this point. */
    if (transport) {
	if (!type_can_occupy_without(newtype, transport, unit)) {
	    /* Can the type occupy and survive in the same cell as its 
	       transport? */
	    if (!type_can_occupy_cell(newtype, unit->x, unit->y)
		|| !type_survives_in_cell(newtype, unit->x, unit->y))
	      fkillunit = TRUE;
	    // If so, then place the unit in the cell. 
	    /* (We don't worry about ACP or MP at this point. Perhaps this 
		is a bad assumption, but it makes life easier....) */
	    else {
		leave_cell(unit); // This also frees it from the transport.
		enter_cell(unit, x, y);
	    }
	}
    }
    else {
	if (!type_can_occupy_cell_without(newtype, unit->x, unit->y, unit)
	    || !type_survives_in_cell(newtype, unit->x, unit->y))
	  fkillunit = TRUE;
    }
    if (fkillunit) {
    	/* Give any occupants a chance to escape. */
	rescue_occupants(unit);
	/* Say goodbye to the unit. */
    	kill_unit(unit, H_UNIT_VANISHED);
    	Dprintf(
"%s killed by type change since new type %s could not sit in (%d, %d)",
		unit_desig(unit), u_type_name(newtype), unit->x, unit->y);
    }
    return !fkillunit;
}

/* Test if any occupant of a given unit belongs to the given side. */
/* {Arbiter Function} */

int
side_owns_occupant_of_unit(Side *side, Unit *unit)
{
    Unit *occ = NULL;

    assert_error(side, "Attempted to access a NULL side");
    if (unit->occupant == NULL)
      return FALSE;
    for_all_occupants(unit, occ) {
	if (occ->side == side)
	  return TRUE;
	if (occ->occupant) {
	    if (side_owns_occupant_of_unit(side, occ))
	      return TRUE;
	}
    }
    return FALSE;
}

/* Test if the side has a seeing occupant inside a transport. */
/* {Arbiter Function} */

int
side_owns_viewer_in_unit(Side *side, Unit *unit)
{
    Unit *occ = NULL;

    assert_error(side, "Attempted to access a NULL side");
    if (unit->occupant == NULL)
      return FALSE;
    for_all_occupants(unit, occ) {
	/* (TODO: Account for transport adding vision range to occ.) */
	if ((occ->side == side) && (0 <= u_vision_range(unit->type)))
	  return TRUE;
	if (occ->occupant) {
	    if (side_owns_viewer_in_unit(side, occ))
	      return TRUE;
	}
    }
    return FALSE;
}

/* Return the chance of one utype seeing another, given distance. */

int
see_chance(int u, int u2, int dist)
{
    assert_error(is_unit_type(u), "Invalid unit type encountered");
    assert_error(is_unit_type(u2), "Invalid unit type encountered");
    if (u_vision_range(u) < dist)
      return 0;
    if (u_see_always(u2))
      return 100;
    switch (dist) {
      case 0:
	return uu_see_at(u, u2);
      case 1:
	return uu_see_adj(u, u2);
      default:
	return uu_see(u, u2);
    }
    return 0;
}

/* Return the chance of one unit seeing another. */
/* {Arbiter Function} */

int
see_chance(Unit *seer, Unit *tosee)
{
    int dist = -1, chance = 0;
    int u = NONUTYPE, u2 = NONUTYPE;
    int t = NONTTYPE, ta = NONTTYPE;
    int tv = UNSEEN;

    assert_error(in_play(seer), "Attempted to access an out-of-play unit.");
    assert_error(in_play(tosee), "Attempted to access an out-of-play unit.");
    tv = terrain_view(seer->side, tosee->x, tosee->y);
    if (UNSEEN == tv)
      return 0;
    t = vterrain(tv);
    u = seer->type;
    u2 = tosee->type;
    /* If visibility is 0 in any terrain, then the chance of seeing is 0. */
    /* (TODO: Implement 'ut_adds_visibility' and 'ut_multiplies_visibility'.) */
    if (0 >= ut_visibility(u2, t))
      return 0;
    if (any_aux_terrain_defined()) {
	for_all_aux_terrain_types(ta) {
	    if (aux_terrain_defined(ta) 
		&& aux_terrain_at(tosee->x, tosee->y, ta) 
		&& (0 >= ut_visibility(u2, ta)))
	      return 0;
	}
    }
    /* (TODO: Implement 'uu_transport_adds_vision', 
	'uu_transport_multiples_vision', 'uu_occ_adds_vision', and 
	'uu_occ_multiples_vision'.) */
    if (seer->transport && (0 == uu_occ_vision(u, seer->transport->type)))
      return 0;
    dist = distance(seer->x, seer->y, tosee->x, tosee->y);
    /* Basic see-chance between 2 units, accounting for distance. */
    chance = see_chance(u, u2, dist);
    /* Terrain modifications to see-chance. */
    chance = (chance * ut_visibility(u2, t)) / 100;
    if (any_aux_terrain_defined()) {
	for_all_aux_terrain_types(ta) {
	    if (aux_terrain_defined(ta) 
		&& aux_terrain_at(tosee->x, tosee->y, ta)) 
	      chance = (chance * ut_visibility(u2, ta)) / 100;
	}
    }
    /* Transport modifications to see-chance. */
    /* (TODO: Implement 'uu_transport_adds_visibility', 
	'uu_transport_multiplies_visibility', 'uu_occ_adds_visibility', 
	and 'uu_occ_multiplies_visibility'.) */
    if (seer->transport)
      chance = (chance * uu_occ_vision(u, seer->transport->type)) / 100;
    /* Lighting modification to see-chance. */
    /* (TODO: Should test for twilight effects.) */
    /* (TODO: Implement 'ut_night_adds_visibility' and 
	'ut_night_multiplies_visibility'.) */
    if (night_at(tosee->x, tosee->y)) {
	if (((ut_vision_night_effect(u, t) * u_vision_range(u)) / 100) < dist)
	  chance = 0;
    }
    return chance;
}

/* A unit occupies a cell by adding itself to the list of occupants.
   It will not occupy a transport even if one is at this position
   (other code should have taken care of this case already) If
   something goes wrong, return false.  This routine is heavily
   used. */

int
enter_cell(Unit *unit, int x, int y)
{
    /* This is essential since some calls to enter_cell do not wrap x correctly.
    Fixes bug that caused units to disappear when they crossed the date line, 
    and also bug that placed new units at off-world positions. */
    x = wrapx(x);

#ifdef DEBUGGING
    /* Not necessarily an error, but indicative of bugs elsewhere. */
    if (unit->x >= 0 || unit->y >= 0) {
	run_warning("unit %d occupying cell (%d, %d), was at (%d %d)",
		    unit->id, x, y, unit->x, unit->y);
    }
#endif /* DEBUGGING */
    /* Always check this one, but not necessarily fatal. */
    if (!inside_area(x, y)) {
	run_warning("No cell at %d,%d, %s can't enter it",
		    x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
#if (0)
    if (!type_can_occupy_cell(unit->type, x, y)) {
	run_warning("Cell at %d,%d is too full for %s",
		    x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
#endif
    add_unit_to_ustack(unit, x, y);
    /* Set the location slots now. */
    enter_cell_aux(unit, x, y);
    /* Inevitable side-effect of appearing in the new location. */
    all_see_occupy(unit, x, y, TRUE);
    kick_out_enemy_users(unit->side, x, y);
    return TRUE;
}

int
type_survives_in_cell(int u, int x, int y)
{
	int c;
	                                                                                                              
	/* First check if there is a safe connection to sit on. */
	for_all_connection_types(c) {
		if (aux_terrain_defined(c)
		    && any_connections_at(x, y, c)
		    && type_survives_in_terrain(u, c)) {
			return TRUE;
		}
	}
	/* If not, check if the ground terrain itself is safe. */
	if (type_survives_in_terrain(u, terrain_at(x, y))) {
		return TRUE;
	}
	return FALSE;
}

int
type_survives_in_terrain(int u, int t)
{
	if (ut_vanishes_on(u, t))
	    return FALSE;
	if (ut_wrecks_on(u, t))
	    return FALSE;
	return TRUE;
}

/* Return true if the given unit type can fit onto the given cell. */

/* (should eventually account for variable-size units) */

static int *toc_numtypes;

int
type_can_occupy_cell(int u, int x, int y)
{
    int t = terrain_at(x, y), u2, u3, numthistype = 0, fullness = 0;
    Unit *unit2;

    if (t_capacity(t) <= 0 && ut_capacity_x(u, t) <= 0) {
	return FALSE;
    }
    if (toc_numtypes == NULL) {
	toc_numtypes = (int *) xmalloc(numutypes * sizeof(int));
    }
    for_all_unit_types(u3) {
	toc_numtypes[u3] = 0;
    }
    for_all_stack(x, y, unit2) {
	u2 = unit2->type;
	++toc_numtypes[u2];
	if (u2 == u) {
	    ++numthistype;
	}
	/* Only count against fullness if exclusive capacity exceeded. */
	if (toc_numtypes[u2] > ut_capacity_x(u2, t)) {
	    fullness += ut_size(u2, t);
	}
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= ut_capacity_x(u, t)) {
	return TRUE;
    }
    /* Otherwise decide on the basis of fullness. */
    if (fullness + ut_size(u, t) <= t_capacity(t)) {
    	return TRUE;
    }
    return FALSE;
}

/* Same as above but uses unit views instead of units. */

int
side_thinks_type_can_occupy_cell(Side *side, int u, int x, int y)
{
    int t = terrain_at(x, y), u2, u3, numthistype = 0, fullness = 0;
    UnitView *uview2;

    if (t_capacity(t) <= 0 && ut_capacity_x(u, t) <= 0) {
	return FALSE;
    }
    if (toc_numtypes == NULL) {
	toc_numtypes = (int *) xmalloc(numutypes * sizeof(int));
    }
    for_all_unit_types(u3) {
	toc_numtypes[u3] = 0;
    }
    for_all_view_stack(side, x, y, uview2) {
	u2 = uview2->type;
	++toc_numtypes[u2];
	if (u2 == u) {
	    ++numthistype;
	}
	/* Only count against fullness if exclusive capacity exceeded. */
	if (toc_numtypes[u2] > ut_capacity_x(u2, t)) {
	    fullness += ut_size(u2, t);
	}
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= ut_capacity_x(u, t)) {
	return TRUE;
    }
    /* Otherwise decide on the basis of fullness. */
    if (fullness + ut_size(u, t) <= t_capacity(t)) {
    	return TRUE;
    }
    return FALSE;
}

int
type_can_occupy_empty_cell(int u, int x, int y)
{
    int t = terrain_at(x, y);

    return type_can_occupy_terrain(u, t);
}

int
type_can_occupy_terrain(int u, int t)
{
    if (t_capacity(t) >= ut_size(u, t))
        return TRUE;
    if (ut_capacity_x(u, t) > 0)
        return TRUE;
    return FALSE;
}

/* Similar, but don't count a specific given unit when calculating. */
/* (should share with can_occupy_cell, make unit3 be optional arg?) */

int
can_occupy_cell_without(Unit *unit, int x, int y, Unit *unit3)
{
    return type_can_occupy_cell_without(unit->type, x, y, unit3);
}

static int *tocw_numtypes;

int
type_can_occupy_cell_without(int u, int x, int y, Unit *unit3)
{
    int t = terrain_at(x, y), u2, u3, numthistype = 0, fullness = 0;
    Unit *unit2;

    if (t_capacity(t) <= 0 && ut_capacity_x(u, t) <= 0) {
	return FALSE;
    }
    if (tocw_numtypes == NULL) {
	tocw_numtypes = (int *) xmalloc(numutypes * sizeof(int));
    }
    for_all_unit_types(u3) {
	tocw_numtypes[u3] = 0;
    }
    for_all_stack(x, y, unit2) {
	/* This is the only way this function differs from type_can_occupy_cell. */
	if (unit2 == unit3) {
		continue;
	}
	u2 = unit2->type;
	++tocw_numtypes[u2];
	if (u2 == u) {
		++numthistype;
	}
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tocw_numtypes[u2] > ut_capacity_x(u2, t)) {
	    fullness += ut_size(u2, t);
	}
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= ut_capacity_x(u, t)) {
	return TRUE;
    }
    /* Otherwise decide on the basis of fullness. */
    if (fullness + ut_size(u, t) <= t_capacity(t)) {
	return TRUE;
    }
    return FALSE;
}

/* Tests if side can put a new unit at (x, y) either in the terrain or within 
a friendly unit. Tests both for survival and capacity, unlike the can_occupy 
functions. Intended for use when creating new units rather than moving an
existing unit, therefore does not test if the type can also enter the cell or 
one of the stack units. */

int
side_can_put_type_at(Side *side, int u, int x, int y)
{
	Unit *unit;

	/* First test if the new unit can sit in the terrain. */
	if (type_can_occupy_cell(u, x, y)
	    && type_survives_in_cell(u, x, y)) {
		return TRUE;
	}
	/* Then test if it can sit within an existing friendly unit. */
	for_all_stack_with_occs(x, y, unit) {
		if (enemy_side(unit->side, side)) {
			continue;
		}
		if (type_can_occupy(u, unit)) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Same as above but checks for capacity rather than actual space.
Intended for path-finiding etc. where space may become available
while the unit is on its way there. Does however check for actual
space if immobile units are present. */

int
side_thinks_it_can_put_type_at(Side *side, int u, int x, int y)
{
	UnitView *uview;

	/* If we don't know what's out there, everything is possible. 
	This "blissful ignorance" rule has some weird consequences, 
	but it is necessary to avoid cheating. */
	if (!terrain_visible(side, x, y)) {
		return TRUE;
	}
	/* First test if the side thinks the new unit can sit in the terrain. */
	if (side_thinks_type_can_occupy_cell(side, u, x, y)
	    && type_survives_in_cell(u, x, y)) {
		return TRUE;
	}
	/* Then test if the side thinks it can sit within a friendly unit. */
	for_all_view_stack(side, x, y, uview) {
		if (enemy_side(side_n(uview->siden), side)) {
			continue;
		}
		if (side_thinks_type_can_occupy(side, u, uview)) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Same as above but tests for room in the absence of a particular 
unit (usually the creator). */

int
side_can_put_type_at_without(Side *side, int u, int x, int y, Unit *unit)
{
	Unit *unit2;

	/* First test if the new unit can sit in the terrain. */
	if (type_can_occupy_cell_without(u, x, y, unit)
	    && type_survives_in_cell(u, x, y)) {
		return TRUE;
	}
	/* Then test if it can sit within an existing friendly unit. */
	for_all_stack_with_occs(x, y, unit2) {
		if (enemy_side(unit2->side, side)) {
			continue;
		}
		if (type_can_occupy_without(u, unit2, unit)) {
			return TRUE;
		}
	}
	return FALSE;
}

int
side_thinks_it_can_put_type_at_without(Side *side, int u, int x, int y, Unit *unit)
{
	UnitView *uview;

	/* First test if the new unit can sit in the terrain. */
	if (type_can_occupy_cell_without(u, x, y, unit)
	    && type_survives_in_cell(u, x, y)) {
		return TRUE;
	}
	/* Then test if it can sit within an existing friendly unit. */
	for_all_view_stack_with_occs(side, x, y, uview) {
		if (enemy_side(side_n(uview->siden), side)) {
			continue;
		}
		if (side_thinks_type_can_occupy_without(u, uview, unit)) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Recursive helper to update everybody's position.  This should be one of
   two routines that modify actual unit positions (leaving is the other). */

void
enter_cell_aux(Unit *unit, int x, int y)
{
    int u = unit->type;
    Unit *occ;

#ifdef DEBUGGING
    /* Not necessarily an error, but indicative of bugs elsewhere. */
    if (unit->x >= 0 || unit->y >= 0) {
	run_warning("unit %d occupying cell (%d, %d), was at (%d %d)",
		    unit->id, x, y, unit->x, unit->y);
    }
#endif /* DEBUGGING */
    if (!in_area(x, y))
      run_error("trying to enter cell outside world");
    /* Actually set the unit position. */
    set_unit_position(unit, x, y, unit->z);
    /* Increment viewing coverage. */
    cover_area(unit->side, unit, unit->transport, -1, -1, x, y);
    /* If vision range is 0, allow glimpses of adjacent cell terrain.
       This applies to terrain only, adjacent units cannot be seen. */
    if (u_vision_range(u) == 0)
      glimpse_adjacent_terrain(unit);
    /* Do for all the occupants too, recursively. */
    for_all_occupants(unit, occ) {
	enter_cell_aux(occ, x, y);
    }
}

/* Decide whether the given unit can actually be in the given transport. */
/* (this still needs to account for multipart units) */

/* The following three functions all consider a real transport and now
call the same function (type_can_occupy) to do the work. */

int
can_occupy(Unit *unit, Unit *transport)
{
    /* Intercept nonsensical arguments. */
    if (transport == unit)
      return FALSE;
    return type_can_occupy(unit->type, transport);
}

int
type_can_occupy(int u, Unit *transport)
{
    int utransport = transport->type, u3, o, space = 0;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int numfacilities = 0;
    int nummobiles = 0;
    int ucap, uucap;
    Unit *occ;

    /* Don't allow occupation of incomplete transports unless the unit is
       of a type that can help complete or the transport self-builds. Note:
       this test is questionable, but keep for now after fixing self-build
       bug. */
    if (!completed(transport) 
        && uu_acp_to_build(u, utransport) < 1
        && u_cp_per_self_build(utransport) < 1) {
	return FALSE;
    }
    ucap = u_capacity(utransport);
    uucap = uu_capacity_x(utransport, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;
    for_all_unit_types(u3)
      tmp_u_array[u3] = 0;
    /* Compute the transport's fullness. */
    for_all_occupants(transport, occ) {
    	o = occ->type;
	++numalltypes;
	++tmp_u_array[occ->type];
	if (o == u)
	  ++numthistype;
	if (u_facility(o))
	  ++numfacilities;
	if (mobile(o))
	  ++nummobiles;
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tmp_u_array[o] > uu_capacity_x(utransport, o)) {
	    occvolume += uu_size(o, utransport);
	}
    }
    
    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */
    
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(utransport, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(utransport, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(utransport) >= 0
	    && u_facility(u)
	    && numfacilities + 1 > u_facility_total_max(utransport))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(utransport) >= 0
	    && mobile(u)
	    && nummobiles + 1 > u_mobile_total_max(utransport))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(utransport) >= 0
        && numalltypes + 1 > u_occ_total_max(utransport))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (occvolume + uu_size(u, utransport) <= ucap);
    return space;
}

int
side_thinks_type_can_occupy(Side *side, int u, UnitView *transport)
{
    int utransport = transport->type, u3, o, space = 0;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int numfacilities = 0;
    int nummobiles = 0;
    int ucap, uucap;
    UnitView *occ;

    /* Don't allow occupation of incomplete transports unless the unit is
       of a type that can help complete or the transport self-builds. Note:
       this test is questionable, but keep for now after fixing self-build
       bug. */
    if (!transport->complete 
        && uu_acp_to_build(u, utransport) < 1
        && u_cp_per_self_build(utransport) < 1) {
	return FALSE;
    }
    ucap = u_capacity(utransport);
    uucap = uu_capacity_x(utransport, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;
    for_all_unit_types(u3)
      tmp_u_array[u3] = 0;
    /* Compute the transport's fullness. */
    for_all_occupant_views(transport, occ) {
    	o = occ->type;
	++numalltypes;
	++tmp_u_array[occ->type];
	if (o == u)
	  ++numthistype;
	if (u_facility(o))
	  ++numfacilities;
	if (mobile(o))
	  ++nummobiles;
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tmp_u_array[o] > uu_capacity_x(utransport, o)) {
	    occvolume += uu_size(o, utransport);
	}
    }
    
    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */
    
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(utransport, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(utransport, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(utransport) >= 0
	    && u_facility(u)
	    && numfacilities + 1 > u_facility_total_max(utransport))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(utransport) >= 0
	    && mobile(u)
	    && nummobiles + 1 > u_mobile_total_max(utransport))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(utransport) >= 0
        && numalltypes + 1 > u_occ_total_max(utransport))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (occvolume + uu_size(u, utransport) <= ucap);
    return space;
}

/* This function a clone of type_can_occupy which ignores a specific
unit (usually the creator) when testing for room. */

int
type_can_occupy_without(int u, Unit *transport, Unit *unit)
{
    int utransport = transport->type, u3, o, space = 0;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int numfacilities = 0;
    int nummobiles = 0;
    int ucap, uucap;
    Unit *occ;

    /* Don't allow occupation of incomplete transports unless the unit is
       of a type that can help complete or the transport self-builds. Note:
       this test is questionable, but keep for now after fixing self-build
       bug. */
    if (!completed(transport) 
        && uu_acp_to_build(u, utransport) < 1
        && u_cp_per_self_build(utransport) < 1) {
	return FALSE;
    }
    ucap = u_capacity(utransport);
    uucap = uu_capacity_x(utransport, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;
    for_all_unit_types(u3)
      tmp_u_array[u3] = 0;
    /* Compute the transport's fullness. */
    for_all_occupants(transport, occ) {
	/* This is the only way this function differs from type_can_occupy. */
    	if (occ == unit) {
    		continue;
    	}
    	o = occ->type;
	++numalltypes;
	++tmp_u_array[occ->type];
	if (o == u)
	  ++numthistype;
	if (u_facility(o))
	  ++numfacilities;
	if (mobile(o))
	  ++nummobiles;
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tmp_u_array[o] > uu_capacity_x(utransport, o)) {
	    occvolume += uu_size(o, utransport);
	}
    }
    
    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */
    
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(utransport, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(utransport, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(utransport) >= 0
	    && u_facility(u)
	    && numfacilities + 1 > u_facility_total_max(utransport))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(utransport) >= 0
	    && mobile(u)
	    && nummobiles + 1 > u_mobile_total_max(utransport))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(utransport) >= 0
        && numalltypes + 1 > u_occ_total_max(utransport))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (occvolume + uu_size(u, utransport) <= ucap);
    return space;
}

int
side_thinks_type_can_occupy_without(int u, UnitView *transport, Unit *unit)
{
    int utransport = transport->type, u3, o, space = 0;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int numfacilities = 0;
    int nummobiles = 0;
    int ucap, uucap;
    UnitView *occ;

    /* Don't allow occupation of incomplete transports unless the unit is
       of a type that can help complete or the transport self-builds. Note:
       this test is questionable, but keep for now after fixing self-build
       bug. */
    if (!transport->complete 
        && uu_acp_to_build(u, utransport) < 1
        && u_cp_per_self_build(utransport) < 1) {
	return FALSE;
    }
    ucap = u_capacity(utransport);
    uucap = uu_capacity_x(utransport, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;
    for_all_unit_types(u3)
      tmp_u_array[u3] = 0;
    /* Compute the transport's fullness. */
    for_all_occupant_views(transport, occ) {
	/* This is the only way this function differs from type_can_occupy. */
    	if (view_unit(occ) == unit) {
    		continue;
    	}
    	o = occ->type;
	++numalltypes;
	++tmp_u_array[occ->type];
	if (o == u)
	  ++numthistype;
	if (u_facility(o))
	  ++numfacilities;
	if (mobile(o))
	  ++nummobiles;
	/* Only count against fullness if exclusive capacity exceeded. */
	if (tmp_u_array[o] > uu_capacity_x(utransport, o)) {
	    occvolume += uu_size(o, utransport);
	}
    }
    
    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */
    
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(utransport, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(utransport, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(utransport) >= 0
	    && u_facility(u)
	    && numfacilities + 1 > u_facility_total_max(utransport))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(utransport) >= 0
	    && mobile(u)
	    && nummobiles + 1 > u_mobile_total_max(utransport))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(utransport) >= 0
        && numalltypes + 1 > u_occ_total_max(utransport))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (occvolume + uu_size(u, utransport) <= ucap);
    return space;
}

/* This function is special in that it considers only the type of the
transport (which has not been created yet). It is a clone of 
type_can_occupy which assumes that no other occs exist. */

int
type_can_occupy_empty_type(int u, int u2)
{
    int ucap, uucap, space = 0;

    ucap = u_capacity(u2);
    uucap = uu_capacity_x(u2, u);
    if (ucap <= 0 && uucap <= 0)
      return FALSE;

    /* It is not very logical that dedicated space is not counted when we
    consider the upper limit on the total number of occupants (either of
    this type or in general). However, since the code works like this,
    games that use dedicated space hopefully take it into account. HR. */
    
    /* Can carry if dedicated space available. */
    if (1 <= uucap)
      return TRUE;
    /* Check upper limit on count of occupants of this type. */
    if (uu_occ_max(u2, u) >= 0
        && 1 - uucap > uu_occ_max(u2, u))
      return FALSE;
	/* Check upper limit on number of facilities. */
	if (u_facility_total_max(u2) >= 0
	    && u_facility(u)
	    && 1 > u_facility_total_max(u2))
		return FALSE;
	/* Check upper limit on number of mobiles. */
	if (u_mobile_total_max(u2) >= 0
	    && mobile(u)
	    && 1 > u_mobile_total_max(u2))
		return FALSE;
    /* Check upper limit on count of occupants of all types. */
    if (u_occ_total_max(u2) >= 0
        && 1 > u_occ_total_max(u2))
      return FALSE;
    /* Can carry if general unit hold has room. */
    space = (uu_size(u, u2) <= ucap);
    return space;
}

/* Tests if type can have any occupants at all. */

int
type_can_have_occs(int u)
{
    int u2;

    for_all_unit_types(u2) {
	if (could_carry(u, u2))
	  return TRUE;
    }
    return FALSE;
}

/* Find out if an unit is in the correct position in a stack 
   (cell or transport), relative to another unit. */

int
is_at_correct_ustack_position(Unit *unit, Unit *unit2)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int uso = -1, u2so = -1;

    assert_error(unit && alive(unit), 
		 "Attempted to place a dead unit in a ustack");
    assert_error(unit2 && alive(unit2), 
		 "Attempted to compare an unit to a dead unit");
    u = unit->type;
    u2 = unit2->type;
    uso = u_stack_order(u);
    u2so = u_stack_order(u2);
    /* Enforce side ordering 1st. */
    if (unit->side) {
	if (unit2->side) {
	    if (unit->side->id > unit2->side->id)
	      return FALSE;
	    if (unit->side->id < unit2->side->id)
	      return TRUE;
	}
	else
	  return FALSE;
    }
    else {
	if (unit2->side)
	  return TRUE;
    }
    /* Enforce stack ordering 2nd. */
    if (uso < u2so)
      return FALSE;
    if (uso > u2so)
      return TRUE;
    /* Enforce utype ordering 3rd. */
    if (u > u2)
      return FALSE;
    if (u < u2)
      return TRUE;
    /* Enforce unit ID ordering 4th. */
    if (unit->id < unit2->id)
      return TRUE;
    return FALSE;
}

/* Units become occupants by linking into the transport's occupant list. */

void
enter_transport(Unit *unit, Unit *transport)
{
    assert_error(unit && alive(unit), 
		 "Attempted to enter a dead unit into a transport");
    assert_error(transport && alive(transport),
		 "Attempted to enter an unit into a dead transport");
    assert_error(unit != transport, "Attempted to enter an unit into itself");
    /* Don't enter transport if there is no room. */
    if (!can_occupy(unit, transport)) {
	/* Disband the unit if it is incomplete. */
    	if (!completed(unit))
	    notify(unit->side, "%s is full. New unit disbanded on creation.", 
		   transport->name);			
	    kill_unit(unit, H_UNIT_DISBANDED);
	return;
     }	
    /* Add unit to transport's ustack. */
    add_unit_to_ustack(unit, transport);
    /* Point from the unit back to its transport. */
    unit->transport = transport;
    /* If the transport is not yet on the map (such as when patching
       object refs during readin), skip anything that needs the transport's
       location.  It will be handled when the transport is placed. */
    if (inside_area(transport->x, transport->y)) {
	/* Set the passenger's coords to match the transport's. */
	enter_cell_aux(unit, transport->x, transport->y);
	/* Others might be watching. */
	all_see_occupy(unit, transport->x, transport->y, FALSE);
    }
}

/* Unit departs from a cell by zeroing out pointer if in cell or by being
   removed from the list of transport occupants. */

/* Dead units (hp = 0) may be run through here, so don't error out. */

void
leave_cell(Unit *unit)
{
    int ux = unit->x, uy = unit->y;
    Unit *transport = unit->transport;

    if (ux < 0 || uy < 0) {
	/* Sometimes called twice */
    } else if (transport != NULL) {
	leave_transport(unit);
	leave_cell_aux(unit, transport);
	all_see_leave(unit, ux, uy, FALSE);
	/* not all_see_cell here because can't see inside transports */
	update_unit_display(transport->side, transport, TRUE);
    } else {
	remove_unit_from_ustack(unit);
	/* Now bash the coords. */
	leave_cell_aux(unit, NULL);
	/* Now let everybody observe that the unit is gone. */
	all_see_leave(unit, ux, uy, TRUE);
    }
}

/* When leaving, remove view coverage, record old position, and then
   trash the old coordinates just in case.  Catches many bugs.  Do
   this for all the occupants as well. */

static void
leave_cell_aux(Unit *unit, Unit *oldtransport)
{
    Unit *occ;

    if (unit->x < 0 && unit->y < 0)
      run_warning("unit %s has already left the cell", unit_desig(unit));
    /* Stash the old coords. */
    unit->prevx = unit->x;  unit->prevy = unit->y;
    /* Set to a recognizable value. */
    unit->x = -1;  unit->y = -1;
    /* Make any occupants leave too. */
    for_all_occupants(unit, occ) {
	leave_cell_aux(occ, unit);
    }
    /* Decrement viewing coverage around our old location. */
    cover_area(unit->side, unit, oldtransport, unit->prevx, unit->prevy, -1, -1);
}

/* Disembarking unlinks from the list of passengers only, leaves the unit
   hanging in limbo, so should have it occupy something immediately. */

void
leave_transport(Unit *unit)
{
    Unit *transport = unit->transport, *occ;

    if (unit == transport) {
    	run_error("Unit is trying to leave itself");
    }
    if (unit == transport->occupant) {
	transport->occupant = unit->nexthere;
    } else {
	for_all_occupants(transport, occ) {
	    if (unit == occ->nexthere) {
		occ->nexthere = occ->nexthere->nexthere;
		break;
	    }
	}
    }
    /* Bash the now-spurious link. */
    unit->transport = NULL;
}

int
change_cell(Unit *unit, int x, int y)
{
    int ux = unit->x, uy = unit->y;
    Unit *transport = unit->transport;

    /* Always check this one, but not necessarily fatal. */
    if (!inside_area(x, y)) {
	run_warning("No cell at %d,%d, %s can't enter it",
		    x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
    if (!type_can_occupy_cell(unit->type, x, y)) {
	run_warning("Cell at %d,%d is too full for %s",
		    x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
    if (transport != NULL) {
	leave_transport(unit);
	update_unit_display(transport->side, transport, TRUE);
    } else {
	remove_unit_from_ustack(unit);
    }
    add_unit_to_ustack(unit, x, y);
    change_cell_aux(unit, ux, uy, x, y);
    all_see_leave(unit, ux, uy, (transport == NULL));
    /* Inevitable side-effect of appearing in the new location. */
    all_see_occupy(unit, x, y, TRUE);
    return TRUE;
}

void
set_unit_position(Unit *unit, int x, int y, int z)
{
    int u, t, tmpz;

    /* Actually set the unit position. */
    unit->x = x;  unit->y = y;  unit->z = z;
    /* Constrain the altitude according to terrain if nonzero. */
    if (unit->z != 0) {
	if ((unit->z & 1) == 0) {
	    u = unit->type;
	    t = terrain_at(x, y);
	    tmpz = unit->z / 2;
	    tmpz = min(tmpz, ut_alt_max(u, t));
	    tmpz = max(tmpz, ut_alt_min(u, t));
	    unit->z = tmpz * 2;
	} else {
	    /* (should adjust connection type?) */
	}
    }
}

static void
change_cell_aux(Unit *unit, int x0, int y0, int x, int y)
{
    int u = unit->type;
    Unit *occ;

    /* Stash the old coords. */
    unit->prevx = x0;  unit->prevy = y0;
    set_unit_position(unit, x, y, unit->z);
    /* Change viewing coverage. */
    cover_area(unit->side, unit, unit->transport, x0, y0, x, y);
    /* If vision range is 0, allow glimpses of adjacent cell terrain.
       This applies to terrain only, adjacent units cannot be seen. */
    if (u_vision_range(u) == 0)
      glimpse_adjacent_terrain(unit);
    /* Do for all the occupants too, recursively. */
    for_all_occupants(unit, occ) {
	change_cell_aux(occ, x0, y0, x, y);
    }
}

/* Splice the unit into the given transport's ustack. */

void
add_unit_to_ustack(Unit *unit, Unit *transport)
{
    Unit *topunit = NULL, *unit2 = NULL, *prevunit = NULL, *nextunit = NULL;

    assert_error(unit, "Attempted to add a NULL unit to a transport");
    assert_error(transport, "Attempted to add an unit to a NULL transport");
    topunit = transport->occupant;
    if (topunit) {
    	/* Insert the entering unit into the occupant list at
	   its correct position. */
    	for_all_occupants(transport, unit2) {
	    if (is_at_correct_ustack_position(unit, unit2)) {
		nextunit = unit2;
		if (unit2 == topunit)
		  topunit = unit;
		break;
	    }
	    prevunit = unit2;
    	}
    	if (prevunit != NULL)
    	  prevunit->nexthere = unit;
    } else {
    	topunit = unit;
    }
    unit->nexthere = nextunit;
    transport->occupant = topunit;
}

/* Put the given unit into the cell and/or unit stack at the given
   location.  Do not modify the unit's xyz properties, just the unit
   layer and links to other units. */

void
add_unit_to_ustack(Unit *unit, int x, int y)
{
    Unit *topunit = NULL, *unit2 = NULL, *prevunit = NULL, *nextunit = NULL;

    assert_error(unit, "Attempted to add a NULL unit to a location");
    assert_error(inside_area(x, y), 
		 "Attempted to an unit outside the playing area");
    topunit = unit_at(x, y);
    if (topunit) {
    	/* Insert the entering unit into the stack at its correct position. */
    	for_all_stack(x, y, unit2) {
	    if (is_at_correct_ustack_position(unit, unit2)) {
		nextunit = unit2;
		if (unit2 == topunit)
		  topunit = unit;
		break;
	    }
	    prevunit = unit2;
    	}
    	if (prevunit != NULL)
	  prevunit->nexthere = unit;
    } else {
    	topunit = unit;
    }
    unit->nexthere = nextunit;
    set_unit_at(x, y, topunit);
}

/* Remove the given unit from the stack at its current location. */

void
remove_unit_from_ustack(Unit *unit)
{
    int ux = unit->x, uy = unit->y;
    Unit *other;

    /* Unsplice ourselves from the list of units in this cell. */
    if (unit == unit_at(ux, uy)) {
	set_unit_at(ux, uy, unit->nexthere);
    } else {
	for_all_stack(ux, uy, other) {
	    if (unit == other->nexthere) {
		other->nexthere = other->nexthere->nexthere;
		break;
	    }
	} 
    }
    /* Bash this now-spurious link. */
    unit->nexthere = NULL;
}

/* Add only the terrain types of adjacent cells to the unit's side's
   view, don't show units or anything else.  This is a workaround
   for units with a vision range of 0. */

void
glimpse_adjacent_terrain(Unit *unit)
{
    int x = unit->x, y = unit->y, dir, x1, y1;
    Side *side = unit->side;

    if (u_vision_range(unit->type) == 0
	&& unit->transport == NULL
	&& !g_see_all()
	&& !g_terrain_seen()
	&& side != NULL) {
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		if (terrain_view(side, x1, y1) == UNSEEN) {
		    set_terrain_view(side, x1, y1,
				     buildtview(terrain_at(x1, y1)));
		    update_cell_display(side, x1, y1,
					UPDATE_ALWAYS | UPDATE_ADJ);
		}
	    }
	}
    }
}

/* Given an overfull unit, spew out occupants until back within limits. */

static int *num_each_type, *shared_each_type;

static void
eject_excess_occupants(Unit *unit)
{
    int u, u2 = unit->type, overfull = TRUE, count;
    int numalltypes = 0, occvolume = 0;
    Unit *occ;

    if (num_each_type == NULL)
      num_each_type = (int *) xmalloc(numutypes * sizeof(int));
    if (shared_each_type == NULL)
      shared_each_type = (int *) xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u)
      num_each_type[u] = shared_each_type[u] = 0;
    /* Eject occupants overflowing counts in shared space. */
    for_all_occupants(unit, occ)
      ++num_each_type[occ->type];
    for_all_unit_types(u) {
    	if (num_each_type[u] > uu_capacity_x(u2, u)) {
	    shared_each_type[u] = num_each_type[u] - uu_capacity_x(u2, u);
	    if (uu_occ_max(u2, u) >= 0
	    	&& shared_each_type[u] > uu_occ_max(u2, u)) {
		count = shared_each_type[u] - uu_occ_max(u2, u);
		while (count > 0) {
		    for_all_occupants(unit, occ) {
			if (occ->type == u) {
			    eject_occupant(unit, occ);
			    --count;
			    break;
			}
		    }
		}
	    }
	}
    }
    /* Eject occupants over the total max count allowed. */
    for_all_occupants(unit, occ)
      ++numalltypes;
    if (u_occ_total_max(u2) >= 0 && numalltypes > u_occ_total_max(u2)) {
    	count = numalltypes - u_occ_total_max(u2);
    	while (unit->occupant != NULL) {
	    eject_occupant(unit, unit->occupant);
	    if (--count <= 0)
	      break;
    	}
    }
    /* Eject occupants overflowing volume of shared space. */
    while (overfull) {
	for_all_unit_types(u)
	  num_each_type[u] = 0;
	occvolume = 0;
	for_all_occupants(unit, occ)
	  ++num_each_type[occ->type];
	for_all_unit_types(u) {
	    occvolume +=
	      max(0, num_each_type[u] - uu_capacity_x(u2, u)) * uu_size(u, u2);
	}
	if (occvolume > u_capacity(u2)) {
	    overfull = TRUE;
	    eject_occupant(unit, unit->occupant);
	} else {
	    overfull = FALSE;
	}
    }
}

/* Given that an occupant must leave its transport, decide what happens; either
   move out into the open, into another unit, or vanish. */

void
eject_occupant(Unit *unit, Unit *occ)
{
    if (!in_play(unit) || !in_play(occ))
      return;
    /* If the occupant is mobile and the current cell has room, let it escape
       but be stacked in the transport's cell. */
    if (mobile(occ->type)
        && type_survives_in_cell(occ->type, unit->x, unit->y)
        && type_can_occupy_cell(occ->type, unit->x, unit->y)) {
        leave_cell(occ);
        enter_cell(occ, unit->x, unit->y);
        return;
    }
    /* (should let occupants escape into other units in cell) */
    /* (should let occupants with acp escape into adj cells) */
    /* Evaporating the occupant is our last option. */
    kill_unit(occ, H_UNIT_KILLED);
}

/* Handle the general situation of a unit changing allegiance from one side
   to another.  This is a common internal routine, so no messages here. */

void
change_unit_side(Unit *unit, Side *newside, int reason, Unit *agent)
{
    using namespace Xconq::AI;

    int ux = unit->x, uy = unit->y, lostself = FALSE;
    Side *oldside = unit->side;
    Unit *occ;
    OpRole *oprole = NULL;

    if (oldside == newside)
      return;
    /* Fail if the unit may not be on the new side. */
    if (!unit_allowed_on_side(unit, newside))
	return;
    if (reason >= 0)
      record_unit_side_change(unit, newside, (HistEventType)reason, agent);
    /* Skip screen updates if the side just lost the game. */
    if (reason != H_SIDE_LOST) {
	/* Last view of unit on its old side. */
	update_unit_display(oldside, unit, TRUE);
    }
    /* Don't change occupants' side if the transport changed 
    side due to being wrecked.  */
    if (reason != H_UNIT_WRECKED 
        && reason != H_UNIT_WRECKED_IN_ACCIDENT) {
	    for_all_occupants(unit, occ) {
		change_unit_side(occ, newside, reason, agent);
	    }
    }
    /* Adjust view coverage.  The sequencing here is to make sure that no
       viewing coverage gets left on or off inadvertantly. */
    if (alive(unit) && inside_area(ux, uy)) {
	/* Uncover the current viewed area. */
	cover_area(unit->side, unit, unit->transport, ux, uy, -1, -1);
	/* Actually set the side slot of the unit here. */
	set_unit_side(unit, newside);
	// Release the side's operational role for the unit, if necessary.
	if (oprole = find_oprole(oldside, unit->id))
	    release_oprole(oprole);
	/* Check if this was a self unit. */
	if (oldside->self_unit == unit)
	    lostself = TRUE;
	/* Always redo the unit's number. */
	unit->number = 0;
	assign_unit_number(unit);
	/* Update the cancarry vector if this is a transport. */
	if (u_is_transport(unit->type)) {
	    update_cancarry_vector(unit->side);
	}
	/* Cover it for the new side now. */
	cover_area(unit->side, unit, unit->transport, -1, -1, ux, uy);
	/* Skip screen updates if the side just lost the game. */
	if (reason != H_SIDE_LOST) {
	    /* A freebie for the unit's previous side. */
	    see_exact(oldside, ux, uy);
	}
    }
    /* Handle lost self units. */
    if (lostself
        /* If we already lost the game, we don't want the fact that our 
	   former units change side being interpreted as capture. 
	   Nor do we want to call side_loses again below. */
        && reason != H_SIDE_LOST)
      resurrect_self_unit(unit, unit->side, newside);
    /* The new side gets to decide the unit's new plans.  */
    init_unit_plan (unit);
    /* Skip screen updates if the side just lost the game. */
    if (reason != H_SIDE_LOST) {
	/* Now we see the unit as belonging to someone else. */
	update_unit_display(oldside, unit, TRUE);
	update_unit_display(newside, unit, TRUE);
    }
}

/* Attempt to resurrect self as new unit. Handle the consequences of 
   failing to do so. */

void
resurrect_self_unit(Unit *unit, Side *oldside, Side *newside)
{
    char unitname[BUFSIZE];
    char unitname2[BUFSIZE];
    char newsidename[BUFSIZE];
    char oldsidename[BUFSIZE];
    Unit *unit2 = NULL;

    assert_error(unit, "Attempted to access a NULL unit");
    assert_error(oldside, "Attempted to access a NULL side");
    if (!newside)
      newside = oldside;
    oldside->self_unit = NULL;
    strncpy(unitname, short_unit_handle(unit), BUFSIZE);
    strncpy(newsidename, short_side_title(newside), BUFSIZE);
    strncpy(oldsidename, side_adjective(oldside), BUFSIZE);
    /* Find a new self unit if possible. */
    if (u_self_resurrects(unit->type)) {
    for_all_side_units(oldside, unit2) {
	if (is_active(unit2)
	    && u_can_be_self(unit2->type)) {
	    oldside->self_unit = unit2;
	    strcpy(unitname2, short_unit_handle(unit2));
	    break;
	}
    } /* for all side units */
    } /* if self resurrects */
    if (oldside->ingame) {
	/* Unit was given to or captured by another side. */
	if (oldside != newside) {
	    notify_all(
"THE %s %s %s IS NOW IN THE HANDS OF %s!", 
		       all_capitals(oldsidename), 
		       ((!mobile(unit->type) || u_advanced(unit->type))
			    ? "CAPITAL" : "LEADER"),
		       all_capitals(unitname), 
		       all_capitals(newsidename));
	}
	/* Unit is longer the self unit for some other reason 
	   (change-type, for example). */
	else {
	    notify_all(
"%s NO LONGER IS THE %s OF THE %s SIDE.",
		       all_capitals(unitname), 
		       ((!mobile(unit->type) || u_advanced(unit->type))
			    ? "CAPITAL" : "LEADER"),
		       all_capitals(oldsidename));
	}
	/* Mention a new self unit, if one was found. */
	if (oldside->self_unit) {
	    notify_all("THE %s LEADERSHIP RELOCATES TO %s!", 
		       all_capitals(oldsidename), 
		       all_capitals(unitname2));
	}
	/* Handle side loss if no new self unit was found and one is 
	   required. */
	else {
	    if (g_self_required()) {
		strcpy(oldsidename, short_side_title(oldside));
		if (newside != oldside) {
		    notify_all("%s HAS BEEN DEFEATED BY %s!", 
			       all_capitals(oldsidename), 
			       all_capitals(newsidename));
		    side_loses(oldside, newside, -2);
		}
		else {
		    notify_all("%s HAS FALLEN!", all_capitals(oldsidename));
		    side_loses(oldside, NULL, -2);
		}
	    }
	}
    }
}

/* This is a general test as to whether the given unit can be
   on the given side. */

int
unit_allowed_on_side(Unit *unit, Side *side)
{
    int u;

    if (unit == NULL)
      return FALSE;
    u = unit->type;
    return new_unit_allowed_on_side(u, side);
}

/* It is crucial to pass the unit type instead of the unit itself as
   argument in cases where the test must be applied before a new unit
   is created, to prevent the appearance of independent ghost units
   that never got the correct side set. Fortunately, there is no need
   to pass a real unit since the test only uses the type. */

int
new_unit_allowed_on_side(int u, Side *side)
{
    int u2, sum;

    /* Test general limitations on the type. */
    if (!type_allowed_on_side(u, side))
      return FALSE;
    /* Test specific game limits. */
    if (u_type_per_side_max(u) >= 0) {
	if (side->numunits[u] >= u_type_per_side_max(u))
	  return FALSE;
    }
    if (g_units_per_side_max() >= 0) {
	sum = 0;
	for_all_unit_types(u2) {
	    sum += side->numunits[u2];
	}
	if (sum >= g_units_per_side_max())
	  return FALSE;
    }
    return TRUE;
}

/* Inability to build a unit should not preclude its capture
   etc. Separate calls to has_advance_to_build have therefore been
   introduced everywhere in the code. */

int
type_allowed_on_side(int u, Side *side)
{
    int u2;
    ParamBoxSide paramboxs;

    if (side->uavail == NULL) {
	side->uavail = (short *) xmalloc(numutypes * sizeof(short));
	for_all_unit_types(u2) {
	    paramboxs.side = side;
	    side->uavail[u2] =
	      eval_boolean_expression(u_possible_sides(u2),
				      fn_test_side_in_sideclass, TRUE,
				      (ParamBox*)&paramboxs);
	}
    }
    return side->uavail[u];
}

int
type_ever_available(int u, Side *side)
{
    int u2 = NONUTYPE, u3 = NONUTYPE, m = NONMTYPE, t = NONTTYPE;
    int a = NONATYPE;
    Unit *unit;

    if (!type_allowed_on_side(u, side))
      return FALSE;
    for_all_unit_types(u2) {
        if (!type_allowed_on_side(u2, side))
          continue;
	if (could_create(u2, u))
	  return TRUE;
        if (could_capture(u2, u, side))
          return TRUE;
        if (could_change_type_to(u, u2))
          return TRUE;
        if (u_wrecked_type(u2) == u)
          return TRUE;
	if (u_auto_upgrade_to(u2) == u)
	  return TRUE;
	for_all_unit_types(u3) {
	    if (!type_allowed_on_side(u3, side))
	      continue;
	    if (uu_wrecked_type_if_killed(u3, u2) == u)
	      return TRUE;
	}
	for_all_material_types(m) {
	    /* (Should also check to see if utype can actually starve from 
		need of the material.) */
	    if (um_wrecked_type_if_starved(u2, m) == u)
	      return TRUE;
	}
	for_all_terrain_types(t) {
	    /* (Should also check to see if utype can actually be on 
	        the damaging terrain.) */
	    if (ut_wrecked_type_if_attrited(u2, t) == u)
	      return TRUE;
	}
    }
    for_all_advance_types(a) {
	/* Assume that the advance can be researched. */
	if (ua_needed_to_build(u, a))
	  return TRUE;
    }
    for_all_side_units(side, unit) {
	if (unit->type == u)
	  return TRUE;
    }
    return FALSE;
}

int
num_sides_allowed(int u)
{
    int rslt;
    Side *side;

    rslt = 0;
    for_all_sides(side) {
	if (type_allowed_on_side(u, side)) {
	    ++rslt;
	}
    }
    return rslt;
}

/* Return true if unit1 trusts unit2.  */
int
unit_trusts_unit(Unit *unit1, Unit *unit2)
{
    return (unit1->side == unit2->side
	    || trusted_side(unit1->side, unit2->side));
}

/* Put the given unit on the given side, without all the fancy
   effects.  Important to handle independents, because this gets
   called during init.  This is the only way that a unit's side may be
   altered. */

/* Note that this may be run on dead units, as part of clearing out a
   side's units, in which case we just want to relink, don't care
   about testing whether the type is allowed or not. */

void
set_unit_side(Unit *unit, Side *side)
{
    int u = unit->type;
    Side *oldside, *newside;

    /* Might not have anything to do. */
    if (unit->side == side)
      return;
    /* Subtract from the counts for the ex-side. */
    oldside = unit->side;
    if (oldside->numunits)
      --(oldside->numunits[u]);
    /* Set the unit's slot. */
    /* Note that indep units have a NULL side, even though there
	   is an actual side object for independents. */
    unit->side = side;
    /* Make sure this unit is off anybody else's list. */
    delete_unit(unit);
    newside = side;
    insert_unit(newside->unithead, unit);
    /* Add to counts for the side. */
    if (newside->numunits)
      ++(newside->numunits[u]);
    /* Invalidate both sides' point value caches. */
    oldside->point_value_valid = FALSE;
    newside->point_value_valid = FALSE;
    /* Bump the tech level if owning this type helps. */
    if (side != NULL
	&& side->tech[u] < u_tech_from_ownership(u)) {
	side->tech[u] = u_tech_from_ownership(u);
	/* (should update any displays of tech - how to ensure?) */
    }
}

/* The origside is more of a historical record or cache, doesn't need
   the elaboration that unit side change does. */

void
set_unit_origside(Unit *unit, Side *side)
{
    unit->origside = side;
}

void
set_unit_name(Side *side, Unit *unit, char *newname)
{
    /* Always turn 0-length names into NULL. */
    if (newname != NULL && strlen(newname) == 0)
      newname = NULL;
    /* Don't do anything if the name didn't actually change. */
    if ((unit->name == NULL && newname == NULL)
        || (unit->name != NULL
	    && newname != NULL
	    && strcmp(unit->name, newname) == 0))
      return;
    /* Record this in the history. */
    record_unit_name_change(unit, newname);
    unit->name = newname;
    update_unit_display(side, unit, TRUE);
    update_unit_display(unit->side, unit, TRUE);
    /* (should also send to any other side directly viewing this unit!) */
}

/* Given an amount to add, add it to the unit's hp, being careful
   to check the limits.  Expect the caller to update the unit's
   display, for instance there may be a health bar to change. */

void
add_to_unit_hp(Unit *unit, int hp)
{
    int hpmax;

    unit->hp += hp;
    hpmax = u_hp(unit->type);
    if (unit->hp > hpmax)
      unit->hp = hpmax;
    unit->hp2 += hp;
    if (unit->hp2 > hpmax)
      unit->hp2 = hpmax;
}

void
change_morale(Unit *unit, int sign, int morchange)
{
    int u = unit->type, oldmorale;

    if (morchange != 0) {
	oldmorale = unit->morale;
	unit->morale += (sign * prob_fraction(morchange));
	if (unit->morale < 0)
	  unit->morale = 0;
	if (unit->morale > u_morale_max(u))
	  unit->morale = u_morale_max(u);
	if (unit->morale != oldmorale) {
	    update_unit_display(unit->side, unit, TRUE);
	    /* (should also send to any other side directly viewing this unit?) */
	}
    }
}

int
disband_unit(Side *side, Unit *unit)
{
    int rslt;

#ifdef DESIGNERS
    if (side->designer) {
	return designer_disband(unit);
    }
#endif /* DESIGNERS */
    if (side_can_disband(side, unit)) {
	rslt = disband_unit_directly(side, unit);
	if (rslt) {
	    /* Nothing to do */
	} else if (unit->plan) {
	    set_disband_task(unit);
	} else {
	    /* In order for this to work, we would need a way to direct one
	       sort of unit to disband another.  Just fail for now. */
	    return FALSE;
	}
	return TRUE;
    } else {
	return FALSE;
    }
}

int
disband_unit_directly(Side *side, Unit *unit)
{
    if (side_can_disband(side, unit)) {
	if (!completed(unit)) {
	    /* Nothing complicated about getting rid of an incomplete unit. */
	    kill_unit(unit, H_UNIT_DISBANDED);
	    return TRUE;
	} else {
	    return FALSE;
	}
    } else {
	return FALSE;
    }
}

void
wreck_unit(Unit *unit, HistEventType hevttype, WreckType wrecktype, 
	   int wreckarg, Unit *agent)
{
    int wreckedutype = NONUTYPE;
    int u = NONUTYPE;

    assert_error(unit, "Tried to wreck a null unit.");
    u = unit->type;
    wreckedutype = u_wrecked_type(u);
    /* Determine if the unit has a specialized wreck type depending on the 
       conditions under which it wrecked. */
    if (wreckarg > -1) {
	switch (wrecktype) {
	  case WRECK_TYPE_KILLED:
	    assert_warning_break(is_unit_type(wreckarg),
"Invalid unit type argument in unit wrecking code.");
	    if (uu_wrecked_type_if_killed(u, wreckarg) != NONUTYPE)
	      wreckedutype = uu_wrecked_type_if_killed(u, wreckarg);
	    break;
	  case WRECK_TYPE_STARVED:
	    assert_warning_break(is_material_type(wreckarg),
"Invalid material type argument in unit wrecking code.");
	    if (um_wrecked_type_if_starved(u, wreckarg) != NONUTYPE)
	      wreckedutype = um_wrecked_type_if_starved(u, wreckarg);
	    break;
	  case WRECK_TYPE_ATTRITED:
	    assert_warning_break(is_terrain_type(wreckarg),
"Invalid terrain type argument in unit wrecking code.");
	    if (ut_wrecked_type_if_attrited(u, wreckarg) != NONUTYPE)
	      wreckedutype = ut_wrecked_type_if_attrited(u, wreckarg);
	    break;
	  case WRECK_TYPE_UNSPECIFIED: case WRECK_TYPE_ACCIDENT:
	  case WRECK_TYPE_TERRAIN: case WRECK_TYPE_SIDE_LOST:
	  default:
	    break;
	}
    }
    /* Change the unit's type and side. */
    change_unit_type(unit, wreckedutype, hevttype, 
		     (agent ? agent->side : NULL));
    /* Changing the type will kill the unit if there is no room
       for the new type. */
    if (alive(unit)) {
	    /* Restore to default hp for the new type. */
	    unit->hp = unit->hp2 = u_hp(unit->type);
	    /* Get rid of occupants if now overfull. */
	    eject_excess_occupants(unit);
    }
}

/* Remove a unit from play.  This is different from making it
   available for reallocation - only the unit flusher can do that.  We
   remove all the passengers too, recursively.  Sometimes units are
   "killed twice", so be sure not to run all this twice.  Also count
   up occupant deaths, being sure not to count the unit itself as an
   occupant. */

void
kill_unit(Unit *unit, int reason)
{
    int ux = unit->x, uy = unit->y;

    if (alive(unit)) {
	leave_cell(unit);
	/* A freebie for the unit's side. */
	see_exact(unit->side, ux, uy);
	kill_unit_aux(unit, reason);
    }
}

/* Do the self unit specific parts of killing a unit. */
static void
kill_self_unit(Unit *unit, char *unitname)
{
    resurrect_self_unit(unit, unit->side, NULL);
#if (0)
    int u = unit->type;
    char sidename[BUFSIZE];

    unit->side->self_unit = NULL;
    strcpy(sidename, side_adjective(unit->side));
    if (u_self_resurrects(u)) {
	/* Find a new self unit if possible. */
	Unit *unit2;

	for_all_side_units(unit->side, unit2) {
	    if (is_active(unit2)
		&& u_can_be_self(unit2->type)) {
		char unitname2[BUFSIZE];
		unit->side->self_unit = unit2;
		strcpy(unitname2, short_unit_handle(unit2));
		if (!mobile(unit->type) || u_advanced(unit->type)) {
		    notify_all("THE %s CAPITAL %s HAS BEEN DESTROYED!", 
			       all_capitals(sidename), all_capitals(unitname)); 
		    notify_all("THE %s GOVERNMENT ESCAPES TO %s!", 
			       all_capitals(sidename), all_capitals(unitname2));
		} else {
		    notify_all("THE %s LEADER %s HAS BEEN KILLED!", 
			       all_capitals(sidename), all_capitals(unitname));
		    notify_all("THE NEW %s LEADER IS %s!", 
			       all_capitals(sidename), all_capitals(unitname2));
		}
		break;
	    }	    
	}	    
    }
    /* We failed to appoint a new self-unit ... */
    if (unit->side->ingame
	&& unit->side->self_unit == NULL) {
	if (!mobile(unit->type) || u_advanced(unit->type)) {
	    notify_all("THE %s CAPITAL %s HAS BEEN DESTROYED!", 
		       all_capitals(sidename), all_capitals(unitname));
	} else {
	    notify_all("THE %s LEADER %s HAS BEEN KILLED!", 
		       all_capitals(sidename), all_capitals(unitname));
	}
	/* ... and we really need one. */
	if (g_self_required()) {
	    strcpy(sidename, short_side_title(unit->side));
	    notify_all("%s HAS LOST THE GAME!", all_capitals(sidename));
	    /* If the capital/leader has been killed, we lose to NULL, rather
	       than to the killing side. */
	    side_loses(unit->side, NULL, -2);
	}
    }
#endif
}

/* Trash it now - occupant doesn't need to leave_cell.  Also record
   the event, and update the apropriate display.  The unit here should
   be known to be alive. */

void
kill_unit_aux(Unit *unit, int reason)
{
    int u = unit->type;
    char unitname[BUFSIZE];
    Unit *occ;
    Side *side = unit->side;
    
    /* Save this handle before the unit is killed. */
    strcpy(unitname, short_unit_handle(unit));
    /* Now kill the unit. */
    unit->hp = 0;
    /* Get rid of the unit's plan/tasks.  This should be safe, because
       unit death should only happen during action execution and in
       between turns, and plans/tasks should not be in use at those times. */
    /* Might not be anything to dispose of. */
    if (unit->plan != NULL) {
	free_plan(unit);
	unit->plan = NULL;
    }
    /* Maybe enter the loss into the historical record. */
    if (reason >= 0)
      record_unit_death(unit, (HistEventType)reason);
    remove_unit_from_vector(unit->side->actionvector, unit, -1);
    if (side != NULL) {
	/* Invalidate the side's point value cache. */
	side->point_value_valid = FALSE;
	update_unit_display(side, unit, TRUE);
    }
    /* Kill all the occupants in turn. */
    for_all_occupants(unit, occ) {
	if (alive(occ))
	  kill_unit_aux(occ, reason);
	/* We must take each occ out of the world explcitly, 
	    since 'kill_unit_aux' doesn't do that in the recursive call. */
	leave_cell(occ);
    }
    /* Advanced unit support. */
    if (u_advanced(unit->type))
    	free_used_cells(unit);
    /* Update global counts. */
    --numliveunits;
    --(numlivebytype[u]);
    if (unit->side->self_unit == unit) {
        kill_self_unit(unit, unitname);
    }
}

/* Free up all cells used by unit that passed away. */

void
free_used_cells(Unit *unit)
{
    int x, y;

    for_all_cells_within_reach(unit, x, y) {
	if (!inside_area(x, y))
	  continue;
	if (user_at(x, y) == unit->id) 
	  set_user_at(x, y, NOUSER);
    }
    unit->usedcells = 0;
}

/* Get rid of all dead units at once.

   (This routine is basically a garbage collector, and should not be
   called during a unit list traversal.) The process starts by finding
   the first live unit, making it the head, then linking around all in
   the middle.  Dead units stay on the dead unit list for each side
   until that side has had a chance to move.  Then they are finally
   flushed in a permanent fashion. */

static void flush_one_unit(Unit *unit);

void
flush_dead_units(void)
{
    Unit *unit, *prevunit, *nextunit;

    if (unitlist == NULL)
      return;
    unit = unitlist;
    while (!alive(unit)) {
	nextunit = unit->unext;
	delete_unit(unit);
	flush_one_unit(unit);
	unit = nextunit;
	if (unit == NULL)
	  break;
    }
    unitlist = unit;
    /* Since the first unit of unitlist is guaranteed live now,
       we know that prevunit will always be set correctly;
       but mollify insufficiently intelligent compilers. */
    prevunit = NULL;
    for_all_units(unit) {
	if (!alive(unit)) {
	    nextunit = unit->unext;
	    prevunit->unext = unit->unext;
	    delete_unit(unit);
	    flush_one_unit(unit);
	    unit = prevunit;
	} else {
	    prevunit = unit;
	}
    }
}

/* Keep it clean - hit all links to other places.  Some might not be
   strictly necessary, but this is not an area to take chances with. */

static void
flush_one_unit(Unit *unit)
{
    unit->id = -1;
    unit->imf = NULL;
    unit->image_name = NULL;
    unit->occupant = NULL;
    unit->transport = NULL;
    unit->nexthere = NULL;
    unit->prev = NULL;
    unit->unext = NULL;
    /* Add it on the front of the list of available units. */
    unit->next = freeunits;
    freeunits = unit;
}

/* Do a bubble sort.
   Data is generally coherent, so bubble sort not too bad if we allow
   early termination when everything is already in order.  */

/* If slowness objectionable, replace with something clever, but be
   sure that average performance in real games is what's being improved. */

void
sort_units(int byidonly)
{
    int flips;
    int passes = 0;
    register Unit *unit, *nextunit;
    Side *side;

    for_all_sides(side) {
	passes = 0;
	flips = TRUE;
	while (flips) {
	    flips = FALSE;
	    for_all_side_units(side, unit) {
		if (unit->next != side->unithead
		    && (byidonly ? ((unit->id - unit->next->id) > 0)
				 : (compare_units(unit, unit->next) > 0))) {
		    flips = TRUE;
		    /* Reorder the units by fiddling with their links. */
		    nextunit = unit->next;
		    unit->prev->next = nextunit;
		    nextunit->next->prev = unit;
		    nextunit->prev = unit->prev;
		    unit->next = nextunit->next;
		    nextunit->next = unit;
		    unit->prev = nextunit;
		}
		++passes;
	    }
	}
    }
    Dprintf("Sorting passes = %d\n", passes);
}

static int
compare_units(Unit *unit1, Unit *unit2)
{
    if (unit1->type != unit2->type)
      return (unit1->type - unit2->type);
    if (unit1->name && unit2->name == NULL)
      return -1;
    if (unit1->name == NULL && unit2->name)
      return 1;
    if (unit1->name && unit2->name)
      return strcmp(unit1->name, unit2->name);
    if (unit1->number != unit2->number)
      return (unit1->number - unit2->number);
    /* Ids impose a total ordering. */
    return (unit1->id - unit2->id);
}

#if 0		/* Unused. */

/* Useful for the machine player to know how long it can move this
   piece before it should go home.  Assumes can't replenish from
   terrain.  Result may be negative, in which case it's time to go! */

int
moves_till_low_supplies(Unit *unit)
{
    int u = unit->type, m, moves = 1234567, tmp;

    for_all_material_types(m) {
	if ((um_consumption_per_move(u, m) > 0)) {
	    tmp = (unit->supply[m] - um_storage_x(u, m) / 2) / um_consumption_per_move(u, m);
	    moves = min(moves, tmp);
	}
    }
    return moves;
}

#endif

/* Short, unreadable, but greppable listing of unit.  Primarily useful
   for debugging and warnings.  We use several buffers and rotate between
   them so we can call this more than once in a single printf. */

char *
unit_desig(Unit *unit)
{
    int i;
    char *shortbuf;

    /* Allocate if not yet done so. */
    for (i = 0; i < NUMSHORTBUFS; ++i) {
	if (shortbufs[i] == NULL)
	  shortbufs[i] = (char *)xmalloc(BUFSIZE);
    }
    /* Note that we test here, so that unit_desig(NULL) can be used
       to allocate any space that this routine might need later. */
    if (unit == NULL)
      return "no unit";
    shortbuf = shortbufs[curshortbuf];
    curshortbuf = (curshortbuf + 1) % NUMSHORTBUFS;
    if (unit->id == -1) {
	sprintf(shortbuf, "s%d head", side_number(unit->side));
	return shortbuf;
    } else if (is_unit_type(unit->type)) {
	sprintf(shortbuf, "s%d %s %d (%d,%d",
		side_number(unit->side), shortest_unique_name(unit->type),
		unit->id, unit->x, unit->y);
	if (unit->z != 0)
	  tprintf(shortbuf, ",%d", unit->z);
	if (unit->transport)
	  tprintf(shortbuf, ",in%d", unit->transport->id);
	strcat(shortbuf, ")");  /* close out the unit location */
	return shortbuf;
    } else {
	return "!garbage unit!";
    }
}

/* Short, unreadable, but greppable listing of unit that omits anything
   that changes from turn to turn. */

char *
unit_desig_no_loc(Unit *unit)
{
    char *shortbuf;

    if (unit == NULL)
      return "no unit";
    /* Allocate if not yet done so. */
    if (shortbufs[curshortbuf] == NULL)
      shortbufs[curshortbuf] = (char *)xmalloc(BUFSIZE);
    shortbuf = shortbufs[curshortbuf];
    curshortbuf = (curshortbuf + 1) % NUMSHORTBUFS;
    if (unit->id == -1) {
	sprintf(shortbuf, "s%d head", side_number(unit->side));
	return shortbuf;
    } else if (is_unit_type(unit->type)) {
	sprintf(shortbuf, "s%d %-3.3s %d",
		side_number(unit->side), shortest_unique_name(unit->type),
		unit->id);
	return shortbuf;
    } else {
	return "!garbage unit!";
    }
}

/* Come up with a unit type name that fits in the given space. */

char *
utype_name_n(int u, int n)
{
    char *utypename, *shortname, *rawname;

    utypename = u_type_name(u);
    if (n <= 0 || strlen(utypename) <= (size_t)n) {
	return utypename;
    } else if (n == 1 && !empty_string(u_uchar(u))) {
	/* Use the unit char if possible. */
	return u_uchar(u);
    } else if (!empty_string(u_short_name(u))) {
    	shortname = u_short_name(u);
	if (strlen(shortname) <= (size_t)n) {
	    return shortname;
	} else {
	    rawname = shortname;
	}
    } else {
    	rawname = utypename;
    }
    /* Copy what will fit. */
    if (utypenamen == NULL)
      utypenamen = (char *)xmalloc(BUFSIZE);
    if (n > BUFSIZE - 1)
      n = BUFSIZE - 1;
    strncpy(utypenamen, rawname, n);
    utypenamen[n] = '\0';
    return utypenamen;
}

char *
shortest_unique_name(int u)
{
    char namebuf[BUFSIZE], *name1;
    int u1, u2, i, len, allhavechars, *firstuniq, firstuniq1;

    /* Don't try to allocate shortestnames before numutypes has been
       defined. This will cause crashes later on as the program
       mistakenly believes that all shortestnames[u] have been
       allocated just because shortestnames != NULL. */
    if (numutypes == 0)
      return NULL;
    if (shortestnames == NULL) {
	shortestnames = (char **) xmalloc(numutypes * sizeof(char *));
	firstuniq = (int *) xmalloc(numutypes * sizeof(int));
	/* First use game definition's single chars if possible. */
	allhavechars = TRUE;
	for_all_unit_types(u1) {
	    firstuniq[u1] = -1;
	    if (!empty_string(u_uchar(u1))) {
		namebuf[0] = (u_uchar(u1))[0];
		namebuf[1] = '\0';
		shortestnames[u1] = copy_string(namebuf);
		firstuniq[u1] = 0;
	    } else {
		allhavechars = FALSE;
	    }
	}
	if (!allhavechars) {
	    /* Start with copies of full name for all types not
               already named. */
	    for_all_unit_types(u1) {
		if (shortestnames[u1] == NULL) {
		    shortestnames[u1] = copy_string(u_type_name(u1));
		    firstuniq[u1] = 0;
		}
	    }
	    for_all_unit_types(u1) {
		if (firstuniq[u1] < 0) {
		    name1 = shortestnames[u1];
		    firstuniq1 = firstuniq[u1];
		    for_all_unit_types(u2) {
			if (u1 != u2) {
			    /* Look through the supposedly minimal
			       unique substring and see if it is the
			       same. */
			    for (i = 0; i < firstuniq1; ++i ) {
				if (name1[i] != (shortestnames[u2])[i]) {
				    break;
				}
			    }
			    /* If so, must extend the unique substring. */
			    if (i == firstuniq1) {
				/* Look for the first nonmatching char. */
				while (name1[firstuniq1] == (shortestnames[u2])[firstuniq1]) {
				    ++firstuniq1;
				}
			    }
			}
		    }
		    firstuniq[u1] = firstuniq1;
		}
	    }
	    /* For any types where the unique short name is shorter
	       than the seed name, truncate appropriately. */
	    longest_shortest = 0;
	    for_all_unit_types(u1) {
		if (size_t(firstuniq[u1] + 1) < strlen(shortestnames[u1])) {
		    (shortestnames[u1])[firstuniq[u1] + 1] = '\0';
		}
		len = strlen(shortestnames[u1]);
		if (len > longest_shortest)
		  longest_shortest = len;
	    }
	} else {
		longest_shortest = 1;
	}
	if (Debug) {
	    for_all_unit_types(u1) {
		Dprintf("Shortest type name: %s for %s\n",
			shortestnames[u1], u_type_name(u1));
	    }
	}
    }
    return shortestnames[u];
}

/* Similar to shortest_unique_name, but returns a generic name/char
   instead. */

char *
shortest_generic_name(int u)
{
    char namebuf[BUFSIZE], *name1;
    int u1, u2, i, allhavechars, *firstuniq, firstuniq1;

    if (shortestgnames == NULL) {
	shortestgnames = (char **) xmalloc(numutypes * sizeof(char *));
	firstuniq = (int *) xmalloc(numutypes * sizeof(int));
	/* First use game definition's single chars if possible. */
	allhavechars = TRUE;
	for_all_unit_types(u1) {
	    firstuniq[u1] = -1;
	    if (!empty_string(u_gchar(u1))) {
		namebuf[0] = (u_gchar(u1))[0];
		namebuf[1] = '\0';
		shortestgnames[u1] = copy_string(namebuf);
		firstuniq[u1] = 0;
	    } else if (!empty_string(u_uchar(u1))) {
		namebuf[0] = (u_uchar(u1))[0];
		namebuf[1] = '\0';
		shortestgnames[u1] = copy_string(namebuf);
		firstuniq[u1] = 0;
	    } else {
		allhavechars = FALSE;
	    }
	}
	if (!allhavechars) {
	    /* Start with copies of full name for all types not
               already named. */
	    for_all_unit_types(u1) {
		if (shortestgnames[u1] == NULL) {
		    name1 = (!empty_string(u_generic_name(u1)) ? u_generic_name(u1) : u_type_name(u1));
		    shortestgnames[u1] = copy_string(name1);
		    firstuniq[u1] = 0;
		}
	    }
	    for_all_unit_types(u1) {
		if (firstuniq[u1] < 0) {
		    name1 = shortestgnames[u1];
		    firstuniq1 = firstuniq[u1];
		    for_all_unit_types(u2) {
			if (u1 != u2) {
			    /* Look through the supposedly minimal
			       unique substring and see if it is the
			       same. */
			    for (i = 0; i < firstuniq1; ++i ) {
				if (name1[i] != (shortestgnames[u2])[i]) {
				    break;
				}
			    }
			    /* If so, must extend the unique substring. */
			    if (i == firstuniq1) {
				/* Look for the first nonmatching char. */
				while (name1[firstuniq1] == (shortestgnames[u2])[firstuniq1]) {
				    ++firstuniq1;
				}
			    }
			}
		    }
		    firstuniq[u1] = firstuniq1;
		}
	    }
	    /* For any types where the unique short name is shorter
	       than the seed name, truncate appropriately. */
	    for_all_unit_types(u1) {
		if (size_t(firstuniq[u1] + 1) < strlen(shortestgnames[u1])) {
		    (shortestgnames[u1])[firstuniq[u1] + 1] = '\0';
		}
	    }
	}
	if (Debug) {
	    for_all_unit_types(u1) {
		Dprintf("Shortest generic type name: %s for %s\n",
			shortestgnames[u1], u_type_name(u1));
	    }
	}
    }
    return shortestgnames[u];
}

/* This formats an actorstate readably. */

char *
actorstate_desig(ActorState *as)
{
    if (actorstatebuf == NULL)
      actorstatebuf = (char *)xmalloc(BUFSIZE);
    if (as != NULL) {
	sprintf(actorstatebuf, "acp %d/%d %s",
		as->acp, as->initacp, action_desig(&(as->nextaction)));
	return actorstatebuf;
    } else {
	return "no act";
    }
}

/* Search for a unit with the given id number. */

/* This is used a lot, it should be sped up. */

Unit *
find_unit(int n)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->id == n && alive(unit))
	  return unit;
    }
    return NULL;
}

/* Same, but don't by picky about liveness. */

Unit *
find_unit_dead_or_alive(int n)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->id == n)
	  return unit;
    }
    return NULL;
}

/* Find a unit with the given name, either alive or dead. */

Unit *
find_unit_by_name(char *nm)
{
    Unit *unit;

    if (nm == NULL)
      return NULL;
    for_all_units(unit) {
	if (unit->name != NULL && strcmp(unit->name, nm) == 0)
	  return unit;
    }
    return NULL;
}

/* Find a unit with the given number, either alive or dead. */

Unit *
find_unit_by_number(int nb)
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->number == nb)
	  return unit;
    }
    return NULL;
}

/* Find a unit with the given symbol, either alive or dead. */

Unit *
find_unit_by_symbol(Obj *sym)
{
    Unit *unit;

    if (sym == lispnil)
      return NULL;
    for_all_units(unit) {
	if (equal(unit_symbol(unit), sym))
	  return unit;
    }
    return NULL;
}

/* Insert the given unit after the other given unit. */

void
insert_unit(Unit *unithead, Unit *unit)
{
    unit->next = unithead->next;
    unit->prev = unithead;
    unithead->next->prev = unit;
    unithead->next = unit;
}

/* Delete the unit from its list. */

void
delete_unit(Unit *unit)
{
    unit->next->prev = unit->prev;
    unit->prev->next = unit->next;
}

#if 0 /* not used, although they seem useful... */
int
num_occupants(Unit *unit)
{
    int num = 0;
    Unit *occ;

    for_all_occupants(unit, occ) {
	num += 1;
    }
    return num;
}

int
num_units_at(int x, int y)
int x, y;
{
    int num = 0;
    Unit *unit;

    if (!in_area(x, y)) {
	run_warning("num_units_at %d,%d??", x, y);
	return 0;
    }
    for_all_stack(x, y, unit) {
	num += 1;
    }
    return num;
}
#endif

/* Call this to doublecheck invariants on units. */

void
check_all_units(void)
{
    Unit *unit;

    for_all_units(unit) {
	check_unit(unit);
    }
}

void
check_unit(Unit *unit)
{
    if (alive(unit) && unit->transport && !alive(unit->transport)) {
    	run_warning("%s is inside a dead transport", unit_desig(unit));
    }
    /* etc */
}

UnitVector *
make_unit_vector(int initsize)
{
    UnitVector *vec;
	
    vec = (UnitVector *)
      xmalloc(sizeof(UnitVector) + initsize * sizeof(UnitVectorEntry));
    vec->size = initsize;
    vec->numunits = 0;
    return vec;
}

void
clear_unit_vector(UnitVector *vec)
{
    vec->numunits = 0;
}

UnitVector *
add_unit_to_vector(UnitVector *vec, Unit *unit, int flag)
{
    int i;
    UnitVector *newvec;

    /* Can't add to something that doesn't exist! */
    if (vec == NULL)
      run_error("No actionvector!");
    /* (should search to see if already present) */
    if (vec->numunits >= vec->size) {
	newvec = make_unit_vector((3 * vec->size) / 2);
	newvec->numunits = vec->numunits;
	for (i = 0; i < vec->numunits; ++i) {
	    newvec->units[i] = vec->units[i];
	}
	free(vec);
	vec = newvec;
    }
    ((vec->units)[vec->numunits]).unit = unit;
    ((vec->units)[vec->numunits]).flag = flag;
    ++(vec->numunits);
    return vec;
}

void
remove_unit_from_vector(UnitVector *vec, Unit *unit, int pos)
{
    int j;

    /* It's probably a bug that the vector is null sometimes,
       but don't flip out over it. */
    if (vec == NULL)
      return;
    /* Search for unit in vector. */
    if (pos < 0) {
	for (j = 0; j < vec->numunits; ++j) {
	    if (unit == vec->units[j].unit) {
		pos = j;
		break;
	    }
	}
    }
    if (pos < 0)
      return;
    if (unit != vec->units[pos].unit)
      run_error("unit mismatch in remove_unit_from_vector, %s not at %d",
		unit_desig(unit), pos);	
    for (j = pos + 1; j < vec->numunits; ++j)
      vec->units[j-1] = vec->units[j];
    --(vec->numunits);
}

enum sortkeys tmpsortkeys[MAXSORTKEYS];

static int
compare_units_by_keys(CONST void *e1, CONST void *e2)
{
    int i;
    Unit *unit1 = ((UnitVectorEntry *) e1)->unit;
    Unit *unit2 = ((UnitVectorEntry *) e2)->unit;
    
    if (unit1 == unit2)
      return 0;
    if (unit1 == NULL)
      return 1;
    if (unit2 == NULL)
      return -1;
    for (i = 0; i < MAXSORTKEYS; ++i) {
	switch (tmpsortkeys[i]) {
	  case byside:
	    if (unit1->side != unit2->side) {
		int s1 = side_number(unit1->side);
		int s2 = side_number(unit2->side);
		
		/* Put independents at the end of any list. */
		if (s1 == 0)
		  s1 = numsides + 1;
		if (s2 == 0)
		  s2 = numsides + 1;
		return (s1 - s2);
	    }
	    break;
	  case bytype:
	    if (unit1->type != unit2->type) {
		return (unit1->type - unit2->type);
	    }
	    break;
	  case byname:
	    if (unit1->name) {
		if (unit2->name) {
		    return strcmp(unit1->name, unit2->name);
		} else {
		    return -1;
		}
	    } else if (unit1->number > 0) {
		if (unit2->name) {
		    return 1;
		} else if (unit2->number > 0) {
		    return (unit1->number - unit2->number);
		} else {
		    return -1;
		}
	    } else if (unit2->name) {
		return 1;
	    } else if (unit2->number > 0) {
		return 1;
	    }
	    break;
	  case byactorder:
	    /* (should sort by action priority?) */
	    break;
	  case bylocation:
	    if (unit1->y != unit2->y) {
		return (unit2->y - unit1->y);
	    } else if (unit1->x != unit2->x) {
		return (unit1->x - unit2->x);
	    } else {
		/* Both units are at the same location. Sort by transport. */
		if (unit1->transport) {
		    if (unit2->transport) {
		    } else {
			return 1;
		    }
		} else {
		    if (unit2->transport) {
			return -1;
		    } else {
		    }
		}
	    }
	    break;
	  case bynothing:
	    return (unit1->id - unit2->id);
	  default:
	    break;
	}
    }
    /* Unit ids are all unique, so this is a reliable default sort key. */
    return (unit1->id - unit2->id);
}

void
sort_unit_vector(UnitVector *vec)
{
    qsort(vec->units, vec->numunits, sizeof(UnitVectorEntry),
	  compare_units_by_keys);
}

#ifdef DESIGNERS

/* A designer can call this to create an arbitrary unit during the game. */

Unit *
designer_create_unit(Side *side, int u, int s, int x, int y)
{
    Unit *newunit;
    Side *side2;

    side2 = side_n(s);
    if (!type_can_occupy_cell(u, x, y)
        || !type_survives_in_cell(u, x, y)
        || !new_unit_allowed_on_side(u, side2)) {
        	return NULL;
    }
    newunit = create_unit(u, TRUE);
    if (newunit == NULL)
      return NULL;
    if (s != 0) {
	side2 = side_n(s);
	set_unit_side(newunit, side2);
	set_unit_origside(newunit, side2);
	/* (should ensure that any changed counts are set correctly) */
    }
    init_supply(newunit);
    Xconq::suppress_reactions = TRUE;
    enter_cell(newunit, x, y);
    update_cell_display(side, x, y, UPDATE_ALWAYS);
    update_unit_display(side, newunit, TRUE);
    Xconq::suppress_reactions = FALSE;
    return newunit;
}

/* Move a unit to a given location instantly, with all sides observing. */

int
designer_teleport(Unit *unit, int x, int y, Unit *other)
{
    int oldx = unit->x, oldy = unit->y, rslt;
    Side *side2;

    Xconq::suppress_reactions = TRUE;
    if (other != NULL && can_occupy(unit, other)) {
	leave_cell(unit);
	enter_transport(unit, other);
	all_see_cell(x, y);
	rslt = TRUE;
    } else if (type_can_occupy_cell(unit->type, x, y)
    	&& type_survives_in_cell(unit->type, x, y)) {
	change_cell(unit, x, y);
	rslt = TRUE;
    } else {
	rslt = FALSE;
    }
    if (rslt) {
	/* Provide accurate info on affected cells. */
	for_all_sides(side2) {
	    if (is_designer(side2)) {
		see_exact(side2, oldx, oldy);
		see_exact(side2, x, y);
	    }
	}
    }
    Xconq::suppress_reactions = FALSE;
    return rslt;
}

int
designer_change_side(Unit *unit, Side *side)
{
    Side *side2;

    change_unit_side(unit, side, -1, NULL);
    for_all_sides(side2) {
	if (1 /* side2 should see change */) {
	    update_unit_display(side2, unit, TRUE);
	}
    }
    return TRUE;
}

int
designer_disband(Unit *unit)
{
    kill_unit(unit, -1);
    return TRUE;
}

#endif /* DESIGNERS */

/* Unit-related functions moved here from actions.c and plan.c. */

//! Could u be on t?

int
could_be_on(int u, int t)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_terrain_type(t),
		 "Attempted to manipulate an invalid ttype");
    if ((t_capacity(t) < ut_size(u, t)) && (0 >= ut_capacity_x(u, t)))
      return FALSE;
    return TRUE;
}

/* Functions returning general abilities of a unit. */

int
can_develop(Unit *unit)
{
    return type_can_develop(unit->type);
}

int
type_can_develop(int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (could_develop(u, u2))
	  return TRUE;
    }
    return FALSE;
}

/* This is true if the given location has some kind of material for the
   unit to extract. */

int
can_extract_at(Unit *unit, int x, int y, int *mp)
{
    int m;
    Unit *unit2;

    /* Can't do anything with an unseen location. */
    if (unit->side != NULL
	&& terrain_view(unit->side, x, y) == UNSEEN)
      return FALSE;
    for_all_material_types(m) {
	if (um_acp_to_extract(unit->type, m) > 0) {
	    /* Look for case of extraction from terrain. */
	    if (any_cell_materials_defined()
		&& cell_material_defined(m)
		&& material_at(x, y, m) > 0) {
		*mp = m;
		return TRUE;
	    }
	    /* Then look for extraction from independent unit. */
	    if (g_no_indepside_ingame()) {
		    for_all_stack(x, y, unit2) {
			if (in_play(unit2)
			    && indep(unit2)
			    && unit2->supply[m] > 0) {
			    *mp = m;
			    return TRUE;
			}
		    }
	    }
	}
    }
    return FALSE;
}

/* This is true if the given location has some kind of material for the
   unit to transfer. */

int
can_load_at(Unit *unit, int x, int y, int *mp)
{
    int m;
    Unit *unit2;

    /* Can't do anything with an unseen location. */
    if (unit->side != NULL
	&& terrain_view(unit->side, x, y) == UNSEEN)
      return FALSE;
    for_all_material_types(m) {
	if (um_acp_to_load(unit->type, m) > 0) {
	    for_all_stack(x, y, unit2) {
		if (in_play(unit2)
		    && unit2->side == unit->side
		    && type_max_acp(unit2->type) == 0
		    && um_acp_to_unload(unit2->type, m) > 0) {
		    *mp = m;
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/* This tests whether the given unit is capable of doing repair. */
/*! 
    \todo Move to 'aiunit.cc' where it probably belongs. 
    \todo Fix prototype for separate actor/agent.
*/

int
can_change_type(Unit *unit)
{
    int u2 = NONUTYPE;
    int rslt = A_ANY_CANNOT_DO;

    for_all_unit_types(u2) {
      if (valid(rslt = can_change_type_to(unit, unit, u2)))
	return rslt;
    }
    return A_ANY_CANNOT_DO;
}

int
could_change_type(int u)
{
    int u2;
	
    if (u_auto_upgrade_to(u) != NONUTYPE)
      return TRUE;
    for_all_unit_types(u2) {
        if (could_change_type_to(u, u2))
          return TRUE;
    }
    return FALSE;
}

int
can_change_type_to(Unit *unit, int u2)
{
    return (valid(can_change_type_to(unit, unit, u2)));
}

int
could_change_type_to(int u, int u2)
{
    if (uu_change_type_to(u, u2)) {
#if (0)
        if (u_acp_independent(u))
          return TRUE;
        if (uu_acp_to_change_type(u, u2) > 0)
          return TRUE;
#else
	return TRUE;
#endif
    }
    return FALSE;
}

int
can_disband(Unit *unit)
{
    return (type_can_disband(unit->type) || !completed(unit));
}

int
type_can_disband(int u)
{
    return (u_acp_to_disband(u) > 0);
}

int
side_can_disband(Side *side, Unit *unit)
{
    if (is_designer(side))
      return TRUE;
    return (side_controls_unit(side, unit)
	    && can_disband(unit));
}

/* This tests whether the given unit is capable of adding terrain. */

int
can_add_terrain(Unit *unit)
{
    return type_can_add_terrain(unit->type);
}

int
type_can_add_terrain(int u)
{
    int t;
	
    for_all_terrain_types(t) {
	if (ut_acp_to_add_terrain(u, t) > 0)
	  return TRUE;
    }
    return FALSE;
}

/* This tests whether the given unit is capable of removing terrain. */

int
can_remove_terrain(Unit *unit)
{
    return type_can_remove_terrain(unit->type);
}

int
type_can_remove_terrain(int u)
{
    int t;
	
    for_all_terrain_types(t) {
	if (ut_acp_to_remove_terrain(u, t) > 0)
	  return TRUE;
    }
    return FALSE;
}

/* Checks if there is an incomplete unit within range that we should 
resume building. */

Unit *
incomplete_build_target(Unit *unit)
{
	int	x, y, u, building, range = 0;
	Unit	*unit2, *unit3;

	/* If there is a creation ID, then look at that unit first. */
	if (unit->creation_id > 0) {
	    unit2 = find_unit(unit->creation_id);
	    if (in_play(unit2) && !completed(unit2))
	      return unit2;
	}
	/* First compute how far away we need to look for build targets. */
	for_all_unit_types(u) {
            if (could_build(unit->type, u)
                && side_can_build(unit->side, u)) {
                    range = max(range, uu_build_range(unit->type, u));
            }
	}
	/* Then look for build targets. */
	for_all_cells_within_range(unit->x, unit->y, range, x,  y) {
            if (!inside_area(x, y)) {
                continue;
            }
            if (!units_visible(unit->side, x, y)) {
                continue;
            }
            for_all_stack_with_occs(x, y, unit2) {
                if (in_play(unit2) 
                    && !completed(unit2)
                    && unit->side == unit2->side
                    && could_build(unit->type, unit2->type)
                    && side_can_build(unit->side, unit2->type)
                    && uu_build_range(unit->type, unit2->type)
                        >= distance(unit->x, unit->y, unit2->x, unit2->y)) {
                    /* Also check that nobody else is building this unit. */
                    building = FALSE;
                    for_all_side_units(unit->side, unit3) {
                        if (could_build(unit3->type, unit2->type)
                            && unit3->plan
                            && unit3->plan->tasks
                            && unit3->plan->tasks->type == TASK_BUILD
                            && unit3->plan->tasks->args[1] == unit2->id) {
                                building = TRUE;
                                break;
                        }
                    }
                    if (!building) {
                        return unit2;
                    }
                }
            }
        }   	
        return NULL;
}

/* These functions test if the given utype belonging to the given side
can build various classes of units. */

int
can_build_attackers(Side *side, int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (u_offensive_worth(u2) > 0
	    && could_create(u, u2)
	    && side_can_build(side, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_build_defenders(Side *side, int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (u_defensive_worth(u2) > 0
	    && could_create(u, u2)
	    && side_can_build(side, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_build_explorers(Side *side, int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (u_ai_explorer_worth(u2) > 0
	    && could_create(u, u2)
	    && side_can_build(side, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_build_colonizers(Side *side, int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (u_colonizer_worth(u2) > 0
	    && could_create(u, u2)
	    && side_can_build(side, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_build_facilities(Side *side, int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (u_facility_worth(u2) > 0
	    && could_create(u, u2)
	    && side_can_build(side, u2))
	  return TRUE;
    }
    return FALSE;
}

/* True if the given unit is a sort that can build other units. */

int
can_build(Unit *unit)
{
    int u2;

    for_all_unit_types(u2) {
	if (unit_can_build_type(unit, u2))
	  return TRUE;
    }
    return FALSE;
}

/* True if a unit of a given type and side can build other units. */

int type_can_build(int u, Side *side)
{
    int u2 = NONUTYPE;

    for_all_unit_types(u2) {
        if (type_can_build_type(u, side, u2))
          return TRUE;
    }
    return FALSE;
}

/* Tests if a unit can build a type at a specific location either directly
or by moving within range. */

int
unit_can_build_type_at(Unit *unit, int u2, int x, int y)
{
    int dist, range, u = unit->type, resume = FALSE;
    Side *side = unit->side;
    Unit *unit2;
    
    if (!in_area(x, y)){
	notify(side, "Cannot build outside the world.");
	notify(side, "Please pick a new site (or Escape to cancel).");
	return FALSE;
    }
    /* This should have been tested before getting this far but we check 
       it anyway. */
    if (!inside_area(x, y)){
	notify(side, "Cannot build on the edge of the world.");
	notify(side, "Please pick a new site (or Escape to cancel).");
	return FALSE;
    }
    /* This should have been tested before getting this far but we check 
       it anyway. */
    if (!type_can_build_type(u, side, u2)){
	notify(side, "%s cannot build %s. Wrong type.", 
	       unit_handle(side, unit), u_type_name(u2));
	return FALSE;
    }
    // Check for merger into transport.
    // TODO: Get the info that would be more certain about this.
    if (unit->transport && (u2 == unit->transport->type)
	&& u_advanced(u2)
	&& (x == unit->transport->x) && (y == unit->transport->y))
	return TRUE;
    /* Check if there is an incomplete unit in the cell that we clicked 
       on. Note: we don't want to use find_unit_to_complete here since it 
       also checks the current build target, irrespective of its location, 
       and searches adjacent cells for build targets. */
    for_all_stack_with_occs(x, y, unit2) {
	if (in_play(unit2)
	    && !completed(unit2)
	    && unit2->type == u2
	    && unit2->side == unit->side) {
		resume = TRUE;
		break;
	}
    }
    /* Consider both build range and create range. */
    range = min(uu_create_range(u, u2), uu_build_range(u, u2));
    dist = distance(unit->x, unit->y, x, y);
    /* We are trying to build a unit in the same cell. */
    if (dist == 0) {
	/* We can always resume building a unit in the same cell. */
	if (resume) {
		return TRUE;
	}
	/* Check that there is room to create a new unit. */
	if (side_thinks_it_can_put_type_at(side, u2, x, y)
	    /* There may still be room without the creator. */
	    || (side_thinks_it_can_put_type_at_without(side, u2, x, y, unit)
	    /* The creator may die in the process ... */
		&& (uu_hp_to_garrison(u, u2) >= u_hp(u)
	    /* ... or it may fit inside the created unit. */
	    || type_can_occupy_empty_type(u, u2)))) {
	    return TRUE;		    
	} else {
	    notify(side, "%s cannot build %s at (%d,%d). No room.", 
	    unit_handle(side, unit), u_type_name(u2), x, y);
	    notify(side, "Please pick a new site (or Escape to cancel).");
	    return FALSE;
	}
    /* We are trying to build a unit elsewhere. */
    } else { 
	/* If we need to create a new unit and there is no room,
	    then we are done. */
	if (!resume && !side_thinks_it_can_put_type_at(side, u2, x, y)) {
	    notify(side, "%s cannot build %s at (%d,%d). No room.", 
		    unit_handle(side, unit), u_type_name(u2), x, y);
	    notify(side, "Please pick a new site (or Escape to cancel).");
	    return FALSE;
	}
	/* First test if we are already within range. */
	if (dist <= range) {
		return TRUE;
	/* For mobile units we test if we can get within range. */
	} else if (mobile(u)) {
	    /* This handy path function tests
		not only if we can reach a point, 
		but also if we can get within range of that point. */
	    if (1 /*choose_move_direction(unit, x, y, range) >= 0*/) {
		return TRUE;
	    } else {
		notify(side, 
		"%s cannot build %s at (%d,%d). No way there.", 
		unit_handle(side, unit), u_type_name(u2), x, y);
		notify(side, 
		       "Please pick a new site (or Escape to cancel).");
		return FALSE;		    	
	    }
	/* Nothing else to do for immobile units. */
	} else { 
	    notify(side, 
		   "%s cannot build %s at (%d,%d). Too far away.", 
		   unit_handle(side, unit), u_type_name(u2), x, y);
	    notify(side, 
		   "Please pick a new site (or Escape to cancel).");
	    return FALSE;
	}
    }
    return FALSE;
}

/* True if the given unit can build a given type of unit. */

int
can_build_type(Unit *unit, int u2)
{
    int u = unit->type;

    if (!could_create(u, u2)) {
    	DMprintf("%s was told to build %s which it cannot create.\n",
                 unit_desig(unit), shortest_generic_name(u2));
	return FALSE;
    }
    if (!could_build(u, u2)) {
    	DMprintf("%s was told to build %s which it cannot build.\n",
                 unit_desig(unit), shortest_generic_name(u2));
	return FALSE;
    }
    if (!unit->side->could_construct[u2])
	return FALSE;
    /* Success if the unit can start building right away. */
    if (side_can_build(unit->side, u2))
      return TRUE;
    return FALSE;
}

/* Legacy call to 'can_develop_or_build_type'. */

int
unit_can_build_type(Unit *unit, int u2)
{
    return can_develop_or_build_type(unit, u2);
}

/* True if the given unit can develop/build a given type of unit. */

int
can_develop_or_build_type(Unit *unit, int u2)
{
    int u = unit->type;

    if (!could_create(u, u2)) {
    	DMprintf("%s was told to build %s which it cannot create.\n",
                 unit_desig(unit), shortest_generic_name(u2));
	return FALSE;
    }
    if (!could_build(u, u2)) {
    	DMprintf("%s was told to build %s which it cannot build.\n",
                 unit_desig(unit), shortest_generic_name(u2));
	return FALSE;
    }
    /* Success if the unit can start building right away. */
    if (side_can_build(unit->side, u2))
      return TRUE;
    /* Or, the unit might be able to work on development first, then
       will switch to building automatically when the tech is
       sufficient. */
    if (could_develop(u, u2) && side_can_develop(unit->side, u2))
      return TRUE;
    return FALSE;
}

/* True if a unit type could develop/build a given other unit type. */

int
type_can_build_type(int u, Side *side, int u2)
{
    if (!could_create(u, u2)) {
	return FALSE;
    }
    if (!could_build(u, u2)) {
	return FALSE;
    }
    /* Success if the unit can start building right away. */
    if (side_can_build(side, u2))
      return TRUE;
    /* Or, the unit might be able to work on development first, then
       will switch to building automatically when the tech is
       sufficient. */
    if (could_develop(u, u2) && side_can_develop(side, u2))
      return TRUE;
    return FALSE;
}

int
can_build_or_help(Unit *unit)
{
    int u2;

    for_all_unit_types(u2) {
	if (could_create(unit->type, u2)
	    || could_build(unit->type, u2)
	    || could_develop(unit->type, u2))
	  return TRUE;
    }
    return FALSE;
}

int
can_research(Unit *unit)
{
    /* Kind of crude, but works for now. */
    if (u_advanced(unit->type))
      return TRUE;
    return FALSE;
}

int
can_produce(Unit *unit)
{
    int m;

    for_all_material_types(m) {
	if (um_acp_to_produce(unit->type, m))
	  return TRUE;
    }
    return FALSE;
}

int
total_production(Unit *unit, int m)
{
    int t = terrain_at(unit->x, unit->y);
    int prod;
    
    prod = base_production(unit, m);
    /* This simplified calcualtion does not include productivity limits
    or the probabilistic nature of terrain productivity effects. */
    prod *= ut_productivity(unit->type, t);
    prod /= 100;
    /* Add in the production cache for advanced units. */
    if (u_advanced(unit->type)) {
    	prod += unit->production[m];
    }
    return prod;
}

int
base_production(Unit *unit, int m)
{
    int u = unit->type, occprod;

    if (unit->transport) {
	occprod = um_occ_production(u, m);
	return (occprod >= 0 ? occprod : um_base_production(u, m));
    } else {
	return um_base_production(u, m);
    }
}

int
total_consumption(Unit *unit, int m)
{
    int consum;
    
    consum = base_consumption(unit, m);
    /* Add in size-dependent consumption for advanced units. */
    if (u_advanced(unit->type)) {
    	consum += unit->size * um_consumption_per_size(unit->type, m);
    }
    return consum;
}

int
base_consumption(Unit *unit, int m)
{
    int consum;
    
    consum = um_base_consumption(unit->type, m);
    /* Multiply in effect of being an occupant. */
    if (unit->transport) {
    	consum *= um_consumption_as_occupant(unit->type, m);
    }
    return consum;
}

/* Check how long a unit can sit where it is. */

int
survival_time(Unit *unit)
{
    int m, least = 99999, rate, tmp;

    for_all_material_types(m) {
	rate = total_consumption(unit, m) - total_production(unit, m);
	if (rate > 0) {
	    tmp = unit->supply[m];
	    least = min(least, tmp / rate);
	}
    }
    return least;
}

/* Tests if unit will not move due to being immobile or having
a stationary goal. */

int
will_not_move(Unit *unit)
{
	if (!mobile(unit->type))
	    return TRUE;
	if (!is_active(unit))
	    return TRUE;
	/* The human player may have put the unit to sleep in order
	to serve as a garrison or sentry. */
	if (unit->plan && unit->plan->asleep)
	    return TRUE;
	/* The AI may have assigned the unit to occupy something. */
	if (unit->plan && unit->plan->maingoal
	    && ((unit->plan->maingoal->type == GOAL_UNIT_OCCUPIED
		 && unit->transport
		 && unit->transport == find_unit(unit->plan->maingoal->args[0]))
		|| (unit->plan->maingoal->type == GOAL_CELL_OCCUPIED
		    && unit->plan->maingoal->args[0] == unit->x
		    && unit->plan->maingoal->args[1] == unit->y))) {
 	    return TRUE;
	}
	return FALSE;
}

int
needs_material_to_move(Unit *unit, int m)
{
	if (mobile(unit->type)
	    && um_consumption_per_move(unit->type, m) > 0) {
		return TRUE;
	}
	return FALSE;
}

int
needs_material_to_survive(Unit *unit, int m)
{
	if (total_consumption(unit, m) > 0) {
		return TRUE;
	}
	return FALSE;
}

/* Test if unit can move out into adjacent cells. */

int
can_move(Unit *unit)
{
    int d, x, y;

    for_all_directions(d) {
	if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
	    /* (should account for world-leaving options?) */
	    if (could_live_on(unit->type, terrain_at(x, y)))
	      return TRUE;
	}
    }
    return FALSE;
}

/* This is the maximum distance from "home" that a unit can expect to get,
   travelling on its most hostile terrain type. */

int
operating_range_worst(int u)
{
    int m, t, prod, range, worstrange = area.maxdim;

    for_all_material_types(m) {
    	if (um_base_consumption(u, m) > 0) {
	    for_all_terrain_types(t) {
	    	if (!terrain_always_impassable(u, t)) {
	    	    prod = (um_base_production(u, m) * ut_productivity(u, t)) 
                           / 100;
                    /* (Need to consider direct withdrawals from treasury.) */
	    	    if (prod < um_base_consumption(u, m)) {
			range = um_storage_x(u, m) / 
                                (um_base_consumption(u, m) - prod);
			if (range < worstrange)
			  worstrange = range;
		    }
		}
	    }
	}
    }
    return worstrange;
}

/* Worst operating range for a real unit with given supplies. */

int
real_operating_range_worst(Unit *unit)
{
    int m, t, prod, range, worstrange = area.maxdim;
    int u = unit->type;

    for_all_material_types(m) {
    	if (um_base_consumption(u, m) > 0) {
	    for_all_terrain_types(t) {
	    	if (!terrain_always_impassable(u, t)) {
	    	    prod = (um_base_production(u, m) * ut_productivity(u, t)) 
                           / 100;
                    /* (Need to consider direct withdrawals from treasury.) */
	    	    if (prod < um_base_consumption(u, m)) {
			range = unit->supply[m] / 
                                (um_base_consumption(u, m) - prod);
			if (range < worstrange)
			  worstrange = range;
		    }
		}
	    }
	}
    }
    return worstrange;
}

/* Same, but for best terrain. */

int
operating_range_best(int u)
{
    int m, t, prod, range, tbestrange, tbest = 0, bestrange = 0;
    int moves, consump;

    for_all_terrain_types(t) {
	if (!terrain_always_impassable(u, t)) {
	    tbestrange = area.maxdim;
	    for_all_material_types(m) {
		consump = 0;
		moves = (type_max_acp(u) * type_max_speed(u)) / 100;
		if (um_consumption_per_move(u, m) > 0) {
		    consump = moves * um_consumption_per_move(u, m);
		}
		if (moves <= 0)
		  moves = 1;
		if (um_base_consumption(u, m) > 0) {
		    consump = max(consump, um_base_consumption(u, m));
		}
                /* (Need to consider direct withdrawals from treasury.) */
	    	prod = (um_base_production(u, m) * ut_productivity(u, t)) / 100;
	    	if (prod > 0) {
		    consump = max(0, consump - prod);
		}
		if (consump > 0) {
		    range = (um_storage_x(u, m) * moves) / consump;
		    if (range < tbestrange)
		      tbestrange = range;
		}
	    }
	    if (tbestrange > bestrange) {
		bestrange = tbestrange;
		tbest = t;
	    }
	}
    }
    return bestrange;
}

/* Best operating range for a real unit with given supplies. */

int
real_operating_range_best(Unit *unit)
{
    int m, t, prod, range, tbestrange, tbest = 0, bestrange = 0;
    int moves, consump, tmoves = 0;
    int u = unit->type;
    int u2 = NONUTYPE;
    Unit *unit2 = NULL;

    if (unit->transport && mobile(unit->transport->type) 
	&& !u_is_carrier(unit->transport->type))
      return real_operating_range_best(unit->transport);
    for_all_terrain_types(t) {
	if (!terrain_always_impassable(u, t)) {
	    tbestrange = area.maxdim;
	    for_all_material_types(m) {
		consump = 0;
		moves = (type_max_acp(u) * type_max_speed(u)) / 100;
		if (um_consumption_per_move(u, m) > 0) {
		    consump = moves * um_consumption_per_move(u, m);
		}
		if (moves <= 0)
		  moves = 1;
#if (0)
		/* Is transport a refuelling source? */
		if (unit->transport 
		    && valid(can_refuel(unit->transport, unit, m)))
		  unit2 = unit->transport;
		/* Account for resupplier that may pull away. */
		if (unit2) {
		    u2 = unit->transport->type;
		    if (mobile(u2) && u_is_carrier(u2)) {
			tmoves = (type_max_acp(u2) * type_max_speed(u2)) / 100;
			if (um_consumption_per_move(u, m) > 0)
			  consump += tmoves * um_consumption_per_move(u, m);
		    }
		}
#endif
		if (um_base_consumption(u, m) > 0) {
		    consump = max(consump, um_base_consumption(u, m));
		}
                /* (Need to consider direct withdrawals from treasury.) */
	    	prod = (um_base_production(u, m) * ut_productivity(u, t)) / 100;
		prod += 
		    (um_base_production(u, m) * ut_productivity_adj(u, t)) / 
		    100;
	    	if (prod > 0) {
		    consump = max(0, consump - prod);
		}
		if (consump > 0) {
		    range = (unit->supply[m] * moves) / consump;
		    if (range < tbestrange)
		      tbestrange = range;
		}
	    }
	    if (tbestrange > bestrange) {
		bestrange = tbestrange;
		tbest = t;
	    }
	}
    }
    return bestrange;
}

/* Fill an utype array from a GDL list. */

void
fill_utype_array_from_lisp(int *typeary, Obj *typeobj)
{
    int u = NONUTYPE;
    char *uname = NULL;
    Obj *rest = lispnil, *curobj = lispnil;

    /* Sanity Checks. */
    assert_error(typeobj, "Attempted to access a NULL GDL form");
    assert_error(typeary, "Attempted to fill a NULL utype array");
    if (lispnil == typeobj) {
	run_warning("Bad unit type object encountered");
	return;
    }
    /* Test symbols. */
    if (symbolp(typeobj)) {
	/* If all utypes were specified, then adjust utype array accordingly. */
	if (match_keyword(typeobj, K_USTAR)) {
	    for_all_unit_types(u)
	      typeary[u] = TRUE;
	}
	/* Else if the symbol is bound, expand it. */
	else if (boundp(typeobj))
	  fill_utype_array_from_lisp(typeary, eval_symbol(typeobj));
	/* Else, see if we can match the symbol name to an unit type name. */
	else
	  fill_utype_array_from_lisp(typeary, new_string(c_string(typeobj)));
    }
    /* Test string values. */
    else if (stringp(typeobj)) {
	uname = c_string(typeobj);
	for_all_unit_types(u) {
	    if (!strcmp(u_type_name(u), uname)) {
		typeary[u] = TRUE;
		break;
	    }
	}
	if (u >= numutypes)
	  run_warning("Bad unit type name, \"%s\" encountered", uname);
    }
    /* Test integer values. */
    else if (numberp(typeobj) || utypep(typeobj)) {
	u = c_number(typeobj);
	if (!is_unit_type(u))
	  run_warning("Bad unit type number, %d, encountered", u);
	else
	  typeary[u] = TRUE;
    }
    /*! \todo Handle utype objects? */
    /* Iterate through a list. */
    else if (consp(typeobj)) {
	/* Take care of things such as 'append'. */
	typeobj = eval(typeobj);
	/* Now go through the list. */
	for_all_list(typeobj, rest) {
	    curobj = car(rest);
	    if ((lispnil == curobj) || consp(curobj)) {
		run_warning("Bad unit type encountered");
		continue;
	    }
	    else {
		fill_utype_array_from_lisp(typeary, curobj);
	    }
	}
    }
    else
      run_warning("Bad unit type encountered");
}

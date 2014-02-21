/* The movement-related actions of Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

enum {
    over_nothing = 0,
    over_own = 1,
    over_border = 2,
    over_all = 3
};

/* We can't declare all the action functions as static because some of them
   are in other files, but don't let them be visible to all files. */

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,prepfn,netprepfn,DOFN,checkfn,ARGDECL,doc)  \
  extern int DOFN ARGDECL;

#include "action.def"

extern int retreating;
extern int retreating_from;

extern void allocate_used_cells(Unit *unit);	/* From run.c */
extern void free_used_cells(Unit *unit); 	/* From unit.c */

static int border_slide_possible(int u2, int ox, int oy, int nx, int ny); 
static int unit_traverse_blockable_by(Unit *unit, Unit *unit2);
static int any_friendly_at(Unit *unit, int x, int y);
static int total_entry_cost(int u1, int u3, int x1, int y1, int z1, int u2, int x2, int y2, int z2);
static int number_member(int x, Obj *lis);
static int wind_value(Unit *unit, int angle, int force, Obj *effect, int maxval);
static void detonate_on_approach_around(Unit *unit);
static int damaged_value(Unit *unit, Obj *effect, int maxval);
static void try_detonate_on_approach(int x, int y);
static void test_blocking_zoc(int x, int y);

static int maybe_react_to_move(Unit *unit, int ox, int oy);
static void consume_move_supplies(Unit *unit);

static int tmprslt;

/* Cache variables. */
int *cache__type_max_speed_from_any_occs = NULL;
int *cache__type_max_speed = NULL;

/* Does the unit currently have enough MP? */

int
has_enough_mp(Unit *unit, Unit *unit2, int mp)
{
    int u = NONUTYPE, u2 = NONUTYPE;

    assert_error(unit, "Tried to access a null unit.");
    assert_error(unit2, "Tried to access a null unit.");
    u = unit->type;
    u2 = unit2->type;
    if (!can_be_actor(unit))
      return FALSE;
    return (((unit->act->acp * unit_speed(unit2, unit2->x, unit2->y)) / 100
	     + u_free_mp(u2)) >= mp);
}

/* Can the unit ever have enough MP at its present location? */

int
can_have_enough_mp(Unit *unit, Unit *unit2, int mp)
{
    int u = NONUTYPE, u2 = NONUTYPE;

    assert_error(unit, "Tried to access a null unit.");
    assert_error(unit2, "Tried to access a null unit.");
    u = unit->type;
    u2 = unit2->type;
    if (!can_be_actor(unit))
      return FALSE;
    return ((((((u_acp_max(u) > 0) ? u_acp_max(u) : new_acp_for_turn(unit)) -
	       u_acp_min(u)) * unit_speed(unit2, unit2->x, unit2->y)) / 100
	     + u_free_mp(u2)) >= mp);
}

/* What is the maximum speed that can be gained from having any occupant? */
/* (To do this calculation truthfully, we need to use a knapsack algorithm,
    because it may be that some combination of occ types yields the best
    results. Nonetheless, the function, in its current form, at least tells
    us whether or not any occ type will boost the MP.) */

int
type_max_speed_from_any_occs(int u)
{
    int u2 = NONUTYPE, u3 = NONUTYPE;
    int speed = 0, maxspeed = 0;

    if (!cache__type_max_speed_from_any_occs) {
	cache__type_max_speed_from_any_occs = (int *)xmalloc(numutypes * 
							     sizeof(int));
	for_all_unit_types(u3) {
	    for_all_unit_types(u2) {
		speed = uu_occ_adds_acp(u3, u2);
		if (speed) {
		    if (type_can_occupy_empty_type(u2, u3))
			maxspeed = max(speed, maxspeed);
		}
	    }
	    cache__type_max_speed_from_any_occs[u3] = maxspeed;
	}
    }
    return cache__type_max_speed_from_any_occs[u];
}

/* What is the maximum speed an unit type can have, all things considered? */

int
type_max_speed(int u)
{
    int speed = 0;
    int u3 = NONUTYPE;

    if (!cache__type_max_speed) {
	cache__type_max_speed = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u3) {
	    speed = u_speed(u3);
	    speed += type_max_speed_from_any_occs(u3);
	    /* (Should consider multiplicative effects.) */
	    cache__type_max_speed[u3] = speed;
	}
    }
    return cache__type_max_speed[u];
}

//! Could uactor move umover?
/*! 
    \todo Allow ACP-indep movement.
    \todo Should cache results.
*/

int
could_move(int uactor, int umover)
{
    int m = NONMTYPE;

    /* Sanity checks. */
    assert_error(is_unit_type(uactor), "Attempted to access an invalid utype");
    assert_error(is_unit_type(umover), "Attempted to access an invalid utype");
    /* ACP-indep utypes cannot move. (For now.) */
    if (u_acp_independent(uactor))
      return FALSE;
    /* Must cost > 0 ACP. (For now.) */
    if (0 >= u_acp_to_move(umover))
      return FALSE;
    /* Must be able to have/acquire enough materials to move. */
    for_all_material_types(m) {
	if (0 < um_to_move(umover, m)) {
	    if (!um_storage_x(umover, m)) {
		if (!um_takes_from_treasury(umover, m))
		  return FALSE;
	    }
	    else {
		if (um_storage_x(umover, m) < um_to_move(umover, m))
		  return FALSE;
	    }
	}
	if (0 < um_consumption_per_move(umover, m)) {
	    if (!um_storage_x(umover, m)) {
		if (!um_takes_from_treasury(umover, m))
		  return FALSE;
	    }
	    else {
		if (um_storage_x(umover, m) < 
		    um_consumption_per_move(umover, m))
		  return FALSE;
	    }
	}
    }
    return TRUE;
}

//! Can a given unit move?
/*! 
    \todo Allow ACP-indep movement.
    \todo Allow taking from treasury.
*/

int
can_move(Unit *actor, Unit *mover)
{
    int acp = 0, acpextra = 0;
    int u2 = NONUTYPE;
    int m = NONMTYPE;

    /* Is the ACP source unit in play? */
    if (!in_play(actor))
      return A_ANY_ERROR;
    /* Is the unit-to-move in play? */
    if (!in_play(mover))
      return A_ANY_ERROR;
    u2 = mover->type;
    /* Check if the utype could ever move. */
    if (!could_move(actor->type, u2))
      return A_ANY_CANNOT_DO;
    /* If ACP source unit is ACP-less, 
	then it cannot supply ACP to the unit to be moved. */
    if (!actor->act)
      return A_ANY_CANNOT_DO;
    acp = u_acp_to_move(u2);
    /* If this action is a part of retreating, add more acp to represent the
       motivational power of needing to run away... */
    if (retreating && (actor == mover)) {
	if (retreating_from != NONUTYPE) {
	    acpextra = uu_acp_retreat(u2, retreating_from);
	}
    }
    /* Can ACP source unit ever have anough ACP to move? */
    if (!can_have_enough_acp(actor, max(0, acp - acpextra)))
      return A_ANY_CANNOT_DO;
    /* Does ACP source unit have enough ACP to cause a move this turn? */
    if (!has_enough_acp(actor, max(0, acp - acpextra)))
      return A_ANY_NO_ACP;
    /* Does mover unit have enough materials to act? */
    if (!has_supply_to_act(mover))
      return A_ANY_NO_MATERIAL;
    /* Does mover unit have enough materials to move? */
    for_all_material_types(m) {
    	/* We have to have a minimum amount of a certain material to move. */
	if (mover->supply[m] < um_to_move(u2, m))
	  return A_ANY_NO_MATERIAL;
    	/* We also need enough consumable materials (fuel) for the move. */
	if (mover->supply[m] < um_consumption_per_move(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* Movement actions. */

/* Record a move action as the next to do. */

int
prep_move_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_MOVE;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* The basic act of moving.  This attempts to move and maybe fails,
   but takes no corrective action.  Note that this requires space in
   the destination cell, will not board, attack, etc - all that is
   task- and plan-level behavior. */

int
do_move_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int u, u2, t, rslt, speed, mpcost, acpcost, ox, oy, oz, ot, dist;
    int nummoves, dirs[NUMDIRS], numdirs, i, ix, iy, ndist;

    u = unit->type;  
    u2 = unit2->type;
    t = terrain_at(x, y);
    ox = unit2->x;  
    oy = unit2->y;  
    oz = unit2->z;
    ot = terrain_at(ox, oy);
    speed = 100;
    mpcost = 1;
    dist = distance(ox, oy, x, y);
    if (dist == 0) {
	/* Change of altitude within same cell and/or transport departure. */
	if (unit->transport != NULL && z == unit->z) { 
		if (ut_vanishes_on(u2, t) && !can_move_via_conn(unit2, x, y)) {
		    notify(unit2->side, "%s vanishes into the %s!",
			   unit_handle(unit2->side, unit2), t_type_name(t));
		    kill_unit(unit2, H_UNIT_VANISHED);
		    rslt = A_MOVE_UNIT_GONE;
		} else if (ut_wrecks_on(u2, t) 
			   && !can_move_via_conn(unit2, x, y)) {
		    notify(unit2->side, "%s wrecks in the %s!",
			   unit_handle(unit2->side, unit2), t_type_name(t));
		    if (u_wrecked_type(u2) == NONUTYPE) {
			/* Occupants always die if the wrecked unit 
			   disappears. */
			kill_unit(unit, H_UNIT_WRECKED);
		    } else {
			/* Wreck the unit.  Note that we want to do the 
			   wrecking even if the unit will vanish, so that 
			   occupants can escape if allowed for. */
			if (!ut_vanishes_on(u_wrecked_type(u2), t)) {
			    speed = unit_speed(unit2, x, y);
			    mpcost = move_unit(unit2, x, y);
			}
			wreck_unit(unit2, H_UNIT_WRECKED, WRECK_TYPE_TERRAIN,
				   t, NULL);
			/* Now make it go away, taking unlucky occupants 
			   with it. */
			if (ut_vanishes_on(u2, t)) {
			    kill_unit(unit2, H_UNIT_VANISHED);
			}
		    }
		    rslt = A_MOVE_UNIT_GONE;
	    } else {
	    	mpcost = move_unit(unit2, x, y);
	    }
	} else {
	    unit2->z = z;
	    /* (should do more stuff - LOS view might have changed) */
	}
	acpcost = 1;
	nummoves = 1;
	rslt = A_ANY_DONE;
    } else if (dist == 1
	       || (dist == 2 && border_slide_possible(u2, ox, oy, x, y))) {
	/* Movement to an adjacent cell. */
	if (!inside_area(x, y)) {
	    /* (should notify all observers) */
	    notify(unit2->side, "%s leaves the area!",
		   unit_handle(unit2->side, unit2));
	    kill_unit(unit2, H_UNIT_LEFT_WORLD);
	    rslt = A_ANY_DONE;
	} else if (ut_vanishes_on(u2, t) && !can_move_via_conn(unit2, x, y)) {
	    notify(unit2->side, "%s vanishes into the %s!",
		   unit_handle(unit2->side, unit2), t_type_name(t));
	    kill_unit(unit2, H_UNIT_VANISHED);
	    rslt = A_MOVE_UNIT_GONE;
	} else if (ut_wrecks_on(u2, t) && !can_move_via_conn(unit2, x, y)) {
	    notify(unit2->side, "%s wrecks in the %s!",
		   unit_handle(unit2->side, unit2), t_type_name(t));
	    if (u_wrecked_type(u2) == NONUTYPE) {
		/* Occupants always die if the wrecked unit disappears. */
		kill_unit(unit, H_UNIT_WRECKED);
	    } else {
		/* Wreck the unit.  Note that we want to do the wrecking even
		   if the unit will vanish, so that occupants can escape if
		   allowed for. */
		if (!ut_vanishes_on(u_wrecked_type(u2), t)) {
		    speed = unit_speed(unit2, x, y);
		    mpcost = move_unit(unit2, x, y);
		}
		wreck_unit(unit2, H_UNIT_WRECKED, WRECK_TYPE_TERRAIN, t, NULL);
		/* Now make it go away, taking unlucky occupants with. */
		if (ut_vanishes_on(u2, t)) {
		    kill_unit(unit2, H_UNIT_VANISHED);
		}
	    }
	    rslt = A_MOVE_UNIT_GONE;
#if 0 /* seems better just to prevent via move costs... */
	} else if (ut_vanishes_on(u2, ot)
		   && (unit->transport == NULL
		       || uu_ferry_on_leave(unit->transport->type, u2) < over_own)
		   && !can_move_via_conn(unit2, x, y)) {
	    /* This case is if the unit is already on a connection in
	       hostile terrain and tries to move to hospitable terrain
	       without using a bridge, or if it is in a transport that
	       won't ferry across hostile terrain. */
	    notify(unit2->side, "%s vanishes into the %s!",
		   unit_handle(unit2->side, unit2), t_type_name(t));
	    kill_unit(unit2, H_UNIT_VANISHED);
	    rslt = A_MOVE_UNIT_GONE;
	} else if (ut_wrecks_on(u2, ot)
		   && (unit->transport == NULL
		       || uu_ferry_on_leave(unit->transport->type, u2) < over_own)
		   && !can_move_via_conn(unit2, x, y)) {
	    notify(unit2->side, "%s wrecks in the %s!",
		   unit_handle(unit2->side, unit2), t_type_name(t));
	    if (u_wrecked_type(u2) == NONUTYPE) {
		/* Occupants always die if the wrecked unit disappears. */
		kill_unit(unit, H_UNIT_WRECKED);
	    } else {
		/* Wreck the unit.  Note that we want to do the wrecking even
		   if the unit will vanish, so that occupants can escape if
		   allowed for. */
		wreck_unit(unit2);
		/* Now make it go away, taking unlucky occupants with. */
		if (ut_vanishes_on(u2, t)) {
		    kill_unit(unit2, H_UNIT_VANISHED);
		}
	    }
	    rslt = A_MOVE_UNIT_GONE;
#endif
	} else {
	    speed = unit_speed(unit2, x, y);
	    mpcost = move_unit(unit2, x, y);
	    /* ZOC move cost is added after action is done. */
	    mpcost += zoc_move_cost(unit2, ox, oy, oz);
	    rslt = A_ANY_DONE;
	}
	if (alive(unit)) {
	    if (speed > 0) {
		acpcost = (mpcost * 100) / speed;
	    } else {
		acpcost = 1;
	    }
	}
	nummoves = 1;
    } else {
	/* Movement by a distance of at least 2.  This is similar to
	   the move-to task, but the unit uses up only mp and doesn't
	   get to contemplate directions. */
	mpcost = 0;
	acpcost = 0;
	nummoves = 0;
	/* Stop the iteration if the unit stumbles into a mine field. */
	while (dist > 0 && alive(unit2)) {
	    numdirs = choose_move_dirs(unit2, x, y, TRUE,
				       plausible_move_dir, sort_directions, dirs);
	    if (numdirs == 0)
	      break;
	    for (i = 0; i < numdirs; ++i) {
	    	/* Should be unit2 ? */
		interior_point_in_dir(unit->x, unit->y, dirs[i], &ix, &iy);
		if (type_can_occupy_cell(unit2->type, ix, iy)) {
		    ox = unit2->x;  
		    oy = unit2->y;  
		    oz = unit2->z;
		    speed = unit_speed(unit2, ix, iy);
		    mpcost += move_unit(unit2, ix, iy);
		    mpcost += zoc_move_cost(unit2, ox, oy, oz);
		    if (alive(unit)) {
			if (speed > 0) {
			    acpcost += (mpcost * 100 * 100) / speed;
			} else {
			    acpcost += 1;
			}
		    }
		    break;
		}
	    }
	    ndist = distance(unit2->x, unit2->y, x, y);
	    if (ndist >= dist)
	      break;
	    dist = ndist;
	    ++nummoves;
	}
	/* Scale accumulated cost back down. */
	acpcost /= 100;
	/* Make sure a sensible result is returned. */
	if (alive(unit2)) {
	    rslt = A_ANY_DONE;
	} else {
	    rslt = A_MOVE_UNIT_GONE;
	}
    }
    if (alive(unit)) {
	acpcost = max(acpcost, u_acp_to_move(u2));
	if (acpcost < 1)
	  acpcost = 1;
	use_up_acp(unit, acpcost);
    }
    /* Count the unit as having actually moved. */
    if (alive(unit2) && unit2->act)
      unit2->act->actualmoves += nummoves;
    return rslt;
}

int
check_move_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int u, u2, u3, ox, oy, oz, mpavail, totcost, speed, dist;
    int acpavail = 0, acpextra = 0;
    int rslt = A_ANY_OK;
    int save_x, save_y, save_z;

    if (!valid(rslt = can_move(unit, unit2)))
      return rslt;
    /* Note that edge cell dests (used to leave the world) are allowed. */
    if (!in_area(x, y))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    ox = unit2->x;  oy = unit2->y;  oz = unit2->z;
    acpavail = unit->act->acp;
    /* If this action is a part of retreating, add more acp to represent the
       motivational power of needing to run away... */
    if (retreating && unit == unit2) {
	if (retreating_from != NONUTYPE) {
	    acpextra = uu_acp_retreat(u2, retreating_from);
	    acpavail += acpextra;
	}
#if (0)
	/* (should not have to modify the unit, 
	    but succeeding calls need this) */
	unit->act->acp = acpavail;
#endif
    }
    /* Destination is outside the world and we're not allowed to leave. */
    if (!inside_area(x, y) && u_mp_to_leave_world(u2) < 0)
	return A_MOVE_CANNOT_LEAVE_WORLD;
    /* Check if the destination is within our move range. */
    dist = distance(ox, oy, x, y);
    if ((dist == 2 && !border_slide_possible(u, ox, oy, x, y))
	&& dist > u_move_range(u2))
      return A_ANY_TOO_FAR;
    /* Disallow nonzero altitudes for now. */
    if (z > 0)
      return A_ANY_TOO_FAR;
    /* Check if the destination is in a blocking ZOC. */
    if (in_blocking_zoc(unit, x, y, z))
      return A_MOVE_BLOCKING_ZOC;
    /* Now start looking at the move costs. */
    u3 = (unit2->transport ? unit2->transport->type : NONUTYPE);
    totcost = total_move_cost(u2, u3, ox, oy, oz, x, y, z);
    /* this is something of a kludge, but the ZOC code really wants the unit
     * to have already moved */
    save_x = unit2->x;
    save_y = unit2->y;
    save_z = unit2->z;
    unit2->x = x;
    unit2->y = y;
    unit2->z = z;
    totcost += zoc_move_cost(unit2, ox, oy, oz);
    unit2->x = save_x;
    unit2->y = save_y;
    unit2->z = save_z;
    speed = unit_speed(unit2, x, y);
    /* Adjust total move cost accounting for extra ACP.
	Note that total move cost may be 0 in this case, 
	even though that is usually not allowed. */
    totcost = max(0, totcost - ((acpextra * speed) / 100));
    /* A unit with an adjusted speed of zero cannot move except within
       a cell. */
    if (speed == 0 && !(ox == x && oy == y && oz == z))
      return A_ANY_CANNOT_DO;
    /* Check if unit can possibly have enough mp for the move. */
    if (!can_have_enough_mp(unit, unit2, totcost))
      return A_ANY_CANNOT_DO; /* (Should have A_MOVE_CANNOT_DO) */
    /* Now check if unit has enough mp this turn. */
    mpavail = (acpavail * speed) / 100;
    /* take into account acp-min in computing mpavail; Massimo */
    if (u_acp_min(u) < 0)
      mpavail = ((acpavail - u_acp_min(u)) * speed) / 100;
    /* Zero mp always disallows movement, unless intra-cell. */
    if (mpavail <= 0 && !(ox == x && oy == y && oz == z))
      return A_MOVE_NO_MP;
    /* The free mp might get us enough moves, so add it before comparing. */
    if (mpavail + u_free_mp(u2) < totcost)
      return A_MOVE_NO_MP;
    /* If destination is too small or already full, we can't move into it. We 
    don't check for type_survives_in_cell, so players (both human and AIs)
    are free to kill or wreck their units by moving into deadly terrain. In
    most games, such moves are however prohibited either by limiting the
    space or by setting mp-to-enter-terrain to 99. */
    if (!type_can_occupy_cell(unit2->type, x, y))
      return A_MOVE_DEST_FULL;
    return A_ANY_OK;
}

int
can_move_via_conn(Unit *unit, int nx, int ny)
{
    int c, dir;

    if (numconntypes == 0)
      return FALSE;
    for_all_connection_types(c) {
	if (aux_terrain_defined(c)
	    && (dir = closest_dir(nx - unit->x, ny - unit->y)) >= 0
	    && connection_at(unit->x, unit->y, dir, c)
	    && !ut_vanishes_on(unit->type, c)
	    && !ut_wrecks_on(unit->type, c))
	  return TRUE;
    }
    return FALSE;
}

int
border_slide_possible(int u, int ox, int oy, int nx, int ny)
{
    int dx, dy, dir, bx, by, bdir, b;

    dx = nx - ox;  dy = ny - oy;
    /* Pick the cell on the right-hand side of the border in question.
       Since the distance is 2, and border sliding only works for
       cells on a "diagonal", choose the dir case-by-case. */
    if (dx == -2 && dy == 1) {
	dir = NORTHWEST;
    } else if (dx == -1) {
	if (dy == -1) {
	    dir = WEST;
	} else if (dy == 2) {
	    dir = NORTHEAST;
	} else {
	    return FALSE;
	}
    } else if (dx == 1) {
	if (dy == -2) {
	    dir = SOUTHWEST;
	} else if (dy == 1) {
	    dir = EAST;
	} else {
	    return FALSE;
	}
    } else if (dx == 2 && dy == -1) {
	dir = SOUTHEAST;
    } else {
	return FALSE;
    }
    bdir = left_dir(left_dir(dir));
    point_in_dir(ox, oy, dir, &bx, &by);
    for_all_border_types(b) {
	if (aux_terrain_defined(b)
	    && border_at(bx, by, bdir, b)
	    && ut_mp_to_traverse(u, b) >= 0)
	  return TRUE;
    }
    return FALSE;
}

int
unit_speed(Unit *unit, int nx, int ny)
{
    int u = unit->type, speed, x = unit->x, y = unit->y;
    int dir, angle1, windval1, angle2, windval2;
    int occeff, totocceff, totoccdenom;
    Unit *occ;

    /* Dead units can sometimes end up here, but don't bother
       complaining. */
    if (unit->hp <= 0)
      return 0;
    speed = u_speed(u);
    /* Consider additive/subtractive effects on speed. */
    /* Effects by occupants: mobility assistance, encumberance, etc.... */
    if (unit->occupant) {
	for_all_occupants(unit, occ) {
	    if (in_play(occ) && completed(occ)) {
		speed += uu_occ_adds_speed(u, occ->type);
	    }
	}
    }
    /* Consider multiplicative effects on speed. */
    /* Effect due to damage. */
    if (unit->hp < u_hp_max(u) && u_speed_damage_effect(u) != lispnil) {
	speed = damaged_value(unit, u_speed_damage_effect(u), speed);
    }
    /* The speed effect due to wind is an average of the wind effect
       in the current cell and in the new cell. */
    if (winds_defined() && u_speed_wind_effect(u) != lispnil) {
	dir = closest_dir(nx - x, ny - y);
	angle1 = angle_with(dir, wind_dir_at(x, y));
	windval1 = wind_value(unit, angle1, wind_force_at(x, y),
			      u_speed_wind_effect(u), 10000);
	angle2 = angle_with(dir, wind_dir_at(nx, ny));
	windval2 = wind_value(unit, angle2, wind_force_at(nx, ny),
			      u_speed_wind_effect(u), 10000);
	speed = (speed * ((windval1 + windval2) / 2)) / 100;
    }
    if (unit->occupant /* and any occupant speed factors */) {
	totocceff = 100;
	totoccdenom = 100;
	for_all_occupants(unit, occ) {
	    if (in_play(occ) && completed(occ)) {
		occeff = uu_occ_multiplies_speed(u, occ->type);
		if (occeff != 100) {
		    totocceff *= occeff;
		    totoccdenom *= 100;
		}
	    }
    	}
    	speed = (speed * totocceff) / totoccdenom;
    }
    /* Clip to limits. */
    speed = max(speed, u_speed_min(u));
    speed = min(speed, u_speed_max(u));
    return speed;
}

/* Compute and return value for a damaged unit, using a list of
   (hp val) pairs and interpolating between them. */

int
damaged_value(Unit *unit, Obj *effect, int maxval)
{
    int u, err, rslt;

    u = unit->type;
    err = interpolate_in_list_ext(unit->hp, effect, 0, 0, 0, 0, u_hp(u),
				  maxval, &rslt);
    if (err != 0) {
	run_warning("cannot get damaged value for %s at hp %d, using 100",
		    u_type_name(u), unit->hp);
	rslt = 100;
    }
    return rslt;
}

/* Compute and return the wind's effect on a unit, using a list of lists
   and interpolating between them. */

int
wind_value(Unit *unit, int angle, int force, Obj *effect, int maxval)
{
    int err, rslt;
    Obj *rest, *head, *key, *val;

    for_all_list(effect, rest) {
	head = car(rest);
	key = car(head);
	if ((numberp(key) && angle == c_number(key))
	    || (symbolp(key))
	    || (consp(key) && number_member(angle, key))) {
	    val = cadr(head);
	    if (numberp(val))
	      return c_number(val);
	    else {
		err = interpolate_in_list(force, val, &rslt);
		if (err == 0) {
		    return rslt;
		} else {
		    run_warning("no value for wind angle=%d force=%d",
				angle, force);
		    return maxval;
		}
	    }
	}
    }
    return maxval;
}

int
number_member(int x, Obj *lis)
{
    Obj *rest;

    if (lis == lispnil) {
	return FALSE;
    } else if (!consp(lis)) {
	/* should probably be an error of some sort */
	return FALSE;
    }
    for (rest = lis; rest != lispnil; rest = cdr(rest)) {
	if (numberp(car(rest)) && x == c_number(car(rest)))
	  return TRUE;
    }
    return FALSE;
}

/* Conduct the actual move (used in both normal moves and some
   combat).  Note that the new x,y may be the same as the old; this
   will happen if an occupant is getting off a transport but staying
   in the same cell. */

int
move_unit(Unit *unit, int nx, int ny)
{
    int u = unit->type, u3, ox = unit->x, oy = unit->y, oz = unit->z;
    int nz = oz;
    SideMask observers;

    u3 = (unit->transport ? unit->transport->type : NONUTYPE);
    maybe_lose_track(unit, nx, ny);
    /* Make sure terrain usage is zeroed before moving. */
    if (u_advanced(unit->type))
    	free_used_cells(unit);
    /* Disappear from the old location and appear at the new one. */
    if (unit->transport == NULL /* should be unconditional, but bugs still */) {
	change_cell(unit, nx, ny);
    } else {
	leave_cell(unit);
	enter_cell(unit, nx, ny);
    }
    if (0 /* record movement */) {
	observers = add_side_to_set(unit->side, NOSIDES);
	/* (should let other watching sides see event also) */
	record_event(H_UNIT_MOVED, observers, unit->id, nx, ny);
    }
    maybe_track(unit);
    /* Movement may set off other people's alarms. */
    maybe_react_to_move(unit, ox, oy);
    /* Might have auto-detonations in response. */
    if (max_detonate_on_approach_range >= 0) {
	detonate_on_approach_around(unit);
	/* A detonation might have been fatal, get out now if so. */
	if (!alive(unit))
	  return 1;
    }
    /* The people at the new location may change sides immediately. */
    if (people_sides_defined()
	&& any_people_side_changes
	&& probability(people_surrender_chance(u, nx, ny))) {
	change_people_side_around(nx, ny, u, unit->side);
    }
    if (control_sides_defined()) {
	if (ut_control_range(u, terrain_at(nx, ny)) >= 0) {
	    change_control_side_around(nx, ny, u, unit->side);
	}
    }
    kick_out_enemy_users(unit->side, nx, ny);
    /* Reallocating used cells already here is not strictly necessary since this is
    also done at the start of run_production, the only place where terrain usage
    matters. However, one may imagine future versions of the game where terrain 
    usage is evaluated elsewhere. It is therefore a good idea to update it as soon as 
    possible. */
    if (u_advanced(unit->type))
	allocate_used_cells(unit);
    /* Use up supplies as directed. */
    consume_move_supplies(unit);
    /* a hack */
    update_cell_display(unit->side, ox, oy, UPDATE_ALWAYS);
    /* Always return the mp cost, even if the mover died. */
    return total_move_cost(u, u3, ox, oy, oz, nx, ny, nz);
}

/* Given a location, change the controlling side. */

void
change_control_side_around(int x, int y, int u, Side *side)
{
    int con = control_side_at(x, y), s = side_number(side), pop;
    Side *oldside, *peopside, *side2;

    /* Do nothing if the cell cannot be controlled by a side. */
    if (con == NOCONTROL)
      return;
    /* Do nothing if we control this cell already. */
    if (con == s)
      return;
    oldside = side_n(con);
    /* Don't challenge control by our allies. */
    if (trusted_side(side, oldside))
      return;
    /* Return an ally's area. */
    if (people_sides_defined()
	&& (pop = people_side_at(x, y)) != NOBODY) {
	peopside = side_n(pop);
	if (trusted_side(side, peopside))
	  s = pop;
    }
    /* Change the cell's control. */
    set_control_side_at(x, y, s);
    /* All of our buddies get to see what goes on here. */
    for_all_sides(side2) {
	if (side == side2 || trusted_side(side, side2)) {
	    add_cover(side2, x, y, 1);
	}
    }
    update_cell_display_all_sides(x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    /* Previous side(s) lose free coverage. */
    for_all_sides(side2) {
	if (!trusted_side(side, side2)
	    && (oldside == side2 || trusted_side(oldside, side2))) {
	    add_cover(side2, x, y, -1);
	    /* Update coverage display. */
	    update_cell_display(side2, x, y, UPDATE_COVER);
	}
    }
    /* (should add ability to change adjacent cells also) */
}

int
can_move_at_all(Unit *unit)
{
    return (type_max_speed(unit->type) > 0);
}

/* This is true if the given location is in a blocking zoc for the unit. */

int
in_blocking_zoc(Unit *unit, int x, int y, int z)
{
    int u = unit->type, t = terrain_at(x, y), dir, x1, y1, t1, u2, range;
    Unit *unit2;

    if (max_zoc_range < 0)
      return FALSE;
    if (max_zoc_range >= 0) {
	for_all_stack(x, y, unit2) {
	    u2 = unit2->type;
	    range = zoc_range(unit2, u);
	    if (range >= 0
		&& is_active(unit2)
		&& !trusted_side(unit->side, unit2->side) /* should make a better test */
		&& uu_mp_to_enter_own(unit->type, u2) < 0
		&& ut_zoc_into(u2, t)
		&& ut_zoc_from_terrain(u2, t) > 0)
	      return TRUE;
	}
    }
    if (max_zoc_range >= 1) {
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		for_all_stack(x1, y1, unit2) {
		    u2 = unit2->type;
		    range = zoc_range(unit2, u);
		    t1 = terrain_at(x1, y1);
		    if (range >= 1
			&& is_active(unit2)
			&& unit_blockable_by(unit, unit2)
			&& ut_zoc_into(u2, t)
			&& ut_zoc_from_terrain(u2, t1) > 0)
		      return TRUE;
		    if (range >= 1
			&& is_active(unit2)
			&& unit_traverse_blockable_by(unit, unit2)
			&& distance(unit->x, unit->y, x1, y1) <= range
			/* (should check that zoc goes into old terrain) */
			&& ut_zoc_into(u2, t)
			&& ut_zoc_from_terrain(u2, t1) > 0
			&& !any_friendly_at(unit, x, y)
			)
		      return TRUE;
		}
	    }
	}
    }
    if (max_zoc_range >= 2) {
	tmprslt = FALSE;
	tmpunit = unit;
	apply_to_ring(x, y, 2, max_zoc_range, test_blocking_zoc);
	return tmprslt;
    }
    return FALSE;
}

static void
test_blocking_zoc(int x, int y)
{
    int u = tmpunit->type, u2, t = terrain_at(x, y), range;
    Unit *unit2;

    for_all_stack(x, y, unit2) {
	u2 = unit2->type;
	range = zoc_range(unit2, u);
	if (range >= distance(x, y, tmpunit->x, tmpunit->y)
	    && is_active(unit2)
	    && unit_blockable_by(tmpunit, unit2)
	    && ut_zoc_into(u2, terrain_at(tmpunit->x, tmpunit->y))
	    && ut_zoc_from_terrain(u2, t) > 0)
          tmprslt = TRUE;
    }
}

/* This is true if unit2 wants to block unit from moving. */

int
unit_blockable_by(Unit *unit, Unit *unit2)
{
    /* should make a better test than just trust - some types
       might not need to side relationship? */
    return (!trusted_side(unit->side, unit2->side)
	    && uu_mp_to_enter_zoc(unit->type, unit2->type) < 0);
}

/* This is true if unit2 wants to block unit from moving. */

int
unit_traverse_blockable_by(Unit *unit, Unit *unit2)
{
    /* should make a better test than just trust - some types
       might not need to side relationship? */
    return (!trusted_side(unit->side, unit2->side)
	    && uu_mp_to_traverse_zoc(unit->type, unit2->type) < 0);
}

int
any_friendly_at(Unit *unit, int x, int y)
{
    Unit *unit2;

    for_all_stack(x, y, unit2) {
	if (unit_trusts_unit(unit, unit2))
	  return TRUE;
    }
    return FALSE;
}

/* Compute the number of move points that will be needed to do the given
   move. */

int
total_move_cost(int u, int u2, int x1, int y1, int z1, int x2, int y2, int z2)
{
    int cost, ferry, b, c, conncost, travcost, dist, dir;

    if (z1 != 0 || z2 != 0) {
    	/* should write these calcs eventually */
    }
    dist = distance(x1, y1, x2, y2);
    if (dist == 0) {
	if (z2 != z1) {
	    /* (should have parms for up/down in same cell) */
	    return 1;
	} else {
    	    /* Unit is leaving a transport and moving into the open here;
    	       free of charge. */
    	    return 0;
    	}
    } else if (dist == 1) {
    	/* (fall through) */
    } else {
    	/* Border slide or multiple cell move. */
    	/* (should implement) */
    	return dist;
    }
    cost = 0;
    ferry = 0;
    if (u2 != NONUTYPE) {
	/* Charge for leaving the transport. */
    	cost += uu_mp_to_leave(u, u2);
    	/* See what type of ferrying we're going to get. */
    	ferry = uu_ferry_on_leave(u2, u);
    }
    if (ferry < over_own) {
    	cost += ut_mp_to_leave(u, terrain_at(x1, y1));
	/* Add in effect of any coatings at the origin. */
	if (numcoattypes > 0) {
	    for_all_terrain_types(c) {
		if (t_is_coating(c)
		    && aux_terrain_defined(c)
		    && aux_terrain_at(x1, y1, c) > 0) {
		    cost += ut_mp_to_leave(u, c);
		}
	    }
	}
    }
    if (numbordtypes > 0 && ferry < over_border) {
	/* Add any necessary border crossing costs. */
	dir = closest_dir(x2 - x1, y2 - y1);
	if (dir >= 0) {
	    for_all_border_types(b) {
		if (aux_terrain_defined(b)
		    && border_at(x1, y1, dir, b)) {
		    cost += ut_mp_to_enter(u, b);
		}
	    }
	}
    }
    if (ferry < over_all) {
	cost += ut_mp_to_enter(u, terrain_at(x2, y2));
	/* Add in effect of any coatings at the destination. */
	if (numcoattypes > 0) {
	    for_all_terrain_types(c) {
		if (t_is_coating(c)
		    && aux_terrain_defined(c)
		    && aux_terrain_at(x2, y2, c) > 0) {
		    cost += ut_mp_to_enter(u, c);
		}
	    }
	}
    }
    /* Use a connection traversal if it would be cheaper.  This is
       only automatic if the connection on/off costs are small enough,
       otherwise the unit has to do explicit actions to get on the
       connection and off again. */
    if (numconntypes > 0) {
	/* Try each connection type to see if it's better. */
	dir = closest_dir(x2 - x1, y2 - y1);
	if (dir >= 0) {
	    for_all_connection_types(c) {
		if (aux_terrain_defined(c)
		    && connection_at(x1, y1, dir, c)
		    && ((travcost = ut_mp_to_traverse(u, c)) >= 0)) {
		    conncost = ut_mp_to_enter(u, c)
		      + travcost
		      + ut_mp_to_leave(u, c);
		    cost = min(cost, conncost);
		}
	    }
	}
    }
    /* The cost of leaving the world is always an addon. */
    if (!inside_area(x2, y2)) {
    	cost += u_mp_to_leave_world(u);
    }
    /* Any (inter-cell) movement must always cost at least 1 mp. */
    if (cost < 1)
      cost = 1;
    return cost;
}

int
zoc_range(Unit *unit, int u2)
{
    int u = unit->type;

    return (uu_zoc_range(u, u2)
	    * ut_zoc_from_terrain(u, terrain_at(unit->x, unit->y))) / 100;
}

static int tmpmpcost, tmpox, tmpoy;

static void zoc_cost_fn(int x, int y);

int
zoc_move_cost(Unit *unit, int ox, int oy, int oz)
{
    int u = unit->type, u2, t1, t2, cost, mpcost, x, y, dir, x1, y1, range;
    Unit *unit2;

    /* If this is negative, ZOCs are not part of this game. */
    if (max_zoc_range < 0)
      return 0;
    if (!in_play(unit))
      return 0;
    mpcost = 0;
    x = unit->x;  y = unit->y;
    t1 = terrain_at(ox, oy);
    t2 = terrain_at(x, y);
    if (max_zoc_range == 0 || max_zoc_range == 1) {
        /* ZOCs of units in old cell. */
	for_all_stack(ox, oy, unit2) {
	    u2 = unit2->type;
            range = zoc_range(unit2, u);
	    if (is_active(unit2)
		&& unit2->side != unit->side
		&& range >= 0
		&& ut_zoc_into(u2, t1)
		/* should account for from-terrain also */
		)
	      mpcost = max(mpcost, uu_mp_to_leave_zoc(u, u2));
	}
	/* ZOCs of units in new cell. */
	for_all_stack(x, y, unit2) {
	    u2 = unit2->type;
            range = zoc_range(unit2, u);
	    if (is_active(unit2)
		&& unit2->side != unit->side
		&& range >= 0
		&& ut_zoc_into(u2, t2))
	      mpcost = max(mpcost, uu_mp_to_enter_zoc(u, u2));
	}
    }
    if (max_zoc_range > 0) {
	if (max_zoc_range == 1) {
	    /* ZOCs may be into adjacent cells. */
	    /* Look for everybody that was exerting ZOC into the old
               location. */
	    /* (should calc with stacked units also) */
	    for_all_directions(dir) {
		if (point_in_dir(ox, oy, dir, &x1, &y1)) {
		    for_all_stack(x1, y1, unit2) {
			u2 = unit2->type;
			range = zoc_range(unit2, u);
			if (in_play(unit2) /* should be is_active? */
			    && unit2->side != unit->side  /* and unfriendly */
			    && range >= 1
			    && ut_zoc_into(u2, t1)) {
			    if (1 /* leaving zoc */) {
				cost = uu_mp_to_leave_zoc(u, u2);
			    } else {
				cost = uu_mp_to_traverse_zoc(u, u2);
			    }
			    mpcost = max(mpcost, cost);
			}
			/* (and occupants?) */
		    }
		}
	    }
	    /* Look for everybody that is now exerting ZOC into the
               new location. */
	    /* (should calc with stacked units also) */
	    for_all_directions(dir) {
		if (point_in_dir(x, y, dir, &x1, &y1)) {
		    for_all_stack(x1, y1, unit2) {
			u2 = unit2->type;
			range = zoc_range(unit2, u);
			if (is_active(unit2)
			    && unit2->side != unit->side  /* and unfriendly */
			    && range >= 1
			    && ut_zoc_into(u2, t2)) {
			    if (1 /* entering zoc */) {
				cost = uu_mp_to_enter_zoc(u, u2);
			    } else {
				cost = uu_mp_to_traverse_zoc(u, u2);
			    }
			    mpcost = max(mpcost, cost);
			}
			/* (and occupants?) */
		    }
		}
	    }
	} else {
	    tmpmpcost = mpcost;
	    tmpox = ox;  tmpoy = oy;
	    tmpunit = unit;
	    apply_to_area(ox, oy, max_zoc_range + distance(ox, oy, x, y),
			  zoc_cost_fn);
	    mpcost = tmpmpcost;
	}
    }
    return mpcost;
}

static int
allowed_in_zoc(Unit *unit, Unit *unit2)
{
    /* should make a better test */
    return trusted_side(unit->side, unit2->side);
}

static void zoc_cost_fn(int x, int y);

static void
zoc_cost_fn(int x, int y)
{
    int u, u2, x0, y0, dist, odist, range, cost;
    Unit *unit2;

    u = tmpunit->type;
    x0 = tmpunit->x;  y0 = tmpunit->y;
    dist = distance(x0, y0, x, y);
    odist = distance(tmpox, tmpoy, x, y);
    for_all_stack(x, y, unit2) {
	if (is_active(unit2)) {
	    u2 = unit2->type;
	    range = zoc_range(unit2, u);
	    if (dist <= range) {
		if (odist <= range) {
		    /* Traversing the unit2's ZOC. */
		    if (!allowed_in_zoc(tmpunit, unit2)
			&& ut_zoc_into(u2, terrain_at(x0, y0))) {
			cost = uu_mp_to_traverse_zoc(u, u2);
			tmpmpcost = max(tmpmpcost, cost);
		    }
		} else {
		    /* Entering the unit2's ZOC. */
		    if (!allowed_in_zoc(tmpunit, unit2)
			&& ut_zoc_into(u2, terrain_at(x0, y0))) {
			cost = uu_mp_to_enter_zoc(u, u2);
			tmpmpcost = max(tmpmpcost, cost);
		    }
		}
	    } else {
		if (odist <= range) {
		    /* Leaving the unit2's ZOC. */
		    if (!allowed_in_zoc(tmpunit, unit2)
			&& ut_zoc_into(u2, terrain_at(x0, y0))) {
			cost = uu_mp_to_leave_zoc(u, u2);
			tmpmpcost = max(tmpmpcost, cost);
		    }
		}
	    }
	}
    }
}

/* This is a hook to handle any reactions to the unit's successful move. */

int
maybe_react_to_move(Unit *unit, int ox, int oy)
{
    return 0;
}

static void
try_detonate_on_approach(int x, int y)
{
    int dist;
    Unit *unit;

    dist = distance(tmpunit->x, tmpunit->y, x, y);
    for_all_stack(x, y, unit) {
	if (unit->side != tmpunit->side
	    && dist <= uu_detonate_approach_range(unit->type, tmpunit->type)
	    /* (should make doctrine-based decision about whether to go off) */
	    && !was_detonated(unit)
	    ) {
	    /* Found one, now set it off! */
	    detonate_unit(unit, unit->x, unit->y, unit->z);
	}
    }
}

/* For the given unit, search around the given area looking for other
   units might detonate just because of the proximity. */

void
detonate_on_approach_around(Unit *unit)
{
    int maxrange;

    tmpunit = unit;
    apply_to_area(unit->x, unit->y, max_detonate_on_approach_range,
		  try_detonate_on_approach);
    maxrange = max(detonate_urange_max(), max_t_detonate_effect_range)
      + max_detonate_on_approach_range;
    reckon_damage_around(unit->x, unit->y, maxrange, unit);
}

/* Use up the supply consumed by a successful move.  Also, the move might
   have used up essentials and left the unit without its survival needs,
   so check for this case and maybe hit/kill the unit. */

void  
consume_move_supplies(Unit *unit)
{
    int u = unit->type, m, checkstarve = FALSE;
    
    for_all_material_types(m) {
	if (um_consumption_per_move(u, m) > 0) {
	    unit->supply[m] -= um_consumption_per_move(u, m);
	    /* Don't let supply go below zero. */
	    if (unit->supply[m] <= 0) {
		unit->supply[m] = 0;
		checkstarve = TRUE;
	    }
	}
    }
    if (checkstarve)
      maybe_starve(unit, FALSE);
    /* Trigger any supply alarms. */
    if (alive(unit)
    	&& unit->plan
    	&& !unit->plan->supply_is_low
    	&& past_halfway_point(unit)
    	) {
    	unit->plan->supply_is_low = TRUE;
    	update_unit_display(unit->side, unit, TRUE); 
    }
}

/* Movement into another unit. */

/* Record an enter action as the next to do. */

int
prep_enter_action(Unit *unit, Unit *unit2, Unit *newtransport)
{
    if (unit == NULL || unit->act == NULL || unit2 == NULL
	|| newtransport == NULL)
      return FALSE;
    unit->act->nextaction.type = ACTION_ENTER;
    unit->act->nextaction.args[0] = newtransport->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

int
do_enter_action(Unit *unit, Unit *unit2, Unit *newtransport)
{
    int u2, u3, u4, ox, oy, oz, ot, nx, ny, nz, speed, acpcost, mpcost, rslt;

    u2 = unit2->type;
    ox = unit2->x;  oy = unit2->y;  oz = unit2->z;
    ot = terrain_at(ox, oy);
    u3 = newtransport->type;
    nx = newtransport->x;  ny = newtransport->y;  nz = newtransport->z;
    mpcost = 1;
    if (ut_vanishes_on(u2, ot)
	&& uu_ferry_on_enter(u3, u2) < over_all
	&& (unit->transport == NULL
	    || uu_ferry_on_leave(unit->transport->type, u2) < over_own)
	&& !can_move_via_conn(unit2, nx, ny)) {
	/* This case is if the unit is already on a connection in
	   hostile terrain and tries to enter without using a bridge,
	   or if it is on a transport in hostile terrain that doesn't
	   ferry. */
	notify(unit2->side, "%s vanishes into the %s!",
	       unit_handle(unit2->side, unit2), t_type_name(ot));
	kill_unit(unit2, H_UNIT_VANISHED);
	rslt = A_MOVE_UNIT_GONE;
    } else if (ut_wrecks_on(u2, ot)
	       && uu_ferry_on_enter(u3, u2) < over_all
	       && (unit->transport == NULL
		   || uu_ferry_on_leave(unit->transport->type, u2) < over_own)
	       && !can_move_via_conn(unit2, nx, ny)) {
	notify(unit2->side, "%s wrecks in the %s!",
	       unit_handle(unit2->side, unit2), t_type_name(ot));
	if (u_wrecked_type(u2) == NONUTYPE) {
	    /* Occupants always die if the wrecked unit disappears. */
	    kill_unit(unit, H_UNIT_WRECKED);
	} else {
	    /* Wreck the unit.  Note that we want to do the wrecking even
	       if the unit will vanish, so that occupants can escape if
	       allowed for. */
	    wreck_unit(unit, H_UNIT_WRECKED, WRECK_TYPE_TERRAIN, ot, NULL);
	    /* Now make it go away, taking unlucky occupants with. */
	    if (ut_vanishes_on(u2, ot)) {
		kill_unit(unit2, H_UNIT_VANISHED);
	    }
	}
	rslt = A_MOVE_UNIT_GONE;
    } else {
	/* Change the unit's position. */
	leave_cell(unit2);
	enter_transport(unit2, newtransport);
	/* Calculate how much acp has been used up. */
	u4 = (unit2->transport ? unit2->transport->type : NONUTYPE);
	mpcost = total_entry_cost(u2, u4, ox, oy, oz, u3, nx, ny, nz);
	rslt = A_ANY_DONE;
    }
    if (alive(unit)) {
	speed = unit_speed(unit2, nx, ny);
	if (speed > 0) {
	    acpcost = (mpcost * 100) / speed;
	} else {
	    acpcost = 1;
	}
	use_up_acp(unit, acpcost + uu_acp_to_enter(u2, u3));
    }
    return rslt;
}

int
check_enter_action(Unit *unit, Unit *unit2, Unit *newtransport)
{
    int u, u2, unewtransport, u4, u2x, u2y;
    int unewtransportx, unewtransporty, totcost, speed, mpavail, m;
    int ox, oy, oz, nx, ny, nz;

    if (!in_play(unit))
      return A_ANY_ERROR;
    if (!in_play(unit2))
      return A_ANY_ERROR;
    if (!in_play(newtransport))
      return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;  unewtransport = newtransport->type;
    if (uu_acp_to_enter(u2, unewtransport) < 1)
      return A_ANY_CANNOT_DO;
    if (!can_have_enough_acp(unit, uu_acp_to_enter(u2, unewtransport)))
      return A_ANY_CANNOT_DO;
    /* Can't enter self. */
    if (unit2 == newtransport)
      return A_ANY_ERROR;
    u2x = unit2->x;  u2y = unit2->y;
    unewtransportx = newtransport->x;  unewtransporty = newtransport->y;
    ox = unit2->x;  oy = unit2->y;  oz = unit2->z;
    nx = newtransport->x;  ny = newtransport->y;  nz = newtransport->z;
    if (!between(0, distance(ox, oy, nx, ny), 1))
      return A_ANY_ERROR;
    if (!sides_allow_entry(unit2, newtransport))
      return A_ANY_ERROR;
    if (!can_occupy(unit2, newtransport))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, uu_acp_to_enter(u2, unewtransport)))
      return A_ANY_NO_ACP;
    u4 = (unit2->transport ? unit2->transport->type : NONUTYPE);
    totcost = total_entry_cost(u2, u4, ox, oy, oz, unewtransport, nx, ny, nz);
    speed = unit_speed(unit2, unewtransportx, unewtransporty);
    if (speed > 0 && unit->act) {
	mpavail = (unit->act->acp * speed) / 100;
    } else {
	mpavail = 0;
    }
    /* If transport picks up the unit itself, no need to check mp. */
    if (uu_ferry_on_enter(unewtransport, u2) < over_all) {
	/* Zero mp always disallows movement. */
	if (mpavail <= 0)
	  return A_MOVE_NO_MP;
	/* The free mp might get us enough moves, so add it before
           comparing. */
	if (mpavail + u_free_mp(u2) < totcost)
	  return A_MOVE_NO_MP;
    }
    /* We have to have a minimum level of supply to be able to move. */
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_move(u2, m))
	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* This tests whether the relationship between the sides of a unit
   and a prospective transport allows for entry of the unit. */

int
sides_allow_entry(Unit *unit, Unit *transport)
{
    if (unit->side == indepside) {
    	if (transport->side == indepside) {
    	    return TRUE;
    	} else {
	    /* (should fix this - table does not apply to indeps entering?) */
    	    return uu_can_enter_indep(unit->type, transport->type);
    	}
    } else {
    	if (transport->side == indepside) {
    	    return uu_can_enter_indep(unit->type, transport->type);
    	} else {
	    /* Note that because this is for an explicit action, the unit
	       must trust the transport, so the only test is whether the
	       transports trusts the unit enough to have it as an occupant. */
    	    return unit_trusts_unit(transport, unit);
    	}
    }
}

/* This computes the total mp cost of entering a transport. */

int
total_entry_cost(int u1, int u3, int x1, int y1, int z1,
		 int u2, int x2, int y2, int z2)
{
    int cost = 0, ferryout, ferryin, t1, t2, b, dir, conncost, travcost, c;

    ferryout = over_nothing;
    ferryin = uu_ferry_on_enter(u2, u1);
    dir = closest_dir(x2 - x1, y2 - y1);
    if (u3 != NONUTYPE) {
	/* Charge for leaving the transport. */
    	cost += uu_mp_to_leave(u1, u3);
    	ferryout = uu_ferry_on_leave(u3, u1);
    }
    /* (should include possibility of using conn to cross terrain) */
    /* Maybe add cost to leave terrain of own cell. */
    if (ferryout < over_own && ferryin < over_all && (dir >= 0)) {
	t1 = terrain_at(x1, y1);
    	cost += ut_mp_to_leave(u1, t1);
    }
    /* Maybe add cost to cross one (or more) borders. */
    if (numbordtypes > 0 && ferryout < over_border && ferryin < over_border) {
	if (dir >= 0) {
	    for_all_border_types(b) {
		if (aux_terrain_defined(b)
		    && border_at(x1, y1, dir, b)) {
		    cost += ut_mp_to_enter(u1, b);
		}
	    }
	}
    }
    /* Maybe even have to pay cost of crossing destination's terrain. */
    if (ferryout < over_all && ferryin < over_own) {
	t2 = terrain_at(x2, y2);
    	cost += ut_mp_to_enter(u1, t2);
    }
    /* Use a connection traversal if it would be cheaper.  This is
       only automatic if the connection on/off costs are small enough,
       otherwise the unit has to do explicit actions to get on the
       connection and off again. */
    if (numconntypes > 0) {
	/* Try each connection type to see if it's better. */
	if (dir >= 0) {
	    for_all_connection_types(c) {
		if (aux_terrain_defined(c)
		    && connection_at(x1, y1, dir, c)
		    && ((travcost = ut_mp_to_traverse(u1, c)) >= 0)) {
		    conncost = ut_mp_to_enter(u1, c)
		      + travcost
		      + ut_mp_to_leave(u1, c);
		    cost = min(cost, conncost);
		}
	    }
	}
    }
    /* Add the actual cost of entry. */
    cost += uu_mp_to_enter(u1, u2);
    /* Movement must always cost at least 1 mp. */
    if (cost < 1)
      cost = 1;
    return cost;
}

/* This function now correctly calculates both the acp cost
and the available acps. However, it does not take into account
the fact that connections may make the terrain passable. Nor
does it take into account the fact acps may be spent in order
to leave the previous cell, or if there is room for the unit
type within the terrain.  */

int
terrain_always_impassable(int u, int t)
{
    int mpcost;

    if (ut_vanishes_on(u, t))
      return TRUE;
    if (ut_wrecks_on(u, t))
      return TRUE;
    /* Start with cost of entering a cell. */
    mpcost = ut_mp_to_enter(u, t);
    /* Check if we can have enough mps. */
    if (type_can_have_enough_mp(u, mpcost)) {
	return FALSE;
    }
    return TRUE;
}

/* This function calculates if a unit type can move within a
given terrain, i.e. from one cell to another of the same terrain
type. It does take into account the possibility that acps may be
spent on leaving a cell. It does not check if there is room for
the unit within the given terrain or if the unit can survive
on it. */

int
type_can_move_in_terrain(int u, int t)
{
    int mpcost;

    /* Start with cost of entering a cell. */
    mpcost = ut_mp_to_enter(u, t);
    /* Add in cost for leaving the old cell. */
    mpcost += ut_mp_to_leave(u, t);
    /* Check if we can have enough mps. */
    if (type_can_have_enough_mp(u, mpcost)) {
	return TRUE;
    }
    return FALSE;
}

int
type_can_have_enough_mp(int u, int mp)
{
    int acpcost;

    /* Substract the free mps. */
    mp -= u_free_mp(u);
    /* But don't go negative. */
    mp = max(0, mp);
    if (0 != type_max_speed(u)) {
        /* Convert mps into acps. */
        acpcost = (mp * 100) / type_max_speed(u);
        /* Each move costs at least u_acp_to_move ... */
        acpcost = max(acpcost, u_acp_to_move(u));
        /* ... and also at least 1 acp no matter what. */
        acpcost = max(acpcost, 1);
        /* Check if we can have enough acps. */
        if (type_can_have_enough_acp(u, acpcost)) {
            return TRUE;
        }
    }
    return FALSE;
}


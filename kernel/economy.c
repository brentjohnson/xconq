/* The backdrop economy code for XConq
   Copyright (C) 1986-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

static int excess_left(int x, int y);
static int need_left(int x, int y);
static void try_transfer_to_cell(int x, int y);
static void try_sharing_with_cell(int x, int y);
static void distribute_materials_1(void);
static void distribute_materials_0(void);
static void share_scarce_materials_0(void);
static void try_transfer(Unit *from, Unit *to, int m);
static void try_sharing_need(Unit *from, Unit *to, int m);
static void try_transfer_aux(Unit *from, Unit *to, int m);
static int can_satisfy_need(Unit *unit, int m, int need);
static void run_people_consumption(void);
static void run_cell_consumption(void);
static void run_unit_base_consumption(void);
static void run_unit_starvation(void);

/* Flags indicating whether various sets of calculations need to be done. */

/* Typically we set to -1 to indicate that the value is uncomputed, then detect
   when the value is first needed and compute a true/false value, which we then
   use thereafter. */

short any_unit_production = -1;

short any_terrain_production = -1;

short any_people_production = -1;

short any_people_consumption = -1;

short any_cell_consumption = -1;

short any_unit_base_consumption = -1;

short any_unit_starvation = -1;

short max_terrain_to_terrain_length = -1;

int tmpexcess;
int tmpneed;

/* We've "found what we were searching for" when the excess to distribute
   is gone. */

static int
excess_left(int x, int y)
{
    return (tmpexcess > 0);
}

static int
need_left(int x, int y)
{
    return (tmpneed > 0);
}

static void
try_transfer_to_cell(int x, int y)
{
    Unit *unit2, *occ;

    if (tmpexcess <= 0)
      return;
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {
	    try_transfer(tmpunit, unit2, tmpmtype);
	}
    }
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {

    	/* Bug fix. Units inside a transport inside a base or city were
    	previously unable to resupply directly from the base. Using 
    	for_all_occs_with_occs instead of for_all_occupants here 
    	enables such resupply. */
    	
	    for_all_occs_with_occs(unit2, occ) {
		if (in_play(occ) && occ->side == tmpunit->side) {
		    try_transfer(tmpunit, occ, tmpmtype);
		}
	    }
	}
    }
}

static void
try_sharing_with_cell(int x, int y)
{
    Unit *unit2, *occ;

    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit_trusts_unit(unit2, tmpunit)) {
	    try_sharing_need(unit2, tmpunit, tmpmtype);
	}
    }
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit_trusts_unit(unit2, tmpunit)) {
	    /* Bug fix (same as above). */
	    for_all_occs_with_occs(unit2, occ) {
		if (in_play(occ) && unit_trusts_unit(occ, tmpunit)) {
		    try_sharing_need(occ, tmpunit, tmpmtype);
		}
	    }
	}
    }
}

/* The main routine does production, distribution, and discarding in order. */

static int *ttotals;
static int *utotals;

void
run_economy(void)
{
    int u, m, t, t2 = NONTTYPE, amt, x, y, x1, y1, m1, m2;
    int prod, ptivity, dir, ptivityadj, stor, oldamt, newamt;
    Unit *unit;
    
    if (nummtypes == 0)
      return;
    if (any_unit_production < 0) {
	any_unit_production = FALSE;
	for_all_unit_types(u) {
	    for_all_material_types(m) {
		if (max(um_base_production(u, m),
			um_occ_production(u, m)) > 0) {
		    any_unit_production = TRUE;
		    break;
		}
	    }
	}
	Dprintf("Any unit production: %d\n", any_unit_production);
    }
    if (any_terrain_production < 0) {
	any_terrain_production = FALSE;
	for_all_terrain_types(t) {
	    for_all_material_types(m) {
		if (tm_production(t, m) > 0) {
		    any_terrain_production = TRUE;
		    break;
		}
	    }
	}
	Dprintf("Any terrain production: %d\n", any_terrain_production);
    }
    if (any_people_production < 0) {
	any_people_production = FALSE;
	for_all_material_types(m1) {
	    if (m_people(m1) > 0) {
		for_all_material_types(m2) {
		    if (mm_people_production(m1, m2) > 0) {
			any_people_production = TRUE;
			break;
		    }
		}
	    }
	}
	Dprintf("Any people production: %d\n", any_people_production);
    }
    /* Note that the flags above only test for subphases; we have to run
       this overall, because redistribution may happen even if nothing
       is being produced or consumed automatically. */
    Dprintf("Running economy\n");
    if (Debug) {
	ttotals = (int *) xmalloc(nummtypes * sizeof(int));
	utotals = (int *) xmalloc(nummtypes * sizeof(int));
	for_all_material_types(m)
	  ttotals[m] = utotals[m] = 0;
    }
    /* Make new materials but don't clip to storage capacities yet. */
    if ((any_terrain_production || any_people_production)
	&& any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		for_all_cells(x, y) {
		    if (any_terrain_production) {
			t = terrain_at(x, y);
			prod = tm_production(t, m);
			for_all_aux_terrain_types(t2) {
			    if (aux_terrain_defined(t2)
				&& aux_terrain_at(x, y, t2)) {
				prod += tm_production(t2, m);
			    }
			}
			if (prod > 0) {
			    oldamt = material_at(x, y, m);
			    newamt = min(oldamt + prod, TABHI);
			    set_material_at(x, y, m, newamt);
			    if (Debug) {
				stor = tm_storage_x(t, m);
				if (newamt > stor)
				  ttotals[m] += (newamt - stor);
			    }
			}
		    }
		    if (any_people_production) {
			for_all_material_types(m1) {
			    if (m_people(m1) > 0) {
				prod = mm_people_production(m1, m);
				if (prod > 0) {
				    oldamt = material_at(x, y, m);
				    newamt = min(oldamt + prod, TABHI);
				    set_material_at(x, y, m, newamt);
				    if (Debug) {
					stor = tm_storage_x(t, m);
					if (newamt > stor)
					  ttotals[m] += (newamt - stor);
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
    if (any_unit_production) {
	for_all_units(unit) {
	    if (in_play(unit) && completed(unit)) {
		u = unit->type;
		for_all_material_types(m) {
		    t = terrain_at(unit->x, unit->y);
		    prod = base_production(unit, m);
		    if (prod > 0) {
			ptivity = ut_productivity(u, t);
			/* Note that we've switched to hundredths. */
			ptivity = max(ptivity, um_productivity_min(u, m));
			ptivity = min(ptivity, um_productivity_max(u, m));
			amt = prob_fraction(prod * ptivity);
			if (1 /* any adj productivity */) {
			    ptivityadj = 0;
			    for_all_directions(dir) {
				if (point_in_dir(unit->x, unit->y, dir, 
                                    &x1, &y1))
				  ptivityadj = max(ptivityadj, 
                                                   ut_productivity_adj(u, 
                                                        terrain_at(x1, y1)));
			    }
			    ptivityadj = max(ptivityadj, 
                                             um_productivity_min(u, m));
			    ptivityadj = min(ptivityadj, 
                                             um_productivity_max(u, m));
			    amt += prob_fraction(prod * ptivityadj);
			}
			unit->supply[m] += min(amt, TABHI - unit->supply[m]);
			if (Debug && unit->supply[m] > um_storage_x(u, m))
			  utotals[m] += (unit->supply[m] - um_storage_x(u, m));
		    }
		}
	    }
	}
    }
    Dprintf("Overflow is:");
    Dprintf("  (for terrain)");
    for_all_material_types(m)
      Dprintf(" %d", ttotals[m]);
    Dprintf("  (for units)");
    for_all_material_types(m)
      Dprintf(" %d", utotals[m]);
    Dprintf("\n");
    switch (g_backdrop_model()) {
        case 1:
            distribute_materials_1();
            break;
        case 0:
        default:
            distribute_materials_0();
            break;
    }
    /* Throw away excess that can't be stored anywhere. */
    if (Debug) {
	for_all_material_types(m)
	  ttotals[m] = utotals[m] = 0;
    }
    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		for_all_cells(x, y) {
		    t = terrain_at(x, y);
		    stor = tm_storage_x(t, m);
		    oldamt = material_at(x, y, m);
		    newamt = min(oldamt, stor);
		    set_material_at(x, y, m, newamt);
		    if (Debug && newamt < oldamt)
		      ttotals[m] += (oldamt - newamt);
		}
	    }
	}
    }
    for_all_units(unit) {
	u = unit->type;
	for_all_material_types(m) {
	    stor = um_storage_x(u, m);
	    oldamt = unit->supply[m];
	    newamt = min(oldamt, stor);
	    newamt = max(newamt, 0);
	    unit->supply[m] = newamt;
	    if (Debug && newamt < oldamt)
	      utotals[m] += (oldamt - newamt);
	}
    }
    Dprintf("Discarded ");
    Dprintf("  (from terrain)");
    for_all_material_types(m)
      Dprintf(" %d", ttotals[m]);
    Dprintf("  (from units)");
    for_all_material_types(m)
      Dprintf(" %d", utotals[m]);
    Dprintf("\n");

    if (g_backdrop_model()==0)
        share_scarce_materials_0();
    /* Finally, reset supply alarms. */
    for_all_units(unit) {
	if (in_play(unit) && unit->plan != NULL) {
	    if (unit->plan->supply_is_low
		&& !past_halfway_point(unit)) {
		unit->plan->supply_alarm = TRUE;
		unit->plan->supply_is_low = FALSE;
		update_unit_display(unit->side, unit, TRUE); 
	    }
	}
    }
}

typedef enum {
    mh_unit,
    mh_treasury,
    mh_cell,
} MaterialHandlerType;

typedef struct a_materialhandler {
    MaterialHandlerType mh_type;
    long *store, cap, doclevel;
    Unit *u;
    Side *s;
    int x, y;
} MaterialHandler;

static int can_transfer_material(int m, MaterialHandler *mh_from,
				 MaterialHandler *mh_to);
static int material_gap(MaterialHandler *mh_from, MaterialHandler *mh_to);

static int make_mh_from_unit(MaterialHandler *mh, int m, Unit *u);
static int make_mh_from_treasury(MaterialHandler *mh, int m, Side *s);
static int make_mh_from_cell(MaterialHandler *mh, int m, int x, int y);

/* macro used several times in finding maximum-gap unit; not a function
 * because it needs to touch a bunch of local variables */
#define CHECK_MAXGAP \
    if (can_transfer_material(m, mh_from+from_ptr, &mh_tmp)) { \
        gap = material_gap(mh_from+from_ptr, &mh_tmp); \
        if (gap > maxgap) tiesize = 0; \
        if ((gap >= maxgap) && (xrandom(++tiesize) == 0)) { \
	    maxgap = gap; \
	    mh_to = mh_tmp; \
	} \
    }

static void
distribute_materials_1(void)
{
    int m;
    int i, j, x, y;
    Unit *u;
    Side *s;
    MaterialHandler *mh_from=NULL;
    MaterialHandler mh_tmp, mh_to;
    int from_count, from_allocated=0, from_ptr;
    int gap, maxgap, tiesize;
    int range, t;
   
    /* figure out whether we ever do cell-to-cell transfers, and how far */
    if (max_terrain_to_terrain_length < 0) {
        int max_in_length=0, max_out_length=0, max_length, max_mod, t;
       
        for_all_material_types(m) {
	    max_length = 0;
	    max_mod = 0;
	    for_all_terrain_types(t) {
	        if (t_is_cell(t))
		    max_length = max(max_length, tm_tt_in_length(t, m));
	        else
		    max_mod += max(0, tm_tt_in_length(t, m));
	    }
	    max_in_length = max(max_in_length, max_length + max_mod);

	    max_length = 0;
	    max_mod = 0;
	    for_all_terrain_types(t) {
	        if (t_is_cell(t))
		    max_length = max(max_length, tm_tt_out_length(t, m));
	        else
		    max_mod += max(0, tm_tt_out_length(t, m));
	    }
	    max_out_length = max(max_out_length, max_length + max_mod);
	}

        max_terrain_to_terrain_length = max(0,
	    min(max_in_length, max_out_length));
    }
   
    for_all_material_types(m) {
        /* figure out how many handlers there are */
        from_count = 0;
        for_all_sides(s) {
	    if (make_mh_from_treasury(&mh_tmp, m, s))
	        from_count++;
	}
        for_all_cells(x, y) {
	    if (make_mh_from_cell(&mh_tmp, m, x, y))
	        from_count++;
	}
        for_all_units(u) {
	    if (make_mh_from_unit(&mh_tmp, m, u))
	        from_count++;
	}
        if (from_count <= 0) continue;

        /* allocate space for them */
        if (from_allocated<from_count) {
	    if (mh_from!=NULL) free(mh_from);
	    mh_from = (MaterialHandler *)xmalloc(sizeof(MaterialHandler)
						 *from_count);
	    from_allocated = from_count;
	}
       
        /* make the initial list */
        from_ptr = 0;
        for_all_sides(s) {
	    if (make_mh_from_treasury(mh_from+from_ptr, m, s))
	        from_ptr++;
	}
        for_all_cells(x, y) {
	    if (make_mh_from_cell(mh_from+from_ptr, m, x, y))
	        from_ptr++;
	}
        for_all_units(u) {
	    if (make_mh_from_unit(mh_from+from_ptr, m, u))
	        from_ptr++;
	}

        /* shuffle it */
	for (i = 0; i < from_ptr-1; i++) {
	    j = i+xrandom(from_ptr-i);
	    if (j != i) {
	        mh_tmp = mh_from[i];
	        mh_from[i] = mh_from[j];
	        mh_from[j] = mh_tmp;
	    }
	}
       
        /* for each from handler */
        for (from_ptr = 0; from_ptr < from_count; from_ptr++) {
	    while (*(mh_from[from_ptr].store) > mh_from[from_ptr].doclevel) {
	        /* find possible recipients */
	        maxgap = 0;
	        tiesize = 0;
	        for_all_sides(s) {
		    if (make_mh_from_treasury(&mh_tmp, m, s))
		        CHECK_MAXGAP;
		}

	        switch (mh_from[from_ptr].mh_type) {
		    case mh_unit:
		        range = um_ut_out_length(
			    mh_from[from_ptr].u->type, m);
		        break;

		    case mh_cell:
		        range = tm_tt_out_length(
			    terrain_at(mh_from[from_ptr].x,
				mh_from[from_ptr].y), m);
		        for_all_aux_terrain_types(t) {
			    if (aux_terrain_defined(t)
				&& aux_terrain_at(mh_from[from_ptr].x,
				mh_from[from_ptr].y, t))
				range += max(tm_tt_out_length(t, m), 0);
			}
		        range = min(range, max_terrain_to_terrain_length);
		        if (range == 0)
		            range = -1;
		        break;

		    default:
		        /* Only units and cells can transfer to cells. */
		        range = -1;
		        break;
		}

	        for_all_cells_within_range(mh_from[from_ptr].x,
		    mh_from[from_ptr].y, range, x, y) {
		    if (make_mh_from_cell(&mh_tmp, m, x, y))
		        CHECK_MAXGAP;
		}

	        for_all_units(u) {
		    if (make_mh_from_unit(&mh_tmp, m, u))
		        CHECK_MAXGAP;
	        }
	       
	        /* stop if there is no gap left */
	        if (maxgap <= 0) break;
	       
	        /* transfer a quarter of the gap rounding up */
	        maxgap = min((maxgap + 3) / 4,
		    *(mh_from[from_ptr].store) - mh_from[from_ptr].doclevel);
	        *(mh_to.store) += maxgap;
	        *(mh_from[from_ptr].store) -= maxgap;
	    }
	}
    }
   
   /* free dynamic memory */
   free(mh_from);
}

/* return true if mh_from is allowed to transfer material m to mh_to;
 * should take into account distance, sides, etc. */
static int
can_transfer_material(int m, MaterialHandler *mh_from, MaterialHandler *mh_to)
{
    int in_length, out_length, t, dir;

    switch (mh_from->mh_type) {
        case mh_unit:
            switch (mh_to->mh_type) {
	        case mh_unit: /* unit->unit */
	            return unit_trusts_unit(mh_from->u, mh_to->u)
		        && cell_is_within_range(mh_from->x, mh_from->y,
			    um_outlength(mh_from->u->type, m),
			    mh_to->x, mh_to->y)
			&& cell_is_within_range(mh_to->x, mh_to->y,
			    um_inlength(mh_to->u->type, m),
			    mh_from->x, mh_from->y);

	        case mh_treasury: /* unit->treasury */
	            return (mh_from->u->side == mh_to->s)
		        && um_gives_to_treasury(mh_from->u->type, m);

	        case mh_cell: /* unit->cell */
	            return cell_is_within_range(mh_from->x, mh_from->y,
			tm_ut_in_length(terrain_at(mh_to->x, mh_to->y), m),
			mh_to->x, mh_to->y);

	        default:
	            return FALSE;
	    }

        case mh_treasury: /* treasury->unit, can't go anywhere else */
            return (mh_to->mh_type == mh_unit)
	        && (mh_from->s == mh_to->u->side)
	        && um_takes_from_treasury(mh_to->u->type, m);

        case mh_cell:
            switch (mh_to->mh_type) {
	        case mh_unit: /* cell->unit */
	            return cell_is_within_range(mh_from->x, mh_from->y,
			    um_tu_in_length(mh_to->u->type, m),
			    mh_to->x, mh_to->y)
		        && cell_is_within_range(mh_to->x, mh_to->y,
			    tm_tu_out_length(
				terrain_at(mh_from->x, mh_from->y), m),
			    mh_from->x, mh_from->y);

	        case mh_cell: /* cell->cell */
	            in_length = tm_tt_in_length(terrain_at(mh_to->x,
			mh_to->y), m);
	            out_length = tm_tt_out_length(terrain_at(mh_from->x,
			mh_from->y), m);
	            dir = approx_dir(mh_to->x - mh_from->x,
				     mh_to->y - mh_from->y);
	            for_all_aux_terrain_types(t) {
		        if (aux_terrain_defined(t)) {
			    if (t_is_coating(t)) {
				if (aux_terrain_at(mh_from->x, mh_from->y, t))
				    out_length += tm_tt_out_length(t, m);
				if (aux_terrain_at(mh_to->x, mh_to->y, t))
				    in_length += tm_tt_in_length(t, m);
			    } else {
			        /* we abuse the fact that border_at and
				    connection_at are defined identically */
			        if (border_at(mh_from->x, mh_from->y, dir, t))
				    out_length += tm_tt_out_length(t, m);
			        if (border_at(mh_to->x, mh_to->y,
					      opposite_dir(dir), t))
				    in_length += tm_tt_in_length(t, m);
			    }
			}
		    }
	            return cell_is_within_range(mh_from->x, mh_from->y,
			min(in_length, out_length), mh_to->x, mh_to->y);

	        default:
	            return FALSE;
	    }

        default:
            return FALSE;
    }
}

/* return the "gap", that is, how much material is available at the FROM mh
 * over what the TO mh already has.  Note that if the FROM mh is oversupplied,
 * then it'll try to transfer material even to units that already have more
 * than it does, if they have spare capacity. */
static int
material_gap(MaterialHandler *mh_from, MaterialHandler *mh_to)
{
    return min(mh_to->cap-*(mh_to->store),
	       max(*(mh_from->store)-*(mh_to->store),
		   *(mh_from->store)-mh_from->cap));
}

static int
make_mh_from_unit(MaterialHandler *mh, int m, Unit *u)
{
    if ((u==NULL) || (u->supply==NULL) || !is_active(u))
        return FALSE;
    if ((u->supply[m]<=0) && (um_storage_x(u->type, m)<=0))
        return FALSE;
    mh->mh_type = mh_unit;
    mh->store = u->supply+m;
    mh->cap = um_storage_x(u->type, m);
    /* Should this sometimes use rearm doctrine instead of resupply?  The
     * other code only seems to ever use resupply. */
    if (g_backdrop_ignore_doctrine())
        mh->doclevel = 0;
    else
        mh->doclevel = (unit_doctrine(u)->resupply_percent * mh->cap) / 100;
    mh->u = u;
    mh->x = u->x;
    mh->y = u->y;
    return TRUE;
}

static int
make_mh_from_treasury(MaterialHandler *mh, int m, Side *s)
{
    if ((s==NULL) || !(side_has_treasury(s,m)))
        return FALSE;
    mh->mh_type = mh_treasury;
    mh->store = s->treasury+m;
    mh->cap = g_treasury_size();
    mh->doclevel = 0;
    mh->s = s;
    return TRUE;
}

static int make_mh_from_cell(MaterialHandler *mh, int m, int x, int y)
{
    int cap;
   
    if (!in_area(x, y))
        return FALSE;
    cap = tm_storage_x(terrain_at(x, y), m);
    if (cap <= 0)
        return FALSE;

    allocate_area_material(m); /* FIXME do this in caller */
    mh->mh_type = mh_cell;
    mh->store = area.materials[m] + (area.width*y + wrapx(x));
    mh->cap = cap;
    mh->doclevel = 0;
    mh->x = x;
    mh->y = y;
    return TRUE;
}

static void
distribute_materials_0(void)
{
    int u, m, dist, x1, y1;
    int stor;
    Unit *unit;
    
    /* Move stuff around - try to get rid of any excess. */
    /* (should also do cell-cell, cell-unit, unit-cell xfers) */
    for_all_units(unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		stor = um_storage_x(u, m);
		tmpexcess = unit->supply[m] - stor;
		if (tmpexcess > 0) {
		    dist = um_outlength(u, m);
		    if (dist >= 0) {
		    	tmpunit = unit;
		    	tmpmtype = m;
			search_and_apply(unit->x, unit->y, dist, excess_left,
					 &x1, &y1, 1, try_transfer_to_cell, 0);
		    }
		}
                /* Try to stock side treasury with any remaining excess. */
                if ((tmpexcess > 0) 
                    && produces_for_treasury_directly(unit, m)) {
                    /* (Should check to make sure that we are not 
                        overflowing treasury before dumping to it.) */
                    unit->side->treasury[m] += tmpexcess;
                    unit->supply[m] -= tmpexcess;
                    tmpexcess = 0;
                }
	    }
	}
    }
}

static void
share_scarce_materials_0(void)
{
    int u, m, dist, x1, y1;
    Unit *unit;
    
    /* This next phase is for sharing of scarcer supplies.  We scan
       for units that are in need of supplies, then search for possible
       suppliers.  This phase does not check the out-length of the supplier -
       is that a bug or a feature?  */
    for_all_units(unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		/* (should use doctrine to set level) */
		/* Since do_resupply _task continues until we are full,
		we have to fill up the unit. */
/*		if (2 * unit->supply[m] < um_storage_x(u, m)) { */
		if (unit->supply[m] < um_storage_x(u, m)) {
		    dist = um_inlength(u, m);
		    if (dist >= 0) {
			tmpunit = unit;
			tmpmtype = m;
			/* Round up, to handle fractions usefully. */
/*			tmpneed = (um_storage_x(u, m) + 1) / 2 - unit->supply[m]; */
			/* Since do_resupply _task continues until we are full,
			we have to fill up the unit. */
			tmpneed = um_storage_x(u, m)  - unit->supply[m];
			search_and_apply(unit->x, unit->y, dist, need_left,
					 &x1, &y1, 1, try_sharing_with_cell,
					 0);
		    }
		}
	    }
	}
    }
}

/* Give away supplies, but save enough to stay alive for a couple turns. */

static void
try_transfer(Unit *from, Unit *to, int m)
{
    int oldsupply = from->supply[m];

    try_transfer_aux(from, to, m);
    tmpexcess -= (oldsupply - from->supply[m]);
}

void
try_sharing_need(Unit *from, Unit *to, int m)
{
    int oldsupply = to->supply[m];

    if ((-1 == um_outlength(from->type, m)) && (from != to->transport))
      return;
    try_transfer_aux(from, to, m);
    /* Decrement our need by whatever we picked up in this transfer. */
    tmpneed -= (to->supply[m] - oldsupply);
}

/* Note that this may be called on newly-completed units during a turn. */

void
try_sharing(Unit *from, Unit *to, int m)
{
    try_transfer_aux(from, to, m);
}

/* Material redistribution uses this routine to move supplies around
   between units far apart or on the same cell. Try to do reasonable
   things with the materials.  Net producers are much more willing to
   give away supplies than net consumers. */

static void
try_transfer_aux(Unit *from, Unit *to, int m)
{
    int nd, u = from->type, u2 = to->type, fromrate, torate;

    /* Don't waste fuel on units that will not move. */
    if (will_not_move(to)
        && !needs_material_to_survive(to, m)) {
     	return;   
    }
    /* Extra safeguard. 
	A disabled output should have been caught by callers.
    */
    if ((-1 == um_outlength(u, m)) && (from != to->transport))
      return;
    if (from != to &&
	um_inlength(u2, m) >= distance(from->x, from->y, to->x, to->y)) {
	if (completed(to)) {
	    /* Try for the transfer only if we're below capacity. */
	    nd = um_storage_x(u2, m) - to->supply[m];
	    if (nd  > 0) {
		if ((um_base_production(u, m) > um_base_consumption(u, m))
		    || (survival_time(to) < 3)
		    || (will_not_move(from)
		    	&& !needs_material_to_survive(from, m))
		    || (um_storage_x(u, m) >= um_storage_x(u2, m) / 4)) {
		    if (can_satisfy_need(from, m, nd)) {
			transfer_supply(from, to, m, nd);
		    } else if (can_satisfy_need(from, m, max(1, nd/2))) {
			transfer_supply(from, to, m, max(1, nd/2));
		    } else if (from->supply[m] > um_storage_x(u, m)) {
			/* This is not supposed to happen ... */
#if 0			/* (but it does as long as run_economy works the way it does) */
			run_warning("%s has more %s than it should be able to store.", 
				short_unit_handle(from), m_type_name(m));
#endif
			/* .. but transfer excess anyway. */
			transfer_supply(from, to, m,
					(from->supply[m]
					 - um_storage_x(u, m)));
		    /* If from doesn't need m itself, give it all away. */
		    } else if (will_not_move(from)
		        && !needs_material_to_survive(from, m)) {
			transfer_supply(from, to, m, max(from->supply[m], nd));
		    }
		} else {
		    fromrate = type_max_speed(u) * 
			       um_consumption_per_move(u, m) * 3;
		    fromrate = max(1, fromrate);
		    torate = type_max_speed(u2) * 
			     um_consumption_per_move(u2, m) * 3;
		    torate = max(1, torate);
		    if ((from->supply[m] / fromrate)
			> (to->supply[m] / torate)) {
			transfer_supply(from, to, m,
						min(nd, 
						(um_storage_x(u, m) - 8 > from->supply[m] ?
						(8 + from->supply[m]) : um_storage_x(u, m)) / 9));
		    }
		}
	    }
	} else {
	    /* Incomplete units don't need supply, but they are a
	       handy overflow repository. */
	    if (from->supply[m] > um_storage_x(u, m)
		&& to->supply[m] < um_storage_x(u2, m)) {
		/* This is not supposed to happen ... */
#if 0		/* (but it does as long as run_economy works the way it does) */
		run_warning("%s has more %s than it should be able to store.", 
			short_unit_handle(from), m_type_name(m));
#endif
		/* Calculate the limit on how much we can transfer usefully. */
		nd = min(um_storage_x(u2, m) - to->supply[m],
			 from->supply[m] - um_storage_x(u, m));
		/* .. but transfer excess anyway. */
		transfer_supply(from, to, m, nd);
	    }
	}
    }
}

/* This estimates if a need can be met.  Note that total transfer of
   requested amount is not a good idea, since the supplies might be
   essential to the unit that has them first.  If we're not worried
   about resupply, or the request is very small, then we can spare it. */

/* (should replace with doctrine/plan controls, check underlying terrain) */

static int
can_satisfy_need(Unit *unit, int m, int need)
{
    int supp = unit->supply[m];
    int stor = um_storage_x(unit->type, m);
    /* (should cache these) */
#if 0
    int lim = (unit_doctrine(unit)->resupply_percent * stor) / 100;

    if (supp > lim)
      return (need < ((supp - lim) * 7) / 8);
    else
      return (need < lim / 8);
#endif
    /* The above did not work well in the case of airplanes rearming
    from a carrier. Try to be more generous by sharing half of the
    remaining supply, even if this means that we eventually have to 
    go back to port and resupply the donor. This policy is also the
    best way of leveling out supplies between a larger number of 
    units.
    Note: testing for resupply_percent is also the wrong thing to do 
    in case we are rearming. */
    return (need < supp / 2);

}

void
run_all_consumption(void)
{
    run_people_consumption();
    run_cell_consumption();
    run_unit_base_consumption();
    run_unit_starvation();
}

/* Handle consumption by people. */

static void
run_people_consumption(void)
{
    int mm1, mm2, x, y, m1, m2, t, consum, oldamt, newamt, newtype;

    /* Precompute whether any people consumption ever happens. */
    if (any_people_consumption < 0) {
	any_people_consumption = FALSE;
	for_all_material_types(mm1) {
	    for_all_material_types(mm2) {
	    	if (mm_people_consumption(mm1, mm2) > 0) {
		    any_people_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_people_consumption)
	      break;
	}
	Dprintf("Any consumption by people: %d\n", any_people_consumption);
    }
    if (!any_people_consumption)
      return;
    if (!any_cell_materials_defined())
      return;
    Dprintf("Running people consumption\n");
    for_all_material_types(m1) {
	if (cell_material_defined(m1)) {
	    for_all_material_types(m2) {
		if (cell_material_defined(m2)) {
		    consum = mm_people_consumption(m1, m2);
		    if (consum > 0) {
			for_all_cells(x, y) {
			    oldamt = material_at(x, y, m2);
			    newamt = oldamt - consum;
			    if (newamt < 0) {
				newamt = 0;
				/* Check for exhaustion. */
				/* (should share with cell consumption) */
				t = terrain_at(x, y);
				if (probability(tm_change_on_exhaust(t, m2)) &&
				    tm_exhaust_type(t, m2) != NONTTYPE) {
				    newtype = tm_exhaust_type(t, m2);
				    /* Change the terrain's type. */
				    change_terrain_type(x, y, newtype);
				}
			    }
			    set_material_at(x, y, m2, newamt);
			}
		    }
		}
	    }
	}
    }
}

/* Handle consumption by terrain. */

static void
run_cell_consumption()
{
    int x, y, t, m, t2, m2, consum, oldamt, newamt, willchange, newtype;

    /* Precompute whether any cell base consumption ever happens. */
    if (any_cell_consumption < 0) {
	any_cell_consumption = FALSE;
	for_all_terrain_types(t2) {
	    for_all_material_types(m2) {
	    	if (tm_consumption(t2, m2) > 0) {
		    any_cell_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_cell_consumption)
	      break;
	}
	Dprintf("Any consumption by cells: %d\n", any_cell_consumption);
    }
    if (!any_cell_consumption)
      return;
    if (!any_cell_materials_defined())
      return;
    Dprintf("Running cell consumption\n");
    for_all_cells(x, y) {
	t = terrain_at(x, y);
	willchange = FALSE;
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		consum = tm_consumption(t, m);
		oldamt = material_at(x, y, m);
		newamt = oldamt - consum;
		if (newamt < 0) {
		    newamt = 0;
		    /* Check for exhaustion. */
		    if (!willchange &&
			probability(tm_change_on_exhaust(t, m)) &&
			tm_exhaust_type(t, m) != NONTTYPE) {
			willchange = TRUE;
			newtype = tm_exhaust_type(t, m);
		    }
		}
		set_material_at(x, y, m, newamt);
	    }
	}
	if (willchange) {
	    /* Change the terrain's type. */
	    change_terrain_type(x, y, newtype);
	}
    }
}

/* Handle base consumption by units. */

static void
run_unit_base_consumption(void)
{
    int u, m, usedup, consump, checkstarve;
    Unit *unit;

    /* Precompute whether any base consumption ever happens. */
    if (any_unit_base_consumption < 0) {
	any_unit_base_consumption = FALSE;
	for_all_unit_types(u) {
	    for_all_material_types(m) {
	    	if (um_base_consumption(u, m) > 0) {
		    any_unit_base_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_unit_base_consumption)
	      break;
	}
	Dprintf("Any unit base consumption: %d\n", any_unit_base_consumption);
    }
    if (!any_unit_base_consumption)
      return;
    Dprintf("Running unit base consumption\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    u = unit->type;
	    checkstarve = FALSE;
	    for_all_material_types(m) {
		if (um_base_consumption(u, m) > 0
		    && !(unit->transport != NULL
			 && um_consumption_as_occupant(u, m) == 0)) {
		    /* Calculate what was already consumed by movement. */
		    usedup = 0;
		    if (unit->act != NULL)
		      usedup = unit->act->actualmoves * 
                               um_consumption_per_move(u, m);
		    consump = um_base_consumption(u, m);
		    /* If being transported, modify the base consumption. */
		    if (unit->transport != NULL)
		      consump = (consump * um_consumption_as_occupant(u, m)) 
                                / 100;
#if 0               /* (should think about this code a bit more) */
		    /* Modify consumption if temperature effects. */
		    effect = u_consume_temp_effect(u);
		    if (temperatures_defined() && effect != lispnil) {
			tempeff = interpolate_in_list(temperature_at(unit->x, 
                                                                     unit->y), 
                                                      effect, FALSE, 999);
			consump = (consump * tempeff) / 100;
		    }
#endif
		    /* Subtract consumption that was not already used up in 
                       movement. */
		    if (usedup < consump) {
                        if (consumes_from_treasury_directly(unit, m)
                            && (unit_side_treasury(unit)[m] >= 
                                (consump - usedup)))
                            unit_side_treasury(unit)[m] -= (consump - usedup);
                        else
		            unit->supply[m] -= (consump - usedup);
                    }
		    if (unit->supply[m] <= 0)
		      checkstarve = TRUE;
		}
	    } /* for_all_material_types */
	    /* Consider triggering the low-supply flag. */
	    if (alive(unit)
		&& unit->plan
		&& !unit->plan->supply_is_low
		&& past_halfway_point(unit)
		) {
		unit->plan->supply_is_low = TRUE;
		update_unit_display(unit->side, unit, TRUE); 
	    }
	}
    }
}

/* Starvation is a separate computation that follows consumption, because it
   may be that units lose necessary material in other ways than consumption,
   and the loss may be slow enough that this has to run in each of several
   turns, and not just once at the point of supply exhaustion. */

static void
run_unit_starvation(void)
{
    int u, m;
    Unit *unit;

    /* Precompute whether any starvation ever happens. */
    if (any_unit_starvation < 0) {
	any_unit_starvation = FALSE;
	for_all_unit_types(u) {
	    for_all_material_types(m) {
	    	if (um_hp_per_starve(u, m) > 0) {
		    any_unit_starvation = TRUE;
		    break;
	    	}
	    }
	    if (any_unit_starvation)
	      break;
	}
	Dprintf("Any unit starvation: %d\n", any_unit_starvation);
    }
    if (!any_unit_starvation)
      return;
    Dprintf("Running unit starvation\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    maybe_starve(unit, TRUE);
	}
    }
}

/* What happens to a unit that runs out of supplies.  If it can survive
   on nothing, then there may be a few turns of grace, depending on
   how the dice roll... */

void
maybe_starve(Unit *unit, int partial)
{
    int u = unit->type, m, starv, oneloss, hploss = 0;

    for_all_material_types(m) {
	if (unit->supply[m] <= 0) {
	    starv = um_hp_per_starve(u, m);
	    if (starv > 0) {
	        /* FIXME: this non-determinism can cause the unit to suffer
		 * the chance of dying more times than it deserves,
		 * if maybe_starve is called during a turn. */
		oneloss = prob_fraction(starv);
		hploss = max(hploss, oneloss);
	    }
	}
    }
    if (hploss > 0) {
	if ((hploss >= unit->hp) || partial) {
	    /* (FIXME: Should notify the player somehow what happened.
              Since the unit doesn't exist after this, they can't even
              see which material it was low on.) */
            /* (Is the above comment still applicable now that the general-
	       purpose damage code is used?)  */
	    unit->hp2 -= hploss;
	    damage_unit(unit, starvation_dmg, NULL);
	}
    }
}

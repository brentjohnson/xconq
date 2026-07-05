/* supply.c 1.0 Sep-7-1998 - implementation of the (new) supply system.
 * 
 * Copyright (C) 1998, 1999 Sami Perttu
 * Send bug reports to perttu@cc.helsinki.fi or the Xconq mailing list.
 * 
 * Xconq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.  See the file COPYING.
 * 
 * Module interface:
 * 
 * run_supply();
 * 	Run the supply algorithm for all sides. Called right after
 *	run_economy() from run2.c.
 * run_supply_side(side);
 * 	Run the supply algorithm for a specified side. Should be used in
 * 	sequential games to resolve supply at the beginning of each player's
 * 	turn.
 * boolean = supply_system_in_use();
 * 	Can be called by interfaces to find out whether the supply system is
 * 	used for a game or not. If not, then "supply flow" and "supply
 * 	connectedness" shouldn't be displayed for units.
 * compute_supply_zones(side, material);
 * 	Compute supply potentials for a specified material. On return, the
 * 	layer tmp1 will contain the potentials. This function is meant to be
 *	called by interfaces.
 * compute_unit_supply_zone(side, unit, material);
 * 	Same as above, but compute only the supply zone resulting from the
 *	given unit's supply-potential property.
 * 
 * A sixth function might be added later:
 * 
 * run_indep_supply();
 * 	Run supply for the independent side. Independent supply sources could
 *	provide supply to non-independent units as well, depending on some
 *	as-yet-unimplemented GDL tables.
 * 
 * Everything else is declared static. No initialization calls need to be
 * made: the supply system takes care of that automatically.
 * 
 * Some terrain optimizations (for_all_border_types, for_all_connection_types)
 * should find their way to init.c and world.h.
 * 
 * One supply calculation pass for a side consists of several phases:
 * 
 * 1. Initialize temporary layers.
 * 2. Find all units with a positive supply-potential property.
 * 3. Expand supply lines in descending order of supply zone potential.
 * 4. Establish supply zones with a flood-fill algorithm, calculating net
 *    material balance in each zone.
 * 5. For each zone: distribute supplies, ignoring any remnants. Then sort
 *    units and distribute the remainder. Ties are broken randomly.
 * 
 * One equivalence class of materials is processed in one supply calculation
 * pass. An equivalence class consists of all material types whose parameters
 * concerning the establishment of supply zones are identical.
 * 
 * Read the docs (?) for more info.
 */

#include "conq.h"

#define INITIAL_WORKSPACE	1024
#define MAX_FLOW		16384
#define MAX_CONN		25600

/* Accessor macro */
#define um_clip_in_threshold(u, m) umclipinthreshold[nummtypes * (u) + (m)]

typedef struct heap_node_struct {
    int pot;		/* supply potential */
    short x, y;		/* cell coordinates */
} heap_node;

typedef struct zone_node_struct {
    short re;		/* zone potential, material remainder or supply weight;
			 * used as a sort key in the latter two cases */
    short uw;		/* the worth of one unit of material in s_flow */
    Unit *unit;		/* the unit */
} zone_node;

typedef struct material_stats_struct {
    int mtype;		/* material type */
    long supply;	/* (net) supply */
    long demand;	/* (net) demand */
    long weight;	/* supply weight */
    long wdemand;	/* weighted (net) demand */
    struct material_stats_struct *next;		/* next material in this class */
    struct material_stats_struct *next_class;	/* next equivalence class */
} material_stats;

/* Copying the whole structure (~8 bytes) is a bit wasteful, but it's way
 * simpler than working with a pointer-based heap. */
#define heap_node_copy(dst, src) \
{ (dst)->pot = (src)->pot; (dst)->x = (src)->x; (dst)->y = (src)->y; }

/* Static protos. */
static void init_supply_system(void);
static void init_tmp_layers(void);
static void process_supply_class(Side *side, material_stats *mclass);
static void add_unit_heap(Unit *unit, int m);
static void run_supply_lines(Side *side, int snum, int m);
static void establish_supply_zone(Side *side, int x, int y, material_stats *mclass);
static void process_unit_supply(Unit *unit, material_stats *mclass, int pot);
static int calculate_supply_demand(Unit *unit, int m, material_stats *res, int pot, int rec_stats);
static void heap_init(void);
static void heap_insert(int pot, short x, short y);
static int heap_deque(heap_node *hn);
static void zone_init(void);
static void zone_insert(Unit *unit, int wd);
static void zone_sort(void);
static void zone_shuffle(void);
static int compare_zone_nodes(const void *a, const void *b);
static void extend_workspace(int semantic_bytes);
static void run_supply_side(Side *side);

static int supply_initialized = FALSE;

/* Equivalence classes of materials. */
static int mclass_count;
static material_stats *mstats = NULL;
static material_stats *first_mstat;

/* Clipped supply-in-thresholds (should modify tables directly??) */
static short *umclipinthreshold;

/* Net importance array */
static int *neti;

/* Workspace for sorting and the heap. */
static void *ws;
static int ws_size = 0;

/* These point to the same region as ws. */
static zone_node *zone_ptr;
static int zone_size;
static int zone_entries;
static heap_node *heap_ptr;
static int heap_size;
static int heap_entries;

/* Emergency hack. */
#if 0
#undef Dprintf
#define Dprintf printf
#endif

void
run_supply(void)
{
    Side *side;

    /* Run the algorithm for all sides in the game (except the independent
     * side). */
    for_all_sides(side) {
	run_supply_side(side);
    }
}

void
run_supply_side(Side *side)
{
    Unit *unit;
    material_stats *mclass;

    /* Make sure we're set up. */
    init_supply_system();

    /* Do we need to run at all? */
    if (!mclass_count)
      return;
    
    /* Initialize a_unit variables. */
    for_all_side_units(side, unit) {
	/* If no materials are important to a unit, it is always considered to
	 * have optimal supply status. */
	if (neti[unit->type]) {
	    unit->s_flow = unit->s_conn = 0;
	} else {
	    unit->s_flow = MAX_FLOW;
	    unit->s_conn = MAX_CONN;
	}
    }

    Dprintf("Running supply for side %d.\n", side_number(side));
    
    /* Run the algorithm for all equivalence classes of materials. */
    for (mclass = first_mstat; mclass != NULL; mclass = mclass->next) {
	process_supply_class(side, mclass);
    }
}

int
supply_system_in_use(void)
{
    init_supply_system();

    return mclass_count ? TRUE : FALSE;
}

#if 0		/* Unused. */

void
compute_supply_zones(side, m)
Side *side;
int m;
{
    Unit *unit;

    init_supply_system();
    if (!mclass_count) return;

    init_tmp_layers();

    heap_init();
    for_all_side_units(side, unit) add_unit_heap(unit, m);

    run_supply_lines(side, side_number(side), m);
}

void
compute_unit_supply_zone(side, unit, m)
Side *side;
Unit *unit;
int m;
{
    init_supply_system();
    if (!mclass_count) return;

    init_tmp_layers();

    heap_init();
    add_unit_heap(unit, m);
    
    run_supply_lines(side, side_number(side), m);
}

#endif

/* init_supply_system() determines if, and how, the supply algorithm will
 * be used. */

static int *mad;

static void
init_supply_system(void)
{
    material_stats *ms, *ps = NULL, *pc = NULL;
    int m, m2, u, cond, t;

    if (supply_initialized)
      return;
    supply_initialized = TRUE;

    Dprintf("Initializing the supply system.\n");

    mclass_count = 0;

    /* Exit immediately if there are no material types. */
    if (!nummtypes) return;

    if (mad == NULL)
      mad = (int *) xmalloc(nummtypes * sizeof(int));

    /* Allocate space for material information. */
    mstats = (material_stats *)xmalloc(nummtypes * sizeof(material_stats));

    /* Clip supply-in-threshold. */
    umclipinthreshold = (short *)xmalloc(nummtypes * numutypes * sizeof(short));
    
    for_all_unit_types(u) for_all_material_types(m) {
	um_clip_in_threshold(u, m) = min(um_supply_in_threshold(u, m), um_storage_x(u, m));
    }
    
    /* Initialize the net importance array. */
    neti = (int *)xmalloc(numutypes * sizeof(int));
    
    for_all_unit_types(u) {
	neti[u] = 0;
	for_all_material_types(m) neti[u] += um_supply_importance(u, m);
    }
    
    /* Analyze supply zone parameters and set up equivalence classes for
     * materials. */
    ms = mstats;
    
    for_all_material_types(m)
      mad[m] = FALSE;
    
    for_all_material_types(m) {
	
	/* Already added? */
	if (mad[m]) continue;
	
	Dprintf("Analyzing material %d.\n", m);

	/* Find out if this material type is used at all in the context of the
	 * supply system. */
	cond = FALSE;
	
	for_all_unit_types(u) {
	    if (um_supply_potential(u, m) > 0) { cond = TRUE; break; }
	}
	
	if (!cond) Dprintf("Supply system not needed with this material.\n");

	/* If not, skip to the next material. */
	if (!cond) continue;
	
	/* Establish a new equivalence class. */
	++mclass_count;
	ms->next       = NULL;
	ms->mtype      = m;
	ms->next_class = pc;
	mad[m]           = TRUE;
	ps               = ms++;
	
	Dprintf("New equivalence class established.\n");
	
	/* Find any matching materials and add them to the class. */
	for_all_material_types(m2) {

	    if (mad[m2]) continue;
	    
	    cond = TRUE;
	    
	    for_all_unit_types(u) {
		if (um_supply_potential(u, m)
		      != um_supply_potential(u, m2)
		   || um_supply_interdiction_at_for_material(u, m)
		      != um_supply_interdiction_at_for_material(u, m2)
		   || um_supply_interdiction_adjacent_for_material(u, m)
		      != um_supply_interdiction_adjacent_for_material(u, m2)) {
		    cond = FALSE;
		    break;
		}
	    }
	    
	    if (cond) {
		for_all_terrain_types(t) {
		    if (tm_supply_deterioration(t, m)
		          != tm_supply_deterioration(t, m2)
		       || tm_supply_enemy_interdiction(t, m)
		          != tm_supply_enemy_interdiction(t, m2)
		       || tm_supply_neutral_interdiction(t, m)
		          != tm_supply_neutral_interdiction(t, m2)) {
			cond = FALSE;
			break;
		    }
		}
	    }
	    
	    if (cond) {
		/* All parameters relating to the establishment of supply
		 * zones are identical. */
		ms->next       = ps;
		ms->mtype      = m2;
		ms->next_class = pc;
		mad[m2]          = TRUE;
		ps               = ms++;
		
		Dprintf("Material %d added to the class.\n", m2);
	    }
	}

	/* Link this class to the next. */
	pc = ps;
    }

    /* Link to the first material_stats structure. */
    first_mstat = ps;
    
    Dprintf("Initialization complete. Memory allocated = %d bytes\n",
	    (size_t)ms - (size_t)mstats + nummtypes * numutypes * sizeof(short)
	    + numutypes * sizeof(int));

    /* Allocate workspace if needed. */
    if (mclass_count) extend_workspace(0);

    /* Finally, trim memory taken by material_stats. */
    if (ms == mstats) {
		/* Defends against a portability bug */
		ms = mstats + 1;
    } 
    if (!realloc(mstats, (size_t)ms - (size_t)mstats)) {
		Dprintf("realloc() failed; exiting");
		exit(1);
	}
}

static void
init_tmp_layers(void)
{
    int x, y;
    
    Dprintf("Initializing scratch layers.\n");

    /* Make sure we have the necessary scratch layers. */
    allocate_area_scratch(2);

    /* Initialize the scratch layers. tmp2 is lazily evaluated.
     * 
     * tmp1 = supply potentials
     * tmp2 = interdiction by enemy/neutral control/units */
    for_all_cells(x, y) {
	set_tmp1_at(x, y, 0);
	set_tmp2_at(x, y, -1);	/* -1 = indeterminate */
    }
}

static void
process_supply_class(Side *side, material_stats *mclass)
{
    int x, y, t;
    int m, n;
    int snum;
    long nrem;
    Unit *unit;
    material_stats us, *ms;
    zone_node *zn;
    
    /* Initialize the heap. */
    heap_init();
    
    /* Set m to any material representing this equivalence class. */
    m = mclass->mtype;
    
    Dprintf("Processing material class related to material %d.\n", m);

    /* Find out this side's number. */
    snum = side_number(side);

    /* Initialize temporary layers. */
    init_tmp_layers();
    
    Dprintf("Looking for applicable units.\n");
    
    /* Find all cells which contain a unit with a non-zero supply-potential and
     * insert them into the heap. */
    for_all_side_units(side, unit) {
	add_unit_heap(unit, m);
    }

    Dprintf("Now expanding supply zones.\n\n");

    run_supply_lines(side, snum, m);

    Dprintf("Complete. Establishing supply zones:\n");
    
    /* Layer tmp1 contains supply potentials for each cell. Now we need to find
     * the actual supply zones (continuous areas of non-zero supply potentials).
     */
    for_all_cells(x, y) if (tmp1_at(x, y)) {
	    
	Dprintf("Supply zone found at (%d, %d). Processing:\n", x, y);

	for (ms = mclass; ms != NULL; ms = ms->next) {
	    ms->demand = 0;
	    ms->supply = 0;
	    ms->wdemand = 0;
	}
	
	zone_init();
	
	establish_supply_zone(side, x, y, mclass);

	/* Process the supply zone. We now know the net supply and demand of
	 * each material in the equivalence class, and have a list of all relevant
	 * units in the supply zone. Alas, we have to calculate unit-specific
	 * supply and demand anew, since storing them could potentially hog a lot
	 * of memory. */
	ms = mclass;

	do {
	    Dprintf("Net balance for material %d: supply=%d, demand=%d, weighted demand=%d.\n",
		    ms->mtype, ms->supply, ms->demand, ms->wdemand);

	    m = ms->mtype;
	
	    /* Shuffle the zone entries. This is the most straightforward
	     * way of ensuring random distribution of any remainder
	     * materials. */
	    zone_shuffle();

	    if (!(ms->supply && ms->demand)) continue;
	    
	    Dprintf("There's a total of %d units in this supply zone.\n", zone_entries);
	    
	    if (ms->supply >= ms->demand) {
		    
		/* There's enough of material ms->mtype for
		 * everyone.
		 */
		nrem = ms->demand;
		
		for(zn = zone_ptr, n = zone_entries; n; --n, ++zn) {

		    (void)calculate_supply_demand(zn->unit, m, &us, zn->re, FALSE);
		
		    zn->unit->supply[m] += us.demand;
		
		    if (neti[zn->unit->type]) {
			zn->unit->s_flow +=
			  ((int)um_supply_importance(zn->unit->type, m) << 14)
			  / neti[zn->unit->type];
		    }

		    if (us.demand) 
		      Dprintf("Gave %d of material %d to (%d, %d).\n",
			      us.demand, m, zn->unit->x, zn->unit->y);

		    if (us.supply) {
			t        = (us.supply * ms->demand) / ms->supply;
			zn->re = (us.supply * ms->demand) % ms->supply;
			/* Round down during the first pass. */
			zn->unit->supply[m] -= t;
			Dprintf("Took %d of material %d from (%d, %d).\n",
				t, m, zn->unit->x, zn->unit->y);
			nrem -= t;
		    } else zn->re = 0;
		}


		if (nrem) {
		    /* Let's hope qsort() doesn't mess up the
		     * beautiful shuffling we did earlier... */
		    zone_sort();
		
		    for(zn = zone_ptr; nrem; --nrem, ++zn) {
			Dprintf("Took remainder from %d at (%d, %d) with %d.\n",
				zn->unit->type, zn->unit->x, zn->unit->y, zn->re);
			zn->unit->supply[m]--;
		    }
		}
	    } else {
		/* There's a shortage of material ms->mtype.
		 */
		nrem = ms->supply;
		
		for(zn = zone_ptr, n = zone_entries; n; --n, ++zn) {
		    /* Some of this is stupid, will have to optimize later */
		    (void)calculate_supply_demand(zn->unit, m, &us, zn->re, FALSE);
		    zn->uw = zn->re;
		    zn->re = us.weight;
		}

		/* Sort by supply weight. */
		zone_sort();
		
		for(zn = zone_ptr, n = zone_entries; n; --n, ++zn) {
			    
		    (void)calculate_supply_demand(zn->unit, m, &us, zn->uw, FALSE);
		    
		    zn->unit->supply[m] -= us.supply;
		
		    if (us.supply)
		      Dprintf("Took %d of material %d from (%d, %d).\n",
			      us.supply, m, zn->unit->x, zn->unit->y);

		    if (us.wdemand) {
			t = (us.wdemand * ms->supply) / ms->wdemand;
			
			if (t >= us.demand) {
			    if (neti[zn->unit->type]) {
				zn->unit->s_flow +=
				  ((int)um_supply_importance(zn->unit->type, m) << 14)
				  / neti[zn->unit->type];
			    }
			    zn->unit->supply[m] += us.demand;
			    zn->re = 0;
			    zn->uw = 0;
			    nrem -= t;
			    /* Update weighted demand. */
			    ms->wdemand -= (t - us.demand) * us.weight;
			    Dprintf("Gave %d of material %d to (%d, %d).\n",
				    us.demand, m, zn->unit->x, zn->unit->y, 0);

			} else {
			
			    if (neti[zn->unit->type]) {
				zn->uw = ((int)um_supply_importance(zn->unit->type, m) << 14) / neti[zn->unit->type];
				zn->unit->s_flow += (zn->uw * t) / us.demand;
				zn->uw /= us.demand;
			    } else zn->uw = 0;
			
			    zn->unit->supply[m] += t;
			    zn->re = (us.wdemand * ms->supply) % ms->wdemand;
			    nrem -= t;

			    Dprintf("Gave %d of material %d to (%d, %d); %d of weighted demand left.\n",
				    t, m, zn->unit->x, zn->unit->y, zn->re);
			}
		    } else zn->re = 0;
		}

		if (nrem) {
		    zone_sort();
		
		    for(zn = zone_ptr; nrem; --nrem, ++zn) {
			Dprintf("Distributed surplus to %d at (%d, %d).\n",
				zn->unit->type, zn->unit->x, zn->unit->y);
			zn->unit->supply[m]++;
			if (neti[zn->unit->type]) zn->unit->s_flow += zn->uw;
		    }
		}
	    }
	} while((ms = ms->next) != NULL);

    }

    Dprintf("Finished processing this supply class.\n");
}

static void
add_unit_heap(Unit *unit, int m)
{
    Unit *tr;
    int ss;
    
    /* Get the basic potential. */
    ss  = um_supply_potential(unit->type, m);

    if (!ss) return;

    /* Effect of terrain on supply potential. */
    ss *= ut_supply_potential_terrain_effect(unit->type,
					     terrain_at(unit->x, unit->y));

    /* Effect of occupancy on supply potential. */
    if ((tr = unit->transport) != NULL) {
	/* This improves accuracy but could conceivably overflow?? */
	ss *= uu_occupant_supply_potential(unit->type, tr->type);
	ss /= 10000;
    } else
      ss /= 100;

    Dprintf("Potential %d, modified to %d, found in a unit of type %d at (%d, %d).\n",
	    um_supply_potential(unit->type, m), ss, unit->type, unit->x, unit->y);

    /* Add to the heap unless a previously processed unit in the same cell
     * had a higher supply potential (only the maximum counts). */

    if (ss > tmp1_at(unit->x, unit->y)) {
	set_tmp1_at(unit->x, unit->y, ss);
	heap_insert(ss, unit->x, unit->y);
    }
}

static void
run_supply_lines(Side *side, int snum, int m)
{
    heap_node hn;
    int x, y, tt, td, t2, cnum, cc, nx, ny, pot;
    int dir, dir2, ss;
    Unit *unit;
    
    /* Expand supply lines in order of descending supply zone potential. The
     * ordering is necessary to avoid pathological cases.
     */
    while(heap_deque(&hn)) {

	Dprintf("Potential %d at (%d, %d).\n", hn.pot, hn.x, hn.y);
	
	/* Has this cell already been reached from a stronger supply line? */
	if (tmp1_at(hn.x, hn.y) > hn.pot) continue;
	
	/* Attempt to run supply to adjacent cells. */
	for_all_directions(dir) {

	    if (point_in_dir(hn.x, hn.y, dir, &x, &y)) {

		tt = terrain_at(x, y);		/* cell type */
		ss = tmp1_at(x, y);		/* previous supply potential (or 0) */

		/* Evaluate enemy/neutral control/interdiction */
		if (tmp2_at(x, y) < 0) {

		    /* Check if nobody/another side is controlling the cell. */
		    if (area.controlside && (cnum = control_side_at(x, y)) != snum) {
			if (cnum == NOCONTROL) t2 = tm_supply_neutral_interdiction(tt, m);
			else if (!side->trusts || !side->trusts[cnum]) {
			    /* Should use reciprocal relation instead in the test? TODO? */
			    t2 = tm_supply_enemy_interdiction(tt, m);
			} else t2 = 0;
		    } else t2 = 0;

		    /* Enemy interdiction in the cell. */
		    for_all_stack(x, y, unit) {
			if (unit->side != side 
			   && (!side->trusts || !side->trusts[side_number(side)])) {
			    t2 += (ut_supply_interdiction_at(unit->type, tt) *
				   um_supply_interdiction_at_for_material(unit->type, m)) / 100;
			}
		    }

		    /* Interdiction from adjacent cells. */
		    for_all_directions(dir2) if (point_in_dir(x, y, dir2, &nx, &ny)) {
			for_all_stack(nx, ny, unit) {
			    if (unit->side != side && (!side->trusts || !side->trusts[side_number(side)])) {
				t2 += (ut_supply_interdiction_adjacent(unit->type, tt) *
				       um_supply_interdiction_adjacent_for_material(unit->type, m)) / 100;
			    }
			}
		    }

		    set_tmp2_at(x, y, t2);
		}
	    
		pot = hn.pot - tmp2_at(x, y);
		
		/* Stop here if we aren't stronger than all previous supply
		 * attempts. */
		if (pot <= ss) continue;
		
		/* Calculate deterioration from cell and border terrain. */
		td = tm_supply_deterioration(tt, m);
		
		for_all_border_types(tt) {
		    if (border_at(hn.x, hn.y, dir, tt)) td += tm_supply_deterioration(tt, m);
		}
		
		/* Find the cheapest connection type. Connection terrain is
		 * assumed to "run over" any border terrain. */
		cc = td;
		for_all_connection_types(tt) {
		    if (connection_at(hn.x, hn.y, dir, tt)
		       && tm_supply_deterioration(tt, m) < cc) {
			cc = tm_supply_deterioration(tt, m);
		    }
		}

		/* Compute the final potential and compare. */
		pot -= min(cc, td);
		if (pot <= ss) continue;

		/* OK, add this cell to the heap. */
		set_tmp1_at(x, y, pot);
		heap_insert(pot, x, y);
	    }
	}
    }
}


/* Establish a supply zone recursively. */
static void
establish_supply_zone(Side *side, int x, int y, material_stats *mclass)
{
    Unit *unit;
    int dir, x1, y1, pot;

    pot = tmp1_at(x, y);
    set_tmp1_at(x, y, 0);

    for_all_stack(x, y, unit) {
	if (unit->side == side) process_unit_supply(unit, mclass, pot);
    }

    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1) && tmp1_at(x1, y1)) {
	    (void)establish_supply_zone(side, x1, y1, mclass);
	}
    }
}

static void
process_unit_supply(Unit *unit, material_stats *mclass, int pot)
{
    Unit *occ;
    material_stats *ms;
    material_stats res;
    int sd = FALSE;

    /* This is a convenient place to process any occupants. */
    for_all_occupants(unit, occ) process_unit_supply(occ, mclass, pot);

    Dprintf("Processing supply for a unit of type %d at (%d, %d) with potential %d;\n",
	    unit->type, unit->x, unit->y, pot);

    /* Now the unit. */
    for (ms = mclass; ms != NULL; ms = ms->next) {
	if (calculate_supply_demand(unit, ms->mtype, &res, pot, TRUE)) {

	    sd = TRUE;
	    
	    Dprintf("Material %d status: supply=%d, demand=%d, weighted demand=%d.\n",
		    ms->mtype, res.supply, res.demand, res.wdemand);
	    
	    ms->supply  += res.supply;
	    ms->demand  += res.demand;
	    ms->wdemand += res.wdemand;
	}
    }
    
    if (sd) {
	/* Link up. */
	zone_insert(unit, pot);
    }
}

/* This function is called twice or thrice per unit per material. */
static int
calculate_supply_demand(Unit *unit, int m, material_stats *res, int pot, int rec_stats)
{
    int s, t, u;
    int sd = FALSE;
    int capm;

    u = unit->type;
    s = unit->supply[m];
    
    /* Calculate any supply capacity deterioration. */
    t = um_supply_capacity_threshold(u, m);
    
    if (pot < t) {
	capm = (pot * 100 + (t - pot) * um_supply_capacity_deterioration(u, m)) / t;
    } else {
	capm = 100;
    }

    if (rec_stats && neti[u]) {
	/* Update connectedness. */
	unit->s_conn += ((capm << 8) * (int)um_supply_importance(u, m)) / (int)neti[u];
    }

    /* Calculate supply. */
    t = um_supply_out_threshold(u, m);

    if (t >= 0 && s > t) {
	if (um_supply_out_max(u, m) == -1) res->supply = s - t;
	else res->supply = min((um_supply_out_max(u, m) * capm) / 100, s - t);

	if (res->supply) sd = TRUE;
    } else {
	res->supply  = 0;
    }

    /* Calculate demand. */
    t = um_clip_in_threshold(u, m);

    if (t > 0 && s < t) {

	if (um_supply_in_max(u, m) == -1) res->demand = t - s;
	else res->demand = min((um_supply_in_max(u, m) * capm) / 100, t - s);

	if (res->demand) {
	    sd = TRUE;
	} else {
	    res->wdemand = 0;
	    res->weight  = 0;

	    /* Not sure about this... supply lines so bad that cannot receive
	     * anything, but inflow still at 100%? */
	    if (rec_stats && neti[u]) {
		unit->s_flow += ((int)um_supply_importance(u, m) << 14) / neti[u];
	    }

	    return sd;
	}

	/* Calculate weighted demand. */
	if (um_supply_starve_weight(u, m) != um_supply_in_weight(u, m)) {
	    res->weight  = (s * um_supply_in_weight(u, m) + (t - s) * um_supply_starve_weight(u, m)) / t;
	} else {
	    res->weight  = um_supply_in_weight(u, m);
	}

	res->wdemand = res->demand * res->weight;

    } else {
	res->demand  = 0;
	res->weight  = 0;
	res->wdemand = 0;

	/* If didn't need any supplies, add to inflow anyway */
	if (t > 0 && rec_stats && neti[u]) {
	    unit->s_flow += ((int)um_supply_importance(u, m) << 14) / neti[u];
	}
    }
    
    return sd;
}

static void
heap_init(void)
{
    heap_entries = 0;
}

static void
heap_insert(int pot, short x, short y)
{
    int i, f;
    
    if (++heap_entries == heap_size) extend_workspace(heap_entries * sizeof(heap_node));

    i = heap_entries;
    f = i >> 1;

    while(f && pot > heap_ptr[f].pot) {
	heap_node_copy(&heap_ptr[i], &heap_ptr[f]);
	i   = f;
	f >>= 1;
    }

    heap_ptr[i].pot = pot;
    heap_ptr[i].x   = x;
    heap_ptr[i].y   = y;
}

static int
heap_deque(heap_node *hn)
{
    int i = 2, f = 1, epot;
    heap_node *enode;
    
    if (!heap_entries) return FALSE;
    
    enode = &heap_ptr[heap_entries];	/* Last entry in the heap
					 * (the first slot is empty). */
    epot  = enode->pot;		/* Priority of the last entry. */

    /* Get the top of the heap. */
    heap_node_copy(hn, &heap_ptr[1]);
    
    /* Ripple the remaining entries up in the heap. */
    while(i < heap_entries) {
	if (heap_ptr[i].pot > heap_ptr[i + 1].pot) {
	    if (epot > heap_ptr[i].pot) {
		heap_node_copy(&heap_ptr[f], enode);
		--heap_entries;
		return TRUE;
	    }
	    heap_node_copy(&heap_ptr[f], &heap_ptr[i]);
	    f  = i;
	    i += i;
	} else {
	    if (epot > heap_ptr[i + 1].pot) {
		heap_node_copy(&heap_ptr[f], enode);
		--heap_entries;
		return TRUE;
	    }
	    heap_node_copy(&heap_ptr[f], &heap_ptr[i + 1]);
	    f = i + 1;
	    i = f + f;
	}
    }

    if (i == heap_entries && heap_ptr[i].pot > epot) {
	heap_node_copy(&heap_ptr[f], &heap_ptr[i]);
	heap_node_copy(&heap_ptr[i], enode);
    } else {
	heap_node_copy(&heap_ptr[f], enode);
    }

    --heap_entries;
    return TRUE;
}

static void
zone_init(void)
{
    zone_entries = 0;
}

static void
zone_insert(Unit *unit, int re)
{
    if (zone_entries == zone_size) extend_workspace(zone_entries * sizeof(zone_node));
    
    zone_ptr[zone_entries].re   = re;
    zone_ptr[zone_entries].unit = unit;
    
    ++zone_entries;
}

/* Just use qsort() for now and hope it doesn't do anything insane. */
static void
zone_sort(void)
{
    qsort((void *)ws, zone_entries, sizeof(zone_node), compare_zone_nodes);
}

/* Randomize units' positions in the zone. */
static void
zone_shuffle(void)
{
    zone_node t, *zn1, *zn2;
    int n;
    
    for(n = zone_entries, zn1 = zone_ptr; n; --n, ++zn1) {
	
	zn2 = &zn1[xrandom(n)];
	
	t.re        = zn1->re;
	t.uw        = zn1->uw;
	t.unit      = zn1->unit;
	zn1->re   = zn2->re;
	zn1->uw   = zn2->uw;
	zn1->unit = zn2->unit;
	zn2->re   = t.re;
	zn2->uw   = t.uw;
	zn2->unit = t.unit;
    }
}

static int
compare_zone_nodes(const void *a, const void *b)
{
    return (int)(((zone_node *)b)->re - ((zone_node *)a)->re);
}

static void
extend_workspace(int semantic_bytes)
{
    void *new_ws;

    if (!ws_size)
      ws_size = INITIAL_WORKSPACE;
    else
      ws_size <<= 1;

    /* Should try realloc() here but who cares. */
    new_ws    = xmalloc(ws_size);

    if (semantic_bytes)
      memcpy(new_ws, ws, semantic_bytes);

    ws        = new_ws;
    zone_ptr  = (zone_node *)new_ws;
    heap_ptr  = (heap_node *)new_ws;

    /* Let's assume nothing about type sizes... */
    heap_size = ws_size / sizeof(heap_node);
    zone_size = ws_size / sizeof(zone_node) - 1;
}

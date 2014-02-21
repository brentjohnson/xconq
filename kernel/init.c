/* General Xconq initialization.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Initialization is complicated, because Xconq needs lots of setup
   for maps, units, sides, and the like.  The data must also be able
   to come from saved games, scenarios, bare maps in files, type
   definitions, or be synthesized if necessary. */

/* The general theory is that everything gets set to a known empty
   state, then all files are read, then all the synthesis methods get
   run.  Files/readers and synth methods are each responsible for
   preventing fatal conflicts. */

#include "conq.h"
#include "kernel.h"
#include "version.h"

using namespace Xconq;

static void check_occs_on_completion(void);
static int advance_is_part_of_advance_cycle(int a, int *alist, int alistsz);
static void init_advances_synopsis(void);
static void disallow_more_types(void);
static void cache_possible_types(void);
static void kill_excess_units(void);
static void trial_assign_player(int i, Player *player);
static void patch_object_references(void);
static void init_view_cell(int x, int y);
static void maybe_init_view_cell(int x, int y);
static int adj_seen_terrain(int x, int y, Side *side);
static void init_ranged_views(void);
static void init_side_advantage(Side *side);
static int make_random_date(int calls, int runs);
static int make_weather(int calls, int runs);
static void configure_sides(void);
static void init_all_views(void);
static int make_initial_materials(int calls, int runs);
static void init_side_balance(void);
static void check_consistency(void);

/* This is true after the game is totally synthesized, but players may
   not all be in the game yet. */

int gamedefined = FALSE;

enum setup_stage current_stage = initial_stage;

/* These are handy tmp vars, usually used with function args. */

short tmputype;
short tmpmtype;
short tmpttype;
short tmpatype;

/* The array of player/side matchups. */

Assign *assignments = NULL;

/* True when no AIs should be set up at game start. */

short initially_no_ai;

/* The name of the default game module. */

char *standard_game_name;

/* This is the main string printing buffer.  It is statically allocated
   because it is used even before the main data structures are inited. */

char spbuf[BUFSIZE];

/* This is an auxiliary buffer.  Also needs to be statically allocated. */

char tmpbuf[BUFSIZE];

/* This is true if the interface was requested to put up a new game
   dialog (kind of a random file for this, but *is* init-related). */

int option_popup_new_game_dialog;

/* Used by any code that is concerned with automatic reactions to a change 
   in Xconq, such as an unit entering a cell. */

namespace Xconq {
    int suppress_reactions = FALSE;
}

#ifdef DESIGNERS

/* This is true if all sides with displays should become designers
   automatically. */

int allbedesigners = FALSE;

/* This is the current count of how many sides are designers. */

int numdesigners = 0;

#endif /* DESIGNERS */

int max_zoc_range;

short *u_possible;

/* The table of all available synthesis methods. */

struct synthtype {
    int key;
    int (*fn)(int calls, int runs);
    int dflt;
    int calls;
    int runs;
} synthmethods[] = {
    { K_MAKE_FRACTAL_PTILE_TERRAIN, make_fractal_terrain, TRUE, 0, 0 },
    { K_MAKE_RANDOM_TERRAIN, make_random_terrain, FALSE, 0, 0 },
    { K_MAKE_EARTHLIKE_TERRAIN, make_earthlike_terrain, FALSE, 0, 0 },
    { K_MAKE_MAZE_TERRAIN, make_maze_terrain, FALSE, 0, 0 },
    { K_MAKE_RIVERS, make_rivers, TRUE, 0, 0 },
    { K_MAKE_COUNTRIES, make_countries, TRUE, 0, 0 },
    { K_MAKE_INDEPENDENT_UNITS, make_independent_units, TRUE, 0, 0 },
    { K_MAKE_INITIAL_MATERIALS, make_initial_materials, TRUE, 0, 0 },
    { K_NAME_UNITS_RANDOMLY, name_units_randomly, TRUE, 0, 0 },
    { K_NAME_GEOGRAPHICAL_FEATURES, name_geographical_features, TRUE, 0, 0 },
    { K_MAKE_ROADS, make_roads, TRUE, 0, 0 },
    { K_MAKE_WEATHER, make_weather, TRUE, 0, 0 },
    { K_MAKE_RANDOM_DATE, make_random_date, FALSE, 0, 0 },
    { -1, NULL, 0, 0, 0 }
};

/* Allow environment vars to override compiled-in library and game. */

void
init_library_path(char *path)
{
    char *xconqlib;
    LibraryPath *image_path;

    xconqlib = getenv("XCONQLIB");
    if (empty_string(xconqlib)) {
	if (!empty_string(path)) {
	    xconqlib = path;
	} else {
	    xconqlib = default_library_pathname();
	}
    }
    xconq_libs = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    xconq_libs->path = copy_string(xconqlib);
    /* Add the pathname to the binary images directory, as an
       alternate place to look. */
    image_path = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    image_path->path = copy_string(default_images_pathname(xconqlib));
    xconq_libs->next = image_path;
    last_user_xconq_lib = NULL;
    standard_game_name = getenv("XCONQSTANDARDGAME");
    if (empty_string(standard_game_name))
      standard_game_name = STANDARD_GAME;
}

void
add_library_path(char *path)
{
    LibraryPath *lib;

    /* A NULL path indicates that all the existing paths should go away. */
    if (path == NULL) {
	xconq_libs = last_user_xconq_lib = NULL;
	return;
    }
    lib = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    lib->path = copy_string(path);
    /* Insert this after the previous user-specified library, if there
       was any, but before the default library places. */
    if (last_user_xconq_lib != NULL) {
	lib->next = last_user_xconq_lib->next;
	last_user_xconq_lib->next = lib;
    } else {
	lib->next = xconq_libs;
	xconq_libs = lib;
    }
    last_user_xconq_lib = lib;
}

/* This is a general init that prepares data structures to be filled in
   by a game definition.  It should run *before* command line parsing. */

void
init_data_structures(void)
{
    init_xrandom(-1);
    init_lisp();
    init_types();
    init_globals();
    init_namers();
    init_world();
    init_history();
    init_sides();
#if 0
    init_agreements();
#endif
    init_units();
    init_nlang();
    init_help();
    init_scorekeepers();
    init_actions();
    init_tasks();
    init_ai_types();
    init_write();
    curdatestr = (char *)xmalloc(BUFSIZE);
}

/* Build the default list of synthesis methods. */

Obj *
g_synth_methods_default(void)
{
    int i;
    Obj *synthlist = lispnil, *synthlistend = lispnil, *tmp;

    for (i = 0; synthmethods[i].key >= 0; ++i) {
	if (synthmethods[i].dflt) {
	    tmp = cons(intern_symbol(keyword_name(
				       (enum keywords)synthmethods[i].key)),
		       lispnil);
	    if (synthlist == lispnil) {
		synthlist = synthlistend = tmp;
	    } else {
		set_cdr(synthlistend, tmp);
		synthlistend = tmp;
	    }
	}
    }
#if (0)
    /* Now alter the global variable to contain this list. */
    set_g_synth_methods(synthlist);
#endif
    return synthlist;
}

/* Game setup happens in several stages. */

void
start_game_load_stage(void)
{
    current_stage = game_load_stage;
    /* Advance everybody else to this stage. */
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_start_game_load();
}

/* Advance to variant setting. */

void
start_variant_setup_stage(void)
{
    current_stage = variant_setup_stage;
    /* Advance everybody else to this stage. */
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_start_variant_setup();
}

void
start_player_pre_setup_stage(void)
{
    do_module_variants(mainmodule, lispnil);
    /* Stop accepting variant settings. */
    current_stage = player_pre_setup_stage;
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_variants_chosen();
}

void
start_player_setup_stage(void)
{
    make_trial_assignments();
    /* Start accepting player settings. */
    current_stage = player_setup_stage;
    /* Advance everybody else to this stage. */
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_start_player_setup();
}

void
start_game_ready_stage(void)
{
    /* Stop accepting player settings. */
    current_stage = game_ready_stage;
    /* Advance everybody else to this stage. */
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_players_assigned();
}

static void
check_occs_on_completion(void)
{
    int u = NONUTYPE, u2 = NONUTYPE, u3 = NONUTYPE;
    int updated = TRUE;
    int **pregnancies = NULL;

    pregnancies = (int **)xmalloc(numutypes * sizeof(int *));
    for_all_unit_types(u) {
	pregnancies[u] = (int *)xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u2) {
	    pregnancies[u][u2] = uu_complete_occs_on_completion(u, u2);
#if (0) // Not necessary, since we start them with 0 instead of 1 CP now.
	    if (1 == u_cp(u2))
		pregnancies[u][u2] += uu_incomplete_occs_on_completion(u, u2);
#endif
	}
    }
    while (updated) {
	updated = FALSE;
	for_all_unit_types(u) {
	    for_all_unit_types(u2) {
		if (pregnancies[u][u2]) {
		    // Cycle found.
		    if (u == u2) {
			init_error(
"Cycle detected in occs-on-completion graph. %s is an ancestor of itself", 
				   u_type_name(u));
			return;
		    }
		    // Inherit pregnancies.
		    for_all_unit_types(u3) {
			if (pregnancies[u2][u3] && !pregnancies[u][u3]) {
			    pregnancies[u][u3] = TRUE;
			    updated = TRUE;
			}
		    }
		    if (updated)
		      break;
		}
	    }
	    if (updated)
	      break;
	}
    }
}

/* Determine if a given advance is part of a cycle. Construct list of
   "touched" advances. Check list before adding another to it. If the
   advance is already in the list, then we have a cycle. */

int G_advances_graph_has_cycles = FALSE;
PackedBoolTable *G_advances_synopsis = NULL;

static void
init_advances_synopsis(void)
{
    int a = NONATYPE, a2 = NONATYPE, a3 = NONATYPE;
    int updated = TRUE;

    G_advances_synopsis = create_packed_bool_table(numatypes, numatypes);
    init_packed_bool_table(G_advances_synopsis);
    for_all_advance_types(a) {
	for_all_advance_types(a2) {
	    set_packed_bool(G_advances_synopsis, a, a2, 
			    aa_needed_to_research(a, a2));
	}
    }
    while (updated) {
	updated = FALSE;
	for_all_advance_types(a) {
	    for_all_advance_types(a2) {
		if (get_packed_bool(G_advances_synopsis, a, a2)) {
		    if (a == a2) {
			G_advances_graph_has_cycles = TRUE;
			init_warning(
"Cycle detected in advances graph. %s is an ancestor of itself", 
				     a_type_name(a));
			return;
		    }
		    for_all_advance_types(a3) {
			if (get_packed_bool(G_advances_synopsis, a2, a3) 
			    && !get_packed_bool(G_advances_synopsis, a, a3)) {
			    set_packed_bool(G_advances_synopsis, a, a3, TRUE);
			    updated = TRUE;
			}
		    }
		    if (updated)
		      break;
		}
	    }
	    if (updated)
	      break;
	}
    }
}

static int
advance_is_part_of_advance_cycle(int a, int *alist, int alistsz)
{
    int i = -1, a2 = NONATYPE, cyclefound = FALSE;
    int *alist2 = NULL;

    if (alistsz >= numatypes)
      return FALSE;
    for_all_advance_types(a2) {
	if (aa_needed_to_research(a, a2)) {
	    for (i = 0; i < alistsz; ++i) {
		/* If cycle detected, then immediately pull out. */
		if (alist[i] == a2)
		  return TRUE;
	    }
	    alist2 = (int *)xmalloc((alistsz + 1) * sizeof(int));
	    memcpy(alist2, alist, alistsz * sizeof(int));
	    alist2[alistsz] = a2;
	    cyclefound = advance_is_part_of_advance_cycle(a2, alist2,
							  alistsz + 1);
	    free(alist2);
	    if (cyclefound)
	      return TRUE;
	}
    }
    return FALSE;
}

/* Run a doublecheck on plausibility of game parameters.  Additional
   checks are performed elsewhere as needed, for instance during
   random generation.  Serious mistakes exit now, since they can cause
   all sorts of strange behavior and core dumps.  It's a little more
   friendly to only exit at the end of the tests, so all the mistakes
   can be found at once. */

/* In theory, if a game passes these tests, then Xconq will never crash. */

void
check_game_validity(void)
{
    int failed = FALSE, movers = FALSE, actors = FALSE;
    int u1, u2, t1, t2, i, methkey, found, a = NONATYPE;
    Obj *synthlist = g_synth_methods(), *methods, *method;

    /* No more types allowed after this point. */
    disallow_more_types();
    /* See if any cycles exist in the advances graph. */
    if (numatypes)
      init_advances_synopsis();
    /* Patch all cross-references. */
    patch_object_references();
    /* We must have at least one kind of unit. */
    if (numutypes < 1) {
	init_error("no unit types have been defined");
	failed = TRUE;
    }
    check_occs_on_completion();
    /* OK not to have any types of materials. */
    /* We must have at least one kind of terrain. */
    if (numttypes < 1) {
	init_error("no terrain types have been defined");
	failed = TRUE;
    }
    /* Make sure inter-country distances relate correctly. */
    if (g_separation_min() >= 0
	&& g_separation_max() >= 0
	&& !(g_separation_min() <= g_separation_max())) {
	init_warning("country separations %d to %d screwed up",
		     g_separation_min(), g_separation_max());
    }
    /* Check all method names in the synthesis list. */
    for_all_list(synthlist, methods) {
	method = car(methods);
	found = FALSE;
	if (symbolp(method)) {
	    methkey = keyword_code(c_string(method));
	    for (i = 0; synthmethods[i].key >= 0; ++i) {
		if (methkey == synthmethods[i].key) {
		    found = TRUE;
		    break;
		}
	    }
	}
	if (!found) {
	    sprintlisp(spbuf, method, BUFSIZE);
	    init_warning("bad synthesis method `%s', ignoring", spbuf);
	}
    }
    /* (Need more general game checks.) */
    max_zoc_range = -1;
    /* Check that all the unit names and chars are distinct. */
    for_all_unit_types(u1) {
	for_all_unit_types(u2) {
#if 0	    /* It may be desirable to have several unit types with the same
	    name, e.g. to make it possible to use different unit images for
	    different sides. */

	    /* Only do "upper triangle" of utype x utype matrix. */
	    if (u1 < u2) {
		if (strcmp(u_type_name(u1), u_type_name(u2)) == 0) {
		    init_warning(
		     "unit types %d and %d are both named \"%s\"",
				 u1, u2, u_type_name(u1));
		    /* This is bad but not disastrous, so don't fail. */
		}
	    }
#endif
	    if (uu_zoc_range(u1, u2) > max_zoc_range) {
		max_zoc_range = uu_zoc_range(u1, u2);
	    }
	}
    }
    /* (Eventually check material types also.) */
    /* Check that all terrain names and chars are distinct. */
    for_all_terrain_types(t1) {
	for_all_terrain_types(t2) {
	    /* Only do "upper triangle" of ttype x ttype matrix. */
	    if (t1 < t2) {
		if (strcmp(t_type_name(t1), t_type_name(t2)) == 0) {
		    init_warning(
		     "terrain types %d and %d are both named \"%s\"",
		     t1, t2, t_type_name(t1));
		    /* This is bad but not disastrous, so don't fail. */
		}
		/* Should check that colors are different from each other
		   and from builtin colors? */
	    }
	}
    }
    /* Check various unit type properties. */
    for_all_unit_types(u1) {
	/* Can't make use of this yet, so error out if anybody tries. */
	if (u_available(u1) != 1) {
	    init_error("unit type %d must always be available", u1);
	    failed = TRUE;
	}
	/* should be part of general bounds check */
	if (u_cp(u1) < 0) {
	    init_error("unit type %d has nonpositive cp", u1);
	    failed = TRUE;
	}
	if (u_hp(u1) <= 0) {
	    init_error("unit type %d has nonpositive hp", u1);
	    failed = TRUE;
	}
	if (u_speed(u1) > 0) {
	    movers = TRUE;
	}
	if (u_acp(u1) > 0) {
	    actors = TRUE;
	}
	if (u_wrecked_type(u1) == u1) {
	    init_error("unit type %d wrecks to same type.", u1);
	    failed = TRUE;
	}
    }
    /* If nothing can move and nothing can build, this will probably be
       a really dull game, but there may be such games, do don't say
       anything normally. */
    if (numutypes > 0 && !actors) {
	Dprintf("No actors have been defined.\n");
    }
    if (numutypes > 0 && !movers) {
	Dprintf("No movers have been defined.\n");
    }
    if (numttypes > 0 && numcelltypes == 0) {
	init_error("no terrain type has been allowed for cells");
	failed = TRUE;
    }
    /* This is a really bad game definition, leave before we crash.
       This would only be executed on systems where init_error doesn't
       exit immediately. */
    if (failed) {
	exit(0);
    }
    Dprintf("Finished checking game design.\n");
    Dprintf("It defines %d unit types, %d material types, %d terrain types",
	    numutypes, nummtypes, numttypes);
    Dprintf(", %d advance types.\n", numatypes);
}

/* Finalize the scorekeepers. */

void
finalize_scorekeepers(void)
{
    Scorekeeper *sk = NULL;
    SideMask whomask = NOSIDES, knowntomask = NOSIDES;
    Side *side = NULL;
    ParamBoxSide paramboxs;

    for_all_scorekeepers(sk) {
	/* If scorekeeper only applies to certain side classes, 
	   then generate an appropriate 'whomask'. */
	whomask = knowntomask = NOSIDES;
	if (sk->who != lispnil) {
	    for_all_sides(side) {
		paramboxs.side = side;
		if (eval_boolean_expression(sk->who, 
					    fn_test_side_in_sideclass, 
					    FALSE, (ParamBox*)&paramboxs))
		  whomask = add_side_to_set(side, whomask);
	    }
	    sk->whomask = whomask;
	}
	/* If scorekeeper only is visible to certain side classes, 
	   then generate an appropriate 'knowntomask'. */
	if (sk->knownto != lispnil) {
	    for_all_sides(side) {
		paramboxs.side = side;
		if (eval_boolean_expression(sk->knownto, 
					    fn_test_side_in_sideclass, 
					    FALSE, (ParamBox*)&paramboxs))
		  knowntomask = add_side_to_set(side, knowntomask);
	    }
	    sk->knowntomask = knowntomask;
	}
    }
}

/* Calculate the values of global variables that are used everywhere. */

void
calculate_globals(void)
{
    int u1, u2;
    Side *side;

    /* The game is now completely defined; no further user-specified
       changes (such as variants or player choices) can occur. */
    gamedefined = TRUE;
    /* Now that all sides are in place, finalize the scorekeepers. */
    finalize_scorekeepers();
    /* Calculate globals pertaining to the world and area. */
    calculate_world_globals();
    /* This needs to be precalculated instead of as-needed, since
       range can be validly both negative and positive, so no way to
       distinguish uninitialized. */
    {
	int u1, u2, range;
    	extern int max_detonate_on_approach_range;

	max_detonate_on_approach_range = -1;
	for_all_unit_types(u1) {
	    for_all_unit_types(u2) {
		range = uu_detonate_approach_range(u1, u2);
		max_detonate_on_approach_range =
		  max(range, max_detonate_on_approach_range);
	    }
	}
    }
    /* Set up the cache of what each side can build.  We do it here
       because if it's too early, type_allowed_on_side will get filled
       in with values that don't reflect side props such as its class. */
    for_all_sides(side) {
	update_canbuild_vector(side);    	 
	update_cancarry_vector(side);    	 
   	if (numatypes > 0) {
	    update_canresearch_vector(side);
	}
    }
    /* Set up other caches. */
    cache__type_max_night_acp_from_any_terrains = NULL;
    type_max_night_acp_from_any_terrains(0);
    cache__type_max_acp_from_any_occs = NULL;
    type_max_acp_from_any_occs(0);
    cache__type_max_acp = NULL;
    type_max_acp(0);
    cache__type_max_speed_from_any_occs = NULL;
    type_max_speed_from_any_occs(0);
    cache__type_max_speed = NULL;
    type_max_speed(0);
    // Do we need to run auto-repair code?
    for_all_unit_types(u1) {
	for_all_unit_types(u2) {
	    if (could_auto_repair(u2, u1)) {
		any_auto_repair = TRUE;
		break;
	    }
	}
	if (0 < u_hp_recovery(u1)) 
	    any_hp_recovery = TRUE;
	if (any_auto_repair || any_hp_recovery)
	    break;
    }
    Dprintf("Any auto repair: %d\n", any_auto_repair);
    // Precalculate some useful info regarding auto-repair.
    if (!cv__could_be_auto_repaired) {
	cv__could_be_auto_repaired = 
	    (short *)xmalloc(numutypes * sizeof(short));
	cv__auto_repaired_range_max = 
	    (short *) xmalloc(numutypes * sizeof(short));
	for_all_unit_types(u1) {
	    cv__could_be_auto_repaired[u1] = FALSE;
	    cv__auto_repaired_range_max[u1] = -1;
	}
    }
    if (any_auto_repair) {
	for_all_unit_types(u1) {
	    for_all_unit_types(u2) {
		if (uu_auto_repair(u2, u1) > 0) {
		    cv__could_be_auto_repaired[u1] = TRUE;
		    cv__auto_repaired_range_max[u1] =
			max(cv__auto_repaired_range_max[u1],
			    uu_auto_repair_range(u2, u1));
		}
	    }
	}
    }
    /* Find out if any unit types could automatically change type to 
       others. */
    for_all_unit_types(u1) {
        if (is_unit_type(u_auto_upgrade_to(u1))) {
            any_auto_change_types = TRUE;
            break;
        }
    }
}

static void
disallow_more_types(void)
{
    /* Turn off the addition of more types in case read_forms 
    failed to do that. */
    if (canaddttype) {
	disallow_more_terrain_types();
    }
    if (canaddutype) {
	disallow_more_unit_types();
    }
    if (canaddmtype) {
	disallow_more_material_types();
    }
    if (canaddatype) {
	disallow_more_advance_types();
    }
}

/* Clean up all the objects and cross-references. NOTE: this may be
   called multiple times!  It should not have any additional effects
   if called more than once. */

static void
patch_object_references(void)
{
    Unit	*unit, *transport, *occ;
    UnitView	*uview;
    Obj		*utref, *utorig;
    Side	*side;
    int		id;

    /* This cannot be called as part of prealloc_debug(), since that
       can occur before any types are defined.  This should happen
       after all unit types have been defined but before a first save;
       this is as good a place as any. */
    shortest_unique_name(0);
    /* Use read-in ids to fill in side slots that point to other objects. */
    for_all_sides(side) {
	if (side->playerid >= 0) {
	    side->player = find_player(side->playerid);
	}
	if (side->controlled_by_id >= 0) {
	    side->controlled_by = side_n(side->controlled_by_id);
	}
	if (side->self_unit_id > 0) {
	    side->self_unit = find_unit(side->self_unit_id);
	}
    }
    for_all_units(unit) {
	/* It's possible that dead units got read in, so check. */
	if (alive(unit)) {
	    if (unit->transport_id != lispnil) {
		transport = NULL;
		utref = utorig = unit->transport_id;
		/* We have a Lisp object; use it to identify a particular
		   unit as the transport. */
		if (symbolp(utref) && boundp(utref)) {
		    utref = symbol_value(utref);
		}
		if (numberp(utref)) {
		    transport = find_unit(c_number(utref));
		    if (transport == NULL)
		      init_warning("could not find a transport id %d for %s",
				   c_number(utref), unit_desig(unit));
		} else if (stringp(utref)) {
		    transport = find_unit_by_name(c_string(utref));
		    if (transport == NULL)
		      init_warning(
"could not find a transport named \"%s\" for %s",
				   c_string(utref), unit_desig(unit));
		} else if (symbolp(utref)) {
		    transport = find_unit_by_symbol(utref);
		    if (transport == NULL)
		      init_warning(
"could not find a transport with sym \"%s\" for %s",
				   c_string(utref), unit_desig(unit));
		} else {
		    /* not a recognized way to refer to a unit */
		    sprintlisp(tmpbuf, utorig, BUFSIZE);
		    init_warning("could not find transport %s for %s",
				 tmpbuf, unit_desig(unit));
		}
		/* It is important to make sure that unit->x, and
		   unit->y are negative at this point.  Otherwise, the
		   coverage will be messed up for units put into
		   transports that have not yet been placed.  They
		   will be covered for entering the cell, and again
		   when the transport enters the cell. */
		if (transport != NULL) {
		    if (transport != unit->transport) {
			if (unit->transport != NULL) {
			    leave_transport(unit);
			}
			/* (also check that this is a valid transport type?) */
			enter_transport(unit, transport);
		    }
		} else {
		    /* (could let the unit enter the cell, 
			or could make it infinitely postponed) */
		}
	    } else {
		/* Check that the unit's location is meaningful. */
		if (!inside_area(unit->x, unit->y)) {
		    if (inside_area(unit->prevx, unit->prevy)) {
			/* First try to just drop the unit in the cell. */
			if (type_can_occupy_cell(unit->type, 
						 unit->prevx, unit->prevy)
			    && type_survives_in_cell(unit->type, 
						     unit->prevx, 
						     unit->prevy)) {
			    enter_cell(unit, unit->prevx, unit->prevy);
			/* There was no room in the terrain. */
			} else {
			    /* Search for a transport that unit can enter. */
			    for_all_stack(unit->prevx, unit->prevy, transport) {
				/* The important question is if the putative 
				   transport trusts unit, and not if unit 
				   trust the transport. */
				if (unit_trusts_unit(transport, unit)
				    && can_occupy(unit, transport)) {
				    enter_transport(unit, transport);
				    break;
				}
			    }
			    /* We didn't find a transport. Try to load as 
			       many units as possible from this cell into unit,
			        and then try to enter the cell again. */
			    if (!inside_area(unit->x, unit->y)) {
				/* Note: we cannot iterate over 
				   for_all_stack here since occ entering unit 
				   will break the nexthere link. */
				for_all_units(occ) {
				    if (occ->x == unit->prevx
				        && occ->y == unit->prevy
					/* We are only interested in top-level 
					   units. */
					&& occ->transport == NULL
				        /* The important question is if unit 
					   trust the putative occ, and not if 
					   the occ trust unit. */
				        && unit_trusts_unit(unit, occ)
				        && can_occupy(occ, unit)) {
					    leave_cell(occ);
					    enter_transport(occ, unit);
				    }
				}
				/* Now try to enter the cell again. */
				enter_cell(unit, unit->prevx, unit->prevy);
			    }
			    if (!inside_area(unit->x, unit->y)) {
				init_warning("Can't put %s at %d,%d (%s)",
					     unit_desig(unit),
					     unit->prevx, unit->prevy,
					     (type_can_occupy_cell(unit->type, 
								   unit->prevx,
								   unit->prevy) 
					     ? "deadly terrain" : "no room"));
			    }
			}
			/* This prevents attempts to do a second enter_cell
			   during initialization. */
			unit->prevx = unit->prevy = -1;
		    } else if (unit->prevx == -1 && unit->prevy == -1) {
		    	/* This will be left alone - should have pos
                           filled in later. */
		    } else if (unit->cp >= 0) {
			/* Warn, but only if there's no good reason
			   for the unit to have an offworld
			   position. */
			if (area.fullwidth == 0) {
			    init_warning(
"%s is at offworld location, left there",
					 unit_desig(unit));
			}
			/* This will make it be a reinforcement. */
			unit->cp = -1;
		    }
		}
	    }
	    /* Make sure that side numbering will use only new numbers. */
	    if (unit->side != NULL) {
		if (unit->number > 0) {
		    (unit->side->counts)[unit->type] =
		      max((unit->side->counts)[unit->type], 1 + unit->number);
		}
	    } else {
		/* Trash the numbers on indep units. */
		unit->number = 0;
	    }
	    if (completed(unit)) {
		if (unit->act == NULL)
		  init_unit_actorstate(unit, TRUE);
		/* Restore acp that wasn't written out because it was
		   the most normal value. */
		if (unit->act && unit->act->acp < u_acp_min(unit->type))
		  unit->act->acp = u_acp(unit->type);
		/* Restore initacp that wasn't written out because it
		   was the normal value. */
		if (unit->act && unit->act->initacp < u_acp_min(unit->type))
		  unit->act->initacp = u_acp(unit->type);
		/* Might already have a plan, leave alone if so. */
		if (unit->plan == NULL) {
		    init_unit_plan(unit);
		}
	    }
	    /* Maybe fill in a formation's leader with a unit. */
	    if (unit->plan != NULL
		&& unit->plan->formation != NULL) {
		id = unit->plan->formation->args[0];
		if (id > 0) {
		    unit->plan->funit = find_unit(id);
		    if (unit->plan->funit == NULL) {
			init_warning(
"%s missing leader #%d, cancelling formation",
				     unit_desig(unit), id);
			unit->plan->formation = NULL;
		    }
		}
	    }
	} else {
	    /* Dead units need to be disentangled from anything that
	       might have been done to them.  For instance, a module
	       might include a standard collection of units, but then
	       follow up by removing some of those units, and can do
	       it by setting hp == 0.  We want this to work
	       consistently and reliably. */
	    /* Null this out, any possible unit reference is useless. */
	    unit->transport = NULL;
	    if (inside_area(unit->x, unit->y)) {
		leave_cell(unit);
	    }
	}
    }
    /* Unit views are read in before units (with the sides) so we
    must also patch the uview->unit links. */
    for_all_unit_views(uview) {
    	uview->unit = find_unit(uview->id);
    }
}

/* Make up a proposed side/player assignment, creating sides and players
   as necessary.  Lock down any assignments that should not be changed,
   but leave everything to be changed as desired. */

/* (when does locking get done?) */

void
make_trial_assignments(void)
{
    int i = 0;
    Side *side;
    Player *player;

    Dprintf("Making trial side/player assignments\n");
    /* Fill in the sides' predefined default and range of initial
       advantage. */
    for_all_sides(side) {
    	init_side_advantage(side);
    }
    /* Quick check to make sure we don't have more peers than available 
       sides. */
    /*! \note We must ensure elsewhere that that numremotes is not allowed 
	      g_sides_max(). */
    if (numremotes > g_sides_max())
      init_warning(
"A total of %d peers are networked, but only %d sides are available",
		   numremotes, g_sides_max());
    /* Ensure that we have the minimal required number of sides. Only 
       real sides increment numsides, so it will equal g_sides_min or 
       numremotes when we are done. However, the total number of 
       sides including indepside (numtotsides) will equal g_sides_min + 1 or
       numremotes + 1. */
    while (numsides < max(g_sides_min(), numremotes)) {
	make_up_a_side();
    }
    /* Continue to add sides up to the wanted default number, but don't
       exceed g_sides_max in case we are using a default g_sides_wanted
       which is higher than g_sides_max. */
       while (numsides < min(g_sides_wanted(), g_sides_max())) {
	make_up_a_side();
    }
    /* Keep adding sides if we have unassigned players up to the limit
       set by g_sides_max. Note: numplayers - 1 since the indepside now 
       has a player. */
    while (numsides < min(numplayers - 1, g_sides_max())) {
	make_up_a_side();
    }
    /* Kick indepside out of the game if it is unnecessary. */
    if (!indepside_needed())
      indepside->ingame = FALSE;

    /* Put in all the sides. */
    for_all_sides(side) {
	assignments[i++].side = side;
    }
    /* First create a player to handle indepside if necessary. */
    if (numplayers < indepside->ingame) {
	add_player();
    }
    /* Add remote players. */
    if (numremotes > 0) {
    	add_remote_players();
    }
    /* If only one player (the indepside player) has been created so
       far, make one that is human-run, presumably by the person who
       started up this program. */
    if (numplayers == indepside->ingame) {
	add_default_player();
    }
    if (numtotsides < numplayers) {
	/* We have too many players. */
	init_warning("too many players (%d) for %d sides, ignoring extra",
		     numplayers, numtotsides);
    }
    /* Make any prespecified assignments. */
    for (i = 0; i < numtotsides; ++i) {
	if (assignments[i].side != NULL
	    && assignments[i].side->player != NULL
	    && assignments[i].player == NULL) {
	    trial_assign_player(i, assignments[i].side->player);
	    /* (should this assignment be locked?) */
	}
    }
    /* Assign any remaining players. */
    player = playerlist;
    for (i = 0; i < numtotsides; ++i) {
	if (assignments[i].side != NULL
	    && assignments[i].side->ingame
	    && assignments[i].player == NULL) {
	    for (; player != NULL; player = player->next) {
		if (player->side == NULL) {
		    trial_assign_player(i, player);
		    break;
		}
	    }
	}
    }
    /* Add default players for sides with none (in separate loop so player
       creation doesn't confuse player list traversal above).  Don't
       add for any sides not actually participating in the game. */
    for (i = 0; i < numtotsides; ++i) {
	if (assignments[i].side != NULL
	    && assignments[i].side->ingame
	    && assignments[i].player == NULL) {
	    player = add_player();
	    /* Default players are always AIs (at least for now). */
	    if (!initially_no_ai)
	      player->aitypename = "mplayer";
	    trial_assign_player(i, player);
	}
    }
    /* Finally set the indepside AI type correctly. */
    if (indepside->ingame) {
    	if (indepside->player->aitypename) {
	/* If a player already was assigned, do nothing. */
	} else if (g_indepside_has_ai()) {
	/* Use mplayer by default if indepside should have an AI. */
	    indepside->player->aitypename = "mplayer";
    	} else {
    	/* Else set the AI to NULL. */
	    indepside->player->aitypename = NULL;
    	}
	/* Hoist the Jolly Roger if indepside has an AI and does
	not already have a flag that was read from the module. */
	if (indepside->player->aitypename
	    && strcmp("none", indepside->emblemname) == 0) {
	        indepside->emblemname = "flag-pirate";
    	}
    }
    /* This warning can happen if there are more players than active
       sides. */
    for_all_players(player) {
	if (player->side == NULL) {
	    init_warning("Player %s not given a side", player_desig(player));
	}
    }
    /* At this point, we have matching sides and players, ready to be
       rearranged if desired. */
}


/* Returns true if indepside already has units or may get units made. */

int
indepside_needed(void)
{
    Unit *unit;
    int	u, t;
	
    /* First check what the game designer thought about this. */
    if (g_no_indepside_ingame())
      return FALSE;
    /* Then check existing (preassigned) units. */
    for_all_units(unit) {
	if (unit->side == indepside)
	  return TRUE;
    }
    /* Then check if independent units will ever be made. */
    for_all_unit_types(u) {
	if (u_indep_growth(u) > 0 || u_indep_near_start(u) > 0)
	  return TRUE;
	for_all_terrain_types(t) {
	    if (ut_indep_density(u, t) > 0 
		&& ut_vanishes_on(u, t) != TRUE) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

static void
trial_assign_player(int i, Player *player)
{
    assignments[i].player = player;
    player->side = assignments[i].side;
    (assignments[i].side)->player = player;
    /* Set the player's advantage to be the side's advantage, if defined. */
    if (assignments[i].player != NULL
	&& assignments[i].player->advantage == 0) {
	assignments[i].player->advantage = assignments[i].side->advantage;
    }
    Dprintf("Tentatively assigned %s to %s%s\n",
	    side_desig(assignments[i].side),
	    player_desig(assignments[i].player),
	    (assignments[i].locked ? " (locked)" : ""));
}

/* Create a random side with default characteristics. */

Side *
make_up_a_side(void)
{
    extern Obj *side_defaults;

    Side *side = create_side();

    if (side == NULL) {
	run_error("could not create a side");
	return NULL;
    }
    fill_in_side(side, side_defaults, FALSE);
    make_up_side_name(side);
    init_side_advantage(side);
    Dprintf("Made up a side %s\n", side_desig(side));
    return side;
}

/* If undefined, seed a side's advantage and allowable range from the
   global values. */

void
init_side_advantage(Side *side)
{
    /* Set up the default and range of initial advantages. */
    if (side->advantage == 0)
      side->advantage = g_advantage_default();
    if (side->minadvantage == 0)
      side->minadvantage = g_advantage_min();
    if (side->maxadvantage == 0)
      side->maxadvantage = g_advantage_max();
    if (side->maxadvantage == 0) {
	int i, methkey, found = FALSE;
	Obj *synthlist = g_synth_methods(), *methods, *method;

	for_all_list(synthlist, methods) {
	    method = car(methods);
	    if (symbolp(method)) {
		methkey = keyword_code(c_string(method));
		for (i = 0; synthmethods[i].key >= 0; ++i) {
		    if (methkey == synthmethods[i].key) {
			found = TRUE;
			break;
		    }
		}
	    }
	    if (found) {
		if (synthmethods[i].key == K_MAKE_COUNTRIES) {
		    int u, var = FALSE;

		    for_all_unit_types(u) {
			if (u_start_with(u) > 0) {
			    var = TRUE;
			    break;
			}
		    }
		    if (var)
		      side->maxadvantage = 10;
		}
	    }
	}
    }
    /* If we're not getting any guidance from global defaults, just
       set to 1. */
    if (side->minadvantage == 0)
      side->minadvantage = 1;
    if (side->maxadvantage == 0)
      side->maxadvantage = side->minadvantage;
}

/* Add a side and a player to go with it (used by interfaces). */

int
add_side_and_player(void)
{
    int n;
    Side *side;
    Player *player;

    side = make_up_a_side();
    if (side == NULL)
      return -1;
    n = numtotsides - 1;
    assignments[n].side = side;
    player = add_player();
    player->aitypename = next_ai_type_name(NULL);
    assignments[n].player = player;
    player->side = assignments[n].side;
    (assignments[n].side)->player = player;
    /* Set the player's advantage to be the side's advantage, if not
       already set. */
    if (player->advantage == 0) {
	player->advantage = side->advantage;
    }
    /* Return the new side's position in the assignment array. */
    return n;
}

void
set_player_advantage(int n, int newadv)
{
    Player *player;

    player = assignments[n].player;
    if (player != NULL)
      player->advantage = newadv;
    /* (should complain otherwise) */
    /* (should check limits) */
}

void
rename_side_for_player(int n, int which)
{
    Obj *entry;
    Side *side;

    /* Don't rename indepside. */
    if (n == 0)
	return;
    side = assignments[n].side;
    side->name = side->noun = side->pluralnoun = side->adjective = NULL;
    if (which >= 0) {
	entry = elt(g_side_lib(), which);
	/* Take away the optional weighting number. */
	if (numberp(car(entry)))
	  entry = cdr(entry);
	fill_in_side(side, entry, FALSE);
    } else {
	make_up_side_name(side);
    }
}

/* Given the name of an AI type, assign it to the player at the given
   position in the assignments array. */

void
set_ai_for_player(int n, char *aitype)
{
    Player *player;

    player = assignments[n].player;
    if (player == NULL)
        return;
    if (empty_string(aitype) || find_ai_type(aitype) > 0) {
	player->aitypename = aitype;
	if (player->side == indepside) {
		// Toggle the Jolly Roger on or off,
		//   if the indepside gains or loses control by a human
		//   or an ai. However, always keep a flag that was
		//   assigned by the game module. 
		if (empty_string(aitype) 
		    && empty_string(player->displayname)
		    && strcmp("flag-pirate", indepside->emblemname) == 0) {
		    indepside->emblemname = "none";
		    set_g_indepside_has_ai(FALSE);
		// Otherwise, we fly the Jolly Roger.
		} else if (strcmp("none", indepside->emblemname) == 0) {
		    indepside->emblemname = "flag-pirate";
		    set_g_indepside_has_ai(TRUE);
	    	}
	}
	/* Needed to flush out any existing AIs in saved games. */
	set_side_ai(player->side, aitype);
    } else {
	init_warning("\"%s\" is not a known AI type name", aitype);
    }
}

/* This can be used by interfaces to exchange players between one side and
   another. */

int
exchange_players(int n, int n2)
{
    int i;
    Player *tmpplayer = assignments[n].player;

    if (n < 0)
      n = 0;
    if (n2 < 0) {
	for (i = n + 1; i <= numtotsides + n; ++i) {
	    n2 = i % numtotsides;
	    if (assignments[n2].side && (assignments[n2].side)->ingame)
	      break;
	}
	/* No sides to exchange with, return. */
    	if (i == numtotsides + n)
	  return -1;
    }

#if 0
    /* Don't switch with the indepside player. */
    if (n2 == 0)
    	n2 = 1;
#endif

    assignments[n].player = assignments[n2].player;
    assignments[n2].player = tmpplayer;
    /* Doesn't seem like these should be needed, but they are. */
    assignments[n].player->side = assignments[n].side;
    assignments[n].side->player = assignments[n].player;
    assignments[n2].player->side = assignments[n2].side;
    assignments[n2].side->player = assignments[n2].player;
    /* Toggle the Jolly Roger on or off if the indepside gains or loses
    control by a human or an ai. However, always keep a flag that was
    assigned by the game module. */
    if (indepside->player) {
	    if (empty_string(indepside->player->aitypename)
	        && empty_string(indepside->player->displayname)
	        && strcmp("flag-pirate", indepside->emblemname) == 0) {
	    	indepside->emblemname = "none";
	    }
	    if ((!empty_string(indepside->player->aitypename)
	    	|| !empty_string(indepside->player->displayname))
	    	&& strcmp("none", indepside->emblemname) == 0) {
	    	indepside->emblemname = "flag-pirate";
	    }
    }
    return n2;
}

extern int nextsideid, nextplayerid;				/* From side.c. */
extern void return_default_colorname(Side *side);	/* Ditto. */

int
remove_side_and_player(int s)
{
    Side * side, *side2;
    Player *player;
    Unit *unit;
    int n;

    side = side_n(s);
    /* We are trying to remove a side that does not exist. */
    if (!side) {
	Dprintf("Trying to remove non-existing side %d.\n", s); 
	return FALSE;
    }
    /* We cannot remove a side that is assigned to a human player. */ 
    if (side->player->displayname)
        return FALSE;
    /* Nor can we remove the indepside. */
    if (side == indepside)
        return FALSE;        
    /* We need a minimum number of sides. */
    if (numsides == g_sides_min())
        return FALSE;        

    /* Give back the default color so that it can be used by a new side. */
    return_default_colorname(side);

    /* Give any units to indepside if ingame, or else kill them. */
    for_all_units(unit) {
    	/* Can't use for_all_side_units this early during startup. */
	if (unit->side == side) {
		if (indepside->ingame) {
			change_unit_side(unit, indepside, H_SIDE_LOST, NULL);
		} else {
			kill_unit(unit, -1);
		}
	}
    }
    /* First assign a new player list head if required. This is not necessary 
    for the side list, since indepside (which we cannot delete) always is the 
    list head. However, it is possible to delete the first side in a game where
    the indepside has no player, in which case the deleted side's player is the
    player list head. */  
    if (playerlist == side->player) {
    	if (side->player->next) {
		playerlist = side->player->next;
	/* It should not be possible to delete a side if its player is the only 
	player since this player should always be a human player but a buggy 
	interface might try. */
	} else {
		Dprintf("Trying to remove dside (interface bug).\n"); 
		return FALSE;
	}
    }
    /* Patch up the side list and the assignments array. */
    for_all_sides(side2) {
    	n = side2->id;
	if (side2->next && side2->next == side) {
		/* If the removed side was the last side in the list we make 
		the side before it the new last side. */
		if (side == lastside) {
			lastside = side2;
			side2->next = NULL;
		/* else we make the side before it point to the side after it. */
		} else {
			side2->next = side->next;
		}
	}
	/* Decrement assignments and side numbers for all sides after the
	removed side by 1. */
    	if (n > side->id) {
    		assignments[n - 1].side = side2;
    		assignments[n - 1].player = side2->player;
    		side2->id--;
	}
    }
    /* The patch up the player list. */
    for_all_players(player) {
    	if (player->next && player->next == side->player) {
    		/* If the removed side's player was the last player in the list,
    		we make the player before it the last player. */
    		if (side->player == last_player) {
    			last_player = player;
    			player->next = NULL;
    		/* else we make the player before it point to the player after it. */
    		} else {
    			player->next = side->player->next;
    		}
	}
	/* Decrement the ids of all players after the removed side's player by 1. */
	if (player->id > side->player->id) {
		player->id--;
	}	    
    }
    /* Decrement various globals by 1. */
    --numsides;
    --numtotsides;
    --numplayers;
    --nextsideid;
    --nextplayerid;

    /* Reclaim some memory. Not everything can be freed. For example, there is 
    no support for deallocating lisp objects, so the game will crash if you do.
    Nor can the default doctrine be freed. */
 
    free(side->trusts);
    free(side->trades);
    free(side->counts);
    free(side->tech);
    free(side->inittech);
    free(side->numunits);
    free(side->gaincounts);
    free(side->losscounts);
    free(side->atkstats);
    free(side->hitstats);
    free(side->advance);
    free(side->canresearch);
    free(side->treasury);
    free(side->c_rates);
    free(side->unitnamers);
    free(side->startwith);
    free(side->action_priorities);
    free(side->already_seen);
    free(side->already_seen_indep);
    free(side->udoctrine);
    free(side->orders);
    free(side->last_order);
    free(side->terrview);
    free(side->auxterrview);
    free(side->auxterrviewdate);
    free(side->terrviewdate);
    free(side->unit_views);
    free(side->materialview);
    free(side->materialviewdate);
    free(side->tempview);
    free(side->tempviewdate);
    free(side->cloudview);
    free(side->cloudbottomview);
    free(side->cloudheightview);
    free(side->cloudviewdate);
    free(side->windview);
    free(side->windviewdate);
    free(side->curactor);
    free(side->ai);

    free(player);
    free(side);
    
    /* Don't leave any bogus pointers around. */
    player = NULL;
    side = NULL;

    return TRUE;
}

/* Synthesis methods fill in whatever is not fixed by game modules or by
   the player(s). */

void
run_synth_methods(void)
{
    int i, methkey, found, rslt;
    Obj *restlist, *done, *synthlist, *methods, *method, *donelist;

    /* Restore any saved state for synthmethods. */
    if (!gameinited) {
	for_all_list(g_synth_done(), restlist) {
	    done = car(restlist);
	    method = car(done);
	    found = FALSE;
	    if (symbolp(method)) {
		methkey = keyword_code(c_string(method));
		for (i = 0; synthmethods[i].key >= 0; ++i) {
		    if (methkey == synthmethods[i].key) {
			found = TRUE;
			synthmethods[i].calls = c_number(cadr(done));
			synthmethods[i].runs = c_number(caddr(done));
			break;
		    }
		}
	    } else {
		init_warning("non-symbol in synthesis list");
	    }
	    if (!found) {
		sprintlisp(spbuf, method, BUFSIZE);
		init_warning("bad synthesis method %s, ignoring", spbuf);
	    }
	}
    }
    /* Make sure no dead units get saved. */
    flush_dead_units();
    /* Make a consistent ordering of units. */
    sort_units(TRUE);
    synthlist = g_synth_methods();
    Dprintf("Will run syntheses ");
    Dprintlisp(synthlist);
    Dprintf("\n");
    donelist = lispnil;
    for_all_list(synthlist, methods) {
	method = car(methods);
	found = FALSE;
	if (symbolp(method)) {
	    methkey = keyword_code(c_string(method));
	    for (i = 0; synthmethods[i].key >= 0; ++i) {
		if (methkey == synthmethods[i].key) {
		    found = TRUE;
		    if (!gameinited && synthmethods[i].calls == 0) {
			rslt = (*synthmethods[i].fn)(synthmethods[i].calls,
						     synthmethods[i].runs);
			++(synthmethods[i].calls);
			if (rslt)
			  ++(synthmethods[i].runs);
		    }
		    break;
		}
	    }
	    donelist = cons(cons(method,
				 cons(new_number(synthmethods[i].calls),
				      cons(new_number(synthmethods[i].runs),
					   lispnil))),
			    donelist);
	} else {
	    init_warning("non-symbol in synthesis list");
	}
	if (!found) {
	    sprintlisp(spbuf, method, BUFSIZE);
	    init_warning("bad synthesis method %s, ignoring", spbuf);
	}
    }
    set_g_synth_done(reverse(donelist));
}

int
get_synth_method_uses(int methkey, int *calls, int *runs)
{
    int i;

    for (i = 0; synthmethods[i].key >= 0; ++i) {
	if (methkey == synthmethods[i].key) {
	    *calls = synthmethods[i].calls;
	    *runs = synthmethods[i].runs;
	    return TRUE;
	}
    }
    return FALSE;
}

int
make_weather(int calls, int runs)
{
    int x, y, winddir, t, var;
    extern int maxclouds;	

    if (maxclouds > 0) {
	if (!clouds_defined()) {
	    allocate_area_clouds();
	    for_all_cells(x, y) {
		t = terrain_at(x, y);
		/* Vary the cloud cover randomly across the range of possibilities. */
		var = t_clouds_max(t) - t_clouds_min(t);
		if (var > 0)
		  var = xrandom(var + 1);
		set_raw_cloud_at(x, y, t_clouds_min(t) + var);
	    }
	}
    }
    if (maxwindforce > 0) {
	if (!winds_defined()) {
	    allocate_area_winds();
	    if (g_wind_mix_range() > 0) {
		/* Make all winds start in the same direction. */
		/* Initial variation will randomize. */
		winddir = random_dir();
		for_all_cells(x, y) {
		    set_wind_at(x, y, winddir,
				t_wind_force_avg(terrain_at(x, y)));
		}
	    } else {
		for_all_cells(x, y) {
		    set_wind_at(x, y, random_dir(),
				t_wind_force_avg(terrain_at(x, y)));
		}
	    }
	}
    }
    return TRUE;
}

/* Set the starting date/time to a random value within a given range. */

int
make_random_date(int calls, int runs)
{
    int n;

    /* Don't bother if the initial date has been set explicitly. */
    if (!empty_string(g_initial_date()))
      return FALSE;
    if (empty_string(g_initial_date_min()))
      return FALSE;
    if (empty_string(g_initial_date_max()))
      return FALSE;
    /* Use the low end of the date range as a starting place for date
       calculations. */
    set_initial_date(g_initial_date_min());
    n = turns_between(g_initial_date_min(), g_initial_date_max());
    /* Now bump the starting date to its random place. */
    set_initial_date(absolute_date_string(xrandom(n) + 1));
    return TRUE;
}

/* The final init cleans up various stuff. */

void
final_init(void)
{
    Side *side;

    /* Fill in any empty doctrines. */
    for_all_sides(side) {
	init_doctrine(side);
    }
    /* Make sure each side has a self-unit if it needs one. */
    for_all_sides(side) {
	init_self_unit(side);
    }
    /* At this point we should be ready to roll.  Any inconsistencies
       hereafter will be fatal. */
    check_consistency();
    cache_possible_types();
    kill_excess_units();
    /* Count the units initially present. */
    init_side_balance();
    /* Fix up garbled view data. */
    init_all_views();
    configure_sides();
    /* Check again, just to be sure. */
    check_consistency();
    create_game_help_nodes();
    /* Set up the scores to be attached to each side. */
    init_scores();
    final_init_world();
    init_run();
    /* Start the recording of history. */
    start_history();
    /* Compute and cache AI/planning data. */
    ai_init_shared();
#ifdef DEBUGGING
    /* Make sure that any debugging-related allocation is done. */
    if (Debug || DebugG || DebugM)
      prealloc_debug();
    /* Report on memory consumption. */
    Dprintf("One side is %d bytes.\n", sizeof(Side));
    Dprintf("One unit is %d bytes, one plan is %d bytes.\n",
	    sizeof(Unit), sizeof(Plan));
    Dprintf("One task is %d bytes.\n", sizeof(Task));
    Dprintf("One unit view is %d bytes.\n", sizeof(UnitView));
    Dprintf("One hist event is %d bytes, one past unit is %d bytes.\n",
	    sizeof(HistEvent), sizeof(PastUnit));
    if (Debug)
      report_malloc();
#endif /* DEBUGGING */
}

static void
kill_excess_units(void)
{
    Unit *unit;

    for_all_units(unit) {
	/* Offworld units with no scheduled appearance should evaporate. */
	if (!inside_area(unit->x, unit->y)
	    && unit_appear_turn(unit) < 0) {
	    kill_unit(unit, -1);
	}
    }
    /* Make the units' storage available. */
    flush_dead_units();
}

/* Load up any player-specified configuration data. */

void
configure_sides(void)
{
    Side *side;

    for_all_sides(side) {
	load_side_config(side);
    }
}

/* Calculate what each side knows about the world. */

void
init_all_views(void)
{
    int x, y, i = 0, todo = max(1, numtotsides * area.numcells);
    int terrainset;
    Side *side;
    extern int suppress_see_wakeup;

    suppress_see_wakeup = TRUE;
    /* Set up the basic view structures for all sides first. */
    for_all_sides(side) {
	side->see_all = g_see_all();
	terrainset = init_view(side);
	calc_coverage(side);
	tmpside = side;
	if (g_terrain_seen() || side->see_all) {
	    for_all_cells(x, y) {
		init_view_cell(x, y);
	    }
	} else {
	    for_all_cells(x, y) {
		if (!terrainset) {
		    set_terrain_view(side, x, y, UNSEEN);
		}
		/* (should do already-seen down below?) */
		if (terrain_view(side, x, y) != UNSEEN) {
		    init_view_cell(x, y);
	    	}
	    }
	}
	/* We're not normally allowed to choose whether to display all
	       or not. */
	side->may_set_show_all = FALSE;
	/* Display appearance normally matches viewing abilities. */
	side->show_all = side->see_all;
	/* Can't think of any other place to put this... */
	calc_start_xy(side);
    }
    /* Possibly nothing more to do here. */
    if (g_see_all())
      return;
    /* Now we go to computing the actual view of the units in each
       cell, as well as the view coverage count (really?). */
    announce_lengthy_process("Computing current view at each location");
    for_all_sides(side) {
	for_all_cells(x, y) {
	    ++i;
    	    if (i % 100 == 0)
    	      announce_progress((100 * i) / todo);
	    see_cell(side, x, y);
	}
    }
    finish_lengthy_process();
    init_ranged_views();
    suppress_see_wakeup = FALSE;
}

/* This is a helper routine.  The helper is applied to each cell,
   decides what is visible in that cell. */

/* This only works from already_seen, does not account for coverage. */

/* (should only be run once/cell/side, use a scratch layer to keep track?) */

static void
init_view_cell(int x, int y)
{
    int u, chance;
    Unit *unit;

    /* Guaranteed to see the terrain accurately. */
    set_terrain_view(tmpside, x, y, buildtview(terrain_at(x, y)));
    /* If this cell is under observation, don't need to do anything special. */
    if (cover(tmpside, x, y) > 0)
      return;
    /* Scan all the units here to see if any are visible. */
    for_all_stack(x, y, unit) {
    	if (in_play(unit)) {
    	    u = unit->type;
	    if (u_see_always(u)) {
	    	see_exact(tmpside, x, y);
	    	/* or flag unit as spotted? */
		set_cover(tmpside, x, y, 1);
	    	return;
	    }
	    /* Only check already-seen if we didn't read in a set of
	       unit views already. */
	    if (!tmpside->unit_view_restored) {
		chance = already_seen_chance(tmpside, unit);
		if (probability(chance)) {
		    add_unit_view(tmpside, unit);
		}
	    }
	}
    }
    /* We get to see the weather conditions here. */
    if (winds_defined())
      set_wind_view(tmpside, x, y, raw_wind_at(x, y));
}

/* Get the already-seen chance from either the side, if defined, or
   from the unit type. */

int
already_seen_chance(Side *side, Unit *unit)
{
    int u = unit->type, val;

    if (indep(unit)) {
	if (side != NULL
	    && side->already_seen_indep != NULL) {
	    val = side->already_seen_indep[u];
	    if (val >= 0)
	      return val;
	}
	return u_already_seen_indep(u);
    } else {
	if (side != NULL
	    && side->already_seen != NULL) {
	    val = side->already_seen[u];
	    if (val >= 0)
	      return val;
	}
	return u_already_seen(u);
    }
}

static void
maybe_init_view_cell(int x, int y)
{
    int dir, x1, y1;

    if (adj_seen_terrain(x, y, tmpside) && flip_coin()) {
	init_view_cell(x, y);
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		init_view_cell(x1, y1);
	    }
	}
    }
}

static int
adj_seen_terrain(int x, int y, Side *side)
{
    int dir, x1, y1;

    if (!inside_area(x, y))
      return FALSE;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    if (terrain_view(side, x1, y1) != UNSEEN)
	      return TRUE;
	}
    }
    return FALSE;
}

/* Do ranged initial views from units. */

static void
init_ranged_views(void)
{
    int rad, x, y, pop, dir, x1, y1, i = 0;
    Unit *unit;
    Side *side, *side2;

    /* Don't run if nothing exists to look at. */
    if (!terrain_defined())
      return;
    /* Skip if everything already known, side creation got these cases. */
    if (g_see_all() || g_terrain_seen())
      return;
    announce_lengthy_process("Computing ranged and people views");
    /* Compute the view for each side. */ 
    for_all_sides(side) {
	/* Set this so the helper fn has a side to use. */
	tmpside = side;
	/* View from our own and other units. */
	for_all_units(unit) {
	    if (trusted_side(unit->side, side)) {
		/* The unit always sees itself. */
		see_exact(side, unit->x, unit->y);
		/* It may also see things nearby. */
		rad = u_seen_radius(unit->type);
		if (rad >= area.maxdim) {
		    /* Special optimization - view the whole area. */
		    for_all_cells(x, y) {
			init_view_cell(x, y);
		    }
		    /* Note that we're not done; other units may be able to
		       supply more exact views of their vicinities than
		       would init_view_cell from a distant unit. */
		} else if (rad >= 0) {
		    apply_to_area(unit->x, unit->y, rad, init_view_cell);
		}
	    }
	}
	/* The people see everything in the cells that they are in, plus the
	   normally visible things in adjacent cells. */
	if (people_sides_defined()) {
	    for_all_interior_cells(x, y) {
		pop = people_side_at(x, y);
		side2 = side_n(pop);
		if (pop != NOBODY && trusted_side(side2, side)) {
		    see_exact(side, x, y);
		    for_all_directions(dir) {
			if (point_in_dir(x, y, dir, &x1, &y1)) {
			    init_view_cell(x1, y1);
			}
		    }
		}
	    }
	}
	if (side->finalradius > 0) {
	    /* (should also view terrain adj to each of these cells, since the
	       viewing represents exploration) */
	    apply_to_ring(side->startx, side->starty,
			  1, side->finalradius - 1,
			  init_view_cell);
	    apply_to_ring(side->startx, side->starty,
			  side->finalradius - 2, side->finalradius + 2,
			  maybe_init_view_cell);
	}
	announce_progress((100 * i++) / numtotsides);
    }
    finish_lengthy_process();
}

/* Method to give all units and terrain a basic stockpile of supply. */

int
make_initial_materials(int calls, int runs)
{
    int m, t, doany, x, y, didany = FALSE;

    /* Go over each material and terrain type, looking for nonzero
       material in terrain possibilities, then alloc and fill in layers
       as needed. */
    for_all_material_types(m) {
	doany = FALSE;
	for_all_terrain_types(t) {
	    tmp_t_array[t] = min(tm_storage_x(t, m), tm_initial(t, m));
	    if (tmp_t_array[t] > 0)
	      doany = TRUE;
	}
	if (doany) {
	    allocate_area_material(m);
	    for_all_cells(x, y) {
		t = terrain_at(x, y);
		set_material_at(x, y, m, tmp_t_array[t]);
	    }
	    didany = TRUE;
	}
    }
    return didany;
}

/* Give the unit what it is declared to have stockpiled
   at the start of a game. */

void
init_supply(Unit *unit)
{
    int m, u = unit->type;

    for_all_material_types(m) {
	unit->supply[m] = min(um_storage_x(u, m), um_initial(u, m));
    }
}

/* Count all the initial units in each side's balance sheet, but only
   if all the balance sheets are completely blank. */

void
init_side_balance(void)
{
    int u;
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	for_all_unit_types(u) {
	    if (total_gain(side, u) > 0)
	      return;
	}
    }
    for_all_units(unit) {
	count_gain(unit->side, unit->type, initial_gain);
    }
}

/* This routine does a set of checks to make sure that Xconq's data
   is in a valid state.  This is particularly important after init,
   since the combination of files and synthesis methods may have
   caused some sort of disaster. */

void
check_consistency(void)
{
    int x, y;

    /* If no terrain, make a flat area of all ttype 0. */
    if (!terrain_defined()) {
	init_warning("No terrain defined, substituting type 0");
	allocate_area_terrain();
	for_all_cells(x, y) {
	    if (inside_area(x, y)) {
		set_terrain_at(x, y, 0);
	    }
	}
	add_edge_terrain();
    }
    if (numtotsides <= 0) {
	init_error("There are no player sides at all in this game");
    } else if (numtotsides < numplayers) {
	init_warning("Only made %d of the %d sides requested",
		     numtotsides, numplayers);
    }
    /* Need any kind of unit checks? */
}

/* Build a vector of unit types that can possibly appear in the
   current game. */
/* (This vector does not account for every exotic game design, but
   this vector is currently only used when formatting the final
   statistics table.) */

static void
cache_possible_types(void)
{
    int u, u2, rescan;
    Unit *unit;

    u_possible = (short *) xmalloc(numutypes * sizeof(short));
    for_all_units(unit) {
	u_possible[unit->type] = TRUE;
    }
    rescan = TRUE;
    while (rescan) {
	rescan = FALSE;
	for_all_unit_types(u) {
	    if (u_possible[u]) {
		for_all_unit_types(u2) {
		    if (could_create(u, u2)
			&& !u_possible[u2]) {
			u_possible[u2] = TRUE;
			rescan = TRUE;
		    }
		}
	    }
	}
    }
}

/* This does the actual assignment of players to sides, and initializes the
   side structures appropriately. */

void
assign_players_to_sides(void)
{
    int i, numdisplays = 0, numdisplayswanted = 0, numais = 0, n = 0;
    Side *side;
    Player *player;

    announce_lengthy_process("Assigning players to sides");
    for (i = 0; i < numtotsides; ++i) {
	announce_progress((100 * i) / numtotsides);
	side = assignments[i].side;
	player = assignments[i].player;
	if (player == NULL)
	  continue;
	canonicalize_player(player);
	/* Fix any mistaken advantages. */
	/* This is a warning here because properly-done interfaces shouldn't
	   allow any mistaken advantages to get this far. */
	if (player->advantage < side->minadvantage) {
	    init_warning("Requested advantage of %d for %s is too low, will be set to %d",
			 player->advantage, player_desig(player), side->minadvantage);
	    player->advantage = side->minadvantage;
	}
	if (player->advantage > side->maxadvantage) {
	    init_warning("Requested advantage of %d for %s is too high, will be set to %d",
			 player->advantage, player_desig(player), side->maxadvantage);
	    player->advantage = side->maxadvantage;
	}
	/* Call the interface code to initialize the side's display, if
	   it wants to use one (the interface has to decide). */
	if (side_wants_display(side)) {
	    ++numdisplayswanted;
	    if (player->rid == my_rid) {
		init_ui(side);
	    } else {
		init_remote_ui(side);
	    }
	}
	if (side_has_display(side)) {
	    ++numdisplays;
	}
	/* Count the desired AIs, for setup below. */
	if (side_wants_ai(side)) {
	    ++numais;
	}
	Dprintf("Assigned %s to %s\n",
		side_desig(side), player_desig(player));
    }
    finish_lengthy_process();
    if (numdisplays < numdisplayswanted) {
	if (numdisplays < 1) {
	    init_warning("None of the %d requested displays opened",
			 numdisplayswanted);
	} else {
	    init_warning("Only %d of %d requested displays opened",
			 numdisplays, numdisplayswanted);
	}
    } else if (numdisplays == 0) {
	init_warning("Need at least one local or remote display to run");
    }
#ifdef DESIGNERS
    /* Make each displayed side into a designer if it was requested. */
    if (allbedesigners) {
    	for_all_sides(side) {
	    if (side_has_display(side)) {
		become_designer(side);
	    }
    	}
    }
#endif /* DESIGNERS */
    if (numais > 0) {
	announce_lengthy_process("Setting up AIs");
	for (i = 0; i < numtotsides; ++i) {
	    if (numais > 1)
	      announce_progress((100 * n++) / numais);
	    side = assignments[i].side;
	    if (side_wants_ai(side)) {
		init_ai(side);
	    }
	}
	finish_lengthy_process();
    }
}

/* Make sure any debugging I/O routines have allocated their space.
   Usually routines like side_desig allocate their working space on
   demand, but if the first demand occurs during a game being saved
   because allocation fails, then we're in big trouble.  So this
   routine, which need only be be called when debugging is turned
   on, calls allocation-needing things in such a way to cause them
   to allocate their working space. */

void
prealloc_debug(void)
{
    side_desig(NULL);
    player_desig(NULL);
    unit_desig(NULL);
}

/* Return the program version. */

char *
version_string(void)
{
    return VERSION;
}

/* Return the copyright notice. */

char *
copyright_string(void)
{
    return COPYRIGHT;
}

/* Return the license string. */

char *
license_string(void)
{
    return "\
Xconq is free software and you are welcome to distribute copies of it\n\
under certain conditions; type \"o copying\" to see the conditions.\n\
There is absolutely no warranty for Xconq; type \"o warranty\" for details.\n\
";
}

/* This comment is a fake reference to K_NO_X, which is a keyword used
   to clear the subtype-x property of terrain, but is not actually
   mentioned in the code - besides here anyway. :-) */

// xConq
// Game modules.

// $Id: module.cc,v 1.4 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1897-1989   Stanley T. Shebs
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
    \brief Game modules.
    \ingroup grp_gdl
*/

#include "gdl/gvars.h"
#include "gdl/tables.h"
#include "gdl/kernel.h"
#include "gdl/ui.h"
#include "gdl/namer.h"
#include "gdl/media/imf.h"
#include "gdl/unit/unit.h"
#include "gdl/unit/pastunit.h"
#include "gdl/unit/unitview.h"
#include "gdl/side/side.h"
#include "gdl/player.h"
#include "gdl/history.h"
#include "gdl/score.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Module *modulelist;

Module *curmodule;

FILE *wfp;

Module *reshaper;

// Global Variables: Validation

int G_advances_graph_has_cycles = FALSE;

// Global Variables: Game State

int midturnrestore = FALSE;

// Global Variables: Game Setup

int max_zoc_range;

PackedBoolTable *G_advances_synopsis = NULL;

// Global Variables: Buffers

//! Buffer for shortest names.
char *shortestbuf;

// Local Variables

//! The main module defining the game in effect.
static Module *mainmodule;

//! Module for capturing snapshot of game state.
/*!
    A pre-allocated module used for when we're saving the game and may
    not do any more allocation.
*/
static Module *spare_module;

//! Name of spare module file.
static char *spare_file_name;

// Local Variables: Behavior Options

//! True if all variants should be made available.
/*! Irrespective of the game designer's spec. */
static int force_all_variants = FALSE;

// Local Variables: Buffers

//! Temporary buffer.
/*!
    This is an auxiliary buffer.
    Needs to be statically allocated because it is used before main
    data structures are inited.
*/
static char tmpbuf[BUFSIZE];

//! Buffer for module description.
static char *moduledesigbuf = NULL;

// Local Variables: Synthesis Methods

//! The table of all available synthesis methods.
/*!
    \todo Make data structure and global extern
          so that they can be used by init code.
*/
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

// Local Function Declarations: Notifications

//! Complain about useless form.
static void useless_form_warning(Module *module, Obj *form);

// Local Function Declarations: Validation

//! Ensure that advances graph is a DAG.
/*!
    Determine if a given advance is part of a cycle. Construct list of
    "touched" advances. Check list before adding another to it. If the
    advance is already in the list, then we have a cycle.
*/
static void init_advances_synopsis(void);

//! Ensure that occupancy graph is a DAG.
static void check_occs_on_completion(void);

//! Prevent decalration of more types.
static void disallow_more_types(void);

//! Relink objects by ID.
/*!
    Clean up all the objects and cross-references.
    \note This may be called multiple times!
          It should not have any additional effects if called more than once.
*/
static void patch_object_references(void);

// Local Function Declarations: GDL I/O

//! Attempt to open saved game.
static FILE *open_module_saved_game(Module *module);
//! Attempt to open game in library.
static FILE *open_module_library_file(Module *module);

//! Return array of variants from variant-defining form.
static Variant *interp_variant_defns(Obj *lis);

//! Add standard set of variants.
static void add_std_var(
    Variant *varray, int key, char *name, char *help, Obj *dflt);
//! Process one variant for given module.
static void do_one_variant(Module *module, struct a_variant *var, Obj *varsetdata);

//! Include module into one being processed.
/*! Inclusion is half-module-like, not strictly textual. */
static void include_module(Obj *form, Module *module);
//! Start affirmative branch of test form in module.
static void start_conditional(Obj *form, Module *module);
//! Start negative branch of test form in module.
static void start_else(Obj *form, Module *module);
//! End test form.
static void end_conditional(Obj *form, Module *module);

//! Serialize variants to GDL.
static void write_variants(Variant *varray);

// Notifications

static
void
useless_form_warning(Module *module, Obj *form)
{
    char posbuf[BUFSIZE], buf[BUFSIZE];

    if (!actually_read_lisp)
      return;
    module_and_line(module, posbuf);
    sprintlisp(buf, form, BUFSIZE);
    init_warning("%sA useless form: %s", posbuf, buf);
}

// Queries

void
module_and_line(Module *module, char *buf)
{
    if (module) {
	if (module->startlineno != module->endlineno) {
	    sprintf(buf, "%s:%d-%d: ",
		    module->name, module->startlineno, module->endlineno);
	} else {
	    sprintf(buf, "%s:%d: ",
		    module->name, module->startlineno);
	}
    } else {
	buf[0] = '\0';
    }
}

char *
module_desig(Module *module)
{
    if (moduledesigbuf == NULL)
      moduledesigbuf = (char *)xmalloc(BUFSIZE);
    sprintf(moduledesigbuf, "module %s (%s)",
	    module->name, (module->title ? module->title : "no title"));
    return moduledesigbuf;
}

// Validation

static
void
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

static
void
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

static
void
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

static
void
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

// Lifecycle Management

Module *
create_game_module(char *name)
{
    Module *module = (Module *) xmalloc(sizeof(Module));

    /* Fill in nonzero slots. */
    copy_module(module, NULL);
    /* The module's name must never be NULL. */
    if (name == NULL)
      name = "";
    module->name = copy_string(name);
    /* Add to front of module list. */
    module->next = modulelist;
    modulelist = module;
    return module;
}

void
copy_module(Module *module, Module *origmodule)
{
    char *name;
    Module *next, *incl, *nextincl, *lastincl;

    /* No matter what, preserve the existing name. */
    name = module->name;
    if (origmodule != NULL) {
	/* Save away the module's links to other modules. */
	next = module->next;
	incl = module->include;
	nextincl = module->nextinclude;
	lastincl = module->lastinclude;
	memcpy(module, origmodule, sizeof(Module));
	/* By default, we copy everything through; but keep a separate
	   copy of the original name and variants, because the calling
	   routine may overwrite with other values. */
	/* (should test each property individually here?) */
	if (empty_string(origmodule->origmodulename)) {
	    /* If no original module, use the main module. */
	    module->origmodulename = origmodule->name;
	    module->origvariants = origmodule->variants;
	    module->origversion = origmodule->version;
	}
	/* Kill off caches that should not be replicated. */
	module->contents = NULL;
	module->sp = NULL;
	module->fp = NULL;
	module->open = FALSE;
	module->loaded = FALSE;
	/* Restore links. */
	module->next = next;
	module->include = incl;
	module->nextinclude = nextincl;
	module->lastinclude = lastincl;
    } else {
	/* If there is no original from which to copy, just do some
	   generic clearing/init. */
	memset(module, 0, sizeof(Module));
	module->blurb = lispnil;
	module->instructions = lispnil;
	module->notes = lispnil;
	module->designnotes = lispnil;
	module->startlineno = 1;
	module->endlineno = 1;
    }
    module->name = name;
}

// Game Setup

Module *
find_game_module(char *name)
{
    Module *module;

    if (name != NULL) {
	for_all_modules(module) {
	    if (module->name && strcmp(name, module->name) == 0)
	      return module;
	}
    }
    return NULL;
}

Module *
add_game_module(char *name, Module *includer)
{
    Module *module = NULL, *other = NULL;

    module = get_game_module(name);
    if (!module) {
	init_error("Could not include module %s", name);
	return NULL;
    }
    if (includer) {
	/* Add to the end of the list of include files. */
	if (includer->include == NULL) {
	    /* Avoid creating cycles. */
	    for (other = module->nextinclude; other != NULL;
		 other = other->nextinclude) {
		if (!strcmp(includer->name, other->name))
		  return module;
	    }
	    includer->include = includer->lastinclude = module;
	} else {
	    for (other = includer->include; other != NULL;
		 other = other->nextinclude) {
		/* already here, just return it. */
		if (module == other)
		  return module;
	    }
	    includer->lastinclude->nextinclude = module;
	    includer->lastinclude = module;
	}
    } else {
	/* an error? */
    }
    return module;
}

Module *
get_game_module(char *name)
{
    Module *module = find_game_module(name);

    if (module != NULL)
      return module;
    return create_game_module(name);
}

// GDL I/O

FILE *
open_module_saved_game(Module *module)
{
	char fullnamebuf[PATH_SIZE];
	LibraryPath *p;
	FILE *fp;

	/* Can't open saved games without a filename. */
	if (empty_string(module->filename)) {
		return NULL;
	}
	/* Search the working dir first in case we picked a game file in a
	non-standard location. Only do this in single player games.*/
	if (numremotes == 0) {
		fp = open_file(module->filename, "r");
		if (fp) {
		    return fp;
		}
	}
	/* First search the saved games directory for filename. */
	make_pathname(game_homedir(), module->filename, NULL, fullnamebuf);
	/* Now try to open the file. */
	fp = open_file(fullnamebuf, "r");
	if (fp) {
		return fp;
	}
	/* Then search the library paths for filename. */
	for_all_library_paths(p) {
		make_pathname(p->path, module->filename, NULL, fullnamebuf);
		/* Now try to open the file. */
		fp = open_file(fullnamebuf, "r");
		if (fp) {
			return fp;
		}
	}
	return NULL;
}

FILE *
open_module_library_file(Module *module)
{
	char fullnamebuf[PATH_SIZE];
	LibraryPath *p;
	FILE *fp;

	/* Can't open anonymous library modules. */
	if (empty_string(module->name)) {
		return NULL;
	}
	/* Search the working dir first in case we picked a game file in a
	non-standard location. Only do this in single player games.*/
	if (numremotes == 0) {
		/* First look for "name.g". */
		make_pathname("", module->name, "g", fullnamebuf);
		fp = open_file(fullnamebuf, "r");
		if (fp) {
		    	return fp;
		}
		/* Then look for "name" in case the game desinger did not
		use the g file extension. */
		make_pathname("", module->name, NULL, fullnamebuf);
		/* Now try to open the file. */
		fp = open_file(fullnamebuf, "r");
		if (fp) {
		    	return fp;
		}
	}
	/* First search the library paths for "name.g". */
	for_all_library_paths(p) {
		make_pathname(p->path, module->name, "g", fullnamebuf);
		/* Now try to open the file. */
		fp = open_file(fullnamebuf, "r");
		if (fp) {
		    	return fp;
		}
	}
	/* Then search the library paths for "name" in case the game
	designer did not use the g extension. */
	for_all_library_paths(p) {
		make_pathname(p->path, module->name, NULL, fullnamebuf);
		/* Now try to open the file. */
		fp = open_file(fullnamebuf, "r");
		if (fp) {
		    	return fp;
		}
	}
	return NULL;
}

int
open_module(Module *module, int dowarn)
{
    FILE *fp = NULL;

    /* Don't open more than once. */
    if (module->open) {
    	if (dowarn) {
		init_warning("Module \"%s\" is already open, ignoring attempt to reopen",
			     module->name);
	}
	return FALSE;
    }
    /* Don't open if already loaded. */
    if (module->loaded) {
    	if (dowarn) {
		init_warning("Module \"%s\" is already loaded, ignoring attempt to reload",
		     	module->name);
	}
	return FALSE;
    }
    /* Uninterpreted contents already available, init the ptr. */
    if (module->contents) {
	module->sp = module->contents;
	Dprintf("Reading module \"%s\" from string ...\n", module->name);
    /* First check if this is a saved game. */
    } else if ((fp = open_module_saved_game(module)) != NULL) {
	Dprintf("Reading module \"%s\" from saved game file \"%s\" ...\n",
		module->name, module->filename);
	module->fp = fp;
    /* Then look for a library module. */
    } else if ((fp = open_module_library_file(module)) != NULL) {
	Dprintf("Reading module \"%s\" from the library ...\n",
		module->name);
	module->fp = fp;
    } else {
	if (dowarn) {
	    if (module->name) {
	    	init_warning("Can't find module \"%s\" anywhere",
			     module->name);
	    } else {
	    	init_warning("Can't find unnamed module anywhere");
	    }
	}
	return FALSE;
    }
    /* It worked, mark this module as open. */
    module->open = TRUE;
    return TRUE;
}

void
close_module(Module *module)
{
    if (module->sp) {
	module->sp = NULL;
    }
    if (module->fp) {
	fclose(module->fp);
	module->fp = NULL;
    }
    module->open = FALSE;
}

void
interp_form(Module *module, Obj *form)
{
    Obj *thecar;
    char *name;

    /* Put the passed-in module into a global; for use in error messages. */
    curmodule = module;
    if (consp(form) && symbolp(thecar = car(form))) {
	name = c_string(thecar);
	if (Debug) {
	    /* If in a module, report the line number(s) of a form. */
	    if (module != NULL) {
		Dprintf("Line %d", module->startlineno);
		if (module->endlineno != module->startlineno)
		  Dprintf("-%d", module->endlineno);
	    }
	    Dprintf(": (%s ", name);
	    Dprintlisp(cadr(form));
	    if (cddr(form) != lispnil) {
		Dprintf(" ");
		Dprintlisp(caddr(form));
		if (cdddr(form) != lispnil)
		  Dprintf(" ...");
	    }
	    Dprintf(")\n");
	}
	switch (keyword_code(name)) {
	  case K_GAME_MODULE:
	    interp_game_module(form, module);
	    load_base_module(module);
	    break;
	  case K_UNIT_TYPE:
	    interp_utype(form);
	    break;
	  case K_MATERIAL_TYPE:
	    interp_mtype(form);
	    break;
	  case K_TERRAIN_TYPE:
	    interp_ttype(form);
	    break;
	  case K_ADVANCE_TYPE:
	    interp_atype(form);
	    break;
	  case K_TABLE:
	    interp_table(form);
	    break;
          case K_DEFINE:
	    interp_variable(form, TRUE);
	    break;
	  case K_SET:
	    interp_variable(form, FALSE);
	    break;
	  case K_UNDEFINE:
	    undefine_variable(form);
	    break;
	  case K_ADD:
	    add_properties(form);
	    break;
	  case K_WORLD:
	    interp_world(form);
	    break;
	  case K_AREA:
	    interp_area(form);
	    break;
	  case K_SIDE:
	    interp_side(form, NULL);
	    break;
	  case K_SIDE_DEFAULTS:
	    side_defaults = cdr(form);
	    break;
	  case K_DOCTRINE:
	    interp_doctrine(form);
	    break;
	  case K_INDEPENDENT_UNITS:
	    interp_side(form, indepside);
	    break;
	  case K_PLAYER:
	    /* If this is a network game we don't want to load any saved players. */
	    if (my_rid == 0) {
	    	interp_player(form);
	    }
	    break;
#if 0
	  case K_AGREEMENT:
	    interp_agreement(form);
	    break;
#endif
	  case K_SCOREKEEPER:
	    interp_scorekeeper(form);
	    break;
	  case K_EVT:
	    interp_history(form);
	    break;
	  case K_EXU:
	    interp_past_unit(form);
	    break;
	  case K_UNIT:
	    /* We must have some unit types! */
	    if (numutypes == 0)
	      load_default_game();
	    interp_unit(cdr(form));
	    break;
	  case K_UNIT_DEFAULTS:
	    interp_unit_defaults(cdr(form));
	    break;
	  case K_NAMER:
	    interp_namer(form);
	    break;
	  case K_IMF:
	    interp_imf(form);
	    break;
	  case K_INCLUDE:
	    include_module(form, module);
	    break;
	  case K_IF:
	    start_conditional(form, module);
	    break;
	  case K_ELSE:
	    start_else(form, module);
	    break;
	  case K_END_IF:
	    end_conditional(form, module);
	    break;
	  case K_PRINT:
	    print_form(cadr(form));
	    break;
	  default:
	    if (numutypes == 0)
	      load_default_game();
	    if ((boundp(thecar) && utypep(symbol_value(thecar)))
		|| utype_from_name(name) != NONUTYPE) {
		interp_unit(form);
	    } else {
		useless_form_warning(module, form);
	    }
	}
    } else {
	useless_form_warning(module, form);
    }
}

void
read_forms(Module *module)
{
    Obj *form;

    Dprintf("Trying to read a new format file \"%s\"...\n", module->name);
    if (module->fp != NULL) {
	while ((form = read_form(module->fp,
			     &(module->startlineno),
			     &(module->endlineno)))
		!= lispeof) {
	    interp_form(module, form);
	}
    } else {
	while ((form = read_form_from_string(module->sp,
			     &(module->startlineno),
			     &(module->endlineno),
			     &(module->sp)))
		!= lispeof) {
	    interp_form(module, form);
	}
    }
    /* Clean up after any print forms that might have been done. */
    end_printing_forms();
    Dprintf("... Done reading \"%s\".\n", module->name);
}

void
interp_game_module(Obj *form, Module *module)
{
    char *name = NULL, *propname, *strval = NULL;
    Obj *props = cdr(form), *bdg, *propval;

    /* This can't work if the form appears out of context (such as when
       typed in), so just escape. */
    if (module == NULL)
      return;
    /* Collect and set the module name if supplied by this form. */
    if (stringp(car(props))) {
	name = c_string(car(props));
	props = cdr(props);
    }
    if (name != NULL) {
	if (empty_string(module->name)) {
	    module->name = name;
	} else {
	    if (strcmp(name, module->name) != 0) {
		read_warning("Module name `%s' does not match declared"
			     " name `%s', ignoring declared name",
			     module->name, name);
	    }
	}
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (stringp(propval))
	  strval = c_string(propval);
	switch (keyword_code(propname)) {
	  case K_TITLE:
	    module->title = strval;
	    break;
	  case K_BLURB:
	    module->blurb = propval;
	    break;
	  case K_PICTURE_NAME:
	    module->picturename = strval;
	    break;
	  case K_BASE_MODULE:
	    module->basemodulename = strval;
	    break;
	  case K_DEFAULT_BASE_MODULE:
	    module->defaultbasemodulename = strval;
	    break;
	  case K_BASE_GAME:
	    module->basegame = strval;
	    break;
	  case K_FILENAME:
	    module->filename = strval;
	    break;
	  case K_INSTRUCTIONS:
	    /* The instructions are a list of strings. */
	    module->instructions = propval;
	    break;
	  case K_VARIANTS:
	    module->variants = interp_variant_defns(cdr(bdg));
	    break;
	  case K_NOTES:
	    /* The player notes are a list of strings. */
	    module->notes = propval;
	    break;
	  case K_DESIGN_NOTES:
	    /* The design notes are a list of strings. */
	    module->designnotes = propval;
	    break;
	  case K_VERSION:
	    module->version = strval;
	    break;
	  case K_ORIGINAL_MODULE:
	    module->origmodulename = strval;
	    break;
	  case K_ORIGINAL_VARIANTS:
	    module->origvariants = interp_variant_defns(cdr(bdg));
	    break;
	  case K_ORIGINAL_VERSION:
	    module->origversion = strval;
	    break;
	  default:
	    unknown_property("game module", module->name, propname);
	}
    }
    /* Maybe add standard variants, even if the module had no variants
       at all. */
    if (force_all_variants && module->variants == NULL)
      module->variants = interp_variant_defns(lispnil);
}

int
load_game_description(Module *module)
{
    Obj *form, *thecar;
    char *name;

    /* Module is already open, don't mess with it. */
    if (module->open)
      return FALSE;
    if (open_module(module, FALSE)) {
	if ((form = read_form(module->fp,
			      &(module->startlineno),
			      &(module->endlineno)))
	    != lispeof) {
	    if (consp(form) && symbolp(thecar = car(form))) {
		name = c_string(thecar);
		if (keyword_code(name) == K_GAME_MODULE) {
		    interp_game_module(form, module);
		    close_module(module);
		    /* Note that module is still not considered "loaded". */
		    return TRUE;
		}
	    }
	}
	close_module(module);
    }
    return FALSE;
}

void
load_game_module(Module *module, int dowarn)
{
    char ch;
    Module *module2;

    if (numutypes == 0) {
	load_game_description(module);
	if (!empty_string(module->defaultbasemodulename)) {
	    module2 = get_game_module(module->defaultbasemodulename);
	    load_game_module(module2, TRUE);
	}
    }
    if (open_module(module, dowarn)) {
	if (module->fp) {
	    /* Peek at the first character - was 'X' in old format files. */
	    ch = getc(module->fp);
	    ungetc(ch, module->fp);
	    if (ch == 'X') {
		init_error(
"\"%s\" is probably an obsolete Xconq file; in any case, it cannot be used.",
			   module->filename);
	    } else {
		/* New format, read it all. */
		read_forms(module);
	    }
	} else {
	    /* (should be able to read from contents string) */
	}
	/* We're done, can close. */
	close_module(module);
	/* Mark the module as having been loaded - note that this will
	   happen even if there were horrible errors. */
	module->loaded = TRUE;
	/* If the turn number has been set explicitly to a positive
	   value, assume that a saved game is being restored into the
	   middle of the turn. */
	if (g_turn() > 0) {
	    midturnrestore = TRUE;
	}
	/* If the random state has been set explicitly to a
	   nonnegative value, use it to reseed the generator. */
	if (g_random_state() >= 0) {
		init_xrandom(g_random_state());
	}
   }
}

void
load_base_module(Module *module)
{
    char *basename = module->basemodulename;
    Module *basemodule;

    if (!empty_string(basename)) {
	basemodule = find_game_module(basename);
	if (basemodule == NULL)
	  basemodule = add_game_module(basename, module);
	if (basemodule->loaded) {
	    Dprintf("Base module `%s' already loaded.\n", basename);
	} else {
	    Dprintf("Loading base module `%s' ...\n", basename);
	    load_game_module(basemodule, FALSE);
	    Dprintf("... Done loading `%s'.\n", basename);
	}
    }
}

void
load_default_game(void)
{
    extern char *standard_game_name;
    char *defaultname = standard_game_name;
    Module *module, *module2;

    /* If we have a different default module, use it instead. */
    if (mainmodule != NULL
	&& !empty_string(mainmodule->defaultbasemodulename)) {
    	defaultname = mainmodule->defaultbasemodulename;
    }
    module = get_game_module(defaultname);
    if (mainmodule == NULL)
      mainmodule = module;
    load_game_description(module);
    /* Recurse one level of default base module. */
    /* (should recurse indefinitely, or not?) */
    if (!empty_string(module->defaultbasemodulename)) {
	module2 = get_game_module(module->defaultbasemodulename);
	load_game_module(module2, TRUE);
    }
    load_game_module(module, TRUE);
}

static Variant *
interp_variant_defns(Obj *lis)
{
    int i = 0, len;
    Obj *varrest, *vardef, *varhead, *dflt;
    Variant *varray, *var;

    len = length(lis);
    /* Ensure space for all standard variants, whether or not listed. */
    if (force_all_variants)
      len += 10;
    /* Get out of here if no variants to be defined. */
    if (len == 0)
      return NULL;
    varray = (Variant *) xmalloc((len + 1) * sizeof(Variant));
    i = 0;
    for_all_list(lis, varrest) {
	vardef = car(varrest);
	var = varray + i;
	var->id = var->dflt = var->range = var->cases = lispnil;
	if (symbolp(vardef)) {
	    var->id = vardef;
	    /* Make the variant's name be the same as its symbol. */
	    var->name = c_string(var->id);
	} else if (consp(vardef)) {
	    varhead = car(vardef);
	    /* If a string is at the front, it is a name to use in the
	       interface - peel off and record. */
	    if (stringp(varhead)) {
		var->name = c_string(varhead);
		vardef = cdr(vardef);
	    }
	    varhead = car(vardef);
	    if (symbolp(varhead)) {
		var->id = varhead;
		if (var->name == NULL)
		  var->name = c_string(var->id);
		vardef = cdr(vardef);
	    } else {
		read_warning("variant with no symbol (#%d), ignoring",
			     len - length(varrest));
		/* Go to next list element, leaving i unchanged. */
		continue;
	    }
	    /* Pick up a help string if specified. */
	    varhead = car(vardef);
	    if (stringp(varhead)) {
		var->help = c_string(varhead);
		vardef = cdr(vardef);
	    }
	    /* Pick up a default value if specified. */
	    varhead = car(vardef);
	    if (!consp(varhead)) {
	    	var->dflt = varhead;
	    	vardef = cdr(vardef);
	    } else if (match_keyword(var->id, K_WORLD_SIZE)) {
	    	var->dflt = varhead;
	    	vardef = cdr(vardef);
	    } else if (match_keyword(var->id, K_REAL_TIME)) {
	    	var->dflt = varhead;
	    	vardef = cdr(vardef);
	    }
	    /* (should recognize and pick up a range spec if present) */
	    /* Case clauses are everything that's left over. */
	    var->cases = vardef;
	} else {
	    read_warning("bad variant (#%d), ignoring", len - length(varrest));
	    /* Go to next list element, leaving i unchanged. */
	    continue;
	}
	++i;
    }
    /* Terminate the array with an id that never appears otherwise. */
    varray[i].id = lispnil;
    /* Now add or fill in standard predefined variants. */
    add_std_var(varray, K_WORLD_SEEN, "World Seen",
		"If set, world's terrain will be seen at start of game.",
		new_number(g_terrain_seen()));
    add_std_var(varray, K_SEE_ALL, "See All",
		"If set, everything in the game is seen all the time.",
		new_number(g_see_all()));
    add_std_var(varray, K_SEQUENTIAL, "Sequential Play",
		"If set, each side moves one at a time.",
		new_number(g_use_side_priority()));
    add_std_var(varray, K_PEOPLE, "People",
		"If set, cities have populations and can grow in size.",
		new_number(g_people()));
    add_std_var(varray, K_ECONOMY, "Economy",
		"If set, economy code is enabled.",
		new_number(g_economy()));
    add_std_var(varray, K_SUPPLY, "Supply System",
		"If set, supply system code is enabled.",
		new_number(g_supply()));
    for (i = 0; varray[i].id != lispnil; ++i) {
	var = &(varray[i]);
	/* Fill in a default help string so interfaces don't have to. */
	if (var->help == NULL)
	  var->help = "(No help is available.)";
	/* Fill in the default values array so it can seed any
	   interactive tweaking code in interfaces. */
	dflt = eval(var->dflt);
	if (numberp(dflt)) {
	    var->hasintvalue = TRUE;
	    var->intvalue = c_number(dflt);
	    var->newvalues[0] = var->intvalue;
	} else if (match_keyword(var->id, K_WORLD_SIZE)) {
	    /* Pick up area width/height as a pair of numbers. */
	    if (dflt != lispnil) {
		var->newvalues[0] = c_number(car(dflt));
		var->newvalues[1] = c_number(cadr(dflt));
	    } else {
		var->newvalues[0] = DEFAULTWIDTH;
		var->newvalues[1] = DEFAULTHEIGHT;
	    }
	    /* Pick up an optional circumference. */
	    if (cddr(dflt) != lispnil) {
		var->newvalues[2] = c_number(caddr(dflt));
	    } else {
		var->newvalues[2] = DEFAULTCIRCUMFERENCE;
	    }
	} else if (match_keyword(var->id, K_REAL_TIME)) {
	    /* Defaults all go to zero. */
	    if (dflt != lispnil) {
		var->newvalues[0] = c_number(car(dflt));
		if (cdr(dflt) != lispnil) {
		    var->newvalues[1] = c_number(cadr(dflt));
		    if (cddr(dflt) != lispnil)
		      var->newvalues[2] = c_number(caddr(dflt));
		}
	    }
	}
    }
    return varray;
}

static
void
add_std_var(Variant *varray, int key, char *name, char *help, Obj *dflt)
{
    int i;
    Variant *var;

    for (i = 0; varray[i].id != lispnil; ++i) {
	var = &(varray[i]);
	/* If this variant was given already, just add supplementary
           properties as needed. */
	if (match_keyword(var->id, (enum keywords)key)) {
	    if (strcmp(c_string(var->id), var->name) == 0)
	      var->name = name;
	    if (var->help == NULL)
	      var->help = help;
	    /* (should set the default too?) */
	    return;
	}
    }
    /* Add a variant definition to the end of the array.  Note that
       we're assuming sufficient space was allocated to get away with
       this. */
    if (force_all_variants) {
	var = &(varray[i]);
	var->id = intern_symbol(keyword_name((enum keywords)key));
	var->name = name;
	var->help = help;
	var->dflt = dflt;
	var->range = lispnil;
	var->cases = lispnil;
	if (numberp(dflt)) {
	    var->hasintvalue = TRUE;
	    var->intvalue = c_number(dflt);
	    var->newvalues[0] = var->intvalue;
	}
	++i;
	varray[i].id = lispnil;
    }
}

static
void
do_one_variant(Module *module, Variant *var, Obj *varsetdata)
{
    int val, caseval;
    int width = 0, height = 0, circumference /*, latitude, longitude*/;
    int rtime, rtimeperturn, rtimeperside;
    char *vartypename = c_string(var->id);
    Obj *restcases, *headcase, *rest, *filler, *rawcaseval;

    if (Debug) {
	if (readerrbuf == NULL)
	  readerrbuf = (char *) xmalloc(BUFSIZE);
	if (varsetdata != lispnil)
	  sprintlisp(readerrbuf, varsetdata, BUFSIZE);
	else
	  sprintf(readerrbuf, "%d %d %d",
		  var->newvalues[0], var->newvalues[1], var->newvalues[2]);
    	Dprintf("Module %s variant %s being set to `%s'\n",
	    	module_desig(module), vartypename, readerrbuf);
    }
    switch (keyword_code(vartypename)) {
      case K_WORLD_SEEN:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_terrain_seen(val);
	break;
      case K_SEE_ALL:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_see_all(val);
	break;
      case K_SEQUENTIAL:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_use_side_priority(val);
	break;
      case K_PEOPLE:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_people(val);
	break;
      case K_ECONOMY:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_economy(val);
	break;
      case K_SUPPLY:
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	set_g_supply(val);
	break;
      case K_WORLD_SIZE:
      	filler = lispnil;
	if (varsetdata != lispnil) {
	    filler = varsetdata;
	}
	/* Pick the width and height out of the list. */
	if (filler != lispnil) {
	    width = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    width = var->newvalues[0];
	}
	if (filler != lispnil) {
	    height = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    height = var->newvalues[1];
	}
	/* Pick up a circumference if given. */
	if (filler != lispnil) {
	    circumference = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    circumference = var->newvalues[2];
	}
	/* This is more useful after the circumference has been set. */
	if (width > 0 && height > 0)
	  set_area_shape(width, height, TRUE);
	set_world_circumference(circumference, TRUE);
#if 0 /* (redo as separate variant) */
	/* Pick up latitude and longitude if given. */
	if (filler != lispnil) {
	    latitude = c_number(eval(car(filler)));
	    /* (should use a setter routine?) */
	    area.latitude = latitude;
	    filler = cdr(filler);
	}
	if (filler != lispnil) {
	    longitude = c_number(eval(car(filler)));
	    /* (should use a setter routine?) */
	    area.longitude = longitude;
	    filler = cdr(filler);
	}
#endif
	/* (should record settings somehow) */
	break;
      case K_REAL_TIME:
      	filler = lispnil;
	if (varsetdata != lispnil) {
	    filler = varsetdata;
	}
	if (filler != lispnil) {
	    rtime = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    rtime = var->newvalues[0];
	}
	if (filler != lispnil) {
	    rtimeperside = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    rtimeperside = var->newvalues[1];
	}
	if (filler != lispnil) {
	    rtimeperturn = c_number(eval(car(filler)));
	    filler = cdr(filler);
	} else {
	    rtimeperturn = var->newvalues[2];
	}
	/* If the values were specified, tweak the official
	   realtime globals. */
	if (rtime > 0)
	  set_g_rt_for_game(rtime);
	if (rtimeperside > 0)
	  set_g_rt_per_side(rtimeperside);
	if (rtimeperturn > 0)
	  set_g_rt_per_turn(rtimeperturn);
	/* (should record settings somehow) */
	break;
      default:
	/* This is the general case. */
	val = var->newvalues[0];
	if (varsetdata != lispnil)
	  val = c_number(eval(car(varsetdata)));
	var->hasintvalue = TRUE;
	var->intvalue = val;
	for_all_list(var->cases, restcases) {
	    headcase = car(restcases);
	    rawcaseval = eval(car(headcase));
	    if (numberp(rawcaseval))
	      caseval = c_number(rawcaseval);
	    if (numberp(rawcaseval) && caseval == val) {
		for_all_list(cdr(headcase), rest) {
		    interp_form(module, car(rest));
	    	}
	    }
	}
	/* Clean up after printing, might have been print forms in variant. */
	end_printing_forms();
	break;
    }
    /* Flag the variant as having been specified. */
    var->used = TRUE;
}

void
do_module_variants(Module *module, Obj *lis)
{
    int i, found, didsome;
    Obj *restset, *varset;
    Variant *var;

    if (module->variants == NULL)
      return; /* error? */
    didsome = FALSE;
    for_all_list(lis, restset) {
	varset = car(restset);
	found = FALSE;
	for (i = 0; module->variants[i].id != lispnil; ++i) {
	    var = &(module->variants[i]);
	    if (equal(car(varset), var->id)) {
		do_one_variant(module, var, cdr(varset));
		found = TRUE;
		didsome = TRUE;
	    }
	}
	if (!found) {
	    read_warning("Mystifying variant");
	}
    }
    /* Now implement all the defaults. */
    for (i = 0; module->variants[i].id != lispnil; ++i) {
	var = &(module->variants[i]);
	if (!var->used) {
	    do_one_variant(module, var, lispnil);
	    didsome = TRUE;
	}
    }
    if (didsome) {
	/* Recheck everything, the variants might have broken something. */
	check_game_validity();
    }
}

static
void
include_module(Obj *form, Module *module)
{
    char *name;
    Obj *mname = cadr(form);
    Module *submodule;

    SYNTAX(mname, (symbolp(mname) || stringp(mname)),
	   "included module name not a string or symbol");
    name = c_string(mname);
    Dprintf("Including \"%s\" ...\n", name);
    submodule = add_game_module(name, module);
    load_game_module(submodule, FALSE);
    if (submodule->loaded) {
    	do_module_variants(submodule, cddr(form));
    }
    Dprintf("... Done including \"%s\".\n", name);
}

static
void
start_conditional(Obj *form, Module *module)
{
    Obj *testform, *rslt;

    testform = cadr(form);
    rslt = eval(testform);
    if (numberp(rslt) && c_number(rslt) == 1) {
	actually_read_lisp = TRUE;
    } else {
	actually_read_lisp = FALSE;
    }
}

static
void
start_else(Obj *form, Module *module)
{
    /* (should match up with cond read stack) */
    actually_read_lisp = !actually_read_lisp;
}

static
void
end_conditional(Obj *form, Module *module)
{
    /* should match up with cond read stack */
    actually_read_lisp = TRUE;
}

void
init_write(void)
{
    spare_module = create_game_module("spare module");
    shortestbuf = (char *) xmalloc(BUFSIZE);
    spare_file_name = (char *) xmalloc(PATH_SIZE);
}

static
void
write_variants(Variant *varray)
{
    int i;
    Obj *rest;
    Variant *var;

    for (i = 0; varray[i].id != lispnil; ++i) {
	var = &(varray[i]);
	space_form();
	start_form("");
	/* Write the variant's string name if different from symbol. */
	if (!empty_string(var->name)
	    && !(symbolp(var->id)
		 && strcmp(var->name, c_string(var->id)) == 0)) {
	    add_to_form(escaped_string(var->name));
	    space_form();
	}
	add_form_to_form(var->id);
	if (var->dflt != lispnil) {
	    space_form();
	    add_form_to_form(var->dflt);
	}
	for_all_list(var->cases, rest) {
	    space_form();
	    add_form_to_form(car(rest));
	}
	end_form();
    }
}

int
reshape_the_output(Module *module)
{
    return (module->maybe_reshape
	    && (module->subarea_width != area.width
		|| module->subarea_height != area.height
		|| module->subarea_x != 0
		|| module->subarea_y != 0
		|| module->final_subarea_width != area.width
		|| module->final_subarea_height != area.height
		|| module->final_subarea_x != 0
		|| module->final_subarea_y != 0
		|| module->final_width != area.width
		|| module->final_height != area.height
		|| module->final_circumference != world.circumference));
}

int
write_game_module(Module *module, char *fname)
{
    if (module->filename == NULL && tmprid == 0) {
	/* (should be an error?) */
	return FALSE;
    }
    if (module->filename && fname) {
	wfp = open_file(fname, "w");
    } else {
	wfp = NULL;
    }
    if (wfp != NULL || tmprid > 0) {
	/* Write the definition of this game module. */
	start_form(key(K_GAME_MODULE));
	add_to_form(escaped_string(module->name));
	newline_form();
	space_form();
	if (module->def_all) {
	    write_str_prop(key(K_TITLE), module->title,
			   "", FALSE, TRUE);
	    write_lisp_prop(key(K_BLURB), module->blurb,
			    lispnil, FALSE, FALSE, TRUE);
	    write_str_prop(key(K_PICTURE_NAME), module->picturename,
			   "", FALSE, TRUE);
	    write_str_prop(key(K_BASE_MODULE), module->basemodulename,
			   "", FALSE, TRUE);
	    write_str_prop(key(K_DEFAULT_BASE_MODULE), module->defaultbasemodulename,
			   "", FALSE, TRUE);
	    write_str_prop(key(K_BASE_GAME), module->basegame,
			   "", FALSE, TRUE);
	    write_str_prop(key(K_FILENAME), module->filename,
			   "", FALSE, TRUE);
	    write_str_prop(key(K_VERSION), module->version,
			   "", FALSE, TRUE);
	    if (module->variants) {
		space_form();
		start_form(key(K_VARIANTS));
		write_variants(module->variants);
		end_form();
		newline_form();
	    }
	    write_str_prop(key(K_ORIGINAL_MODULE), module->origmodulename,
			   "", FALSE, TRUE);
	    if (module->origvariants) {
		space_form();
		start_form(key(K_ORIGINAL_VARIANTS));
		write_variants(module->origvariants);
		end_form();
		newline_form();
	    }
	}
	space_form();
	end_form();
	newline_form();
	newline_form();
	if (module->def_all || module->def_types)
	  write_types();
	if (module->def_all || module->def_tables)
	  write_tables(module->compress_tables);
	if (module->def_all || module->def_globals)
	  write_globals();
	if (module->def_all || module->def_scoring)
	  write_scorekeepers();
	doreshape = reshape_the_output(module);
	reshaper = module;
	if (module->def_all || module->def_world)
	  write_world();
	if (module->def_all || module->def_areas)
	  write_areas(module);
	if (module->def_all || module->def_sides)
	  write_doctrines();
	if (module->def_all || module->def_sides)
	  write_sides(module);
	if (module->def_all || module->def_players)
	  write_players();
#if 0
	if (module->def_all || module->def_agreements)
	  write_agreements();
#endif
	if (module->def_all || module->def_units)
	  write_units(module);
	if (module->def_all || module->def_history)
	  write_history();
	if (module->def_all)
	  write_namers();
	if (module->def_all)
	  write_images();
	/* Write the game notes here (seems reasonable, no deeper reason). */
	if (module->instructions != lispnil) {
	    start_form(key(K_GAME_MODULE));
	    space_form();
	    write_lisp_prop(key(K_INSTRUCTIONS), module->instructions,
			    lispnil, FALSE, FALSE, TRUE);
	    newline_form();
	    space_form();
	    space_form();
	    end_form();
	    newline_form();
	    newline_form();
	}
	if (module->notes != lispnil) {
	    start_form(key(K_GAME_MODULE));
	    space_form();
	    write_lisp_prop(key(K_NOTES), module->notes,
			    lispnil, FALSE, FALSE, TRUE);
	    newline_form();
	    space_form();
	    space_form();
	    end_form();
	    newline_form();
	    newline_form();
	}
	if (module->designnotes != lispnil) {
	    start_form(key(K_GAME_MODULE));
	    space_form();
	    write_lisp_prop(key(K_DESIGN_NOTES), module->designnotes,
			    lispnil, FALSE, FALSE, TRUE);
	    newline_form();
	    space_form();
	    space_form();
	    end_form();
	    newline_form();
	    newline_form();
	}
	if (wfp != NULL)
	  fclose(wfp);
	else
	  flush_write();
	return TRUE;
    } else {
	return FALSE;
    }
}

int
write_entire_game_state(char *fname)
{
    Module *module;
    int rslt;

    /* Record the attempt to save state.  Do it before actually saving,
       so that it appears in the saved history. */
    if (!memory_exhausted && !debugging_state_sync)
      record_event(H_GAME_SAVED, ALLSIDES);
    /* No additional allocation should ever happen during saving,
       so complain if it does. */
    xmalloc_warnings = TRUE;
    module = spare_module;
    copy_module(module, mainmodule);
    module->name = mainmodule->name;
    module->filename = find_name(fname);
    module->version = NULL;
    module->variants = NULL;
    /* We write all types etc, so suppress base module load attempts. */
    module->basemodulename = NULL;
    module->defaultbasemodulename = NULL;
    module->compress_tables = TRUE;
    module->compress_layers = TRUE;
    module->def_all = TRUE;
    rslt = write_game_module(module, fname);
    xmalloc_warnings = FALSE;
    /* Record that the game's state is accurately saved away. */
    if (rslt) {
	gamestatesafe = TRUE;
	if (!memory_exhausted && !debugging_state_sync) {
	    /* Report success to the users.  We can't use notify_event
	       because we want to report the filename. */
	    notify_all("Game was saved to \"%s\".", fname);
	}
    }
    return rslt;
}

#if (0) // Temp disable.

/* Empty out the list of modules. */

void
clear_game_modules(void)
{
    modulelist = mainmodule = NULL;
}

/* Read info about a side's preferences and setup. */

/* This assumes one form only, probably too restrictive. */
/* should read all the forms, use the relevant ones. */
/* (how does this interact with other defaults?) */
/* (should be delayed until player can confirm it...) */

/* (update to work like other module stuff? then can use resources etc) */
/* (fix so that correct name can be found reliably) */

int
load_side_config(Side *side)
{
#if 0
    FILE *fp;
    Obj *config;
    Module *module;

    /* (should incorp config name somehow, also be sys-dependent) */
    module = create_game_module(side->player->name);

    /* Don't use fopen directly, use open_module which indirectly calls
    open_file to ensure mac/unix cross-platform compatibility. */
    if (open_module(module, FALSE)) {
	if ((config = read_form(module->fp,
				&(module->startlineno),
				&(module->endlineno)))
	    != lispeof) {
	    /* interpret the config */
	    Dprintf("Interpreting %s config form", side_desig(side));
	    Dprintlisp(config);
	    Dprintf("\n");
	    fill_in_side(side, config, TRUE);
	} else {
	    /* no config form in the file */
	}
    } else {
	init_warning("Module \"%s\" could not be opened", module->name);
	/* Not a disaster, keep going */
    }
#endif
    return FALSE;
}

void
set_variant_value(int which, int v1, int v2, int v3)
{
    Variant *var;

    var = &(mainmodule->variants[which]);
    var->newvalues[0] = v1;
    var->newvalues[1] = v2;
    var->newvalues[2] = v3;
}

void
init_module_reshape(Module *module)
{
    /* Seed all the reshaping parameters with reasonable values. */
    module->maybe_reshape = TRUE;
    module->subarea_width = area.width;
    module->subarea_height = area.height;
    module->subarea_x = module->subarea_y = 0;
    module->final_subarea_width = area.width;
    module->final_subarea_height = area.height;
    module->final_subarea_x = module->final_subarea_y = 0;
    module->final_width = area.width;  module->final_height = area.height;
    module->final_circumference = world.circumference;
}

/* Check if the proposed reshape will actually work. */

int
valid_reshape(Module *module)
{
    /* (should check hexagon shaping) */
    if (module->subarea_width > area.width
	|| module->subarea_height > area.height)
      return FALSE;
    /* (should check other offsets) */
    if (module->final_width < 3 || module->final_height < 3)
      return FALSE;
    return TRUE;
}

char *
saved_game_filename(void)
{
    char *str, name[BUFSIZE];

    sprintf(name, "%s-%d.xcq", mainmodule->name, g_turn());
    str = game_filename("XCONQSAVEFILE", name);
    return str;
}

char *
checkpoint_filename(int n)
{
    char *str, name[BUFSIZE];

    sprintf(name, "check%d.xcq", n);
    str = game_filename(NULL, name);
    return str;
}

char *
statistics_filename(void)
{
    /* No need to cache name, will only get requested once. */
    return game_filename("XCONQSTATSFILE", STATSFILE);
}

char *
preferences_filename(void)
{
    return game_filename("XCONQPREFERENCES", PREFERENCESFILE);
}

/* Legacy filename support. */
char *
old_preferences_filename(void)
{
    return game_filename("XCONQPREFERENCES", OLD_PREFERENCESFILE);
}

#endif // Temp disable.

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

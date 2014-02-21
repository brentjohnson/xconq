// xConq
// Game Module Objects and Functions

// $Id: module.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

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
    \brief Game module objects and functions.
    \ingroup grp_gdl
    \todo Move synthesis methods to more appropriate location.
*/

#ifndef XCONQ_GDL_MODULE_H
#define XCONQ_GDL_MODULE_H

#include "gdl/lisp.h"

// Iterator Macros

//! Iterate over modules.
#define for_all_modules(m)  \
  for (m = modulelist; m != NULL; m = m->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Game module variant.
/*! A variant describes an option that is available to players starting
    up a game, as well as modules including each other.
*/
typedef struct a_variant {
    //! Unique ID.
    Obj *id;
    //! Displayable name.
    char *name;
    //! Help string.
    char *help;
    //! Default GDL object value.
    Obj *dflt;
    //! Range of values.
    Obj *range;
    //! Actions to do on matches.
    Obj *cases;
    //! True, if variant has been set already.
    int used;
    //! True, if variant has integer value.
    int hasintvalue;
    //! Integer value of variant.
    int intvalue;
    //! New values not yet acted on.
    int newvalues[3];
} Variant;

//! Game module.
/*!
    A file module records relevant info about the module, what it included,
    how to write it out, etc.
*/
typedef struct a_module {
    //! Name of module.
    char *name;
    //! Displayable title.
    char *title;
    //! Game blurb.
    Obj *blurb;
    //! Name of associated picture.
    char *picturename;
    //! Name of module that this one includes/inherits from.
    char *basemodulename;
    //! Name of default module to include, if something missing.
    char *defaultbasemodulename;
    //! Name of game that this one is based on.
    char *basegame;
    //! GDL object containing game instructions.
    Obj *instructions;
    //! GDL object containing game notes.
    Obj *notes;
    //! GDL object containing game design notes.
    Obj *designnotes;
    //! Version string.
    char *version;
    //! Array of variants for module.
    Variant *variants;
    //! Name of pure module before game save.
    char *origmodulename;
    //! Array of variants from pure module before game save.
    Variant *origvariants;
    //! Version string from original module.
    char *origversion;
    //! Module contents.
    char *contents;
    //! Pointer into string.
    char *sp;
    //! Module's ilename.
    char *filename;
    //! Pointer into file buffer.
    FILE *fp;
    //! Line number at start of form.
    int startlineno;
    //! Line number at end of form.
    int endlineno;
    /* Primarily for designer use, to control writing of a module */
    //! True, if all data is to be written out.
    short def_all;
    //! True, if all types are to be written out.
    short def_types;
    //! True, if all tables are to be written out.
    short def_tables;
    //! True, if tables are to be written out in compressed form.
    short compress_tables;
    //! True, if GDL globals are to written out.
    short def_globals;
    //! True, if scores and scorekeepers are to be written out.
    short def_scoring;
    //! True, if world is to be written out.
    short def_world;
    //! True, if area is to be written out.
    short def_areas;
    //! True, if terrain layer is to be written out.
    short def_area_terrain;
    //! True, if other layers are to be written out.
    short def_area_misc;
    //! True, if area weather is to be written out.
    short def_area_weather;
    //! True, if area materials are to be written out.
    short def_area_material;
    //! True, if layers are to be written out in compressed form.
    short compress_layers;
    //! True, if sides are to be written out.
    short def_sides;
    //! True, if side views of area are to be written out.
    short def_side_views;
    //! True, if side doctrines are to be written out.
    short def_side_doctrines;
    //! True, if players are to be written out.
    short def_players;
#if 0
    //! True, if agreements are to be written out.
    short def_agreements;
#endif
    //! True, if units are to be written out.
    short def_units;
    //! True, if unit ID references are to be written out.
    short def_unit_ids;
    //! True, if unit properties are to be written out.
    short def_unit_props;
    //! True, if unit actions are to be written out.
    short def_unit_acts;
    //! True, if unit plans are to be written out.
    short def_unit_plans;
    //! True, if game history is to be written out.
    short def_history;
    //! True, if going to save diff shape of area.
    short maybe_reshape;
    //! Subarea width.
    int subarea_width;
    //! Subarea height.
    int subarea_height;
    //! Subarea x offset.
    int subarea_x;
    //! Subarea y offset.
    int subarea_y;
    //! Final subarea width.
    int final_subarea_width;
    //! Final subarea height.
    int final_subarea_height;
    //! Final subarea x offset.
    int final_subarea_x;
    //! Final subarea y offset.
    int final_subarea_y;
    //! Final width.
    int final_width;
    //! Final height.
    int final_height;
    //! Final circumference.
    int final_circumference;
    //! True, if currently open.
    short open;
    //! True, if already loaded.
    short loaded;
    //! Next module.
    struct a_module *next;
    //! List of included modules.
    struct a_module *include;
    //! Next included sibling module.
    struct a_module *nextinclude;
    //! Last included module.
    struct a_module *lastinclude;
} Module;

// Global Variables

//! List of modules.
/*!
    List of all known modules. Their descriptions can co-exist in memory,
    even if their contents cannot.
*/
extern Module *modulelist;

//! Module from which forms are being read and interpreted.
extern Module *curmodule;

//! File being written to.
extern FILE *wfp;

//! Module used for reshaping game area output.
extern Module *reshaper;

// Global Variables: Validation

//! Does advances grpah have cycles?
extern int G_advances_graph_has_cycles;

// Global Variables: Game State

//! True, if game in safe state for saving.
extern int gamestatesafe;

//! True, if game will start in middle of a turn.
extern int midturnrestore;

// Global Variables: Game Setup

//! Maximum zone of control range.
extern int max_zoc_range;

//! Table of advances dependencies.
extern PackedBoolTable *G_advances_synopsis;

// Global Variables: Buffers

//! Buffer for shortest names.
extern char *shortestbuf;

// Queries

//! Imprint into buffer the module name and line number range being parsed.
extern void module_and_line(Module *module, char *buf);

//! Description of module.
extern char *module_desig(Module *module);

// Synthesis Methods

//! Synthesize random date.
extern int make_random_date(int calls, int runs);
//! Synthesize weather.
extern int make_weather(int calls, int runs);
//! Synthesize initial materials.
extern int make_initial_materials(int calls, int runs);
//! Synthesize rivers.
extern int make_rivers(int calls, int runs);
//! Syntehsize roads.
extern int make_roads(int calls, int runs);
//! Synthesize fractal terrain.
extern int make_fractal_terrain(int calls, int runs);
//! Synthesize random terrain.
extern int make_random_terrain(int calls, int runs);
//! Synthesize Earth-like terrain.
extern int make_earthlike_terrain(int calls, int runs);
//! Synthesize maze terrain.
extern int make_maze_terrain(int calls, int runs);
//! Synthesize names for geographical features.
extern int name_geographical_features(int calls, int runs);
//! Synthesize random names for units.
extern int name_units_randomly(int calls, int runs);
//! Synthesize side countries.
extern int make_countries(int calls, int runs);
//! Synthesize independent units.
extern int make_independent_units(int calls, int runs);

// Validation

//! Check to make sure loaded game is valid.
/*!
    Run a doublecheck on plausibility of game parameters.  Additional
    checks are performed elsewhere as needed, for instance during
    random generation.  Serious mistakes exit now, since they can cause
    all sorts of strange behavior and core dumps.  It's a little more
    friendly to only exit at the end of the tests, so all the mistakes
    can be found at once.
    In theory, if a game passes these tests, then Xconq will never crash.
*/
extern void check_game_validity(void);

// Lifecycle Management

//! Create a new game module in memory.
extern Module *create_game_module(char *name);
//! Copy properties of original module into target module.
extern void copy_module(Module *module, Module *origmodule);

// Game Setup

//! Locate module with given name.
extern Module *find_game_module(char *name);
//! Include a module of given name within module.
extern Module *add_game_module(char *name, Module *includer);
//! Produce a module of given name, either by finding or creating it.
extern Module *get_game_module(char *name);

// GDL I/O

//! Given module, attempt to open it.
extern int open_module(Module *module, int dowarn);
//! Close module.
extern void close_module(Module *module);

//! Read entire module file, interpreting all forms in it.
/*!
    \note Does this interp game-module form twice if description previously
	  loaded?
*/
extern void read_forms(Module *module);
//! Basic interpreter for form appearing in a module.
extern void interp_form(Module *module, Obj *form);

//! Parse module form.
extern void interp_game_module(Obj *form, Module *module);

//! Attempt to load first form in module.
/*! Attempt to read just the first form in a module and use it as a
    description of the module.  Return true if this worked, false otherwise.
*/
extern int load_game_description(Module *module);

//! Load game module.
/*!
    Game files can live in library directories or somewhere else.
    This function tries to find a file, open it, and load the contents.
*/
extern void load_game_module(Module *module, int dowarn);
//! Load base module for game.
extern void load_base_module(Module *module);
//! Load default game module.
/*!
    Sometimes we find ourselves lacking a game to provide meaning and
    context for interpretation; this routine loads the standard game
    (or a specified alternative default) immediately, but only makes
    it the main module if none defined.
*/
extern void load_default_game(void);

//! Setup list of variants for given module.
/*!
    Interpret the given list of variants, using the given list to
    fill in values not already supplied in the variant.
*/
extern void do_module_variants(Module *module, Obj *lis);

//! Preparation and preallocation for writing.
extern void init_write(void);

/*
    NOTE: 'write_entire_game_state' is forward declared in an "earlier"
    namespace. The declaration eventually should be moved here.
*/

//! True, if any actual reshaping is required.
extern int reshape_the_output(Module *module);

//! Serialize game module to GDL.
/*!
    Given a game module telling what is in the module, write out a file
    containing the requested content.  Return true if all went OK.
*/
extern int write_game_module(Module *module, char *fname);

#if (0) // Temp disable.

/*! \brief Iterate through included modules.
 *
 * Iteration over the list of \ref a_module "modules" included in a module.
 * \param m is the module to iterate through the included moudles.
 * \param sub is the sub module iteration variable.
 */
#define for_all_includes(m,sub)  \
  for (sub = m->include; sub != NULL; sub = sub->nextinclude)

/*! \brief Main \ref a_module "module". */
extern Module *mainmodule;

/* Declarations of module functions. */

extern void clear_game_modules(void);
extern void init_module_reshape(Module *module);
extern int valid_reshape(Module *module);
extern char *saved_game_filename(void);
extern char *checkpoint_filename(int n);
extern char *statistics_filename(void);
extern char *preferences_filename(void);
extern char *old_preferences_filename(void);

#endif // Temp disable.

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_MODULE_H

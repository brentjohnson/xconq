/* Definitions for game modules in Xconq.
   Copyright (C) 1991-1996, 1999, 2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/module.h
 * \brief Definitions for game modules in XConq.
 */

/*! \brief Variant.
 *
 * A variant describes an option that is available to players starting
 * up a game, as well as modules including each other.
 */
typedef struct a_variant {
    Obj *id;            	/*!< unique id */
    const char *name;         	/*!< displayable name */
    const char *help;         	/*!< help string */
    Obj *dflt;          	/*!< pointer to default value \Object */
    Obj *range;         	/*!< description of range of values */
    Obj *cases;         	/*!< actions to do on matches */
    int used;           	/*!< true if the variant has been set to a value already */
    int hasintvalue;    	/*!< true if integer value defined */
    int intvalue;       	/*!< integer value of the variant */
    int newvalues[3];	/*!< new values not yet acted on */
} Variant;

/*! \brief Module.
 *
 * A file module records relevant info about the module, what it included,
 * how to write it out, etc. 
 */
typedef struct a_module {
    const char *name;                     		/*!< the actual unique name of the module */
    const char *title;                    		/*!< a readable display name */
    Obj *blurb;              		/*!< game blurb */
    const char *picturename;              	/*!< name of a descriptive image */
    const char *basemodulename;           	/*!< name of the module that this one includes */
    const char *defaultbasemodulename;	/*!< what game to load if something missing */
    const char *basegame;                 	/*!< what general game this is based on */
    Obj *instructions;              	/*!< basic instructions */
    Obj *notes;                     		/*!< player notes */
    Obj *designnotes;               	/*!< designer notes */
    const char *version;                  		/*!< the version of this module */
    Variant *variants;              	/*!< array of player-choosable variants */
    const char *origmodulename;           	/*!< module this was originally (before save) */
    Variant *origvariants;          	/*!< variants chosen for the original module */
    const char *origversion;              	/*!< the version of the original module */
    char *contents;                 		/* a string with the actual contents */
    const char *sp;                    		/*!< "string pointer" a la file pointer */
    const char *filename;                 	/*!< the filename */
    FILE *fp;                       		/*!< the stdio file buffer */
    int startlineno;                		/*!< line number being read at start of form */
    int endlineno;                  		/*!< line number being read at end of form */
    /* Primarily for designer use, to control writing of a module */
    short def_all;                  		/*!< true if all data is to be written out (define all). */
    short def_types;                	/*!< define types. */
    short def_tables;               	/*!< define tables. */
    short compress_tables;          	/*!< true if tables should be compressed when written */
    short def_globals;              	/*!< define globals. */
    short def_scoring;              	/*!< define scoring. */
    short def_world;                	/*!< define world. */
    short def_areas;                	/*!< define areas. */
    short def_area_terrain;         	/*!< define area terrain. */
    short def_area_misc;            	/*!< define area miscellaneous. */
    short def_area_weather;         	/*!< define area weather. */
    short def_area_material;        	/*!< define area materials. */
    short compress_layers;          	/*!< true if layer data should be compressed when written */
    short def_sides;                		/*!< define sides. */
    short def_side_views;           	/*!< define side views. */
    short def_side_doctrines;       	/*!< define side doctrines. */
    short def_players;              	/*!< define players. */
#if 0
    short def_agreements;           	/*!< define agreements. */
#endif
    short def_units;                		/*!< define units. */
    short def_unit_ids;             	/*!< define unit ids. */
    short def_unit_props;           	/*!< define unit properties. */
    short def_unit_acts;            	/*!< define unit actions. */
    short def_unit_plans;           	/*!< define unit plans. */
    short def_history;              	/*!< define history. */
    short maybe_reshape;            	/*!< true if going to save diff shape of area */
    int subarea_width;              	/*!< Syb Area wudth. */
    int subarea_height;             	/*!< Sub Area height. */
    int subarea_x;                  		/*!< Sub Area x. */
    int subarea_y;                  		/*!< Sub Area y. */
    int final_subarea_width;        	/*!< Final Sub Area Width. */
    int final_subarea_height;       	/*!< Final Sub Area Height. */
    int final_subarea_x;            	/*!< Final Sub Area x. */
    int final_subarea_y;            	/*!< Final Sub Area y. */
    int final_width;                		/*!< Final width. */
    int final_height;               		/*!< Final height. */
    int final_circumference;        	/*!< Final circumference. */
    short open;                     		/*!< true if currently open */
    short loaded;                   		/*!< true if already loaded */
    struct a_module *next;          	/*!< pointer to next module */
    struct a_module *include;       	/*!< pointer to first included module */
    struct a_module *nextinclude;	/*!< pointer to next included module */
    struct a_module *lastinclude;	/*!< pointer to last included module */
} Module;

/*! \brief Iterate modules.
 *
 * Iteration over the list of \ref a_module "modules".
 * \param m is the module iteration variable.
 */
#define for_all_modules(m)  \
  for (m = modulelist; m != NULL; m = m->next)

/*! \brief Iterate through included modules.
 *
 * Iteration over the list of \ref a_module "modules" included in a module.
 * \param m is the module to iterate through the included moudles.
 * \param sub is the sub module iteration variable.
 */
#define for_all_includes(m,sub)  \
  for (sub = m->include; sub != NULL; sub = sub->nextinclude)

/*! \brief List of \ref a_module "Modules". */
extern Module *modulelist;
/*! \brief Main \ref a_module "module". */
extern Module *mainmodule;

/* Declarations of module functions. */

extern void clear_game_modules(void);
extern Module *create_game_module(const char *name);
extern Module *find_game_module(const char *name);
extern Module *get_game_module(const char *name);
extern Module *add_game_module(const char *name, Module *includer);
extern void load_default_game(void);
extern int load_game_description(Module *module);
extern void load_game_module(Module *module, int dowarn);
extern void load_base_module(Module *module);
extern int open_module(Module *module, int dowarn);
extern void read_forms(Module *module);
extern void copy_module(Module *module, Module *origmodule);
extern void init_module_reshape(Module *module);
extern int reshape_the_output(Module *module);
extern int valid_reshape(Module *module);
extern void close_module(Module *module);
extern char *module_desig(Module *module);
extern void do_module_variants(Module *module, Obj *lis);
extern const char *saved_game_filename(void);
extern const char *checkpoint_filename(int n);
extern const char *statistics_filename(void);
extern const char *preferences_filename(void);
extern const char *old_preferences_filename(void);



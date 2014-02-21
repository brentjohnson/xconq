/* Game modules for Xconq.
   Copyright (C) 1991-1996, 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"

extern char *readerrbuf;

static void do_one_variant(Module *module, struct a_variant *var, Obj *varsetdata);
static FILE *open_module_library_file(Module *module);
static FILE *open_module_saved_game(Module *module);

/* List of all known modules. Their descriptions can co-exist in memory,
   even if their contents cannot. */

Module *modulelist;

/* The main module defining the game in effect. */

Module *mainmodule;

char *moduledesigbuf = NULL;

/* Empty out the list of modules. */

void
clear_game_modules(void)
{
    modulelist = mainmodule = NULL;
}

/* Create a brand-new game module. */

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


/* Produce a module of the given name, either by finding it or creating it. */

Module *
get_game_module(char *name)
{
    Module *module = find_game_module(name);

    if (module != NULL)
      return module;
    return create_game_module(name);
}

/* Make a game module for the given name and maybe bolt it into the include
   list of another module. */

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

/* Sometimes we find ourselves lacking a game to provide meaning and
   context for interpretation; this routine loads the standard game
   (or a specified alternative default) immediately, but only makes
   it the main module if none defined. */

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

/* Attempt to read just the first form in a module and use it as a
   description of the module.  Return true if this worked, false
   otherwise. */

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

/* Game files can live in library directories or somewhere else.  This
   function tries to find a file, open it, and load the contents. */

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
		init_error("\"%s\" is probably an obsolete Xconq file; in any case, it cannot be used.",
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

/* Given a module, attempt to open it. */

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

/* Attempt to open a saved game. */

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

/* Attempt to open a library file. */

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

/* Read an entire file, attempting to pick up objects in it. */

/* (does this interp game-module form twice if description previously
   loaded?) */

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
set_variant_value(int which, int v1, int v2, int v3)
{
    Variant *var;

    var = &(mainmodule->variants[which]);
    var->newvalues[0] = v1;
    var->newvalues[1] = v2;
    var->newvalues[2] = v3;
}

/* Interpret the given list of variants, using the given list to
   fill in values not already supplied in the variant. */

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

/* Implement the effect of the single given variant. */

static void
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

/* Given a module and a original, copy over properties of the original
   module. */

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

/* This is true if any actual reshaping is required. */

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

/* Close the module. */

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

/* Return a description of the module. */

char *
module_desig(Module *module)
{
    if (moduledesigbuf == NULL)
      moduledesigbuf = (char *)xmalloc(BUFSIZE);
    sprintf(moduledesigbuf, "module %s (%s)",
	    module->name, (module->title ? module->title : "no title"));
    return moduledesigbuf;
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

/* (random code below, should be sent to better places) */

/* If a special symbol, we might not have to fail.  Note that although
   this looks like Lisp-level code, it knows about unit types and so
   forth, so is higher-level. */

int
lazy_bind(Obj *sym)
{
    int u, m, t, a;
    Obj *value;

    switch (keyword_code(c_string(sym))) {
      case K_USTAR:
	value = lispnil;
	/* Since consing glues onto the front, iterate backwards
	   through the types. */
	for (u = numutypes - 1; u >= 0; --u)
	  value = cons(new_utype(u), value);
	break;
      case K_MSTAR:
	value = lispnil;
	for (m = nummtypes - 1; m >= 0; --m)
	  value = cons(new_mtype(m), value);
	break;
      case K_TSTAR:
	value = lispnil;
	for (t = numttypes - 1; t >= 0; --t)
	  value = cons(new_ttype(t), value);
	break;
      case K_ASTAR:
	value = lispnil;
	for (a = numatypes - 1; a >= 0; --a)
	  value = cons(new_atype(a), value);
	break;
      default:
	return FALSE;
    }
    setq(sym, value);
    return TRUE;
}

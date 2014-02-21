/* Interpretation of Xconq GDL.
   Copyright (C) 1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Syntax is el cheapo Lisp. */

#include "conq.h"
#include "kernel.h"
#include "imf.h"

extern int actually_read_lisp;

extern int nextsideid;

static void module_and_line(Module *module, char *buf);
static void init_constant(int key, int val);
static void init_self_eval(int key);
static void useless_form_warning(Module *module, Obj *form);
static void include_module(Obj *form, Module *module);
static void start_conditional(Obj *form, Module *module);
static void start_else(Obj *form, Module *module);
static void end_conditional(Obj *form, Module *module);
static Variant *interp_variant_defns(Obj *lis);
static void add_std_var(Variant *varray, int key, char *name, char *help,
			Obj *dflt);
static void interp_utype(Obj *form);
static void fill_in_utype(int u, Obj *list);
static int set_utype_property(int u, char *propname, Obj *val);
static void interp_mtype(Obj *form);
static void fill_in_mtype(int m, Obj *list);
static void interp_ttype(Obj *form);
static void fill_in_ttype(int t, Obj *list);
static void interp_atype(Obj *form);
static void fill_in_atype(int s, Obj *list);

static void interp_table(Obj *form);
static void add_to_table(Obj *tablename, int tbl, Obj *clauses);
static void interp_one_clause(Obj *tablename, int tbl, int lim1, int lim2,
			      Obj *indexes1, Obj *indexes2, Obj *values);
static void interp_variable(Obj *form, int isnew);
static void undefine_variable(Obj *form);
static void add_properties(Obj *form);
static int list_lengths_match(Obj *types, Obj *values, char *formtype,
			      Obj *form);
static void add_to_utypes(Obj *types, Obj *prop, Obj *values);
static void add_to_mtypes(Obj *types, Obj *prop, Obj *values);
static void add_to_ttypes(Obj *types, Obj *prop, Obj *values);
static void add_to_atypes(Obj *types, Obj *prop, Obj *values);
static void interp_world(Obj *form);
static void interp_area(Obj *form);
static void add_to_area(Obj *form);
static void fill_in_terrain(Obj *contents);
static void fill_in_aux_terrain(Obj *contents);
static void fill_in_people_sides(Obj *contents);
static void fill_in_control_sides(Obj *contents);
static void fill_in_features(Obj *contents);
static void fill_in_elevations(Obj *contents);
static void fill_in_cell_material(Obj *contents);
static void fill_in_temperatures(Obj *contents);
static void fill_in_winds(Obj *contents);
static void fill_in_clouds(Obj *contents);
static void fill_in_cloud_bottoms(Obj *contents);
static void fill_in_cloud_heights(Obj *contents);
static void fill_in_users(Obj *contents);

static void interp_side(Obj *form, Side *side);
static void interp_unit_views(Side *side, Obj *vlist);
static void interp_standing_order(Side *side, Obj *form);
static void check_name_uniqueness(Side *side, char *str, char *kind);
static void merge_unit_namers(Side *side, Obj *lis);
static void interp_side_value_list(short *arr, Obj *lis);
static void interp_side_mask_list(SideMask *mask, Obj *lis);
static void interp_atkstats_list(Side *side, Obj *lis);
static void interp_hitstats_list(Side *side, Obj *lis);
static void read_view_layer(Side *side, Obj *contents,
			    void (*setter)(int, int, int));
static void read_aux_terrain_view_layer(Side *side, Obj *contents,
					void (*setter)(int, int, int));
static void read_material_view_layer(Side *side, Obj *contents,
				     void (*setter)(int, int, int));
static void read_default_doctrine(Side *side, Obj *props);
static void read_utype_doctrine(Side *side, Obj *list);
static void interp_doctrine(Obj *form);
static void fill_in_doctrine(struct a_doctrine *doctrine, Obj *props);
static void interp_player(Obj *form);
static void fill_in_player(struct a_player *player, Obj *props);
#if 0
static void interp_agreement(Obj *form);
#endif
static void interp_unit_defaults(Obj *form);
static Unit *interp_unit(Obj *form);
static Obj *find_unit_spec_by_name(char *name);
static Obj *find_unit_spec_by_number(int num);
static void interp_action(Action *action, Obj *props);
static void interp_unit_plan(Unit *unit, Obj *props);
static Task *interp_task(Obj *form);
static Goal *interp_goal(Obj *form);
static void interp_namer(Obj *form);
static void interp_scorekeeper(Obj *form);
static void interp_history(Obj *form);
static void interp_past_unit(Obj *form);

static void unknown_property(char *type, char *inst, char *name);

static int lookup_goal_type(char *name);
static int lookup_action_type(char *name);

static void set_u_internal_name(int u, char *s);
static void set_u_type_name(int u, char *s);
static void set_m_type_name(int m, char *s);
static void set_t_type_name(int t, char *s);
static void set_a_type_name(int a, char *x);
static int lookup_plan_type(char *name);

static void interp_utype_list(short *arr, Obj *lis);
static void interp_utype_value_list(short *arr, Obj *lis);
static void interp_mtype_value_list(short *arr, Obj *lis);
static void interp_treasury_list(long *arr, Obj *lis);
static void interp_atype_value_list(short *arr, Obj *lis);
static void read_layer(Obj *contents, void (*setter)(int, int, int));
static void read_rle(Obj *contents, void (*setter)(int, int, int), short *chartable);

/* This is true if all variants should be made available, irrespective
   of the game designer's spec. */

int force_all_variants = FALSE;

/* This is the module from which forms are being read and
   interpreted, if they are coming from a module. */

Module *curmodule;

Obj *cond_read_stack;

/* True if game will start up in the middle of a turn. */

int midturnrestore = FALSE;

/* The count of cells that did not have valid terrain data. */

int numbadterrain = 0;

/* True if should warn about bad terrain. */

int warnbadterrain = TRUE;

char *readerrbuf;

/* This is the list of side defaults that will be applied
   to all sides read subsequently. */

Obj *side_defaults;

/* Defaults to use for filling in unit properties. */

int uxoffset = 0, uyoffset = 0;

static short default_unit_side_number = -1;

static short default_unit_origside_number = -1;

static short default_unit_cp = -1;

static short default_unit_hp = -1;

static short default_unit_cxp = -1;

static short default_unit_z = -1;

static short default_transport_id = -1;

static short *default_tooling;

Obj *unit_specs;

Obj *last_unit_spec;

Obj *default_unit_spec;

/* Globals used to communicate with the RLE reader. */

short layer_use_default;
int layer_default;
int layer_multiplier;
int layer_adder;
short layer_area_x, layer_area_y;
short layer_area_w, layer_area_h;

int ignore_specials;

/* This is the table of keywords. */

struct a_key {
    char *name;
    short key;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,CODE)  { NAME, CODE },

#include "keyword.def"

    { NULL, 0 }
};

/* Given a string, return the enum of the matching keyword,
   if found, else -1. */

int
keyword_code(char *str)
{
    int i;

    /* (should do a binary search first, then switch to exhaustive) */
    for (i = 0; keywordtable[i].name != NULL; ++i) {
	if (strcmp(str, keywordtable[i].name) == 0)
	  return keywordtable[i].key;
    }
    return (-1);
}

char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

#define TYPEPROP(TYPES, N, DEFNS, I, TYPE)  \
  ((TYPE *) &(((char *) (&(TYPES[N])))[DEFNS[I].offset]))[0]

/* This is a generic syntax check and escape. */

#define SYNTAX(X,TEST,MSG)  \
  if (!(TEST)) {  \
      syntax_error((X), (MSG));  \
      return;  \
  }
  
#define SYNTAX_RETURN(X,TEST,MSG,RET)  \
  if (!(TEST)) {  \
      syntax_error((X), (MSG));  \
      return (RET);  \
  }

void  
syntax_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    read_warning("syntax error in `%s' - %s", readerrbuf, msg);
}

/* This is specifically for typechecking. */

#define TYPECHECK(PRED,X,MSG)  \
  if (!PRED(X)) {  \
      type_error((X), (MSG));  \
      return;  \
  }

#define TYPECHECK_RETURN(PRED,X,MSG,RET)  \
  if (!PRED(X)) {  \
      type_error((X), (MSG));  \
      return (RET);  \
  }

void
type_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    read_warning("type error in `%s' - %s", readerrbuf, msg);
}

/* Parse the (propertyname value) lists that most forms use. */

#define PARSE_PROPERTY(BDG,NAME,VAL)  \
  SYNTAX(BDG, (consp(BDG) && symbolp(car(BDG))), "property binding");  \
  (NAME) = c_string(car(BDG));  \
  (VAL) = cadr(BDG);

#define PARSE_PROPERTY_RETURN(BDG,NAME,VAL,RET)  \
  SYNTAX_RETURN(BDG, (consp(BDG) && symbolp(car(BDG))), "property binding", (RET));  \
  (NAME) = c_string(car(BDG));  \
  (VAL) = cadr(BDG);

static void
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

static void
init_constant(int key, int val)
{
    Obj *sym = intern_symbol(keyword_name((enum keywords)key));

    setq(sym, new_number(val));
    flag_as_constant(sym);
}

static void
init_self_eval(int key)
{
    Obj *sym = intern_symbol(keyword_name((enum keywords)key));

    setq(sym, sym);
    flag_as_constant(sym);
}

void
init_predefined_symbols(void)
{
    /* Predefined constants. */
    init_constant(K_FALSE, FALSE);
    init_constant(K_TRUE, TRUE);
    init_constant(K_NON_UNIT, -1);
    init_constant(K_NON_MATERIAL, -1);
    init_constant(K_NON_TERRAIN, -1);
    init_constant(K_NON_ADVANCE, -1);
    init_constant(K_CELL, 0);
    init_constant(K_BORDER, 1);
    init_constant(K_CONNECTION, 2);
    init_constant(K_COATING, 3);
    init_constant(K_RIVER_X, 10);
    init_constant(K_VALLEY_X, 11);
    init_constant(K_ROAD_X, 12);
    init_constant(K_OVER_NOTHING, 0);
    init_constant(K_OVER_OWN, 1);
    init_constant(K_OVER_BORDER, 2);
    init_constant(K_OVER_ALL, 3);
    init_constant(K_WIN, 1);
    init_constant(K_LOSE, -1);
    init_constant(K_DRAW, 0);
    /* Random self-evaluating symbols. */
    init_self_eval(K_AND);
    init_self_eval(K_OR);
    init_self_eval(K_NOT);
    init_self_eval(K_RESET);
    init_self_eval(K_USUAL);
    /* Leave these unbound so that first ref computes correct list. */
    intern_symbol(keyword_name(K_USTAR));
    intern_symbol(keyword_name(K_MSTAR));
    intern_symbol(keyword_name(K_TSTAR));
    intern_symbol(keyword_name(K_ASTAR));

    /* These just need to be inited somewhere before reading. */
    side_defaults = lispnil;
    unit_specs = last_unit_spec = lispnil;
    default_unit_spec = lispnil;
    /* And for this. */
    if (readerrbuf == NULL)
      readerrbuf = (char *) xmalloc(BUFSIZE);
}

/* This is the basic interpreter of a form appearing in a module. */

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

static void
useless_form_warning(Module *module, Obj *form)
{
    char posbuf[BUFSIZE], buf[BUFSIZE];

    if (!actually_read_lisp)
      return;
    module_and_line(module, posbuf);
    sprintlisp(buf, form, BUFSIZE);
    init_warning("%sA useless form: %s", posbuf, buf);
}

/* Inclusion is half-module-like, not strictly textual. */

static void
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

static void
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

static void
start_else(Obj *form, Module *module)
{
    /* (should match up with cond read stack) */
    actually_read_lisp = !actually_read_lisp;
}

static void
end_conditional(Obj *form, Module *module)
{
    /* should match up with cond read stack */
    actually_read_lisp = TRUE;
}

/* Given a list of variant-defining forms, allocate and return an
   array of variant objects. */

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

/* Digest the form defining the module as a whole. */

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

/* Create a new type of unit and fill in info about it. */

static void
interp_utype(Obj *form)
{
    int u;
    Obj *name = cadr(form), *utype;

    TYPECHECK(symbolp, name, "unit-type name not a symbol");
    if (!canaddutype)
      read_warning("Should not be defining more unit types");
    u = create_unit_type();
    utype = new_utype(u);
    /* Set default values for the unit type's props first. */
    /* Any default type name shouldn't confuse the code below. */
    default_unit_type(u);
    setq(name, utype);
    /* Set the values of random props. */
    fill_in_utype(u, cddr(form));
    /* If no internal type name string given, use the defined name. */
    if (empty_string(u_internal_name(u)))
      set_u_internal_name(u, c_string(name));
    if (empty_string(u_type_name(u)))
      set_u_type_name(u, u_internal_name(u));
    /* If the official type name is different from the internal name,
       make it a variable bound to the type. */
    if (strcmp(u_type_name(u), u_internal_name(u)) != 0)
      setq(intern_symbol(u_type_name(u)), utype);
    /* Blast any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_USTAR)));
    eval_symbol(intern_symbol(keyword_name(K_USTAR)));
}

/* Trudge through assorted properties, filling them in. */

static void
fill_in_utype(int u, Obj *list)
{
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	      continue;
	set_utype_property(u, propname, val);
    }
}

/* Given a unit type, property name, and a value, find the
   definition of the property and set its value. */

static int
set_utype_property(int u, char *propname, Obj *val)
{
    int i, found = FALSE, numval;
    char *strval;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
	if (strcmp(propname, utypedefns[i].name) == 0) {
	    if (utypedefns[i].intgetter) {
		val = eval(val);
		if (!numberishp(val)) {
		    read_warning("unit type %s property %s value not a number",
				 u_type_name(u), utypedefns[i].name);
		    return TRUE;
		}
		numval = c_number(val);
		if (!between(utypedefns[i].lo, numval, utypedefns[i].hi)) {
		    read_warning("unit type %s property %s value %d not between %d and %d",
				 u_type_name(u), utypedefns[i].name, numval,
				 utypedefns[i].lo, utypedefns[i].hi);
		    /* Let it pass through, at least for now. */
		}
		TYPEPROP(utypes, u, utypedefns, i, short) = numval;
	    } else if (utypedefns[i].strgetter) {
		val = eval(val);
		if (!stringp(val)) {
		    read_warning("unit type %s property %s value not a string",
				 u_type_name(u), utypedefns[i].name);
		    return TRUE;
		}
		strval = c_string(val);
		TYPEPROP(utypes, u, utypedefns, i, char *) = strval;
	    } else {
		TYPEPROP(utypes, u, utypedefns, i, Obj *) = val;
	    }
	    found = TRUE;
	    break;
	}
    }
    if (!found)
      unknown_property("unit type", u_type_name(u), propname);
    return found;
}

/* Declare a new type of material and fill in info about it. */

static void
interp_mtype(Obj *form)
{
    int m;
    Obj *name = cadr(form), *mtype;
    
    TYPECHECK(symbolp, name, "material-type name not a symbol");
    if (!canaddmtype)
      read_warning("Should not be defining more material types");
    m = create_material_type();
    mtype = new_mtype(m);
    /* Set default values for the material type's properties first. */
    default_material_type(m);
    setq(name, mtype);
    /* Set the values of random props. */
    fill_in_mtype(m, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(m_type_name(m)))
      set_m_type_name(m, c_string(name));
    /* Blast and remake any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_MSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_MSTAR)));
}

/* Declare a new type of advance and fill in info about it. */

static void
interp_atype(Obj *form)
{
    int a;
    Obj *name = cadr(form), *atype;
    
    TYPECHECK(symbolp, name, "advance type name not a symbol");
    if (!canaddatype)
      read_warning("Should not be defining more advance types");
    a = create_advance_type();
    atype = new_atype(a);
    /* Set default values for the material type's properties first. */
    default_advance_type(a);
    setq(name, atype);
    /* Set the values of random props. */
    fill_in_atype(a, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(a_type_name(a)))
      set_a_type_name(a, c_string(name));
    /* Blast and remake any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_ASTAR)));
    eval_symbol(intern_symbol(keyword_name(K_ASTAR)));
}

/* Go through a list of prop name/value pairs and fill in the
   material type description from them. */

static void
fill_in_mtype(int m, Obj *list)
{
    int i, found, numval;
    char *strval;
    Obj *bdg, *val;
    char *propname;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, mtypedefns[i].name) == 0) {
		if (mtypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("material type %s property %s value not a number",
				     m_type_name(m), mtypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(mtypedefns[i].lo, numval, mtypedefns[i].hi)) {
			read_warning("material type %s property %s value %d not between %d and %d",
				     m_type_name(m), mtypedefns[i].name, numval,
				     mtypedefns[i].lo, mtypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(mtypes, m, mtypedefns, i, short) = numval;
		} else if (mtypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("material type %s property %s value not a string",
				     m_type_name(m), mtypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(mtypes, m, mtypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(mtypes, m, mtypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("material type", m_type_name(m), propname);
    }
}

/* Go through a list of prop name/value pairs and fill in the
   advance type description from them. */

static void
fill_in_atype(int a, Obj *list)
{
    int i, found, numval;
    char *strval;
    Obj *bdg, *val;
    char *propname;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; atypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, atypedefns[i].name) == 0) {
		if (atypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("advance type %s property %s value not a number",
				     a_type_name(a), atypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(atypedefns[i].lo, numval, atypedefns[i].hi)) {
			read_warning("advance type %s property %s value %d not between %d and %d",
				     a_type_name(a), atypedefns[i].name, numval,
				     atypedefns[i].lo, atypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(atypes, a, atypedefns, i, short) = numval;
		} else if (atypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("advance type %s property %s value not a string",
				     a_type_name(a), atypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(atypes, a, atypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(atypes, a, atypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("advance type", a_type_name(a), propname);
    }
}

/* Declare a new type of terrain and fill in info about it. */

static void
interp_ttype(Obj *form)
{
    int t;
    Obj *name = cadr(form), *ttype;

    TYPECHECK(symbolp, name, "terrain-type name not a symbol");
    if (!canaddttype)
      read_warning("Should not be defining more terrain types");
    t = create_terrain_type();
    ttype = new_ttype(t);
    /* Set default values for the terrain type's props first. */
    default_terrain_type(t);
    setq(name, ttype);
    /* Set the values of random properties. */
    fill_in_ttype(t, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(t_type_name(t)))
      set_t_type_name(t, c_string(name));
    /* Blast and remake any cached list of all types. */
    makunbound(intern_symbol(keyword_name(K_TSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_TSTAR)));
}

/* Go through a list of prop name/value pairs and fill in the
   terrain type description from them. */

static void
fill_in_ttype(int t, Obj *list)
{
    int i, found, numval;
    char *strval;
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, ttypedefns[i].name) == 0) {
		if (ttypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("terrain type %s property %s value not a number",
				     t_type_name(t), ttypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(ttypedefns[i].lo, numval, ttypedefns[i].hi)) {
			read_warning("terrain type %s property %s value %d not between %d and %d",
				     t_type_name(t), ttypedefns[i].name, numval,
				     ttypedefns[i].lo, ttypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(ttypes, t, ttypedefns, i, short) = numval;
		} else if (ttypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("terrain type %s property %s value not a string",
				     t_type_name(t), ttypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(ttypes, t, ttypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(ttypes, t, ttypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("terrain type", t_type_name(t), propname);
    }
}

/* Fill in a table. */

static void
interp_table(Obj *form)
{
    int i, found, reset = TRUE;
    Obj *formsym = cadr(form), *body = cddr(form);
    char *tablename;

    /* It's impossible to know whether all the types indexed by a
       specific table have been defined when the table form is
       evaluated, so don't try any heuristic tricks to load a default
       game here. */
    TYPECHECK(symbolp, formsym, "table name not a symbol");
    tablename = c_string(formsym);
    /* Don't read in tables used internally only. */
    if (strncmp(tablename, "zz-", 3) == 0)
        return;
    found = FALSE;
    /* Search in the list of tables for one with the given name. */
    for (i = 0; tabledefns[i].name != NULL; ++i) {
	if (strcmp(tablename, tabledefns[i].name) == 0) {
	    if (match_keyword(car(body), K_ADD)) {
		body = cdr(body);
		reset = FALSE;
	    }
	    allocate_table(i, reset);
	    add_to_table(formsym, i, body);
	    found = TRUE;
	    break;
	}
    }
    if (!found)
      read_warning( "Undefined table `%s'", tablename);
}

/* Given a table and a list of value-setting clauses, fill in the table. */

#define INDEXP(typ, x) \
  ((typ == UTYP) ? utypep(x) : ((typ == MTYP) ? mtypep(x) : ((typ == TTYP) ? ttypep(x) : atypep(x))))

#define nonlist(x) (!consp(x) && x != lispnil)

#define CHECK_INDEX_1(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index1, (x))) {  \
      char errbuf[BUFSIZE]; \
      sprintlisp(errbuf, (x), BUFSIZE); \
      read_warning("table %s index 1 value `%s' has wrong type, skipping clause",  \
		   tabledefns[tbl].name, errbuf);  \
      return;  \
  }

#define CHECK_INDEX_2(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index2, (x))) {  \
      char errbuf[BUFSIZE]; \
      sprintlisp(errbuf, (x), BUFSIZE); \
      read_warning("table %s index 2 value `%s' has wrong type, skipping clause",  \
		   tabledefns[tbl].name, errbuf);  \
      return;  \
  }

#define CHECK_VALUE(tbl, x)  \
  if (!numberishp(x)) {  \
      read_warning("table %s value is not a number, skipping clause",  \
		   tabledefns[tbl].name);  \
      return;  \
  } \
  { int checknum = c_number(x);  \
    if (!between(tabledefns[tbl].lo, checknum, tabledefns[tbl].hi)) {  \
	read_warning("table %s value %d not within bounds %d to %d, skipping clause",  \
		     tabledefns[tbl].name, checknum, tabledefns[tbl].lo, tabledefns[tbl].hi);  \
	return;  \
    }  \
  }

#define CHECK_LISTS(tablename, lis1, lis2)  \
  if (consp(lis2)  \
      && !list_lengths_match(lis1, lis2, "table", tablename))  {  \
      return;  \
  }


static void
add_to_table(Obj *tablename, int tbl, Obj *clauses)
{
    int i, num, lim1, lim2;
    Obj *clause, *indexes1, *indexes2, *values;

    lim1 = numtypes_from_index_type(tabledefns[tbl].index1);
    lim2 = numtypes_from_index_type(tabledefns[tbl].index2);
    for ( ; clauses != lispnil; clauses = cdr(clauses)) {
	clause = car(clauses);
	switch (clause->type) {
	  case SYMBOL:
	    clause = eval_symbol(clause);
	    TYPECHECK(numberp, clause, "table clause does not eval to number");
	    /* Now treat it as a number. */
	  case NUMBER:
	    /* A constant value for the table - blast over everything. */
	    CHECK_VALUE(tbl, clause);
	    num = c_number(clause);
	    /* Make sure the table is allocated first. */
	    if (tabledefns[tbl].table == NULL)
	      allocate_table(tbl, TRUE);
	    for (i = 0; i < lim1 * lim2; ++i)
	      (*(tabledefns[tbl].table))[i] = num;
	    break;
	  case CONS:
	    /* Evaluate the three parts of a clause. */
	    indexes1 = eval(car(clause));
	    indexes2 = eval(cadr(clause));
	    values = eval(caddr(clause));
	    if (cdddr(clause) != lispnil) {
		sprintlisp(readerrbuf, clause, BUFSIZE);
		read_warning("In table `%s', extra junk at end of clause `%s', ignoring",
			     c_string(tablename), readerrbuf);
	    }
	    interp_one_clause(tablename, tbl, lim1, lim2,
			      indexes1, indexes2, values);
	    break;
	  case STRING:
	    break; /* error? */
	  default:
	    /* who knows? */
	    break;
	}
    }
}

static void
interp_one_clause(Obj *tablename, int tbl, int lim1, int lim2,
		  Obj *indexes1, Obj *indexes2, Obj *values)
{
    int i, j, num;
    Obj *tmp1, *tmp2, *in1, *in2, *value, *subvalue;

    if (nonlist(indexes1)) {
	CHECK_INDEX_1(tbl, indexes1);
	i = c_number(indexes1);
	if (nonlist(indexes2) ) {
	    CHECK_INDEX_2(tbl, indexes2);
	    j = c_number(indexes2);
	    value = values;
	    CHECK_VALUE(tbl, value);
	    num = c_number(value);
	    (*(tabledefns[tbl].table))[lim2 * i + j] = num;
	} else {
	    CHECK_LISTS(tablename, indexes2, values);
	    for_all_list(indexes2, tmp2) {
		in2 = car(tmp2);
		CHECK_INDEX_2(tbl, in2);
		j = c_number(in2);
		value = (consp(values) ? car(values) : values);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
		if (consp(values))
		  values = cdr(values);
	    }
	}
    } else {
	CHECK_LISTS(tablename, indexes1, values);
	for_all_list(indexes1, tmp1) {
	    in1 = car(tmp1);
	    CHECK_INDEX_1(tbl, in1);
	    i = c_number(in1);
	    value = (consp(values) ? car(values) : values);
	    if (nonlist(indexes2)) {
		CHECK_INDEX_2(tbl, indexes2);
		j = c_number(indexes2);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
	    } else {
		if (nonlist(value)) {
		    subvalue = value;
		    CHECK_VALUE(tbl, subvalue);
		    num = c_number(subvalue);
		    for_all_list(indexes2, tmp2) {
			in2 = car(tmp2);
			CHECK_INDEX_2(tbl, in2);
			j = c_number(in2);
			(*(tabledefns[tbl].table))[lim2 * i + j] = num;
		    }
		} else {
		    CHECK_LISTS(tablename, indexes2, value);
		    for_all_list(indexes2, tmp2) {
			in2 = car(tmp2);
			CHECK_INDEX_2(tbl, in2);
			j = c_number(in2);
			subvalue = car(value);
			CHECK_VALUE(tbl, subvalue);
			num = c_number(subvalue);
			(*(tabledefns[tbl].table))[lim2 * i + j] = num;
			value = cdr(value);
		    }
		}
	    }
	    if (consp(values))
	      values = cdr(values);
	}
    }
}

/* Set the binding of an existing known variable. */

static void
interp_variable(Obj *form, int isnew)
{
    int i, numval;
    Obj *var = cadr(form);
    Obj *val = eval(caddr(form));
    char *name;

    SYNTAX(form, symbolp(var), "variable is not a symbol");
    name = c_string(var);

    if (isnew) {
	if (boundp(var))
	  read_warning("Symbol `%s' has been bound already, overwriting", name);
	setq(var, val);
    } else {
	/* Look through the set of defined globals. */
	for (i = 0; vardefns[i].name != 0; ++i) {
	    if (strcmp(name, vardefns[i].name) == 0) {
		if (vardefns[i].intgetter != NULL) {
		    TYPECHECK(numberishp, val, "is not a number or type");
		    numval = c_number(val);
		    if (!between(vardefns[i].lo, numval, vardefns[i].hi)) {
			read_warning("global %s value %d not between %d and %d",
				     vardefns[i].name, numval,
				     vardefns[i].lo, vardefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    (*(vardefns[i].intsetter))(numval);
		} else if (vardefns[i].strgetter != NULL) {
		    TYPECHECK(stringp, val, "is not a string");
		    (*(vardefns[i].strsetter))(c_string(val));
		} else if (vardefns[i].objgetter != NULL) {
		    (*(vardefns[i].objsetter))(val);
		} else {
#ifdef __cplusplus
		    throw "snafu";
#else
		    abort();
#endif
		}
		return;
	    }
	}
	/* Try as a random symbol. */
	if (boundp(var)) {
	    setq(var, val);
	    return;
	}
	/* Out of luck. */
	read_warning("Can't set unknown global named `%s'", name);
    }
}

static void
undefine_variable(Obj *form)
{
    Obj *var = cadr(form);

    if (!symbolp(var)) {
	read_warning("Can't undefine a non-symbol!");
	return;
    }
    makunbound(var);
}

/* General function to augment types. */

static void
add_properties(Obj *form)
{
    Obj *rest, *types, *prop, *values;

    rest = cdr(form);
    types = eval(car(rest));
    rest = cdr(rest);
    prop = car(rest);
    SYNTAX(form, symbolp(prop), "not a property name in third position");
    rest = cdr(rest);
    SYNTAX(form, rest != lispnil, "no property values supplied");
    if (!is_quoted_lisp(car(rest)))
      values = eval(car(rest));
    else
      values = car(rest);
    /* Complain about, but ignore, extra things. */
    if (cdr(rest) != lispnil) {
	sprintlisp(readerrbuf, form, BUFSIZE);
	read_warning("Extra junk at the end of `%s', ignoring", readerrbuf);
    }
    if (utypep(types) || (consp(types) && utypep(car(types)))) {
	add_to_utypes(types, prop, values);
    } else if (mtypep(types) || (consp(types) && mtypep(car(types)))) {
	add_to_mtypes(types, prop, values);
    } else if (ttypep(types) || (consp(types) && ttypep(car(types)))) {
	add_to_ttypes(types, prop, values);
    } else if (atypep(types) || (consp(types) && atypep(car(types)))) {
	add_to_atypes(types, prop, values);
    } else {
	sprintlisp(readerrbuf, form, BUFSIZE);
	read_warning("No types to add to in `%s'", readerrbuf);
    }
}

/* Compare a list of types with a list of values, complain if
   they don't match up. */

static int
list_lengths_match(Obj *types, Obj *values, char *formtype, Obj *form)
{
    if (length(types) != length(values)) {
	sprintlisp(spbuf, form, BUFSIZE);
	read_warning("Lists of differing lengths (%d vs %d) in %s `%s'",
		     length(types), length(values), formtype, spbuf);
	return FALSE;
    }
    return TRUE;
}

static void
add_to_utypes(Obj *types, Obj *prop, Obj *values)
{
    char *propname = c_string(prop);
    Obj *lis1, *lis2, *carlis1;

    if (utypep(types)) {
	set_utype_property(types->v.num, propname, values);
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "utype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(utypep, carlis1, "not a unit type");
		if (!set_utype_property(carlis1->v.num, propname, car(lis2)))
		  break;
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(utypep, carlis1, "not a unit type");
		if (!set_utype_property(carlis1->v.num, propname, values))
		  break;
	    }
	}
    }
}

static void
add_to_mtypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (mtypep(types)) {
	fill_in_mtype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "mtype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(mtypep, carlis1, "not a unit type");
		fill_in_mtype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(mtypep, carlis1, "not a material type");
		fill_in_mtype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

static void
add_to_ttypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (ttypep(types)) {
	fill_in_ttype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "ttype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(ttypep, carlis1, "not a terrain type");
		fill_in_ttype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(ttypep, carlis1, "not a terrain type");
		fill_in_ttype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

static void
add_to_atypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (atypep(types)) {
	fill_in_atype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "atype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(atypep, carlis1, "not an advance type");
		fill_in_atype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(atypep, carlis1, "not an advance type");
		fill_in_atype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

/* Interpret a world-specifying form. */

static void
interp_world(Obj *form)
{
    int numval;
    Obj *props, *bdg, *propval;
    char *propname;

    props = cdr(form);
    if (symbolp(car(props))) {
	/* This is the id of the world (eventually). */
	props = cdr(props);
    }
    if (numberp(car(props))) {
    	set_world_circumference(c_number(car(props)), TRUE);
	props = cdr(props);
    }
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (numberp(propval))
	  numval = c_number(propval);
	switch (keyword_code(propname)) {
	  case K_CIRCUMFERENCE:
	    set_world_circumference(numval, TRUE);
	    break;
	  case K_DAY_LENGTH:
	    world.daylength = numval;
	    break;
	  case K_YEAR_LENGTH:
	    world.yearlength = numval;
	    break;
	  case K_AXIAL_TILT:
	    world.axial_tilt = numval;
	    break;
	  case K_DAYLIGHT_FRACTION:
	    world.daylight_fraction = numval;
	    break;
	  case K_TWILIGHT_FRACTION:
	    world.twilight_fraction = numval;
	    break;
	  default:
	    unknown_property("world", "", propname);
	}
    }
}

/* Only one area, of fixed size.  Created anew if shape/size is supplied, else
   just modified. */

static void
interp_area(Obj *form)
{
    int newarea = FALSE, newwidth = 0, newheight = 0, numval;
    Obj *props, *subprop, *bdg, *propval, *rest, *val;
    char *propname, *strval;

    props = cdr(form);
    if (match_keyword(car(props), K_ADD)) {
	add_to_area(cdr(props));
	return;
    }
    /* (eventually this will be an id or name) */
    if (symbolp(car(props))) {
	props = cdr(props);
	newarea = TRUE;
    }
    /* Collect the width of the area. */
    if (numberp(car(props))) {
	newwidth = newheight = c_number(car(props));
    	if (area.fullwidth == 0)
	  newarea = TRUE;
	if (area.fullwidth > 0 && area.fullwidth != newwidth)
	  read_warning("weird areas - %d vs %d", area.fullwidth, newwidth);
	props = cdr(props);
    }
    /* Collect the height of the area. */
    if (numberp(car(props))) {
	newheight = c_number(car(props));
    	if (area.fullheight == 0)
	  newarea = TRUE;
	if (area.fullheight > 0 && area.fullheight != newheight)
	  read_warning("weird areas - %d vs %d", area.fullheight, newheight);
	props = cdr(props);
    }
    /* See if we're restricting ourselves to a piece of a larger area. */
    if (consp(car(props))
        && match_keyword(car(car(props)), K_RESTRICT)) {
        subprop = cdr(car(props));
	val = car(subprop);
        if (numberp(val)) {
	    area.fullwidth = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fullheight = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fullx = c_number(val);
	    subprop = cdr(subprop);
	    val = car(subprop);
	    TYPECHECK(numberp, val, "restriction parm not a number");
	    area.fully = c_number(val);
        } else if (match_keyword(car(subprop), K_RESET)) {
	    area.fullwidth = area.fullheight = 0;
	    area.fullx = area.fully = 0;
        } else {
	    syntax_error(car(props), "not 4 numbers or 'reset'");
	    return;
        }
	props = cdr(props);        
    }
    /* If this is setting the area's shape for the first time,
       actually do it. */
    if (newarea)
      set_area_shape(newwidth, newheight, TRUE);
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (numberp(propval))
	  numval = c_number(propval);
	rest = cdr(bdg);
	switch (keyword_code(propname)) {
	  case K_WIDTH:
	    /* Note that this may fail if the height has to change at
	       at the same time. */
	    set_area_shape(numval, area.height, TRUE);
	    break;
	  case K_HEIGHT:
	    /* Note that this may fail if the width has to change at
	       at the same time. */
	    set_area_shape(area.width, numval, TRUE);
	    break;
	  case K_LATITUDE:
	    area.latitude = numval;
	    break;
	  case K_LONGITUDE:
	    area.longitude = numval;
	    break;
	  case K_PROJECTION:
	    area.projection = numval;
	    break;
	  case K_CELL_WIDTH:
	    area.cellwidth = numval;
	    break;
	  case K_SUN:
	    area.sunx = numval;
	    area.suny = c_number(cadr(rest));
	    break;
	  case K_TEMPERATURE_YEAR_CYCLE:
	    area.temp_year = propval;
	    break;
	  case K_IMAGE_NAME:
	    strval = NULL;
	    if (stringp(propval))
	      strval = c_string(propval);
	    area.image_name = strval;
	    break;
	  case K_TERRAIN:
	    fill_in_terrain(rest);
	    break;
	  case K_AUX_TERRAIN:
	    fill_in_aux_terrain(rest);
	    break;
	  case K_PEOPLE_SIDES:
	    fill_in_people_sides(rest);
	    break;
	  case K_CONTROL_SIDES:
	    fill_in_control_sides(rest);
	    break;
	  case K_FEATURES:
	    fill_in_features(rest);
	    break;
	  case K_ELEVATIONS:
	    fill_in_elevations(rest);
	    break;
	  case K_MATERIAL:
	    fill_in_cell_material(rest);
	    break;
	  case K_TEMPERATURES:
	    fill_in_temperatures(rest);
	    break;
	  case K_WINDS:
	    fill_in_winds(rest);
	    break;
	  case K_CLOUDS:
	    fill_in_clouds(rest);
	    break;
	  case K_CLOUD_BOTTOMS:
	    fill_in_cloud_bottoms(rest);
	    break;
	  case K_CLOUD_HEIGHTS:
	    fill_in_cloud_heights(rest);
	    break;
	  case K_USER:
	    fill_in_users(rest);
	    break;
	  default:
	    unknown_property("area", "", propname);
	}
    }
}

/* The general plan of reading is similar for all layers - create a blank
   layer if none allocated, then call read_layer and pass a function that will
   actually put a value into a cell of the layer.  We need to define those
   functions because most of the setters are macros, and because we can do
   some extra error checking. */

/* Read the area terrain. */

static void
fill_in_terrain(Obj *contents)
{
    /* We must have some terrain types or we're going to lose bigtime. */
    if (numttypes == 0)
      load_default_game();
    numbadterrain = 0;
    /* Make sure the terrain layer exists. */
    if (!terrain_defined())
      allocate_area_terrain();
    read_layer(contents, fn_set_terrain_at);
    if (numbadterrain > 0) {
	read_warning("%d occurrences of unknown terrain in all",
		     numbadterrain);
    }
}

/* Read a layer of auxiliary terrain. */

static void
fill_in_aux_terrain(Obj *contents)
{
    int t;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && ttypep(typeval = eval(typesym))) {
	t = c_number(typeval);
	contents = cdr(contents);
	/* Make sure aux terrain space exists, but don't overwrite. */
	allocate_area_aux_terrain(t);
	tmpttype = t;
	read_layer(contents, fn_set_aux_terrain_at);
	/* Ensure that borders and connections have all their bits
	   correctly set. */
	patch_linear_terrain(t);
    } else {
	/* not a valid aux terrain type */
    }
}

static void
fill_in_people_sides(Obj *contents)
{
    /* Make sure the people sides layer exists. */
    allocate_area_people_sides();
    read_layer(contents, fn_set_people_side_at);
}

static void
fill_in_control_sides(Obj *contents)
{
    /* Make sure the people sides layer exists. */
    allocate_area_control_sides();
    read_layer(contents, fn_set_control_side_at);
}

/* This should recompute size etc of all these features too. */

static void
fill_in_features(Obj *contents)
{
    int fid;
    Obj *featspec, *flist, *feattype, *featname;
    Feature *feat;

    init_features();
    for_all_list(car(contents), flist) {
	featspec = car(flist);
	fid = 0;
	feat = NULL;
	if (numberp(car(featspec))) {
	    fid = c_number(car(featspec));
	    feat = find_feature(fid);
	    featspec = cdr(featspec);
	}
	if (feat == NULL) {
	    feattype = car(featspec);
	    featname = cadr(featspec);
	    if ((stringp(feattype) || symbolp(feattype))
		&& (stringp(featname) || symbolp(featname))) {
		feat = create_feature(c_string(feattype), c_string(featname));
		if (fid > 0)
		  feat->id = fid;
	    } else {
		syntax_error(featspec, "not a valid feature spec");
	    }
	} else {
	    /* complain if data doesn't match? */
	}
    }
    read_layer(cdr(contents), fn_set_raw_feature_at);
}

static void
fill_in_elevations(Obj *contents)
{
    /* Make sure the elevation layer exists. */
    allocate_area_elevations();
    read_layer(contents, fn_set_elevation_at);
    compute_elevation_bounds();
}

static void
fill_in_cell_material(Obj *contents)
{
    int m;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && mtypep(typeval = eval(typesym))) {
	m = c_number(typeval);
	contents = cdr(contents);
	/* Make sure this material layer exists. */
	allocate_area_material(m);
	tmpmtype = m;
	read_layer(contents, fn_set_material_at);
    } else {
	/* not a valid material type spec, should warn */
    }
}

static void
fill_in_temperatures(Obj *contents)
{
    /* Make sure the temperature layer exists. */
    allocate_area_temperatures();
    read_layer(contents, fn_set_temperature_at);
}

static void
fill_in_winds(Obj *contents)
{
    /* Make sure the winds layer exists. */
    allocate_area_winds();
    read_layer(contents, fn_set_raw_wind_at);
}

static void
fill_in_clouds(Obj *contents)
{
    /* Make sure the clouds layer exists. */
    allocate_area_clouds();
    read_layer(contents, fn_set_raw_cloud_at);
}

static void
fill_in_cloud_bottoms(Obj *contents)
{
    /* Make sure the cloud bottoms layer exists. */
    allocate_area_cloud_bottoms();
    read_layer(contents, fn_set_raw_cloud_bottom_at);
}

static void
fill_in_cloud_heights(Obj *contents)
{
    /* Make sure the cloud heights layer exists. */
    allocate_area_cloud_heights();
    read_layer(contents, fn_set_raw_cloud_height_at);
}

static void
fill_in_users(Obj *contents)
{
    /* Make sure the user layer exists. */
    if (!user_defined()) 
    	allocate_area_users();
    read_layer(contents, fn_set_user_at);
}

static void
add_to_area(Obj *spec)
{
    int x0, y0, w, h, t, x, y;
    char *propname;
    Obj *val, *typeval;

    /* Collect the dimensions of the area to modify. */
    if (numberp(car(spec))) {
	x0 = c_number(car(spec));
	spec = cdr(spec);
    }
    if (numberp(car(spec))) {
	y0 = c_number(car(spec));
	spec = cdr(spec);
    }
    if (numberp(car(spec))) {
	w = c_number(car(spec));
	spec = cdr(spec);
	if (numberp(car(spec))) {
	    h = c_number(car(spec));
	    spec = cdr(spec);
	} else {
	    h = w;
	}
    } else {
	w = h = 1;
    }
    PARSE_PROPERTY(spec, propname, val);
    switch (keyword_code(propname)) {
      case K_TERRAIN:
	if (symbolp(val) && ttypep(typeval = eval(val))) {
	    t = c_number(typeval);
	    for (x = x0; x < x0 + w; ++x) {
		for (y = y0; y < y0 + h; ++y) {
		    if (in_area(x, y)) {
			set_terrain_at(x, y, t);
		    }
		}
	    }
	}
	break;
      case K_AUX_TERRAIN:
	break;
      case K_PEOPLE_SIDES:
	break;
      case K_CONTROL_SIDES:
	break;
      case K_FEATURES:
	break;
      case K_ELEVATIONS:
	break;
      case K_MATERIAL:
	break;
      case K_TEMPERATURES:
	break;
      case K_WINDS:
	break;
      case K_CLOUDS:
	break;
      case K_CLOUD_BOTTOMS:
	break;
      case K_CLOUD_HEIGHTS:
	break;
      default:
	unknown_property("area", "", propname);
    }
}

/* Interpret a side spec. */

static void
interp_side(Obj *form, Side *side)
{
    int id = -1;
    Obj *ident = lispnil, *sym = lispnil, *props = cdr(form);

    // Get optional side identifier, if there is one.
    if (!listp(car(props))) {
	ident = car(props);
	props = cdr(props);
    }
    // Get side from numeric ID.
    if (numberp(ident)) {
	id = c_number(ident);
	side = side_n(id);
    }
    // Maybe get side from symbolic ID.
    else if (symbolp(ident)) {
	if (boundp(ident) && numberp(symbol_value(ident))) {
	    id = c_number(symbol_value(ident));
	    side = side_n(id);
	} else {
	    sym = ident;
	}
    }
    // Maybe get side from string ID.
    else if (stringp(ident)) {
	side = find_side_by_name(c_string(ident));
    } 
    // Else, we want to create a new side.
    else; 
    // Additionally get the side symbol, if we haven't already.
    if (symbolp(car(props)) && (lispnil == sym)) {
	sym = car(props);
	props = cdr(props);
    }
#if (0) // Do we want to do this or not?
    // Require the 'add' keyword, if we are modifying an existing side.
    if (side && nextsideid) {
	if (!match_keyword(car(props), K_ADD))
	    syntax_error(car(props), "attempting to redefine an existing side");
    }
#endif
    // If indepside (which always exists) was read from the form, 
    //	make sure it is completed if necessary.
    if ((side == indepside) && !nextsideid) {
    	// Does not actually create indepside, but fills in its slots.
    	side = create_side();
    }
    // Else if no side was looked up, then we are dealing with new side.
    else if (!side) {
	// If the side that was read from the form does not exist yet
        // we have to create it. 
    	// Make sure indepside is completed first. This has to be done
    	// now and not earlier since we cannot allocate all its slots
    	// until we know numutypes etc. which we just got from reading
    	// this form. */
    	if (nextsideid == 0) {
    	    /* Does not actually create indepside, but fills in its slots. */
	    create_side();
    	}
    	// Then create (and fill in) the new side.
	side = create_side();
	// Replace with the read-in id if it was given.
	if (id >= 0)
	  side->id = id;
    }
    // Bind optional symbol to side.
    if (sym != lispnil) {
	// Record the symbol as going with this side.
	side->symbol = sym;
	// Also let it eval to the side's id, warning about conflicts.
	if (boundp(sym))
	  read_warning("Symbol `%s' has been bound already, overwriting",
		       c_string(sym));
	setq(sym, new_number(side->id));
    }
    // Apply the current side defaults first.
    fill_in_side(side, side_defaults, FALSE);
    // Now fill in from the explicitly specified properties.
    fill_in_side(side, props, FALSE);
    Dprintf("  Got side %s\n", side_desig(side));
}

/* Given a side, fill in some of its properties. */

void
fill_in_side(Side *side, Obj *props, int userdata)
{
    int numval, isnumber, permission, u;
    char *propname, *strval = NULL, *checkuniq;
    Obj *bdg, *rest, *propval;

    /* Use this macro with every slot that the user may not arbitrarily
       change when the game starts up. */
#define CHECK_PERMISSION  \
	    if (userdata) {  \
		permission = FALSE;  \
		break;  \
	    }

    for (; props != lispnil; props = cdr(props)) {
	permission = TRUE;
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (symbolp(propval))
	  propval = eval(propval);
	numval = eval_number(propval, &isnumber);
	if (stringp(propval))
	  strval = c_string(propval);
	rest = cdr(bdg);
	checkuniq = NULL;
	switch (keyword_code(propname)) {
	  case K_NAME:
	    side->name = strval;
	    checkuniq = "name";
	    break;
	  case K_LONG_NAME:
	    side->longname = strval;
	    checkuniq = "long name";
	    break;
	  case K_SHORT_NAME:
	    side->shortname = strval;
	    checkuniq = "short name";
	    break;
	  case K_NOUN:
	    side->noun = strval;
	    checkuniq = "noun";
	    break;
	  case K_PLURAL_NOUN:
	    side->pluralnoun = strval;
	    checkuniq = "plural noun";
	    break;
	  case K_ADJECTIVE:
	    side->adjective = strval;
	    checkuniq = "adjective";
	    break;
	  case K_COLOR:
	    side->colorscheme = strval;
	    checkuniq = "color";
	    break;
	  case K_EMBLEM_NAME:
	    side->emblemname = strval;
	    checkuniq = "emblem";
	    break;
	  case K_UNIT_NAMERS:
	    // Allocate space if not already done so.
	    if (side->unitnamers == NULL)
	      side->unitnamers = (char **) xmalloc(numutypes * sizeof(char *));
	    merge_unit_namers(side, rest);
	    break;
	  case K_FEATURE_NAMERS:
	    // An empty list means to clear the side's list, 
	    //	otherwise we "merge" by gluing onto the front.
	    if (rest == lispnil)
	      side->featurenamers = lispnil;
	    else
	      side->featurenamers = append_two_lists(rest, side->featurenamers);
	    break;
	  case K_NAMES_LOCKED:
	    side->nameslocked = numval;
	    break;
	  case K_UNITS:
	    side->possible_units = propval;
	    break;
	  case K_CLASS:
	    CHECK_PERMISSION;
	    side->sideclass = strval;
	    break;
	  case K_SELF_UNIT:
	    CHECK_PERMISSION;
	    side->self_unit_id = numval;
	    break;
	  case K_CONTROLLED_BY:
	    CHECK_PERMISSION;
	    side->controlled_by_id = numval;
	    break;
	  case K_TRUSTS:
	    interp_side_value_list(side->trusts, rest);
	    break;
	  case K_TRADES:
	    interp_side_value_list(side->trades, rest);
	    break;
	  case K_START_WITH:
	    CHECK_PERMISSION;
	    if (side->startwith == NULL)
	      side->startwith = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->startwith, rest);
	    break;
	  case K_NEXT_NUMBERS:
	    CHECK_PERMISSION;
	    if (side->counts == NULL)
	      side->counts = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->counts, rest);
	    break;
	  case K_TECH:
	    CHECK_PERMISSION;
	    if (side->tech == NULL)
	      side->tech = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->tech, rest);
	    break;
	  case K_INIT_TECH:
	    CHECK_PERMISSION;
	    if (side->inittech == NULL)
	      side->inittech = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(side->inittech, rest);
	    break;
	  case K_ACTION_PRIORITIES:
	    CHECK_PERMISSION;
	    if (side->action_priorities == NULL) {
		side->action_priorities = (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->action_priorities[u] = -1;
	    }
	    interp_utype_value_list(side->action_priorities, rest);
	    break;
	  case K_ALREADY_SEEN:
	    CHECK_PERMISSION;
	    if (side->already_seen == NULL) {
		side->already_seen = (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->already_seen[u] = -1;
	    }
	    interp_utype_value_list(side->already_seen, rest);
	    break;
	  case K_ALREADY_SEEN_INDEPENDENT:
	    CHECK_PERMISSION;
	    if (side->already_seen_indep == NULL) {
		side->already_seen_indep = 
		    (short *) xmalloc(numutypes * sizeof(short));
		for_all_unit_types(u)
		  side->already_seen_indep[u] = -1;
	    }
	    interp_utype_value_list(side->already_seen_indep, rest);
	    break;
	  case K_DEFAULT_DOCTRINE:
	    read_default_doctrine(side, rest);
	    break;
	  case K_DOCTRINES:
	    read_utype_doctrine(side, rest);
	    break;
	  case K_DOCTRINES_LOCKED:
	    side->doctrines_locked = numval;
	    break;
	  case K_TERRAIN_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_terrain_view);
	    break;
	  case K_TERRAIN_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_terrain_view_date);
	    break;
	  case K_AUX_TERRAIN_VIEW:
	    CHECK_PERMISSION;
	    read_aux_terrain_view_layer(side, rest, fn_set_aux_terrain_view);
	    break;
	  case K_AUX_TERRAIN_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_aux_terrain_view_layer(
		side, rest, fn_set_aux_terrain_view_date);
	    break;
	  case K_UNIT_VIEWS:
	    CHECK_PERMISSION;
	    interp_unit_views(side, rest);
	    side->unit_view_restored = TRUE;
	    break;
	  case K_MATERIAL_VIEW:
	    CHECK_PERMISSION;
	    read_material_view_layer(side, rest, fn_set_material_view);
	    break;
	  case K_MATERIAL_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_material_view_layer(side, rest, fn_set_material_view_date);
	    break;
	  case K_TEMPERATURE_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_temp_view);
	    break;
	  case K_TEMPERATURE_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_temp_view_date);
	    break;
	  case K_CLOUD_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_view);
	    break;
	  case K_CLOUD_BOTTOM_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_bottom_view);
	    break;
	  case K_CLOUD_HEIGHT_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_height_view);
	    break;
	  case K_CLOUD_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_cloud_view_date);
	    break;
	  case K_WIND_VIEW:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_wind_view);
	    break;
	  case K_WIND_VIEW_DATES:
	    CHECK_PERMISSION;
	    read_view_layer(side, rest, fn_set_wind_view_date);
	    break;
	  case K_ACTIVE:
	    CHECK_PERMISSION;
	    side->ingame = numval;
	    break;
	  case K_EVER_ACTIVE:
	    CHECK_PERMISSION;
	    side->everingame = numval;
	    break;
	  case K_PRIORITY:
	    CHECK_PERMISSION;
	    side->priority = numval;
	    break;
	  case K_STATUS:
	    CHECK_PERMISSION;
	    side->status = numval;
	    break;
	  case K_SCORES:
	    CHECK_PERMISSION;
	    /* The data in this slot will be interpreted later,
	       after scorekeepers exist. */
	    side->rawscores = rest;
	    break;
	  case K_WILLING_TO_DRAW:
	    side->willingtodraw = numval;
	    break;
	  case K_FINISHED_TURN:
	    side->finishedturn = numval;
	    break;
	  case K_TURN_TIME_USED:
	    CHECK_PERMISSION;
	    side->turntimeused = numval;
	    break;
	  case K_TOTAL_TIME_USED:
	    CHECK_PERMISSION;
	    side->totaltimeused = numval;
	    break;
	  case K_TIMEOUTS:
	    CHECK_PERMISSION;
	    side->timeouts = numval;
	    break;
	  case K_TIMEOUTS_USED:
	    CHECK_PERMISSION;
	    side->timeoutsused = numval;
	    break;
	  case K_ADVANTAGE:
	    CHECK_PERMISSION;
	    side->advantage = numval;
	    break;
	  case K_ADVANTAGE_MIN:
	    CHECK_PERMISSION;
	    side->minadvantage = numval;
	    break;
	  case K_ADVANTAGE_MAX:
	    CHECK_PERMISSION;
	    side->maxadvantage = numval;
	    break;
	  case K_INITIAL_CENTER_AT:
	    /* (should support unit names, etc here also) */
	    side->init_center_x = numval;
	    side->init_center_y = c_number(caddr(bdg));
	    break;
	  case K_PLAYER:
	    side->playerid = numval;
	    break;
	  case K_INSTRUCTIONS:
	    /* The instructions are a list of strings. */
	    side->instructions = propval;
	    break;
	  case K_GAIN_COUNTS:
	    CHECK_PERMISSION;
	    interp_short_array(
		side->gaincounts, rest, numutypes * num_gain_reasons);
	    break;
	  case K_LOSS_COUNTS:
	    CHECK_PERMISSION;
	    interp_short_array(
		side->losscounts, rest, numutypes * num_loss_reasons);
	    break;
	  case K_ATTACK_STATS:
	    CHECK_PERMISSION;
	    interp_atkstats_list(side, rest);
	    break;
	  case K_HIT_STATS:
	    CHECK_PERMISSION;
	    interp_hitstats_list(side, rest);
	    break;
	  case K_AI_DATA:
	    /* The data in this slot will be interpreted when the AI is
	       ready to look at it. */
	    side->aidata = rest;
	    break;
	  case K_UI_DATA:
	    /* The data in this slot will be interpreted when the interface
	       is ready to look at it. */
	    side->uidata = rest;
	    break;
	  case K_STANDING_ORDER:
	    interp_standing_order(side, rest);
	    break;
	  case K_CURRENT_ADVANCE:
	    side->research_topic = numval;
	    break;
	  case K_ADVANCE_GOAL:
	    side->research_goal = numval;
	    break;
	  case K_ADVANCES_DONE:
	    interp_atype_value_list(side->advance, rest);
	    break;
	  case K_TREASURY:
	    interp_treasury_list(side->treasury, rest);
	    break;
	  case K_COULD_ACT_WITH:
	    interp_utype_value_list(side->could_act_with, rest);
	    break;
	  case K_COULD_CONSTRUCT:
	    interp_utype_value_list(side->could_construct, rest);
	    break;
	  case K_COULD_DEVELOP:
	    interp_utype_value_list(side->could_develop, rest);
	    break;
	  case K_COULD_RESEARCH:
	    interp_atype_value_list(side->could_research, rest);
	    break;
	  default:
	    unknown_property("side", side_desig(side), propname);
	}
	if (!permission) {
	    read_warning("No permission to set property `%s'", propname);
	}
	if (checkuniq != NULL) {
	    check_name_uniqueness(side, strval, checkuniq);
	}
    }
    /* Calculate the plural form of the noun for units if it was not
       explicitly defined. */
    if (side->noun != NULL && side->pluralnoun == NULL) {
	side->pluralnoun = copy_string(plural_form(side->noun));
    }
}

/* Given a list of views of units, make unit view objects and plug
   them into the side's list of unit views. */

static void
interp_unit_views(Side *side, Obj *vlist)
{
    int u;
    Obj *rest, *viewform, *props;
    UnitView *uview;

    for_all_list(vlist, rest) {
	viewform = car(rest);
	u = NONUTYPE;
	if (symbolp(car(viewform))) {
	    u = utype_from_symbol(car(viewform));
	}
	if (u == NONUTYPE) {
	    read_warning("bad uview");
	    return;
	}
	uview = (UnitView *) xmalloc(sizeof(UnitView));
	/* Insert it into the global list. */
	uview->vnext = viewlist;
	viewlist = uview;
	uview->type = u;
	props = cdr(viewform);
	if (!consp(car(props))) {
	    uview->siden = c_number(eval(car(props)));
	    props = cdr(props);
	}
	if (symbolp(car(props))) {
             uview->name = c_string(car(props));
         	    props = cdr(props);
         }
	if (numberp(car(props))) {
	    uview->size = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->x = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->y = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->complete = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->date = c_number(car(props));
	    props = cdr(props);
	}
	if (numberp(car(props))) {
	    uview->id = c_number(car(props));
	    props = cdr(props);
	}
	if (symbolp(car(props))) {
            uview->image_name = c_string(car(props));
	    props = cdr(props);
         }
	add_unit_view_raw(side, uview, uview->x, uview->y);
    }
}

static short *rd_u_arr;

static void 
interp_standing_order(Side *side, Obj *form)
{
    enum sordercond condtyp;
    int u;
    char *condname;
    Obj *rest = form, *subform;
    StandingOrder *sorder;

    if (rd_u_arr == NULL)
      rd_u_arr = (short *) xmalloc(numutypes * sizeof(short));

    sorder = new_standing_order();
    for_all_unit_types(u)
      rd_u_arr[u] = FALSE;
    interp_utype_list(rd_u_arr, car(rest));
    for_all_unit_types(u)
      sorder->types[u] = rd_u_arr[u];
    rest = cdr(rest);
    subform = car(rest);
    condname = c_string(car(subform));
    switch (keyword_code(condname)) {
      case K_ALWAYS:
	condtyp = sorder_always;
        break;
      case K_AT:
	condtyp = sorder_at;
        break;
      case K_IN:
	condtyp = sorder_in;
        break;
      case K_NEAR:
	condtyp = sorder_near;
        break;
      default:
	unknown_property("standing order condition", side_desig(side), condname);
	return;
    }
    sorder->condtype = condtyp;
    subform = cdr(subform);
    if (subform != lispnil) {
	sorder->a1 = c_number(car(subform));
	subform = cdr(subform);
    }
    if (subform != lispnil) {
	sorder->a2 = c_number(car(subform));
	subform = cdr(subform);
    }
    if (subform != lispnil) {
	sorder->a3 = c_number(car(subform));
    }
    rest = cdr(rest);
    sorder->task = interp_task(car(rest));

    add_standing_order(side, sorder, 1);
}

static void
check_name_uniqueness(Side *side, char *str, char *kind)
{
    if (name_in_use(side, str)) {
	init_warning("Side %s `%s' is already in use", kind, str);
    }
}

/* Given a list of (utype str) pairs, set unit namers appropriately. */

static void
merge_unit_namers(Side *side, Obj *lis)
{
    int u, u2 = 0;
    Obj *rest, *element, *types, *typesrest, *type1, *namer;

    for_all_list(lis, rest) {
	element = car(rest);
	if (consp(element)) {
	    types = eval(car(element));
	    namer = cadr(element);
	    if (stringp(namer) || symbolp(namer)) {
		if (utypep(types)) {
		    u = c_number(types);
		    side->unitnamers[u] = c_string(namer);
		} else if (listp(types)) {
		    /* If a list of unit types, iterate. */
		    for_all_list(types, typesrest) {
			type1 = car(typesrest);
			if (utypep(type1)) {
			    u = c_number(type1);
			    side->unitnamers[u] = c_string(namer);
			} else {
			    syntax_error(type1, "not a unit type");
			}
		    }
		} else {
		    syntax_error(types, "garbled unit types for namer");
		}
	    } else {
		syntax_error(element, "garbled unit namer");
	    }
	} else if (stringp(element) || symbolp(element)) {
	    if (u2 < numutypes) {
		side->unitnamers[u2++] = c_string(element);
	    } else {
		read_warning("more unit namers than unit types, ignoring extra");
	    }
	} else {
	    syntax_error(element, "not a valid unit namer");
	}
    }
}

static void
interp_side_mask_list(SideMask *maskp, Obj *lis)
{
    Obj *rest, *head;

    if (maskp == NULL)
      run_error("null mask for side mask list?");
    *maskp = NOSIDES;
    for_all_list(lis, rest) {
    	head = car(rest);
	if (numberp(head)) {
	    *maskp = c_number(head);
	    return;
	} else if (symbolp(head)) {
	    int s2 = c_number(eval(head));

	    if (between(1, s2, g_sides_max()))
	      add_side_to_set(side_n(s2), *maskp);
	    else
	      read_warning("bad side spec");
	} else if (consp(head)) {
	    Obj *sidespec = car(head);
	    int s2, val2 = c_number(cadr(head));

	    if (numberp(sidespec) || symbolp(sidespec)) {
		s2 = c_number(eval(sidespec));
		if (between(1, s2, g_sides_max())) {
		    if (val2)
		      add_side_to_set(side_n(s2), *maskp);
		    /* (should have option to remove a side from mask) */
		} else
		  read_warning("bad side spec");
	    } else if (consp(sidespec)) {
	    	read_warning("not implemented");
	    } else {
	    	read_warning("not implemented");
	    }
	} else {
	    read_warning("not implemented");
	}
    }
}

static void
interp_side_value_list(short *arr, Obj *lis)
{
    int s = 0;
    Obj *rest, *head;

    if (arr == NULL)
      run_error("null array for side value list?");
    for_all_list(lis, rest) {
    	head = car(rest);
	if (numberp(head)) {
	    if (s > g_sides_max())
	      break;
	    arr[s++] = c_number(head);
	} else if (symbolp(head)) {
	    int s2 = c_number(eval(head));

	    if (between(1, s2, g_sides_max()))
	      arr[s2] = TRUE;
	    else
	      read_warning("bad side spec");
	} else if (consp(head)) {
	    Obj *sidespec = car(head);
	    int s2, val2 = c_number(cadr(head));

	    if (numberp(sidespec) || symbolp(sidespec)) {
		s2 = c_number(eval(sidespec));
		if (between(1, s2, g_sides_max()))
		  arr[s2] = val2;
		else
		  read_warning("bad side spec");
	    } else if (consp(sidespec)) {
	    	read_warning("not implemented");
	    } else {
	    	read_warning("not implemented");
	    }
	} else {
	    read_warning("not implemented");
	}
    }
}

static void
interp_atkstats_list(Side *side, Obj *lis)
{
    int u;
    Obj *rest, *head;

    for_all_list(lis, rest) {
    	head = car(rest);
    	if (consp(head) && symbolp(car(head))) {
	    u = utype_from_symbol(car(head));
	    if (!is_unit_type(u)) {
		init_warning("non unit type");
		continue;
	    }
	    if (side->atkstats[u] == NULL)
	      side->atkstats[u] = (long *) xmalloc(numutypes * sizeof(long));
	    interp_long_array(side->atkstats[u], cdr(head), numutypes);
	}
    }
}

static void
interp_hitstats_list(Side *side, Obj *lis)
{
    int u;
    Obj *rest, *head;

    for_all_list(lis, rest) {
    	head = car(rest);
    	if (consp(head) && symbolp(car(head))) {
	    u = utype_from_symbol(car(head));
	    if (!is_unit_type(u)) {
		init_warning("non unit type");
		continue;
	    }
	    if (side->hitstats[u] == NULL)
	      side->hitstats[u] = (long *) xmalloc(numutypes * sizeof(long));
	    interp_long_array(side->hitstats[u], cdr(head), numutypes);
	}
    }
}

static void
read_view_layer(Side *side, Obj *contents,
		void (*setter)(int x, int y, int val))
{
    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    read_layer(contents, setter);
}

static void
read_aux_terrain_view_layer(Side *side, Obj *contents,
			    void (*setter)(int x, int y, int val))
{
    int t;
    Obj *typesym = car(contents), *typeval;

    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    if (!symbolp(typesym)) {
	return;
    }
    typeval = eval(typesym);
    if (!ttypep(typeval)) {
	return;
    }
    t = c_number(typeval);
    if (!t_is_cell(t)) {
	return;
    }
    contents = cdr(contents);
    tmpttype = t;
    read_layer(contents, setter);
}

static void
read_material_view_layer(Side *side, Obj *contents,
			 void (*setter)(int x, int y, int val))
{
    int m;
    Obj *typesym = car(contents), *typeval;

    if (g_see_all())
      return;
    init_view(side);
    tmpside = side;
    if (!symbolp(typesym)) {
	return;
    }
    typeval = eval(typesym);
    if (!mtypep(typeval)) {
	return;
    }
    m = c_number(typeval);
    contents = cdr(contents);
    tmpmtype = m;
    read_layer(contents, setter);
}

/* Get the default doctrine for the side, looking for it by either
   name or number. */

static void
read_default_doctrine(Side *side, Obj *props)
{
    int id = 0;
    char *name = NULL;
    Obj *ident = lispnil;
    Doctrine *doctrine = NULL;

    if (props == lispnil) {
	read_warning("no doctrine information supplied");
	return;
    }
    ident = car(props);
    if (numberp(ident)) {
	id = c_number(ident);
	doctrine = find_doctrine(id);
    } else if (symbolp(ident) || stringp(ident)) {
	name = c_string(ident);
	doctrine = find_doctrine_by_name(name);
    } else {
	type_error(ident, "invalid doctrine reference");
	return;
    }
    /* Make sure we have a doctrine to fill in the side's field. */
    if (doctrine == NULL) {
	doctrine = new_doctrine(id);
	doctrine->name = name;
    }
    side->default_doctrine = doctrine;
}

/* Get per-unit-type doctrines for the given side. */

static void
read_utype_doctrine(Side *side, Obj *lis)
{
    int u, id;
    char *name;
    Obj *item, *rest, *ulist, *props, *ident;
    Doctrine *doctrine;
    
    if (rd_u_arr == NULL)
      rd_u_arr = (short *) xmalloc(numutypes * sizeof(short));

    for_all_list(lis, rest) {
	item = car(rest);
	ulist = car(item);
	props = cdr(item);
	for_all_unit_types(u)
	  rd_u_arr[u] = FALSE;
	interp_utype_list(rd_u_arr, ulist);
	ident = car(props);
	id = 0;
	name = NULL;
	if (numberp(ident)) {
	    id = c_number(ident);
	    doctrine = find_doctrine(id);
	} else if (symbolp(ident) || stringp(ident)) {
	    name = c_string(ident);
	    doctrine = find_doctrine_by_name(name);
	} else {
	    type_error(ident, "invalid doctrine reference");
	    return;
	}
	if (doctrine == NULL) {
	    doctrine = new_doctrine(id);
	    doctrine->name = name;
	}
	for_all_unit_types(u) {
	    if (rd_u_arr[u]) {
		if (side->udoctrine == NULL)
		  init_doctrine(side);
		side->udoctrine[u] = doctrine;
	    }
	}
    }
}

/* Interpret a form as a doctrine. */

static void
interp_doctrine(Obj *form)
{
    int id = -1;
    char *name = NULL;
    Obj *ident = lispnil, *props = cdr(form);
    Doctrine *doctrine = NULL;

    /* Pick up an optional numeric or symbolic id. */
    if (props != lispnil) {
	if (!consp(car(props))) {
	    ident = car(props);
	    props = cdr(props);
	}
    }
    /* Use the id to find an existing doctrine. */
    if (numberp(ident)) {
	id = c_number(ident);
	doctrine = find_doctrine(id);
    }
    if (symbolp(ident)) {
	name = c_string(ident);
	doctrine = find_doctrine_by_name(name);
    }
    if (doctrine == NULL) {
	doctrine = new_doctrine(id);
	doctrine->name = name;
    }
    /* Give the symbol a binding so eval'ing works. */
    if (symbolp(ident))
      setq(ident, new_string(name));
    fill_in_doctrine(doctrine, props);
    Dprintf("  Got doctrine\n");
}

static void
fill_in_doctrine(Doctrine *doctrine, Obj *props)
{
    int numval;
    char *propname;
    Obj *bdg, *val;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	if (numberp(val))
	  numval = c_number(val);
	switch (keyword_code(propname)) {
	  case K_RESUPPLY_PERCENT:
	    doctrine->resupply_percent = numval;
	    break;
	  case K_REARM_PERCENT:
	    doctrine->rearm_percent = numval;
	    break;
	  case K_REPAIR_PERCENT:
	    doctrine->repair_percent = numval;
	    break;
	  case K_CONSTRUCTION_RUN:
	    interp_utype_value_list(doctrine->construction_run, cdr(bdg));
	    break;
	  case K_LOCKED:
	    doctrine->locked = numval;
	    break;
	  default:
	    unknown_property("doctrine", "", propname);
	}
    }
}

/* Interpret a form that defines a player. */

static void
interp_player(Obj *form)
{
    int id = -1;
    Obj *ident = lispnil, *props = cdr(form);
    Player *player = NULL;

    if (props != lispnil) {
	if (!consp(car(props))) {
	    ident = car(props);
	    props = cdr(props);
	}
    }
    if (numberp(ident)) {
	id = c_number(ident);
	player = find_player(id);
    }
    if (player == NULL) {
	player = add_player();
    }
    if (id >= 0)
      player->id = id;
    fill_in_player(player, props);
    Dprintf("  Got player %s\n", player_desig(player));
}

static void
fill_in_player(Player *player, Obj *props)
{
    char *propname, *strval;
    Obj *bdg, *propval;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (stringp(propval))
	  strval = c_string(propval);
	switch (keyword_code(propname)) {
	  case K_NAME:
	    player->name = strval;
	    break;
	  case K_CONFIG_NAME:
	    player->configname = strval;
	    break;
	  case K_DISPLAY_NAME:
	    player->displayname = strval;
	    break;
	  case K_AI_TYPE_NAME:
	    player->aitypename = strval;
	    break;
	  case K_INITIAL_ADVANTAGE:
	    player->advantage = c_number(propval);
	    break;
	  case K_PASSWORD:
	    player->password = strval;
	    break;
	  default:
	    unknown_property("player", player_desig(player), propname);
	}
    }
    canonicalize_player(player);
}

#if 0

/* Create and fill in an agreement, as specified by the form. */

static void
interp_agreement(Obj *form)
{
    int id = 0;
    char *propname;
    Obj *props = cdr(form), *agid, *bdg, *val;
    Agreement *ag;

    agid = car(props);
    if (numberp(agid)) {
    	id = c_number(agid);
	/* should use the number eventually */
	props = cdr(props);
    }
    if (1 /* must create a new agreement object */) {
	ag = create_agreement(id);
	/* Fill in defaults for the slots. */
	ag->state = draft;  /* default for now */
	ag->drafters = NOSIDES;
	ag->proposers = NOSIDES;
	ag->signers = NOSIDES;
	ag->willing = NOSIDES;
	ag->knownto = NOSIDES;
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_TYPE_NAME:
	    ag->agtype = c_string(val);
	    break;
	  case K_NAME:
	    ag->name = c_string(val);
	    break;
	  case K_STATE:
	    ag->state = c_number(val);
	    break;
	  case K_TERMS:
	    ag->terms = val;
	    break;
	  case K_DRAFTERS:
	    break;
	  case K_PROPOSERS:
	    break;
	  case K_SIGNERS:
	    break;
	  case K_WILLING_TO_SIGN:
	    break;
	  case K_KNOWN_TO:
	    break;
	  case K_ENFORCEMENT:
	    ag->enforcement = c_number(val);
	    break;
	    break;
	  default:
	    unknown_property("agreement", "", propname);
	}
    }
}

#endif

static void
interp_unit_defaults(Obj *form)
{
    int u, numval, wasnum, variablelength;
    Obj *props = form, *bdg, *val;
    char *propname;

    default_unit_spec = form;
    if (match_keyword(car(props), K_RESET)) {
	/* Reset all the tweakable defaults. */
	uxoffset = 0, uyoffset = 0;
	default_unit_side_number = -1;
	default_unit_origside_number = -1;
	default_unit_cp = -1;
	default_unit_hp = -1;
	default_unit_cxp = -1;
	default_unit_z = -1;
	default_transport_id = -1;
	if (default_tooling != NULL) {
	    for_all_unit_types(u)
	      default_tooling[u] = -1;
	}
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	numval = 0;
	wasnum = FALSE;
	if (numberp(val)) {
	    numval = c_number(val);
	    wasnum = TRUE;
	}
	variablelength = FALSE;
	/* Note that not all unit slots can get default values. */
	switch (keyword_code(propname)) {
	  case K_AT:
	    uxoffset = numval;
	    uyoffset = c_number(caddr(bdg));
	    /* The property is not really variable-length, but mollify
               the error check. */
	    variablelength = TRUE;
	    break;
	  case K_S:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    default_unit_side_number = numval;
	    break;
	  case K_OS:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    default_unit_origside_number = numval;
	    break;
	  case K_CP:
	    default_unit_cp = numval;
	    break;
	  case K_HP:
	    default_unit_hp = numval;
	    break;
	  case K_CXP:
	    default_unit_cxp = numval;
	    break;
	  case K_TP:
	    disallow_more_unit_types();
	    if (default_tooling == NULL)
	      default_tooling = (short *) xmalloc(numutypes * sizeof(short));
	    interp_utype_value_list(default_tooling, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_IN:
	    default_transport_id = numval;
	    break;
	  case K_Z:
	    default_unit_z = numval;
	    break;
	  default:
	    unknown_property("unit-defaults", "", propname);
	}
	if (!variablelength && cddr(bdg) != lispnil)
	  read_warning("Extra junk in a default %s property, ignoring",
		       propname);
    }
}

/* Try to find a unit type named by the string. */
/* (should move these elsewhere, but where?) */

int
utype_from_name(char *str)
{
    char *tmpstr;
    int u;
    Obj *sym;

    if (empty_string(str))
      return NONUTYPE;
    for_all_unit_types(u) {
	if (strcmp(str, u_type_name(u)) == 0)
	  return u;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && utypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    /* Try some less common possibilities. */
    for_all_unit_types(u) {
	tmpstr = u_short_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    for_all_unit_types(u) {
	tmpstr = u_long_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    return NONUTYPE;
}

/* Try to find a material type named by the string. */

int
mtype_from_name(char *str)
{
    int m;
    Obj *sym;

    for_all_material_types(m) {
	if (strcmp(str, m_type_name(m)) == 0)
	  return m;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && mtypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONMTYPE;
}

/* Try to find an advance type named by the string. */

int
atype_from_name(char *str)
{
    int a;
    Obj *sym;

    for_all_advance_types(a) {
	if (strcmp(str, a_type_name(a)) == 0)
	  return a;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && atypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONATYPE;
}

/* Try to find a terrain type named by the string. */

int
ttype_from_name(char *str)
{
    int t;
    Obj *sym;

    for_all_terrain_types(t) {
	if (strcmp(str, t_type_name(t)) == 0)
	  return t;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && ttypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONTTYPE;
}

/* Try to find a unit type named by the string. */

int
utype_from_symbol(Obj *sym)
{
    char *str, *tmpstr;
    int u;

    if (boundp(sym) && utypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    str = c_string(sym);
    for_all_unit_types(u) {
	if (strcmp(str, u_type_name(u)) == 0)
	  return u;
    }
    /* Try some less common possibilities. */
    for_all_unit_types(u) {
	tmpstr = u_short_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    for_all_unit_types(u) {
	tmpstr = u_long_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    return NONUTYPE;
}

/* This creates an individual unit and fills in data about it. */

static Unit *
interp_unit(Obj *form)
{
    int u, u2, tp_varies, numval, numval2, wasnum, nuid = 0, variablelength, unitnum;
    int nusn = -1, nuosn = -1;
    char *propname, *unitname;
    Obj *head = car(form), *props = cdr(form), *bdg, *val, *bdgrest, *val2;
    Obj *save, *unitspec;
    Unit *unit, *unit2;
    extern int nextid;

    if (!g_create_units()) {
	/* Stash the unit form and current defaults for later use. */
	Dprintf("Saving the unit spec ");
	Dprintlisp(form);
	Dprintf("\n");
	save = cons(cons(form, cons(default_unit_spec, lispnil)), lispnil);
	if (unit_specs != lispnil) {
	    set_cdr(last_unit_spec, save);
	    last_unit_spec = save;
	} else {
	    unit_specs = last_unit_spec = save;
	}
	return NULL;
    }
    Dprintf("Reading a unit from ");
    Dprintlisp(form);
    Dprintf("\n");
    if (symbolp(head)) {
	u = utype_from_symbol(head);
     	if (u != NONUTYPE) {
	    unit = create_unit(u, FALSE);
	    canaddmtype = FALSE;  /* (Why?) */
	    if (unit == NULL) {
		read_warning("Failed to create a unit, skipping the form");
		return NULL;
	    }
	} else {
	    read_warning("\"%s\" not a known unit type, skipping the form",
			 c_string(head));
	    return NULL;
	}
    } else if (stringp(head)) {
	unitname = c_string(head);
	unit = find_unit_by_name(unitname);
	if (unit == NULL) {
	    if (unit_specs != lispnil) {
		unitspec = find_unit_spec_by_name(unitname);
		if (unitspec != lispnil) {
		    /* First set up the saved defaults. */
		    interp_unit_defaults(cadr(unitspec));
		    unit = interp_unit(car(unitspec));
		    /* Restore the current defaults. */
		    interp_unit_defaults(default_unit_spec);
		    if (unit == NULL) {
			read_warning("Failed to create unit from saved spec");
			return NULL;
		    }
		} else {
		    read_warning("No unit spec named \"%s\", skipping the form",
				 unitname);
		    return NULL;
		}
	    } else {
		read_warning("No unit named \"%s\", skipping the form",
			     unitname);
		return NULL;
	    }
    	}
    } else if (numberp(head)) {
	unitnum = c_number(head);
	unit = find_unit_by_number(unitnum);
	if (unit == NULL) {
	    if (unit_specs != lispnil) {
		unitspec = find_unit_spec_by_number(unitnum);
		if (unitspec != lispnil) {
		    /* First set up the saved defaults. */
		    interp_unit_defaults(cadr(unitspec));
		    unit = interp_unit(car(unitspec));
		    /* Restore the current defaults. */
		    interp_unit_defaults(default_unit_spec);
		    if (unit == NULL) {
			read_warning("Failed to create unit from saved spec");
			return NULL;
		    }
		} else {
		    read_warning("No unit spec numbered %d, skipping the form",
				 unitnum);
		    return NULL;
		}
	    } else {
		read_warning("No unit numbered %d, skipping the form",
			     unitnum);
		return NULL;
	    }
    	}
    }
    /* At this point we're guaranteed to have a unit to work with. */
    /* Modify the unit according to current defaults. */
    if (default_unit_side_number >= 0)
      nusn = default_unit_side_number;
    if (default_unit_origside_number >= 0)
      nuosn = default_unit_origside_number;
    if (default_unit_cp >= 0)
      unit->cp = default_unit_cp;
    if (default_unit_hp >= 0)
      unit->hp = unit->hp2 = default_unit_hp;
    if (default_unit_cxp >= 0)
      unit->cxp = default_unit_cxp;
    /* First default supplies using the generic supply initialize. */
    init_supply(unit);
    /* Fill in any default toolings that might have been set up. */
    if (default_tooling != NULL) {
	tp_varies = FALSE;
	for_all_unit_types(u2) {
	    if (default_tooling[u2] > 0) {
		tp_varies = TRUE;
		break;
	    }
	}
	if (tp_varies) {
	    if (unit->tooling == NULL)
	      init_unit_tooling(unit);
	    for_all_unit_types(u2)
	      unit->tooling[u] = default_tooling[u];
	}
    }
    /* Peel off fixed-position properties, if they're supplied. */
    if (numberp(car(props))) {
	unit->prevx = c_number(car(props)) + uxoffset - area.fullx;
	props = cdr(props);
    }
    if (numberp(car(props))) {
	unit->prevy = c_number(car(props)) + uyoffset - area.fully;
	props = cdr(props);
    }
    if (props != lispnil && !consp(car(props))) {
	nusn = c_number(eval(car(props)));
	props = cdr(props);
    }
    /* Now crunch through optional stuff.  The unit's properties must *already*
       be correct. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY_RETURN(bdg, propname, val, unit);
	numval = 0;
	wasnum = FALSE;
	if (numberp(val)) {
	    numval = c_number(val);
	    wasnum = TRUE;
	}
	variablelength = FALSE;
	switch (keyword_code(propname)) {
	  case K_N:
	    unit->name = c_string(val);
	    break;
	  case K_IMAGE_NAME:
	    unit->image_name = c_string(val);
	    break;
	  case K_SHARP:
	    nuid = numval;
	    break;
	  case K_S:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    nusn = numval;
	    break;
	  case K_OS:
	    if (!wasnum)
	      numval = c_number(eval(val));
	    nuosn = numval;
	    break;
	  case K_AT:
	    if (numberp(caddr(bdg))) {
		numval2 = c_number(caddr(bdg));
	    } else {
		type_error(caddr(bdg), "not a number");
		numval2 = 0;
	    }
	    unit->prevx = numval + uxoffset - area.fullx;
	    unit->prevy = numval2 + uyoffset - area.fully;
	    variablelength = TRUE;
	    break;
	  case K_NB:
	    unit->number = numval;
	    break;
	  case K_CP:
	    unit->cp = numval;
	    break;
	  case K_HP:
	    /* Note if the unit is read in as damaged, we don't want
	       damage reckoning to replace with a default hp2. */
	    unit->hp = unit->hp2 = numval;
	    break;
	  case K_CXP:
	    unit->cxp = numval;
	    break;
	  case K_MO:
	    unit->morale = numval;
	    break;
	  case K_TRK:
	    interp_side_mask_list(&(unit->tracking), cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_M:
	    interp_treasury_list(unit->supply, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_TP:
	    if (unit->tooling == NULL)
	      init_unit_tooling(unit);
	    interp_utype_value_list(unit->tooling, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_OPINIONS:
	    if (unit->opinions == NULL)
	      init_unit_opinions(unit, numsides);
	    if (unit->opinions != NULL) {
		interp_side_value_list(unit->opinions, cdr(bdg));
	    } else {
		read_warning("Unit %s cannot have opinions, ignoring attempt to set",
			     unit_desig(unit));
	    }
	    variablelength = TRUE;
	    break;
	  case K_IN:
	    unit->transport_id = val;
	    break;
	  case K_ACP:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE, numval);
	    if (unit->act != NULL)
	      unit->act->acp = numval;
	    else
	      read_warning("Unit %s cannot have acp, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_ACP0:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE, numval);
	    if (unit->act != NULL)
	      unit->act->initacp = numval;
	    else
	      read_warning("Unit %s cannot have acp0, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_AM:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE);
	    if (unit->act != NULL)
	      unit->act->actualmoves = numval;
	    else
	      read_warning("Unit %s cannot have am, ignoring attempt to set",
			   unit_desig(unit));
	    break;
	  case K_A:
	    if (unit->act == NULL)
	      init_unit_actorstate(unit, TRUE);
	    if (unit->act != NULL)
	      interp_action(&(unit->act->nextaction), val);
	    else
	      read_warning("Unit %s cannot have action, ignoring attempt to set",
			   unit_desig(unit));
	    variablelength = TRUE;
	    break;
	  case K_PLAN:
	    interp_unit_plan(unit, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_Z:
	    unit->z = numval;
	    break;
	  case K_SIZE:
	    unit->size = numval;
	    break;
	  case K_REACH:
	    unit->reach = numval;
	    break;
	  case K_USEDCELLS:
	    unit->usedcells = numval;
	    break;
	  case K_MAXCELLS:
	    unit->maxcells = numval;
	    break;
	  case K_CURADVANCE:
	    unit->curadvance = numval;
	    break;
	  case K_PRODUCTION:
	    interp_mtype_value_list(unit->production, cdr(bdg));
	    variablelength = TRUE;
	    break;
	  case K_POPULATION:
	    unit->population = numval;
	    break;
	  case K_CP_STASH:
	    unit->cp_stash = numval;
	    break;
          case K_CREATION_ID:
            unit->creation_id = numval;
            break;
	  case K_POINT_VALUE:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->point_value = numval;
	    break;
	  case K_APPEAR:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->appear = numval;
	    /* Get the xy variation if supplied. */
	    bdgrest = cddr(bdg);
	    if (bdgrest != lispnil) {
		val2 = car(bdgrest);
		if (numberp(val2)) {
		    unit->extras->appear_var_x = c_number(val2);
		    bdgrest = cdr(bdgrest);
		    val2 = car(bdgrest);
		    if (numberp(val2)) {
			unit->extras->appear_var_y = c_number(val2);
		    }
		}
	    }
	    variablelength = TRUE;
	    break;
	  case K_DISAPPEAR:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->disappear = numval;
	    break;
	  case K_PRIORITY:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->priority = numval;
	    break;
	  case K_SYM:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->sym = val;
	    break;
	  case K_SIDES:
	    if (unit->extras == NULL)
	      init_unit_extras(unit);
	    unit->extras->sides = cdr(bdg);
	    variablelength = TRUE;
	    break;
	  default:
	    unknown_property("unit", unit_desig(unit), propname);
	}
	if (!variablelength && cddr(bdg) != lispnil)
	  read_warning("Extra junk in the %s property of %s, ignoring",
		       propname, unit_desig(unit));
    }
    /* If the unit id was given, assign it to the unit, avoiding
       duplication. */
    if (nuid > 0) {
    	/* If this id is already in use by some other unit, complain. */
    	unit2 = find_unit(nuid);
    	if (unit2 != NULL && unit2 != unit)
	  init_error("Id %d already in use by %s", nuid, unit_desig(unit2)); 
     	/* Guaranteed distinct, safe to use. */
	unit->id = nuid;
	/* Ensure that future random ids won't step on this one. */
	nextid = max(nextid, nuid + 1);
    }
    if (nusn >= 0) {
	/* (should check that this is an allowed side?) */
	set_unit_side(unit, side_n(nusn));
    }
    if (nuosn >= 0) {
	/* (should check that this is an allowed side?) */
	set_unit_origside(unit, side_n(nuosn));
    } else {
	set_unit_origside(unit, unit->side);
    }
    Dprintf("  Got %s\n", unit_desig(unit));
    return unit;
}

static Obj *
find_unit_spec_by_name(char *name)
{
    Obj *rest, *spec, *props, *bdg, *val;
    char *propname;

    for_all_list(unit_specs, rest) {
	spec = car(car(rest));
	for_all_list(spec, props) {
	    bdg = car(props);
	    if (consp(bdg)) {
		PARSE_PROPERTY_RETURN(bdg, propname, val, lispnil);
		if (keyword_code(propname) == K_N) {
		    if (strcmp(name, c_string(val)) == 0)
		      return car(rest);
		}
	    }
	}
    }
    return lispnil;
}

static Obj *
find_unit_spec_by_number(int num)
{
    Obj *rest, *spec, *props, *bdg, *val;
    char *propname;

    for_all_list(unit_specs, rest) {
	spec = car(car(rest));
	for_all_list(spec, props) {
	    bdg = car(props);
	    if (consp(bdg)) {
		PARSE_PROPERTY_RETURN(bdg, propname, val, lispnil);
		if (keyword_code(propname) == K_NB) {
		    if (num == c_number(val))
		      return car(rest);
		}
	    }
	}
    }
    return lispnil;
}

static void
interp_action(Action *action, Obj *form)
{
    int atype, i, numargs;
    char *argtypes;
    Obj *actiontypesym, *arg;

    actiontypesym = car(form);
    form = cdr(form);
    atype = lookup_action_type(c_string(actiontypesym));
    if (atype < 0) {
	/* (should complain about this?) */
	return;
    }
    action->type = (ActionType)atype;
    argtypes = actiondefns[atype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	arg = car(form);
	SYNTAX(arg, numberp(arg), "action arg must be a number");
	action->args[i] = c_number(arg);
	form = cdr(form);
    }
    if (form != lispnil) {
	/* (should allow non-ids also) */
	action->actee = c_number(car(form));
	form = cdr(form);
    }
    /* (should complain about any leftovers) */
}

/* Fill in a unit's plan. */

static void
interp_unit_plan(Unit *unit, Obj *props)
{
    int isnumber, numval;
    Obj *bdg, *propval, *plantypesym, *val, *trest;
    char *propname;
    Goal *goal;
    Plan *plan;
    Task *task, *task1;

    if (unit->plan == NULL) {
	/* Create the plan explicitly, even if unit type doesn't allow it
	   (type might be changed later in the reading process). */
	plan = (Plan *) xmalloc(sizeof(Plan));
	/* From init_unit_plan: can't call it directly, might not behave
	   right (should fix to be callable from here - problem is that
	   other unit props such as cp might not be set right yet) */
	/* Allow AIs to make this unit do things. */
	plan->aicontrol = TRUE;
	/* Enable supply alarms by default. */
	plan->supply_alarm = TRUE;
	/* Attach to unit. */
	unit->plan = plan;
    }
    plantypesym = car(props);
    SYNTAX(props, symbolp(plantypesym), "plan type must be a symbol");
    plan->type = (PlanType)lookup_plan_type(c_string(plantypesym));
    props = cdr(props);
    val = car(props);
    if (numberp(val)) {
	plan->creation_turn = c_number(val);
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	numval = eval_number(propval, &isnumber);
	switch (keyword_code(propname)) {
	  case K_INITIAL_TURN:
	    plan->initial_turn = numval;
	    break;
	  case K_FINAL_TURN:
	    plan->final_turn = numval;
	    break;
	  case K_ASLEEP:
	    plan->asleep = numval;
	    break;
	  case K_RESERVE:
	    plan->reserve = numval;
	    break;
	  case K_DELAYED:
	    plan->delayed = numval;
	    break;
	  case K_WAIT:
	    plan->waitingfortasks = numval;
	    break;
	  case K_AI_CONTROL:
	    plan->aicontrol = numval;
	    break;
	  case K_SUPPLY_ALARM:
	    plan->supply_alarm = numval;
	    break;
	  case K_SUPPLY_IS_LOW:
	    plan->supply_is_low = numval;
	    break;
	  case K_WAIT_TRANSPORT:
	    plan->waitingfortransport = numval;
	    break;
	  case K_GOAL:
	    goal = interp_goal(cdr(bdg));
	    plan->maingoal = goal;
	    break;
	  case K_FORMATION:
	    goal = interp_goal(cdr(bdg));
	    plan->formation = goal;
	    /* Object patching will fill in plan's funit slot later. */
	    break;
	  case K_TASKS:
	    task1 = NULL;
	    for_all_list(cdr(bdg), trest) {
	    	task = interp_task(car(trest));
		if (task) {
		    /* Add tasks to the agenda in same order as saved. */
		    if (task1) {
			task1->next = task;
		    } else {
			plan->tasks = task;
		    }
		    task1 = task;
		}
	    }
	    break;
	  default:
	    unknown_property("unit plan", unit_desig(unit), propname);
	}
    }
}

static Task *
interp_task(Obj *form)
{
    int tasktype, numargs, i;
    char *tasktypename, *argtypes;
    Obj *tasktypesym;
    Task *task;

    tasktypesym = car(form);
    SYNTAX_RETURN(form, symbolp(tasktypesym), "task type must be a symbol",
		  NULL);
    tasktypename = c_string(tasktypesym);
    tasktype = lookup_task_type(tasktypename);
    if (tasktype < 0) {
	read_warning("Task type `%s' not recognized", tasktypename);
	return NULL;
    }
    task = create_task((TaskType)tasktype);
    form = cdr(form);
    if (numberp(car(form))) {
	task->execnum = c_number(car(form));
	form = cdr(form);
    }
    if (numberp(car(form))) {
	task->retrynum = c_number(car(form));
	form = cdr(form);
    }
    argtypes = taskdefns[tasktype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	SYNTAX_RETURN(form, numberp(car(form)), "task arg must be a number", NULL);
	task->args[i] = c_number(car(form));
	form = cdr(form);
    }
    /* Warn about unused data, but not a serious problem. */
    if (form != lispnil)
      read_warning("Excess args for task %s", task_desig(task));
    return task;
}

static Goal *
interp_goal(Obj *form)
{
    int goaltype, tf, numargs, i;
    char *argtypes;
    Obj *goaltypesym;
    Goal *goal;
    Side *side;

    SYNTAX_RETURN(form, numberp(car(form)), "goal side must be a number", NULL);
    side = side_n(c_number(car(form)));
    form = cdr(form);
    SYNTAX_RETURN(form, numberp(car(form)), "goal tf must be a number", NULL);
    tf = c_number(car(form));
    form = cdr(form);
    goaltypesym = car(form);
    SYNTAX_RETURN(form, symbolp(goaltypesym), "goal type must be a symbol", NULL);
    goaltype = lookup_goal_type(c_string(goaltypesym));
    goal = create_goal((GoalType)goaltype, side, tf);
    form = cdr(form);
    argtypes = goaldefns[goaltype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	if (argtypes[i] == 'U'
	    && (stringp(car(form)))) {
	    /* (should find unit by name) */
	} else {
	    SYNTAX_RETURN(form, numberp(car(form)), "goal arg must be a number", NULL);
	}
	goal->args[i] = c_number(car(form));
	form = cdr(form);
    }
    /* Warn about unused data, but not a serious problem. */
    if (form != lispnil)
      read_warning("Excess args for goal %s", goal_desig(goal));
    return goal;
}

/* Make a namer from the form. */

static void
interp_namer(Obj *form)
{
    Obj *id = cadr(form), *meth = caddr(form);

    if (symbolp(id)) {
	setq(id, make_namer(id, meth));
    }
}

/* Make a scorekeeper from the given form. */

static void
interp_scorekeeper(Obj *form)
{
    int id = 0;
    char *propname;
    Obj *props = cdr(form), *bdg, *propval;
    Scorekeeper *sk = NULL;

    if (numberp(car(props))) {
	id = c_number(car(props));
	props = cdr(props);
    }
    if (id > 0) {
	sk = find_scorekeeper(id);
    }
    /* Create a new scorekeeper object if necessary. */
    if (sk == NULL) {
	sk = create_scorekeeper();
	if (id > 0) {
	    sk->id = id;
	}
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	switch (keyword_code(propname)) {
	  case K_TITLE:
	    sk->title = c_string(propval);
	    break;
	  case K_WHEN:
	    sk->when = propval;
	    break;
	  case K_APPLIES_TO:
	    sk->who = propval;
	    break;
	  case K_KNOWN_TO:
	    sk->knownto = propval;
	    break;
	  case K_TRIGGER:
	    sk->trigger = propval;
	    break;
	  case K_DO:
	    sk->body = propval;
	    break;
	  case K_TRIGGERED:
	    sk->triggered = c_number(propval);
	    break;
	  case K_KEEP_SCORE:
	    if (symbolp(propval) && boundp(propval))
	      propval = eval_symbol(propval);
	    sk->keepscore = c_number(propval);
	    break;
	  case K_INITIAL_SCORE:
	    sk->initial = c_number(propval);
	    break;
	  case K_NOTES:
	    sk->notes = propval;
	    break;
	  default:
	    unknown_property("scorekeeper", "??", propname);
	}
    }
}

/* Make a past unit from the form. */

static void
interp_past_unit(Obj *form)
{
    int u = NONUTYPE, nid;
    char *propname;
    Obj *props, *bdg, *propval, *idval;
    PastUnit *pastunit;

    Dprintf("Reading a past unit from ");
    Dprintlisp(form);
    Dprintf("\n");
    props = cdr(form);
    idval = car(props);
    TYPECHECK(numberp, idval, "id not a number")
    nid = c_number(idval);
    props = cdr(props);
    if (symbolp(car(props))) {
	u = utype_from_symbol(car(props));
	props = cdr(props);
    }
    if (u == NONUTYPE) {
	read_warning("bad exu");
	return;
    }
    pastunit = create_past_unit(u, nid);
    /* Peel off fixed-position properties, if they're supplied. */
    if (numberp(car(props))) {
	pastunit->x = c_number(car(props));
	props = cdr(props);
    }
    if (numberp(car(props))) {
	pastunit->y = c_number(car(props));
	props = cdr(props);
    }
    if (!consp(car(props))) {
	pastunit->side = side_n(c_number(eval(car(props))));
	props = cdr(props);
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	switch (keyword_code(propname)) {
	  case K_Z:
	    pastunit->z = c_number(propval);
	    break;
	  case K_N:
	    pastunit->name = c_string(propval);
	    break;
	  case K_NB:
	    pastunit->number = c_number(propval);
	    break;
	  default:
	    unknown_property("exu", "??", propname);
	}
    }
}

/* Make a historical event from the form. */

static void
interp_history(Obj *form)
{
    int startdate, type, i;
    char *evttype;
    SideMask observers;
    Obj *props, *val;
    HistEvent *hevt;

    Dprintf("Reading a hist event from ");
    Dprintlisp(form);
    Dprintf("\n");
    props = cdr(form);
    /* Get the event's date. */
    val = car(props);
    TYPECHECK(numberp, val, "date not a number")
    startdate = c_number(val);
    props = cdr(props);
    /* Get the event's type. */
    val = car(props);
    TYPECHECK(symbolp, val, "type not a symbol")
    evttype = c_string(val);
    /* (should be in separate routine) */
    type = -1;
    for (i = 0; hevtdefns[i].name != NULL; ++i) {
	if (strcmp(evttype, hevtdefns[i].name) == 0) {
	    type = i;
	    break;
	}
    }
    if (type < 0) {
	read_warning("Historical event type `%s' not recognized", evttype);
	return;
    }
    props = cdr(props);
    /* Get the bit vector of observers. */
    if (numberp(car(props))) {
	observers = c_number(car(props));
	props = cdr(props);
    } else if (symbolp(car(props))
	       && keyword_code(c_string(car(props))) == K_ALL) {
	observers = ALLSIDES;
	props = cdr(props);
    } else {
	syntax_error(form, "bad hevt observers");
	return;
    }
    hevt = create_historical_event((HistEventType)type);
    hevt->startdate = startdate;
    hevt->observers = observers;
    /* Read up to 4 remaining numbers. */
    i = 0;
    for (; props != lispnil && i < 4; props = cdr(props)) {
	hevt->data[i++] = c_number(car(props));
    }
    /* Insert the newly created event. */
    /* (linking code should be in its own routine) */
    hevt->next = history;
    hevt->prev = history->prev;
    history->prev->next = hevt;
    history->prev = hevt;
}

/* Property name is unknown, either misspelled or misapplied. */

static void
unknown_property(char *type, char *inst, char *name)
{
    read_warning("The %s form %s has no property named %s", type, inst, name);
}

/* This is like init_warning, but with a module and line(s) glued in. */

void
read_warning(char *str, ...)
{
    char buf[BUFSIZE];
    va_list ap;

    module_and_line(curmodule, buf);

    va_start(ap, str);
    vtprintf(buf, str, ap);
    va_end(ap);

    if (warnings_logged)
      log_warning("READ", buf);
    if (warnings_suppressed)
      return;
    low_init_warning(buf);
}

static void set_u_internal_name(int u, char *s) { utypes[u].iname = s; }
static void set_u_type_name(int u, char *s) { utypes[u].name = s; }
static void set_m_type_name(int m, char *s) { mtypes[m].name = s; }
static void set_t_type_name(int t, char *s) { ttypes[t].name = s; }
static void set_a_type_name(int a, char *s) { atypes[a].name = s; }

static int
lookup_action_type(char *name)
{
    int i;

    for (i = 0; actiondefns[i].name != NULL; ++i)
      if (strcmp(name, actiondefns[i].name) == 0)
	return i;
    return -1;
}

static int
lookup_goal_type(char *name)
{
    int i;

    for (i = 0; goaldefns[i].name != NULL; ++i)
      if (strcmp(name, goaldefns[i].name) == 0)
	return i; /* should get real enum? */
    return GOAL_NO;
}

static int
lookup_plan_type(char *name)
{
    int i;
    extern char *plantypenames[];

    for (i = 0; plantypenames[i] != NULL; ++i)
      /* should get real enum */
      if (strcmp(name, plantypenames[i]) == 0)
	return i;
    return PLAN_NONE;
}

int
lookup_task_type(char *name)
{
    int i;

    for (i = 0; taskdefns[i].name != NULL; ++i)
      if (strcmp(name, taskdefns[i].name) == 0)
	return i; /* should get real enum? */
    return -1;
}

/* Note that the array is not initialized, allows for multiple calls. */

void
interp_utype_list(short *arr, Obj *lis)
{
    int u = 0;
    Obj *rest, *head;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    lis = eval(lis);
    if (!consp(lis))
      lis = cons(lis, lispnil);
    if (numberp(car(lis)) && length(lis) == numutypes) {
	u = 0;
	for_all_list(lis, rest)
	  arr[u++] = c_number(car(rest));
    } else {
	for_all_list(lis, rest) {
	    head = car(rest);
	    if (utypep(head)) {
		arr[c_number(head)] = TRUE;
	    } else {
		/* syntax error */
	    }
	}
    }
}

/* Given an array of unit types, and a list of pairs of symbolic unit
   types and values, fill in the array. */

void
interp_utype_value_list(short *arr, Obj *lis)
{
    int u = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *uval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (u < numutypes) {
	    	arr[u++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (utypep(types)) {
		u = types->v.num;
	    	arr[u++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    uval = car(subrest);
		    TYPECHECK(utypep, uval, "not a unit type");
		    u = uval->v.num;
		    arr[u++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    uval = car(subrest);
		    TYPECHECK(utypep, uval, "not a unit type");
		    u = uval->v.num;
		    arr[u++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

/* Given an array of material types, and a list of pairs of symbolic
   material types and values, fill in the array. */

void
interp_mtype_value_list(short *arr, Obj *lis)
{
    int m = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *mval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (m < nummtypes) {
	    	arr[m++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (mtypep(types)) {
		m = types->v.num;
	    	arr[m++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

/* Same as above but with long arr, so that it can handle treasuries. */

void
interp_treasury_list(long *arr, Obj *lis)
{
    int m = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *mval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (m < nummtypes) {
	    	arr[m++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (mtypep(types)) {
		m = types->v.num;
	    	arr[m++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

/* Given an array of advance types, and a list of pairs of symbolic
   advance types and values, fill in the array. */

void
interp_atype_value_list(short *arr, Obj *lis)
{
    int a = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *aval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (a < numatypes) {
	    	arr[a++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (atypep(types)) {
		a = types->v.num;
	    	arr[a++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    aval = car(subrest);
		    TYPECHECK(atypep, aval, "not an advance");
		    a = aval->v.num;
		    arr[a++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    aval = car(subrest);
		    TYPECHECK(atypep, aval, "not an advance");
		    a = aval->v.num;
		    arr[a++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

/* Read a whole layer, including specs for how to interpret the data. */

void
read_layer(Obj *contents, void (*setter)(int, int, int))
{
    int i, slen, n, ix, len, usechartable = FALSE;
    char *str;
    short chartable[256];
    Obj *rest, *desc, *rest2, *subdesc, *sym, *num;

    layer_use_default = FALSE;
    layer_default = 0;
    layer_multiplier = 1;
    layer_adder = 0;
    layer_area_x = area.fullx;  layer_area_y = area.fully;
    layer_area_w = area.width;  layer_area_h = area.height;
    if (area.fullwidth > 0)
      layer_area_w = area.fullwidth;
    if (area.fullheight > 0)
      layer_area_h = area.fullheight;
    ignore_specials = FALSE;
    for_all_list(contents, rest) {
	desc = car(rest);
	if (stringp(desc)) {
	    /* Read from here to the end of the list, interpreting as
	       contents. */
	    read_rle(rest, setter, (usechartable ? chartable : NULL));
	    return;
	} else if (consp(desc) && symbolp(car(desc))) {
	    switch (keyword_code(c_string(car(desc)))) {
	      case K_CONSTANT:
		/* should set to a constant value taken from cadr */
		read_warning("Constant layers not supported yet");
		return;
	      case K_SUBAREA:
	        /* should apply data to a subarea */
		read_warning("Layer subareas not supported yet");
		break;
	      case K_XFORM:
		layer_multiplier = c_number(cadr(desc));
		layer_adder = c_number(caddr(desc));
		break;
	      case K_BY_BITS:
		break;
	      case K_BY_CHAR:
		/* Assign each char to its corresponding index. */
		/* First seed the table with a 1-1 map. */
		for (i = 0; i < 255; ++i)
		  chartable[i] = 0;
		for (i = 'a'; i <= '~'; ++i)
		  chartable[i] = i - 'a';
		for (i = ':'; i <= '['; ++i)
		  chartable[i] = i - ':' + 30;
		str = c_string(cadr(desc));
		len = strlen(str);
		for (i = 0; i < len; ++i) {
		    chartable[(int) str[i]] = i;
		    /* If special chars in by-char string, flag it. */
		    if (str[i] == '*' || str[i] == ',')
		      ignore_specials = TRUE;
		}
		usechartable = TRUE;
		break;
	      case K_BY_NAME:
		/* Work through list and match names to numbers. */
		/* First seed the table with a 1-1 map. */
		for (i = 0; i < 255; ++i)
		  chartable[i] = 0;
		for (i = 'a'; i <= '~'; ++i)
		  chartable[i] = i - 'a';
		for (i = ':'; i <= '['; ++i)
		  chartable[i] = i - ':' + 30;
		desc = cdr(desc);
		/* Support optional explicit string a la by-char. */
		if (stringp(car(desc))) {
		    str = c_string(car(desc));
		    slen = strlen(str);
		    for (i = 0; i < slen; ++i)
		      chartable[(int) str[i]] = i;
		    desc = cdr(desc);
		} else {
		    str = NULL;
		}
		i = 0;
		for (rest2 = desc; rest2 != lispnil; rest2 = cdr(rest2)) {
		    subdesc = car(rest2);
		    if (symbolp(subdesc)) {
		    	sym = subdesc;
		    	ix = i++;
		    } else if (consp(subdesc)) {
		    	sym = car(subdesc);
		    	num = cadr(subdesc);
		    	TYPECHECK(numberp, num,
				  "by-name explicit value is not a number");
		    	ix = c_number(num);
		    } else {
		    	read_warning("garbage by-name subdesc, ignoring");
		    	continue;
		    }
		    /* Eval the symbol into something resembling a value. */
		    sym = eval(sym);
		    TYPECHECK(numberishp, sym,
			      "by-name index is not a number or type");
		    n = c_number(sym);
		    chartable[(str ? str[ix] : (ix <= 29 ? ('a' + ix) : (':' + ix - 30)))] = n;
		}
		usechartable = TRUE;
		break;
	      default:
		sprintlisp(readerrbuf, desc, BUFSIZE);
		read_warning("Ignoring garbage terrain description %s",
			     readerrbuf);
	    }
	}
    }
}

/* General RLE reader.  This basically parses the run lengths and calls
   the function that records what was read. */

void
read_rle(Obj *contents, void (*setter)(int, int, int), short *chartable)
{
    char ch, *rowstr;
    int i, x, y, run, val, sawval, sawneg, sgn, x1, y1, numbadchars = 0;
    Obj *rest;

    rest = contents;
    y = layer_area_h - 1;
    while (rest != lispnil && y >= 0) {
	/* should error check ... */
	rowstr = c_string(car(rest));
	i = 0;
	x = 0;  /* depends on shape of saved data... */
	while ((ch = rowstr[i++]) != '\0' && x < layer_area_w) {
	    sawval = FALSE;
	    sawneg = FALSE;
	    if (isdigit(ch) || ch == '-') {
		if (ch == '-') {
		    sawneg = TRUE;
		    ch = rowstr[i++];
		    /* A minus sign by itself is a problem. */
		    if (!isdigit(ch))
		      goto recovery;
		}
		/* Interpret a substring of digits as a run length. */
		run = ch - '0';
		while ((ch = rowstr[i++]) != 0 && isdigit(ch)) {
		    run = run * 10 + ch - '0';
		}
		/* A '*' separates a run and a numeric value. */
		if (ch == '*' && !ignore_specials) {
		    /* A negative run length is a problem. */
		    if (sawneg)
		      goto recovery;
		    ch = rowstr[i++];
		    /* If we're seeing garbled data, skip to the next line. */
		    if (ch == '\0')
		      goto recovery;
		    /* Recognize a negative number. */
		    sgn = 1;
		    if (ch == '-') {
			sgn = -1;
			ch = rowstr[i++];
		    }
		    /* Interpret these digits as a value. */
		    if (isdigit(ch)) {
			val = ch - '0';
			while ((ch = rowstr[i++]) != 0 && isdigit(ch)) {
			    val = val * 10 + ch - '0';
			}
			sawval = TRUE;
			val = sgn * val;
		    } else {
			/* Some other char seen - just ignore the '*' then. */
		    }
		    /* If we're seeing garbled data, skip to the next line. */
		    if (ch == '\0')
		      goto recovery;
		}
		/* If we're seeing garbled data, skip to the next line. */
		if (ch == '\0')
		  goto recovery;
	    } else {
		run = 1;
	    }
	    if (ch == ',' && !ignore_specials) {
	    	if (!sawval) {
		    /* This was a value instead of a run length. */
		    val = run;
		    /* If it was prefixed with a minus sign originally,
		       negate the value. */
		    if (sawneg)
		      val = - val;
		    run = 1;
		} else {
		    /* Comma is just being a separator. */
		}
	    } else if (chartable != NULL) {
		val = chartable[(int) ch];
	    } else if (between('a', ch, '~')) {
		val = ch - 'a';
	    } else if (between(':', ch, '[')) {
		val = ch - ':' + 30;
	    } else {
	    	/* Warn about strange characters. */
		++numbadchars;
		if (numbadchars <= 5) {
		    read_warning(
		     "Bad char '%c' (0x%x) in layer, using NUL instead",
				 ch, ch);
		    /* Clarify that we're not going to report all bad chars. */
		    if (numbadchars == 5)
		      read_warning(
		     "Additional bad chars will not be reported individually");
		}
		val = 0;
	    }
	    val = val * layer_multiplier + layer_adder;
	    /* Given a run of values, stuff them into the layer. */
	    while (run-- > 0) {
	    	x1 = x - layer_area_x;  
	    	y1 = y - layer_area_y;
	    	if (in_area(x1, y1))
		  (*setter)(x1, y1, val);
		++x;
	    }
	}
      recovery:
	/* Fill-in string may be too short for this row; just leave
	   the rest of it alone, assume that somebody has assured that
	   the contents are reasonable. */
	rest = cdr(rest);
	y--;
    }
    /* Report the count of garbage chars, in case there were a great many. */
    if (numbadchars > 0)
      init_warning("A total of %d bad chars were present", numbadchars);
}


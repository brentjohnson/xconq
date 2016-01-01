/* Xconq game module writing.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2003, 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"
#include "imf.h"

using namespace Xconq;

extern ImageFamily **recorded_imfs;
extern int num_recorded_imfs;
extern int debugging_state_sync;

#define key(x) (keyword_name(x))

static void write_variants(Variant *varray);
static void write_types(void);
static void write_tables(int compress);
static void write_type_name_list(int typ, int *flags, int dim);
#if 0
static void write_type_value_list(int typ, int *flags, int dim,
					   int (*getter)(int, int), int i);
#endif
static void write_table(const char *name, int (*getter)(int, int), int dflt,
			int typ1, int typ2, int valtype, int compress);
static void write_world(void);
static void write_areas(Module *module);
static void write_area_terrain(int compress);
static void write_area_aux_terrain(int compress);
static void write_area_features(int compress);
static void write_area_elevations(int compress);
static void write_area_people_sides(int compress);
static void write_area_control_sides(int compress);
static void write_area_materials(int compress);
static void write_area_temperatures(int compress);
static void write_area_clouds(int compress);
static void write_area_winds(int compress);
static void write_area_users(int compress);
static void write_globals(void);
static void write_scorekeepers(void);
static void write_doctrines(void);
static void write_sides(Module *module);
static void write_side_properties(Side *side);
static void write_standing_orders(Side *side);
static int fn_terrain_view(int x, int y);
static int fn_aux_terrain_view(int x, int y);
static int fn_aux_terrain_view_date(int x, int y);
static int fn_terrain_view_date(int x, int y);
static int fn_material_view(int x, int y);
static int fn_material_view_date(int x, int y);
static int fn_temp_view(int x, int y);
static int fn_temp_view_date(int x, int y);
static int fn_cloud_view(int x, int y);
static int fn_cloud_bottom_view(int x, int y);
static int fn_cloud_height_view(int x, int y);
static int fn_cloud_view_date(int x, int y);
static int fn_wind_view(int x, int y);
static int fn_wind_view_date(int x, int y);
static void write_side_view(Side *side, int compress);
static void write_one_side_view_layer(int propkey, int (*fn)(int x, int y));
static void write_players(void);
static void write_player(struct a_player *player);
#if 0
static void write_agreements(void);
static void write_agreement(struct a_agreement *agreement);
#endif
static void write_units(Module *module);
static void write_unit_properties(Unit *unit);
static void write_unit_act(Unit *unit);
static void write_action(Action *action, int id);
static void write_unit_plan(Unit *unit);
static void write_task(Task *task);
static void write_goal(Goal *goal, int keyword);
static void write_rle(int (*datafn)(int, int), int lo, int hi,
		      int (*translator)(int), int compress);
static void write_run(int run, int val);
static void write_namers(void);
static void write_history(void);
static void write_past_unit(PastUnit *pastunit);
static void write_historical_event(HistEvent *hevt);
static void write_images(void);
static int reshaped_point(int x1, int y1, int *x2p, int *y2p);
static int original_point(int x1, int y1, int *x2p, int *y2p);

static void start_form(const char *hd);
static void add_to_form(const char *x);
static void add_to_form_no_space(const char *x);
static void add_char_to_form(int x);
static void add_num_to_form(int x);
static void add_num_to_form_no_space(int x);
static void add_num_or_dice_to_form(int x, int valtype);
static void add_form_to_form(Obj *x);
static void end_form(void);
static void newline_form(void);
static void space_form(void);
static void write_bool_prop(const char *name, int value,
			    int dflt, int nodefaulting, int addnewline);
static void write_num_prop(const char *name, int value,
			   int dflt, int nodefaulting, int addnewline);
static void write_str_prop(const char *name, const char *value,
			   const char *dflt, int nodefaulting, int addnewline);
static int string_not_default(const char *str, const char *dflt);
static void write_lisp_prop(const char *name, struct a_obj *value,
			    struct a_obj *dflt, int nodefaulting,
			    int as_cdr, int addnewline);
static void write_utype_value_list(const char *name, short *arr,
				   int dflt, int addnewline);
static void write_mtype_value_list(const char *name, short *arr,
				   int dflt, int addnewline);
static void write_treasury_list(const char *name, long *arr,
				   int dflt, int addnewline);
static void write_atype_value_list(const char *name, short *arr,
				   int dflt, int addnewline);
static void write_side_value_list(const char *name, short *arr,
				  int dflt, int addnewline);
static void write_utype_string_list(const char *name, const char **arr,
				    const char *dflt, int addnewline);
static char *shortest_escaped_name(int u);

/* The pointer to the file being written to. */

static FILE *wfp;

/* True if the area is to be saved to a different size than it is now. */

static int doreshape;

static Module *reshaper;

/* A pre-allocated module used for when we're saving the game and may
   not do any more allocation. */

static Module *spare_module;

static char *shortestbuf;

static int tmpcompress;

static char *spare_file_name;

/* Preparation and preallocation for writing. */

void
init_write(void)
{
    spare_module = create_game_module("spare module");
    shortestbuf = (char *) xmalloc(BUFSIZE);
    spare_file_name = (char *) xmalloc(PATH_SIZE);
}

/* Figure out what module name to use, based on the filename, and
   return a pointer to it (located within spare_file_name). */

char *
find_name(const char *fname)
{
    const char *delims;
    char *name;
    char *token;

    /* First remove any Unix, Mac or Windows pathnames. */
    strncpy(spare_file_name, fname, PATH_SIZE);
    spare_file_name[PATH_SIZE] = '\0';

    name = spare_file_name;
    delims = "/:\\";
    token = strtok(name, delims);
    while (token != NULL) {
	name = token;
	token = strtok(NULL, delims);
    }
    /* Remove any leading dots left from Unix pathnames. */
    name += strspn(name, ".");

    return name;
}

/* A saved game should include everything necessary to recreate a game
   exactly.  It is important that this routine have the option to not
   use graphics, since it may be called when graphics code fails, and
   not to allocate memory, since it may be called upon memory
   exhaustion.  Returns TRUE if the save was successful. */

int
write_entire_game_state(const char *fname)
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

/* Given a game module telling what is in the module, write out a file
   containing the requested content.  Return true if all went OK. */

int
write_game_module(Module *module, const char *fname)
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

static void
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

/* Write definitions of all the types. */

static void
write_types(void)
{
    int u, m, t, i, ival;
    const char *name, *sval;
    Obj *obj;

    /* (or write out all the default values first for doc, then
       only write changed values) */

    for_all_unit_types(u) {
	start_form(key(K_UNIT_TYPE));
	name = shortest_escaped_name(u);
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; utypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(utypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (utypedefns[i].intgetter) {
		ival = (*(utypedefns[i].intgetter))(u);
		write_num_prop(utypedefns[i].name, ival,
			       utypedefns[i].dflt, FALSE, TRUE);
	    } else if (utypedefns[i].strgetter) {
		sval = (*(utypedefns[i].strgetter))(u);
		/* Special-case a couple possibly-redundant slots. */
		if (utypedefns[i].strgetter == u_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		if (utypedefns[i].strgetter == u_internal_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(utypedefns[i].name, sval,
			       utypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(utypedefns[i].objgetter))(u);
		write_lisp_prop(utypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
    for_all_material_types(m) {
	start_form(key(K_MATERIAL_TYPE));
	name = escaped_symbol(m_type_name(m));
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(mtypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (mtypedefns[i].intgetter) {
		ival = (*(mtypedefns[i].intgetter))(m);
		write_num_prop(mtypedefns[i].name, ival,
			       mtypedefns[i].dflt, FALSE, TRUE);
	    } else if (mtypedefns[i].strgetter) {
		sval = (*(mtypedefns[i].strgetter))(m);
		/* Special-case a a possibly-redundant slot. */
		if (mtypedefns[i].strgetter == m_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(mtypedefns[i].name, sval,
			       mtypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(mtypedefns[i].objgetter))(m);
		write_lisp_prop(mtypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
    for_all_terrain_types(t) {
	start_form(key(K_TERRAIN_TYPE));
	name = escaped_symbol(t_type_name(t));
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(ttypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (ttypedefns[i].intgetter) {
		ival = (*(ttypedefns[i].intgetter))(t);
		write_num_prop(ttypedefns[i].name, ival,
			       ttypedefns[i].dflt, FALSE, TRUE);
	    } else if (ttypedefns[i].strgetter) {
		sval = (*(ttypedefns[i].strgetter))(t);
		/* Special-case a a possibly-redundant slot. */
		if (ttypedefns[i].strgetter == t_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(ttypedefns[i].name, sval,
			       ttypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(ttypedefns[i].objgetter))(t);
		write_lisp_prop(ttypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    for_all_advance_types(t) {
	start_form(key(K_ADVANCE_TYPE));
	add_to_form(escaped_symbol(a_type_name(t)));
	newline_form();
	space_form();
	for (i = 0; atypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(atypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (atypedefns[i].intgetter) {
		ival = (*(atypedefns[i].intgetter))(t);
		write_num_prop(atypedefns[i].name, ival,
			       atypedefns[i].dflt, FALSE, TRUE);
	    } else if (atypedefns[i].strgetter) {
		sval = (*(atypedefns[i].strgetter))(t);
		/* Special-case a a possibly-redundant slot. */
		if (atypedefns[i].strgetter == a_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(atypedefns[i].name, sval,
			       atypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(atypedefns[i].objgetter))(t);
		write_lisp_prop(atypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
}

/* Write definitions of all the tables. */

static void
write_tables(int compress)
{
    int tbl;

    newline_form();
    for (tbl = 0; tabledefns[tbl].name != 0; ++tbl) {
	/* Don't write out tables used internally only, unless debugging. */
	if ((strncmp(tabledefns[tbl].name, "zz-", 3) == 0) && !Debug)
	  continue;
	if (*(tabledefns[tbl].table) != NULL) {
	    write_table(tabledefns[tbl].name,
			tabledefns[tbl].getter, tabledefns[tbl].dflt,
			tabledefns[tbl].index1, tabledefns[tbl].index2,
			tabledefns[tbl].valtype, compress);
	    newline_form();
	}
    }
}

#define star_from_typ(typ)  \
  ((typ) == UTYP ? "u*" : ((typ) == MTYP ? "m*" : ((typ) == TTYP ? "t*" : "a*")))

#define name_from_typ(typ, i)  \
  ((typ) == UTYP ? shortest_escaped_name(i) : ((typ) == MTYP ? m_type_name(i) : \
  ((typ) == TTYP ? t_type_name(i) : a_type_name(i))))

static void
write_type_name_list(int typ, int *flags, int dim)
{
    int j, first = TRUE, listlen = 0;

    if (flags == NULL)
      return;
    for (j = 0; j < dim; ++j)
      if (flags[j])
        ++listlen;
    if (listlen > 1)
      start_form("");
    for (j = 0; j < dim; ++j) {
	if (flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      space_form();
	    add_to_form_no_space(escaped_symbol(name_from_typ(typ, j)));
	}
    }
    if (listlen > 1)
      end_form();
}

#if 0
/* Write out a list of values in a table. */

static void
write_type_value_list(typ, flags, dim, getter, i)
int typ, *flags, dim, (*getter)(int, int), i;
{
    int j, first = TRUE, listlen = 0;

    for (j = 0; j < dim; ++j)
      if (flags == NULL || flags[j])
        ++listlen;
    if (listlen > 1)
      start_form("");
    for (j = 0; j < dim; ++j) {
	if (flags == NULL || flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      space_form();
	    add_num_to_form_no_space((*getter)(i, j));
	}
    }
    if (listlen > 1)
      end_form();
}
#endif

/* A simple histogram struct - count and value, that's all. */

struct histo { int count, val; };

/* Sort into *descending* order by count. */

static int histo_compare(CONST void *x, CONST void *y);

static int
histo_compare(CONST void *x, CONST void *y)
{
    return ((struct histo *) y)->count - ((struct histo *) x)->count;
}

/* Write out a single table.  Only write it if it contains non-default
   values, and try to find runs of constant value, since tables can be
   really large, but often have constant areas within them. */

static void
write_table(const char *name, int (*getter)(int, int), int dflt, int typ1, int typ2,
	    int valtype, int compress)
{
    int i, j, k, colvalue, constcol, next;
    int numrandoms, nextrowdiffers, writeconst;
    int sawfirst, constrands, constval;
    int dim1 = numtypes_from_index_type(typ1);
    int dim2 = numtypes_from_index_type(typ2);
    struct histo mostcommon[500]; /* more than max of num[utma]types */
    int indexes1[500], randoms[500];

    start_form(key(K_TABLE));
    add_to_form(name);
    add_to_form(" ;");
    add_num_or_dice_to_form(dflt, valtype);
    if (!compress) {
	/* Write every value separately. */
	for (i = 0; i < dim1; ++i) {
	    newline_form();
	    space_form();
	    space_form();
	    start_form(escaped_symbol(name_from_typ(typ1, i)));
	    add_to_form((char *)star_from_typ(typ2));
	    space_form();
	    start_form("");
	    for (j = 0; j < dim2; ++j) {
		add_num_or_dice_to_form((*getter)(i, j), valtype);
	    }
	    end_form();
	    end_form();
	}
    } else if (dim1 <= dim2) {
        /* Analyze the table by rows. */
	for (k = 0; k < dim1; ++k)
	  indexes1[k] = FALSE;
	for (i = 0; i < dim1; ++i) {
	    /* First see if this row has all the same values as the next. */
	    indexes1[i] = TRUE;
	    nextrowdiffers = FALSE;
	    if (i < dim1 - 1) {
	    	for (j = 0; j < dim2; ++j) {
	    	    if ((*getter)(i, j) != (*getter)(i + 1, j)) {
	    	    	nextrowdiffers = TRUE;
	    	    	break;
	    	    }
	    	}
	    } else {
	    	/* The last row is *always* "different". */
	    	nextrowdiffers = TRUE;
	    }
	    /* (should look at *all* rows to find matching rows before
	       dumping one) */
	    if (nextrowdiffers) {
		/* Make a histogram of all the values in this row. */
		mostcommon[0].count = 1;
		mostcommon[0].val = (*getter)(i, 0);
		next = 1;
		for (j = 0; j < dim2; ++j) {
		    for (k = 0; k < next; ++k) {
			if (mostcommon[k].val == (*getter)(i, j)) {
			    ++(mostcommon[k].count);
			    break;
			}
		    }
		    if (k == next) {
			mostcommon[next].count = 1;
			mostcommon[next].val = (*getter)(i, j);
			++next;
		    }
		}
		if (next == 1 && mostcommon[0].val == dflt) {
		    /* Entire row(s) is/are just the default table value. */
		} else {
		    writeconst = FALSE;
		    numrandoms = 0;
		    if (next == 1) {
			/* Only one value in the row(s). */
			writeconst = TRUE;
		    } else {
			qsort(mostcommon, next, sizeof(struct histo),
			      histo_compare);
			if (mostcommon[0].count >= (3 * dim2) / 4) {
			    /* The most common value in this row(s) is
			       not the only value, but it is worth
			       writing into a separate clause. */
			    writeconst = TRUE;
			    for (j = 0; j < dim2; ++j) {
				/* Flag the other values as needing to be
				   written separately. */
				randoms[j] =
				  (mostcommon[0].val != (*getter)(i, j));
				if (randoms[j])
				  ++numrandoms;
			    }
			} else {
			    /* Flag all in the row as randoms. */
			    for (j = 0; j < dim2; ++j) {
				randoms[j] = TRUE;
				++numrandoms;
			    }
			}
		    }
		    /* Write out the most common value (if
		       non-default) in the row(s), expressing it with
		       a clause that applies the value to the entire
		       row(s). */
		    if (writeconst && mostcommon[0].val != dflt) {
			newline_form();
			space_form();
			space_form();
			start_form("");
			write_type_name_list(typ1, indexes1, dim1);
			add_to_form((char *)star_from_typ(typ2));
			add_num_or_dice_to_form(mostcommon[0].val, valtype);
			end_form();
		    }
		    /* Now override the most common value with any
		       exceptions. */
		    if (numrandoms > 0) {
			constrands = TRUE;
			sawfirst = FALSE;
			for (j = 0; j < dim2; ++j) {
			    if (randoms[j]) {
			        if (!sawfirst) {
				    constval = (*getter)(i, j);
				    sawfirst = TRUE;
			        }
			        if (sawfirst && constval != (*getter)(i, j)) {
				    constrands = FALSE;
				    break;
			        }
			    }
			}
			if (constrands) {
			    newline_form();
			    space_form();
			    space_form();
			    start_form("");
			    write_type_name_list(typ1, indexes1, dim1);
			    space_form();
			    write_type_name_list(typ2, randoms, dim2);
			    add_num_or_dice_to_form(constval, valtype);
			    end_form();
			} else {
			    /* We have a group of rows with varying data
			       in the columns; write a separate row. */
			    for (j = 0; j < dim2; ++j) {
				if (randoms[j]) {
				    newline_form();
				    space_form();
				    space_form();
				    start_form("");
				    write_type_name_list(typ1, indexes1, dim1);
				    add_to_form(escaped_symbol(name_from_typ(typ2, j)));
				    add_num_or_dice_to_form((*getter)(i, j), valtype);
				    end_form();
				}
			    }
			}
		    }
		}
		/* Reset the row flags in preparation for the next group
		   of rows whose contents match each other. */
		for (k = 0; k < dim1; ++k)
		  indexes1[k] = FALSE;
	    }
	}
    } else {
        /* Analyze the table by columns. */
        /* Don't work as hard to optimize; this case should be uncommon,
	   since there are usually more types of units than
	   materials or terrain. */
	for (j = 0; j < dim2; ++j) {
	    constcol = TRUE;
	    colvalue = (*getter)(0, j);
	    for (i = 0; i < dim1; ++i) {
		if ((*getter)(i, j) != colvalue) {
		    constcol = FALSE;
		    break;
		}
	    }
	    if (!constcol || colvalue != dflt) {
		newline_form();
		space_form();
		space_form();
		start_form((char *)star_from_typ(typ1));
		add_to_form(escaped_symbol(name_from_typ(typ2, j)));
		/* Write out either a single constant value or a list of
		   varying values, as appropriate. */
		if (constcol) {
		    add_num_or_dice_to_form(colvalue, valtype);
		} else {
		    space_form();
		    start_form("");
		    for (i = 0; i < dim1; ++i) {
			add_num_or_dice_to_form((*getter)(i, j), valtype);
		    }
		    end_form();
		}
		end_form();
	    }
	}
    }
    newline_form();
    space_form();
    space_form();
    end_form();
    newline_form();
}

/* Write info about the whole world. */

static void
write_world(void)
{
    newline_form();
    start_form(key(K_WORLD));
    /* K_CIRCUMFERENCE always written. */
    add_num_to_form((doreshape ? reshaper->final_circumference : world.circumference));
    write_num_prop(key(K_DAY_LENGTH), world.daylength, 1, FALSE, FALSE);
    write_num_prop(key(K_YEAR_LENGTH), world.yearlength, 1, FALSE, FALSE);
    write_num_prop(key(K_AXIAL_TILT), world.axial_tilt, 0, FALSE, FALSE);
    write_num_prop(key(K_DAYLIGHT_FRACTION), world.daylight_fraction, 0, FALSE, FALSE);
    write_num_prop(key(K_TWILIGHT_FRACTION), world.twilight_fraction, 0, FALSE, FALSE);
    end_form();
    newline_form();
}

/* Write info about the area in the world.  This code uses run-length
   encoding to reduce the size of each written layer as much as
   possible.  Note also that each layer is written as a separate form,
   so that the Lisp reader doesn't have to read really large forms
   back in. */

static void
write_areas(Module *module)
{
    int all = module->def_all, compress = module->compress_layers;

    newline_form();
    /* Write the basic dimensions. */
    start_form(key(K_AREA));
    /* K_WIDTH, K_HEIGHT written if nonzero. */
    if (area.width > 0 || area.height > 0) {
	add_num_to_form((doreshape ? reshaper->final_width : area.width));
	add_num_to_form((doreshape ? reshaper->final_height : area.height));
    }
    /* Write all the scalar properties. */
    write_num_prop(key(K_LATITUDE), area.latitude, 0, 0, FALSE);
    write_num_prop(key(K_LONGITUDE), area.longitude, 0, 0, FALSE);
    write_num_prop(key(K_PROJECTION), area.projection, 0, 0, FALSE);
    write_num_prop(key(K_CELL_WIDTH), area.cellwidth, 0, 0, FALSE);
    if (area.sunx != area.width / 2 || area.suny != area.halfheight) {
	start_form(key(K_SUN));
	add_num_to_form(area.sunx);
	add_num_to_form(area.suny);
	end_form();
	newline_form();
    }
    write_lisp_prop(key(K_TEMPERATURE_YEAR_CYCLE), area.temp_year,
		    lispnil, FALSE, FALSE, TRUE);
    write_str_prop(key(K_IMAGE_NAME), area.image_name,
		   "", FALSE, TRUE);
    end_form();
    newline_form();
    /* Write the area's layers, each as a separate form. */
    if (all || module->def_area_terrain)
      write_area_terrain(compress);
    if (all || module->def_area_terrain)
      write_area_aux_terrain(compress);
    if (all || module->def_area_misc)
      write_area_features(compress);
    if (all || module->def_area_misc)
      write_area_elevations(compress);
    if (all || module->def_area_misc)
      write_area_people_sides(compress);
    if (all || module->def_area_misc)
      write_area_control_sides(compress);
    if (all || module->def_area_material)
      write_area_materials(compress);
    if (all || module->def_area_weather)
      write_area_temperatures(compress);
    if (all || module->def_area_weather)
      write_area_clouds(compress);
    if (all || module->def_area_weather)
      write_area_winds(compress);
   /* It is alloc_area_terrain that handles the user layer. */
   if (all || module->def_area_terrain)
      write_area_users(compress);
}

/* Write out the city usage layer. */

static void
write_area_users(int compress)
{
    extern int nextid;

    if (!user_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_USER));
    newline_form();
    write_rle(fn_user_at, NOUSER, nextid, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

static void
write_area_terrain(int compress)
{
    int t;

    /* Terrain might not be set up when downloading at beginning of game,
       don't try to write if so. */
    if (area.terrain == NULL)
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_TERRAIN));
    newline_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_terrain_types(t) {
	/* Break the list into groups of 5 per line. */
    	if (t % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form(escaped_symbol(t_type_name(t)));
	add_num_to_form(t);
	end_form();
    }
    end_form();
    newline_form();
    write_rle(fn_terrain_at, 0, numttypes - 1, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

static void
write_area_aux_terrain(int compress)
{
    int t;

    for_all_terrain_types(t) {
	if (aux_terrain_defined(t)) {
	    start_form(key(K_AREA));
	    space_form();
	    start_form(key(K_AUX_TERRAIN));
	    add_to_form(escaped_symbol(t_type_name(t)));
	    newline_form();
	    tmpttype = t;
	    write_rle(fn_aux_terrain_at, 0, 127, NULL, compress);
	    space_form();
	    space_form();
	    end_form();
	    end_form();
	    newline_form();
	}
    }
}

static void
write_area_features(int compress)
{
    Feature *feature;

    if (featurelist == NULL || !features_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_FEATURES));
    space_form();
    start_form("");
    newline_form();
    /* Dump out the list of features first. */
    for_all_features(feature) {
	space_form();
	space_form();
	space_form();
	start_form("");
	add_num_to_form(feature->id);
	add_to_form(escaped_string(feature->feattype));
	add_to_form(escaped_string(feature->name));
	end_form();
	newline_form();
    }
    space_form();
    space_form();
    end_form();
    newline_form();
    /* Now record which features go with which cells. */
    write_rle(fn_feature_at, 0, -1, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

static int fn_elevation_at_offset(int x, int y);

static int
fn_elevation_at_offset(int x, int y)
{
    return (elev_at(x, y) - area.minelev);
}

static void
write_area_elevations(int compress)
{
    if (!elevations_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_ELEVATIONS));
    if (area.minelev != 0) {
	space_form();
	start_form(key(K_XFORM));
	add_num_to_form(1);
	add_num_to_form(area.minelev);
	end_form();
    }
    newline_form();
    write_rle(fn_elevation_at_offset, 0, area.maxelev - area.minelev,
	      NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

/* Write out people sides for the area. */

static void
write_area_people_sides(int compress)
{
    Side *side;

    if (!people_sides_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_PEOPLE_SIDES));
    newline_form();
    space_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_sides(side) {
	/* Break the list into groups of 5 per line. */
	if (side_number(side) % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form("");
	add_num_to_form(side_number(side)); /* should be symbol */
	add_num_to_form(side_number(side));
	end_form();
    }
    end_form();
    newline_form();
    /* Value of NOBODY is guaranteed to be the largest valid. */
    write_rle(fn_people_side_at, 0, NOBODY, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

/* Write out control sides for the area. */

static void
write_area_control_sides(int compress)
{
    Side *side;

    if (!control_sides_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_CONTROL_SIDES));
    newline_form();
    space_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_sides(side) {
	/* Break the list into groups of 5 per line. */
	if (side_number(side) % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form("");
	add_num_to_form(side_number(side)); /* should be symbol */
	add_num_to_form(side_number(side));
	end_form();
    }
    end_form();
    newline_form();
    /* Value of NOCONTROL is guaranteed to be the largest valid. */
    write_rle(fn_control_side_at, 0, NOCONTROL, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

static void
write_area_materials(int compress)
{
    int m;

    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		start_form(key(K_AREA));
		space_form();
		start_form(key(K_MATERIAL));
		add_to_form(escaped_symbol(m_type_name(m)));
		newline_form();
		tmpmtype = m;
		write_rle(fn_material_at, 0, 127, NULL, compress);
		end_form();
		end_form();
		newline_form();
	    }
	}
    }
}

static void
write_area_temperatures(int compress)
{
    if (!temperatures_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_TEMPERATURES));
    newline_form();
    write_rle(fn_temperature_at, -9999, 9999, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

static void
write_area_clouds(int compress)
{
    if (clouds_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUDS));
	newline_form();
	write_rle(fn_raw_cloud_at, 0, 127, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (cloud_bottoms_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUD_BOTTOMS));
	newline_form();
	write_rle(fn_raw_cloud_bottom_at, 0, 9999, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (cloud_heights_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUD_HEIGHTS));
	newline_form();
	write_rle(fn_raw_cloud_height_at, 0, 9999, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
}

static void
write_area_winds(int compress)
{
    if (winds_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_WINDS));
	newline_form();
	write_rle(fn_raw_wind_at, -32767, 32767, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
}

/* Write the globals.  The "complete" flag forces all values out, even
   if they match the compiled-in defaults. */

static void
write_globals(void)
{
    int i, complete = FALSE;
    Obj *val;
    time_t now;

    /* Snapshot realtime values, but only after they've been set up. */
    if (g_elapsed_time() >= 0) {
	time(&now);
	set_g_elapsed_time(idifftime(now, game_start_in_real_time));
    }

    newline_form();

    for (i = 0; vardefns[i].name != 0; ++i) {
	if (debugging_state_sync
	    && strcmp(vardefns[i].name, "elapsed-real-time") == 0)
	  continue;
	if (vardefns[i].intgetter != NULL) {
	    if (complete || (*(vardefns[i].intgetter))() != vardefns[i].dflt) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		add_num_to_form((*(vardefns[i].intgetter))());
		end_form();
		newline_form();
	    }
	} else if (vardefns[i].strgetter != NULL) {
	    if (complete
		|| string_not_default((*(vardefns[i].strgetter))(),
				      vardefns[i].dfltstr)) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		add_to_form(escaped_string((*(vardefns[i].strgetter))()));
		end_form();
		newline_form();
	    }
	} else if (vardefns[i].objgetter != NULL) {
	    val = (*(vardefns[i].objgetter))();
	    if (complete
		|| val != lispnil
		/* Must write out explicit nil setting if the default
		   function is defined, so it won't re-run on startup. */
		|| (val == lispnil && vardefns[i].dfltfn != NULL)) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		space_form();
		/* Suppress evaluation upon readin. */
		add_to_form_no_space("'");
		add_form_to_form((*(vardefns[i].objgetter))());
		end_form();
		newline_form();
	    }
	} else {
#ifdef __cplusplus
	    throw "snafu";
#else
	    abort();
#endif
	}
    }
}

/* Write all the scorekeepers. */

static void
write_scorekeepers(void)
{
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	start_form(key(K_SCOREKEEPER));
	add_num_to_form(sk->id);
	newline_form();
	space_form();
	write_str_prop(key(K_TITLE), sk->title, "", FALSE, TRUE);
	write_lisp_prop(key(K_WHEN), sk->when, lispnil, FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_APPLIES_TO), sk->who, lispnil, 
			FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_KNOWN_TO), sk->who, lispnil, FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_TRIGGER), sk->trigger, lispnil, 
			FALSE, FALSE, TRUE);
	write_lisp_prop(key(K_DO), sk->body, lispnil, FALSE, FALSE, TRUE);
	write_num_prop(key(K_TRIGGERED), sk->triggered, 0, FALSE, TRUE); 
	write_num_prop(key(K_KEEP_SCORE), sk->keepscore, 0, FALSE, TRUE); 
	write_num_prop(key(K_INITIAL_SCORE), sk->initial, -10001, FALSE, TRUE); 
	write_lisp_prop(key(K_NOTES), sk->notes, lispnil, FALSE, FALSE, TRUE);
	space_form();
	end_form();
	newline_form();
    }
}

static void
write_doctrines(void)
{
    Doctrine *doc;

    for_all_doctrines(doc) {
	/* (should filter out doctrines with no non-default values) */
	start_form(key(K_DOCTRINE));
	if (doc->name)
	  add_to_form(escaped_symbol(doc->name));
	else
	  add_num_to_form(doc->id);
	newline_form();
	space_form();
	write_num_prop(key(K_RESUPPLY_PERCENT), doc->resupply_percent,
		       /* (should get these defaults from a common place) */
		       50, FALSE, TRUE); 
	write_num_prop(key(K_REARM_PERCENT), doc->rearm_percent,
		       20, FALSE, TRUE); 
	write_num_prop(key(K_REPAIR_PERCENT), doc->repair_percent,
		       35, FALSE, TRUE); 
	write_utype_value_list(key(K_CONSTRUCTION_RUN), doc->construction_run,
			       0, TRUE);
	write_num_prop(key(K_LOCKED), doc->locked, 0, FALSE, TRUE); 
	space_form();
	end_form();
	newline_form();
    }
}

/* Write declarations of all the sides. */

static void
write_sides(Module *module)
{
    Side *side;

    newline_form();
    add_to_form(";");
    add_num_to_form(numtotsides);
    add_to_form("sides including the independent 0 side.");
    newline_form();
    Dprintf("Will try to write %d sides ...\n", numtotsides);
    for_all_real_sides(side) {
	start_form(key(K_SIDE));
	add_num_to_form(side->id);
	if (symbolp(side->symbol))
	  add_to_form(escaped_symbol(c_string(side->symbol)));
	newline_form();
	space_form();
	write_side_properties(side);
	space_form();
	end_form();
	newline_form();
	if (module->def_all || module->def_side_views)
	  write_side_view(side, module->compress_layers);
	Dprintf("  Wrote side %s\n", side_desig(side));
    }
    /* Even though the indepside is another side, the properties of
       independent units are a separate type of form. */
    start_form(key(K_INDEPENDENT_UNITS));
    newline_form();
    write_side_properties(indepside);
    space_form();
    end_form();
    newline_form();
    Dprintf("  Wrote independent unit properties\n");
    Dprintf("... Done writing sides\n");
}

/* Write random properties of a side. */

static void
write_side_properties(Side *side)
{
    int i, u, u2, anyudoctrines;

    write_str_prop(key(K_NAME), side->name, "", FALSE, TRUE);
    write_str_prop(key(K_LONG_NAME), side->longname, "", FALSE, TRUE);
    write_str_prop(key(K_SHORT_NAME), side->shortname, "", FALSE, TRUE);
    write_str_prop(key(K_NOUN), side->noun, "", FALSE, TRUE);
    write_str_prop(key(K_PLURAL_NOUN), side->pluralnoun, "", FALSE, TRUE);
    write_str_prop(key(K_ADJECTIVE), side->adjective, "", FALSE, TRUE);
    write_str_prop(key(K_COLOR), side->colorscheme, "", FALSE, TRUE);
    write_str_prop(key(K_EMBLEM_NAME), side->emblemname, "", FALSE, TRUE);
    write_utype_string_list(key(K_UNIT_NAMERS), side->unitnamers, "", TRUE);
    write_lisp_prop(
	key(K_UNITS), side->possible_units, lispnil, FALSE, FALSE, TRUE);
    write_str_prop(key(K_CLASS), side->sideclass, "", FALSE, TRUE);
    write_bool_prop(key(K_ACTIVE), side->ingame, 1, FALSE, TRUE);
    /* side->everingame is only interesting if it differs from ingame,
       so use the value of ingame as the default. */
    write_bool_prop(key(K_EVER_ACTIVE), side->everingame, side->ingame,
		    FALSE, TRUE);
    write_num_prop(key(K_PRIORITY), side->priority, -1, FALSE, TRUE);
    write_num_prop(key(K_STATUS), side->status, 0, FALSE, TRUE);
    if (!debugging_state_sync)
      write_num_prop(key(K_TURN_TIME_USED), side->turntimeused, 0, FALSE, TRUE);
    if (!debugging_state_sync)
      write_num_prop(
	key(K_TOTAL_TIME_USED), side->totaltimeused, 0, FALSE, TRUE);
    write_num_prop(key(K_TIMEOUTS), side->timeouts, 0, FALSE, TRUE);
    write_num_prop(key(K_TIMEOUTS_USED), side->timeoutsused, 0, FALSE, TRUE);
    write_num_prop(key(K_FINISHED_TURN), side->finishedturn, 0, FALSE, TRUE);
    write_num_prop(key(K_WILLING_TO_DRAW), side->willingtodraw, 0, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE), side->advantage, 1, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE_MIN), side->minadvantage, 1, FALSE, TRUE);
    write_num_prop(key(K_ADVANTAGE_MAX), side->maxadvantage, 1, FALSE, TRUE);
    write_num_prop(
	key(K_CONTROLLED_BY), 
	(side->controlled_by ? side_number(side->controlled_by) : -1), 
	-1, FALSE, TRUE);
    write_num_prop(
	key(K_SELF_UNIT), 
	(side->self_unit ? side->self_unit->id : 0), 0, FALSE, TRUE);
    write_num_prop(
	key(K_PLAYER), (side->player ? side->player->id : -1), -1, FALSE, TRUE);
    write_num_prop(key(K_DEFAULT_DOCTRINE),
    		   (side->default_doctrine ? side->default_doctrine->id : 0), 
		   0, FALSE, TRUE);
    write_num_prop(key(K_CURRENT_ADVANCE), side->research_topic, NOADVANCE, 
		   FALSE, TRUE);
    write_num_prop(key(K_ADVANCE_GOAL), side->research_goal, NONATYPE, FALSE, 
		   TRUE);
    write_atype_value_list(key(K_ADVANCES_DONE), side->advance, 0, TRUE);
    write_treasury_list(key(K_TREASURY), side->treasury, 0, TRUE);
    anyudoctrines = FALSE;
    if (side->udoctrine != NULL) {
	for_all_unit_types(u) {
	    if (side->udoctrine[u] != side->default_doctrine) {
		anyudoctrines = TRUE;
		break;
	    }
	}
    }
    if (anyudoctrines) {
	space_form();
	start_form(key(K_DOCTRINES));
	for_all_unit_types(u) {
	    if (side->udoctrine[u] != side->default_doctrine) {
		space_form();
		start_form(shortest_escaped_name(u));
		if (side->udoctrine[u]->name)
		  add_to_form(escaped_symbol(side->udoctrine[u]->name));
		else
		  add_num_to_form(side->udoctrine[u]->id);
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    write_side_value_list(key(K_TRUSTS), side->trusts, FALSE, TRUE);
    write_side_value_list(key(K_TRADES), side->trades, FALSE, TRUE);
    write_utype_value_list(key(K_START_WITH), side->startwith, 0, TRUE);
    write_utype_value_list(key(K_NEXT_NUMBERS), side->counts, 0, TRUE);
    write_utype_value_list(key(K_TECH), side->tech, 0, TRUE);
    write_utype_value_list(key(K_INIT_TECH), side->inittech, 0, TRUE);
    write_utype_value_list(key(K_ALREADY_SEEN), side->already_seen, -1, TRUE);
    write_utype_value_list(
	key(K_ALREADY_SEEN_INDEPENDENT), side->already_seen_indep, -1, TRUE);
    write_standing_orders(side);
    if (side->scores) {
	space_form();
	start_form(key(K_SCORES));
	for (i = 0; i < numscores; ++i) {
	    add_num_to_form(side->scores[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    write_lisp_prop(key(K_INSTRUCTIONS), side->instructions,
		    lispnil, FALSE, FALSE, TRUE);
    /* Write out statistics. */
    if (side->gaincounts != NULL) {
	space_form();
	start_form(key(K_GAIN_COUNTS));
	for (i = 0; i < numutypes * num_gain_reasons; ++i) {
	    add_num_to_form(side->gaincounts[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->losscounts != NULL) {
	space_form();
	start_form(key(K_LOSS_COUNTS));
	for (i = 0; i < numutypes * num_loss_reasons; ++i) {
	    add_num_to_form(side->losscounts[i]);
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->atkstats != NULL) {
	space_form();
	start_form(key(K_ATTACK_STATS));
	for_all_unit_types(u) {
	    if (side->atkstats[u] != NULL) {
		newline_form();
		space_form();
		space_form();
		space_form();
		start_form(shortest_escaped_name(u));
		for_all_unit_types(u2) {
		    add_num_to_form(side_atkstats(side, u, u2));
		}
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->hitstats != NULL) {
	space_form();
	start_form(key(K_HIT_STATS));
	for_all_unit_types(u) {
	    if (side->hitstats[u] != NULL) {
		newline_form();
		space_form();
		space_form();
		space_form();
		start_form(shortest_escaped_name(u));
		for_all_unit_types(u2) {
		    add_num_to_form(side_hitstats(side, u, u2));
		}
		end_form();
	    }
	}
	end_form();
	newline_form();
	space_form();
    }
    if (side->could_act_with)
	write_utype_value_list(
	    key(K_COULD_ACT_WITH), side->could_act_with, FALSE, TRUE);
    if (side->could_construct)
	write_utype_value_list(
	    key(K_COULD_CONSTRUCT), side->could_construct, FALSE, TRUE);
    if (side->could_develop)
	write_utype_value_list(
	    key(K_COULD_DEVELOP), side->could_develop, FALSE, TRUE);
    if (side->could_research)
	write_atype_value_list(
	    key(K_COULD_RESEARCH), side->could_research, FALSE, TRUE);
    /* Have the AI paste its useful state into distinct element of
       side->aidata. */
    if (side_has_local_ai(side)) {
	ai_save_state(side);
    }
    write_lisp_prop(key(K_AI_DATA), side->aidata, lispnil, FALSE, TRUE, TRUE);
    write_lisp_prop(key(K_UI_DATA), side->uidata, lispnil, FALSE, TRUE, TRUE);
}

static void 
write_standing_orders(Side *side)
{
    int u, u1, numtypes, numargs;
    const char *str = NULL;
    StandingOrder *sorder;

    for (sorder = side->orders; sorder != NULL; sorder = sorder->next) {
	space_form();
	start_form(key(K_STANDING_ORDER));
	/* (should break into a write_utype_list) */
	numtypes = u1 = 0;
	for_all_unit_types(u) {
	    if (sorder->types[u]) {
		++numtypes;
		u1 = u;
	    }
	}
	if (numtypes == 1) {
	    add_to_form(shortest_escaped_name(u1));
	} else if (numtypes == numutypes) {
	    add_to_form(key(K_USTAR));
	} else {	
	    space_form();
	    start_form("");
	    for_all_unit_types(u) 
	      add_num_to_form(sorder->types[u]);
	    end_form();
	}
	space_form();
	switch (sorder->condtype) {
	  case sorder_always:
	    str = key(K_ALWAYS);
	    numargs = 0;
	    break;
	  case sorder_at:
	    str = key(K_AT);
	    numargs = 2;
	    break;
	  case sorder_in:
	    str = key(K_IN);
	    numargs = 1;
	    break;
	  case sorder_near:
	    str = key(K_NEAR);
	    numargs = 3;
	    break;
	  default:
	    case_panic("standing order condition type", sorder->condtype);
	    break;
	}
	start_form(str);
	if (numargs > 0)
	  add_num_to_form(sorder->a1);
	if (numargs > 1)
	  add_num_to_form(sorder->a2);
	if (numargs > 2)
	  add_num_to_form(sorder->a3);
	end_form();
	space_form();
	write_task(sorder->task);
	end_form();
	newline_form();
	space_form();
    }
}
/* Write about what has been seen in the area. */

static int
fn_terrain_view(int x, int y)
{
    return terrain_view(tmpside, x, y);
}

static int
fn_terrain_view_date(int x, int y)
{
    return terrain_view_date(tmpside, x, y);
}

static int
fn_aux_terrain_view(int x, int y)
{
    return aux_terrain_view(tmpside, x, y, tmpttype);
}

static int
fn_aux_terrain_view_date(int x, int y)
{
    return aux_terrain_view_date(tmpside, x, y, tmpttype);
}

static int
fn_material_view(int x, int y)
{
    return material_view(tmpside, x, y, tmpmtype);
}

static int
fn_material_view_date(int x, int y)
{
    return material_view_date(tmpside, x, y, tmpmtype);
}

static int
fn_temp_view(int x, int y)
{
    return temperature_view(tmpside, x, y);
}

static int
fn_temp_view_date(int x, int y)
{
    return temperature_view_date(tmpside, x, y);
}

static int
fn_cloud_view(int x, int y)
{
    return cloud_view(tmpside, x, y);
}

static int
fn_cloud_bottom_view(int x, int y)
{
    return cloud_bottom_view(tmpside, x, y);
}

static int
fn_cloud_height_view(int x, int y)
{
    return cloud_height_view(tmpside, x, y);
}

static int
fn_cloud_view_date(int x, int y)
{
    return cloud_view_date(tmpside, x, y);
}

static int
fn_wind_view(int x, int y)
{
    return wind_view(tmpside, x, y);
}

static int
fn_wind_view_date(int x, int y)
{
    return wind_view_date(tmpside, x, y);
}

static void
write_side_view(Side *side, int compress)
{
    int t, m, x, y;
    UnitView *uview;

    tmpside = side;
    tmpcompress = compress;
    if (side->terrview)
      write_one_side_view_layer(K_TERRAIN_VIEW, fn_terrain_view);
    if (!g_see_terrain_always()) {
	if (side->terrview)
	  write_one_side_view_layer(K_TERRAIN_VIEW_DATES,
				    fn_terrain_view_date);
	if (numcelltypes < numttypes) {
	    for_all_terrain_types(t) {
		if (!t_is_cell(t)) {
		    tmpttype = t;
		    if (side->auxterrview[t])
		      write_one_side_view_layer(K_AUX_TERRAIN_VIEW,
						fn_aux_terrain_view);
	/* Caused crashes since the code that allocates this layer in init_view
	is currently commented out. */
#if 0
		    if (side->auxterrviewdate[t])
		      write_one_side_view_layer(K_AUX_TERRAIN_VIEW_DATES,
						fn_aux_terrain_view_date);
#endif
		}
	    }
        }
    }
    if (side->materialview) {
	for_all_material_types(m) {
	    if (1) {
		tmpmtype = m;
		if (side->materialview[m])
		  write_one_side_view_layer(K_MATERIAL_VIEW, fn_material_view);
		if (side->materialviewdate[m])
		  write_one_side_view_layer(K_MATERIAL_VIEW_DATES,
					    fn_material_view_date);
	    }
	}
    }
    /* Write out unit view objects. */
    if (!side->see_all && side->unit_views != NULL) {
	newline_form();
	start_form(key(K_SIDE));
	add_num_to_form(side->id);
	space_form();
	start_form(key(K_UNIT_VIEWS));
	newline_form();
	for_all_cells(x, y) {
	    for_all_view_stack_with_occs(side, x, y, uview) {
		/* (should filter out side's own units?) */
		space_form();
		space_form();
		start_form(shortest_escaped_name(uview->type));
		add_num_to_form(uview->siden);
		add_to_form(uview->name);
		add_num_to_form(uview->size);
		add_num_to_form(uview->x);
		add_num_to_form(uview->y);
		add_num_to_form(uview->complete);
		add_num_to_form(uview->date);
		add_num_to_form(uview->id);
		add_to_form(uview->image_name);
		end_form();
		newline_form();
	    }
	}
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (!g_see_weather_always()) {
	if (side->tempview) {
	    write_one_side_view_layer(K_TEMPERATURE_VIEW, fn_temp_view);
	    write_one_side_view_layer(K_TEMPERATURE_VIEW_DATES,
				      fn_temp_view_date);
	}
	if (side->cloudview) {
	    write_one_side_view_layer(K_CLOUD_VIEW, fn_cloud_view);
	    write_one_side_view_layer(K_CLOUD_BOTTOM_VIEW,
				      fn_cloud_bottom_view);
	    write_one_side_view_layer(K_CLOUD_HEIGHT_VIEW,
				      fn_cloud_height_view);
	    write_one_side_view_layer(K_CLOUD_VIEW_DATES, fn_cloud_view_date);
	}
	if (side->windview) {
	    write_one_side_view_layer(K_WIND_VIEW, fn_wind_view);
	    write_one_side_view_layer(K_WIND_VIEW_DATES, fn_wind_view_date);
	}
    }
}

static void
write_one_side_view_layer(int propkey, int (*fn)(int x, int y))
{
    newline_form();
    start_form(key(K_SIDE));
    add_num_to_form(tmpside->id);
    space_form();
    start_form(key((enum keywords)propkey));
    newline_form();
    write_rle(fn, -32767, 32767, NULL, tmpcompress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

/* Dump all the players in the game. */

static void
write_players(void)
{
    Side *side;

    Dprintf("Will try to write players ...\n");
    for_all_sides(side) {
	if (side->player != NULL) {
	    write_player(side->player);
	    Dprintf("Wrote player %s,\n", player_desig(side->player));
	}
    }
    Dprintf("... Done writing players.\n");
}

/* Dump everything about a single given player. */

static void
write_player(Player *player)
{
    start_form(key(K_PLAYER));
    add_num_to_form(player->id);
    newline_form();
    space_form();
    write_str_prop(key(K_NAME), player->name, "", FALSE, TRUE);
    write_str_prop(key(K_CONFIG_NAME), player->configname, "", FALSE, TRUE);
    write_str_prop(key(K_DISPLAY_NAME), player->displayname, "", FALSE, TRUE);
    write_str_prop(key(K_AI_TYPE_NAME), player->aitypename, "", FALSE, TRUE);
    space_form();
    end_form();
    newline_form();
}

#if 0

static void
write_agreements(void)
{
    Agreement *ag;

    for_all_agreements(ag) {
	write_agreement(ag);
    }
}

static void
write_agreement(Agreement *agreement)
{
    start_form(key(K_AGREEMENT));
    add_num_to_form(agreement->id);
    newline_form();
    space_form();
    write_str_prop(key(K_TYPE_NAME), agreement->agtype, "", FALSE, TRUE);
    write_str_prop(key(K_NAME), agreement->name, "", FALSE, TRUE);
    write_num_prop(key(K_STATE), agreement->state, 0, FALSE, TRUE);
    write_lisp_prop(key(K_TERMS), agreement->terms, lispnil, FALSE, FALSE, TRUE);
    write_num_prop(key(K_DRAFTERS), agreement->drafters, 0, FALSE, TRUE);
    write_num_prop(key(K_PROPOSERS), agreement->proposers, 0, FALSE, TRUE);
    write_num_prop(key(K_SIGNERS), agreement->signers, 0, FALSE, TRUE);
    write_num_prop(key(K_WILLING_TO_SIGN), agreement->willing, 0, FALSE, TRUE);
    write_num_prop(key(K_KNOWN_TO), agreement->knownto, 0, FALSE, TRUE);
    write_num_prop(key(K_ENFORCEMENT), agreement->enforcement, 0, FALSE, TRUE);
    space_form();
    end_form();
    newline_form();
}

#endif

/* Should write out "unit groups" with dict prepended, then can use with
   multiple games */

/* Write the unit section of a game module. */

static void
write_units(Module *module)
{
    int x0, y0, x, y, numtowrite;
    Unit *unit;
    Side *loopside;

    /* Make sure no dead units get saved. */
    flush_dead_units();
    /* Make a consistent ordering. */
    sort_units(module->def_all || module->def_unit_ids);
    numtowrite = 0;
    for_all_sides(loopside) {
	for_all_side_units(loopside, unit) {
	    if (alive(unit))
	      ++numtowrite;
	}
    }
    add_to_form(";");
    add_num_to_form(numtowrite);
    add_to_form(" units");
    newline_form();
    /* Need to write out the defaults being assumed subsequently. */
    /* maybe use those in postprocessing. */
    start_form(key(K_UNIT_DEFAULTS));
    end_form();
    newline_form();
    Dprintf("Writing %d units ...\n", numliveunits);
    for_all_sides(loopside) {
	for_all_side_units(loopside, unit) {
	    if (alive(unit)) {
		/* K_AT always written */
		/* K_S always written */
		/* If the unit will appear later, must write out that
		   later position, possibly mapped to a new place if
		   the map is being reshaped. */
		if (unit->cp < 0 && unit_appear_turn(unit) >= 0) {
		    x0 = (- unit->prevx);  y0 = (- unit->prevy);
		} else {
		    x0 = unit->x;  y0 = unit->y;
		}
		if (doreshape) {
		    reshaped_point(x0, y0, &x, &y);
		} else {
		    x = x0;  y = y0;
		}
		/* If these were negative values made positive for the
		   purposes of reshaping, make them negative again. */
		if (unit->cp < 0 && unit_appear_turn(unit) >= 0) {
		    x = (- x);  y = (- y);
		}
		start_form(shortest_escaped_name(unit->type));
		add_num_to_form(x);
		add_num_to_form(y);
		if (unit->side != NULL && unit->side->symbol != lispnil)
		  add_to_form(escaped_symbol(c_string(unit->side->symbol)));
		else
		  add_num_to_form(side_number(unit->side));
		write_num_prop(key(K_Z), unit->z, 0, FALSE, FALSE);
		write_str_prop(key(K_N), unit->name, NULL, FALSE, FALSE);
		write_num_prop(key(K_OS), side_number(unit->origside),
			       side_number(unit->side), FALSE, FALSE);
		/* Maybe write the unit's id. */
		if (module->def_all
		    || module->def_unit_ids
		    || (unit->occupant
			&& unit->name == NULL
			&& unit_symbol(unit) == lispnil))
		  write_num_prop(key(K_SHARP), unit->id, 0, FALSE, FALSE);
		/* Need this to get back into the right transport. */
		if (unit->transport) {
		    if (unit_symbol(unit->transport) != lispnil
		        	&& !module->def_all
			&& !module->def_unit_ids) {
			write_lisp_prop(key(K_IN),
					unit_symbol(unit->transport),
					lispnil, FALSE, TRUE, FALSE);
		    } else if (unit->transport->name
		        	       && !module->def_all
			       && !module->def_unit_ids) {
			write_str_prop(key(K_IN), unit->transport->name,
				       NULL, FALSE, FALSE);
		    } else {
			write_num_prop(key(K_IN), unit->transport->id,
				       0, FALSE, FALSE);
		    }
		}
		/* Always need this, because occupants may reference. */
		write_lisp_prop(key(K_SYM), unit_symbol(unit), lispnil,
				FALSE, TRUE, FALSE);
		/* Write optional info about the units. */
		if (module->def_all || module->def_unit_props)
		  write_unit_properties(unit);
		if (module->def_all || module->def_unit_acts)
		  write_unit_act(unit);
		if (module->def_all || module->def_unit_plans)
		  write_unit_plan(unit);
		/* close the unit out */
		end_form();
		newline_form();
		Dprintf("Wrote %s\n", unit_desig(unit));
	    }
	}
	newline_form();
    }
    Dprintf("... Done writing units.\n");
}

/* Write random properties, but only if they have non-default values. */

static void
write_unit_properties(Unit *unit)
{
    write_str_prop(key(K_IMAGE_NAME), unit->image_name, NULL, FALSE, FALSE);
    write_num_prop(key(K_NB), unit->number, 0, FALSE, FALSE);
    write_num_prop(key(K_HP), unit->hp, u_hp(unit->type), FALSE, FALSE);
    write_num_prop(key(K_CP), unit->cp, u_cp(unit->type), FALSE, FALSE);
    write_num_prop(key(K_CXP), unit->cxp, 0, FALSE, FALSE);
    write_num_prop(key(K_MO), unit->morale, 0, FALSE, FALSE);
    write_num_prop(key(K_TRK), unit->tracking, NOSIDES, FALSE, FALSE);
    write_utype_value_list(key(K_TP), unit->tooling, 0, FALSE);
    write_side_value_list(key(K_OPINIONS), unit->opinions, 0, FALSE);
    /* Getting the defaults right for the supply list requires more
       cleverness, alway write for now. */
    write_treasury_list(key(K_M), unit->supply, -1, FALSE);
    write_num_prop(key(K_POINT_VALUE), unit_point_value(unit), -1, FALSE, 
                   FALSE);
    write_num_prop(key(K_APPEAR), unit_appear_turn(unit), -1, FALSE, FALSE);
    /* (should do appear x,y also here?) */
    write_num_prop(key(K_DISAPPEAR), unit_disappear_turn(unit), -1, FALSE, 
                   FALSE);
    write_lisp_prop(key(K_SIDES), unit_sides(unit), lispnil, FALSE, TRUE, 
                    FALSE);
    /* Advanced unit support. */
    if (!u_advanced(unit->type))
      return;
    write_num_prop(key(K_SIZE), unit->size, 0, FALSE, FALSE);	
    write_num_prop(key(K_REACH), unit->reach, 0, FALSE, FALSE);		
    write_num_prop(key(K_USEDCELLS), unit->usedcells, 0, FALSE, FALSE);
    write_num_prop(key(K_MAXCELLS), unit->maxcells, 1, FALSE, FALSE);
    write_num_prop(key(K_CURADVANCE), unit->curadvance, -1, FALSE, FALSE);
    write_num_prop(key(K_POPULATION), unit->population, 1, FALSE, FALSE);
    write_mtype_value_list(key(K_PRODUCTION), unit->production, 0, FALSE);
    write_num_prop(key(K_CP_STASH), unit->cp_stash, FALSE, FALSE, FALSE);
    write_num_prop(key(K_CREATION_ID), unit->creation_id, 0, FALSE, FALSE);
}

/* Write out the unit's current actor state. */

static void
write_unit_act(Unit *unit)
{
    int acp = u_acp(unit->type);
    ActorState *act = unit->act;

    /* Actor state is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (act != NULL
	&& (act->acp != acp
	    || act->initacp != acp
	    || act->nextaction.type != ACTION_NONE
	    || act->actualmoves)) {
	if (1) {
	   newline_form();
	   space_form();
	}
	write_num_prop(key(K_ACP), act->acp, acp, FALSE, FALSE);
	write_num_prop(key(K_ACP0), act->initacp, acp, FALSE, FALSE);
	write_num_prop(key(K_AM), act->actualmoves, 0, FALSE, FALSE);
	if (act->nextaction.type != ACTION_NONE) {
	    space_form();
	    start_form(key(K_A));
	    space_form();
	    write_action(&(act->nextaction), unit->id);
	    end_form();
	}
    }
}

static void
write_action(Action *action, int id)
{
    int atype, i, slen;

    atype = action->type;
    start_form(actiondefns[atype].name);
    slen = strlen(actiondefns[atype].argtypes);
    for (i = 0; i < slen; ++i)
      add_num_to_form(action->args[i]);
    if (action->actee != 0 && action->actee != id)
      add_num_to_form(action->actee);
    end_form();
}

/* Write out the unit's current plan. */

static void
write_unit_plan(Unit *unit)
{
    Task *task;
    Plan *plan = unit->plan;

    /* The plan is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (plan) {
	if (1) {
	    newline_form();
	    space_form();
	}
    	space_form();
    	start_form(key(K_PLAN));
    	add_to_form(plantypenames[plan->type]);
	add_num_to_form(plan->creation_turn);
	write_num_prop(key(K_INITIAL_TURN), plan->initial_turn, 0, FALSE, FALSE);
	write_num_prop(key(K_FINAL_TURN), plan->final_turn, 0, FALSE, FALSE);
	write_bool_prop(key(K_ASLEEP), plan->asleep, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_RESERVE), plan->reserve, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_DELAYED), plan->delayed, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_WAIT), plan->waitingfortasks, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_AI_CONTROL), plan->aicontrol, TRUE, FALSE, FALSE);
	write_bool_prop(key(K_SUPPLY_ALARM), plan->supply_alarm, TRUE, FALSE, FALSE);
	write_bool_prop(key(K_SUPPLY_IS_LOW), plan->supply_is_low, FALSE, FALSE, FALSE);
	write_bool_prop(key(K_WAIT_TRANSPORT), plan->waitingfortransport, FALSE, FALSE, FALSE);
	if (plan->maingoal)
	  write_goal(plan->maingoal, K_GOAL);
	if (plan->formation)
	  write_goal(plan->formation, K_FORMATION);
	if (plan->tasks) {
    	    space_form();
    	    start_form(key(K_TASKS));
    	    for_all_tasks(plan, task) {
	    	space_form();
	    	write_task(task);
	    }
	    end_form();
	}
	end_form();
    }
}

static void
write_task(Task *task)
{
    int i, numargs;
    const char *argtypes;

    if (!is_task_type(task->type)) {
	run_warning("Bad task type %d while writing, skipping it", task->type);
	return;
    }
    start_form(taskdefns[task->type].name);
    add_num_to_form(task->execnum);
    add_num_to_form(task->retrynum);
    argtypes = taskdefns[task->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(task->args[i]);
    end_form();
}

static void
write_goal(Goal *goal, int keyword)
{
    int i, numargs;
    const char *argtypes;

    space_form();
    start_form(key((enum keywords)keyword));
    add_num_to_form(side_number(goal->side));
    add_num_to_form(goal->tf);
    add_to_form(goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(goal->args[i]);
    end_form();
}

/* Write all the historical events recorded so far. */

static void
write_history(void)
{
    PastUnit *pastunit;
    HistEvent *hevt;

    /* Write all the past units that might be mentioned in events.  These
       should already be sorted by id. */
    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next)
      write_past_unit(pastunit);
    newline_form();
    /* Now write all the events, doing the first one separately so as to
       simplify testing for the end of the history list (which is circular). */
    write_historical_event(history);
    for (hevt = history->next; hevt != history; hevt = hevt->next)
      write_historical_event(hevt);
    newline_form();
}

static void
write_past_unit(PastUnit *pastunit)
{
    start_form(key(K_EXU));
    add_num_to_form(pastunit->id);
    add_to_form(shortest_escaped_name(pastunit->type));
    add_num_to_form(pastunit->x);
    add_num_to_form(pastunit->y);
    add_num_to_form(side_number(pastunit->side));
    write_num_prop(key(K_Z), pastunit->z, 0, FALSE, FALSE);
    write_str_prop(key(K_N), pastunit->name, NULL, FALSE, FALSE);
    write_num_prop(key(K_NB), pastunit->number, 0, FALSE, FALSE);
    end_form();
    newline_form();
}

static void
write_historical_event(HistEvent *hevt)
{
    int i;
    const char *descs;

    /* Might be reasons not to write this event. */
    if (hevt->startdate < 0)
      return;
    start_form(key(K_EVT));
    add_num_to_form(hevt->startdate);
    add_to_form(hevtdefns[hevt->type].name);
    if (hevt->observers == ALLSIDES)
      add_to_form(key(K_ALL));
    else
      add_num_to_form(hevt->observers);
    descs = hevtdefns[hevt->type].datadescs;
    for (i = 0; descs[i] != '\0'; ++i) {
	switch (descs[i]) {
	  case 'm':
	  case 'n':
	  case 's':
	  case 'S':
	  case 'u':
	  case 'U':
	  case 'x':
	  case 'y':
	    add_num_to_form(hevt->data[i]);
	    break;
	  default:
	    run_warning("'%c' is not a recognized history data desc char",
			descs[i]);
	    break;
	}
    }
    end_form();
    newline_form();
}

static void
write_namers(void)
{
    extern Obj *namerlist;
    Obj *rest, *namer;

    for_all_list(namerlist, rest) {
	namer = car(rest);
	start_form(key(K_NAMER));
	space_form();
	add_form_to_form(namer->v.ptr.sym);
	newline_form();
	space_form();
	space_form();
	add_form_to_form((Obj *) namer->v.ptr.data);
	newline_form();
	space_form();
	space_form();
	end_form();
	newline_form();
	newline_form();
    }
}

/* The comparison function for the image list just does "strcmp" order
   and *requires* that all image families be named and named uniquely. */

static void sort_all_recorded_images(void);

static int image_name_compare(CONST void *imf1, CONST void *imf2);

static int
image_name_compare(CONST void *imf1, CONST void *imf2)
{
    return strcmp((*((ImageFamily **) imf1))->name,
		  (*((ImageFamily **) imf2))->name);
}

static void
sort_all_recorded_images(void)
{
    qsort(&(recorded_imfs[0]), num_recorded_imfs, sizeof(ImageFamily *),
	  image_name_compare);
}

static void
write_images(void)
{
    int i;

    if (recorded_imfs == NULL || num_recorded_imfs == 0)
      return;
    sort_all_recorded_images();
    for (i = 0; i < num_recorded_imfs; ++i) {
	write_imf(wfp, recorded_imfs[i]);
    }
}

/* This is a generalized routine to do run-length-encoding of area
   layers.  It uses hook fns to acquire data at a point and an
   optional translator to do any last-minute fixing.  It can use
   either a char or numeric encoding, depending on the expected range
   of values. */

static void
write_rle(int (*datafn)(int, int), int lo, int hi, int (*translator)(int),
	  int compress)
{
    int width, height, x, y, x0, y0, run, runval, val, trval;
    int numbad = 0;

    width = area.width;  height = area.height;
    if (doreshape) {
	width = reshaper->final_width;  height = reshaper->final_height;
    }
    /* Iterate through the rows from the top down. */
    for (y = height - 1; y >= 0; --y) {
	space_form();
	space_form();
	add_char_to_form('"');
	run = 0;
	x0 = 0;  y0 = y;
	if (doreshape)
	  original_point(0, y, &x0, &y0);
	val = (*datafn)(x0, y0);
	/* Zero out anything not in the world, unless reshaping. */
	if (!doreshape && !in_area(x0, y0))
	  val = 0;
	/* Check that the data falls within bounds, clip if not. */
	if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
	    ++numbad;
	    if (val < lo)
	      val = lo;
	    if (val > hi)
	      val = hi;
	}
	runval = val;
	for (x = 0; x < width; ++x) {
	    x0 = x;  y0 = y;
	    if (doreshape)
	      original_point(x, y, &x0, &y0);
	    val = (*datafn)(x0, y0);
	    /* Zero out anything not in the world, unless reshaping. */
	    if (!doreshape && !in_area(x0, y0))
	      val = 0;
	    /* Check that the data falls within bounds, clip if not. */
	    if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
		++numbad;
		if (val < lo)
		  val = lo;
		if (val > hi)
		  val = hi;
	    }
	    if (val == runval && compress) {
		run++;
	    } else {
		trval = (translator != NULL ? (*translator)(runval) : runval);
		write_run(run, trval);
		/* Start a new run. */
		runval = val;
		run = 1;
	    }
	}
	/* Finish off the row. */
	trval = (translator != NULL ? (*translator)(val) : val);
	write_run(run, trval);
	add_char_to_form('"');
	newline_form();
    }
    if (numbad > 0) {
	run_warning("%d values not between %d and %d", numbad, lo, hi);
    }
}

/* Write a single run, using the most compact encoding possible.
   0 - 29 is 'a' - '~', 30 - 63 is ':' - '[' */ 

static void
write_run(int run, int val)
{
    if (run > 1) {
	add_num_to_form_no_space(run);
	if (!between(0, val, 63))
	  add_char_to_form('*');
    }
    if (between(0, val, 29)) {
	add_char_to_form(val + 'a');
    } else if (between(30, val, 63)) {
	add_char_to_form(val - 30 + ':');
    } else {
	add_num_to_form_no_space(val);
	add_char_to_form(',');
    }
}

/* Compute and return the corresponding point in an area being reshaped. */

static int
reshaped_point(int x1, int y1, int *x2p, int *y2p)
{
    *x2p = (((x1 - reshaper->subarea_x) * reshaper->final_subarea_width )
	    / reshaper->subarea_width ) + reshaper->final_subarea_x;
    *y2p = (((y1 - reshaper->subarea_y) * reshaper->final_subarea_height)
	    / reshaper->subarea_height) + reshaper->final_subarea_y;
    return TRUE;
}

static int
original_point(int x1, int y1, int *x2p, int *y2p)
{
    *x2p = (((x1 - reshaper->final_subarea_x) * reshaper->subarea_width )
	    / reshaper->final_subarea_width ) + reshaper->subarea_x;
    *y2p = (((y1 - reshaper->final_subarea_y) * reshaper->subarea_height)
	    / reshaper->final_subarea_height) + reshaper->subarea_y;
    return inside_area(*x2p, *y2p);
}

/* Little routines to do low-level syntax to either file or remote machine. */

static void
start_form(const char *hd)
{
    if (wfp) {
	fprintf(wfp, "(%s", hd);
    } else {
	add_to_packet("(");
	add_to_packet(hd);
    }
}

static void
add_to_form(const char *x)
{
    if (wfp) {
	fprintf(wfp, " %s", x);
    } else {
	add_to_packet(" ");
	add_to_packet(x);
    }
}

static void
add_to_form_no_space(const char *x)
{
    if (wfp) {
	fputs(x, wfp);
    } else {
	add_to_packet(x);
    }
}

static void
add_char_to_form(int x)
{
    char buf[2];

    if (wfp) {
	fprintf(wfp, "%c", x);
    } else {
	buf[0] = x;
	buf[1] = '\0';
	add_to_packet(buf);
    }
}

static void
add_num_to_form(int x)
{
    char buf[30];

    if (wfp) {
	fprintf(wfp, " %d", x);
    } else {
	sprintf(buf, " %d", x);
	add_to_packet(buf);
    }
}

static void
add_num_to_form_no_space(int x)
{
    char buf[30];

    if (wfp) {
	fprintf(wfp, "%d", x);
    } else {
	sprintf(buf, "%d", x);
	add_to_packet(buf);
    }
}

/* Write either a normal value or a dice spec, as appropriate. */

static void
add_num_or_dice_to_form(int x, int valtype)
{
    char valbuf [BUFSIZE];

    if (TABDICE1 == valtype)
        dice1_desc(valbuf, (DiceRep)x);
    else if (TABDICE2 == valtype)
        dice2_desc(valbuf, (DiceRep)x);
    else
        snprintf(valbuf, BUFSIZE, "%d", x);
    if (wfp)
	fputs(valbuf, wfp);
    else
	add_to_packet(valbuf);
}

char *onemorebuf;

static void
add_form_to_form(Obj *x)
{
    if (wfp) {
	fprintlisp(wfp, x);
    } else {
	/* Even this might not be enough (should have a better strategy
	   for downloading large Lisp objects) */
	if (onemorebuf == NULL)
	  onemorebuf = (char *)xmalloc(50000);
	sprintlisp(onemorebuf, x, 50000);
	add_to_packet(onemorebuf);
    }
}

static void
end_form(void)
{
    if (wfp) {
	fputs(")", wfp);
    } else {
	add_to_packet(")");
    }
}

static void
newline_form(void)
{
    if (wfp) {
	fprintf(wfp, "\n");
    } else {
	add_to_packet("\n");
    }
}

static void
space_form(void)
{
    if (wfp) {
	fputs(" ", wfp);
    } else {
	add_to_packet(" ");
    }
}

static void
write_bool_prop(const char *name, int value, int dflt, int nodefaulting,
		int addnewline)
{
    if (nodefaulting || value != dflt) {
	space_form();
	start_form(name);
	add_to_form((char *)(value ? "true" : "false"));
	end_form();
	if (addnewline) {
	    newline_form();
	    space_form();
	}
    }
}

static void
write_num_prop(const char *name, int value, int dflt, int nodefaulting,
	       int addnewline)
{
    if (nodefaulting || value != dflt) {
	space_form();
	start_form(name);
	add_num_to_form(value);
	end_form();
	if (addnewline) {
	    newline_form();
	    space_form();
	}
    }
}

/* Handle the writing of a single string-valued property. */

static void
write_str_prop(const char *name, const char *value, const char *dflt, int nodefaulting,
	       int addnewline)
{
    char *tmp = NULL;
    int len = 0;
    if (nodefaulting || string_not_default(value, dflt)) {
	space_form();
	start_form(name);
        if (NULL != value){
            len = strlen(value);
            if (((BUFSIZE - 3) / 2) < len) {
                tmp = safe_escaped_string(value, len);
                add_to_form(tmp);
                free(tmp);
            }
            else {
                add_to_form(escaped_string(value));
            }
        }
        else {
            add_to_form(escaped_string(value));
        }
	end_form();
	if (addnewline) {
	    newline_form();
	    space_form();
	}
    }
}

static int
string_not_default(const char *str, const char *dflt)
{
    if (empty_string(dflt)) {
	if (empty_string(str)) {
	    return FALSE;
	} else {
	    return TRUE;
	}
    } else {
	if (empty_string(str)) {
	    return TRUE;
	} else {
	    return (strcmp(str, dflt) != 0);
	}
    }
}

static void
write_lisp_prop(const char *name, Obj *value, Obj *dflt, int nodefaulting,
		int as_cdr, int addnewline)
{
    Obj *rest;

    /* Sanity check. */
    if (value == NULL) {
	run_warning("Property \"%s\" has a bad value NULL, ignoring", name);
	return;
    }
    if (nodefaulting || !equal(value, dflt)) {
	space_form();
	start_form(name);
	if (as_cdr && consp(value)) {
	    for_all_list(value, rest) {
	    	space_form();
	    	add_form_to_form(car(rest));
	    }
	} else {
	    space_form();
	    add_form_to_form(value);
	}
	end_form();
	if (addnewline) {
	    newline_form();
	    space_form();
	}
    }
}

static void
write_utype_value_list(const char *name, short *arr, int dflt, int addnewline)
{
    int u, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for_all_unit_types(u) {
	if (arr[u] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	add_num_to_form(arr[u]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

static void
write_mtype_value_list(const char *name, short *arr, int dflt, int addnewline)
{
    int m, writeany;

    if (nummtypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_material_types(m) {
	if (arr[m] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_material_types(m) {
	add_num_to_form(arr[m]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

/* Same as above but with long arr, so it can handle treasuries. */

static void
write_treasury_list(const char *name, long *arr, int dflt, int addnewline)
{
    int m, writeany;

    if (nummtypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_material_types(m) {
	if (arr[m] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_material_types(m) {
	add_num_to_form(arr[m]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

static void
write_atype_value_list(const char *name, short *arr, int dflt, int addnewline)
{
    int a, writeany;

    if (numatypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_advance_types(a) {
	if (arr[a] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_advance_types(a) {
	add_num_to_form(arr[a]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

static void
write_side_value_list(const char *name, short *arr, int dflt, int addnewline)
{
    int s, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for (s = 0; s <= numsides; ++s) {
	if (arr[s] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for (s = 0; s <= numsides; ++s) {
	add_num_to_form(arr[s]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

static void
write_utype_string_list(const char *name, const char **arr, const char *dflt, int addnewline)
{
    int u, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for_all_unit_types(u) {
	if (arr[u] != dflt /* bogus, should use strcmp */) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	add_to_form(escaped_string(arr[u]));
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

/* Return the shortest properly escaped name that can be used to identify
   unit type. */

char *
shortest_escaped_name(int u)
{
    const char *internalname = u_internal_name(u);

    sprintf(shortestbuf, "%s", escaped_symbol(internalname));
    return shortestbuf;
}


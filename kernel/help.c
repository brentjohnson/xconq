/* Online help support for Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is basically support code for interfaces, which handle the
   actual help interaction themselves. */

/* This file must also be translated (mostly) for non-English Xconq. */

#include "conq.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <fcntl.h>
#ifdef __cplusplus
}
#endif

using namespace Xconq;

/* Extra functions. */

int may_detonate(int u);
int any_ut_capacity_x(int u);
int any_mp_to_enter_unit(int u);
int any_mp_to_leave_unit(int u);
int any_enter_indep(int u);

/* Help system internal variables. */

static int help_output_cc = HELP_OUTPUT_CC_NONE;
static HelpOutputMode help_output_mode = HELP_OUTPUT_PLAIN_TEXT;
static char *help_output_dir = NULL;
static FILE *help_output_tmp_filep = NULL;
static FILE *help_output_toc_filep = NULL;

static HelpPageDefn hpagedefns [] = {
    /* HELP_PAGE_NONE */
    { NULL, NULL, HELP_PAGE_NONE, HELP_PAGE_NONE, HELP_PAGE_NONE },
    /* HELP_PAGE_MASTER_INDEX */
    { NULL, "Master Game Index", HELP_PAGE_NONE, HELP_PAGE_NONE, 
      HELP_PAGE_NONE },
    /* HELP_PAGE_TOC */
    { "index", "Table of Contents", HELP_PAGE_MASTER_INDEX, HELP_PAGE_NONE, 
      HELP_PAGE_NONE },
    /* HELP_PAGE_COPYRIGHT */
    { "copyleft", "Copyright", HELP_PAGE_TOC, HELP_PAGE_CONCEPTS, 
      HELP_PAGE_WARRANTY },
    /* HELP_PAGE_WARRANTY */
    { "warranty", "Warranty (or lack thereof)", HELP_PAGE_TOC, 
      HELP_PAGE_COPYRIGHT, HELP_PAGE_NEWS },
    /* HELP_PAGE_NEWS */
    { "news", "News (or lack thereof)", HELP_PAGE_TOC, HELP_PAGE_WARRANTY, 
      HELP_PAGE_INSTRUCTIONS },
    /* HELP_PAGE_INSTRUCTIONS */
    { "isns", "Instructions", HELP_PAGE_TOC, HELP_PAGE_NEWS, 
      HELP_PAGE_GAME_OVERVIEW },
    /* HELP_PAGE_GAME_OVERVIEW */
    { "overview", "Game Overview", HELP_PAGE_TOC, HELP_PAGE_INSTRUCTIONS, 
      HELP_PAGE_SCORING },
    /* HELP_PAGE_SCORING */
    { "scoring", "Scoring", HELP_PAGE_TOC, HELP_PAGE_GAME_OVERVIEW, 
      HELP_PAGE_MODULES },
    /* HELP_PAGE_MODULES */
    { "modules", "Modules", HELP_PAGE_TOC, HELP_PAGE_SCORING, 
      HELP_PAGE_GAME_SETUP },
    /* HELP_PAGE_GAME_SETUP */
    { "config", "Game Setup", HELP_PAGE_TOC, HELP_PAGE_MODULES, 
      HELP_PAGE_WORLD },
    /* HELP_PAGE_WORLD */
    { "world", "World", HELP_PAGE_TOC, HELP_PAGE_GAME_SETUP, HELP_PAGE_UTYPE },
    /* HELP_PAGE_UTYPE */
    { "utype", "Unit", HELP_PAGE_TOC, HELP_PAGE_WORLD, HELP_PAGE_TTYPE },
    /* HELP_PAGE_TTYPE */
    { "ttype", "Terrain", HELP_PAGE_TOC, HELP_PAGE_UTYPE, HELP_PAGE_MTYPE }, 
    /* HELP_PAGE_MTYPE */
    { "mtype", "Material", HELP_PAGE_TOC, HELP_PAGE_TTYPE, HELP_PAGE_ATYPE },
    /* HELP_PAGE_ATYPE */
    { "atype", "Advance", HELP_PAGE_TOC, HELP_PAGE_MTYPE, HELP_PAGE_NONE },
    /* HELP_PAGE_CONCEPTS */
    { "concepts", "Game Concepts", HELP_PAGE_TOC, HELP_PAGE_ATYPE, 
      HELP_PAGE_COPYRIGHT }
};
#define HELP_PAGE_LAST	HELP_PAGE_CONCEPTS

/* Obstack allocation and deallocation routines.  */

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

/* Help system internal functions. */

static void describe_help_system(int arg, const char *key, TextBuffer *buf);
static void describe_instructions(int arg, const char *key, TextBuffer *buf);
static void describe_synth_run(TextBuffer *buf, int methkey);
static void describe_world(int arg, const char *key, TextBuffer *buf);
static int histogram_compare(const void *h1, const void *h2);
static void describe_news(int arg, const char *key, TextBuffer *buf);
static void describe_concepts(int arg, const char *key, TextBuffer *buf);
static void describe_game_design(int arg, const char *key, TextBuffer *buf);
static void describe_utype(int u, const char *key, TextBuffer *buf);
static void describe_utype_movement(int u, const char *key, TextBuffer *buf);
static void describe_utype_actions(int u, const char *key, TextBuffer *buf);
static void describe_utype_side_attributes(int u, const char *key, TextBuffer *buf);
static void describe_utype_ai_attributes(int u, const char *key, TextBuffer *buf);
static void describe_mtype(int m, const char *key, TextBuffer *buf);
static void describe_ttype(int t, const char *key, TextBuffer *buf);
static void describe_atype(int t, const char *key, TextBuffer *buf);

static void describe_scorekeepers(int arg, const char *key, TextBuffer *buf);
static void describe_setup(int arg, const char *key, TextBuffer *buf);
static void describe_game_modules(int arg, const char *key, TextBuffer *buf);
static void describe_game_module_aux(TextBuffer *buf, Module *module,
				     int level);
static void describe_module_notes(TextBuffer *buf, Module *module);

static int u_property_not_default(int (*fn)(int i), int dflt);
static int t_property_not_default(int (*fn)(int i), int dflt);
static int uu_table_row_not_default(int u, int (*fn)(int i, int j), int dflt);
static int ut_table_row_not_default(int u, int (*fn)(int i, int j), int dflt);
static int um_table_row_not_default(int u, int (*fn)(int i, int j), int dflt);
static int ua_table_row_not_default(int u, int (*fn)(int i, int j), int dflt);
static int tt_table_row_not_default(int t, int (*fn)(int i, int j), int dflt);
#if 0
static int tm_table_row_not_default(int t, int (*fn)(int i, int j), int dflt);
#endif
static int aa_table_row_not_default(int a1, int (*fn)(int i, int j), int dflt);
static int uu_table_column_not_default(int u, int (*fn)(int i, int j),
				       int dflt);
static int aa_table_column_not_default(int a1, int (*fn)(int i, int j),
				       int dflt);
static void u_property_desc(TextBuffer *buf, int (*fn)(int),
			    void (*formatter)(TextBuffer *, int));
static void t_property_desc(TextBuffer *buf, int (*fn)(int),
			    void (*formatter)(TextBuffer *, int));
static void uu_table_row_desc(TextBuffer *buf, int u, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int),
			      const char *connect);
static void uu_table_column_desc(TextBuffer *buf, int u, int (*fn)(int, int),
				 void (*formatter)(TextBuffer *, int),
				 char *connect);
static void uu_table_rowcol_desc(TextBuffer *buf, int u, int (*fn)(int, int),
				 void (*formatter)(TextBuffer *, int),
				 const char *connect, int rowcol);
static void ut_table_row_desc(TextBuffer *buf, int u, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int),
			      const char *connect);
static void um_table_row_desc(TextBuffer *buf, int u, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int));
static void ua_table_row_desc(TextBuffer *buf, int u, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int));
static void tt_table_row_desc(TextBuffer *buf, int t1, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int));
#if 0
static void tm_table_row_desc(TextBuffer *buf, int t, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int));
#endif
static void aa_table_row_desc(TextBuffer *buf, int a1, int (*fn)(int, int),
			      void (*formatter)(TextBuffer *, int));
static void aa_table_column_desc(TextBuffer *buf, int a1, int (*fn)(int, int),
				 void (*formatter)(TextBuffer *, int));
static void aa_table_rowcol_desc(TextBuffer *buf, int a1, int (*fn)(int, int),
				 void (*formatter)(TextBuffer *, int),
				 int rowcol);
static void tb_value_desc(TextBuffer *buf, int val);
static void tb_fraction_desc(TextBuffer *buf, int val);
static void tb_percent_desc(TextBuffer *buf, int val);
static void tb_percent_100th_desc(TextBuffer *buf, int val);
static void tb_probfraction_desc(TextBuffer *buf, int val);
static void tb_dice_desc(TextBuffer *buf, int val);
static void tb_mult_desc(TextBuffer *buf, int val);
static void tb_bool_desc(TextBuffer *buf, int val);
#if 0
static void append_number(TextBuffer *buf, int value, int dflt);
#endif
static void append_help_phrase(TextBuffer *buf, const char *phrase);
static void append_notes(TextBuffer *buf, Obj *notes);

static void init_help_pages(void);
static void meta_prep_help_cc(HelpPage hpage, int hpageidx);
static void prep_help_cc_toc(void);
static void prep_help_cc_any(char *ccfilename, char *sectionname);
static void flush_help_cc_tmp(HelpNode *helpnode);
static void meta_finish_help_cc(HelpPage hpage, int hpageidx, 
				HelpNode *helpnode);
static void finish_help_cc_toc(void);
static void flush_help_cc(FILE *ccfile, HelpNode *helpnode);
static void write_help_file_navbar(FILE *ccfilep, HelpPage hpage, int hpageidx);
static void write_help_section_header(FILE *ccfile, const char *headerdata);
static void write_help_section_footer(FILE *ccfilep, const char *footerdata);
static void write_help_subsection_header(FILE *ccfile, const char *headerdata);
static void write_help_subsection_footer(FILE *ccfile, const char *footerdata);
static void write_help_subsection_break(FILE *ccfile);

static const char *help_newline(void);
static const char *help_indent(unsigned int spaces);

/* The first help node in the chain. */

HelpNode *first_help_node;

/* The last help node. */

HelpNode *last_help_node;

/* The help node with help system info. */

HelpNode *help_system_node;

/* The help node with copying and copyright info. */

HelpNode *copying_help_node;

/* The help node with (non-)warranty info. */

HelpNode *warranty_help_node;

HelpNode *default_prev_help_node;

/* Set the help output carbon copy. */
void
set_help_output_cc(int cctarget)
{
    switch(cctarget) {
      case HELP_OUTPUT_CC_NONE: case HELP_OUTPUT_CC_FILES:
	help_output_cc = cctarget;
	break;
      default:
	run_error("Attempted to set an invalid help carbon copy target.");
    }
}

/* Set the help output mode. */
void
set_help_output_mode(HelpOutputMode houtmode)
{
    switch(houtmode) {
      case HELP_OUTPUT_PLAIN_TEXT: case HELP_OUTPUT_HTML:
	help_output_mode = houtmode;
	break;
      default:
	run_error("Attempted to set an unsupported help output mode.");
    }
}

/* Set the help output directory. */
void
set_help_output_dir(char *dir)
{
#if (!defined MAC)
    if (access(dir, F_OK)) 
      run_error("Attempted to access a missing directory.");
#endif
    help_output_dir = dir;
}

/* Set the help TOC file stream pointer. */
void
set_help_toc_filep(FILE *htocfilep)
{
    help_output_toc_filep = htocfilep;
}

/* Create the initial help node and link it to itself.  Subsequent
   nodes will be inserted later, after a game has been loaded. */

void
init_help(void)
{
    HelpNode *node = NULL;

    init_help_pages();
    prep_help_cc_toc();
    /* Note that we can't use add_help_node to set up the first help
       node. */
    first_help_node = create_help_node();
    first_help_node->key = "--- BASIC HELP ---";
    first_help_node->prev = first_help_node->next = first_help_node;
    last_help_node = first_help_node;
    help_system_node =
      add_help_node("help system", describe_help_system, 0, first_help_node);
    meta_prep_help_cc(HELP_PAGE_COPYRIGHT, -1);
    copying_help_node =
      add_help_node("copyright", describe_copyright, 0, help_system_node);
    meta_finish_help_cc(HELP_PAGE_COPYRIGHT, -1, copying_help_node);
    meta_prep_help_cc(HELP_PAGE_WARRANTY, -1);
    warranty_help_node =
      add_help_node("warranty", describe_warranty, 0, copying_help_node);
    meta_finish_help_cc(HELP_PAGE_WARRANTY, -1, warranty_help_node);
    /* Set the place for new nodes to appear normally. */
    default_prev_help_node = copying_help_node;
    meta_prep_help_cc(HELP_PAGE_NEWS, -1);
    node = add_help_node("news", describe_news, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_NEWS, -1, node);
}

/* This function creates the actual set of help nodes for the kernel. */

void
create_game_help_nodes(void)
{
    int u, m, t, a;
    const char *name, *longname;
    HelpNode *node;

    add_help_node("--- MODULE INFO ---", NULL, 0, NULL);
    meta_prep_help_cc(HELP_PAGE_INSTRUCTIONS, -1);
    node = add_help_node("instructions", describe_instructions, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_INSTRUCTIONS, -1, node);
    meta_prep_help_cc(HELP_PAGE_GAME_OVERVIEW, -1);
    node = add_help_node("game overview", describe_game_design, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_GAME_OVERVIEW, -1, node);
    meta_prep_help_cc(HELP_PAGE_SCORING, -1);
    node = add_help_node("scoring", describe_scorekeepers, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_SCORING, -1, node);
    meta_prep_help_cc(HELP_PAGE_MODULES, -1);
    node = add_help_node("modules", describe_game_modules, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_MODULES, -1, node);
    meta_prep_help_cc(HELP_PAGE_GAME_SETUP, -1);
    node = add_help_node("game setup", describe_setup, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_GAME_SETUP, -1, node);
    meta_prep_help_cc(HELP_PAGE_WORLD, -1);
    node = add_help_node("world", describe_world, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_WORLD, -1, node);
    if (numutypes) {
        add_help_node("--- UNIT TYPES ---", NULL, 0, NULL);
	write_help_toc_entry(NULL, "Unit Types", 0);
    }
    for_all_unit_types(u) {
	longname = u_long_name(u);
	if (!empty_string(longname)) {
	    sprintf(spbuf, "%s (%s)", longname, u_type_name(u));
	    name = copy_string(spbuf);
	} else {
	    name = u_type_name(u);
	}
	meta_prep_help_cc(HELP_PAGE_UTYPE, u);
	node = add_help_node(name, describe_utype, u, NULL);
	node->nclass = utypenode;
	meta_finish_help_cc(HELP_PAGE_UTYPE, u, node);
    }
    if (nummtypes) {
        add_help_node("--- MATERIALS ---", NULL, 0, NULL);
	write_help_toc_entry(NULL, "Material Types", 0);
    }
    for_all_material_types(m) {
	meta_prep_help_cc(HELP_PAGE_MTYPE, m);
	node = add_help_node(m_type_name(m), describe_mtype, m, NULL);
	node->nclass = mtypenode;
	meta_finish_help_cc(HELP_PAGE_MTYPE, m, node);
    }
    if (numttypes) {
        add_help_node("--- TERRAIN TYPES ---", NULL, 0, NULL);
	write_help_toc_entry(NULL, "Terrain Types", 0);
    }
    for_all_terrain_types(t) {
	meta_prep_help_cc(HELP_PAGE_TTYPE, t);
	node = add_help_node(t_type_name(t), describe_ttype, t, NULL);
	node->nclass = ttypenode;
	meta_finish_help_cc(HELP_PAGE_TTYPE, t, node);
    }
    if (numatypes) {
        add_help_node("--- ADVANCES ---", NULL, 0, NULL);
	write_help_toc_entry(NULL, "Advances", 0);
    }
    for_all_advance_types(a) {
	meta_prep_help_cc(HELP_PAGE_ATYPE, a);
	node = add_help_node(a_type_name(a), describe_atype, a, NULL);
	node->nclass = atypenode;
	meta_finish_help_cc(HELP_PAGE_ATYPE, a, node);
    }
    add_help_node("--- GENERAL INFO ---", NULL, 0, NULL);
    meta_prep_help_cc(HELP_PAGE_CONCEPTS, -1);
    node = add_help_node("general concepts", describe_concepts, 0, NULL);
    meta_finish_help_cc(HELP_PAGE_CONCEPTS, -1, node);
    /* Invalidate any existing topics node. */
    first_help_node->text = NULL;
    finish_help_cc_toc();
}

/* Create an empty help node. */

HelpNode *
create_help_node(void)
{
    HelpNode *node = (HelpNode *) xmalloc(sizeof(HelpNode));

    node->key = NULL;
    node->fn = NULL;
    node->nclass = miscnode;
    node->arg = 0;
    node->text = NULL;
    node->prev = node->next = NULL;
    return node;
}

/* Add a help node after the given node. */

HelpNode *
add_help_node(const char *key, void (*fn)(int t, const char *key, TextBuffer *buf),
	      int arg, HelpNode *prevnode)
{
    HelpNode *node, *nextnode;

    if (empty_string(key)) {
	run_error("empty help key");
    }
    node = create_help_node();
    node->key = key;
    node->fn = fn;
    node->arg = arg;
    if (prevnode == NULL)
      prevnode = default_prev_help_node->prev;
    nextnode = prevnode->next;
    node->prev = prevnode;
    node->next = nextnode;
    prevnode->next = node;
    nextnode->prev = node;
    /* Might need to fix last help node. */
    last_help_node = first_help_node->prev;
    return node;
}

/* Given a string and node, find the next node whose key matches. */

HelpNode *
find_help_node(HelpNode *node, const char *str)
{
    HelpNode *tmp;

    /* Note that the search wraps around. */
    for (tmp = node->next; tmp != node; tmp = tmp->next) {
    	if (strcmp(tmp->key, str) == 0)
    	  return tmp;
    }
    return NULL;
}

/* Return the string containing the text of the help node, possibly
   computing it first. */

char *
get_help_text(HelpNode *node)
{
    TextBuffer tbuf;

    if (node != NULL) {
	/* Maybe calculate the text to display. */
	if (node->text == NULL) {
	    if (node->fn != NULL) {
		/* (should allow for variable-size allocation) */
	    	obstack_begin(&(tbuf.ostack), 200);
		if (1) {
		    node->textend = 0;
		    (*(node->fn))(node->arg, node->key, &tbuf);
		    obstack_1grow(&(tbuf.ostack), '\0');
		    node->text = copy_string((char *)obstack_finish(&(tbuf.ostack)));
		    obstack_free(&(tbuf.ostack), 0);
		    node->textend = strlen(node->text);
		} else {
		    /* Ran out of memory... (would never get here though!) */
		}
	    } else if (strspn(node->key, "-") > 2) {
		/* Generate an empty separator. */
		sprintf(spbuf, " ");
		node->text = copy_string(spbuf);
		node->textend = strlen(node->text);
	    } else {
		/* Generate a default message if nothing to compute help. */
		sprintf(spbuf, "%s: No info available.", node->key);
		node->text = copy_string(spbuf);
		node->textend = strlen(node->text);
	    }
	    Dprintf("Size of help node \"%s\" text is %d\n", node->key, node->textend);
	}
	return node->text;
    } else {
	return NULL;
    }
}

static void
describe_help_system(int arg, const char *key, TextBuffer *buf)
{
    tbcatline(buf, "This is the header node of the Xconq help system.");
    tbcatline(buf, "Go forward or backward from here to see the online help.");
}

/* Create a raw list of help topics by just iterating through all the nodes,
   except for the topics node itself. */

void
describe_topics(int arg, const char *key, TextBuffer *buf)
{
    HelpNode *topics, *tmp;

    topics = find_help_node(first_help_node, "topics");
    /* Unlikely that we'll call this without the topics node existing
       already, but just in case... */
    if (topics == NULL)
      return;
    for (tmp = topics->next; tmp != topics; tmp = tmp->next) {
	tbprintf(buf, "%s", tmp->key);
	tbcat(buf, "\n");
    }
}

/* Get the news file and put it into text buffer. */

static void
describe_news(int arg, const char *key, TextBuffer *buf)
{
    FILE *fp;

    fp = open_file(news_filename(), "r");
    if (fp != NULL) {
	tbcat(buf, "XCONQ NEWS\n\n");
	while (fgets(spbuf, BUFSIZE-1, fp) != NULL) {
	    tbcat(buf, spbuf);
	}
	fclose(fp);
    } else {
	tbcat(buf, "(no news)");
    }
}

/* Describe general game concepts in a general way.  If a concept does
   not apply to the game in effect, then just say it's not part of
   this game (would be confusing if the online doc described things
   irrelevant to the specific game). */

static void
describe_concepts(int arg, const char *key, TextBuffer *buf)
{
    tbcat(buf, "Hit points (HP) represent the overall condition of ");
    tbcatline(buf, "the unit.");
    tbcat(buf, "Action points (ACP) are what a unit needs to be able ");
    tbcat(buf, "to do anything at all.  Typically a unit will use 1 ACP ");
    tbcatline(buf, "to move 1 cell.");
    tbcat(buf, "Movement points (MP) represent varying costs of movement ");
    tbcat(buf, "actions, such as a difficult-to-cross border.  The number ");
    tbcat(buf, "of movement points is added up then divided by unit's speed ");
    tbcatline(buf, "to get the total number of acp used up by a move.");
#if (0)
    if (0) {
    } else {
	tbcatline(buf, "No combat experience (CXP) in this game.");
    }
    if (0) {
    } else {
	tbcatline(buf, "No morale (MO) in this game.");
    }
#endif
    tbcat(buf, "Each unit that can do anything has a plan, and a list of ");
    tbcatline(buf, "tasks to perform.");
    /* (should describe more general concepts) */
}

static void
describe_instructions(int arg, const char *key, TextBuffer *buf)
{
    Obj *instructions = mainmodule->instructions;

    if (instructions != lispnil) {
	append_notes(buf, instructions);
    } else {
	tbcat(buf, "(no instructions supplied)");
    }
}

/* Spit out all the general game_design parameters in a readable fashion. */

static void
describe_game_design(int arg, const char *key, TextBuffer *buf)
{
    int u, m, t, a;
    
    /* Replicate title and blurb? (should put title at head of
       windows, and pages if printed) */
    tbprintf(buf, "*** %s ***%s",
	     (mainmodule->title ? mainmodule->title : mainmodule->name), 
	     help_newline());
    tbcatline(buf, "");
    tbprintf(buf, "This game includes %d unit types and %d terrain types",
	     numutypes, numttypes);
    if (nummtypes > 0) {
	tbprintf(buf, ", along with %d material types", nummtypes);
    }
    if (numatypes > 0) {
	tbprintf(buf, ", and it has %d types of advances", numatypes);
    }
    tbcatline(buf, ".");
    if (g_sides_min() == g_sides_max()) {
    	tbprintf(buf, "Exactly %d sides may play.%s", g_sides_min(), 
		 help_newline());
    } else {
	tbprintf(buf, 
"Number of sides to play may range from %d to %d, defaulting to %d.%s",
		 g_sides_min(), g_sides_max(), g_sides_wanted(), 
		 help_newline());
    }
    tbcatline(buf, "");
    if (g_advantage_min() == g_advantage_max())
      tbprintf(buf, "Side advantages are fixed.%s", help_newline());
    else
      tbprintf(buf, 
"Side advantages may range from %d to %d, defaulting to %d.%s",
	       g_advantage_min(), g_advantage_max(), g_advantage_default(), 
	       help_newline());
    tbcatline(buf, "");
    if (g_see_all()) {
	tbcatline(buf, "Everything is always seen by all sides.");
    } else {
    	if (g_see_terrain_always()) {
	    tbcatline(buf, "Terrain view is always accurate once seen.");
    	}
    	/* (should only have if any weather to be seen) */
    	if ((any_temp_variation || any_wind_variation || any_clouds)
	    && g_see_weather_always()) {
	    tbcatline(buf, 
		      "Weather view is always accurate once terrain seen.");
    	}
    	if (g_terrain_seen()) {
	    tbcatline(buf, "World terrain is already seen by all sides.");
    	}
    }
    tbcatline(buf, "");
    if (g_last_turn() != gvar_i_default(g_last_turn)) {
	tbprintf(buf, "Game can go for up to %d turns", g_last_turn());
	if (g_extra_turn() > 0) {
	    tbprintf(buf, ", with %d%% chance of additional turn thereafter.",
		     g_extra_turn());
	}
	tbcatline(buf, ".");
    }
    if (g_rt_for_game() > 0) {
	tbprintf(buf, "Entire game can last up to %d minutes.%s",
		 g_rt_for_game() / 60, help_newline());
    }
    if (g_rt_per_turn() > 0) {
	tbprintf(buf, "Each turn can last up to %d minutes.%s",
		g_rt_per_turn() / 60, help_newline());
    }
    if (g_rt_per_side() > 0) {
	tbprintf(buf, "Each side gets a total %d minutes to act.%s",
		g_rt_per_side() / 60, help_newline());
    }
    if (g_units_in_game_max() >= 0) {
	tbprintf(buf, "Limited to no more than %d units in all.%s",
		 g_units_in_game_max(), help_newline());
    }
    if (g_units_per_side_max() >= 0) {
	tbprintf(buf, "Limited to no more than %d units per side.%s",
		 g_units_per_side_max(), help_newline());
    }
    if (g_use_side_priority()) {
	tbcatline(buf, "Sides move sequentially, in priority order.");
    } else {
	tbcatline(buf, "Sides move simultaneously.");
    }
    if (any_temp_variation) {
	tbprintf(buf, 
		 "Lowest possible temperature is %d, at an elevation of %d.%s",
		 g_temp_floor(), g_temp_floor_elev(), help_newline());
	tbprintf(buf, "Temperatures averaged to range %d.%s",
		 g_temp_mod_range(), help_newline());
    }
    tbprintf(buf, "%sUnit Types:%s", help_newline(), help_newline());
    for_all_unit_types(u) {
	tbprintf(buf, "%s%s", help_indent(2), u_type_name(u));
	if (!empty_string(u_help(u)))
	  tbprintf(buf, " (%s)", u_help(u));
	tbcatline(buf, "");
#ifdef DESIGNERS
	/* Show designers a bit more. */
	if (numdesigners > 0) {
	    tbcat(buf, "    [");
	    if (!empty_string(u_uchar(u)))
	      tbprintf(buf, "char '%s'", u_uchar(u));
	    else
	      tbcat(buf, "no char");
	    if (!empty_string(u_gchar(u)))
	      tbprintf(buf, "generic char '%s'", u_gchar(u));
	    else
	      tbcat(buf, "no generic char");
	    if (!empty_string(get_string(u_image_name(u))))
	      tbprintf(buf, ", image \"%s\"", get_string(u_image_name(u)));
	    if (!empty_string(u_generic_name(u)))
	      tbprintf(buf, ", generic name \"%s\"", u_generic_name(u));
	    if (u_desc_format(u) != lispnil) {
	        tbcat(buf, ", special format");
	    }
	    tbcatline(buf, "]");
	}
#endif /* DESIGNERS */
    }
    tbprintf(buf, "%sTerrain Types:%s", help_newline(), help_newline());
    for_all_terrain_types(t) {
	tbprintf(buf, "%s%s", help_indent(2), t_type_name(t));
	if (!empty_string(t_help(t)))
	  tbprintf(buf, " (%s)", t_help(t));
	tbcatline(buf, "");
#ifdef DESIGNERS
	/* Show designers a bit more. */
	if (numdesigners > 0) {
	    tbcat(buf, "    [");
	    if (!empty_string(t_char(t)))
	      tbprintf(buf, "char '%s'", t_char(t));
	    else
	      tbcat(buf, "no char");
	    if (!empty_string(t_image_name(t)))
	      tbprintf(buf, ", image \"%s\"", t_image_name(t));
	    tbcatline(buf, "]");
	}
#endif /* DESIGNERS */
    }
    if (nummtypes > 0) {
	tbprintf(buf, "%sMaterial Types:%s", help_newline(), help_newline());
	for_all_material_types(m) {
	    tbprintf(buf, "%s%s", help_indent(2), m_type_name(m));
	    if (!empty_string(m_help(m)))
	      tbprintf(buf, " (%s)", m_help(m));
	    tbcatline(buf, "");
#ifdef DESIGNERS
	    /* Show designers a bit more. */
	    if (numdesigners > 0) {
		tbcat(buf, "    [");
		if (!empty_string(m_char(m)))
		  tbprintf(buf, "char '%s'", m_char(m));
		else
		  tbcat(buf, "no char");
		if (!empty_string(m_image_name(m)))
		  tbprintf(buf, ", image \"%s\"", m_image_name(m));
		tbcatline(buf, "]");
	    }
#endif /* DESIGNERS */
	}
    }
    if (numatypes > 0) {
	tbprintf(buf, "%sAdvances:%s", help_newline(), help_newline());
	for_all_advance_types(a) {
	    tbprintf(buf, "%s%s", help_indent(2), a_type_name(a));
	    if (!empty_string(a_help(a)))
	      tbprintf(buf, " (%s)", a_help(a));
	    tbcatline(buf, "");
#ifdef DESIGNERS
	    /* Show designers a bit more. */
	    if (numdesigners > 0) {
		tbcat(buf, "    [");
		if (!empty_string(a_image_name(a)))
		  tbprintf(buf, ", image \"%s\"", a_image_name(a));
		tbcatline(buf, "]");
	    }
#endif /* DESIGNERS */
	}
    }
#ifdef DESIGNERS
    /* Show designers a bit more. */
    if (numdesigners > 0) {
	tbcat(buf, "FOR DESIGNERS:\n");
	tbprintf(buf, "Unseen terrain char is \"%s\".%s", g_unseen_char(), 
		 help_newline());
	tbprintf(buf, "Scorefile name is \"%s\".%s", g_scorefile_name(), 
		 help_newline());
    }
#endif /* DESIGNERS */
}

/* Display game module info to a side. */

static void
describe_game_modules(int arg, const char *key, TextBuffer *buf)
{
    if (mainmodule != NULL) {
	/* First put out basic module info. */
	describe_game_module_aux(buf, mainmodule, 0);
	/* Now do the lengthy module notes (with no indentation). */
	describe_module_notes(buf, mainmodule);
    } else {
	tbcat(buf, "(No game module information is available.)");
    }
}

/* Recurse down through included modules to display docs on each.
   Indents each file by inclusion level.  Note that modules cannot
   be loaded more than once, so each will be described only once here. */

static void   
describe_game_module_aux(TextBuffer *buf, Module *module, int level)
{
    int i;
    char indentbuf[BUFSIZE];
    char dashbuf[BUFSIZE];
    int indentsz = 0;
    Module *submodule;

    dashbuf[0] = '\0';
    indentbuf[0] = '\0';
    indentsz = strlen(help_indent(3));
    for (i = 0; i < level; ++i) {
	strcat(dashbuf, "-- ");
	strncat(indentbuf, help_indent(3), BUFSIZE - (indentsz * i));
    }
    tbprintf(buf, "%s\"%s\"", dashbuf,
	    (module->title ? module->title : module->name));
    /* Display the true name of the module if not the same as the title. */
    if (module->title != NULL && strcmp(module->title, module->name) != 0) {
	tbprintf(buf, " (\"%s\")", module->name);
    }
    if (module->version != NULL) {
	tbprintf(buf, " (version \"%s\")", module->version);
    }
    tbcatline(buf, help_indent(10));
    if (module->blurb != lispnil) {
	tbcat(buf, indentbuf);
	append_notes(buf, module->blurb);
	tbcatline(buf, "");
    } else {
	tbprintf(buf, "%s(no description)%s", indentbuf, help_newline());
    }
    if (module->notes != lispnil) {
	tbprintf(buf, "%sNotes to \"%s\":%s", indentbuf, module->name, 
		 help_newline());
	append_notes(buf, module->notes);
	tbcatline(buf, "");
    }
    tbcatline(buf, "");
    /* Now describe any included modules. */
    for_all_includes(module, submodule) {
	describe_game_module_aux(buf, submodule, level + 1);
    }
}

/* Dump the module designer's notes into the given buffer.  When doing
   submodules, don't indent. */

static void
describe_module_notes(TextBuffer *buf, Module *module)
{
    Module *submodule;

#ifdef DESIGNERS
    /* Only show design notes if any designers around. */
    if (numdesigners > 0 && module->designnotes != lispnil) {
	tbprintf(buf, "Design Notes to \"%s\":%s", module->name, 
		 help_newline());
	append_notes(buf, module->designnotes);
	tbcatline(buf, "");
    }
#endif /* DESIGNERS */
    for_all_includes(module, submodule) {
	describe_module_notes(buf, submodule);
    }
}

int
any_ut_capacity_x(int u)
{
    int t;
	
    for_all_terrain_types(t) {
	if (ut_capacity_x(u, t) != 0)
	  return TRUE;
    }
    return FALSE;
}

int
any_mp_to_enter_unit(int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (uu_mp_to_enter(u, u2) != 0)
	  return TRUE;
    }
    return FALSE;
}

int
any_mp_to_leave_unit(int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (uu_mp_to_leave(u, u2) != 0)
	  return TRUE;
    }
    return FALSE;
}

int
any_enter_indep(int u)
{
    int u2;
	
    for_all_unit_types(u2) {
	if (uu_can_enter_indep(u, u2))
	  return TRUE;
    }
    return FALSE;
}

/* Full details on the given type of unit. */

static void
describe_utype(int u, const char *key, TextBuffer *buf)
{
    int m, usesm, a;

    append_help_phrase(buf, u_help(u));
    /* Display the point value of the unit. */
    if (u_point_value(u) > 0) {
	tbprintf(buf, "Point Value: %d%s%s", u_point_value(u), 
		 help_newline(), help_newline());
    }
    /* Display the designer's notes for this type. */
    if (u_notes(u) != lispnil) {
	tbcatline(buf, "Notes:");
	append_notes(buf, u_notes(u));
	tbprintf(buf, "%s%s", help_newline(), help_newline());
    }
    /* Display side attributes. */
    describe_utype_side_attributes(u, key, buf);
    /* Display action attributes. */
    describe_utype_actions(u, key, buf);
    /* Display movement attributes. */
    describe_utype_movement(u, key, buf);
    tbprintf(buf, "Hit Points (HP): %d.", u_hp_max(u));
    if (u_hp_recovery(u) != 0) {
	tbprintf(buf, "%sRecovers by ", help_indent(2));
	tb_fraction_desc(buf, u_hp_recovery(u));
	tbprintf(buf, " HP each turn");
	if (u_hp_to_recover(u) > 0)
	  tbprintf(buf, ", if over %d HP", u_hp_to_recover(u));
	tbcat(buf, ".");
    }
    tbcatline(buf, "");
    if (ut_table_row_not_default(u, ut_vanishes_on, 0)) {
	tbprintf(buf, "Vanishes if in: ");
	ut_table_row_desc(buf, u, ut_vanishes_on, tb_bool_desc, "");
	tbcatline(buf, ".");
    }
    if (ut_table_row_not_default(u, ut_wrecks_on, 0)) {
	tbprintf(buf, "Immediate wreck if in: ");
	ut_table_row_desc(buf, u, ut_wrecks_on, tb_bool_desc, "");
	tbcatline(buf, ".");
    }
    /* Describe unit's transport capabilities. */
    if (u_capacity(u) > 0
	|| uu_table_row_not_default(u, uu_capacity_x, 0)) {
	if (u_capacity(u) > 0) {
	    tbprintf(buf, "Generic capacity for units is %d.%s",
		     u_capacity(u), help_newline());
	    if (uu_table_column_not_default(u, uu_size, 1)) {
		tbcat(buf, "Relative sizes of occupants: ");
		uu_table_column_desc(buf, u, uu_size, NULL, NULL);
		tbcatline(buf, ".");
	    }
	}
	if (uu_table_row_not_default(u, uu_capacity_x, 0)) {
	    tbcat(buf, "Dedicated space for units: ");
	    uu_table_row_desc(buf, u, uu_capacity_x, NULL, NULL);
	    tbcatline(buf, ".");
	}
	/* (should only display if < capacities would allow?) */
	if (uu_table_row_not_default(u, uu_occ_max, -1)) {
	    tbcat(buf, "Maximum number of occupants: ");
	    uu_table_row_desc(buf, u, uu_occ_max, NULL, NULL);
	    tbcatline(buf, ".");
	}
	if (u_occ_total_max(u) >= 0) {
	    tbprintf(buf, "Maximum total of %d for all types together.%s",
		     u_occ_total_max(u), help_newline());
	}
    }
    if (any_ut_capacity_x(u)) {
	tbcat(buf, "Exclusive terrain capacity: ");
        ut_table_row_desc(buf, u, ut_capacity_x, NULL, NULL);
	tbcatline(buf, ".");
    }
    if (uu_table_row_not_default(u, uu_zoc_range, 0)) {
	tbcat(buf, "Exerts ZOC out to: ");
        uu_table_row_desc(buf, u, uu_zoc_range, NULL, NULL);
	tbcatline(buf, ".");
	if (ut_table_row_not_default(u, ut_zoc_into, 1)) {
	    tbcat(buf, "Exerts ZOC into: ");
	    ut_table_row_desc(buf, u, ut_zoc_into, tb_bool_desc, "in");
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_zoc_from_terrain, 100)) {
	    tbcat(buf, "Effect of own terrain: ");
	    ut_table_row_desc(buf, u, ut_zoc_from_terrain, tb_mult_desc, "in");
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_mp_to_enter_zoc, -1)) {
	    tbcat(buf, "MP to enter ZOC: ");
	    uu_table_row_desc(buf, u, uu_mp_to_enter_zoc, NULL, NULL);
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_mp_to_leave_zoc, 0)) {
	    tbcat(buf, "MP to leave ZOC: ");
	    uu_table_row_desc(buf, u, uu_mp_to_leave_zoc, NULL, NULL);
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_mp_to_traverse_zoc, 0)) {
	    tbcat(buf, "MP to traverse ZOC: ");
	    uu_table_row_desc(buf, u, uu_mp_to_traverse_zoc, NULL, NULL);
	    tbcatline(buf, ".");
	}
    }
    if (u_cxp_max(u) != 0) {
	tbprintf(buf, "Combat experience (CXP) maximum: %d.%s", u_cxp_max(u), 
		 help_newline());
    }
    if (u_morale_max(u) != 0) {
	tbprintf(buf, "Morale maximum: %d", u_morale_max(u));
	if (u_morale_recovery(u) != 0)
	  tbprintf(buf, ", recover %s each turn", u_morale_recovery(u));
	tbcatline(buf, ".");
    }
    if (u_cp(u) != 1) {
	tbprintf(buf, "Construction points (CP): %d.%s", u_cp(u), 
		 help_newline());
    }
    if (u_tech_to_see(u) != 0) {
	tbprintf(buf, "Tech to see: %d.%s", u_tech_to_see(u), 
		 help_newline());
    }
    if (u_tech_to_own(u) != 0) {
	tbprintf(buf, "Tech to own: %d.%s", u_tech_to_own(u), 
		 help_newline());
    }
    if (u_tech_to_use(u) != 0) {
	tbprintf(buf, "Tech to use: %d.%s", u_tech_to_use(u), 
		 help_newline());
    }
    if (u_tech_to_build(u) != 0) {
	tbprintf(buf, "Tech to build: %d.%s", u_tech_to_build(u), 
		 help_newline());
    }
    if (u_tech_to_change_type_to(u)) {
	tbprintf(buf, "Tech to change type to: %d.%s", 
		 u_tech_to_change_type_to(u), help_newline());
    }
    if (u_tech_max(u) != 0) {
	tbprintf(buf, "Tech max: %d.%s", u_tech_max(u), help_newline());
    }
    if (u_tech_max(u) != 0 && u_tech_per_turn_max(u) != PROPHI) {
	tbprintf(buf, "Tech increase per turn max: %d.%s", 
		 u_tech_per_turn_max(u), help_newline());
    }
    if (u_tech_from_ownership(u) != 0) {
	tbprintf(buf, "Tech guaranteed by ownership: %d.%s", 
		 u_tech_from_ownership(u), help_newline());
    }
    if (u_tech_leakage(u) != 0) {
	tbprintf(buf, "Tech leakage: %d.%s", u_tech_leakage(u), 
		 help_newline());
    }
    if (type_max_acp(u) > 0
        && type_can_develop(u) > 0
        ) {
        tbprintf(buf, "%sDevelop:%s", help_newline(), help_newline());
	tbcat(buf, "ACP to develop: ");
        uu_table_row_desc(buf, u, uu_acp_to_develop, NULL, NULL);
	tbcatline(buf, ".");
	tbcat_si(buf, "Tech gained: ");
        uu_table_row_desc(buf, u, uu_tech_per_develop, tb_fraction_desc, NULL);
	tbcatline(buf, ".");
    }
    if (type_max_acp(u) > 0
        && (could_create_any(u) || could_build_any(u))) {
        tbprintf(buf, "%sConstruction:%s", help_newline(), help_newline());
        if (could_create_any(u)) {
	    tbcat(buf, "ACP to create: ");
	    uu_table_row_desc(buf, u, uu_acp_to_create, NULL, NULL);
	    tbcatline(buf, ".");
	    if (uu_table_row_not_default(u, uu_create_range, 0)) {
  		tbcat_si(buf, "Creation distance max: ");
		uu_table_row_desc(buf, u, uu_create_range, NULL, NULL);
 		tbcatline(buf, ".");
 	    }
	    if (uu_table_row_not_default(u, uu_creation_cp, 1)) {
  		tbcat_si(buf, "CP upon creation: ");
		uu_table_row_desc(buf, u, uu_creation_cp, NULL, NULL);
 		tbcatline(buf, ".");
 	    }
	}
        if (could_build_any(u)) {
	    tbcat(buf, "ACP to build: ");
	    uu_table_row_desc(buf, u, uu_acp_to_build, NULL, NULL);
	    tbcatline(buf, ".");
	    if (uu_table_row_not_default(u, uu_cp_per_build, 1)) {
 		tbcat_si(buf, "CP added per build: ");
 		uu_table_row_desc(buf, u, uu_cp_per_build, NULL, NULL);
 		tbcatline(buf, ".");
 	    }
        }
        if (u_cp_per_self_build(u) > 0) {
	    tbprintf(buf, 
"Can finish building self at %d cp, will add %d cp per action.%s",
		    u_cp_to_self_build(u), u_cp_per_self_build(u), 
		    help_newline());
        }
	if (uu_table_row_not_default(u, uu_build_range, 0)) {
	    tbcat(buf, "Range at which can build: ");
	    uu_table_row_desc(buf, u, uu_build_range, NULL, NULL);
	    tbcatline(buf, ".");
	} else {
	    tbcatline(buf, "Can build at own location.");
	}
        /* Toolup help. */
        if (could_toolup_for_any(u)) {
	    tbcat(buf, "ACP to toolup: ");
	    uu_table_row_desc(buf, u, uu_acp_to_toolup, NULL, NULL);
	    tbcatline(buf, ".");
	    tbcat_si(buf, "TP/toolup action: ");
	    uu_table_row_desc(buf, u, uu_tp_per_toolup, NULL, NULL);
	    tbcatline(buf, ".");
	    /* (should put these with type beING built...) */
	    tbcat_si(buf, "TP to build: ");
	    uu_table_row_desc(buf, u, uu_tp_to_build, NULL, NULL);
	    tbcatline(buf, ".");
	    tbcat_si(buf, "TP max: ");
	    uu_table_row_desc(buf, u, uu_tp_max, NULL, NULL);
	    tbcatline(buf, ".");
        }
    }
    if ((type_max_acp(u) > 0
	 && (could_attack_any(u)
	     || could_fire_at_any(u) > 0
	     || could_capture_any(u) > 0
	    ))
	|| may_detonate(u)
	|| uu_table_row_not_default(u, uu_protection, 100)
	|| uu_table_row_not_default(u, uu_retreat_chance, 0)
	|| uu_table_row_not_default(u, uu_acp_retreat, 0)
	|| u_wrecked_type(u) != NONUTYPE
	) {
	tbprintf(buf, "%sCombat:%s", help_newline(), help_newline());
	if (could_attack_any(u)) {
	    tbcat(buf, "Can attack (ACP ");
	    uu_table_row_desc(buf, u, uu_acp_to_attack, NULL, "vs");
	    tbcatline(buf, ").");
	    if (uu_table_row_not_default(u, uu_attack_range, 1)) {
		tbcat(buf, "Attack range is ");
		uu_table_row_desc(buf, u, uu_attack_range, NULL, "vs");
		tbcatline(buf, ".");
		tbcat(buf, "Attack range min is ");
		uu_table_row_desc(buf, u, uu_attack_range_min, NULL, "vs");
		tbcatline(buf, ".");
	    }
	}
   	if (u_acp_to_fire(u) > 0) {
	    tbprintf(buf, "Can fire (%d ACP), at ranges", u_acp_to_fire(u));
	    if (u_range_min(u) > 0) {
		tbprintf(buf, " from %d", u_range_min(u));
	    }
	    tbprintf(buf, " up to %d", u_range(u));
	    tbcatline(buf, ".");
	}
	tbcat(buf, "Hit chances are ");
	uu_table_row_desc(buf, u, uu_hit, tb_percent_desc, "vs");
	tbcatline(buf, ".");
   	if (u_acp_to_fire(u) > 0) {
	    if (uu_table_row_not_default(u, uu_fire_hit, -1)) {
		tbcat(buf, "Hit chances if firing are ");
		uu_table_row_desc(buf, u, uu_fire_hit, tb_percent_desc, "vs");
		tbcatline(buf, ".");
	    } else {
		tbcatline(buf, 
		          "Hit chances if firing same as for regular combat.");
	    }
   	}
	if (ut_table_row_not_default(u, ut_attack_terrain_effect, 100)) {
	    tbcat(buf, "Effect of attacker's terrain is ");
	    ut_table_row_desc(buf, u, ut_attack_terrain_effect,
			      tb_mult_desc, "in");
	    tbcatline(buf, ".");
	}
	tbcat(buf, "Damage is ");
	uu_table_row_desc(buf, u, uu_damage, tb_dice_desc, "vs");
	tbcatline(buf, ".");
	if (uu_table_row_not_default(u, uu_tp_damage, 0)) {
	    tbcat(buf, "Tooling damage is ");
	    uu_table_row_desc(buf, u, uu_tp_damage, tb_dice_desc, NULL);
	    tbcatline(buf, ".");
	}
   	if (u_acp_to_fire(u) > 0) {
	    if (uu_table_row_not_default(u, uu_fire_damage, -1)) {
		tbcat(buf, "Damage if firing is ");
		uu_table_row_desc(buf, u, uu_fire_damage, tb_dice_desc, "vs");
		tbcatline(buf, ".");
	    } else {
		tbcatline(buf, "Damages if firing same as for regular combat.");
	    }
	}
	if (uu_table_row_not_default(u, uu_acp_to_defend, 1)) {
	    tbcat(buf, "If attacked, ACP to defend is ");
	    uu_table_row_desc(buf, u, uu_acp_to_defend, NULL, "vs");
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_defend_terrain_effect, 100)) {
	    tbcat(buf, "If attacked, effect of own terrain is ");
	    ut_table_row_desc(buf, u, ut_defend_terrain_effect,
			      tb_mult_desc, "in");
	    tbcatline(buf, ".");
	}
	if (um_table_row_not_default(u, um_hit_by, 0)) {
	    tbcat(buf, "Ammo needed to hit unit: ");
	    um_table_row_desc(buf, u, um_hit_by, NULL);
	    tbcatline(buf, ".");
	}
	if (could_capture_any(u) > 0) {
	    tbcat(buf, "Can capture (ACP ");
	    uu_table_row_desc(buf, u, uu_acp_to_capture, NULL, "vs");
	    tbcatline(buf, ").");
	    tbcat(buf, "Chance to capture: ");
	    uu_table_row_desc(buf, u, uu_capture, tb_percent_desc, "vs");
	    tbcatline(buf, ".");
	    if (uu_table_row_not_default(u, uu_indep_capture, -1)) {
		tbcat(buf, "Chance to capture indep: ");
		uu_table_row_desc(buf, u, uu_indep_capture,
				  tb_percent_desc, "vs");
		tbcatline(buf, ".");
	    }
	}
	if (may_detonate(u)) {
	    /* Display all the different ways that a unit might detonate. */
	    if (u_acp_to_detonate(u) > 0) {
		tbprintf(buf, "Can detonate self (%d ACP).%s",
			 u_acp_to_detonate(u), help_newline());
	    }
	    if (u_detonate_on_death(u)) {
		tbprintf(buf,
			 "%d%% chance to detonate if destroyed in combat.%s",
			 u_detonate_on_death(u), help_newline());
	    }
	    if (uu_table_row_not_default(u, uu_detonate_on_hit, 0)) {
		tbcat(buf, "Chance to detonate upon being hit: ");
		uu_table_row_desc(buf, u, uu_detonate_on_hit, tb_percent_desc,
				  NULL);
		tbcatline(buf, ".");
	    }
	    if (uu_table_row_not_default(u, uu_detonate_on_capture, 0)) {
		tbcat(buf, "Chance to detonate upon capture: ");
		uu_table_row_desc(buf, u, uu_detonate_on_capture,
				  tb_percent_desc, NULL);
		tbcatline(buf, ".");
	    }
	    if (uu_table_row_not_default(u, uu_detonate_approach_range, -1)) {
		tbcat(buf, "Will detonate upon approach within range: ");
		uu_table_row_desc(buf, u, uu_detonate_approach_range,
				  NULL, NULL);
		tbcatline(buf, ".");
	    }
	    if (ut_table_row_not_default(u, ut_detonation_accident, 0)) {
		tbcat(buf, "Chance of accidental detonation: ");
		ut_table_row_desc(buf, u, ut_detonation_accident,
				  tb_percent_desc, "in");
		tbcatline(buf, ".");
	    }
	    tbcat(buf, "Detonation damage at ground zero is ");
	    uu_table_row_desc(buf, u, uu_detonation_damage_at, tb_dice_desc, 
			      NULL);
	    tbcatline(buf, ".");
	    /* (should only display if effect range > 0) */
	    tbcat(buf, "Detonation damage to adjacent units is ");
	    uu_table_row_desc(buf, u, uu_detonation_damage_adj, tb_dice_desc, 
			      NULL);
	    tbcatline(buf, ".");
	    tbcat(buf, "Range of detonation effect on units is ");
	    uu_table_row_desc(buf, u, uu_detonation_range, NULL, NULL);
	    tbcatline(buf, ".");
	    /* Damage decreases as inverse square of distance. */
	    if (ut_table_row_not_default(u, ut_detonation_damage, 0)) {
		tbcat(buf, "Chance of detonation damage to terrain is ");
		ut_table_row_desc(buf, u, ut_detonation_damage, NULL, NULL);
		tbcatline(buf, ".");
		tbcat(buf, "Range of detonation effect on terrain is ");
		ut_table_row_desc(buf, u, ut_detonation_range, NULL, NULL);
		tbcatline(buf, ".");
	    }
	    if (u_hp_per_detonation(u) < u_hp_max(u)) {
		tbprintf(buf, "Loses %d HP per detonation.%s",
			 u_hp_per_detonation(u), help_newline());
	    } else {
		tbcatline(buf, "Always destroyed by detonation.");
	    }
	}
	if (uu_table_row_not_default(u, uu_hp_min, 0)) {
	    tbcat(buf, "Combat never reduces defender's HP below ");
	    uu_table_row_desc(buf, u, uu_hp_min, NULL, "vs");
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_protection, 100)) {
	    tbcat(buf, "Protection of occupants/transport is ");
	    uu_table_row_desc(buf, u, uu_protection, tb_mult_desc, NULL);
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_occ_combat, 100)) {
	    tbcat(buf, "Combat effectiveness as occupant is ");
	    uu_table_row_desc(buf, u, uu_occ_combat, tb_percent_desc, "in");
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_retreat_chance, 0)) {
	    tbcat(buf, "Chance to retreat from combat is ");
	    uu_table_row_desc(buf, u, uu_retreat_chance, tb_percent_desc,
			      "vs");
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_acp_retreat, 0)) {
	    tbcat(buf, "Extra ACP for retreating is ");
	    uu_table_row_desc(buf, u, uu_acp_retreat, NULL, "vs");
	    tbcatline(buf, ".");
	}
	if (u_wrecked_type(u) != NONUTYPE) {
	    tbprintf(buf, "Becomes %s when destroyed.%s",
		     u_type_name(u_wrecked_type(u)), help_newline());
	}
    }
    if (type_max_acp(u) > 0
        && (u_acp_to_change_side(u) > 0
            || u_acp_to_disband(u) > 0
	    || uu_table_row_not_default(u, uu_acp_to_repair, 0)
	    || (uu_table_row_not_default(u, uu_change_type_to, 0)
		&& uu_table_row_not_default(u, uu_acp_to_change_type, 0))
	    || ut_table_row_not_default(u, ut_acp_to_add_terrain, 0)
	    || ut_table_row_not_default(u, ut_acp_to_remove_terrain, 0)
            )) {
	tbprintf(buf, "%sOther Actions:%s", help_newline(), help_newline());
	if (u_acp_to_change_side(u) > 0) {
	    tbprintf(buf, "Can be given to another side (%d ACP).%s",
		    u_acp_to_change_side(u), help_newline());
	}
	if (u_acp_to_disband(u) > 0) {
	    tbprintf(buf, "Can be disbanded (%d ACP)", u_acp_to_disband(u));
	    if (u_hp_per_disband(u) < u_hp_max(u)) {
	    	tbprintf(buf, ", losing %d HP per action",
			 u_hp_per_disband(u));
	    }
	    tbcatline(buf, ".");
	    if (um_table_row_not_default(u, um_supply_per_disband, 0)) {
		tbprintf(buf, "Supply yield per disband action: ");
		um_table_row_desc(buf, u, um_supply_per_disband, NULL);
		tbcatline(buf, ".");
	    }
	    if (um_table_row_not_default(u, um_recycleable, 0)) {
		tbprintf(buf, "Additional material when unit is gone: ");
		um_table_row_desc(buf, u, um_recycleable, NULL);
		tbcatline(buf, ".");
	    }
	}
	if (uu_table_row_not_default(u, uu_acp_to_repair, 0)) {
	    tbcat(buf, "ACP for explicit repair is ");
	    uu_table_row_desc(buf, u, uu_acp_to_repair, NULL, NULL);
	    tbcatline(buf, ".");
	    tbcat(buf, "Explicit repair performance is ");
	    uu_table_row_desc(buf, u, uu_hp_per_repair, NULL, NULL);
	    tbcatline(buf, ".");
	}
	/* (More work still needs to be done, so that we can accomodate 
	    'u_auto_upgrade_to' and ACP-less 'change-type' possibilities.) */
	if (uu_table_row_not_default(u, uu_change_type_to, FALSE)) {
	    if (uu_table_row_not_default(u, uu_acp_to_change_type, 0)) {
		tbcat(buf, "ACP to change type is ");
		uu_table_row_desc(buf, u, uu_acp_to_change_type, NULL, NULL);
		tbcatline(buf, ".");
		if (um_table_row_not_default(u, um_to_change_type, 0)) {
		    tbcat(buf, "Material to change type is ");
		    um_table_row_desc(buf, u, um_to_change_type, NULL);
		    tbcatline(buf, ".");
		}
		if (ua_table_row_not_default(u, ua_to_change_type, FALSE)) {
		    tbcat(buf, "Advance to change type is ");
		    ua_table_row_desc(buf, u, ua_to_change_type, tb_bool_desc);
		    tbcatline(buf, ".");
		}
		if (uu_table_row_not_default(u, uu_size_to_change_type, 0)) {
		    tbcat(buf, "Size to change type is ");
		    uu_table_row_desc(buf, u, uu_size_to_change_type, NULL, 
				      NULL);
		    tbcatline(buf, ".");
		}
		if (uu_table_row_not_default(u, uu_cxp_to_change_type, 0)) {
		    tbcat(buf, "Combat experience to change type is ");
		    uu_table_row_desc(buf, u, uu_cxp_to_change_type, NULL, 
				      NULL);
		    tbcatline(buf, ".");
		}
	    }
	}
	if (ut_table_row_not_default(u, ut_acp_to_add_terrain, 0)) {
	    tbcat(buf, "ACP to add/change terrain is ");
	    ut_table_row_desc(buf, u, ut_acp_to_add_terrain, NULL, NULL);
	    tbcatline(buf, ".");
	    if (ut_table_row_not_default(u, ut_alter_range, 0)) {
		tbcat_si(buf, "Can alter up to range ");
		ut_table_row_desc(buf, u, ut_alter_range, NULL, NULL);
		tbcatline(buf, ".");
	    }
	}
	if (ut_table_row_not_default(u, ut_acp_to_remove_terrain, 0)) {
	    tbcat(buf, "ACP to remove terrain is ");
	    ut_table_row_desc(buf, u, ut_acp_to_remove_terrain, NULL, NULL);
	    tbcatline(buf, ".");
	    if (ut_table_row_not_default(u, ut_alter_range, 0)) {
		tbcat_si(buf, "Can alter up to range ");
		ut_table_row_desc(buf, u, ut_alter_range, NULL, NULL);
		tbcatline(buf, ".");
	    }
	}
    }
    if (!g_see_all()) {
    	tbprintf(buf, "%sVision:%s", help_newline(), help_newline());
	tbprintf(buf, "%d%% chance to be seen at outset of game.%s",
		 u_already_seen(u), help_newline());
	tbprintf(buf, 
		 "%d%% chance to be seen at outset of game if independent.%s",
		 u_already_seen_indep(u), help_newline());
	if (u_see_always(u))
	  tbcatline(buf, "Always seen if terrain has been seen.");
	else
	  tbcatline(buf, "Not always seen even if terrain has been seen.");
	/* (should only say if can be an occupant) */
	if (uu_table_row_not_default(u, uu_occ_vision, 100)) {
	    tbcat(buf, "Vision effect when occupying: ");
	    uu_table_row_desc(buf, u, uu_occ_vision, NULL, "in");
	    tbcatline(buf, ".");
	}
	/* (should only say if unit can have occupants) */
	if (u_see_occupants(u))
	  tbcatline(buf, "Occupants seen if unit has been seen.");
	else
	  tbcatline(buf, "Occupants not seen even if unit has been seen.");
	switch (u_vision_range(u)) {
	  case -1:
	    tbcatline(buf, "Can never see other units.");
	    break;
	  case 0:
	    tbcatline(buf, "Can see other units at own location.");
	    break;
	  case 1:
	    /* Default range, no need to say anything. */
	    break;
	  default:
	    tbprintf(buf, 
		     "Can see units up to %d cells away.%s", u_vision_range(u), 
		     help_newline());
	    break;
	}
	if (u_vision_range(u) > 0 && !u_can_see_behind(u)) {
	    tbcat(buf, "Vision is line-of-sight");
	    if (u_can_see_behind(u) > 0)
	      tbprintf(buf, ", seeing behind obstacles.");
	    tbcatline(buf, ".");
	    if (ut_table_row_not_default(u, ut_eye_height, 0)) {
		tbcat(buf, "Effective eye height is ");
		ut_table_row_desc(buf, u, ut_eye_height, NULL, "in");
		tbcatline(buf, ".");
	    }
	}
	if (ut_table_row_not_default(u, ut_body_height, 0)) {
	    tbcat(buf, "Effective body height is ");
	    ut_table_row_desc(buf, u, ut_body_height, NULL, "in");
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_weapon_height, 0)) {
	    tbcat(buf, "Effective weapon height is ");
	    ut_table_row_desc(buf, u, ut_weapon_height, NULL, "in");
	    tbcatline(buf, ".");
	}
	if (u_vision_range(u) >= 0
	    && uu_table_row_not_default(u, uu_see_at, 100)) {
	    tbcat(buf, "Chance to see if in same cell is ");
	    uu_table_row_desc(buf, u, uu_see_at, tb_percent_desc, NULL);
	    tbcatline(buf, ".");
	}
	if (u_vision_range(u) >= 1
	    && uu_table_row_not_default(u, uu_see_adj, 100)) {
	    tbcat(buf, "Chance to see if adjacent is ");
	    uu_table_row_desc(buf, u, uu_see_adj, tb_percent_desc, NULL);
	    tbcatline(buf, ".");
	}
	if (u_vision_range(u) >= 2
	    && uu_table_row_not_default(u, uu_see, 100)) {
	    tbcat(buf, "Chance to see in general is ");
	    uu_table_row_desc(buf, u, uu_see, tb_percent_desc, NULL);
	    tbcatline(buf, ".");
	}
	if (u_see_terrain_captured(u) > 0)
	  tbprintf(buf, 
"%d%% chance for enemy to see your terrain view if this type captured.%s",
		   u_see_terrain_captured(u), help_newline());
    }
    if (ut_table_row_not_default(u, ut_accident_hit, 0)
	|| ut_table_row_not_default(u, ut_accident_vanish, 0)) {
	tbprintf(buf, "%sAccidents:%s", help_newline(), help_newline());
	if (ut_table_row_not_default(u, ut_accident_hit, 0)) {
	    tbcat(buf, "Chance to be damaged in an accident: ");
	    ut_table_row_desc(buf, u, ut_accident_hit, tb_percent_100th_desc, 
			      "in");
	    tbcatline(buf, ".");
	    tbcat(buf, "Amount of damage: ");
	    ut_table_row_desc(buf, u, ut_accident_damage, tb_dice_desc, "in");
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_accident_vanish, 0)) {
	    tbcat(buf, "Chance to vanish in an accident: ");
	    ut_table_row_desc(buf, u, ut_accident_vanish,
			      tb_percent_100th_desc, "in");
	    tbcatline(buf, ".");
	}
    }
    if (ut_table_row_not_default(u, ut_attrition, 0)) {
	tbprintf(buf, "%sAttrition in HP:%s", help_newline(), help_newline());
	ut_table_row_desc(buf, u, ut_attrition, tb_probfraction_desc, "in");
	tbcatline(buf, ".");
    }
    /* Don't bother with this if economy code is not running. */
    if (nummtypes > 0 && g_economy()) {
	tbprintf(buf, "%sMaterial Handling:%s", help_newline(), help_newline());
	for_all_material_types(m) {
	    usesm = FALSE;
	    tbprintf(buf, "%s%s", help_indent(2), m_type_name(m));
	    if (um_base_production(u, m) > 0) {
		tbprintf(buf, ", %d basic production",
			 um_base_production(u, m));
		usesm = TRUE;
	    }
	    if (um_occ_production(u, m) >= 0) {
		tbprintf(buf, ", %d basic production if occupant",
			 um_occ_production(u, m));
		usesm = TRUE;
	    }
	    if (um_acp_to_extract(u, m) > 0) {
		tbprintf(buf, ", %d ACP to extract", um_acp_to_extract(u, m));
		usesm = TRUE;
	    }
	    if (um_storage_x(u, m) > 0) {
		tbprintf(buf, ", %d storage", um_storage_x(u, m));
		if (um_initial(u, m) > 0) {
		    tbprintf(buf, " (%d at start of game)",
			     min(um_initial(u, m), um_storage_x(u, m)));
		}
		usesm = TRUE;
	    }
	    if (um_base_consumption(u, m) > 0) {
		tbprintf(buf, ", %d basic consumption",
			 um_base_consumption(u, m));
		if (um_consumption_as_occupant(u, m) != 100) {
		    tbprintf(buf, ", times %d%% if occupant",
			     um_consumption_as_occupant(u, m));
		}
		usesm = TRUE;
	    }
	    if (um_to_act(u, m) != 0) {
		tbprintf(buf, ", needs %d to act at all",
			 um_to_act(u, m));
		usesm = TRUE;
	    }
	    if (um_to_move(u, m) != 0) {
		tbprintf(buf, ", needs %d to move",
			 um_to_move(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_move(u, m) != 0) {
		tbprintf(buf, ", %d consumed per move",
			 um_consumption_per_move(u, m));
		usesm = TRUE;
	    }
	    if (um_to_attack(u, m) != 0) {
		tbprintf(buf, ", needs %d (weapons or equipment) to attack",
			 um_to_attack(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_attack(u, m) != 0) {
		tbprintf(buf, ", %d (ammo) consumed per attack",
			 um_consumption_per_attack(u, m));
		usesm = TRUE;
	    }
	    if (u_acp_to_fire(u) > 0 && um_to_fire(u, m) != 0) {
		tbprintf(buf, ", needs %d (weapons or equipment) to fire",
			 um_to_fire(u, m));
		usesm = TRUE;
	    }
	    if (u_acp_to_fire(u) > 0 && um_consumption_per_fire(u, m) != 0) {
		tbprintf(buf, ", %d (ammo) consumed when firing",
			 um_consumption_per_fire(u, m));
		usesm = TRUE;
	    }
	    if (um_to_create(u, m) != 0) {
		tbprintf(buf, ", builder needs %d to create",
			 um_to_create(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_on_creation(u, m) != 0) {
		tbprintf(buf, ", %d used in creation",
			 um_consumption_on_creation(u, m));
		usesm = TRUE;
	    }
	    if (um_to_build(u, m) != 0) {
		tbprintf(buf, ", builder needs %d to build anything",
			 um_to_build(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_build(u, m) != 0) {
		tbprintf(buf, ", %d used to build anything",
			 um_consumption_per_build(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_built(u, m) != 0) {
		tbprintf(buf, ", %d used to perform a build upon",
			 um_consumption_per_built(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_cp(u, m) != 0) {
		tbprintf(buf, ", %d used to add 1 CP to",
			 um_consumption_per_cp(u, m));
		usesm = TRUE;
	    }
	    if (um_to_repair(u, m) != 0) {
		tbprintf(buf, ", repairer needs %d to repair",
			 um_to_repair(u, m));
		usesm = TRUE;
	    }
	    if (um_consumption_per_repair(u, m) != 0) {
		tbprintf(buf, ", %d used to restore 1 HP",
			 um_consumption_per_repair(u, m));
		usesm = TRUE;
	    }
	    if (usesm) {
		if (um_inlength(u, m) > 0) {
		    tbprintf(buf, ", receive from up to %d cells away",
			     um_inlength(u, m));
		}
		if (um_outlength(u, m) > 0) {
		    tbprintf(buf, ", send up to %d cells away",
			     um_outlength(u, m));
		}
	    } else {
		tbcat(buf, " (none)");
	    }
	    tbcatline(buf, "");
	}
	/* Productivity adjustment due to terrain applies to all
	   material types equally, but only display if unit
	   produces. */
	if (um_table_row_not_default(u, um_base_production, 0)) {
	    if (ut_table_row_not_default(u, ut_productivity, 100)) {
		tbcat(buf, "Productivity adjustment for terrain is ");
		ut_table_row_desc(buf, u, ut_productivity, tb_mult_desc, "in");
		tbcatline(buf, ".");
	    }
	    if (ut_table_row_not_default(u, ut_productivity_adj, 0)) {
		tbcat(buf, "Productivity adjustment for adjacent terrain is ");
		ut_table_row_desc(buf, u, ut_productivity_adj, tb_mult_desc,
				  "in");
		tbcatline(buf, ".");
	    }
	    if (um_table_row_not_default(u, um_productivity_min, 0)) {
		tbcat(buf, "Minimum net adjustment is ");
		um_table_row_desc(buf, u, um_productivity_min, tb_mult_desc);
		tbcatline(buf, ".");
	    }
	    if (um_table_row_not_default(u, um_productivity_max, TABHI)) {
		tbcat(buf, "Maximum net adjustment is ");
		um_table_row_desc(buf, u, um_productivity_max, tb_mult_desc);
		tbcatline(buf, ".");
	    }
	}
    }
    if (numatypes > 0) {
	/* Now also prints "None" if no advances are required. */
	int	found = FALSE;

	tbprintf(buf, "%sRequired advances to build: ", help_newline());
	for_all_advance_types(a) {
	    if (ua_needed_to_build(u, a) > 0) {
		if (found)
			tbprintf(buf, ", ");
		tbprintf(buf, "%s", a_type_name(a));
		found = TRUE;
	    }
	}
	if (found)
	  tbcatline(buf, ".");		
	else
	  tbcatline(buf, "None.");
    }
    tbcatline(buf, "");
    /* (should display weather interaction here) */
    if (uu_table_row_not_default(u, uu_auto_repair, 0)) {
	tbcat(buf, "Can auto-repair lost HP of other units: ");
	uu_table_row_desc(buf, u, uu_auto_repair, tb_probfraction_desc, NULL);
	tbcatline(buf, ".");
	if (uu_table_row_not_default(u, uu_auto_repair_range, 0)) {
	    tbcat(buf, "Range of auto-repair: ");
	    uu_table_row_desc(buf, u, uu_auto_repair_range, NULL, NULL);
	    tbcatline(buf, ".");
	}
    }
    if (u_spy_chance(u) > 0 /* and random event in use */) {
	tbprintf(buf, "%d%% chance to spy, on units up to %d away.%s",
		 u_spy_chance(u), u_spy_range(u), help_newline());
    }
    if (u_revolt(u) > 0 /* and random event in use */) {
	tb_fraction_desc(buf, u_revolt(u));
	tbcatline(buf, "% chance of revolt.");
    }
    if (u_lost_wreck(u) > 0
	|| u_lost_vanish(u) > 0
	|| u_lost_revolt(u) > 0
	|| uu_table_row_not_default(u, uu_lost_surrender, 0)) {
	tbprintf(buf, "%sFate if side loses:", help_newline());
	if (u_lost_wreck(u) > 0) {
	    tbcat(buf, "  ");
	    tb_percent_100th_desc(buf, u_lost_wreck(u));
	    tbcat(buf, " chance to wreck");
	}
	if (u_lost_wreck(u) < 10000 && u_lost_vanish(u) > 0) {
	    tbcat(buf, "  ");
	    tb_percent_100th_desc(buf, u_lost_vanish(u));
	    tbcat(buf, " chance to vanish");
	}
	if (u_lost_wreck(u) < 10000
	    && u_lost_vanish(u) < 10000
	    && u_lost_revolt(u) > 0) {
	    tbcat(buf, "  ");
	    tb_percent_100th_desc(buf, u_lost_revolt(u));
	    tbcat(buf, " chance to revolt");
	}
	if (u_lost_wreck(u) < 10000
	    && u_lost_vanish(u) < 10000
	    && u_lost_revolt(u) < 10000
	    && uu_table_row_not_default(u, uu_lost_surrender, 0)) {
	    tbcat(buf, " chance to surrender to nearby unit, ");
	    uu_table_row_desc(buf, u, uu_lost_surrender,
			      tb_percent_100th_desc, "to");
	}
	if (u_lost_wreck(u) == 0
	    && u_lost_vanish(u) == 0
	    && u_lost_revolt(u) == 0
	    && !uu_table_row_not_default(u, uu_lost_surrender, 0))
	  tbcat(buf, " survival");
	tbprintf(buf, ".%s%s", help_newline(), help_newline());
    }
    if (u_type_in_game_max(u) >= 0) {
	tbprintf(buf, "At most %d allowed in a game.%s", u_type_in_game_max(u), 
		 help_newline());
    }
#ifdef DESIGNERS
    if (numdesigners > 0) {
	tbcat(buf, "FOR DESIGNERS:\n");
	tbprintf(buf, "Internal name is \"%s\".%s", u_internal_name(u), 
		 help_newline());
	tbprintf(buf, "Short name is \"%s\".%s", u_short_name(u), 
		 help_newline());
	if (u_assign_number(u))
	  tbcatline(buf, "New units get assigned a number.");
	tbcatline(buf, "");
    }
#endif /* DESIGNERS */
    describe_utype_ai_attributes(u, key, buf);
}

/* Describe the unit type's ability to move. */

static void
describe_utype_movement(int u, const char *key, TextBuffer *buf)
{
    int speedvaries = FALSE;

    tbcatline(buf, 
	      "Movement, Movement Points (MP), and Speeds (MP/ACP Ratios):");
    if (type_max_speed(u) <= 0) {
	tbprintf(buf, "%sIs always immobile.%s", help_indent(2), 
		 help_newline());
    }
    else {
	if (u_acp_to_move(u) > 0) {
	    tbprintf(buf, "%sUses a minimum of %d ACP per move.%s", 
		     help_indent(2), u_acp_to_move(u), help_newline());
	} else {
	    tbprintf(buf, "%sCannot move by its own action.%s",
		     help_indent(2), help_newline());
	}
	if (u_speed(u) != uprop_i_default(u_speed)) {
	    tbprintf(buf, "%sUnadjusted speed is ", help_indent(2));
	    tb_percent_desc(buf, u_speed(u));
	    tbcatline(buf, ".");
	}
	if (uu_table_row_not_default(u, uu_occ_adds_speed, 
				     table_default(uu_occ_adds_speed))) {
	    tbprintf(buf, "%sOccupant adds speed: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_occ_adds_speed, tb_percent_desc, NULL);
	    tbcatline(buf, ".");
	    speedvaries = TRUE;
	}
	if (uu_table_row_not_default(u, uu_occ_multiplies_speed,
				     table_default(uu_occ_multiplies_speed))) {
	    tbprintf(buf, "%sOccupant multiplies speed: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_occ_multiplies_speed, tb_mult_desc, 
			      NULL);
	    tbcatline(buf, ".");
	    speedvaries = TRUE;
	}
	if (u_speed_wind_effect(u) != uprop_l_default(u_speed_wind_effect)) {
	    /* (should add mech to describe in detail) */
	    tbprintf(buf, "%sWind affects speed.%s", help_indent(2), 
		     help_newline());
	    speedvaries = TRUE;
	}
	if (u_speed_damage_effect(u) != 
	    uprop_l_default(u_speed_damage_effect)) {
	    /* (should add mech to describe in detail) */
	    tbprintf(buf, "%sDamage affects speed.%s", help_indent(2), 
		     help_newline());
	    speedvaries = TRUE;
	}
	/* (should only list variation limits if actually needed to clip) */
	if (speedvaries) {
	    tbprintf(buf, "%sSpeed variation limited to between ", 
		     help_indent(2));
	    tb_percent_desc(buf, u_speed_min(u));
	    tbcat(buf, " and ");
	    tb_percent_desc(buf, u_speed_max(u));
	    tbcatline(buf, ".");
	}
	tbprintf(buf, "%sNeeds MP to enter terrain: ", help_indent(2));
	ut_table_row_desc(buf, u, ut_mp_to_enter, NULL, "into");
	tbcatline(buf, ".");
	if (ut_table_row_not_default(u, ut_mp_to_leave,
				     table_default(ut_mp_to_leave))) {
	    tbprintf(buf, "%sNeeds MP to leave terrain: ", help_indent(2));
	    ut_table_row_desc(buf, u, ut_mp_to_leave, NULL, "in");
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_mp_to_traverse,
				     table_default(ut_mp_to_traverse))) {
	    tbprintf(buf, "%sNeeds MP to traverse terrain: ", help_indent(2));
	    ut_table_row_desc(buf, u, ut_mp_to_traverse, NULL, "across");
	    tbcatline(buf, ".");
	}
	if (any_mp_to_enter_unit(u)) {
	    tbprintf(buf, "%sMP to enter unit: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_mp_to_enter, NULL, "into");
	    tbcatline(buf, ".");
	}
	if (any_mp_to_leave_unit(u)) {
	    tbprintf(buf, "%sMP to leave unit: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_mp_to_leave, NULL, "in");
	    tbcatline(buf, ".");
	}
	if (any_enter_indep(u)) {
	    tbprintf(buf, "%sCan enter independent units: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_can_enter_indep, tb_bool_desc, NULL);
	    tbcatline(buf, ".");
	}
	if (u_mp_to_leave_world(u) > uprop_i_default(u_mp_to_leave_world)) {
	    tbprintf(buf, "%sNeeds %d MP to leave the world entirely.%s", 
		     help_indent(2), u_mp_to_leave_world(u), help_newline());
	}
	if (u_free_mp(u) > 0) {
	    tbprintf(buf, 
		     "%sGets up to %d free MP if needed to finish a move.%s", 
		     help_indent(2), u_free_mp(u), help_newline());
	}
    }
    tbcatline(buf, "");
}

/* Describe the unit type's ability to act. */

static void
describe_utype_actions(int u, const char *key, TextBuffer *buf)
{
    tbcatline(buf, "Actions and Action Points (ACP):");
    /* (Should enumerate possible actions here.) */
    /* ACP independent. */
    if (u_acp_independent(u)) {
	tbprintf(buf, "%sActs independently of ACP restrictions.%s", 
		 help_indent(2), help_newline());
    }
    /* Actionless. */
    else if (type_max_acp(u) <= 0) {
	tbprintf(buf, "%sDoes not act.%s", help_indent(2), help_newline());
    }
    /* ACP-dependent actions. */
    else {
	/* Limits on total ACP. */
	if (u_acp_min(u) != uprop_i_default(u_acp_min))
	  tbprintf(buf, "%sCan act until %d ACP left.%s", help_indent(2), 
		   u_acp_min(u), help_newline());
	if (u_acp_max(u) != uprop_i_default(u_acp_max)) {
	  tbprintf(buf, "%sWill never exceed %d ACP.%s", help_indent(2), 
		   u_acp_max(u), help_newline());
	}
	/* Limits on new ACP per turn. */
	if (u_acp_turn_min(u) != uprop_i_default(u_acp_turn_min))
	  tbprintf(buf, "%sGuaranteed to gain at least %d new ACP per turn.%s",
		   help_indent(2), u_acp_turn_min(u), help_newline());
	if (u_acp_turn_max(u) != uprop_i_default(u_acp_turn_max))
	  tbprintf(buf, "%sWill never exceed %d new ACP per turn.%s",
		   help_indent(2), u_acp_turn_max(u), help_newline());
	/* Basic new ACP per turn. */
	if (u_acp(u) != uprop_i_default(u_acp))
	  tbprintf(buf, "%sReceives basic allotment of %d new ACP per turn.%s",
		   help_indent(2), u_acp(u), help_newline());
	/* Possible added ACP per turn. */
	if (uu_table_row_not_default(u, uu_occ_adds_acp, 
				     table_default(uu_occ_adds_acp))) {
	    tbprintf(buf, "%sOccupant adds ACP: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_occ_adds_acp, NULL, NULL);
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_night_adds_acp,
				     table_default(ut_night_adds_acp))) {
	    tbprintf(buf, "%sNight adds ACP: ", help_indent(2));
	    ut_table_row_desc(buf, u, ut_night_adds_acp, NULL, "in");
	    tbcatline(buf, ".");
	}
	if (u_acp_damage_effect(u) != uprop_l_default(u_acp_damage_effect)) {
	    /* (Should write out ACP damage interpolation list.) */
	    tbprintf(buf, "%sDamage affects ACP.%s", help_indent(2), 
		     help_newline());
	}
	/* Possible ACP multipliers. */
	if (uu_table_row_not_default(u, uu_occ_multiplies_acp,
				     table_default(uu_occ_multiplies_acp))) {
	    tbprintf(buf, "%sOccupant multiplies ACP: ", help_indent(2));
	    uu_table_row_desc(buf, u, uu_occ_multiplies_acp, tb_mult_desc, 
			      NULL);
	    tbcatline(buf, ".");
	}
	if (ut_table_row_not_default(u, ut_night_multiplies_acp,
				     table_default(ut_night_multiplies_acp))) {
	    tbprintf(buf, "%sNight multiplies ACP: ", help_indent(2));
	    ut_table_row_desc(buf, u, ut_night_multiplies_acp, tb_mult_desc, 
			      "in");
	    tbcatline(buf, ".");
	}
	/* (Should consider other multipliers such as temperature.) */
	/* Free ACP. */
	if (u_free_acp(u) != uprop_i_default(u_free_acp)) {
	    tbprintf(buf, "%sAllowed %d free ACP to complete an action.%s", 
		     help_indent(2), u_free_acp(u), help_newline());
	}
    }
    tbcatline(buf, "");
}

/* Describe the unit type's relevance to the sides. */

static void
describe_utype_side_attributes(int u, const char *key, TextBuffer *buf)
{
    int hasattribs = FALSE, first = TRUE;
    Side *side = NULL;
    char sidetmpbuf[BUFSIZE];

    tbcatline(buf, "Side Attributes:");
    /* Possible sides. */
    if (u_possible_sides(u) != lispnil) {
	tbprintf(buf, "%sCan belong to the following sides: ", help_indent(2));
	first = TRUE;
	for_all_sides(side) {
	    if (type_allowed_on_side(u, side)) {
		if (first)
		  first = FALSE;
		else
		  tbcat(buf, ", ");
		tbcat(buf, shortest_side_title(side, sidetmpbuf));
	    }
    	}
    	tbcatline(buf, ".");
	hasattribs = TRUE;
    }
    /* (Should mention whether the unit can ever be available to the 
	side of the player viewing the help.) */
    /* Self unit. */
    if (u_can_be_self(u)) {
    	tbcatline_si(buf, "Can be a side leader/capital (aka. \"self\") unit.");
    	if (u_self_changeable(u))
	  tbcatline_si(buf, "Side can opt for a different self unit instead.");
    	if (u_self_resurrects(u))
	  tbcatline_si(buf, "Upon death, annoint a new self unit.");
	/* (Should move the following test to side help.) */
	if (g_self_required())
	  tbcatline_si(buf, "(Side must always have a self unit during game.)");
	/* (If an unit of this type is the side's self unit, then should 
	    mention its name and location.) */
	hasattribs = TRUE;
    }
    /* How many per side? */
    if (u_type_per_side_max(u) >= 0) {
	tbprintf(buf, "%sAt most %d allowed on each side in a game.%s",
		 help_indent(2), u_type_per_side_max(u), help_newline());
	hasattribs = TRUE;
    }
    /* (Should tell how many the side of the player currently owns. And 
	perhaps how many allied sides own in addition.) */
    if (!u_direct_control(u)) {
	tbcatline_si(buf, "Cannot be controlled directly by side.");
	hasattribs = TRUE;
    }
    if (!hasattribs)
      tbcatline_si(buf, "None.");
    tbcatline(buf, "");
}

/* Describe the unit type's AI attributes. */

static void
describe_utype_ai_attributes(int u, const char *key, TextBuffer *buf)
{
    int hasattribs = FALSE;

    tbcatline(buf, "AI Attributes:");
    if (u_colonizer(u)) {
	tbcatline_si(buf, "Can build advanced units.");
	hasattribs = TRUE;
    }
    if (u_facility(u)) {
	tbcatline_si(buf, "Considered to be an immobile facility.");
	hasattribs = TRUE;
    }
    if (u_ground_mobile(u)) {
	tbcatline_si(buf, "Considered to be a mobile ground unit.");
	hasattribs = TRUE;
    }
    if (u_naval_mobile(u)) {
	tbcatline_si(buf, "Considered to be a mobile naval unit.");
	hasattribs = TRUE;
    }
    if (u_air_mobile(u)) {
	tbcatline_si(buf, "Considered to be a mobile air unit.");
	hasattribs = TRUE;
    }
    if (u_minimal_sea_for_docks(u) < PROPHI) {
	tbprintf(buf, 
"%sRequires >= %d accessible, liquid cells to build naval units.%s",
		 help_indent(2), u_minimal_sea_for_docks(u), help_newline());
	hasattribs = TRUE;
    }
    if (u_ai_peace_garrison(u) > 0) {
	tbprintf(buf, "%sRequests a total peacetime garrison of %d units.%s", 
		 help_indent(2), u_ai_peace_garrison(u), help_newline());
	hasattribs = TRUE;
    }
    if (u_ai_war_garrison(u) > 0) {
	tbprintf(buf, "%sRequests a total wartime garrison of %d units.%s",
		 help_indent(2), u_ai_war_garrison(u), help_newline());
	hasattribs = TRUE;
    }
    if (u_ai_enemy_alert_range(u) > 0) {
	tbprintf(buf, 
		 "%sRange within which enemies will cause an alert is %d.%s",
		 help_indent(2), u_ai_enemy_alert_range(u), help_newline());
	hasattribs = TRUE;
    }
    if (u_ai_tactical_range(u) > 0) {
	tbprintf(buf, "%sBasic tactical computation range is %d.%s",
		 help_indent(2), u_ai_tactical_range(u), help_newline());
	hasattribs = TRUE;
    }
    if (!hasattribs)
      tbcatline_si(buf, "None.");
    tbcatline(buf, "");
}

int
may_detonate(int u)
{
    return ((type_max_acp(u) > 0 && u_acp_to_detonate(u) > 0)
	    || u_detonate_on_death(u) > 0
	    || uu_table_row_not_default(u, uu_detonate_on_hit, 0)
	    || uu_table_row_not_default(u, uu_detonate_on_capture, 0)
	    || uu_table_row_not_default(u, uu_detonate_approach_range, -1)
	    || ut_table_row_not_default(u, ut_detonation_accident, 0)
	    );
}

static void
dump_material_unit_summary(TextBuffer * buf,const char * category, 
                           int (*func)(int,int), int m, int no, int fire_f)
{
    int cnt = 0;
    int u;
    int p;

    tbprintf(buf,"%s: ",category);
    for_all_unit_types(u) {
	p = (*func)(u,m);
	if(fire_f && !(u_acp_to_fire(u) > 0))
            continue;
	if(p != no) {
	    cnt++;
	    tbprintf(buf,"%s:%d ",u_type_name(u),p);
	}
    }
    if(cnt == 0)
        tbprintf(buf,"(none)%s%s", help_newline(), help_newline());
    else
        tbprintf(buf,"%s%s", help_newline(), help_newline());
    return;
}

static void
describe_mtype(int m, const char *key, TextBuffer *buf)
{
    int u;
    int cnt;
    int o;
    int i;
    int s;

    append_help_phrase(buf, m_help(m));
    /* Display the designer's notes for this type. */
    if (m_notes(m) != lispnil) {
	tbprintf(buf, "%sNotes:%s", help_newline(), help_newline());
	append_notes(buf, m_notes(m));
	tbprintf(buf, "%s%s", help_newline(), help_newline());
    }
    if (m_people(m) > 0) {
	tbprintf(buf, "1 of this represents %d individuals.", m_people(m));
    }
    /* (should display unit columns here) */
    /* And here is a summary */
    tbprintf(buf, "%s%s", help_newline(), help_newline());
    dump_material_unit_summary(
        buf,"Base production", um_base_production, m, 0, 0);
    dump_material_unit_summary(
        buf,"Base consumption", um_base_consumption, m, 0, 0);
    dump_material_unit_summary(buf,"Storage", um_storage_x, m, 0, 0);
    dump_material_unit_summary(buf,"Consumption percentage as occupant", 
	    	               um_consumption_as_occupant, m, 100, 0);
    dump_material_unit_summary(buf,"Production as occupant", 
	    	               um_occ_production, m, -1, 0);
    dump_material_unit_summary(
        buf,"ACP to extract", um_acp_to_extract, m, 0, 0);
    tbprintf(
        buf, "%sMinimum required by a unit for the following activities.%s%s", 
	help_newline(), help_newline(), help_newline());
    dump_material_unit_summary(buf, "To act", um_to_act, m, 0, 0);
    dump_material_unit_summary(buf, "To move", um_to_move, m, 0, 0);
    dump_material_unit_summary(buf, "To attack", um_to_attack, m, 0, 0);
    dump_material_unit_summary(buf, "To fire", um_to_fire, m, 0, 1);
    tbprintf(buf, "%sIn addition to basic consumption.%s%s", help_newline(), 
	     help_newline(), help_newline(), help_newline());
    dump_material_unit_summary(buf, "Consumption per move", 
			       um_consumption_per_move, m, 0, 0);
    dump_material_unit_summary(buf, "Consumption per attack", 
			       um_consumption_per_attack, m, 0, 0);
    dump_material_unit_summary(buf, "Consumption per fire", 
			       um_consumption_per_fire, m, 0, 1);
    tbprintf(buf, "%sAny unit building another unit,%s", help_newline(), 
	     help_newline());
    tbcatline(buf, "needs (but does not necessarily use)");
    tbcatline(buf, "the following minimum amounts for the given unit.");
    dump_material_unit_summary(
        buf, "Needed to create ", um_to_create, m, 0, 0);
    dump_material_unit_summary(
        buf, "Needed to build ", um_to_build, m, 0, 0);
    tbprintf(buf, "%sAny building unit consumes as follows%s%s", 
	     help_newline(), help_newline(), help_newline());
    dump_material_unit_summary(buf,"On creation of", 
			       um_consumption_on_creation, m, 0, 0);
    dump_material_unit_summary(buf,"Per build of", 
			       um_consumption_per_build, m, 0, 0);
    tbprintf(
        buf,"%sAny unit repairing other units needs/uses these amounts%s%s", 
	help_newline(), help_newline(), help_newline());
    dump_material_unit_summary(
        buf, "To be able to repair ", um_to_repair, m, 0, 0);
    dump_material_unit_summary(buf, "Consumption per repair of 1 HP for", 
	                       um_consumption_per_repair, m, 0, 0);
    /* 
     * Select a minimal set of functions for the test,
     * after all, if it doesnt store any, it cant use the material.
     */
    tbcatline(
        buf, "Distances that a unit can send or receive this material.");
    tbcatline(buf, "Format is unit:send:receive.");
    tbcatline(buf, "(NB Distances are free of terrain effects)");
    for_all_unit_types(u) {
        /* (Should consider the case where a material is sent directly 
            to the side treasury.) */
	if (um_base_production(u,m) > 0 
	    || um_base_consumption(u,m) > 0
	    || um_storage_x(u,m) > 0) {
		o = um_outlength(u,m);
		i = um_inlength(u,m);
		if (o >= 0 && i >= 0)
	    	    tbprintf(buf, "%s:%d:%d ", u_type_name(u), o, i);
		else if (i >= 0)
	    	    tbprintf(buf, "%s:-:%d ", u_type_name(u), i);
	}
    }
    tbprintf(buf, "%s%s", help_newline(), help_newline());
    tbprintf(buf, "Initial unit quantities at game start:");
    cnt = 0;
    for_all_unit_types(u) {
	i = um_initial(u,m);
	s = um_storage_x(u,m);
	if( s > 0 && i >= 0 && i < s) {
	    tbprintf(buf, "%s:%d ", u_type_name(u), i);
	    cnt++;
	}
    }
    if (!cnt)
        tbprintf(buf,"(none)");
    tbprintf(buf,"%s%s", help_newline(), help_newline());
    return;
}

static void
describe_ttype(int t, const char *key, TextBuffer *buf)
{
    int m, ct;
    int u;
    int e;
    int speed;
    int acp;
    int mp;
    int l;
    int eff;

    append_help_phrase(buf, t_help(t));
    /* Display the subtype. */
    switch (t_subtype(t)) {
      case cellsubtype:
	break;
      case bordersubtype:
	tbcatline(buf, " (a border type)");
	break;
      case connectionsubtype:
	tbcatline(buf, " (a connection type)");
	break;
      case coatingsubtype:
	tbcatline(buf, " (a coating type)");
	break;
    }
    /* Display the designer's notes for this type. */
    if (t_notes(t) != lispnil) {
	tbprintf(buf, "%sNotes:%s", help_newline(), help_newline());
	append_notes(buf, t_notes(t));
	tbprintf(buf, "%s%s", help_newline(), help_newline());
    }
    if (t_liquid(t))
      tbcatline(buf, "Represents water or other liquid.");
    tbprintf(buf, "Generic unit capacity is %d.%s", t_capacity(t), 
	     help_newline());
    if (t_people_max(t) >= 0) {
	tbprintf(buf, "Up to %d people may live in this type of terrain.%s",
		 t_people_max(t), help_newline());
    }
    if (any_elev_variation) {
	if (t_elev_min(t) == t_elev_max(t)) {
	    tbprintf(buf, "Elevation is always %d.%s",
		    t_elev_min(t), help_newline());
	} else {
	    tbprintf(buf, "Elevations fall between %d and %d.%s",
		    t_elev_min(t), t_elev_max(t), help_newline());
	}
    }
    if (t_thickness(t) > 0) {
	tbprintf(buf, "Thickness is %d.%s", t_thickness(t), help_newline());
    }
    if (any_temp_variation) {
	if (t_temp_min(t) == t_temp_max(t)) {
	    tbprintf(buf, "Temperature is always %d.%s",
		     t_temp_min(t), help_newline());
	} else {
	    tbprintf(buf, 
                     "Temperatures fall between %d and %d, averaging %d.%s",
		     t_temp_min(t), t_temp_max(t), t_temp_avg(t), 
		     help_newline());
	}
	if (t_temp_variability(t) > 0) {
	    tbprintf(buf, "Temperature varies randomly, up to %d each turn.%s",
		     t_temp_variability(t), help_newline());
	}
    }
    if (any_wind_variation) {
	if (t_wind_force_min(t) == t_wind_force_max(t)) {
	    tbprintf(buf, "Wind force is always %d.%s",
		     t_wind_force_min(t), help_newline());
	} else {
	    tbprintf(buf, 
                     "Wind forces fall between %d and %d, averaging %d.%s",
		     t_wind_force_min(t), t_wind_force_max(t), 
                     t_wind_force_avg(t), help_newline());
	}
	if (t_wind_force_variability(t) > 0) {
	    tbprintf(buf, 
                     "%d%% chance each turn that wind force will change.%s",
		     t_wind_force_variability(t), help_newline());
	}
	if (t_wind_variability(t) > 0) {
	    tbprintf(buf, 
                     "%d%% chance each turn that wind direction will change.%s",
		     t_wind_variability(t), help_newline());
	}
    }
    if (any_clouds) {
	if (t_clouds_min(t) == t_clouds_max(t)) {
	    tbprintf(buf, "Cloud cover is always %d.%s",
		     t_clouds_min(t), help_newline());
	} else {
	    tbprintf(buf, "Cloud cover falls between %d and %d%s",
		     t_clouds_min(t), t_clouds_max(t), help_newline());
	}
    }
    /* Display relationships with materials. */
    if (nummtypes > 0) {
	for_all_material_types(m) {
	    if (tm_storage_x(t, m) > 0
		|| tm_production(t, m) > 0
		|| tm_consumption(t, m) > 0
		|| tm_prod_from_terrain(t, m) > 0) {
		tbprintf(buf, "%s:", m_type_name(m));
	    }
	    if (tm_storage_x(t, m) > 0) {
	    	tbprintf(buf, "%sCan store up to %d", help_indent(2), 
			 tm_storage_x(t, m));
	    	tbprintf(buf, " (normally starts game with %d)",
			 min(tm_initial(t, m), tm_storage_x(t, m)));
	    	tbcatline(buf, ".");
	    	tbprintf(buf, 
"%sSides will%s always know current amount accurately.%s",
			 help_indent(2), (tm_see_always(t, m) ? "" : " not"), 
			 help_newline());
	    }
	    if (tm_production(t, m) > 0 || tm_consumption(t, m) > 0) {
		tbprintf(buf, "%sProduces %d and consumes %d each turn.%s",
			 help_indent(2), tm_production(t, m), 
			 tm_consumption(t, m), help_newline());
	    }
	    if (tm_prod_from_terrain(t, m) > 0) {
		tbprintf(buf,
"%sAdvanced units can use this terrain to produce %d each turn.%s",
			 help_indent(2), tm_prod_from_terrain(t, m), 
			 help_newline());
	    }
	}
    }
    /* Display relationships with any coating terrain types. */
    if (numcoattypes > 0) {
	tbcatline(buf, "Coatings:");
    	for_all_terrain_types(ct) {
	    if (t_is_coating(ct)) {
		tbprintf(buf, "%s coats, depths %d up to %d",
			 t_type_name(ct),
			 tt_coat_min(ct, t), tt_coat_max(ct, t));
	    }
    	}
    }
    /* Display damaged types. */
    if (tt_table_row_not_default(t, tt_damaged_type, 0)) {
	tbcat_si(buf, "Type after being damaged: ");
	tt_table_row_desc(buf, t, tt_damaged_type, NULL);
	tbcatline(buf, ".");
    }
    /* Display exhaustion types. */
    if (nummtypes > 0) {
	for_all_material_types(m) {
	    if (tm_change_on_exhaust(t, m) > 0 
		&& tm_exhaust_type(t, m) != NONTTYPE) {
		tbprintf(buf, 
			 "If exhausted of %s, %d%% chance to change to %s.%s",
			 m_type_name(m), tm_change_on_exhaust(t, m),
			 t_type_name(tm_exhaust_type(t, m)), help_newline());
	    }
	}
    }
#ifdef DESIGNERS
    if (numdesigners > 0) {
	tbprintf(buf, "%sFOR DESIGNERS:%s", help_newline(), help_newline());
	if (t_subtype_x(t) == 
	    c_number(symbol_value(intern_symbol(keyword_name(K_RIVER_X))))) {
	    tbcatline(buf, "Considered to be a type of river.");
	} else if (t_subtype_x(t) == 
		   c_number(
		     symbol_value(intern_symbol(keyword_name(K_ROAD_X))))) {
	    tbcatline(buf, "Considered to be a type of road.");
	}
	if (tt_table_row_not_default(t, tt_drawable, 1)) {
	    tbcat(buf, "Draw over terrain: ");
	    tt_table_row_desc(buf, t, tt_drawable, NULL);
	    tbcatline(buf, ".");
	}
    }
#endif /* DESIGNERS */
    tbprintf(buf, "%s%sUnits able to enter, with given MP cost.%s", 
	     help_newline(), help_newline(), help_newline());
    for_all_unit_types(u) {
	e = ut_mp_to_enter(u,t);
	speed = type_max_speed(u);
	acp = type_max_acp(u);
	mp = (acp*speed)/100;
	if (speed == 0)
            continue;
	if (e > mp)
            continue;
	tbprintf(buf, "%s:%d ", u_type_name(u), e);
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());
    tbcatline(buf, "Units able to leave, with given MP cost.");
    for_all_unit_types(u) {
	l = ut_mp_to_leave(u,t);
	speed = type_max_speed(u);
	acp = type_max_acp(u);
	mp = (acp*speed)/100;
	if (speed == 0)
            continue;
	if (l <= mp)
            tbprintf(buf, "%s:%d ", u_type_name(u), l);
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());
    tbcatline(buf, "An attacker in this terrain, attacking the given units,");
    tbcatline(buf, "has the following multiplier, ( > 1 = advantage).");
    for_all_unit_types(u) {
	eff = ut_attack_terrain_effect(u,t);
	if (eff == 100)
	  continue;
	tbprintf(buf, "%s:%s%d.%d ", u_type_name(u), "*", eff / 100, eff % 100);
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());

    tbcatline(buf, "A defender in this terrain,");
    tbcatline(buf, "has the following multiplier, ( < 1 = advantage).");
    for_all_unit_types(u) {
	eff = ut_defend_terrain_effect(u,t);
	if (eff == 100)
            continue;
	tbprintf(buf, "%s:%s%d.%d ", u_type_name(u), "*", eff / 100, eff % 100);
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());
    tbcatline(buf,"The following units vanish in this terrain.");
    for_all_unit_types(u) {
	if (ut_vanishes_on(u,t))
            tbprintf(buf, "%s ", u_type_name(u));
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());
    tbcatline(buf, "The following units wreck in this terrain.");
    for_all_unit_types(u) {
	if (ut_wrecks_on(u,t))
            tbprintf(buf, "%s ", u_type_name(u));
    }
    tbprintf(buf,"%s%s", help_newline(), help_newline());
}

static void
describe_atype(int a, const char *key, TextBuffer *buf)
{
    int	u, found = FALSE;
    
    append_help_phrase(buf, a_help(a));
    /* Display the designer's notes for this type. */
    if (a_notes(a) != lispnil) {
	tbprintf(buf, "%sNotes:%s", help_newline(), help_newline());
	append_notes(buf, a_notes(a));
	tbprintf(buf, "%s%s", help_newline(), help_newline());
    }
    tbprintf(buf, "Research points needed for discovery: %d.%s", a_rp(a), 
	     help_newline());
    tbcatline(buf, "");
    if (aa_table_row_not_default(a, aa_needed_to_research, 0)) {
	tbcat(buf, "Prerequisites: ");
	aa_table_row_desc(buf, a, aa_needed_to_research, tb_bool_desc);
	tbprintf(buf, ".%s%s", help_newline(), help_newline());
    } else {
	tbprintf(buf, "No prerequisites.%s%s", help_newline(), help_newline());
    }
    if (aa_table_column_not_default(a, aa_needed_to_research, 0)) {
	tbcat(buf, "Needed for: ");
	aa_table_column_desc(buf, a, aa_needed_to_research, tb_bool_desc);
	tbprintf(buf, ".%s%s", help_newline(), help_newline());
    } else {
	tbprintf(buf, "Not needed for any further advances.%s%s", 
		 help_newline(), help_newline());
    }
    for_all_unit_types(u) {
	if (ua_needed_to_build(u, a) > 0) {
		if (found) {
			tbprintf(buf, ", ");
		} else {
			tbcat(buf, "Enables: ");
		}
		tbprintf(buf, "%s", u_type_name(u));
		found = TRUE;
	}
    }
    if (found) {
	tbcatline(buf, ".");		
    } else	{
	tbcatline(buf, "Does not enable any new unit types.");
    }
}

static void
describe_scorekeepers(int arg, const char *key, TextBuffer *buf)
{
    int i = 1;
    Scorekeeper *sk;

    if (scorekeepers == NULL) {
	tbcat(buf, "No scores are being kept.");
    } else {
	for_all_scorekeepers(sk) {
	    if (numscorekeepers > 1) {
		tbprintf(buf, "%d.  ", i++);
	    }
	    if (symbolp(sk->body)
		&& match_keyword(sk->body, K_LAST_SIDE_WINS)) {
		tbcat(buf, "The last side left in the game wins.");
		/* (should mention point values also) */
	    }
	    else if (symbolp(sk->body)
		&& match_keyword(sk->body, K_LAST_ALLIANCE_WINS)) {
		tbcat(buf, "The last alliance left in the game wins.");
		/* (should mention point values also) */
	    } else {
		tbcat(buf, "(an indescribably complicated scorekeeper)");
	    }
	    tbcatline(buf, "");
	}
    }
}

/* List each synthesis method and its parameters. */

static void
describe_setup(int arg, const char *key, TextBuffer *buf)
{
    int u, t, t2, methkey;
    Obj *synthlist, *methods, *method;
    int firstitem = TRUE;
    
    synthlist = g_synth_methods();
    if (synthlist == lispnil)
      tbcatline(buf, "No synthesis done when setting up this game.");
    else
      tbcatline(buf, "Synthesis done when setting up this game:");
    for (methods = synthlist; methods != lispnil; methods = cdr(methods)) {
	tbcatline(buf, "");
	method = car(methods);
	if (symbolp(method)) {
	    methkey = keyword_code(c_string(method));
	    switch (methkey) {
	      case K_MAKE_COUNTRIES:
		tbcat(buf, "Countries:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		if (g_radius_min() != gvar_i_default(g_radius_min)) {
		    tbprintf(buf, "%s%d cells across", help_indent(2), 
			     2 * g_radius_min() + 1);
		    if (g_separation_min() != gvar_i_default(g_separation_min))
		      tbprintf(buf, ", at least %d cells apart", 
			       g_separation_min());
		    if (g_separation_max() != gvar_i_default(g_separation_max))
		      tbprintf(buf, ", and at most %d cells apart",
			       g_separation_max());
		    tbcatline(buf, "");
		}
		if (t_property_not_default(t_country_min, 
					   tprop_i_default(t_country_min))) {
		    tbprintf(buf, "%sMinimum terrain in each country: ", 
			     help_indent(2));
		    t_property_desc(buf, t_country_min, NULL);
		    tbcatline(buf, ".");
		}
		if (t_property_not_default(t_country_max, 
					   tprop_i_default(t_country_max))) {
		    tbprintf(buf, "%sMaximum terrain in each country: ",
			     help_indent(2));
		    t_property_desc(buf, t_country_max, NULL);
		    tbcatline(buf, ".");
		}
		if (u_property_not_default(u_start_with, 
					   uprop_i_default(u_start_with))) {
		    tbprintf(buf, "%sStart with: ", help_indent(2));
		    u_property_desc(buf, u_start_with, NULL);
		    tbcatline(buf, ".");
		}
		if (u_property_not_default(u_indep_near_start, 
					   uprop_i_default(
					     u_indep_near_start))) {
		    tbprintf(buf, "%sIndependents nearby: ", help_indent(2));
		    u_property_desc(buf, u_indep_near_start, NULL);
		    tbcatline(buf, ".");
		}
		tbprintf(buf, "%sFavored terrain:%s", help_indent(2), 
			 help_newline());
		for_all_unit_types(u) {
		    if (u_start_with(u) > 0 || u_indep_near_start(u)) {
			tbprintf(buf, "%s%s: ", help_indent(4), u_type_name(u));
			ut_table_row_desc(buf, u, ut_favored, tb_percent_desc, 
					  NULL);
			tbcatline(buf, ".");
		    }
		}
		if (g_radius_max() != 0) {
		    tbprintf(buf, "%sCountry growth:%s", help_indent(2), 
			     help_newline());
		    if (g_radius_max() == -1) {
			tbprintf(buf, "%sUp to entire world", help_indent(4));
		    } else {
			tbprintf(buf, "%sUp to %d cells across",
				 help_indent(4), 2 * g_radius_max() + 1);
		    }
		    tbprintf(buf, ", %d chance to stop if blocked.%s",
			     g_growth_stop(), help_newline());
		    if (t_property_not_default(t_country_growth, 
					       tprop_i_default(
						 t_country_growth))) {
			tbprintf(buf, "%sGrowth chance, by terrain: ",
				 help_indent(2));
			t_property_desc(buf, t_country_max, tb_percent_desc);
			tbcatline(buf, ".");
		    }
		    if (t_property_not_default(t_country_takeover, 
					       tprop_i_default(
						 t_country_takeover))) {
			tbprintf(buf, "%sTakeover chance, by terrain: ", 
				 help_indent(2));
			t_property_desc(buf, t_country_takeover, NULL);
			tbcatline(buf, ".");
		    }
		    if (u_property_not_default(u_unit_growth, 
					       uprop_i_default(
						 u_unit_growth))) {
			tbprintf(buf, "%sChance for additional unit: ", 
				 help_indent(2));
			u_property_desc(buf, u_unit_growth, NULL);
			tbcatline(buf, ".");
		    }
		    if (u_property_not_default(u_indep_growth, 
					       uprop_i_default(
						 u_indep_growth))) {
			tbprintf(buf, 
				 "%sChance for additional independent unit: ", 
				 help_indent(2));
			u_property_desc(buf, u_indep_growth, NULL);
			tbcatline(buf, ".");
		    }
		    if (u_property_not_default(u_unit_takeover, 
					       uprop_i_default(
						 u_unit_takeover))) {
			tbprintf(buf, "%sChance to take over units: ",
				 help_indent(2));
			u_property_desc(buf, u_unit_takeover, NULL);
			tbcatline(buf, ".");
		    }
		    if (u_property_not_default(u_indep_takeover, 
					       uprop_i_default(
						 u_indep_takeover))) {
			tbprintf(buf, 
				 "%sChance to take over independent unit: ", 
				 help_indent(2));
			u_property_desc(buf, u_indep_takeover, NULL);
			tbcatline(buf, ".");
		    }
		    if (u_property_not_default(u_country_units_max, 
					       uprop_i_default(
						 u_country_units_max))) {
			tbprintf(buf, "%sMaximum units in country: ", 
				 help_indent(2));
			u_property_desc(buf, u_country_units_max, NULL);
			tbcatline(buf, ".");
		    }
		    if (t_property_not_default(t_country_people, 
					       tprop_i_default(
						 t_country_people))) {
			tbprintf(buf, "%sPeople takeover chance, by terrain: ",
				 help_indent(2));
			t_property_desc(buf, t_country_people, NULL);
			tbcatline(buf, ".");
		    }
		    if (t_property_not_default(t_indep_people, 
					       tprop_i_default(
						 t_indep_people))) {
			tbprintf(buf, "%sIndependent people chance: ",
				 help_indent(2));
			t_property_desc(buf, t_indep_people, NULL);
			tbcatline(buf, ".");
		    }
		}
		break;
	      case K_MAKE_EARTHLIKE_TERRAIN:
		tbcat(buf, "Earthlike terrain:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		tbprintf(buf, "%sTerrain around edge is %s.%s",
			 help_indent(2), t_type_name(g_edge_terrain()), 
			 help_newline());
		/* (should describe tt_adj_terr_effect workings here) */
		break;
	      case K_MAKE_FRACTAL_PTILE_TERRAIN:
		tbcat(buf, "Fractal percentile terrain:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		tbprintf(buf, "%sAlt blobs density %d, size %d, height %d%s",
			 help_indent(2), g_alt_blob_density(), 
			 g_alt_blob_size(), g_alt_blob_height(), 
			 help_newline());
		tbprintf(buf, "%s%d smoothing passes%s", help_indent(4), 
			 g_alt_smoothing(), help_newline());
		tbprintf(buf, "%sLower percentiles: ", help_indent(4));
		t_property_desc(buf, t_alt_min, tb_percent_desc);
		tbcatline(buf, ".");
		tbprintf(buf, "%sUpper percentiles: ", help_indent(4));
		t_property_desc(buf, t_alt_max, tb_percent_desc);
		tbcatline(buf, ".");
		tbprintf(buf, "%sWet blobs density %d, size %d, height %d%s",
			 help_indent(2), g_wet_blob_density(), 
			 g_wet_blob_size(), g_wet_blob_height(), 
			 help_newline());
		tbprintf(buf, "%s%d smoothing passes%s", help_indent(4), 
			 g_wet_smoothing(), help_newline());
		tbprintf(buf, "%sLower percentiles: ", help_indent(4));
		t_property_desc(buf, t_wet_min, tb_percent_desc);
		tbcatline(buf, ".");
		tbprintf(buf, "%sUpper percentiles: ", help_indent(4));
		t_property_desc(buf, t_wet_max, tb_percent_desc);
		tbcatline(buf, ".");
		tbprintf(buf, "%sTerrain around edge is %s.%s",
			 help_indent(2), t_type_name(g_edge_terrain()), 
			 help_newline());
		/* (should describe tt_adj_terr_effect workings here) */
		break;
	      case K_MAKE_INDEPENDENT_UNITS:
		tbcat(buf, "Independent units:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		for_all_unit_types(u) {
		    if (ut_table_row_not_default(u, ut_indep_density, 
						 table_default(
						   ut_indep_density))) {
			tbprintf(buf, "%sChance of independent %s: ",
				 help_indent(2), u_type_name(u));
			ut_table_row_desc(buf, u, ut_indep_density,
					  tb_percent_100th_desc, "in");
			tbcatline(buf, ".");
		    }
		}
		/* (should show indep people) */
		break;
	      case K_MAKE_INITIAL_MATERIALS:
		tbcat(buf, "Materials:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		/* (should show unit and terrain initial supply) */
		break;
	      case K_MAKE_MAZE_TERRAIN:
		tbcat(buf, "Maze terrain:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		tbprintf(buf, "%s%d.%2.2d%% of maze is room.%s",
			 help_indent(2), g_maze_room() / 100, 
			 g_maze_room() % 100, help_newline());
		tbcat(buf, "Room terrain types will be");
		for_all_terrain_types(t) {
		    if (t_maze_room_occurrence(t) > 0) {
			tbprintf(buf, " %s(%d)", t_type_name(t),
				 t_maze_room_occurrence(t));
		    }
		}
		tbcatline(buf, ".");
		tbprintf(buf, "%s%d.%2.2d%% of maze is passageway.%s",
			 help_indent(2), g_maze_passage() / 100, 
			 g_maze_passage() % 100, help_newline());
		tbcat(buf, "Passageway terrain types will be");
		for_all_terrain_types(t) {
		    if (t_maze_passage_occurrence(t) > 0) {
			tbprintf(buf, " %s(%d)", t_type_name(t),
				 t_maze_passage_occurrence(t));
		    }
		}
		tbcatline(buf, ".");
		tbprintf(buf, "%sTerrain around edge is %s.%s",
			 help_indent(2), t_type_name(g_edge_terrain()), 
			 help_newline());
		break;
	      case K_MAKE_RANDOM_DATE:
		tbcat(buf, "Random date:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		break;
	      case K_MAKE_RANDOM_TERRAIN:
		tbcat(buf, "Random terrain:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		tbprintf(buf, "%sTerrain types will be", help_indent(2));
		for_all_terrain_types(t) {
		    if (t_occurrence(t) > 0) {
			tbprintf(buf, " %s(%d)", t_type_name(t),
				 t_occurrence(t));
		    }
		}
		tbcatline(buf, ".");
		tbprintf(buf, "%sTerrain around edge is %s.%s",
			 help_indent(2), t_type_name(g_edge_terrain()), 
			 help_newline());
		break;
	      case K_MAKE_RIVERS:
		tbcat(buf, "Rivers:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		if (t_property_not_default(t_river_chance, 
					   tprop_i_default(t_river_chance))) {
		    tbprintf(buf, "%sChance to be river source: ", 
			     help_indent(2));
		    t_property_desc(buf, t_river_chance,
				    tb_percent_100th_desc);
		    tbcatline(buf, ".");
		    if (g_river_sink_terrain() != NONTTYPE)
		      tbprintf(buf, "%sSink is %s.%s",
			       help_indent(2), 
			       t_type_name(g_river_sink_terrain()), 
			       help_newline());
		    else
		      tbprintf(buf, "%sNo special sink terrain.%s", 
			       help_indent(2), help_newline());
		}
		for_all_terrain_types(t) {
		    if (!t_is_border(t))
		      continue;
		    if (K_RIVER_X != t_subtype_x(t))
		      continue;
		    if (tt_table_row_not_default(t, tt_adj_terr_effect, 
						 table_default(
						   tt_adj_terr_effect))) {
			tbprintf(buf, "%sRiver of type %s incompatible with ",
			      help_indent(2), t_type_name(t));
			firstitem = TRUE;
			for_all_terrain_types(t2) {
			    if (tt_adj_terr_effect(t, t2) >= 0) {
				if (!firstitem)
				  tbprintf(buf, ", %s", t_type_name(t2));
				else {
				    tbprintf(buf, " %s", t_type_name(t2));
				    firstitem = FALSE;
				}
			    }
			}
			tbcatline(buf, ".");
		    }
		}
		break;
	      case K_MAKE_ROADS:
		tbcat(buf, "Roads:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		tbprintf(buf, "%sChance to run:%s", help_indent(2), 
			 help_newline());
		for_all_unit_types(u) {
		    if (uu_table_row_not_default(u, uu_road_chance, 0)) {
			tbprintf(buf, "%sFrom %s: ", help_indent(2), 
				 u_type_name(u));
			uu_table_row_desc(buf, u, uu_road_chance,
					  tb_percent_desc, "to");
			tbcatline(buf, "");
		    }
		    if (u_spur_chance(u) > 0) {
			tbprintf(buf, 
"%s%d%% chance of spur, if within %d of road.%s",
				 help_indent(4), u_spur_chance(u), 
				 u_spur_range(u), help_newline());
		    }
		    if (u_road_to_edge_chance(u) > 0) {
			tbprintf(buf, "%s%d%% chance of road to edge.%s",
				 help_indent(4), u_road_to_edge_chance(u), 
				 help_newline());
		    }
		}
		for_all_terrain_types(t) {
		    if (t_subtype(t) == cellsubtype
			&& tt_table_row_not_default(t, tt_road_into_chance, 
						    table_default(
						      tt_road_into_chance))) {
			tbprintf(buf, "%sRouting of road from %s: ",
				 help_indent(2), t_type_name(t));
			/* Note: this is actually a weight, not a
			   percent chance. */
			tt_table_row_desc(buf, t, tt_road_into_chance,
					  tb_value_desc /*, "to" */);
			tbcatline(buf, ".");
		    }
		}
		if (g_edge_road_density() > 0) {
		    tbcat(buf, help_indent(2));
		    tb_percent_100th_desc(buf, g_edge_road_density());
		    tbprintf(buf, "%sof edge gets road run to another edge.%s",
			     help_indent(2), help_newline());
		}
		break;
	      case K_MAKE_WEATHER:
		tbcat(buf, "Weather:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		break;
	      case K_NAME_GEOGRAPHICAL_FEATURES:
		tbcat(buf, "Names for geographical features:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		break;
	      case K_NAME_UNITS_RANDOMLY:
		tbcat(buf, "Names for units:");
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		break;
	      default:
		tbprintf(buf, "%s:", c_string(method));
		describe_synth_run(buf, methkey);
		tbcatline(buf, "");
		break;
	    }
	} else if (consp(method)) {
	    /* (what?) */
	}
    }
    tbcatline(buf, "");
    if (g_side_lib() != lispnil)
      tbprintf(buf, "%d choices in side library.%s", length(g_side_lib()),
	       help_newline());
}

static void
describe_synth_run(TextBuffer *buf, int methkey)
{
    int calls, runs;
  
    if (get_synth_method_uses(methkey, &calls, &runs)) {
	if (calls > 0) {
	    if (calls == 1 && runs == 1) {
		tbcat(buf, " (was run)");
	    } else if (calls == 1 && runs == 0) {
		tbcat(buf, " (was not run)");
	    } else {
		tbprintf(buf, " (was called %d times, was run %d times)",
			calls, runs);
	    }
	} else {
	    tbcat(buf, " (not attempted)");
	}
    } else {
	tbcat(buf, " (? ? ?)");
    }
}

static void
describe_world(int arg, const char *key, TextBuffer *buf)
{
    tbprintf(buf, "World circumference: %d.%s", world.circumference, 
	     help_newline());
    tbcatline(buf, "");
    tbprintf(buf, "Area in world: %d wide x %d high", area.width, area.height);
    if (area.width == world.circumference)
      tbprintf(buf, " (wraps completely around world).%s", help_indent(2));
    else
      tbprintf(buf, ".%s", help_indent(2));

    tbprintf(buf, "Latitude: %d.%sLongitude: %d.\n", area.latitude, 
	     help_indent(2), area.longitude);
    tbcatline(buf, "");
    if (elevations_defined()) {
	tbprintf(buf, "Elevations range from %d to %d, averaging %d.%s",
		 area.minelev, area.maxelev, area.avgelev, help_newline());
	tbprintf(buf, "Cells are %d elevation units across.%s", 
		 area.cellwidth, help_newline());
    }
    if (world.yearlength > 1) {
	tbprintf(buf, "Length of year: %d turns.%s", world.yearlength, 
		 help_newline());
    }
    if (world.daylength != 1) {
	tbprintf(buf, "Length of day: %d turns.%s", world.daylength, 
		 help_newline());
	tbprintf(buf, "Percentage daylight: %d%%.%s", world.daylight_fraction, 
		 help_newline());
	tbprintf(buf, "Percentage twilight: %d%%.%s",
		 world.twilight_fraction - world.daylight_fraction, 
		 help_newline());
    }
    if (area.temp_year != lispnil) {
	/* (should describe temperature year cycle here) */
    }
#ifdef DESIGNERS
    if (numdesigners > 0) {
	tbcatline(buf, "FOR DESIGNERS:");
	tbprintf(buf, "Area projection is %d.%s", area.projection, 
		 help_newline());
	tbprintf(buf, "Default contour line color is \"%s\".%s",
		 g_contour_color(), help_newline());
	tbprintf(buf, "Default country border color is \"%s\".%s",
		 g_country_border_color(), help_newline());
	tbprintf(buf, "Default frontline color is \"%s\".%s",
		 g_frontline_color(), help_newline());
	tbprintf(buf, "Default grid color is \"%s\".%s",
		 g_grid_color(), help_newline());
	tbprintf(buf, "Default meridian color is \"%s\".%s",
		 g_meridian_color(), help_newline());
	tbprintf(buf, "Default shoreline color is \"%s\".%s",
		 g_shoreline_color(), help_newline());
	tbprintf(buf, "Default unit name color is \"%s\".%s",
		 g_unit_name_color(), help_newline());
	tbprintf(buf, "Default unseen color is \"%s\".%s",
		 g_unseen_color(), help_newline());
    }
#endif /* DESIGNERS */
}

/* The following globals don't make sense for online help, but are listed
   here so that a cross-check of *.def and online help doesn't list them
   as undocumented: g_random_state g_run_serial_number g_turn. */

/* This describes a command (from cmd.def et al) in a way that all
   interfaces can use. */

void
describe_command (int ch, const char *name, const char *help, int onechar, TextBuffer *buf)
{
    if (onechar && ch != '\0') {
	if (ch < ' ' || ch > '~') { 
	    tbprintf(buf, "^%c  ", (ch ^ 0x40));
	} else if (ch == ' ') {
	    tbprintf(buf, "'%c' ", ch);
	} else {
	    tbprintf(buf, " %c  ", ch);
	}
    } else if (!onechar && ch == '\0') {
	tbcat(buf, "\"");
	tbcat(buf, name);
	tbcat(buf, "\"");
    } else
      return;
    tbcat(buf, " ");
    tbcat(buf, help);
    tbcatline(buf, "");
}

static int
u_property_not_default(int (*fn)(int i), int dflt)
{
    int u, val;

    for_all_unit_types(u) {
	val = (*fn)(u);
	if (val != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
t_property_not_default(int (*fn)(int i), int dflt)
{
    int t, val;

    for_all_terrain_types(t) {
	val = (*fn)(t);
	if (val != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
uu_table_row_not_default(int u, int (*fn)(int i, int j), int dflt)
{
    int u2, val2;

    for_all_unit_types(u2) {
	val2 = (*fn)(u, u2);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
ut_table_row_not_default(int u, int (*fn)(int i, int j), int dflt)
{
    int t, val2;

    for_all_terrain_types(t) {
	val2 = (*fn)(u, t);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
um_table_row_not_default(int u, int (*fn)(int i, int j), int dflt)
{
    int m, val2;

    for_all_material_types(m) {
	val2 = (*fn)(u, m);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
ua_table_row_not_default(int u, int (*fn)(int i, int j), int dflt)
{
    int a, val2;

    for_all_advance_types(a) {
	val2 = (*fn)(u, a);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
tt_table_row_not_default(int t1, int (*fn)(int i, int j), int dflt)
{
    int t2, val2;

    for_all_terrain_types(t2) {
	val2 = (*fn)(t1, t2);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

#if 0 /* not used currently */
static int
tm_table_row_not_default(int t, int (*fn)(int i, int j), int dflt)
{
    int m, val2;

    for_all_material_types(m) {
	val2 = (*fn)(t, m);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}
#endif

static int
aa_table_row_not_default(int a1, int (*fn)(int i, int j), int dflt)
{
    int a2, val2;

    for_all_advance_types(a2) {
	val2 = (*fn)(a1, a2);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
uu_table_column_not_default(int u, int (*fn)(int i, int j), int dflt)
{
    int u2, val2;

    for_all_unit_types(u2) {
	val2 = (*fn)(u2, u);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

static int
aa_table_column_not_default(int a1, int (*fn)(int i, int j), int dflt)
{
    int a2, val2;

    for_all_advance_types(a2) {
	val2 = (*fn)(a2, a1);
	if (val2 != dflt)
	  return TRUE;
    }
    return FALSE;
}

struct histo {
    int val, num;
};

/* This compare will sort histogram entries in *reverse* order
   (most common values first). */

static int
histogram_compare(CONST void *h1, CONST void *h2)
{
    if (((struct histo *) h2)->num != ((struct histo *) h1)->num) {
    	return ((struct histo *) h2)->num - ((struct histo *) h1)->num;
    } else {
    	return ((struct histo *) h2)->val - ((struct histo *) h1)->val;
    }
}

static struct histo *u_histogram;

static void
u_property_desc(TextBuffer *buf, int (*fn)(int i),
		void (*formatter)(TextBuffer *buf, int val))
{
    int val, u, val2, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (u_histogram == NULL)
      u_histogram =
	(struct histo *) xmalloc(numutypes * sizeof(struct histo));
    /* Compute a histogram of all the values for the given property. */
    numentries = 0;
    val = (*fn)(0);
    u_histogram[numentries].val = val;
    u_histogram[numentries].num = 1;
    ++numentries;
    for_all_unit_types(u) {
	val2 = (*fn)(u);
	if (val2 == val) {
	    ++(u_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == u_histogram[i].val) {
		    ++(u_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		u_histogram[numentries].val = val2;
		u_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbcat(buf, " for all unit types");
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(u_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (u_histogram[0].num * 2 >= numutypes) {
	(*formatter)(buf, u_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, u_histogram[i].val);
	tbcat(buf, " for ");
	first = TRUE;
	for_all_unit_types(u) {
	    if ((*fn)(u) == u_histogram[i].val) {
		if (!first)
		  /* For this and similar situations, we need a space
		     in addition to the comma, so interface elements
		     (such as the Mac's text display) can decide to
		     add line breaks. */
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, u_type_name(u));
	    }
	}
    }
}

static struct histo *t_histogram;

static void
t_property_desc(TextBuffer *buf, int (*fn)(int i),
		void (*formatter)(TextBuffer *buf, int val))
{
    int val, t, val2, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (t_histogram == NULL)
      t_histogram =
	(struct histo *) xmalloc(numttypes * sizeof(struct histo));
    /* Compute a histogram of all the values for the given property. */
    numentries = 0;
    val = (*fn)(0);
    t_histogram[numentries].val = val;
    t_histogram[numentries].num = 1;
    ++numentries;
    for_all_terrain_types(t) {
	val2 = (*fn)(t);
	if (val2 == val) {
	    ++(t_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == t_histogram[i].val) {
		    ++(t_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		t_histogram[numentries].val = val2;
		t_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbcat(buf, " for all terrain types");
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(t_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (t_histogram[0].num * 2 >= numttypes) {
	(*formatter)(buf, t_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, t_histogram[i].val);
	tbcat(buf, " for ");
	first = TRUE;
	for_all_terrain_types(t) {
	    if ((*fn)(t) == t_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, t_type_name(t));
	    }
	}
    }
}

/* Generate a textual description of a single unit's interaction with all other
   unit types wrt a given table. */

static void
uu_table_row_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val), const char *connect)
{
    uu_table_rowcol_desc(buf, u, fn, formatter, connect, 0);
}

static void
uu_table_column_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		     void (*formatter)(TextBuffer *buf, int val),
		     char *connect)
{
    uu_table_rowcol_desc(buf, u, fn, formatter, connect, 1);
}

static void
uu_table_rowcol_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		     void (*formatter)(TextBuffer *buf, int val),
		     const char *connect, int rowcol)
{
    int val, val2, u2, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (empty_string(connect))
      connect = "for";
    if (u_histogram == NULL)
      u_histogram =
	(struct histo *) xmalloc(numutypes * sizeof(struct histo));
    val = (rowcol ? (*fn)(0, u) : (*fn)(u, 0));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    u_histogram[numentries].val = val;
    u_histogram[numentries].num = 1;
    ++numentries;
    for_all_unit_types(u2) {
	val2 = (rowcol ? (*fn)(u2, u) : (*fn)(u, u2));
	if (val2 == val) {
	    ++(u_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == u_histogram[i].val) {
		    ++(u_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		u_histogram[numentries].val = val2;
		u_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbprintf(buf, " %s all unit types", connect);
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(u_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (u_histogram[0].num * 2 >= numutypes) {
	(*formatter)(buf, u_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, u_histogram[i].val);
	tbprintf(buf, " %s ", connect);
	first = TRUE;
	for_all_unit_types(u2) {
	    val2 = (rowcol ? (*fn)(u2, u) : (*fn)(u, u2));
	    if (val2 == u_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, u_type_name(u2));
	    }
	}
    }
}

/* Generate a textual description of a single unit's interaction with all
   terrain types wrt a given table. */

static void
ut_table_row_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val), const char *connect)
{
    int val = (*fn)(u, 0), val2, t, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (empty_string(connect))
      connect = "for";
    if (u_histogram == NULL)
      u_histogram =
	(struct histo *) xmalloc(numutypes * sizeof(struct histo));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    u_histogram[numentries].val = val;
    u_histogram[numentries].num = 1;
    ++numentries;
    for_all_terrain_types(t) {
	val2 = (*fn)(u, t);
	if (val2 == val) {
	    ++(u_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == u_histogram[i].val) {
		    ++(u_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		u_histogram[numentries].val = val2;
		u_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbprintf(buf, " %s all terrain types", connect);
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(u_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (u_histogram[0].num * 2 >= numttypes) {
	(*formatter)(buf, u_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, u_histogram[i].val);
	tbprintf(buf, " %s ", connect);
	first = TRUE;
	for_all_terrain_types(t) {
	    if ((*fn)(u, t) == u_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, t_type_name(t));
	    }
	}
    }
}

static void
um_table_row_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val))
{
    int val = (*fn)(u, 0), val2, m, constant = TRUE, found;
    int i, numentries, first;
    const char *connect = "vs";

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (u_histogram == NULL)
      u_histogram =
	(struct histo *) xmalloc(numutypes * sizeof(struct histo));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    u_histogram[numentries].val = val;
    u_histogram[numentries].num = 1;
    ++numentries;
    for_all_material_types(m) {
	val2 = (*fn)(u, m);
	if (val2 == val) {
	    ++(u_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == u_histogram[i].val) {
		    ++(u_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		u_histogram[numentries].val = val2;
		u_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbprintf(buf, " %s all material types", connect);
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(u_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (u_histogram[0].num * 2 >= nummtypes) {
	(*formatter)(buf, u_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, u_histogram[i].val);
	tbprintf(buf, " %s ", connect);
	first = TRUE;
	for_all_material_types(m) {
	    if ((*fn)(u, m) == u_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, m_type_name(m));
	    }
	}
    }
}

static void
ua_table_row_desc(TextBuffer *buf, int u, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val))
{
    int val = (*fn)(u, 0), val2, a, constant = TRUE, found;
    int i, numentries, first;
    const char *connect = "vs";

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (u_histogram == NULL)
      u_histogram =
	(struct histo *) xmalloc(numutypes * sizeof(struct histo));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    u_histogram[numentries].val = val;
    u_histogram[numentries].num = 1;
    ++numentries;
    for_all_advance_types(a) {
	val2 = (*fn)(u, a);
	if (val2 == val) {
	    ++(u_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == u_histogram[i].val) {
		    ++(u_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		u_histogram[numentries].val = val2;
		u_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbprintf(buf, " %s all advance types", connect);
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(u_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (u_histogram[0].num * 2 >= nummtypes) {
	(*formatter)(buf, u_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, u_histogram[i].val);
	tbprintf(buf, " %s ", connect);
	first = TRUE;
	for_all_advance_types(a) {
	    if ((*fn)(u, a) == u_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, a_type_name(a));
	    }
	}
    }
}

static void
tt_table_row_desc(TextBuffer *buf, int t0, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val))
{
    int val = (*fn)(t0, 0), val2, t, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (t_histogram == NULL)
      t_histogram =
	(struct histo *) xmalloc(numttypes * sizeof(struct histo));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    t_histogram[numentries].val = val;
    t_histogram[numentries].num = 1;
    ++numentries;
    for_all_terrain_types(t) {
	val2 = (*fn)(t0, t);
	if (val2 == val) {
	    ++(t_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == t_histogram[i].val) {
		    ++(t_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		t_histogram[numentries].val = val2;
		t_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbcat(buf, " for all terrain types");
    	return;
    }
    /* Not a constant row; sort the histogram and compose a description. */
    qsort(t_histogram, numentries, sizeof(struct histo), histogram_compare);
    /* Show a "by default" clause if at least half of the entries are all
       the same. */
    if (t_histogram[0].num * 2 >= numttypes) {
	(*formatter)(buf, t_histogram[0].val);
	tbcat(buf, " by default");
    	i = 1;
    } else {
    	i = 0;
    }
    for (; i < numentries; ++i) {
	if (i > 0)
	  tbcat(buf, ", ");
	(*formatter)(buf, t_histogram[i].val);
	tbcat(buf, " vs ");
	first = TRUE;
	for_all_terrain_types(t) {
	    if ((*fn)(t0, t) == t_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, t_type_name(t));
	    }
	}
    }
}

static void
aa_table_row_desc(TextBuffer *buf, int a0, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val))
{
    aa_table_rowcol_desc(buf, a0, fn, formatter, 0);
}

static void
aa_table_column_desc(TextBuffer *buf, int a1, int (*fn)(int i, int j),
		  void (*formatter)(TextBuffer *buf, int val))
{
    aa_table_rowcol_desc(buf, a1, fn, formatter, 1);
}

static struct histo *a_histogram;

static void
aa_table_rowcol_desc(TextBuffer *buf, int a1, int (*fn)(int i, int j),
		     void (*formatter)(TextBuffer *buf, int val), int rowcol)
{
    int val = (*fn)(0, a1), val2, a, constant = TRUE, found;
    int i, numentries, first;

    if (formatter == NULL)
      formatter = tb_value_desc;
    if (a_histogram == NULL)
      a_histogram =
	(struct histo *) xmalloc(numatypes * sizeof(struct histo));
    val = (rowcol ? (*fn)(0, a1) : (*fn)(a1, 0));
    /* Compute a histogram of all the values in the row of the table. */
    numentries = 0;
    a_histogram[numentries].val = val;
    a_histogram[numentries].num = 1;
    ++numentries;
    for_all_advance_types(a) {
	val2 = (rowcol ? (*fn)(a, a1) : (*fn)(a1, a));
	if (val2 == val) {
	    ++(a_histogram[0].num);
	} else {
	    constant = FALSE;
	    found = FALSE;
	    for (i = 1; i < numentries; ++i) {
		if (val2 == a_histogram[i].val) {
		    ++(a_histogram[i].num);
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		a_histogram[numentries].val = val2;
		a_histogram[numentries].num = 1;
		++numentries;
	    }
	}
    }
    /* The constant table/row case is easily disposed of. */
    if (constant) {
	(*formatter)(buf, val);
	tbcat(buf, " for all advance types");
    	return;
    }
    /* Not a constant column; sort the histogram and compose a description. */
    qsort(a_histogram, numentries, sizeof(struct histo), histogram_compare);
    for (i = 0; i < numentries; ++i) {
	/* Skip over advances that are not needed. */
    	if (a_histogram[i].val == 0)
    	    continue;
	first = TRUE;
	for_all_advance_types(a) {
	    val2 = (rowcol ? (*fn)(a, a1) : (*fn)(a1, a));
	    if (val2 == a_histogram[i].val) {
		if (!first)
		  tbcat(buf, ", ");
		else
		  first = FALSE;
		tbcat(buf, a_type_name(a));
	    }
	}
    }
}

#if 0 /* not currently used */
/* A simple table-printing utility. Blanks out default values so they don't
   clutter the table. */

static void
append_number(buf, value, dflt)
TextBuffer *buf;
int value, dflt;
{
    if (value != dflt) {
	tbprintf(buf, "%5d ", value);
    } else {
	tbprintf(buf, "%s", help_indent(6));
    }
}
#endif

static void
append_help_phrase(TextBuffer *buf, const char *phrase)
{
    if (empty_string(phrase))
      return;

    /* Extra new line makes display less cluttered. */
    tbcat(buf, "----- ");
    tbcat(buf, phrase);
    tbprintf(buf, " -----%s%s", help_newline(), help_newline());
}

static void
append_notes(TextBuffer *buf, Obj *notes)
{
    const char *notestr;
    Obj *rest;

    if (stringp(notes)) {
	notestr = c_string(notes);
	if (strlen(notestr) > 0) { 
	    tbcat(buf, notestr);
	    tbcat(buf, " ");
	} else {
	    tbcatline(buf, "");
	}
    } else if (consp(notes)) {
	for_all_list(notes, rest) {
	    append_notes(buf, car(rest));
	}
    } else {
	run_warning("notes not list or strings, ignoring");
    }
}

void
append_blurb_strings(char *buf, Obj *notes)
{
    const char *str;
    Obj *rest;

    if (stringp(notes)) {
	str = c_string(notes);
	if (strlen(str) > 0) { 
	    tnprintf(buf, BLURBSIZE, str);
	    tnprintf(buf, BLURBSIZE, " ");
	} else {
	    tnprintf(buf, BLURBSIZE, help_newline());
	}
    } else if (consp(notes)) {
	for_all_list(notes, rest) {
	    append_blurb_strings(buf, car(rest));
	}
    } else {
	run_warning("blurb not list or strings, ignoring");
    }
}

void
notify_instructions(void)
{
    Obj *instructions = mainmodule->instructions, *rest;

    if (instructions != lispnil) {
	if (stringp(instructions)) {
	    notify_all("%s", c_string(instructions));
	} else if (consp(instructions)) {
	    for (rest = instructions; rest != lispnil; rest = cdr(rest)) {
		if (stringp(car(rest))) {
		    notify_all("%s", c_string(car(rest)));
		} else {
		    /* (should probably warn about this case too) */
		}
	    }
	} else {
	    run_warning("Instructions are of wrong type");
	}
    } else {
	notify_all("(no instructions supplied)");
    }
}

/* Print the news file onto the console if there is anything to print. */

void
print_any_news(void)
{
    FILE *fp;

    fp = open_library_file(news_filename());
    if (fp != NULL) {
	printf("\n                              XCONQ NEWS\n\n");
	while (fgets(spbuf, BUFSIZE-1, fp) != NULL) {
	    fputs(spbuf, stdout);
	}
	/* Add another blank line, to separate from init printouts. */
	printf("\n");
	fclose(fp);
    }
}

/* Generate a readable description of the game (design) being played. */
/* This works by writing out appropriate help nodes, along with some
   indexing material.  This does *not* do interface-specific help,
   such as commands. */

void
print_game_description_to_file(FILE *fp)
{
    HelpNode *node;

    /* (need to work on which nodes to dump out) */
    for (node = first_help_node; node != first_help_node; node = node->next) {
	get_help_text(node);
	if (node->text != NULL) {
	    fprintf(fp, "\014\n%s\n", node->key);
	    fprintf(fp, "%s\n", node->text);
	}
    }
}

static void
tb_value_desc(TextBuffer *buf, int val)
{
    char charbuf[30];

    sprintf(charbuf, "%d", val);
    tbcat(buf, charbuf);
}

static void
tb_fraction_desc(TextBuffer *buf, int val)
{
    char charbuf[30];

    if (val % 100 == 0)
      sprintf(charbuf, "%d", val / 100);
    else
      sprintf(charbuf, "%d.%2.2d", val / 100, val % 100);
    tbcat(buf, charbuf);
}

static void
tb_percent_desc(TextBuffer *buf, int val)
{
    tb_value_desc(buf, val);
    tbcat(buf, "%");
}

static void
tb_percent_100th_desc(TextBuffer *buf, int val)
{
    tb_fraction_desc(buf, val);
    tbcat(buf, "%");
}

/* Print out a prob_fraction-style value */
static void
tb_probfraction_desc(TextBuffer *buf, int val)
{
    char charbuf[80];

    if (val % 100 == 0) {
	sprintf(charbuf, "%d", val / 100);
    } else if (val < 100) {
	sprintf(charbuf, "%u%% chance of 1", val);
    } else {
	sprintf(charbuf, "%u%% chance of %u (otherwise %u)", val % 100,
	    val / 100 + 1, val / 100);
    }
    tbcat(buf, charbuf);
}

static void
tb_dice_desc(TextBuffer *buf, int val)
{
    char charbuf[30];

    dice2_desc(charbuf, (DiceRep)val);
    tbcat(buf, charbuf);
}

static void
tb_mult_desc(TextBuffer *buf, int val)
{
    char charbuf[30];

    sprintf(charbuf, "*%d.%2.2d", val / 100, val % 100);
    tbcat(buf, charbuf);
}

static void
tb_bool_desc(TextBuffer *buf, int val)
{
    tbcat(buf, (char *)(val ? "true" : "false"));
}

void
tbprintf(TextBuffer *buf, const char *str, ...)
{
    va_list ap;
    char line[300];

    va_start(ap, str);
    vsnprintf(line, 300, str, ap);
    tbcat(buf, line);
    va_end(ap);
}

#undef bcopy
#define bcopy(a,b,c) memcpy(b,a,c)

void
tbcat(TextBuffer *buf, const char *str)
{
    obstack_grow(&(buf->ostack), str, strlen(str));
}

void
tbcat_si(TextBuffer *buf, const char *str)
{
    tbprintf(buf, "%s", help_indent(2));
    obstack_grow(&(buf->ostack), str, strlen(str));
}

void
tbcatline(TextBuffer *buf, const char *str)
{
    assert_error(buf, "Tried to concatenate a line to a NULL text buffer");
    assert_error(str, "Tried to concatenate a NULL string to a text buffer");
    tbcat(buf, str);
    tbcat(buf, help_newline());
}

void
tbcatline_si(TextBuffer *buf, const char *str)
{
    assert_error(buf, "Tried to concatenate a line to a NULL text buffer");
    assert_error(str, "Tried to concatenate a NULL string to a text buffer");
    tbprintf(buf, "%s%s%s", help_indent(2), str, help_newline());
}

/* Prepares a newline for the help system in the current output format. */
static const char *
help_newline(void)
{
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	return "\n";
      case HELP_OUTPUT_HTML:
	return "<BR>\n";
      default:
	return NULL;
    }
    return NULL;
}

/* Prepares an indent for the help system in the current output format. */
static const char *
help_indent(unsigned int spaces)
{
    int i = 0;
    int indentchars = 0;
    
    if (!spaces)
      return "";
    memset(tmpbuf, 0, BUFSIZE);
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	indentchars = spaces;
	break;
      case HELP_OUTPUT_HTML:
	indentchars = spaces * 12;
	break;
      default: break;
    }
    assert_warning_return(indentchars <= BUFSIZE, 
"Number of requested indentation chars exceeds buffer size",
			  "");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	memset(tmpbuf, ' ', spaces);
	break;
      case HELP_OUTPUT_HTML:
	for (i = 0; i < spaces; ++i)
	  strcat(tmpbuf, "&nbsp;&nbsp;");
	break;
      default: break;
    }
    return tmpbuf;
}

/* Prepares help file for a "Table of Contents" section. */

void
prep_help_cc_toc(void)
{
    const char *ccfilebname = "index";
    const char *ccfileext = NULL;
    char ccfilename [BUFSIZE];

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    ccfileext = get_help_file_extension();
    make_pathname(NULL, ccfilebname, ccfileext, ccfilename);
    help_output_toc_filep = prep_help_file(ccfilename);
    write_help_file_header(help_output_toc_filep, "Table of Contents");
    write_help_section_header(help_output_toc_filep, "Table of Contents");
    if (mainmodule && mainmodule->blurb && stringp(mainmodule->blurb))
      fprintf(help_output_toc_filep, 
	      "<CENTER><H5>&quot;%s&quot;</H5></CENTER><BR>\n", 
	      c_string(mainmodule->blurb));
    write_help_file_navbar(help_output_toc_filep, HELP_PAGE_TOC, -1);
}

/* Prepares help file for any standard section using the temp file buffer. */

static void
meta_prep_help_cc(HelpPage hpage, int hpageidx)
{
    char namebuf [BUFSIZE];
    char numbuf [BUFSIZE];
    char filebnamebuf [BUFSIZE];
    int indentlvl = 0;

    strncpy(filebnamebuf, hpagedefns[hpage].filebname, BUFSIZE);
    strncpy(namebuf, hpagedefns[hpage].name, BUFSIZE);
    if (-1 < hpageidx) {
	switch (hpage) {
	  case HELP_PAGE_UTYPE:
	    strncpy(namebuf, u_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_TTYPE:
	    strncpy(namebuf, t_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_MTYPE:
	    strncpy(namebuf, m_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_ATYPE:
	    strncpy(namebuf, a_type_name(hpageidx), BUFSIZE);
	    break;
	  default: break;
	}
	sprintf(numbuf, "%d", hpageidx);
	strcat(filebnamebuf, numbuf);
	indentlvl = 1;
    }
    prep_help_cc_any(filebnamebuf, namebuf);
    write_help_file_navbar(help_output_tmp_filep, hpage, hpageidx);
    write_help_toc_entry(filebnamebuf, namebuf, indentlvl);
}

/* Prepares help file for any section. */

void
prep_help_cc_any(char *ccfilebname, char *sectionname)
{
    const char *ccfileext = NULL;
    char ccfilename [BUFSIZE];

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfilebname, 
		 "Attempted to prepare an help file with a NULL name");
    assert_error(ccfilebname[0], 
		 "Attempted to prepare an help file with an empty name");
    assert_error(sectionname, 
		 "Attempted to prepare an help file with a NULL section name");
    ccfileext = get_help_file_extension();
    make_pathname(NULL, ccfilebname, ccfileext, ccfilename);
    help_output_tmp_filep = prep_help_file(ccfilename);
    write_help_file_header(help_output_tmp_filep, sectionname);
    write_help_section_header(help_output_tmp_filep, sectionname);
}

/* Generic core for prepping a new help system CC file. */

FILE *
prep_help_file(char *hfilename)
{
    FILE *ccfile = NULL;
    char *ccfilepath = NULL;
    
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return NULL;
    assert_error(hfilename, 
		 "Attempted to open a help file with a NULL name.");
    assert_error(hfilename[0],
		 "Attempted to open a help file with an empty name.");
    ccfilepath = (char *)xmalloc(strlen(help_output_dir) + 10 + 
				 strlen(hfilename));
    make_pathname(help_output_dir, hfilename, NULL, ccfilepath);
    ccfile = fopen(ccfilepath, "w");
    if (NULL == ccfile)
      run_error("Failed to open %s for writing.", ccfilepath);
    return ccfile;
}

/* Add TOC entry to TOC help file stream. */

#define HELP_PLAIN_TEXT_COLS_MAX	72
#define HELP_PLAIN_TEXT_TRUNC_COL	(HELP_PLAIN_TEXT_COLS_MAX - 1)
#define HELP_PLAIN_TEXT_IDX_TRUNC_COL	39

void
write_help_toc_entry(const char *hfilebname, const char *sectionname, int indentlvl)
{
    char ccfilename [BUFSIZE];
    const char *ccfileext = NULL;
    char ptextbuf [HELP_PLAIN_TEXT_COLS_MAX];
    int lookupwidth = 0, indexpos = 0;

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(sectionname, "Tried to index a NULL section name");
    /* Build up the help file name, if the basename is not NULL. */
    if (hfilebname) {
	ccfileext = get_help_file_extension();
	make_pathname("", hfilebname, ccfileext, ccfilename);
    }
    /* Write out the TOC entry line. */
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	/* Fill output buffer with guide dots for easier lookup, ......
	   and that cluttered, information-packed look. ;-) */
	memset(ptextbuf, 0, HELP_PLAIN_TEXT_COLS_MAX);
	/* Prepare section name with left justification, starting in the 
	   leftmost column. */
	snprintf(ptextbuf, HELP_PLAIN_TEXT_IDX_TRUNC_COL, "%s%s", 
		 help_indent(indentlvl*2), sectionname);
	indexpos = min(strlen(ptextbuf), HELP_PLAIN_TEXT_IDX_TRUNC_COL);
	memset(ptextbuf + indexpos, '.', HELP_PLAIN_TEXT_TRUNC_COL - indexpos);
	if (hfilebname) {
	    lookupwidth = strlen(ccfilename) + 2;
	    if (lookupwidth > (HELP_PLAIN_TEXT_COLS_MAX - 10)) 
	      run_error(
"File name is too large for table of contents help file");
	    /* Prepare file name with right justification, starting in the 
	       rightmost column. */
	    sprintf(ptextbuf + (HELP_PLAIN_TEXT_TRUNC_COL - lookupwidth), 
		    "..%s", ccfilename);
	}
	/* Output the prepared buffer. */
	fprintf(help_output_toc_filep, "%s\n", ptextbuf);
	break;
      case HELP_OUTPUT_HTML:
	if (hfilebname)
	    fprintf(help_output_toc_filep, "%s<A Href=\"%s\">%s</A><BR>\n",
		    help_indent(indentlvl*2), ccfilename, sectionname);
	else
	    fprintf(help_output_toc_filep, "%s%s<BR>\n", 
		    help_indent(indentlvl*2), sectionname);
	break;
      default: break;
    }
    fflush(help_output_toc_filep);
}

/* Flush help node to help file stream for any section. */

void
flush_help_cc_tmp(HelpNode *helpnode)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    flush_help_cc(help_output_tmp_filep, helpnode);
}

/* Generic core for flushing a completed help node to help system file. */

void
flush_help_cc(FILE *ccfile, HelpNode *helpnode)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfile, 
		 "Attempted to flush a help node to a NULL help file");
    assert_error(helpnode, 
		 "Attempted to flush a NULL help node to a help file");
    fprintf(ccfile, "%s", get_help_text(helpnode));
    if (helpnode->text) {
	free(helpnode->text);
	helpnode->text = NULL;
    }
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	fprintf(ccfile, "\n");
	break;
      case HELP_OUTPUT_HTML:
	fprintf(ccfile, "\n<BR>\n");
	break;
      default: break;
    }
    fflush(ccfile);
}

/* Finishes help file for "Table of Contents" section. */

void
finish_help_cc_toc(void)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    write_help_section_footer(help_output_toc_filep, "Table of Contents");
    write_help_file_navbar(help_output_toc_filep, HELP_PAGE_TOC, -1);
    write_help_file_footer(help_output_toc_filep, "Table of Contents");
    finish_help_file(help_output_toc_filep);
}

/* Finishes help file for any standard section using the temp file buffer. */

static void
meta_finish_help_cc(HelpPage hpage, int hpageidx, HelpNode *helpnode)
{
    char namebuf [BUFSIZE];
    char numbuf [BUFSIZE];
    char filebnamebuf [BUFSIZE];

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    strncpy(filebnamebuf, hpagedefns[hpage].filebname, BUFSIZE);
    strncpy(namebuf, hpagedefns[hpage].name, BUFSIZE);
    if (-1 < hpageidx) {
	switch (hpage) {
	  case HELP_PAGE_UTYPE:
	    strncpy(namebuf, u_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_TTYPE:
	    strncpy(namebuf, t_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_MTYPE:
	    strncpy(namebuf, m_type_name(hpageidx), BUFSIZE);
	    break;
	  case HELP_PAGE_ATYPE:
	    strncpy(namebuf, a_type_name(hpageidx), BUFSIZE);
	    break;
	  default: break;
	}
	sprintf(numbuf, "%d", hpageidx);
	strcat(filebnamebuf, numbuf);
    }
    flush_help_cc_tmp(helpnode);
    write_help_section_footer(help_output_tmp_filep, namebuf);
    write_help_file_navbar(help_output_tmp_filep, hpage, hpageidx);
    write_help_file_footer(help_output_tmp_filep, namebuf);
    finish_help_file(help_output_tmp_filep);
}

/* Generic core for finishing an help system file. */

void
finish_help_file(FILE *hfilep)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(hfilep, "Attempted to flush a NULL help file.");
    fflush(hfilep);
    fclose(hfilep);
}

/* Writes header for help file. */

void
write_help_file_header(FILE *hfile, const char *headerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(hfile, 
		 "Attempted to write an header to a NULL help file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	assert_error(headerdata, 
		     "Attempted to write an header with NULL header data");
	if (mainmodule)
	  fprintf(hfile, "%s: %s\n\n", mainmodule->name, headerdata);
	else
	  fprintf(hfile, "%s\n\n", headerdata);
	break;
      case HELP_OUTPUT_HTML:
	assert_error(headerdata, 
		     "Attempted to write an header with NULL header data");
	if (mainmodule)
	  fprintf(hfile, 
		  "<HTML>\n<HEAD>\n<TITLE>%s: %s</TITLE>\n</HEAD>\n<BODY>\n",
		  mainmodule->name, headerdata);
	else
	  fprintf(hfile, 
		  "<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n</HEAD>\n<BODY>\n",
		  headerdata);
	break;
      default: break;
    }
}

/* Writes footer for help file. */

void
write_help_file_footer(FILE *hfile, const char *footerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(hfile, 
		 "Attempted to write a footer to a NULL help file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	fprintf(hfile, "\n\n%s\n", help_file_brand());
	break;
      case HELP_OUTPUT_HTML:
	fprintf(hfile, "<BR>\n<H6>%s</H6>\n</BODY>\n</HTML>\n", 
		help_file_brand());
	break;
      default: break;
    }
}

/* Returns a branding mark for help file. */

char *
help_file_brand(void)
{
    /* TODO: Add date and time. */
    snprintf(tmpbuf, BUFSIZE - 1, 
	     "File produced by Xcscribe for Xconq version %s.",
	     version_string() /*, copyright_string() */);
    return tmpbuf;
}

/* Writes header for a help section. */

void
write_help_section_header(FILE *ccfile, const char *headerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfile, 
		 "Attempted to write an header to a NULL help CC file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT: break;
      case HELP_OUTPUT_HTML:
	assert_error(headerdata, 
		     "Attempted to write an header with NULL header data");
	fprintf(ccfile, "<CENTER><H1>%s</H1></CENTER><BR>\n", headerdata);
	break;
      default: break;
    }
}

/* Writes footer for help section. */

void
write_help_section_footer(FILE *ccfilep, const char *footerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfilep, 
		 "Attempted to write a footer to a NULL help file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT: case HELP_OUTPUT_HTML: break;
      default: break;
    }
}

/* Writes header for help subsection. */

void
write_help_subsection_header(FILE *ccfile, char *headerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfile, 
		 "Attempted to write an header to a NULL help CC file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	assert_error(headerdata, 
		     "Attempted to write an header with NULL header data");
	fprintf(ccfile, "%s:\n\n", headerdata);
	break;
      case HELP_OUTPUT_HTML:
	assert_error(headerdata, 
		     "Attempted to write an header with NULL header data");
	fprintf(ccfile, "<CENTER><H4>%s</H4></CENTER><BR>\n", headerdata);
	break;
      default: break;
    }
}

/* Writes footer for help subsection. */

void
write_help_subsection_footer(FILE *ccfile, char *footerdata)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfile, 
		 "Attempted to write a footer to a NULL help CC file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT: case HELP_OUTPUT_HTML: break;
      default: break;
    }
}

/* Writes a break between subsections. */

void
write_help_subsection_break(FILE *ccfile)
{
    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(ccfile, 
		 "Attempted to write a footer to a NULL help CC file");
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	fprintf(ccfile, "\n\n");
	break;
      case HELP_OUTPUT_HTML:
	fprintf(ccfile, "<BR><HR><BR>\n");
	break;
      default: break;
    }
}

/* Returns help file's extension. */

const char *
get_help_file_extension(void)
{
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	return "txt";
      case HELP_OUTPUT_HTML:
	return "html";
      default: break;
    }
    return "";
}

/* Initialize the help pages. */

static void
init_help_pages(void)
{
    char midxbname [BUFSIZE];

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    /* Fixup the master index file basename. */
    memset(midxbname, 0, BUFSIZE);
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
#if (defined(UNIX) || defined(WIN32))
	make_pathname("..", "index", "", midxbname);
#else
	make_pathname("::", "index", "", midxbname);
#endif
	break;
      case HELP_OUTPUT_HTML:
	strcpy(midxbname, "../index");
	break;
      default: break;
    }
    hpagedefns[HELP_PAGE_MASTER_INDEX].filebname = copy_string(midxbname);
    /* Fixup next and previous help pages involving variable-sized help page 
       clusters. */
    hpagedefns[HELP_PAGE_TTYPE].nextpage = 
	((nummtypes <= 0) ? ((numatypes <= 0) ? HELP_PAGE_LAST 
					      : HELP_PAGE_ATYPE)
			  : HELP_PAGE_MTYPE);
    hpagedefns[HELP_PAGE_MTYPE].nextpage = 
	((numatypes <= 0) ? HELP_PAGE_LAST : HELP_PAGE_ATYPE);
    hpagedefns[HELP_PAGE_ATYPE].prevpage = 
	((nummtypes <= 0) ? HELP_PAGE_TTYPE : HELP_PAGE_MTYPE);
    hpagedefns[HELP_PAGE_LAST].prevpage =
	((numatypes <= 0) ? ((nummtypes <= 0) ? HELP_PAGE_TTYPE
					      : HELP_PAGE_MTYPE)
			  : HELP_PAGE_ATYPE);
}

/* Write a navbar for a help page. */

void
write_help_file_navbar(FILE *ccfilep, HelpPage hpage, int hpageidx)
{
    const char *tocfilebname = "index";
    const char *prevfilebname = NULL;
    const char *nextfilebname = NULL;
    const char *tocname = "Table of Contents";
    const char *prevname = NULL;
    const char *nextname = NULL;
    const char *navfileext = NULL;
    int prevhpageidx = -1, nexthpageidx = -1;
    char prevnumbuf [BUFSIZE], nextnumbuf [BUFSIZE];

    if (HELP_OUTPUT_CC_NONE == help_output_cc)
      return;
    assert_error(HELP_PAGE_MASTER_INDEX != hpage, 
"Attempted to write a navigation bar for an invalid help page.");
    assert_error(ccfilep, "Attempted to write to a NULL help file.");
    /* Set up the nav links. */
    switch (hpage) {
      case HELP_PAGE_NONE: return;
      default:
	tocfilebname = hpagedefns[hpagedefns[hpage].tocpage].filebname;
	prevfilebname = hpagedefns[hpagedefns[hpage].prevpage].filebname;
	nextfilebname = hpagedefns[hpagedefns[hpage].nextpage].filebname;
	tocname = hpagedefns[hpagedefns[hpage].tocpage].name;
	prevname = hpagedefns[hpagedefns[hpage].prevpage].name;
	nextname = hpagedefns[hpagedefns[hpage].nextpage].name;
    }
    /* In the case of utypes, ttypes, mtypes, etc..., the links may need to 
       be edited. */
    switch (hpage) {
      case HELP_PAGE_WORLD:
	nexthpageidx = 0;
	break;
      case HELP_PAGE_UTYPE:
	if (hpageidx > 0) {
	    prevfilebname = hpagedefns[hpage].filebname;
	    prevname = hpagedefns[hpage].name;
	    prevhpageidx = hpageidx - 1;
	}
	if (hpageidx < (numutypes - 1)) {
	    nextfilebname = hpagedefns[hpage].filebname;
	    nextname = hpagedefns[hpage].name;
	    nexthpageidx = hpageidx + 1;
	}
	else
	  nexthpageidx = 0;
	break;
      case HELP_PAGE_TTYPE:
	if (hpageidx > 0) {
	    prevfilebname = hpagedefns[hpage].filebname;
	    prevname = hpagedefns[hpage].name;
	    prevhpageidx = hpageidx - 1;
	}
	if (hpageidx < (numttypes - 1)) {
	    nextfilebname = hpagedefns[hpage].filebname;
	    nextname = hpagedefns[hpage].name;
	    nexthpageidx = hpageidx + 1;
	}
	else
	  nexthpageidx = 0;
	break;
      case HELP_PAGE_MTYPE:
	if (hpageidx > 0) {
	    prevfilebname = hpagedefns[hpage].filebname;
	    prevname = hpagedefns[hpage].name;
	    prevhpageidx = hpageidx - 1;
	}
	if (hpageidx < (nummtypes - 1)) {
	    nextfilebname = hpagedefns[hpage].filebname;
	    nextname = hpagedefns[hpage].name;
	    nexthpageidx = hpageidx + 1;
	}
	else
	  nexthpageidx = 0;
	break;
      case HELP_PAGE_ATYPE:
	if (hpageidx > 0) {
	    prevfilebname = hpagedefns[hpage].filebname;
	    prevname = hpagedefns[hpage].name;
	    prevhpageidx = hpageidx - 1;
	}
	if (hpageidx < (numatypes - 1)) {
	    nextfilebname = hpagedefns[hpage].filebname;
	    nextname = hpagedefns[hpage].name;
	    nexthpageidx = hpageidx + 1;
	}
	else
	  nexthpageidx = 0;
	break;
      default: break;
    }
    /* In the case of utypes, ttypes, mtypes, etc..., even more editing may 
       need to be done, since mtypes and atypes may not necessarily be 
       defined for the game. */
    switch (hpage) {
      case HELP_PAGE_TTYPE:
	if (hpageidx == 0)
	  prevhpageidx = numutypes - 1;
	if (hpageidx >= (numttypes - 1))
	  nexthpageidx = ((nummtypes > 0) ? 0 : ((numatypes > 0) ? 0 : -1));
	break;
      case HELP_PAGE_MTYPE:
	if (hpageidx == 0)
	  prevhpageidx = numttypes - 1;
	if (hpageidx >= (nummtypes - 1))
	  nexthpageidx = ((numatypes > 0) ? 0 : -1);
	break;
      case HELP_PAGE_ATYPE:
	if (hpageidx == 0)
	  prevhpageidx = ((nummtypes > 0) ? nummtypes - 1 : numttypes - 1);
	if (hpageidx >= (numatypes - 1)) 
	  nexthpageidx = -1;
	break;
      case HELP_PAGE_LAST:
	prevhpageidx = ((numatypes > 0) ? numatypes - 1 
					: ((nummtypes > 0) ? nummtypes - 1
							   : numttypes - 1));
      default: break;
    }
    /* Get the numeric indices of the prev and next help files, if relevant. */
    /* <<The decimal point can be added to these buffers, if specifying a 
	 "%s" for an empty string causes problems.>> */
    memset(prevnumbuf, 0, BUFSIZE);
    memset(nextnumbuf, 0, BUFSIZE);
    if (-1 < prevhpageidx)
      sprintf(prevnumbuf, "%d", prevhpageidx);
    if (-1 < nexthpageidx)
      sprintf(nextnumbuf, "%d", nexthpageidx);
    /* Get the extension for all the help files. */
    navfileext = get_help_file_extension();
    /* Actually write the navbar. */
    fprintf(ccfilep, "%s", help_newline());
    switch (help_output_mode) {
      case HELP_OUTPUT_PLAIN_TEXT:
	if (tocfilebname && tocname)
	  fprintf(ccfilep, "%s: %s.%s%s", tocname, 
		  tocfilebname, navfileext, help_newline());
	if (prevfilebname && prevname)
	  fprintf(ccfilep, "Previous: %s%s.%s%s", prevfilebname, 
		  prevnumbuf, navfileext, help_newline());
	if (nextfilebname && nextname)
	  fprintf(ccfilep, "Next: %s%s.%s%s", nextfilebname, 
		  nextnumbuf, navfileext, help_newline());
	break;
      case HELP_OUTPUT_HTML:
	/* fprintf(ccfilep, "<CENTER>"); */
	if (tocfilebname && tocname)
	  fprintf(ccfilep, "Up: <A Href=\"%s.%s\">%s</A>%s", tocfilebname, 
		  navfileext, tocname, help_indent(2));
	if (prevfilebname && prevname)
	  fprintf(ccfilep, "Previous: <A Href=\"%s%s.%s\">%s</A>%s", 
		  prevfilebname, prevnumbuf, navfileext, prevname, 
		  help_indent(2));
	if (nextfilebname && nextname)
	  fprintf(ccfilep, "Next: <A Href=\"%s%s.%s\">%s</A>%s", nextfilebname,
		  nextnumbuf, navfileext, nextname, help_indent(2));
	/* fprintf(ccfilep, "</CENTER>%s", help_newline()); */
	fprintf(ccfilep, "%s", help_newline());
	break;
      default: break;
    }
    fprintf(ccfilep, "%s", help_newline());
    fflush(ccfilep);
}

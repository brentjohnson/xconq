/* Toplevel code for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "tkconq.h"
#include "cmdline.h"

void update_world_mouseover(Map *map, int rawx, int rawy);
void autoscroll(Map *map, VP *vp, int which, int rawx, int rawy);
void draw_unit_info(Map *map);

extern Tk_Window tmp_root_window;
extern int tmp_valid;

extern int ok_to_exit;

#ifndef MAC
/* Obstack allocation and deallocation routines. */
#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free
#undef bcopy
#define bcopy(a,b,c)    memcpy(b,a,c)
#endif

/* We only need one tcl interpreter, and this is it. */

Tcl_Interp *interp;

/* Tcl token for network timer handler. */
Tcl_TimerToken hndl_network_timer = NULL;

/* This is the number of pixels around the edge of the map that will
   result in autoscrolling if the cursor is positioned within that
   distance from the edge. */

int autoscroll_width = 40;

/* Delay in ms between the detection of being in the autoscroll
   and the start of actual scrolling. */

int autoscroll_delay = 200;

/* Storage for values of user preferences. */

VP default_vp;
int default_draw_terrain_images;
int default_draw_terrain_patterns;
int default_draw_transitions;
char *default_font_family;
int default_font_size;
char *default_joinhost;
char *default_joinport;
char *default_myname;
int listallgames;

int use_stdio;

/* This flag is true while an runtime error or warning dialog is up;
   it is used to suppress attempts to draw, because they may have been
   the cause of the dialog popping up. */
  
int error_popped_up;

/* True if any variants are available. */

int any_variants;

/* These are true when a standard variant is available to be chosen. */

int vary_world_size;
int vary_real_time;

/* Tcl widgets are numbered 0-29 for checkboxes, -1 for world size
   dialog, -2 for real time dialog.  These map to and from variant
   indexes, which are defined by the order in which they're listed
   in module->variants. */

#define MAXCHECKBOXES 30

int numcheckboxes = MAXCHECKBOXES;

/* Indexes into the array of the main module's variants. */

int checkboxpos[MAXCHECKBOXES];
int worldsizepos = -1;
int realtimepos = -1;

/* Reverse lookup for tcltk widgets -> variants. */

int varrev[100];

/* This flag is set if the resarch dialog is up. */

static int research_popped_up = FALSE;

/* True when a won or lost dialog has been put up on the screen already. */

static int told_outcome = FALSE;

/* This is set when the user has requested exit from the program, used
   when unwinding the game state while getting out. */

static int want_to_exit = FALSE;

/* Declarations of all the tcl commands defined in C code. */

typedef int TclCmdFn(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[]);

TclCmdFn tk_version_string;
TclCmdFn tk_copyright_string;
TclCmdFn tk_game_homedir;
TclCmdFn tk_run_game;
TclCmdFn tk_run_game_idle;
TclCmdFn tk_animate_selection;
TclCmdFn tk_interp_key;
TclCmdFn tk_execute_long_command;
TclCmdFn tk_library_paths;
TclCmdFn tk_numgames;
TclCmdFn tk_any_variants;
TclCmdFn tk_listallgames;
TclCmdFn tk_side_lib_size;
TclCmdFn tk_side_lib_entry;
TclCmdFn tk_side_lib_entry_available;
TclCmdFn tk_interpret_variants;
TclCmdFn tk_set_variant_value;
TclCmdFn tk_numttypes;
TclCmdFn tk_numutypes;
TclCmdFn tk_nummtypes;
TclCmdFn tk_numatypes;
TclCmdFn tk_numsides;
TclCmdFn tk_maxsides;
TclCmdFn tk_minsides;
TclCmdFn tk_dside;
TclCmdFn tk_numtreasury;
TclCmdFn tk_numfeatures;
TclCmdFn tk_numscorekeepers;
TclCmdFn tk_curmap_number;
TclCmdFn tk_game_info;
TclCmdFn tk_atype_name;
TclCmdFn tk_ttype_name;
TclCmdFn tk_t_image_name;
TclCmdFn tk_utype_name;
TclCmdFn tk_u_image_name;
TclCmdFn tk_mtype_name;
TclCmdFn tk_m_image_name;
TclCmdFn tk_side_name;
TclCmdFn tk_side_adjective;
TclCmdFn tk_side_emblem;
TclCmdFn tk_short_side_title;
TclCmdFn tk_side_ingame;
TclCmdFn tk_simple_player_title;
TclCmdFn tk_simple_player_name;
TclCmdFn tk_player_advantage;
TclCmdFn tk_player_aitypename;
TclCmdFn tk_player_displayname;
TclCmdFn tk_min_advantage;
TclCmdFn tk_max_advantage;
TclCmdFn tk_assigned_side;
TclCmdFn tk_assigned_player;
TclCmdFn tk_can_rename;
TclCmdFn tk_adjust_advantage;
TclCmdFn tk_add_side_and_player;
TclCmdFn tk_remove_side_and_player;
TclCmdFn tk_rename_side_for_player;
TclCmdFn tk_set_ai_for_player;
TclCmdFn tk_exchange_players;
TclCmdFn tk_set_indepside_options;
TclCmdFn tk_feature_desc;
TclCmdFn tk_feature_info;
TclCmdFn tk_set_feature_info;
TclCmdFn tk_start_new_game;
TclCmdFn tk_start_saved_game;
TclCmdFn tk_start_player_pre_setup_stage;
TclCmdFn tk_start_game_load_stage;
TclCmdFn tk_start_player_setup_stage;
TclCmdFn tk_launch_game;
TclCmdFn tk_try_host_game;
TclCmdFn tk_try_join_game;
TclCmdFn tk_send_chat;
TclCmdFn tk_set_unit_type;
TclCmdFn tk_mouse_down_cmd;
TclCmdFn tk_mouse_up_cmd;
TclCmdFn tk_mouse_over_cmd;
TclCmdFn tk_world_mouse_down_cmd;
TclCmdFn tk_world_mouse_up_cmd;
TclCmdFn tk_world_mouse_over_cmd;
TclCmdFn tk_save_prefs;
TclCmdFn tk_init_help_list;
TclCmdFn tk_init_help_menu;
TclCmdFn tk_help_goto;
TclCmdFn tk_game_save;
TclCmdFn tk_make_default_player_spec;
TclCmdFn tk_set_design_tool;
TclCmdFn tk_set_design_data;
TclCmdFn tk_create_new_feature;
TclCmdFn tk_destroy_feature;
TclCmdFn tk_designer_fix;
TclCmdFn tk_designer_save;

TclCmdFn tk_numutypes_available;
TclCmdFn tk_utype_actual;
TclCmdFn tk_mtype_actual;
TclCmdFn tk_map_size_at_power;

TclCmdFn tk_center_on_unit;

TclCmdFn tk_advance_needed_to_research;
TclCmdFn tk_set_research_popped_up;
TclCmdFn tk_current_advance;
TclCmdFn tk_available_advance;
TclCmdFn tk_set_side_research;
#if 0
TclCmdFn tk_agreements;
#endif
TclCmdFn tk_get_scores;

TclCmdFn tk_reset_popup_flag;

TclCmdFn tk_set_want_to_exit;
TclCmdFn tk_exit_xconq;

TclCmdFn mapw_cmd;
TclCmdFn imfsample_cmd;

TclCmdFn tk_withdraw_window;

void update_mouseover(Map *map, int rawx, int rawy);

/* Declarations of local functions. */

static void update_side_progress_display(Side *side, Side *side2);
static void update_side_score_display(Side *side, Side *side2);

static void update_material_display(void);

static void help_unit_type(Side *side, Map *map);
static void help_terrain_type(Side *side, Map *map);

static void init_unit_type_list(int u);
static void update_unit_type_list(int u);

static void ui_update_state(void);

static void update_variant_setting(int which);
static void update_assignment(int n);

void check_network(ClientData cldata);

static void init_all_displays(void);

static int withdraw_window(const char *, Tcl_Interp *);

/* Create the one global interpreter, add Xconq-specific commands to it. */

static void tcl_cmd(char *name, TclCmdFn tcmd);

static void
tcl_cmd(char *name, TclCmdFn tcmd)
{
    Tcl_CreateCommand(interp, name, tcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
}

char *
c_to_tcl_string(char **cstr)
{
    int clen = 0, i = 0, tcli = 0;
    char *tclstr = NULL;

    if (cstr) {
        clen = strlen(*cstr);
        tclstr = (char *)xmalloc(clen*5+1);
        for (i = 0; i <= clen; ++i) {
            if ('\\' == cstr[0][i]){
                strcpy(tclstr+i, "\\\\");
                tcli += 4;
            }
            /* Don't ask, because you don't really want to know.... */
            else if ('[' == cstr[0][i]) {
                strcpy(tclstr+i, "\\133");
                tcli += 5;
            }
            else if ('"' == cstr[0][i]) {
                strcpy(tclstr+i, "\\042");
                tcli += 5;
            }
            else 
              tclstr[tcli++] = cstr[0][i];
        }
        free(*cstr);
        *cstr = tclstr;
    }
    return *cstr;
}

void
initial_ui_init(void)
{
    char pathbuf[PATH_SIZE];
    int rslt;

    Tk_Window tkwin;

    Tcl_FindExecutable("xconq");

    interp = Tcl_CreateInterp();

    if (Tcl_Init(interp) == TCL_ERROR) {
	init_error("tcl init failed (%s), exiting", interp->result);
    }

    if (Tk_Init(interp) == TCL_ERROR) {
	init_error("tk init failed (%s), exiting", interp->result);
    }

    tcl_cmd("version_string", tk_version_string);
    tcl_cmd("copyright_string", tk_copyright_string);
    tcl_cmd("game_homedir", tk_game_homedir);
    tcl_cmd("run_game", tk_run_game);
    tcl_cmd("run_game_idle", tk_run_game_idle);
    tcl_cmd("animate_selection", tk_animate_selection);
    tcl_cmd("interp_key", tk_interp_key);
    tcl_cmd("execute_long_command", tk_execute_long_command);
    tcl_cmd("xconq_library_paths", tk_library_paths);
    tcl_cmd("numgames", tk_numgames);
    tcl_cmd("any_variants", tk_any_variants);
    tcl_cmd("listallgames", tk_listallgames);
    tcl_cmd("side_lib_size", tk_side_lib_size);
    tcl_cmd("side_lib_entry", tk_side_lib_entry);
    tcl_cmd("side_lib_entry_available", tk_side_lib_entry_available);
    tcl_cmd("interpret_variants", tk_interpret_variants);
    tcl_cmd("set_variant_value", tk_set_variant_value);
    tcl_cmd("numttypes", tk_numttypes);
    tcl_cmd("numutypes", tk_numutypes);
    tcl_cmd("nummtypes", tk_nummtypes);
    tcl_cmd("numatypes", tk_numatypes);
    tcl_cmd("numsides", tk_numsides);
    tcl_cmd("maxsides", tk_maxsides);
    tcl_cmd("minsides", tk_minsides);
    tcl_cmd("numtreasury", tk_numtreasury);
    tcl_cmd("curmap_number", tk_curmap_number);
    tcl_cmd("numfeatures", tk_numfeatures);
    tcl_cmd("numscorekeepers", tk_numscorekeepers);
    tcl_cmd("game_info", tk_game_info);
    tcl_cmd("atype_name", tk_atype_name);
    tcl_cmd("ttype_name", tk_ttype_name);
    tcl_cmd("t_image_name", tk_t_image_name);
    tcl_cmd("utype_name", tk_utype_name);
    tcl_cmd("u_image_name", tk_u_image_name);
    tcl_cmd("mtype_name", tk_mtype_name);
    tcl_cmd("m_image_name", tk_m_image_name);
    tcl_cmd("side_name", tk_side_name);
    tcl_cmd("side_adjective", tk_side_adjective);
    tcl_cmd("side_emblem", tk_side_emblem);
    tcl_cmd("short_side_title", tk_short_side_title);
    tcl_cmd("side_ingame", tk_side_ingame);
    tcl_cmd("simple_player_title", tk_simple_player_title);
    tcl_cmd("simple_player_name", tk_simple_player_name);
    tcl_cmd("player_advantage", tk_player_advantage);
    tcl_cmd("player_aitypename", tk_player_aitypename);
    tcl_cmd("player_displayname", tk_player_displayname);
    tcl_cmd("min_advantage", tk_min_advantage);
    tcl_cmd("max_advantage", tk_max_advantage);
    tcl_cmd("assigned_side", tk_assigned_side);
    tcl_cmd("assigned_player", tk_assigned_player);
    tcl_cmd("can_rename", tk_can_rename);
    tcl_cmd("adjust_advantage", tk_adjust_advantage);
    tcl_cmd("add_side_and_player", tk_add_side_and_player);
    tcl_cmd("remove_side_and_player", tk_remove_side_and_player);
    tcl_cmd("rename_side_for_player", tk_rename_side_for_player);
    tcl_cmd("set_ai_for_player", tk_set_ai_for_player);
    tcl_cmd("exchange_players", tk_exchange_players);
    tcl_cmd("set_indepside_options", tk_set_indepside_options);
    tcl_cmd("dside", tk_dside);
    tcl_cmd("feature_desc", tk_feature_desc);
    tcl_cmd("feature_info", tk_feature_info);
    tcl_cmd("set_feature_info", tk_set_feature_info);
    tcl_cmd("start_new_game", tk_start_new_game);
    tcl_cmd("start_saved_game", tk_start_saved_game);
    tcl_cmd("start_player_pre_setup_stage", tk_start_player_pre_setup_stage);
    tcl_cmd("start_game_load_stage", tk_start_game_load_stage);
    tcl_cmd("start_player_setup_stage", tk_start_player_setup_stage);
    tcl_cmd("launch_game", tk_launch_game);
    tcl_cmd("try_host_game", tk_try_host_game);
    tcl_cmd("try_join_game", tk_try_join_game);
    tcl_cmd("send_chat", tk_send_chat);
    tcl_cmd("set_unit_type", tk_set_unit_type);
    tcl_cmd("mouse_down_cmd", tk_mouse_down_cmd);
    tcl_cmd("mouse_up_cmd", tk_mouse_up_cmd);
    tcl_cmd("mouse_over_cmd", tk_mouse_over_cmd);
    tcl_cmd("world_mouse_down_cmd", tk_world_mouse_down_cmd);
    tcl_cmd("world_mouse_up_cmd", tk_world_mouse_up_cmd);
    tcl_cmd("world_mouse_over_cmd", tk_world_mouse_over_cmd);
    tcl_cmd("save_preferences", tk_save_prefs);
    tcl_cmd("init_help_list", tk_init_help_list);
    tcl_cmd("init_help_menu", tk_init_help_menu);
    tcl_cmd("help_goto", tk_help_goto);
    tcl_cmd("game_save", tk_game_save);
    tcl_cmd("make_default_player_spec", tk_make_default_player_spec);
    tcl_cmd("set_design_tool", tk_set_design_tool);
    tcl_cmd("set_design_data", tk_set_design_data);
    tcl_cmd("create_new_feature", tk_create_new_feature);
    tcl_cmd("destroy_feature", tk_destroy_feature);
    tcl_cmd("designer_fix", tk_designer_fix);
    tcl_cmd("designer_save", tk_designer_save);
    tcl_cmd("numutypes_available", tk_numutypes_available);
    tcl_cmd("utype_actual", tk_utype_actual);
    tcl_cmd("mtype_actual", tk_mtype_actual);
    tcl_cmd("map_size_at_power", tk_map_size_at_power);
    tcl_cmd("center_on_unit", tk_center_on_unit);
    tcl_cmd("advance_needed_to_research", tk_advance_needed_to_research);
    tcl_cmd("set_research_popped_up", tk_set_research_popped_up);
    tcl_cmd("current_advance", tk_current_advance);
    tcl_cmd("available_advance", tk_available_advance);
    tcl_cmd("set_side_research", tk_set_side_research);
#if 0
    tcl_cmd("agreements", tk_agreements);
#endif
    tcl_cmd("get_scores", tk_get_scores);
    tcl_cmd("reset_popup_flag", tk_reset_popup_flag);
    tcl_cmd("set_want_to_exit", tk_set_want_to_exit);
    tcl_cmd("exit_xconq", tk_exit_xconq);
    tcl_cmd("withdraw_window", tk_withdraw_window);

    tkwin = Tk_MainWindow(interp);

    Tcl_CreateCommand(interp, "map", mapw_cmd,
		      (ClientData) tkwin, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "imfsample", imfsample_cmd,
		      (ClientData) tkwin, (Tcl_CmdDeleteProc *) NULL);

    /* Find Tkconq's Tcl script. */
    {
	int loaded = FALSE, i = 0;
	FILE *fp;
	LibraryPath *p;
#ifndef MAC
	char *relpaths [] = {
	    "../share/xconq/tcltk/tkconq", "tcltk/tkconq", "tkconq", 
	    "xconq/tcltk/tkconq", NULL
	};
	char *librelpaths [] = {
	    "../tcltk/tkconq", "../tkconq", NULL
	};
#else
	char *relpaths [] = {
	    ":tcltk:tkconq", ":tkconq", "::tcltk:tkconq", NULL
	};
	char *librelpaths [] = {
	    NULL
	};
#endif

	/* First try paths relative to the working directory. */
	for (i = 0; relpaths[i]; ++i) {
	    make_pathname("", relpaths[i], "tcl", pathbuf);
	    if ((fp = fopen(pathbuf, "r")) != NULL) {
		fclose(fp);
		rslt = Tcl_EvalFile(interp, pathbuf);
		if (rslt == TCL_ERROR)
		  init_error("Error reading tcl from %s: %s",
			     pathbuf, interp->result);
		loaded = TRUE;
		break;
	    }
	}
	/* Then try paths relative to the library directories. */
	if (!loaded) {
	    for_all_library_paths(p) {
		for (i = 0; librelpaths[i]; ++i) {
		    make_pathname(p->path, librelpaths[i], "tcl", pathbuf);
		    if ((fp = fopen(pathbuf, "r")) != NULL) {
			fclose(fp);
			rslt = Tcl_EvalFile(interp, pathbuf);
			if (rslt == TCL_ERROR)
			  init_error("Error reading tcl from %s: %s",
				     pathbuf, interp->result);
			loaded = TRUE;
			break;
		    }
		}
		if (loaded)
		  break;
	    }
	} /* if (!loaded) */
	/* List all the places that were searched if we failed to load the 
	   script. */
	if (!loaded) {
	    for (i = 0; relpaths[i]; ++i) {
		make_pathname("", relpaths[i], "tcl", pathbuf);
		init_warning("Failed to load tkconq from: %s\n", pathbuf);
	    }
	    for_all_library_paths(p) {
		for (i = 0; librelpaths[i]; ++i) {
		    make_pathname(p->path, librelpaths[i], "tcl", pathbuf);
		    init_warning("Failed to load tkconq from: %s\n", pathbuf);
		}
	    }
	    init_error("tkconq.tcl file could not be loaded");
	}
    } /* { */

    imf_interp_hook = tk_interp_imf;
    imf_load_hook = tk_load_imf;

    update_variant_callback = update_variant_setting;
    update_assignment_callback = update_assignment;

    /* We need to get the network preferences already here
       for use in the chat dialog. */
    get_preferences();

    /* Start the time handler loop. */
    check_network(0);
}

int
tk_library_paths(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
#ifdef AVOID_OBSTACK
    char buf[XCQ_PATH_LEN_MAX*2];
#else
    TextBuffer buf;
    int ostacksize = 0;
#endif
    LibraryPath *path = NULL;

#ifdef AVOID_OBSTACK
    buf[0] = 0;
#else
    obstack_begin(&(buf.ostack), XCQ_PATH_LEN_MAX*2); 
#endif
    
    for_all_library_paths(path) {
	/* Use semicolons as separators, for the benefit of split. */
	/* (It would be clever to to use a char known not to be in any
	   of the paths, but only do that much work when it proves
	   necessary). */
#ifdef AVOID_OBSTACK
	if (!empty_string(buf))
	  strcat(buf, ";");
	strcat(buf, path->path);
#else
        obstack_grow(&(buf.ostack), path->path, strlen(path->path));
        if (path->next)
          obstack_1grow(&(buf.ostack), ';');
#endif
    }
#ifdef AVOID_OBSTACK
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
#else
    obstack_1grow(&(buf.ostack), 0);
    ostacksize = obstack_object_size(&(buf.ostack));
    buf.text = (char *)xmalloc(ostacksize);
    strncpy(buf.text, (const char *)obstack_finish(&(buf.ostack)), ostacksize);
    Tcl_SetResult(interp, buf.text, (void (*)(char *))free);
    obstack_free(&(buf.ostack), 0);
#endif
    return TCL_OK;
}

int
tk_numgames(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    collect_possible_games();
    sprintf(interp->result, "%d", numgames);
    return TCL_OK;
}

int
tk_any_variants(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", any_variants);
    return TCL_OK;
}

int
tk_listallgames(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", listallgames);
    return TCL_OK;
}

int
tk_side_lib_size(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", length(g_side_lib()));
    return TCL_OK;
}

int
tk_side_lib_entry(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int entryi;
    char *name;
    Obj *entry, *rest, *anelt;

    entryi = strtol(argv[1], NULL, 10);
    entry = elt(g_side_lib(), entryi);
    name = "?name?";
    for_all_list(entry, rest) {
	anelt = car(rest);
	if (consp(anelt) && stringp(cadr(anelt))) {
	    name = c_string(cadr(anelt));
	    break;
	}
    }
    Tcl_SetResult(interp, name, TCL_VOLATILE);
    return TCL_OK;
}

int
tk_side_lib_entry_available(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[])
{
    int entryi, rslt;
    char *name;
    Obj *entry, *rest, *anelt;

    entryi = strtol(argv[1], NULL, 10);
    entry = elt(g_side_lib(), entryi);
    name = "?name?";
    rslt = TRUE;
    for_all_list(entry, rest) {
	anelt = car(rest);
	if (consp(anelt) && stringp(cadr(anelt))) {
	    name = c_string(cadr(anelt));
	    if (name_in_use(NULL, name))
	      rslt = FALSE;
	    break;
	}
    }
    sprintf(interp->result, "%d", rslt);
    return TCL_OK;
}

int
tk_interpret_variants(ClientData cldata, Tcl_Interp *interp,
		      int argc, char *argv[])
{
    interpret_variants();
    return TCL_OK;
}

int
tk_set_variant_value(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int n, which;
    /* extern int checkboxpos[], worldsizepos, realtimepos; */

    /* Translate tcl/tk index to variant number. */
    n = strtol(argv[1], NULL, 10);
    if (n == -1)
      which = worldsizepos;
    else if (n == -2)
      which = realtimepos;
    else
      which = checkboxpos[n];
    net_set_variant_value(which,
			  strtol(argv[2], NULL, 10),
			  strtol(argv[3], NULL, 10),
			  strtol(argv[4], NULL, 10));
    return TCL_OK;
}

int
tk_numttypes(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numttypes);
    return TCL_OK;
}

int
tk_numutypes(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numutypes);
    return TCL_OK;
}

int
tk_nummtypes(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", nummtypes);
    return TCL_OK;
}

int
tk_numatypes(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numatypes);
    return TCL_OK;
}

int
tk_numsides(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numsides);
    return TCL_OK;
}

int
tk_maxsides(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", g_sides_max());
    return TCL_OK;
}

int
tk_minsides(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", g_sides_min());
    return TCL_OK;
}

int
tk_numfeatures(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numfeatures);
    return TCL_OK;
}

int
tk_numscorekeepers(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numscorekeepers);
    return TCL_OK;
}

int
tk_numtreasury(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int m, rslt = 0;

    for_all_material_types(m) {
	if (m_treasury(m))
	  ++rslt;
    }
    sprintf(interp->result, "%d", rslt);
    return TCL_OK;
}

int
tk_curmap_number(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int n = 0;
    Map *map;

    for_all_maps(map) {
	if (map == dside->ui->curmap) {
		n = map->number;
		break;
	}
    }
    sprintf(interp->result, "%d", n);
    return TCL_OK;
}

int
tk_ttype_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int t;

    t = strtol(argv[1], NULL, 10);
    if (is_terrain_type(t))
      Tcl_SetResult(interp, t_type_name(t), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?t?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_atype_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int a;

    a = strtol(argv[1], NULL, 10);
    if (is_advance_type(a))
      Tcl_SetResult(interp, a_type_name(a), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?a?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_t_image_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int t;
    char *str;

    t = strtol(argv[1], NULL, 10);
    if (is_terrain_type(t)) {
	str = t_image_name(t);
	if (empty_string(str))
	  str = t_type_name(t);
        Tcl_SetResult(interp, str, TCL_VOLATILE);
    } else
      Tcl_SetResult(interp, "?t?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_utype_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int u;

    u = strtol(argv[1], NULL, 10);
    if (is_unit_type(u))
      Tcl_SetResult(interp, u_type_name(u), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?u?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_u_image_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int u = -1, i = 0, choice = -1;
    char *str = NULL;
    ImageFamily *imf = NULL;

    u = strtol(argv[1], NULL, 10);
    if (is_unit_type(u)) {
	/* Use the designated image name. */
	if (stringp(u_image_name(u))) {
	    str = get_string(u_image_name(u));
	    if (empty_string(str)) {
	      str = u_internal_name(u);
	    }
	}
	else if (lispnil == u_image_name(u))
	  str = u_internal_name(u);
	/* Else pick a random image from the list for this unit type. */
	else if (consp(u_image_name(u))) {
	    choice = xrandom(numuimages[u]);
	    for (imf = uimages[u]; imf != NULL; imf = imf->next) {
		if (i == choice) {
		    str = imf->name;
		    break;
		}
		++i;
	    }
	}
	/* Else use unit's name, and hope for the best. */
	else 
	  str = u_internal_name(u);
	Tcl_SetResult(interp, str, TCL_VOLATILE);
    } else {
	Tcl_SetResult(interp, "?u?", TCL_VOLATILE);
    }
    return TCL_OK;
}

int
tk_mtype_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int m;

    m = strtol(argv[1], NULL, 10);
    if (is_material_type(m))
      Tcl_SetResult(interp, m_type_name(m), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?m?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_m_image_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int m;
    char *str;

    m = strtol(argv[1], NULL, 10);
    if (is_material_type(m)) {
	str = m_image_name(m);
        Tcl_SetResult(interp, str, TCL_VOLATILE);
    } else
      Tcl_SetResult(interp, "?m?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_side_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;

    s = strtol(argv[1], NULL, 10);
    if (between(0, s, numsides))
      Tcl_SetResult(interp, side_name(side_n(s)), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_side_adjective(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;

    s = strtol(argv[1], NULL, 10);
    if (between(0, s, numsides))
      Tcl_SetResult(interp, side_adjective(side_n(s)), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_side_emblem(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;

    s = strtol(argv[1], NULL, 10);
    if (s == 0)
      Tcl_SetResult(interp, indepside->emblemname, TCL_VOLATILE);
    else if (between(1, s, numsides)) {
	/* Try to get the emblem that was actually set up - it may be
	   a default or something else different from the official
	   emblem name. */
	if (dside && dside->ui && dside->ui->eimages[s])
          Tcl_SetResult(interp, dside->ui->eimages[s]->name, TCL_VOLATILE);
	else if (side_n(s)->emblemname)
          Tcl_SetResult(interp, (side_n(s))->emblemname, TCL_VOLATILE);
	else
          Tcl_SetResult(interp, "null", TCL_VOLATILE);
    } else
      Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_short_side_title(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;

    s = strtol(argv[1], NULL, 10);
    if (between(0, s, numsides))
      /* (should handle empty string?) */
      Tcl_SetResult(interp, short_side_title(side_n(s)), TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_side_ingame(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;

    s = strtol(argv[1], NULL, 10);
    if (between(0, s, numsides))
      sprintf(interp->result, "%d", side_n(s)->ingame);
    else
      Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_simple_player_title(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int p;
    char abuf[300];
    Player *player;

    p = strtol(argv[1], NULL, 10);
    player = find_player(p);
    simple_player_title(abuf, player);
    Tcl_SetResult(interp, abuf, TCL_VOLATILE);
    return TCL_OK;
}

int
tk_simple_player_name(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int p;
    char abuf[300];
    Player *player;

    p = strtol(argv[1], NULL, 10);
    player = find_player(p);
    simple_player_name(abuf, player);
    Tcl_SetResult(interp, abuf, TCL_VOLATILE);
    return TCL_OK;
}

int
tk_player_advantage(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int p;
    Player *player;

    p = strtol(argv[1], NULL, 10);
    player = find_player(p);
    sprintf(interp->result, "%d", (player ? player->advantage : 0));
    return TCL_OK;
}

int
tk_player_aitypename(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int p;
    Player *player;

    p = strtol(argv[1], NULL, 10);
    player = find_player(p);
    if (player->aitypename)
      Tcl_SetResult(interp, player->aitypename, TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_player_displayname(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int p;
    Player *player;

    p = strtol(argv[1], NULL, 10);
    player = find_player(p);
    if (player->displayname)
      Tcl_SetResult(interp, player->displayname, TCL_VOLATILE);
    else
      Tcl_SetResult(interp, "", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_min_advantage(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;
    Side *side;

    s = strtol(argv[1], NULL, 10);
    side = side_n(s);
    sprintf(interp->result, "%d", (side ? side->minadvantage : 0));
    return TCL_OK;
}

int
tk_max_advantage(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;
    Side *side;

    s = strtol(argv[1], NULL, 10);
    side = side_n(s);
    sprintf(interp->result, "%d", (side ? side->maxadvantage : 0));
    return TCL_OK;
}

int
tk_assigned_side(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int i;
    Side *side;

    i = strtol(argv[1], NULL, 10);
    if (between(0, i, numsides)) {
	side = assignments[i].side;
	sprintf(interp->result, "%d", side->id);
    } else {
        Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    }
    return TCL_OK;
}

int
tk_assigned_player(ClientData cldata, Tcl_Interp *interp,
		   int argc, char *argv[])
{
    int i;
    Player *player;

    i = strtol(argv[1], NULL, 10);
    if (between(0, i, numsides)) {
	player = assignments[i].player;
	sprintf(interp->result, "%d", (player ? player->id : 0));
    } else {
        Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    }
    return TCL_OK;
}

int
tk_can_rename(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int s;
    Side *side;

    s = strtol(argv[1], NULL, 10);
    side = side_n(s);
    sprintf(interp->result, "%d", side != NULL && !side->nameslocked);
    return TCL_OK;
}

int
tk_adjust_advantage(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    int p, amt;
    Player *player;

    p = strtol(argv[1], NULL, 10);
    amt = strtol(argv[2], NULL, 10);
    player = assignments[p].player;
    net_set_player_advantage(p, player->advantage + amt);
    return TCL_OK;
}

int
tk_add_side_and_player(ClientData cldata, Tcl_Interp *interp,
		       int argc, char *argv[])
{
    int n;

    n = net_add_side_and_player();
    /* Return the position of the new side/player in the assignment array. */
    sprintf(interp->result, "%d", n);
    return TCL_OK;
}

int
tk_remove_side_and_player(ClientData cldata, Tcl_Interp *interp,
		       int argc, char *argv[])
{
    int rslt, s;

    s = strtol(argv[1], NULL, 10);
    rslt = net_remove_side_and_player(s);
    /* Return TRUE or FALSE depending on the outcome. */
    sprintf(interp->result, "%d", rslt);
    return TCL_OK;
}

int
tk_rename_side_for_player(ClientData cldata, Tcl_Interp *interp,
			  int argc, char *argv[])
{
    int n, n2;

    n = strtol(argv[1], NULL, 10);
    n2 = strtol(argv[2], NULL, 10);
    net_rename_side_for_player(n, n2);
    return TCL_OK;
}

int
tk_set_ai_for_player(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int n;
    char *aitype;
    Player *player;

    n = strtol(argv[1], NULL, 10);
    if (strcmp(argv[2], "-cycle") == 0) {
	player = assignments[n].player;
	aitype = next_ai_type_name(player->aitypename);
    } else {
	aitype = copy_string(argv[2]);
    }
    net_set_ai_for_player(n, aitype);
    return TCL_OK;
}

int
tk_exchange_players(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    int n, n2;

    n = strtol(argv[1], NULL, 10);
    n2 = strtol(argv[2], NULL, 10);
    if (between(0, n, numsides)) {
	n2 = net_exchange_players(n, n2);
	sprintf(interp->result, "%d", n2);
    } else {
        Tcl_SetResult(interp, "?s?", TCL_VOLATILE);
    }
    return TCL_OK;
}

int
tk_set_indepside_options(ClientData cldata, Tcl_Interp *interp,
			 int argc, char *argv[])
{
    int n, n2, n3, n4, n5;
    int u = NONUTYPE;

    if (1 < argc) {
	n = strtol(argv[1], NULL, 10);
	for_all_unit_types(u)
	    indepside->could_act_with[u] = n;
    }
    if (2 < argc) {
	n2 = strtol(argv[2], NULL, 10);
	for_all_unit_types(u)
	    indepside->could_construct[u] = n2;
    }
    if (3 < argc) {
	n3 = strtol(argv[3], NULL, 10);
	set_g_indepside_can_research(n3);
    }
    if (4 < argc) {
	n4 = strtol(argv[4], NULL, 10);
	set_g_indepside_has_economy(n4);
    }
    if (5 < argc) {
	n5 = strtol(argv[5], NULL, 10);
	set_g_indepside_has_treasury(n5);
    }
#if (0)
    if (n || n2 || n3)
	set_g_indepside_has_ai(TRUE);
#endif
    return TCL_OK;
}

int
tk_dside(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", dside->id);
    return TCL_OK;
}

int
tk_feature_info(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int fid;
    Feature *feature;

    fid = strtol(argv[1], NULL, 10);
    feature = find_feature(fid);
    if (feature == NULL)
      sprintf(interp->result, "{\"%s\" \"%s\"}", "no %t", "feature");
    else if (feature->name != NULL || feature->feattype != NULL)
      sprintf(interp->result, "{\"%s\" \"%s\"}",
	      (feature->name ? feature->name : ""),
	      (feature->feattype ? feature->feattype : ""));
    else
      Tcl_SetResult(interp, "{?name? ?type?}", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_feature_desc(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char abuf[BUFSIZE], *fdesc;
    int fid;
    Feature *feature;

    fid = strtol(argv[1], NULL, 10);
    feature = find_feature(fid);
    fdesc = feature_desc(feature, abuf);
    /* The following is really only good for a menu entry, other uses
       of feature_desc might prefer something else. */
    if (fdesc == NULL)
      fdesc = "no feature";
    Tcl_SetResult(interp, fdesc, TCL_VOLATILE);
    return TCL_OK;
}

int
tk_set_feature_info(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    int fid;
    char *fname, *ftypename;
    Feature *feature;

    fid = strtol(argv[1], NULL, 10);
    feature = find_feature(fid);
    fname = argv[2];
    ftypename = argv[3];
    if (feature != NULL) {
	net_set_feature_name(feature, fname);
	net_set_feature_type_name(feature, ftypename);
    }
    return TCL_OK;
}

int
tk_version_string(ClientData cldata, Tcl_Interp *interp, int argc,
		  char *argv[])
{
    Tcl_SetResult(interp, version_string(), TCL_VOLATILE);
    return TCL_OK;
}

int
tk_copyright_string(ClientData cldata, Tcl_Interp *interp, int argc,
		  char *argv[])
{
    Tcl_SetResult(interp, copyright_string(), TCL_VOLATILE);
    return TCL_OK;
}

int
tk_game_homedir(ClientData cldata, Tcl_Interp *interp, int argc,
		  char *argv[])
{
    Tcl_SetResult(interp, game_homedir(), TCL_VOLATILE);
    return TCL_OK;
}

int
tk_game_info(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char *name, *title, *version, *bname, *picture, *bmodule, *bpicture = NULL;
    char blurb[BLURBSIZE], bigbuf[BLURBSIZE + 25];
    int gamei, rslt;
    Module *basemodule;

    gamei = strtol(argv[1], NULL, 10);
    name = title = version = bname = "";
    basemodule = find_game_module(possible_games[gamei]->basemodulename);
    if (possible_games[gamei] != NULL) {
	name = possible_games[gamei]->name;
	title = possible_games[gamei]->title;
	if (title == NULL)
	  title = "";
	version = possible_games[gamei]->version;
	if (version == NULL)
	  version = "";
	picture = possible_games[gamei]->picturename;
	if (picture == NULL)
	  picture = "";
	bmodule = possible_games[gamei]->basemodulename;
	if (bmodule == NULL)
	  bmodule = "";
	if (basemodule)
	  bpicture = possible_games[gamei]->picturename;
	if (bpicture == NULL)
	  bpicture = "";
	if (possible_games[gamei]->blurb != lispnil) {
		strcpy(blurb, "");
		append_blurb_strings(blurb, possible_games[gamei]->blurb);
	} else {
		strcpy(blurb, "(no description)");
	}
	bname = (char *)(possible_games[gamei] -> basemodulename ? "-  " : "");
    }
    eval_tcl_cmd("set game_name(%d) \"%s\"", gamei, name);
    eval_tcl_cmd("set game_title(%d) \"%s\"", gamei, title);
    eval_tcl_cmd("set game_version(%d) \"%s\"", gamei, version);
    eval_tcl_cmd("set game_base_name(%d) \"%s\"", gamei, bname);
    eval_tcl_cmd("set game_picturename(%d) \"%s\"", gamei, picture);
    eval_tcl_cmd("set game_basemodule(%d) \"%s\"", gamei, bmodule);
    eval_tcl_cmd("set game_basemodule_picturename(%d) \"%s\"", gamei, bpicture);
    /* The blurb might be too big (>500) to feed to eval_tcl_cmd. */
    sprintf (bigbuf, "set game_blurb(%d) \"%s\"", gamei, blurb);
    rslt = Tcl_GlobalEval(interp, bigbuf);
    if (rslt == TCL_ERROR) {
	fprintf(stderr, "Error: %s\n", interp->result);
	fprintf(stderr, "Error: while processing the %s blurb.\n", name);
    }
    return TCL_OK;
}

int
tk_start_new_game(ClientData cldata, Tcl_Interp *interp,
		  int argc, char *argv[])
{
    int gamei;

    gamei = strtol(argv[1], NULL, 10);
    mainmodule = possible_games[gamei];
    load_game_module(mainmodule, TRUE);
    check_game_validity();
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_game_module();
    start_variant_setup_stage();
    return TCL_OK;
}

int
tk_start_saved_game(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    /* Get an anonymous module. */
    mainmodule = get_game_module(NULL);
    mainmodule->filename = copy_string(find_name(argv[1]));
    load_game_module(mainmodule, TRUE);
    check_game_validity();
    if (my_rid > 0 && my_rid == master_rid)
      broadcast_game_module();
    /* (skip over?) */
    start_variant_setup_stage();
    return TCL_OK;
}

int
tk_start_player_pre_setup_stage(ClientData cldata, Tcl_Interp *interp,
				int argc, char *argv[])
{
    start_player_pre_setup_stage();
    return TCL_OK;
}

int
tk_start_game_load_stage(ClientData cldata, Tcl_Interp *interp,
				int argc, char *argv[])
{
    start_game_load_stage();
    return TCL_OK;
}

int
tk_start_player_setup_stage(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[])
{
    int u = NONUTYPE;

    start_player_setup_stage();
    eval_tcl_cmd("set indepside_ai %d", g_indepside_has_ai());
    for_all_unit_types(u) {
	if (indepside->could_act_with[u]) {
	    eval_tcl_cmd("set indepside_act %d", TRUE);
	    break;
	}
    }
    for_all_unit_types(u) {
	if (indepside->could_construct[u]) {
	    eval_tcl_cmd("set indepside_build %d", TRUE);
	    break;
	}
    }
    eval_tcl_cmd("set indepside_research %d", g_indepside_can_research());
    eval_tcl_cmd("set indepside_economy %d", g_indepside_has_economy());
    eval_tcl_cmd("set indepside_treasury %d", g_indepside_has_treasury());
    return TCL_OK;
}

int
tk_launch_game(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    launch_game();
    return TCL_OK;
}

int
tk_try_host_game(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    host_the_game(argv[1]);
    return TCL_OK;
}

int
tk_try_join_game(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int rslt;

    rslt = try_join_game(argv[1]);
    sprintf(interp->result, "%d", rslt);
    return TCL_OK;
}

int
tk_send_chat(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    net_send_chat(my_rid, argv[1]);
    return TCL_OK;
}

int
tk_run_game(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int maxactions, rslt = 0, interval;
    int cs1, cs2;

    if (argc != 2) {
	run_warning("wrong # args (%d) to run_game", argc - 1);
	return TCL_OK;
    }

    maxactions = strtol(argv[1], NULL, 10);

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    cs1 = cs2 = 0;
    if (my_rid > 0 && my_rid != master_rid)
      cs1 = game_checksum();
    run_local_ai(1, 20);
    if (my_rid > 0 && my_rid != master_rid)
      cs2 = game_checksum();
    if (cs1 != cs2)
      run_warning("Checksum %d -> %d after run_local_ai\n", cs1, cs2);

    /* Run the kernel itself. */
    rslt += net_run_game(maxactions);
    
#if 0
    cs1 = cs2 = 0;
    if (my_rid > 0 && my_rid != master_rid)
      cs1 = game_checksum();
    run_local_ai(2, 20);
    if (my_rid > 0 && my_rid != master_rid)
      cs2 = game_checksum();
    if (cs1 != cs2)
      run_warning("Checksum %d -> %d after run_local_ai\n", cs1, cs2);
#endif

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    /* We are done. */
    if (want_to_exit
	/* Single players can quit as soon as end_the_game has been run. */
	&& ((my_rid == 0 && ok_to_exit)
	/* Hosts can quit only when everybody else have disconnected. */
	    || (my_rid == 1 && ok_to_exit /*&& numremotes == 1*/)
	/* Clients can quit whenever they want. */
	    || (my_rid > 1))) {			
	exit_xconq(dside);
    }
    /* Set up to call it again in a little while. */
    /* If things are happening, call again quickly, for responsiveness. */
#if (0)
    interval = 10;
#else
    interval = 1;
#endif
    /* If nothing seems to be happening right now, do a few times/sec,
       just in case. */
    if (rslt == 0)
#if (0)
      interval = 250;
#else
      interval = 50;
#endif
    sprintf(interp->result, "%d", interval);
    return TCL_OK;
}

int
tk_run_game_idle(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    Map *map;
    Unit *unit, *unit2;
    char *activity = "tk_rg_idle";

    if (!active_display(dside))
      return TCL_OK;
    record_activity_start(activity, 0);
    /* See if we should jump to another unit and make it current. */
    for_all_maps(map) {
	if (map->widget == NULL)
	  continue;
	unit = map->curunit;
	if (map->autoselect) {
	    if (in_play(unit)
		&& unit->id == map->curunit_id
		&& side_controls_unit(dside, unit)
		&& has_acp_left(unit)
		&& (unit->plan ? !unit->plan->asleep : TRUE)
		&& (unit->plan ? !unit->plan->reserve : TRUE)
		&& (unit->plan ? !unit->plan->delayed : TRUE)
		) {
		if (!in_middle(map, unit->x, unit->y)
		    && !map->scrolled_to_unit) {
		    put_on_screen(map, unit->x, unit->y);
		    map->scrolled_to_unit = TRUE;
		}
	    } else {
		unit2 = autonext_unit_inbox(dside, unit, widget_vp(map));
		if (unit2
		    && unit2->plan
		    && !unit2->plan->asleep
		    && !unit2->plan->reserve
		    && !unit2->plan->delayed
		    && unit2->plan->waitingfortasks) {
		    /* Use this unit */
		} else {
		    if (!in_play(unit)
			|| unit->id != map->curunit_id
			|| !side_controls_unit(dside, unit))
		      unit = NULL;
		    unit2 = autonext_unit(dside, unit);
		}
		if (unit2 != unit)
		  map->scrolled_to_unit = FALSE;
		if (unit2 != NULL)
		  set_current_unit(map, unit2);
	    }
	} else {
	    /* Even when not auto-selecting, the selected unit may pass
	       out of our control. */
	    if (!in_play(unit)
		|| unit->id != map->curunit_id
		|| !(side_controls_unit(dside, unit) || endofgame))
	      unit = NULL;
	    set_current_unit(map, unit);
	}
	update_mouseover(map, map->last_rawx[0], map->last_rawy[0]);
	update_world_mouseover(map, map->last_rawx[1], map->last_rawy[1]);
    }
    record_activity_end(activity, 0);
    return TCL_OK;
}

int
tk_animate_selection(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    Map *map;
    char *activity = "tk_anim_sel";

    if (active_display(dside)) {
	for_all_maps(map) {
	    if (map->autoselect
		&& map->widget != NULL
		&& in_play(map->curunit)
		&& map->curunit->id == map->curunit_id) {
		record_activity_start(activity, map->curunit->id);
		map->anim_state = (map->anim_state + 1) % 8;
		update_cell(map, map->curunit->x, map->curunit->y);
		record_activity_end(activity, map->anim_state);
	    } else {
	    	/* This is a hack to ensure that the outline rect in the 
		   worldmap always is updated during scrolling now when it is 
		   not redrawn as often as previously. */ 
		update_cell(map, area.halfwidth, area.halfheight);
	    }
	}
    }
    return TCL_OK;
}

int
tk_set_unit_type(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int mapn, u;
    Map *map;

    mapn = strtol(argv[1], NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    u = strtol(argv[2], NULL, 10);
    map->inptype = u;
    return TCL_OK;
}

int
tk_mouse_down_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int mapn, rawx, rawy, button;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    button = strtol(argv[4], NULL, 10);
    DGprintf("down map%d %d %d %d\n", mapn, rawx, rawy, button);
    handle_mouse_down(map, rawx, rawy, button);
    return TCL_OK;
}

int
tk_mouse_up_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int mapn, rawx, rawy, button;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    button = strtol(argv[4], NULL, 10);
    DGprintf("up map%d %d %d %d\n", mapn, rawx, rawy, button);
    handle_mouse_up(map, rawx, rawy, button);
    return TCL_OK;
}

int
tk_mouse_over_cmd(ClientData cldata, Tcl_Interp *interp,
		  int argc, char *argv[])
{
    int mapn, rawx, rawy;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    if (is_designer(dside))
      paint_on_drag(map, rawx, rawy, 0);
    update_mouseover(map, rawx, rawy);
    return TCL_OK;
}

void
update_mouseover(Map *map, int rawx, int rawy)
{
    int x, y, u;
    enum mapmode prevtmpmode;
    Unit *unit, *unit2;
    UnitView *uview;
    Side *side2;
    VP *vp;

    DGprintf("over map%d %d %d\n", map->number, rawx, rawy);
    tmpbuf[0] = '\0';
    if ((rawx >= 0 || rawy >= 0) && map->widget != NULL) {
	vp = widget_vp(map);
	oneliner(dside, vp, rawx, rawy);
	eval_tcl_cmd("update_mouseover %d \"%s\"", map->number, tmpbuf);
	prevtmpmode = map->tmp_mode;
	map->tmp_mode = no_tmp_mode;
	unit = map->curunit;
	/* Modify the cursor depending on the type of unit it is over. */
	if (unit != NULL && !is_designer(dside)) {
	    nearest_cell(vp, rawx, rawy, &x, &y, NULL, NULL);
	    nearest_unit_view(dside, vp, rawx, rawy, &uview);
	    if (in_area(x, y)) {
		if (vp->show_all) {
		    nearest_unit(dside, vp, rawx, rawy, &unit2);
		    if (unit2 != NULL) {
			if (unit_could_attack(unit, unit2->type, unit2->side,
					      x, y))
			  map->tmp_mode = attack_mode;
		    }
		} else if (uview != NULL) {
		    u = uview->type;
		    side2 = side_n(uview->siden);
		    if (unit_could_attack(unit, u, side2, x, y))
		      map->tmp_mode = attack_mode;
		}
	    }
	}
	if (map->tmp_mode != prevtmpmode) {
	  set_tool_cursor(map, 0);
	}
	/* Check if we need to do any autoscrolling. */
	if (rawx > 0 && rawx < autoscroll_width && vp->sx > vp->sxmin
	    || rawy > 0 && rawy < autoscroll_width && vp->sy > vp->symin
	    || rawx > (vp->pxw - autoscroll_width) && vp->sx < vp->sxmax
	    || rawy > (vp->pxh - autoscroll_width) && vp->sy < vp->symax) {
	autoscroll(map, vp, 0, rawx, rawy);
	/* If not, reset the scroll mode. */
	} else {
		map->scroll_mode[0] = no_scroll_mode;
		set_tool_cursor(map, 0);
	}
    }
    map->last_rawx[0] = rawx;  
    map->last_rawy[0] = rawy;
}

int
tk_world_mouse_down_cmd(ClientData cldata, Tcl_Interp *interp,
			int argc, char *argv[])
{
    int mapn, rawx, rawy, button;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    button = strtol(argv[4], NULL, 10);
    DGprintf("world down map%d %d %d %d\n", mapn, rawx, rawy, button);
    handle_world_mouse_down(map, rawx, rawy, button);
    return TCL_OK;
}

int
tk_world_mouse_up_cmd(ClientData cldata, Tcl_Interp *interp,
		      int argc, char *argv[])
{
    int mapn, rawx, rawy, button;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    button = strtol(argv[4], NULL, 10);
    DGprintf("world up map%d %d %d %d\n", mapn, rawx, rawy, button);
    handle_world_mouse_up(map, rawx, rawy, button);
    return TCL_OK;
}

int
tk_world_mouse_over_cmd(ClientData cldata, Tcl_Interp *interp,
		  int argc, char *argv[])
{
    int mapn, rawx, rawy;
    Map *map;

    mapn = strtol(argv[1] + 2, NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    rawx = strtol(argv[2], NULL, 10);  rawy = strtol(argv[3], NULL, 10);
    update_world_mouseover(map, rawx, rawy);
    return TCL_OK;
}

void
update_world_mouseover(Map *map, int rawx, int rawy)
{
    VP *vp;

    DGprintf("over map%d world %d %d\n", map->number, rawx, rawy);
    if ((rawx >= 0 || rawy >= 0) && map->worldw != NULL) {
	vp = worldw_vp(map);
	/* Check if we need to do any autoscrolling. */
	if (rawx > 0 && rawx < autoscroll_width && vp->sx > vp->sxmin
	    || rawy > 0 && rawy < autoscroll_width && vp->sy > vp->symin
	    || rawx > (vp->pxw - autoscroll_width) && vp->sx < vp->sxmax
	    || rawy > (vp->pxh - autoscroll_width) && vp->sy < vp->symax) {
		autoscroll(map, vp, 1, rawx, rawy);
	/* If not, reset the scroll mode. */
	} else {
		map->scroll_mode[1] = no_scroll_mode;
		set_tool_cursor(map, 1);
	}
    }
    map->last_rawx[1] = rawx;  
    map->last_rawy[1] = rawy;
}

/* Autoscroll if the mouse is near the edge of a map. */

void
autoscroll(Map *map, VP *vp, int which, int rawx, int rawy)
{
    int xdelta, ydelta;
    enum mapmode prevscrollmode;

    prevscrollmode = map->scroll_mode[which];
    map->scroll_mode[which] = no_scroll_mode;
    xdelta = ydelta = 0;
    /* Note that rawx,rawy may be validly 0,0 here, so avoid that case
       - means that autoscrolling won't work on the exact edge of the
       map, but not likely to be a problem in practice. */
    /* Also note that we don't do diagonal autoscrolling - the code
       has to do two scrolls in different directions in quick
       succession, and the net effect is to "shake" the display;
       amusing, but it got old after two minutes of testing... */
    if (rawx > 0 && rawx < autoscroll_width && vp->sx > vp->sxmin) {
	map->scroll_mode[which] = scroll_left_mode;
	xdelta = -1;
    } else if (rawy > 0 && rawy < autoscroll_width && vp->sy > vp->symin) {
	map->scroll_mode[which] = scroll_up_mode;
	ydelta = -1;
    } else if (rawx > (vp->pxw - autoscroll_width) && vp->sx < vp->sxmax) {
	map->scroll_mode[which] = scroll_right_mode;
	xdelta = 1;
    } else if (rawy > (vp->pxh - autoscroll_width) && vp->sy < vp->symax) {
	map->scroll_mode[which] = scroll_down_mode;
	ydelta = 1;
    }
    if (xdelta != 0 || ydelta != 0) {
	if (map->autoscroll_delaying[which]) {
	    if (n_ms_elapsed(autoscroll_delay)) {
		eval_tcl_cmd("autoscroll %d %d %d %d", map->number, which, xdelta, ydelta);
	    }
	} else {
	    record_ms();
	    map->autoscroll_delaying[which] = TRUE;
	}
    } else {
	map->autoscroll_delaying[which] = FALSE;
    }
    /* Make the cursor change appearance immediately, even while
       delaying the autoscroll. */
    if (map->scroll_mode[which] != prevscrollmode)
      set_tool_cursor(map, which);
}

void
set_current_unit(Map *map, Unit *unit)
{
    Unit *oldunit = map->curunit;
    int x, y, x1, y1;
    
    if (unit == oldunit)
      return;
    if (unit == NULL
	|| (in_play(unit) && side_controls_unit(dside, unit))
	|| endofgame) {
	map->curunit = unit;
	map->curunit_id = (unit ? unit->id : 0);
    }
    /* Make sure the unit is actually visible on-screen. */
    if (unit != NULL) {
	put_on_screen(map, unit->x, unit->y);
	map->scrolled_to_unit = TRUE;
    }
    /* (should only do this if map not scrolled) */
    if (oldunit) {
	x = oldunit->x;
	y = oldunit->y;
    	update_cell(map, x, y);
    	/* Make sure the resource icons are erased for the
    	old selected unit. */
    	if (any_resources
    	    && u_advanced(oldunit->type)) {
    		for_all_cells_within_reach(oldunit, x1, y1) {
    			if (user_at(x1, y1) == oldunit->id) {
    				update_cell(map, x1, y1);
    			}
    		}
    	}
    }
    if (map->curunit) {
	x = map->curunit->x;
	y = map->curunit->y;
    	update_cell(map, x, y);
    	/* Make sure the resource icons are drawn for the
    	new selected unit. */
    	if (any_resources
    	    && u_advanced(map->curunit->type)) {
    		for_all_cells_within_reach(map->curunit, x1, y1) {
    			if (user_at(x1, y1) == map->curunit->id) {
    				update_cell(map, x1, y1);
    			}
    		}
    	}
    }
    draw_unit_info(map);
    update_action_controls_info(map);
}

static int empty_unit_info;

void
draw_unit_info(Map *map)
{
    char infobuf[BUFSIZE];
    int mapn = map->number, u, mrow, x = -1, y = -1;
    Unit *unit = NULL;
    Task *task = NULL;
#if (0)
    Action *action = NULL;
#endif
    ImageFamily *uimf, *eimf;

    unit = map->curunit;
    if (!in_play(unit)) {
	if (!empty_unit_info) {
	    eval_tcl_cmd("update_unit_info %d curunit 0", mapn);
	    eval_tcl_cmd("update_unit_info %d handle \"(no unit)\"", mapn);
	    eval_tcl_cmd("update_unit_info %d loc \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d occ \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d hp \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d stack \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d s0 \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d s1 \"\"", mapn);
#if 0  /* too much screen real estate */
	    eval_tcl_cmd("update_unit_info %d s2 \"\"", mapn);
#endif
	    eval_tcl_cmd("update_unit_info %d aux0 \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d plan \"\"", mapn);
	    eval_tcl_cmd("update_unit_info %d t0 \"\"", mapn);
#if 0  /* too much screen real estate */
	    eval_tcl_cmd("update_unit_info %d t1 \"\"", mapn);
#endif
	    eval_tcl_cmd("update_unit_picture %d \"(no)\" \"(no)\"", mapn);
	    empty_unit_info = TRUE;
	}
	return;
    }
    empty_unit_info = FALSE;
    u = unit->type;
    eval_tcl_cmd("update_unit_info %d curunit %d", mapn, unit->id);
    /* Update the image displayed. */
    uimf = unit->imf;
    eimf = dside->ui->eimages[unit->side->id];
    eval_tcl_cmd("update_unit_picture %d \"%s\" \"%s\"",
		 mapn,
		 (uimf ? uimf->name : "(no)"),
		 (eimf ? eimf->name : "(no)"));
    /* Say which unit this is. */
    strcpy(infobuf, unit_handle(dside, unit));
    capitalize(infobuf);
    eval_tcl_cmd("update_unit_info %d handle \"%s\"", mapn, infobuf);
    x = unit->x;  y = unit->y;
    location_desc(infobuf, dside, unit, u, x, y);
    eval_tcl_cmd("update_unit_info %d loc \"%s\"", mapn, infobuf);
    /* Very briefly list the numbers and types of the occupants. */
    occupants_desc(infobuf, unit);
    eval_tcl_cmd("update_unit_info %d occ \"%s\"", mapn, infobuf);
    /* Display the "important" parameters. */
    infobuf[0] = '\0';
    /* Size is most important, but only applies to advanced units. */
    if (u_advanced(u)) {
	size_desc(tmpbuf, unit, TRUE);
	strcat(infobuf, tmpbuf);
	strcat(infobuf, "   ");
    }
    /* (should say something about parts?) */
    hp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    strcat(infobuf, "   ");
    acp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    cxp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    morale_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    point_value_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    /* Crude hack, should be replaced with something better */
    if (supply_system_in_use()) 
      sprintf(&infobuf[strlen(infobuf)],
	      "   Supply in flow=%d%% connectedness=%d%%",
	      supply_inflow(unit), supply_connectedness(unit));
    eval_tcl_cmd("update_unit_info %d hp \"%s\"", mapn, infobuf);
#if 0  /* doesn't seem so great in practice */
    if (unit->hp * 4 <= u_hp(unit->type))
      eval_tcl_cmd(".m1.leftside.uf.unitinfo itemconfig hp -fill red");
    else if (unit->hp * 2 <= u_hp(unit->type))
      eval_tcl_cmd(".m1.leftside.uf.unitinfo itemconfig hp -fill orange");
    else
      eval_tcl_cmd(".m1.leftside.uf.unitinfo itemconfig hp -fill black");
#endif
    /* List other stack members here. */
    others_here_desc(infobuf, unit);
    eval_tcl_cmd("update_unit_info %d stack \"%s\"", mapn, infobuf);
    /* Describe the state of all the supplies. */
    for (mrow = 0; mrow < 2; ++mrow) {
	supply_desc(infobuf, unit, mrow);
	eval_tcl_cmd("update_unit_info %d s%d \"%s\"", mapn, mrow, infobuf);
    }
    /* Share the last line between additional supply and tooling. */
    if (!supply_desc(infobuf, unit, 2))
      tooling_desc(infobuf, unit);
    eval_tcl_cmd("update_unit_info %d aux0 \"%s\"", mapn, infobuf);
    /* Describe the current plan and task agenda. */
    plan_desc(infobuf, unit);
    eval_tcl_cmd("update_unit_info %d plan \"%s\"", mapn, infobuf);
    infobuf[0] = '\0';
    task = NULL;
    if (unit->plan)
      task = unit->plan->tasks;
    if (task) {
	task_desc(infobuf, unit->side, unit, task);
	task = task->next;
	if (task) {
	    strcat(infobuf, "; then ");
	    task_desc(infobuf+strlen(infobuf), unit->side, unit, task);
	}
    }
#if (0)
    if (unit->act && ((unit->act->nextaction).type != ACTION_NONE))
      action = &(unit->act->nextaction);
    if (!task && action) {
	action_desc(infobuf, action, unit);
    }
#endif
    eval_tcl_cmd("update_unit_info %d t0 \"%s\"", mapn, infobuf);
}

void
ui_mainloop(void)
{
    Tcl_Time ttime;

    ttime.sec = 0;
    ttime.usec = 1000;
    while (Tk_GetNumMainWindows() > 0) {
    	while(Tcl_DoOneEvent(TCL_ALL_EVENTS));
#if (0)
	/* (Note: Without 'TCL_DONT_WAIT', there seems to be some problems 
	    with some versions of multithreaded Tcl on some systems.) */
    	while(Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT));
#endif
	Tcl_WaitForEvent(&ttime);
    }
}

/* Mandatory Definition: 'run_ui_idler'. Handle some events to make 
   UI response smoother during kernel and AI operations. */

void
run_ui_idler(void)
{
    while(Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT));
}

enum setup_stage last_stage = initial_stage;

/* Stupid hack needed to prevent X server from toppling when more than one 
   peer is being setup on localhost. The problem seems to be that if, 
   for good example, two peers are popping up the variants dialog, and 
   both are querying fonts like mad (to fill out the variants checkboxes), 
   then one of them can become frozen while waiting to get a font 
   (backtrace shows one performing a 'select' on a font file descriptor set, 
   called by 'XLoadQueryFont'). This hack seems to prevent the problem. 
   (If you still have problems and are on an older machine, you may wish to 
   increase the sleep time in 'check_network' if 'Xconq::host_is_localhost' 
   is true.) */

namespace Xconq {
    extern int host_is_localhost;
}

/* Check the network during setup. */

void
check_network(ClientData cldata)
{
    extern int quitter;
    int noreload = FALSE;

    if (hosting || my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	run_ui_idler();
	if (my_rid > 0
	    && my_rid != master_rid
	    && current_stage != last_stage) {
	    /* Hack: prevent client hangs and X11 crashes. */
	    if (Xconq::host_is_localhost)
	      Tcl_Sleep(1000 * (my_rid - 1));
	    if (current_stage == game_load_stage) {
		eval_tcl_cmd("popup_game_dialog");
	    } else if (current_stage == variant_setup_stage) {
		eval_tcl_cmd("popup_variants_dialog");
	    } else if (current_stage == player_setup_stage) {
		eval_tcl_cmd("set_variants");
	    } else if (current_stage == game_ready_stage) {
		eval_tcl_cmd("set_players");
		noreload = TRUE;
	    }
	    last_stage = current_stage;
	}
	if (quitter != 0) {
	    close_remote_connection(quitter);
	    eval_tcl_cmd("insert_chat_string %d %d \"has quit\"",
			 my_rid, quitter);
	    --numremotes;
	    quitter = 0;
	}
    }
    if (!noreload)
      hndl_network_timer = Tcl_CreateTimerHandler(100, check_network, 
						  (ClientData) 0);
}

/* All update_xxx_display callbacks are here. */

void
update_area_display(Side *side)
{
    Map *map;

    if (!active_display(side))
      return;
    for_all_maps(map)
      redraw_map(map);
}

extern int suppress_update_cell_display;

/* Draw an individual detailed hex, as a row of one, on all maps. */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    int dir, x1, y1;
    Map *map;
    VP *vp;

    if (!active_display(side) || suppress_update_cell_display)
      return;
      
    for_all_maps(map) {
	vp = widget_vp(map);
         /* Moved here from update_cell. */
         if (!((flags & UPDATE_ALWAYS)
	  || ((flags & UPDATE_COVER) && vp->draw_cover)
	  || ((flags & UPDATE_TEMP) && vp->draw_temperature)
	  || ((flags & UPDATE_CLOUDS) && vp->draw_clouds)
	  || ((flags & UPDATE_WINDS) && vp->draw_winds)
	  )) {
             continue;
	}
	update_cell(map, x, y);
	/* Disabling both checks below fixed the tcltk update problem.
	Should figure out what the real problem is. */
	if (flags & UPDATE_ADJ) {
	    if ((side->terrview != NULL && vp->hw > 10)
		|| vp->draw_people
		|| vp->draw_control
		|| vp->draw_feature_boundaries
		|| (vp->draw_elevations && numdesigners > 0)
		) {
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)
			/* A totally unseen adjacent cell will have
			   nothing worth redrawing. */
			&& (terrain_view(dside, x1, y1) != UNSEEN
			    || numdesigners > 0)) {
			update_cell(map, x1, y1);
		    }
		}
	    }
	}
    }
}

/* The kernel calls this to update info about the given side. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    int m;
    char sidebuf[BUFSIZE], playerbuf[BUFSIZE];

    if (!active_display(side))
      return;
    if (side2 == NULL)
      return;

    /* Build up and write the textual description of the side. */
    sidebuf[0] = '\0';
    if (is_designer(side2))
      strcat(sidebuf, "(designer)");
    strcat(sidebuf, short_side_title(side2));
    if (side2->willingtodraw) {
	strcat(sidebuf, "[draw]");
    }
    playerbuf[0] = '\0';
    if (side2->player) {
	simple_player_title(playerbuf, side2->player);
    }
    eval_tcl_cmd("update_game_side_info %d {%s} {%s} %d %d %d %d",
		 side2->id, sidebuf, playerbuf, side2->everingame, beforestart, 
		 side2->ingame, side2->status);
    update_side_progress_display(side, side2);
    update_side_score_display(side, side2);
    for_all_material_types(m) {
	if (m_treasury(m)) {
	    eval_tcl_cmd("update_side_treasury %d %d %d",
			 side2->id, mtype_indexes[m], side2->treasury[m]);
	}
    }
    if (nummtypes > 0) {
    	update_material_display();
    }	
}

static int idle_research_notified = FALSE;

void
update_research_display(Side *side)
{
	/* Research is under human control and we need a new topic. */
	if (dside->research_topic == NOADVANCE
	    && dside->ingame
	    && !side_has_ai(dside)
	    && !dside->autoresearch) {
		/* Open the dialog if necessary. */
		if (!research_popped_up) {
			notify(dside, "Your wise men are waiting for a research task.");
			eval_tcl_cmd("popup_research_dialog");
			/* Prevent buzzing due to further redraws. */
			idle_research_notified = TRUE;
		/* Redraw the dialog if already open, but only once. */
		} else if (!idle_research_notified) {
			notify(dside, "Your wise men are waiting for a research task.");
			/* Kick the window manager awake. */
			eval_tcl_cmd("bring_to_front .research ");
			eval_tcl_cmd("fill_research_dialog");
			/* Prevent buzzing due to further redraws. */
			idle_research_notified = TRUE;
		} else {
			eval_tcl_cmd("bring_to_front .research ");
		}
	/* Else just update the dialog (if already open). */
	} else {
		eval_tcl_cmd("fill_research_dialog");
		/* Reset the redraw flag. */
		idle_research_notified = FALSE;
	}
}

/* For sides that are active in the game, update their progress
   display. */

static void
update_side_progress_display(Side *side, Side *side2)
{
    int totacp, percentleft, percentresv, activ;
    extern int curpriority;

    /* No progress display if no game action for this side. */
    if (!side2->ingame || endofgame)
      return;
    activ = FALSE;
    totacp = percentleft = percentresv = 0;
    totacp = side_initacp(side2);
    if (totacp > 0) {
	percentleft = (100 * side_acp(side2)) / totacp;
	percentleft = limitn(0, percentleft, 100);
	percentresv = (100 * side_acp_reserved(side2)) / totacp;
	percentresv = limitn(0, percentresv, percentleft);
    }
    activ = (!g_use_side_priority() || curpriority == side2->priority);
    eval_tcl_cmd("update_side_progress %d %d %d %d %d",
		 side2->id, activ, percentleft, percentresv,
		 side2->finishedturn);
}

static void
update_side_score_display(Side *side, Side *side2)
{
    char *scoredesc;
    Scorekeeper *sk;
    int i;

    if (keeping_score() && side2->everingame) {
	i = 1;
	for_all_scorekeepers(sk) {
	    if (!sk->keepscore)
	      continue;
	    scoredesc = side_score_desc(spbuf, side2, sk);
	    eval_tcl_cmd("update_game_side_score score%d%d \"%s\"",
			 i, side2->id, scoredesc);
	    ++i;
	}
    }
}

/* The kernel calls this to update info about the given unit. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    Map *map;

    if (!active_display(side))
      return;
    if (unit == NULL)
      return;
    if (inside_area(unit->x, unit->y)) {
	update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
    }
    for_all_maps(map) {
	if (unit == map->curunit) {
	    draw_unit_info(map);
	    update_action_controls_info(map);
	}
    }
    if (unit->side != NULL) {
	update_side_progress_display(side, unit->side);
	update_side_score_display(side, unit->side);
    }
    update_unit_type_list(unit->type);
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    /* There isn't really much to skip over from what regular unit
       display does, so just call it. */
    update_unit_display(side, unit, rightnow);
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    Map *map;

    if (!active_display(side))
      return;

    update_unit_type_list(unit->type);

    for_all_maps(map) {
	if (unit == map->curunit)
	  map->scrolled_to_unit = FALSE;
    }
}

/* The kernel calls this to update the global game state. */

void
update_turn_display(Side *side, int rightnow)
{
    Map *map, *curmap;
    Unit *unit;

    if (!active_display(side))
      return;

    eval_tcl_cmd("update_game_state \"%s\"", curdatestr);
    for_all_maps(map) {
	if ((unit = map->curunit) != NULL) {
	    if (inside_area(unit->x, unit->y)) {
		update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
	    }
	    draw_unit_info(map);
	}
    }
    /* Pop up the game over dialog. */
    if ((side_lost(dside) || endofgame) && !told_outcome) {
	eval_tcl_cmd("update_show_all_info %d", dside->may_set_show_all);
	/* Force every map into a see-all/survey mode. */
	/* But first save curmap. */
	curmap = dside->ui->curmap;
	for_all_maps(map) {
	    set_show_all(map, TRUE);
	    redraw_map(map);
	    update_action_controls_info(map);
	    if (map->mode != survey_mode) {
		dside->ui->curmap = map;
		do_survey(dside);
	    }
	}
	/* Now restore curmap. */
	dside->ui->curmap = curmap;
	eval_tcl_cmd("set endofgame 1");
	if (side_won(dside)) {
	    eval_tcl_cmd("popup_game_over_dialog won");
	} else if (side_lost(dside)) {
	    eval_tcl_cmd("popup_game_over_dialog lost");
	} else {
	    eval_tcl_cmd("popup_game_over_dialog over");
	}
	told_outcome = TRUE;
    }
}

void
update_action_display(Side *side, int rightnow)
{
    Map *map;
    Unit *unit;

    if (!active_display(side))
      return;

    for_all_maps(map) {
	if ((unit = map->curunit) != NULL) {
	    if (inside_area(unit->x, unit->y)) {
		update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
	    }
	    draw_unit_info(map);
	}
	update_action_controls_info(map);
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
}

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int now)
{
    Map *map;

    if (!active_display(side))
      return;
    for_all_maps(map) {
	draw_fire_line(map, unit, unit2, 0, 0);
    }
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m,
			 int rightnow)
{
    Map *map;

    if (!active_display(side))
      return;
    for_all_maps(map) {
	draw_fire_line(map, unit, NULL, x, y);
    }
}

/* Updates to clock need to be sure that display changes immediately. */

void
update_clock_display(Side *side, int rightnow)
{
}

void
update_message_display(Side *side, Side *sender, char *str, int rightnow)
{
    if (!active_display(side))
      return;

    notify(side, "From %s: \"%s\"",
	   (sender != NULL ? short_side_title(sender) : "<anon>"), str);
}

void
update_all_progress_displays(char *str, int s)
{
}

void
action_point(Side *side, int x, int y)
{
    Map *map;

    if (!active_display(side))
      return;
    if (!inside_area(x, y))
      return;

    for_all_maps(map) {
	if (map->follow_action && !in_middle(map, x, y)) {
	    put_on_screen(map, x, y);
	    map->scrolled_to_unit = TRUE;
	}
    }
}

void
flush_display_buffers(Side *side)
{
}

void
update_everything()
{
}

/* This is called by generic Unix crash-handling code, no need for
   us to do anything special. */

void
close_displays()
{
}

int
schedule_movie(Side *side, char *movie, ...)
{
    va_list ap;
    int i;
    enum movie_type itype;

    if (!active_display(side))
      return FALSE;
    if (side->ui->numscheduled >= 10)
      return FALSE;
    memset(&(side->ui->movies[side->ui->numscheduled]), 0, sizeof(struct a_movie));
    side->ui->movies[side->ui->numscheduled].type = movie;
    itype = movie_null;
    if (strcmp(movie, "miss") == 0)
      itype = movie_miss;
    else if (strcmp(movie, "hit") == 0)
      itype = movie_hit;
    else if (strcmp(movie, "hit-short") == 0)
      itype = movie_hit_short;
    else if (strcmp(movie, "death") == 0)
      itype = movie_death;
    else if (strcmp(movie, "nuke") == 0)
      itype = movie_nuke;
    else if (strcmp(movie, "sound") == 0)
      itype = movie_sound;
    else if (strcmp(movie, "flash") == 0)
      itype = movie_flash;
    else
      /* Do nothing. */
      return FALSE;
    side->ui->movies[side->ui->numscheduled].itype = itype;
    va_start(ap, movie);
    for (i = 0; i < 5; ++i)
      side->ui->movies[side->ui->numscheduled].args[i] = va_arg(ap, int);
    va_end(ap);
    ++side->ui->numscheduled;
    return TRUE;
}

void
play_movies(SideMask sidemask)
{
    int j, unitid, btype, dur, x, y;
    Map *map;
    Unit *unit;

    if (!active_display(dside))
      return;
    if (!should_play_movies())
      return;
    if (side_in_set(dside, sidemask)) {
	for (j = 0; j < dside->ui->numscheduled; ++j) {
	    btype = -1;
	    dur = 400;
	    switch (dside->ui->movies[j].itype) {
	      case movie_null:
		break;
	      case movie_miss:
		if (btype < 0)
		  btype = 0;
	      case movie_hit:
	      case movie_hit_short:
		if (btype < 0)
		  btype = 1;
		if (dside->ui->movies[j].itype == movie_hit_short)
		  dur = 100;
	      case movie_death:
		if (btype < 0)
		  btype = 2;
		unitid = dside->ui->movies[j].args[0];
		unit = find_unit(unitid);
		if (unit == NULL || !in_area(unit->x, unit->y))
		  continue;
		for_all_maps(map) {
		    draw_unit_blast(map, unit, btype, dur);
		}
		break;
	      case movie_nuke:
		x = dside->ui->movies[j].args[0];
		y = dside->ui->movies[j].args[1];
		for_all_maps(map) {
		    draw_cell_blast(map, x, y, 3, 800);
		}
		break;
	      case movie_flash:
		/* Flashing does the whole screen. */
		for_all_maps(map) {
		    draw_cell_blast(map, 0, 0, 10, 100);
		}
		break;
	      default:
		break;
	    }
	}
	dside->ui->numscheduled = 0;
    }
}

/* Beep the beeper! */

void
beep(Side *side)
{
    eval_tcl_cmd("bell");
}

void
low_notify(Side *side, char *str)
{
    eval_tcl_cmd("low_notify {%s\n}", str);
}

void
popup_game_dialog(void)
{
    eval_tcl_cmd("popup_splash_screen");
}

int *last_num_units_in_play;
int *last_num_units_incomplete;

void
init_redraws(void)
{
    int u;
    Side *side2;
    Map *map;

    if (dside->ui != NULL) {
	/* The moment of truth - up to now output has been suppressed. */
	dside->ui->active = TRUE;
    }

    for_all_sides(side2) {
	update_side_display(dside, side2, TRUE);
    }
    if (last_num_units_in_play == NULL)
      last_num_units_in_play = (int *) xmalloc(numutypes * sizeof(int));
    if (last_num_units_incomplete == NULL)
      last_num_units_incomplete = (int *) xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u) {
	last_num_units_in_play[u] = last_num_units_incomplete[u] = -1;
	init_unit_type_list(u);
	update_unit_type_list(u);
    }
    update_turn_display(dside, TRUE);

    for_all_maps(map)
      set_tool_cursor(map, 0);
}

static void
init_unit_type_list(int u)
{
    int i;
    extern int longest_shortest;
    char *unamestr = NULL;

    if (!between(0, u, numutypes))
      return;
    i = utype_indexes[u];
    /* Only show a char for the unit type if all have single chars,
       otherwise blank all. */
    unamestr = copy_string(shortest_unique_name(u));
    eval_tcl_cmd("update_unitlist_char %d \"%s\"",
		 i, (longest_shortest == 1 ? c_to_tcl_string(&unamestr) : " "));
    /* (should do other columns too) */
    eval_tcl_cmd("update_unitlist_name %d \"%s\"", i, u_type_name(u));
    free(unamestr);

    return;
}

static void
update_unit_type_list(int u)
{
    int i, num;

    if (last_num_units_in_play == NULL)
      last_num_units_in_play = (int *) xmalloc(numutypes * sizeof(int));
    if (last_num_units_incomplete == NULL)
      last_num_units_incomplete = (int *) xmalloc(numutypes * sizeof(int));

    if (!between(0, u, numutypes))
      return;
    i = utype_indexes[u];
    /* Our unit total (right-justified) */
    num = num_units_in_play(dside, u);
    if (num != last_num_units_in_play[u]) {
	spbuf[0] = '\0';
	if (num > 0) {
	    sprintf(spbuf, "%d", num);
	}
	eval_tcl_cmd("update_unitlist_count %d \"%s\"", i, spbuf);
	last_num_units_in_play[u] = num;
    }
    /* Our units under construction (left-justified) */
    num = num_units_incomplete(dside, u);
    if (num != last_num_units_incomplete[u]) {
	spbuf[0] = '\0';
	if (num > 0) {
	    sprintf(spbuf, "(%d)", num);
	}
	eval_tcl_cmd("update_unitlist_incomplete %d \"%s\"", i, spbuf);
	last_num_units_incomplete[u] = num;
    }
    /* (should do other columns too) */
}

static void
update_material_display(void)
{
	int	m;
	int	supply;
	int	production;
	int	storage;

	for_all_material_types(m) {
		eval_tcl_cmd("update_unitlist_material %d \"%s\"", m, 
					m_type_name(m));

		supply = side_material_supply(dside, m);
		eval_tcl_cmd("update_unitlist_supply %d \"%s\"", m, 
					format_number(spbuf, supply));

		production = side_material_production(dside, m);
		eval_tcl_cmd("update_unitlist_production %d \"%s\"", m, 
					format_number(spbuf, production));

		storage = side_material_storage(dside, m);
		if (storage > 0) {
			sprintf(spbuf,"%d%%", (int) (supply * 100.0 / storage));
		} else {
			sprintf(spbuf,"%s","");
		}
		eval_tcl_cmd("update_unitlist_capacity %d \"%s\"", m, spbuf);
	}
}

/* Transfer tcl preference settings to C code preferences. */

int
tk_save_prefs(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char *strval, prefbuf[5000];
    FILE *fp;

    strval = Tcl_GetVar2(interp, "prefs", "power", TCL_GLOBAL_ONLY);
    default_vp.power = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "grid", TCL_GLOBAL_ONLY);
    default_vp.draw_grid = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "coverage", TCL_GLOBAL_ONLY);
    default_vp.draw_cover = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "elevations", TCL_GLOBAL_ONLY);
    default_vp.draw_elevations = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "people", TCL_GLOBAL_ONLY);
    default_vp.draw_people = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "control", TCL_GLOBAL_ONLY);
    default_vp.draw_control = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "temperature", TCL_GLOBAL_ONLY);
    default_vp.draw_temperature = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "winds", TCL_GLOBAL_ONLY);
    default_vp.draw_winds = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "clouds", TCL_GLOBAL_ONLY);
    default_vp.draw_clouds = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "unit_names", TCL_GLOBAL_ONLY);
    default_vp.draw_names = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "feature_names", TCL_GLOBAL_ONLY);
    default_vp.draw_feature_names = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "feature_boundaries", TCL_GLOBAL_ONLY);
    default_vp.draw_feature_boundaries = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "meridians", TCL_GLOBAL_ONLY);
    default_vp.draw_meridians = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "meridian_interval", TCL_GLOBAL_ONLY);
    default_vp.meridian_interval = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "ai", TCL_GLOBAL_ONLY);
    default_vp.draw_ai = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "terrain_images", TCL_GLOBAL_ONLY);
    default_draw_terrain_images = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "terrain_patterns", TCL_GLOBAL_ONLY);
    default_draw_terrain_patterns = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "transitions", TCL_GLOBAL_ONLY);
    default_draw_transitions = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "font_family", TCL_GLOBAL_ONLY);
    default_font_family = copy_string(strval);
    strval = Tcl_GetVar2(interp, "prefs", "font_size", TCL_GLOBAL_ONLY);
    default_font_size = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "listallgames", TCL_GLOBAL_ONLY);
    listallgames = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "want_checkpoints", TCL_GLOBAL_ONLY);
    want_checkpoints = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "checkpoint_interval", TCL_GLOBAL_ONLY);
    checkpoint_interval = strtol(strval, NULL, 10);
    strval = Tcl_GetVar2(interp, "prefs", "joinhost", TCL_GLOBAL_ONLY);
    default_joinhost = strval;
    strval = Tcl_GetVar2(interp, "prefs", "joinport", TCL_GLOBAL_ONLY);
    default_joinport = strval;
    strval = Tcl_GetVar2(interp, "prefs", "myname", TCL_GLOBAL_ONLY);
    default_myname = strval;
    ui_update_state();
    sprintlisp(prefbuf, find_at_key(dside->uidata, "unix"), 5000);
    if ((fp = fopen(preferences_filename(), "w")) != NULL) {
	fputs(prefbuf, fp);
	fputs("\n", fp);
	fclose(fp);
    }
    return TCL_OK;
}

static void
ui_update_state(void)
{
    Obj *state = lispnil;

    push_binding(&state, intern_symbol("default-power"),
		 new_number(default_vp.power));
    push_binding(&state, intern_symbol("default-draw-grid"),
		 new_number(default_vp.draw_grid));
    push_binding(&state, intern_symbol("default-draw-coverage"),
		 new_number(default_vp.draw_cover));
    push_binding(&state, intern_symbol("default-draw-elevations"),
		 new_number(default_vp.draw_elevations));
    push_binding(&state, intern_symbol("default-draw-lighting"),
		 new_number(default_vp.draw_lighting));
    push_binding(&state, intern_symbol("default-draw-people"),
		 new_number(default_vp.draw_people));
    push_binding(&state, intern_symbol("default-draw-control"),
		 new_number(default_vp.draw_control));
    push_binding(&state, intern_symbol("default-draw-temperature"),
		 new_number(default_vp.draw_temperature));
    push_binding(&state, intern_symbol("default-draw-winds"),
		 new_number(default_vp.draw_winds));
    push_binding(&state, intern_symbol("default-draw-clouds"),
		 new_number(default_vp.draw_clouds));
    push_binding(&state, intern_symbol("default-draw-unit-names"),
		 new_number(default_vp.draw_names));
    push_binding(&state, intern_symbol("default-draw-feature-names"),
		 new_number(default_vp.draw_feature_names));
    push_binding(&state, intern_symbol("default-draw-feature-boundaries"),
		 new_number(default_vp.draw_feature_boundaries));
    push_binding(&state, intern_symbol("default-draw-meridians"),
		 new_number(default_vp.draw_meridians));
    push_binding(&state, intern_symbol("default-meridian-interval"),
		 new_number(default_vp.meridian_interval));
    push_binding(&state, intern_symbol("default-draw-ai"),
		 new_number(default_vp.draw_ai));
    push_binding(&state, intern_symbol("default-draw-terrain-images"),
		 new_number(default_draw_terrain_images));
    push_binding(&state, intern_symbol("default-draw-terrain-patterns"),
		 new_number(default_draw_terrain_patterns));
    push_binding(&state, intern_symbol("default-draw-transitions"),
		 new_number(default_draw_transitions));
    push_binding(&state, intern_symbol("default-font-family"),
		 new_string(default_font_family));
    push_binding(&state, intern_symbol("default-font-size"),
		 new_number(default_font_size));
    push_binding(&state, intern_symbol("listallgames"),
		 new_number(listallgames));
    push_binding(&state, intern_symbol("want-checkpoints"),
		 new_number(want_checkpoints));
    push_binding(&state, intern_symbol("checkpoint-interval"),
		 new_number(checkpoint_interval));
    push_binding(&state, intern_symbol("default-joinhost"),
		 new_string(default_joinhost));
    push_binding(&state, intern_symbol("default-joinport"),
		 new_string(default_joinport));
    push_binding(&state, intern_symbol("default-myname"),
		 new_string(default_myname));
  
    dside->uidata = replace_at_key(dside->uidata, "unix", state);
}

static void interp_unix_ui_data(Obj *uispec);

void
get_preferences(void)
{
    int startlineno = 0, endlineno = 0;
    Obj *uispec;
    FILE *fp;

    if ((fp = fopen(preferences_filename(), "r")) != NULL) {
	uispec = read_form(fp, &startlineno, &endlineno);
	interp_unix_ui_data(uispec);
	fclose(fp);
    } else if ((fp = fopen(old_preferences_filename(), "r")) != NULL) {
	uispec = read_form(fp, &startlineno, &endlineno);
	interp_unix_ui_data(uispec);
	fclose(fp);
    }
}

/* Given a list of preference specifications, decipher them and set
   appropriate C variables, also pass into tcl preferences code. */

static void
interp_unix_ui_data(Obj *uispec)
{
    int numval;
    char *strval = NULL;
    char *name;
    Obj *rest, *bdg;

    for_all_list(uispec, rest) {
	bdg = car(rest);
	if (!consp(bdg)) {
	    /* Don't complain out loud normally, confusing to users
	       because preferences are under Xconq and not user
	       control. */
	    Dprintf("Syntax error in preference binding?\n");
	    continue;
	}
	if (symbolp(car(bdg))) {
	    name = c_string(car(bdg));
	    strval = NULL;
	    numval = 0;
	    if (numberp(cadr(bdg))) {
		numval = c_number(cadr(bdg));
	    } else if (stringp(cadr(bdg))) {
		strval = c_string(cadr(bdg));
	    } else {
		Dprintf("Preference property `%s' not a number or string, setting to zero\n",
			name);
	    }
	    if (strcmp(name, "default-power") == 0) {
		default_vp.power = numval;
		eval_tcl_cmd("set_pref_value power %d", numval);
	    } else if (strcmp(name, "default-draw-grid") == 0) {
		default_vp.draw_grid = numval;
		eval_tcl_cmd("set_pref_value grid %d", numval);
	    } else if (strcmp(name, "default-draw-coverage") == 0) {
		default_vp.draw_cover = numval;
		eval_tcl_cmd("set_pref_value coverage %d", numval);
	    } else if (strcmp(name, "default-draw-elevations") == 0) {
		default_vp.draw_cover = numval;
		eval_tcl_cmd("set_pref_value elevations %d", numval);
	    } else if (strcmp(name, "default-draw-lighting") == 0) {
		default_vp.draw_lighting = numval;
		eval_tcl_cmd("set_pref_value lighting %d", numval);
	    } else if (strcmp(name, "default-draw-people") == 0) {
		default_vp.draw_people = numval;
		eval_tcl_cmd("set_pref_value people %d", numval);
	    } else if (strcmp(name, "default-draw-control") == 0) {
		default_vp.draw_control = numval;
		eval_tcl_cmd("set_pref_value control %d", numval);
	    } else if (strcmp(name, "default-draw-temperature") == 0) {
		default_vp.draw_temperature = numval;
		eval_tcl_cmd("set_pref_value temperature %d", numval);
	    } else if (strcmp(name, "default-draw-winds") == 0) {
		default_vp.draw_winds = numval;
		eval_tcl_cmd("set_pref_value winds %d", numval);
	    } else if (strcmp(name, "default-draw-clouds") == 0) {
		default_vp.draw_clouds = numval;
		eval_tcl_cmd("set_pref_value clouds %d", numval);
	    } else if (strcmp(name, "default-draw-unit-names") == 0) {
		default_vp.draw_names = numval;
		eval_tcl_cmd("set_pref_value unit_names %d", numval);
	    } else if (strcmp(name, "default-draw-feature-names") == 0) {
		default_vp.draw_feature_names = numval;
		eval_tcl_cmd("set_pref_value feature_names %d", numval);
	    } else if (strcmp(name, "default-draw-feature-boundaries") == 0) {
		default_vp.draw_feature_boundaries = numval;
		eval_tcl_cmd("set_pref_value feature_boundaries %d", numval);
	    } else if (strcmp(name, "default-draw-meridians") == 0) {
		default_vp.draw_meridians = numval;
		eval_tcl_cmd("set_pref_value meridians %d", numval);
	    } else if (strcmp(name, "default-meridian-interval") == 0) {
		default_vp.meridian_interval = numval;
		eval_tcl_cmd("set_pref_value meridian_interval %d", numval);
	    } else if (strcmp(name, "default-draw-ai") == 0) {
		default_vp.draw_ai = numval;
		eval_tcl_cmd("set_pref_value ai %d", numval);
	    } else if (strcmp(name, "default-draw-terrain-images") == 0) {
		default_draw_terrain_images = numval;
		eval_tcl_cmd("set_pref_value terrain_images %d", numval);
	    } else if (strcmp(name, "default-draw-terrain-patterns") == 0) {
		default_draw_terrain_patterns = numval;
		eval_tcl_cmd("set_pref_value terrain_patterns %d", numval);
	    } else if (strcmp(name, "default-draw-transitions") == 0) {
		default_draw_transitions = numval;
		eval_tcl_cmd("set_pref_value transitions %d", numval);
	    } else if (strcmp(name, "default-font-family") == 0) {
		default_font_family = copy_string(strval);
		eval_tcl_cmd("set_pref_value font_family \"%s\"", strval);
	    } else if (strcmp(name, "default-font-size") == 0) {
		default_font_size = numval;
		eval_tcl_cmd("set_pref_value font_size %d", numval);
	    } else if (strcmp(name, "listallgames") == 0) {
	    	listallgames = numval;
		eval_tcl_cmd("set_pref_value listallgames %d", numval);
	    } else if (strcmp(name, "want-checkpoints") == 0) {
		want_checkpoints = numval;
		eval_tcl_cmd("set_pref_value want_checkpoints %d", numval);
	    } else if (strcmp(name, "checkpoint-interval") == 0) {
		checkpoint_interval = numval;
		eval_tcl_cmd("set_pref_value checkpoint_interval %d", numval);
	    } else if (strcmp(name, "default-joinhost") == 0) {
		default_joinhost = copy_string(strval);
		eval_tcl_cmd("set_pref_value joinhost \"%s\"", strval);
	    } else if (strcmp(name, "default-joinport") == 0) {
		default_joinport = copy_string(strval);
		eval_tcl_cmd("set_pref_value joinport \"%s\"", strval);
	    } else if (strcmp(name, "default-myname") == 0) {
		default_myname = copy_string(strval);
		eval_tcl_cmd("set_pref_value myname \"%s\"", strval);
	    } else {
		/* Note unrecognized properties, but don't bother the user. */
		Dprintf("Preference binding `%s' unrecognized\n", name);
	    }
	} else {
	    /* As with above comment. */
	    Dprintf("Syntax error in preference binding head?\n");
	}
    }
}

void
popup_help(Side *side, HelpNode *node)
{
    eval_tcl_cmd("popup_help_dialog");
}

HelpNode *cur_help_node;

#define NODESTACKSIZE 50

HelpNode **node_stack;

int node_stack_pos;

static void describe_map(int arg, char *key, TextBuffer *buf);

static void
describe_map(int arg, char *key, TextBuffer *buf)
{
    tbcat(buf, "** In MOVE mode (crosshair cursor):\n");
    tbcat(buf, "The next unit that can do anything will be selected ");
    tbcat(buf, "automatically.  It is indicated by a moving marquee.\n");
    tbcat(buf, "Left- or right-click anywhere in the map to move the selected unit there.\n");
    tbcat(buf, "When all units have moved, the turn ends and the next ");
    tbcat(buf, "turn starts immediately.\n");
    tbcat(buf, "\n");
    tbcat(buf, "** In SURVEY mode (magnifying glass cursor):\n");
    tbcat(buf, "Left-click on any of your units to make it the selected one.\n");
    tbcat(buf, "To move it, right-click anywhere in the map to move there; ");
    tbcat(buf, "or use the 'm'ove command, or 'z' to switch ");
    tbcat(buf, "to move mode.\n");
    tbcat(buf, "The turn ends only when you do <ret> or pick the menu item ");
    tbcat(buf, "Side->End This Turn.\n");
    tbcat(buf, "\n");
    tbcat(buf, "** In both modes:\n");
    tbcat(buf, "If the destination is far away, the unit will take as many ");
    tbcat(buf, "turns as it needs to get there.\n");
    tbcat(buf, "If the unit is adjacent to an enemy unit, and you click ");
    tbcat(buf, "on the enemy, your unit will attack.\n");
    tbcat(buf, "\n");
    tbcat(buf, "Nearly all keyboard and menu commands work on only ");
    tbcat(buf, "the selected unit.\n");
    tbcat(buf, "\n");
}

int
tk_init_help_list(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
	HelpNode *tmp;
	
	if (key_commands_help_node == NULL) {
		key_commands_help_node =
		  add_help_node("commands", describe_key_commands, 0, first_help_node);
	}
	if (long_commands_help_node == NULL) {
		long_commands_help_node =
		  add_help_node("long commands", describe_long_commands, 0,
				key_commands_help_node);
	}
	if (map_help_node == NULL) {
		map_help_node = add_help_node("map", describe_map, 0, first_help_node);
	}
	eval_tcl_cmd("add_help_topic_key \"%s\"", first_help_node->key);
	for (tmp = first_help_node->next; tmp != first_help_node; tmp = tmp->next) {
	    eval_tcl_cmd("add_help_topic_key \"%s\"", tmp->key);
	}
	cur_help_node = first_help_node;
	/* Allocate the node stack. */
	if (node_stack == NULL)
	  node_stack =
	    (HelpNode **) xmalloc(NODESTACKSIZE * sizeof(HelpNode *));
	node_stack_pos = 0;
	return TCL_OK;
}

int
tk_init_help_menu(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
	HelpNode *tmp;
	int m;
	
	/* Get the map number. */
	m = strtol(argv[1], NULL, 10);
	
	if (key_commands_help_node == NULL) {
	    key_commands_help_node =
	      add_help_node("commands", describe_key_commands, 0, 
			    first_help_node);
	}
	if (long_commands_help_node == NULL) {
	    long_commands_help_node =
	      add_help_node("long commands", describe_long_commands, 0,
			    key_commands_help_node);
	}
	if (map_help_node == NULL) {
	    map_help_node = add_help_node("map", describe_map, 0, 
					  first_help_node);
	}
	eval_tcl_cmd("add_help_topic_menu \"%d\" \"%s\"", m, 
		     first_help_node->key);
	for (tmp = first_help_node->next; tmp != first_help_node; 
	     tmp = tmp->next) {
	    eval_tcl_cmd("add_help_topic_menu \"%d\" \"%s\"", m, tmp->key);
	}
	cur_help_node = first_help_node;
	/* Allocate the node stack. */
	if (node_stack == NULL)
	  node_stack =
	    (HelpNode **) xmalloc(NODESTACKSIZE * sizeof(HelpNode *));
	node_stack_pos = 0;
	return TCL_OK;
}

extern HelpNode *help_system_node;

int
tk_help_goto(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char *arg = argv[1], *nclassname;
    int i, rslt;
    HelpNode *node, *tmp, *prev;
    TextBuffer tclbuf;
    char intbuf [100];

    /* If this is the first time we came here, build up the table of
       contents. */
    if (cur_help_node == NULL) {
	key_commands_help_node =
	  add_help_node("commands", describe_key_commands, 0, first_help_node);
	long_commands_help_node =
	  add_help_node("long commands", describe_long_commands, 0,
			key_commands_help_node);
	map_help_node =
	add_help_node("map", describe_map, 0, first_help_node);
	eval_tcl_cmd("add_help_topic_key \"%s\"", first_help_node->key);
	for (tmp = first_help_node->next; tmp != first_help_node; 
	     tmp = tmp->next) {
	    eval_tcl_cmd("add_help_topic_key \"%s\"", tmp->key);
	}
	cur_help_node = map_help_node;
	/* Allocate the node stack. */
	if (node_stack == NULL)
	  node_stack =
	    (HelpNode **) xmalloc(NODESTACKSIZE * sizeof(HelpNode *));
	node_stack_pos = 0;
    }
    prev = cur_help_node;
    if (strcmp(arg, "help") == 0) {
	cur_help_node = help_system_node;
    } else if (strcmp(arg, "prev") == 0) {
	cur_help_node = cur_help_node->prev;
    } else if (strcmp(arg, "next") == 0) {
	cur_help_node = cur_help_node->next;
    } else if (strcmp(arg, "back") == 0) {
	/* Pop the last-visited node from the stack. */
	if (node_stack_pos > 0)
	  cur_help_node = node_stack[--node_stack_pos];
    } else {
	node = find_help_node(cur_help_node, arg);
	if (node)
	  cur_help_node = node;
	else
	  beep(dside);
    }
    /* Only add to the stack of nodes visited if we didn't do "back". */
    if (strcmp(arg, "back") != 0) {
	/* If the stack of nodes is full, move them all down by one. */
	if (node_stack_pos >= NODESTACKSIZE) {
	    for (i = 1; i < NODESTACKSIZE; ++i)
	      node_stack[i - 1] = node_stack[i];
	    node_stack_pos = NODESTACKSIZE - 1;
	}
	node_stack[node_stack_pos++] = prev;
    }
    get_help_text(cur_help_node);
    /* Make a string representing the node class. */
    switch (cur_help_node->nclass) {
      case utypenode:
	nclassname = "u";
	break;
      case mtypenode:
	nclassname = "m";
	break;
      case ttypenode:
	nclassname = "t";
	break;
      case atypenode:
	nclassname = "a";
	break;
      default:
	nclassname = "?";
    }
    obstack_begin(&(tclbuf.ostack), 2048);
    obstack_grow(&(tclbuf.ostack), "update_help {", strlen("update_help {"));
    obstack_grow(&(tclbuf.ostack), cur_help_node->key, 
		 strlen(cur_help_node->key));
    obstack_grow(&(tclbuf.ostack), "} {", strlen("} {"));
    obstack_grow(&(tclbuf.ostack), cur_help_node->text, 
		 strlen(cur_help_node->text));
    obstack_grow(&(tclbuf.ostack), "} {", strlen("} {"));
    obstack_grow(&(tclbuf.ostack), nclassname, strlen(nclassname));
    obstack_grow(&(tclbuf.ostack), "} ", strlen("} "));
    sprintf(intbuf, "%d", cur_help_node->arg);
    obstack_grow(&(tclbuf.ostack), intbuf, strlen(intbuf));
    obstack_1grow(&(tclbuf.ostack), 0);
    rslt = Tcl_Eval(interp, (char *)obstack_finish(&(tclbuf.ostack)));
    obstack_free(&(tclbuf.ostack), 0);
#if (0)
    /* Write it all into the buffer going to the Tcl interpreter. */
    sprintf(tclbuf, "update_help {%s} {%s} {%s} %d",
	     cur_help_node->key, cur_help_node->text, nclassname,
	     cur_help_node->arg);
    rslt = Tcl_Eval(interp, tclbuf);
#endif
    if (rslt == TCL_ERROR) {
	fprintf(stderr, "Error: %s\n", interp->result);
	fprintf(stderr, "Error: while updating help node %s\n",
		cur_help_node->key);
    }
    return TCL_OK;
}

/* Given a typed character, decide what to do with it. */

int
tk_interp_key(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int mapn, rawx, rawy, winx, winy, x, y, cancelled;
    Map *map;
    Tk_Window tkwin;
    void (*fn)(Side *sidex, Map *mapx, int cancelledx);

    /* Find the map indicated by the first arg. */
    mapn = strtol(argv[1], NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    map->inpch = *(argv[2]);
    /* Test explicitly for the Return key and substitute \n instead. This is
    necessary for things to work correctly on the Mac. */
    if (strcmp("Return", argv[3]) == 0) {
    	map->inpch = '\n';
    }
    if (strcmp("Escape", argv[3]) == 0) {
    	map->inpch = ESCAPE_CHAR;
    }
    /* Handle scrolling of the map. */
    if (strcmp("Right", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 1 0", map->number);
	return TCL_OK;
    }
    if (strcmp("Left", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 -1 0", map->number);
	return TCL_OK;
    }
    if (strcmp("Up", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 0 -1", map->number);
	return TCL_OK;
    }
    if (strcmp("Down", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 0 1", map->number);
	return TCL_OK;
    }
    /* Diagonal scrolling of the map. */
    if (strcmp("Home", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 -1 -1", map->number);
	return TCL_OK;
    }
    if (strcmp("Prior", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 1 -1", map->number);
	return TCL_OK;
    }
    if (strcmp("Next", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 1 1", map->number);
	return TCL_OK;
    }
    if (strcmp("End", argv[3]) == 0) {
	eval_tcl_cmd("autoscroll %d 0 -1 1", map->number);
	return TCL_OK;
    }
    rawx = strtol(argv[4], NULL, 10);  
    rawy = strtol(argv[5], NULL, 10);
    tkwin = Tk_CoordsToWindow(rawx, rawy, Tk_MainWindow(interp));
    map->inpsx = map->inpsy = -1;
    map->inpx = map->inpy = -1;
    if (tkwin && map->widget != NULL) {
	Tk_GetRootCoords(tkwin, &winx, &winy);
	map->inpsx = rawx - winx;  map->inpsy = rawy - winy;
	if (nearest_cell(widget_vp(map), map->inpsx, map->inpsy,
			 &x, &y, NULL, NULL)) {
	    map->inpx = x;  map->inpy = y;
	}
    }
    DGprintf("key %d %d -> %d %d\n",
	     map->inpsx, map->inpsy, map->inpx, map->inpy);
    /* Call the modal handler if defined, giving it side and cancel
       flag. */
    if (map->modalhandler) {
	fn = map->modalhandler;
	cancelled = (map->inpch == ESCAPE_CHAR);
	/* Remove the handler - will restore itself if needed. */
	map->modalhandler = NULL;
	(*fn)(dside, map, cancelled);
	if (cancelled) {
	    eval_tcl_cmd("clear_command_line %d", map->number);
	    notify(dside, "Cancelled.");
	}
    } else if (isdigit(map->inpch)) {
	/* Digits are assumed to be part of a command prefix, such as
	   a repetition count. */
	if (map->prefixarg < 0) {
	    map->prefixarg = 0;
	} else {
	    map->prefixarg *= 10;
	}
	map->prefixarg += (map->inpch - '0');
	/* Return the prefix arg so we can display it. */
	sprintf(interp->result, "%d", map->prefixarg);
	return TCL_OK;
    } else {
	/* In any other situation, the character is a single-letter
	   command. */
	dside->prefixarg = map->prefixarg;
	dside->ui->beepcount = 0;
	dside->ui->curmap = map;
	execute_command(dside, map->inpch);
    }
    /* Clear the command char, so menu selects and other issuers of
       commands aren't confused with keystroke commands. */
    map->inpch = '\0';
    /* Reset the prefix arg unless there is still more modal input
       to be read. */
    if (map->modalhandler == NULL)
      map->prefixarg = -1;
    /* Return the prefix arg so we can display it. */
    sprintf(interp->result, "%d", map->prefixarg);
    return TCL_OK;
}

int
tk_execute_long_command(ClientData cldata, Tcl_Interp *interp,
			int argc, char *argv[])
{
    int mapn;
    char *cmd = argv[2], *ncmd;
    Map *map;

    /* Find the map indicated by the first arg. */
    mapn = strtol(argv[1], NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    /* Cheap fallback. */
    if (mapn == 0)
      map = dside->ui->maps;
    /* If the long command has a numeric prefix, take it to be the
       prefix argument and peel off.  */
    if (isdigit(*cmd)) {
        map->prefixarg = strtol(cmd, &ncmd, 10);
	cmd = ncmd;
    }
    dside->ui->curmap = map;
    execute_long_command(dside, cmd);
    return TCL_OK;
}

int
tk_game_save(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char *filename = argv[1];

    net_save_game(filename);
    return TCL_OK;
}

int
tk_make_default_player_spec(ClientData cldata, Tcl_Interp *interp,
		   int argc, char *argv[])
{
    char *arg = argv[1];

    default_player_spec = arg;
    return TCL_OK;
}



int
tk_set_design_tool(ClientData cldata, Tcl_Interp *interp,
		   int argc, char *argv[])
{
    char *arg = argv[1];
    Map *map;

    if (strcmp(arg, "normal") == 0) {
	dside->ui->curdesigntool = survey_mode;
    } else if (strcmp(arg, "terrain") == 0) {
	switch (t_subtype(dside->ui->curttype)) {
	  case bordersubtype:
	    dside->ui->curdesigntool = bord_paint_mode;
	    break;
	  case connectionsubtype:
	    dside->ui->curdesigntool = conn_paint_mode;
	    break;
	  case coatingsubtype:
	    dside->ui->curdesigntool = coat_paint_mode;
	    break;
	  default:
	    dside->ui->curdesigntool = cell_paint_mode;
	    break;
	}
    } else if (strcmp(arg, "unit") == 0) {
	dside->ui->curdesigntool = unit_paint_mode;
    } else if (strcmp(arg, "people") == 0) {
	dside->ui->curdesigntool = people_paint_mode;
    } else if (strcmp(arg, "control") == 0) {
	dside->ui->curdesigntool = control_paint_mode;
    } else if (strcmp(arg, "feature") == 0) {
	dside->ui->curdesigntool = feature_paint_mode;
    } else if (strcmp(arg, "material") == 0) {
	dside->ui->curdesigntool = material_paint_mode;
    } else if (strcmp(arg, "elevation") == 0) {
	dside->ui->curdesigntool = elevation_paint_mode;
    } else if (strcmp(arg, "temperature") == 0) {
	dside->ui->curdesigntool = temperature_paint_mode;
    } else if (strcmp(arg, "clouds") == 0) {
	dside->ui->curdesigntool = clouds_paint_mode;
    } else if (strcmp(arg, "winds") == 0) {
	dside->ui->curdesigntool = winds_paint_mode;
    } else if (strcmp(arg, "view") == 0) {
	dside->ui->curdesigntool = view_paint_mode;
    } else {
	run_warning("bogus design tool %s, ignoring\n", arg);
	return TCL_OK;
    }
    for_all_maps(map) {
	map->mode = (enum mapmode)dside->ui->curdesigntool;
	set_tool_cursor(map, 0);
    }
    return TCL_OK;
}

/* This macro implements cycling of a variable through a set of consecutive
   values, with direction controlled by the shift key.  If the limit is 0,
   then the cycling part is not done. */

#define OPTION_CYCLE(var, lo, hi, n, dir)  \
  if ((hi) - (lo) > 0) {  \
    (var) = (((var) + ((dir) < 0 ? -(n) : (n)) - (lo) + ((hi) - (lo))) % ((hi) - (lo))) + (lo);  \
  } else {  \
    (var) = ((var) + ((dir) < 0 ? -(n) : (n)));  \
  }

int
tk_set_design_data(ClientData cldata, Tcl_Interp *interp,
		   int argc, char *argv[])
{
    int set, dir, val;
    char *type = argv[1];
    Map *map;

    dir = 0;
    if (strcmp(argv[2], "incr") == 0) {
	set = FALSE;
	dir = 1;
	if (argc == 4) {
	    dir *= strtol(argv[3], NULL, 10);
	}
    } else if (strcmp(argv[2], "decr") == 0) {
	set = FALSE;
	dir = -1;
	if (argc == 4) {
	    dir *= strtol(argv[3], NULL, 10);
	}
    } else {
	set = TRUE;
	val = strtol(argv[2], NULL, 10);
    }

    if (strcmp(type, "curttype") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curttype, 0, numttypes, 1, dir);
	    val = dside->ui->curttype;
	}
	dside->ui->curttype = val;
	switch (t_subtype(dside->ui->curttype)) {
	  case bordersubtype:
	    dside->ui->curdesigntool = bord_paint_mode;
	    break;
	  case connectionsubtype:
	    dside->ui->curdesigntool = conn_paint_mode;
	    break;
	  case coatingsubtype:
	    dside->ui->curdesigntool = coat_paint_mode;
	    break;
	  default:
	    dside->ui->curdesigntool = cell_paint_mode;
	    break;
	}
    } else if (strcmp(type, "curbgttype") == 0) {
	dside->ui->curbgttype = val;
    } else if (strcmp(type, "curutype") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curutype, 0, numutypes, 1, dir);
	    val = dside->ui->curutype;
	}
	dside->ui->curutype = val;
    } else if (strcmp(type, "curusidenumber") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curusidenumber, 0, numsides + 1, 1, dir);
	    val = dside->ui->curusidenumber;
	}
	dside->ui->curusidenumber = val;
    } else if (strcmp(type, "curpeoplenumber") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curpeoplenumber, 0, numsides + 1, 1, dir);
	    val = dside->ui->curpeoplenumber;
	}
	dside->ui->curpeoplenumber = val;
    } else if (strcmp(type, "curcontrolnumber") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curcontrolnumber, 0, numsides + 1, 1, dir);
	    val = dside->ui->curcontrolnumber;
	}
	dside->ui->curcontrolnumber = val;
    } else if (strcmp(type, "curfid") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curfid, 0, numfeatures, 1, dir);
	    val = dside->ui->curfid;
	}
	dside->ui->curfid = val;
    } else if (strcmp(type, "curelevation") == 0) {
	dside->ui->curelevation = (set ? val : dir);
	dside->ui->curelevationcode = (set ? 0 : 1);
    } else if (strcmp(type, "curelevation_vary") == 0) {
	dside->ui->curelevationvary = val;
    } else if (strcmp(type, "curtemperature") == 0) {
	dside->ui->curtemperature = val;
    } else if (strcmp(type, "curcloudtype") == 0) {
	dside->ui->curcloudtype = val;
    } else if (strcmp(type, "curcloudbottom") == 0) {
	dside->ui->curcloudbottom = val;
    } else if (strcmp(type, "curcloudheight") == 0) {
	dside->ui->curcloudheight = val;
    } else if (strcmp(type, "curwinddir") == 0) {
	dside->ui->curwinddir = val;
    } else if (strcmp(type, "curwindforce") == 0) {
	dside->ui->curwindforce = val;
    } else if (strcmp(type, "curbrushradius") == 0) {
	if (!set) {
	    OPTION_CYCLE(dside->ui->curbrushradius, 0, 11, 1, dir);
	    val = dside->ui->curbrushradius;
	}
	dside->ui->curbrushradius = val;
    }
    for_all_maps(map) {
	set_tool_cursor(map, 0);
    }
    sprintf(interp->result, "%d", val);
    return TCL_OK;
}

int
tk_create_new_feature(ClientData cldata, Tcl_Interp *interp,
		      int argc, char *argv[])
{
    extern int nextfid;
    Feature *feature;

    /* Default to using the next available id as the name.  Since
       deletion of a feature whose desc is a number only doesn't
       work right (menu gets confused), add a letter by default. */
    sprintf(spbuf, "f%d", nextfid);
    feature = net_create_feature("feature", copy_string(spbuf));
    dside->ui->legends = NULL;
    /* (this won't work if networking is on, feature might not
       exist yet) */
    sprintf(interp->result, "%d", (feature ? feature->id : 0));
    return TCL_OK;
}

int
tk_destroy_feature(ClientData cldata, Tcl_Interp *interp,
		      int argc, char *argv[])
{
    int fid;
    Feature *feature;

    fid = strtol(argv[1], NULL, 10);
    feature = find_feature(fid);
    if (feature != NULL)
      net_destroy_feature(feature);
    dside->ui->legends = NULL;
    /* (this won't work if networking is on, feature might not
       exist yet) */
    return TCL_OK;
}

int
tk_designer_fix(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int oldmin = area.minelev, oldmax = area.maxelev;

    fix_elevations();
    update_contour_intervals();
    if (oldmin != area.minelev || oldmax != area.maxelev)
      do_refresh(dside);
    return TCL_OK;
}

int
tk_designer_save(ClientData cldata, Tcl_Interp *interp,
		 int argc, char *argv[])
{
    char *options;
    Module *module;

    module = create_game_module(NULL);
    module->title = "Designer-saved data";
    module->compress_layers = TRUE;
    init_module_reshape(module);

    module->name = copy_string(argv[1]);
    module->filename = copy_string(find_name(argv[2]));
    options = argv[3];
    if (strstr(options, " !compress "))
      module->compress_layers = FALSE;
    if (strstr(options, " all "))
      module->def_all = TRUE;
    if (strstr(options, " types "))
      module->def_types = TRUE;
    if (strstr(options, " tables "))
      module->def_tables = TRUE;
    if (strstr(options, " globals "))
      module->def_globals = TRUE;
    if (strstr(options, " scoring "))
      module->def_scoring = TRUE;
    if (strstr(options, " world "))
      module->def_world = TRUE;
    if (strstr(options, " area "))
      module->def_areas = TRUE;
    if (strstr(options, " terrain "))
      module->def_area_terrain = TRUE;
    if (strstr(options, " areamisc "))
      module->def_area_misc = TRUE;
    if (strstr(options, " weather "))
      module->def_area_weather = TRUE;
    if (strstr(options, " material "))
      module->def_area_material = TRUE;
    if (strstr(options, " sides "))
      module->def_sides = TRUE;
    if (strstr(options, " views "))
      module->def_side_views = TRUE;
    if (strstr(options, " docts "))
      module->def_side_doctrines = TRUE;
    if (strstr(options, " players "))
      module->def_players = TRUE;
#if 0
    if (strstr(options, " agreements "))
      module->def_agreements = TRUE;
#endif
    if (strstr(options, " units "))
      module->def_units = TRUE;
    if (strstr(options, " unitids "))
      module->def_unit_ids = TRUE;
    if (strstr(options, " unitprops "))
      module->def_unit_props = TRUE;
    if (strstr(options, " unitactions "))
      module->def_unit_acts = TRUE;
    if (strstr(options, " unitplans "))
      module->def_unit_plans = TRUE;
    if (strstr(options, " history "))
      module->def_history = TRUE;
    if (!write_game_module(module, copy_string(argv[2])))
      run_warning("Could not write the module \"%s\"!", module->filename);
    return TCL_OK;
}

int
tk_numutypes_available(ClientData cldata, Tcl_Interp *interp,
		       int argc, char *argv[])
{
    int num, u;

    num = 0;
    for_all_unit_types(u) {
	if (utype_indexes[u] >=0)
	  ++num;
    }
    sprintf(interp->result, "%d", num);
    return TCL_OK;
}

int
tk_utype_actual(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int n, u;

    n = strtol(argv[1], NULL, 10);
    for_all_unit_types(u) {
	if (utype_indexes[u] == n) {
	    sprintf(interp->result, "%d", u);
	    return TCL_OK;
	}
    }
    /* (should make error) */
    sprintf(interp->result, "%d", -1);
    return TCL_OK;
}

int
tk_mtype_actual(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int n, m;

    n = strtol(argv[1], NULL, 10);
    for_all_material_types(m) {
	if (mtype_indexes[m] == n) {
	    sprintf(interp->result, "%d", m);
	    return TCL_OK;
	}
    }
    /* (should make error) */
    sprintf(interp->result, "%d", -1);
    return TCL_OK;
}

int
tk_map_size_at_power(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int viewpow;

    viewpow = strtol(argv[1], NULL, 10);
    sprintf(interp->result, "%d %d", area.width * hws[viewpow], 0);
    return TCL_OK;
}

int
tk_center_on_unit(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int mapn, n;
    Map *map;
    Unit *unit;

    /* Find the map indicated by the first arg. */
    mapn = strtol(argv[1], NULL, 10);
    for_all_maps(map) {
	if (map->number == mapn)
	  break;
    }
    /* Cheap fallback. */
    if (mapn == 0)
      map = dside->ui->maps;
    n = find_units_matching(dside, argv[2], &unit);
    if (n == 1 && in_play(unit)) {
	recenter(map, unit->x, unit->y);
    }
    sprintf(interp->result, "%d", n);
    return TCL_OK;
}

int
tk_available_advance(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int n, a, i;

    n = strtol(argv[1], NULL, 10);
    i = 0;
    for_all_advance_types(a) {
	if (side_can_research(dside, a)) {
	    if (i == n) {
                Tcl_SetResult(interp, a_type_name(a), TCL_VOLATILE);
		return TCL_OK;
	    } else {
		++i;
	    }
	}
    }
    Tcl_SetResult(interp, "?", TCL_VOLATILE);
    return TCL_OK;
}

int
tk_advance_needed_to_research(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int this_advance, needed_advance;

    this_advance = strtol(argv[1], NULL, 10);
    needed_advance = strtol(argv[2], NULL, 10);
    if (numatypes > 0 
        && is_advance_type(this_advance)
        && is_advance_type(needed_advance)) {        
          sprintf(interp->result, "%d", aa_needed_to_research(this_advance, needed_advance));
	return TCL_OK;
    } else {
	/* (should make error) */
	sprintf(interp->result, "%d", -1);
	return TCL_OK;
    }
}

int
tk_current_advance(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    if (dside->research_topic == NOADVANCE
        || dside->research_topic == NONATYPE) {
        Tcl_SetResult(interp, "Nothing", TCL_VOLATILE);
	return TCL_OK;
    } else {
        Tcl_SetResult(interp, a_type_name(dside->research_topic), TCL_VOLATILE);
	return TCL_OK;
    }
}

int
tk_set_research_popped_up(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    research_popped_up = strtol(argv[1], NULL, 10);
    return TCL_OK;
}

int
tk_set_side_research(ClientData cldata, Tcl_Interp *interp,
		     int argc, char *argv[])
{
    int n, a, i;

    if (strcmp(argv[1], "nothing") == 0) {
	notify(dside, "Your wise men will rest.");
	net_set_side_research_topic(dside, NONATYPE);
	return TCL_OK;
    }
    n = strtol(argv[1], NULL, 10);
    i = 0;
    for_all_advance_types(a) {
	if (side_can_research(dside, a)) {
	    if (i == n) {
		notify(dside,
		       "Your wise men will research %s.",
		       a_type_name(a));
		net_set_side_research_topic(dside, a);
		return TCL_OK;
	    } else {
		++i;
	    }
	}
    }
    return TCL_OK;
}

#if 0

int
tk_agreements(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    char listbuf[BUFSIZE];
    Agreement *ag;

    listbuf[0] = '\0';
    for_all_agreements(ag) {
	tprintf(listbuf, " %d", ag->id);
    }
    Tcl_SetResult(interp, listbuf, TCL_VOLATILE);
    return TCL_OK;
}

#endif

int
tk_get_scores(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    Tcl_SetResult(interp, get_scores(dside), TCL_VOLATILE);
    return TCL_OK;
}

int
tk_reset_popup_flag(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    error_popped_up = FALSE;
    return TCL_OK;
}

int
tk_set_want_to_exit(ClientData cldata, Tcl_Interp *interp,
		    int argc, char *argv[])
{
    want_to_exit = strtol(argv[1], NULL, 10);
    return TCL_OK;
}

int
tk_exit_xconq(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    exit_xconq(dside);
    return TCL_OK;
}

void
place_legends(Side *side)
{
    int nf = numfeatures;

    if (!features_defined() || nf <= 0)
      return;
    if (side != dside)
      return;
    if (dside->ui->legends == NULL)
      dside->ui->legends = (Legend *) xmalloc((nf + 1) * sizeof(Legend));
    place_feature_legends(dside->ui->legends, nf, side, 0, 1);
}

void
eval_tcl_cmd(char *fmt, ...)
{
    char tclbuf[500], backup[500];
    int rslt;
    va_list ap;

    if (empty_string(fmt))
      return;

    va_start(ap, fmt);
    vsprintf(tclbuf, fmt, ap);
    va_end(ap);
    strcpy(backup, tclbuf);
    rslt = Tcl_GlobalEval(interp, tclbuf);
    if (rslt == TCL_ERROR) {
	fprintf(stderr, "Error: %s\n", interp->result);
	fprintf(stderr, "Error: while evaluating `%s'\n", backup);
    }
}

/* Go through all the game's variants and set up appropriate tcl
   variables. */

static void interpret_checkbox(Variant *var, int which, int n);

void
interpret_variants(void)
{
    int i;
    char *vartypename;
    char *gamename;
    Variant *var;

    any_variants = FALSE;
    vary_world_size = vary_real_time = FALSE;
    /* Set default behavior for all the checkboxes. */
    for (i = 0; i < MAXCHECKBOXES; ++i) {
	eval_tcl_cmd("set varianttext(%d) \"\"", i);
	eval_tcl_cmd("set variantstate(%d) disabled", i);
	eval_tcl_cmd("set varianthelp(%d) nothing", i);
    }
    /* Get the game name and set the corresponding tcl global. */
    if (mainmodule) {
	gamename = (mainmodule->title ? mainmodule->title : mainmodule->name);
	eval_tcl_cmd("set selected_game_title \"%s\"", gamename);
    }
    if (mainmodule == NULL || mainmodule->variants == NULL)
      return;
    /* The first six checkboxes are assigned to common types of
       variants. */
    numcheckboxes = 6;
    for (i = 0; mainmodule->variants[i].id != lispnil; ++i) {
	var = &(mainmodule->variants[i]);
	any_variants = TRUE;
	vartypename = c_string(var->id);
	switch (keyword_code(vartypename)) {
	  case K_WORLD_SEEN:
	    interpret_checkbox(var, i, 0);
	    break;
	  case K_SEE_ALL:
	    interpret_checkbox(var, i, 1);
	    break;
	  case K_SEQUENTIAL:
	    interpret_checkbox(var, i, 2);
	    break;
	  case K_PEOPLE:
	    interpret_checkbox(var, i, 3);
	    break;
	  case K_ECONOMY:
	    interpret_checkbox(var, i, 4);
	    break;
	  case K_SUPPLY:
	    interpret_checkbox(var, i, 5);
	    break;
	  case K_WORLD_SIZE:
	    /* If the area is already set up, it's too late. */
	    /* (this seems wrong?) */
	    if (area.width > 0 || area.height > 0)
	      break;
	    vary_world_size = TRUE;
	    worldsizepos = i;
	    varrev[i] = -1;
	    break;
	  case K_REAL_TIME:
	    vary_real_time = TRUE;
	    realtimepos = i;
	    varrev[i] = -2;
	    break;
	  default:
	    if (numcheckboxes < MAXCHECKBOXES)
	      interpret_checkbox(var, i, numcheckboxes++);
	    else
	      init_warning("too many variants, can't set all of them");
	    break;
	}
	update_variant_setting(i);
    }
    /* Set flags that enable/disable subdialogs. */
    eval_tcl_cmd("set vary_world_size %d", vary_world_size);
    eval_tcl_cmd("set vary_real_time %d", vary_real_time);
}

static void
interpret_checkbox(Variant *var, int which, int n)
{
    eval_tcl_cmd("set variantstate(%d) active", n);
    eval_tcl_cmd("set varianttext(%d) \"%s\"", n, var->name);
    eval_tcl_cmd("set varianthelp(%d) \"%s\"", n, var->help);
    checkboxpos[n] = which;
    varrev[which] = n;
}

static void
init_all_displays(void)
{
    int numdisplays;
    Side *side;

    numdisplays = 0;
    for_all_sides(side) {
	if (side_has_display(side)) {
	    if (side_has_local_display(side))
	      init_display();
	    ++numdisplays;
	}
    }
    if (numdisplays == 0) {
	if (use_stdio)
	  fprintf(stderr, "Must have at least one display to start.\n");
	exit(0);
    }
}

int mapn = 1;

void
create_map(void)
{
    int u;
    Map *map;
    Side *side2;

    DGprintf("Creating map\n");
    map = (Map *) xmalloc(sizeof(Map));

    map->number = mapn++;

    map->mode = move_mode;
    map->autoselect = TRUE;
    map->move_on_click = TRUE;

    map->prefixarg = -1;
    map->inptype = NONUTYPE;

    map->uvec = (short *) xmalloc(numutypes * sizeof(short));
    map->ustr = (char *) xmalloc((numutypes + 1) * sizeof(char));
    map->tvec = (short *) xmalloc(numttypes * sizeof(short));
    map->tstr = (char *) xmalloc((numttypes + 1) * sizeof(char));

    /* Newest map goes on the front of the list. */
    map->next = dside->ui->maps;
    dside->ui->maps = map;
    /* Make this be the current map.  (a little dubious) */
    dside->ui->curmap = map;

    eval_tcl_cmd("create_map_window %d", map->number);

    /* Inherit the side's show_all setting. */
    set_show_all(map, dside->show_all);

    for_all_sides(side2) {
	update_side_display(dside, side2, TRUE);
    }
    if (last_num_units_in_play == NULL)
      last_num_units_in_play = (int *) xmalloc(numutypes * sizeof(int));
    if (last_num_units_incomplete == NULL)
      last_num_units_incomplete = (int *) xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u) {
	last_num_units_in_play[u] = last_num_units_incomplete[u] = -1;
	init_unit_type_list(u);
	update_unit_type_list(u);
    }
    update_turn_display(dside, TRUE);

    set_tool_cursor(map, 0);
    eval_tcl_cmd("update_mode %d move", map->number);
}

static void enable_in_unit_type_list(Side *side, Map *map, int u, int flag);

static void
enable_in_unit_type_list(Side *side, Map *map, int u, int flag)
{
    eval_tcl_cmd("enable_unitlist %d %d %d",
		 map->number, utype_indexes[u], flag);
}

/* Prompt for a type of a unit from player, maybe only allowing some
   types to be accepted.  Also allow specification of no unit type.
   We do this by scanning the vector, building a string of chars and a
   vector of unit types, so as to be able to map back when done. */

int
ask_unit_type(Side *side, Map *map, char *prompt, int *possibles,
	      void (*handler)(Side *side, Map *map, int cancelled))
{
    int u, numtypes = 0;

    for_all_unit_types(u) {
	if (possibles == NULL || possibles[u]) {
	    map->uvec[numtypes] = u;
	    map->ustr[numtypes] = unitchars[u];
	    ++numtypes;
	    enable_in_unit_type_list(side, map, u, 1);
	} else {
	    enable_in_unit_type_list(side, map, u, -1);
	}
    }
    map->ustr[numtypes] = '\0';
    if (numtypes > 1) {
	eval_tcl_cmd("ask_unit_type_mode %d {%s [%s]}",
		     map->number, prompt, map->ustr);
	map->modalhandler = handler;
    }
    return numtypes;
}

/* Do something with the char or unit type that the player entered. */

int
grok_unit_type(Side *side, Map *map, int *typep)
{
    int i, u;

    *typep = NONUTYPE;
    if (map->inptype != NONUTYPE) {
	/* Collect the type saved from a mouse click or other input. */
	*typep = map->inptype;
	/* Reset so doesn't affect subsequent unit type queries. */
	map->inptype = NONUTYPE;
    } else if (map->inpch != '\0') {
	if (map->inpch == '?') {
	    help_unit_type(side, map);
	    return FALSE;
	}
	i = iindex(map->inpch, map->ustr);
	if (i >= 0) {
	    *typep = map->uvec[i];
	} else {
	    notify(side, "Must type a unit type char from the list, or <esc>");
	    return FALSE;
	}
    } else {
	notify(side, "weird");
	return FALSE;
    }
    eval_tcl_cmd("ask_unit_type_done %d", map->number);
    /* Reset the appearance of the unit type list. */
    for_all_unit_types(u) {
	enable_in_unit_type_list(side, map, u, 0);
    }
    /* Make the unit type string be empty. */
    map->ustr[0] = '\0';
    return TRUE;
}

void
cancel_unit_type(Side *side, Map *map)
{
    int u;

    /* Reset the appearance of the unit type list. */
    for_all_unit_types(u) {
	enable_in_unit_type_list(side, map, u, 0);
    }
}

static void
help_unit_type(Side *side, Map *map)
{
    int i;
    char helpbuf[BUFSIZE];

    helpbuf[0] = '\0';
    for (i = 0; map->ustr[i] != '\0'; ++i) {
	/* Put out several types on each line. */
	if (i % 4 == 0) {
	    if (i > 0) {
		notify(side, "%s", helpbuf);
	    }
	    /* Indent each line a bit (also avoids notify's
	       auto-capitalization). */
	    strcpy(helpbuf, "  ");
	}
	tprintf(helpbuf, "%c %s, ", map->ustr[i], u_type_name(map->uvec[i]));
    }
    /* Add an extra helpful comment, then dump any leftovers. */
    tprintf(helpbuf, "? for this help info"); 
    notify(side, "%s", helpbuf);
}

int
ask_terrain_type(Side *side, Map *map, char *prompt, int *possibles,
		 void (*handler)(Side *side, Map *map, int cancelled))
{
    int numtypes = 0, t;

    for_all_terrain_types(t) {
	if (possibles == NULL || possibles[t]) {
	    map->tvec[numtypes] = t;
	    map->tstr[numtypes] = terrchars[t];
	    ++numtypes;
	}
    }
    map->tstr[numtypes] = '\0';
    if (numtypes > 1) {
	eval_tcl_cmd("ask_terrain_type_mode %d {%s [%s]}",
		     map->number, prompt, map->tstr);
	map->modalhandler = handler;
    }
    return numtypes;
}

/* Do something with the char or terrain type that the player entered. */

int
grok_terrain_type(Side *side, Map *map, int *typep)
{
    int i;

    *typep = NONTTYPE;
    if (map->inpch == '?') {
	help_terrain_type(dside, map);
	return FALSE;
    }
    i = iindex(map->inpch, map->tstr);
    if (i >= 0) {
	*typep = map->tvec[i];
	eval_tcl_cmd("ask_terrain_type_done %d", map->number);
	return TRUE;
    } else {
	notify(dside, "Must type a terrain type char or <esc>");
	return FALSE;
    }
}

static void
help_terrain_type(Side *side, Map *map)
{
    int i;
    char helpbuf[BUFSIZE];

    for (i = 0; map->tstr[i] != '\0'; ++i) {
	/* Put out several types on each line. */
	if (i % 4 == 0) {
	    if (i > 0) {
		notify(side, "%s", helpbuf);
	    }
	    /* Indent each line a bit (also avoids confusion due to
	       notify's capitalization). */
	    strcpy(helpbuf, "  ");
	}
	tprintf(helpbuf, "%c %s, ", map->tstr[i], t_type_name(map->tvec[i]));
    }
    /* Add an extra helpful comment, then dump any leftovers. */
    tprintf(helpbuf, "? for this help info"); 
    notify(side, "%s", helpbuf);
}

/* User is asked to pick a position on map.  This will iterate until the
   space bar designates the final position. */

/* (should change the cursor temporarily) */

void
ask_position(Side *side, Map *map, char *prompt,
	     void (*handler)(Side *side, Map *map, int cancel))
{
    eval_tcl_cmd("ask_position_mode %d {%s [click to set]}",
		 map->number, prompt);
    map->answer[0] = '\0';
    map->modalhandler = handler;
}

int
grok_position(Side *side, Map *map, int *xp, int *yp, Unit **unitp)
{
    if (in_area(map->inpx, map->inpy)) {
	*xp = map->inpx;  *yp = map->inpy;
	if (unitp != NULL)
	  *unitp = map->inpunit;
	eval_tcl_cmd("ask_position_done %d", map->number);
	return TRUE;
    } else {
	/* Make any possible usage attempts fail. */
	*xp = *yp = -1;
	if (unitp != NULL)
	  *unitp = NULL;
	return FALSE;
    }
}

/* Prompt for a yes/no answer with a settable default. */

void
ask_bool(Side *side, Map *map, char *question, int dflt,
	 void (*handler)(Side *side, Map *map, int cancelled))
{
    eval_tcl_cmd("ask_bool_mode %d {%s} %d", map->number, question, dflt);
    map->answer[0] = '\0';
    map->tmpint = dflt;
    map->modalhandler = handler;
}

/* Figure out what the answer actually is, keeping the default in mind. */

int
grok_bool(Side *side, Map *map)
{
    int dflt = map->tmpint;
    char ch = map->inpch;

    if (dflt ? (lowercase(ch) == 'n') : (lowercase(ch) == 'y'))
      dflt = !dflt;
    eval_tcl_cmd("ask_bool_done %d", map->number);
    return dflt;
}

/* Read a string from the prompt window.  Deletion is allowed, and a
   text cursor (an underscore) is displayed. */

void
ask_string(Side *side, Map *map, char *prompt, char *dflt,
	   void (*handler)(Side *side, Map *map, int cancelled))
{
    /* Default must be non-NULL. */
    if (dflt == NULL)
      dflt = "";
    sprintf(map->answer, "%s", dflt);
    eval_tcl_cmd("ask_string_mode %d {%s} {%s}",
		 map->number, prompt, map->answer);
    map->modalhandler = handler;
}

/* Dig a character from the input and add it into the string.
   Keep returning FALSE until we get something, then make a copy
   of the result string and return TRUE. */

int
grok_string(Side *side, Map *map, char **strp)
{
    char ch = map->inpch;
    int len;

    /* Note that we can't use '?' for help here, might be a part of
       the string to be returned. */
    if (ch == '\r' || ch == '\n') {
	*strp = copy_string(map->answer);
	eval_tcl_cmd("ask_string_done %d", map->number);
	return TRUE;
    } else {
	len = strlen(map->answer);
	if (ch == BACKSPACE_CHAR || ch == DELETE_CHAR) {
	    if (len > 0)
	      --len;
	} else {
	    map->answer[len++] = ch;
	}
	map->answer[len] = '\0';
	eval_tcl_cmd("update_string_mode %d {%s}", map->number, map->answer);
	return FALSE;
    }
}

void
ask_side(Side *side, Map *map, char *prompt, Side *dfltside,
	 void (*handler)(Side *side, Map *map, int cancelled))
{
    char *dfltstr;

    dfltstr = (dfltside == NULL ? (char *)"nobody" : side_name(dfltside));
    strcpy(map->answer, dfltstr);
    eval_tcl_cmd("ask_side_mode %d {%s} {%s}",
		 map->number, prompt, map->answer);
    map->modalhandler = handler;
}

int
grok_side(Side *side, Map *map, Side **side2p)
{
    char ch = map->inpch;
    int len, sidenum;
    Side *side3;

    *side2p = NULL;
    if (ch == '?') {
	notify(side, "  Type in the name or number of a side,");
	notify(side, "  or \"nobody\" to answer with no side.");
	/* A flag to suppress complaints... */
	*side2p = side;
	return FALSE;
    } else if (ch == '\r' || ch == '\n') {
	if (empty_string(map->answer)
	    || strcmp(map->answer, "nobody") == 0) {
#if 0 /* experimental */
	    *side2p = indepside;
#endif
	    eval_tcl_cmd("ask_side_done %d", map->number);
	    return TRUE;
	}
	if (isdigit(*(map->answer))) {
	    sidenum = strtol(map->answer, NULL, 10);
	    side3 = side_n(sidenum);
	    if (side3) {
		*side2p = side3;
		eval_tcl_cmd("ask_side_done %d", map->number);
		return TRUE;
	    }
	    beep(side);
	    notify(side, "\"%s\" is not a valid side number", map->answer);
	    return FALSE;
	}
	for_all_sides(side3) {
	    if ((!empty_string(side3->name)
		 && strstr(side3->name, map->answer))
		|| (!empty_string(side3->adjective)
		    && strstr(side3->adjective, map->answer))
		|| (!empty_string(side3->pluralnoun)
		    && strstr(side3->pluralnoun, map->answer))
		|| (!empty_string(side3->noun)
		    && strstr(side3->noun, map->answer))) {
		*side2p = side3;
		eval_tcl_cmd("ask_side_done %d", map->number);
		return TRUE;
	    }
	}
	beep(side);
	notify(side, "\"%s\" is not recognized as a side name", map->answer);
	return FALSE;
    } else {
	len = strlen(map->answer);
	if (ch == BACKSPACE_CHAR || ch == DELETE_CHAR) {
	    if (len > 0)
	      --len;
	} else {
	    map->answer[len++] = ch;
	}
	map->answer[len] = '\0';
	eval_tcl_cmd("update_side_mode %d {%s}", map->number, map->answer);
	return FALSE;
    }
}

void
add_remote_locally(int rid, char *str)
{
    eval_tcl_cmd("set master_rid %d", master_rid);
    eval_tcl_cmd("add_program %d %d {%s}", my_rid, rid, str);
}

void
send_chat(int rid, char *str)
{
    /* Add unix linefeeds if necessary. */
    if (!strchr(str, '\n'))
        strcat(str, "\n");
    eval_tcl_cmd("insert_chat_string %d %d {%s}", my_rid, rid, str);
}

/* Given a set of numbers that represent the current desired value of
   a given variant, update the tcl values for that variant (which has
   the side effect of updating the displayed widgets). */

static void
update_variant_setting(int which)
{
    int rev = varrev[which];
    Variant *var = &(mainmodule->variants[which]);

    if (rev == -1) {
	eval_tcl_cmd("set new_width %d", var->newvalues[0]);
	eval_tcl_cmd("set new_height %d", var->newvalues[1]);
	eval_tcl_cmd("set new_circumference %d", var->newvalues[2]);
    } else if (rev == -2) {
	eval_tcl_cmd("set new_time_for_game %d", var->newvalues[0]);
	eval_tcl_cmd("set new_time_per_side %d", var->newvalues[1]);
	eval_tcl_cmd("set new_time_per_turn %d", var->newvalues[2]);
    } else {
	eval_tcl_cmd("set variantvalue(%d) %d", rev, var->newvalues[0]);
    }
}

/* Update the display of the given side/player assignment. */

static void
update_assignment(int n)
{
    eval_tcl_cmd("update_player_entry %d", n);
    eval_tcl_cmd("update_allplayer_buttons");
}

/* Reading is usually pretty fast, so don't do anything special here. */

void
announce_read_progress(void)
{
}

int announced = FALSE;

char *announcemsg = NULL;

/* Announce the start of a time-consuming computation. */

void
announce_lengthy_process(char *msg)
{
    n_seconds_elapsed(0);
    announcemsg = copy_string(msg);
    if (announcemsg && use_stdio) {
	printf("%s;", announcemsg);
	free(announcemsg);
	announcemsg = NULL;
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the making of progress on the computation. */

void
announce_progress(int percentdone)
{
    if (n_seconds_elapsed(2) && use_stdio) {
	printf(" %d%%,", percentdone);
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the end of the time-consuming computation. */

void
finish_lengthy_process(void)
{
    if (announced && use_stdio) {
	printf(" done.\n");
	announced = FALSE;
    }
}

/* An init error needs to have the command re-run. */

void
low_init_error(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Error: %s.\n", str);
	fflush(stderr);
    } else {
	eval_tcl_cmd("popup_init_error_dialog {%s}", str);
    }
    exit(1);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Warning: %s.\n", str);
	fflush(stderr);
    } else {
	eval_tcl_cmd("popup_init_warning_dialog {%s}", str);
    }
}

/* Run errors are fatal, so we try to save state and get out. */

void
low_run_error(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Error: %s.\n", str);
	fflush(stderr);
	fprintf(stderr, "Saving the game...");
	close_displays();
	write_entire_game_state(saved_game_filename());
	fprintf(stderr, " done.\n");
	exit(1);
    }
    error_popped_up = TRUE;
    /* It might be that the game is already over, make sure the
       tcl code knows that. */
    if (endofgame)
      eval_tcl_cmd("set endofgame 1");
    eval_tcl_cmd("popup_run_error_dialog {%s}", str);
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    /* Dump to console, just for the record. */
    if (use_stdio) {
	fprintf(stderr, "Warning: %s.\n", str);
	fflush(stderr);
    }
    error_popped_up = TRUE;
    /* It might be that the game is already over, make sure the
       tcl code knows that. */
    if (endofgame)
      eval_tcl_cmd("set endofgame 1");
    eval_tcl_cmd("popup_run_warning_dialog {%s}", str);
}

/* This should be called before any sort of normal exit. */

void
exit_xconq(Side *side)
{
    if (numremotes > 1 && !ok_to_exit) {
	/* Let the master know we're getting out. */
	send_quit();
    }
    close_displays();
    Tcl_Exit(0);
}

/* Sync the randstate. */

void
sync_randstate(void)
{
    flush_outgoing_queue();
    receive_data(0, MAXPACKETS);
    if ((my_rid > 0) && (my_rid == master_rid)) {
	broadcast_randstate();
    }
    else {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
    }
}

/* Run all the steps that bring a game up once all the players and
   sides have been decided on. */

int
launch_game(void)
{
    Player *player;

    /* Give all unassigned players to the master program. */
    if (numremotes > 0) {
	for_all_players(player) {
	    if (player->rid == 0)
		player->rid = master_rid;
	}
    }
    /* Tell all the other programs it's time to launch too. */
    start_game_ready_stage();
    /* Get rid of the 'check_network' timer. */
    Tcl_DeleteTimerHandler(hndl_network_timer);
    /* Do the time-consuming part of setup calculations. */
    calculate_globals();
    run_synth_methods();
    final_init();
    assign_players_to_sides();
    eval_tcl_cmd("do_initial_setup");
    place_legends(dside);
    /* Get the displays set up, but don't draw anything yet. */
    init_all_displays();
    /* Now bring up the init data on each display. */
    init_redraws();
    /* Set up the signal handlers. */
    init_signal_handlers();
#ifdef UNIX
    init_x_signal_handlers();
#endif
    /* (should notify players of all the game options in effect) */
    notify_instructions();
    return TRUE;
}

void
unit_research_dialog(Unit *unit)
{
    auto_pick_unit_research(unit);
}

void
print_form(Obj *form)
{
#ifdef UNIX
    print_form_and_value(stdout, form);
#else
    print_form_and_value(xcq_fstdout, form);
#endif
}

void
end_printing_forms(void)
{
}

int
tk_withdraw_window(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    if (2 > argc){
        /* TODO: How should we report the error? */
        return TCL_ERROR;
    }
    return withdraw_window(argv[1], interp); 
}

/*
 * Does the equivalent of "wm withdraw" in Tcl/Tk with the additional 
 *  step of destroying the window on X11 to get rid of the BadWindow messages.
 */

int
withdraw_window(const char * windowpath, Tcl_Interp * interp)
{
    Tk_Window tkwin = NULL;
#ifdef UNIX
    Window xid = 0;
#endif

    tkwin = Tk_NameToWindow(interp, (char *)windowpath, Tk_MainWindow(interp));
    if (NULL == tkwin){
        /* Assume that the user may have closed the window.
            ...for now, anyway...
            We may get smarter about this someday.
        */ 
        Dprintf("Could not find window %s.\n", windowpath);
    } else{
#ifdef UNIX
        xid = Tk_WindowId(tkwin);
#endif
        Tk_UnmapWindow(tkwin);
#ifdef UNIX
        Tk_DestroyWindow(tkwin);
        Dprintf("Unmapped window %s with XID %x.\n", windowpath, (unsigned)xid);
#else
        Dprintf("Unmapped window %s.\n", windowpath);
#endif
    }
    return TCL_OK;
}


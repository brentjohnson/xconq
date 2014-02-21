/* Main include file for Xconq.
   Copyright (C) 1991-2000 Stanley T. Shebs.
   Copyright (C) 2003-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/conq.h
 * \brief Main include file for Xconq.
 *
 * This is the include file that nearly all Xconq source files
 * should include (exceptions would be very low-level or generic
 * sources).
 */


/* Elements of bitmask controlling display updates. */

/*! Display Update: always
 */
#define UPDATE_ALWAYS 1

/*! Display Update: adjacent
 */
#define UPDATE_ADJ 2

/*! Display Update: cover
 */
#define UPDATE_COVER 4

/*! Display Update: termperature
 */
#define UPDATE_TEMP 8

/*! Display Update: clouds
 */
#define UPDATE_CLOUDS 16

/*! Display Update: winds
 */
#define UPDATE_WINDS 32

/* The top-level Xconq namespace. */
namespace Xconq {}

#include "config.h"
#include "parambox.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "game.h"
#include "player.h"
#include "side.h"
#include "unit.h"
#include "world.h"
#include "history.h"
#include "score.h"
#include "help.h"
#include "tp.h"
#include "system.h"

/* Some globals. */
#ifndef UNIX
extern FILE *xcq_fstdout;
#endif

/* The several stages of game setup. */

/*! \brief Game setup stages
 *
 * The game startup sequence */
enum setup_stage {
    initial_stage,          		/*!< Initalization */
    game_load_stage,        	/*!< Load Game file(s) */
    variant_setup_stage,    	/*!< Variant setup */
    player_pre_setup_stage,	/*!< Pre player setup */
    player_setup_stage,     	/*!< Player setup */
    game_ready_stage        	/*!< Game ready */
};

/*! \brief Current setup stage */
extern enum setup_stage current_stage;

/* Declarations of globally visible globals. */

/*! \brief Game defined flag. */
extern int gamedefined;
/*! \brief Game initialized flag. */
extern int gameinited;
/*! \brief Game compromised flag. */
extern int compromised;
/*! \brief Game not started flag. */
extern int beforestart;
/*! \brief End of game flag. */
extern int endofgame;
/*! \brief Game in safe state flag. */
extern int gamestatesafe;
/*! \brief Post action scoring to do flag. */
extern int any_post_action_scores;
/*! \brief People side changes? */
extern short any_people_side_changes;
/*! \brief Maximum zone of control range. */
extern int max_zoc_range;
/*! \brief Warnings suppressed flag. */
extern int warnings_suppressed;
/*! \brief Game started in real time flag. */
extern time_t game_start_in_real_time;
/*! \brief Turn play start in real time. */
extern time_t turn_play_start_in_real_time;
/*! \brief Unit posible. */
extern short *u_possible;
/*! \brief We want checkpoint files. */
extern int want_checkpoints;
/*! \brief Checkpoint interval. */
extern int checkpoint_interval;
/*! \brief Statistics wanted. */
extern int statistics_wanted;
/*! \brief Number of sound plays . */
extern int numsoundplays;
/*! \brief All players are designers. */
extern int allbedesigners;
/*! \brief Tempoary event data 1. */
extern int tmphevtdata1;
/*! ??? */
extern int max_detonate_on_approach_range;
/*! \brief Maximum range detonation can affect terrain. */
extern int max_t_detonate_effect_range;
/*! \brief Memory warning flag. */
extern int xmalloc_warnings;
/*! \brief Memory exhausted flag. */
extern int memory_exhausted;
/*! \brief Feature list. */
extern Feature *featurelist;
/*! \brief Doctrine list. */
extern Doctrine *doctrine_list;
/* \brief Current date string. */
extern char *curdatestr;

/*! \brief sun directly over x co-ordinate. */
extern int sunx;
/*! \brief sun directly over y co-ordinate. */
extern int suny;

/*! \brief ??? */
extern int daynight;
/*! \brief ??? */
extern int area_lighting;

/*! \brief Warnings logged. */
extern int warnings_logged;

/* Task Management Flags */
//! Push onto the top of the task agenda.
#define ADD_TO_AGENDA_AS_LIFO	0
//! Enqueue at the end of the task agenda.
#define ADD_TO_AGENDA_AS_FIFO	1
//! Clear task agenda, and set as new agenda item.
#define CLEAR_AGENDA		99

/* Declarations of functions that must be supplied by an interface. */

extern void announce_read_progress(void);
extern void announce_lengthy_process(char *msg);
extern void announce_progress(int pctdone);
extern void finish_lengthy_process(void);

extern Player *add_default_player(void);

extern UnitVector *get_selected_units(Side *side);

extern void init_ui(Side *side);

extern int active_display(Side *side);

extern void run_ui_idler(void);

extern void update_turn_display(Side *side, int rightnow);
extern void update_area_display(Side *side);
extern void update_action_display(Side *side, int rightnow);
extern void update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow);
extern void update_event_display(Side *side, HistEvent *hevt, int rightnow);
extern void update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int rightnow);
extern void update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m, int rightnow);
extern void update_clock_display(Side *side, int rightnow);
extern void update_side_display(Side *side, Side *side2, int rightnow);
extern void update_unit_display(Side *side, Unit *unit, int rightnow);
extern void update_unit_acp_display(Side *side, Unit *unit, int rightnow);
extern void update_message_display(Side *side, Side *side2, char *str, int rightnow);
extern void update_cell_display(Side *side, int x, int y, int flags);
extern void update_all_progress_displays(char *str, int s);
extern void update_research_display(Side *side);
extern void update_everything(void);
extern void flush_display_buffers(Side *side);

extern int schedule_movie(Side *side, char *movie, ...);
extern void play_movies(SideMask sidemask);

extern void action_point(Side *side, int x, int y);

extern void notify_all(char *str, ...);
extern void notify(Side *side, char *str, ...);
extern void vnotify(Side *side, char *fmt, va_list ap);
extern void cmd_error(Side *side, char *fmt, ...);
extern void low_notify(Side *side, char *str);

extern void init_warning(char *str, ...);
extern void low_init_warning(char *str);
extern void init_error(char *str, ...);
extern void low_init_error(char *str);
extern void run_warning(char *str, ...);
extern void low_run_warning(char *str);
extern void run_error(char *str, ...);
extern void low_run_error(char *str);

extern void print_form(Obj *form);
extern void end_printing_forms(void);

extern void unit_research_dialog(Unit *unit);

extern void place_legends(Side *side);

/* Declarations of functions not elsewhere declared. */

/* actions.c */

namespace Xconq {
    //! An action mask can hold 64 types of actions.
    typedef Z64u    ActionMask;
}
//! Cannot toolup for any?
#define ACTM_CANNOT_TOOLUP	    0x1
//! Cannot create any?
#define ACTM_CANNOT_CREATE	    0x2
//! Cannot build upon any?
#define ACTM_CANNOT_BUILD	    0x4
//! Cannot repair any?
#define ACTM_CANNOT_REPAIR	    0x8
// TODO: Define more.

extern void init_actions(void);
extern int transfer_supply(Unit *from, Unit *to, int m, int amount);
extern void garrison_unit(Unit *unit, Unit *unit2);
extern void make_unit_complete(Unit *unit);
extern int execute_action(Unit *unit, Action *action);
extern int can_have_enough_acp(Unit *unit, int acp);
extern int type_can_have_enough_acp(int u, int acp);
extern int has_enough_acp(Unit *unit, int acp);

/* Treasury Access Checking Functions */
//! Could u take m from treasury, assuming side has treasury?
extern int could_take_from_treasury(int u, int m);
//! Could u on a given side take m from treasury?
extern int could_take_from_treasury(int u, Side *side, int m);
//! Could u take m directly from treasury, assuming side has treasury?
extern int could_take_directly_from_treasury(int u, int m);
//! Could u on a given side take m directly from treasury?
extern int could_take_directly_from_treasury(int u, Side *side, int m);
//! Could u give m to treasury, assuming side has treasury?
extern int could_give_to_treasury(int u, int m);
//! Could u on a given side give m to treasury?
extern int could_give_to_treasury(int u, Side *side, int m);
//! Could u give m directly to treasury, assuming side has treasury?
extern int could_give_directly_to_treasury(int u, int m);
//! Could u on a given side give m directly to treasury?
extern int could_give_directly_to_treasury(int u, Side *side, int m);

/* Materials Request/Availability Flags */
//! Use agent's side's treasury rather than actor's side's treasury.
#define MReq_AGENT_TREASURY	1
/* Materials Request/Availability Functions */
//! What are current and max materials availabilities for an actor-agent pair?
extern void get_materials_availability(
    Unit *actor, Unit *agent, long *p_mavail, long *p_mavailmax, int flags = 0);
//! Is materials requirement satisfied by availabilities?
extern int can_meet_materials_requirement(
    long *p_mreq, long *p_mavail, long *p_mavailmax);
//! Consume materials of a given unit with a given consumption getter.
/*!
    Takes a provided TableUM getter function, and applies consumption
    for each relevant material to the given unit or, possibly, side.
    Optionally takes an utype so be used with the getter instead of the
    agent's utype.
    Note that requested amounts can be negative, which will cause a gain
    for the given unit or side treasury, if possible.
*/
extern void consume_materials(
    Unit *actor, Unit *consumer, int (*getter)(int u, int m), 
    int u3 = NONUTYPE, int mult = 1);
//! Consume materials of a given unit according to given consumption vector.
extern void consume_materials(Unit *actor, Unit *consumer, long *p_mreq);

extern int has_supply_to_act(Unit *unit);
extern void use_up_acp(Unit *unit, int acp);
extern int construction_possible(int u2);
extern int any_construction_possible(void);
extern int storage_possible(int m);
extern int any_storage_possible(void);
extern char *action_desig(Action *act);
extern int type_max_night_acp_from_any_terrains(int u);
extern int type_max_acp_from_any_occs(int u);
extern int type_max_acp(int u);
extern int type_max_speed_from_any_occs(int u);
extern int type_max_speed(int u);
extern int can_act(Unit *actor, Unit *agent);

//! Does the constructor have enough tooling to construct u2?
extern int has_enough_tooling(Unit *constructor, int u2);
//! Could u toolup for u2?
extern int could_toolup_for(int u, int u2);
//! Could u toolup for any utype?
extern int could_toolup_for_any(int u);
//! Can given actor-constructor pair toolup for given utype?
extern int can_toolup_for(Unit *actor, Unit *constructor, int u3);

//! Could u create u2?
extern int could_create(int u, int u2);
//! Could u create any utype?
extern int could_create_any(int u);
//! Can given actor-creator pair create given utype?
extern int can_create_common(
    Unit *actor, Unit *creator, int u3, int x, int y,
    int could = 0, int acpcost_spcl = -1, long *p_mreq_spcl = NULL);

//! Could u build u2?
extern int could_build(int u, int u2);
//! Could u build any utype?
extern int could_build_any(int u);
//! Can given actor-builder pair build on a given utype?
extern int can_build(Unit *actor, Unit *builder, int u3);
//! Can given actor-builder pair build on a given unit?
extern int can_build(
    Unit *actor, Unit *builder, Unit *buildee,
    int could = 0, int acpcost_spcl = -1, long *p_mreq_spcl = NULL);

//! Could u auto-repair u2?
/*! \note Does not check hp-recovery mechanism. */
extern int could_auto_repair(int u, int u2);
//! Can given repairer auto-repair given repairee?
/*! \note Includes check of hp-recovery mechanism. */
extern int can_auto_repair(Unit *repairer, Unit *repairee);
//! Could u explicitly repair u2?
extern int could_repair(int u, int u2);
//! Can given actor-repairer pair explicitly repair given utype?
extern int can_repair(Unit *actor, Unit *repairer, int u3);
//! Can given repairer explicitly repair given repairee?
extern int can_repair(Unit *actor, Unit *repairer, Unit *repairee);

extern int can_change_type_to(int u, int u2, Side *side);
extern int can_change_type_to(Unit *actor, Unit *morpher, int u3);
extern int should_play_movies(void);

extern int *cache__type_max_night_acp_from_any_terrains;
extern int *cache__type_max_acp_from_any_occs;
extern int *cache__type_max_acp;
extern int *cache__type_max_speed_from_any_occs;
extern int *cache__type_max_speed;

/* ai.c */

extern void init_ai(Side *side);
extern void init_ai_types(void);
extern void ai_init_shared(void);
extern int find_ai_type(char *aitype);
extern void set_side_ai(Side *side, char *aitype);
extern int run_local_ai(int when, int maxplanning);
extern char *ai_type_name(int n);
extern char *next_ai_type_name(char *aitype);
extern char *ai_type_help(int n);
extern void ai_react_to_new_side(Side *side, Side *side2);
extern void ai_receive_message(Side *side, Side *sender, char *str);
extern void ai_save_state(Side *side);
extern int ai_region_at(Side *side, int x, int y);
extern char *ai_at_desig(Side *side, int x, int y);
extern Goal *create_goal(GoalType type, Side *side, int tf);
extern char *goal_desig(Goal *goal);

extern void ai_pick_side_research(Side *side);

/* combat.c */

//! Chance that u will hit u2 by firing.
extern int fire_hit_chance(int u, int u2);
//! Damage that u does to u2 by firing.
extern int fire_damage(int u, int u2);

//! Chance for u to capture independent u2.
extern int indep_capture_chance(int u, int u2);
//! Chance for u to capture u2 on a give side.
extern int capture_chance(int u, int u2, Side *side2);

//! Could u fire at u2?
extern int could_fire_at(int u, int u2);
//! Could u fire at >= 1 utypes?
extern int could_fire_at_any(int u);
//! Maximum fire range of u against u2.
extern int fire_range(int u, int u2);
//! Best fire range of u against u2.
extern int fire_range_best(int u, int u2);
//! Maximum fire range of u.
extern int fire_range_max(int u);
//! Can a given unit fire?
extern int can_fire(Unit *acpsrc, Unit *firer);
//! Can a given unit fire at a given utype?
extern int can_fire_at(Unit *acpsrc, Unit *firer, int u);
//! Can a given unit fire at >= 1 enemy utypes?
extern int can_fire_at_any(Unit *actor, Unit *firer);

//! Could u hit u2 by attacks?
extern int could_attack(int u, int u2);
//! Could the given utype attack >= 1 utypes?
extern int could_attack_any(int u);
//! Attack range of u against u2.
extern int attack_range(int u, int u2);
//! Maximum attack range of u.
extern int attack_range_max(int u);
//! Can a given unit attack a given utype?
extern int can_attack(Unit *actor, Unit *attacker, int u3);
//! Can a given unit attack >= 1 enemy utypes?
extern int can_attack_any(Unit *actor, Unit *attacker);

//! Could u detonate?
extern int could_detonate(int u);
//! Could u damage u2 by a detonation in same cell?
extern int could_damage_by_0dist_detonation(int u, int u2);
//! Could u damage u2 by a detonation at range?
extern int could_damage_by_ranged_detonation(int u, int u2);
//! Could u damage u2 by a detonation?
extern int could_damage_by_detonation(int u, int u2);
//! Blast radius of detonation from u against u2.
extern int detonate_urange(int u, int u2);
//! Max blast radius of detonation from u.
extern int detonate_urange_max(int u);
//! Max blast radius of detonation from any utype.
extern int detonate_urange_max(void);
//! Can given unit detonate?
/* \todo Remove ACP restrictions. */
extern int can_detonate(Unit *acpsrc, Unit *detonator);

//! Could u hit u2 by any method?
extern int could_hit(int u, int u2);
//! Hit range of u against u2.
extern int hit_range(int u, int u2);
//! Maximum hit range of u.
extern int hit_range_max(int u);
//! Maximum hit range of all utypes.
extern int hit_range_max(void);

//! Could u capture u2 by fire?
extern int could_capture_by_fire(int u, int u2, Side *oside);
//! Could u capture u2 by attacks?
extern int could_capture_by_attacks(int u, int u2, Side *oside);
//! Could u capture u2 by direct capture?
extern int could_capture_by_capture(int u, int u2, Side *oside);
//! Could u capture u2 by any means?
extern int could_capture(int u, int u2, Side *oside);
//! Could the given utype capture any utypes by any means?
extern int could_capture_any(int u);
//! Direct capture range of u against u2.
extern int direct_capture_range(int u, int u2, Side *side2);
//! Can a given unit capture a given utype on a given side?
extern int can_capture(Unit *acpsrc, Unit *captor, int u3, Side *eside);

extern int defended_by_occupants(Unit *unit);
extern int occ_can_defend_transport(int o, int t);
extern int enough_to_garrison(Unit *unit, int defenders);

/* init.c */

extern int G_advances_graph_has_cycles;
extern PackedBoolTable *G_advances_synopsis;

extern void init_library_path(char *path);
extern void add_library_path(char *path);
extern void init_data_structures(void);
extern void start_game_load_stage(void);
extern void start_variant_setup_stage(void);
extern void start_player_pre_setup_stage(void);
extern void start_player_setup_stage(void);
extern void start_game_ready_stage(void);
extern void check_game_validity(void);
extern void calculate_globals(void);
extern void make_trial_assignments(void);
extern Side *make_up_a_side(void);
extern int add_side_and_player(void);
extern int remove_side_and_player(int s);
extern void run_synth_methods(void);
extern void final_init(void);
extern void init_supply(Unit *unit);
extern void assign_players_to_sides(void);
extern char *version_string(void);
extern char *copyright_string(void);
extern char *license_string(void);
extern int already_seen_chance(Side *side, Unit *unit);
extern int get_synth_method_uses(int methkey, int *calls, int *runs);
extern int indepside_needed(void);

/* mknames.c */

extern Obj *make_namer(Obj *sym, Obj *meth);
extern void init_namers(void);
extern void make_up_side_name(Side *side);
extern int name_in_use(Side *side, char *str);
extern int name_units_randomly(int calls, int runs);
extern char *unit_namer(Unit *unit);
extern char *propose_unit_name(Unit *unit);
extern void make_up_unit_name(Unit *unit);
extern void assign_unit_number(Unit *unit);
extern char *run_namer(Obj *namer);

/* mkrivers.c */

extern int make_rivers(int calls, int runs);

/* mkroads.c */

extern int make_roads(int calls, int runs);

/* mkterr.c */

extern int make_fractal_terrain(int calls, int runs);
extern int make_random_terrain(int calls, int runs);
extern int make_earthlike_terrain(int calls, int runs);
extern int make_maze_terrain(int calls, int runs);
extern int name_geographical_features(int calls, int runs);
extern void add_edge_terrain(void);

/* mkunits.c */

extern int make_countries(int calls, int runs);
extern int make_independent_units(int calls, int runs);

/* move.c */

extern int total_move_cost(int u, int u2, int x1, int y1, int z1, int x2, int y2, int z2);
extern int zoc_move_cost(Unit *unit, int ox, int oy, int oz);
extern int zoc_range(Unit *unit, int u2);
extern int unit_blockable_by(Unit *unit, Unit *unit2);
extern int sides_allow_entry(Unit *unit, Unit *transport);
extern int in_blocking_zoc(Unit *unit, int x, int y, int z);
extern int can_move_at_all(Unit *unit);
extern int can_move_via_conn(Unit *unit, int nx, int ny);
extern int unit_speed(Unit *unit, int nx, int ny);
extern int terrain_always_impassable(int u, int t);
extern int type_can_move_in_terrain(int u, int t);
extern int type_can_have_enough_mp(int u, int mp);
extern int could_move(int uactor, int umover);
extern int can_move(Unit *actor, Unit *mover);

/* nlang.c */

extern void init_nlang(void);
extern char *short_side_title(Side *side);
extern char *shortest_side_title(Side *side2, char *buf);
extern char *long_player_title(char *buf, Player *player,
			       char *thisdisplayname);
extern char *short_player_title(char *buf, Player *player,
				char *thisdisplayname);
extern char *simple_player_title(char *buf, Player *player);
extern char *simple_player_name(char *buf, Player *player);
extern char *unit_handle(Side *side, Unit *unit);
extern char *short_unit_handle(Unit *unit);
extern char *medium_long_unit_handle(Unit *unit);
extern void name_or_number(Unit *unit, char *buf);
extern char *apparent_unit_handle(Side *side, Unit *unit, Side *side2);
extern void construction_desc(char *buf, Unit *unit, int u);
extern void research_desc(char *buf, Unit *unit, int a);
extern void researchible_desc(char *buf, Unit *unit, int a);
extern void constructible_desc(char *buf, Side *side, int u, Unit *unit);
extern void historical_event_desc(Side *side, HistEvent *hevt, char *buf);
extern char *action_result_desc(int rslt);
extern void advance_failure_desc(char *buf, Unit *unit, HistEventType reason);
extern char *feature_desc(Feature *feature, char *buf);
extern void size_desc(char *buf, Unit *unit, int label);
extern void hp_desc(char *buf, Unit *unit, int label);
extern void acp_desc(char *buf, Unit *unit, int label);
extern void cxp_desc(char *buf, Unit *unit, int label);
extern void morale_desc(char *buf, Unit *unit, int label);
extern void point_value_desc(char *buf, Unit *unit, int label);
extern void plan_desc(char *buf, Unit *unit);
extern void task_desc(char *buf, Side *side, Unit *unit, Task *task);
extern void action_desc(char *buf, Action *action, Unit *unit);
extern void time_desc(char *buf, int seconds, int maxtime);
extern void notify_doctrine(Side *side, char *spec);
extern void report_combat(Unit *unit, Unit *atker, char *str);
extern void report_give(Side *side, Unit *unit, Unit *unit2, short *rslts);
extern void report_take(Side *side, Unit *unit, int needed, short *rslts);
extern char *ordinal_suffix(int n);
extern char *plural_form(char *word);
extern char *capitalize(char *buf);
extern char *all_capitals(char *buf);
extern char *absolute_date_string(int date);
extern void write_unit_record(FILE *fp, Side *side);
extern void write_side_results(FILE *fp, Side *side);
extern void write_combat_results(FILE *fp, Side *side);
extern void record_missing_image(int typtyp, char *str);
extern int missing_images(char *buf);
extern void notify_all_of_resignation(Side *side, Side *side2);
extern int short_side_title_plural_p(Side *side);
extern char *side_score_desc(char *buf, Side *side, Scorekeeper *sk);
extern void location_desc(char *buf, Side *side, Unit *unit, int u,
			  int x, int y);
extern void destination_desc(char *buf, Side *side, Unit *unit,
			     int x, int y, int z);
extern int supply_desc(char *buf, Unit *unit, int mrow);
extern int tooling_desc(char *buf, Unit *unit);
extern char *sidemask_desc(char *buf, SideMask sidemask);
extern void latlong_desc(char *buf, int x, int y, int xf, int yf, int which);
extern void others_here_desc(char *buf, Unit *unit);
extern void occupants_desc(char *buf, Unit *unit);
extern void set_initial_date(char *str);
extern int find_event_type(Obj *sym);
extern int pattern_matches_event(Obj *pattern, HistEvent *hevt);
extern void event_desc_from_list(Side *side, Obj *lis, HistEvent *hevt,
				 char *buf);
extern int turns_between(char *str1, char *str2);
extern char *goal_desc(char *buf, Goal *goal);
extern char *feature_name_at(int x, int y);
extern void linear_desc(char *buf, int x, int y);
extern void elevation_desc(char *buf, int x, int y);
extern void temperature_desc(char *buf, int x, int y);
extern char *format_number(char *buf, int value);

/* plan.c */

	/* called by ais - should not be in plan.c. */

extern int ai_victim_here(int x, int y, int *numvictims);
extern int target_here(int x, int y);
extern int useful_captureable_here(int x, int y);
extern int est_completion_time(Unit *unit, int u2);
extern char *plan_desig(Plan *plan);
extern int low_on_supplies_one(Unit *unit);
extern int low_on_ammo_one(Unit *unit);

	/* called by take_supplies in ui.c (should not be in ui.c). */

extern int past_halfway_point(Unit *unit);

	/* called by skelconq which should not do that. */

extern void make_plausible_random_args(char *argtypestr, int i, int *args, Unit *unit);

/* read.c */

extern void type_error(Obj *x, char *msg);
extern void interp_form(Module *module, Obj *form);
extern void interp_game_module(Obj *form, Module *module);
extern void fill_in_side(Side *side, Obj *props, int userdata);
extern void read_warning(char *str, ...);
extern int utype_from_name(char *str);
extern int utype_from_symbol(Obj *sym);
extern int mtype_from_name(char *str);
extern int ttype_from_name(char *str);
extern int atype_from_name(char *str);
extern int lookup_task_type(char *name);

/* run.c */

extern void init_run(void);
extern int run_game(int maxactions);
extern int realtime_game(void);
extern int all_others_willing_to_save(Side *side);
extern int all_others_willing_to_quit(Side *side);
extern void end_the_game(void);
extern void update_canresearch_vector(Side *side);
extern void update_side_research_goal_availability(Side *side);
extern void update_canbuild_vector(Side *side);
extern void update_cancarry_vector(Side *side);
extern void kick_out_enemy_users(Side *side, int x, int y);
extern void set_play_rate(int slow, int fast);
extern int production_at(int x, int y, int m);
extern void auto_pick_unit_research(Unit *unit);
extern void auto_pick_side_research(Side *side);

/* run2.c */

extern int new_acp_for_turn(Unit *unit);
extern int new_acp_for_turn_if_at(Unit *unit, int x, int y);
extern int people_surrender_chance(int u, int x, int y);
extern void update_cell_display_all_sides(int x, int y, int flags);
extern void try_sharing(Unit *from, Unit *to, int m);
extern int damaged_acp(Unit *unit, Obj *effect);
extern void set_unit_acp_for_turn(Unit *unit);
extern int people_surrender_chance(int u, int x, int y);
extern int choose_move_direction(Unit *unit, int x, int y, int range);
extern int choose_move_dirs(Unit *unit, int tx, int ty, int shortest, int (*dirtest)(Unit *, int), void (*dirsort)(Unit *, int *, int), int *dirs);
extern int plausible_move_dir(Unit *unit, int dir);
extern void parse_player_spec(Player *player, char *spec);

/* side.c */

extern UnitView *find_unit_view(Side *side, Unit *unit);
extern Obj * choose_side_research_goal_from_weighted_list(Obj *lis, 
							  int *totalweightp,
							  Side *side);

/* supply.c */

extern void run_supply(void);
extern int supply_system_in_use(void);

/* task.c */

//! Do not hit via any method?
#define HIT_METHOD_NONE		0x0
//! Hit via attack?
#define HIT_METHOD_ATTACK	0x1
//! Hit via fire?
#define HIT_METHOD_FIRE		0x2
//! Hit via detonation?
#define HIT_METHOD_DETONATE	0x4
//! Hit via transport?
#define HIT_METHOD_TRANSPORT	0x8
//! Hit via occupants?
#define HIT_METHOD_OCCUPANT	0x10

//! Hit via any method?
#define HIT_METHOD_ANY	\
    (HIT_METHOD_ATTACK | HIT_METHOD_FIRE | HIT_METHOD_DETONATE \
     | HIT_METHOD_TRANSPORT | HIT_METHOD_OCCUPANT)

//! Default number of times ot run 'hit-unit' task.
#define HIT_TRIES_DFLT	3

//! Do not capture via any method?
#define CAPTURE_METHOD_NONE		0x0
//! Capture via direct capture?
#define CAPTURE_METHOD_CAPTURE		0x1
//! Capture via attack?
#define CAPTURE_METHOD_ATTACK		0x2
//! Capture via adjacent fire?
#define CAPTURE_METHOD_FIRE		0x4
//! Capture via detonation?
#define CAPTURE_METHOD_DETONATE		0x8
//! Capture via transport?
#define CAPTURE_METHOD_TRANSPORT	0x16
//! Capture via occupants?
#define CAPTURE_METHOD_OCCUPANT		0x32

//! Capture via any method?
#define CAPTURE_METHOD_ANY	\
    (CAPTURE_METHOD_CAPTURE \
     | CAPTURE_METHOD_ATTACK | CAPTURE_METHOD_FIRE | CAPTURE_METHOD_DETONATE \
     | CAPTURE_METHOD_TRANSPORT | CAPTURE_METHOD_OCCUPANT)

//! Default number of times ot run 'capture' task.
#define CAPTURE_TRIES_DFLT	3

	/* called by the ais - should not be in task.c. */

extern Unit *find_unit_to_complete(Unit *unit, Task *task);

	/* called by skelconq - which should not do that. */

extern Task *create_task(TaskType type);

/* tp.c */

extern void dump_checksums(char *str);

/* write.c */

extern void init_write(void);
extern int write_game_module(Module *module, char *fname);

/* Declarations of functions only visible to Xconq kernel.
   Copyright (C) 1996-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/kernel.h
 * This file contains declarations used by the kernel.
 * \todo Add to this when understood better.
 */

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,PREPFN,netprepfn,dofn,checkfn,ARGDECL,doc)  \
  extern int PREPFN ARGDECL;

#include "action.def"

#undef  DEF_TASK
#define DEF_TASK(name,dname,code,argtypes,dofn,CREATEFN,SETFN,netsetfn,PUSHFN,netpushfn,ARGDECL)  \
  extern void SETFN ARGDECL;  \
  extern void PUSHFN ARGDECL;

#include "task.def"

/* Used by the repair code. */

enum repairmode {
    NO_REPAIR,
    SELF_REPAIR,
    HP_RECOVERY,
    AUTO_REPAIR
};

namespace Xconq {
    //! Any hp-recovery in game?
    extern short any_hp_recovery;
    //! Any auto-repair in game?
    extern short any_auto_repair;
    //! Could utype be auto-repaired by at least one utype?
    extern short *cv__could_be_auto_repaired;
    //! Out to what range could utype be auto-repaired?
    extern short *cv__auto_repaired_range_max;
}

/* Used by the resupply code. */

extern int *lowm;
extern int numlow; 

/* Used by the auto upgrade code. */

extern short any_auto_change_types;

/* Used by the wrecking code. */

typedef enum {
    WRECK_TYPE_UNSPECIFIED = 0,
    WRECK_TYPE_KILLED,
    WRECK_TYPE_STARVED,
    WRECK_TYPE_ACCIDENT,
    WRECK_TYPE_ATTRITED,
    WRECK_TYPE_TERRAIN,
    WRECK_TYPE_SIDE_LOST
} WreckType;

/* Used by any code that deals with automatic reactions to a change, such as 
   an unit entering a cell. */

namespace Xconq {
    extern int suppress_reactions;
}

/* combat.c */

extern void rescue_occupants(Unit *unit);
extern void reckon_damage_around(int x, int y, int r, Unit *detonator);
extern void damage_unit(Unit *unit, enum damage_reasons reason, Unit *agent);
extern void capture_unit(Unit *unit, Unit *pris, int captype);
extern int detonate_unit(Unit *unit, int x, int y, int z);

/* economy.c */

extern void run_economy(void);
extern void run_all_consumption(void);

/* init.c */

extern void set_player_advantage(int n, int newadv);
extern void rename_side_for_player(int n, int which);
extern void set_ai_for_player(int n, const char *aitype);
extern int exchange_players(int n, int n2);

/* mkterr.c */

extern void set_edge_terrain(int doterr);

/* module.c */

extern void set_variant_value(int which, int v1, int v2, int v3);

/* move.c */

extern int move_unit(Unit *unit, int nx, int ny);
extern void change_control_side_around(int x, int y, int u, Side *side);
extern int has_enough_mp(Unit *unit, Unit *unit2, int mp);
extern int can_have_enough_mp(Unit *unit, Unit *unit2, int mp);

/* plan.c */

extern void set_unit_plan_type(Side *side, Unit *unit, int type);
extern void set_unit_asleep(Side *side, Unit *unit, int flag, int recurse);
extern void set_unit_reserve(Side *side, Unit *unit, int flag, int recurse);
extern void set_unit_ai_control(Side *side, Unit *unit, int flag, int recurse);
extern void set_unit_main_goal(Side *side, Unit *unit, Goal *goal);
extern void set_unit_curadvance(Side *side, Unit *unit, int a);
extern void set_unit_researchdone(Side *side, Unit *unit, int flag);
extern void set_unit_waiting_for_transport(Side *side, Unit *unit, int flag);
extern void wake_unit(Side *side, Unit *unit, int forcewakeoccs);
extern void selectively_wake_unit(Side *side, Unit *unit, int wakeoccs, 
				  int forcewakeoccs);
extern void wake_area(Side *side, int x, int y, int n, int occs);
extern void set_formation(Unit *unit, Unit *leader, int x, int y, int dist, int flex);
extern void delay_unit(Unit *unit, int flag);
extern int clear_task_agenda(Unit *unit);
extern void clear_task_outcome(Unit *unit);
extern void force_replan(Unit *unit);
extern int execute_plan(Unit *unit);
extern int is_in_formation(Unit *unit);
extern int move_into_formation(Unit *unit);
extern void free_plan(Unit *unit);
extern int execute_standing_order(Unit *unit, int addtask);
extern void set_waiting_for_tasks(Unit *unit, int flag);
extern void pop_task(Plan *plan);
extern Task * find_unit_task_by_type(Unit *unit, TaskType tt);

/* run.c */

extern void finish_turn(Side *side);
extern void resign_game(Side *side, Side *side2);

/* run2.c */

extern void change_people_side_around(int x, int y, int u, Side *side);
extern void maybe_starve(Unit *unit, int partial);
extern void request_additional_side(const char *playerspec);
extern int total_acp_for_turn(Unit *unit);

/* Functions in run2.c, only called from run.c. */

extern void run_people(void);
extern void run_advanced_units(void);
extern void run_turn_start(void);
extern void run_restored_turn_start(void);
extern void run_turn_end(void);

/* side.c */

extern void maybe_track(Unit *unit);
extern void maybe_lose_track(Unit *unit, int nx, int ny);
extern UnitView *add_unit_view(Side *side, Unit *unit);
extern void add_unit_view_raw(Side *side, UnitView *uview, int x, int y);
extern void flush_stale_views(void);

extern void set_side_name(Side *side, Side *side2, char *newname);
extern void set_side_longname(Side *side, Side *side2, char *newname);
extern void set_side_shortname(Side *side, Side *side2, char *newname);
extern void set_side_noun(Side *side, Side *side2, char *newname);
extern void set_side_pluralnoun(Side *side, Side *side2, char *newname);
extern void set_side_adjective(Side *side, Side *side2, char *newname);
extern void set_side_emblemname(Side *side, Side *side2, const char *newname);
extern void set_side_colorscheme(Side *side, Side *side2, char *newname);
extern void become_designer(Side *side);
extern void become_nondesigner(Side *side);
extern void set_trust(Side *side, Side *side2, int val);
extern void set_autofinish(Side *side, int value);
extern void set_autoresearch(Side *side, int value);
extern void set_willing_to_save(Side *side, int flag);
extern void set_willing_to_draw(Side *side, int flag);
extern void send_message(Side *side, SideMask sidemask, const char *str);
extern void set_side_self_unit(Side *side, struct a_unit *unit);
extern void set_doctrine(Side *side, const char *spec);
extern void set_controlled_by(Side *side, Side *side2, int val);
extern void set_side_research_topic(Side *side, int a);
extern void set_side_research_goal(Side *side, int a);
extern void set_side_startx(Side *side, int x);
extern void set_side_starty(Side *side, int y);
#ifdef DESIGNERS
extern void paint_view(Side *side, int x, int y, int r, int tview, int uview);
#endif /* DESIGNERS */

/* task.c */

extern void init_tasks(void);
extern TaskOutcome execute_task(Unit *unit);
extern int target_visible(Unit *unit, Task *task);
extern int direct_access_to(int x, int y);

extern void resume_build_task(Unit *unit, Unit *unit2, int run, int x, int y);
extern void add_task(Unit *unit, int pos, Task *task);
extern Task *clone_task(Task *oldtask);

extern void sort_directions(Unit *unit, int *dirs, int numdirs);
extern void free_task(Task *task);
extern const char *parse_task(Side *side, const char *str, Task **taskp);
extern char *task_desig(Task *task);

extern int can_resupply_from_here(int x, int y);
extern int can_auto_resupply_self(Unit *unit, int *materials, int numtypes);
extern int can_repair_from_here(int x, int y);

/* tp.c */

		/* called in init.c. */

extern void init_remote_ui(Side *side);
extern void add_remote_players(void);
extern void broadcast_start_game_load(void);
extern void broadcast_start_variant_setup(void);
extern void broadcast_variants_chosen(void);
extern void broadcast_start_player_setup(void);
extern void broadcast_players_assigned(void);

		/* called in write.c. */

extern void add_to_packet(const char *str);
extern void flush_write(void);

/* unit.c */

extern int can_be_actor(Unit *unit);
extern int change_cell(Unit *unit, int x, int y);
extern void wreck_unit(Unit *unit, HistEventType hevttype, 
		       WreckType wrecktype, int wreckarg, Unit *agent);
extern void change_unit_side(Unit *unit, Side *newside, int reason, Unit *agent);
extern void set_unit_side(Unit *unit, Side *side);
extern void set_unit_origside(Unit *unit, Side *side);
extern void set_unit_name(Side *side, Unit *unit, char *newname);
extern int disband_unit(Side *side, Unit *unit);
extern void kill_unit(Unit *unit, int reason);
extern void change_morale(Unit *unit, int sign, int morchange);
#ifdef DESIGNERS
extern Unit *designer_create_unit(Side *side, int u, int s, int x, int y);
extern int designer_teleport(Unit *unit, int x, int y, Unit *other);
extern int designer_change_side(Unit *unit, Side *side);
extern int designer_disband(Unit *unit);
#endif /* DESIGNERS */
extern void init_units(void);
extern Unit *create_bare_unit(int type);
extern Unit *create_unit(int type, int makebrains);
extern void init_unit_actorstate(Unit *unit, int flagacp, int acp = 0);
extern void init_unit_plan(Unit *unit);
extern void init_unit_tooling(Unit *unit);
extern void init_unit_opinions(Unit *unit, int nsides);
extern void init_unit_extras(Unit *unit);
extern void change_unit_type(Unit *unit, int newtype, int reason, 
			     Side *newside);
extern int enter_cell(Unit *unit, int x, int y);
extern void enter_transport(Unit *unit, Unit *transport);
extern void leave_cell(Unit *unit);
extern void leave_transport(Unit *unit);
extern void flush_dead_units(void);
extern void add_to_unit_hp(Unit *unit, int hp);
extern void fill_utype_array_from_lisp(int *typeary, Obj *typeobj);

/* world.c */

extern void divide_into_regions(char *tlayer, TRegion **rlayer, int liquid);
extern Feature *create_feature(const char *feattype, const char *name);
extern void set_feature_type_name(Feature *feature, char *feattype);
extern void set_feature_name(Feature *feature, char *name);
extern void destroy_feature(Feature *feature);
extern void renumber_features(void);
#ifdef DESIGNERS
extern void paint_cell(Side *side, int x, int y, int r, int t);
extern void paint_border(Side *side, int x, int y, int dir, int t, int mode);
extern void paint_connection(Side *side, int x, int y, int dir, int t, int mode);
extern void paint_coating(Side *side, int x, int y, int r, int t, int depth);
extern void paint_people(Side *side, int x, int y, int r, int s);
extern void paint_control(Side *side, int x, int y, int r, int s);
extern void paint_feature(Side *side, int x, int y, int r, int f);
extern void paint_elevation(Side *side, int x, int y, int r, int code, int elev, int vary);
extern void paint_temperature(Side *side, int x, int y, int r, int temp);
extern void paint_material(Side *side, int x, int y, int r, int m, int amt);
extern void paint_clouds(Side *side, int x, int y, int r, int cloudtype, int bot, int hgt);
extern void paint_winds(Side *side, int x, int y, int r, int dir, int force);
#endif /* DESIGNERS */
extern void toggle_user_at(Unit *unit, int x, int y);

/* Miscellany */

/* Can a given type on a given side consume a given material from the
   side's treasury directly? */
#define type_consumes_from_treasury_directly(utype, side, m) \
    (side_has_treasury((side), (m)) && um_takes_from_treasury((utype), (m)) \
     && (um_storage_x((utype), (m)) <= 0))

/* Can a given unit consume a given material from its side's treasury 
   directly? */
#define consumes_from_treasury_directly(unit, m) \
    (side_has_treasury((unit)->side, (m)) \
     && um_takes_from_treasury((unit)->type, (m)) \
     && (um_storage_x((unit)->type, (m)) <= 0))

/* Can a given type on a given side produce a given material for the 
   side's treasury directly? */
#define type_produces_for_treasury_directly(utype, side, m) \
    (side_has_treasury((side), (m)) && um_gives_to_treasury((utype), (m)) \
    && (um_storage_x((utype), (m)) <= 0))

/* Can a given unit produce a given material for its side's treasury
   directly? */
#define produces_for_treasury_directly(unit, m) \
    (side_has_treasury((unit)->side, (m)) \
    && um_gives_to_treasury((unit)->type, (m)) \
    && (um_storage_x((unit)->type, (m)) <= 0))


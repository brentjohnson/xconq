/* Definitions of functions visible to interfaces in Xconq.
   Copyright (C) 1996-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* (should doublecheck that all of the decls in this file either have
   impls or are removed) */

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,prepfn,NETPREPFN,dofn,checkfn,ARGDECL,doc)  \
  extern int NETPREPFN ARGDECL;

#include "action.def"

#undef  DEF_TASK
#define DEF_TASK(name,dname,code,argtypes,dofn,createfn,setfn,NETSETFN,pushfn,NETPUSHFN,ARGDECL)  \
  extern void NETSETFN ARGDECL;  \
  extern void NETPUSHFN ARGDECL;

#include "task.def"

/* Max number of packets received by each call to receive_data. */

#define MAXPACKETS 32767

/* File names below refer to locations of the kernel core functions. */

/* ai.c */

extern void net_set_side_ai(Side *side, const char *aitype);

/* module.c */

extern void net_set_variant_value(int which, int v1, int v2, int v3);

/* init.c */

extern int net_add_side_and_player(void);
extern int net_remove_side_and_player(int s);
extern void net_set_player_advantage(int n, int newadv);
extern void net_rename_side_for_player(int n, int which);
extern void net_set_ai_for_player(int n, const char *aitype);
extern int net_exchange_players(int n, int n2);

/* run2.c */

extern void net_request_additional_side(char *playerspec);

/* run.c */

extern int net_run_game(int maxactions);
extern void net_finish_turn(Side *side);
extern void net_resign_game(Side *side, Side *side2);

/* side.c */

extern void net_set_side_name(Side *side, Side *side2, char *newname);
extern void net_set_side_longname(Side *side, Side *side2, char *newname);
extern void net_set_side_shortname(Side *side, Side *side2, char *newname);
extern void net_set_side_noun(Side *side, Side *side2, char *newname);
extern void net_set_side_pluralnoun(Side *side, Side *side2, char *newname);
extern void net_set_side_adjective(Side *side, Side *side2, char *newname);
extern void net_set_side_emblemname(Side *side, Side *side2, const char *newname);
extern void net_set_side_colorscheme(Side *side, Side *side2, char *newname);
extern void net_become_designer(Side *side);
extern void net_become_nondesigner(Side *side);
extern void net_set_trust(Side *side, Side *side2, int val);
extern void net_set_autofinish(Side *side, int value);
extern void net_set_autoresearch(Side *side, int value);
extern void net_set_willing_to_save(Side *side, int flag);
extern void net_set_willing_to_draw(Side *side, int flag);
extern void net_send_message(Side *side, SideMask sidemask, const char *str);
extern void net_set_side_self_unit(Side *side, struct a_unit *unit);
extern void net_set_doctrine(Side *side, const char *spec);
extern void net_set_controlled_by(Side *side, Side *side2, int val);
extern void net_set_side_research_topic(Side *side, int a);
extern void net_set_side_research_goal(Side *side, int a);
extern void net_set_side_startx(Side *side, int x);
extern void net_set_side_starty(Side *side, int y);

#ifdef DESIGNERS
extern void net_paint_view(Side *side, int x, int y, int r, int tview, int uview);
#endif /* DESIGNERS */

/* plan.c */

extern void net_set_unit_plan_type(Side *side, Unit *unit, int type);
extern void net_set_unit_asleep(Side *side, Unit *unit, int flag, int recurse);
extern void net_set_unit_reserve(Side *side, Unit *unit, int flag, int recurse);
extern void net_set_unit_ai_control(Side *side, Unit *unit, int flag, int recurse);
extern void net_set_unit_main_goal(Side *side, Unit *unit, Goal *goal);
extern void net_set_unit_curadvance(Side *side, Unit *unit, int a);
extern void net_set_unit_researchdone(Side *side, Unit *unit, int flag);
extern void net_set_unit_waiting_for_transport(Side *side, Unit *unit, int flag);
extern void net_wake_unit(Side *side, Unit *unit, int wakeocc);
extern void net_wake_area(Side *side, int x, int y, int n, int occs);
extern void net_set_formation(Unit *unit, Unit *leader, int x, int y, int dist, int flex);
extern void net_delay_unit(Unit *unit, int flag);
extern int net_clear_task_agenda(Side *side, Unit *unit);
extern int net_clear_task_outcome(Side *side, Unit *unit);
extern void net_force_replan(Unit *unit);

/* unit.c */

extern void net_set_unit_name(Side *side, Unit *unit, char *newname);
extern int net_disband_unit(Side *side, Unit *unit);

#ifdef DESIGNERS
extern Unit *net_designer_create_unit(Side *side, int u, int s, int x, int y);
extern int net_designer_teleport(Unit *unit, int x, int y, Unit *other);
extern int net_designer_change_side(Unit *unit, Side *side);
extern int net_designer_disband(Unit *unit);
#endif /* DESIGNERS */

/* task.c */

extern void net_add_task(Unit *unit, int pos, Task *task);

/* world.c */

extern Feature *net_create_feature(const char *feattype, char *name);
extern void net_set_feature_type_name(Feature *feature, char *feattype);
extern void net_set_feature_name(Feature *feature, char *name);
extern void net_destroy_feature(Feature *feature);
extern void net_renumber_features(void);

#ifdef DESIGNERS
extern void net_paint_cell(Side *side, int x, int y, int r, int t);
extern void net_paint_border(Side *side, int x, int y, int dir, int t, int mode);
extern void net_paint_connection(Side *side, int x, int y, int dir, int t, int mode);
extern void net_paint_coating(Side *side, int x, int y, int r, int t, int depth);
extern void net_paint_people(Side *side, int x, int y, int r, int s);
extern void net_paint_control(Side *side, int x, int y, int r, int s);
extern void net_paint_feature(Side *side, int x, int y, int r, int f);
extern void net_paint_elevation(Side *side, int x, int y, int r, int code, int elev, int vary);
extern void net_paint_temperature(Side *side, int x, int y, int r, int temp);
extern void net_paint_material(Side *side, int x, int y, int r, int m, int amt);
extern void net_paint_clouds(Side *side, int x, int y, int r, int cloudtype, int bot, int hgt);
extern void net_paint_winds(Side *side, int x, int y, int r, int dir, int force);

		/* should fix kernel separation! */

extern void fix_elevations(void);

#endif /* DESIGNERS */

/* tp.c */

extern void net_toggle_user_at(int u, int x, int y);

	/* called by all interfaces. */

extern int send_join(const char *str);
extern void receive_data(int timeout, int lim);
extern int host_the_game(char *hostport);
extern int try_join_game(char *hostport);
extern void net_save_game(char *fname);

	/* called by the unix interface. */

extern void send_version(int rid);	
extern void broadcast_game_module(void);
extern void net_send_chat(int rid, char *str);
extern int game_checksum(void);
extern void send_quit(void);
extern void flush_outgoing_queue(void);

extern void (*update_variant_callback)(int which);
extern void (*update_assignment_callback)(int n);

	/* called by cmdline.c. */

extern void net_update_player(Player *player);


	/* called by the x11 interface. */

extern void download_to_player(Player *player);

	/* Called by Tkconq. */

extern void broadcast_randstate(void);

/* Declarations of functions that must be supplied by an interface. */

extern void add_remote_locally(int rid, const char *str);
extern void send_chat(int rid, char *str);
extern void make_default_player_spec(void);

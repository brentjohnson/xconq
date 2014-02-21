/* Implementation of the transfer protocol for networked Xconq.
   Copyright (C) 1996-2000 Stanley T. Shebs.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING. */

/* This is the implementation of the high-level protocol. */

#include "conq.h"
#include "kernel.h"
#include "kpublic.h"

extern long randstate;

/* Iteration over all remote programs, by rid. */

#define for_all_remotes(rid) \
  for ((rid) = 1; (rid) <= numremotes; ++(rid)) \
    if (online[rid])

/* Definitions of special characters in the protocol. */

#define STARTPKT '$'
#define ENDPKT '^'
#define ESCAPEPKT '!'

#define STARTPKTESC '%'
#define ENDPKTESC '&'
#define ESCAPEPKTESC '@'

static void broadcast_command_5(char *cmd, int val, int val2, int val3,
				int val4, int val5);
static void broadcast_side_property(Side *side, char *prop, int val);
static void broadcast_side_property_2(Side *side, char *prop, int val,
				      int val2);
static void broadcast_side_str_property(Side *side, char *prop, char *val);
static void broadcast_unit_property(Side *side, Unit *unit, char *prop,
				    int val);
static void broadcast_unit_property_2(Side *side, Unit *unit, char *prop,
				      int val, int val2);
static void broadcast_unit_property_5(Side *side, Unit *unit, char *prop,
				      int val, int val2, int val3, int val4,
				      int val5);
static void broadcast_unit_str_property(Side *side, Unit *unit, char *prop,
					char *val);
static void broadcast_add_task(Unit *unit, int pos, Task *task);
static void broadcast_layer_change(char *layername, Side *side, int x, int y,
				   int a1, int a2, int a3, int a4);
static void broadcast_packet(char *buf);
static void save_outgoing_packet(int id, char *inbuf);
static void save_incoming_packet(int id, char *inbuf);
static void remove_chars(char *buf, int n);
static void receive_net_message(char *str);
static void receive_command(char *str);
static void receive_action(char *str);
static void receive_player_prop(char *str);
static void receive_quit(char *str);
static void receive_bugoff(int rid);
static void receive_side_prop(char *str);
static void receive_task(char *str);
static void receive_unit_prop(char *str);
static void receive_world_prop(char *str);
static void receive_run_game(char *str);
static void receive_game_checksum(char *str);
static void receive_error(int id, char *str);
static void receive_remote_program(char *str);
static void receive_chat(char *str);
static void receive_variant_setting(char *str);
static void receive_assignment_setting(char *str);
static int tohex(int x);
static int fromhex(int x);
static int flush_incoming_queue(int lim);
static void send_game_checksum_error(int rid, int my_csum, int master_csum);

static int send_packet(int id, char *buf);
static void broadcast_game_checksum(void);
static void broadcast_action(Unit *unit, int actee, ActionType atype,
			     int arg0, int arg1, int arg2, int arg3);
static void receive_packet(int id, char *buf);
static void add_remote_program(int rid, char *name);
static void download_game_module(int rid);
static void send_remote_id(int rid);
static void send_bugoff(int rid);

static void csum_printf(char *str, ...);

static int dumped_checksums = FALSE;
static FILE *cfp = NULL;

/* True if we are in the process of flushing the incoming queue. */

int flushing = FALSE;

/* True if we process (and rebroadcast) packets as soon as they are received. */

int process_packets = TRUE;

/* Turns on output of network traffic through notifications. */

int display_traffic = FALSE;

/* This is true if the program expects to host the game and also be the
   master when all the remote programs are hooked up. */

int hosting;

/* This is the total number of programs in the game. */

int numremotes;

int numremotewaiting;

/* The program to which we are currently downloading. */

int tmprid;

char *remote_player_specs[MAXSIDES];

int online[MAXSIDES];

int expecting_ack;

int timeout_warnings = TRUE;

int sendnow;

/* Flag indicating that we're in the middle of downloading. */

int downloading;

char *downloadbuf;

int dlbufend;

long new_randstate;

void (*update_variant_callback)(int which) = NULL;

void (*update_assignment_callback)(int n) = NULL;

int quitter;

/* Flag indicating that we were bounced from an ongoing game. */

int bounced = FALSE;

char *default_player_spec = NULL;

/* Given a specification for the game to be hosted (such as a port
   number), set up to host and return success/failure. */

int
host_the_game(char *hostport)
{
    int i, rslt;
    Player *player;
    extern char *remote_player_specs[MAXSIDES];
    extern int online[MAXSIDES];

    hosting = TRUE;
    /* If we call this before a game is loaded, numsides will be zero
       and this won't do anything, which is what we want. */
    for (i = 0; i < numsides; ++i) {
	if (assignments[i].side && (assignments[i].side)->ingame) {
	    player = assignments[i].player;
	    if (player->displayname != NULL) {
		player->rid = 1;
	    } else if (player->name) {
		player->remotewanted = player->name;
	    }
	}
    }
    rslt = open_remote_connection(hostport, hosting);
    if (rslt == 0) {
	hosting = FALSE;
	return FALSE;
    }
    /* As the host, we arbitrarily give ourselves remote id #1. */
    my_rid = master_rid = 1;
    /* Compose a plausible default player spec if necessary. */
    if (default_player_spec == NULL)
      make_default_player_spec();
    /* Record our online and active status. */
    online[my_rid] = TRUE;
    remote_player_specs[my_rid] = copy_string(default_player_spec);
    add_remote_locally(my_rid, default_player_spec);
    numremotes = 1;
    return TRUE;
}

/* Try to join a game at the given host and port. */

int
try_join_game(char *hostport)
{
    int rslt;

    hosting = FALSE;
    rslt = open_remote_connection(copy_string(hostport), hosting);
    /* If failure, return quietly and let caller handle interaction. */
    if (rslt == 0) {
	close_remote_connection(0);
	return FALSE;
    }
    sendnow = TRUE;
    /* Compose a player spec if necessary. */
    if (default_player_spec == NULL)
      make_default_player_spec();
      
#ifdef MACOSX
    /* For mysterious reasons, Mac OSX requires a brief pause here, or
    send_join will fail. Mac OS9 (Classic or Carbon) works fine without
    this hack. */
    {
    	extern int last_ticks;
    	
    	last_ticks = TickCount();
    	while(!n_seconds_elapsed(1));
    }
#endif

    /* Send a join packet and then wait until we're officially added
       to the game. */
    rslt = send_join(default_player_spec);
    sendnow = FALSE;
    if (rslt) {
	master_rid = 1;
	while (my_rid == 0) {
	    /* (should have a timeout just in case) */
	    receive_data(0, MAXPACKETS);
	    /* We received a bugoff message. */
	    if (bounced) {
		close_remote_connection(0);
		/* Reset the flag in case we want to try another host. */
		bounced = FALSE;
		return DONE;
	    }
	}
    } else {
	close_remote_connection(0);
	return FALSE;
    }
    /* We've succeeded in getting connected.  Note that this just
       means that we're in the system of synchronized programs,
       not that we have a side/player in the game. */
    return TRUE;
}

/* If a side is being played from a remote program, we don't need the
   full user interface structure, just make a placeholder that records
   the program's id. */

void
init_remote_ui(Side *side)
{
    if (side->rui == NULL) {
	side->rui = (RUI *) xmalloc(sizeof(RUI));
    }
    /* At present, rui->rid is unused. */
    side->rui->rid = side->player->rid;
}

/* For a new remote program with given remote id and player spec,
   record it locally and also broadcast to all remotes currently
   online. */

void
add_remote_program(int rid, char *name)
{
    int rid2;

    numremotes = max(rid, numremotes);
    online[rid] = TRUE;
    /* Inform our new program of all the existing programs. */
    for_all_remotes(rid2) {
	if (rid2 != rid) {
	    sprintf(spbuf, "p%d %s", rid2, remote_player_specs[rid2]);
	    send_packet(rid, spbuf);
	}
    }
    remote_player_specs[rid] = copy_string(name);
    add_remote_locally(rid, name);
    /* Now broadcast info about the new program to everybody. */
    sprintf(spbuf, "p%d %s", rid, name);
    broadcast_packet(spbuf);
}

/* For each remote program (including us), add a player corresponding
   to the spec supplied by that program. */

void
add_remote_players(void)
{
    int rid;
    Player *player;

    for_all_remotes(rid) {
	/* First look for an existing player object that's waiting to
	   be associated with a remote program. */
	player = NULL;
	for_all_players(player) {
	    if (!empty_string(player->name)
		&& strcmp(player->name, "?") == 0)
	      break;
	}
	/* Otherwise create a new player. */
	if (player == NULL)
	  player = add_player();
	parse_player_spec(player, remote_player_specs[rid]);
	canonicalize_player(player);
	player->rid = rid;
    }
}

/* Send a message asking to join the game.  Return TRUE if the host
   actually acknowledged us, FALSE if no response. Note: since send_packet
   waits 30 seconds for each ack, we don't want to get stuck forever here. 
   Instead, we simply return on failure and let the user try to connect once 
   more. */

int
send_join(char *str)
{
    int successful;

    sprintf(spbuf, "j%s", str);
    timeout_warnings = FALSE;
    successful = send_packet(0, spbuf);
    timeout_warnings = TRUE;
    return successful;
}

/* Tell the remote program what its id will be during this game. */

void
send_remote_id(int rid)
{
    sprintf(spbuf, "r%d", rid);
    send_packet(rid, spbuf);
}

/* Tell the remote program to bug off. */

void
send_bugoff(int rid)
{
    sprintf(spbuf, "B");
    send_packet(rid, spbuf);
    close_remote_connection(rid);
}

void
net_send_chat(int rid, char *str)
{
    if (my_rid == master_rid) {
	send_chat(rid, str);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "c%d %s", rid, str);
	broadcast_packet(spbuf);
    }
}

void
send_version(int rid)
{
    /* In order to guarantee that the kernels can actually stay in
       sync, we require that the program versions match exactly. */
    sprintf(spbuf, "V%s", version_string());
    send_packet(rid, spbuf);
}

void
net_set_variant_value(int which, int v1, int v2, int v3)
{
    if (my_rid == master_rid) {
	set_variant_value(which, v1, v2, v3);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "v %d %d %d %d", which, v1, v2, v3);
	broadcast_packet(spbuf);
    }
}

void
net_set_player_advantage(int n, int newadv)
{
    if (my_rid == master_rid) {
	set_player_advantage(n, newadv);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a%d advantage %d", n, newadv);
	broadcast_packet(spbuf);
    }
}

int
net_add_side_and_player(void)
{
    int rslt = -1;

    if (my_rid == master_rid) {
	rslt = add_side_and_player();
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a0 add");
	broadcast_packet(spbuf);
    }
    return rslt;
}

int
net_remove_side_and_player(int s)
{
    int rslt = -1;

    if (my_rid == master_rid) {
	rslt = remove_side_and_player(s);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a remove %d", s);
	broadcast_packet(spbuf);
    }
    return rslt;
}

void
net_rename_side_for_player(int n, int which)
{
    if (my_rid == master_rid) {
	rename_side_for_player(n, which);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a%d rename %d", n, which);
	broadcast_packet(spbuf);
    }
}

void
net_set_ai_for_player(int n, char *aitype)
{
    if (my_rid == master_rid) {
	set_ai_for_player(n, aitype);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a%d ai %s", n, (aitype ? aitype : ""));
	broadcast_packet(spbuf);
    }
}

int
net_exchange_players(int n, int n2)
{
    int rslt = -1;

    if (my_rid == master_rid) {
	rslt = exchange_players(n, n2);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "a%d exchange %d", n, n2);
	broadcast_packet(spbuf);
    }
    return rslt;
}

void
net_update_player(Player *player)
{
    sprintf(spbuf, "a%d update %s", player->id, player_desig(player));
    broadcast_packet(spbuf);
}

/* Given the id of a remote program that has just connected,
   download our own current state, as a module. */

void
download_game_module(int rid)
{
    int rslt;
    Module *module;

    if (1 /* sending name only */) {
	/* If the module has a filename it is a saved game. */
    	if (!empty_string(mainmodule->filename)) {
		sprintf(spbuf, "f %s", mainmodule->filename);
	/* else it is a library module. */
	} else {    	
		sprintf(spbuf, "g %s", mainmodule->name);
	}
	send_packet(rid, spbuf);
	/* (should send some sort of state checksum also) */
	return;
    }
    /* The indepside might not have been filled in yet (such as when
       connecting before sides/players have been set up), but we need
       it to be correct before going into write_game_module. */
    if (numtotsides == 0)
      create_side();
    /* Record the rid in a global so the module-writing code knows
       where to send all the packets. */
    tmprid = rid;
    send_packet(rid, "gameModule");
    module = create_game_module("*download*");
    copy_module(module, mainmodule);
    /* This module is not associated with a file. */
    module->filename = NULL;
    /* The module will sent in its entirety, so suppress any attempt
       to load a base module. */
    module->basemodulename = NULL;
    module->compress_tables = TRUE;
    module->compress_layers = TRUE;
    module->def_all = TRUE; /* for now */
    downloading = TRUE;
    rslt = write_game_module(module, NULL);
    downloading = FALSE;
    send_packet(rid, "\neludoMemag\n");
    tmprid = 0;
}

#ifndef MAC
#define DOWNLOADPACKETSIZE 200
#else
#define DOWNLOADPACKETSIZE 40
#endif

static char *notherbuf;

void
add_to_packet(char *str)
{
    if (notherbuf == NULL)
      notherbuf = (char *)xmalloc(DOWNLOADPACKETSIZE + 10);
    if (strlen(str) + strlen(notherbuf) > DOWNLOADPACKETSIZE) {
	/* No room in the packet, send it and start on a new one. */
	send_packet(tmprid, notherbuf);
	notherbuf[0] = '\0';
	/* If the string is long, break it into multiple packets. */
	while (strlen(str) > DOWNLOADPACKETSIZE) {
	    strncpy(notherbuf, str, DOWNLOADPACKETSIZE);
	    notherbuf[DOWNLOADPACKETSIZE] = '\0';
	    send_packet(tmprid, notherbuf);
	    notherbuf[0] = '\0';
	    str += DOWNLOADPACKETSIZE;
	}
    }
    strcat(notherbuf, str);
}

void
flush_write(void)
{
    if (!empty_string(notherbuf)) {
	send_packet(tmprid, notherbuf);
	notherbuf[0] = '\0';
    }
}

/* Make sure every remote program has the same game that we do. */

void
broadcast_game_module(void)
{
    int rid;

    for_all_remotes(rid) {
	if (rid != my_rid) {
	    download_game_module(rid);
	}
    }
}

void
broadcast_start_game_load(void)
{
    sprintf(spbuf, "s");
    broadcast_packet(spbuf);
}

void
broadcast_start_variant_setup(void)
{
    sprintf(spbuf, "vSTART");
    broadcast_packet(spbuf);
}

void
broadcast_variants_chosen(void)
{
    /* Once variants have been chosen, programs will advance to making
       trial assignments immediately, so make sure randstates are in
       sync first. */
    sprintf(spbuf, "R%ld", randstate);
    broadcast_packet(spbuf); 
    sprintf(spbuf, "vOK");
    broadcast_packet(spbuf);
}

void
broadcast_start_player_setup(void)
{
    sprintf(spbuf, "aSTART");
    broadcast_packet(spbuf);
}

void
broadcast_players_assigned(void)
{
    sprintf(spbuf, "aOK");
    broadcast_packet(spbuf);
}

/* Broadcast the randstate of the peer. */
void
broadcast_randstate(void)
{
    sprintf(spbuf, "R%ld", randstate);
    broadcast_packet(spbuf);
}

void
broadcast_game_checksum(void)
{
    sprintf(spbuf, "Z%d %d", my_rid, game_checksum());
    broadcast_packet(spbuf); 
}

void
send_game_checksum_error(int rid, int my_csum, int master_csum)
{
    sprintf(spbuf, "Echecksum %d %d %d", my_rid, my_csum, master_csum);
    send_packet(rid, spbuf); 
}

time_t last_checksum_time;

int
net_run_game(int maxactions)
{
	int oldsernum, oldstate, sendcheck, oldcsum, newcsum, numdone = 0;
    	time_t now;

	/* Network clients should get out of here. */
	if (my_rid != master_rid)
    	    return 0;
	/* Dont process or rebroadcast any packets for now. */    	  
    	process_packets = FALSE;
	if (numremotes > 0) {
	    oldsernum = g_run_serial_number();
	    oldstate = randstate;
	    /* Send occasional checksums at random times. */
	    sendcheck = FALSE;
	    if (0 /* for now */) {
		time(&now);
		sendcheck = (idifftime(now, last_checksum_time) >= 2);
		if (sendcheck)
		  last_checksum_time = now;
	    }
	    if (sendcheck)
	      broadcast_game_checksum();
	    oldcsum = game_checksum();
	}
	/* This is where the master's real run_game call happens. */
	numdone = run_game(maxactions);
	if (numremotes > 0) {
	    if (numdone > 0) {

		/* This checksum broadcast was the source of all the synch
		errors. It made it possible for the client to send data while
		the host was waiting for an ack that would change the state of
		the game (after rebroadcasting by the host). Since we are at a
		point where the host but not the client has executed run_game
		nothing must be allowed to happen here that will change the
		state of the game. */
#if 0
		sprintf(spbuf, "Za%d %d", my_rid, oldcsum);
		broadcast_packet(spbuf);
#endif
		sprintf(spbuf, "X%d %d %d", maxactions, oldsernum, oldstate);
		broadcast_packet(spbuf);
		/* This caused the network game to hang on exit. */
		if (!endofgame) {
		    broadcast_game_checksum();
	    	}
	    } else {
		newcsum = game_checksum();
		if (newcsum != oldcsum)
		  notify_all("Checksum changed %d -> %d when numdone == 0",
			      oldcsum, newcsum);
	    }
	}
	/* Turn on packet processing. */
	process_packets = TRUE;
	return numdone;
}

void
net_request_additional_side(char *playerspec)
{
    if (my_rid == master_rid) {
	request_additional_side(playerspec);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "Padd %s", (playerspec ? playerspec : ""));
	broadcast_packet(spbuf);
    }
}

void
net_send_message(Side *side, SideMask sidemask, char *str)
{
    if (my_rid == master_rid) {
	send_message(side, sidemask, str);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "M%d %d %s", side_number(side), sidemask, str);
	broadcast_packet(spbuf);
    }
}

static void
broadcast_command_5(char *cmd, int a1, int a2, int a3, int a4, int a5)
{
    sprintf(spbuf, "C%s %d %d %d %d %d", cmd, a1, a2, a3, a4, a5);
    broadcast_packet(spbuf);
}

void
net_resign_game(Side *side, Side *side2)
{
    if (my_rid == master_rid) {
	resign_game(side, side2);
    }
    if (numremotes > 0) {
      broadcast_side_property(side, "resign",
			      (side2 ? side_number(side2) : -1));
    }
}

void
net_save_game(char *fname)
{
    char *name;

     /* First save the game locally wherever the user wanted it. */
    write_entire_game_state(fname);
    /* Peel off the path before broadcasting so that standardized
    locations are used to save the network game. */
    name = copy_string(find_name(fname));
    if (my_rid > 0 && my_rid == master_rid) {
   	save_game(name);
   }
    if (numremotes > 0) {
	sprintf(spbuf, "w%s", name);
	broadcast_packet(spbuf);
    }
}

/* Side property tweaking. */

void
net_set_side_name(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_name(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "name", newname);
    }
}

void
net_set_side_longname(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_longname(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "longname", newname);
    }
}

void
net_set_side_shortname(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_shortname(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "shortname", newname);
    }
}

void
net_set_side_noun(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_noun(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "noun", newname);
    }
}

void
net_set_side_pluralnoun(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_pluralnoun(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "pluralnoun", newname);
    }
}

void
net_set_side_adjective(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_adjective(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "adjective", newname);
    }
}

void
net_set_side_emblemname(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_emblemname(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "emblemname", newname);
    }
}

void
net_set_side_colorscheme(Side *side, Side *side2, char *newname)
{
    if (my_rid == master_rid) {
	set_side_colorscheme(side, side2, newname);
    }
    if (numremotes > 0)  {
	broadcast_side_str_property(side2, "colorscheme", newname);
    }
}

void
net_finish_turn(Side *side)
{
    if (my_rid == master_rid) {
	finish_turn(side);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "fin", 0);
    }
}

void
net_set_trust(Side *side, Side *side2, int val)
{
    if (my_rid == master_rid) {
	set_trust(side, side2, val);
    }
    if (numremotes > 0)  {
	broadcast_side_property_2(side, "trust", side_number(side2), val);
    }
}

void
net_set_controlled_by(Side *side, Side *side2, int val)
{
    if (my_rid == master_rid) {
	set_controlled_by(side, side2, val);
    }
    if (numremotes > 0)  {
	broadcast_side_property_2(side, "controlledby", side_number(side2), val);
    }
}

void
net_set_autofinish(Side *side, int value)
{
    if (my_rid == master_rid) {
	set_autofinish(side, value);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "af", value);
    }
}

void
net_set_autoresearch(Side *side, int value)
{
    if (my_rid == master_rid) {
	set_autoresearch(side, value);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "ar", value);
    }
}

void
net_set_willing_to_save(Side *side, int flag)
{
    if (my_rid == master_rid) {
	set_willing_to_save(side, flag);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "save", flag);
    }
}

void
net_set_willing_to_draw(Side *side, int flag)
{
    if (my_rid == master_rid) {
	set_willing_to_draw(side, flag);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "draw", flag);
    }
}

void
net_set_side_self_unit(Side *side, Unit *unit)
{
    if (my_rid == master_rid) {
	set_side_self_unit(side, unit);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "self", (unit ? unit->id : 0));
    }
}

void
net_set_side_ai(Side *side, char *aitype)
{
    if (my_rid == master_rid) {
	set_side_ai(side, aitype);
    }
    if (numremotes > 0)  {
	if (aitype == NULL)
	  aitype = "";
	broadcast_side_str_property(side, "ai", aitype);
    }
}

void
net_set_doctrine(Side *side, char *spec)
{
    if (my_rid == master_rid) {
	set_doctrine(side, spec);
    }
    if (numremotes > 0)  {
	if (spec == NULL)
	  spec = "";
	broadcast_side_str_property(side, "doctrine", spec);
    }
}

void
net_set_side_research_topic(Side *side, int a)
{
    if (my_rid == master_rid) {
	set_side_research_topic(side, a);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "research_topic", a);
    }
}

void
net_set_side_research_goal(Side *side, int a)
{
    if (my_rid == master_rid) {
	set_side_research_goal(side, a);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "research_goal", a);
    }
}

void
net_set_side_startx(Side *side, int x)
{
    if (my_rid == master_rid) {
	set_side_startx(side, x);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "startx", x);
    }
}

void
net_set_side_starty(Side *side, int y)
{
    if (my_rid == master_rid) {
	set_side_starty(side, y);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "starty", y);
    }
}

#ifdef DESIGNERS

void
net_become_designer(Side *side)
{
    if (my_rid == master_rid) {
	become_designer(side);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "designer", TRUE);
    }
}

void
net_become_nondesigner(Side *side)
{
    if (my_rid == master_rid) {
	become_nondesigner(side);
    }
    if (numremotes > 0)  {
	broadcast_side_property(side, "designer", FALSE);
    }
}

void
net_paint_view(Side *side, int x, int y, int r, int tview, int uview)
{
    if (my_rid == master_rid) {
	paint_view(side, x, y, r, tview, uview);
    }
    if (numremotes > 0)  {
	sprintf(spbuf, "D%d view %d %d %d %d %d %d",
		side_number(side), side_number(side), x, y, r, tview, uview);
	broadcast_packet(spbuf);
    }
}

#endif /* DESIGNERS */

static void
broadcast_side_property(Side *side, char *prop, int val)
{
    sprintf(spbuf, "S%d %s %d", side_number(side), prop, val);
    broadcast_packet(spbuf);
}

static void
broadcast_side_property_2(Side *side, char *prop, int val, int val2)
{
    sprintf(spbuf, "S%d %s %d %d", side_number(side), prop, val, val2);
    broadcast_packet(spbuf);
}

static void
broadcast_side_str_property(Side *side, char *prop, char *val)
{
    sprintf(spbuf, "S%d %s %s", side_number(side), prop, val);
    broadcast_packet(spbuf);
}

/* Unit property tweaking. */

void
net_set_unit_name(Side *side, Unit *unit, char *newname)
{
    if (my_rid == master_rid) {
	set_unit_name(side, unit, newname);
    }
    if (numremotes > 0) {
	broadcast_unit_str_property(side, unit, "name", newname);
    }
}

void
net_set_unit_plan_type(Side *side, Unit *unit, int type)
{
    if (my_rid == master_rid) {
	set_unit_plan_type(side, unit, type);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "plan", type);
    }
}

void
net_set_unit_asleep(Side *side, Unit *unit, int flag, int recurse)
{
    if (my_rid == master_rid) {
	set_unit_asleep(side, unit, flag, recurse);
    }
    if (numremotes > 0) {
	broadcast_unit_property_2(side, unit, "sleep", flag, recurse);
    }
}

void
net_set_unit_reserve(Side *side, Unit *unit, int flag, int recurse)
{
    if (my_rid == master_rid) {
	set_unit_reserve(side, unit, flag, recurse);
    }
    if (numremotes > 0) {
	broadcast_unit_property_2(side, unit, "resv", flag, recurse);
    }
}

void
net_set_unit_main_goal(Side *side, Unit *unit, Goal *goal)
{
    if (my_rid == master_rid) {
	set_unit_main_goal(side, unit, goal);
    }
    if (numremotes > 0) {
	broadcast_unit_property_5(side, unit, "maingoal",
				  (goal ? goal->type : 0),
				  (goal ? goal->args[0] : 0),
				  (goal ? goal->args[1] : 0),
				  (goal ? goal->args[2] : 0),
				  (goal ? goal->args[3] : 0));
    }
}

void
net_set_unit_curadvance(Side *side, Unit *unit, int a)
{
    if (my_rid == master_rid) {
	set_unit_curadvance(side, unit, a);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "curadvance", a);
    }
}

void
net_set_unit_researchdone(Side *side, Unit *unit, int flag)
{
    if (my_rid == master_rid) {
	set_unit_researchdone(side, unit, flag);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "researchdone", flag);
    }
}

void
net_set_unit_waiting_for_transport(Side *side, Unit *unit, int flag)
{
    if (my_rid == master_rid) {
	set_unit_waiting_for_transport(side, unit, flag);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "waittrans", flag);
    }
}

void
net_wake_unit(Side *side, Unit *unit, int wakeocc)
{
    if (my_rid == master_rid) {
	wake_unit(side, unit, wakeocc);
    }
    if (numremotes > 0) {
	broadcast_command_5("wake-unit", side_number(side), unit->id, wakeocc, 0, 0);
    }
}

void
net_wake_area(Side *side, int x, int y, int n, int occs)
{
    if (my_rid == master_rid) {
	wake_area(side, x, y, n, occs);
    }
    if (numremotes > 0) {
	broadcast_command_5("wake-area", side_number(side), x, y, n, occs);
    }
}

void
net_set_unit_ai_control(Side *side, Unit *unit, int flag, int recurse)
{
    if (my_rid == master_rid) {
	set_unit_ai_control(side, unit, flag, recurse);
    }
    if (numremotes > 0) {
	broadcast_unit_property_2(side, unit, "ai", flag, recurse);
    }
}

int
net_clear_task_agenda(Side *side, Unit *unit)
{
    if (my_rid == master_rid) {
	clear_task_agenda(unit);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "clragenda", 0);
    }
    return 0;
}

int
net_clear_task_outcome(Side *side, Unit *unit)
{
    if (my_rid == master_rid) {
	clear_task_outcome(unit);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "clroutcome", 0);
    }
    return 0;
}

void
net_force_replan(Unit *unit)
{
    if (my_rid == master_rid) {
	force_replan(unit);
    }
    if (numremotes > 0) {
	broadcast_unit_property(unit->side, unit, "forcereplan", 0);
    }
}

int
net_disband_unit(Side *side, Unit *unit)
{
    if (my_rid == master_rid) {
	return disband_unit(side, unit);
    }
    if (numremotes > 0) {
	broadcast_unit_property(side, unit, "disband", 0);
    }
    return 0;
}

void
net_set_formation(Unit *unit, Unit *leader, int x, int y, int dist, int flex)
{
    if (my_rid == master_rid) {
	set_formation(unit, leader, x, y, dist, flex);
    }
    if (numremotes > 0) {
	broadcast_unit_property_5(unit->side, unit, "formation",
				  (leader ? leader->id : 0), x, y, dist, flex);
    }
}

void
net_delay_unit(Unit *unit, int flag)
{
    if (my_rid == master_rid) {
	delay_unit(unit, flag);
    }
    if (numremotes > 0) {
	broadcast_unit_property(unit->side, unit, "delay", flag);
    }
}

#ifdef DESIGNERS

/* A designer can call this to create an arbitrary unit during the game. */

Unit *
net_designer_create_unit(Side *side, int u, int s, int x, int y)
{
    Unit *rslt = NULL;

    if (my_rid == master_rid) {
	rslt = designer_create_unit(side, u, s, x, y);
    }
    if (numremotes > 0) {
	broadcast_command_5("designer-create-unit", side_number(side), 
			    u, s, x, y);
    }
    return rslt;
}

/* Move a unit to a given location instantly, with all sides observing.
   This is for use by designers only! */

int
net_designer_teleport(Unit *unit, int x, int y, Unit *other)
{
    int rslt = 0;

    if (my_rid == master_rid) {
	rslt = designer_teleport(unit, x, y, other);
    }
    if (numremotes > 0) {
	broadcast_command_5("designer-teleport", unit->id, x, y, (other ? other->id : 0), 0);
    }
    return rslt;
}

int
net_designer_change_side(Unit *unit, Side *side)
{
    int rslt = 0;

    if (my_rid == master_rid) {
	rslt = designer_change_side(unit, side);
    }
    if (numremotes > 0) {
	broadcast_command_5("designer-change-side", unit->id,
			    side_number(side), 0, 0, 0);
    }
    return rslt;
}

int
net_designer_disband(Unit *unit)
{
    int rslt = 0;

    if (my_rid == master_rid) {
	rslt = designer_disband(unit);
    }
    if (numremotes > 0) {
	broadcast_command_5("designer-disband", unit->id, 0, 0, 0, 0);
    }
    return rslt;
}

#endif /* DESIGNERS */

Feature *
net_create_feature(char *feattype, char *name)
{
    Feature *rslt = NULL;

    if (my_rid == master_rid) {
	rslt = create_feature(feattype, name);
    }
    if (numremotes > 0) {
	run_error("net");
    }
    return rslt;
}

void
net_set_feature_type_name(Feature *feature, char *feattype)
{
    if (my_rid == master_rid) {
	set_feature_type_name(feature, feattype);
    }
    if (numremotes > 0) {
	run_error("net");
    }
}

void
net_set_feature_name(Feature *feature, char *name)
{
    if (my_rid == master_rid) {
	set_feature_name(feature, name);
    }
    if (numremotes > 0) {
	run_error("net");
    }
}

void
net_destroy_feature(Feature *feature)
{
    if (my_rid == master_rid) {
	destroy_feature(feature);
    }
    if (numremotes > 0) {
	run_error("net");
    }
}

void
net_renumber_features(void)
{
    if (my_rid == master_rid) {
	renumber_features();
    }
    if (numremotes > 0) {
	run_error("net");
    }
}

void
net_toggle_user_at(int u, int x, int y)
{
    if (my_rid == master_rid) {
    	toggle_user_at(find_unit(u), x, y);
    }
    if (numremotes > 0) {
	sprintf(spbuf, "Wuser %d %d %d %d", 0, x, y, u);
	broadcast_packet(spbuf);
    }
}

static void
broadcast_unit_property(Side *side, Unit *unit, char *prop, int val)
{
    char buf[BUFSIZE];

    sprintf(buf, "U%d %d %s %d", side_number(side), unit->id, prop, val);
    broadcast_packet(buf);
}

static void
broadcast_unit_property_2(Side *side, Unit *unit, char *prop, int val, int val2)
{
    char buf[BUFSIZE];

    sprintf(buf, "U%d %d %s %d %d", side_number(side), unit->id, prop, val, val2);
    broadcast_packet(buf);
}

static void
broadcast_unit_property_5(Side *side, Unit *unit, char *prop,
			  int val, int val2, int val3, int val4, int val5)
{
    char buf[BUFSIZE];

    sprintf(buf, "U%d %d %s %d %d %d %d %d", side_number(side), unit->id, prop,
	    val, val2, val3, val4, val5);
    broadcast_packet(buf);
}

static void
broadcast_unit_str_property(Side *side, Unit *unit, char *prop, char *val)
{
    char buf[BUFSIZE];

    sprintf(buf, "U%d %d %s %s", side_number(side), unit->id, prop, val);
    broadcast_packet(buf);
}

/* Action networking. */

static void
broadcast_action(Unit *unit, int actee, ActionType atype,
		 int arg0, int arg1, int arg2, int arg3)
{
    int n;
    char buf[BUFSIZE];
    
    sprintf(buf, "A%d", unit->id);
    if (unit->id != actee) {
	tprintf(buf, "/%d", actee);
    }
    tprintf(buf, " %d", atype);
    n = strlen(actiondefns[atype].argtypes);
    if (n > 0) {
	tprintf(buf, " %d", arg0);
    }
    if (n > 1) {
	tprintf(buf, " %d", arg1);
    }
    if (n > 2) {
	tprintf(buf, " %d", arg2);
    }
    if (n > 3) {
	tprintf(buf, " %d", arg3);
    }
    if (n > 4) {
	notify_all("internal error: %d args to action", n);
    }
    broadcast_packet(buf);
}

/* From actions.c. */

int
net_prep_none_action(Unit *unit, Unit *unit2)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_none_action(unit, unit2);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_NONE, 0, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_produce_action(Unit *unit, Unit *unit2, int m, int n)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_produce_action(unit, unit2, m, n);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_PRODUCE, m, n, 0, 0);
    }
    return rslt;
}

int
net_prep_extract_action(Unit *unit, Unit *unit2, int x, int y, int m, int n)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_extract_action(unit, unit2, x, y, m, n);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_EXTRACT, x, y, m, n);
    }
    return rslt;
}

int
net_prep_develop_action(Unit *unit, Unit *unit2, int u3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_develop_action(unit, unit2, u3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_DEVELOP, u3, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_toolup_action(Unit *unit, Unit *unit2, int u3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_toolup_action(unit, unit2, u3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_TOOL_UP, u3, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_create_in_action(Unit *unit, Unit *unit2, int u3, Unit *dest)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_create_in_action(unit, unit2, u3, dest);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_CREATE_IN,
			 u3, dest->id, 0, 0);
    }
    return rslt;
}

int
net_prep_create_at_action(Unit *unit, Unit *unit2, int u3, int x, int y, int z)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_create_at_action(unit, unit2, u3, x, y, z);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_CREATE_AT, u3, x, y, z);
    }
    return rslt;
}

int
net_prep_build_action(Unit *unit, Unit *unit2, Unit *newunit)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_build_action(unit, unit2, newunit);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_BUILD, newunit->id, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_repair_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_repair_action(unit, unit2, unit3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_REPAIR, unit3->id, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_disband_action(Unit *unit, Unit *unit2)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_disband_action(unit, unit2);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_DISBAND, 0, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_change_type_action(Unit *unit, Unit *unit2, int u3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_change_type_action(unit, unit2, u3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_CHANGE_TYPE, u3, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_change_side_action(Unit *unit, Unit *unit2, Side *side)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_change_side_action(unit, unit2, side);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_CHANGE_SIDE,
			 side_number(side), 0, 0, 0);
    }
    return rslt;
}

int
net_prep_alter_cell_action(Unit *unit, Unit *unit2, int x, int y, int t)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_alter_cell_action(unit, unit2, x, y, t);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_ALTER_TERRAIN, x, y, t, 0);
    }
    return rslt;
}

int
net_prep_add_terrain_action(Unit *unit, Unit *unit2,
			    int x, int y, int dir, int t)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_add_terrain_action(unit, unit2, x, y, dir, t);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_ADD_TERRAIN, x, y, dir, t);
    }
    return rslt;
}

int
net_prep_remove_terrain_action(Unit *unit, Unit *unit2,
			       int x, int y, int dir, int t)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_remove_terrain_action(unit, unit2, x, y, dir, t);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_REMOVE_TERRAIN,
			 x, y, dir, t);
    }
    return rslt;
}

/* From combat.c. */

int
net_prep_attack_action(Unit *unit, Unit *unit2, Unit *defender, int n)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_attack_action(unit, unit2, defender, n);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_ATTACK,
			 defender->id, n, 0, 0);
    }
    return rslt;
}

int
net_prep_overrun_action(Unit *unit, Unit *unit2, int x, int y, int z, int n)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_overrun_action(unit, unit2, x, y, z, n);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_OVERRUN, x, y, z, n);
    }
    return rslt;
}

int
net_prep_fire_at_action(Unit *unit, Unit *unit2, Unit *unit3, int m)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_fire_at_action(unit, unit2, unit3, m);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_FIRE_AT, unit3->id, m, 0, 0);
    }
    return rslt;
}

int
net_prep_fire_into_action(Unit *unit, Unit *unit2, int x, int y, int z, int m)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_fire_into_action(unit, unit2, x, y, z, m);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_FIRE_INTO, x, y, z, m);
    }
    return rslt;
}

int
net_prep_capture_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_capture_action(unit, unit2, unit3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_CAPTURE,
			 unit3->id, 0, 0, 0);
    }
    return rslt;
}

int
net_prep_detonate_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_detonate_action(unit, unit2, x, y, z);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_DETONATE, x, y, z, 0);
    }
    return rslt;
}

/* From move.c. */

int
net_prep_move_action(Unit *unit, Unit *unit2, int x, int y, int z)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_move_action(unit, unit2, x, y, z);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_MOVE, x, y, z, 0);
    }
    return rslt;
}

int
net_prep_enter_action(Unit *unit, Unit *unit2, Unit *unit3)
{
    int rslt;

    /* Check the prep action result first and return if it would fail. */
    if (unit == NULL || unit->act == NULL || unit2 == NULL) {
	return FALSE;
    }
    /* Then check the busy flag. */
    if (unit->busy) {
    	return TRUE;
    } else {
    	unit->busy = TRUE;
    }
    if (my_rid == master_rid) {
	rslt = prep_enter_action(unit, unit2, unit3);
    }
    if (numremotes > 0) {
	broadcast_action(unit, unit2->id, ACTION_ENTER, unit3->id, 0, 0, 0);
    }
    return rslt;
}

/* Task networking. */

static Task *tmptask;

/* (should remove, never used) */
void
net_add_task(Unit *unit, int pos, Task *task)
{
    if (my_rid == master_rid) {
	add_task(unit, pos, task);
    }
    if (numremotes > 0) {
	broadcast_add_task(unit, pos, task);
    }
}

void
net_set_capture_task(Unit *unit, int id, int capmethod, int n)
{
    if (my_rid == master_rid) {
	set_capture_task(unit, id, capmethod, n);
    }
    if (numremotes > 0) {
	tmptask = create_capture_task(unit, id, capmethod, n);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_set_occupy_task(Unit *unit, Unit *transport)
{
    if (my_rid == master_rid) {
	set_occupy_task(unit, transport);
    }
    if (numremotes > 0) {
	tmptask = create_occupy_task(unit, transport);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_set_collect_task(Unit *unit, int m, int x, int y)
{
    if (my_rid == master_rid) {
	set_collect_task(unit, m, x, y);
    }
    if (numremotes > 0) {
	tmptask = create_collect_task(unit, m, x, y);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_set_move_to_task(Unit *unit, int x, int y, int dist)
{
    if (my_rid == master_rid) {
	set_move_to_task(unit, x, y, dist);
    }
    if (numremotes > 0) {
	tmptask = create_move_to_task(unit, x, y, dist);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_move_to_task(Unit *unit, int x, int y, int dist)
{
    if (my_rid == master_rid) {
	push_move_to_task(unit, x, y, dist);
    }
    if (numremotes > 0) {
	tmptask = create_move_to_task(unit, x, y, dist);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_set_move_dir_task(Unit *unit, int dir, int n)
{
    if (my_rid == master_rid) {
	set_move_dir_task(unit, dir, n);
    }
    if (numremotes > 0) {
	tmptask = create_move_dir_task(unit, dir, n);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_move_dir_task(Unit *unit, int dir, int n)
{
    if (my_rid == master_rid) {
	push_move_dir_task(unit, dir, n);
    }
    if (numremotes > 0) {
	tmptask = create_move_dir_task(unit, dir, n);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

/* Construct Task */

void
net_set_construct_task(Unit *unit, int u, int run, int transid, int x, int y)
{
    if (my_rid == master_rid) {
	set_construct_task(unit, u, run, transid, x, y);
    }
    if (numremotes > 0) {
	tmptask = create_construct_task(unit, u, run, transid, x, y);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_construct_task(Unit *unit, int u, int run, int transid, int x, int y)
{
    if (my_rid == master_rid) {
	push_construct_task(unit, u, run, transid, x, y);
    }
    if (numremotes > 0) {
	tmptask = create_construct_task(unit, u, run, transid, x, y);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

/* Build Task */

void
net_set_build_task(Unit *unit, int id, int cp)
{
    if (my_rid == master_rid) {
	set_build_task(unit, id, cp);
    }
    if (numremotes > 0) {
	tmptask = create_build_task(unit, id, cp);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_build_task(Unit *unit, int id, int cp)
{
    if (my_rid == master_rid) {
	push_build_task(unit, id, cp);
    }
    if (numremotes > 0) {
	tmptask = create_build_task(unit, id, cp);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

/* Repair Task */

void
net_set_repair_task(Unit *unit, int id, int hp)
{
    if (my_rid == master_rid) {
	set_repair_task(unit, id, hp);
    }
    if (numremotes > 0) {
	tmptask = create_repair_task(unit, id, hp);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_repair_task(Unit *unit, int id, int hp)
{
    if (my_rid == master_rid) {
	push_repair_task(unit, id, hp);
    }
    if (numremotes > 0) {
	tmptask = create_repair_task(unit, id, hp);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_set_develop_task(Unit *unit, int u2, int techgoal)
{
    if (my_rid == master_rid) {
	set_develop_task(unit, u2, techgoal);
    }
    if (numremotes > 0) {
	tmptask = create_develop_task(unit, u2, techgoal);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_develop_task(Unit *unit, int u2, int n)
{
    if (my_rid == master_rid) {
	push_develop_task(unit, u2, n);
    }
    if (numremotes > 0) {
	tmptask = create_develop_task(unit, u2, n);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_push_hit_unit_task(Unit *unit, int id, int hitmethod, int n)
{
    if (my_rid == master_rid) {
	push_hit_unit_task(unit, id, hitmethod, n);
    }
    if (numremotes > 0) {
	tmptask = create_hit_unit_task(unit, id, hitmethod, n);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_set_hit_unit_task(Unit *unit, int id, int hitmethod, int n)
{
    if (my_rid == master_rid) {
	set_hit_unit_task(unit, id, hitmethod, n);
    }
    if (numremotes > 0) {
	tmptask = create_hit_unit_task(unit, id, hitmethod, n);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_set_disband_task(Unit *unit)
{
    if (my_rid == master_rid) {
	set_disband_task(unit);
    }
    if (numremotes > 0) {
	tmptask = create_task(TASK_DISBAND);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_set_resupply_task(Unit *unit, int m)
{
    if (my_rid == master_rid) {
	set_resupply_task(unit, m);
    }
    if (numremotes > 0) {
	tmptask = create_resupply_task(unit, m);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_occupy_task(Unit *unit, Unit *transport)
{
    if (my_rid == master_rid) {
	push_occupy_task(unit, transport);
    }
    if (numremotes > 0) {
	tmptask = create_occupy_task(unit, transport);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_push_pickup_task(Unit *unit, Unit *occ)
{
    if (my_rid == master_rid) {
	push_pickup_task(unit, occ);
    }
    if (numremotes > 0) {
	tmptask = create_pickup_task(unit, occ);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_push_produce_task(Unit *unit, int m, int n)
{
    if (my_rid == master_rid) {
	push_produce_task(unit, m, n);
    }
    if (numremotes > 0) {
	tmptask = create_produce_task(unit, m, n);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

void
net_set_sentry_task(Unit *unit, int n)
{
    if (my_rid == master_rid) {
	set_sentry_task(unit, n);
    }
    if (numremotes > 0) {
	tmptask = create_sentry_task(unit, n);
	broadcast_add_task(unit, CLEAR_AGENDA, tmptask);
	free_task(tmptask);
    }
}

void
net_push_sentry_task(Unit *unit, int n)
{
    if (my_rid == master_rid) {
	push_sentry_task(unit, n);
    }
    if (numremotes > 0) {
	tmptask = create_sentry_task(unit, n);
	broadcast_add_task(unit, 0, tmptask);
	free_task(tmptask);
    }
}

static void
broadcast_add_task(Unit *unit, int pos, Task *task)
{
    int numargs, i;
    char buf[BUFSIZE], *argtypes;

    sprintf(buf, "T%d %d %d %d %d",
	    unit->id, pos, task->type, task->execnum, task->retrynum);
    argtypes = taskdefns[task->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      tprintf(buf, " %d", task->args[i]);
    broadcast_packet(buf);
}

/* World tweaking. */

#ifdef DESIGNERS

void
net_paint_cell(Side *side, int x, int y, int r, int t)
{
    if (my_rid == master_rid) {
	paint_cell(side, x, y, r, t);
    }
    if (numremotes > 0) {
	broadcast_layer_change("cell", side, x, y, r, t, 0, 0);
    }
}

void
net_paint_border(Side *side, int x, int y, int dir, int t, int mode)
{
    if (my_rid == master_rid) {
	paint_border(side, x, y, dir, t, mode);
    }
    if (numremotes > 0) {
	broadcast_layer_change("bord", side, x, y, dir, t, mode, 0);
    }
}

void
net_paint_connection(Side *side, int x, int y, int dir, int t, int mode)
{
    if (my_rid == master_rid) {
	paint_connection(side, x, y, dir, t, mode);
    }
    if (numremotes > 0) {
	broadcast_layer_change("conn", side, x, y, dir, t, mode, 0);
    }
}

void
net_paint_coating(Side *side, int x, int y, int r, int t, int depth)
{
    if (my_rid == master_rid) {
	paint_coating(side, x, y, r, t, depth);
    }
    if (numremotes > 0) {
	broadcast_layer_change("coat", side, x, y, r, t, depth, 0);
    }
}

void
net_paint_people(Side *side, int x, int y, int r, int s)
{
    if (my_rid == master_rid) {
	paint_people(side, x, y, r, s);
    }
    if (numremotes > 0) {
	broadcast_layer_change("peop", side, x, y, r, s, 0, 0);
    }
}

void
net_paint_control(Side *side, int x, int y, int r, int s)
{
    if (my_rid == master_rid) {
	paint_control(side, x, y, r, s);
    }
    if (numremotes > 0) {
	broadcast_layer_change("ctrl", side, x, y, r, s, 0, 0);
    }
}

void
net_paint_feature(Side *side, int x, int y, int r, int f)
{
    if (my_rid == master_rid) {
	paint_feature(side, x, y, r, f);
    }
    if (numremotes > 0) {
	broadcast_layer_change("feat", side, x, y, r, f, 0, 0);
    }
}

void
net_paint_elevation(Side *side, int x, int y, int r, int code, int elev,
		    int vary)
{
    if (my_rid == master_rid) {
	paint_elevation(side, x, y, r, code, elev, vary);
    }
    if (numremotes > 0) {
	broadcast_layer_change("elev", side, x, y, r, code, elev, vary);
    }
}

void
net_paint_temperature(Side *side, int x, int y, int r, int temp)
{
    if (my_rid == master_rid) {
	paint_temperature(side, x, y, r, temp);
    }
    if (numremotes > 0) {
	broadcast_layer_change("temp", side, x, y, r, temp, 0, 0);
    }
}

void
net_paint_material(Side *side, int x, int y, int r, int m, int amt)
{
    if (my_rid == master_rid) {
	paint_material(side, x, y, r, m, amt);
    }
    if (numremotes > 0) {
	broadcast_layer_change("m", side, x, y, r, m, amt, 0);
    }
}

void
net_paint_clouds(Side *side, int x, int y, int r, int cloudtype, int bot,
		 int hgt)
{
    if (my_rid == master_rid) {
	paint_clouds(side, x, y, r, cloudtype, bot, hgt);
    }
    if (numremotes > 0) {
	broadcast_layer_change("clouds", side, x, y, r, cloudtype, 0, 0);
	broadcast_layer_change("cloud-bottoms", side, x, y, r, bot, 0, 0);
	broadcast_layer_change("cloud-heights", side, x, y, r, hgt, 0, 0);
    }
}

void
net_paint_winds(Side *side, int x, int y, int r, int dir, int force)
{
    if (my_rid == master_rid) {
	paint_winds(side, x, y, r, dir, force);
    }
    if (numremotes > 0) {
	broadcast_layer_change("wind", side, x, y, r, dir, force, 0);
    }
}

static void
broadcast_layer_change(char *layername, Side *side, int x, int y,
		       int a1, int a2, int a3, int a4)
{
    char buf[BUFSIZE];

    sprintf(buf, "W%s %d %d %d %d %d %d %d",
	    layername, side->id, x, y, a1, a2, a3, a4);
    broadcast_packet(buf);
}

#endif /* DESIGNERS */

void
send_quit(void)
{
    /* (should have more error-handling, since may be called in bad
       situations) */
    sprintf(spbuf, "Q%d", my_rid);
    broadcast_packet(spbuf);
    flush_outgoing_queue();
}

/* Given a buffer with a packet in it, send it all the places that
   it should go. */

static void
broadcast_packet(char *buf)
{
    int rid;
    extern int in_run_game;
 
    if (in_run_game) {
	notify_all("Attempting to send \"%s\" while in run_game", buf);
    }
    if (my_rid == master_rid) {
	for_all_remotes(rid) {
	    if (rid != my_rid) {
		send_packet(rid, buf);
	    }
	}
    } else {
	/* Send to master; master will echo back eventually. */
	send_packet(master_rid, buf);
    }
}

/* Given a packet to be sent, package it up (add header, embed escape
   chars, add checksum), send, and maybe wait for acknowledgement. */

int
send_packet(int id, char *inbuf)
{
    int i, j, csum, numtimeouts;
    char buf[BUFSIZE];

    if (my_rid != master_rid && !sendnow) {
	/* Add packet to the outgoing queue. */
	save_outgoing_packet(id, inbuf);
	Dprintf("OutQ: %d \"%s\"\n", id, inbuf);
	return TRUE;
    }
    Dprintf("Send: %d \"%s\"\n", id, inbuf);
    j = 0;
    csum = 0;
    buf[j++] = STARTPKT;
    /* Copy the input buffer into the packet, changing any special
       characters into escape sequences along the way. */
    for (i = 0; inbuf[i] != '\0'; ++i) {
	if (inbuf[i] == STARTPKT) {
	    buf[j++] = ESCAPEPKT;
	    buf[j++] = STARTPKTESC;
	} else if (inbuf[i] == ENDPKT) {
	    buf[j++] = ESCAPEPKT;
	    buf[j++] = ENDPKTESC;
	} else if (inbuf[i] == ESCAPEPKT) {
	    buf[j++] = ESCAPEPKT;
	    buf[j++] = ESCAPEPKTESC;
	} else {
	    buf[j++] = inbuf[i];
	}
	csum += (unsigned char) inbuf[i];
    }
    buf[j++] = ENDPKT;
    /* Add on the checksum, in hex. */
    buf[j++] = tohex ((csum >> 4) & 0xf);
    buf[j++] = tohex (csum & 0xf);
    buf[j++] = '\0';
    if (display_traffic)
      notify_all("To %d: %s", id, inbuf);
    low_send(id, buf);
    /* Wait for the packet's receipt to be acknowledged. */
    numtimeouts = 0;
    expecting_ack = TRUE;
    while (expecting_ack) {
    	time_t start_time, now;
	
    	time(&start_time);
	receive_data(30, MAXPACKETS);
	time(&now);
	if (!expecting_ack) {
	    if (display_traffic)
	      notify_all("From %d: ack after %d s", id, now - start_time);
	    break;
	}
	notify_all("Timed out waiting for ack");
	Dprintf("Timed out waiting for ack\n");
	++numtimeouts;
#if (1)
	if (hosting && my_rid == master_rid) {
	    /* If we're the master, we MUST be assured that our
	       packets have been received, since we've already changed
	       our own state and can't undo it. So we will loop here
	       for a long time, can only get out early if run_warning
	       includes a quit option. */
#endif
	    if (numtimeouts > 200)
	      run_error("Timed out %d times", numtimeouts);
#if (1)
	} else {
	    /* If we're not the master, then we haven't actually
	       changed our own state, and so we can continue on
	       safely.  The user will have to retry whatever state
	       change was desired. */
	    expecting_ack = FALSE;
	    return FALSE;
	}
#endif
    }
    return TRUE;
}

struct q_entry {
  int id;
  char *buf;
  struct q_entry *next;
} *outgoing, *outgoing_last, *incoming, *incoming_last;

static void
save_outgoing_packet(int id, char *inbuf)
{
    struct q_entry *entry;

    entry = (struct q_entry *) xmalloc (sizeof (struct q_entry));
    entry->id = id;
    entry->buf = copy_string(inbuf);
    if (outgoing_last) {
	outgoing_last->next = entry;
    } else {
	outgoing = entry;
    }
    outgoing_last = entry;
}

void
flush_outgoing_queue(void)
{
    struct q_entry *entry;

    if (my_rid != master_rid) {
	sendnow = TRUE;
	for (entry = outgoing; entry != NULL; entry = entry->next) {
	    send_packet(entry->id, entry->buf);
	}
	outgoing = outgoing_last = NULL;
	sendnow = FALSE;
    }
}

static void
save_incoming_packet(int id, char *inbuf)
{
    struct q_entry *entry;

    entry = (struct q_entry *) xmalloc (sizeof (struct q_entry));
    entry->id = id;
    entry->buf = copy_string(inbuf);
    if (incoming_last) {
	incoming_last->next = entry;
    } else {
	incoming = entry;
    }
    incoming_last = entry;
}

/* Flush up to lim number of packets. 
   Return the number of packets actually flushed. */

int
flush_incoming_queue(int lim)
{
    struct q_entry *entry;
    int	packets = 0;
    
    if (display_traffic)
	notify_all("Starting to flush the incoming queue.");
    Dprintf("Starting to flush the incoming queue.\n");
    flushing = TRUE;
    for (entry = incoming; entry != NULL && packets < lim; 
	 entry = entry->next) {
	receive_packet(entry->id, entry->buf);
	packets++;
    }
    if (display_traffic)
	notify_all("%d packets flushed from the incoming queue.", packets);
    Dprintf("%d packets flushed from the incoming queue.\n", packets);
    if (entry == NULL) {
    	if (display_traffic)
	    notify_all("The incoming queue is empty.");
	Dprintf("The incoming queue is empty.\n");
	incoming = incoming_last = NULL;
    } else {
    	if (display_traffic)
	    notify_all("Packets still left in the incoming queue.");
	Dprintf("Packets still left in the incoming queue.\n");
    }
    flushing = FALSE;
    return packets;
}

static char *packetbuf;

static char *rsltbuf;

static int rsltid;

#define PACKETBUFSIZE 1000

static time_t nothing_start_time;

static int nothing_count;

static int nothing_timeout;

static void
remove_chars(char *buf, int n)
{
    int i;

    for (i = 0; i < PACKETBUFSIZE - n; ++i)
      buf[i] = buf[i + n];
}

/* Handle the basic process of receiving up to lim number of packets. */

void
receive_data(int timeout, int lim)
{
    char buf[2000], *pktend;
    int retry = 10, gotsome, i, j, len, csum, pktcsum, packets = 0;

    /* (should have buffers for each remote prog) */
    if (packetbuf == NULL)
      packetbuf = (char *)xmalloc(PACKETBUFSIZE);
    if (rsltbuf == NULL)
      rsltbuf = (char *)xmalloc(BUFSIZE);
    if (incoming != NULL && process_packets && !expecting_ack) {
	packets += flush_incoming_queue(lim - packets);
	return;
    }
    while (retry > 0 && packets < lim) {
	if (packetbuf[0] == STARTPKT) {
	    pktend = strchr(packetbuf, ENDPKT);
	    if (pktend != NULL
		&& strlen(packetbuf) >= (pktend - packetbuf + 3)) {
		/* We have accumulated a whole packet. */
		len = pktend - packetbuf - 1;
		/* Copy out the packet's contents, handling escape
		   chars and computing checksum along the way. */
		j = 0;
		csum = 0;
		for (i = 1; i < len + 1; ++i) {
		    if (packetbuf[i] == ESCAPEPKT
			&& packetbuf[i+1] == STARTPKTESC) {
			rsltbuf[j++] = STARTPKT;
			++i;
		    } else if (packetbuf[i] == ESCAPEPKT
			       && packetbuf[i+1] == ENDPKTESC) {
			rsltbuf[j++] = ENDPKT;
			++i;
		    } else if (packetbuf[i] == ESCAPEPKT
			       && packetbuf[i+1] == ESCAPEPKTESC) {
			rsltbuf[j++] = ESCAPEPKT;
			++i;
		    } else {
			rsltbuf[j++] = packetbuf[i];
		    }
		    csum += (unsigned char) rsltbuf[j-1];
		}
		rsltbuf[j++] = '\0';
		csum &= 0xff;
		/* Extract the checksum that was sent. */
		pktcsum = fromhex(packetbuf[len + 2]) << 4;
		pktcsum |= fromhex(packetbuf[len + 3]);
		/* Remove the packet from the buffer. */
		remove_chars(packetbuf, len + 4);
		if (csum != pktcsum && pktcsum != 0) {
		    run_warning("checksum error, received %x, calced %x",
				pktcsum, csum);
		    /* what to do with the packet? should require resend */
		}
		if (expecting_ack) {
		    Dprintf(
"Packet received while expecting ack, saving \"%s\"\n", rsltbuf);
		    save_incoming_packet(rsltid, rsltbuf);
		    /* (Should we ack an unexpected packet?) */
		    Dprintf("Sending ack to unexpected packet\n");
		    low_send(rsltid, "+");
#if (1)
		    /* Check if the unexpected packet was followed by the ack 
		       that we were looking for. This solved all the host 
		       timeout problems with the network code. */
		    if (packetbuf[0] == '+') {
		    	expecting_ack = FALSE;
			Dprintf("Trailing ack received\n");
			remove_chars(packetbuf, 1);
			/* Immediately flush the unexpected packet that was 
			   saved while we were waiting for the ack. 
			   The test for flushing is to avoid infinite 
			   recursion due to rebroadcasting by the master of 
			   packets that are flushed. */
	    		if (process_packets && flushing == FALSE) {
			    packets += flush_incoming_queue(lim - packets);
			}
			/* Always returning here instead of going to
			  look_for_data solved the problem with
			  occasional 30 sec lockups of the host when
			  playing against a tcltk client. These cases
			  would always trigger the characteristic
			  debug message "Rcvd nothing 0 times (timeout
			  0 secs)" below. */
			return;
		    }
#endif
		    goto look_for_data;
		}
		if (rsltid == 0) {
		    run_warning("Packet from rid == 0, discarded \"%s\"",
				rsltbuf);
		    goto look_for_data;
		}
		Dprintf("Sending ack\n");
		if (display_traffic)
		  notify_all("From %d: %s", rsltid, rsltbuf);
		low_send(rsltid, "+");
		/* The interpretation process needs to be last, 
		   because it may in turn cause more packets to be sent. */
		if (process_packets /*&& flushing == FALSE*/ && packets < lim) {
		    receive_packet(rsltid, rsltbuf);
		/* We dont want to process the packet right now. */
		} else {
		    save_incoming_packet(rsltid, rsltbuf);
		}
		retry = 0;
	    } else {
		/* Part of a packet, but not the whole thing -
                   continue waiting. */
		--retry;
		goto look_for_data;
	    }
	/* One of our calls to 'send_packet' will be happy if we receive an 
	   ack, because it can then go ahead a finish up. */
	} else if (packetbuf[0] == '+') {
	    expecting_ack = FALSE;
	    Dprintf("Ack received\n");
	    remove_chars(packetbuf, 1);
	    /* Immediately flush any packets that accumulated while we were
	       waiting for the ack. The test for flushing is to prevent 
	       infinite recursion by the master due to rebroadcasting of 
	       packets that are flushed. */
	    if (incoming != NULL && process_packets && flushing == FALSE) {
		packets += flush_incoming_queue(lim - packets);
		return;
	    }
	    /* If packetbuf is empty we are done. */
	    if (packetbuf[0] == '\0') {
		retry = 0;
	    }
	} else if (strlen(packetbuf) > 0) {
	    Dprintf("Removing garbage char '%c' (0x%x)\n",
		    packetbuf[0], packetbuf[0]);
	    remove_chars(packetbuf, 1);
	} else {
	look_for_data:
	    gotsome = low_receive(&rsltid, buf, BUFSIZE, timeout);
	    /* Debugging printout. */
	    if (gotsome) {
		if (nothing_count > 0) {
		    Dprintf("Rcvd nothing %d times (timeout %d secs)\n",
			    nothing_count, nothing_timeout);
		    nothing_count = 0;
		}
		Dprintf("Rcvd: %d \"%s\"\n", rsltid, (buf ? buf : "<null>"));
	    } else {
		time_t now;

		if (nothing_count == 0)
		  time(&nothing_start_time);
		time(&now);
		if (idifftime(now, nothing_start_time) > 0
		    || timeout != nothing_timeout) {
		    Dprintf("Rcvd nothing %d times (timeout %d secs)\n",
			    nothing_count, nothing_timeout);
		    nothing_count = 0;
		} else {
		    ++nothing_count;
		    nothing_timeout = timeout;
		}
		nothing_timeout = timeout;
	    }
	    if (gotsome) {
		if (strlen(packetbuf) + strlen(buf) > PACKETBUFSIZE) {
		    run_warning("packet buffer overflow");
		    return;
		}
		strcat(packetbuf, buf);
		/* Go around and see if we have something now. */
		--retry;
	    } else {
		retry = 0;
	    }
	}
    }
}

/* Given a received packet, take it apart and do what it says. */

void
receive_packet(int id, char *buf)
{
    int should_rebroadcast;

    Dprintf("From %d: \"%s\"\n", id, buf);
    if (downloading) {
	if (strcmp(buf, "\neludoMemag\n") == 0) {
	    /* We've seen the end-of-module marker, now interpret. */
	    downloading = FALSE;
	    mainmodule = get_game_module("*download*");
	    mainmodule->contents = downloadbuf;
	    /* The sequence below is similar to that in load_game_module. */
	    open_module(mainmodule, FALSE);
	    read_forms(mainmodule);
	    close_module(mainmodule);
	    check_game_validity();
	    start_variant_setup_stage();
	} else {
	    /* Make sure we have space with which to concatenate. */
	    if (strlen(downloadbuf) + strlen(buf) >= 200000) {
		init_error("Exceeded download buffer size");
		return;
	    }
	    /* Concatenate this packet to the others. */
	    strcat(downloadbuf, buf);
	}
    } else {
	should_rebroadcast = TRUE;
	switch (buf[0]) {
	  case 'A':
	    receive_action(buf + 1);
	    break;
	  case 'B':
	    receive_bugoff(id);
	    break;
	  case 'C':
	    receive_command(buf + 1);
	    break;
	  case 'E':
	    receive_error(id, buf + 1);
	    should_rebroadcast = FALSE;
	    break;
	  case 'M':
	    receive_net_message(buf + 1);
	    break;
	  case 'P':
	    receive_player_prop(buf + 1);
	    break;
	  case 'Q':
	    receive_quit(buf + 1);
	    break;
	  case 'R':
	    new_randstate = atoi(buf + 1);
	    if (new_randstate != randstate) {
		Dprintf("Rand state change: %d -> %d\n",
			randstate, new_randstate);
	    } else {
		Dprintf("Rand state matches\n");
	    }
	    randstate = new_randstate;
	    /* This is only issued by the master, no need to rebroadcast. */
	    should_rebroadcast = FALSE;
	    break;
	  case 'S':
	    receive_side_prop(buf + 1);
	    break;
	  case 'T':
	    receive_task(buf + 1);
	    break;
	  case 'U':
	    receive_unit_prop(buf + 1);
	    break;
	  case 'V':
	    if (strcmp(buf + 1, version_string()) != 0) {
		init_warning(
"Xconq versions \"%s\" and \"%s\" should not link up",
			     buf + 1, version_string());
	    }
	    should_rebroadcast = FALSE;
	    break;
	  case 'W':
	    receive_world_prop(buf + 1);
	    break;
	  case 'X':
	    receive_run_game(buf + 1);
	    /* This is only issued by the master, no need to rebroadcast. */
	    should_rebroadcast = FALSE;
	    break;
	  case 'Z':
	    receive_game_checksum(buf + 1);
	    /* Checksums may be broadcast, but not automatically. */
	    should_rebroadcast = FALSE;
	    break;
	  case 'a':
	    if (buf[1] == 'S' && buf[2] == 'T') {
		start_player_setup_stage();
		/* This is only issued by the master, no need to
                   rebroadcast. */
		should_rebroadcast = FALSE;
	    } else if (buf[1] == 'O' && buf[2] == 'K') {
		start_game_ready_stage();
		/* This is only issued by the master, no need to
                   rebroadcast. */
		should_rebroadcast = FALSE;
	    } else {
		receive_assignment_setting(buf + 1);
	    }
	    break;
	  case 'c':
	    receive_chat(buf + 1);
	    break;
	  case 'f':
	  /* We received the filename of a saved game. */
	    if (buf[1] == ' ') {
		Module *module = create_game_module(NULL);
   		module->filename = copy_string(buf + 2);
		mainmodule = module;
		load_game_module(mainmodule, TRUE);
		check_game_validity();
	    } else {
		downloading = TRUE;
		/* (should use obstack here) */
		if (downloadbuf == NULL)
		  downloadbuf = (char *)xmalloc(200000);
		downloadbuf[0] = '\0';
	    }
	    should_rebroadcast = FALSE;
	    break;
	  case 'g':
	  /* We received the name of a library module. */
	    if (buf[1] == ' ') {
		Module *module = get_game_module(buf + 2);
		mainmodule = module;
		load_game_module(mainmodule, TRUE);
		check_game_validity();
	    } else {
		downloading = TRUE;
		/* (should use obstack here) */
		if (downloadbuf == NULL)
		  downloadbuf = (char *)xmalloc(200000);
		downloadbuf[0] = '\0';
	    }
	    should_rebroadcast = FALSE;
	    break;
	  case 'j':
	    /* Only let people join if the game has not yet started. */
	    if (current_stage == initial_stage) {
		    send_remote_id(id);
		    add_remote_program(id, buf + 1);
	    } else {
	    	    send_bugoff(id);
	    }
	    /* Don't rebroadcast, the master will handle specially. */
	    should_rebroadcast = FALSE;
	    break;
	  case 'p':
	    receive_remote_program(buf + 1);
	    /* This is only issued by the master, no need to rebroadcast. */
	    should_rebroadcast = FALSE;
	    break;
	  case 'r':	
	    my_rid = atoi(buf + 1);
	    /* This is only issued by the master, no need to rebroadcast. */
	    should_rebroadcast = FALSE;
	    break;
	  case 's':
	    start_game_load_stage();
	    /* This is only issued by the master, no need to rebroadcast. */
	    should_rebroadcast = FALSE;
	    break; 
	  case 'v':
	    if (buf[1] == 'S' && buf[2] == 'T') {
		start_variant_setup_stage();
		/* This is only issued by the master, no need to
                   rebroadcast. */
		should_rebroadcast = FALSE;
	    } else if (buf[1] == 'O' && buf[2] == 'K') {
		start_player_pre_setup_stage();
		/* This is only issued by the master, no need to
                   rebroadcast. */
		should_rebroadcast = FALSE;
	    } else {
		receive_variant_setting(buf + 1);
	    }
	    break;
	  case 'w':
	    save_game(buf + 1);
	    /* This can be issued by any player. */
	    should_rebroadcast = TRUE;
	    break;
	  default:
	    /* Since the protocol's purpose is to keep multiple
	       executables' state in sync, every kind of packet must
	       be understood. */
	    run_warning("Packet not recognized: \"%s\"\n", buf);
	    break;
	}
	/* The master should rebroadcast most types of packets
	   automatically, so all programs stay in sync. */
	if (my_rid != 0 && my_rid == master_rid && should_rebroadcast)
	  broadcast_packet(buf);
    }
}

static void
receive_net_message(char *str)
{
    int id;
    char *nstr;
    SideMask sidemask;
    Side *side;

    id = strtol(str, &nstr, 10);
    side = side_n(id);
    str = nstr;
    sidemask = strtol(str, &nstr, 10);
    str = nstr;
    ++str;
    /* Note that this is the internal form of "send", which just copies
       to all local recipients. */
    send_message(side, sidemask, str);
}

static void
receive_command(char *str)
{
    int i, args[5];
    char *nstr, *argstr;
    Side *side;
    Unit *unit, *unit2;

    argstr = strchr(str, ' ');
    i = 0;
    while (*argstr != '\0' && i < 5) {
	args[i++] = strtol(argstr, &nstr, 10);
	argstr = nstr;
    }
    if (strncmp(str, "wake-unit ", 9) == 0) {
	side = side_n(args[0]);
	if (side == NULL) {
	    return;
	}
	unit = find_unit(args[1]);
	if (unit == NULL) {
	    return;
	}
	wake_unit(side, unit, args[2]);
    } else if (strncmp(str, "wake-area ", 9) == 0) {
	side = side_n(args[0]);
	if (side == NULL) {
	    return;
	}
	wake_area(side, args[1], args[2], args[3], args[4]);
    } else if (strncmp(str, "designer-create-unit ", strlen("designer-create-unit ")) == 0) {
	side = side_n(args[0]);
	if (side == NULL) {
	    return;
	}
	designer_create_unit(side, args[1], args[2], args[3], args[4]);		
    } else if (strncmp(str, "designer-teleport ", strlen("designer-teleport ")) == 0) {
	unit = find_unit(args[0]);
	if (unit == NULL) {
	    return;
	}
	unit2 = NULL;
	if (args[3] > 0) {
	    unit2 = find_unit(args[3]);
	    if (unit2 == NULL) {
		return;
	    }
	}
	designer_teleport(unit, args[1], args[2], unit2);		
    } else if (strncmp(str, "designer-change-side ", strlen("designer-change-side ")) == 0) {
	unit = find_unit(args[0]);
	if (unit == NULL) {
	    return;
	}
	side = side_n(args[1]);
	designer_change_side(unit, side);		
    } else if (strncmp(str, "designer-disband ", strlen("designer-disband ")) == 0) {
	unit = find_unit(args[0]);
	if (unit == NULL) {
	    return;
	}
	designer_disband(unit);		
    } else {
	run_warning("Unknown C packet \"%s\", ignoring", str);
    }
}

static void
receive_action(char *str)
{
    int unitid, acteeid, i;
    char *nstr;
    Unit *unit;
    Action tmpaction;

    unitid = strtol(str, &nstr, 10);
    str = nstr;
    if (*str == '/') {
	++str;
	acteeid = strtol(str, &nstr, 10);
	str = nstr;
    } else {
	acteeid = unitid;
    }
    tmpaction.actee = acteeid;
    tmpaction.type = (ActionType)strtol(str, &nstr, 10);
    str = nstr;
    i = 0;
    while (*nstr != '\0' && i < 10) {
	tmpaction.args[i++] = strtol(str, &nstr, 10);
	str = nstr;
    }
    unit = find_unit(unitid);
    if (unit == NULL) {
#if 1
	notify_all("RUN WARNING: Packet A refers to missing unit #%d, ignoring", unitid);
#else
	run_warning("Packet A refers to missing unit #%d, ignoring", unitid);
#endif
	return;
    }
    if (unit->act == NULL) {
#if 1
	notify_all("RUN WARNING: Packet A refers to non-acting unit %s, ignoring",
		    unit_desig(unit));
#else	
	run_warning("Packet A refers to non-acting unit %s, ignoring",
		    unit_desig(unit));
#endif
	return;
    }
    /* Lay the action over what is already there; the next run_game
       should actually do it. */
    unit->act->nextaction = tmpaction;
}

static void
receive_task(char *str)
{
    int unitid, pos, tasktype, i;
    char *nstr;
    Unit *unit;
    Task *task;

    unitid = strtol(str, &nstr, 10);
    str = nstr;
    pos = strtol(str, &nstr, 10);
    str = nstr;
    tasktype = strtol(str, &nstr, 10);
    str = nstr;
    task = create_task((TaskType)tasktype);
    task->execnum = strtol(str, &nstr, 10);
    str = nstr;
    task->retrynum = strtol(str, &nstr, 10);
    str = nstr;
    i = 0;
    while (*nstr != '\0' && i < 10) {
	task->args[i++] = strtol(str, &nstr, 10);
	str = nstr;
    }
    unit = find_unit(unitid);
    if (unit == NULL) {
#if 1
	notify_all("RUN WARNING: Packet T refers to missing unit #%d, ignoring", unitid);
#else
	run_warning("Packet T refers to missing unit #%d, ignoring", unitid);
#endif
	return;
    }
    if (unit->plan == NULL) {
#if 1
	notify_all("RUN WARNING: Packet T refers to non-planning unit %s, ignoring",
		    unit_desig(unit));
#else	
	run_warning("Packet T refers to non-planning unit %s, ignoring",
		    unit_desig(unit));
#endif
	return;
    }
    add_task(unit, pos, task);
}

static void
receive_player_prop(char *str)
{
    int val, val2;
    char *nstr;
    Side *side;

    if (strncmp(str, "add ", 4) == 0) {
	str += 4;
	request_additional_side(str);
    } else if (strncmp(str, "assign ", 7) == 0) {
	str += 7;
	val = strtol(str, &nstr, 10);
	str = nstr;
	val2 = strtol(str, &nstr, 10);
	side = side_n(val);
	if (side != NULL)
	  side->player = find_player(val2);
    } else {
	run_warning("Unknown P packet \"%s\", ignoring", str);
    }
}

static void
receive_quit(char *str)
{
    int rid;
    char *nstr;

    rid = strtol(str, &nstr, 10);
    Dprintf("Received quit from %d\n", rid);
    online[rid] = FALSE;
    quitter = rid;
}

static void
receive_bugoff(int rid)
{
    Dprintf("Received bug off from host\n");
    master_rid = 0;
    bounced = TRUE;
}

static void
receive_side_prop(char *str)
{
    int id, sn, val;
    char *nstr;
    Side *side;
    Unit *unit;

    id = strtol(str, &nstr, 10);
    side = side_n(id);
    if (side == NULL)
      return;
    str = nstr;
    ++str;
    if (strncmp(str, "adjective ", 10) == 0) {
	set_side_adjective(side, side, str + 10);
    } else if (strncmp(str, "af ", 3) == 0) {
	str += 3;
	val = strtol(str, &nstr, 10);
	set_autofinish(side, val);
    } else if (strncmp(str, "ar ", 3) == 0) {
	str += 3;
	val = strtol(str, &nstr, 10);
	set_autoresearch(side, val);
    } else if (strncmp(str, "ai ", 3) == 0) {
	str += 3;
	set_side_ai(side, str);
    } else if (strncmp(str, "colorscheme ", 12) == 0) {
	set_side_colorscheme(side, side, str + 12);
	} else if (strncmp(str, "controlledby ", 13) == 0) {
	str += 13;
	sn = strtol(str, &nstr, 10);
	str = nstr;
	val = strtol(str, &nstr, 10);
	set_controlled_by(side, side_n(sn), val);
    } else if (strncmp(str, "designer ", 9) == 0) {
	str += 9;
	val = strtol(str, &nstr, 10);
	if (val)
	  become_designer(side);
	else
	  become_nondesigner(side);
    } else if (strncmp(str, "doctrine ", 9) == 0) {
	str += 9;
	set_doctrine(side, str);
    } else if (strncmp(str, "draw ", 5) == 0) {
	str += 5;
	val = strtol(str, &nstr, 10);
	set_willing_to_draw(side, val);
    } else if (strncmp(str, "emblemname ", 11) == 0) {
	set_side_emblemname(side, side, str + 11);
    } else if (strncmp(str, "fin ", 4) == 0) {
	finish_turn(side);
    } else if (strncmp(str, "longname ", 9) == 0) {
	set_side_longname(side, side, str + 9);
    } else if (strncmp(str, "name ", 5) == 0) {
	set_side_name(side, side, str + 5);
    } else if (strncmp(str, "noun ", 5) == 0) {
	set_side_noun(side, side, str + 5);
    } else if (strncmp(str, "pluralnoun ", 11) == 0) {
	set_side_pluralnoun(side, side, str + 11);
    } else if (strncmp(str, "resign ", 7) == 0) {
	str += 7;
	val = strtol(str, &nstr, 10);
	resign_game(side, (val < 0 ? NULL : side_n(val)));
    } else if (strncmp(str, "save ", 5) == 0) {
	str += 5;
	val = strtol(str, &nstr, 10);
	set_willing_to_save(side, val);
    } else if (strncmp(str, "self ", 5) == 0) {
	str += 5;
	val = strtol(str, &nstr, 10);
	unit = find_unit(val);
	set_side_self_unit(side, unit);
    } else if (strncmp(str, "research_topic ", 15) == 0) {
	str += 15;
	val = strtol(str, &nstr, 10);
	set_side_research_topic(side, val);
    } else if (strncmp(str, "research_goal ", 14) == 0) {
	str += 14;
	val = strtol(str, &nstr, 10);
    } else if (strncmp(str, "shortname ", 10) == 0) {
	set_side_shortname(side, side, str + 10);
    } else if (strncmp(str, "startx ", 7) == 0) {
	str += 7;
	val = strtol(str, &nstr, 10);
	set_side_startx(side, val);
    } else if (strncmp(str, "starty ", 7) == 0) {
	str += 7;
	val = strtol(str, &nstr, 10);
	set_side_starty(side, val);
    } else if (strncmp(str, "trust ", 6) == 0) {
	str += 6;
	sn = strtol(str, &nstr, 10);
	str = nstr;
	val = strtol(str, &nstr, 10);
	set_trust(side, side_n(sn), val);
    } else {
	run_warning("Unknown S packet \"%s\", ignoring", str);
    }
}

static void
receive_unit_prop(char *str)
{
    int sid, uid, val, val2;
    char *nstr;
    Unit *unit;
    Side *side;

    /* Collect the side. */
    sid = strtol(str, &nstr, 10);
    side = side_n(sid);
    /* Note that the side may be NULL. */
    str = nstr;
    /* Collect the unit. */
    uid = strtol(str, &nstr, 10);
    unit = find_unit(uid);
    if (unit == NULL) {
#if 1
	notify_all("RUN WARNING: Packet with invalid unit id %d, ignoring packet", uid);
#else
	run_warning("Packet with invalid unit id %d, ignoring packet", uid);
#endif
	return;
    }
    str = nstr;
    ++str;
    /* Decode the property being set. */
    if (strncmp(str, "ai ", 3) == 0) {
	val = strtol(str + 3, &nstr, 10);
	str = nstr;
	val2 = strtol(str, &nstr, 10);
	set_unit_ai_control(side, unit, val, val2);
    } else if (strncmp(str, "clragenda ", 5) == 0) {
	clear_task_agenda(unit);
    } else if (strncmp(str, "clroutcome ", 5) == 0) {
	clear_task_outcome(unit);
    } else if (strncmp(str, "delay ", 6) == 0) {
	val = strtol(str + 6, &nstr, 10);
	delay_unit(unit, val);
    } else if (strncmp(str, "disband ", 8) == 0) {
	disband_unit(side, unit);
    } else if (strncmp(str, "forcereplan ", 12) == 0) {
	force_replan(unit);
    } else if (strncmp(str, "formation ", 10) == 0) {
	run_warning("need to interp formation packet");
    } else if (strncmp(str, "maingoal ", 9) == 0) {
	Goal *goal;
	int i;

	str += 9;
	val = strtol(str, &nstr, 10);
	if (val > 0) {
	    str = nstr;
	    goal = create_goal((GoalType)val, side, TRUE);
	    for (i = 0; i < 4; ++i) {
		val = strtol(str, &nstr, 10);
		str = nstr;
		goal->args[i] = val;
	    }
	} else {
	    goal = NULL;
	}
	set_unit_main_goal(side, unit, goal);
    } else if (strncmp(str, "curadvance ", 11) == 0) {
	val = strtol(str + 11, &nstr, 10);
	set_unit_curadvance(side, unit, val);
    } else if (strncmp(str, "researchdone ", 13) == 0) {
	val = strtol(str + 13, &nstr, 10);
	set_unit_researchdone(side, unit, val);
    } else if (strncmp(str, "name ", 5) == 0) {
	set_unit_name(side, unit, str + 5);
    } else if (strncmp(str, "plan ", 5) == 0) {
	val = strtol(str + 5, &nstr, 10);
	set_unit_plan_type(side, unit, val);
    } else if (strncmp(str, "resv ", 5) == 0) {
	str += 5;
	val = strtol(str, &nstr, 10);
	str = nstr;
	val2 = strtol(str, &nstr, 10);
	set_unit_reserve(side, unit, val, val2);
    } else if (strncmp(str, "sleep ", 6) == 0) {
	str += 6;
	val = strtol(str, &nstr, 10);
	str = nstr;
	val2 = strtol(str, &nstr, 10);
	set_unit_asleep(side, unit, val, val2);
    } else if (strncmp(str, "waittrans ", 10) == 0) {
	str += 10;
	val = strtol(str, &nstr, 10);
	set_unit_waiting_for_transport(side, unit, val);
    } else {
	run_warning("Unknown U packet \"%s\", ignoring", str);
    }
}

static void
receive_world_prop(char *str)
{
    int id, x, y, a1, a2, a3, a4;
    char *str1, *str2, *nstr;
    Side *side;

    /* This is necessary since we modify the string below. */
    str1 = copy_string(str);
    str2 = strchr(str1, ' ');
    if (str2 == NULL) {
	return;
    }
    *str2 = '\0';
    ++str2;
    id = strtol(str2, &nstr, 10);
    str2 = nstr;
    x = strtol(str2, &nstr, 10);
    str2 = nstr;
    y = strtol(str2, &nstr, 10);
    str2 = nstr;
    a1 = strtol(str2, &nstr, 10);
    str2 = nstr;
    a2 = strtol(str2, &nstr, 10);
    str2 = nstr;
    a3 = strtol(str2, &nstr, 10);
    str2 = nstr;
    a4 = strtol(str2, &nstr, 10);
    side = side_n(id);
    if (strcmp(str1, "cell") == 0) {
	paint_cell(side, x, y, a1, a2);
    } else if (strcmp(str1, "bord") == 0) {
	paint_border(side, x, y, a1, a2, a3);
    } else if (strcmp(str1, "conn") == 0) {
	paint_connection(side, x, y, a1, a2, a3);
    } else if (strcmp(str1, "coat") == 0) {
	paint_coating(side, x, y, a1, a2, a3);
    } else if (strcmp(str1, "peop") == 0) {
	paint_people(side, x, y, a1, a2);
    } else if (strcmp(str1, "ctrl") == 0) {
	paint_control(side, x, y, a1, a2);
    } else if (strcmp(str1, "feat") == 0) {
	paint_feature(side, x, y, a1, a2);
    } else if (strcmp(str1, "elev") == 0) {
	paint_elevation(side, x, y, a1, a2, a3, a4);
    } else if (strcmp(str1, "temp") == 0) {
	paint_temperature(side, x, y, a1, a2);
    } else if (strcmp(str1, "m") == 0) {
	paint_material(side, x, y, a1, a2, a3);
    } else if (strcmp(str1, "wind") == 0) {
	paint_winds(side, x, y, a1, a2, a3);
    } else if (strcmp(str1, "user") == 0) {
    	toggle_user_at(find_unit(a1), x, y);
    } else {
	run_warning("Unknown W packet \"%s\", ignoring", str);
    }
}

static void
receive_run_game(char *str)
{
    int maxactions, newsernum;
    char *reststr, *nreststr;

    maxactions = strtol(str, &reststr, 10);
    newsernum = strtol(reststr, &nreststr, 10);
    reststr = nreststr;
    new_randstate = strtol(reststr, &nreststr, 10);
    set_g_run_serial_number(newsernum);
    if (new_randstate != randstate) {
	Dprintf("Rand state change: %d -> %d\n", randstate, new_randstate);
    } else {
	Dprintf("Rand state matches\n");
    }
    randstate = new_randstate;
    /* This where non-masters actually call run_game. */
    run_game(maxactions);
}

static void
receive_game_checksum(char *str)
{
    int before_run = FALSE, other_rid, other_csum, our_csum;
    char *reststr, *nreststr;

    if (*str == 'a') {
	before_run = TRUE;
	++str;
    }
    other_rid = strtol(str, &reststr, 10);
    other_csum = strtol(reststr, &nreststr, 10);
    our_csum = game_checksum();
    if (our_csum != other_csum) {
	/* If we're about to lose, make sure the master knows about it. */
	if (my_rid != master_rid)
	  send_game_checksum_error(master_rid, our_csum, other_csum);
	/* This would be a warning normally, but being out of sync
	   isn't always a real problem apparently... */
	notify_all("Game is out of sync! (received %d from %d, computed %d%s)",
		    other_csum, other_rid, our_csum,
		    (before_run ? ", before run_game" : ""));
	Dprintf("Game is out of sync! (received %d from %d, computed %d%s)\n",
		other_csum, other_rid, our_csum, 
		(before_run ? ", before run_game" : ""));
	if (!dumped_checksums) {
		sprintf(tmpbuf, 
			"CLIENT (rid %d) checksums at first sync error:", 
			my_rid);
		dump_checksums(tmpbuf);
	    	dumped_checksums = TRUE;
    		notify_all("Checksums dumped into Xconq-Client.Csums");
		Dprintf("Checksums dumped into Xconq-Client.Csums\n");
	}
    }
}

/* Compute a single integer derived from the contents of the game state. */

#define add_int_to_checksum(cs, x) ((cs) += (x));

int
game_checksum(void)
{
    int csum, i;
    Side *side;
    Unit *unit;
    Plan *plan;
    Task *task;

    csum = 0;
    for_all_sides(side) {
	add_int_to_checksum(csum, side->id);
	add_int_to_checksum(csum, side->self_unit_id);
	add_int_to_checksum(csum, side->controlled_by_id);
	add_int_to_checksum(csum, side->ingame);
	add_int_to_checksum(csum, side->everingame);
	add_int_to_checksum(csum, side->status);
	add_int_to_checksum(csum, side->willingtodraw);
	add_int_to_checksum(csum, side->autofinish);
	add_int_to_checksum(csum, side->finishedturn);
	add_int_to_checksum(csum, side->advantage);
    }
    for_all_units(unit) {
	add_int_to_checksum(csum, unit->id);
	add_int_to_checksum(csum, unit->number);
	add_int_to_checksum(csum, unit->x);
	add_int_to_checksum(csum, unit->y);
	add_int_to_checksum(csum, unit->z);
	add_int_to_checksum(csum, unit->hp);
	if (unit->transport) {
	    add_int_to_checksum(csum, unit->transport->id);
	}
        add_int_to_checksum(csum, unit->creation_id);
	plan = unit->plan;
	if (unit->plan) {
	    add_int_to_checksum(csum, plan->type);
	    add_int_to_checksum(csum, plan->asleep);
	    add_int_to_checksum(csum, plan->reserve);
	    add_int_to_checksum(csum, plan->delayed);
	    add_int_to_checksum(csum, plan->waitingfortasks);
	    add_int_to_checksum(csum, plan->aicontrol);
	    for_all_tasks(plan, task) {
		add_int_to_checksum(csum, task->type);
		for (i = 0; i < MAXTASKARGS; ++i)
		  add_int_to_checksum(csum, task->args[i]);
		add_int_to_checksum(csum, task->execnum);
		add_int_to_checksum(csum, task->retrynum);
	    }
	}
    }
    return csum;
}

/* Checksums go to a file. */

static void
csum_printf(char *str, ...)
{
	va_list ap;

	va_start(ap, str);
	vfprintf(cfp, str, ap);
	va_end(ap);
}


void
dump_checksums(char *str)
{
	Side *side;
	Unit *unit;
	Plan *plan;
	Task *task;
	int i;

	if (my_rid == 0) {
		/* Should never happen. */
		cfp = open_file("Xconq.Csums", "a");
	} else if (my_rid == master_rid) {
		cfp = open_file("Xconq-Master.Csums", "a");
	} else {
		cfp = open_file("Xconq-Client.Csums", "a");
	}
	csum_printf("%s\n\n", str);
	csum_printf("GAME  %s\n\n", mainmodule->name);
	csum_printf("TURN %d  RUN #%d\n\n", g_turn(), g_run_serial_number());
	csum_printf("RANDSTATE %d\n\n", randstate);
	for_all_sides(side) {
		csum_printf("SIDE %d %s\n", side->id, 
			    shortest_side_title(side, tmpbuf));
		csum_printf("self_unit_id %d\n", side->self_unit_id);
		csum_printf("controlled_by_id %d\n", side->controlled_by_id);
		csum_printf("ingame %d\n", side->ingame);
		csum_printf("everingame %d\n", side->everingame);
		csum_printf("status %d\n", side->status);
		csum_printf("willingtodraw %d\n", side->willingtodraw);
		csum_printf("autofinish %d\n", side->autofinish);
		csum_printf("finishedturn %d\n", side->finishedturn);
		csum_printf("advantage %d\n\n", side->advantage);
	}
	for_all_units(unit) {
		csum_printf("UNIT #%d %s\n", unit->id, short_unit_handle(unit));
		csum_printf("side %d\n", unit->side->id);
		csum_printf("type %s\n", u_type_name(unit->type));
		csum_printf("number %d\n", unit->number);
		csum_printf("x %d\n", unit->x);
		csum_printf("y %d\n", unit->y);
		csum_printf("z %d\n", unit->z);
		csum_printf("hp %d\n", unit->hp);
		if (unit->transport) {
			csum_printf("transport %d\n", unit->transport->id);
		}
		plan = unit->plan;
		if (unit->plan) {
			csum_printf("	PLAN %s\n", capitalize(plantypenames[plan->type]));
			csum_printf("	asleep %d\n", plan->asleep);
			csum_printf("	reserve %d\n", plan->reserve);
			csum_printf("	delayed %d\n", plan->delayed);
			csum_printf("	waitingfortasks %d\n", plan->waitingfortasks);
			csum_printf("	aicontrol %d\n", plan->aicontrol);
			for_all_tasks(plan, task) {
				csum_printf("		TASK %s\n", capitalize(taskdefns[task->type].display_name));
				for (i = 0; i < MAXTASKARGS; ++i) {
					csum_printf("		args %d: %d\n", i, task->args[i]);
				}
				csum_printf("		execnum %d\n", task->execnum);
				csum_printf("		retrynum %d\n", task->retrynum);
			}
		}
		csum_printf("\n");
	}
	fclose(cfp);
}

static void
receive_error(int id, char *str)
{
    /* (should decode specific types of errors?) */
    notify_all("Error from #%d, \"%s\"", id, str);
    Dprintf("Error from #%d, \"%s\"\n", id, str);

    if (!dumped_checksums) {
	sprintf(tmpbuf, "MASTER (rid %d) checksums at first sync error:", my_rid);
	dump_checksums(tmpbuf);
	dumped_checksums = TRUE;
	notify_all("Checksums dumped into Xconq-Master.Csums");
	Dprintf("Checksums dumped into Xconq-Master.Csums\n");
    }
}

static void
receive_remote_program(char *str)
{
    int rid;
    char *nstr;

    rid = strtol(str, &nstr, 10);
    str = nstr + 1;
    numremotes = max(rid, numremotes);
    online[rid] = TRUE;
    remote_player_specs[rid] = copy_string(str);
    /* (should do with a function pointer) */
    add_remote_locally(rid, str);
}

static void
receive_chat(char *str)
{
    int rid;
    char *nstr;

    rid = strtol(str, &nstr, 10);
    str = nstr + 1;
    /* (should do with a function pointer) */
    send_chat(rid, str);
}

static void
receive_variant_setting(char *str)
{
    int which, v1, v2, v3;
    char *nstr;

    ++str;
    which = strtol(str, &nstr, 10);
    str = nstr + 1;
    v1 = strtol(str, &nstr, 10);
    str = nstr + 1;
    v2 = strtol(str, &nstr, 10);
    str = nstr + 1;
    v3 = strtol(str, &nstr, 10);
    set_variant_value(which, v1, v2, v3);
    if (update_variant_callback)
      (*update_variant_callback)(which);
}

static void
receive_assignment_setting(char *str)
{
    int n, n2, val;
    char *nstr, *aitype;
    Player *player;

    n = strtol(str, &nstr, 10);
    str = nstr;
    ++str;
    if (strncmp(str, "add", 3) == 0) {
	n = add_side_and_player();
    } else if (strncmp(str, "remove ", 7) == 0) {
	n = strtol(str + 7, NULL, 10);
	remove_side_and_player(n);
  	if (update_assignment_callback) {
		for (n2 = n; n2 <= numsides + 1; n2++) {
			(*update_assignment_callback)(n2);
		}
	} 
    } else if (strncmp(str, "advantage ", 10) == 0) {
	val = strtol(str + 10, NULL, 10);
	player = assignments[n].player;
	if (player != NULL) {
	    player->advantage = val;
	}
    } else if (strncmp(str, "ai ", 3) == 0) {
	aitype = NULL;
	if (str[3] != '\0')
	  aitype = copy_string(str + 3);
	set_ai_for_player(n, aitype);
    } else if (strncmp(str, "exchange ", 9) == 0) {
	n2 = strtol(str + 9, NULL, 10);
	n2 = exchange_players(n, n2);
	if (update_assignment_callback)
	  (*update_assignment_callback)(n2);
    } else if (strncmp(str, "rename ", 7) == 0) {
	n2 = strtol(str + 7, NULL, 10);
	rename_side_for_player(n, n2);
    } else if (strncmp(str, "update ", 7) == 0) {
	nstr = str + 7;
	player = find_player(n);
	if (player == NULL)
	  player = add_player();
	parse_player_spec(player, nstr);
	return; /* a hack, need to handle callback below when windows not up */
    } else {
	run_error("assignment packet \"%s\" not recognized", str);
    }
    if (n >= 0) {
	/* Most settings affect the same assignment, so always update it
	   here. */
	if (update_assignment_callback)
	  (*update_assignment_callback)(n);
    }
}

/* Convert hex digit A to a number.  */

static int
fromhex(int a)
{
    if (a >= '0' && a <= '9')
      return a - '0';
    else if (a >= 'a' && a <= 'f')
      return a - 'a' + 10;
    else 
      run_warning ("Reply contains invalid hex digit %d", a);
    return 0;
}

/* Convert number NIB to a hex digit.  */

static int
tohex(int nib)
{
    if (nib < 10)
      return '0' + nib;
    else
      return 'a' + nib - 10;
}

/* (this routine is almost certainly incorrect now - only used on Macs) */
/* Wrong. This code is being used by the x11 interface. */

static void send_assignment(int id, Side *side, Player *player);
static void send_randstate(int id);
                                                  
void
download_to_player(Player *player)
{
    int i;

    download_game_module(player->rid);
    /* Send it the current list of side/player assignments. */
    for (i = 0; i < numsides; ++i) {
	send_assignment(player->rid, assignments[i].side,
					assignments[i].player);
    }
                                                  
    /* Make it run everything up to the player setup phase. */
    send_randstate(player->rid);
    /* If errors, should set player rid back to zero */                                                  
 }

void
send_assignment(int rid, Side *side, Player *player)
{
    sprintf(spbuf, "Passign %d %d", side->id, player->id);
    send_packet(rid, spbuf);                                               
 }

void
send_randstate(int rid)
{
    sprintf(spbuf, "R%ld", randstate);
    send_packet(rid, spbuf);                                              
 }

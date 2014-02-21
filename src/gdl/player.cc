// xConq
// Lifecycle management and ser/deser of players.

// $Id: player.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006	    Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Lifecycle management and ser/deser of players.
    \ingroup grp_gdl
*/

#include "gdl/side/side.h"
#include "gdl/player.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Player *playerlist;
Player *last_player;
int numplayers;

// Global Variables: Counters

int nextplayerid;

// Local Variables: Buffers

//! Buffer to store a player designator.
static char *playerdesigbuf = NULL;

// Local Function Declarations: Game Setup

//! Read player properties from GDL form.
static void fill_in_player(struct a_player *player, Obj *props);

// Queries

Player *
find_player(int n)
{
    Player *player;

    for_all_players(player) {
       if (player->id == n)
	return player;
    }
    return NULL;
}

char *
player_desig(Player *player)
{
    if (playerdesigbuf == NULL)
      playerdesigbuf = (char *)xmalloc(BUFSIZE);
    if (player != NULL) {
	snprintf(playerdesigbuf, BUFSIZE, "%s,%s/%s@%s+%d",
		(player->name ? player->name : ""),
		(player->aitypename ? player->aitypename : ""),
		(player->configname ? player->configname : ""),
		(player->displayname ? player->displayname : ""),
		player->advantage);
    } else {
	sprintf(playerdesigbuf, "nullplayer");
    }
    return playerdesigbuf;
}

// GDL I/O

void
canonicalize_player(Player *player)
{
    if (player == NULL)
      return;
    if (empty_string(player->displayname))
      player->displayname = NULL;
    if (empty_string(player->aitypename))
      player->aitypename = NULL;
    /* This seems like a logical place to canonicalize an AI type
       of "ai" into a specific type, but we don't necessarily know
       the best default until the game is closer to starting. */
}

static
void
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

void
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

void
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

void
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

// Game Setup

Player *
add_player(void)
{
    Player *player = (Player *) xmalloc(sizeof(Player));

    player->id = nextplayerid++;
    /* Note that all names and suchlike slots are NULL. */
    ++numplayers;
    /* Add this one to the end of the player list. */
    if (last_player == NULL) {
	playerlist = last_player = player;
    } else {
	last_player->next = player;
	last_player = player;
    }
    Dprintf("Added player #%d\n", player->id);
    return player;
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

// xConq
// Players

// $Id: player.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

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
    \brief Players
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_PLAYER_H
#define XCONQ_GDL_PLAYER_H

#include "gdl/lisp.h"

// Iterator Macros

//! Iterate over all players.
#define for_all_players(v) \
    for (v = playerlist; v != NULL; v = v->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Player.
/*! This structure holds the player information. */
typedef struct a_player {
    //! Unique ID.
    short id;
    //! Name of player.
    char *name;
    //! Name of configuration.
    char *configname;
    //! Name of desired display.
    char *displayname;
    //! Name of AI type.
    char *aitypename;
    //! Player's desired initial advantage.
    short advantage;
    //! Name of remote desired for player.
    char *remotewanted;
    //! Encrypted password of player.
    char *password;
    //! Peer ID.
    int rid;
    //! Side associated with player.
    struct a_side *side;
    //! Next player.
    struct a_player *next;
} Player;

// Global Variables

//! List of players.
extern Player *playerlist;
//! Last player in list.
extern Player *last_player;
//! Number of players.
extern int numplayers;

// Global Variables: Counters

//! Use to generate ID number of each player.
extern int nextplayerid;

// Queries

//! Get player from ID.
extern Player *find_player(int n);

//! Get designator string for given player.
/*! Make a printable identification of the given player. The output
    here must be correctly parseable according to player spec rules,
    although it doesn't have to be pretty or minimal.
*/
extern char *player_desig(Player *player);

// Game Setup

//! Create new player data structure.
/*! Also adds to list of players. */
extern Player *add_player(void);

// GDL I/O

//! Transform player object into regularized form.
extern void canonicalize_player(Player *player);

//! Read player from GDL form.
extern void interp_player(Obj *form);

//! Serialize player to GDL.
extern void write_player(struct a_player *player);
//! Serialize players to GDL.
extern void write_players(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_PLAYER_H

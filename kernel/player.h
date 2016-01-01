/* Definitions for players in Xconq.
   Copyright (C) 1992, 1993, 1994, 1996, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/player.h
 * \brief Definitions for players in Xconq.
 */

/*! \brief Player.
 *
 * This structure holds the player information.
 */
typedef struct a_player {
    short id;               	/*!< unique id for the player */
    const char *name;          	/*!< proper name of the player */
    const char *configname;    	/*!< name of a particular configuration */
    const char *displayname;   	/*!< name of the desired display */
    const char *aitypename;    	/*!< name of an AI type */
    short advantage;        	/*!< player's desired initial advantage */
    const char *remotewanted;  	/*!< name of remote desired for player */
    const char *password;      	/*!< encrypted password of the player */
    int rid;                	/*!< identifier of the player's program */
    struct a_side *side;    	/*!< the \ref a_side "Side" being played */
    struct a_player *next;	/*!< pointer to the next Player */
} Player;

/*! \brief Player/side assignment.
 *
 * This is the mapping between players and sides. */
typedef struct {
    struct a_side *side;        /*!< the \ref a_side "Side" */
    struct a_player *player;	/*!< the \Player assigned to the side */
    int locked;                 /*!< true if the assignment can't be changed */
} Assign;

/*! \brief Iteration over all players.
 *
 * For header for iterating through all \Players.
 * \param v is the player iteration variable.
 */
#define for_all_players(v) \
    for (v = playerlist; v != NULL; v = v->next)

/*! \brief List of \Players. */
extern Player *playerlist;
/*! \brief Last \Player in list. */
extern Player *last_player;

/*! \brief List of player/side assignments */
extern Assign *assignments;

extern Player *add_player(void);
extern Player *find_player(int n);
extern void canonicalize_player(Player *player);
extern char *player_desig(Player *player);

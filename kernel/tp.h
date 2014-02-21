/* Definitions for game transport protocol.
   Copyright (C) 1996, 1997, 1999, 2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/tp.h
 * \brief Definitions for game transport protocol.
 */

/* \brief Remote user interface. */
typedef struct a_rui {
    int rid;            /*!< Remote id. */
    int active;     	/*!< Active flag. */
} RUI;

/*! \brief Remote AI. */
typedef struct a_rai {
    int active;    	/*!< Active flag. */
} RAI;

/*! \brief Is hosting? */
extern int hosting;
/*! \brief Number of remotes. */
extern int numremotes;
/*! \brief Number of remotes waiting. */
extern int numremotewaiting;
/*! \brief My remote id. */
extern int my_rid;
/*! \brief Master remote id. */
extern int master_rid;
/*! \brief Temporary remote id. */
extern int tmprid;
/*! \brief Download flag. */
extern int downloading;
/*! \brief Send now flag. */
extern int sendnow;
/*! \brief Default player specification. */
extern char *default_player_spec;



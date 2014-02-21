// xConq
// Lifecycle management and ser/deser of scorekeepers and scores.

// $Id: score.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of scorekeepers and scores.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_SCORE_H
#define XCONQ_GDL_SCORE_H

#include "gdl/lisp.h"
#include "gdl/side/side.h"

// Iterator Macros

//! Iterate over all scorekeepers.
#define for_all_scorekeepers(sk)  \
  for ((sk) = scorekeepers;  (sk) != NULL;  (sk) = (sk)->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Scorekeeper Data
typedef struct a_scorekeeper {
    //! Unique ID number.
    short id;
    //! Title to display.
    char *title;
    //! When to evaluate.
    Obj *when;
    //! Which sides to apply to.
    Obj *who;
    //! Bitvector of sides to apply to.
    SideMask whomask;
    //! Which sides know about this scorekeeper.
    Obj *knownto;
    //! Bitvector of sides known to.
    SideMask knowntomask;
    //! Keep score?
    short keepscore;
    //! Initial value of numeric score.
    int initial;
    //! Test to decide when to trigger.
    Obj *trigger;
    //! Triggered?
    short triggered;
    //! Body to evaluate.
    Obj *body;
    //! How to record in scorefile.
    Obj *record;
    //! Notes on the scorekeeper.
    Obj *notes;
    //! Index of this scorekeeper's index into scores array.
    short scorenum;
    //! Next scorekeeper.
    struct a_scorekeeper *next;
} Scorekeeper;

// Global Variables

//! Scorekeeper list.
extern Scorekeeper *scorekeepers;
//! Number of scorekeepers.
extern int numscorekeepers;

//! Number of scores being kept.
extern int numscores;

//! Update any scores after action?
extern int any_post_action_scores;
//! Update any scores after event?
extern int any_post_event_scores;

// Queries

//! Get scorekeeper from ID.
extern Scorekeeper *find_scorekeeper(int id);

// Lifecycle Management

//! Create new scorekeeper data structure.
extern Scorekeeper *create_scorekeeper(void);

// GDL I/O

//! Read scorekeeper from GDL form.
extern void interp_scorekeeper(Obj *form);

//! Serialize scorekeeper to GDL.
extern void write_scorekeepers(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SCORE_H

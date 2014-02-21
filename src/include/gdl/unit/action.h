// xConq
// Lifecycle management and serlialization/deserialization for unit actions.

// $Id: action.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Lifecycle management and serlialization/deserialization for unit actions.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_ACTION_H
#define XCONQ_GDL_UNIT_ACTION_H

#include "gdl/lisp.h"
#include "gdl/unit/unit_FWD.h"
#include "gdl/side/side_FWD.h"

// Global Constant Macros

//! Maximum number of arguments to action.
#define MAXACTIONARGS 4

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifdef  DEF_ACTION
#undef  DEF_ACTION
#endif
#define DEF_ACTION(name,CODE,args,prepfn,netprepfn,dofn,checkfn,argdecl,doc)    CODE,

//! Unit action types.
typedef enum actiontype {

#include "gdl/unit/action.def"

    NUMACTIONTYPES

} ActionType;

//! Action definition.
typedef struct a_actiondefn {
    //! Action type ID.
    ActionType typecode;
    //! Name of action.
    char *name;
    //! String encoding of argument types.
    char *argtypes;
} ActionDefn;

//! Action.
typedef struct a_action {
    //! Action type ID.
    ActionType type;
    //! Parameter slots.
    int args[MAXACTIONARGS];
    //! ID of unit being affected by action.
    int actee;
    //! Next action.
    struct a_action *next;
} Action;

//! Unit's actor state.
typedef struct a_actorstate {
    //! How much ACP available at beginning of turn?
    short initacp;
    //! How much ACP left?
    short acp;
    //! Actions performed this turn.
    short actualmoves;
    //! Next action to perform.
    Action nextaction;
} ActorState;

// Global Variables: Action Definitions

//! Action definitions array.
extern ActionDefn actiondefns[];

// Actions

#ifdef  DEF_ACTION
#undef  DEF_ACTION
#endif
#define DEF_ACTION(name,code,args,prepfn,netprepfn,DOFN,checkfn,ARGDECL,doc)  \
    extern int DOFN ARGDECL;
#include "gdl/unit/action.def"

// Queries

//! Can unit act?
extern int can_be_actor(Unit *unit);

// Lifecycle Management

//! Initialize or alter unit's actorstate.
/*! Alter the actorstate object to be correct for this unit. Allow for
    artificially injecting ACP, since this is useful in cases when an unit
    is "enabled" by an occupant, but the occupant has not yet been read in
    during game restoration.
*/
extern void init_unit_actorstate(Unit *unit, int flagacp, int acp = 0);

// GDL I/O

//! Read unit action from GDL form.
extern void interp_action(Action *action, Obj *props);

//! Serialize action to GDL.
extern void write_action(Action *action, int id);
//! Serialize unit actor state to GDL.
extern void write_unit_act(Unit *unit);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_ACTION_H

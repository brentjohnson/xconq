// xConq
// Lifecycle management and serialization/deserialization of unit goals.

// $Id: goal.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2005-2006   Eric A. McDonald

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
    \brief Lifecycle management and serialization/deserialization of unit goals.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_GOAL_H
#define XCONQ_GDL_UNIT_GOAL_H

#include "gdl/side/side_FWD.h"

// Global Constant Macros

//! Maximum number of goal arguments.
#define MAXGOALARGS 5

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifdef  DEF_GOAL
#undef  DEF_GOAL
#endif
#define DEF_GOAL(name,dname,GOALTYPE,args) GOALTYPE,

//! Goal Type.
/*! The different types of goals.
    A goal is a predicate object that can be tested to see whether it has
    been achieved.  As such, it is a relatively static object and may be
    shared.
*/
enum goaltype {

#include "gdl/unit/goal.def"

    g_t_dummy

};
typedef enum goaltype GoalType;

//! Goal definition.
typedef struct a_goaldefn {
    //! Internal name.
    char *name;
    //! Displayable name.
    char *display_name;
    //! String encoding of parameter types.
    char *argtypes;
} GoalDefn;

//! Goal.
typedef struct a_goal {
    //! Type ID of goal.
    GoalType type;
    //! Goal achieved? True/false.
    short tf;
    //! Side which has goal.
    Side *side;
    //! Parameter slots.
    short args[MAXGOALARGS];
} Goal;

// Global Variables: Goal Definitions

//!  Goal definitions array.
extern GoalDefn goaldefns[];

// Queries

//! Description of goal.
extern char *goal_desig(Goal *goal);

// Lifecycle Management

// Create goal of given side for given side, and set its achievement flag.
extern Goal *create_goal(GoalType type, Side *side, int tf);

// GDL I/O

//! Read unit goal from GDL form.
extern Goal *interp_goal(Obj *form);

//! Serialize unit goal to GDL.
extern void write_goal(Goal *goal, int keyword);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_GOAL_H

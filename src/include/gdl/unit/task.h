// xConq
// Lifecycle management and serialization/deserialization of unit tasks.

// $Id: task.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and serialization/deserialization of unit tasks.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UNIT_TASK_H
#define XCONQ_GDL_UNIT_TASK_H

#include "gdl/lisp.h"
#include "gdl/unit/unit_FWD.h"

// Global Constant Macros: Task Parameters

//! Maximum arguments for a task.
#define MAXTASKARGS 6

// Function Macros: Queries

//! Is argument a task type?
#define is_task_type(x) (between(0, (x), NUMTASKTYPES - 1))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifdef  DEF_TASK
#undef  DEF_TASK
#endif
#define DEF_TASK(name,dname,CODE,argtypes,dofn,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl)   CODE,

//! Task type.
/*! A task is a single executable element of a unit's plan.  Each task
    type is something that has been found useful or convenient to
    encapsulate as a step in a plan.  Tasks are also useful for human
    interfaces to use instead of actions, since they have retry
    capabilities that are not part of action execution.
    Note that a task's arguments are not the same as the arguments
    passed to the functions that create and push tasks.  For instance,
    the 'c' task argument is actually a state variable recording a
    direction choice; this is important for coherent routefinding, but
    is not needed in order to create a movement task.
*/
typedef enum a_tasktype {

#include "gdl/unit/task.def"

    NUMTASKTYPES

} TaskType;

//! Task outcome.
typedef enum a_taskoutcome {
    //! Unkonwn task.
    TASK_UNKNOWN,
    //! Task failed.
    TASK_FAILED,
    //! Task incomplete.
    TASK_IS_INCOMPLETE,
    //! Task prepped action.
    TASK_PREPPED_ACTION,
    //! Task complete.
    TASK_IS_COMPLETE
} TaskOutcome;

//! Task.
typedef struct a_task {
    //! Type ID of task.
    TaskType type;
    //! Parameter slots.
    int args[MAXTASKARGS];
    //! How many times task has been run.
    short execnum;
    //! How many failures so far.
    short retrynum;
    //! Next task.
    struct a_task *next;
} Task;

//! Task definition.
typedef struct a_taskdefn {
    //! Internal name of task type.
    char *name;
    //! Display name of task type.
    char *display_name;
    //! Task args encoding string.
    char *argtypes;
    //! Task function.
    TaskOutcome (*exec)(Unit *unit, Task *task);
} TaskDefn;

// Global Variables: Tasks

//!  Task definitions array.
extern TaskDefn taskdefns[];

// Tasks

#ifdef  DEF_TASK
#undef  DEF_TASK
#endif
#define DEF_TASK(name,dname,code,argtypes,DOFN,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl)  \
  extern TaskOutcome DOFN(Unit *unit, Task *task);

#include "gdl/unit/task.def"

// Queries

//! Get task type from name.
extern int lookup_task_type(char *name);

//! Place task description in buffer.
/*!
    Describe a task in a brief but intelligible way.  Note that the
    given side and/or unit may be NULL, such as when describing a task
    not yet assigned to a unit (standing orders for instance).
*/
extern void task_desc(char *buf, Side *side, Unit *unit, Task *task);
//! Describe task.
/*! Intended for debugging. */
extern char *task_desig(Task *task);

// Lifecycle Management

//! Create new task.
extern Task *create_task(TaskType type);

//! Put task back onto list of free tasks.
extern void free_task(Task *task);

// GDL I/O

//! Read task from GDL form.
extern Task *interp_task(Obj *form);

//! Serialize task to GDL.
extern void write_task(Task *task);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UNIT_TASK_H

// xConq
// Lifecycle management and ser/deser for unit tasks.

// $Id: task.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser for unit tasks.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/dir.h"
#include "gdl/gamearea/area.h"
#include "gdl/side/side.h"

#ifndef INITMAXTASKS
//! Number of tasks in allocation block.
/*!
    This is the number of tasks to allocate initially.  More will always be
    allocated as needed, so this should be a "reasonable" value.
*/
#define INITMAXTASKS    100
#endif

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Task Definitions

TaskDefn taskdefns[] = {

#ifdef  DEF_TASK
#undef  DEF_TASK
#endif
#define DEF_TASK(NAME,DNAME,code,ARGTYPES,DOFN,createfn,setfn,netsetfn,pushfn,netpushfn,argdecl)  \
  { NAME, DNAME, ARGTYPES, DOFN },

#include "gdl/unit/task.def"

    { NULL, NULL, NULL }

};

// Local Variables

//! List of available task objects.
static Task *freetasks;

// Local Variables: Buffers

//! Buffer that task debug info goes into.
static char *taskbuf;

// Local Functions: Lifecycle Management

//! Allocate new block of tasks.
static void allocate_task_block(void);

// Queries

int
lookup_task_type(char *name)
{
    int i;

    for (i = 0; taskdefns[i].name != NULL; ++i)
      if (strcmp(name, taskdefns[i].name) == 0)
	return i; /* should get real enum? */
    return -1;
}

void
task_desc(char *buf, Side *side, Unit *unit, Task *task)
{
    int i, slen, arg0, arg1, arg2, arg3, arg4, tp;
    char *argtypes, *sidedesc, *utypedesc;
    Unit *unit2;

    if (task == NULL) {
	buf[0] = '\0';
	return;
    }
    arg0 = task->args[0];
    arg1 = task->args[1];
    arg2 = task->args[2];
    arg3 = task->args[3];
    arg4 = task->args[4];
    sprintf(buf, "%s ", taskdefns[task->type].display_name);
    switch (task->type) {
      case TASK_BUILD:
	if (arg0 != 0) {
	    unit2 = find_unit(arg0);
	    if (unit2 != NULL) {
		tprintf(
		    buf, "%s: %d/%d done", medium_long_unit_handle(unit2),
		    unit2->cp, u_cp(unit2->type));
	    }
	}
#if (0)
	else if (is_unit_type(arg0)) {
	    tp = (unit->tooling ? unit->tooling[arg0] : 0);
	    if (tp < uu_tp_to_build(unit->type, arg0)) {
		tprintf(buf, " (tooling up: %d/%d)", tp,
			uu_tp_to_build(unit->type, arg0));
	    }
	}
	if (arg3 > 1) {
	    tprintf(buf, " (%d%s of %d)", arg2 + 1, ordinal_suffix(arg2 + 1),
		    arg3);
	}
#endif
	break;
      case TASK_CAPTURE:
	unit2 = find_unit(arg0);
	tprintf(buf, "%s", medium_long_unit_handle(unit2));
	if (in_play(unit2))
	    tprintf(buf, " at (%d,%d)", unit2->x, unit2->y);
	break;
      case TASK_COLLECT:
	tprintf(buf, "%s around ", m_type_name(arg0));
	tprintf(buf, "%d,%d", arg1, arg2);
	break;
      case TASK_CONSTRUCT:
	tprintf(buf, "%d %s ", arg1, u_type_name(arg0));
	unit2 = find_unit(arg2);
	if (in_play(unit2))
	    tprintf(buf, "in %s",
		    unit == unit2 ? "self" : medium_long_unit_handle(unit2));
	else
	    tprintf(buf, "at %d,%d", arg3, arg4);
	break;
      case TASK_DEVELOP:
	tprintf(buf, "tech for %s: %d/%d", u_type_name(arg0),
		(side ? side->tech[arg0] : 0), arg1);
	break;
      case TASK_HIT_POSITION:
	tprintf(buf, "at %d,%d", arg0, arg1);
	break;
      case TASK_HIT_UNIT:
	unit2 = find_unit(arg0);
	tprintf(buf, "%s", medium_long_unit_handle(unit2));
	if (in_play(unit2))
	    tprintf(buf, " at (%d,%d)", unit2->x, unit2->y);
	break;
      case TASK_MOVE_DIR:
	tprintf(buf, "%s, %d times", dirnames[arg0], arg1);
	break;
      case TASK_MOVE_TO:
	if (unit != NULL && unit->x == arg0 && unit->y == arg1) {
	    tprintf(buf, "here");
	} else {
	    if (arg3 == 0) {
		/* do nothing */
	    } else if (arg3 == 1) {
		tprintf(buf, "adj ");
	    } else {
		tprintf(buf, "within %d of ", arg3);
	    }
	    tprintf(buf, "%d,%d", arg0, arg1);
	}
        break;
      case TASK_OCCUPY:
	unit2 = find_unit(arg0);
	if (unit2 != NULL) {
	    tprintf(buf, "%s ", medium_long_unit_handle(unit2));
	    if (!empty_string(unit2->name)) {
		tprintf(buf, "(%d,%d)", unit2->x, unit2->y);
	    } else {
	   	tprintf(buf, "at %d,%d", unit2->x, unit2->y);
	    }
	} else {
	    tprintf(buf, "unknown unit #%d", arg0);
	}
	break;
      case TASK_PICKUP:
	unit2 = find_unit(arg0);
	if (unit2 != NULL) {
	    tprintf(buf, "%s", medium_long_unit_handle(unit2));
	} else {
	    tprintf(buf, "unknown unit #%d", arg0);
	}
	break;
      case TASK_REPAIR:
	unit2 = find_unit(arg0);
	if (!unit2)
	    tprintf(buf, "unknown unit #%d", arg0);
	else if (unit2->id == unit->id)
	    tprintf(buf, "self");
	else
	    tprintf(buf, "%s", medium_long_unit_handle(unit2));
	break;
      case TASK_RESUPPLY:
	if (arg0 != NONMTYPE)
	  tprintf(buf, "%s", m_type_name(arg0));
	else
	  tprintf(buf, "all");
	if (arg1 != 0) {
	    unit2 = find_unit(arg1);
	    if (unit2 != NULL) {
		tprintf(buf, " at %s", short_unit_handle(unit2));
	    }
	}
	break;
      case TASK_SENTRY:
	/* (should display as calendar time) */
	tprintf(buf, "for %d turns", arg0);
	break;
      case TASK_PRODUCE:
	tprintf(buf, "%s (%d/%d)", mtypes[task->args[0]].name, task->args[2],
		task->args[1]);
	break;
      default:
	/* Default is just to dump out the raw data about the task. */
	tprintf(buf, "raw");
	argtypes = taskdefns[task->type].argtypes;
	slen = strlen(argtypes);
	for (i = 0; i < slen; ++i) {
	    tprintf(buf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	break;
    }
    if (Debug) {
	/* Include the number of executions and retries. */
	tprintf(buf, " x %d", task->execnum);
	if (task->retrynum > 0) {
	    tprintf(buf, " fail %d", task->retrynum);
	}
    }
}

char *
task_desig(Task *task)
{
    int i, slen;
    char *argtypes;

    if (taskbuf == NULL)
      taskbuf = (char *)xmalloc(BUFSIZE);
    if (task) {
	sprintf(taskbuf, "{%s", taskdefns[task->type].name);
	argtypes = taskdefns[task->type].argtypes;
	slen = strlen(argtypes);
	for (i = 0; i < slen; ++i) {
	    tprintf(taskbuf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	tprintf(taskbuf, " x %d", task->execnum);
	if (task->retrynum > 0) {
	    tprintf(taskbuf, " fail %d", task->retrynum);
	}
	strcat(taskbuf, "}");
    } else {
	sprintf(taskbuf, "no task");
    }
    return taskbuf;
}

// Lifecycle Management

void
allocate_task_block(void)
{
    int i;

    freetasks = (Task *) xmalloc(INITMAXTASKS * sizeof(Task));
    /* Chain the tasks together. */
    for (i = 0; i < INITMAXTASKS; ++i) {
	freetasks[i].next = &freetasks[i+1];
    }
    freetasks[INITMAXTASKS-1].next = NULL;
}

Task *
create_task(TaskType type)
{
    int i;
    Task *task;

    /* Scarf up some more memory if we need it. */
    if (freetasks == NULL) {
	allocate_task_block();
    }
    /* Peel off a task from the free list. */
    task = freetasks;
    freetasks = task->next;
    /* Reset its slots. */
    task->type = type;
    task->execnum = 0;
    task->retrynum = 0;
    for (i = 0; i < MAXTASKARGS; ++i)
      task->args[i] = 0;
    task->next = NULL;
    return task;
}

void
free_task(Task *task)
{
    task->next = freetasks;
    freetasks = task;
}

// GDL I/O

Task *
interp_task(Obj *form)
{
    int tasktype, numargs, i;
    char *tasktypename, *argtypes;
    Obj *tasktypesym;
    Task *task;

    tasktypesym = car(form);
    SYNTAX_RETURN(form, symbolp(tasktypesym), "task type must be a symbol",
		  NULL);
    tasktypename = c_string(tasktypesym);
    tasktype = lookup_task_type(tasktypename);
    if (tasktype < 0) {
	read_warning("Task type `%s' not recognized", tasktypename);
	return NULL;
    }
    task = create_task((TaskType)tasktype);
    form = cdr(form);
    if (numberp(car(form))) {
	task->execnum = c_number(car(form));
	form = cdr(form);
    }
    if (numberp(car(form))) {
	task->retrynum = c_number(car(form));
	form = cdr(form);
    }
    argtypes = taskdefns[tasktype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	SYNTAX_RETURN(form, numberp(car(form)), "task arg must be a number", NULL);
	task->args[i] = c_number(car(form));
	form = cdr(form);
    }
    /* Warn about unused data, but not a serious problem. */
    if (form != lispnil)
      read_warning("Excess args for task %s", task_desig(task));
    return task;
}

void
write_task(Task *task)
{
    int i, numargs;
    char *argtypes;

    if (!is_task_type(task->type)) {
	run_warning("Bad task type %d while writing, skipping it", task->type);
	return;
    }
    start_form(taskdefns[task->type].name);
    add_num_to_form(task->execnum);
    add_num_to_form(task->retrynum);
    argtypes = taskdefns[task->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(task->args[i]);
    end_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

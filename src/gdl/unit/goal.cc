// xConq
// Lifecycle management and ser/deser of unit goals.

// $Id: goal.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of unit goals.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/unit/unit.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Goal Definitions

//! Goal definitions.
/*!
    \showinitializer
*/
GoalDefn goaldefns[] = {

#ifdef  DEF_GOAL
#undef  DEF_GOAL
#endif
#define DEF_GOAL(NAME,DNAME,code,ARGTYPES) { NAME, DNAME, ARGTYPES },

#include "gdl/unit/goal.def"

    { NULL, NULL, NULL }

};

// Local Variables: Buffers

char *goalbuf = NULL;

// Local Functions: Queries

//! Get goal type from name.
static int lookup_goal_type(char *name);

// Queries

static
int
lookup_goal_type(char *name)
{
    int i;

    for (i = 0; goaldefns[i].name != NULL; ++i)
      if (strcmp(name, goaldefns[i].name) == 0)
	return i; /* should get real enum? */
    return GOAL_NO;
}

char *
goal_desig(Goal *goal)
{
    int numargs, i, arg;
    char *argtypes;

    if (goal == NULL)
      return "<null goal>";
    if (goalbuf == NULL)
      goalbuf = (char *)xmalloc(BUFSIZE);
    sprintf(goalbuf, "<goal s%d %s%s",
	    side_number(goal->side), (goal->tf ? "" : "not "),
	    goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	arg = goal->args[i];
	switch (argtypes[i]) {
	  case 'h':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  case 'm':
	    if (is_material_type(arg))
	      tprintf(goalbuf, " %s", m_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'S':
	    tprintf(goalbuf, " `%s'", side_desig(side_n(arg)));
	    break;
	  case 'u':
	    if (is_unit_type(arg))
	      tprintf(goalbuf, " %s", u_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'U':
	    tprintf(goalbuf, " `%s'", unit_desig(find_unit(arg)));
	    break;
	  case 'w':
	    tprintf(goalbuf, " %dx", arg);
	    break;
	  case 'x':
	    tprintf(goalbuf, " %d,", arg);
	    break;
	  case 'y':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  default:
	    tprintf(goalbuf, " %d", arg);
	    break;
	}
    }
    strcat(goalbuf, ">");
    return goalbuf;
}

// Lifecycle Management

Goal *
create_goal(GoalType type, Side *side, int tf)
{
    Goal *goal = (Goal *) xmalloc(sizeof(Goal));

    goal->type = type;
    goal->side = side;
    goal->tf = tf;
    return goal;
}

// GDL I/O

Goal *
interp_goal(Obj *form)
{
    int goaltype, tf, numargs, i;
    char *argtypes;
    Obj *goaltypesym;
    Goal *goal;
    Side *side;

    SYNTAX_RETURN(form, numberp(car(form)), "goal side must be a number", NULL);
    side = side_n(c_number(car(form)));
    form = cdr(form);
    SYNTAX_RETURN(form, numberp(car(form)), "goal tf must be a number", NULL);
    tf = c_number(car(form));
    form = cdr(form);
    goaltypesym = car(form);
    SYNTAX_RETURN(form, symbolp(goaltypesym), "goal type must be a symbol", NULL);
    goaltype = lookup_goal_type(c_string(goaltypesym));
    goal = create_goal((GoalType)goaltype, side, tf);
    form = cdr(form);
    argtypes = goaldefns[goaltype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	if (argtypes[i] == 'U'
	    && (stringp(car(form)))) {
	    /* (should find unit by name) */
	} else {
	    SYNTAX_RETURN(form, numberp(car(form)), "goal arg must be a number", NULL);
	}
	goal->args[i] = c_number(car(form));
	form = cdr(form);
    }
    /* Warn about unused data, but not a serious problem. */
    if (form != lispnil)
      read_warning("Excess args for goal %s", goal_desig(goal));
    return goal;
}

void
write_goal(Goal *goal, int keyword)
{
    int i, numargs;
    char *argtypes;

    space_form();
    start_form(key((enum keywords)keyword));
    add_num_to_form(side_number(goal->side));
    add_num_to_form(goal->tf);
    add_to_form(goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(goal->args[i]);
    end_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

// xConq
// Lifecycle management and ser/deser of unit actions.

// $Id: action.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1986-1989   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of unit actions.
    \ingroup grp_gdl
*/

#include "gdl/tables.h"
#include "gdl/unit/unit.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

//! Action function table.
/*!
    The table of all the action functions declared above.  It uses
    CODE_DEF_ACTION and action.def above to generate an "extern int"
    declaration for every action (prep, do, and * check).  Below it
    generates a table with 3 entries for each function, a code number
    (CODE), the name of the function (NAME), and the extended
    declaration list for the function (ARGS).
*/
ActionDefn actiondefns[] = {

#ifdef  DEF_ACTION
#undef  DEF_ACTION
#endif
#define DEF_ACTION(NAME,CODE,ARGS,prepfn,netprepfn,dofn,checkfn,argdecl,doc)  \
    { CODE, NAME, ARGS },

#include "gdl/unit/action.def"

    { (ActionType)-1, NULL, NULL }

};

// Local Functions: Queries

//! Get action type from name.
static int lookup_action_type(char *name);

// Queries

static
int
lookup_action_type(char *name)
{
    int i;

    for (i = 0; actiondefns[i].name != NULL; ++i)
      if (strcmp(name, actiondefns[i].name) == 0)
	return i;
    return -1;
}

int
can_be_actor(Unit *unit)
{
    int factor = FALSE;
    Unit *occ = NULL;
    int t = NONTTYPE;

    assert_error(unit, "Attempted to access a null unit.");
    if (unit->cp < 0)
      return FALSE;
    /* Could the unit get acp every turn? */
    factor = (u_acp(unit->type) > 0);
    /* Is the unit ACP-independent? ACP-indep units still need plan. */
    if (!factor)
      factor = acp_indep(unit);
    /* Does an occupant give the unit ACP? */
    if (!factor) {
	for_all_occupants(unit, occ) {
	    if (0 < uu_occ_adds_acp(unit->type, occ->type)) {
		factor = TRUE;
		break;
	    }
	}
    }
    /* Does the unit's terrain at night give the unit ACP? */
    if (!factor && in_play(unit) && night_at(unit->x, unit->y)) {
	t = terrain_at(unit->x, unit->y);
	factor = (0 < ut_night_adds_acp(unit->type, t));
	/* (Should also consider connectors and coatings.) */
    }
    return factor;
}

// Lifecycle Management

void
init_unit_actorstate(Unit *unit, int flagacp, int acp)
{
    if (can_be_actor(unit) || (0 < acp)) {
	/* Might already have an actorstate, don't realloc if so;
	   but do clear an existing actorstate, since might be reusing. */
	if (unit->act == NULL)
	  unit->act = (ActorState *) xmalloc(sizeof(ActorState));
	else
	  memset((char *) unit->act, 0, sizeof(ActorState));
	/* Indicate that the action points have not been set. */
	if (flagacp)
	  unit->act->acp = unit->act->initacp = u_acp_min(unit->type) - 1;
	/* Flag the action as undefined. */
	unit->act->nextaction.type = ACTION_NONE;
    } else {
	if (unit->act != NULL)
	  free(unit->act);
	unit->act = NULL;
    }
}

// GDL I/O

void
interp_action(Action *action, Obj *form)
{
    int atype, i, numargs;
    char *argtypes;
    Obj *actiontypesym, *arg;

    actiontypesym = car(form);
    form = cdr(form);
    atype = lookup_action_type(c_string(actiontypesym));
    if (atype < 0) {
	/* (should complain about this?) */
	return;
    }
    action->type = (ActionType)atype;
    argtypes = actiondefns[atype].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	if (form == lispnil)
	  break;
	arg = car(form);
	SYNTAX(arg, numberp(arg), "action arg must be a number");
	action->args[i] = c_number(arg);
	form = cdr(form);
    }
    if (form != lispnil) {
	/* (should allow non-ids also) */
	action->actee = c_number(car(form));
	form = cdr(form);
    }
    /* (should complain about any leftovers) */
}

void
write_action(Action *action, int id)
{
    int atype, i, slen;

    atype = action->type;
    start_form(actiondefns[atype].name);
    slen = strlen(actiondefns[atype].argtypes);
    for (i = 0; i < slen; ++i)
      add_num_to_form(action->args[i]);
    if (action->actee != 0 && action->actee != id)
      add_num_to_form(action->actee);
    end_form();
}

void
write_unit_act(Unit *unit)
{
    int acp = u_acp(unit->type);
    ActorState *act = unit->act;

    /* Actor state is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (act != NULL
	&& (act->acp != acp
	    || act->initacp != acp
	    || act->nextaction.type != ACTION_NONE
	    || act->actualmoves)) {
	if (1) {
	   newline_form();
	   space_form();
	}
	write_num_prop(key(K_ACP), act->acp, acp, FALSE, FALSE);
	write_num_prop(key(K_ACP0), act->initacp, acp, FALSE, FALSE);
	write_num_prop(key(K_AM), act->actualmoves, 0, FALSE, FALSE);
	if (act->nextaction.type != ACTION_NONE) {
	    space_form();
	    start_form(key(K_A));
	    space_form();
	    write_action(&(act->nextaction), unit->id);
	    end_form();
	}
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

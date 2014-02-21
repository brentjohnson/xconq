// xConq
// Lifecycle management and ser/deser of per-side standing orders.

// $Id: sorder.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of per-side standing orders.
    \ingroup grp_gdl
*/

#include "gdl/ui.h"
#include "gdl/types.h"
#include "gdl/unit/task.h"
#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Function Declarations: Queries

static int order_conds_match(StandingOrder *sorder, StandingOrder *sorder2);

// Queries

char *
standing_order_desc(StandingOrder *sorder, char *buf)
{
    int u, v = -1, i = 1;
    int *args;

    for_all_unit_types(u) {
	if (sorder->types[u]) {
	    if (v < 0)
	      v = u;
	} else {
	    i = 0;
	}
    }
    if (v < 0 || sorder->task == NULL) {
	strcpy(buf, "invalid");
	return buf;
    }
    sprintf(buf, "if %s ", (i ? "all" : u_type_name(v)));

    switch (sorder->condtype) {
    case sorder_at:
	tprintf(buf, "at %d,%d ", sorder->a1, sorder->a2);
	break;
    case sorder_in:
	tprintf(buf, "in %s ", short_unit_handle(find_unit(sorder->a1)));
	break;
    case sorder_near:
	tprintf(buf, "within %d %d,%d ", sorder->a3,  sorder->a1, sorder->a2);
	break;
    default:
	strcat(buf, "unknown");
	return buf;
    }

    i = sorder->task->type;
    args = sorder->task->args;
    switch (i) {
    case TASK_MOVE_TO:
	tprintf(buf, "%s %d,%d", taskdefns[i].display_name, args[0], args[1]);
	break;
    case TASK_SENTRY:
	tprintf(buf, "%s %d", taskdefns[i].display_name, args[0]);
	break;
    default:
	task_desc(buf+strlen(buf), NULL, NULL, sorder->task);
	break;
    }
    return buf;
}

static
int
order_conds_match(StandingOrder *sorder, StandingOrder *sorder2)
{
    return (sorder->condtype == sorder2->condtype
	    && sorder->a1 == sorder2->a1
	    && sorder->a2 == sorder2->a2
	    && sorder->a3 == sorder2->a3);
}

// Lifecycle Management

StandingOrder *
new_standing_order(void)
{
    StandingOrder *sorder;

    sorder = (StandingOrder *) xmalloc(sizeof(StandingOrder));
    sorder->types = (char *)xmalloc(numutypes);
    return sorder;
}

void
add_standing_order(Side *side, StandingOrder *sorder, int pos)
{
    StandingOrder *sorder2, *saved;

    if (sorder->task == NULL) {
	/* Cancelling an order. */
	saved = NULL;
	if (side->orders == NULL) {
	    /* No orders, so nothing to do. */
	    notify(side, "No orders to cancel");
	} else if (order_conds_match(sorder, side->orders)) {
	    /* Delete the first order in the list. */
	    saved = side->orders;
	    if (side->last_order == side->orders)
	      side->last_order = side->orders->next;
	    side->orders = side->orders->next;
	} else {
	    for (sorder2 = side->orders; sorder2->next != NULL; sorder2 = sorder2->next) {
		if (order_conds_match(sorder, sorder2->next)) {
		    saved = sorder2->next;
		    if (side->last_order == sorder2->next)
		      side->last_order = sorder2->next->next;
		    sorder2->next = sorder2->next->next;
		    break;
		}
	    }
	    /* If we're here, no match; might be user error, but can't be sure,
	       so don't say anything. */
	}
	if (saved != NULL) {
	    notify(side, "Cancelled order `%s'", standing_order_desc(saved, spbuf));
	}
    } else if (pos == 0) {
	/* Add order to front of list. */
	sorder->next = side->orders;
	side->orders = sorder;
	if (side->last_order == NULL)
	  side->last_order = sorder;
    } else if (side->last_order != NULL) {
	/* Add order to end of list. */
	side->last_order->next = sorder;
	side->last_order = sorder;
    } else {
	/* First standing order for the side. */
	side->orders = side->last_order = sorder;
    }
}

// GDL I/O

void
interp_standing_order(Side *side, Obj *form)
{
    enum sordercond condtyp;
    int u;
    char *condname;
    Obj *rest = form, *subform;
    StandingOrder *sorder;

    if (rd_u_arr == NULL)
      rd_u_arr = (short *) xmalloc(numutypes * sizeof(short));

    sorder = new_standing_order();
    for_all_unit_types(u)
      rd_u_arr[u] = FALSE;
    interp_utype_list(rd_u_arr, car(rest));
    for_all_unit_types(u)
      sorder->types[u] = rd_u_arr[u];
    rest = cdr(rest);
    subform = car(rest);
    condname = c_string(car(subform));
    switch (keyword_code(condname)) {
      case K_ALWAYS:
	condtyp = sorder_always;
        break;
      case K_AT:
	condtyp = sorder_at;
        break;
      case K_IN:
	condtyp = sorder_in;
        break;
      case K_NEAR:
	condtyp = sorder_near;
        break;
      default:
	unknown_property("standing order condition", side_desig(side), condname);
	return;
    }
    sorder->condtype = condtyp;
    subform = cdr(subform);
    if (subform != lispnil) {
	sorder->a1 = c_number(car(subform));
	subform = cdr(subform);
    }
    if (subform != lispnil) {
	sorder->a2 = c_number(car(subform));
	subform = cdr(subform);
    }
    if (subform != lispnil) {
	sorder->a3 = c_number(car(subform));
    }
    rest = cdr(rest);
    sorder->task = interp_task(car(rest));

    add_standing_order(side, sorder, 1);
}

void
write_standing_orders(Side *side)
{
    int u, u1, numtypes, numargs;
    char *str = NULL;
    StandingOrder *sorder;

    for (sorder = side->orders; sorder != NULL; sorder = sorder->next) {
	space_form();
	start_form(key(K_STANDING_ORDER));
	/* (should break into a write_utype_list) */
	numtypes = u1 = 0;
	for_all_unit_types(u) {
	    if (sorder->types[u]) {
		++numtypes;
		u1 = u;
	    }
	}
	if (numtypes == 1) {
	    add_to_form(shortest_escaped_name(u1));
	} else if (numtypes == numutypes) {
	    add_to_form(key(K_USTAR));
	} else {
	    space_form();
	    start_form("");
	    for_all_unit_types(u)
	      add_num_to_form(sorder->types[u]);
	    end_form();
	}
	space_form();
	switch (sorder->condtype) {
	  case sorder_always:
	    str = key(K_ALWAYS);
	    numargs = 0;
	    break;
	  case sorder_at:
	    str = key(K_AT);
	    numargs = 2;
	    break;
	  case sorder_in:
	    str = key(K_IN);
	    numargs = 1;
	    break;
	  case sorder_near:
	    str = key(K_NEAR);
	    numargs = 3;
	    break;
	  default:
	    case_panic("standing order condition type", sorder->condtype);
	    break;
	}
	start_form(str);
	if (numargs > 0)
	  add_num_to_form(sorder->a1);
	if (numargs > 1)
	  add_num_to_form(sorder->a2);
	if (numargs > 2)
	  add_num_to_form(sorder->a3);
	end_form();
	space_form();
	write_task(sorder->task);
	end_form();
	newline_form();
	space_form();
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

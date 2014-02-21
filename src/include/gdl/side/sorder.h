// xConq
// Lifecycle management and ser/deser of per-side standing orders.

// $Id: sorder.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

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

#ifndef XCONQ_GDL_SIDE_SORDER_H
#define XCONQ_GDL_SIDE_SORDER_H

#include "gdl/lisp.h"
#include "gdl/side/side_FWD.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Standing order conditions.
/*!
    This modifies a standing order to be limited to a particular location/unit.
*/
enum sordercond {
    //! Apply always.
    sorder_always,
    //! Apply when at.
    sorder_at,
    //! Apply when in.
    sorder_in,
    //! Apply when near.
    sorder_near
};

//! Standing order.
/*!
    A standing order is a conditional order that applies to any matching
    unit not already doing a task.
*/
typedef struct a_standing_order {
    //! Array of unit types to which the order applies.
    char *types;
    //! On what condition to trigger order.
    enum sordercond condtype;
    //! First parameter to test (x, y, etc...).
    int a1;
    //! Second parameter to test (x, y, etc...).
    int a2;
    //! Third parameter to test (x, y, etc...).
    int a3;
    //! Task associated with standing order.
    struct a_task *task;
    //! Next standing order.
    struct a_standing_order *next;
} StandingOrder;

// Queries

//! Get description of standing order.
/*! Intended for debugging use. */
extern char *standing_order_desc(StandingOrder *sorder, char *buf);

// Lifecycle Management

//! Create new standing order.
extern StandingOrder *new_standing_order(void);

//! Add standing order to given side at front or back of list of orders.
/*!
    Add a new standing order for the side.  This function can add to front
    or back of existing list of orders.
*/
extern void add_standing_order(Side *side, StandingOrder *sorder, int pos);

// GDL I/O

//! Read standing order from GDL form.
extern void interp_standing_order(Side *side, Obj *form);

//! Serialize standing orders to GDL.
extern void write_standing_orders(Side *side);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SIDE_SORDER_H

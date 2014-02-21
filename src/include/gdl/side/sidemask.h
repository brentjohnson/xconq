// xConq
// Lifecycle management and ser/deser of side masks.

// $Id: sidemask.h,v 1.2 2006/05/30 02:34:14 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of side masks.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_SIDE_SIDEMASK_H
#define XCONQ_GDL_SIDE_SIDEMASK_H

#include "gdl/lisp.h"

#if (MAXSIDES < 31)

//! Add side to vector.
/*!
    Sets the bit corresponding to the side number,
    or the zero'th bit if the side pointer is nil
    for legacy support of indepsides.
*/
#define add_side_to_set(side,mask) ((mask) | (1 << ((side) ? (side)->id : 0)))

//! Remove side from mask.
/*!
    Resets the bit corresponding to the side number,
    or the zero'th bit if the side pointer is nil
    for legacy support of indepsides.
*/
#define remove_side_from_set(side,mask) \
  ((mask) & ~(1 << ((side) ? (side)->id : 0)))

//! Is side in set?
/*!
    This returns a non-zero value if the side is set.
*/
#define side_in_set(side,mask) ((mask) & (1 << ((side) ? (side)->id : 0)))

#else // (MAXSIDES < 31)
#   error "Too many sides. Set 'MAXSIDES' < 31."
[[[-: ***error "Too many sides. Set 'MAXSIDES' < 31."*** :-]]]
#endif // (MAXSIDES < 31)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

// Right now, we are constrained to less than 31 sides.
#if (MAXSIDES < 31)

//! Side mask.
/*! A side mask is a bit vector, where the bit position corresponds to the
side number.  It currently is implemented on tha basis of a 32 bit
integer, so it cannot support more than 30 players.
*/
typedef Z32 SideMask;

//! No sides is all zero bits.
#define NOSIDES (0)
//! All sides is all ones.
#define ALLSIDES (-1)

#else
#   error "Too many sides. Set 'MAXSIDES' < 31."
[[[-: ***error "Too many sides. Set 'MAXSIDES' < 31."*** :-]]]
#endif // if MAXSIDES > 31

// GDL I/O

//! Read sidemask from GDL list.
extern void interp_side_mask_list(SideMask *mask, Obj *lis);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_SIDE_SIDEMASK_H

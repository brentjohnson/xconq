// xConq
// Packed Boolean Table

// $Id: pbooltbl.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999	    Stanley T. Shebs
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
    \brief Packed boolean table.
    \ingroup grp_util

    \todo Replace with something from STL or else a STL-like 'bittable' class.
*/

#ifndef XCONQ_PBOOLTBL_H
#define XCONQ_PBOOLTBL_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

//! Table of Packed Booleans.
/*!
    This structure contains an array of packed booleans.
*/
typedef struct a_packed_bool_table {
    int rdim1size;      //! Reduced size of the first table dimension.
    int dim2size;       //! Normal size of the second dimension.
    short sizeofint;    //! Precalculated sizeof(int) in bits.
    short whichway;     //! Which packing direction is more efficient?
    int *pbools;        //! Pointer to the packed booleans.
} PackedBoolTable;

//! Create a packed boolean table.
extern PackedBoolTable* create_packed_bool_table(int m, int n);
//! Initialize a packed boolean table.
extern void init_packed_bool_table(PackedBoolTable* tbl);
//! Destroy a packed boolean table.
extern void destroy_packed_bool_table(PackedBoolTable* tbl);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#ifdef get_packed_bool
#undef get_packed_bool
#endif
//! Get a boolean value from a table of packed booleans.
#define get_packed_bool(tbl,m,n)    \
    (((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] & (1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint)) ? TRUE : FALSE) 

#ifdef set_packed_bool
#undef set_packed_bool
#endif
//! Set a boolean value in a table of packed booleans.
#define set_packed_bool(tbl,m,n,boolval)    \
    ((boolval) ? ((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] |= (1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint)) : ((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] &= ~(1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint))) 

#ifdef valid_packed_bool_table
#undef valid_packed_bool_table
#endif
//! Validate a supposed table of packed booleans.
#define valid_packed_bool_table(tbl) \
    (((tbl) != NULL) && ((tbl)->pbools != NULL) && ((tbl)->sizeofint > 0) && ((tbl)->rdim1size >= 0) && ((tbl)->dim2size > 0))

#endif // ifndef XCONQ_PBOOLTBL_H
